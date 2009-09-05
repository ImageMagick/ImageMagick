/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             AAA   V   V  IIIII                              %
%                            A   A  V   V    I                                %
%                            AAAAA  V   V    I                                %
%                            A   A   V V     I                                %
%                            A   A    V    IIIII                              %
%                                                                             %
%                                                                             %
%            Read Microsoft Audio/Visual Interleaved Image Format             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                Nathan Brown                                 %
%                                March  2004                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include <setjmp.h>
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

#if defined(MAGICKCORE_JPEG_DELEGATE)
#define JPEG_INTERNAL_OPTIONS
#if defined(__MINGW32__)
# define XMD_H 1  /* Avoid conflicting typedef for INT32 */
#endif
#undef HAVE_STDLIB_H
#include "jpeglib.h"
#include "jerror.h"
#endif

/*
  Define declarations.
*/
#define ICC_MARKER  (JPEG_APP0+2)
#define ICC_PROFILE  "ICC_PROFILE"
#define IPTC_MARKER  (JPEG_APP0+13)
#define XML_MARKER  (JPEG_APP0+1)
#define MaxBufferExtent  8192

/*
  Typedef declarations.
*/
#if defined(MAGICKCORE_JPEG_DELEGATE)

typedef struct _ErrorManager
{
  Image
    *image;

  jmp_buf
    error_recovery;

  boolean
    verbose;
} ErrorManager;

typedef struct _SourceManager
{
  struct jpeg_source_mgr
    manager;

  Image
    *image;

  unsigned char
    *source_data;

  size_t
    source_length;

  JOCTET
    *buffer;

  boolean
    start_of_blob;
} SourceManager;
#endif


/*
  Typedef declaractions.
*/
typedef struct _AVIInfo
{
  unsigned long
    delay,
    max_data_rate,
    pad_granularity,
    flags,
    total_frames,
    initial_frames,
    number_streams,
    buffer_size,
    width,
    height,
    time_scale,
    data_rate,
    start_time,
    data_length;
} AVIInfo;

typedef struct _BMPInfo
{
  unsigned long
    size,
    width,
    height,
    planes,
    bits_per_pixel;

  char
    compression[5];

  unsigned long
    image_size,
    x_pixels,
    y_pixels,
    number_colors,
    important_colors;
} BMPInfo;

typedef struct _AVIStreamInfo
{
  char
    data_type[5],
    data_handler[5];

  unsigned long
    flags,
    priority,
    initial_frames,
    time_scale,
    data_rate,
    start_time,
    data_length,
    buffer_size,
    quality,
    sample_size;
} AVIStreamInfo;

/*
  MJPEG Information from Microsoft SDK
*/
#if defined(MAGICKCORE_JPEG_DELEGATE)
#ifndef NOJPEGDIB

/* New DIB Compression Defines */
#define JPEG_DIB        mmioFOURCC('J','P','E','G')    /* Still image JPEG DIB biCompression */
#define MJPG_DIB        mmioFOURCC('M','J','P','G')    /* Motion JPEG DIB biCompression     */

/* JPEGColorSpaceID Definitions */
#define JPEG_Y          1       /* Y only component of YCbCr */
#define JPEG_YCbCr      2       /* YCbCr as define by CCIR 601 */
#define JPEG_RGB        3       /* 3 component RGB */

/* Structure definitions */

#if defined(MAGICK_FUTURE)
typedef struct tagJPEGINFOHEADER {
    /* compression-specific fields */
    /* these fields are defined for 'JPEG' and 'MJPG' */
    DWORD       JPEGSize;
    DWORD       JPEGProcess;

    /* Process specific fields */
    DWORD       JPEGColorSpaceID;
    DWORD       JPEGBitsPerSample;
    DWORD       JPEGHSubSampling;
    DWORD       JPEGVSubSampling;
} JPEGINFOHEADER;
#endif

/* Default DHT Segment */

static const unsigned char MJPGDHTSeg[0x1A8] = {
 /* JPEG DHT Segment for YCrCb omitted from MJPG data */
0xFF,0xD8,0xFF,0xC4,0x01,0xA2,
0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x01,0x00,0x03,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
0x08,0x09,0x0A,0x0B,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,
0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,
0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,
0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,
0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,
0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
0xF8,0xF9,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,
0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,
0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A,
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,
0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,
0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
0xF9,0xFA,0xFF,0xD9
};

/* End DHT default */

/* End JPEG */
#endif
#endif

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

#if defined(MAGICKCORE_JPEG_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J P E G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJPEGImage() reads a JPEG image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadJPEGImage method is:
%
%      Image *ReadJPEGImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType EmitMessage(j_common_ptr jpeg_info,int level)
{
  char
    message[JMSG_LENGTH_MAX];

  ErrorManager
    *error_manager;

  Image
    *image;

  (jpeg_info->err->format_message)(jpeg_info,message);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  if (error_manager->verbose != MagickFalse)
    (void) fprintf(stdout,"%s\n",message);
  if (level < 0)
    {
      if ((jpeg_info->err->num_warnings == 0) ||
          (jpeg_info->err->trace_level >= 3))
        ThrowBinaryException(CorruptImageError,(char *) message,
          image->filename);
      jpeg_info->err->num_warnings++;
    }
  else
    if (jpeg_info->err->trace_level >= level)
      ThrowBinaryException(CoderError,(char *) message,image->filename);
  return(MagickTrue);
}

static boolean FillInputBuffer(j_decompress_ptr cinfo)
{
  SourceManager
    *source;

  source=(SourceManager *) cinfo->src;
  if (source->image)
    source->manager.bytes_in_buffer=(size_t) ReadBlob(source->image,
      MagickMin(source->source_length,MaxBufferExtent),source->buffer);
  else
    {
      source->manager.bytes_in_buffer=(size_t) MagickMin(source->source_length,
        MaxBufferExtent);
      (void) CopyMagickMemory(source->buffer,source->source_data,
        source->manager.bytes_in_buffer);
      source->source_data+=source->manager.bytes_in_buffer;
    }
  source->source_length-=source->manager.bytes_in_buffer;
  if (source->manager.bytes_in_buffer == 0)
    {
      if (source->start_of_blob != 0)
        ERREXIT(cinfo,JERR_INPUT_EMPTY);
      WARNMS(cinfo,JWRN_JPEG_EOF);
      source->buffer[0]=(JOCTET) 0xff;
      source->buffer[1]=(JOCTET) JPEG_EOI;
      source->manager.bytes_in_buffer=2;
    }
  source->manager.next_input_byte=source->buffer;
  source->start_of_blob=FALSE;
  return(TRUE);
}

static int GetCharacter(j_decompress_ptr jpeg_info)
{
  if (jpeg_info->src->bytes_in_buffer == 0)
    (void) (*jpeg_info->src->fill_input_buffer)(jpeg_info);
  jpeg_info->src->bytes_in_buffer--;
  return((int) GETJOCTET(*jpeg_info->src->next_input_byte++));
}

static void InitializeSource(j_decompress_ptr cinfo)
{
  SourceManager
    *source;

  source=(SourceManager *) cinfo->src;
  source->start_of_blob=TRUE;
}

static void JPEGErrorHandler(j_common_ptr jpeg_info)
{
  ErrorManager
    *error_manager;

  (void) EmitMessage(jpeg_info,0);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  longjmp(error_manager->error_recovery,1);
}

static boolean ReadComment(j_decompress_ptr jpeg_info)
{
  char
    *comment;

  ErrorManager
    *error_manager;

  Image
    *image;

  register char
    *p;

  register long
    i;

  size_t
    length;

  /*
    Determine length of comment.
  */
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
  length+=GetCharacter(jpeg_info);
  length-=2;
  if (length <= 0)
    return(MagickTrue);
  comment=(char *) NULL;
  if (~length >= MaxTextExtent)
    comment=(char *) AcquireQuantumMemory(length+MaxTextExtent,
      sizeof(*comment));
  if (comment == (char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Read comment.
  */
  i=(long) length-1;
  for (p=comment; i-- >= 0; p++)
    *p=(char) GetCharacter(jpeg_info);
  *p='\0';
  (void) SetImageProperty(image,"comment",comment);
  comment=DestroyString(comment);
  return(MagickTrue);
}

static boolean ReadICCProfile(j_decompress_ptr jpeg_info)
{
  char
    magick[12];

  ErrorManager
    *error_manager;

  Image
    *image;

  MagickBooleanType
    status;

  register long
    i;

  register unsigned char
    *p;

  size_t
    length;

  StringInfo
    *icc_profile,
    *profile;

  /*
    Read color profile.
  */
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  if (length < 2)
    return(MagickFalse);
  length-=2;
  if (length <= 14)
    {
      while (length-- != 0)
        (void) GetCharacter(jpeg_info);
      return(MagickTrue);
    }
  for (i=0; i < 12; i++)
    magick[i]=(char) GetCharacter(jpeg_info);
  if (LocaleCompare(magick,ICC_PROFILE) != 0)
    {
      /*
        Not a ICC profile, return.
      */
      for (i=0; i < (long) (length-12); i++)
        (void) GetCharacter(jpeg_info);
      return(MagickTrue);
    }
  (void) GetCharacter(jpeg_info);  /* id */
  (void) GetCharacter(jpeg_info);  /* markers */
  if (length < 14)
    return(MagickFalse);
  length-=14;
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  profile=AcquireStringInfo(length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  p=GetStringInfoDatum(profile);
  for (i=(long) GetStringInfoLength(profile)-1; i >= 0; i--)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  icc_profile=(StringInfo *) GetImageProfile(image,"icc");
  if (icc_profile != (StringInfo *) NULL)
    ConcatenateStringInfo(icc_profile,profile);
  else
    {
      status=SetImageProfile(image,"icc",profile);
      if (status == MagickFalse)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: ICC, %lu bytes",(unsigned long) length);
  return(MagickTrue);
}

static boolean ReadIPTCProfile(j_decompress_ptr jpeg_info)
{
  char
    magick[MaxTextExtent];

  ErrorManager
    *error_manager;

  Image
    *image;

  MagickBooleanType
    status;

  register long
    i;

  register unsigned char
    *p;

  size_t
    length,
    tag_length;

  StringInfo
    *iptc_profile,
    *profile;

#if defined(GET_ONLY_IPTC_DATA)
  unsigned char
    tag[MaxTextExtent];
#endif

  /*
    Determine length of binary data stored here.
  */
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  if (length <= 2)
    return(MagickTrue);
  length-=2;
  tag_length=0;
#if defined(GET_ONLY_IPTC_DATA)
  *tag='\0';
#endif
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  iptc_profile=(StringInfo *) GetImageProfile(image,"iptc");
  if (iptc_profile == (StringInfo *) NULL)
    {
#if defined(GET_ONLY_IPTC_DATA)
      /*
        Find the beginning of the iptc portion of the binary data.
      */
      for (*tag='\0'; length > 0; )
      {
        *tag=GetCharacter(jpeg_info);
        *(tag+1)=GetCharacter(jpeg_info);
        if (length < 2)
          return(MagickFalse);
        length-=2;
        if ((*tag == 0x1c) && (*(tag+1) == 0x02))
          break;
      }
      tag_length=2;
#else
      /*
        Validate that this was written as a Photoshop resource format slug.
      */
      for (i=0; i < 10; i++)
        magick[i]=(char) GetCharacter(jpeg_info);
      magick[10]='\0';
      if (length <= 10)
        return(MagickTrue);
      length-=10;
      if (LocaleCompare(magick,"Photoshop ") != 0)
        {
          /*
            Not a ICC profile, return.
          */
          for (i=0; i < (long) length; i++)
            (void) GetCharacter(jpeg_info);
          return(MagickTrue);
        }
      /*
        Remove the version number.
      */
      for (i=0; i < 4; i++)
        (void) GetCharacter(jpeg_info);
      if (length <= 4)
        return(MagickTrue);
      length-=4;
      tag_length=0;
#endif
    }
  if (length == 0)
    return(MagickTrue);
  profile=AcquireStringInfo(length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  p=GetStringInfoDatum(profile);
  for (i=(long) GetStringInfoLength(profile)-1; i >= 0; i--)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  iptc_profile=(StringInfo *) GetImageProfile(image,"iptc");
  if (iptc_profile != (StringInfo *) NULL)
    ConcatenateStringInfo(iptc_profile,profile);
  else
    {
      status=SetImageProfile(image,"iptc",profile);
      if (status == MagickFalse)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: iptc, %lu bytes",(unsigned long) length);
  return(MagickTrue);
}

static boolean ReadProfile(j_decompress_ptr jpeg_info)
{
  char
    name[MaxTextExtent];

  ErrorManager
    *error_manager;

  Image
    *image;

  int
    marker;

  MagickBooleanType
    status;

  register long
    i;

  register unsigned char
    *p;

  size_t
    length;

  StringInfo
    *profile;

  /*
    Read generic profile.
  */
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  if (length <= 2)
    return(MagickTrue);
  length-=2;
  marker=jpeg_info->unread_marker-JPEG_APP0;
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  (void) FormatMagickString(name,MaxTextExtent,"APP%d",marker);
  profile=AcquireStringInfo(length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  p=GetStringInfoDatum(profile);
  for (i=(long) GetStringInfoLength(profile)-1; i >= 0; i--)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  if (marker == 1)
    {
      p=GetStringInfoDatum(profile);
      if ((length > 4) && (LocaleNCompare((char *) p,"exif",4) == 0))
        (void) CopyMagickString(name,"iptc",MaxTextExtent);
      if ((length > 5) && (LocaleNCompare((char *) p,"http:",5) == 0))
        (void) CopyMagickString(name,"xmp",MaxTextExtent);
    }
  status=SetImageProfile(image,name,profile);
  if (status == MagickFalse)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: %s, %lu bytes",name,(unsigned long) length);
  return(MagickTrue);
}

static void SkipInputData(j_decompress_ptr cinfo,long number_bytes)
{
  SourceManager
    *source;

  if (number_bytes <= 0)
    return;
  source=(SourceManager *) cinfo->src;
  while (number_bytes > (long) source->manager.bytes_in_buffer)
  {
    number_bytes-=(long) source->manager.bytes_in_buffer;
    (void) FillInputBuffer(cinfo);
  }
  source->manager.next_input_byte+=(size_t) number_bytes;
  source->manager.bytes_in_buffer-=(size_t) number_bytes;
}

static void TerminateSource(j_decompress_ptr cinfo)
{
  cinfo=cinfo;
}

static void JPEGSourceManagerMemory(j_decompress_ptr cinfo,unsigned char *source_data, unsigned long source_length)
{
  SourceManager
    *source;

  cinfo->src=(struct jpeg_source_mgr *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,sizeof(SourceManager));
  source=(SourceManager *) cinfo->src;
  source->buffer=(JOCTET *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,MaxBufferExtent*sizeof(JOCTET));
  source=(SourceManager *) cinfo->src;
  source->manager.init_source=InitializeSource;
  source->manager.fill_input_buffer=FillInputBuffer;
  source->manager.skip_input_data=SkipInputData;
  source->manager.resync_to_restart=jpeg_resync_to_restart;
  source->manager.term_source=TerminateSource;
  source->manager.bytes_in_buffer=0;
  source->manager.next_input_byte=NULL;
  source->image=0;
  source->source_data=source_data;
  source->source_length=source_length;
}

static void JPEGSourceManagerBLOB(j_decompress_ptr cinfo,Image *image, unsigned long source_length)
{
  SourceManager
    *source;

  cinfo->src=(struct jpeg_source_mgr *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,sizeof(SourceManager));
  source=(SourceManager *) cinfo->src;
  source->buffer=(JOCTET *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,MaxBufferExtent*sizeof(JOCTET));
  source=(SourceManager *) cinfo->src;
  source->manager.init_source=InitializeSource;
  source->manager.fill_input_buffer=FillInputBuffer;
  source->manager.skip_input_data=SkipInputData;
  source->manager.resync_to_restart=jpeg_resync_to_restart;
  source->manager.term_source=TerminateSource;
  source->manager.bytes_in_buffer=0;
  source->manager.next_input_byte=NULL;
  source->image=image;
  source->source_data=0;
  source->source_length=source_length;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecodeImage unpacks the packed image pixels into runlength-encoded
%  pixel packets.
%
%  The format of the DecodeImage method is:
%
%      MagickBooleanType DecodeImage(Image *image,
%        const MagickBooleanType compression,unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o compression:  A value of 1 means the compressed pixels are runlength
%      encoded for a 256-color bitmap.  A value of 2 means a 16-color bitmap.
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the decoding process.
%
*/
static MagickBooleanType DecodeImage(Image *image,
  const MagickBooleanType compression,unsigned char *pixels)
{
#if !defined(__WINDOWS__) || defined(__MINGW32__)
#define BI_RLE8  1
#endif

  long
    y;

  register long
    i,
    x;

  register unsigned char
    *p,
    *q;

  ssize_t
    count;

  unsigned char
    byte;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pixels != (unsigned char *) NULL);
  (void) ResetMagickMemory(pixels,0,(size_t) image->columns*image->rows*
    sizeof(*pixels));
  byte=0;
  x=0;
  p=pixels;
  q=pixels+(size_t) image->columns*image->rows;
  for (y=0; y < (long) image->rows; )
  {
    if ((p < pixels) || (p >= q))
      break;
    count=(ssize_t) ReadBlobByte(image);
    if ((int) count == EOF)
      break;
    if (count != 0)
      {
        count=(ssize_t) MagickMin((size_t) count,(size_t) (q-p));
        /*
          Encoded mode.
        */
        byte=(unsigned char) ReadBlobByte(image);
        if (compression == BI_RLE8)
          {
            for (i=0; i < (long) count; i++)
              *p++=(unsigned char) byte;
          }
        else
          {
            for (i=0; i < (long) count; i++)
              *p++=(unsigned char)
                ((i & 0x01) != 0 ? (byte & 0x0f) : ((byte >> 4) & 0x0f));
          }
        x+=count;
      }
    else
      {
        /*
          Escape mode.
        */
        count=(ssize_t) ReadBlobByte(image);
        if (count == 0x01)
          return(MagickTrue);
        switch (count)
        {
          case 0x00:
          {
            /*
              End of line.
            */
            x=0;
            y++;
            p=pixels+y*image->columns;
            break;
          }
          case 0x02:
          {
            /*
              Delta mode.
            */
            x+=ReadBlobByte(image);
            y+=ReadBlobByte(image);
            p=pixels+y*image->columns+x;
            break;
          }
          default:
          {
            /*
              Absolute mode.
            */
            count=(ssize_t) MagickMin((size_t) count,(size_t) (q-p));
            if (compression == BI_RLE8)
              for (i=0; i < (long) count; i++)
                *p++=(unsigned char) ReadBlobByte(image);
            else
              for (i=0; i < (long) count; i++)
              {
                if ((i & 0x01) == 0)
                  byte=(unsigned char) ReadBlobByte(image);
                *p++=(unsigned char)
                  ((i & 0x01) != 0 ? (byte & 0x0f) : ((byte >> 4) & 0x0f));
              }
            x+=count;
            /*
              Read pad byte.
            */
            if (compression == BI_RLE8)
              {
                if ((count & 0x01) != 0)
                  if (ReadBlobByte(image) == EOF)
                    ThrowBinaryException(CorruptImageError,
                      "UnexpectedEndOfFile",image->filename);
              }
            else
              if (((count & 0x03) == 1) || ((count & 0x03) == 2))
                if (ReadBlobByte(image) == EOF)
                  ThrowBinaryException(CorruptImageError,
                    "UnexpectedEndOfFile",image->filename);
            break;
          }
        }
      }
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  if (ReadBlobByte(image) == EOF)
    ThrowBinaryException(CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (ReadBlobByte(image) == EOF)
    ThrowBinaryException(CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s A V I                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsAVI() returns MagickTrue if the image format type, identified by the
%  magick string, is Audio/Video Interleaved file format.
%
%  The format of the IsAVI method is:
%
%      unsigned long IsAVI(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsAVI(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"RIFF",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d A V I I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadAVIImage() reads an Audio Video Interleave image file and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadAVIImage method is:
%
%      Image *ReadAVIImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadAVIImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *DecodeAVIImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image,
    *images;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  register long
    i;

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
  (void) CloseBlob(image);
  (void) DestroyImageList(image);
  /*
    Convert AVI to PPM with delegate.
  */
  image=AcquireImage(image_info);
  read_info=CloneImageInfo(image_info);
  (void) InvokeDelegate(read_info,image,"avi:decode",(char *) NULL,exception);
  image=DestroyImage(image);
  /*
    Read PNG files.
  */
  images=NewImageList();
  for (i=1; ; i++)
  {
    (void) FormatMagickString(read_info->filename,MaxTextExtent,"%08ld.png",i);
    if (IsPathAccessible(read_info->filename) == MagickFalse)
      break;
    image=ReadImage(read_info,exception);
    if (image == (Image *) NULL)
      break;
    (void) CopyMagickString(image->magick,image_info->magick,MaxTextExtent);
    image->scene=(unsigned long) i;
    AppendImageToList(&images,image);
    if (read_info->number_scenes != 0)
      if (i >= (long) (read_info->scene+read_info->number_scenes-1))
        break;
  }
  /*
    Relinquish resources.
  */
  for (i=1; ; i++)
  {
    (void) FormatMagickString(read_info->filename,MaxTextExtent,"%08ld.png",i);
    if (IsPathAccessible(read_info->filename) == MagickFalse)
      break;
    (void) RelinquishUniqueFileResource(read_info->filename);
  }
  read_info=DestroyImageInfo(read_info);
  return(images);
}

static Image *ReadAVIImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  AVIInfo
    avi_info;

  AVIStreamInfo
    stream_info;

  BMPInfo
    bmp_info;

  char
    id[MaxTextExtent];

  Image
    *image;

  IndexPacket
    index;

  long
    y;

  MagickBooleanType
    riff_chunk,
    status;

  MagickOffsetType
    offset;

  PixelPacket
    *colormap;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  register long
    i;

  register unsigned char
    *p;

  ssize_t
    count;

  unsigned char
    *pixels;

  unsigned long
    bit,
    bytes_per_line,
    chunk_size,
    number_colors;

#if defined(MAGICKCORE_JPEG_DELEGATE)
  const char
    *value;

  ErrorManager
    error_manager;

  GeometryInfo
    geometry_info;

  JSAMPLE
    *jpeg_pixels;

  JSAMPROW
    scanline[1];

  struct jpeg_decompress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  unsigned long
    number_pixels,
    units;
#endif

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
  (void) ResetMagickMemory(&avi_info,0,sizeof(AVIInfo));
  (void) ResetMagickMemory(&bmp_info,0,sizeof(BMPInfo));
  colormap=(PixelPacket *) NULL;
  number_colors=0;

#if defined(MAGICKCORE_JPEG_DELEGATE)
  /*
    Initialize JPEG image structure.
  */
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=(void (*)(j_common_ptr,int)) EmitMessage;
  jpeg_info.err->error_exit=(void (*)(j_common_ptr)) JPEGErrorHandler;
  jpeg_pixels=(JSAMPLE *) NULL;
  error_manager.image=image;
  error_manager.verbose=(int) image_info->verbose;
  if (setjmp(error_manager.error_recovery) != 0)
    {
      if (jpeg_pixels != (JSAMPLE *) NULL)
        jpeg_pixels=(unsigned char *) RelinquishMagickMemory(jpeg_pixels);
      jpeg_destroy_decompress(&jpeg_info);
      InheritException(exception,&image->exception);
      number_pixels=image->columns*image->rows;
      if (number_pixels != 0)
        return(GetFirstImageInList(image));
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  jpeg_info.client_data=(void *) &error_manager;
  jpeg_create_decompress(&jpeg_info);
  JPEGSourceManagerMemory(&jpeg_info, (unsigned char *) &MJPGDHTSeg, 0x1A8);
  jpeg_read_header(&jpeg_info, FALSE);
#endif
  for (riff_chunk=MagickTrue; ; riff_chunk=MagickFalse)
  {
    count=ReadBlob(image,4,(unsigned char *) id);
    if (count != 4)
      break;
    id[4]='\0';
    if ((riff_chunk == MagickTrue) && (memcmp(id,"RIFF",4) != 0))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    chunk_size=ReadBlobLSBLong(image);
    if (EOFBlob(image) != MagickFalse)
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    if (chunk_size == 0)
      break;
    if ((chunk_size & 0x01) != 0)
      chunk_size++;
    if ((GetBlobSize(image) != 0) &&
        ((MagickSizeType) chunk_size > GetBlobSize(image)))
      ThrowReaderException(CorruptImageError,"NotEnoughPixelData");
    if (image_info->verbose != MagickFalse)
      {
        (void) fprintf(stdout,"AVI cid %s\n",id);
        (void) fprintf(stdout,"  chunk size %lu\n",chunk_size);
      }
    if ((LocaleCompare(id,"00db") == 0) || (LocaleCompare(id,"00dc") == 0))
      {

        /*
          Read JPG compressed image, uses only YCbCr color space encoding
          and does not include the JPEG Huffman table, as it is pre-defined.
        */
        if (LocaleCompare(bmp_info.compression,"MJPG") == 0)
          {
#if defined(MAGICKCORE_JPEG_DELEGATE)
            JPEGSourceManagerBLOB(&jpeg_info,image,chunk_size);
            jpeg_set_marker_processor(&jpeg_info,JPEG_COM,ReadComment);
            jpeg_set_marker_processor(&jpeg_info,ICC_MARKER,ReadICCProfile);
            jpeg_set_marker_processor(&jpeg_info,IPTC_MARKER,ReadIPTCProfile);
            for (i=1; i < 16; i++)
              if ((i != 2) && (i != 13) && (i != 14))
                jpeg_set_marker_processor(&jpeg_info,(int) i+JPEG_APP0,ReadProfile);
            i=jpeg_read_header(&jpeg_info,MagickTrue);
            /*
              Set image resolution.
            */
            units=0;
            if ((jpeg_info.saw_JFIF_marker != 0) && (jpeg_info.X_density != 1) &&
              (jpeg_info.Y_density != 1))
            {
              image->x_resolution=(double) jpeg_info.X_density;
              image->y_resolution=(double) jpeg_info.Y_density;
              units=(unsigned long) jpeg_info.density_unit;
            }
            value=GetImageProperty(image,"exif:XResolution");
            if (value != (char *) NULL)
            {
              (void) ParseGeometry(value,&geometry_info);
              if (geometry_info.sigma != 0)
                image->x_resolution=geometry_info.rho/geometry_info.sigma;
              (void) SetImageProperty(image,"exif:XResolution",(char *) NULL);
            }
            value=GetImageProperty(image,"exif:YResolution");
            if (value != (char *) NULL)
            {
              (void) ParseGeometry(value,&geometry_info);
              if (geometry_info.sigma != 0)
                image->y_resolution=geometry_info.rho/geometry_info.sigma;
              (void) SetImageProperty(image,"exif:YResolution",(char *) NULL);
            }
            if (units == 1)
              image->units=PixelsPerInchResolution;
            if (units == 2)
              image->units=PixelsPerCentimeterResolution;
            number_pixels=image->columns*image->rows;
            if (number_pixels != 0)
            {
              double
                scale_factor;

              /*
              Let the JPEG library subsample for us.
              */
              jpeg_calc_output_dimensions(&jpeg_info);
              image->magick_columns=jpeg_info.output_width;
              image->magick_rows=jpeg_info.output_height;
              scale_factor=(double) jpeg_info.output_width/image->columns;
              if (scale_factor > ((double) jpeg_info.output_height/image->rows))
                scale_factor=(double) jpeg_info.output_height/image->rows;
              jpeg_info.scale_denom=(unsigned int) scale_factor;
              jpeg_calc_output_dimensions(&jpeg_info);
              if (image->debug != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "Scale factor: %ld",(long) scale_factor);
            }
#if (JPEG_LIB_VERSION >= 61) && defined(D_PROGRESSIVE_SUPPORTED)
#ifdef D_LOSSLESS_SUPPORTED
            image->interlace=jpeg_info.process == JPROC_PROGRESSIVE ?
              PlaneInterlace : NoInterlace;
            image->compression=jpeg_info.process == JPROC_LOSSLESS ?
              LosslessJPEGCompression : JPEGCompression;
            if (jpeg_info.data_precision > 8)
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionWarning,
                "12-bit JPEG not supported. Reducing pixel data to 8 bits",
                "`%s'",image->filename);
#else
            image->interlace=jpeg_info.progressive_mode != 0 ? PlaneInterlace :
            NoInterlace;
            image->compression=JPEGCompression;
#endif
#else
            image->compression=JPEGCompression;
            image->interlace=PlaneInterlace;
#endif
            if ((image_info->colors != 0) && (image_info->colors <= 256))
            {
              /*
              Let the JPEG library quantize for us.
              */
              jpeg_info.quantize_colors=MagickTrue;
              jpeg_info.desired_number_of_colors=(int) image_info->colors;
              if (AcquireImageColormap(image,image_info->colors) == MagickFalse)
                ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            }
            (void) jpeg_start_decompress(&jpeg_info);
            image->columns=jpeg_info.output_width;
            image->rows=jpeg_info.output_height;
            image->depth=(unsigned long) jpeg_info.data_precision;
            if (jpeg_info.out_color_space == JCS_CMYK)
              image->colorspace=CMYKColorspace;
            if ((jpeg_info.output_components == 1) &&
              (jpeg_info.quantize_colors == MagickFalse))
            {
              if (AcquireImageColormap(image,1UL << image->depth) == MagickFalse)
                ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            }
            if (image->debug != MagickFalse)
            {
              if (image->interlace == PlaneInterlace)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "Interlace: progressive");
              else
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "Interlace: nonprogressive");
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "Data precision: %d",(int) jpeg_info.data_precision);
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "Geometry: %dx%d",(int) jpeg_info.output_width,(int)
                jpeg_info.output_height);
#ifdef D_LOSSLESS_SUPPORTED
              if (image->compression==LosslessJPEGCompression)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "Quality: 100 (lossless)");
              else
#endif
              {
                long
                  hashval,
                  sum;

                /*
                  Log the JPEG quality that was used for compression.
                */
                sum=0;
                for (i=0; i < NUM_QUANT_TBLS; i++)
                {
                  int
                    j;

                  if (jpeg_info.quant_tbl_ptrs[i] != NULL)
                    for (j=0; j < DCTSIZE2; j++)
                    {
                      UINT16
                        *c;

                      c=jpeg_info.quant_tbl_ptrs[i]->quantval;
                      sum+=c[j];
                    }
                }
                if ((jpeg_info.quant_tbl_ptrs[0] != NULL) &&
                  (jpeg_info.quant_tbl_ptrs[1] != NULL))
                {
                  int
                    hash[]=
                  {
                    1020, 1015,  932,  848,  780,  735,  702,  679,  660,  645,
                      632,  623,  613,  607,  600,  594,  589,  585,  581,  571,
                      555,  542,  529,  514,  494,  474,  457,  439,  424,  410,
                      397,  386,  373,  364,  351,  341,  334,  324,  317,  309,
                      299,  294,  287,  279,  274,  267,  262,  257,  251,  247,
                      243,  237,  232,  227,  222,  217,  213,  207,  202,  198,
                      192,  188,  183,  177,  173,  168,  163,  157,  153,  148,
                      143,  139,  132,  128,  125,  119,  115,  108,  104,   99,
                      94,   90,   84,   79,   74,   70,   64,   59,   55,   49,
                      45,   40,   34,   30,   25,   20,   15,   11,    6,    4,
                      0
                  },
                  sums[]=
                    {
                      32640,32635,32266,31495,30665,29804,29146,28599,28104,27670,
                        27225,26725,26210,25716,25240,24789,24373,23946,23572,22846,
                        21801,20842,19949,19121,18386,17651,16998,16349,15800,15247,
                        14783,14321,13859,13535,13081,12702,12423,12056,11779,11513,
                        11135,10955,10676,10392,10208, 9928, 9747, 9564, 9369, 9193,
                        9017, 8822, 8639, 8458, 8270, 8084, 7896, 7710, 7527, 7347,
                        7156, 6977, 6788, 6607, 6422, 6236, 6054, 5867, 5684, 5495,
                        5305, 5128, 4945, 4751, 4638, 4442, 4248, 4065, 3888, 3698,
                        3509, 3326, 3139, 2957, 2775, 2586, 2405, 2216, 2037, 1846,
                        1666, 1483, 1297, 1109,  927,  735,  554,  375,  201,  128,
                        0
                    };

                    hashval=(long) ((jpeg_info.quant_tbl_ptrs[0]->quantval[2]+
                      jpeg_info.quant_tbl_ptrs[0]->quantval[53]+
                      jpeg_info.quant_tbl_ptrs[1]->quantval[0]+
                      jpeg_info.quant_tbl_ptrs[1]->quantval[DCTSIZE2-1]));
                    for (i=0; i < 100; i++)
                    {
                      if ((hashval >= hash[i]) || (sum >= sums[i]))
                      {
                        if ((hashval > hash[i]) || (sum > sums[i]))
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Quality: %ld (approximate)",i+1);
                        else
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Quality: %ld",i+1);
                        break;
                      }
                    }
                }
                else
                  if (jpeg_info.quant_tbl_ptrs[0] != NULL)
                  {
                    int
                      bwhash[]=
                    {
                      510,  505,  422,  380,  355,  338,  326,  318,  311,  305,
                        300,  297,  293,  291,  288,  286,  284,  283,  281,  280,
                        279,  278,  277,  273,  262,  251,  243,  233,  225,  218,
                        211,  205,  198,  193,  186,  181,  177,  172,  168,  164,
                        158,  156,  152,  148,  145,  142,  139,  136,  133,  131,
                        129,  126,  123,  120,  118,  115,  113,  110,  107,  105,
                        102,  100,   97,   94,   92,   89,   87,   83,   81,   79,
                        76,   74,   70,   68,   66,   63,   61,   57,   55,   52,
                        50,   48,   44,   42,   39,   37,   34,   31,   29,   26,
                        24,   21,   18,   16,   13,   11,    8,    6,    3,    2,
                        0
                    },
                    bwsum[]=
                      {
                        16320,16315,15946,15277,14655,14073,13623,13230,12859,12560,
                          12240,11861,11456,11081,10714,10360,10027, 9679, 9368, 9056,
                          8680, 8331, 7995, 7668, 7376, 7084, 6823, 6562, 6345, 6125,
                          5939, 5756, 5571, 5421, 5240, 5086, 4976, 4829, 4719, 4616,
                          4463, 4393, 4280, 4166, 4092, 3980, 3909, 3835, 3755, 3688,
                          3621, 3541, 3467, 3396, 3323, 3247, 3170, 3096, 3021, 2952,
                          2874, 2804, 2727, 2657, 2583, 2509, 2437, 2362, 2290, 2211,
                          2136, 2068, 1996, 1915, 1858, 1773, 1692, 1620, 1552, 1477,
                          1398, 1326, 1251, 1179, 1109, 1031,  961,  884,  814,  736,
                          667,  592,  518,  441,  369,  292,  221,  151,   86,   64,
                          0
                      };

                      hashval=(long) ((jpeg_info.quant_tbl_ptrs[0]->quantval[2]+
                        jpeg_info.quant_tbl_ptrs[0]->quantval[53]));
                      for (i=0; i < 100; i++)
                      {
                        if ((hashval >= bwhash[i]) || (sum >= bwsum[i]))
                        {
                          if ((hashval > bwhash[i]) || (sum > bwsum[i]))
                            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Quality: %ld (approximate)",i+1);
                          else
                            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Quality: %ld",i+1);
                          break;
                        }
                      }
                  }
              }
              switch (jpeg_info.out_color_space)
              {
              case JCS_CMYK:
                {
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Colorspace: CMYK");
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Sampling factors: (%d,%d),(%d,%d),(%d,%d),(%d,%d)",
                    jpeg_info.comp_info[0].h_samp_factor,
                    jpeg_info.comp_info[0].v_samp_factor,
                    jpeg_info.comp_info[1].h_samp_factor,
                    jpeg_info.comp_info[1].v_samp_factor,
                    jpeg_info.comp_info[2].h_samp_factor,
                    jpeg_info.comp_info[2].v_samp_factor,
                    jpeg_info.comp_info[3].h_samp_factor,
                    jpeg_info.comp_info[3].v_samp_factor);
                  break;
                }
              case JCS_GRAYSCALE:
                {
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Colorspace: GRAYSCALE");
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Sampling factors: (%d,%d)",
                    jpeg_info.comp_info[0].h_samp_factor,
                    jpeg_info.comp_info[0].v_samp_factor);
                  break;
                }
              case JCS_RGB:
                {
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Colorspace: RGB");
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Sampling factors: (%d,%d),(%d,%d),(%d,%d)",
                    jpeg_info.comp_info[0].h_samp_factor,
                    jpeg_info.comp_info[0].v_samp_factor,
                    jpeg_info.comp_info[1].h_samp_factor,
                    jpeg_info.comp_info[1].v_samp_factor,
                    jpeg_info.comp_info[2].h_samp_factor,
                    jpeg_info.comp_info[2].v_samp_factor);
                  break;
                }
              default:
                {
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Colorspace: %d",jpeg_info.out_color_space);
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Sampling factors: (%d,%d),(%d,%d),(%d,%d),(%d,%d)",
                    jpeg_info.comp_info[0].h_samp_factor,
                    jpeg_info.comp_info[0].v_samp_factor,
                    jpeg_info.comp_info[1].h_samp_factor,
                    jpeg_info.comp_info[1].v_samp_factor,
                    jpeg_info.comp_info[2].h_samp_factor,
                    jpeg_info.comp_info[2].v_samp_factor,
                    jpeg_info.comp_info[3].h_samp_factor,
                    jpeg_info.comp_info[3].v_samp_factor);
                  break;
                }
              }
            }
            if (image_info->ping != MagickFalse)
              {
                jpeg_destroy_decompress(&jpeg_info);
                (void) CloseBlob(image);
                return(GetFirstImageInList(image));
              }
            jpeg_pixels=(JSAMPLE *) AcquireQuantumMemory((size_t)
              image->columns,jpeg_info.output_components*sizeof(JSAMPLE));
            if (jpeg_pixels == (JSAMPLE *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            /*
            Convert JPEG pixels to pixel packets.
            */
            scanline[0]=(JSAMPROW) jpeg_pixels;
            for (y=0; y < (long) image->rows; y++)
            {
              (void) jpeg_read_scanlines(&jpeg_info,scanline,1);
              p=jpeg_pixels;
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetAuthenticIndexQueue(image);
              if (jpeg_info.data_precision > 8)
              {
                if (jpeg_info.output_components == 1)
                  for (x=0; x < (long) image->columns; x++)
                  {
                    index=ConstrainColormapIndex(image,16*(unsigned long)
                      GETJSAMPLE(*p));
                    indexes[x]=(IndexPacket) index;
                    *q++=image->colormap[(long) index];
                    p++;
                  }
                else
                  for (x=0; x < (long) image->columns; x++)
                  {
                    q->red=ScaleShortToQuantum((unsigned short)
                      (16*GETJSAMPLE(*p++)));
                    q->green=ScaleShortToQuantum((unsigned short)
                      (16*GETJSAMPLE(*p++)));
                    q->blue=ScaleShortToQuantum((unsigned short)
                      (16*GETJSAMPLE(*p++)));
                    if (image->colorspace == CMYKColorspace)
                      indexes[x]=ScaleShortToQuantum((unsigned short)
                        (16*GETJSAMPLE(*p++)));
                    q++;
                  }
              }
              else
                if (jpeg_info.output_components == 1)
                  for (x=0; x < (long) image->columns; x++)
                  {
                    index=ConstrainColormapIndex(image,(unsigned long)
                      GETJSAMPLE(*p));
                    indexes[x]=(IndexPacket) index;
                    *q++=image->colormap[(long) index];
                    p++;
                  }
                else
                  for (x=0; x < (long) image->columns; x++)
                  {
                    q->red=ScaleCharToQuantum((unsigned char) GETJSAMPLE(*p++));
                    q->green=ScaleCharToQuantum((unsigned char)
                      GETJSAMPLE(*p++));
                    q->blue=ScaleCharToQuantum((unsigned char)
                      GETJSAMPLE(*p++));
                    if (image->colorspace == CMYKColorspace)
                      indexes[x]=ScaleCharToQuantum((unsigned char)
                        GETJSAMPLE(*p++));
                    q++;
                  }
                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            if (jpeg_info.quantize_colors != MagickFalse)
              for (i=0; i < jpeg_info.actual_number_of_colors; i++)
              {
                image->colormap[i].red=
                  ScaleCharToQuantum(jpeg_info.colormap[0][i]);
                image->colormap[i].green=
                  ScaleCharToQuantum(jpeg_info.colormap[1][i]);
                image->colormap[i].blue=
                  ScaleCharToQuantum(jpeg_info.colormap[2][i]);
              }
              if (image->colorspace == CMYKColorspace)
              {
                /*
                Correct CMYK levels.
                */
                for (y=0; y < (long) image->rows; y++)
                {
                  q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
                  if (q == (PixelPacket *) NULL)
                    break;
                  indexes=GetAuthenticIndexQueue(image);
                  for (x=0; x < (long) image->columns; x++)
                  {
                    q->red=(Quantum) (QuantumRange-q->red);
                    q->green=(Quantum) (QuantumRange-q->green);
                    q->blue=(Quantum) (QuantumRange-q->blue);
                    indexes[x]=(IndexPacket) (QuantumRange-indexes[x]);
                    q++;
                  }
                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    break;
                }
              }
            /*
              Consume Buffer.
            */
            chunk_size=(unsigned long) ((SourceManager *)
              jpeg_info.src)->source_length;
            for ( ; chunk_size != 0; chunk_size--)
              if (ReadBlobByte(image) == EOF)
                ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
            /*
              Free jpeg resources.
            */
            (void) jpeg_finish_decompress(&jpeg_info);
            jpeg_pixels=(unsigned char *) RelinquishMagickMemory(jpeg_pixels);
#else
            (void) CloseBlob(image);
            DestroyImageList(image);
            return(DecodeAVIImage(image_info,exception));
#endif
          }
        /*
          Read BMP image
        */
        else
        {
          /*
            Initialize image structure.
          */
          image->columns=avi_info.width;
          image->rows=avi_info.height;
          image->depth=8;
          image->units=PixelsPerCentimeterResolution;
          image->x_resolution=(double) bmp_info.x_pixels/100.0;
          image->y_resolution=(double) bmp_info.y_pixels/100.0;
          status=AcquireImageColormap(image,number_colors != 0 ? number_colors :
            256);
          if (status == MagickFalse)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          if (number_colors != 0)
            (void) CopyMagickMemory(image->colormap,colormap,(size_t)
              number_colors*sizeof(PixelPacket));
          if ((image_info->ping != MagickFalse) &&
              (image_info->number_scenes != 0))
            if (image->scene >= (image_info->scene+image_info->number_scenes-1))
              break;
          bytes_per_line=4*((image->columns*bmp_info.bits_per_pixel+31)/32);
          pixels=(unsigned char *) AcquireQuantumMemory((size_t) image->rows,
            MagickMax(bytes_per_line,image->columns+256UL)*sizeof(*pixels));
          if (pixels == (unsigned char *) NULL)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          (void) ResetMagickMemory(pixels,0,(size_t)  MagickMax(
            bytes_per_line,image->columns+1)*image->rows);
          if (LocaleCompare(id,"00db") == 0)
            {
              count=ReadBlob(image,(size_t) bytes_per_line*image->rows,pixels);
              if (count != (ssize_t) (bytes_per_line*image->rows))
                ThrowReaderException(CorruptImageError,
                  "InsufficientImageDataInFile");
            }
          else
            {
              status=DecodeImage(image,MagickTrue,pixels);
              if (status == MagickFalse)
                ThrowReaderException(CorruptImageError,
                  "UnableToRunlengthDecodeImage");
            }
          /*
          Convert BMP raster image to pixel packets.
          */
          switch ((int) bmp_info.bits_per_pixel)
          {
            case 1:
            {
              /*
              Convert bitmap scanline.
              */
              for (y=(long) image->rows-1; y >= 0; y--)
              {
                p=pixels+(image->rows-y-1)*bytes_per_line;
                q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
                if (q == (PixelPacket *) NULL)
                  break;
                indexes=GetAuthenticIndexQueue(image);
                for (x=0; x < ((long) image->columns-7); x+=8)
                {
                  for (bit=0; bit < 8; bit++)
                  {
                    index=(IndexPacket)
                      (((*p) & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
                    indexes[x+bit]=index;
                    *q++=image->colormap[(long) index];
                  }
                  p++;
                }
                if ((image->columns % 8) != 0)
                {
                  for (bit=0; bit < (image->columns % 8); bit++)
                  {
                    index=(IndexPacket)
                      (((*p) & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
                    indexes[x+bit]=index;
                    *q++=image->colormap[(long) index];
                  }
                  p++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
                offset=(MagickOffsetType) image->rows-y-1;
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,LoadImageTag,y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
              break;
            }
          case 4:
            {
              /*
              Convert PseudoColor scanline.
              */
              for (y=(long) image->rows-1; y >= 0; y--)
              {
                p=pixels+(image->rows-y-1)*bytes_per_line;
                q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
                if (q == (PixelPacket *) NULL)
                  break;
                indexes=GetAuthenticIndexQueue(image);
                for (x=0; x < ((long) image->columns-1); x+=2)
                {
                  index=ConstrainColormapIndex(image,((*p >> 4) & 0xf));
                  indexes[x]=index;
                  *q++=image->colormap[(long) index];
                  index=ConstrainColormapIndex(image,(*p & 0xf));
                  indexes[x+1]=index;
                  *q++=image->colormap[(long) index];
                  p++;
                }
                if ((image->columns % 2) != 0)
                {
                  index=ConstrainColormapIndex(image,((*p >> 4) & 0xf));
                  indexes[x]=index;
                  *q++=image->colormap[(long) index];
                  p++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
                offset=(MagickOffsetType) image->rows-y-1;
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,LoadImageTag,y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
              break;
            }
            case 8:
            {
              /*
              Convert PseudoColor scanline.
              */
              bytes_per_line=image->columns;
              for (y=(long) image->rows-1; y >= 0; y--)
              {
                p=pixels+(image->rows-y-1)*bytes_per_line;
                q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
                if (q == (PixelPacket *) NULL)
                  break;
                indexes=GetAuthenticIndexQueue(image);
                for (x=0; x < (long) image->columns; x++)
                {
                  index=ConstrainColormapIndex(image,*p);
                  indexes[x]=index;
                  *q=image->colormap[(long) index];
                  p++;
                  q++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
                offset=(MagickOffsetType) image->rows-y-1;
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,LoadImageTag,y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
              break;
            }
            case 16:
            {
              unsigned long
                word;

              /*
              Convert PseudoColor scanline.
              */
              bytes_per_line=image->columns << 1;
              image->storage_class=DirectClass;
              for (y=(long) image->rows-1; y >= 0; y--)
              {
                p=pixels+(image->rows-y-1)*bytes_per_line;
                q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
                if (q == (PixelPacket *) NULL)
                  break;
                for (x=0; x < (long) image->columns; x++)
                {
                  word=(unsigned long) (*p++);
                  word|=(*p++ << 8);
                  q->red=ScaleCharToQuantum(ScaleColor5to8((word >> 11) &
                    0x1f));
                  q->green=ScaleCharToQuantum(ScaleColor6to8((word >> 5) &
                    0x3f));
                  q->blue=ScaleCharToQuantum(ScaleColor5to8(word & 0x1f));
                  q++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
                offset=(MagickOffsetType) image->rows-y-1;
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,LoadImageTag,y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
              break;
            }
            case 24:
            case 32:
            {
              /*
              Convert DirectColor scanline.
              */
              image->storage_class=DirectClass;
              for (y=(long) image->rows-1; y >= 0; y--)
              {
                p=pixels+(image->rows-y-1)*bytes_per_line;
                q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
                if (q == (PixelPacket *) NULL)
                  break;
                for (x=0; x < (long) image->columns; x++)
                {
                  q->blue=ScaleCharToQuantum(*p++);
                  q->green=ScaleCharToQuantum(*p++);
                  q->red=ScaleCharToQuantum(*p++);
                  if (image->matte != MagickFalse)
                    q->opacity=ScaleCharToQuantum(*p++);
                  q++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
                offset=(MagickOffsetType) image->rows-y-1;
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,LoadImageTag,y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
              break;
            }
          default:
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
        }

        /*
          Setup for next image
        */

        if ((unsigned long) image->scene < (avi_info.total_frames-1))
          {
            /*
              Allocate next image structure.
            */
            AcquireNextImage(image_info,image);
            if (GetNextImageInList(image) == (Image *) NULL)
              {
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
            image=SyncNextImageInList(image);
            status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
              GetBlobSize(image));
            if (status == MagickFalse)
              break;
          }
        continue;
      }
    if (LocaleCompare(id,"avih") == 0)
      {
        /*
          AVI header.
        */
        if (image_info->verbose != MagickFalse)
          (void) fprintf(stdout,"  AVI header\n");
        avi_info.delay=ReadBlobLSBLong(image);
        avi_info.max_data_rate=ReadBlobLSBLong(image);
        avi_info.pad_granularity=ReadBlobLSBLong(image);
        avi_info.flags=ReadBlobLSBLong(image);
        avi_info.total_frames=ReadBlobLSBLong(image);
        avi_info.initial_frames=ReadBlobLSBLong(image);
        avi_info.number_streams=ReadBlobLSBLong(image);
        avi_info.buffer_size=ReadBlobLSBLong(image);
        avi_info.width=ReadBlobLSBLong(image);
        avi_info.height=ReadBlobLSBLong(image);
        avi_info.time_scale=ReadBlobLSBLong(image);
        avi_info.data_rate=ReadBlobLSBLong(image);
        avi_info.start_time=ReadBlobLSBLong(image);
        avi_info.data_length=ReadBlobLSBLong(image);
        continue;
      }
    if (LocaleCompare(id,"idx1") == 0)
      {
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    if (LocaleCompare(id,"JUNK") == 0)
      {
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    if (LocaleCompare(id,"LIST") == 0)
      {
        /*
          List of chunks.
        */
        count=ReadBlob(image,4,(unsigned char *) id);
        if (image_info->verbose != MagickFalse)
          (void) fprintf(stdout,"  List type %s\n",id);
        continue;
      }
    if (LocaleCompare(id,"RIFF") == 0)
      {
        /*
          File header.
        */
        count=ReadBlob(image,4,(unsigned char *) id);
        if (image_info->verbose != MagickFalse)
          (void) fprintf(stdout,"  RIFF form type %s\n",id);
        continue;
      }
    if (LocaleCompare(id,"strf") == 0)
      {
        /*
          Stream format.
        */
        if (image_info->verbose != MagickFalse)
          (void) fprintf(stdout,"  Stream fcc\n");
        if (LocaleCompare(stream_info.data_type,"vids") == 0)
          {
            bmp_info.size=ReadBlobLSBLong(image);
            bmp_info.width=ReadBlobLSBLong(image);
            bmp_info.height=ReadBlobLSBLong(image);
            bmp_info.planes=ReadBlobLSBShort(image);
            bmp_info.bits_per_pixel=ReadBlobLSBShort(image);
            count=ReadBlob(image,4,(unsigned char *) bmp_info.compression);
            bmp_info.compression[4]='\0';
            bmp_info.image_size=ReadBlobLSBLong(image);
            bmp_info.x_pixels=ReadBlobLSBLong(image);
            bmp_info.y_pixels=ReadBlobLSBLong(image);
            bmp_info.number_colors=ReadBlobLSBLong(image);
            bmp_info.important_colors=ReadBlobLSBLong(image);
            chunk_size-=40;
            number_colors=bmp_info.number_colors;
            if ((number_colors == 0) && (bmp_info.bits_per_pixel <= 8))
              number_colors=1 << bmp_info.bits_per_pixel;
            if (number_colors != 0)
              {
                colormap=(PixelPacket *) AcquireQuantumMemory((size_t)
                  number_colors,sizeof(*colormap));
                if (colormap == (PixelPacket *) NULL)
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed");
                for (i=0; i < (long) number_colors; i++)
                {
                  colormap[i].blue=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  colormap[i].green=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  colormap[i].red=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  if (ReadBlobByte(image) == EOF)
                    ThrowReaderException(CorruptImageError,
                      "UnexpectedEndOfFile");
                  chunk_size-=4;
                }
              }
            for ( ; chunk_size != 0; chunk_size--)
              if (ReadBlobByte(image) == EOF)
                ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
            if (image_info->verbose != MagickFalse)
              (void) fprintf(stdout,"Video compression: %s\n",
                bmp_info.compression);
            continue;
          }
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    if (LocaleCompare(id,"strh") == 0)
      {
        if (image_info->verbose != MagickFalse)
          (void) fprintf(stdout,"  Stream header\n");
        count=ReadBlob(image,4,(unsigned char *) stream_info.data_type);
        stream_info.data_type[4]='\0';
        count=ReadBlob(image,4,(unsigned char *) stream_info.data_handler);
        stream_info.data_handler[4]='\0';
        stream_info.flags=ReadBlobLSBLong(image);
        stream_info.priority=ReadBlobLSBLong(image);
        stream_info.initial_frames=ReadBlobLSBLong(image);
        stream_info.time_scale=ReadBlobLSBLong(image);
        stream_info.data_rate=ReadBlobLSBLong(image);
        stream_info.start_time=ReadBlobLSBLong(image);
        stream_info.data_length=ReadBlobLSBLong(image);
        stream_info.buffer_size=ReadBlobLSBLong(image);
        stream_info.quality=ReadBlobLSBLong(image);
        stream_info.sample_size=ReadBlobLSBLong(image);
        if ((chunk_size & 0x01) != 0)
          chunk_size++;
        for (chunk_size-=48; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        if (image_info->verbose != MagickFalse)
          (void) fprintf(stdout,"AVI Test handler: %s\n",
            stream_info.data_handler);
        continue;
      }
    if (LocaleCompare(id,"strn") == 0)
      {
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    if (LocaleCompare(id,"vedt") == 0)
      {
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    if (strlen(id) == 4 && LocaleCompare(id + 2,"wb") == 0)
      {
        /*
          Skip audio chunks (##wb).
        */
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    if (LocaleCompare(id,"ISFT") == 0)
      {
        /*
          Skip Info Chunks.
        */
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    if (LocaleCompare(id,"IDIT") == 0)
      {
        /*
          Skip Timecode Information (TODO: seeking).
        */
        for ( ; chunk_size != 0; chunk_size--)
          if (ReadBlobByte(image) == EOF)
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        continue;
      }
    (void) CloseBlob(image);
    DestroyImageList(image);
    return(DecodeAVIImage(image_info,exception));
  }
  (void) CloseBlob(image);
  if ((image->columns == 0) || (image->rows == 0))
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r A V I I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterAVIImage() adds properties for the AVI image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterAVIImage method is:
%
%      unsigned long RegisterAVIImage(void)
%
*/
ModuleExport unsigned long RegisterAVIImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("AVI");
  entry->decoder=(DecodeImageHandler *) ReadAVIImage;
  entry->magick=(IsImageFormatHandler *) IsAVI;
  entry->description=ConstantString("Microsoft Audio/Visual Interleaved");
  entry->module=ConstantString("AVI");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A V I I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterAVIImage() removes format registrations made by the
%  AVI module from the list of supported formats.
%
%  The format of the UnregisterAVIImage method is:
%
%      UnregisterAVIImage(void)
%
*/
ModuleExport void UnregisterAVIImage(void)
{
  (void) UnregisterMagickInfo("AVI");
}

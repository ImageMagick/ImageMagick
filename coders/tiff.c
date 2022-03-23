/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        TTTTT  IIIII  FFFFF  FFFFF                           %
%                          T      I    F      F                               %
%                          T      I    FFF    FFF                             %
%                          T      I    F      F                               %
%                          T    IIIII  F      F                               %
%                                                                             %
%                                                                             %
%                        Read/Write TIFF Image Format                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#ifdef __VMS
#define JPEG_SUPPORT 1
#endif
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/profile.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread_.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "coders/coders-private.h"
#include "coders/psd-private.h"
#if defined(MAGICKCORE_TIFF_DELEGATE)
# if defined(MAGICKCORE_HAVE_TIFFCONF_H)
#  include <tiffconf.h>
# endif
# include <tiff.h>
# include <tiffio.h>
# if !defined(COMPRESSION_ADOBE_DEFLATE)
#  define COMPRESSION_ADOBE_DEFLATE  8
# endif
# if !defined(PREDICTOR_HORIZONTAL)
# define PREDICTOR_HORIZONTAL  2
# endif
# if !defined(TIFFTAG_COPYRIGHT)
#  define TIFFTAG_COPYRIGHT  33432
# endif
# if !defined(TIFFTAG_OPIIMAGEID)
#  define TIFFTAG_OPIIMAGEID  32781
# endif
# if defined(COMPRESSION_ZSTD) && defined(MAGICKCORE_ZSTD_DELEGATE)
#   include <zstd.h>
# endif

#if (TIFFLIB_VERSION >= 20201219)
#if defined(MAGICKCORE_HAVE_STDINT_H) || defined(MAGICKCORE_WINDOWS_SUPPORT)
#  undef uint16
#  define uint16  uint16_t
#  undef uint32
#  define uint32  uint32_t
#  undef uint64
#  define uint64  uint64_t
#endif
#endif

/*
  Typedef declarations.
*/
typedef enum
{
  ReadYCCKMethod,
  ReadStripMethod,
  ReadTileMethod,
  ReadGenericMethod
} TIFFMethodType;

typedef struct _PhotoshopProfile
{
  StringInfo
    *data;

  MagickOffsetType
    offset;

  size_t
    length,
    extent,
    quantum;
} PhotoshopProfile;

/*
  Global declarations.
*/
static MagickThreadKey
  tiff_exception;

static SemaphoreInfo
  *tiff_semaphore = (SemaphoreInfo *) NULL;

static TIFFErrorHandler
  error_handler,
  warning_handler;

static volatile MagickBooleanType
  instantiate_key = MagickFalse;

/*
  Forward declarations.
*/
static Image *
  ReadTIFFImage(const ImageInfo *,ExceptionInfo *);

static MagickBooleanType
  WriteGROUP4Image(const ImageInfo *,Image *,ExceptionInfo *),
  WritePTIFImage(const ImageInfo *,Image *,ExceptionInfo *),
  WriteTIFFImage(const ImageInfo *,Image *,ExceptionInfo *);

static MagickOffsetType TIFFSeekCustomStream(const MagickOffsetType offset,
  const int whence,void *user_data)
{
  PhotoshopProfile
    *profile;

  profile=(PhotoshopProfile *) user_data;
  switch (whence)
  {
    case SEEK_SET:
    default:
    {
      if (offset < 0)
        return(-1);
      profile->offset=offset;
      break;
    }
    case SEEK_CUR:
    {
      if (((offset > 0) && (profile->offset > (MAGICK_SSIZE_MAX-offset))) ||
          ((offset < 0) && (profile->offset < (MAGICK_SSIZE_MIN-offset))))
        {
          errno=EOVERFLOW;
          return(-1);
        }
      if ((profile->offset+offset) < 0)
        return(-1);
      profile->offset+=offset;
      break;
    }
    case SEEK_END:
    {
      if (((MagickOffsetType) profile->length+offset) < 0)
        return(-1);
      profile->offset=profile->length+offset;
      break;
    }
  }

  return(profile->offset);
}

static MagickOffsetType TIFFTellCustomStream(void *user_data)
{
  PhotoshopProfile
    *profile;

  profile=(PhotoshopProfile *) user_data;
  return(profile->offset);
}

static void InitPSDInfo(const Image *image,PSDInfo *info)
{
  (void) memset(info,0,sizeof(*info));
  info->version=1;
  info->columns=image->columns;
  info->rows=image->rows;
  info->mode=10; /* Set the mode to a value that won't change the colorspace */
  info->channels=1U;
  info->min_channels=1U;
  info->has_merged_image=MagickFalse;
  if (image->storage_class == PseudoClass)
    info->mode=2; /* indexed mode */
  else
    {
      info->channels=(unsigned short) image->number_channels;
      info->min_channels=info->channels;
      if (image->alpha_trait == BlendPixelTrait)
        info->min_channels--;
    }
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s T I F F                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsTIFF() returns MagickTrue if the image format type, identified by the
%  magick string, is TIFF.
%
%  The format of the IsTIFF method is:
%
%      MagickBooleanType IsTIFF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsTIFF(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\115\115\000\052",4) == 0)
    return(MagickTrue);
  if (memcmp(magick,"\111\111\052\000",4) == 0)
    return(MagickTrue);
#if defined(TIFF_VERSION_BIG)
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick,"\115\115\000\053\000\010\000\000",8) == 0)
    return(MagickTrue);
  if (memcmp(magick,"\111\111\053\000\010\000\000\000",8) == 0)
    return(MagickTrue);
#endif
  return(MagickFalse);
}

#if defined(MAGICKCORE_TIFF_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d G R O U P 4 I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadGROUP4Image() reads a raw CCITT Group 4 image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadGROUP4Image method is:
%
%      Image *ReadGROUP4Image(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t WriteLSBLong(FILE *file,const unsigned int value)
{
  unsigned char
    buffer[4];

  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  buffer[2]=(unsigned char) (value >> 16);
  buffer[3]=(unsigned char) (value >> 24);
  return(fwrite(buffer,1,4,file));
}

static Image *ReadGROUP4Image(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    c,
    unique_file;

  MagickBooleanType
    status;

  size_t
    length;

  ssize_t
    offset,
    strip_offset;

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
    Write raw CCITT Group 4 wrapped as a TIFF image file.
  */
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    ThrowImageException(FileOpenError,"UnableToCreateTemporaryFile");
  length=fwrite("\111\111\052\000\010\000\000\000\016\000",1,10,file);
  if (length != 10)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  length=fwrite("\376\000\003\000\001\000\000\000\000\000\000\000",1,12,file);
  length=fwrite("\000\001\004\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(unsigned int) image->columns);
  length=fwrite("\001\001\004\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(unsigned int) image->rows);
  length=fwrite("\002\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\003\001\003\000\001\000\000\000\004\000\000\000",1,12,file);
  length=fwrite("\006\001\003\000\001\000\000\000\000\000\000\000",1,12,file);
  length=fwrite("\021\001\003\000\001\000\000\000",1,8,file);
  strip_offset=10+(12*14)+4+8;
  length=WriteLSBLong(file,(unsigned int) strip_offset);
  length=fwrite("\022\001\003\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(unsigned int) image_info->orientation);
  length=fwrite("\025\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\026\001\004\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(unsigned int) image->rows);
  length=fwrite("\027\001\004\000\001\000\000\000\000\000\000\000",1,12,file);
  offset=(ssize_t) ftell(file)-4;
  length=fwrite("\032\001\005\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(unsigned int) (strip_offset-8));
  length=fwrite("\033\001\005\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(unsigned int) (strip_offset-8));
  length=fwrite("\050\001\003\000\001\000\000\000\002\000\000\000",1,12,file);
  length=fwrite("\000\000\000\000",1,4,file);
  length=WriteLSBLong(file,(unsigned int) image->resolution.x);
  length=WriteLSBLong(file,1);
  status=MagickTrue;
  for (length=0; (c=ReadBlobByte(image)) != EOF; length++)
    if (fputc(c,file) != c)
      status=MagickFalse;
  offset=(ssize_t) fseek(file,(ssize_t) offset,SEEK_SET);
  length=WriteLSBLong(file,(unsigned int) length);
  if (ferror(file) != 0)
    {
      (void) fclose(file);
      ThrowImageException(FileOpenError,"UnableToCreateTemporaryFile");
    }
  (void) fclose(file);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  /*
    Read TIFF image.
  */
  read_info=CloneImageInfo((ImageInfo *) NULL);
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,"%s",filename);
  image=ReadTIFFImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick_filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick,"GROUP4",MagickPathExtent);
    }
  (void) RelinquishUniqueFileResource(filename);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}
#endif

#if defined(MAGICKCORE_TIFF_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d T I F F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadTIFFImage() reads a Tagged image file and returns it.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the ReadTIFFImage method is:
%
%      Image *ReadTIFFImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline unsigned char ClampYCC(double value)
{
  value=255.0-value;
  if (value < 0.0)
    return((unsigned char)0);
  if (value > 255.0)
    return((unsigned char)255);
  return((unsigned char)(value));
}

static MagickBooleanType DecodeLabImage(Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        break;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        a,
        b;

      a=QuantumScale*GetPixela(image,q)+0.5;
      if (a > 1.0)
        a-=1.0;
      b=QuantumScale*GetPixelb(image,q)+0.5;
      if (b > 1.0)
        b-=1.0;
      SetPixela(image,QuantumRange*a,q);
      SetPixelb(image,QuantumRange*b,q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      {
        status=MagickFalse;
        break;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType ReadProfile(Image *image,const char *name,
  const unsigned char *datum,ssize_t length,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  StringInfo
    *profile;

  if (length < 4)
    return(MagickFalse);
  profile=BlobToStringInfo(datum,(size_t) length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  status=SetImageProfile(image,name,profile,exception);
  profile=DestroyStringInfo(profile);
  return(status);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int TIFFCloseBlob(thandle_t image)
{
  (void) CloseBlob((Image *) image);
  return(0);
}

static void TIFFErrors(const char *,const char *,va_list)
  magick_attribute((__format__ (__printf__,2,0)));

static void TIFFErrors(const char *module,const char *format,va_list error)
{
  char
    message[MagickPathExtent];

  ExceptionInfo
    *exception;

#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsnprintf(message,MagickPathExtent-2,format,error);
#else
  (void) vsprintf(message,format,error);
#endif
  message[MagickPathExtent-2]='\0';
  (void) ConcatenateMagickString(message,".",MagickPathExtent);
  exception=(ExceptionInfo *) GetMagickThreadValue(tiff_exception);
  if (exception != (ExceptionInfo *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),CoderError,message,
      "`%s'",module);
}

static toff_t TIFFGetBlobSize(thandle_t image)
{
  return((toff_t) GetBlobSize((Image *) image));
}

static MagickBooleanType TIFFGetProfiles(TIFF *tiff,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  uint32
    length = 0;

  unsigned char
    *profile = (unsigned char *) NULL;

  status=MagickTrue;
#if defined(TIFFTAG_ICCPROFILE)
  if ((TIFFGetField(tiff,TIFFTAG_ICCPROFILE,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    status=ReadProfile(image,"icc",profile,(ssize_t) length,exception);
#endif
#if defined(TIFFTAG_PHOTOSHOP)
  if ((TIFFGetField(tiff,TIFFTAG_PHOTOSHOP,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    status=ReadProfile(image,"8bim",profile,(ssize_t) length,exception);
#endif
#if defined(TIFFTAG_RICHTIFFIPTC) && (TIFFLIB_VERSION >= 20191103)
  if ((TIFFGetField(tiff,TIFFTAG_RICHTIFFIPTC,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    {
      const TIFFField
        *field;

      field=TIFFFieldWithTag(tiff,TIFFTAG_RICHTIFFIPTC);
      if (TIFFFieldDataType(field) == TIFF_LONG)
        {
          if (TIFFIsByteSwapped(tiff) != 0)
            TIFFSwabArrayOfLong((uint32 *) profile,(size_t) length);
          status=ReadProfile(image,"iptc",profile,4L*length,exception);
        }
      else
        status=ReadProfile(image,"iptc",profile,length,exception);
    }
#endif
#if defined(TIFFTAG_XMLPACKET)
  if ((TIFFGetField(tiff,TIFFTAG_XMLPACKET,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    {
      StringInfo
        *dng;

      status=ReadProfile(image,"xmp",profile,(ssize_t) length,exception);
      dng=BlobToStringInfo(profile,length);
      if (dng != (StringInfo *) NULL)
        {
          const char
            *target = "dc:format=\"image/dng\"";

          if (strstr((char *) GetStringInfoDatum(dng),target) != (char *) NULL)
            (void) CopyMagickString(image->magick,"DNG",MagickPathExtent);
          dng=DestroyStringInfo(dng);
        }
    }
#endif
  if ((TIFFGetField(tiff,34118,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    status=ReadProfile(image,"tiff:34118",profile,(ssize_t) length,
      exception);
  if ((TIFFGetField(tiff,37724,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    status=ReadProfile(image,"tiff:37724",profile,(ssize_t) length,exception);
  return(status);
}

static MagickBooleanType TIFFGetProperties(TIFF *tiff,Image *image,
  ExceptionInfo *exception)
{
  char
    message[MagickPathExtent],
    *text = (char *) NULL;

  MagickBooleanType
    status;

  uint32
    count,
    type;

  status=MagickTrue;
  if ((TIFFGetField(tiff,TIFFTAG_ARTIST,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:artist",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_COPYRIGHT,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:copyright",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_DATETIME,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:timestamp",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_DOCUMENTNAME,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:document",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_HOSTCOMPUTER,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:hostcomputer",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_IMAGEDESCRIPTION,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"comment",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_MAKE,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:make",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_MODEL,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:model",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_OPIIMAGEID,&count,&text) == 1) &&
      (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      status=SetImageProperty(image,"tiff:image-id",message,exception);
    }
  if ((TIFFGetField(tiff,TIFFTAG_PAGENAME,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"label",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_SOFTWARE,&text) == 1) &&
      (text != (char *) NULL))
    status=SetImageProperty(image,"tiff:software",text,exception);
  if ((TIFFGetField(tiff,33423,&count,&text) == 1) && (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      status=SetImageProperty(image,"tiff:kodak-33423",message,exception);
    }
  if ((TIFFGetField(tiff,36867,&count,&text) == 1) && (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      status=SetImageProperty(image,"tiff:kodak-36867",message,exception);
    }
  if (TIFFGetField(tiff,TIFFTAG_SUBFILETYPE,&type) == 1)
    switch (type)
    {
      case 0x01:
      {
        status=SetImageProperty(image,"tiff:subfiletype","REDUCEDIMAGE",
          exception);
        break;
      }
      case 0x02:
      {
        status=SetImageProperty(image,"tiff:subfiletype","PAGE",exception);
        break;
      }
      case 0x04:
      {
        status=SetImageProperty(image,"tiff:subfiletype","MASK",exception);
        break;
      }
      default:
        break;
    }
  return(status);
}

static void TIFFSetImageProperties(TIFF *tiff,Image *image,
  const char *tag,ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    filename[MagickPathExtent];

  FILE
    *file;

  int
    unique_file;

  /*
    Set EXIF or GPS image properties.
  */
  unique_file=AcquireUniqueFileResource(filename);
  file=(FILE *) NULL;
  if (unique_file != -1)
    file=fdopen(unique_file,"rb+");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      (void) RelinquishUniqueFileResource(filename);
      (void) ThrowMagickException(exception,GetMagickModule(),WandError,
        "UnableToCreateTemporaryFile","`%s'",filename);
      return;
    }
  TIFFPrintDirectory(tiff,file,0);
  (void) fseek(file,0,SEEK_SET);
  while (fgets(buffer,(int) sizeof(buffer),file) != NULL)
  {
    char
      *p,
      property[MagickPathExtent],
      value[MagickPathExtent];

    (void) StripMagickString(buffer);
    p=strchr(buffer,':');
    if (p == (char *) NULL)
      continue;
    *p='\0';
    (void) sprintf(property,"%s%.1024s",tag,buffer);
    (void) sprintf(value,"%s",p+1);
    (void) StripMagickString(value);
    (void) SetImageProperty(image,property,value,exception);
  }
  (void) fclose(file);
  (void) RelinquishUniqueFileResource(filename);
}

static void TIFFGetEXIFProperties(TIFF *tiff,Image *image,
  const ImageInfo* image_info,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_TIFFREADEXIFDIRECTORY)
  const char
    *option;

  tdir_t
    directory;

#if defined(TIFF_VERSION_BIG)
  uint64
#else
  uint32
#endif
    offset;

  /*
    Read EXIF properties.
  */
  option=GetImageOption(image_info,"tiff:exif-properties");
  if (IsStringFalse(option) != MagickFalse)
    return;
  offset=0;
  if (TIFFGetField(tiff,TIFFTAG_EXIFIFD,&offset) != 1)
    return;
  directory=TIFFCurrentDirectory(tiff);
  if (TIFFReadEXIFDirectory(tiff,offset) == 1)
    TIFFSetImageProperties(tiff,image,"exif:",exception);
  TIFFSetDirectory(tiff,directory);
#else
  magick_unreferenced(tiff);
  magick_unreferenced(image);
  magick_unreferenced(image_info);
  magick_unreferenced(exception);
#endif
}

static void TIFFGetGPSProperties(TIFF *tiff,Image *image,
  const ImageInfo* image_info,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_TIFFREADGPSDIRECTORY)
  const char
    *option;

  tdir_t
    directory;

#if defined(TIFF_VERSION_BIG)
  uint64
#else
  uint32
#endif
    offset;

  /*
    Read GPS properties.
  */
  option=GetImageOption(image_info,"tiff:gps-properties");
  if (IsStringFalse(option) != MagickFalse)
    return;
  offset=0;
  if (TIFFGetField(tiff,TIFFTAG_GPSIFD,&offset) != 1)
    return;
  directory=TIFFCurrentDirectory(tiff);
  if (TIFFReadGPSDirectory(tiff,offset) == 1)
    TIFFSetImageProperties(tiff,image,"exif:GPS",exception);
  TIFFSetDirectory(tiff,directory);
#else
  magick_unreferenced(tiff);
  magick_unreferenced(image);
  magick_unreferenced(image_info);
  magick_unreferenced(exception);
#endif
}

static int TIFFMapBlob(thandle_t image,tdata_t *base,toff_t *size)
{
  *base=(tdata_t *) GetBlobStreamData((Image *) image);
  if (*base != (tdata_t *) NULL)
    *size=(toff_t) GetBlobSize((Image *) image);
  if (*base != (tdata_t *) NULL)
    return(1);
  return(0);
}

static tsize_t TIFFReadBlob(thandle_t image,tdata_t data,tsize_t size)
{
  tsize_t
    count;

  count=(tsize_t) ReadBlob((Image *) image,(size_t) size,
    (unsigned char *) data);
  return(count);
}

static int TIFFReadPixels(TIFF *tiff,const tsample_t sample,
  const ssize_t row,tdata_t scanline)
{
  int
    status;

  status=TIFFReadScanline(tiff,scanline,(uint32) row,sample);
  return(status);
}

static toff_t TIFFSeekBlob(thandle_t image,toff_t offset,int whence)
{
  return((toff_t) SeekBlob((Image *) image,(MagickOffsetType) offset,whence));
}

static void TIFFUnmapBlob(thandle_t image,tdata_t base,toff_t size)
{
  (void) image;
  (void) base;
  (void) size;
}

static void TIFFWarnings(const char *,const char *,va_list)
  magick_attribute((__format__ (__printf__,2,0)));

static void TIFFWarnings(const char *module,const char *format,va_list warning)
{
  char
    message[MagickPathExtent];

  ExceptionInfo
    *exception;

#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsnprintf(message,MagickPathExtent-2,format,warning);
#else
  (void) vsprintf(message,format,warning);
#endif
  message[MagickPathExtent-2]='\0';
  (void) ConcatenateMagickString(message,".",MagickPathExtent);
  exception=(ExceptionInfo *) GetMagickThreadValue(tiff_exception);
  if (exception != (ExceptionInfo *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
      message,"`%s'",module);
}

static tsize_t TIFFWriteBlob(thandle_t image,tdata_t data,tsize_t size)
{
  tsize_t
    count;

  count=(tsize_t) WriteBlob((Image *) image,(size_t) size,
    (unsigned char *) data);
  return(count);
}

static TIFFMethodType GetJPEGMethod(Image* image,TIFF *tiff,uint16 photometric,
  uint16 bits_per_sample,uint16 samples_per_pixel)
{
#define BUFFER_SIZE 2048

  MagickOffsetType
    position,
    offset;

  size_t
    i;

  TIFFMethodType
    method;

#if defined(TIFF_VERSION_BIG)
  uint64
    *value = (uint64 *) NULL;
#else
  uint32
    *value = (uint32 *) NULL;
#endif

  unsigned char
    buffer[BUFFER_SIZE+32];

  unsigned short
    length;

  /*
    Only support 8 bit for now.
  */
  if ((photometric != PHOTOMETRIC_SEPARATED) || (bits_per_sample != 8) ||
      (samples_per_pixel != 4))
    return(ReadGenericMethod);
  /*
    Search for Adobe APP14 JPEG marker.
  */
  if (!TIFFGetField(tiff,TIFFTAG_STRIPOFFSETS,&value) || (value == NULL))
    return(ReadStripMethod);
  position=TellBlob(image);
  offset=(MagickOffsetType) (value[0]);
  if (SeekBlob(image,offset,SEEK_SET) != offset)
    return(ReadStripMethod);
  method=ReadStripMethod;
  if (ReadBlob(image,BUFFER_SIZE,buffer) == BUFFER_SIZE)
    {
      for (i=0; i < BUFFER_SIZE; i++)
      {
        while (i < BUFFER_SIZE)
        {
          if (buffer[i++] == 255)
           break;
        }
        while (i < BUFFER_SIZE)
        {
          if (buffer[++i] != 255)
           break;
        }
        if (buffer[i++] == 216) /* JPEG_MARKER_SOI */
          continue;
        length=(unsigned short) (((unsigned int) (buffer[i] << 8) |
          (unsigned int) buffer[i+1]) & 0xffff);
        if (i+(size_t) length >= BUFFER_SIZE)
          break;
        if (buffer[i-1] == 238) /* JPEG_MARKER_APP0+14 */
          {
            if (length != 14)
              break;
            /* 0 == CMYK, 1 == YCbCr, 2 = YCCK */
            if (buffer[i+13] == 2)
              method=ReadYCCKMethod;
            break;
          }
        i+=(size_t) length;
      }
    }
  (void) SeekBlob(image,position,SEEK_SET);
  return(method);
}

static ssize_t TIFFReadCustomStream(unsigned char *data,const size_t count,
  void *user_data)
{
  PhotoshopProfile
    *profile;

  size_t
    total;

  MagickOffsetType
    remaining;

  if (count == 0)
    return(0);
  profile=(PhotoshopProfile *) user_data;
  remaining=(MagickOffsetType) profile->length-profile->offset;
  if (remaining <= 0)
    return(-1);
  total=MagickMin(count, (size_t) remaining);
  (void) memcpy(data,profile->data->datum+profile->offset,total);
  profile->offset+=total;
  return(total);
}

static CustomStreamInfo *TIFFAcquireCustomStreamForReading(
  PhotoshopProfile *profile,ExceptionInfo *exception)
{
  CustomStreamInfo
    *custom_stream;

  custom_stream=AcquireCustomStreamInfo(exception);
  if (custom_stream == (CustomStreamInfo *) NULL)
    return(custom_stream);
  SetCustomStreamData(custom_stream,(void *) profile);
  SetCustomStreamReader(custom_stream,TIFFReadCustomStream);
  SetCustomStreamSeeker(custom_stream,TIFFSeekCustomStream);
  SetCustomStreamTeller(custom_stream,TIFFTellCustomStream);
  return(custom_stream);
}

static void TIFFReadPhotoshopLayers(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *option;

  const StringInfo
    *profile;

  CustomStreamInfo
    *custom_stream;

  Image
    *layers;

  ImageInfo
    *clone_info;

  PhotoshopProfile
    photoshop_profile;

  PSDInfo
    info;

  ssize_t
    i;

  if (GetImageListLength(image) != 1)
    return;
  if ((image_info->number_scenes == 1) && (image_info->scene == 0))
    return;
  option=GetImageOption(image_info,"tiff:ignore-layers");
  if (option != (const char * ) NULL)
    return;
  profile=GetImageProfile(image,"tiff:37724");
  if (profile == (const StringInfo *) NULL)
    return;
  for (i=0; i < (ssize_t) profile->length-8; i++)
  {
    if (LocaleNCompare((const char *) (profile->datum+i),
        image->endian == MSBEndian ? "8BIM" : "MIB8",4) != 0)
      continue;
    i+=4;
    if ((LocaleNCompare((const char *) (profile->datum+i),
         image->endian == MSBEndian ? "Layr" : "ryaL",4) == 0) ||
        (LocaleNCompare((const char *) (profile->datum+i),
         image->endian == MSBEndian ? "LMsk" : "ksML",4) == 0) ||
        (LocaleNCompare((const char *) (profile->datum+i),
         image->endian == MSBEndian ? "Lr16" : "61rL",4) == 0) ||
        (LocaleNCompare((const char *) (profile->datum+i),
         image->endian == MSBEndian ? "Lr32" : "23rL",4) == 0))
      break;
  }
  i+=4;
  if (i >= (ssize_t) (profile->length-8))
    return;
  photoshop_profile.data=(StringInfo *) profile;
  photoshop_profile.length=profile->length;
  custom_stream=TIFFAcquireCustomStreamForReading(&photoshop_profile,exception);
  if (custom_stream == (CustomStreamInfo *) NULL)
    return;
  layers=CloneImage(image,0,0,MagickTrue,exception);
  if (layers == (Image *) NULL)
    {
      custom_stream=DestroyCustomStreamInfo(custom_stream);
      return;
    }
  (void) DeleteImageProfile(layers,"tiff:37724");
  AttachCustomStream(layers->blob,custom_stream);
  SeekBlob(layers,(MagickOffsetType) i,SEEK_SET);
  InitPSDInfo(layers,&info);
  clone_info=CloneImageInfo(image_info);
  clone_info->number_scenes=0;
  (void) ReadPSDLayers(layers,clone_info,&info,exception);
  clone_info=DestroyImageInfo(clone_info);
  DeleteImageFromList(&layers);
  if (layers != (Image *) NULL)
    {
      SetImageArtifact(image,"tiff:has-layers","true");
      AppendImageToList(&image,layers);
      while (layers != (Image *) NULL)
      {
        SetImageArtifact(layers,"tiff:has-layers","true");
        DetachBlob(layers->blob);
        layers=GetNextImageInList(layers);
      }
    }
  custom_stream=DestroyCustomStreamInfo(custom_stream);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static Image *ReadTIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define ThrowTIFFException(severity,message) \
{ \
  if (pixel_info != (MemoryInfo *) NULL) \
    pixel_info=RelinquishVirtualMemory(pixel_info); \
  if (quantum_info != (QuantumInfo *) NULL) \
    quantum_info=DestroyQuantumInfo(quantum_info); \
  TIFFClose(tiff); \
  ThrowReaderException(severity,message); \
}

  float
    *chromaticity = (float *) NULL,
    x_position,
    y_position,
    x_resolution,
    y_resolution;

  Image
    *image;

  int
    tiff_status = 0;

  MagickBooleanType
    more_frames;

  MagickSizeType
    number_pixels;

  MagickStatusType
    status;

  MemoryInfo
    *pixel_info = (MemoryInfo *) NULL;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  ssize_t
    i,
    scanline_size,
    y;

  TIFF
    *tiff;

  TIFFMethodType
    method;

  uint16
    compress_tag = 0,
    bits_per_sample = 0,
    endian = 0,
    extra_samples = 0,
    interlace = 0,
    max_sample_value = 0,
    min_sample_value = 0,
    orientation = 0,
    pages = 0,
    photometric = 0,
    *sample_info = NULL,
    sample_format = 0,
    samples_per_pixel = 0,
    units = 0,
    value = 0;

  uint32
    height,
    rows_per_strip,
    width;

  unsigned char
    *pixels;

  void
    *sans[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

  /*
    Open image.
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
  (void) SetMagickThreadValue(tiff_exception,exception);
  tiff=TIFFClientOpen(image->filename,"rb",(thandle_t) image,TIFFReadBlob,
    TIFFWriteBlob,TIFFSeekBlob,TIFFCloseBlob,TIFFGetBlobSize,TIFFMapBlob,
    TIFFUnmapBlob);
  if (tiff == (TIFF *) NULL)
    {
      if (exception->severity == UndefinedException)
        ThrowReaderException(CorruptImageError,"UnableToReadImageData");
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (exception->severity > ErrorException)
    {
      TIFFClose(tiff);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (image_info->number_scenes != 0)
    {
      /*
        Generate blank images for subimage specification (e.g. image.tif[4].
        We need to check the number of directores because it is possible that
        the subimage(s) are stored in the photoshop profile.
      */
      if (image_info->scene < (size_t) TIFFNumberOfDirectories(tiff))
        {
          for (i=0; i < (ssize_t) image_info->scene; i++)
          {
            status=TIFFReadDirectory(tiff) != 0 ? MagickTrue : MagickFalse;
            if (status == MagickFalse)
              {
                TIFFClose(tiff);
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
            AcquireNextImage(image_info,image,exception);
            if (GetNextImageInList(image) == (Image *) NULL)
              {
                TIFFClose(tiff);
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
            image=SyncNextImageInList(image);
          }
      }
  }
  more_frames=MagickTrue;
  do
  {
    /* TIFFPrintDirectory(tiff,stdout,MagickFalse); */
    photometric=PHOTOMETRIC_RGB;
    if ((TIFFGetField(tiff,TIFFTAG_IMAGEWIDTH,&width) != 1) ||
        (TIFFGetField(tiff,TIFFTAG_IMAGELENGTH,&height) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_PHOTOMETRIC,&photometric,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_COMPRESSION,&compress_tag,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_FILLORDER,&endian,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_PLANARCONFIG,&interlace,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLESPERPIXEL,&samples_per_pixel,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,&bits_per_sample,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLEFORMAT,&sample_format,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_MINSAMPLEVALUE,&min_sample_value,sans) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_MAXSAMPLEVALUE,&max_sample_value,sans) != 1))
      {
        TIFFClose(tiff);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    if (((sample_format != SAMPLEFORMAT_IEEEFP) || (bits_per_sample != 64)) &&
        ((bits_per_sample <= 0) || (bits_per_sample > 32)))
      {
        TIFFClose(tiff);
        ThrowReaderException(CorruptImageError,"UnsupportedBitsPerPixel");
      }
    if (samples_per_pixel > MaxPixelChannels)
      {
        TIFFClose(tiff);
        ThrowReaderException(CorruptImageError,"MaximumChannelsExceeded");
      }
    if (sample_format == SAMPLEFORMAT_IEEEFP)
      (void) SetImageProperty(image,"quantum:format","floating-point",
        exception);
    switch (photometric)
    {
      case PHOTOMETRIC_MINISBLACK:
      {
        (void) SetImageProperty(image,"tiff:photometric","min-is-black",
          exception);
        break;
      }
      case PHOTOMETRIC_MINISWHITE:
      {
        (void) SetImageProperty(image,"tiff:photometric","min-is-white",
          exception);
        break;
      }
      case PHOTOMETRIC_PALETTE:
      {
        (void) SetImageProperty(image,"tiff:photometric","palette",exception);
        break;
      }
      case PHOTOMETRIC_RGB:
      {
        (void) SetImageProperty(image,"tiff:photometric","RGB",exception);
        break;
      }
      case PHOTOMETRIC_CIELAB:
      {
        (void) SetImageProperty(image,"tiff:photometric","CIELAB",exception);
        break;
      }
      case PHOTOMETRIC_LOGL:
      {
        (void) SetImageProperty(image,"tiff:photometric","CIE Log2(L)",
          exception);
        break;
      }
      case PHOTOMETRIC_LOGLUV:
      {
        (void) SetImageProperty(image,"tiff:photometric","LOGLUV",exception);
        break;
      }
#if defined(PHOTOMETRIC_MASK)
      case PHOTOMETRIC_MASK:
      {
        (void) SetImageProperty(image,"tiff:photometric","MASK",exception);
        break;
      }
#endif
      case PHOTOMETRIC_SEPARATED:
      {
        (void) SetImageProperty(image,"tiff:photometric","separated",exception);
        break;
      }
      case PHOTOMETRIC_YCBCR:
      {
        (void) SetImageProperty(image,"tiff:photometric","YCBCR",exception);
        break;
      }
      default:
      {
        (void) SetImageProperty(image,"tiff:photometric","unknown",exception);
        break;
      }
    }
    if (image->debug != MagickFalse)
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Geometry: %ux%u",
          (unsigned int) width,(unsigned int) height);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Interlace: %u",
          interlace);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Bits per sample: %u",bits_per_sample);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Min sample value: %u",min_sample_value);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Max sample value: %u",max_sample_value);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Photometric "
          "interpretation: %s",GetImageProperty(image,"tiff:photometric",
          exception));
      }
    image->columns=(size_t) width;
    image->rows=(size_t) height;
    image->depth=(size_t) bits_per_sample;
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Image depth: %.20g",
        (double) image->depth);
    image->endian=MSBEndian;
    if (endian == FILLORDER_LSB2MSB)
      image->endian=LSBEndian;
#if defined(MAGICKCORE_HAVE_TIFFISBIGENDIAN)
    if (TIFFIsBigEndian(tiff) == 0)
      {
        (void) SetImageProperty(image,"tiff:endian","lsb",exception);
        image->endian=LSBEndian;
      }
    else
      {
        (void) SetImageProperty(image,"tiff:endian","msb",exception);
        image->endian=MSBEndian;
      }
#endif
    if ((photometric == PHOTOMETRIC_MINISBLACK) ||
        (photometric == PHOTOMETRIC_MINISWHITE))
      (void) SetImageColorspace(image,GRAYColorspace,exception);
    if (photometric == PHOTOMETRIC_SEPARATED)
      (void) SetImageColorspace(image,CMYKColorspace,exception);
    if (photometric == PHOTOMETRIC_CIELAB)
      (void) SetImageColorspace(image,LabColorspace,exception);
    if ((photometric == PHOTOMETRIC_YCBCR) &&
        (compress_tag != COMPRESSION_OJPEG) &&
        (compress_tag != COMPRESSION_JPEG))
      (void) SetImageColorspace(image,YCbCrColorspace,exception);
    status=TIFFGetProfiles(tiff,image,exception);
    if (status == MagickFalse)
      {
        TIFFClose(tiff);
        return(DestroyImageList(image));
      }
    status=TIFFGetProperties(tiff,image,exception);
    if (status == MagickFalse)
      {
        TIFFClose(tiff);
        return(DestroyImageList(image));
      }
    TIFFGetEXIFProperties(tiff,image,image_info,exception);
    TIFFGetGPSProperties(tiff,image,image_info,exception);
    if ((TIFFGetFieldDefaulted(tiff,TIFFTAG_XRESOLUTION,&x_resolution,sans) == 1) &&
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_YRESOLUTION,&y_resolution,sans) == 1))
      {
        image->resolution.x=x_resolution;
        image->resolution.y=y_resolution;
      }
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_RESOLUTIONUNIT,&units,sans,sans) == 1)
      {
        if (units == RESUNIT_INCH)
          image->units=PixelsPerInchResolution;
        if (units == RESUNIT_CENTIMETER)
          image->units=PixelsPerCentimeterResolution;
      }
    if ((TIFFGetFieldDefaulted(tiff,TIFFTAG_XPOSITION,&x_position,sans) == 1) &&
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_YPOSITION,&y_position,sans) == 1))
      {
        image->page.x=CastDoubleToLong(ceil(x_position*
          image->resolution.x-0.5));
        image->page.y=CastDoubleToLong(ceil(y_position*
          image->resolution.y-0.5));
      }
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_ORIENTATION,&orientation,sans) == 1)
      image->orientation=(OrientationType) orientation;
    if (TIFFGetField(tiff,TIFFTAG_WHITEPOINT,&chromaticity) == 1)
      {
        if ((chromaticity != (float *) NULL) && (*chromaticity != 0.0))
          {
            image->chromaticity.white_point.x=chromaticity[0];
            image->chromaticity.white_point.y=chromaticity[1];
          }
      }
    if (TIFFGetField(tiff,TIFFTAG_PRIMARYCHROMATICITIES,&chromaticity) == 1)
      {
        if ((chromaticity != (float *) NULL) && (*chromaticity != 0.0))
          {
            image->chromaticity.red_primary.x=chromaticity[0];
            image->chromaticity.red_primary.y=chromaticity[1];
            image->chromaticity.green_primary.x=chromaticity[2];
            image->chromaticity.green_primary.y=chromaticity[3];
            image->chromaticity.blue_primary.x=chromaticity[4];
            image->chromaticity.blue_primary.y=chromaticity[5];
          }
      }
#if defined(MAGICKCORE_HAVE_TIFFISCODECCONFIGURED) || (TIFFLIB_VERSION > 20040919)
    if ((compress_tag != COMPRESSION_NONE) &&
        (TIFFIsCODECConfigured(compress_tag) == 0))
      {
        TIFFClose(tiff);
        ThrowReaderException(CoderError,"CompressNotSupported");
      }
#endif
    switch (compress_tag)
    {
      case COMPRESSION_NONE: image->compression=NoCompression; break;
      case COMPRESSION_CCITTFAX3: image->compression=FaxCompression; break;
      case COMPRESSION_CCITTFAX4: image->compression=Group4Compression; break;
      case COMPRESSION_JPEG:
      {
         image->compression=JPEGCompression;
#if defined(JPEG_SUPPORT)
         {
           char
             sampling_factor[MagickPathExtent];

           uint16
             horizontal,
             vertical;

           tiff_status=TIFFGetField(tiff,TIFFTAG_YCBCRSUBSAMPLING,&horizontal,
             &vertical);
           if (tiff_status == 1)
             {
               (void) FormatLocaleString(sampling_factor,MagickPathExtent,
                 "%dx%d",horizontal,vertical);
               (void) SetImageProperty(image,"jpeg:sampling-factor",
                 sampling_factor,exception);
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "Sampling Factors: %s",sampling_factor);
             }
         }
#endif
        break;
      }
      case COMPRESSION_OJPEG: image->compression=JPEGCompression; break;
#if defined(COMPRESSION_LZMA)
      case COMPRESSION_LZMA: image->compression=LZMACompression; break;
#endif
      case COMPRESSION_LZW: image->compression=LZWCompression; break;
      case COMPRESSION_DEFLATE: image->compression=ZipCompression; break;
      case COMPRESSION_ADOBE_DEFLATE: image->compression=ZipCompression; break;
#if defined(COMPRESSION_WEBP)
      case COMPRESSION_WEBP: image->compression=WebPCompression; break;
#endif
#if defined(COMPRESSION_ZSTD)
      case COMPRESSION_ZSTD: image->compression=ZstdCompression; break;
#endif
      default: image->compression=RLECompression; break;
    }
    quantum_info=(QuantumInfo *) NULL;
    if ((photometric == PHOTOMETRIC_PALETTE) &&
        (pow(2.0,1.0*bits_per_sample) <= MaxColormapSize))
      {
        size_t
          colors;

        colors=(size_t) GetQuantumRange(bits_per_sample)+1;
        if (AcquireImageColormap(image,colors,exception) == MagickFalse)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
      }
    value=(unsigned short) image->scene;
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_PAGENUMBER,&value,&pages,sans) == 1)
      image->scene=value;
    if (image->storage_class == PseudoClass)
      {
        size_t
          range;

        uint16
          *blue_colormap = (uint16 *) NULL,
          *green_colormap = (uint16 *) NULL,
          *red_colormap = (uint16 *) NULL;

        /*
          Initialize colormap.
        */
        tiff_status=TIFFGetField(tiff,TIFFTAG_COLORMAP,&red_colormap,
          &green_colormap,&blue_colormap);
        if (tiff_status == 1)
          {
            if ((red_colormap != (uint16 *) NULL) &&
                (green_colormap != (uint16 *) NULL) &&
                (blue_colormap != (uint16 *) NULL))
              {
                range=255;  /* might be old style 8-bit colormap */
                for (i=0; i < (ssize_t) image->colors; i++)
                  if ((red_colormap[i] >= 256) || (green_colormap[i] >= 256) ||
                      (blue_colormap[i] >= 256))
                    {
                      range=65535;
                      break;
                    }
                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  image->colormap[i].red=ClampToQuantum(((double)
                    QuantumRange*red_colormap[i])/range);
                  image->colormap[i].green=ClampToQuantum(((double)
                    QuantumRange*green_colormap[i])/range);
                  image->colormap[i].blue=ClampToQuantum(((double)
                    QuantumRange*blue_colormap[i])/range);
                }
              }
          }
      }
    if (image_info->ping != MagickFalse)
      {
        if (image_info->number_scenes != 0)
          if (image->scene >= (image_info->scene+image_info->number_scenes-1))
            break;
        goto next_tiff_frame;
      }
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      {
        TIFFClose(tiff);
        return(DestroyImageList(image));
      }
    status=SetImageColorspace(image,image->colorspace,exception);
    status&=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      {
        TIFFClose(tiff);
        return(DestroyImageList(image));
      }
    /*
      Allocate memory for the image and pixel buffer.
    */
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
    if (sample_format == SAMPLEFORMAT_UINT)
      status=SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
    if (sample_format == SAMPLEFORMAT_INT)
      status=SetQuantumFormat(image,quantum_info,SignedQuantumFormat);
    if (sample_format == SAMPLEFORMAT_IEEEFP)
      status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
    if (status == MagickFalse)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
    status=MagickTrue;
    switch (photometric)
    {
      case PHOTOMETRIC_MINISBLACK:
      {
        quantum_info->min_is_white=MagickFalse;
        break;
      }
      case PHOTOMETRIC_MINISWHITE:
      {
        quantum_info->min_is_white=MagickTrue;
        break;
      }
      default:
        break;
    }
    extra_samples=0;
    tiff_status=TIFFGetFieldDefaulted(tiff,TIFFTAG_EXTRASAMPLES,&extra_samples,
      &sample_info,sans);
    if (tiff_status == 1)
      {
        (void) SetImageProperty(image,"tiff:alpha","unspecified",exception);
        if (extra_samples == 0)
          {
            if ((samples_per_pixel == 4) && (photometric == PHOTOMETRIC_RGB))
              image->alpha_trait=BlendPixelTrait;
          }
        else
          for (i=0; i < extra_samples; i++)
          {
            image->alpha_trait=BlendPixelTrait;
            if (sample_info[i] == EXTRASAMPLE_ASSOCALPHA)
              {
                SetQuantumAlphaType(quantum_info,AssociatedQuantumAlpha);
                (void) SetImageProperty(image,"tiff:alpha","associated",
                  exception);
              }
            else
              if (sample_info[i] == EXTRASAMPLE_UNASSALPHA)
                {
                  SetQuantumAlphaType(quantum_info,DisassociatedQuantumAlpha);
                  (void) SetImageProperty(image,"tiff:alpha","unassociated",
                    exception);
                }
          }
      }
    if (image->alpha_trait != UndefinedPixelTrait)
      (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
    method=ReadGenericMethod;
    rows_per_strip=(uint32) image->rows;
    if (TIFFGetField(tiff,TIFFTAG_ROWSPERSTRIP,&rows_per_strip) == 1)
      {
        char
          buffer[MagickPathExtent];

        (void) FormatLocaleString(buffer,MagickPathExtent,"%u",
          (unsigned int) rows_per_strip);
        (void) SetImageProperty(image,"tiff:rows-per-strip",buffer,exception);
        method=ReadStripMethod;
        if (rows_per_strip > (uint32) image->rows)
          rows_per_strip=(uint32) image->rows;
      }
    if (TIFFIsTiled(tiff) != MagickFalse)
      {
        uint32
          columns,
          rows;

        if ((TIFFGetField(tiff,TIFFTAG_TILEWIDTH,&columns) != 1) ||
            (TIFFGetField(tiff,TIFFTAG_TILELENGTH,&rows) != 1))
          ThrowTIFFException(CoderError,"ImageIsNotTiled");
        if ((AcquireMagickResource(WidthResource,columns) == MagickFalse) ||
            (AcquireMagickResource(HeightResource,rows) == MagickFalse))
          ThrowTIFFException(ImageError,"WidthOrHeightExceedsLimit");
        method=ReadTileMethod;
      }
    if ((photometric == PHOTOMETRIC_LOGLUV) ||
        (compress_tag == COMPRESSION_CCITTFAX3))
      method=ReadGenericMethod;
    if (image->compression == JPEGCompression)
      method=GetJPEGMethod(image,tiff,photometric,bits_per_sample,
        samples_per_pixel);
    quantum_info->endian=LSBEndian;
    scanline_size=TIFFScanlineSize(tiff);
    if (scanline_size <= 0)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
    number_pixels=MagickMax((MagickSizeType) image->columns*samples_per_pixel*
      pow(2.0,ceil(log(bits_per_sample)/log(2.0))),image->columns*
      rows_per_strip);
    if ((double) scanline_size > 1.5*number_pixels)
      ThrowTIFFException(CorruptImageError,"CorruptImage");
    number_pixels=MagickMax((MagickSizeType) scanline_size,number_pixels);
    pixel_info=AcquireVirtualMemory(number_pixels,sizeof(uint32));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    (void) memset(pixels,0,number_pixels*sizeof(uint32));
    quantum_type=GrayQuantum;
    if (image->storage_class == PseudoClass)
      quantum_type=IndexQuantum;
    if (interlace != PLANARCONFIG_SEPARATE)
      {
        size_t
          pad;

        pad=(size_t) MagickMax((ssize_t) samples_per_pixel-1,0);
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            if (image->storage_class == PseudoClass)
              quantum_type=IndexAlphaQuantum;
            else
              quantum_type=samples_per_pixel == 1 ? AlphaQuantum :
                GrayAlphaQuantum;
          }
        if ((samples_per_pixel > 2) && (interlace != PLANARCONFIG_SEPARATE))
          {
            quantum_type=RGBQuantum;
            pad=(size_t) MagickMax((size_t) samples_per_pixel-3,0);
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                quantum_type=RGBAQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
              }
            if (image->colorspace == CMYKColorspace)
              {
                quantum_type=CMYKQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
                if (image->alpha_trait != UndefinedPixelTrait)
                  {
                    quantum_type=CMYKAQuantum;
                    pad=(size_t) MagickMax((size_t) samples_per_pixel-5,0);
                  }
              }
            status=SetQuantumPad(image,quantum_info,pad*((bits_per_sample+7) >>
              3));
            if (status == MagickFalse)
              ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
          }
      }
    switch (method)
    {
      case ReadYCCKMethod:
      {
        /*
          Convert YCC TIFF image.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          Quantum
            *magick_restrict q;

          ssize_t
            x;

          unsigned char
            *p;

          tiff_status=TIFFReadPixels(tiff,0,y,(char *) pixels);
          if (tiff_status == -1)
            break;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=pixels;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelCyan(image,ScaleCharToQuantum(ClampYCC((double) *p+
              (1.402*(double) *(p+2))-179.456)),q);
            SetPixelMagenta(image,ScaleCharToQuantum(ClampYCC((double) *p-
              (0.34414*(double) *(p+1))-(0.71414*(double ) *(p+2))+
              135.45984)),q);
            SetPixelYellow(image,ScaleCharToQuantum(ClampYCC((double) *p+
              (1.772*(double) *(p+1))-226.816)),q);
            SetPixelBlack(image,ScaleCharToQuantum((unsigned char) *(p+3)),q);
            q+=GetPixelChannels(image);
            p+=4;
          }
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
        break;
      }
      case ReadStripMethod:
      {
        unsigned char
          *p;

        size_t
          extent;

        ssize_t
          stride,
          strip_id;

        tsize_t
          strip_size;

        unsigned char
          *strip_pixels;

        /*
          Convert stripped TIFF image.
        */
        extent=(samples_per_pixel+1)*TIFFStripSize(tiff);
#if defined(TIFF_VERSION_BIG)
        extent+=image->columns*sizeof(uint64);
#else
        extent+=image->columns*sizeof(uint32);
#endif
        strip_pixels=(unsigned char *) AcquireQuantumMemory(extent,
          sizeof(*strip_pixels));
        if (strip_pixels == (unsigned char *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(strip_pixels,0,extent*sizeof(*strip_pixels));
        stride=TIFFVStripSize(tiff,1);
        strip_id=0;
        p=strip_pixels;
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          size_t
            rows_remaining;

          switch (i)
          {
            case 0: break;
            case 1: quantum_type=GreenQuantum; break;
            case 2: quantum_type=BlueQuantum; break;
            case 3:
            {
              quantum_type=AlphaQuantum;
              if (image->colorspace == CMYKColorspace)
                quantum_type=BlackQuantum;
              break;
            }
            case 4: quantum_type=AlphaQuantum; break;
            default: break;
          }
          rows_remaining=0;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            Quantum
              *magick_restrict q;

            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            if (rows_remaining == 0)
              {
                strip_size=TIFFReadEncodedStrip(tiff,strip_id,strip_pixels,
                  TIFFStripSize(tiff));
                if (strip_size == -1)
                  break;
                rows_remaining=rows_per_strip;
                if ((y+rows_per_strip) > (ssize_t) image->rows)
                  rows_remaining=(rows_per_strip-(y+rows_per_strip-
                    image->rows));
                p=strip_pixels;
                strip_id++;
              }
            (void) ImportQuantumPixels(image,(CacheView *) NULL,
              quantum_info,quantum_type,p,exception);
            p+=stride;
            rows_remaining--;
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
          if ((samples_per_pixel > 1) && (interlace != PLANARCONFIG_SEPARATE))
            break;
        }
        strip_pixels=(unsigned char *) RelinquishMagickMemory(strip_pixels);
        break;
      }
      case ReadTileMethod:
      {
        unsigned char
          *p;

        size_t
          extent;

        uint32
          columns,
          rows;

        unsigned char
          *tile_pixels;

        /*
          Convert tiled TIFF image.
        */
        if ((TIFFGetField(tiff,TIFFTAG_TILEWIDTH,&columns) != 1) ||
            (TIFFGetField(tiff,TIFFTAG_TILELENGTH,&rows) != 1))
          ThrowTIFFException(CoderError,"ImageIsNotTiled");
        number_pixels=(MagickSizeType) columns*rows;
        if (HeapOverflowSanityCheck(rows,sizeof(*tile_pixels)) != MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        extent=4*MagickMax(rows*TIFFTileRowSize(tiff),TIFFTileSize(tiff));
#if defined(TIFF_VERSION_BIG)
        extent+=image->columns*sizeof(uint64);
#else
        extent+=image->columns*sizeof(uint32);
#endif
        tile_pixels=(unsigned char *) AcquireQuantumMemory(extent,
          sizeof(*tile_pixels));
        if (tile_pixels == (unsigned char *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(tile_pixels,0,extent*sizeof(*tile_pixels));
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          switch (i)
          {
            case 0: break;
            case 1: quantum_type=GreenQuantum; break;
            case 2: quantum_type=BlueQuantum; break;
            case 3:
            {
              quantum_type=AlphaQuantum;
              if (image->colorspace == CMYKColorspace)
                quantum_type=BlackQuantum;
              break;
            }
            case 4: quantum_type=AlphaQuantum; break;
            default: break;
          }
          for (y=0; y < (ssize_t) image->rows; y+=rows)
          {
            ssize_t
              x;

            size_t
              rows_remaining;

            rows_remaining=image->rows-y;
            if ((ssize_t) (y+rows) < (ssize_t) image->rows)
              rows_remaining=rows;
            for (x=0; x < (ssize_t) image->columns; x+=columns)
            {
              size_t
                columns_remaining,
                row;

              columns_remaining=image->columns-x;
              if ((ssize_t) (x+columns) < (ssize_t) image->columns)
                columns_remaining=columns;
              tiff_status=TIFFReadTile(tiff,tile_pixels,(uint32) x,(uint32) y,
                0,i);
              if (tiff_status == -1)
                break;
              p=tile_pixels;
              for (row=0; row < rows_remaining; row++)
              {
                Quantum
                  *magick_restrict q;

                q=GetAuthenticPixels(image,x,y+row,columns_remaining,1,
                  exception);
                if (q == (Quantum *) NULL)
                  break;
                (void) ImportQuantumPixels(image,(CacheView *) NULL,
                  quantum_info,quantum_type,p,exception);
                p+=TIFFTileRowSize(tiff);
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
            }
          }
          if ((samples_per_pixel > 1) && (interlace != PLANARCONFIG_SEPARATE))
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) i,
                samples_per_pixel);
              if (status == MagickFalse)
                break;
            }
        }
        tile_pixels=(unsigned char *) RelinquishMagickMemory(tile_pixels);
        break;
      }
      case ReadGenericMethod:
      default:
      {
        MemoryInfo
          *generic_info = (MemoryInfo * ) NULL;

        uint32
          *p;

        /*
          Convert generic TIFF image.
        */
        if (HeapOverflowSanityCheck(image->rows,sizeof(*pixels)) != MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        number_pixels=(MagickSizeType) image->columns*image->rows;
#if defined(TIFF_VERSION_BIG)
        number_pixels+=image->columns*sizeof(uint64);
#else
        number_pixels+=image->columns*sizeof(uint32);
#endif
        generic_info=AcquireVirtualMemory(number_pixels,sizeof(uint32));
        if (generic_info == (MemoryInfo *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        p=(uint32 *) GetVirtualMemoryBlob(generic_info);
        tiff_status=TIFFReadRGBAImage(tiff,(uint32) image->columns,(uint32)
          image->rows,(uint32 *) p,0);
        if (tiff_status == -1)
          {
            generic_info=RelinquishVirtualMemory(generic_info);
            break;
          }
        p+=(image->columns*image->rows)-1;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          ssize_t
            x;

          Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          q+=GetPixelChannels(image)*(image->columns-1);
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(image,ScaleCharToQuantum((unsigned char)
              TIFFGetR(*p)),q);
            SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
              TIFFGetG(*p)),q);
            SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
              TIFFGetB(*p)),q);
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                TIFFGetA(*p)),q);
            p--;
            q-=GetPixelChannels(image);
          }
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
        generic_info=RelinquishVirtualMemory(generic_info);
        break;
      }
    }
    pixel_info=RelinquishVirtualMemory(pixel_info);
    SetQuantumImageType(image,quantum_type);
  next_tiff_frame:
    if (quantum_info != (QuantumInfo *) NULL)
      quantum_info=DestroyQuantumInfo(quantum_info);
    if (tiff_status == -1)
      {
        status=MagickFalse;
        break;
      }
    if (photometric == PHOTOMETRIC_CIELAB)
      DecodeLabImage(image,exception);
    if ((photometric == PHOTOMETRIC_LOGL) ||
        (photometric == PHOTOMETRIC_MINISBLACK) ||
        (photometric == PHOTOMETRIC_MINISWHITE))
      {
        image->type=GrayscaleType;
        if (bits_per_sample == 1)
          image->type=BilevelType;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    more_frames=TIFFReadDirectory(tiff) != 0 ? MagickTrue : MagickFalse;
    if (more_frames != MagickFalse)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,image->scene-1,
          image->scene);
        if (status == MagickFalse)
          break;
      }
  } while ((status != MagickFalse) && (more_frames != MagickFalse));
  TIFFClose(tiff);
  if (status != MagickFalse)
    TIFFReadPhotoshopLayers(image_info,image,exception);
  if ((image_info->number_scenes != 0) &&
      (image_info->scene >= GetImageListLength(image)))
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r T I F F I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterTIFFImage() adds properties for the TIFF image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterTIFFImage method is:
%
%      size_t RegisterTIFFImage(void)
%
*/

#if defined(MAGICKCORE_TIFF_DELEGATE)
#if defined(MAGICKCORE_HAVE_TIFFMERGEFIELDINFO) && defined(MAGICKCORE_HAVE_TIFFSETTAGEXTENDER)
static TIFFExtendProc
  tag_extender = (TIFFExtendProc) NULL;

static void TIFFIgnoreTags(TIFF *tiff)
{
  char
    *q;

  const char
    *p,
    *tags;

  Image
   *image;

  ssize_t
    i;

  size_t
    count;

  TIFFFieldInfo
    *ignore;

  if (TIFFGetReadProc(tiff) != TIFFReadBlob)
    return;
  image=(Image *)TIFFClientdata(tiff);
  tags=GetImageArtifact(image,"tiff:ignore-tags");
  if (tags == (const char *) NULL)
    return;
  count=0;
  p=tags;
  while (*p != '\0')
  {
    while ((isspace((int) ((unsigned char) *p)) != 0))
      p++;

    (void) strtol(p,&q,10);
    if (p == q)
      return;

    p=q;
    count++;

    while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
      p++;
  }
  if (count == 0)
    return;
  i=0;
  p=tags;
  ignore=(TIFFFieldInfo *) AcquireQuantumMemory(count,sizeof(*ignore));
  if (ignore == (TIFFFieldInfo *) NULL)
    return;
  /*
    This also sets field_bit to 0 (FIELD_IGNORE).
  */
  (void) memset(ignore,0,count*sizeof(*ignore));
  while (*p != '\0')
  {
    while ((isspace((int) ((unsigned char) *p)) != 0))
      p++;

    ignore[i].field_tag=(ttag_t) strtol(p,&q,10);

    p=q;
    i++;

    while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
      p++;
  }
  (void) TIFFMergeFieldInfo(tiff,ignore,(uint32) count);
  ignore=(TIFFFieldInfo *) RelinquishMagickMemory(ignore);
}

static void TIFFTagExtender(TIFF *tiff)
{
  static const TIFFFieldInfo
    TIFFExtensions[] =
    {
      { 37724, -3, -3, TIFF_UNDEFINED, FIELD_CUSTOM, 1, 1,
        (char *) "PhotoshopLayerData" },
      { 34118, -3, -3, TIFF_UNDEFINED, FIELD_CUSTOM, 1, 1,
        (char *) "Microscope" }
    };

  TIFFMergeFieldInfo(tiff,TIFFExtensions,sizeof(TIFFExtensions)/
    sizeof(*TIFFExtensions));
  if (tag_extender != (TIFFExtendProc) NULL)
    (*tag_extender)(tiff);
  TIFFIgnoreTags(tiff);
}
#endif
#endif

ModuleExport size_t RegisterTIFFImage(void)
{
#define TIFFDescription  "Tagged Image File Format"

  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

#if defined(MAGICKCORE_TIFF_DELEGATE)
  if (tiff_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&tiff_semaphore);
  LockSemaphoreInfo(tiff_semaphore);
  if (instantiate_key == MagickFalse)
    {
      if (CreateMagickThreadKey(&tiff_exception,NULL) == MagickFalse)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      error_handler=TIFFSetErrorHandler(TIFFErrors);
      warning_handler=TIFFSetWarningHandler(TIFFWarnings);
#if defined(MAGICKCORE_HAVE_TIFFMERGEFIELDINFO) && defined(MAGICKCORE_HAVE_TIFFSETTAGEXTENDER)
      if (tag_extender == (TIFFExtendProc) NULL)
        tag_extender=TIFFSetTagExtender(TIFFTagExtender);
#endif
      instantiate_key=MagickTrue;
    }
  UnlockSemaphoreInfo(tiff_semaphore);
#endif
  *version='\0';
#if defined(TIFF_VERSION)
  (void) FormatLocaleString(version,MagickPathExtent,"%d",TIFF_VERSION);
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  {
    const char
      *p;

    ssize_t
      i;

    p=TIFFGetVersion();
    for (i=0; (i < (MagickPathExtent-1)) && (*p != 0) && (*p != '\n'); i++)
      version[i]=(*p++);
    version[i]='\0';
  }
#endif

  entry=AcquireMagickInfo("TIFF","GROUP4","Raw CCITT Group4");
#if defined(MAGICKCORE_TIFF_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadGROUP4Image;
  entry->encoder=(EncodeImageHandler *) WriteGROUP4Image;
#endif
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderUseExtensionFlag;
  entry->format_type=ImplicitFormatType;
  entry->mime_type=ConstantString("image/tiff");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TIFF","PTIF","Pyramid encoded TIFF");
#if defined(MAGICKCORE_TIFF_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTIFFImage;
  entry->encoder=(EncodeImageHandler *) WritePTIFImage;
#endif
  entry->flags|=CoderEndianSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderUseExtensionFlag;
  entry->mime_type=ConstantString("image/tiff");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TIFF","TIF",TIFFDescription);
#if defined(MAGICKCORE_TIFF_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTIFFImage;
  entry->encoder=(EncodeImageHandler *) WriteTIFFImage;
#endif
  entry->flags|=CoderEndianSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags^=CoderUseExtensionFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/tiff");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TIFF","TIFF",TIFFDescription);
#if defined(MAGICKCORE_TIFF_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTIFFImage;
  entry->encoder=(EncodeImageHandler *) WriteTIFFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTIFF;
  entry->flags|=CoderEndianSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderUseExtensionFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/tiff");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TIFF","TIFF64","Tagged Image File Format (64-bit)");
#if defined(TIFF_VERSION_BIG)
  entry->decoder=(DecodeImageHandler *) ReadTIFFImage;
  entry->encoder=(EncodeImageHandler *) WriteTIFFImage;
#endif
  entry->flags|=CoderEndianSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderUseExtensionFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/tiff");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r T I F F I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterTIFFImage() removes format registrations made by the TIFF module
%  from the list of supported formats.
%
%  The format of the UnregisterTIFFImage method is:
%
%      UnregisterTIFFImage(void)
%
*/
ModuleExport void UnregisterTIFFImage(void)
{
  (void) UnregisterMagickInfo("TIFF64");
  (void) UnregisterMagickInfo("TIFF");
  (void) UnregisterMagickInfo("TIF");
  (void) UnregisterMagickInfo("PTIF");
#if defined(MAGICKCORE_TIFF_DELEGATE)
  if (tiff_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&tiff_semaphore);
  LockSemaphoreInfo(tiff_semaphore);
  if (instantiate_key != MagickFalse)
    {
#if defined(MAGICKCORE_HAVE_TIFFMERGEFIELDINFO) && defined(MAGICKCORE_HAVE_TIFFSETTAGEXTENDER)
      if (tag_extender == (TIFFExtendProc) NULL)
        (void) TIFFSetTagExtender(tag_extender);
#endif
      if (DeleteMagickThreadKey(tiff_exception) == MagickFalse)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      (void) TIFFSetWarningHandler(warning_handler);
      (void) TIFFSetErrorHandler(error_handler);
      instantiate_key=MagickFalse;
    }
  UnlockSemaphoreInfo(tiff_semaphore);
  RelinquishSemaphoreInfo(&tiff_semaphore);
#endif
}

#if defined(MAGICKCORE_TIFF_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e G R O U P 4 I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteGROUP4Image() writes an image in the raw CCITT Group 4 image format.
%
%  The format of the WriteGROUP4Image method is:
%
%      MagickBooleanType WriteGROUP4Image(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WriteGROUP4Image(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  FILE
    *file;

  Image
    *huffman_image;

  ImageInfo
    *write_info;

  int
    unique_file;

  MagickBooleanType
    status;

  ssize_t
    i;

  ssize_t
    count;

  TIFF
    *tiff;

  toff_t
    *byte_count,
    strip_size;

  unsigned char
    *buffer;

  /*
    Write image as CCITT Group4 TIFF image to a temporary file.
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
  huffman_image=CloneImage(image,0,0,MagickTrue,exception);
  if (huffman_image == (Image *) NULL)
    {
      (void) CloseBlob(image);
      return(MagickFalse);
    }
  huffman_image->endian=MSBEndian;
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        filename);
      return(MagickFalse);
    }
  (void) FormatLocaleString(huffman_image->filename,MagickPathExtent,"tiff:%s",
    filename);
  if (IsImageMonochrome(image) == MagickFalse)
    (void) SetImageType(huffman_image,BilevelType,exception);
  write_info=CloneImageInfo((ImageInfo *) NULL);
  SetImageInfoFile(write_info,file);
  if (IsImageMonochrome(image) == MagickFalse)
    (void) SetImageType(image,BilevelType,exception);
  (void) SetImageDepth(image,1,exception);
  write_info->compression=Group4Compression;
  write_info->type=BilevelType;
  status=WriteTIFFImage(write_info,huffman_image,exception);
  (void) fflush(file);
  write_info=DestroyImageInfo(write_info);
  if (status == MagickFalse)
    {
      huffman_image=DestroyImage(huffman_image);
      (void) fclose(file);
      (void) RelinquishUniqueFileResource(filename);
      return(MagickFalse);
    }
  tiff=TIFFOpen(filename,"rb");
  if (tiff == (TIFF *) NULL)
    {
      huffman_image=DestroyImage(huffman_image);
      (void) fclose(file);
      (void) RelinquishUniqueFileResource(filename);
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      return(MagickFalse);
    }
  /*
    Allocate raw strip buffer.
  */
  if (TIFFGetField(tiff,TIFFTAG_STRIPBYTECOUNTS,&byte_count) != 1)
    {
      TIFFClose(tiff);
      huffman_image=DestroyImage(huffman_image);
      (void) fclose(file);
      (void) RelinquishUniqueFileResource(filename);
      return(MagickFalse);
    }
  strip_size=byte_count[0];
  for (i=1; i < (ssize_t) TIFFNumberOfStrips(tiff); i++)
    if (byte_count[i] > strip_size)
      strip_size=byte_count[i];
  buffer=(unsigned char *) AcquireQuantumMemory((size_t) strip_size,
    sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      TIFFClose(tiff);
      huffman_image=DestroyImage(huffman_image);
      (void) fclose(file);
      (void) RelinquishUniqueFileResource(filename);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image_info->filename);
    }
  /*
    Compress runlength encoded to 2D Huffman pixels.
  */
  for (i=0; i < (ssize_t) TIFFNumberOfStrips(tiff); i++)
  {
    count=(ssize_t) TIFFReadRawStrip(tiff,(uint32) i,buffer,strip_size);
    if (WriteBlob(image,(size_t) count,buffer) != count)
      status=MagickFalse;
  }
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  TIFFClose(tiff);
  huffman_image=DestroyImage(huffman_image);
  (void) fclose(file);
  (void) RelinquishUniqueFileResource(filename);
  (void) CloseBlob(image);
  return(status);
}
#endif

#if defined(MAGICKCORE_TIFF_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P T I F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePTIFImage() writes an image in the pyrimid-encoded Tagged image file
%  format.
%
%  The format of the WritePTIFImage method is:
%
%      MagickBooleanType WritePTIFImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WritePTIFImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  Image
    *images,
    *next,
    *pyramid_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  PointInfo
    resolution;

  size_t
    columns,
    rows;

  /*
    Create pyramid-encoded TIFF image.
  */
  images=NewImageList();
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    Image
      *clone_image;

    clone_image=CloneImage(next,0,0,MagickFalse,exception);
    if (clone_image == (Image *) NULL)
      break;
    clone_image->previous=NewImageList();
    clone_image->next=NewImageList();
    (void) SetImageProperty(clone_image,"tiff:subfiletype","none",exception);
    AppendImageToList(&images,clone_image);
    columns=next->columns;
    rows=next->rows;
    resolution=next->resolution;
    while ((columns > 64) && (rows > 64))
    {
      columns/=2;
      rows/=2;
      resolution.x/=2;
      resolution.y/=2;
      pyramid_image=ResizeImage(next,columns,rows,image->filter,exception);
      if (pyramid_image == (Image *) NULL)
        break;
      DestroyBlob(pyramid_image);
      pyramid_image->blob=ReferenceBlob(next->blob);
      pyramid_image->resolution=resolution;
      (void) SetImageProperty(pyramid_image,"tiff:subfiletype","REDUCEDIMAGE",
        exception);
      AppendImageToList(&images,pyramid_image);
    }
  }
  status=MagickFalse;
  if (images != (Image *) NULL)
    {
      /*
        Write pyramid-encoded TIFF image.
      */
      images=GetFirstImageInList(images);
      write_info=CloneImageInfo(image_info);
      write_info->adjoin=MagickTrue;
      (void) CopyMagickString(write_info->magick,"TIFF",MagickPathExtent);
      (void) CopyMagickString(images->magick,"TIFF",MagickPathExtent);
      status=WriteTIFFImage(write_info,images,exception);
      images=DestroyImageList(images);
      write_info=DestroyImageInfo(write_info);
    }
  return(status);
}
#endif

#if defined(MAGICKCORE_TIFF_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteTIFFImage() writes an image in the Tagged image file format.
%
%  The format of the WriteTIFFImage method is:
%
%      MagickBooleanType WriteTIFFImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _TIFFInfo
{
  RectangleInfo
    tile_geometry;

  unsigned char
    *scanline,
    *scanlines,
    *pixels;
} TIFFInfo;

static void DestroyTIFFInfo(TIFFInfo *tiff_info)
{
  assert(tiff_info != (TIFFInfo *) NULL);
  if (tiff_info->scanlines != (unsigned char *) NULL)
    tiff_info->scanlines=(unsigned char *) RelinquishMagickMemory(
      tiff_info->scanlines);
  if (tiff_info->pixels != (unsigned char *) NULL)
    tiff_info->pixels=(unsigned char *) RelinquishMagickMemory(
      tiff_info->pixels);
}

static MagickBooleanType EncodeLabImage(Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        break;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        a,
        b;

      a=QuantumScale*GetPixela(image,q)-0.5;
      if (a < 0.0)
        a+=1.0;
      b=QuantumScale*GetPixelb(image,q)-0.5;
      if (b < 0.0)
        b+=1.0;
      SetPixela(image,QuantumRange*a,q);
      SetPixelb(image,QuantumRange*b,q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      {
        status=MagickFalse;
        break;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType GetTIFFInfo(const ImageInfo *image_info,
  TIFF *tiff,TIFFInfo *tiff_info)
{
#define TIFFStripSizeDefault  1048576

  const char
    *option;

  MagickStatusType
    flags;

  uint32
    tile_columns,
    tile_rows;

  assert(tiff_info != (TIFFInfo *) NULL);
  (void) memset(tiff_info,0,sizeof(*tiff_info));
  option=GetImageOption(image_info,"tiff:tile-geometry");
  if (option == (const char *) NULL)
    {
      size_t
        extent;

      uint32
        rows,
        rows_per_strip;

      extent=TIFFScanlineSize(tiff);
      rows_per_strip=TIFFStripSizeDefault/(extent == 0 ? 1 : (uint32) extent);
      rows_per_strip=16*(((rows_per_strip < 16 ? 16 : rows_per_strip)+1)/16);
      TIFFGetField(tiff,TIFFTAG_IMAGELENGTH,&rows);
      if (rows_per_strip > rows)
        rows_per_strip=rows;
      option=GetImageOption(image_info,"tiff:rows-per-strip");
      if (option != (const char *) NULL)
        rows_per_strip=(uint32) strtoul(option,(char **) NULL,10);
      rows_per_strip=TIFFDefaultStripSize(tiff,rows_per_strip);
      (void) TIFFSetField(tiff,TIFFTAG_ROWSPERSTRIP,rows_per_strip);
      return(MagickTrue);
    }
  /*
    Create tiled TIFF, ignore "tiff:rows-per-strip".
  */
  flags=ParseAbsoluteGeometry(option,&tiff_info->tile_geometry);
  if ((flags & HeightValue) == 0)
    tiff_info->tile_geometry.height=tiff_info->tile_geometry.width;
  tile_columns=(uint32) tiff_info->tile_geometry.width;
  tile_rows=(uint32) tiff_info->tile_geometry.height;
  TIFFDefaultTileSize(tiff,&tile_columns,&tile_rows);
  (void) TIFFSetField(tiff,TIFFTAG_TILEWIDTH,tile_columns);
  (void) TIFFSetField(tiff,TIFFTAG_TILELENGTH,tile_rows);
  tiff_info->tile_geometry.width=tile_columns;
  tiff_info->tile_geometry.height=tile_rows;
  if ((TIFFScanlineSize(tiff) <= 0) || (TIFFTileSize(tiff) <= 0))
    {
      DestroyTIFFInfo(tiff_info);
      return(MagickFalse);
    }
  tiff_info->scanlines=(unsigned char *) AcquireQuantumMemory((size_t)
    tile_rows*TIFFScanlineSize(tiff),sizeof(*tiff_info->scanlines));
  tiff_info->pixels=(unsigned char *) AcquireQuantumMemory((size_t)
    tile_rows*TIFFTileSize(tiff),sizeof(*tiff_info->scanlines));
  if ((tiff_info->scanlines == (unsigned char *) NULL) ||
      (tiff_info->pixels == (unsigned char *) NULL))
    {
      DestroyTIFFInfo(tiff_info);
      return(MagickFalse);
    }
  return(MagickTrue);
}

static tmsize_t TIFFWritePixels(TIFF *tiff,TIFFInfo *tiff_info,ssize_t row,
  tsample_t sample,Image *image)
{
  ssize_t
    i;

  tmsize_t
    status;

  size_t
    number_tiles,
    tile_width;

  ssize_t
    bytes_per_pixel,
    j,
    k,
    l;

  unsigned char
    *p,
    *q;

  if (TIFFIsTiled(tiff) == 0)
    return(TIFFWriteScanline(tiff,tiff_info->scanline,(uint32) row,sample));
  /*
    Fill scanlines to tile height.
  */
  i=(ssize_t) (row % tiff_info->tile_geometry.height)*TIFFScanlineSize(tiff);
  (void) memcpy(tiff_info->scanlines+i,(char *) tiff_info->scanline,
    (size_t) TIFFScanlineSize(tiff));
  if (((size_t) (row % tiff_info->tile_geometry.height) !=
      (tiff_info->tile_geometry.height-1)) &&
      (row != (ssize_t) (image->rows-1)))
    return(0);
  /*
    Write tile to TIFF image.
  */
  status=0;
  bytes_per_pixel=TIFFTileSize(tiff)/(ssize_t) (
    tiff_info->tile_geometry.height*tiff_info->tile_geometry.width);
  number_tiles=(image->columns+tiff_info->tile_geometry.width)/
    tiff_info->tile_geometry.width;
  for (i=0; i < (ssize_t) number_tiles; i++)
  {
    tile_width=(i == (ssize_t) (number_tiles-1)) ? image->columns-(i*
      tiff_info->tile_geometry.width) : tiff_info->tile_geometry.width;
    for (j=0; j < (ssize_t) ((row % tiff_info->tile_geometry.height)+1); j++)
      for (k=0; k < (ssize_t) tile_width; k++)
      {
        if (bytes_per_pixel == 0)
          {
            p=tiff_info->scanlines+(j*TIFFScanlineSize(tiff)+(i*
              tiff_info->tile_geometry.width+k)/8);
            q=tiff_info->pixels+(j*TIFFTileRowSize(tiff)+k/8);
            *q++=(*p++);
            continue;
          }
        p=tiff_info->scanlines+(j*TIFFScanlineSize(tiff)+(i*
          tiff_info->tile_geometry.width+k)*bytes_per_pixel);
        q=tiff_info->pixels+(j*TIFFTileRowSize(tiff)+k*bytes_per_pixel);
        for (l=0; l < bytes_per_pixel; l++)
          *q++=(*p++);
      }
    if ((i*tiff_info->tile_geometry.width) != image->columns)
      status=TIFFWriteTile(tiff,tiff_info->pixels,(uint32) (i*
        tiff_info->tile_geometry.width),(uint32) ((row/
        tiff_info->tile_geometry.height)*tiff_info->tile_geometry.height),0,
        sample);
    if (status < 0)
      break;
  }
  return(status);
}

static ssize_t TIFFWriteCustomStream(unsigned char *data,const size_t count,
  void *user_data)
{
  PhotoshopProfile
    *profile;

  if (count == 0)
    return(0);
  profile=(PhotoshopProfile *) user_data;
  if ((profile->offset+(MagickOffsetType) count) >=
        (MagickOffsetType) profile->extent)
    {
      profile->extent+=count+profile->quantum;
      profile->quantum<<=1;
      SetStringInfoLength(profile->data,profile->extent);
    }
  (void) memcpy(profile->data->datum+profile->offset,data,count);
  profile->offset+=count;
  return(count);
}

static CustomStreamInfo *TIFFAcquireCustomStreamForWriting(
  PhotoshopProfile *profile,ExceptionInfo *exception)
{
  CustomStreamInfo
    *custom_stream;

  custom_stream=AcquireCustomStreamInfo(exception);
  if (custom_stream == (CustomStreamInfo *) NULL)
    return(custom_stream);
  SetCustomStreamData(custom_stream,(void *) profile);
  SetCustomStreamWriter(custom_stream,TIFFWriteCustomStream);
  SetCustomStreamSeeker(custom_stream,TIFFSeekCustomStream);
  SetCustomStreamTeller(custom_stream,TIFFTellCustomStream);
  return(custom_stream);
}

static MagickBooleanType TIFFWritePhotoshopLayers(Image* image,
  const ImageInfo *image_info,EndianType endian,ExceptionInfo *exception)
{
  BlobInfo
    *blob;

  CustomStreamInfo
    *custom_stream;

  Image
    *base_image,
    *next;

  ImageInfo
    *clone_info;

  MagickBooleanType
    status;

  PhotoshopProfile
    profile;

  PSDInfo
    info;

  StringInfo
    *layers;

  base_image=CloneImage(image,0,0,MagickFalse,exception);
  if (base_image == (Image *) NULL)
    return(MagickTrue);
  clone_info=CloneImageInfo(image_info);
  if (clone_info == (ImageInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  profile.offset=0;
  profile.quantum=MagickMinBlobExtent;
  layers=AcquireStringInfo(profile.quantum);
  if (layers == (StringInfo *) NULL)
    {
      base_image=DestroyImage(base_image);
      clone_info=DestroyImageInfo(clone_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  profile.data=layers;
  profile.extent=layers->length;
  custom_stream=TIFFAcquireCustomStreamForWriting(&profile,exception);
  if (custom_stream == (CustomStreamInfo *) NULL)
    {
      base_image=DestroyImage(base_image);
      clone_info=DestroyImageInfo(clone_info);
      layers=DestroyStringInfo(layers);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  blob=CloneBlobInfo((BlobInfo *) NULL);
  if (blob == (BlobInfo *) NULL)
    {
      base_image=DestroyImage(base_image);
      clone_info=DestroyImageInfo(clone_info);
      layers=DestroyStringInfo(layers);
      custom_stream=DestroyCustomStreamInfo(custom_stream);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  DestroyBlob(base_image);
  base_image->blob=blob;
  next=base_image;
  while (next != (Image *) NULL)
    next=SyncNextImageInList(next);
  AttachCustomStream(base_image->blob,custom_stream);
  InitPSDInfo(image,&info);
  base_image->endian=endian;
  WriteBlobString(base_image,"Adobe Photoshop Document Data Block");
  WriteBlobByte(base_image,0);
  WriteBlobString(base_image,base_image->endian == LSBEndian ? "MIB8ryaL" :
    "8BIMLayr");
  status=WritePSDLayers(base_image,clone_info,&info,exception);
  if (status != MagickFalse)
    {
      SetStringInfoLength(layers,(size_t) profile.offset);
      status=SetImageProfile(image,"tiff:37724",layers,exception);
    }
  next=base_image;
  while (next != (Image *) NULL)
  {
    CloseBlob(next);
    next=next->next;
  }
  layers=DestroyStringInfo(layers);
  clone_info=DestroyImageInfo(clone_info);
  custom_stream=DestroyCustomStreamInfo(custom_stream);
  return(status);
}

static void TIFFSetProfiles(TIFF *tiff,Image *image)
{
  const char
    *name;

  const StringInfo
    *profile;

  if (image->profiles == (void *) NULL)
    return;
  ResetImageProfileIterator(image);
  for (name=GetNextImageProfile(image); name != (const char *) NULL; )
  {
    profile=GetImageProfile(image,name);
    if (GetStringInfoLength(profile) == 0)
      {
        name=GetNextImageProfile(image);
        continue;
      }
#if defined(TIFFTAG_XMLPACKET)
    if (LocaleCompare(name,"xmp") == 0)
      (void) TIFFSetField(tiff,TIFFTAG_XMLPACKET,(uint32) GetStringInfoLength(
        profile),GetStringInfoDatum(profile));
#endif
#if defined(TIFFTAG_ICCPROFILE)
    if (LocaleCompare(name,"icc") == 0)
      (void) TIFFSetField(tiff,TIFFTAG_ICCPROFILE,(uint32) GetStringInfoLength(
        profile),GetStringInfoDatum(profile));
#endif
    if (LocaleCompare(name,"iptc") == 0)
      {
        const TIFFField
          *field;

        size_t
          length;

        StringInfo
          *iptc_profile;

        iptc_profile=CloneStringInfo(profile);
        length=GetStringInfoLength(profile)+4-(GetStringInfoLength(profile) &
          0x03);
        SetStringInfoLength(iptc_profile,length);
        field=TIFFFieldWithTag(tiff,TIFFTAG_RICHTIFFIPTC);
        if (TIFFFieldDataType(field) == TIFF_LONG)
          {
            if (TIFFIsByteSwapped(tiff))
              TIFFSwabArrayOfLong((uint32 *) GetStringInfoDatum(iptc_profile),
                (unsigned long) (length/4));
            (void) TIFFSetField(tiff,TIFFTAG_RICHTIFFIPTC,(uint32)
              GetStringInfoLength(iptc_profile)/4,GetStringInfoDatum(
                iptc_profile));
          }
        else
          (void) TIFFSetField(tiff,TIFFTAG_RICHTIFFIPTC,(uint32)
            GetStringInfoLength(iptc_profile),GetStringInfoDatum(
              iptc_profile));
        iptc_profile=DestroyStringInfo(iptc_profile);
      }
#if defined(TIFFTAG_PHOTOSHOP)
    if (LocaleCompare(name,"8bim") == 0)
      (void) TIFFSetField(tiff,TIFFTAG_PHOTOSHOP,(uint32)
        GetStringInfoLength(profile),GetStringInfoDatum(profile));
#endif
    if (LocaleCompare(name,"tiff:37724") == 0)
      (void) TIFFSetField(tiff,37724,(uint32) GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
    if (LocaleCompare(name,"tiff:34118") == 0)
      (void) TIFFSetField(tiff,34118,(uint32) GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
    name=GetNextImageProfile(image);
  }
}

static void TIFFSetProperties(TIFF *tiff,const MagickBooleanType adjoin,
  Image *image,ExceptionInfo *exception)
{
  const char
    *value;

  value=GetImageArtifact(image,"tiff:document");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_DOCUMENTNAME,value);
  value=GetImageArtifact(image,"tiff:hostcomputer");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_HOSTCOMPUTER,value);
  value=GetImageArtifact(image,"tiff:artist");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_ARTIST,value);
  value=GetImageArtifact(image,"tiff:timestamp");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_DATETIME,value);
  value=GetImageArtifact(image,"tiff:make");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_MAKE,value);
  value=GetImageArtifact(image,"tiff:model");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_MODEL,value);
  value=GetImageArtifact(image,"tiff:software");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_SOFTWARE,value);
  value=GetImageArtifact(image,"tiff:copyright");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_COPYRIGHT,value);
  value=GetImageArtifact(image,"kodak-33423");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,33423,value);
  value=GetImageArtifact(image,"kodak-36867");
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,36867,value);
  value=GetImageProperty(image,"label",exception);
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_PAGENAME,value);
  value=GetImageProperty(image,"comment",exception);
  if (value != (const char *) NULL)
    (void) TIFFSetField(tiff,TIFFTAG_IMAGEDESCRIPTION,value);
  value=GetImageArtifact(image,"tiff:subfiletype");
  if (value != (const char *) NULL)
    {
      if (LocaleCompare(value,"REDUCEDIMAGE") == 0)
        (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_REDUCEDIMAGE);
      else
        if (LocaleCompare(value,"PAGE") == 0)
          (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_PAGE);
        else
          if (LocaleCompare(value,"MASK") == 0)
            (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_MASK);
    }
  else
    {
      uint16
        page,
        pages;

      page=(uint16) image->scene;
      pages=(uint16) GetImageListLength(image);
      if ((adjoin != MagickFalse) && (pages > 1))
        (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_PAGE);
      (void) TIFFSetField(tiff,TIFFTAG_PAGENUMBER,page,pages);
    }
}

static MagickBooleanType WriteTIFFImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const char
    *mode,
    *option;

  CompressionType
    compression;

  EndianType
    endian_type;

  int
    tiff_status = 0;

  MagickBooleanType
    adjoin,
    preserve_compression,
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  ssize_t
    i;

  size_t
    imageListLength,
    length;

  ssize_t
    y;

  TIFF
    *tiff;

  TIFFInfo
    tiff_info;

  uint16
    bits_per_sample,
    compress_tag,
    endian,
    photometric,
    predictor;

  unsigned char
    *pixels;

  void
    *sans[2] = { NULL, NULL };

  /*
    Open TIFF file.
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
  (void) SetMagickThreadValue(tiff_exception,exception);
  endian_type=(HOST_FILLORDER == FILLORDER_LSB2MSB) ? LSBEndian : MSBEndian;
  option=GetImageOption(image_info,"tiff:endian");
  if (option != (const char *) NULL)
    {
      if (LocaleNCompare(option,"msb",3) == 0)
        endian_type=MSBEndian;
      if (LocaleNCompare(option,"lsb",3) == 0)
        endian_type=LSBEndian;
    }
  mode=endian_type == LSBEndian ? "wl" : "wb";
#if defined(TIFF_VERSION_BIG)
  if (LocaleCompare(image_info->magick,"TIFF64") == 0)
    mode=endian_type == LSBEndian ? "wl8" : "wb8";
#endif
  tiff=TIFFClientOpen(image->filename,mode,(thandle_t) image,TIFFReadBlob,
    TIFFWriteBlob,TIFFSeekBlob,TIFFCloseBlob,TIFFGetBlobSize,TIFFMapBlob,
    TIFFUnmapBlob);
  if (tiff == (TIFF *) NULL)
    return(MagickFalse);
  if (exception->severity > ErrorException)
    {
      TIFFClose(tiff);
      return(MagickFalse);
    }
  (void) DeleteImageProfile(image,"tiff:37724");
  scene=0;
  adjoin=image_info->adjoin;
  imageListLength=GetImageListLength(image);
  option=GetImageOption(image_info,"tiff:preserve-compression");
  preserve_compression=IsStringTrue(option);
  do
  {
    /*
      Initialize TIFF fields.
    */
    if ((image_info->type != UndefinedType) &&
        (image_info->type != OptimizeType) &&
        (image_info->type != image->type))
      (void) SetImageType(image,image_info->type,exception);
    compression=image_info->compression;
    if (preserve_compression != MagickFalse)
      compression=image->compression;
    switch (compression)
    {
      case FaxCompression:
      case Group4Compression:
      {
        if (IsImageMonochrome(image) == MagickFalse)
          {
            if (IsImageGray(image) == MagickFalse)
              (void) SetImageType(image,BilevelType,exception);
            else
              (void) SetImageDepth(image,1,exception);
          }
        image->depth=1;
        break;
      }
      case JPEGCompression:
      {
        (void) SetImageStorageClass(image,DirectClass,exception);
        (void) SetImageDepth(image,8,exception);
        break;
      }
      default:
        break;
    }
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    if ((image->storage_class != PseudoClass) && (image->depth >= 32) &&
        (quantum_info->format == UndefinedQuantumFormat) &&
        (IsHighDynamicRangeImage(image,exception) != MagickFalse))
      {
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          {
            quantum_info=DestroyQuantumInfo(quantum_info);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
      }
    if ((LocaleCompare(image_info->magick,"PTIF") == 0) &&
        (GetPreviousImageInList(image) != (Image *) NULL))
      (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_REDUCEDIMAGE);
    if ((image->columns != (uint32) image->columns) ||
        (image->rows != (uint32) image->rows))
      ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
    (void) TIFFSetField(tiff,TIFFTAG_IMAGELENGTH,(uint32) image->rows);
    (void) TIFFSetField(tiff,TIFFTAG_IMAGEWIDTH,(uint32) image->columns);
    switch (compression)
    {
      case FaxCompression:
      {
        compress_tag=COMPRESSION_CCITTFAX3;
        option=GetImageOption(image_info,"quantum:polarity");
        if (option == (const char *) NULL)
          SetQuantumMinIsWhite(quantum_info,MagickTrue);
        break;
      }
      case Group4Compression:
      {
        compress_tag=COMPRESSION_CCITTFAX4;
        option=GetImageOption(image_info,"quantum:polarity");
        if (option == (const char *) NULL)
          SetQuantumMinIsWhite(quantum_info,MagickTrue);
        break;
      }
#if defined(COMPRESSION_JBIG)
      case JBIG1Compression:
      {
        compress_tag=COMPRESSION_JBIG;
        break;
      }
#endif
      case JPEGCompression:
      {
        compress_tag=COMPRESSION_JPEG;
        break;
      }
#if defined(COMPRESSION_LZMA)
      case LZMACompression:
      {
        compress_tag=COMPRESSION_LZMA;
        break;
      }
#endif
      case LZWCompression:
      {
        compress_tag=COMPRESSION_LZW;
        break;
      }
      case RLECompression:
      {
        compress_tag=COMPRESSION_PACKBITS;
        break;
      }
      case ZipCompression:
      {
        compress_tag=COMPRESSION_ADOBE_DEFLATE;
        break;
      }
#if defined(COMPRESSION_ZSTD)
      case ZstdCompression:
      {
        compress_tag=COMPRESSION_ZSTD;
        break;
      }
#endif
      case NoCompression:
      default:
      {
        compress_tag=COMPRESSION_NONE;
        break;
      }
    }
#if defined(MAGICKCORE_HAVE_TIFFISCODECCONFIGURED) || (TIFFLIB_VERSION > 20040919)
    if ((compress_tag != COMPRESSION_NONE) &&
        (TIFFIsCODECConfigured(compress_tag) == 0))
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          "CompressionNotSupported","`%s'",CommandOptionToMnemonic(
          MagickCompressOptions,(ssize_t) compression));
        compress_tag=COMPRESSION_NONE;
        compression=NoCompression;
      }
#else
      switch (compress_tag)
      {
#if defined(CCITT_SUPPORT)
        case COMPRESSION_CCITTFAX3:
        case COMPRESSION_CCITTFAX4:
#endif
#if defined(YCBCR_SUPPORT) && defined(JPEG_SUPPORT)
        case COMPRESSION_JPEG:
#endif
#if defined(LZMA_SUPPORT) && defined(COMPRESSION_LZMA)
        case COMPRESSION_LZMA:
#endif
#if defined(LZW_SUPPORT)
        case COMPRESSION_LZW:
#endif
#if defined(PACKBITS_SUPPORT)
        case COMPRESSION_PACKBITS:
#endif
#if defined(ZIP_SUPPORT)
        case COMPRESSION_ADOBE_DEFLATE:
#endif
        case COMPRESSION_NONE:
          break;
        default:
        {
          (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
            "CompressionNotSupported","`%s'",CommandOptionToMnemonic(
            MagickCompressOptions,(ssize_t) compression));
          compress_tag=COMPRESSION_NONE;
          compression=NoCompression;
          break;
        }
      }
#endif
    if (image->colorspace == CMYKColorspace)
      {
        photometric=PHOTOMETRIC_SEPARATED;
        (void) TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,4);
        (void) TIFFSetField(tiff,TIFFTAG_INKSET,INKSET_CMYK);
      }
    else
      {
        /*
          Full color TIFF raster.
        */
        if (image->colorspace == LabColorspace)
          {
            photometric=PHOTOMETRIC_CIELAB;
            EncodeLabImage(image,exception);
          }
        else
          if (IsYCbCrCompatibleColorspace(image->colorspace) != MagickFalse)
            {
              photometric=PHOTOMETRIC_YCBCR;
              (void) TIFFSetField(tiff,TIFFTAG_YCBCRSUBSAMPLING,1,1);
              (void) SetImageStorageClass(image,DirectClass,exception);
              status=SetQuantumDepth(image,quantum_info,8);
              if (status == MagickFalse)
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
            }
          else
            photometric=PHOTOMETRIC_RGB;
        (void) TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,3);
        if ((image_info->type != TrueColorType) &&
            (image_info->type != TrueColorAlphaType))
          {
            if ((image_info->type != PaletteType) &&
                (IdentifyImageCoderGray(image,exception) != MagickFalse))
              {
                photometric=(uint16) (quantum_info->min_is_white !=
                  MagickFalse ? PHOTOMETRIC_MINISWHITE :
                  PHOTOMETRIC_MINISBLACK);
                (void) TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,1);
              }
            else
              if ((image->storage_class == PseudoClass) &&
                  (image->alpha_trait == UndefinedPixelTrait))
                {
                  size_t
                    depth;

                  /*
                    Colormapped TIFF raster.
                  */
                  (void) TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,1);
                  photometric=PHOTOMETRIC_PALETTE;
                  depth=1;
                  while ((GetQuantumRange(depth)+1) < image->colors)
                    depth<<=1;
                  status=SetQuantumDepth(image,quantum_info,depth);
                  if (status == MagickFalse)
                    ThrowWriterException(ResourceLimitError,
                      "MemoryAllocationFailed");
                }
          }
      }
    (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_FILLORDER,&endian,sans);
    if ((compress_tag == COMPRESSION_CCITTFAX3) ||
        (compress_tag == COMPRESSION_CCITTFAX4))
      {
         if ((photometric != PHOTOMETRIC_MINISWHITE) &&
             (photometric != PHOTOMETRIC_MINISBLACK))
          {
            compress_tag=COMPRESSION_NONE;
            endian=FILLORDER_MSB2LSB;
          }
      }
    option=GetImageOption(image_info,"tiff:fill-order");
    if (option != (const char *) NULL)
      {
        if (LocaleNCompare(option,"msb",3) == 0)
          endian=FILLORDER_MSB2LSB;
        if (LocaleNCompare(option,"lsb",3) == 0)
          endian=FILLORDER_LSB2MSB;
      }
    (void) TIFFSetField(tiff,TIFFTAG_COMPRESSION,compress_tag);
    (void) TIFFSetField(tiff,TIFFTAG_FILLORDER,endian);
    (void) TIFFSetField(tiff,TIFFTAG_BITSPERSAMPLE,quantum_info->depth);
    if (image->alpha_trait != UndefinedPixelTrait)
      {
        uint16
          extra_samples,
          sample_info[1],
          samples_per_pixel;

        /*
          TIFF has a matte channel.
        */
        extra_samples=1;
        sample_info[0]=EXTRASAMPLE_UNASSALPHA;
        option=GetImageOption(image_info,"tiff:alpha");
        if (option != (const char *) NULL)
          {
            if (LocaleCompare(option,"associated") == 0)
              sample_info[0]=EXTRASAMPLE_ASSOCALPHA;
            else
              if (LocaleCompare(option,"unspecified") == 0)
                sample_info[0]=EXTRASAMPLE_UNSPECIFIED;
          }
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLESPERPIXEL,
          &samples_per_pixel,sans);
        (void) TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,samples_per_pixel+1);
        (void) TIFFSetField(tiff,TIFFTAG_EXTRASAMPLES,extra_samples,
          &sample_info);
        if (sample_info[0] == EXTRASAMPLE_ASSOCALPHA)
          SetQuantumAlphaType(quantum_info,AssociatedQuantumAlpha);
      }
    (void) TIFFSetField(tiff,TIFFTAG_PHOTOMETRIC,photometric);
    switch (quantum_info->format)
    {
      case FloatingPointQuantumFormat:
      {
        (void) TIFFSetField(tiff,TIFFTAG_SAMPLEFORMAT,SAMPLEFORMAT_IEEEFP);
        (void) TIFFSetField(tiff,TIFFTAG_SMINSAMPLEVALUE,quantum_info->minimum);
        (void) TIFFSetField(tiff,TIFFTAG_SMAXSAMPLEVALUE,quantum_info->maximum);
        break;
      }
      case SignedQuantumFormat:
      {
        (void) TIFFSetField(tiff,TIFFTAG_SAMPLEFORMAT,SAMPLEFORMAT_INT);
        break;
      }
      case UnsignedQuantumFormat:
      {
        (void) TIFFSetField(tiff,TIFFTAG_SAMPLEFORMAT,SAMPLEFORMAT_UINT);
        break;
      }
      default:
        break;
    }
    (void) TIFFSetField(tiff,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    if (photometric == PHOTOMETRIC_RGB)
      if ((image_info->interlace == PlaneInterlace) ||
          (image_info->interlace == PartitionInterlace))
        (void) TIFFSetField(tiff,TIFFTAG_PLANARCONFIG,PLANARCONFIG_SEPARATE);
    predictor=0;
    switch (compress_tag)
    {
      case COMPRESSION_JPEG:
      {
#if defined(JPEG_SUPPORT)
        if (image_info->quality != UndefinedCompressionQuality)
          (void) TIFFSetField(tiff,TIFFTAG_JPEGQUALITY,image_info->quality);
        (void) TIFFSetField(tiff,TIFFTAG_JPEGCOLORMODE,JPEGCOLORMODE_RAW);
        if (IssRGBCompatibleColorspace(image->colorspace) != MagickFalse)
          {
            const char
              *value;

            (void) TIFFSetField(tiff,TIFFTAG_JPEGCOLORMODE,JPEGCOLORMODE_RGB);
            if (IsYCbCrCompatibleColorspace(image->colorspace) != MagickFalse)
              {
                const char
                  *sampling_factor;

                GeometryInfo
                  geometry_info;

                MagickStatusType
                  flags;

                sampling_factor=(const char *) NULL;
                value=GetImageProperty(image,"jpeg:sampling-factor",exception);
                if (value != (char *) NULL)
                  {
                    sampling_factor=value;
                    if (image->debug != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "  Input sampling-factors=%s",sampling_factor);
                  }
                if (image_info->sampling_factor != (char *) NULL)
                  sampling_factor=image_info->sampling_factor;
                if (sampling_factor != (const char *) NULL)
                  {
                    flags=ParseGeometry(sampling_factor,&geometry_info);
                    if ((flags & SigmaValue) == 0)
                      geometry_info.sigma=geometry_info.rho;
                    (void) TIFFSetField(tiff,TIFFTAG_YCBCRSUBSAMPLING,(uint16)
                      geometry_info.rho,(uint16) geometry_info.sigma);
                  }
                }
          }
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,
          &bits_per_sample,sans);
        if (bits_per_sample == 12)
          (void) TIFFSetField(tiff,TIFFTAG_JPEGTABLESMODE,JPEGTABLESMODE_QUANT);
#endif
        break;
      }
      case COMPRESSION_ADOBE_DEFLATE:
      {
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,
          &bits_per_sample,sans);
        if (((photometric == PHOTOMETRIC_RGB) ||
             (photometric == PHOTOMETRIC_SEPARATED) ||
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          predictor=PREDICTOR_HORIZONTAL;
        (void) TIFFSetField(tiff,TIFFTAG_ZIPQUALITY,(long) (
          image_info->quality == UndefinedCompressionQuality ? 7 :
          MagickMin((ssize_t) image_info->quality/10,9)));
        break;
      }
      case COMPRESSION_CCITTFAX3:
      {
        /*
          Byte-aligned EOL.
        */
        (void) TIFFSetField(tiff,TIFFTAG_GROUP3OPTIONS,4);
        break;
      }
      case COMPRESSION_CCITTFAX4:
        break;
#if defined(LZMA_SUPPORT) && defined(COMPRESSION_LZMA)
      case COMPRESSION_LZMA:
      {
        if (((photometric == PHOTOMETRIC_RGB) ||
             (photometric == PHOTOMETRIC_SEPARATED) ||
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          predictor=PREDICTOR_HORIZONTAL;
        (void) TIFFSetField(tiff,TIFFTAG_LZMAPRESET,(long) (
          image_info->quality == UndefinedCompressionQuality ? 7 :
          MagickMin((ssize_t) image_info->quality/10,9)));
        break;
      }
#endif
      case COMPRESSION_LZW:
      {
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,
          &bits_per_sample,sans);
        if (((photometric == PHOTOMETRIC_RGB) ||
             (photometric == PHOTOMETRIC_SEPARATED) ||
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          predictor=PREDICTOR_HORIZONTAL;
        break;
      }
#if defined(WEBP_SUPPORT) && defined(COMPRESSION_WEBP)
      case COMPRESSION_WEBP:
      {
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,
          &bits_per_sample,sans);
        if (((photometric == PHOTOMETRIC_RGB) ||
             (photometric == PHOTOMETRIC_SEPARATED) ||
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          predictor=PREDICTOR_HORIZONTAL;
        (void) TIFFSetField(tiff,TIFFTAG_WEBP_LEVEL,image_info->quality);
        if (image_info->quality >= 100)
          (void) TIFFSetField(tiff,TIFFTAG_WEBP_LOSSLESS,1);
        break;
      }
#endif
#if defined(ZSTD_SUPPORT) && defined(COMPRESSION_ZSTD)
      case COMPRESSION_ZSTD:
      {
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,
          &bits_per_sample,sans);
        if (((photometric == PHOTOMETRIC_RGB) ||
             (photometric == PHOTOMETRIC_SEPARATED) ||
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          predictor=PREDICTOR_HORIZONTAL;
        (void) TIFFSetField(tiff,TIFFTAG_ZSTD_LEVEL,22*image_info->quality/
          100.0);
        break;
      }
#endif
      default:
        break;
    }
    if ((compress_tag == COMPRESSION_LZW) ||
        (compress_tag == COMPRESSION_ADOBE_DEFLATE))
      {
        if (quantum_info->format == FloatingPointQuantumFormat)
          predictor=PREDICTOR_FLOATINGPOINT;
        option=GetImageOption(image_info,"tiff:predictor");
        if (option != (const char * ) NULL)
          predictor=(uint16) strtol(option,(char **) NULL,10);
        if (predictor != 0)
          (void) TIFFSetField(tiff,TIFFTAG_PREDICTOR,predictor);
      }
    if ((image->resolution.x != 0.0) && (image->resolution.y != 0.0))
      {
        unsigned short
          units;

        /*
          Set image resolution.
        */
        units=RESUNIT_NONE;
        if (image->units == PixelsPerInchResolution)
          units=RESUNIT_INCH;
        if (image->units == PixelsPerCentimeterResolution)
          units=RESUNIT_CENTIMETER;
        (void) TIFFSetField(tiff,TIFFTAG_RESOLUTIONUNIT,(uint16) units);
        (void) TIFFSetField(tiff,TIFFTAG_XRESOLUTION,image->resolution.x);
        (void) TIFFSetField(tiff,TIFFTAG_YRESOLUTION,image->resolution.y);
        if ((image->page.x < 0) || (image->page.y < 0))
          (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
            "TIFF: negative image positions unsupported","%s",image->filename);
        if ((image->page.x > 0) && (image->resolution.x > 0.0))
          {
            /*
              Set horizontal image position.
            */
            (void) TIFFSetField(tiff,TIFFTAG_XPOSITION,(float) image->page.x/
              image->resolution.x);
          }
        if ((image->page.y > 0) && (image->resolution.y > 0.0))
          {
            /*
              Set vertical image position.
            */
            (void) TIFFSetField(tiff,TIFFTAG_YPOSITION,(float) image->page.y/
              image->resolution.y);
          }
      }
    if (image->chromaticity.white_point.x != 0.0)
      {
        float
          chromaticity[6];

        /*
          Set image chromaticity.
        */
        chromaticity[0]=(float) image->chromaticity.red_primary.x;
        chromaticity[1]=(float) image->chromaticity.red_primary.y;
        chromaticity[2]=(float) image->chromaticity.green_primary.x;
        chromaticity[3]=(float) image->chromaticity.green_primary.y;
        chromaticity[4]=(float) image->chromaticity.blue_primary.x;
        chromaticity[5]=(float) image->chromaticity.blue_primary.y;
        (void) TIFFSetField(tiff,TIFFTAG_PRIMARYCHROMATICITIES,chromaticity);
        chromaticity[0]=(float) image->chromaticity.white_point.x;
        chromaticity[1]=(float) image->chromaticity.white_point.y;
        (void) TIFFSetField(tiff,TIFFTAG_WHITEPOINT,chromaticity);
      }
    option=GetImageOption(image_info,"tiff:write-layers");
    if (IsStringTrue(option) != MagickFalse)
      {
        (void) TIFFWritePhotoshopLayers(image,image_info,endian_type,exception);
        adjoin=MagickFalse;
      }
    if ((LocaleCompare(image_info->magick,"PTIF") != 0) &&
        (adjoin != MagickFalse) && (imageListLength > 1))
      {
        (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_PAGE);
        if (image->scene != 0)
          (void) TIFFSetField(tiff,TIFFTAG_PAGENUMBER,(uint16) image->scene,
            imageListLength);
      }
    if (image->orientation != UndefinedOrientation)
      (void) TIFFSetField(tiff,TIFFTAG_ORIENTATION,(uint16) image->orientation);
    else
      (void) TIFFSetField(tiff,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
    TIFFSetProfiles(tiff,image);
    {
      uint16
        page,
        pages;

      page=(uint16) scene;
      pages=(uint16) imageListLength;
      if ((LocaleCompare(image_info->magick,"PTIF") != 0) &&
          (adjoin != MagickFalse) && (pages > 1))
        (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_PAGE);
      (void) TIFFSetField(tiff,TIFFTAG_PAGENUMBER,page,pages);
    }
    (void) TIFFSetProperties(tiff,adjoin,image,exception);
    /*
      Write image scanlines.
    */
    if (GetTIFFInfo(image_info,tiff,&tiff_info) == MagickFalse)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    if (compress_tag == COMPRESSION_CCITTFAX4)
      (void) TIFFSetField(tiff,TIFFTAG_ROWSPERSTRIP,(uint32) image->rows);
    quantum_info->endian=LSBEndian;
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    tiff_info.scanline=(unsigned char *) GetQuantumPixels(quantum_info);
    switch (photometric)
    {
      case PHOTOMETRIC_CIELAB:
      case PHOTOMETRIC_YCBCR:
      case PHOTOMETRIC_RGB:
      {
        /*
          RGB TIFF image.
        */
        switch (image_info->interlace)
        {
          case NoInterlace:
          default:
          {
            quantum_type=RGBQuantum;
            if (image->alpha_trait != UndefinedPixelTrait)
              quantum_type=RGBAQuantum;
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                quantum_type,pixels,exception);
              (void) length;
              tiff_status=TIFFWritePixels(tiff,&tiff_info,y,0,image);
              if (tiff_status == -1)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case PlaneInterlace:
          case PartitionInterlace:
          {
            /*
              Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
            */
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                RedQuantum,pixels,exception);
              tiff_status=TIFFWritePixels(tiff,&tiff_info,y,0,image);
              if (tiff_status == -1)
                break;
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,100,400);
                if (status == MagickFalse)
                  break;
              }
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                GreenQuantum,pixels,exception);
              tiff_status=TIFFWritePixels(tiff,&tiff_info,y,1,image);
              if (tiff_status == -1)
                break;
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,200,400);
                if (status == MagickFalse)
                  break;
              }
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                BlueQuantum,pixels,exception);
              tiff_status=TIFFWritePixels(tiff,&tiff_info,y,2,image);
              if (tiff_status == -1)
                break;
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,300,400);
                if (status == MagickFalse)
                  break;
              }
            if (image->alpha_trait != UndefinedPixelTrait)
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                const Quantum
                  *magick_restrict p;

                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                length=ExportQuantumPixels(image,(CacheView *) NULL,
                  quantum_info,AlphaQuantum,pixels,exception);
                tiff_status=TIFFWritePixels(tiff,&tiff_info,y,3,image);
                if (tiff_status == -1)
                  break;
              }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,400,400);
                if (status == MagickFalse)
                  break;
              }
            break;
          }
        }
        break;
      }
      case PHOTOMETRIC_SEPARATED:
      {
        /*
          CMYK TIFF image.
        */
        quantum_type=CMYKQuantum;
        if (image->alpha_trait != UndefinedPixelTrait)
          quantum_type=CMYKAQuantum;
        if (image->colorspace != CMYKColorspace)
          (void) TransformImageColorspace(image,CMYKColorspace,exception);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          tiff_status=TIFFWritePixels(tiff,&tiff_info,y,0,image);
          if (tiff_status == -1)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case PHOTOMETRIC_PALETTE:
      {
        uint16
          *blue,
          *green,
          *red;

        /*
          Colormapped TIFF image.
        */
        red=(uint16 *) AcquireQuantumMemory(65536,sizeof(*red));
        green=(uint16 *) AcquireQuantumMemory(65536,sizeof(*green));
        blue=(uint16 *) AcquireQuantumMemory(65536,sizeof(*blue));
        if ((red == (uint16 *) NULL) || (green == (uint16 *) NULL) ||
            (blue == (uint16 *) NULL))
          {
            if (red != (uint16 *) NULL)
              red=(uint16 *) RelinquishMagickMemory(red);
            if (green != (uint16 *) NULL)
              green=(uint16 *) RelinquishMagickMemory(green);
            if (blue != (uint16 *) NULL)
              blue=(uint16 *) RelinquishMagickMemory(blue);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        /*
          Initialize TIFF colormap.
        */
        (void) memset(red,0,65536*sizeof(*red));
        (void) memset(green,0,65536*sizeof(*green));
        (void) memset(blue,0,65536*sizeof(*blue));
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          red[i]=ScaleQuantumToShort(image->colormap[i].red);
          green[i]=ScaleQuantumToShort(image->colormap[i].green);
          blue[i]=ScaleQuantumToShort(image->colormap[i].blue);
        }
        (void) TIFFSetField(tiff,TIFFTAG_COLORMAP,red,green,blue);
        red=(uint16 *) RelinquishMagickMemory(red);
        green=(uint16 *) RelinquishMagickMemory(green);
        blue=(uint16 *) RelinquishMagickMemory(blue);
      }
      default:
      {
        /*
          Convert PseudoClass packets to contiguous grayscale scanlines.
        */
        quantum_type=IndexQuantum;
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            if (photometric != PHOTOMETRIC_PALETTE)
              quantum_type=GrayAlphaQuantum;
            else
              quantum_type=IndexAlphaQuantum;
           }
         else
           if (photometric != PHOTOMETRIC_PALETTE)
             quantum_type=GrayQuantum;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          tiff_status=TIFFWritePixels(tiff,&tiff_info,y,0,image);
          if (tiff_status == -1)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    if (image->colorspace == LabColorspace)
      DecodeLabImage(image,exception);
    DestroyTIFFInfo(&tiff_info);
    /* TIFFPrintDirectory(tiff,stdout,MagickFalse); */
    if (tiff_status == -1)
      {
        status=MagickFalse;
        break;
      }
    if (TIFFWriteDirectory(tiff) == 0)
      {
        status=MagickFalse;
        break;
      }
    image=SyncNextImageInList(image);
    if (image == (Image *) NULL)
      break;
    status=SetImageProgress(image,SaveImagesTag,scene++,imageListLength);
    if (status == MagickFalse)
      break;
  } while (adjoin != MagickFalse);
  TIFFClose(tiff);
  return(status);
}
#endif

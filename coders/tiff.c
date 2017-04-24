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
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
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

#ifdef __VMS
#define JPEG_SUPPORT 1
#endif

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
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
#include "MagickCore/pixel-private.h"
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
#if defined(MAGICKCORE_TIFF_DELEGATE)
# if defined(MAGICKCORE_HAVE_TIFFCONF_H)
#  include "tiffconf.h"
# endif
# include "tiff.h"
# include "tiffio.h"
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
#include "psd-private.h"

/*
  Typedef declarations.
*/
typedef enum
{
  ReadSingleSampleMethod,
  ReadRGBAMethod,
  ReadCMYKAMethod,
  ReadYCCKMethod,
  ReadStripMethod,
  ReadTileMethod,
  ReadGenericMethod
} TIFFMethodType;

#if defined(MAGICKCORE_HAVE_TIFFREADEXIFDIRECTORY)
typedef struct _ExifInfo
{
  unsigned int
    tag,
    type,
    variable_length;

  const char
    *property;
} ExifInfo;

static const ExifInfo
  exif_info[] = {
    { EXIFTAG_EXPOSURETIME, TIFF_RATIONAL, 0, "exif:ExposureTime" },
    { EXIFTAG_FNUMBER, TIFF_RATIONAL, 0, "exif:FNumber" },
    { EXIFTAG_EXPOSUREPROGRAM, TIFF_SHORT, 0, "exif:ExposureProgram" },
    { EXIFTAG_SPECTRALSENSITIVITY, TIFF_ASCII, 0, "exif:SpectralSensitivity" },
    { EXIFTAG_ISOSPEEDRATINGS, TIFF_SHORT, 1, "exif:ISOSpeedRatings" },
    { EXIFTAG_OECF, TIFF_NOTYPE, 0, "exif:OptoelectricConversionFactor" },
    { EXIFTAG_EXIFVERSION, TIFF_NOTYPE, 0, "exif:ExifVersion" },
    { EXIFTAG_DATETIMEORIGINAL, TIFF_ASCII, 0, "exif:DateTimeOriginal" },
    { EXIFTAG_DATETIMEDIGITIZED, TIFF_ASCII, 0, "exif:DateTimeDigitized" },
    { EXIFTAG_COMPONENTSCONFIGURATION, TIFF_NOTYPE, 0, "exif:ComponentsConfiguration" },
    { EXIFTAG_COMPRESSEDBITSPERPIXEL, TIFF_RATIONAL, 0, "exif:CompressedBitsPerPixel" },
    { EXIFTAG_SHUTTERSPEEDVALUE, TIFF_SRATIONAL, 0, "exif:ShutterSpeedValue" },
    { EXIFTAG_APERTUREVALUE, TIFF_RATIONAL, 0, "exif:ApertureValue" },
    { EXIFTAG_BRIGHTNESSVALUE, TIFF_SRATIONAL, 0, "exif:BrightnessValue" },
    { EXIFTAG_EXPOSUREBIASVALUE, TIFF_SRATIONAL, 0, "exif:ExposureBiasValue" },
    { EXIFTAG_MAXAPERTUREVALUE, TIFF_RATIONAL, 0, "exif:MaxApertureValue" },
    { EXIFTAG_SUBJECTDISTANCE, TIFF_RATIONAL, 0, "exif:SubjectDistance" },
    { EXIFTAG_METERINGMODE, TIFF_SHORT, 0, "exif:MeteringMode" },
    { EXIFTAG_LIGHTSOURCE, TIFF_SHORT, 0, "exif:LightSource" },
    { EXIFTAG_FLASH, TIFF_SHORT, 0, "exif:Flash" },
    { EXIFTAG_FOCALLENGTH, TIFF_RATIONAL, 0, "exif:FocalLength" },
    { EXIFTAG_SUBJECTAREA, TIFF_NOTYPE, 0, "exif:SubjectArea" },
    { EXIFTAG_MAKERNOTE, TIFF_NOTYPE, 0, "exif:MakerNote" },
    { EXIFTAG_USERCOMMENT, TIFF_NOTYPE, 0, "exif:UserComment" },
    { EXIFTAG_SUBSECTIME, TIFF_ASCII, 0, "exif:SubSecTime" },
    { EXIFTAG_SUBSECTIMEORIGINAL, TIFF_ASCII, 0, "exif:SubSecTimeOriginal" },
    { EXIFTAG_SUBSECTIMEDIGITIZED, TIFF_ASCII, 0, "exif:SubSecTimeDigitized" },
    { EXIFTAG_FLASHPIXVERSION, TIFF_NOTYPE, 0, "exif:FlashpixVersion" },
    { EXIFTAG_PIXELXDIMENSION, TIFF_LONG, 0, "exif:PixelXDimension" },
    { EXIFTAG_PIXELYDIMENSION, TIFF_LONG, 0, "exif:PixelYDimension" },
    { EXIFTAG_RELATEDSOUNDFILE, TIFF_ASCII, 0, "exif:RelatedSoundFile" },
    { EXIFTAG_FLASHENERGY, TIFF_RATIONAL, 0, "exif:FlashEnergy" },
    { EXIFTAG_SPATIALFREQUENCYRESPONSE, TIFF_NOTYPE, 0, "exif:SpatialFrequencyResponse" },
    { EXIFTAG_FOCALPLANEXRESOLUTION, TIFF_RATIONAL, 0, "exif:FocalPlaneXResolution" },
    { EXIFTAG_FOCALPLANEYRESOLUTION, TIFF_RATIONAL, 0, "exif:FocalPlaneYResolution" },
    { EXIFTAG_FOCALPLANERESOLUTIONUNIT, TIFF_SHORT, 0, "exif:FocalPlaneResolutionUnit" },
    { EXIFTAG_SUBJECTLOCATION, TIFF_SHORT, 0, "exif:SubjectLocation" },
    { EXIFTAG_EXPOSUREINDEX, TIFF_RATIONAL, 0, "exif:ExposureIndex" },
    { EXIFTAG_SENSINGMETHOD, TIFF_SHORT, 0, "exif:SensingMethod" },
    { EXIFTAG_FILESOURCE, TIFF_NOTYPE, 0, "exif:FileSource" },
    { EXIFTAG_SCENETYPE, TIFF_NOTYPE, 0, "exif:SceneType" },
    { EXIFTAG_CFAPATTERN, TIFF_NOTYPE, 0, "exif:CFAPattern" },
    { EXIFTAG_CUSTOMRENDERED, TIFF_SHORT, 0, "exif:CustomRendered" },
    { EXIFTAG_EXPOSUREMODE, TIFF_SHORT, 0, "exif:ExposureMode" },
    { EXIFTAG_WHITEBALANCE, TIFF_SHORT, 0, "exif:WhiteBalance" },
    { EXIFTAG_DIGITALZOOMRATIO, TIFF_RATIONAL, 0, "exif:DigitalZoomRatio" },
    { EXIFTAG_FOCALLENGTHIN35MMFILM, TIFF_SHORT, 0, "exif:FocalLengthIn35mmFilm" },
    { EXIFTAG_SCENECAPTURETYPE, TIFF_SHORT, 0, "exif:SceneCaptureType" },
    { EXIFTAG_GAINCONTROL, TIFF_RATIONAL, 0, "exif:GainControl" },
    { EXIFTAG_CONTRAST, TIFF_SHORT, 0, "exif:Contrast" },
    { EXIFTAG_SATURATION, TIFF_SHORT, 0, "exif:Saturation" },
    { EXIFTAG_SHARPNESS, TIFF_SHORT, 0, "exif:Sharpness" },
    { EXIFTAG_DEVICESETTINGDESCRIPTION, TIFF_NOTYPE, 0, "exif:DeviceSettingDescription" },
    { EXIFTAG_SUBJECTDISTANCERANGE, TIFF_SHORT, 0, "exif:SubjectDistanceRange" },
    { EXIFTAG_IMAGEUNIQUEID, TIFF_ASCII, 0, "exif:ImageUniqueID" },
    { 0, 0, 0, (char *) NULL }
};
#endif
#endif  /* MAGICKCORE_TIFF_DELEGATE */

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
#if defined(MAGICKCORE_TIFF_DELEGATE)
static Image *
  ReadTIFFImage(const ImageInfo *,ExceptionInfo *);

static MagickBooleanType
  WriteGROUP4Image(const ImageInfo *,Image *,ExceptionInfo *),
  WritePTIFImage(const ImageInfo *,Image *,ExceptionInfo *),
  WriteTIFFImage(const ImageInfo *,Image *,ExceptionInfo *);
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

static inline size_t WriteLSBLong(FILE *file,const size_t value)
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
  length=WriteLSBLong(file,image->columns);
  length=fwrite("\001\001\004\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,image->rows);
  length=fwrite("\002\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\003\001\003\000\001\000\000\000\004\000\000\000",1,12,file);
  length=fwrite("\006\001\003\000\001\000\000\000\000\000\000\000",1,12,file);
  length=fwrite("\021\001\003\000\001\000\000\000",1,8,file);
  strip_offset=10+(12*14)+4+8;
  length=WriteLSBLong(file,(size_t) strip_offset);
  length=fwrite("\022\001\003\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(size_t) image_info->orientation);
  length=fwrite("\025\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\026\001\004\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,image->rows);
  length=fwrite("\027\001\004\000\001\000\000\000\000\000\000\000",1,12,file);
  offset=(ssize_t) ftell(file)-4;
  length=fwrite("\032\001\005\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(size_t) (strip_offset-8));
  length=fwrite("\033\001\005\000\001\000\000\000",1,8,file);
  length=WriteLSBLong(file,(size_t) (strip_offset-8));
  length=fwrite("\050\001\003\000\001\000\000\000\002\000\000\000",1,12,file);
  length=fwrite("\000\000\000\000",1,4,file);
  length=WriteLSBLong(file,(long) image->resolution.x);
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
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
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
      status=MagickFalse;
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
  if (status == MagickFalse)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  return(MagickTrue);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int TIFFCloseBlob(thandle_t image)
{
  (void) CloseBlob((Image *) image);
  return(0);
}

static void TIFFErrors(const char *module,const char *format,va_list error)
{
  char
    message[MagickPathExtent];

  ExceptionInfo
    *exception;

#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsnprintf(message,MagickPathExtent,format,error);
#else
  (void) vsprintf(message,format,error);
#endif
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

static void TIFFGetProfiles(TIFF *tiff,Image *image,MagickBooleanType ping,
  ExceptionInfo *exception)
{
  uint32
    length;

  unsigned char
    *profile;

  length=0;
  if (ping == MagickFalse)
    {
#if defined(TIFFTAG_ICCPROFILE)
      if ((TIFFGetField(tiff,TIFFTAG_ICCPROFILE,&length,&profile) == 1) &&
          (profile != (unsigned char *) NULL))
        (void) ReadProfile(image,"icc",profile,(ssize_t) length,exception);
#endif
#if defined(TIFFTAG_PHOTOSHOP)
      if ((TIFFGetField(tiff,TIFFTAG_PHOTOSHOP,&length,&profile) == 1) &&
          (profile != (unsigned char *) NULL))
        (void) ReadProfile(image,"8bim",profile,(ssize_t) length,exception);
#endif
#if defined(TIFFTAG_RICHTIFFIPTC)
      if ((TIFFGetField(tiff,TIFFTAG_RICHTIFFIPTC,&length,&profile) == 1) &&
          (profile != (unsigned char *) NULL))
        {
          if (TIFFIsByteSwapped(tiff) != 0)
            TIFFSwabArrayOfLong((uint32 *) profile,(size_t) length);
          (void) ReadProfile(image,"iptc",profile,4L*length,exception);
        }
#endif
#if defined(TIFFTAG_XMLPACKET)
      if ((TIFFGetField(tiff,TIFFTAG_XMLPACKET,&length,&profile) == 1) &&
          (profile != (unsigned char *) NULL))
        (void) ReadProfile(image,"xmp",profile,(ssize_t) length,exception);
#endif
      if ((TIFFGetField(tiff,34118,&length,&profile) == 1) &&
          (profile != (unsigned char *) NULL))
        (void) ReadProfile(image,"tiff:34118",profile,(ssize_t) length,
          exception);
    }
  if ((TIFFGetField(tiff,37724,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    (void) ReadProfile(image,"tiff:37724",profile,(ssize_t) length,exception);
}

static void TIFFGetProperties(TIFF *tiff,Image *image,ExceptionInfo *exception)
{
  char
    message[MagickPathExtent],
    *text;

  uint32
    count,
    length,
    type;

  unsigned long
    *tietz;

  if ((TIFFGetField(tiff,TIFFTAG_ARTIST,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:artist",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_COPYRIGHT,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:copyright",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_DATETIME,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:timestamp",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_DOCUMENTNAME,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:document",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_HOSTCOMPUTER,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:hostcomputer",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_IMAGEDESCRIPTION,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"comment",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_MAKE,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:make",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_MODEL,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:model",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_OPIIMAGEID,&count,&text) == 1) &&
      (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      (void) SetImageProperty(image,"tiff:image-id",message,exception);
    }
  if ((TIFFGetField(tiff,TIFFTAG_PAGENAME,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"label",text,exception);
  if ((TIFFGetField(tiff,TIFFTAG_SOFTWARE,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:software",text,exception);
  if ((TIFFGetField(tiff,33423,&count,&text) == 1) &&
      (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      (void) SetImageProperty(image,"tiff:kodak-33423",message,exception);
    }
  if ((TIFFGetField(tiff,36867,&count,&text) == 1) &&
      (text != (char *) NULL))
    {
      if (count >= MagickPathExtent)
        count=MagickPathExtent-1;
      (void) CopyMagickString(message,text,count+1);
      (void) SetImageProperty(image,"tiff:kodak-36867",message,exception);
    }
  if (TIFFGetField(tiff,TIFFTAG_SUBFILETYPE,&type) == 1)
    switch (type)
    {
      case 0x01:
      {
        (void) SetImageProperty(image,"tiff:subfiletype","REDUCEDIMAGE",
          exception);
        break;
      }
      case 0x02:
      {
        (void) SetImageProperty(image,"tiff:subfiletype","PAGE",exception);
        break;
      }
      case 0x04:
      {
        (void) SetImageProperty(image,"tiff:subfiletype","MASK",exception);
        break;
      }
      default:
        break;
    }
  if ((TIFFGetField(tiff,37706,&length,&tietz) == 1) &&
      (tietz != (unsigned long *) NULL))
    {
      (void) FormatLocaleString(message,MagickPathExtent,"%lu",tietz[0]);
      (void) SetImageProperty(image,"tiff:tietz_offset",message,exception);
    }
}

static void TIFFGetEXIFProperties(TIFF *tiff,Image *image,
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_TIFFREADEXIFDIRECTORY)
  char
    value[MagickPathExtent];

  register ssize_t
    i;

  tdir_t
    directory;

#if defined(TIFF_VERSION_BIG)
  uint64
#else
  uint32
#endif
    offset;

  void
    *sans;

  /*
    Read EXIF properties.
  */
  offset=0;
  if (TIFFGetField(tiff,TIFFTAG_EXIFIFD,&offset) != 1)
    return;
  directory=TIFFCurrentDirectory(tiff);
  if (TIFFReadEXIFDirectory(tiff,offset) != 1)
    {
      TIFFSetDirectory(tiff,directory);
      return;
    }
  sans=NULL;
  for (i=0; exif_info[i].tag != 0; i++)
  {
    *value='\0';
    switch (exif_info[i].type)
    {
      case TIFF_ASCII:
      {
        char
          *ascii;

        ascii=(char *) NULL;
        if ((TIFFGetField(tiff,exif_info[i].tag,&ascii,&sans,&sans) == 1) &&
            (ascii != (char *) NULL) && (*ascii != '\0'))
          (void) CopyMagickString(value,ascii,MagickPathExtent);
        break;
      }
      case TIFF_SHORT:
      {
        if (exif_info[i].variable_length == 0)
          {
            uint16
              shorty;

            shorty=0;
            if (TIFFGetField(tiff,exif_info[i].tag,&shorty,&sans,&sans) == 1)
              (void) FormatLocaleString(value,MagickPathExtent,"%d",shorty);
          }
        else
          {
            int
              tiff_status;

            uint16
              *shorty;

            uint16
              shorty_num;

            tiff_status=TIFFGetField(tiff,exif_info[i].tag,&shorty_num,&shorty,
              &sans,&sans);
            if (tiff_status == 1)
              (void) FormatLocaleString(value,MagickPathExtent,"%d",
                shorty_num != 0 ? shorty[0] : 0);
          }
        break;
      }
      case TIFF_LONG:
      {
        uint32
          longy;

        longy=0;
        if (TIFFGetField(tiff,exif_info[i].tag,&longy,&sans,&sans) == 1)
          (void) FormatLocaleString(value,MagickPathExtent,"%d",longy);
        break;
      }
#if defined(TIFF_VERSION_BIG)
      case TIFF_LONG8:
      {
        uint64
          long8y;

        long8y=0;
        if (TIFFGetField(tiff,exif_info[i].tag,&long8y,&sans,&sans) == 1)
          (void) FormatLocaleString(value,MagickPathExtent,"%.20g",(double)
            ((MagickOffsetType) long8y));
        break;
      }
#endif
      case TIFF_RATIONAL:
      case TIFF_SRATIONAL:
      case TIFF_FLOAT:
      {
        float
          floaty;

        floaty=0.0;
        if (TIFFGetField(tiff,exif_info[i].tag,&floaty,&sans,&sans) == 1)
          (void) FormatLocaleString(value,MagickPathExtent,"%g",(double)
            floaty);
        break;
      }
      case TIFF_DOUBLE:
      {
        double
          doubley;

        doubley=0.0;
        if (TIFFGetField(tiff,exif_info[i].tag,&doubley,&sans,&sans) == 1)
          (void) FormatLocaleString(value,MagickPathExtent,"%g",doubley);
        break;
      }
      default:
        break;
    }
    if (*value != '\0')
      (void) SetImageProperty(image,exif_info[i].property,value,exception);
  }
  TIFFSetDirectory(tiff,directory);
#else
  (void) tiff;
  (void) image;
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

static int32 TIFFReadPixels(TIFF *tiff,size_t bits_per_sample,
  tsample_t sample,ssize_t row,tdata_t scanline)
{
  int32
    status;

  (void) bits_per_sample;
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

static void TIFFWarnings(const char *module,const char *format,va_list warning)
{
  char
    message[MagickPathExtent];

  ExceptionInfo
    *exception;

#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsnprintf(message,MagickPathExtent,format,warning);
#else
  (void) vsprintf(message,format,warning);
#endif
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

  register size_t
    i;

  TIFFMethodType
    method;

#if defined(TIFF_VERSION_BIG)
  uint64
#else
  uint32
#endif
    **value;

  unsigned char
    buffer[BUFFER_SIZE+32];

  unsigned short
    length;

  /* only support 8 bit for now */
  if ((photometric != PHOTOMETRIC_SEPARATED) || (bits_per_sample != 8) ||
      (samples_per_pixel != 4))
    return(ReadGenericMethod);
  /* Search for Adobe APP14 JPEG Marker */
  if (!TIFFGetField(tiff,TIFFTAG_STRIPOFFSETS,&value))
    return(ReadRGBAMethod);
  position=TellBlob(image);
  offset=(MagickOffsetType) (value[0]);
  if (SeekBlob(image,offset,SEEK_SET) != offset)
    return(ReadRGBAMethod);
  method=ReadRGBAMethod;
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

static void TIFFReadPhotoshopLayers(Image* image,const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  const char
    *option;

  const StringInfo
    *layer_info;

  Image
    *layers;

  PSDInfo
    info;

  register ssize_t
    i;

  if (GetImageListLength(image) != 1)
    return;
  if ((image_info->number_scenes == 1) && (image_info->scene == 0))
    return;
  option=GetImageOption(image_info,"tiff:ignore-layers");
  if (option != (const char * ) NULL)
    return;
  layer_info=GetImageProfile(image,"tiff:37724");
  if (layer_info == (const StringInfo *) NULL)
    return;
  for (i=0; i < (ssize_t) layer_info->length-8; i++)
  {
    if (LocaleNCompare((const char *) (layer_info->datum+i),
        image->endian == MSBEndian ? "8BIM" : "MIB8",4) != 0)
      continue;
    i+=4;
    if ((LocaleNCompare((const char *) (layer_info->datum+i),
         image->endian == MSBEndian ? "Layr" : "ryaL",4) == 0) ||
        (LocaleNCompare((const char *) (layer_info->datum+i),
         image->endian == MSBEndian ? "LMsk" : "ksML",4) == 0) ||
        (LocaleNCompare((const char *) (layer_info->datum+i),
         image->endian == MSBEndian ? "Lr16" : "61rL",4) == 0) ||
        (LocaleNCompare((const char *) (layer_info->datum+i),
         image->endian == MSBEndian ? "Lr32" : "23rL",4) == 0))
      break;
  }
  i+=4;
  if (i >= (ssize_t) (layer_info->length-8))
    return;
  layers=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  (void) DeleteImageProfile(layers,"tiff:37724");
  AttachBlob(layers->blob,layer_info->datum,layer_info->length);
  SeekBlob(layers,(MagickOffsetType) i,SEEK_SET);
  info.version=1;
  info.columns=layers->columns;
  info.rows=layers->rows;
  info.channels=(unsigned short) layers->number_channels;
  /* Setting the mode to a value that won't change the colorspace */
  info.mode=10;
  ReadPSDLayers(layers,image_info,&info,MagickFalse,exception);
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
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static Image *ReadTIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  const char
    *option;

  float
    *chromaticity,
    x_position,
    y_position,
    x_resolution,
    y_resolution;

  Image
    *image;

  int
    tiff_status;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register ssize_t
    i;

  size_t
    pad;

  ssize_t
    y;

  TIFF
    *tiff;

  TIFFMethodType
    method;

  uint16
    compress_tag,
    bits_per_sample,
    endian,
    extra_samples,
    interlace,
    max_sample_value,
    min_sample_value,
    orientation,
    pages,
    photometric,
    *sample_info,
    sample_format,
    samples_per_pixel,
    units,
    value;

  uint32
    height,
    rows_per_strip,
    width;

  unsigned char
    *tiff_pixels;

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
  do
  {
DisableMSCWarning(4127)
    if (0 && (image_info->verbose != MagickFalse))
      TIFFPrintDirectory(tiff,stdout,MagickFalse);
RestoreMSCWarning
    if ((TIFFGetField(tiff,TIFFTAG_IMAGEWIDTH,&width) != 1) ||
        (TIFFGetField(tiff,TIFFTAG_IMAGELENGTH,&height) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_COMPRESSION,&compress_tag) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_FILLORDER,&endian) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_PLANARCONFIG,&interlace) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLESPERPIXEL,&samples_per_pixel) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,&bits_per_sample) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLEFORMAT,&sample_format) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_MINSAMPLEVALUE,&min_sample_value) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_MAXSAMPLEVALUE,&max_sample_value) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_PHOTOMETRIC,&photometric) != 1))
      {
        TIFFClose(tiff);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
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
      SetImageColorspace(image,GRAYColorspace,exception);
    if (photometric == PHOTOMETRIC_SEPARATED)
      SetImageColorspace(image,CMYKColorspace,exception);
    if (photometric == PHOTOMETRIC_CIELAB)
      SetImageColorspace(image,LabColorspace,exception);
    TIFFGetProfiles(tiff,image,image_info->ping,exception);
    TIFFGetProperties(tiff,image,exception);
    option=GetImageOption(image_info,"tiff:exif-properties");
    if (IsStringFalse(option) == MagickFalse) /* enabled by default */
      TIFFGetEXIFProperties(tiff,image,exception);
    (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLESPERPIXEL,
      &samples_per_pixel);
    if ((TIFFGetFieldDefaulted(tiff,TIFFTAG_XRESOLUTION,&x_resolution) == 1) &&
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_YRESOLUTION,&y_resolution) == 1))
      {
        image->resolution.x=x_resolution;
        image->resolution.y=y_resolution;
      }
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_RESOLUTIONUNIT,&units) == 1)
      {
        if (units == RESUNIT_INCH)
          image->units=PixelsPerInchResolution;
        if (units == RESUNIT_CENTIMETER)
          image->units=PixelsPerCentimeterResolution;
      }
    if ((TIFFGetFieldDefaulted(tiff,TIFFTAG_XPOSITION,&x_position) == 1) &&
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_YPOSITION,&y_position) == 1))
      {
        image->page.x=(ssize_t) ceil(x_position*image->resolution.x-0.5);
        image->page.y=(ssize_t) ceil(y_position*image->resolution.y-0.5);
      }
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_ORIENTATION,&orientation) == 1)
      image->orientation=(OrientationType) orientation;
    if (TIFFGetField(tiff,TIFFTAG_WHITEPOINT,&chromaticity) == 1)
      {
        if (chromaticity != (float *) NULL)
          {
            image->chromaticity.white_point.x=chromaticity[0];
            image->chromaticity.white_point.y=chromaticity[1];
          }
      }
    if (TIFFGetField(tiff,TIFFTAG_PRIMARYCHROMATICITIES,&chromaticity) == 1)
      {
        if (chromaticity != (float *) NULL)
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

           int
             tiff_status;

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
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_PAGENUMBER,&value,&pages) == 1)
      image->scene=value;
    if (image->storage_class == PseudoClass)
      {
        int
          tiff_status;

        size_t
          range;

        uint16
          *blue_colormap,
          *green_colormap,
          *red_colormap;

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
      return(DestroyImageList(image));
    /*
      Allocate memory for the image and pixel buffer.
    */
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      {
        TIFFClose(tiff);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    if (sample_format == SAMPLEFORMAT_UINT)
      status=SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
    if (sample_format == SAMPLEFORMAT_INT)
      status=SetQuantumFormat(image,quantum_info,SignedQuantumFormat);
    if (sample_format == SAMPLEFORMAT_IEEEFP)
      status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
    if (status == MagickFalse)
      {
        TIFFClose(tiff);
        quantum_info=DestroyQuantumInfo(quantum_info);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
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
    tiff_status=TIFFGetFieldDefaulted(tiff,TIFFTAG_EXTRASAMPLES,&extra_samples,
      &sample_info);
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
                SetQuantumAlphaType(quantum_info,DisassociatedQuantumAlpha);
                (void) SetImageProperty(image,"tiff:alpha","associated",
                  exception);
              }
            else
              if (sample_info[i] == EXTRASAMPLE_UNASSALPHA)
                (void) SetImageProperty(image,"tiff:alpha","unassociated",
                  exception);
          }
      }
    method=ReadGenericMethod;
    if (TIFFGetField(tiff,TIFFTAG_ROWSPERSTRIP,&rows_per_strip) == 1)
      {
        char
          value[MagickPathExtent];

        method=ReadStripMethod;
        (void) FormatLocaleString(value,MagickPathExtent,"%u",
          (unsigned int) rows_per_strip);
        (void) SetImageProperty(image,"tiff:rows-per-strip",value,exception);
      }
    if ((samples_per_pixel >= 3) && (interlace == PLANARCONFIG_CONTIG))
      if ((image->alpha_trait == UndefinedPixelTrait) ||
          (samples_per_pixel >= 4))
        method=ReadRGBAMethod;
    if ((samples_per_pixel >= 4) && (interlace == PLANARCONFIG_SEPARATE))
      if ((image->alpha_trait == UndefinedPixelTrait) ||
          (samples_per_pixel >= 5))
        method=ReadCMYKAMethod;
    if ((photometric != PHOTOMETRIC_RGB) &&
        (photometric != PHOTOMETRIC_CIELAB) &&
        (photometric != PHOTOMETRIC_SEPARATED))
      method=ReadGenericMethod;
    if (image->storage_class == PseudoClass)
      method=ReadSingleSampleMethod;
    if ((photometric == PHOTOMETRIC_MINISBLACK) ||
        (photometric == PHOTOMETRIC_MINISWHITE))
      method=ReadSingleSampleMethod;
    if ((photometric != PHOTOMETRIC_SEPARATED) &&
        (interlace == PLANARCONFIG_SEPARATE) && (bits_per_sample < 64))
      method=ReadGenericMethod;
    if (image->compression == JPEGCompression)
      method=GetJPEGMethod(image,tiff,photometric,bits_per_sample,
        samples_per_pixel);
    if (compress_tag == COMPRESSION_JBIG)
      method=ReadStripMethod;
    if (TIFFIsTiled(tiff) != MagickFalse)
      method=ReadTileMethod;
    quantum_info->endian=LSBEndian;
    quantum_type=RGBQuantum;
    tiff_pixels=(unsigned char *) AcquireMagickMemory(MagickMax(
      TIFFScanlineSize(tiff),(ssize_t) (image->columns*samples_per_pixel*
      pow(2.0,ceil(log(bits_per_sample)/log(2.0))))));
    if (tiff_pixels == (unsigned char *) NULL)
      {
        TIFFClose(tiff);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    switch (method)
    {
      case ReadSingleSampleMethod:
      {
        /*
          Convert TIFF image to PseudoClass MIFF image.
        */
        quantum_type=IndexQuantum;
        pad=(size_t) MagickMax((size_t) samples_per_pixel-1,0);
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            if (image->storage_class != PseudoClass)
              {
                quantum_type=samples_per_pixel == 1 ? AlphaQuantum :
                  GrayAlphaQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-2,0);
              }
            else
              {
                quantum_type=IndexAlphaQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-2,0);
              }
          }
        else
          if (image->storage_class != PseudoClass)
            {
              quantum_type=GrayQuantum;
              pad=(size_t) MagickMax((size_t) samples_per_pixel-1,0);
            }
        status=SetQuantumPad(image,quantum_info,pad*pow(2,ceil(log(
          bits_per_sample)/log(2))));
        if (status == MagickFalse)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,tiff_pixels,exception);
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
      case ReadRGBAMethod:
      {
        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        pad=(size_t) MagickMax((size_t) samples_per_pixel-3,0);
        quantum_type=RGBQuantum;
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            quantum_type=RGBAQuantum;
            pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
          }
        if (image->colorspace == CMYKColorspace)
          {
            pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
            quantum_type=CMYKQuantum;
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                quantum_type=CMYKAQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-5,0);
              }
          }
        status=SetQuantumPad(image,quantum_info,pad*((bits_per_sample+7) >> 3));
        if (status == MagickFalse)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,tiff_pixels,exception);
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
      case ReadCMYKAMethod:
      {
        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            register Quantum
              *magick_restrict q;

            int
              status;

            status=TIFFReadPixels(tiff,bits_per_sample,(tsample_t) i,y,(char *)
              tiff_pixels);
            if (status == -1)
              break;
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            if (image->colorspace != CMYKColorspace)
              switch (i)
              {
                case 0: quantum_type=RedQuantum; break;
                case 1: quantum_type=GreenQuantum; break;
                case 2: quantum_type=BlueQuantum; break;
                case 3: quantum_type=AlphaQuantum; break;
                default: quantum_type=UndefinedQuantum; break;
              }
            else
              switch (i)
              {
                case 0: quantum_type=CyanQuantum; break;
                case 1: quantum_type=MagentaQuantum; break;
                case 2: quantum_type=YellowQuantum; break;
                case 3: quantum_type=BlackQuantum; break;
                case 4: quantum_type=AlphaQuantum; break;
                default: quantum_type=UndefinedQuantum; break;
              }
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              quantum_type,tiff_pixels,exception);
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
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
      case ReadYCCKMethod:
      {
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          unsigned char
            *p;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=tiff_pixels;
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
        register uint32
          *p;

        /*
          Convert stripped TIFF image to DirectClass MIFF image.
        */
        i=0;
        p=(uint32 *) NULL;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          if (i == 0)
            {
              if (TIFFReadRGBAStrip(tiff,(tstrip_t) y,(uint32 *) tiff_pixels) == 0)
                break;
              i=(ssize_t) MagickMin((ssize_t) rows_per_strip,(ssize_t)
                image->rows-y);
            }
          i--;
          p=((uint32 *) tiff_pixels)+image->columns*i;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(image,ScaleCharToQuantum((unsigned char)
              (TIFFGetR(*p))),q);
            SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
              (TIFFGetG(*p))),q);
            SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
              (TIFFGetB(*p))),q);
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                (TIFFGetA(*p))),q);
            p++;
            q+=GetPixelChannels(image);
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
      case ReadTileMethod:
      {
        register uint32
          *p;

        uint32
          *tile_pixels,
          columns,
          rows;

        /*
          Convert tiled TIFF image to DirectClass MIFF image.
        */
        if ((TIFFGetField(tiff,TIFFTAG_TILEWIDTH,&columns) != 1) ||
            (TIFFGetField(tiff,TIFFTAG_TILELENGTH,&rows) != 1))
          {
            TIFFClose(tiff);
            ThrowReaderException(CoderError,"ImageIsNotTiled");
          }
        (void) SetImageStorageClass(image,DirectClass,exception);
        number_pixels=(MagickSizeType) columns*rows;
        if (HeapOverflowSanityCheck(rows,sizeof(*tile_pixels)) != MagickFalse)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        tile_pixels=(uint32 *) AcquireQuantumMemory(columns,rows*
          sizeof(*tile_pixels));
        if (tile_pixels == (uint32 *) NULL)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        for (y=0; y < (ssize_t) image->rows; y+=rows)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q,
            *magick_restrict tile;

          size_t
            columns_remaining,
            rows_remaining;

          rows_remaining=image->rows-y;
          if ((ssize_t) (y+rows) < (ssize_t) image->rows)
            rows_remaining=rows;
          tile=QueueAuthenticPixels(image,0,y,image->columns,rows_remaining,
            exception);
          if (tile == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x+=columns)
          {
            size_t
              column,
              row;

            if (TIFFReadRGBATile(tiff,(uint32) x,(uint32) y,tile_pixels) == 0)
              break;
            columns_remaining=image->columns-x;
            if ((ssize_t) (x+columns) < (ssize_t) image->columns)
              columns_remaining=columns;
            p=tile_pixels+(rows-rows_remaining)*columns;
            q=tile+GetPixelChannels(image)*(image->columns*(rows_remaining-1)+
              x);
            for (row=rows_remaining; row > 0; row--)
            {
              if (image->alpha_trait != UndefinedPixelTrait)
                for (column=columns_remaining; column > 0; column--)
                {
                  SetPixelRed(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetR(*p)),q);
                  SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetG(*p)),q);
                  SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetB(*p)),q);
                  SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetA(*p)),q);
                  p++;
                  q+=GetPixelChannels(image);
                }
              else
                for (column=columns_remaining; column > 0; column--)
                {
                  SetPixelRed(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetR(*p)),q);
                  SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetG(*p)),q);
                  SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetB(*p)),q);
                  p++;
                  q+=GetPixelChannels(image);
                }
              p+=columns-columns_remaining;
              q-=GetPixelChannels(image)*(image->columns+columns_remaining);
            }
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
        tile_pixels=(uint32 *) RelinquishMagickMemory(tile_pixels);
        break;
      }
      case ReadGenericMethod:
      default:
      {
        MemoryInfo
          *pixel_info;

        register uint32
          *p;

        uint32
          *pixels;

        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        number_pixels=(MagickSizeType) image->columns*image->rows;
        if (HeapOverflowSanityCheck(image->rows,sizeof(*pixels)) != MagickFalse)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        pixel_info=AcquireVirtualMemory(image->columns,image->rows*
          sizeof(uint32));
        if (pixel_info == (MemoryInfo *) NULL)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        pixels=(uint32 *) GetVirtualMemoryBlob(pixel_info);
        (void) TIFFReadRGBAImage(tiff,(uint32) image->columns,(uint32)
          image->rows,(uint32 *) pixels,0);
        /*
          Convert image to DirectClass pixel packets.
        */
        p=pixels+number_pixels-1;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register ssize_t
            x;

          register Quantum
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
        pixel_info=RelinquishVirtualMemory(pixel_info);
        break;
      }
    }
    tiff_pixels=(unsigned char *) RelinquishMagickMemory(tiff_pixels);
    SetQuantumImageType(image,quantum_type);
  next_tiff_frame:
    if (quantum_info != (QuantumInfo *) NULL)
      quantum_info=DestroyQuantumInfo(quantum_info);
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
    status=TIFFReadDirectory(tiff) != 0 ? MagickTrue : MagickFalse;
    if (status != MagickFalse)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,image->scene-1,
          image->scene);
        if (status == MagickFalse)
          break;
      }
  } while (status != MagickFalse);
  TIFFClose(tiff);
  TIFFReadPhotoshopLayers(image,image_info,exception);
  if (image_info->number_scenes != 0)
    {
      if (image_info->scene >= GetImageListLength(image))
        {
          /* Subimage was not found in the Photoshop layer */
          image=DestroyImageList(image);
          return((Image *)NULL);
        }
    }
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

  register ssize_t
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
  /* This also sets field_bit to 0 (FIELD_IGNORE) */
  ResetMagickMemory(ignore,0,count*sizeof(*ignore));
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

ModuleExport size_t RegisterTIFFImage(void)
{
#define TIFFDescription  "Tagged Image File Format"

  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

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
  *version='\0';
#if defined(TIFF_VERSION)
  (void) FormatLocaleString(version,MagickPathExtent,"%d",TIFF_VERSION);
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  {
    const char
      *p;

    register ssize_t
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
  entry->flags^=CoderAdjoinFlag;
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

  register ssize_t
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
  (void) SetImageType(huffman_image,BilevelType,exception);
  write_info=CloneImageInfo((ImageInfo *) NULL);
  SetImageInfoFile(write_info,file);
  (void) SetImageDepth(image,1,exception);
  (void) SetImageType(image,BilevelType,exception);
  write_info->compression=Group4Compression;
  write_info->type=BilevelType;
  (void) SetImageOption(write_info,"quantum:polarity","min-is-white");
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
      pyramid_image->resolution=resolution;
      (void) SetImageProperty(pyramid_image,"tiff:subfiletype","REDUCEDIMAGE",
        exception);
      AppendImageToList(&images,pyramid_image);
    }
  }
  images=GetFirstImageInList(images);
  /*
    Write pyramid-encoded TIFF image.
  */
  write_info=CloneImageInfo(image_info);
  write_info->adjoin=MagickTrue;
  (void) CopyMagickString(write_info->magick,"TIFF",MagickPathExtent);
  (void) CopyMagickString(images->magick,"TIFF",MagickPathExtent);
  status=WriteTIFFImage(write_info,images,exception);
  images=DestroyImageList(images);
  write_info=DestroyImageInfo(write_info);
  return(status);
}
#endif

#if defined(MAGICKCORE_TIFF_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
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
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType GetTIFFInfo(const ImageInfo *image_info,
  TIFF *tiff,TIFFInfo *tiff_info)
{
  const char
    *option;

  MagickStatusType
    flags;

  uint32
    tile_columns,
    tile_rows;

  assert(tiff_info != (TIFFInfo *) NULL);
  (void) ResetMagickMemory(tiff_info,0,sizeof(*tiff_info));
  option=GetImageOption(image_info,"tiff:tile-geometry");
  if (option == (const char *) NULL)
    {
      uint32
        rows_per_strip;

      option=GetImageOption(image_info,"tiff:rows-per-strip");
      if (option != (const char *) NULL)
        rows_per_strip=(size_t) strtol(option,(char **) NULL,10);
      else
        if (TIFFGetField(tiff,TIFFTAG_IMAGELENGTH,&rows_per_strip) == 0)
          rows_per_strip=0;  /* use default */
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

static int32 TIFFWritePixels(TIFF *tiff,TIFFInfo *tiff_info,ssize_t row,
  tsample_t sample,Image *image)
{
  int32
    status;

  register ssize_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    number_tiles,
    tile_width;

  ssize_t
    bytes_per_pixel,
    j,
    k,
    l;

  if (TIFFIsTiled(tiff) == 0)
    return(TIFFWriteScanline(tiff,tiff_info->scanline,(uint32) row,sample));
  /*
    Fill scanlines to tile height.
  */
  i=(ssize_t) (row % tiff_info->tile_geometry.height)*TIFFScanlineSize(tiff);
  (void) CopyMagickMemory(tiff_info->scanlines+i,(char *) tiff_info->scanline,
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
        size_t
          length;

        StringInfo
          *iptc_profile;

        iptc_profile=CloneStringInfo(profile);
        length=GetStringInfoLength(profile)+4-(GetStringInfoLength(profile) &
          0x03);
        SetStringInfoLength(iptc_profile,length);
        if (TIFFIsByteSwapped(tiff))
          TIFFSwabArrayOfLong((uint32 *) GetStringInfoDatum(iptc_profile),
            (unsigned long) (length/4));
        (void) TIFFSetField(tiff,TIFFTAG_RICHTIFFIPTC,(uint32)
          GetStringInfoLength(iptc_profile)/4,GetStringInfoDatum(iptc_profile));
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

static void TIFFSetProperties(TIFF *tiff,const ImageInfo *image_info,
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
      if ((image_info->adjoin != MagickFalse) && (pages > 1))
        (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_PAGE);
      (void) TIFFSetField(tiff,TIFFTAG_PAGENUMBER,page,pages);
    }
}

static void TIFFSetEXIFProperties(TIFF *tiff,Image *image,
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_TIFFREADEXIFDIRECTORY)
  const char
    *value;

  register ssize_t
    i;

  uint32
    offset;

  /*
    Write EXIF properties.
  */
  offset=0;
  (void) TIFFSetField(tiff,TIFFTAG_SUBIFD,1,&offset);
  for (i=0; exif_info[i].tag != 0; i++)
  {
    value=GetImageProperty(image,exif_info[i].property,exception);
    if (value == (const char *) NULL)
      continue;
    switch (exif_info[i].type)
    {
      case TIFF_ASCII:
      {
        (void) TIFFSetField(tiff,exif_info[i].tag,value);
        break;
      }
      case TIFF_SHORT:
      {
        uint16
          field;

        field=(uint16) StringToLong(value);
        (void) TIFFSetField(tiff,exif_info[i].tag,field);
        break;
      }
      case TIFF_LONG:
      {
        uint16
          field;

        field=(uint16) StringToLong(value);
        (void) TIFFSetField(tiff,exif_info[i].tag,field);
        break;
      }
      case TIFF_RATIONAL:
      case TIFF_SRATIONAL:
      {
        float
          field;

        field=StringToDouble(value,(char **) NULL);
        (void) TIFFSetField(tiff,exif_info[i].tag,field);
        break;
      }
      default:
        break;
    }
  }
  /* (void) TIFFSetField(tiff,TIFFTAG_EXIFIFD,offset); */
#else
  (void) tiff;
  (void) image;
#endif
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

  MagickBooleanType
    debug,
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register ssize_t
    i;

  size_t
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
    photometric;

  unsigned char
    *pixels;

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
  endian_type=UndefinedEndian;
  option=GetImageOption(image_info,"tiff:endian");
  if (option != (const char *) NULL)
    {
      if (LocaleNCompare(option,"msb",3) == 0)
        endian_type=MSBEndian;
      if (LocaleNCompare(option,"lsb",3) == 0)
        endian_type=LSBEndian;;
    }
  switch (endian_type)
  {
    case LSBEndian: mode="wl"; break;
    case MSBEndian: mode="wb"; break;
    default: mode="w"; break;
  }
#if defined(TIFF_VERSION_BIG)
  if (LocaleCompare(image_info->magick,"TIFF64") == 0)
    switch (endian_type)
    {
      case LSBEndian: mode="wl8"; break;
      case MSBEndian: mode="wb8"; break;
      default: mode="w8"; break;
    }
#endif
  tiff=TIFFClientOpen(image->filename,mode,(thandle_t) image,TIFFReadBlob,
    TIFFWriteBlob,TIFFSeekBlob,TIFFCloseBlob,TIFFGetBlobSize,TIFFMapBlob,
    TIFFUnmapBlob);
  if (tiff == (TIFF *) NULL)
    return(MagickFalse);
  scene=0;
  debug=IsEventLogging();
  (void) debug;
  do
  {
    /*
      Initialize TIFF fields.
    */
    if ((image_info->type != UndefinedType) &&
        (image_info->type != OptimizeType))
      (void) SetImageType(image,image_info->type,exception);
    compression=UndefinedCompression;
    if (image->compression != JPEGCompression)
      compression=image->compression;
    if (image_info->compression != UndefinedCompression)
      compression=image_info->compression;
    switch (compression)
    {
      case FaxCompression:
      case Group4Compression:
      {
        (void) SetImageType(image,BilevelType,exception);
        (void) SetImageDepth(image,1,exception);
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
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
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
        SetQuantumMinIsWhite(quantum_info,MagickTrue);
        break;
      }
      case Group4Compression:
      {
        compress_tag=COMPRESSION_CCITTFAX4;
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
          if (image->colorspace == YCbCrColorspace)
            {
              photometric=PHOTOMETRIC_YCBCR;
              (void) TIFFSetField(tiff,TIFFTAG_YCBCRSUBSAMPLING,1,1);
              (void) SetImageStorageClass(image,DirectClass,exception);
              (void) SetImageDepth(image,8,exception);
            }
          else
            photometric=PHOTOMETRIC_RGB;
        (void) TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,3);
        if ((image_info->type != TrueColorType) &&
            (image_info->type != TrueColorAlphaType))
          {
            if ((image_info->type != PaletteType) &&
                (SetImageGray(image,exception) != MagickFalse))
              {
                photometric=(uint16) (quantum_info->min_is_white !=
                  MagickFalse ? PHOTOMETRIC_MINISWHITE :
                  PHOTOMETRIC_MINISBLACK);
                (void) TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,1);
                if ((image->depth == 1) &&
                    (image->alpha_trait == UndefinedPixelTrait))
                  SetImageMonochrome(image,exception);
              }
            else
              if (image->storage_class == PseudoClass)
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
    (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_FILLORDER,&endian);
    if ((compress_tag == COMPRESSION_CCITTFAX3) &&
        (photometric != PHOTOMETRIC_MINISWHITE))
      {
        compress_tag=COMPRESSION_NONE;
        endian=FILLORDER_MSB2LSB;
      }
    else
      if ((compress_tag == COMPRESSION_CCITTFAX4) &&
         (photometric != PHOTOMETRIC_MINISWHITE))
       {
         compress_tag=COMPRESSION_NONE;
         endian=FILLORDER_MSB2LSB;
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
          &samples_per_pixel);
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
    (void) TIFFSetField(tiff,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
    (void) TIFFSetField(tiff,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    if (photometric == PHOTOMETRIC_RGB)
      if ((image_info->interlace == PlaneInterlace) ||
          (image_info->interlace == PartitionInterlace))
        (void) TIFFSetField(tiff,TIFFTAG_PLANARCONFIG,PLANARCONFIG_SEPARATE);
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
            if (image->colorspace == YCbCrColorspace)
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
          &bits_per_sample);
        if (bits_per_sample == 12)
          (void) TIFFSetField(tiff,TIFFTAG_JPEGTABLESMODE,JPEGTABLESMODE_QUANT);
#endif
        break;
      }
      case COMPRESSION_ADOBE_DEFLATE:
      {
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,
          &bits_per_sample);
        if (((photometric == PHOTOMETRIC_RGB) ||
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          (void) TIFFSetField(tiff,TIFFTAG_PREDICTOR,PREDICTOR_HORIZONTAL);
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
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          (void) TIFFSetField(tiff,TIFFTAG_PREDICTOR,PREDICTOR_HORIZONTAL);
        (void) TIFFSetField(tiff,TIFFTAG_LZMAPRESET,(long) (
          image_info->quality == UndefinedCompressionQuality ? 7 :
          MagickMin((ssize_t) image_info->quality/10,9)));
        break;
      }
#endif
      case COMPRESSION_LZW:
      {
        (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,
          &bits_per_sample);
        if (((photometric == PHOTOMETRIC_RGB) ||
             (photometric == PHOTOMETRIC_MINISBLACK)) &&
            ((bits_per_sample == 8) || (bits_per_sample == 16)))
          (void) TIFFSetField(tiff,TIFFTAG_PREDICTOR,PREDICTOR_HORIZONTAL);
        break;
      }
      default:
        break;
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
    if ((LocaleCompare(image_info->magick,"PTIF") != 0) &&
        (image_info->adjoin != MagickFalse) && (GetImageListLength(image) > 1))
      {
        (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_PAGE);
        if (image->scene != 0)
          (void) TIFFSetField(tiff,TIFFTAG_PAGENUMBER,(uint16) image->scene,
            GetImageListLength(image));
      }
    if (image->orientation != UndefinedOrientation)
      (void) TIFFSetField(tiff,TIFFTAG_ORIENTATION,(uint16) image->orientation);
    (void) TIFFSetProfiles(tiff,image);
    {
      uint16
        page,
        pages;

      page=(uint16) scene;
      pages=(uint16) GetImageListLength(image);
      if ((LocaleCompare(image_info->magick,"PTIF") != 0) &&
          (image_info->adjoin != MagickFalse) && (pages > 1))
        (void) TIFFSetField(tiff,TIFFTAG_SUBFILETYPE,FILETYPE_PAGE);
      (void) TIFFSetField(tiff,TIFFTAG_PAGENUMBER,page,pages);
    }
    (void) TIFFSetProperties(tiff,image_info,image,exception);
DisableMSCWarning(4127)
    if (0)
RestoreMSCWarning
      (void) TIFFSetEXIFProperties(tiff,image,exception);
    /*
      Write image scanlines.
    */
    if (GetTIFFInfo(image_info,tiff,&tiff_info) == MagickFalse)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
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
              register const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                quantum_type,pixels,exception);
              (void) length;
              if (TIFFWritePixels(tiff,&tiff_info,y,0,image) == -1)
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
              register const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                RedQuantum,pixels,exception);
              if (TIFFWritePixels(tiff,&tiff_info,y,0,image) == -1)
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
              register const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                GreenQuantum,pixels,exception);
              if (TIFFWritePixels(tiff,&tiff_info,y,1,image) == -1)
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
              register const Quantum
                *magick_restrict p;

              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                BlueQuantum,pixels,exception);
              if (TIFFWritePixels(tiff,&tiff_info,y,2,image) == -1)
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
                register const Quantum
                  *magick_restrict p;

                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                length=ExportQuantumPixels(image,(CacheView *) NULL,
                  quantum_info,AlphaQuantum,pixels,exception);
                if (TIFFWritePixels(tiff,&tiff_info,y,3,image) == -1)
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
          register const Quantum
            *magick_restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          if (TIFFWritePixels(tiff,&tiff_info,y,0,image) == -1)
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
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        /*
          Initialize TIFF colormap.
        */
        (void) ResetMagickMemory(red,0,65536*sizeof(*red));
        (void) ResetMagickMemory(green,0,65536*sizeof(*green));
        (void) ResetMagickMemory(blue,0,65536*sizeof(*blue));
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
          register const Quantum
            *magick_restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          if (TIFFWritePixels(tiff,&tiff_info,y,0,image) == -1)
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
DisableMSCWarning(4127)
    if (0 && (image_info->verbose != MagickFalse))
RestoreMSCWarning
      TIFFPrintDirectory(tiff,stdout,MagickFalse);
    (void) TIFFWriteDirectory(tiff);
    image=SyncNextImageInList(image);
    if (image == (Image *) NULL)
      break;
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  TIFFClose(tiff);
  return(MagickTrue);
}
#endif

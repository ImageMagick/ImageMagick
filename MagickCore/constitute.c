/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     CCCC   OOO   N   N  SSSSS  TTTTT  IIIII  TTTTT  U   U  TTTTT  EEEEE     %
%    C      O   O  NN  N  SS       T      I      T    U   U    T    E         %
%    C      O   O  N N N  ESSS     T      I      T    U   U    T    EEE       %
%    C      O   O  N  NN     SS    T      I      T    U   U    T    E         %
%     CCCC   OOO   N   N  SSSSS    T    IIIII    T     UUU     T    EEEEE     %
%                                                                             %
%                                                                             %
%                  MagickCore Methods to Consitute an Image                   %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               October 1998                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/client.h"
#include "MagickCore/coder-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/constitute-private.h"
#include "MagickCore/delegate.h"
#include "MagickCore/geometry.h"
#include "MagickCore/identify.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/policy.h"
#include "MagickCore/profile.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/statistic.h"
#include "MagickCore/stream.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/timer.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n s t i t u t e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConstituteImage() returns an image from the pixel data you supply.
%  The pixel data must be in scanline order top-to-bottom.  The data can be
%  char, short int, int, float, or double.  Float and double require the
%  pixels to be normalized [0..1], otherwise [0..QuantumRange].  For example, to
%  create a 640x480 image from unsigned red-green-blue character data, use:
%
%      image = ConstituteImage(640,480,"RGB",CharPixel,pixels,&exception);
%
%  The format of the ConstituteImage method is:
%
%      Image *ConstituteImage(const size_t columns,const size_t rows,
%        const char *map,const StorageType storage,const void *pixels,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o columns: width in pixels of the image.
%
%    o rows: height in pixels of the image.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan,
%      Y = yellow, M = magenta, K = black, I = intensity (for grayscale),
%      P = pad.
%
%    o storage: Define the data type of the pixels.  Float and double types are
%      expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose
%      from these types: CharPixel, DoublePixel, FloatPixel, IntegerPixel,
%      LongPixel, QuantumPixel, or ShortPixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ConstituteImage(const size_t columns,const size_t rows,
  const char *map,const StorageType storage,const void *pixels,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    length;

  /*
    Allocate image structure.
  */
  assert(map != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",map);
  assert(pixels != (void *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage((ImageInfo *) NULL,exception);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  length=strlen(map);
  for (i=0; i < (ssize_t) length; i++)
  {
    switch (map[i])
    {
      case 'a':
      case 'A':
      case 'O':
      case 'o':
      {
        image->alpha_trait=BlendPixelTrait;
        break;
      }
      case 'C':
      case 'c':
      case 'm':
      case 'M':
      case 'Y':
      case 'y':
      case 'K':
      case 'k':
      {
        image->colorspace=CMYKColorspace;
        break;
      }
      case 'I':
      case 'i':
      {
        image->colorspace=GRAYColorspace;
        break;
      }
      default:
      {
        if (length == 1)
          image->colorspace=GRAYColorspace;
        break;
      }
    }
  }
  status=SetImageExtent(image,columns,rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  status=ImportImagePixels(image,0,0,columns,rows,map,storage,pixels,exception);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i n g I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PingImage() returns all the properties of an image or image sequence
%  except for the pixels.  It is much faster and consumes far less memory
%  than ReadImage().  On failure, a NULL image is returned and exception
%  describes the reason for the failure.
%
%  The format of the PingImage method is:
%
%      Image *PingImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: Ping the image defined by the file or filename members of
%      this structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static size_t PingStream(const Image *magick_unused(image),
  const void *magick_unused(pixels),const size_t columns)
{
  magick_unreferenced(image);
  magick_unreferenced(pixels);
  return(columns);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport Image *PingImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *ping_info;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  ping_info=CloneImageInfo(image_info);
  ping_info->ping=MagickTrue;
  image=ReadStream(ping_info,&PingStream,exception);
  if (image != (Image *) NULL)
    {
      ResetTimer(&image->timer);
      if (ping_info->verbose != MagickFalse)
        (void) IdentifyImage(image,stdout,MagickFalse,exception);
    }
  ping_info=DestroyImageInfo(ping_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i n g I m a g e s                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PingImages() pings one or more images and returns them as an image list.
%
%  The format of the PingImage method is:
%
%      Image *PingImages(ImageInfo *image_info,const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o filename: the image filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *PingImages(ImageInfo *image_info,const char *filename,
  ExceptionInfo *exception)
{
  char
    ping_filename[MagickPathExtent];

  Image
    *image,
    *images;

  ImageInfo
    *read_info;

  /*
    Ping image list from a file.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) SetImageOption(image_info,"filename",filename);
  (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
  (void) InterpretImageFilename(image_info,(Image *) NULL,image_info->filename,
    (int) image_info->scene,ping_filename,exception);
  if (LocaleCompare(ping_filename,image_info->filename) != 0)
    {
      ExceptionInfo
        *sans;

      ssize_t
        extent,
        scene;

      /*
        Images of the form image-%d.png[1-5].
      */
      read_info=CloneImageInfo(image_info);
      sans=AcquireExceptionInfo();
      (void) SetImageInfo(read_info,0,sans);
      sans=DestroyExceptionInfo(sans);
      if (read_info->number_scenes == 0)
        {
          read_info=DestroyImageInfo(read_info);
          return(PingImage(image_info,exception));
        }
      (void) CopyMagickString(ping_filename,read_info->filename,
        MagickPathExtent);
      images=NewImageList();
      extent=(ssize_t) (read_info->scene+read_info->number_scenes);
      for (scene=(ssize_t) read_info->scene; scene < (ssize_t) extent; scene++)
      {
        (void) InterpretImageFilename(image_info,(Image *) NULL,ping_filename,
          (int) scene,read_info->filename,exception);
        image=PingImage(read_info,exception);
        if (image == (Image *) NULL)
          continue;
        AppendImageToList(&images,image);
      }
      read_info=DestroyImageInfo(read_info);
      return(images);
    }
  return(PingImage(image_info,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadImage() reads an image or image sequence from a file or file handle.
%  The method returns a NULL if there is a memory shortage or if the image
%  cannot be read.  On failure, a NULL image is returned and exception
%  describes the reason for the failure.
%
%  The format of the ReadImage method is:
%
%      Image *ReadImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: Read the image defined by the file or filename members of
%      this structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType IsCoderAuthorized(const char *coder,
  const PolicyRights rights,ExceptionInfo *exception)
{
  if (IsRightsAuthorized(CoderPolicyDomain,rights,coder) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",coder);
      return(MagickFalse);
    }
  return(MagickTrue);
}

MagickExport Image *ReadImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent],
    magick[MagickPathExtent],
    magick_filename[MagickPathExtent];

  const char
    *value;

  const DelegateInfo
    *delegate_info;

  const MagickInfo
    *magick_info;

  DecodeImageHandler
    *decoder;

  ExceptionInfo
    *sans_exception;

  GeometryInfo
    geometry_info;

  Image
    *image,
    *next;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  /*
    Determine image type from filename prefix or suffix (e.g. image.jpg).
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image_info->filename != (char *) NULL);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  read_info=CloneImageInfo(image_info);
  (void) CopyMagickString(magick_filename,read_info->filename,MagickPathExtent);
  (void) SetImageInfo(read_info,0,exception);
  (void) CopyMagickString(filename,read_info->filename,MagickPathExtent);
  (void) CopyMagickString(magick,read_info->magick,MagickPathExtent);
  /*
    Call appropriate image reader based on image type.
  */
  sans_exception=AcquireExceptionInfo();
  magick_info=GetMagickInfo(read_info->magick,sans_exception);
  if (sans_exception->severity == PolicyError)
    magick_info=GetMagickInfo(read_info->magick,exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (magick_info != (const MagickInfo *) NULL)
    {
      if (GetMagickEndianSupport(magick_info) == MagickFalse)
        read_info->endian=UndefinedEndian;
      else
        if ((image_info->endian == UndefinedEndian) &&
            (GetMagickRawSupport(magick_info) != MagickFalse))
          {
            unsigned long
              lsb_first;

            lsb_first=1;
            read_info->endian=(*(char *) &lsb_first) == 1 ? LSBEndian :
              MSBEndian;
         }
    }
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickDecoderSeekableStream(magick_info) != MagickFalse))
    {
      image=AcquireImage(read_info,exception);
      (void) CopyMagickString(image->filename,read_info->filename,
        MagickPathExtent);
      status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
      if (status == MagickFalse)
        {
          read_info=DestroyImageInfo(read_info);
          image=DestroyImage(image);
          return((Image *) NULL);
        }
      if (IsBlobSeekable(image) == MagickFalse)
        {
          /*
            Coder requires a seekable stream.
          */
          *read_info->filename='\0';
          status=ImageToFile(image,read_info->filename,exception);
          if (status == MagickFalse)
            {
              (void) CloseBlob(image);
              read_info=DestroyImageInfo(read_info);
              image=DestroyImage(image);
              return((Image *) NULL);
            }
          read_info->temporary=MagickTrue;
        }
      (void) CloseBlob(image);
      image=DestroyImage(image);
    }
  image=NewImageList();
  decoder=GetImageDecoder(magick_info);
  if (decoder == (DecodeImageHandler *) NULL)
    {
      delegate_info=GetDelegateInfo(read_info->magick,(char *) NULL,exception);
      if (delegate_info == (const DelegateInfo *) NULL)
        {
          (void) SetImageInfo(read_info,0,exception);
          (void) CopyMagickString(read_info->filename,filename,
            MagickPathExtent);
          magick_info=GetMagickInfo(read_info->magick,exception);
          decoder=GetImageDecoder(magick_info);
        }
    }
  if (decoder != (DecodeImageHandler *) NULL)
    {
      /*
        Call appropriate image reader based on image type.
      */
      if (GetMagickDecoderThreadSupport(magick_info) == MagickFalse)
        LockSemaphoreInfo(magick_info->semaphore);
      status=IsCoderAuthorized(read_info->magick,ReadPolicyRights,exception);
      image=(Image *) NULL;
      if (status != MagickFalse)
        image=decoder(read_info,exception);
      if (GetMagickDecoderThreadSupport(magick_info) == MagickFalse)
        UnlockSemaphoreInfo(magick_info->semaphore);
    }
  else
    {
      delegate_info=GetDelegateInfo(read_info->magick,(char *) NULL,exception);
      if (delegate_info == (const DelegateInfo *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
            read_info->magick);
          if (read_info->temporary != MagickFalse)
            (void) RelinquishUniqueFileResource(read_info->filename);
          read_info=DestroyImageInfo(read_info);
          return((Image *) NULL);
        }
      /*
        Let our decoding delegate process the image.
      */
      image=AcquireImage(read_info,exception);
      if (image == (Image *) NULL)
        {
          read_info=DestroyImageInfo(read_info);
          return((Image *) NULL);
        }
      (void) CopyMagickString(image->filename,read_info->filename,
        MagickPathExtent);
      *read_info->filename='\0';
      if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
        LockSemaphoreInfo(delegate_info->semaphore);
      status=InvokeDelegate(read_info,image,read_info->magick,(char *) NULL,
        exception);
      if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
        UnlockSemaphoreInfo(delegate_info->semaphore);
      image=DestroyImageList(image);
      read_info->temporary=MagickTrue;
      if (status != MagickFalse)
        (void) SetImageInfo(read_info,0,exception);
      magick_info=GetMagickInfo(read_info->magick,exception);
      decoder=GetImageDecoder(magick_info);
      if (decoder == (DecodeImageHandler *) NULL)
        {
          if (IsPathAccessible(read_info->filename) != MagickFalse)
            (void) ThrowMagickException(exception,GetMagickModule(),
              MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
              read_info->magick);
          else
            ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
              read_info->filename);
          read_info=DestroyImageInfo(read_info);
          return((Image *) NULL);
        }
      /*
        Call appropriate image reader based on image type.
      */
      if (GetMagickDecoderThreadSupport(magick_info) == MagickFalse)
        LockSemaphoreInfo(magick_info->semaphore);
      status=IsCoderAuthorized(read_info->magick,ReadPolicyRights,exception);
      image=(Image *) NULL;
      if (status != MagickFalse)
        image=(decoder)(read_info,exception);
      if (GetMagickDecoderThreadSupport(magick_info) == MagickFalse)
        UnlockSemaphoreInfo(magick_info->semaphore);
    }
  if (read_info->temporary != MagickFalse)
    {
      (void) RelinquishUniqueFileResource(read_info->filename);
      read_info->temporary=MagickFalse;
      if (image != (Image *) NULL)
        (void) CopyMagickString(image->filename,filename,MagickPathExtent);
    }
  if (image == (Image *) NULL)
    {
      read_info=DestroyImageInfo(read_info);
      return(image);
    }
  if (exception->severity >= ErrorException)
    (void) LogMagickEvent(ExceptionEvent,GetMagickModule(),
      "Coder (%s) generated an image despite an error (%d), "
      "notify the developers",image->magick,exception->severity);
  if (IsBlobTemporary(image) != MagickFalse)
    (void) RelinquishUniqueFileResource(read_info->filename);
  if ((IsSceneGeometry(read_info->scenes,MagickFalse) != MagickFalse) &&
      (GetImageListLength(image) != 1))
    {
      Image
        *clones;

      clones=CloneImages(image,read_info->scenes,exception);
      if (clones != (Image *) NULL)
        {
          image=DestroyImageList(image);
          image=GetFirstImageInList(clones);
        }
    }
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    char
      magick_path[MagickPathExtent],
      *property,
      timestamp[MagickPathExtent];

    const char
      *option;

    const StringInfo
      *profile;

    ssize_t
      option_type;

    static const char
      *source_date_epoch = (const char *) NULL;

    static MagickBooleanType
      epoch_initalized = MagickFalse;

    next->taint=MagickFalse;
    GetPathComponent(magick_filename,MagickPath,magick_path);
    if (*magick_path == '\0' && *next->magick == '\0')
      (void) CopyMagickString(next->magick,magick,MagickPathExtent);
    (void) CopyMagickString(next->magick_filename,magick_filename,
      MagickPathExtent);
    if (IsBlobTemporary(image) != MagickFalse)
      (void) CopyMagickString(next->filename,filename,MagickPathExtent);
    if (next->magick_columns == 0)
      next->magick_columns=next->columns;
    if (next->magick_rows == 0)
      next->magick_rows=next->rows;
    (void) GetImageProperty(next,"exif:*",exception);
    (void) GetImageProperty(next,"icc:*",exception);
    (void) GetImageProperty(next,"iptc:*",exception);
    (void) GetImageProperty(next,"xmp:*",exception);
    value=GetImageProperty(next,"exif:Orientation",exception);
    if (value == (char *) NULL)
      value=GetImageProperty(next,"tiff:Orientation",exception);
    if (value != (char *) NULL)
      {
        next->orientation=(OrientationType) StringToLong(value);
        (void) DeleteImageProperty(next,"tiff:Orientation");
        (void) DeleteImageProperty(next,"exif:Orientation");
      }
    value=GetImageProperty(next,"exif:XResolution",exception);
    if (value != (char *) NULL)
      {
        geometry_info.rho=next->resolution.x;
        geometry_info.sigma=1.0;
        flags=ParseGeometry(value,&geometry_info);
        if (geometry_info.sigma != 0)
          next->resolution.x=geometry_info.rho/geometry_info.sigma;
        if (strchr(value,',') != (char *) NULL)
          next->resolution.x=geometry_info.rho+geometry_info.sigma/1000.0;
        (void) DeleteImageProperty(next,"exif:XResolution");
      }
    value=GetImageProperty(next,"exif:YResolution",exception);
    if (value != (char *) NULL)
      {
        geometry_info.rho=next->resolution.y;
        geometry_info.sigma=1.0;
        flags=ParseGeometry(value,&geometry_info);
        if (geometry_info.sigma != 0)
          next->resolution.y=geometry_info.rho/geometry_info.sigma;
        if (strchr(value,',') != (char *) NULL)
          next->resolution.y=geometry_info.rho+geometry_info.sigma/1000.0;
        (void) DeleteImageProperty(next,"exif:YResolution");
      }
    value=GetImageProperty(next,"exif:ResolutionUnit",exception);
    if (value == (char *) NULL)
      value=GetImageProperty(next,"tiff:ResolutionUnit",exception);
    if (value != (char *) NULL)
      {
        option_type=ParseCommandOption(MagickResolutionOptions,MagickFalse,
          value);
        if (option_type >= 0)
          next->units=(ResolutionType) option_type;
        (void) DeleteImageProperty(next,"exif:ResolutionUnit");
        (void) DeleteImageProperty(next,"tiff:ResolutionUnit");
      }
    if (next->page.width == 0)
      next->page.width=next->columns;
    if (next->page.height == 0)
      next->page.height=next->rows;
    option=GetImageOption(read_info,"caption");
    if (option != (const char *) NULL)
      {
        property=InterpretImageProperties(read_info,next,option,exception);
        (void) SetImageProperty(next,"caption",property,exception);
        property=DestroyString(property);
      }
    option=GetImageOption(read_info,"comment");
    if (option != (const char *) NULL)
      {
        property=InterpretImageProperties(read_info,next,option,exception);
        (void) SetImageProperty(next,"comment",property,exception);
        property=DestroyString(property);
      }
    option=GetImageOption(read_info,"label");
    if (option != (const char *) NULL)
      {
        property=InterpretImageProperties(read_info,next,option,exception);
        (void) SetImageProperty(next,"label",property,exception);
        property=DestroyString(property);
      }
    if (LocaleCompare(next->magick,"TEXT") == 0)
      (void) ParseAbsoluteGeometry("0x0+0+0",&next->page);
    if ((read_info->extract != (char *) NULL) &&
        (read_info->stream == (StreamHandler) NULL))
      {
        RectangleInfo
          geometry;

        SetGeometry(next,&geometry);
        flags=ParseAbsoluteGeometry(read_info->extract,&geometry);
        if ((next->columns != geometry.width) ||
            (next->rows != geometry.height))
          {
            if (((flags & XValue) != 0) || ((flags & YValue) != 0))
              {
                Image
                  *crop_image;

                crop_image=CropImage(next,&geometry,exception);
                if (crop_image != (Image *) NULL)
                  ReplaceImageInList(&next,crop_image);
              }
            else
              if (((flags & WidthValue) != 0) || ((flags & HeightValue) != 0))
                {
                  Image
                    *size_image;

                  flags=ParseRegionGeometry(next,read_info->extract,&geometry,
                    exception);
                  size_image=ResizeImage(next,geometry.width,geometry.height,
                    next->filter,exception);
                  if (size_image != (Image *) NULL)
                    ReplaceImageInList(&next,size_image);
                }
          }
      }
    profile=GetImageProfile(next,"icc");
    if (profile == (const StringInfo *) NULL)
      profile=GetImageProfile(next,"icm");
    profile=GetImageProfile(next,"iptc");
    if (profile == (const StringInfo *) NULL)
      profile=GetImageProfile(next,"8bim");
    if (epoch_initalized == MagickFalse)
      {
        source_date_epoch=getenv("SOURCE_DATE_EPOCH");
        epoch_initalized=MagickTrue;
      }
    if (source_date_epoch == (const char *) NULL)
      {
        (void) FormatMagickTime((time_t) GetBlobProperties(next)->st_mtime,
          MagickPathExtent,timestamp);
        (void) SetImageProperty(next,"date:modify",timestamp,exception);
        (void) FormatMagickTime((time_t) GetBlobProperties(next)->st_ctime,
          MagickPathExtent,timestamp);
        (void) SetImageProperty(next,"date:create",timestamp,exception);
      }
    option=GetImageOption(image_info,"delay");
    if (option != (const char *) NULL)
      {
        flags=ParseGeometry(option,&geometry_info);
        if ((flags & GreaterValue) != 0)
          {
            if (next->delay > (size_t) floor(geometry_info.rho+0.5))
              next->delay=(size_t) floor(geometry_info.rho+0.5);
          }
        else
          if ((flags & LessValue) != 0)
            {
              if (next->delay < (size_t) floor(geometry_info.rho+0.5))
                next->ticks_per_second=(ssize_t) floor(geometry_info.sigma+0.5);
            }
          else
            next->delay=(size_t) floor(geometry_info.rho+0.5);
        if ((flags & SigmaValue) != 0)
          next->ticks_per_second=(ssize_t) floor(geometry_info.sigma+0.5);
      }
    option=GetImageOption(image_info,"dispose");
    if (option != (const char *) NULL)
      {
        option_type=ParseCommandOption(MagickDisposeOptions,MagickFalse,
          option);
        if (option_type >= 0)
          next->dispose=(DisposeType) option_type;
      }
    if (read_info->verbose != MagickFalse)
      (void) IdentifyImage(next,stderr,MagickFalse,exception);
    image=next;
  }
  read_info=DestroyImageInfo(read_info);
  if (GetBlobError(image) != MagickFalse)
    ThrowImageException(FileOpenError,"UnableToReadFile");
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d I m a g e s                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadImages() reads one or more images and returns them as an image list.
%
%  The format of the ReadImage method is:
%
%      Image *ReadImages(ImageInfo *image_info,const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o filename: the image filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ReadImages(ImageInfo *image_info,const char *filename,
  ExceptionInfo *exception)
{
  char
    read_filename[MagickPathExtent];

  Image
    *image,
    *images;

  ImageInfo
    *read_info;

  /*
    Read image list from a file.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  (void) SetImageOption(read_info,"filename",filename);
  (void) CopyMagickString(read_info->filename,filename,MagickPathExtent);
  (void) InterpretImageFilename(read_info,(Image *) NULL,filename,
    (int) read_info->scene,read_filename,exception);
  if (LocaleCompare(read_filename,read_info->filename) != 0)
    {
      ExceptionInfo
        *sans;

      ssize_t
        extent,
        scene;

      /*
        Images of the form image-%d.png[1-5].
      */
      sans=AcquireExceptionInfo();
      (void) SetImageInfo(read_info,0,sans);
      sans=DestroyExceptionInfo(sans);
      if (read_info->number_scenes != 0)
        {
          (void) CopyMagickString(read_filename,read_info->filename,
            MagickPathExtent);
          images=NewImageList();
          extent=(ssize_t) (read_info->scene+read_info->number_scenes);
          scene=(ssize_t) read_info->scene;
          for ( ; scene < (ssize_t) extent; scene++)
          {
            (void) InterpretImageFilename(image_info,(Image *) NULL,
              read_filename,(int) scene,read_info->filename,exception);
            image=ReadImage(read_info,exception);
            if (image == (Image *) NULL)
              continue;
            AppendImageToList(&images,image);
          }
          read_info=DestroyImageInfo(read_info);
          return(images);
        }
    }
  (void) CopyMagickString(read_info->filename,filename,MagickPathExtent);
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d I n l i n e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadInlineImage() reads a Base64-encoded inline image or image sequence.
%  The method returns a NULL if there is a memory shortage or if the image
%  cannot be read.  On failure, a NULL image is returned and exception
%  describes the reason for the failure.
%
%  The format of the ReadInlineImage method is:
%
%      Image *ReadInlineImage(const ImageInfo *image_info,const char *content,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o content: the image encoded in Base64.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ReadInlineImage(const ImageInfo *image_info,
  const char *content,ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *read_info;

  unsigned char
    *blob;

  size_t
    length;

  register const char
    *p;

  /*
    Skip over header (e.g. data:image/gif;base64,).
  */
  image=NewImageList();
  for (p=content; (*p != ',') && (*p != '\0'); p++) ;
  if (*p == '\0')
    ThrowReaderException(CorruptImageError,"CorruptImage");
  p++;
  length=0;
  blob=Base64Decode(p,&length);
  if (length == 0)
    {
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      ThrowReaderException(CorruptImageError,"CorruptImage");
    }
  read_info=CloneImageInfo(image_info);
  (void) SetImageInfoProgressMonitor(read_info,(MagickProgressMonitor) NULL,
    (void *) NULL);
  *read_info->filename='\0';
  *read_info->magick='\0';
  image=BlobToImage(read_info,blob,length,exception);
  blob=(unsigned char *) RelinquishMagickMemory(blob);
  read_info=DestroyImageInfo(read_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteImage() writes an image or an image sequence to a file or file handle.
%  If writing to a file is on disk, the name is defined by the filename member
%  of the image structure.  WriteImage() returns MagickFalse is there is a
%  memory shortage or if the image cannot be written.  Check the exception
%  member of image to determine the cause for any failure.
%
%  The format of the WriteImage method is:
%
%      MagickBooleanType WriteImage(const ImageInfo *image_info,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WriteImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  const char
    *option;

  const DelegateInfo
    *delegate_info;

  const MagickInfo
    *magick_info;

  EncodeImageHandler
    *encoder;

  ExceptionInfo
    *sans_exception;

  ImageInfo
    *write_info;

  MagickBooleanType
    status,
    temporary;

  /*
    Determine image type from filename prefix or suffix (e.g. image.jpg).
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  sans_exception=AcquireExceptionInfo();
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->filename,image->filename,
    MagickPathExtent);
  (void) SetImageInfo(write_info,1,sans_exception);
  if (*write_info->magick == '\0')
    (void) CopyMagickString(write_info->magick,image->magick,MagickPathExtent);
  (void) CopyMagickString(filename,image->filename,MagickPathExtent);
  (void) CopyMagickString(image->filename,write_info->filename,
    MagickPathExtent);
  /*
    Call appropriate image writer based on image type.
  */
  magick_info=GetMagickInfo(write_info->magick,sans_exception);
  if (sans_exception->severity == PolicyError)
    magick_info=GetMagickInfo(write_info->magick,exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (magick_info != (const MagickInfo *) NULL)
    {
      if (GetMagickEndianSupport(magick_info) == MagickFalse)
        image->endian=UndefinedEndian;
      else
        if ((image_info->endian == UndefinedEndian) &&
            (GetMagickRawSupport(magick_info) != MagickFalse))
          {
            unsigned long
              lsb_first;

            lsb_first=1;
            image->endian=(*(char *) &lsb_first) == 1 ? LSBEndian : MSBEndian;
         }
    }
  (void) SyncImageProfiles(image);
  DisassociateImageStream(image);
  option=GetImageOption(image_info,"delegate:bimodal");
  if ((IsStringTrue(option) != MagickFalse) &&
      (write_info->page == (char *) NULL) &&
      (GetPreviousImageInList(image) == (Image *) NULL) &&
      (GetNextImageInList(image) == (Image *) NULL) &&
      (IsTaintImage(image) == MagickFalse) )
    {
      delegate_info=GetDelegateInfo(image->magick,write_info->magick,exception);
      if ((delegate_info != (const DelegateInfo *) NULL) &&
          (GetDelegateMode(delegate_info) == 0) &&
          (IsPathAccessible(image->magick_filename) != MagickFalse))
        {
          /*
            Process image with bi-modal delegate.
          */
          (void) CopyMagickString(image->filename,image->magick_filename,
            MagickPathExtent);
          status=InvokeDelegate(write_info,image,image->magick,
            write_info->magick,exception);
          write_info=DestroyImageInfo(write_info);
          (void) CopyMagickString(image->filename,filename,MagickPathExtent);
          return(status);
        }
    }
  status=MagickFalse;
  temporary=MagickFalse;
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickEncoderSeekableStream(magick_info) != MagickFalse))
    {
      char
        image_filename[MagickPathExtent];

      (void) CopyMagickString(image_filename,image->filename,MagickPathExtent);
      status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      (void) CopyMagickString(image->filename, image_filename,MagickPathExtent);
      if (status != MagickFalse)
        {
          if (IsBlobSeekable(image) == MagickFalse)
            {
              /*
                A seekable stream is required by the encoder.
              */
              write_info->adjoin=MagickTrue;
              (void) CopyMagickString(write_info->filename,image->filename,
                MagickPathExtent);
              (void) AcquireUniqueFilename(image->filename);
              temporary=MagickTrue;
            }
          (void) CloseBlob(image);
        }
    }
  encoder=GetImageEncoder(magick_info);
  if (encoder != (EncodeImageHandler *) NULL)
    {
      /*
        Call appropriate image writer based on image type.
      */
      if (GetMagickEncoderThreadSupport(magick_info) == MagickFalse)
        LockSemaphoreInfo(magick_info->semaphore);
      status=IsCoderAuthorized(write_info->magick,WritePolicyRights,exception);
      if (status != MagickFalse)
        status=encoder(write_info,image,exception);
      if (GetMagickEncoderThreadSupport(magick_info) == MagickFalse)
        UnlockSemaphoreInfo(magick_info->semaphore);
    }
  else
    {
      delegate_info=GetDelegateInfo((char *) NULL,write_info->magick,exception);
      if (delegate_info != (DelegateInfo *) NULL)
        {
          /*
            Process the image with delegate.
          */
          *write_info->filename='\0';
          if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
            LockSemaphoreInfo(delegate_info->semaphore);
          status=InvokeDelegate(write_info,image,(char *) NULL,
            write_info->magick,exception);
          if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
            UnlockSemaphoreInfo(delegate_info->semaphore);
          (void) CopyMagickString(image->filename,filename,MagickPathExtent);
        }
      else
        {
          sans_exception=AcquireExceptionInfo();
          magick_info=GetMagickInfo(write_info->magick,sans_exception);
          if (sans_exception->severity == PolicyError)
            magick_info=GetMagickInfo(write_info->magick,exception);
          sans_exception=DestroyExceptionInfo(sans_exception);
          if ((write_info->affirm == MagickFalse) &&
              (magick_info == (const MagickInfo *) NULL))
            {
              (void) CopyMagickString(write_info->magick,image->magick,
                MagickPathExtent);
              magick_info=GetMagickInfo(write_info->magick,exception);
            }
          encoder=GetImageEncoder(magick_info);
          if (encoder == (EncodeImageHandler *) NULL)
            {
              char
                extension[MagickPathExtent];

              GetPathComponent(image->filename,ExtensionPath,extension);
              if (*extension != '\0')
                magick_info=GetMagickInfo(extension,exception);
              else
                magick_info=GetMagickInfo(image->magick,exception);
              (void) CopyMagickString(image->filename,filename,
                MagickPathExtent);
              encoder=GetImageEncoder(magick_info);
            }
          if (encoder == (EncodeImageHandler *) NULL)
            {
              magick_info=GetMagickInfo(image->magick,exception);
              encoder=GetImageEncoder(magick_info);
              if (encoder == (EncodeImageHandler *) NULL)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  MissingDelegateError,"NoEncodeDelegateForThisImageFormat",
                  "`%s'",write_info->magick);
            }
          if (encoder != (EncodeImageHandler *) NULL)
            {
              /*
                Call appropriate image writer based on image type.
              */
              if (GetMagickEncoderThreadSupport(magick_info) == MagickFalse)
                LockSemaphoreInfo(magick_info->semaphore);
              status=IsCoderAuthorized(write_info->magick,WritePolicyRights,
                exception);
              if (status != MagickFalse)
                status=encoder(write_info,image,exception);
              if (GetMagickEncoderThreadSupport(magick_info) == MagickFalse)
                UnlockSemaphoreInfo(magick_info->semaphore);
            }
        }
    }
  if (temporary != MagickFalse)
    {
      /*
        Copy temporary image file to permanent.
      */
      status=OpenBlob(write_info,image,ReadBinaryBlobMode,exception);
      if (status != MagickFalse)
        {
          (void) RelinquishUniqueFileResource(write_info->filename);
          status=ImageToFile(image,write_info->filename,exception);
        }
      (void) CloseBlob(image);
      (void) RelinquishUniqueFileResource(image->filename);
      (void) CopyMagickString(image->filename,write_info->filename,
        MagickPathExtent);
    }
  if ((LocaleCompare(write_info->magick,"info") != 0) &&
      (write_info->verbose != MagickFalse))
    (void) IdentifyImage(image,stdout,MagickFalse,exception);
  write_info=DestroyImageInfo(write_info);
  if (GetBlobError(image) != MagickFalse)
    ThrowBinaryException(FileOpenError,"UnableToWriteFile",image->filename);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e I m a g e s                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteImages() writes an image sequence into one or more files.  While
%  WriteImage() can write an image sequence, it is limited to writing
%  the sequence into a single file using a format which supports multiple
%  frames.   WriteImages(), however, does not have this limitation, instead it
%  generates multiple output files if necessary (or when requested).  When
%  ImageInfo's adjoin flag is set to MagickFalse, the file name is expected
%  to include a printf-style formatting string for the frame number (e.g.
%  "image%02d.png").
%
%  The format of the WriteImages method is:
%
%      MagickBooleanType WriteImages(const ImageInfo *image_info,Image *images,
%        const char *filename,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o images: the image list.
%
%    o filename: the image filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WriteImages(const ImageInfo *image_info,
  Image *images,const char *filename,ExceptionInfo *exception)
{
#define WriteImageTag  "Write/Image"

  ExceptionInfo
    *sans_exception;

  ImageInfo
    *write_info;

  MagickBooleanType
    proceed;

  MagickOffsetType
    progress;

  MagickProgressMonitor
    progress_monitor;

  MagickSizeType
    number_images;

  MagickStatusType
    status;

  register Image
    *p;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  write_info=CloneImageInfo(image_info);
  *write_info->magick='\0';
  images=GetFirstImageInList(images);
  if (filename != (const char *) NULL)
    for (p=images; p != (Image *) NULL; p=GetNextImageInList(p))
      (void) CopyMagickString(p->filename,filename,MagickPathExtent);
  (void) CopyMagickString(write_info->filename,images->filename,
    MagickPathExtent);
  sans_exception=AcquireExceptionInfo();
  (void) SetImageInfo(write_info,(unsigned int) GetImageListLength(images),
    sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (*write_info->magick == '\0')
    (void) CopyMagickString(write_info->magick,images->magick,MagickPathExtent);
  p=images;
  for ( ; GetNextImageInList(p) != (Image *) NULL; p=GetNextImageInList(p))
  {
    register Image
      *next;

    next=GetNextImageInList(p);
    if (next == (Image *) NULL)
      break;
    if (p->scene >= next->scene)
      {
        register ssize_t
          i;

        /*
          Generate consistent scene numbers.
        */
        i=(ssize_t) images->scene;
        for (p=images; p != (Image *) NULL; p=GetNextImageInList(p))
          p->scene=(size_t) i++;
        break;
      }
  }
  /*
    Write images.
  */
  status=MagickTrue;
  progress_monitor=(MagickProgressMonitor) NULL;
  progress=0;
  number_images=GetImageListLength(images);
  for (p=images; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    if (number_images != 1)
      progress_monitor=SetImageProgressMonitor(p,(MagickProgressMonitor) NULL,
        p->client_data);
    status&=WriteImage(write_info,p,exception);
    if (number_images != 1)
      (void) SetImageProgressMonitor(p,progress_monitor,p->client_data);
    if (write_info->adjoin != MagickFalse)
      break;
    if (number_images != 1)
      {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(p,WriteImageTag,progress,number_images);
        if (proceed == MagickFalse)
          break;
      }
  }
  write_info=DestroyImageInfo(write_info);
  return(status != 0 ? MagickTrue : MagickFalse);
}

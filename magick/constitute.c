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
%                               John Cristy                                   %
%                               October 1998                                  %
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
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/cache.h"
#include "magick/client.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/geometry.h"
#include "magick/identify.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel.h"
#include "magick/policy.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum.h"
#include "magick/resize.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/statistic.h"
#include "magick/stream.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/timer.h"
#include "magick/token.h"
#include "magick/transform.h"
#include "magick/utility.h"

static SemaphoreInfo
  *constitute_semaphore = (SemaphoreInfo *) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n s t i t u t e C o m p o n e n t G e n e s i s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConstituteComponentGenesis() instantiates the constitute component.
%
%  The format of the ConstituteComponentGenesis method is:
%
%      MagickBooleanType ConstituteComponentGenesis(void)
%
*/
MagickExport MagickBooleanType ConstituteComponentGenesis(void)
{
  AcquireSemaphoreInfo(&constitute_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n s t i t u t e C o m p o n e n t T e r m i n u s                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConstituteComponentTerminus() destroys the constitute component.
%
%  The format of the ConstituteComponentTerminus method is:
%
%      ConstituteComponentTerminus(void)
%
*/
MagickExport void ConstituteComponentTerminus(void)
{
  if (constitute_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&constitute_semaphore);
  DestroySemaphoreInfo(&constitute_semaphore);
}

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
MagickExport Image *ConstituteImage(const size_t columns,
  const size_t rows,const char *map,const StorageType storage,
  const void *pixels,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  /*
    Allocate image structure.
  */
  assert(map != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",map);
  assert(pixels != (void *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage((ImageInfo *) NULL);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(OptionError,"NonZeroWidthAndHeightRequired");
  image->columns=columns;
  image->rows=rows;
  (void) SetImageBackgroundColor(image);
  status=ImportImagePixels(image,0,0,columns,rows,map,storage,pixels);
  if (status == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImage(image);
    }
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
  assert(image_info->signature == MagickSignature);
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
        (void) IdentifyImage(image,stdout,MagickFalse);
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
%      Image *PingImages(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *PingImages(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent];

  Image
    *image,
    *images;

  ImageInfo
    *read_info;

  /*
    Ping image list from a file.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) InterpretImageFilename(image_info,(Image *) NULL,image_info->filename,
    (int) image_info->scene,filename);
  if (LocaleCompare(filename,image_info->filename) != 0)
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
      (void) CopyMagickString(filename,read_info->filename,MaxTextExtent);
      images=NewImageList();
      extent=(ssize_t) (read_info->scene+read_info->number_scenes);
      for (scene=(ssize_t) read_info->scene; scene < (ssize_t) extent; scene++)
      {
        (void) InterpretImageFilename(image_info,(Image *) NULL,filename,(int)
          scene,read_info->filename);
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
MagickExport Image *ReadImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent],
    magick[MaxTextExtent],
    magick_filename[MaxTextExtent];

  const char
    *value;

  const DelegateInfo
    *delegate_info;

  const MagickInfo
    *magick_info;

  ExceptionInfo
    *sans_exception;

  GeometryInfo
    geometry_info;

  Image
    *image,
    *next;

  ImageInfo
    *read_info;

  MagickStatusType
    flags,
    thread_support;

  PolicyDomain
    domain;

  PolicyRights
    rights;

  /*
    Determine image type from filename prefix or suffix (e.g. image.jpg).
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image_info->filename != (char *) NULL);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  read_info=CloneImageInfo(image_info);
  (void) CopyMagickString(magick_filename,read_info->filename,MaxTextExtent);
  (void) SetImageInfo(read_info,0,exception);
  (void) CopyMagickString(filename,read_info->filename,MaxTextExtent);
  (void) CopyMagickString(magick,read_info->magick,MaxTextExtent);
  domain=CoderPolicyDomain;
  rights=ReadPolicyRights;
  if (IsRightsAuthorized(domain,rights,read_info->magick) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",read_info->filename);
      read_info=DestroyImageInfo(read_info);
      return((Image *) NULL);
    }
  /*
    Call appropriate image reader based on image type.
  */
  sans_exception=AcquireExceptionInfo();
  magick_info=GetMagickInfo(read_info->magick,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (magick_info != (const MagickInfo *) NULL)
    {
      if (GetMagickEndianSupport(magick_info) == MagickFalse)
        read_info->endian=UndefinedEndian;
      else
        if ((image_info->endian == UndefinedEndian) &&
            (GetMagickRawSupport(magick_info) != MagickFalse))
          {
            size_t
              lsb_first;

            lsb_first=1;
            read_info->endian=(*(char *) &lsb_first) == 1 ? LSBEndian :
              MSBEndian;
         }
    }
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickSeekableStream(magick_info) != MagickFalse))
    {
      MagickBooleanType
        status;

      image=AcquireImage(read_info);
      (void) CopyMagickString(image->filename,read_info->filename,
        MaxTextExtent);
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
  if (constitute_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&constitute_semaphore);
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetImageDecoder(magick_info) == (DecodeImageHandler *) NULL))
    {
      delegate_info=GetDelegateInfo(read_info->magick,(char *) NULL,exception);
      if (delegate_info == (const DelegateInfo *) NULL)
        {
          (void) SetImageInfo(read_info,0,exception);
          (void) CopyMagickString(read_info->filename,filename,MaxTextExtent);
          magick_info=GetMagickInfo(read_info->magick,exception);
        }
    }
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetImageDecoder(magick_info) != (DecodeImageHandler *) NULL))
    {
      thread_support=GetMagickThreadSupport(magick_info);
      if ((thread_support & DecoderThreadSupport) == 0)
        LockSemaphoreInfo(constitute_semaphore);
      image=GetImageDecoder(magick_info)(read_info,exception);
      if ((thread_support & DecoderThreadSupport) == 0)
        UnlockSemaphoreInfo(constitute_semaphore);
    }
  else
    {
      delegate_info=GetDelegateInfo(read_info->magick,(char *) NULL,exception);
      if (delegate_info == (const DelegateInfo *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
            read_info->filename);
          if (read_info->temporary != MagickFalse)
            (void) RelinquishUniqueFileResource(read_info->filename);
          read_info=DestroyImageInfo(read_info);
          return((Image *) NULL);
        }
      /*
        Let our decoding delegate process the image.
      */
      image=AcquireImage(read_info);
      if (image == (Image *) NULL)
        {
          read_info=DestroyImageInfo(read_info);
          return((Image *) NULL);
        }
      (void) CopyMagickString(image->filename,read_info->filename,
        MaxTextExtent);
      *read_info->filename='\0';
      if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
        LockSemaphoreInfo(constitute_semaphore);
      (void) InvokeDelegate(read_info,image,read_info->magick,(char *) NULL,
        exception);
      if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
        UnlockSemaphoreInfo(constitute_semaphore);
      image=DestroyImageList(image);
      read_info->temporary=MagickTrue;
      (void) SetImageInfo(read_info,0,exception);
      magick_info=GetMagickInfo(read_info->magick,exception);
      if ((magick_info == (const MagickInfo *) NULL) ||
          (GetImageDecoder(magick_info) == (DecodeImageHandler *) NULL))
        {
          if (IsPathAccessible(read_info->filename) != MagickFalse)
            (void) ThrowMagickException(exception,GetMagickModule(),
              MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
              read_info->filename);
          else
            ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
              read_info->filename);
          read_info=DestroyImageInfo(read_info);
          return((Image *) NULL);
        }
      thread_support=GetMagickThreadSupport(magick_info);
      if ((thread_support & DecoderThreadSupport) == 0)
        LockSemaphoreInfo(constitute_semaphore);
      image=(Image *) (GetImageDecoder(magick_info))(read_info,exception);
      if ((thread_support & DecoderThreadSupport) == 0)
        UnlockSemaphoreInfo(constitute_semaphore);
    }
  if (read_info->temporary != MagickFalse)
    {
      (void) RelinquishUniqueFileResource(read_info->filename);
      read_info->temporary=MagickFalse;
      if (image != (Image *) NULL)
        (void) CopyMagickString(image->filename,filename,MaxTextExtent);
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
  if ((GetNextImageInList(image) != (Image *) NULL) &&
      (IsSceneGeometry(read_info->scenes,MagickFalse) != MagickFalse))
    {
      Image
        *clones;

      clones=CloneImages(image,read_info->scenes,exception);
      if (clones == (Image *) NULL)
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "SubimageSpecificationReturnsNoImages","`%s'",read_info->filename);
      else
        {
          image=DestroyImageList(image);
          image=GetFirstImageInList(clones);
        }
    }
  if (GetBlobError(image) != MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,
        "AnErrorHasOccurredReadingFromFile",read_info->filename);
      image=DestroyImageList(image);
      read_info=DestroyImageInfo(read_info);
      return((Image *) NULL);
    }
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    char
      magick_path[MaxTextExtent],
      *property,
      timestamp[MaxTextExtent];

    const char
      *option;

    const StringInfo
      *profile;

    next->taint=MagickFalse;
    GetPathComponent(magick_filename,MagickPath,magick_path);
    if (*magick_path == '\0')
      (void) CopyMagickString(next->magick,magick,MaxTextExtent);
    (void) CopyMagickString(next->magick_filename,magick_filename,
      MaxTextExtent);
    if (IsBlobTemporary(image) != MagickFalse)
      (void) CopyMagickString(next->filename,filename,MaxTextExtent);
    if (next->magick_columns == 0)
      next->magick_columns=next->columns;
    if (next->magick_rows == 0)
      next->magick_rows=next->rows;
    value=GetImageProperty(next,"tiff:Orientation");
    if (value == (char *) NULL)
      value=GetImageProperty(next,"exif:Orientation");
    if (value != (char *) NULL)
      {
        next->orientation=(OrientationType) StringToLong(value);
        (void) DeleteImageProperty(next,"tiff:Orientation");
        (void) DeleteImageProperty(next,"exif:Orientation");
      }
    value=GetImageProperty(next,"exif:XResolution");
    if (value != (char *) NULL)
      {
        geometry_info.rho=next->x_resolution;
        geometry_info.sigma=1.0;
        flags=ParseGeometry(value,&geometry_info);
        if (geometry_info.sigma != 0)
          next->x_resolution=geometry_info.rho/geometry_info.sigma;
        (void) DeleteImageProperty(next,"exif:XResolution");
      }
    value=GetImageProperty(next,"exif:YResolution");
    if (value != (char *) NULL)
      {
        geometry_info.rho=next->y_resolution;
        geometry_info.sigma=1.0;
        flags=ParseGeometry(value,&geometry_info);
        if (geometry_info.sigma != 0)
          next->y_resolution=geometry_info.rho/geometry_info.sigma;
        (void) DeleteImageProperty(next,"exif:YResolution");
      }
    value=GetImageProperty(next,"tiff:ResolutionUnit");
    if (value == (char *) NULL)
      value=GetImageProperty(next,"exif:ResolutionUnit");
    if (value != (char *) NULL)
      {
        next->units=(ResolutionType) (StringToLong(value)-1);
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
        property=InterpretImageProperties(read_info,next,option);
        (void) SetImageProperty(next,"caption",property);
        property=DestroyString(property);
      }
    option=GetImageOption(read_info,"comment");
    if (option != (const char *) NULL)
      {
        property=InterpretImageProperties(read_info,next,option);
        (void) SetImageProperty(next,"comment",property);
        property=DestroyString(property);
      }
    option=GetImageOption(read_info,"label");
    if (option != (const char *) NULL)
      {
        property=InterpretImageProperties(read_info,next,option);
        (void) SetImageProperty(next,"label",property);
        property=DestroyString(property);
      }
    if (LocaleCompare(next->magick,"TEXT") == 0)
      (void) ParseAbsoluteGeometry("0x0+0+0",&next->page);
    if ((read_info->extract != (char *) NULL) &&
        (read_info->stream == (StreamHandler) NULL))
      {
        RectangleInfo
          geometry;

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
                    next->filter,next->blur,exception);
                  if (size_image != (Image *) NULL)
                    ReplaceImageInList(&next,size_image);
                }
          }
      }
    profile=GetImageProfile(next,"icc");
    if (profile == (const StringInfo *) NULL)
      profile=GetImageProfile(next,"icm");
    if (profile != (const StringInfo *) NULL)
      {
        next->color_profile.length=GetStringInfoLength(profile);
        next->color_profile.info=GetStringInfoDatum(profile);
      }
    profile=GetImageProfile(next,"iptc");
    if (profile == (const StringInfo *) NULL)
      profile=GetImageProfile(next,"8bim");
    if (profile != (const StringInfo *) NULL)
      {
        next->iptc_profile.length=GetStringInfoLength(profile);
        next->iptc_profile.info=GetStringInfoDatum(profile);
      }
    (void) FormatMagickTime(GetBlobProperties(next)->st_mtime,MaxTextExtent,
      timestamp);
    (void) SetImageProperty(next,"date:modify",timestamp);
    (void) FormatMagickTime(GetBlobProperties(next)->st_ctime,MaxTextExtent,
      timestamp);
    (void) SetImageProperty(next,"date:create",timestamp);
    option=GetImageOption(image_info,"delay");
    if (option != (const char *) NULL)
      {
        GeometryInfo
          geometry_info;

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
      next->dispose=(DisposeType) ParseCommandOption(MagickDisposeOptions,
        MagickFalse,option);
    if (read_info->verbose != MagickFalse)
      (void) IdentifyImage(next,stderr,MagickFalse);
    image=next;
  }
  read_info=DestroyImageInfo(read_info);
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
%      Image *ReadImages(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ReadImages(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent];

  Image
    *image,
    *images;

  ImageInfo
    *read_info;

  /*
    Read image list from a file.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) InterpretImageFilename(image_info,(Image *) NULL,image_info->filename,
    (int) image_info->scene,filename);
  if (LocaleCompare(filename,image_info->filename) != 0)
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
          return(ReadImage(image_info,exception));
        }
      (void) CopyMagickString(filename,read_info->filename,MaxTextExtent);
      images=NewImageList();
      extent=(ssize_t) (read_info->scene+read_info->number_scenes);
      for (scene=(ssize_t) read_info->scene; scene < (ssize_t) extent; scene++)
      {
        (void) InterpretImageFilename(image_info,(Image *) NULL,filename,(int)
          scene,read_info->filename);
        image=ReadImage(read_info,exception);
        if (image == (Image *) NULL)
          continue;
        AppendImageToList(&images,image);
      }
      read_info=DestroyImageInfo(read_info);
      return(images);
    }
  return(ReadImage(image_info,exception));
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
    ThrowReaderException(CorruptImageError,"CorruptImage");
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
%      MagickBooleanType WriteImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType WriteImage(const ImageInfo *image_info,
  Image *image)
{
  char
    filename[MaxTextExtent];

  const char
    *option;

  const DelegateInfo
    *delegate_info;

  const MagickInfo
    *magick_info;

  ExceptionInfo
    *sans_exception;

  ImageInfo
    *write_info;

  MagickBooleanType
    status,
    temporary;

  MagickStatusType
    thread_support;

  PolicyDomain
    domain;

  PolicyRights
    rights;

  /*
    Determine image type from filename prefix or suffix (e.g. image.jpg).
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  sans_exception=AcquireExceptionInfo();
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->filename,image->filename,MaxTextExtent);
  if (*write_info->magick == '\0')
    (void) CopyMagickString(write_info->magick,image->magick,MaxTextExtent);
  (void) SetImageInfo(write_info,1,sans_exception);
  if (LocaleCompare(write_info->magick,"clipmask") == 0)
    {
      if (image->clip_mask == (Image *) NULL)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            OptionError,"NoClipPathDefined","`%s'",image->filename);
          write_info=DestroyImageInfo(write_info);
          return(MagickFalse);
        }
      image=image->clip_mask;
      (void) SetImageInfo(write_info,1,sans_exception);
    }
  (void) CopyMagickString(filename,image->filename,MaxTextExtent);
  (void) CopyMagickString(image->filename,write_info->filename,MaxTextExtent);
  domain=CoderPolicyDomain;
  rights=WritePolicyRights;
  if (IsRightsAuthorized(domain,rights,write_info->magick) == MagickFalse)
    {
      sans_exception=DestroyExceptionInfo(sans_exception);
      write_info=DestroyImageInfo(write_info);
      errno=EPERM;
      ThrowBinaryException(PolicyError,"NotAuthorized",filename);
    }
  /*
    Call appropriate image reader based on image type.
  */
  magick_info=GetMagickInfo(write_info->magick,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (magick_info != (const MagickInfo *) NULL)
    {
      if (GetMagickEndianSupport(magick_info) == MagickFalse)
        image->endian=UndefinedEndian;
      else
        if ((image_info->endian == UndefinedEndian) &&
            (GetMagickRawSupport(magick_info) != MagickFalse))
          {
            size_t
              lsb_first;

            lsb_first=1;
            image->endian=(*(char *) &lsb_first) == 1 ? LSBEndian : MSBEndian;
         }
    }
  (void) SyncImageProfiles(image);
  option=GetImageOption(image_info,"delegate:bimodal");
  if ((option != (const char *) NULL) &&
      (IsMagickTrue(option) != MagickFalse) &&
      (write_info->page == (char *) NULL) &&
      (GetPreviousImageInList(image) == (Image *) NULL) &&
      (GetNextImageInList(image) == (Image *) NULL) &&
      (IsTaintImage(image) == MagickFalse))
    {
      delegate_info=GetDelegateInfo(image->magick,write_info->magick,
        &image->exception);
      if ((delegate_info != (const DelegateInfo *) NULL) &&
          (GetDelegateMode(delegate_info) == 0) &&
          (IsPathAccessible(image->magick_filename) != MagickFalse))
        {
          /*
            Process image with bi-modal delegate.
          */
          (void) CopyMagickString(image->filename,image->magick_filename,
            MaxTextExtent);
          status=InvokeDelegate(write_info,image,image->magick,
            write_info->magick,&image->exception);
          write_info=DestroyImageInfo(write_info);
          (void) CopyMagickString(image->filename,filename,MaxTextExtent);
          return(status);
        }
    }
  status=MagickFalse;
  temporary=MagickFalse;
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickSeekableStream(magick_info) != MagickFalse))
    {
      char
        filename[MaxTextExtent];

      (void) CopyMagickString(filename,image->filename,MaxTextExtent);
      status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
      (void) CopyMagickString(image->filename,filename,MaxTextExtent);
      if (status != MagickFalse)
        {
          if (IsBlobSeekable(image) == MagickFalse)
            {
              /*
                A seekable stream is required by the encoder.
              */
              write_info->adjoin=MagickTrue;
              (void) CopyMagickString(write_info->filename,image->filename,
                MaxTextExtent);
              (void) AcquireUniqueFilename(image->filename);
              temporary=MagickTrue;
            }
          (void) CloseBlob(image);
        }
    }
  if (constitute_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&constitute_semaphore);
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetImageEncoder(magick_info) != (EncodeImageHandler *) NULL))
    {
      /*
        Call appropriate image writer based on image type.
      */
      thread_support=GetMagickThreadSupport(magick_info);
      if ((thread_support & EncoderThreadSupport) == 0)
        LockSemaphoreInfo(constitute_semaphore);
      status=GetImageEncoder(magick_info)(write_info,image);
      if ((thread_support & EncoderThreadSupport) == 0)
        UnlockSemaphoreInfo(constitute_semaphore);
    }
  else
    {
      delegate_info=GetDelegateInfo((char *) NULL,write_info->magick,
        &image->exception);
      if (delegate_info != (DelegateInfo *) NULL)
        {
          /*
            Process the image with delegate.
          */
          *write_info->filename='\0';
          if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
            LockSemaphoreInfo(constitute_semaphore);
          status=InvokeDelegate(write_info,image,(char *) NULL,
            write_info->magick,&image->exception);
          if (GetDelegateThreadSupport(delegate_info) == MagickFalse)
            UnlockSemaphoreInfo(constitute_semaphore);
          (void) CopyMagickString(image->filename,filename,MaxTextExtent);
        }
      else
        {
          sans_exception=AcquireExceptionInfo();
          magick_info=GetMagickInfo(write_info->magick,sans_exception);
          sans_exception=DestroyExceptionInfo(sans_exception);
          if ((write_info->affirm == MagickFalse) &&
              (magick_info == (const MagickInfo *) NULL))
            {
              (void) CopyMagickString(write_info->magick,image->magick,
                MaxTextExtent);
              magick_info=GetMagickInfo(write_info->magick,&image->exception);
            }
          if ((magick_info == (const MagickInfo *) NULL) ||
              (GetImageEncoder(magick_info) == (EncodeImageHandler *) NULL))
            {
              char
                extension[MaxTextExtent];

              GetPathComponent(image->filename,ExtensionPath,extension);
              if (*extension != '\0')
                magick_info=GetMagickInfo(extension,&image->exception);
              else
                magick_info=GetMagickInfo(image->magick,&image->exception);
              (void) CopyMagickString(image->filename,filename,MaxTextExtent);
            }
          if ((magick_info == (const MagickInfo *) NULL) ||
              (GetImageEncoder(magick_info) == (EncodeImageHandler *) NULL))
            {
              magick_info=GetMagickInfo(image->magick,&image->exception);
              if ((magick_info == (const MagickInfo *) NULL) ||
                  (GetImageEncoder(magick_info) == (EncodeImageHandler *) NULL))
                (void) ThrowMagickException(&image->exception,GetMagickModule(),
                  MissingDelegateError,"NoEncodeDelegateForThisImageFormat",
                  "'%s'",image->filename);
              else
                (void) ThrowMagickException(&image->exception,GetMagickModule(),
                  MissingDelegateWarning,"NoEncodeDelegateForThisImageFormat",
                  "'%s'",image->filename);
            }
          if ((magick_info != (const MagickInfo *) NULL) &&
              (GetImageEncoder(magick_info) != (EncodeImageHandler *) NULL))
            {
              /*
                Call appropriate image writer based on image type.
              */
              thread_support=GetMagickThreadSupport(magick_info);
              if ((thread_support & EncoderThreadSupport) == 0)
                LockSemaphoreInfo(constitute_semaphore);
              status=GetImageEncoder(magick_info)(write_info,image);
              if ((thread_support & EncoderThreadSupport) == 0)
                UnlockSemaphoreInfo(constitute_semaphore);
            }
        }
    }
  if (GetBlobError(image) != MagickFalse)
    ThrowFileException(&image->exception,FileOpenError,
      "AnErrorHasOccurredWritingToFile",image->filename);
  if (temporary == MagickTrue)
    {
      /*
        Copy temporary image file to permanent.
      */
      status=OpenBlob(write_info,image,ReadBinaryBlobMode,&image->exception);
      if (status != MagickFalse)
        {
          (void) RelinquishUniqueFileResource(write_info->filename);
          status=ImageToFile(image,write_info->filename,&image->exception);
        }
      (void) CloseBlob(image);
      (void) RelinquishUniqueFileResource(image->filename);
      (void) CopyMagickString(image->filename,write_info->filename,
        MaxTextExtent);
    }
  if ((LocaleCompare(write_info->magick,"info") != 0) &&
      (write_info->verbose != MagickFalse))
    (void) IdentifyImage(image,stderr,MagickFalse);
  write_info=DestroyImageInfo(write_info);
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

  BlobInfo
    *blob;

  ExceptionInfo
    *sans_exception;

  ImageInfo
    *write_info;

  MagickBooleanType
    proceed;

  MagickOffsetType
    i;

  MagickProgressMonitor
    progress_monitor;

  MagickSizeType
    number_images;

  MagickStatusType
    status;

  register Image
    *p;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  write_info=CloneImageInfo(image_info);
  images=GetFirstImageInList(images);
  blob=CloneBlobInfo(images->blob);  /* thread specific I/O handler */
  DestroyBlob(images);
  images->blob=blob;
  if (filename != (const char *) NULL)
    for (p=images; p != (Image *) NULL; p=GetNextImageInList(p))
      (void) CopyMagickString(p->filename,filename,MaxTextExtent);
  (void) CopyMagickString(write_info->filename,images->filename,MaxTextExtent);
  if (*write_info->magick == '\0')
    (void) CopyMagickString(write_info->magick,images->magick,MaxTextExtent);
  sans_exception=AcquireExceptionInfo();
  (void) SetImageInfo(write_info,(unsigned int) GetImageListLength(images),
    sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  p=images;
  for ( ; GetNextImageInList(p) != (Image *) NULL; p=GetNextImageInList(p))
    if (p->scene >= GetNextImageInList(p)->scene)
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
  /*
    Write images.
  */
  status=MagickTrue;
  progress_monitor=(MagickProgressMonitor) NULL;
  i=0;
  number_images=GetImageListLength(images);
  for (p=images; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    if (number_images != 1)
      progress_monitor=SetImageProgressMonitor(p,(MagickProgressMonitor) NULL,
        p->client_data);
    status&=WriteImage(write_info,p);
    GetImageException(p,exception);
    if (number_images != 1)
      (void) SetImageProgressMonitor(p,progress_monitor,p->client_data);
    if (write_info->adjoin != MagickFalse)
      break;
    if (number_images != 1)
      {
        proceed=SetImageProgress(p,WriteImageTag,i++,number_images);
        if (proceed == MagickFalse)
          break;
      }
  }
  write_info=DestroyImageInfo(write_info);
  return(status != 0 ? MagickTrue : MagickFalse);
}

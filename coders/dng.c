/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD   N   N   GGGG                              %
%                            D   D  NN  N  GS                                 %
%                            D   D  N N N  G  GG                              %
%                            D   D  N  NN  G   G                              %
%                            DDDD   N   N   GGGG                              %
%                                                                             %
%                                                                             %
%                  Read the Digital Negative Image Format                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1999                                   %
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
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/opencl.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"
#if defined(MAGICKCORE_RAW_R_DELEGATE)
#include <libraw.h>
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D N G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDNGImage() reads an binary file in the Digital Negative format and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadDNGImage method is:
%
%      Image *ReadDNGImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && defined(MAGICKCORE_OPENCL_SUPPORT)
static void InitializeDcrawOpenCL(ExceptionInfo *exception)
{
  MagickCLDevice
    *devices;

  size_t
    length;

  ssize_t
    i;

  (void) SetEnvironmentVariable("DCR_CL_PLATFORM",NULL);
  (void) SetEnvironmentVariable("DCR_CL_DEVICE",NULL);
  (void) SetEnvironmentVariable("DCR_CL_DISABLED",NULL);
  if (GetOpenCLEnabled() == MagickFalse)
    {
      (void) SetEnvironmentVariable("DCR_CL_DISABLED","1");
      return;
    }
  devices=GetOpenCLDevices(&length,exception);
  if (devices == (MagickCLDevice *) NULL)
    return;
  for (i=0; i < (ssize_t) length; i++)
  {
    const char
      *name;

    MagickCLDevice
      device;

    device=devices[i];
    if (GetOpenCLDeviceEnabled(device) == MagickFalse)
      continue;
    name=GetOpenCLDeviceVendorName(device);
    if (name != (const char *) NULL)
      (void) SetEnvironmentVariable("DCR_CL_PLATFORM",name);
    name=GetOpenCLDeviceName(device);
    if (name != (const char *) NULL)
      (void) SetEnvironmentVariable("DCR_CL_DEVICE",name);
    return;
  }
}
#else
static void InitializeDcrawOpenCL(ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(exception);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  (void) SetEnvironmentVariable("DCR_CL_DISABLED","1");
#endif
}
#endif

#if defined(MAGICKCORE_RAW_R_DELEGATE)
static void SetDNGProperties(Image *image,const libraw_data_t *raw_info,
  ExceptionInfo *exception)
{
  char
    timestamp[MagickTimeExtent];

  (void) SetImageProperty(image,"dng:make",raw_info->idata.make,exception);
  (void) SetImageProperty(image,"dng:camera.model.name",raw_info->idata.model,
    exception);
  (void) FormatMagickTime(raw_info->other.timestamp,sizeof(timestamp),
    timestamp);
  (void) SetImageProperty(image,"dng:create.date",timestamp,exception);
  (void) FormatImageProperty(image,"dng:iso.setting","%.0g",
    (double) raw_info->other.iso_speed);
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,18)
  (void) SetImageProperty(image,"dng:software",raw_info->idata.software,
    exception);
  if (*raw_info->shootinginfo.BodySerial != '\0')
    (void) SetImageProperty(image,"dng:serial.number",
      raw_info->shootinginfo.BodySerial,exception);
  (void) FormatImageProperty(image,"dng:exposure.time","1/%.0g",
    (double) MagickSafeReciprocal(raw_info->other.shutter));
  (void) FormatImageProperty(image,"dng:f.number","%0.1g",
    (double) raw_info->other.aperture);
  (void) FormatImageProperty(image,"dng:max.aperture.value","%0.1g",
    (double) raw_info->lens.EXIF_MaxAp);
  (void) FormatImageProperty(image,"dng:focal.length","%0.1g mm",
    (double) raw_info->other.focal_len);
  (void) FormatImageProperty(image,"dng:wb.rb.levels","%g %g %g %g",
    (double) raw_info->color.cam_mul[0],(double) raw_info->color.cam_mul[2],
    (double) raw_info->color.cam_mul[1],(double) raw_info->color.cam_mul[3]);
  (void) SetImageProperty(image,"dng:lens.type",
    raw_info->lens.makernotes.LensFeatures_suf,exception);
  (void) FormatImageProperty(image,"dng:lens","%0.1g-%0.1gmm f/%0.1g-%0.1g",
    (double) raw_info->lens.makernotes.MinFocal,
    (double) raw_info->lens.makernotes.MaxFocal,
    (double) raw_info->lens.makernotes.MaxAp4MinFocal,
    (double) raw_info->lens.makernotes.MaxAp4MaxFocal);
  (void) FormatImageProperty(image,"dng:lens.f.stops","%0.2f",
    (double) raw_info->lens.makernotes.LensFStops);
  (void) FormatImageProperty(image,"dng:min.focal.length","%0.1f mm",
    (double) raw_info->lens.makernotes.MinFocal);
  (void) FormatImageProperty(image,"dng:max.focal.length","%0.1g mm",
    (double) raw_info->lens.makernotes.MaxFocal);
  (void) FormatImageProperty(image,"dng:max.aperture.at.min.focal","%0.1g",
    (double) raw_info->lens.makernotes.MaxAp4MinFocal);
  (void) FormatImageProperty(image,"dng:max.aperture.at.max.focal","%0.1g",
    (double) raw_info->lens.makernotes.MaxAp4MaxFocal);
  (void) FormatImageProperty(image,"dng:focal.length.in.35mm.format","%d mm",
    raw_info->lens.FocalLengthIn35mmFormat);
#endif
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,20)
  (void) FormatImageProperty(image,"dng:gps.latitude",
    "%.0g deg %.0g' %.2g\" N",
    (double) raw_info->other.parsed_gps.latitude[0],
    (double) raw_info->other.parsed_gps.latitude[1],
    (double) raw_info->other.parsed_gps.latitude[2]);
  (void) FormatImageProperty(image,"dng:gps.longitude",
    "%.0g deg %.0g' %.2g\" W",
    (double) raw_info->other.parsed_gps.longitude[0],
    (double) raw_info->other.parsed_gps.longitude[1],
    (double) raw_info->other.parsed_gps.longitude[2]);
  (void) FormatImageProperty(image,"dng:gps.altitude","%.1g m",
    (double) raw_info->other.parsed_gps.altitude);
#endif
}
#endif

static Image *InvokeDNGDelegate(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  ExceptionInfo
    *sans_exception;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  /*
    Convert DNG to TIFF with delegate program.
  */
  (void) DestroyImageList(image);
  InitializeDcrawOpenCL(exception);
  image=AcquireImage(image_info,exception);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  status=InvokeDelegate(read_info,image,"dng:decode",(char *) NULL,exception);
  image=DestroyImage(image);
  if (status == MagickFalse)
    {
      read_info=DestroyImageInfo(read_info);
      return(image);
    }
  *read_info->magick='\0';
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,"%s.tif",
    read_info->unique);
  sans_exception=AcquireExceptionInfo();
  image=ReadImage(read_info,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (image != (Image *) NULL)
    (void) CopyMagickString(image->magick,read_info->magick,MagickPathExtent);
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  return(image);
}

#if defined(MAGICKCORE_RAW_R_DELEGATE)
static void SetLibRawParams(const ImageInfo *image_info,Image *image,
  libraw_data_t *raw_info)
{
  const char
    *option;

#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,20)
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,21)
  raw_info->rawparams.max_raw_memory_mb=8192;
#else
  raw_info->params.max_raw_memory_mb=8192;
#endif
  option=GetImageOption(image_info,"dng:max-raw-memory");
  if (option != (const char *) NULL)
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,21)
    raw_info->rawparams.max_raw_memory_mb=(unsigned int)
      StringToInteger(option);
#else
   raw_info->params.max_raw_memory_mb=(unsigned int) StringToInteger(option);
#endif
#endif
  raw_info->params.user_flip=0;
  raw_info->params.output_bps=16;
  raw_info->params.use_camera_wb=1;
  option=GetImageOption(image_info,"dng:use-camera-wb");
  if (option == (const char *) NULL)
    option=GetImageOption(image_info,"dng:use_camera_wb");
  if (option != (const char *) NULL)
    raw_info->params.use_camera_wb=IsStringTrue(option) != MagickFalse ? 1 : 0;
  option=GetImageOption(image_info,"dng:use-auto-wb");
  if (option == (const char *) NULL)
    option=GetImageOption(image_info,"dng:use_auto_wb");
  if (option != (const char *) NULL)
    raw_info->params.use_auto_wb=IsStringTrue(option) != MagickFalse ? 1 : 0;
  option=GetImageOption(image_info,"dng:no-auto-bright");
  if (option == (const char *) NULL)
    option=GetImageOption(image_info,"dng:no_auto_bright");
  if (option != (const char *) NULL)
    raw_info->params.no_auto_bright=IsStringTrue(option) != MagickFalse ? 1 : 0;
  option=GetImageOption(image_info,"dng:output-color");
  if (option == (const char *) NULL)
    option=GetImageOption(image_info,"dng:output_color");
  if (option != (const char *) NULL)
    {
      raw_info->params.output_color=StringToInteger(option);
      if (raw_info->params.output_color == 5)
        image->colorspace=XYZColorspace;
    }
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,21)
  option=GetImageOption(image_info,"dng:interpolation-quality");
  if (option != (const char *) NULL)
    {
      int
        value;

      value=StringToInteger(option);
      if (value == -1)
        raw_info->params.no_interpolation=1;
      else
        raw_info->params.user_qual=value;
    }
#endif
}

static void LibRawDataError(void *data,const char *magick_unused(file),
#if defined(MAGICK_LIBRAW_VERSION_TAIL) && MAGICK_LIBRAW_VERSION_TAIL == 202502
  const INT64 offset)
#else
  const int offset)
#endif
{
  magick_unreferenced(file);
  if (offset >= 0)
    {
      ExceptionInfo
        *exception;

      /*
        Value below zero is an EOF and an exception is raised instead.
      */
      exception=(ExceptionInfo *) data;
      (void) ThrowMagickException(exception,GetMagickModule(),
        CorruptImageWarning,"Data corrupted at","`%d'",(int) offset);
  }
}

static void ReadLibRawThumbnail(const ImageInfo *image_info,Image *image,
  libraw_data_t *raw_info,ExceptionInfo *exception)
{
  const char
    *option;

  int
    errcode;

  libraw_processed_image_t
    *thumbnail;

  option=GetImageOption(image_info,"dng:read-thumbnail");
  if (IsStringTrue(option) == MagickFalse)
    return;
  errcode=libraw_unpack_thumb(raw_info);
  if (errcode != LIBRAW_SUCCESS)
    return;
  thumbnail=libraw_dcraw_make_mem_thumb(raw_info,&errcode);
  if (errcode == LIBRAW_SUCCESS)
    {
      StringInfo
        *profile;

      if (thumbnail->type == LIBRAW_IMAGE_JPEG)
        SetImageProperty(image,"dng:thumbnail.type","jpeg",exception);
      else if (thumbnail->type == LIBRAW_IMAGE_BITMAP)
        {
          char
            value[15];

          SetImageProperty(image,"dng:thumbnail.type","bitmap",exception);
          (void) FormatLocaleString(value,sizeof(value),"%hu",thumbnail->bits);
          SetImageProperty(image,"dng:thumbnail.bits",value,exception);
          (void) FormatLocaleString(value,sizeof(value),"%hu",
            thumbnail->colors);
          SetImageProperty(image,"dng:thumbnail.colors",value,exception);
          (void) FormatLocaleString(value,sizeof(value),"%hux%hu",
            thumbnail->width,thumbnail->height);
          SetImageProperty(image,"dng:thumbnail.geometry",value,exception);
        }
      profile=BlobToProfileStringInfo("dng:thumbnail",thumbnail->data,
        thumbnail->data_size,exception);
      (void) SetImageProfilePrivate(image,profile,exception);
    }
  if (thumbnail != (libraw_processed_image_t *) NULL)
    libraw_dcraw_clear_mem(thumbnail);
}

static OrientationType LibRawFlipToOrientation(int flip)
{
  switch(flip)
  {
    case 5:
    {
      return(LeftBottomOrientation);
    }
    case 8:
    {
      return(LeftTopOrientation);
    }
    default:
    {
      return((OrientationType) flip);
    }
  }
}

#endif

static Image *ReadDNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

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
  (void) CloseBlob(image);
  if (LocaleCompare(image_info->magick,"DCRAW") == 0)
    return(InvokeDNGDelegate(image_info,image,exception));
#if defined(MAGICKCORE_RAW_R_DELEGATE)
  {
    int
      errcode;

    libraw_data_t
      *raw_info;

    libraw_processed_image_t
      *raw_image;

    ssize_t
      y;

    StringInfo
      *profile;

    unsigned int
      flags;

    unsigned short
      *p;

    errcode=LIBRAW_UNSPECIFIED_ERROR;
    flags=LIBRAW_OPIONS_NO_DATAERR_CALLBACK;
#if LIBRAW_SHLIB_CURRENT < 23
    flags|=LIBRAW_OPIONS_NO_MEMERR_CALLBACK;
#endif
    raw_info=libraw_init(flags);
    if (raw_info == (libraw_data_t *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          libraw_strerror(errcode),"`%s'",image->filename);
        libraw_close(raw_info);
        return(DestroyImageList(image));
      }
    libraw_set_dataerror_handler(raw_info,LibRawDataError,exception);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
    {
      wchar_t
        *path;

      path=create_wchar_path(image->filename);
      if (path != (wchar_t *) NULL)
        {
          errcode=libraw_open_wfile(raw_info,path);
          path=(wchar_t *) RelinquishMagickMemory(path);
        }
    }
#else
    errcode=libraw_open_file(raw_info,image->filename);
#endif
    if (errcode != LIBRAW_SUCCESS)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          libraw_strerror(errcode),"`%s'",image->filename);
        libraw_close(raw_info);
        return(DestroyImageList(image));
      }
    image->columns=raw_info->sizes.width;
    image->rows=raw_info->sizes.height;
    image->page.width=raw_info->sizes.raw_width;
    image->page.height=raw_info->sizes.raw_height;
    image->page.x=raw_info->sizes.left_margin;
    image->page.y=raw_info->sizes.top_margin;
    image->orientation=LibRawFlipToOrientation(raw_info->sizes.flip);
    ReadLibRawThumbnail(image_info,image,raw_info,exception);
    SetDNGProperties(image,raw_info,exception);
    if (image_info->ping != MagickFalse)
      {
        libraw_close(raw_info);
        return(image);
      }
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      {
        libraw_close(raw_info);
        return(image);
      }
    errcode=libraw_unpack(raw_info);
    if (errcode != LIBRAW_SUCCESS)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          libraw_strerror(errcode),"`%s'",image->filename);
        libraw_close(raw_info);
        return(DestroyImageList(image));
      }
    SetLibRawParams(image_info,image,raw_info);
    errcode=libraw_dcraw_process(raw_info);
    if (errcode != LIBRAW_SUCCESS)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          libraw_strerror(errcode),"`%s'",image->filename);
        libraw_close(raw_info);
        return(DestroyImageList(image));
      }
    raw_image=libraw_dcraw_make_mem_image(raw_info,&errcode);
    if ((errcode != LIBRAW_SUCCESS) ||
        (raw_image == (libraw_processed_image_t *) NULL) ||
        (raw_image->type != LIBRAW_IMAGE_BITMAP) || (raw_image->bits != 16) ||
        (raw_image->colors < 1) || (raw_image->colors > 4))
      {
        if (raw_image != (libraw_processed_image_t *) NULL)
          libraw_dcraw_clear_mem(raw_image);
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          libraw_strerror(errcode),"`%s'",image->filename);
        libraw_close(raw_info);
        return(DestroyImageList(image));
      }
    if (raw_image->colors < 3)
      {
        image->colorspace=GRAYColorspace;
        image->type=raw_image->colors == 1 ? GrayscaleType : GrayscaleAlphaType;
      }
    image->columns=raw_image->width;
    image->rows=raw_image->height;
    image->depth=raw_image->bits;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      {
        libraw_dcraw_clear_mem(raw_image);
        libraw_close(raw_info);
        return(DestroyImageList(image));
      }
    p=(unsigned short *) raw_image->data;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      Quantum
        *q;

      ssize_t
        x;

      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelRed(image,ScaleShortToQuantum(*p++),q);
        if (raw_image->colors > 2)
          {
            SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
            SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
          }
        if ((raw_image->colors) == 2 || (raw_image->colors > 3))
          SetPixelAlpha(image,ScaleShortToQuantum(*p++),q);
        q+=(ptrdiff_t) GetPixelChannels(image);
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
    libraw_dcraw_clear_mem(raw_image);
    /*
      Set DNG image metadata.
    */
    if (raw_info->color.profile != NULL)
      {
        profile=BlobToProfileStringInfo("icc",raw_info->color.profile,
          raw_info->color.profile_length,exception);
        (void) SetImageProfilePrivate(image,profile,exception);
      }
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,18)
    if (raw_info->idata.xmpdata != NULL)
      {
        profile=BlobToProfileStringInfo("xmp",raw_info->idata.xmpdata,
          raw_info->idata.xmplen,exception);
        (void) SetImageProfilePrivate(image,profile,exception);
      }
#endif
    libraw_close(raw_info);
    return(image);
  }
#else
  return(InvokeDNGDelegate(image_info,image,exception));
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D N G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDNGImage() adds attributes for the DNG image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDNGImage method is:
%
%      size_t RegisterDNGImage(void)
%
*/
static inline void RegisterDNGMagickInfo(const char *name,
  const char *description,const char *version)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("DNG",name,description);
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
}

ModuleExport size_t RegisterDNGImage(void)
{
  char
    version[MagickPathExtent];

  *version='\0';
#if defined(MAGICKCORE_RAW_R_DELEGATE)
  (void) CopyMagickString(version,libraw_version(),MagickPathExtent);
#endif
  RegisterDNGMagickInfo("3FR","Hasselblad CFV/H3D39II Raw Format",version);
  RegisterDNGMagickInfo("ARW","Sony Alpha Raw Format",version);
  RegisterDNGMagickInfo("CR2","Canon Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("CR3","Canon Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("CRW","Canon Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("DCR","Kodak Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("DCRAW","Raw Photo Decoder (dcraw)",version);
  RegisterDNGMagickInfo("DNG","Digital Negative Raw Format",version);
  RegisterDNGMagickInfo("ERF","Epson Raw Format",version);
  RegisterDNGMagickInfo("FFF","Hasselblad CFV/H3D39II Raw Format",version);
  RegisterDNGMagickInfo("IIQ","Phase One Raw Format",version);
  RegisterDNGMagickInfo("K25","Kodak Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("KDC","Kodak Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("MDC","Minolta Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("MEF","Mamiya Raw Format",version);
  RegisterDNGMagickInfo("MOS","Aptus Leaf Raw Format",version);
  RegisterDNGMagickInfo("MRW","Sony (Minolta) Raw Format",version);
  RegisterDNGMagickInfo("NEF","Nikon Digital SLR Camera Raw Format",version);
  RegisterDNGMagickInfo("NRW","Nikon Digital SLR Camera Raw Format",version);
  RegisterDNGMagickInfo("ORF","Olympus Digital Camera Raw Format",version);
  RegisterDNGMagickInfo("PEF","Pentax Electronic Raw Format",version);
  RegisterDNGMagickInfo("RAF","Fuji CCD-RAW Graphic Raw Format",version);
  RegisterDNGMagickInfo("RAW","Raw",version);
  RegisterDNGMagickInfo("RMF","Raw Media Format",version);
  RegisterDNGMagickInfo("RW2","Panasonic Lumix Raw Format",version);
  RegisterDNGMagickInfo("RWL","Leica Raw Format",version);
  RegisterDNGMagickInfo("SR2","Sony Raw Format 2",version);
  RegisterDNGMagickInfo("SRF","Sony Raw Format",version);
  RegisterDNGMagickInfo("SRW","Samsung Raw Format",version);
  RegisterDNGMagickInfo("STI","Sinar CaptureShop Raw Format",version);
  RegisterDNGMagickInfo("X3F","Sigma Camera RAW Format",version);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D N G I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDNGImage() removes format registrations made by the
%  BIM module from the list of supported formats.
%
%  The format of the UnregisterBIMImage method is:
%
%      UnregisterDNGImage(void)
%
*/
ModuleExport void UnregisterDNGImage(void)
{
  (void) UnregisterMagickInfo("X3F");
  (void) UnregisterMagickInfo("STI");
  (void) UnregisterMagickInfo("SRW");
  (void) UnregisterMagickInfo("SRF");
  (void) UnregisterMagickInfo("SR2");
  (void) UnregisterMagickInfo("RWL");
  (void) UnregisterMagickInfo("RW2");
  (void) UnregisterMagickInfo("RMF");
  (void) UnregisterMagickInfo("RAF");
  (void) UnregisterMagickInfo("PEF");
  (void) UnregisterMagickInfo("ORF");
  (void) UnregisterMagickInfo("NRW");
  (void) UnregisterMagickInfo("NEF");
  (void) UnregisterMagickInfo("MRW");
  (void) UnregisterMagickInfo("MOS");
  (void) UnregisterMagickInfo("MEF");
  (void) UnregisterMagickInfo("MDC");
  (void) UnregisterMagickInfo("KDC");
  (void) UnregisterMagickInfo("K25");
  (void) UnregisterMagickInfo("IIQ");
  (void) UnregisterMagickInfo("FFF");
  (void) UnregisterMagickInfo("ERF");
  (void) UnregisterMagickInfo("DNG");
  (void) UnregisterMagickInfo("DCRAW");
  (void) UnregisterMagickInfo("DCR");
  (void) UnregisterMagickInfo("CRW");
  (void) UnregisterMagickInfo("CR3");
  (void) UnregisterMagickInfo("CR2");
  (void) UnregisterMagickInfo("ARW");
  (void) UnregisterMagickInfo("3FR");
}

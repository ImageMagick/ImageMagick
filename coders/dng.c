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
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
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
  MagickBooleanType
    opencl_disabled;

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
#if !defined(MAGICKCORE_RAW_R_DELEGATE)
static void InitializeDcrawOpenCL(ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(exception);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  (void) SetEnvironmentVariable("DCR_CL_DISABLED","1");
#endif
}
#endif
#endif

#if defined(MAGICKCORE_RAW_R_DELEGATE)
static void SetDNGProperties(Image *image,const libraw_data_t *raw_info,
  ExceptionInfo *exception)
{
  char
    property[MagickPathExtent],
    timestamp[MagickPathExtent];

  (void) SetImageProperty(image,"dng:make",raw_info->idata.make,exception);
  (void) SetImageProperty(image,"dng:camera.model.name",raw_info->idata.model,
    exception);
  (void) FormatMagickTime(raw_info->other.timestamp,MagickPathExtent,timestamp);
  (void) SetImageProperty(image,"dng:create.date",timestamp,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f",
    raw_info->other.iso_speed);
  (void) SetImageProperty(image,"dng:iso.setting",property,exception);
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,18)
  (void) SetImageProperty(image,"dng:software",raw_info->idata.software,
    exception);
  if (*raw_info->shootinginfo.BodySerial != '\0')
    (void) SetImageProperty(image,"dng:serial.number",
      raw_info->shootinginfo.BodySerial,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.2f",
    raw_info->other.FlashEC);
  (void) SetImageProperty(image,"dng:flash.exposure.compensation",property,
    exception);
  (void) FormatLocaleString(property,MagickPathExtent,"1/%0.1f",
    1.0/raw_info->other.shutter);
  (void) SetImageProperty(image,"dng:exposure.time",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f",
    raw_info->other.aperture);
  (void) SetImageProperty(image,"dng:f.number",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f",
    raw_info->lens.EXIF_MaxAp);
  (void) SetImageProperty(image,"dng:max.aperture.value",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f",
    raw_info->other.focal_len);
  (void) SetImageProperty(image,"dng:focal.length",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%f %f %f %f",
    raw_info->color.cam_mul[0],raw_info->color.cam_mul[2],
    raw_info->color.cam_mul[1],raw_info->color.cam_mul[3]);
  (void) SetImageProperty(image,"dng:wb.rb.levels",property,exception);
  (void) SetImageProperty(image,"dng:lens.type",
    raw_info->lens.makernotes.LensFeatures_suf,exception);
  (void) FormatLocaleString(property,MagickPathExtent,
    "%0.1f-%0.1fmm f/%0.1f-%0.1f",raw_info->lens.makernotes.MinFocal,
    raw_info->lens.makernotes.MaxFocal,
    raw_info->lens.makernotes.MaxAp4MinFocal,
    raw_info->lens.makernotes.MaxAp4MaxFocal);
  (void) SetImageProperty(image,"dng:lens",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.2f",
    raw_info->lens.makernotes.LensFStops);
  (void) SetImageProperty(image,"dng:lens.f.stops",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f mm",
    raw_info->lens.makernotes.MinFocal);
  (void) SetImageProperty(image,"dng:min.focal.length",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f mm",
    raw_info->lens.makernotes.MaxFocal);
  (void) SetImageProperty(image,"dng:max.focal.length",property,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f",
    raw_info->lens.makernotes.MaxAp4MinFocal);
  (void) SetImageProperty(image,"dng:max.aperture.at.min.focal",property,
    exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%0.1f",
    raw_info->lens.makernotes.MaxAp4MaxFocal);
  (void) SetImageProperty(image,"dng:max.aperture.at.max.focal",property,
    exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%d mm",
    raw_info->lens.FocalLengthIn35mmFormat);
  (void) SetImageProperty(image,"dng:focal.length.in.35mm.format",property,
    exception);
#endif
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
  (void) CloseBlob(image);
#if defined(MAGICKCORE_RAW_R_DELEGATE)
  {
    int
      errcode;

    libraw_data_t
      *raw_info;

    libraw_processed_image_t
      *raw_image;

    register ssize_t
      y;

    StringInfo
      *profile;

    unsigned short
      *p;

    errcode=0;
    raw_info=libraw_init(0);
    if (raw_info == (libraw_data_t *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          libraw_strerror(errcode),"`%s'",image->filename);
        libraw_close(raw_info);
        return(DestroyImageList(image));
      }
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && defined(_MSC_VER) && (_MSC_VER > 1310)
    {
      wchar_t
        fileName[MagickPathExtent];

      MultiByteToWideChar(CP_UTF8,0,image->filename,-1,fileName,
        MagickPathExtent);
      errcode=libraw_open_wfile(raw_info,fileName);
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
    raw_info->params.output_bps=16;
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
        (raw_image->colors < 3) || (raw_image->colors > 4))
      {
        if (raw_image != (libraw_processed_image_t *) NULL)
          libraw_dcraw_clear_mem(raw_image);
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          libraw_strerror(errcode),"`%s'",image->filename);
        libraw_close(raw_info);
        return(DestroyImageList(image));
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
      register Quantum
        *q;

      register ssize_t
        x;

      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelRed(image,ScaleShortToQuantum(*p++),q);
        SetPixelGreen(image,ScaleShortToQuantum(*p++),q);
        SetPixelBlue(image,ScaleShortToQuantum(*p++),q);
        if (raw_image->colors > 3)
          SetPixelAlpha(image,ScaleShortToQuantum(*p++),q);
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
    libraw_dcraw_clear_mem(raw_image);
    /*
      Set DNG image metadata.
    */
    if (raw_info->color.profile != NULL)
      {
        profile=BlobToStringInfo(raw_info->color.profile,
          raw_info->color.profile_length);
        if (profile != (StringInfo *) NULL)
          {
            SetImageProfile(image,"ICC",profile,exception);
            profile=DestroyStringInfo(profile);
          }
      }
#if LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,18)
    if (raw_info->idata.xmpdata != NULL)
      {
        profile=BlobToStringInfo(raw_info->idata.xmpdata,
          raw_info->idata.xmplen);
        if (profile != (StringInfo *) NULL)
          {
            SetImageProfile(image,"XMP",profile,exception);
            profile=DestroyStringInfo(profile);
          }
      }
#endif
    SetDNGProperties(image,raw_info,exception);
    libraw_close(raw_info);
    return(image);
  }
#else
  {
    ExceptionInfo
      *sans_exception;

    ImageInfo
      *read_info;

    /*
      Convert DNG to PPM with delegate.
    */
    (void) DestroyImageList(image);
    InitializeDcrawOpenCL(exception);
    image=AcquireImage(image_info,exception);
    read_info=CloneImageInfo(image_info);
    SetImageInfoBlob(read_info,(void *) NULL,0);
    (void) InvokeDelegate(read_info,image,"dng:decode",(char *) NULL,exception);
    image=DestroyImage(image);
    (void) FormatLocaleString(read_info->filename,MagickPathExtent,"%s.png",
      read_info->unique);
    sans_exception=AcquireExceptionInfo();
    image=ReadImage(read_info,sans_exception);
    sans_exception=DestroyExceptionInfo(sans_exception);
    if (image == (Image *) NULL)
      {
        (void) FormatLocaleString(read_info->filename,MagickPathExtent,"%s.ppm",
          read_info->unique);
        image=ReadImage(read_info,exception);
      }
    (void) RelinquishUniqueFileResource(read_info->filename);
    if (image != (Image *) NULL)
      {
        char
          filename[MagickPathExtent],
          *xml;

        ExceptionInfo
          *sans;

        (void) CopyMagickString(image->magick,read_info->magick,
          MagickPathExtent);
        (void) FormatLocaleString(filename,MagickPathExtent,"%s.ufraw",
          read_info->unique);
        sans=AcquireExceptionInfo();
        xml=FileToString(filename,MagickPathExtent,sans);
        (void) RelinquishUniqueFileResource(filename);
        if (xml != (char *) NULL)
          {
            XMLTreeInfo
              *ufraw;

            /*
              Inject.
            */
            ufraw=NewXMLTree(xml,sans);
            if (ufraw != (XMLTreeInfo *) NULL)
              {
                char
                  *content,
                  property[MagickPathExtent];

                const char
                  *tag;

                XMLTreeInfo
                  *next;

                if (image->properties == (void *) NULL)
                  ((Image *) image)->properties=NewSplayTree(
                    CompareSplayTreeString,RelinquishMagickMemory,
                    RelinquishMagickMemory);
                next=GetXMLTreeChild(ufraw,(const char *) NULL);
                while (next != (XMLTreeInfo *) NULL)
                {
                  tag=GetXMLTreeTag(next);
                  if (tag == (char *) NULL)
                    tag="unknown";
                  (void) FormatLocaleString(property,MagickPathExtent,"dng:%s",
                    tag);
                  content=ConstantString(GetXMLTreeContent(next));
                  StripString(content);
                  if ((LocaleCompare(tag,"log") != 0) &&
                      (LocaleCompare(tag,"InputFilename") != 0) &&
                      (LocaleCompare(tag,"OutputFilename") != 0) &&
                      (LocaleCompare(tag,"OutputType") != 0) &&
                      (strlen(content) != 0))
                    (void) AddValueToSplayTree((SplayTreeInfo *)
                      ((Image *) image)->properties,ConstantString(property),
                      content);
                  next=GetXMLTreeSibling(next);
                }
                ufraw=DestroyXMLTree(ufraw);
              }
            xml=DestroyString(xml);
          }
        sans=DestroyExceptionInfo(sans);
      }
    read_info=DestroyImageInfo(read_info);
    return(image);
  }
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
ModuleExport size_t RegisterDNGImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("DNG","3FR","Hasselblad CFV/H3D39II");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","ARW","Sony Alpha Raw Image Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","DNG","Digital Negative");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","CR2","Canon Digital Camera Raw Image Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","CRW","Canon Digital Camera Raw Image Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","DCR","Kodak Digital Camera Raw Image File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","ERF","Epson RAW Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","IIQ","Phase One Raw Image Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","KDC","Kodak Digital Camera Raw Image Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","K25","Kodak Digital Camera Raw Image Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","MEF","Mamiya Raw Image File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","MRW","Sony (Minolta) Raw Image File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","NEF",
    "Nikon Digital SLR Camera Raw Image File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","NRW",
    "Nikon Digital SLR Camera Raw Image File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","ORF","Olympus Digital Camera Raw Image File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","PEF","Pentax Electronic File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","RAF","Fuji CCD-RAW Graphic File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","RAW","Raw");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","RMF","Raw Media Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","RW2","Panasonic Lumix Raw Image");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","SRF","Sony Raw Format");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","SR2","Sony Raw Format 2");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DNG","X3F","Sigma Camera RAW Picture File");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
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
  (void) UnregisterMagickInfo("SR2");
  (void) UnregisterMagickInfo("SRF");
  (void) UnregisterMagickInfo("RW2");
  (void) UnregisterMagickInfo("RMF");
  (void) UnregisterMagickInfo("RAF");
  (void) UnregisterMagickInfo("PEF");
  (void) UnregisterMagickInfo("ORF");
  (void) UnregisterMagickInfo("NRW");
  (void) UnregisterMagickInfo("NEF");
  (void) UnregisterMagickInfo("MRW");
  (void) UnregisterMagickInfo("MEF");
  (void) UnregisterMagickInfo("K25");
  (void) UnregisterMagickInfo("KDC");
  (void) UnregisterMagickInfo("IIQ");
  (void) UnregisterMagickInfo("ERF");
  (void) UnregisterMagickInfo("DCR");
  (void) UnregisterMagickInfo("CRW");
  (void) UnregisterMagickInfo("CR2");
  (void) UnregisterMagickInfo("DNG");
  (void) UnregisterMagickInfo("ARW");
  (void) UnregisterMagickInfo("3FR");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                       Read/Write UHDR Image Format                          %
%                                                                             %
%                              Software Design                                %
%                                    A S                                      %
%                                 Jan 2024                                    %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/module.h"
#include "MagickCore/option.h"
#include "MagickCore/profile.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/shear.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/transform.h"
#if defined(MAGICKCORE_UHDR_DELEGATE)
#include "ultrahdr_api.h"
#include "uhdr.h"
#endif

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_UHDR_DELEGATE)
static MagickBooleanType
  WriteUHDRImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s U H D R                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsUHDR() returns MagickTrue if the input stream is a valid ultra hdr file,
%  MagickFalse otherwise
%
%  The format of the IsUHDR  method is:
%
%      MagickBooleanType IsUHDR(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType IsUHDR(const unsigned char *magick,
  const size_t length)
{
#if defined(MAGICKCORE_UHDR_DELEGATE)
  if (is_uhdr_image((void *) magick,(int) length))
    return(MagickTrue);
#else
  magick_unreferenced(magick);
  magick_unreferenced(length);
#endif
  return(MagickFalse);
}

#if defined(MAGICKCORE_UHDR_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d U H D R I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadUHDRImage retrieves an image via a file descriptor, decodes the image,
%  and returns it. It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadUHDRImage method is:
%
%      Image *ReadUHDRImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static uhdr_color_transfer_t map_ct_to_uhdr_ct(const char *input_ct)
{
  if (!strcmp(input_ct, "hlg"))
    return UHDR_CT_HLG;
  if (!strcmp(input_ct, "pq"))
    return UHDR_CT_PQ;
  if (!strcmp(input_ct, "linear"))
    return UHDR_CT_LINEAR;
  if (!strcmp(input_ct, "srgb"))
    return UHDR_CT_SRGB;
  else
    return UHDR_CT_UNSPECIFIED;
}

static Image *ReadUHDRImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define SetHDRGMProperty(name,value) \
  (void) FormatLocaleString(buffer,sizeof(buffer),"%f",(value)); \
  (void) SetImageProperty(image,"hdrgm:" name,buffer,exception)
#define SetHDRGMPropertyInt(name,value) \
  (void) FormatLocaleString(buffer,sizeof(buffer),"%d",(value)); \
  (void) SetImageProperty(image,"hdrgm:" name,buffer,exception)
#define SetHDRGMPropertySize(name,value) \
  (void) FormatLocaleString(buffer,sizeof(buffer),"%.17g",(double) (value)); \
  (void) SetImageProperty(image,"hdrgm:" name,buffer,exception)
#define SetHDRGMProperty3(name,value,value1,value2) \
  (void) FormatLocaleString(buffer,sizeof(buffer),"%f,%f,%f", \
     (value),(value1),(value2)); \
  (void) SetImageProperty(image,"hdrgm:" name,buffer,exception)
  
  Image
    *image;

  MagickBooleanType
    status;

  image = AcquireImage(image_info, exception);
  status = OpenBlob(image_info, image, ReadBinaryBlobMode, exception);
  if (status == MagickFalse)
    return DestroyImageList(image);

  uhdr_compressed_image_t
    img;

  img.data = GetBlobStreamData(image);
  img.data_sz = GetBlobSize(image);
  img.capacity = GetBlobSize(image);
  img.cg = UHDR_CG_UNSPECIFIED;
  img.ct = UHDR_CT_UNSPECIFIED;
  img.range = UHDR_CR_UNSPECIFIED;

  uhdr_codec_private_t
    *handle = uhdr_create_decoder();

  const char *option = GetImageOption(image_info, "uhdr:output-color-transfer");
  uhdr_color_transfer_t decoded_img_ct =
      (option != (const char *)NULL) ? map_ct_to_uhdr_ct(option) : UHDR_CT_SRGB;

  const char *profile_skip = GetImageOption(image_info, "profile:skip");

  MagickBooleanType
    skip_app_profiles = IsOptionMember("APP",profile_skip),
    skip_exif_profile = IsOptionMember("EXIF",profile_skip),
    skip_gainmap_profile = IsOptionMember("HDRGM",profile_skip),
    skip_icc_profile = IsOptionMember("ICC",profile_skip);

  if (skip_app_profiles != MagickFalse)
    {
      skip_exif_profile=MagickTrue;
      skip_gainmap_profile=MagickTrue;
    }

  uhdr_img_fmt_t
    decoded_img_fmt = UHDR_CT_SRGB;

  if (decoded_img_ct == UHDR_CT_LINEAR)
    decoded_img_fmt = UHDR_IMG_FMT_64bppRGBAHalfFloat;
  else if (decoded_img_ct == UHDR_CT_HLG || decoded_img_ct == UHDR_CT_PQ)
    decoded_img_fmt = UHDR_IMG_FMT_32bppRGBA1010102;
  else if (decoded_img_ct == UHDR_CT_SRGB)
    decoded_img_fmt = UHDR_IMG_FMT_32bppRGBA8888;
  else
    decoded_img_fmt = UHDR_IMG_FMT_UNSPECIFIED;

#define CHECK_IF_ERR(x)                                                                       \
  {                                                                                           \
    uhdr_error_info_t retval = (x);                                                           \
    if (retval.error_code != UHDR_CODEC_OK)                                                   \
    {                                                                                         \
      (void)ThrowMagickException(exception, GetMagickModule(), CoderError,                    \
                                 retval.has_detail ? retval.detail : "unknown error", "`%s'", \
                                 image->filename);                                            \
      uhdr_release_decoder(handle);                                                           \
      CloseBlob(image);                                                                       \
      return DestroyImageList(image);                                                         \
    }                                                                                         \
  }

  CHECK_IF_ERR(uhdr_dec_set_image(handle, &img))
  CHECK_IF_ERR(uhdr_dec_set_out_color_transfer(handle, decoded_img_ct))
  CHECK_IF_ERR(uhdr_dec_set_out_img_format(handle, decoded_img_fmt))
  CHECK_IF_ERR(uhdr_dec_probe(handle))

  image->columns = uhdr_dec_get_image_width(handle);
  image->rows = uhdr_dec_get_image_height(handle);

  if (image_info->ping != MagickFalse)
    {
      uhdr_release_decoder(handle);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }

  CHECK_IF_ERR(uhdr_decode(handle))

  uhdr_raw_image_t
    *dst = uhdr_get_decoded_image(handle);

  /*
    Preserve compressed gain‑map + metadata for transcoding.
  */
  {
    uhdr_mem_block_t
      *gainmap_image = uhdr_dec_get_gainmap_image(handle);
  
    uhdr_gainmap_metadata_t
      *gainmap_info = uhdr_dec_get_gainmap_metadata(handle);
  
    if ((skip_gainmap_profile == MagickFalse) &&
        (gainmap_image != (uhdr_mem_block_t *) NULL) &&
        (gainmap_info != (uhdr_gainmap_metadata_t *) NULL))
    {
      char
        buffer[MagickPathExtent];

      /*
        Set gainmap as a binary profile.
      */
      StringInfo *gainmap_profile = BlobToProfileStringInfo("hdrgm",
        gainmap_image->data,gainmap_image->data_sz,exception);
      (void) SetImageProfilePrivate(image,gainmap_profile,exception);
  
      /*
        Set metadata as properties.
      */
      SetHDRGMProperty3("GainMapMax",
        gainmap_info->max_content_boost[0],gainmap_info->max_content_boost[1],
        gainmap_info->max_content_boost[2]);
      SetHDRGMProperty3("GainMapMin",
        gainmap_info->min_content_boost[0],gainmap_info->min_content_boost[1],
        gainmap_info->min_content_boost[2]);
      SetHDRGMProperty3("Gamma",gainmap_info->gamma[0],
        gainmap_info->gamma[1],gainmap_info->gamma[2]);
      SetHDRGMProperty3("OffsetSDR",gainmap_info->offset_sdr[0],
        gainmap_info->offset_sdr[1],gainmap_info->offset_sdr[2]);
      SetHDRGMProperty3("OffsetHDR",gainmap_info->offset_hdr[0],
        gainmap_info->offset_hdr[1],gainmap_info->offset_hdr[2]);
      SetHDRGMProperty("HDRCapacityMin",gainmap_info->hdr_capacity_min);
      SetHDRGMProperty("HDRCapacityMax",gainmap_info->hdr_capacity_max);
      SetHDRGMPropertyInt("UseBaseColorGrade",gainmap_info->use_base_cg);
      SetHDRGMPropertySize("BaseWidth",image->columns);
      SetHDRGMPropertySize("BaseHeight",image->rows);
    }
  }

  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      uhdr_release_decoder(handle);
      CloseBlob(image);
      return(DestroyImageList(image));
    }

#undef CHECK_IF_ERR

  uhdr_mem_block_t *exif = uhdr_dec_get_exif(handle);
  if ((skip_exif_profile == MagickFalse) && (exif != NULL))
  {
    StringInfo *exif_data = BlobToProfileStringInfo("exif",exif->data,
      exif->data_sz,exception);
    (void) SetImageProfilePrivate(image,exif_data,exception);
  }

  uhdr_mem_block_t *icc = uhdr_dec_get_icc(handle);
  if ((skip_icc_profile == MagickFalse) && (icc != NULL) &&
      (icc->data != NULL) && (icc->data_sz != 0))
  {
    const unsigned char
      *icc_data_start = (const unsigned char *) icc->data;

    size_t
      icc_data_size = icc->data_sz;

    /*
      libultrahdr returns JPEG APP2 ICC chunk data. ImageMagick stores the
      raw ICC payload and adds the APP2 wrapper itself when writing JPEG.
    */
    if ((icc_data_size > 14) &&
        (memcmp(icc_data_start,"ICC_PROFILE\0",12) == 0))
      {
        icc_data_start+=14;
        icc_data_size-=14;
      }
    StringInfo *icc_data = BlobToProfileStringInfo("icc",icc_data_start,
      icc_data_size,exception);
    (void) SetImageProfilePrivate(image,icc_data,exception);
  }

  if (decoded_img_ct == UHDR_CT_LINEAR)
    {
      SetImageColorspace(image,RGBColorspace,exception);
      image->gamma=1.0;
    }
  else if ((decoded_img_ct == UHDR_CT_SRGB) && (dst->cg == UHDR_CG_DISPLAY_P3))
    SetImageColorspace(image,DisplayP3Colorspace,exception);
  else if (decoded_img_ct == UHDR_CT_SRGB)
    SetImageColorspace(image,sRGBColorspace,exception);
  else
    SetImageColorspace(image,RGBColorspace,exception);

  image->compression = JPEGCompression;
  if (decoded_img_fmt == UHDR_IMG_FMT_32bppRGBA8888)
    image->depth = 8;
  else if (decoded_img_fmt == UHDR_IMG_FMT_32bppRGBA1010102)
    image->depth = 10;
  else if (decoded_img_fmt == UHDR_IMG_FMT_64bppRGBAHalfFloat)
    image->depth = 16;
  switch (dst->cg)
  {
  case UHDR_CG_BT_709:
    image->chromaticity.red_primary.x = 0.64;
    image->chromaticity.red_primary.y = 0.33;
    image->chromaticity.green_primary.x = 0.30;
    image->chromaticity.green_primary.y = 0.60;
    image->chromaticity.blue_primary.x = 0.15;
    image->chromaticity.blue_primary.y = 0.06;
    image->chromaticity.white_point.x = 0.3127;
    image->chromaticity.white_point.y = 0.3290;
    break;

  case UHDR_CG_DISPLAY_P3:
    image->chromaticity.red_primary.x = 0.680;
    image->chromaticity.red_primary.y = 0.320;
    image->chromaticity.green_primary.x = 0.265;
    image->chromaticity.green_primary.y = 0.690;
    image->chromaticity.blue_primary.x = 0.150;
    image->chromaticity.blue_primary.y = 0.060;
    image->chromaticity.white_point.x = 0.3127;
    image->chromaticity.white_point.y = 0.3290;
    break;

  case UHDR_CG_BT_2100:
    image->chromaticity.red_primary.x = 0.708;
    image->chromaticity.red_primary.y = 0.292;
    image->chromaticity.green_primary.x = 0.170;
    image->chromaticity.green_primary.y = 0.797;
    image->chromaticity.blue_primary.x = 0.131;
    image->chromaticity.blue_primary.y = 0.046;
    image->chromaticity.white_point.x = 0.3127;
    image->chromaticity.white_point.y = 0.3290;
    break;

  case UHDR_CG_UNSPECIFIED:
    break;
  }

  for (ssize_t y = 0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *q;

    q = QueueAuthenticPixels(image, 0, y, image->columns, 1, exception);
    if (q == (Quantum *) NULL)
    {
      status = MagickFalse;
      break;
    }
    if (decoded_img_fmt == UHDR_IMG_FMT_64bppRGBAHalfFloat)
    {
      uint16_t *row =
          (uint16_t *)dst->planes[UHDR_PLANE_PACKED] + (y * dst->stride[UHDR_PLANE_PACKED] * 4);
      for (ssize_t x = 0; x < (ssize_t)image->columns; x++)
      {
        SetPixelRed(image, ClampToQuantum(QuantumRange * (HalfToSinglePrecision(*row++))), q);
        SetPixelGreen(image, ClampToQuantum(QuantumRange * (HalfToSinglePrecision(*row++))), q);
        SetPixelBlue(image, ClampToQuantum(QuantumRange * (HalfToSinglePrecision(*row++))), q);
        row++;
        q += GetPixelChannels(image);
      }
    }
    else if (decoded_img_fmt == UHDR_IMG_FMT_32bppRGBA1010102)
    {
      uint32_t *row =
          (uint32_t *)dst->planes[UHDR_PLANE_PACKED] + (y * dst->stride[UHDR_PLANE_PACKED]);
      for (ssize_t x = 0; x < (ssize_t)image->columns; x++)
      {
        SetPixelRed(image, ScaleShortToQuantum((unsigned short)((*(row + x) & 0x000003FF) << 6)),
                    q);
        SetPixelGreen(image, ScaleShortToQuantum((unsigned short)((*(row + x) & 0x000FFC00) >> 4)),
                      q);
        SetPixelBlue(image, ScaleShortToQuantum((unsigned short)((*(row + x) & 0x3FF00000) >> 14)),
                     q);
        q += GetPixelChannels(image);
      }
    }
    else if (decoded_img_fmt == UHDR_IMG_FMT_32bppRGBA8888) {
      uint8_t *row =
          (uint8_t *)dst->planes[UHDR_PLANE_PACKED] + (y * dst->stride[UHDR_PLANE_PACKED] * 4);
      for (ssize_t x = 0; x < (ssize_t)image->columns; x++)
      {
        SetPixelRed(image, ScaleCharToQuantum((unsigned char)(*row++)), q);
        SetPixelGreen(image, ScaleCharToQuantum((unsigned char)(*row++)), q);
        SetPixelBlue(image, ScaleCharToQuantum((unsigned char)(*row++)), q);
        row++;
        q += GetPixelChannels(image);
      }
    }

    if (SyncAuthenticPixels(image, exception) == MagickFalse)
    {
      status = MagickFalse;
      break;
    }
  }
  uhdr_release_decoder(handle);
  CloseBlob(image);
  if (status == MagickFalse)
    return (DestroyImageList(image));
  return (GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r U H D R I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterUHDRImage() adds attributes for the Uhdr image format to the list
%  of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterUHDRImage method is:
%
%      size_t RegisterUHDRImage(void)
%
*/
ModuleExport size_t RegisterUHDRImage(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
  entry=AcquireMagickInfo("UHDr","UHDR","Ultra HDR Image Format");
#if defined(MAGICKCORE_UHDR_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadUHDRImage;
  entry->encoder=(EncodeImageHandler *) WriteUHDRImage;
#endif
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->magick=(IsImageFormatHandler *) IsUHDR;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r U H D R I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterUHDRImage() removes format registrations made by the UHDR module
%  from the list of supported formats.
%
%  The format of the UnregisterUHDRImage method is:
%
%      UnregisterUHDRImage(void)
%
*/
ModuleExport void UnregisterUHDRImage(void)
{
  (void) UnregisterMagickInfo("UHDr");
}

#if defined(MAGICKCORE_UHDR_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e U H D R I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteUHDRImage() writes an input HDR intent and SDR intent of an image in
%  UltraHDR format.
%
%  The format of the WriteUHDRImage method is:
%
%      MagickBooleanType WriteUHDRImage(const ImageInfo *image_info,
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

static MagickBooleanType IsFloatEqual(float fa, float fb)
{
  float diff = fa - fb;

  return (diff < 0.0001 && diff > -0.0001) ? MagickTrue : MagickFalse;
}

static uhdr_color_gamut_t getImageColorGamut(ChromaticityInfo *info)
{
  if (!(IsFloatEqual(info->white_point.x, 0.3127f) &&
        IsFloatEqual(info->white_point.y, 0.3290f)))
    return UHDR_CG_UNSPECIFIED;

  if (IsFloatEqual(info->blue_primary.x, 0.150f) &&
      IsFloatEqual(info->blue_primary.y, 0.060f))
  {
    if (IsFloatEqual(info->red_primary.x, 0.64f) &&
        IsFloatEqual(info->red_primary.y, 0.33f) &&
        IsFloatEqual(info->green_primary.x, 0.30f) &&
        IsFloatEqual(info->green_primary.y, 0.60f))
    {
      return UHDR_CG_BT_709;
    }
    if (IsFloatEqual(info->red_primary.x, 0.680f) &&
        IsFloatEqual(info->red_primary.y, 0.320f) &&
        IsFloatEqual(info->green_primary.x, 0.265f) &&
        IsFloatEqual(info->green_primary.y, 0.690f))
    {
      return UHDR_CG_DISPLAY_P3;
    }
  }
  else if (IsFloatEqual(info->blue_primary.x, 0.131f) &&
           IsFloatEqual(info->blue_primary.y, 0.046f) &&
           IsFloatEqual(info->red_primary.x, 0.708f) &&
           IsFloatEqual(info->red_primary.y, 0.292f) &&
           IsFloatEqual(info->green_primary.x, 0.170f) &&
           IsFloatEqual(info->green_primary.y, 0.797f))
  {
    return UHDR_CG_BT_2100;
  }

  return UHDR_CG_UNSPECIFIED;
}

static uhdr_color_gamut_t map_cg_to_uhdr_cg(const char *input_cg)
{
  if (!strcmp(input_cg, "display_p3"))
    return UHDR_CG_DISPLAY_P3;
  if (!strcmp(input_cg, "bt709"))
    return UHDR_CG_BT_709;
  if (!strcmp(input_cg, "bt2100"))
    return UHDR_CG_BT_2100;
  else
    return UHDR_CG_UNSPECIFIED;
}

static uhdr_mem_block_t GetExifProfile(Image *image, ExceptionInfo *exception)
{
  const char
    *name;

  const StringInfo
    *profile;

  uhdr_mem_block_t
    uhdr_profile = { 0 };

  ResetImageProfileIterator(image);
  for (name = GetNextImageProfile(image); name != (const char *)NULL;)
  {
    profile = GetImageProfile(image, name);
    if (LocaleCompare(name, "EXIF") == 0)
    {
      uhdr_profile.data_sz = (unsigned int)GetStringInfoLength(profile);
      uhdr_profile.capacity = uhdr_profile.data_sz;
      if (uhdr_profile.data_sz > 65533L)
        (void)ThrowMagickException(exception, GetMagickModule(), CoderWarning,
                                   "ExifProfileSizeExceedsLimit", "`%s'", image->filename);

      uhdr_profile.data = GetStringInfoDatum(profile);
      return uhdr_profile;
    }
    if (image->debug != MagickFalse)
      (void)LogMagickEvent(CoderEvent, GetMagickModule(), "%s profile: %.17g bytes", name,
                           (double)GetStringInfoLength(profile));
    name = GetNextImageProfile(image);
  }
  return uhdr_profile;
}

static void fillRawImageDescriptor(uhdr_raw_image_t *imgDescriptor, const ImageInfo *image_info,
                                   Image *image, uhdr_img_fmt_t fmt, uhdr_color_transfer_t ct)
{
  const char
    *option;

  imgDescriptor->fmt = fmt;

  if (image->depth >= 10)
  {
    option = GetImageOption(image_info, "uhdr:hdr-color-gamut");
    imgDescriptor->cg = (option != (const char *)NULL) ? map_cg_to_uhdr_cg(option)
                                                       : getImageColorGamut(&image->chromaticity);
  }
  else if (image->depth == 8)
  {
    option = GetImageOption(image_info, "uhdr:sdr-color-gamut");
    imgDescriptor->cg = (option != (const char *)NULL) ? map_cg_to_uhdr_cg(option)
                                                       : getImageColorGamut(&image->chromaticity);
  }

  imgDescriptor->range = imgDescriptor->fmt == UHDR_IMG_FMT_24bppYCbCrP010 ? UHDR_CR_LIMITED_RANGE
                                                                           : UHDR_CR_FULL_RANGE;
  imgDescriptor->ct = ct;
  imgDescriptor->w = image->columns;
  imgDescriptor->h = image->rows;
}

static size_t GetHDRGMPropertySize(const Image *image,const char *name,
  ExceptionInfo *exception)
{
  char
    property[MagickPathExtent];

  const char
    *value;

  (void) FormatLocaleString(property,MagickPathExtent,"hdrgm:%s",name);
  value=GetImageProperty(image,property,exception);
  if (value == (const char *) NULL)
    return(0);
  return((size_t) StringToUnsignedLong(value));
}

static size_t ScaleGainMapExtent(const size_t extent,
  const size_t scaled_extent,const size_t base_extent)
{
  double
    scale;

  if ((extent == 0) || (scaled_extent == 0) || (base_extent == 0))
    return(0);
  scale=((double) extent*(double) scaled_extent)/(double) base_extent;
  if (scale < 1.0)
    return(1);
  if (scale > (double) MAGICK_SSIZE_MAX)
    return(0);
  return(CastDoubleToSizeT(scale+0.5));
}

static ssize_t ScaleGainMapCoordinate(const double coordinate,
  const size_t extent,const size_t base_extent)
{
  double
    scale;

  if ((extent == 0) || (base_extent == 0))
    return(0);
  scale=((double) extent*coordinate)/(double) base_extent;
  if (scale < 0.0)
    return(0);
  if (scale > (double) MAGICK_SSIZE_MAX)
    return(MAGICK_SSIZE_MAX);
  return(CastDoubleToSsizeT(floor(scale+0.5)));
}

static MagickBooleanType IsGainMapBaseGeometry(const size_t base_columns,
  const size_t base_rows,const double columns,const double rows)
{
  if ((columns <= 0.0) || (rows <= 0.0))
    return(MagickFalse);
  if ((base_columns != CastDoubleToSizeT(columns)) ||
      (base_rows != CastDoubleToSizeT(rows)))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ReplaceGainMapImage(Image **gainmap_image,
  Image *transform_image)
{
  if (transform_image == (Image *) NULL)
    return(MagickFalse);
  *gainmap_image=DestroyImageList(*gainmap_image);
  *gainmap_image=transform_image;
  return(MagickTrue);
}

static MagickBooleanType CropGainMapImage(Image **gainmap_image,
  const size_t base_columns,const size_t base_rows,const double columns,
  const double rows,const double x,const double y,ExceptionInfo *exception)
{
  Image
    *crop_image;

  RectangleInfo
    geometry;

  ssize_t
    x0,
    x1,
    y0,
    y1;

  if ((base_columns == 0) || (base_rows == 0) || (columns <= 0.0) ||
      (rows <= 0.0))
    return(MagickFalse);
  if ((x < 0.0) || (y < 0.0) || ((x+columns) > (double) base_columns) ||
      ((y+rows) > (double) base_rows))
    return(MagickFalse);
  x0=ScaleGainMapCoordinate(x,(*gainmap_image)->columns,base_columns);
  y0=ScaleGainMapCoordinate(y,(*gainmap_image)->rows,base_rows);
  x1=ScaleGainMapCoordinate(x+columns,(*gainmap_image)->columns,base_columns);
  y1=ScaleGainMapCoordinate(y+rows,(*gainmap_image)->rows,base_rows);
  if (x0 >= (ssize_t) (*gainmap_image)->columns)
    x0=(ssize_t) (*gainmap_image)->columns-1;
  if (y0 >= (ssize_t) (*gainmap_image)->rows)
    y0=(ssize_t) (*gainmap_image)->rows-1;
  if (x1 > (ssize_t) (*gainmap_image)->columns)
    x1=(ssize_t) (*gainmap_image)->columns;
  if (y1 > (ssize_t) (*gainmap_image)->rows)
    y1=(ssize_t) (*gainmap_image)->rows;
  if (x1 <= x0)
    x1=x0+1;
  if (y1 <= y0)
    y1=y0+1;
  geometry.x=x0;
  geometry.y=y0;
  geometry.width=(size_t) (x1-x0);
  geometry.height=(size_t) (y1-y0);
  crop_image=CropImage(*gainmap_image,&geometry,exception);
  return(ReplaceGainMapImage(gainmap_image,crop_image));
}

static MagickBooleanType ResizeGainMapImage(Image **gainmap_image,
  size_t *base_columns,size_t *base_rows,const size_t columns,
  const size_t rows,const Image *image,ExceptionInfo *exception)
{
  Image
    *resize_image;

  size_t
    target_columns,
    target_rows;

  target_columns=ScaleGainMapExtent((*gainmap_image)->columns,columns,
    *base_columns);
  target_rows=ScaleGainMapExtent((*gainmap_image)->rows,rows,*base_rows);
  if ((target_columns == 0) || (target_rows == 0))
    return(MagickFalse);
  if ((target_columns != (*gainmap_image)->columns) ||
      (target_rows != (*gainmap_image)->rows))
    {
      resize_image=ResizeImage(*gainmap_image,target_columns,target_rows,
        image->filter,exception);
      if (ReplaceGainMapImage(gainmap_image,resize_image) == MagickFalse)
        return(MagickFalse);
    }
  *base_columns=columns;
  *base_rows=rows;
  return(MagickTrue);
}

static MagickBooleanType ApplyGainMapTransform(Image **gainmap_image,
  size_t *base_columns,size_t *base_rows,const Image *image,
  const char *transform,ExceptionInfo *exception)
{
  double
    columns,
    rows,
    source_columns,
    source_rows,
    x,
    y;

  Image
    *transform_image;

  size_t
    rotations;

  if (LocaleNCompare(transform,"crop ",5) == 0)
    {
      if (sscanf(transform+5,"%lfx%lf %lfx%lf%lf%lf",&source_columns,
          &source_rows,&columns,&rows,&x,&y) != 6)
        return(MagickFalse);
      if (IsGainMapBaseGeometry(*base_columns,*base_rows,source_columns,
          source_rows) == MagickFalse)
        return(MagickFalse);
      if (CropGainMapImage(gainmap_image,*base_columns,*base_rows,columns,
          rows,x,y,exception) == MagickFalse)
        return(MagickFalse);
      *base_columns=CastDoubleToSizeT(columns);
      *base_rows=CastDoubleToSizeT(rows);
      return(MagickTrue);
    }
  if (LocaleNCompare(transform,"resize ",7) == 0)
    {
      if (sscanf(transform+7,"%lfx%lf %lfx%lf",&source_columns,
          &source_rows,&columns,&rows) != 4)
        return(MagickFalse);
      if (IsGainMapBaseGeometry(*base_columns,*base_rows,source_columns,
          source_rows) == MagickFalse)
        return(MagickFalse);
      return(ResizeGainMapImage(gainmap_image,base_columns,base_rows,
        CastDoubleToSizeT(columns),CastDoubleToSizeT(rows),image,exception));
    }
  if (LocaleNCompare(transform,"flip ",5) == 0)
    {
      if (sscanf(transform+5,"%lfx%lf",&source_columns,&source_rows) != 2)
        return(MagickFalse);
      if (IsGainMapBaseGeometry(*base_columns,*base_rows,source_columns,
          source_rows) == MagickFalse)
        return(MagickFalse);
      transform_image=FlipImage(*gainmap_image,exception);
      return(ReplaceGainMapImage(gainmap_image,transform_image));
    }
  if (LocaleNCompare(transform,"flop ",5) == 0)
    {
      if (sscanf(transform+5,"%lfx%lf",&source_columns,&source_rows) != 2)
        return(MagickFalse);
      if (IsGainMapBaseGeometry(*base_columns,*base_rows,source_columns,
          source_rows) == MagickFalse)
        return(MagickFalse);
      transform_image=FlopImage(*gainmap_image,exception);
      return(ReplaceGainMapImage(gainmap_image,transform_image));
    }
  if (LocaleNCompare(transform,"rotate ",7) == 0)
    {
      size_t
        next_columns,
        next_rows;

      if (sscanf(transform+7,"%lfx%lf %lf",&source_columns,&source_rows,
          &x) != 3)
        return(MagickFalse);
      if (IsGainMapBaseGeometry(*base_columns,*base_rows,source_columns,
          source_rows) == MagickFalse)
        return(MagickFalse);
      rotations=CastDoubleToSizeT(x) % 4;
      if (rotations == 0)
        return(MagickTrue);
      transform_image=IntegralRotateImage(*gainmap_image,rotations,exception);
      if (ReplaceGainMapImage(gainmap_image,transform_image) == MagickFalse)
        return(MagickFalse);
      if ((rotations == 1) || (rotations == 3))
        {
          next_columns=(*base_rows);
          next_rows=(*base_columns);
          *base_columns=next_columns;
          *base_rows=next_rows;
        }
      return(MagickTrue);
    }
  if (LocaleNCompare(transform,"transpose ",10) == 0)
    {
      size_t
        next_columns;

      if (sscanf(transform+10,"%lfx%lf",&source_columns,&source_rows) != 2)
        return(MagickFalse);
      if (IsGainMapBaseGeometry(*base_columns,*base_rows,source_columns,
          source_rows) == MagickFalse)
        return(MagickFalse);
      transform_image=TransposeImage(*gainmap_image,exception);
      if (ReplaceGainMapImage(gainmap_image,transform_image) == MagickFalse)
        return(MagickFalse);
      next_columns=(*base_rows);
      *base_rows=(*base_columns);
      *base_columns=next_columns;
      return(MagickTrue);
    }
  if (LocaleNCompare(transform,"transverse ",11) == 0)
    {
      size_t
        next_columns;

      if (sscanf(transform+11,"%lfx%lf",&source_columns,&source_rows) != 2)
        return(MagickFalse);
      if (IsGainMapBaseGeometry(*base_columns,*base_rows,source_columns,
          source_rows) == MagickFalse)
        return(MagickFalse);
      transform_image=TransverseImage(*gainmap_image,exception);
      if (ReplaceGainMapImage(gainmap_image,transform_image) == MagickFalse)
        return(MagickFalse);
      next_columns=(*base_rows);
      *base_rows=(*base_columns);
      *base_columns=next_columns;
      return(MagickTrue);
    }
  return(MagickFalse);
}

static StringInfo *EncodeBaseImageProfile(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  Image
    *base_image;

  ImageInfo
    *base_info;

  size_t
    length;

  StringInfo
    *profile,
    *removed_profile;

  void
    *blob;

  base_image=CloneImage(image,0,0,MagickTrue,exception);
  if (base_image == (Image *) NULL)
    return((StringInfo *) NULL);
  removed_profile=RemoveImageProfile(base_image,"hdrgm");
  if (removed_profile != (StringInfo *) NULL)
    removed_profile=DestroyStringInfo(removed_profile);
  base_info=CloneImageInfo(image_info);
  (void) CopyMagickString(base_info->filename,"JPEG:uhdr-base.jpg",
    MagickPathExtent);
  (void) CopyMagickString(base_info->magick,"JPEG",MagickPathExtent);
  if (image->quality > 0)
    base_info->quality=image->quality;
  (void) CopyMagickString(base_image->magick,"JPEG",MagickPathExtent);
  blob=ImageToBlob(base_info,base_image,&length,exception);
  base_image=DestroyImage(base_image);
  base_info=DestroyImageInfo(base_info);
  if (blob == (void *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
        "UnableToEncodeBaseImage","`%s'",image->filename);
      return((StringInfo *) NULL);
    }
  profile=BlobToProfileStringInfo("uhdr-base",blob,length,exception);
  blob=RelinquishMagickMemory(blob);
  return(profile);
}

static StringInfo *TransformGainMapProfile(const ImageInfo *image_info,
  const Image *image,const StringInfo *gainmap_profile,
  MagickBooleanType *transform_status,ExceptionInfo *exception)
{
  const char
    *option,
    *transforms;

  Image
    *gainmap_images;

  ImageInfo
    *gainmap_info;

  MagickBooleanType
    status,
    transformed,
    transform_required;

  size_t
    base_columns,
    base_rows,
    length;

  StringInfo
    *profile;

  void
    *blob;

  assert(transform_status != (MagickBooleanType *) NULL);
  *transform_status=MagickTrue;
  base_columns=GetHDRGMPropertySize(image,"BaseWidth",exception);
  base_rows=GetHDRGMPropertySize(image,"BaseHeight",exception);
  transforms=GetImageProperty(image,"hdrgm:Transform",exception);
  transform_required=((transforms != (const char *) NULL) &&
    (*transforms != '\0')) ? MagickTrue : MagickFalse;
  if ((base_columns == 0) || (base_rows == 0))
    {
      if (transform_required != MagickFalse)
        {
          *transform_status=MagickFalse;
          (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
            "UnableToTransformGainMap","`%s'",image->filename);
        }
      return((StringInfo *) NULL);
    }
  if ((base_columns != image->columns) || (base_rows != image->rows))
    {
      if (transform_required == MagickFalse)
        {
          *transform_status=MagickFalse;
          (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
            "UnableToTransformGainMap","`%s'",image->filename);
          return((StringInfo *) NULL);
        }
    }
  if (transform_required == MagickFalse)
    return((StringInfo *) NULL);
  gainmap_info=CloneImageInfo(image_info);
  (void) CopyMagickString(gainmap_info->filename,"JPEG:hdrgm.jpg",
    MagickPathExtent);
  (void) CopyMagickString(gainmap_info->magick,"JPEG",MagickPathExtent);
  (void) SetImageOption(gainmap_info,"jpeg:detect-uhdr","false");
  gainmap_images=BlobToImage(gainmap_info,GetStringInfoDatum(gainmap_profile),
    GetStringInfoLength(gainmap_profile),exception);
  if (gainmap_images == (Image *) NULL)
    {
      gainmap_info=DestroyImageInfo(gainmap_info);
      *transform_status=MagickFalse;
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
        "UnableToDecodeGainMap","`%s'",image->filename);
      return((StringInfo *) NULL);
    }
  status=MagickTrue;
  transformed=MagickFalse;
  if ((transforms != (const char *) NULL) && (*transforms != '\0'))
    {
      char
        *next,
        *transform,
        *transform_list;

      transform_list=AcquireString(transforms);
      for (transform=transform_list; transform != (char *) NULL; )
      {
        next=strchr(transform,';');
        if (next != (char *) NULL)
          *next++='\0';
        if (*transform != '\0')
          {
            status=ApplyGainMapTransform(&gainmap_images,&base_columns,
              &base_rows,image,transform,exception);
            if (status == MagickFalse)
              break;
            transformed=MagickTrue;
          }
        transform=next;
      }
      transform_list=DestroyString(transform_list);
    }
  if ((status != MagickFalse) &&
      ((base_columns != image->columns) || (base_rows != image->rows)))
    status=MagickFalse;
  if (status == MagickFalse)
    {
      gainmap_info=DestroyImageInfo(gainmap_info);
      gainmap_images=DestroyImageList(gainmap_images);
      *transform_status=MagickFalse;
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
        "UnableToTransformGainMap","`%s'",image->filename);
      return((StringInfo *) NULL);
    }
  if (transformed == MagickFalse)
    {
      gainmap_images=DestroyImageList(gainmap_images);
      gainmap_info=DestroyImageInfo(gainmap_info);
      return((StringInfo *) NULL);
    }
  option=GetImageOption(image_info,"uhdr:gainmap-quality");
  if (option != (const char *) NULL)
    gainmap_info->quality=StringToUnsignedLong(option);
  else if (image->quality > 0)
    gainmap_info->quality=image->quality;
  (void) CopyMagickString(gainmap_images->magick,"JPEG",MagickPathExtent);
  blob=ImageToBlob(gainmap_info,gainmap_images,&length,exception);
  gainmap_images=DestroyImageList(gainmap_images);
  gainmap_info=DestroyImageInfo(gainmap_info);
  if (blob == (void *) NULL)
    {
      *transform_status=MagickFalse;
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
        "UnableToEncodeGainMap","`%s'",image->filename);
      return((StringInfo *) NULL);
    }
  profile=BlobToProfileStringInfo("hdrgm",blob,length,exception);
  blob=RelinquishMagickMemory(blob);
  if (profile == (StringInfo *) NULL)
    {
      *transform_status=MagickFalse;
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
        "UnableToEncodeGainMap","`%s'",image->filename);
    }
  return(profile);
}

static MagickBooleanType WriteUHDRImage(const ImageInfo *image_info,
  Image *images,ExceptionInfo *exception)
{
#define GetHDRGMProperty(name,field) \
  do { \
    const char *v = GetImageProperty(image,"hdrgm:" name,exception); \
    if (v != (const char *) NULL) \
      gainmap_info.field=(float) atof(v); \
  } while (0)
#define GetHDRGMPropertyInt(name,field) \
  do { \
    const char *v = GetImageProperty(image,"hdrgm:" name,exception); \
    if (v != (const char *) NULL) \
      gainmap_info.field=atoi(v); \
  } while (0)
#define GetHDRGMProperty3(name,field0,field1,field2) \
  do { \
    const char *v = GetImageProperty(image,"hdrgm:" name,exception); \
    if (v != (const char *) NULL) \
      (void) sscanf(v,"%f,%f,%f",&gainmap_info.field0,&gainmap_info.field1, \
        &gainmap_info.field2); \
  } while (0)

  Image
    *image = images;

  MagickBooleanType
    status = MagickTrue;

  uhdr_raw_image_t
    hdrImgDescriptor = { 0 },
    sdrImgDescriptor = { 0 };

  uhdr_mem_block_t
    sdr_profile = { 0 },
    hdr_profile = { 0 };

  StringInfo
    *base_image_profile = (StringInfo *) NULL,
    *resized_gainmap_profile = (StringInfo *) NULL;

  uhdr_compressed_image_t
    base_image = { 0 },
    gainmap_image = { 0 };

  uhdr_gainmap_metadata_t
    gainmap_info = { 0 };

  MagickBooleanType
    gainmap_transform_status = MagickTrue,
    preserve_gainmap = MagickFalse;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent, GetMagickModule(), "%s", image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return (status);

  const StringInfo *gainmap_profile = GetImageProfile(image,"hdrgm");

  const size_t
    image_count = GetImageListLength(image);

  if (gainmap_profile != (const StringInfo *) NULL)
    {
      /*
        Preserve gainmap+metadata (for transcoding). If these exist, we do not
        regenerate the gainmap.  Instead, we pass them directly to the encoder.
      */
      /*
        Compressed image descriptor.
      */
      resized_gainmap_profile=TransformGainMapProfile(image_info,image,
        gainmap_profile,&gainmap_transform_status,exception);
      if (gainmap_transform_status == MagickFalse)
        {
          (void) CloseBlob(image);
          return(MagickFalse);
        }
      if (resized_gainmap_profile != (StringInfo *) NULL)
        gainmap_profile=(const StringInfo *) resized_gainmap_profile;
      base_image_profile=EncodeBaseImageProfile(image_info,image,exception);
      if (base_image_profile == (StringInfo *) NULL)
        {
          if (resized_gainmap_profile != (StringInfo *) NULL)
            resized_gainmap_profile=DestroyStringInfo(resized_gainmap_profile);
          (void) CloseBlob(image);
          return(MagickFalse);
        }
      base_image.data=(void *) GetStringInfoDatum(base_image_profile);
      base_image.data_sz=GetStringInfoLength(base_image_profile);
      base_image.capacity=base_image.data_sz;
      base_image.cg=getImageColorGamut(&image->chromaticity);
      base_image.ct=UHDR_CT_SRGB;
      base_image.range=UHDR_CR_FULL_RANGE;
      gainmap_image.data=(void *) GetStringInfoDatum(gainmap_profile);
      gainmap_image.data_sz=GetStringInfoLength(gainmap_profile);
      gainmap_image.capacity=gainmap_image.data_sz;
      gainmap_image.cg=UHDR_CG_UNSPECIFIED;
      gainmap_image.ct=UHDR_CT_UNSPECIFIED;
      gainmap_image.range=UHDR_CR_UNSPECIFIED;
      /*
        Gainmap metadata descriptor.
      */
      GetHDRGMProperty3("GainMapMax",max_content_boost[0],max_content_boost[1],
        max_content_boost[2]);
      GetHDRGMProperty3("GainMapMin",min_content_boost[0],min_content_boost[1],
        min_content_boost[2]);
      GetHDRGMProperty3("Gamma",gamma[0],gamma[1],gamma[2]);
      GetHDRGMProperty3("OffsetSDR",offset_sdr[0],offset_sdr[1],offset_sdr[2]);
      GetHDRGMProperty3("OffsetHDR",offset_hdr[0],offset_hdr[1],offset_hdr[2]);
      GetHDRGMProperty("HDRCapacityMin",hdr_capacity_min);
      GetHDRGMProperty("HDRCapacityMax",hdr_capacity_max);
      GetHDRGMPropertyInt("UseBaseColorGrade",use_base_cg);
      preserve_gainmap=MagickTrue;
    }

  const char
    *option = GetImageOption(image_info,"uhdr:hdr-color-transfer");

  uhdr_color_transfer_t
    hdr_ct = (option != (const char *) NULL) ? map_ct_to_uhdr_ct(option) : UHDR_CT_SRGB;

  if (hdr_ct == UHDR_CT_UNSPECIFIED)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ConfigureWarning,
        "invalid hdr color transfer received, ","%s","exiting ... ");
      return(MagickFalse);
    }

  /*
    HDR intent:
      color transfer linear, MUST be rgba half float - bitdepth is ATLEAST 16
      color transfer hlg or pq, MUST be rgba1010102/p010-bitdepth is ATLEAST 10

    SDR intent
      color transfer sRGB MUST be rgba8888/yuv420 - bitdepth MUST be 8
  */
  int
    hdrIntentMinDepth = hdr_ct == UHDR_CT_LINEAR ? 16 : 10;

  for (int i = 0; i < (ssize_t) image_count; i++)
  {
    /* Classify image as hdr/sdr intent basing on depth */
    int
      bpp;

    ssize_t
      aligned_height,
      aligned_width;

    size_t
      picSize;

    void
      *crBuffer = NULL, *cbBuffer = NULL, *yBuffer = NULL;

    if (((double) image->columns > sqrt(MAGICK_SSIZE_MAX/3.0)) ||
        ((double) image->rows > sqrt(MAGICK_SSIZE_MAX/3.0)))
      {
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "WidthOrHeightExceedsLimit","%s",image->filename);
        goto next_image;
      }
    bpp = (int) image->depth >= hdrIntentMinDepth ? 2 : 1;
    if (IssRGBCompatibleColorspace(image->colorspace) && !IsGrayColorspace(image->colorspace))
    {
      if ((int) image->depth >= hdrIntentMinDepth && hdr_ct == UHDR_CT_LINEAR)
        bpp = 8; /* rgbahalf float */
      else
        bpp = 4; /* rgba1010102 or rgba8888 */
    }
    else if (IsYCbCrCompatibleColorspace(image->colorspace))
    {
      if ((int) image->depth >= hdrIntentMinDepth && hdr_ct == UHDR_CT_LINEAR)
      {
        (void) ThrowMagickException(exception, GetMagickModule(), ConfigureWarning,
          "linear color transfer inputs MUST be compatible with RGB Colorspace, ", "%s",
          "ignoring ...");
        goto next_image;
      }
    }
    else
    {
      (void) ThrowMagickException(exception, GetMagickModule(), ConfigureWarning,
        "Received image with color space incompatible with RGB/YCbCr, ","%s","ignoring ...");
      goto next_image;
    }

    aligned_width = image->columns + (image->columns & 1);
    aligned_height = image->rows + (image->rows & 1);
    if (HeapOverflowSanityCheckGetSize(aligned_width,aligned_height,&picSize) != MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"ImproperImageHeader","%s",image->filename);
        goto next_image;
      }
    if (HeapOverflowSanityCheckGetSize(picSize,bpp,&picSize) != MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"ImproperImageHeader","%s",image->filename);
        goto next_image;
      }
    if (bpp < 4)
      {
        if (HeapOverflowSanityCheckGetSize(picSize,3,&picSize) != MagickFalse)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CorruptImageError,"ImproperImageHeader","%s",image->filename);
            goto next_image;
          }
        picSize/=2;
      }

    if (((int) image->depth < hdrIntentMinDepth) && (image->depth != 8))
    {
      (void) ThrowMagickException(exception, GetMagickModule(), ConfigureWarning,
        "Received image with unexpected bit depth","%s","ignoring ...");
      goto next_image;
    }

    if (((int) image->depth >= hdrIntentMinDepth) &&
        (hdrImgDescriptor.planes[UHDR_PLANE_Y] != NULL))
    {
      (void) ThrowMagickException(exception, GetMagickModule(), ConfigureWarning,
        "Received multiple hdr intent resources, ","%s","overwriting ...");
      RelinquishMagickMemory(hdrImgDescriptor.planes[UHDR_PLANE_Y]);
      hdrImgDescriptor.planes[UHDR_PLANE_Y] = NULL;
    }
    else if ((image->depth == 8) &&
             (sdrImgDescriptor.planes[UHDR_PLANE_Y] != NULL))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ConfigureWarning,
        "Received multiple sdr intent resources, ","%s","overwriting ...");
      RelinquishMagickMemory(sdrImgDescriptor.planes[UHDR_PLANE_Y]);
      sdrImgDescriptor.planes[UHDR_PLANE_Y] = NULL;
    }

    yBuffer = AcquireMagickMemory(picSize);
    if (yBuffer == NULL)
    {
      status = MagickFalse;
      break;
    }

    if ((int) image->depth >= hdrIntentMinDepth)
    {
      if (IsYCbCrCompatibleColorspace(image->colorspace))
      {
        cbBuffer = ((uint16_t *) yBuffer) + aligned_width * aligned_height;
        crBuffer = ((uint16_t *) cbBuffer) + 1;

        fillRawImageDescriptor(&hdrImgDescriptor, image_info, image, UHDR_IMG_FMT_24bppYCbCrP010,
                               hdr_ct);

        hdrImgDescriptor.planes[UHDR_PLANE_Y] = yBuffer;
        hdrImgDescriptor.planes[UHDR_PLANE_UV] = cbBuffer;
        hdrImgDescriptor.planes[UHDR_PLANE_V] = NULL;

        hdrImgDescriptor.stride[UHDR_PLANE_Y] = aligned_width;
        hdrImgDescriptor.stride[UHDR_PLANE_UV] = aligned_width;
        hdrImgDescriptor.stride[UHDR_PLANE_V] = 0;
      }
      else
      {
        fillRawImageDescriptor(&hdrImgDescriptor, image_info, image,
                               hdr_ct == UHDR_CT_LINEAR ? UHDR_IMG_FMT_64bppRGBAHalfFloat
                                                        : UHDR_IMG_FMT_32bppRGBA1010102,
                               hdr_ct);

        hdrImgDescriptor.planes[UHDR_PLANE_PACKED] = yBuffer;
        hdrImgDescriptor.planes[UHDR_PLANE_U] = NULL;
        hdrImgDescriptor.planes[UHDR_PLANE_V] = NULL;

        hdrImgDescriptor.stride[UHDR_PLANE_PACKED] = aligned_width;
        hdrImgDescriptor.stride[UHDR_PLANE_U] = 0;
        hdrImgDescriptor.stride[UHDR_PLANE_V] = 0;
      }

      hdr_profile = GetExifProfile(image, exception);
    }
    else if (image->depth == 8)
    {
      if (IsYCbCrCompatibleColorspace(image->colorspace))
      {
        cbBuffer = ((uint8_t *) yBuffer) + aligned_width * aligned_height;
        crBuffer = ((uint8_t *) cbBuffer) + ((aligned_width / 2) * (aligned_height / 2));

        fillRawImageDescriptor(&sdrImgDescriptor, image_info, image, UHDR_IMG_FMT_12bppYCbCr420,
                               UHDR_CT_SRGB);

        sdrImgDescriptor.planes[UHDR_PLANE_Y] = yBuffer;
        sdrImgDescriptor.planes[UHDR_PLANE_U] = cbBuffer;
        sdrImgDescriptor.planes[UHDR_PLANE_V] = crBuffer;

        sdrImgDescriptor.stride[UHDR_PLANE_Y] = aligned_width;
        sdrImgDescriptor.stride[UHDR_PLANE_U] = aligned_width / 2;
        sdrImgDescriptor.stride[UHDR_PLANE_V] = aligned_width / 2;
      }
      else
      {
        fillRawImageDescriptor(&sdrImgDescriptor, image_info, image, UHDR_IMG_FMT_32bppRGBA8888,
                               UHDR_CT_SRGB);

        sdrImgDescriptor.planes[UHDR_PLANE_PACKED] = yBuffer;
        sdrImgDescriptor.planes[UHDR_PLANE_U] = NULL;
        sdrImgDescriptor.planes[UHDR_PLANE_V] = NULL;

        sdrImgDescriptor.stride[UHDR_PLANE_PACKED] = aligned_width;
        sdrImgDescriptor.stride[UHDR_PLANE_U] = 0;
        sdrImgDescriptor.stride[UHDR_PLANE_V] = 0;
      }

      sdr_profile = GetExifProfile(image, exception);
    }

    for (int y = 0; y < (ssize_t) image->rows; y++)
    {
      const Quantum
        *p;

      ssize_t
        x;

      p = GetVirtualPixels(image, 0, y, image->columns, 1, exception);
      if (p == (const Quantum *) NULL)
      {
        status = MagickFalse;
        break;
      }

      for (x = 0; x < (ssize_t) image->columns; x++)
      {
        if ((int) image->depth >= hdrIntentMinDepth)
        {
          if (hdrImgDescriptor.fmt == UHDR_IMG_FMT_24bppYCbCrP010)
          {
            uint16_t
              *crBase = crBuffer, *cbBase = cbBuffer, *yBase = yBuffer;

            yBase[y * hdrImgDescriptor.stride[UHDR_PLANE_Y] + x] =
              ScaleQuantumToShort(GetPixelY(image, p)) & 0xFFC0;
            if ((y % 2 == 0) && (x % 2 == 0))
            {
              cbBase[y / 2 * hdrImgDescriptor.stride[UHDR_PLANE_UV] + x] =
                ScaleQuantumToShort(GetPixelCb(image, p)) & 0xFFC0;
              crBase[y / 2 * hdrImgDescriptor.stride[UHDR_PLANE_UV] + x] =
                ScaleQuantumToShort(GetPixelCr(image, p)) & 0xFFC0;
            }
          }
          else if (hdrImgDescriptor.fmt == UHDR_IMG_FMT_64bppRGBAHalfFloat)
          {
            uint64_t
              *rgbaBase = yBuffer;

            unsigned short
              r, g, b, a;

            r = SinglePrecisionToHalf(QuantumScale * GetPixelRed(image, p));
            g = SinglePrecisionToHalf(QuantumScale * GetPixelGreen(image, p));
            b = SinglePrecisionToHalf(QuantumScale * GetPixelBlue(image, p));
            a = SinglePrecisionToHalf(QuantumScale * GetPixelAlpha(image, p));

            rgbaBase[y * hdrImgDescriptor.stride[UHDR_PLANE_PACKED] + x] =
                ((uint64_t)a << 48) | ((uint64_t)b << 32) | (g << 16) | (r);
          }
          else
          {
            uint32_t
              *rgbBase = yBuffer;

            unsigned short
              r, g, b;

            r = ScaleQuantumToShort(GetPixelRed(image, p)) & 0xFFC0;
            g = ScaleQuantumToShort(GetPixelGreen(image, p)) & 0xFFC0;
            b = ScaleQuantumToShort(GetPixelBlue(image, p)) & 0xFFC0;

            rgbBase[y * hdrImgDescriptor.stride[UHDR_PLANE_PACKED] + x] =
                (0x3U << 30) | (b << 14) | (g << 4) | (r >> 6);
          }
        }
        else if (image->depth == 8)
        {
          if (sdrImgDescriptor.fmt == UHDR_IMG_FMT_12bppYCbCr420)
          {
            uint8_t
              *crBase = crBuffer, *cbBase = cbBuffer, *yBase = yBuffer;

            yBase[y * sdrImgDescriptor.stride[UHDR_PLANE_Y] + x] =
              ScaleQuantumToChar(GetPixelY(image, p));
            if ((y % 2 == 0) && (x % 2 == 0))
            {
              cbBase[y / 2 * sdrImgDescriptor.stride[UHDR_PLANE_U] + x / 2] =
                ScaleQuantumToChar(GetPixelCb(image, p));
              crBase[y / 2 * sdrImgDescriptor.stride[UHDR_PLANE_V] + x / 2] =
                ScaleQuantumToChar(GetPixelCr(image, p));
            }
          }
          else
          {
            uint32_t
              *rgbBase = yBuffer;

            unsigned char
              r, g, b;

            r = ScaleQuantumToChar(GetPixelRed(image, p));
            g = ScaleQuantumToChar(GetPixelGreen(image, p));
            b = ScaleQuantumToChar(GetPixelBlue(image, p));

            rgbBase[y * sdrImgDescriptor.stride[UHDR_PLANE_PACKED] + x] = (b << 16) | (g << 8) | r;
          }
        }
        p += GetPixelChannels(image);
      }
    }

next_image:
    if (i != (ssize_t) image_count - 1)
    {
      if (GetNextImageInList(image) == (Image *) NULL)
      {
        status = MagickFalse;
        break;
      }
      image = SyncNextImageInList(image);
      if (image == (Image *) NULL)
        break;
    }

    status = SetImageProgress(image, SaveImageTag, (MagickOffsetType)i,
      image_count);
    if (status == MagickFalse)
      break;
  }

  if (status != MagickFalse)
  {
    uhdr_codec_private_t *handle = uhdr_create_encoder();

#define CHECK_IF_ERR(x)                                                                       \
  {                                                                                           \
    uhdr_error_info_t retval = (x);                                                           \
    if (retval.error_code != UHDR_CODEC_OK)                                                   \
    {                                                                                         \
      (void)ThrowMagickException(exception, GetMagickModule(), CoderError,                    \
                                 retval.has_detail ? retval.detail : "unknown error", "`%s'", \
                                 image->filename);                                            \
      uhdr_release_encoder(handle);                                                           \
      status = MagickFalse;                                                                   \
    }                                                                                         \
  }

    if (handle == NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          "FailedToCreateEncoder","`%s'",image->filename);
        status=MagickFalse;
      }

    if ((status != MagickFalse) && (preserve_gainmap != MagickFalse))
      {
        CHECK_IF_ERR(uhdr_enc_set_compressed_image(handle,&base_image,
          UHDR_BASE_IMG))
        if (status != MagickFalse)
          CHECK_IF_ERR(uhdr_enc_set_gainmap_image(handle,&gainmap_image,
            &gainmap_info))
      }
    else
      {
        /* Configure hdr and sdr intents */
        if (status != MagickFalse && hdrImgDescriptor.planes[UHDR_PLANE_Y])
        {
          CHECK_IF_ERR(uhdr_enc_set_raw_image(handle, &hdrImgDescriptor, UHDR_HDR_IMG))
          if (hdr_profile.data_sz != 0)
            CHECK_IF_ERR(uhdr_enc_set_exif_data(handle, &hdr_profile))
        }

        if (status != MagickFalse && sdrImgDescriptor.planes[UHDR_PLANE_Y])
        {
          CHECK_IF_ERR(uhdr_enc_set_raw_image(handle, &sdrImgDescriptor, UHDR_SDR_IMG))
          if (sdr_profile.data_sz != 0)
            CHECK_IF_ERR(uhdr_enc_set_exif_data(handle, &sdr_profile))
        }
      }

    /* Configure encoding settings */
    if (status != MagickFalse && image->quality > 0 && image->quality <= 100)
      CHECK_IF_ERR(uhdr_enc_set_quality(handle, image->quality, UHDR_BASE_IMG))

    const char
      *option;

    if ((status != MagickFalse) && (preserve_gainmap == MagickFalse))
    {
      option = GetImageOption(image_info, "uhdr:gainmap-quality");
      if (option != (const char *)NULL)
        CHECK_IF_ERR(uhdr_enc_set_quality(handle, atoi(option), UHDR_GAIN_MAP_IMG))
    }

    if ((status != MagickFalse) && (preserve_gainmap == MagickFalse))
      CHECK_IF_ERR(uhdr_enc_set_using_multi_channel_gainmap(handle, 1))

    if ((status != MagickFalse) && (preserve_gainmap == MagickFalse))
      CHECK_IF_ERR(uhdr_enc_set_gainmap_scale_factor(handle, 1))

    if ((status != MagickFalse) && (preserve_gainmap == MagickFalse))
      CHECK_IF_ERR(uhdr_enc_set_preset(handle, UHDR_USAGE_BEST_QUALITY))

    /* Configure gainmap metadata */
    if ((status != MagickFalse) && (preserve_gainmap == MagickFalse))
    {
      option = GetImageOption(image_info, "uhdr:gainmap-gamma");
      if (option != (const char *)NULL)
        CHECK_IF_ERR(uhdr_enc_set_gainmap_gamma(handle, atof(option)))
    }

    if ((status != MagickFalse) && (preserve_gainmap == MagickFalse))
    {
      option = GetImageOption(image_info, "uhdr:gainmap-min-content-boost");
      float minContentBoost = option != (const char *)NULL ? atof(option) : FLT_MIN;

      option = GetImageOption(image_info, "uhdr:gainmap-max-content-boost");
      float maxContentBoost = option != (const char *)NULL ? atof(option) : FLT_MAX;

      if (minContentBoost != FLT_MIN || maxContentBoost != FLT_MAX)
        CHECK_IF_ERR(uhdr_enc_set_min_max_content_boost(handle, minContentBoost, maxContentBoost))
    }

    if ((status != MagickFalse) && (preserve_gainmap == MagickFalse))
    {
      option = GetImageOption(image_info, "uhdr:target-display-peak-brightness");
      float targetDispPeakBrightness = option != (const char *)NULL ? atof(option) : -1.0f;

      if (targetDispPeakBrightness > 0.0f)

        CHECK_IF_ERR(uhdr_enc_set_target_display_peak_brightness(handle, targetDispPeakBrightness))
    }

    if (status != MagickFalse)
      CHECK_IF_ERR(uhdr_encode(handle))

    if (status != MagickFalse)
    {
      uhdr_compressed_image_t *output = uhdr_get_encoded_stream(handle);

      (void) WriteBlob(image, output->data_sz, output->data);

      uhdr_release_encoder(handle);
    }
  }
#undef CHECK_IF_ERR

  if (CloseBlob(image) == MagickFalse)
    status = MagickFalse;

  if (hdrImgDescriptor.planes[UHDR_PLANE_Y])
    RelinquishMagickMemory(hdrImgDescriptor.planes[UHDR_PLANE_Y]);

  if (sdrImgDescriptor.planes[UHDR_PLANE_Y])
    RelinquishMagickMemory(sdrImgDescriptor.planes[UHDR_PLANE_Y]);

  if (resized_gainmap_profile != (StringInfo *) NULL)
    resized_gainmap_profile=DestroyStringInfo(resized_gainmap_profile);

  if (base_image_profile != (StringInfo *) NULL)
    base_image_profile=DestroyStringInfo(base_image_profile);

  return status;
}
#endif

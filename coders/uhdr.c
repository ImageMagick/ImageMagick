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
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/module.h"
#include "MagickCore/option.h"
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

  const char *option = GetImageOption(image_info, "uhdr:output-transfer-function");
  uhdr_color_transfer_t decoded_img_ct =
      (option != (const char *)NULL) ? map_ct_to_uhdr_ct(option) : UHDR_CT_UNSPECIFIED;

  uhdr_img_fmt_t
    decoded_img_fmt;

  if (decoded_img_ct == UHDR_CT_LINEAR)
    decoded_img_fmt = UHDR_IMG_FMT_64bppRGBAHalfFloat;
  else if (decoded_img_ct == UHDR_CT_HLG || decoded_img_ct == UHDR_CT_PQ)
    decoded_img_fmt = UHDR_IMG_FMT_32bppRGBA1010102;
  else if (decoded_img_ct == UHDR_CT_SRGB)
    decoded_img_fmt = UHDR_IMG_FMT_32bppRGBA8888;
  else
    decoded_img_fmt = UHDR_IMG_FMT_UNSPECIFIED;

#define CHECK_IF_ERR(x) \
  { \
    uhdr_error_info_t retval = (x); \
    if (retval.error_code != UHDR_CODEC_OK) \
    { \
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError, \
        retval.detail,"`%s'",image->filename); \
      uhdr_release_decoder(handle); \
      CloseBlob(image); \
      return DestroyImageList(image); \
    } \
  }

  CHECK_IF_ERR(uhdr_dec_set_image(handle, &img))
  CHECK_IF_ERR(uhdr_dec_set_out_img_format(handle, decoded_img_fmt));
  CHECK_IF_ERR(uhdr_dec_set_out_color_transfer(handle, decoded_img_ct))
  CHECK_IF_ERR(uhdr_dec_probe(handle))

  image->columns = uhdr_dec_get_image_width(handle);
  image->rows = uhdr_dec_get_image_height(handle);

  if (image_info->ping != MagickFalse)
  {
    uhdr_release_decoder(handle);
    status = CloseBlob(image);
    if (status == MagickFalse)
      return (DestroyImageList(image));
    return (GetFirstImageInList(image));
  }

  CHECK_IF_ERR(uhdr_decode(handle))

  uhdr_raw_image_t
    *dst = uhdr_get_decoded_image(handle);

  status = SetImageExtent(image, image->columns, image->rows, exception);
  if (status == MagickFalse)
  {
    uhdr_release_decoder(handle);
    CloseBlob(image);
    return (DestroyImageList(image));
  }

#undef CHECK_IF_ERR

  uhdr_mem_block_t *exif = uhdr_dec_get_exif(handle);
  if (exif != NULL)
  {
    StringInfo *exif_data = AcquireStringInfo(exif->data_sz);
    memcpy(GetStringInfoDatum(exif_data), exif->data, exif->data_sz);
    (void)SetImageProfile(image, "exif", exif_data, exception);
    exif_data = DestroyStringInfo(exif_data);
  }

  SetImageColorspace(image, RGBColorspace, exception);

  if (decoded_img_ct == UHDR_CT_LINEAR)
    image->gamma = 1.0;

  image->compression = JPEGCompression;
  image->depth = (decoded_img_fmt == UHDR_IMG_FMT_32bppRGBA8888) ? 8 : 10;
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
  entry=AcquireMagickInfo("UHDR","UHDR","Ultra HDR Image Format");
#if defined(MAGICKCORE_UHDR_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadUHDRImage;
  entry->encoder=(EncodeImageHandler *) WriteUHDRImage;
#endif
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
  (void) UnregisterMagickInfo("UHDR");
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
    uhdr_profile = {};

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
      (void)LogMagickEvent(CoderEvent, GetMagickModule(), "%s profile: %.20g bytes", name,
                           (double)GetStringInfoLength(profile));
    name = GetNextImageProfile(image);
  }
  return uhdr_profile;
}

static MagickBooleanType WriteUHDRImage(const ImageInfo *image_info,
  Image *images,ExceptionInfo *exception)
{
  Image
    *image = images;

  MagickBooleanType
    status = MagickTrue;

  uhdr_raw_image_t
    hdrImgDescriptor = {0},
    sdrImgDescriptor = {0};

  uhdr_mem_block_t
    sdr_profile, hdr_profile;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent, GetMagickModule(), "%s", image->filename);
  status = OpenBlob(image_info, image, WriteBinaryBlobMode, exception);
  if (status == MagickFalse)
    return (status);

  for (int i = 0; i < GetImageListLength(image); i++)
  {
    const char
      *option;

    int
      bpp = (image->depth >= 10) ? 2 : 1;

    int
      aligned_width = image->columns + (image->columns & 1);

    int
      aligned_height = image->rows + (image->rows & 1);

    ssize_t
      picSize = aligned_width * aligned_height * bpp * 1.5 /* 2x2 sub-sampling */;

    void
      *crBuffer = NULL, *cbBuffer = NULL, *yBuffer = NULL;

    if (image->colorspace != YCbCrColorspace)
    {
      status = TransformImageColorspace(image, YCbCrColorspace, exception);
      if (status == MagickFalse)
        break;
    }

    yBuffer = AcquireMagickMemory(picSize);
    if (yBuffer == NULL)
    {
      status = MagickFalse;
      break;
    }

    if (image->depth >= 10)
    {
      hdrImgDescriptor.fmt = UHDR_IMG_FMT_24bppYCbCrP010;

      option = GetImageOption(image_info, "uhdr:hdr-color-gamut");
      hdrImgDescriptor.cg = (option != (const char *)NULL)
                                ? map_cg_to_uhdr_cg(option)
                                : getImageColorGamut(&image->chromaticity);

      option = GetImageOption(image_info, "uhdr:hdr-color-transfer");
      hdrImgDescriptor.ct =
          (option != (const char *)NULL) ? map_ct_to_uhdr_ct(option) : UHDR_CT_UNSPECIFIED;

      hdrImgDescriptor.range = UHDR_CR_UNSPECIFIED;
      hdrImgDescriptor.w = image->columns;
      hdrImgDescriptor.h = image->rows;
      hdrImgDescriptor.planes[UHDR_PLANE_Y] = yBuffer;
      cbBuffer = ((unsigned short *) yBuffer) + aligned_width * aligned_height;
      crBuffer = ((unsigned short *) cbBuffer) + 1;
      hdrImgDescriptor.planes[UHDR_PLANE_UV] = cbBuffer;
      hdrImgDescriptor.planes[UHDR_PLANE_V] = NULL;
      hdrImgDescriptor.stride[UHDR_PLANE_Y] = aligned_width;
      hdrImgDescriptor.stride[UHDR_PLANE_UV] = aligned_width;
      hdrImgDescriptor.stride[UHDR_PLANE_V] = 0;

      hdr_profile = GetExifProfile(image, exception);
    }
    else if (image->depth == 8)
    {
      sdrImgDescriptor.fmt = UHDR_IMG_FMT_12bppYCbCr420;

      option = GetImageOption(image_info, "uhdr:sdr-color-gamut");
      sdrImgDescriptor.cg = (option != (const char *)NULL)
                                ? map_cg_to_uhdr_cg(option)
                                : getImageColorGamut(&image->chromaticity);

      sdrImgDescriptor.ct = UHDR_CT_SRGB;
      sdrImgDescriptor.range = UHDR_CR_UNSPECIFIED;
      sdrImgDescriptor.w = image->columns;
      sdrImgDescriptor.h = image->rows;
      sdrImgDescriptor.planes[UHDR_PLANE_Y] = yBuffer;
      cbBuffer = ((uint8_t *) yBuffer) + aligned_width * aligned_height;
      sdrImgDescriptor.planes[UHDR_PLANE_U] = cbBuffer;
      crBuffer = ((uint8_t *) cbBuffer) + ((aligned_width / 2) * (aligned_height / 2));
      sdrImgDescriptor.planes[UHDR_PLANE_V] = crBuffer;
      sdrImgDescriptor.stride[UHDR_PLANE_Y] = aligned_width;
      sdrImgDescriptor.stride[UHDR_PLANE_U] = aligned_width / 2;
      sdrImgDescriptor.stride[UHDR_PLANE_V] = aligned_width / 2;

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
        if (image->depth >= 10)
        {
          unsigned short
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
        else if (image->depth == 8)
        {
          unsigned char
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
        p += GetPixelChannels(image);
      }
    }
    if (i != GetImageListLength(image) - 1)
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
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  }

  if (status != MagickFalse)
  {
    uhdr_codec_private_t *handle = uhdr_create_encoder();

#define CHECK_IF_ERR(x) \
  { \
    uhdr_error_info_t retval = (x); \
    if (retval.error_code != UHDR_CODEC_OK) \
    { \
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError, \
        retval.detail,"`%s'",image->filename); \
      uhdr_release_encoder(handle); \
      status = MagickFalse; \
    } \
  }

    if (image->quality > 0 && image->quality <= 100)
      CHECK_IF_ERR(uhdr_enc_set_quality(handle, image->quality, UHDR_BASE_IMG))

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

  return status;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              JJJ  PPPP    222                               %
%                               J   P   P  2   2                              %
%                               J   PPPP     22                               %
%                            J  J   P       2                                 %
%                             JJ    P      22222                              %
%                                                                             %
%                                                                             %
%                     Read/Write JPEG-2000 Image Format                       %
%                                                                             %
%                                   Cristy                                    %
%                                Nathan Brown                                 %
%                                 June 2001                                   %
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
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
#include <openjpeg.h>
#endif

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
static MagickBooleanType
  WriteJP2Image(const ImageInfo *,Image *,ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J 2 K                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJ2K() returns MagickTrue if the image format type, identified by the
%  magick string, is J2K.
%
%  The format of the IsJ2K method is:
%
%      MagickBooleanType IsJ2K(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsJ2K(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\xff\x4f\xff\x51",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J P 2                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJP2() returns MagickTrue if the image format type, identified by the
%  magick string, is JP2.
%
%  The format of the IsJP2 method is:
%
%      MagickBooleanType IsJP2(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsJP2(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\x0d\x0a\x87\x0a",4) == 0)
    return(MagickTrue);
  if (length < 12)
    return(MagickFalse);
  if (memcmp(magick,"\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a",12) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J P 2 I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJP2Image() reads a JPEG 2000 Image file (JP2) or JPEG 2000
%  codestream (JPC) image file and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image or set of images.
%
%  JP2 support is originally written by Nathan Brown, nathanbrown@letu.edu.
%
%  The format of the ReadJP2Image method is:
%
%      Image *ReadJP2Image(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
static void JP2ErrorHandler(const char *message,void *client_data)
{
  ExceptionInfo
    *exception;

  exception=(ExceptionInfo *) client_data;
  (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
    message,"`%s'","OpenJP2");
}

static OPJ_SIZE_T JP2ReadHandler(void *buffer,OPJ_SIZE_T length,void *context)
{
  Image
    *image;

  ssize_t
    count;

  image=(Image *) context;
  count=ReadBlob(image,(ssize_t) length,(unsigned char *) buffer);
  if (count == 0)
    return((OPJ_SIZE_T) -1);
  return((OPJ_SIZE_T) count);
}

static OPJ_BOOL JP2SeekHandler(OPJ_OFF_T offset,void *context)
{
  Image
    *image;

  image=(Image *) context;
  return(SeekBlob(image,offset,SEEK_SET) < 0 ? OPJ_FALSE : OPJ_TRUE);
}

static OPJ_OFF_T JP2SkipHandler(OPJ_OFF_T offset,void *context)
{
  Image
    *image;

  image=(Image *) context;
  return(SeekBlob(image,offset,SEEK_CUR) < 0 ? -1 : offset);
}

static void JP2WarningHandler(const char *message,void *client_data)
{
  ExceptionInfo
    *exception;

  exception=(ExceptionInfo *) client_data;
  (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
    message,"`%s'","OpenJP2");
}

static OPJ_SIZE_T JP2WriteHandler(void *buffer,OPJ_SIZE_T length,void *context)
{
  Image
    *image;

  ssize_t
    count;

  image=(Image *) context;
  count=WriteBlob(image,(ssize_t) length,(unsigned char *) buffer);
  return((OPJ_SIZE_T) count);
}

static Image *ReadJP2Image(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const char
    *option;

  Image
    *image;

  int
    jp2_status;

  MagickBooleanType
    status;

  opj_codec_t
    *jp2_codec;

  opj_codestream_index_t
    *codestream_index = (opj_codestream_index_t *) NULL;

  opj_dparameters_t
    parameters;

  opj_image_t
    *jp2_image;

  opj_stream_t
    *jp2_stream;

  register ssize_t
    i;

  ssize_t
    y;

  unsigned char
    sans[4];

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
    Initialize JP2 codec.
  */
  if (ReadBlob(image,4,sans) != 4)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) SeekBlob(image,SEEK_SET,0);
  if (LocaleCompare(image_info->magick,"JPT") == 0)
    jp2_codec=opj_create_decompress(OPJ_CODEC_JPT);
  else
    if (IsJ2K(sans,4) != MagickFalse)
      jp2_codec=opj_create_decompress(OPJ_CODEC_J2K);
    else
      jp2_codec=opj_create_decompress(OPJ_CODEC_JP2);
  opj_set_warning_handler(jp2_codec,JP2WarningHandler,exception);
  opj_set_error_handler(jp2_codec,JP2ErrorHandler,exception);
  opj_set_default_decoder_parameters(&parameters);
  option=GetImageOption(image_info,"jp2:reduce-factor");
  if (option != (const char *) NULL)
    parameters.cp_reduce=StringToInteger(option);
  option=GetImageOption(image_info,"jp2:quality-layers");
  if (option != (const char *) NULL)
    parameters.cp_layer=StringToInteger(option);
  if (opj_setup_decoder(jp2_codec,&parameters) == 0)
    {
      opj_destroy_codec(jp2_codec);
      ThrowReaderException(DelegateError,"UnableToManageJP2Stream");
    }
  jp2_stream=opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE,1);
  opj_stream_set_read_function(jp2_stream,JP2ReadHandler);
  opj_stream_set_write_function(jp2_stream,JP2WriteHandler);
  opj_stream_set_seek_function(jp2_stream,JP2SeekHandler);
  opj_stream_set_skip_function(jp2_stream,JP2SkipHandler);
  opj_stream_set_user_data(jp2_stream,image,NULL);
  opj_stream_set_user_data_length(jp2_stream,GetBlobSize(image));
  if (opj_read_header(jp2_stream,jp2_codec,&jp2_image) == 0)
    {
      opj_stream_destroy(jp2_stream);
      opj_destroy_codec(jp2_codec);
      ThrowReaderException(DelegateError,"UnableToDecodeImageFile");
    }
  jp2_status=1;
  if ((image->columns != 0) && (image->rows != 0))
    {
      /*
        Extract an area from the image.
      */
      jp2_status=opj_set_decode_area(jp2_codec,jp2_image,
        (OPJ_INT32) image->extract_info.x,(OPJ_INT32) image->extract_info.y,
        (OPJ_INT32) (image->extract_info.x+(ssize_t) image->columns),
        (OPJ_INT32) (image->extract_info.y+(ssize_t) image->rows));
      if (jp2_status == 0)
        {
          opj_stream_destroy(jp2_stream);
          opj_destroy_codec(jp2_codec);
          opj_image_destroy(jp2_image);
          ThrowReaderException(DelegateError,"UnableToDecodeImageFile");
        }
    }
   if ((image_info->number_scenes != 0) && (image_info->scene != 0))
    jp2_status=opj_get_decoded_tile(jp2_codec,jp2_stream,jp2_image,
      (unsigned int) image_info->scene-1);
  else
    if (image->ping == MagickFalse)
      {
        jp2_status=opj_decode(jp2_codec,jp2_stream,jp2_image);
        if (jp2_status != 0)
          jp2_status=opj_end_decompress(jp2_codec,jp2_stream);
      }
  if (jp2_status == 0)
    {
      opj_stream_destroy(jp2_stream);
      opj_destroy_codec(jp2_codec);
      opj_image_destroy(jp2_image);
      ThrowReaderException(DelegateError,"UnableToDecodeImageFile");
    }
  opj_stream_destroy(jp2_stream);
  for (i=0; i < (ssize_t) jp2_image->numcomps; i++)
  {
    if ((jp2_image->comps[0].dx == 0) || (jp2_image->comps[0].dy == 0) ||
        (jp2_image->comps[0].prec != jp2_image->comps[i].prec) ||
        (jp2_image->comps[0].sgnd != jp2_image->comps[i].sgnd) ||
        ((image->ping == MagickFalse) && (jp2_image->comps[i].data == NULL)))
      {
        opj_destroy_codec(jp2_codec);
        opj_image_destroy(jp2_image);
        ThrowReaderException(CoderError,"IrregularChannelGeometryNotSupported")
      }
  }
  /*
    Convert JP2 image.
  */
  image->columns=(size_t) jp2_image->comps[0].w;
  image->rows=(size_t) jp2_image->comps[0].h;
  image->depth=jp2_image->comps[0].prec;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  image->compression=JPEG2000Compression;
  if (jp2_image->numcomps == 1)
    SetImageColorspace(image,GRAYColorspace,exception);
  else
    if (jp2_image->color_space == 2)
      {
        SetImageColorspace(image,GRAYColorspace,exception);
        if (jp2_image->numcomps > 1)
          image->alpha_trait=BlendPixelTrait;
      }
    else
      if (jp2_image->color_space == 3)
        SetImageColorspace(image,Rec601YCbCrColorspace,exception);
  if (jp2_image->numcomps > 3)
    image->alpha_trait=BlendPixelTrait;
  if (jp2_image->icc_profile_buf != (unsigned char *) NULL)
    {
      StringInfo
        *profile;

      profile=BlobToStringInfo(jp2_image->icc_profile_buf,
        jp2_image->icc_profile_len);
      if (profile != (StringInfo *) NULL)
        SetImageProfile(image,"icc",profile,exception);
    }
  if (image->ping != MagickFalse)
    {
      opj_destroy_codec(jp2_codec);
      opj_image_destroy(jp2_image);
      opj_destroy_cstr_index(&codestream_index);
      return(GetFirstImageInList(image));
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) jp2_image->numcomps; i++)
      {
        double
          pixel,
          scale;

        scale=QuantumRange/(double) ((1UL << jp2_image->comps[i].prec)-1);
        pixel=scale*(jp2_image->comps[i].data[y/jp2_image->comps[i].dy*
          image->columns/jp2_image->comps[i].dx+x/jp2_image->comps[i].dx]+
          (jp2_image->comps[i].sgnd ? 1UL << (jp2_image->comps[i].prec-1) : 0));
        switch (i)
        {
           case 0:
           {
             if (jp2_image->numcomps == 1)
               {
                 SetPixelGray(image,ClampToQuantum(pixel),q);
                 SetPixelAlpha(image,OpaqueAlpha,q);
                 break;
               }
             SetPixelRed(image,ClampToQuantum(pixel),q);
             SetPixelGreen(image,ClampToQuantum(pixel),q);
             SetPixelBlue(image,ClampToQuantum(pixel),q);
             SetPixelAlpha(image,OpaqueAlpha,q);
             break;
           }
           case 1:
           {
             if (jp2_image->numcomps == 2)
               {
                 SetPixelAlpha(image,ClampToQuantum(pixel),q);
                 break;
               }
             SetPixelGreen(image,ClampToQuantum(pixel),q);
             break;
           }
           case 2:
           {
             SetPixelBlue(image,ClampToQuantum(pixel),q);
             break;
           }
           case 3:
           {
             SetPixelAlpha(image,ClampToQuantum(pixel),q);
             break;
           }
        }
      }
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  /*
    Free resources.
  */
  opj_destroy_codec(jp2_codec);
  opj_image_destroy(jp2_image);
  opj_destroy_cstr_index(&codestream_index);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r J P 2 I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJP2Image() adds attributes for the JP2 image format to the list of
%  supported formats.  The attributes include the image format tag, a method
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterJP2Image method is:
%
%      size_t RegisterJP2Image(void)
%
*/
ModuleExport size_t RegisterJP2Image(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  (void) FormatLocaleString(version,MagickPathExtent,"%s",opj_version());
#endif
  entry=AcquireMagickInfo("JP2","JP2","JPEG-2000 File Format Syntax");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jp2");
  entry->magick=(IsImageFormatHandler *) IsJP2;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("JP2","J2C","JPEG-2000 Code Stream Syntax");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jp2");
  entry->magick=(IsImageFormatHandler *) IsJ2K;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("JP2","J2K","JPEG-2000 Code Stream Syntax");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jp2");
  entry->magick=(IsImageFormatHandler *) IsJ2K;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("JP2","JPM","JPEG-2000 File Format Syntax");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jp2");
  entry->magick=(IsImageFormatHandler *) IsJP2;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("JP2","JPT","JPEG-2000 File Format Syntax");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jp2");
  entry->magick=(IsImageFormatHandler *) IsJP2;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("JP2","JPC","JPEG-2000 Code Stream Syntax");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jp2");
  entry->magick=(IsImageFormatHandler *) IsJP2;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r J P 2 I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJP2Image() removes format registrations made by the JP2 module
%  from the list of supported formats.
%
%  The format of the UnregisterJP2Image method is:
%
%      UnregisterJP2Image(void)
%
*/
ModuleExport void UnregisterJP2Image(void)
{
  (void) UnregisterMagickInfo("JPC");
  (void) UnregisterMagickInfo("JPT");
  (void) UnregisterMagickInfo("JPM");
  (void) UnregisterMagickInfo("JP2");
  (void) UnregisterMagickInfo("J2K");
}

#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e J P 2 I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJP2Image() writes an image in the JPEG 2000 image format.
%
%  JP2 support originally written by Nathan Brown, nathanbrown@letu.edu
%
%  The format of the WriteJP2Image method is:
%
%      MagickBooleanType WriteJP2Image(const ImageInfo *image_info,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static void CinemaProfileCompliance(const opj_image_t *jp2_image,
  opj_cparameters_t *parameters)
{
  /*
    Digital Cinema 4K profile compliant codestream.
  */
  parameters->tile_size_on=OPJ_FALSE;
  parameters->cp_tdx=1;
  parameters->cp_tdy=1;
  parameters->tp_flag='C';
  parameters->tp_on=1;
  parameters->cp_tx0=0;
  parameters->cp_ty0=0;
  parameters->image_offset_x0=0;
  parameters->image_offset_y0=0;
  parameters->cblockw_init=32;
  parameters->cblockh_init=32;
  parameters->csty|=0x01;
  parameters->prog_order=OPJ_CPRL;
  parameters->roi_compno=(-1);
  parameters->subsampling_dx=1;
  parameters->subsampling_dy=1;
  parameters->irreversible=1;
  if ((jp2_image->comps[0].w == 2048) || (jp2_image->comps[0].h == 1080))
    {
      /*
        Digital Cinema 2K.
      */
      parameters->cp_cinema=OPJ_CINEMA2K_24;
      parameters->cp_rsiz=OPJ_CINEMA2K;
      parameters->max_comp_size=1041666;
      if (parameters->numresolution > 6)
        parameters->numresolution=6;

    }
  if ((jp2_image->comps[0].w == 4096) || (jp2_image->comps[0].h == 2160))
    {
      /*
        Digital Cinema 4K.
      */
      parameters->cp_cinema=OPJ_CINEMA4K_24;
      parameters->cp_rsiz=OPJ_CINEMA4K;
      parameters->max_comp_size=1041666;
      if (parameters->numresolution < 1)
        parameters->numresolution=1;
      if (parameters->numresolution > 7)
        parameters->numresolution=7;
      parameters->numpocs=2;
      parameters->POC[0].tile=1;
      parameters->POC[0].resno0=0;
      parameters->POC[0].compno0=0;
      parameters->POC[0].layno1=1;
      parameters->POC[0].resno1=parameters->numresolution-1;
      parameters->POC[0].compno1=3;
      parameters->POC[0].prg1=OPJ_CPRL;
      parameters->POC[1].tile=1;
      parameters->POC[1].resno0=parameters->numresolution-1;
      parameters->POC[1].compno0=0;
      parameters->POC[1].layno1=1;
      parameters->POC[1].resno1=parameters->numresolution;
      parameters->POC[1].compno1=3;
      parameters->POC[1].prg1=OPJ_CPRL;
    }
  parameters->tcp_numlayers=1;
  parameters->tcp_rates[0]=((float) (jp2_image->numcomps*jp2_image->comps[0].w*
    jp2_image->comps[0].h*jp2_image->comps[0].prec))/(parameters->max_comp_size*
    8*jp2_image->comps[0].dx*jp2_image->comps[0].dy);
  parameters->cp_disto_alloc=1;
}

static MagickBooleanType WriteJP2Image(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *option,
    *property;

  int
    jp2_status;

  MagickBooleanType
    status;

  opj_codec_t
    *jp2_codec;

  OPJ_COLOR_SPACE
    jp2_colorspace;

  opj_cparameters_t
    parameters;

  opj_image_cmptparm_t
    jp2_info[5];

  opj_image_t
    *jp2_image;

  opj_stream_t
    *jp2_stream;

  register ssize_t
    i;

  ssize_t
    y;

  unsigned int
    channels;

  /*
    Open image file.
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
  /*
    Initialize JPEG 2000 API.
  */
  opj_set_default_encoder_parameters(&parameters);
  for (i=1; i < 6; i++)
    if (((size_t) (1UL << (i+2)) > image->columns) &&
        ((size_t) (1UL << (i+2)) > image->rows))
      break;
  parameters.numresolution=i;
  option=GetImageOption(image_info,"jp2:number-resolutions");
  if (option != (const char *) NULL)
    parameters.numresolution=StringToInteger(option);
  parameters.tcp_numlayers=1;
  parameters.tcp_rates[0]=0;  /* lossless */
  parameters.cp_disto_alloc=1;
  if ((image_info->quality != 0) && (image_info->quality != 100))
    {
      parameters.tcp_distoratio[0]=(double) image_info->quality;
      parameters.cp_fixed_quality=OPJ_TRUE;
    }
  if (image_info->extract != (char *) NULL)
    {
      RectangleInfo
        geometry;

      int
        flags;

      /*
        Set tile size.
      */
      flags=ParseAbsoluteGeometry(image_info->extract,&geometry);
      parameters.cp_tdx=(int) geometry.width;
      parameters.cp_tdy=(int) geometry.width;
      if ((flags & HeightValue) != 0)
        parameters.cp_tdy=(int) geometry.height;
      if ((flags & XValue) != 0)
        parameters.cp_tx0=geometry.x;
      if ((flags & YValue) != 0)
        parameters.cp_ty0=geometry.y;
      parameters.tile_size_on=OPJ_TRUE;
    }
  option=GetImageOption(image_info,"jp2:quality");
  if (option != (const char *) NULL)
    {
      register const char
        *p;

      /*
        Set quality PSNR.
      */
      p=option;
      for (i=0; sscanf(p,"%f",&parameters.tcp_distoratio[i]) == 1; i++)
      {
        if (i > 100)
          break;
        while ((*p != '\0') && (*p != ','))
          p++;
        if (*p == '\0')
          break;
        p++;
      }
      parameters.tcp_numlayers=i+1;
      parameters.cp_fixed_quality=OPJ_TRUE;
    }
  option=GetImageOption(image_info,"jp2:progression-order");
  if (option != (const char *) NULL)
    {
      if (LocaleCompare(option,"LRCP") == 0)
        parameters.prog_order=OPJ_LRCP;
      if (LocaleCompare(option,"RLCP") == 0)
        parameters.prog_order=OPJ_RLCP;
      if (LocaleCompare(option,"RPCL") == 0)
        parameters.prog_order=OPJ_RPCL;
      if (LocaleCompare(option,"PCRL") == 0)
        parameters.prog_order=OPJ_PCRL;
      if (LocaleCompare(option,"CPRL") == 0)
        parameters.prog_order=OPJ_CPRL;
    }
  option=GetImageOption(image_info,"jp2:rate");
  if (option != (const char *) NULL)
    {
      register const char
        *p;

      /*
        Set compression rate.
      */
      p=option;
      for (i=0; sscanf(p,"%f",&parameters.tcp_rates[i]) == 1; i++)
      {
        if (i >= 100)
          break;
        while ((*p != '\0') && (*p != ','))
          p++;
        if (*p == '\0')
          break;
        p++;
      }
      parameters.tcp_numlayers=i+1;
      parameters.cp_disto_alloc=OPJ_TRUE;
    }
  if (image_info->sampling_factor != (const char *) NULL)
    (void) sscanf(image_info->sampling_factor,"%d,%d",
      &parameters.subsampling_dx,&parameters.subsampling_dy);
  property=GetImageProperty(image,"comment",exception);
  if (property != (const char *) NULL)
    parameters.cp_comment=(char *) property;
  channels=3;
  jp2_colorspace=OPJ_CLRSPC_SRGB;
  if (image->colorspace == YUVColorspace)
    {
      jp2_colorspace=OPJ_CLRSPC_SYCC;
      parameters.subsampling_dx=2;
    }
  else
    {
      if (IsGrayColorspace(image->colorspace) != MagickFalse)
        {
          channels=1;
          jp2_colorspace=OPJ_CLRSPC_GRAY;
        }
      else
        (void) TransformImageColorspace(image,sRGBColorspace,exception);
      if (image->alpha_trait != UndefinedPixelTrait)
        channels++;
    }
  parameters.tcp_mct=channels == 3 ? 1 : 0;
  ResetMagickMemory(jp2_info,0,sizeof(jp2_info));
  for (i=0; i < (ssize_t) channels; i++)
  {
    jp2_info[i].prec=(OPJ_UINT32) image->depth;
    jp2_info[i].bpp=(OPJ_UINT32) image->depth;
    if ((image->depth == 1) &&
        ((LocaleCompare(image_info->magick,"JPT") == 0) ||
         (LocaleCompare(image_info->magick,"JP2") == 0)))
      {
        jp2_info[i].prec++;  /* OpenJPEG returns exception for depth @ 1 */
        jp2_info[i].bpp++;
      }
    jp2_info[i].sgnd=0;
    jp2_info[i].dx=parameters.subsampling_dx;
    jp2_info[i].dy=parameters.subsampling_dy;
    jp2_info[i].w=(OPJ_UINT32) image->columns;
    jp2_info[i].h=(OPJ_UINT32) image->rows;
  }
  jp2_image=opj_image_create((OPJ_UINT32) channels,jp2_info,jp2_colorspace);
  if (jp2_image == (opj_image_t *) NULL)
    ThrowWriterException(DelegateError,"UnableToEncodeImageFile");
  jp2_image->x0=parameters.image_offset_x0;
  jp2_image->y0=parameters.image_offset_y0;
  jp2_image->x1=(unsigned int) (2*parameters.image_offset_x0+(image->columns-1)*
    parameters.subsampling_dx+1);
  jp2_image->y1=(unsigned int) (2*parameters.image_offset_y0+(image->rows-1)*
    parameters.subsampling_dx+1);
  if ((image->depth == 12) &&
      ((image->columns == 2048) || (image->rows == 1080) ||
       (image->columns == 4096) || (image->rows == 2160)))
    CinemaProfileCompliance(jp2_image,&parameters);
  if (channels == 4)
    jp2_image->comps[3].alpha=1;
  else
   if ((channels == 2) && (jp2_colorspace == OPJ_CLRSPC_GRAY))
     jp2_image->comps[1].alpha=1;
  /*
    Convert to JP2 pixels.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *p;

    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      for (i=0; i < (ssize_t) channels; i++)
      {
        double
          scale;

        register int
          *q;

        scale=(double) ((1UL << jp2_image->comps[i].prec)-1)/QuantumRange;
        q=jp2_image->comps[i].data+(y/jp2_image->comps[i].dy*
          image->columns/jp2_image->comps[i].dx+x/jp2_image->comps[i].dx);
        switch (i)
        {
          case 0:
          {
            if (jp2_colorspace == OPJ_CLRSPC_GRAY)
              {
                *q=(int) (scale*GetPixelGray(image,p));
                break;
              }
            *q=(int) (scale*GetPixelRed(image,p));
            break;
          }
          case 1:
          {
            if (jp2_colorspace == OPJ_CLRSPC_GRAY)
              {
                *q=(int) (scale*GetPixelAlpha(image,p));
                break;
              }
            *q=(int) (scale*GetPixelGreen(image,p));
            break;
          }
          case 2:
          {
            *q=(int) (scale*GetPixelBlue(image,p));
            break;
          }
          case 3:
          {
            *q=(int) (scale*GetPixelAlpha(image,p));
            break;
          }
        }
      }
      p+=GetPixelChannels(image);
    }
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  if (LocaleCompare(image_info->magick,"JPT") == 0)
    jp2_codec=opj_create_compress(OPJ_CODEC_JPT);
  else
    if (LocaleCompare(image_info->magick,"J2K") == 0)
      jp2_codec=opj_create_compress(OPJ_CODEC_J2K);
    else
      jp2_codec=opj_create_compress(OPJ_CODEC_JP2);
  opj_set_warning_handler(jp2_codec,JP2WarningHandler,exception);
  opj_set_error_handler(jp2_codec,JP2ErrorHandler,exception);
  opj_setup_encoder(jp2_codec,&parameters,jp2_image);
  jp2_stream=opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE,OPJ_FALSE);
  opj_stream_set_read_function(jp2_stream,JP2ReadHandler);
  opj_stream_set_write_function(jp2_stream,JP2WriteHandler);
  opj_stream_set_seek_function(jp2_stream,JP2SeekHandler);
  opj_stream_set_skip_function(jp2_stream,JP2SkipHandler);
  opj_stream_set_user_data(jp2_stream,image,NULL);
  if (jp2_stream == (opj_stream_t *) NULL)
    ThrowWriterException(DelegateError,"UnableToEncodeImageFile");
  jp2_status=opj_start_compress(jp2_codec,jp2_image,jp2_stream);
  if (jp2_status == 0)
    ThrowWriterException(DelegateError,"UnableToEncodeImageFile");
  if ((opj_encode(jp2_codec,jp2_stream) == 0) ||
      (opj_end_compress(jp2_codec,jp2_stream) == 0))
    {
      opj_stream_destroy(jp2_stream);
      opj_destroy_codec(jp2_codec);
      opj_image_destroy(jp2_image);
      ThrowWriterException(DelegateError,"UnableToEncodeImageFile");
    }
  /*
    Free resources.
  */
  opj_stream_destroy(jp2_stream);
  opj_destroy_codec(jp2_codec);
  opj_image_destroy(jp2_image);
  (void) CloseBlob(image);
  return(MagickTrue);
}
#endif

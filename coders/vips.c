/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        V   V  IIIII  PPPP   SSSSS                           %
%                        V   V    I    P   P  SS                              %
%                        V   V    I    PPPP    SSS                            %
%                         V V     I    P         SS                           %
%                          V    IIIII  P      SSSSS                           %
%                                                                             %
%                                                                             %
%                        Read/Write VIPS Image Format                         %
%                                                                             %
%                              Software Design                                %
%                                Dirk Lemstra                                 %
%                                 April 2014                                  %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "coders/coders-private.h"

/*
  Define declarations.
*/
#define VIPS_MAGIC_LSB 0x08f2a6b6U
#define VIPS_MAGIC_MSB 0xb6a6f208U

typedef enum
{
  VIPSBandFormatNOTSET    = -1,
  VIPSBandFormatUCHAR     = 0,  /* Unsigned 8-bit int */
  VIPSBandFormatCHAR      = 1,  /* Signed 8-bit int */
  VIPSBandFormatUSHORT    = 2,  /* Unsigned 16-bit int */
  VIPSBandFormatSHORT     = 3,  /* Signed 16-bit int */
  VIPSBandFormatUINT      = 4,  /* Unsigned 32-bit int */
  VIPSBandFormatINT       = 5,  /* Signed 32-bit int */
  VIPSBandFormatFLOAT     = 6,  /* 32-bit IEEE float */
  VIPSBandFormatCOMPLEX   = 7,  /* Complex (2 floats) */
  VIPSBandFormatDOUBLE    = 8,  /* 64-bit IEEE double */
  VIPSBandFormatDPCOMPLEX = 9   /* Complex (2 doubles) */
} VIPSBandFormat;

typedef enum
{
  VIPSCodingNONE  = 0,  /* VIPS computation format */
  VIPSCodingLABQ  = 2,  /* LABQ storage format */
  VIPSCodingRAD   = 6   /* Radiance storage format */
} VIPSCoding;

typedef enum
{
  VIPSTypeMULTIBAND = 0,   /* Some multiband image */
  VIPSTypeB_W       = 1,   /* Some single band image */
  VIPSTypeHISTOGRAM = 10,  /* Histogram or LUT */
  VIPSTypeFOURIER   = 24,  /* Image in Fourier space */
  VIPSTypeXYZ       = 12,  /* CIE XYZ color space */
  VIPSTypeLAB       = 13,  /* CIE LAB color space */
  VIPSTypeCMYK      = 15,  /* im_icc_export() */
  VIPSTypeLABQ      = 16,  /* 32-bit CIE LAB */
  VIPSTypeRGB       = 17,  /* Some RGB */
  VIPSTypeUCS       = 18,  /* UCS(1:1) color space */
  VIPSTypeLCH       = 19,  /* CIE LCh color space */
  VIPSTypeLABS      = 21,  /* 48-bit CIE LAB */
  VIPSTypesRGB      = 22,  /* sRGB color space */
  VIPSTypeYXY       = 23,  /* CIE Yxy color space */
  VIPSTypeRGB16     = 25,  /* 16-bit RGB */
  VIPSTypeGREY16    = 26   /* 16-bit monochrome */
} VIPSType;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteVIPSImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s V I P S                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsVIPS() returns MagickTrue if the image format type, identified by the
%  magick string, is VIPS.
%
%  The format of the IsVIPS method is:
%
%      MagickBooleanType IsVIPS(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsVIPS(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);

  if (memcmp(magick,"\010\362\246\266",4) == 0)
    return(MagickTrue);

  if (memcmp(magick,"\266\246\362\010",4) == 0)
    return(MagickTrue);

  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I P S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadVIPSImage() reads a VIPS image file and returns it. It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the ReadVIPSImage method is:
%
%      Image *ReadVIPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType IsSupportedCombination(
  const VIPSBandFormat format,const VIPSType type)
{
  switch(type)
  {
    case VIPSTypeB_W:
    case VIPSTypeCMYK:
    case VIPSTypeRGB:
    case VIPSTypesRGB:
      return(MagickTrue);
    case VIPSTypeGREY16:
    case VIPSTypeRGB16:
      switch(format)
      {
        case VIPSBandFormatUSHORT:
        case VIPSBandFormatSHORT:
        case VIPSBandFormatUINT:
        case VIPSBandFormatINT:
        case VIPSBandFormatFLOAT:
        case VIPSBandFormatDOUBLE:
          return(MagickTrue);
        default:
          return(MagickFalse);
      }
    default:
      return(MagickFalse);
  }
}

static inline size_t GetVIPSBandSize(const VIPSBandFormat format)
{
  switch(format)
  {
    case VIPSBandFormatUCHAR:
    case VIPSBandFormatCHAR:
      return(1);
    case VIPSBandFormatUSHORT:
    case VIPSBandFormatSHORT:
      return(2);
    case VIPSBandFormatUINT:
    case VIPSBandFormatINT:
    case VIPSBandFormatFLOAT:
      return(4);
    case VIPSBandFormatDOUBLE:
      return(8);
    default:
      return(0);
  }
}

static inline double ReadVIPSResolution(Image *image)
{
  float
    value;

  /*
    Resolution is stored in pixels/mm; reject NaN, negative and absurd values.
  */
  value=ReadBlobFloat(image);
  if ((value != value) || (value < 0.0f) || (value > 1.0e+9f))
    return(0.0);
  return((double) value);
}

static inline Quantum ReadVIPSPixelNONE(Image *image,
  const VIPSBandFormat format,const VIPSType type)
{
  double
    value;

  switch(format)
  {
    case VIPSBandFormatUCHAR:
    case VIPSBandFormatCHAR:
      return(ScaleCharToQuantum((unsigned char) ReadBlobByte(image)));
    case VIPSBandFormatUSHORT:
    case VIPSBandFormatSHORT:
      return(ScaleShortToQuantum(ReadBlobShort(image)));
    case VIPSBandFormatUINT:
    case VIPSBandFormatINT:
      return(ScaleLongToQuantum(ReadBlobLong(image)));
    case VIPSBandFormatFLOAT:
      value=(double) ReadBlobFloat(image);
      break;
    case VIPSBandFormatDOUBLE:
      value=ReadBlobDouble(image);
      break;
    default:
      return((Quantum) 0);
  }
  switch(type)
  {
    case VIPSTypeB_W:
    case VIPSTypeRGB:
      return(ScaleCharToQuantum(CastDoubleToUChar(value)));
    case VIPSTypeGREY16:
    case VIPSTypeRGB16:
      return(ScaleShortToQuantum(CastDoubleToUShort(value)));
    default:
      return(ClampToQuantum((double) QuantumRange*value));
  }
}

static MagickBooleanType ReadVIPSPixelsNONE(Image *image,
  const VIPSBandFormat format,const VIPSType type,const unsigned int channels,
  ExceptionInfo *exception)
{
  Quantum
    pixel;

  Quantum
    *q;

  ssize_t
    x;

  ssize_t
    y;

  unsigned int
    bands;

  for (y = 0; y < (ssize_t) image->rows; y++)
  {
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      return(MagickFalse);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      pixel=ReadVIPSPixelNONE(image,format,type);
      bands=1;
      SetPixelRed(image,pixel,q);
      if (channels < 3)
        {
          SetPixelGreen(image,pixel,q);
          SetPixelBlue(image,pixel,q);
          if (channels == 2)
            {
              SetPixelAlpha(image,ReadVIPSPixelNONE(image,format,type),q);
              bands++;
            }
        }
      else
        {
          SetPixelGreen(image,ReadVIPSPixelNONE(image,format,type),q);
          SetPixelBlue(image,ReadVIPSPixelNONE(image,format,type),q);
          bands+=2;
          if (channels >= 4)
            {
              pixel=ReadVIPSPixelNONE(image,format,type);
              bands++;
              if (image->colorspace == CMYKColorspace)
                {
                  SetPixelBlack(image,pixel,q);
                  if (channels >= 5)
                    {
                      SetPixelAlpha(image,ReadVIPSPixelNONE(image,format,type),
                        q);
                      bands++;
                    }
                }
              else
                SetPixelAlpha(image,pixel,q);
            }
        }
      /*
        Skip any extra bands we have no use for so the stream stays in sync.
      */
      for ( ; bands < channels; bands++)
        (void) ReadVIPSPixelNONE(image,format,type);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      return(MagickFalse);
    if (EOFBlob(image) != MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"UnexpectedEndOfFile","`%s'",image->filename);
        return(MagickFalse);
      }
  }
  return(MagickTrue);
}

static Image *ReadVIPSImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    *metadata;

  ColorspaceType
    colorspace;

  Image
    *image;

  MagickBooleanType
    alpha,
    status;

  ssize_t
    n;

  unsigned int
    channels,
    marker;

  VIPSBandFormat
    format;

  VIPSCoding
    coding;

  VIPSType
    type;

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
  marker=ReadBlobLSBLong(image);
  if (marker == VIPS_MAGIC_LSB)
    image->endian=LSBEndian;
  else
    if (marker == VIPS_MAGIC_MSB)
      image->endian=MSBEndian;
    else
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=(size_t) ReadBlobLong(image);
  image->rows=(size_t) ReadBlobLong(image);
  channels=ReadBlobLong(image);
  (void) ReadBlobLong(image); /* Legacy */
  format=(VIPSBandFormat) ReadBlobLong(image);
  switch(format)
  {
    case VIPSBandFormatUCHAR:
    case VIPSBandFormatCHAR:
      image->depth=8;
      break;
    case VIPSBandFormatUSHORT:
    case VIPSBandFormatSHORT:
      image->depth=16;
      break;
    case VIPSBandFormatUINT:
    case VIPSBandFormatINT:
    case VIPSBandFormatFLOAT:
      image->depth=32;
      break;
    case VIPSBandFormatDOUBLE:
      image->depth=64;
      break;
    default:
    case VIPSBandFormatCOMPLEX:
    case VIPSBandFormatDPCOMPLEX:
    case VIPSBandFormatNOTSET:
      ThrowReaderException(CoderError,"Unsupported band format");
  }
  coding=(VIPSCoding) ReadBlobLong(image);
  type=(VIPSType) ReadBlobLong(image);
  colorspace=UndefinedColorspace;
  alpha=MagickFalse;
  switch(type)
  {
    case VIPSTypeCMYK:
      colorspace=CMYKColorspace;
      alpha=channels == 5 ? MagickTrue : MagickFalse;
      break;
    case VIPSTypeB_W:
    case VIPSTypeGREY16:
      colorspace=GRAYColorspace;
      alpha=channels == 2 ? MagickTrue : MagickFalse;
      break;
    case VIPSTypeRGB:
    case VIPSTypeRGB16:
      colorspace=RGBColorspace;
      alpha=channels == 4 ? MagickTrue : MagickFalse;
      break;
    case VIPSTypesRGB:
      colorspace=sRGBColorspace;
      alpha=channels == 4 ? MagickTrue : MagickFalse;
      break;
    default:
    case VIPSTypeFOURIER:
    case VIPSTypeHISTOGRAM:
    case VIPSTypeLAB:
    case VIPSTypeLABS:
    case VIPSTypeLABQ:
    case VIPSTypeLCH:
    case VIPSTypeMULTIBAND:
    case VIPSTypeUCS:
    case VIPSTypeXYZ:
    case VIPSTypeYXY:
      ThrowReaderException(CoderError,"Unsupported colorspace");
  }
  image->units=PixelsPerCentimeterResolution;
  image->resolution.x=ReadVIPSResolution(image)*10;
  image->resolution.y=ReadVIPSResolution(image)*10;
  /*
    Legacy, offsets, future
  */
  (void) ReadBlobLongLong(image);
  (void) ReadBlobLongLong(image);
  (void) ReadBlobLongLong(image);
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (image_info->ping == MagickFalse)
    {
      /*
        Validate before any pixel cache gets allocated.
      */
      if (IsSupportedCombination(format,type) == MagickFalse)
        ThrowReaderException(CoderError,
          "Unsupported combination of band format and colorspace");
      if (channels == 0 || channels > 5)
        ThrowReaderException(CoderError,"Unsupported number of channels");
      if (coding != VIPSCodingNONE)
        ThrowReaderException(CoderError,"Unsupported coding");
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  (void) SetImageColorspace(image,colorspace,exception);
  if (alpha != MagickFalse)
    image->alpha_trait=BlendPixelTrait;
  (void) SetImageBackgroundColor(image,exception);
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(image);
    }
  status=ReadVIPSPixelsNONE(image,format,type,channels,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  metadata=(char *) NULL;
  while ((n=ReadBlob(image,MagickPathExtent-1,(unsigned char *) buffer)) > 0)
  {
    buffer[n]='\0';
    if (metadata == (char *) NULL)
      metadata=ConstantString(buffer);
    else
      (void) ConcatenateString(&metadata,buffer);
  }
  if (metadata != (char *) NULL)
    {
      (void) SetImageProperty(image,"vips:metadata",metadata,exception);
      metadata=(char *) RelinquishMagickMemory(metadata);
    }
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r V I P S I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterVIPSImage() adds attributes for the VIPS image format to the list
%  of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterVIPSImage method is:
%
%      size_t RegisterVIPSImage(void)
%
*/
ModuleExport size_t RegisterVIPSImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("VIPS","VIPS","VIPS image");
  entry->decoder=(DecodeImageHandler *) ReadVIPSImage;
  entry->encoder=(EncodeImageHandler *) WriteVIPSImage;
  entry->magick=(IsImageFormatHandler *) IsVIPS;
  entry->flags|=CoderEndianSupportFlag;
  entry->flags&=~CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r V I P S I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterVIPSImage() removes format registrations made by the
%  VIPS module from the list of supported formats.
%
%  The format of the UnregisterVIPSImage method is:
%
%      UnregisterVIPSImage(void)
%
*/
ModuleExport void UnregisterVIPSImage(void)
{
  (void) UnregisterMagickInfo("VIPS");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I P S I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteVIPSImage() writes an image to a file in VIPS image format.
%
%  The format of the WriteVIPSImage method is:
%
%      MagickBooleanType WriteVIPSImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline unsigned char *WriteVIPSPixel(Image *image,const Quantum value,
  unsigned char *q)
{
  if (image->depth == 16)
    {
      unsigned short
        pixel;

      pixel=ScaleQuantumToShort(value);
      if (image->endian == LSBEndian)
        {
          *q++=(unsigned char) pixel;
          *q++=(unsigned char) (pixel >> 8);
        }
      else
        {
          *q++=(unsigned char) (pixel >> 8);
          *q++=(unsigned char) pixel;
        }
    }
  else
    *q++=ScaleQuantumToChar(value);
  return(q);
}

static MagickBooleanType WriteVIPSImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const char
    *metadata;

  const Quantum
    *p;

  MagickBooleanType
    status;

  size_t
    length;

  ssize_t
    x;

  ssize_t
    y;

  unsigned int
    channels;

  unsigned char
    *pixels,
    *q;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) SetImageStorageClass(image,DirectClass,exception);
  switch (image->colorspace)
  {
    case CMYKColorspace:
    case GRAYColorspace:
    case LabColorspace:
    case LCHColorspace:
    case RGBColorspace:
    case XYZColorspace:
    case sRGBColorspace:
      break;
    default:
      /*
        Colorspaces VIPS has no type for are converted, not just relabeled.
      */
      (void) TransformImageColorspace(image,sRGBColorspace,exception);
      break;
  }
  if (image->endian == LSBEndian)
    (void) WriteBlobLSBLong(image,VIPS_MAGIC_LSB);
  else
    (void) WriteBlobLSBLong(image,VIPS_MAGIC_MSB);
  (void) WriteBlobLong(image,(unsigned int) image->columns);
  (void) WriteBlobLong(image,(unsigned int) image->rows);
  channels=image->alpha_trait != UndefinedPixelTrait ? 4 : 3;
  if (IdentifyImageCoderGray(image,exception) != MagickFalse)
    {
      channels=image->alpha_trait != UndefinedPixelTrait ? 2 : 1;
      (void) SetImageColorspace(image,GRAYColorspace,exception);
    }
  else
    if (image->colorspace == CMYKColorspace)
      channels=image->alpha_trait != UndefinedPixelTrait ? 5 : 4;
  /*
    Only 8 and 16 bit samples are written; anything deeper keeps 16 bits.
  */
  image->depth=image->depth > 8 ? 16 : 8;
  (void) WriteBlobLong(image,channels);
  (void) WriteBlobLong(image,0);
  if (image->depth == 16)
    (void) WriteBlobLong(image,(unsigned int) VIPSBandFormatUSHORT);
  else
    (void) WriteBlobLong(image,(unsigned int) VIPSBandFormatUCHAR);
  (void) WriteBlobLong(image,VIPSCodingNONE);
  switch (image->colorspace)
  {
    case CMYKColorspace:
      (void) WriteBlobLong(image,VIPSTypeCMYK);
      break;
    case GRAYColorspace:
      if (image->depth == 16)
        (void) WriteBlobLong(image,VIPSTypeGREY16);
      else
        (void) WriteBlobLong(image,VIPSTypeB_W);
      break;
    case LabColorspace:
      (void) WriteBlobLong(image,VIPSTypeLAB);
      break;
    case LCHColorspace:
      (void) WriteBlobLong(image,VIPSTypeLCH);
      break;
    case RGBColorspace:
      if (image->depth == 16)
        (void) WriteBlobLong(image,VIPSTypeRGB16);
      else
        (void) WriteBlobLong(image,VIPSTypeRGB);
      break;
    case XYZColorspace:
      (void) WriteBlobLong(image,VIPSTypeXYZ);
      break;
    case sRGBColorspace:
    default:
      (void) SetImageColorspace(image,sRGBColorspace,exception);
      (void) WriteBlobLong(image,VIPSTypesRGB);
      break;
  }
  if (image->units == PixelsPerCentimeterResolution)
    {
      (void) WriteBlobFloat(image,(float) (image->resolution.x/10));
      (void) WriteBlobFloat(image,(float) (image->resolution.y/10));
    }
  else
    if (image->units == PixelsPerInchResolution)
      {
        (void) WriteBlobFloat(image,(float) (image->resolution.x/25.4));
        (void) WriteBlobFloat(image,(float) (image->resolution.y/25.4));
      }
    else
      {
        (void) WriteBlobLong(image,0);
        (void) WriteBlobLong(image,0);
      }
  /*
    Legacy, offsets, future.
  */
  (void) WriteBlobPadding(image,24,'\0');
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns,
    channels*(image->depth/8)*sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  length=image->columns*channels*(image->depth/8);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        break;
      }
    q=pixels;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      q=WriteVIPSPixel(image,GetPixelRed(image,p),q);
      if (channels == 2)
        q=WriteVIPSPixel(image,GetPixelAlpha(image,p),q);
      else
        {
          if (channels >= 3)
            {
              q=WriteVIPSPixel(image,GetPixelGreen(image,p),q);
              q=WriteVIPSPixel(image,GetPixelBlue(image,p),q);
            }
          if (channels >= 4)
            {
              if (image->colorspace == CMYKColorspace)
                {
                  q=WriteVIPSPixel(image,GetPixelBlack(image,p),q);
                  if (channels == 5)
                    q=WriteVIPSPixel(image,GetPixelAlpha(image,p),q);
                }
              else
                q=WriteVIPSPixel(image,GetPixelAlpha(image,p),q);
            }
        }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (WriteBlob(image,length,pixels) != (ssize_t) length)
      {
        status=MagickFalse;
        break;
      }
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  metadata=GetImageProperty(image,"vips:metadata",exception);
  if ((status != MagickFalse) && (metadata != (const char*) NULL))
    (void) WriteBlobString(image,metadata);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

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
%  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Define declaractions.
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
  VIPSTypeXYZ       = 12,  /* CIE XYZ colour space */
  VIPSTypeLAB       = 13,  /* CIE LAB colour space */
  VIPSTypeCMYK      = 15,  /* im_icc_export() */
  VIPSTypeLABQ      = 16,  /* 32-bit CIE LAB */
  VIPSTypeRGB       = 17,  /* Some RGB */
  VIPSTypeUCS       = 18,  /* UCS(1:1) colour space */
  VIPSTypeLCH       = 19,  /* CIE LCh colour space */
  VIPSTypeLABS      = 21,  /* 48-bit CIE LAB */
  VIPSTypesRGB      = 22,  /* sRGB colour space */
  VIPSTypeYXY       = 23,  /* CIE Yxy colour space */
  VIPSTypeRGB16     = 25,  /* 16-bit RGB */
  VIPSTypeGREY16    = 26   /* 16-bit monochrome */
} VIPSType;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteVIPSImage(const ImageInfo *,Image *);

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
%      Image *ReadVIPSmage(const ImageInfo *image_info,ExceptionInfo *exception)
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

static inline Quantum ReadVIPSPixelNONE(Image *image,
  const VIPSBandFormat format,const VIPSType type)
{
  switch(type)
  {
    case VIPSTypeB_W:
    case VIPSTypeRGB:
      {
        unsigned char
          c;

        switch(format)
        {
          case VIPSBandFormatUCHAR:
          case VIPSBandFormatCHAR:
            c=(unsigned char) ReadBlobByte(image);
            break;
          case VIPSBandFormatUSHORT:
          case VIPSBandFormatSHORT:
            c=(unsigned char) ReadBlobShort(image);
            break;
          case VIPSBandFormatUINT:
          case VIPSBandFormatINT:
            c=(unsigned char) ReadBlobLong(image);
            break;
          case VIPSBandFormatFLOAT:
            c=(unsigned char) ReadBlobFloat(image);
            break;
          case VIPSBandFormatDOUBLE:
            c=(unsigned char) ReadBlobDouble(image);
            break;
          default:
            c=0;
            break;
        }
        return(ScaleCharToQuantum(c));
      }
    case VIPSTypeGREY16:
    case VIPSTypeRGB16:
      {
        unsigned short
          s;

        switch(format)
        {
          case VIPSBandFormatUSHORT:
          case VIPSBandFormatSHORT:
            s=(unsigned short) ReadBlobShort(image);
            break;
          case VIPSBandFormatUINT:
          case VIPSBandFormatINT:
            s=(unsigned short) ReadBlobLong(image);
            break;
          case VIPSBandFormatFLOAT:
            s=(unsigned short) ReadBlobFloat(image);
            break;
          case VIPSBandFormatDOUBLE:
            s=(unsigned short) ReadBlobDouble(image);
            break;
          default:
            s=0;
            break;
        }
        return(ScaleShortToQuantum(s));
      }
    case VIPSTypeCMYK:
    case VIPSTypesRGB:
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
          return((Quantum) ((float) QuantumRange*(ReadBlobFloat(image)/1.0)));
        case VIPSBandFormatDOUBLE:
          return((Quantum) ((double) QuantumRange*(ReadBlobDouble(
            image)/1.0)));
        default:
          return((Quantum) 0);
      }
    default:
      return((Quantum) 0);
  }
}

static MagickBooleanType ReadVIPSPixelsNONE(Image *image,
  const VIPSBandFormat format,const VIPSType type,const unsigned int channels,
  ExceptionInfo *exception)
{
  Quantum
    pixel;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    x;

  ssize_t
    y;

  for (y = 0; y < (ssize_t) image->rows; y++)
  {
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      return MagickFalse;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      pixel=ReadVIPSPixelNONE(image,format,type);
      SetPixelRed(q,pixel);
      if (channels < 3)
        {
          SetPixelGreen(q,pixel);
          SetPixelBlue(q,pixel);
          if (channels == 2)
            SetPixelAlpha(q,ReadVIPSPixelNONE(image,format,type));
        }
      else
        {
          SetPixelGreen(q,ReadVIPSPixelNONE(image,format,type));
          SetPixelBlue(q,ReadVIPSPixelNONE(image,format,type));
          if (channels == 4)
            {
              if (image->colorspace == CMYKColorspace)
                SetPixelIndex(indexes+x,ReadVIPSPixelNONE(image,format,type))
              else
                SetPixelAlpha(q,ReadVIPSPixelNONE(image,format,type));
            }
          else if (channels == 5)
            {
              SetPixelIndex(indexes+x,ReadVIPSPixelNONE(image,format,type));
              SetPixelAlpha(q,ReadVIPSPixelNONE(image,format,type));
            }
        }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      return MagickFalse;
  }
  return(MagickTrue);
}

static Image *ReadVIPSImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    buffer[MaxTextExtent],
    *metadata;

  Image
    *image;

  MagickBooleanType
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
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  marker=ReadBlobLSBLong(image);
  if (marker == VIPS_MAGIC_LSB)
    image->endian=LSBEndian;
  else if (marker == VIPS_MAGIC_MSB)
    image->endian=MSBEndian;
  else
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=(size_t) ReadBlobLong(image);
  image->rows=(size_t) ReadBlobLong(image);
  status=SetImageExtent(image,image->columns,image->rows);
  if (status == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(DestroyImageList(image));
    }
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
  switch(type)
  {
    case VIPSTypeCMYK:
      SetImageColorspace(image,CMYKColorspace);
      if (channels == 5)
        image->matte=MagickTrue;
      break;
    case VIPSTypeB_W:
    case VIPSTypeGREY16:
      SetImageColorspace(image,GRAYColorspace);
      if (channels == 2)
        image->matte=MagickTrue;
      break;
    case VIPSTypeRGB:
    case VIPSTypeRGB16:
      SetImageColorspace(image,RGBColorspace);
      if (channels == 4)
        image->matte=MagickTrue;
      break;
    case VIPSTypesRGB:
      SetImageColorspace(image,sRGBColorspace);
      if (channels == 4)
        image->matte=MagickTrue;
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
  image->x_resolution=ReadBlobFloat(image)*10;
  image->y_resolution=ReadBlobFloat(image)*10;
  /*
    Legacy, offsets, future
  */
  (void) ReadBlobLongLong(image);
  (void) ReadBlobLongLong(image);
  (void) ReadBlobLongLong(image);
  if (image_info->ping != MagickFalse)
    return(image);
  if (IsSupportedCombination(format,type) == MagickFalse)
    ThrowReaderException(CoderError,
      "Unsupported combination of band format and colorspace");
  if (channels == 0 || channels > 5)
    ThrowReaderException(CoderError,"Unsupported number of channels");
  if (coding == VIPSCodingNONE)
    status=ReadVIPSPixelsNONE(image,format,type,channels,exception);
  else
    ThrowReaderException(CoderError,"Unsupported coding");
  metadata=(char *) NULL;
  while ((n=ReadBlob(image,MaxTextExtent-1,(unsigned char *) buffer)) != 0)
  {
    buffer[n]='\0';
    if (metadata == (char *) NULL)
      metadata=ConstantString(buffer);
    else
      (void) ConcatenateString(&metadata,buffer);
  }
  if (metadata != (char *) NULL)
    SetImageProperty(image,"vips:metadata",metadata);
  (void) CloseBlob(image);
  if (status == MagickFalse)
    return((Image *) NULL);
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
%  RegisterVIPSmage() adds attributes for the VIPS image format to the list
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

  entry=SetMagickInfo("VIPS");
  entry->decoder=(DecodeImageHandler *) ReadVIPSImage;
  entry->encoder=(EncodeImageHandler *) WriteVIPSImage;
  entry->magick=(IsImageFormatHandler *) IsVIPS;
  entry->description=ConstantString("VIPS image");
  entry->endian_support=MagickTrue;
  entry->module=ConstantString("VIPS");
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
%      MagickBooleanType WriteVIPSImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline void WriteVIPSPixel(Image *image, const Quantum value)
{
  if (image->depth == 16)
    (void) WriteBlobShort(image,ScaleQuantumToShort(value));
  else
    (void) WriteBlobByte(image,ScaleQuantumToChar(value));
}

static MagickBooleanType WriteVIPSImage(const ImageInfo *image_info,
  Image *image)
{
  const char
    *metadata;

  MagickBooleanType
    status;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    x;

  ssize_t
    y;

  unsigned int
    channels;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  if (image->endian == LSBEndian)
    (void) WriteBlobLSBLong(image,VIPS_MAGIC_LSB);
  else
    (void) WriteBlobLSBLong(image,VIPS_MAGIC_MSB);
  (void) WriteBlobLong(image,(unsigned int) image->columns);
  (void) WriteBlobLong(image,(unsigned int) image->rows);
  (void) SetImageStorageClass(image,DirectClass);
  channels=image->matte ? 4 : 3;
  if (IsGrayImage(image,&image->exception) != MagickFalse)
    channels=image->matte ? 2 : 1;
  else if (image->colorspace == CMYKColorspace)
    channels=image->matte ? 5 : 4;
  (void) WriteBlobLong(image,channels);
  (void) WriteBlobLong(image,0);
  if (image->depth == 16)
    (void) WriteBlobLong(image,(unsigned int) VIPSBandFormatUSHORT);
  else
    {
      image->depth=8;
      (void) WriteBlobLong(image,(unsigned int) VIPSBandFormatUCHAR);
    }
  (void) WriteBlobLong(image,VIPSCodingNONE);
  switch(image->colorspace)
  {
    case CMYKColorspace:
      (void) WriteBlobLong(image,VIPSTypeCMYK);
      break;
    case GRAYColorspace:
      if (image->depth == 16)
        (void) WriteBlobLong(image, VIPSTypeGREY16);
      else
        (void) WriteBlobLong(image, VIPSTypeB_W);
      break;
    case RGBColorspace:
      if (image->depth == 16)
        (void) WriteBlobLong(image, VIPSTypeRGB16);
      else
        (void) WriteBlobLong(image, VIPSTypeRGB);
      break;
    default:
    case sRGBColorspace:
      (void) SetImageColorspace(image,sRGBColorspace);
      (void) WriteBlobLong(image,VIPSTypesRGB);
      break;
  }
  if (image->units == PixelsPerCentimeterResolution)
    {
      (void) WriteBlobFloat(image,(image->x_resolution / 10));
      (void) WriteBlobFloat(image,(image->y_resolution / 10));
    }
  else if (image->units == PixelsPerInchResolution)
    {
      (void) WriteBlobFloat(image,(image->x_resolution / 25.4));
      (void) WriteBlobFloat(image,(image->y_resolution / 25.4));
    }
  else
    {
      (void) WriteBlobLong(image,0);
      (void) WriteBlobLong(image,0);
    }
  /*
    Legacy, Offsets, Future
  */
  for (y=0; y < 24; y++)
    (void) WriteBlobByte(image,0);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      WriteVIPSPixel(image,GetPixelRed(p));
      if (channels == 2)
        WriteVIPSPixel(image,GetPixelAlpha(p));
      else
        {
          WriteVIPSPixel(image,GetPixelGreen(p));
          WriteVIPSPixel(image,GetPixelBlue(p));
          if (channels >= 4)
            {
              if (image->colorspace == CMYKColorspace)
                WriteVIPSPixel(image,GetPixelIndex(indexes+x));
              else
                WriteVIPSPixel(image,GetPixelAlpha(p));
            }
          else if (channels == 5)
            {
               WriteVIPSPixel(image,GetPixelIndex(indexes+x));
               WriteVIPSPixel(image,GetPixelAlpha(p));
            }
        }
      p++;
    }
  }
  metadata=GetImageProperty(image,"vips:metadata");
  if (metadata != (const char*) NULL)
    WriteBlobString(image,metadata);
  (void) CloseBlob(image);
  return(status);
}

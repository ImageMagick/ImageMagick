/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        FFFFF  IIIII  TTTTT  SSSSS                           %
%                        F        I      T    SS                              %
%                        FFF      I      T     SSS                            %
%                        F        I      T       SS                           %
%                        F      IIIII    T    SSSSS                           %
%                                                                             %
%                                                                             %
%            Read/Write Flexible Image Transport System Images.               %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/module.h"

/*
  Forward declarations.
*/
#define FITSBlocksize  2880UL

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteFITSImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s F I T S                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFITS() returns MagickTrue if the image format type, identified by the
%  magick string, is FITS.
%
%  The format of the IsFITS method is:
%
%      MagickBooleanType IsFITS(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsFITS(const unsigned char *magick,const size_t length)
{
  if (length < 6)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"IT0",3) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick,"SIMPLE",6) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F I T S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFITSImage() reads a FITS image file and returns it.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the ReadFITSImage method is:
%
%      Image *ReadFITSImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double GetFITSPixel(Image *image,int bits_per_pixel)
{
  switch (image->depth >> 3)
  {
    case 1:
      return((double) ReadBlobByte(image));
    case 2:
      return((double) ((short) ReadBlobShort(image)));
    case 4:
    {
      if (bits_per_pixel > 0)
        return((double) ((int) ReadBlobLong(image)));
      return((double) ReadBlobFloat(image));
    }
    case 8:
    {
      if (bits_per_pixel > 0)
        return((double) ((MagickOffsetType) ReadBlobLongLong(image)));
    }
    default:
      break;
  }
  return(ReadBlobDouble(image));
}

static void GetFITSPixelExtrema(Image *image,const int bits_per_pixel,
  double *minima,double *maxima)
{
  double
    pixel;

  MagickOffsetType
    offset;

  MagickSizeType
    number_pixels;

  register MagickOffsetType
    i;

  offset=TellBlob(image);
  number_pixels=(MagickSizeType) image->columns*image->rows;
  *minima=GetFITSPixel(image,bits_per_pixel);
  *maxima=(*minima);
  for (i=1; i < (MagickOffsetType) number_pixels; i++)
  {
    pixel=GetFITSPixel(image,bits_per_pixel);
    if (pixel < *minima)
      *minima=pixel;
    if (pixel > *maxima)
      *maxima=pixel;
  }
  (void) SeekBlob(image,offset,SEEK_SET);
}

static inline double GetFITSPixelRange(const size_t depth)
{
  return((double) ((MagickOffsetType) GetQuantumRange(depth)));
}

static void SetFITSUnsignedPixels(const size_t length,
  const size_t bits_per_pixel,unsigned char *pixels)
{
  register ssize_t
    i;

  for (i=0; i < (ssize_t) length; i++)
  {
    *pixels^=0x80;
    pixels+=bits_per_pixel >> 3;
  }
}

static Image *ReadFITSImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  typedef struct _FITSInfo
  {
    MagickBooleanType
      extend,
      simple;

    int
      bits_per_pixel,
      columns,
      rows,
      number_axes,
      number_planes;

    double
      min_data,
      max_data,
      zero,
      scale;

    EndianType
      endian;
  } FITSInfo;

  char
    *comment,
    keyword[9],
    property[MaxTextExtent],
    value[73];

  double
    pixel,
    scale;

  FITSInfo
    fits_info;

  Image
    *image;

  int
    c;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  register ssize_t
    i,
    x;

  register PixelPacket
    *q;

  ssize_t
    count,
    scene,
    y;

  /*
    Open image file.
  */
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
  /*
    Initialize image header.
  */
  (void) ResetMagickMemory(&fits_info,0,sizeof(fits_info));
  fits_info.extend=MagickFalse;
  fits_info.simple=MagickFalse;
  fits_info.bits_per_pixel=8;
  fits_info.columns=1;
  fits_info.rows=1;
  fits_info.rows=1;
  fits_info.number_planes=1;
  fits_info.min_data=0.0;
  fits_info.max_data=0.0;
  fits_info.zero=0.0;
  fits_info.scale=1.0;
  fits_info.endian=MSBEndian;
  /*
    Decode image header.
  */
  for (comment=(char *) NULL; EOFBlob(image) == MagickFalse; )
  {
    for ( ; EOFBlob(image) == MagickFalse; )
    {
      register char
        *p;

      count=ReadBlob(image,8,(unsigned char *) keyword);
      if (count != 8)
        break;
      for (i=0; i < 8; i++)
      {
        if (isspace((int) ((unsigned char) keyword[i])) != 0)
          break;
        keyword[i]=tolower((int) ((unsigned char) keyword[i]));
      }
      keyword[i]='\0';
      count=ReadBlob(image,72,(unsigned char *) value);
      if (count != 72)
        break;
      value[72]='\0';
      p=value;
      if (*p == '=')
        {
          p+=2;
          while (isspace((int) ((unsigned char) *p)) != 0)
            p++;
        }
      if (LocaleCompare(keyword,"end") == 0)
        break;
      if (LocaleCompare(keyword,"extend") == 0)
        fits_info.extend=(*p == 'T') || (*p == 't') ? MagickTrue : MagickFalse;
      if (LocaleCompare(keyword,"simple") == 0)
        fits_info.simple=(*p == 'T') || (*p == 't') ? MagickTrue : MagickFalse;
      if (LocaleCompare(keyword,"bitpix") == 0)
        fits_info.bits_per_pixel=StringToLong(p);
      if (LocaleCompare(keyword,"naxis") == 0)
        fits_info.number_axes=StringToLong(p);
      if (LocaleCompare(keyword,"naxis1") == 0)
        fits_info.columns=StringToLong(p);
      if (LocaleCompare(keyword,"naxis2") == 0)
        fits_info.rows=StringToLong(p);
      if (LocaleCompare(keyword,"naxis3") == 0)
        fits_info.number_planes=StringToLong(p);
      if (LocaleCompare(keyword,"datamax") == 0)
        fits_info.max_data=StringToDouble(p,(char **) NULL);
      if (LocaleCompare(keyword,"datamin") == 0)
        fits_info.min_data=StringToDouble(p,(char **) NULL);
      if (LocaleCompare(keyword,"bzero") == 0)
        fits_info.zero=StringToDouble(p,(char **) NULL);
      if (LocaleCompare(keyword,"bscale") == 0)
        fits_info.scale=StringToDouble(p,(char **) NULL);
      if (LocaleCompare(keyword,"comment") == 0)
        {
          if (comment == (char *) NULL)
            comment=ConstantString(p);
          else
            (void) ConcatenateString(&comment,p);
        }
      if (LocaleCompare(keyword,"xendian") == 0)
        {
          if (LocaleNCompare(p,"big",3) == 0)
            fits_info.endian=MSBEndian;
          else
            fits_info.endian=LSBEndian;
        }
      (void) FormatLocaleString(property,MaxTextExtent,"fits:%s",keyword);
      (void) SetImageProperty(image,property,p);
    }
    c=0;
    while (((TellBlob(image) % FITSBlocksize) != 0) && (c != EOF))
      c=ReadBlobByte(image);
    if (fits_info.extend == MagickFalse)
      break;
    number_pixels=(MagickSizeType) fits_info.columns*fits_info.rows;
    if ((fits_info.simple != MagickFalse) && (fits_info.number_axes >= 1) &&
        (fits_info.number_axes <= 4) && (number_pixels != 0))
      break;
  }
  /*
    Verify that required image information is defined.
  */
  if (comment != (char *) NULL)
    {
      (void) SetImageProperty(image,"comment",comment);
      comment=DestroyString(comment);
    }
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  number_pixels=(MagickSizeType) fits_info.columns*fits_info.rows;
  if ((fits_info.simple == MagickFalse) || (fits_info.number_axes < 1) ||
      (fits_info.number_axes > 4) || (number_pixels == 0))
    ThrowReaderException(CorruptImageError,"ImageTypeNotSupported");
  for (scene=0; scene < (ssize_t) fits_info.number_planes; scene++)
  {
    image->columns=(size_t) fits_info.columns;
    image->rows=(size_t) fits_info.rows;
    image->depth=(size_t) (fits_info.bits_per_pixel < 0 ? -1 : 1)*
      fits_info.bits_per_pixel;
    image->endian=fits_info.endian;
    image->scene=(size_t) scene;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Initialize image structure.
    */
    SetImageColorspace(image,GRAYColorspace);
    if ((fits_info.min_data != 0.0) || (fits_info.max_data != 0.0))
      {
        if ((fits_info.bits_per_pixel != 0) && (fits_info.max_data == 0.0))
          fits_info.max_data=GetFITSPixelRange((size_t)
            fits_info.bits_per_pixel);
      }
    else
      GetFITSPixelExtrema(image,fits_info.bits_per_pixel,&fits_info.min_data,
        &fits_info.max_data);
    /*
      Convert FITS pixels to pixel packets.
    */
    scale=(double) QuantumRange/(fits_info.scale*(fits_info.max_data-
      fits_info.min_data)+fits_info.zero);
    for (y=(ssize_t) image->rows-1; y >= 0; y--)
    {
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        pixel=GetFITSPixel(image,fits_info.bits_per_pixel);
        SetPixelRed(q,ClampToQuantum(scale*(fits_info.scale*(pixel-
          fits_info.min_data)+fits_info.zero)));
        SetPixelGreen(q,GetPixelRed(q));
        SetPixelBlue(q,GetPixelRed(q));
        q++;
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
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (scene < (ssize_t) (fits_info.number_planes-1))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r F I T S I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterFITSImage() adds attributes for the FITS image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterFITSImage method is:
%
%      size_t RegisterFITSImage(void)
%
*/
ModuleExport size_t RegisterFITSImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("FITS");
  entry->decoder=(DecodeImageHandler *) ReadFITSImage;
  entry->encoder=(EncodeImageHandler *) WriteFITSImage;
  entry->magick=(IsImageFormatHandler *) IsFITS;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Flexible Image Transport System");
  entry->module=ConstantString("FITS");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("FTS");
  entry->decoder=(DecodeImageHandler *) ReadFITSImage;
  entry->encoder=(EncodeImageHandler *) WriteFITSImage;
  entry->magick=(IsImageFormatHandler *) IsFITS;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Flexible Image Transport System");
  entry->module=ConstantString("FTS");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r F I T S I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterFITSImage() removes format registrations made by the
%  FITS module from the list of supported formats.
%
%  The format of the UnregisterFITSImage method is:
%
%      UnregisterFITSImage(void)
%
*/
ModuleExport void UnregisterFITSImage(void)
{
  (void) UnregisterMagickInfo("FITS");
  (void) UnregisterMagickInfo("FTS");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F I T S I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFITSImage() writes a Flexible Image Transport System image to a
%  file as gray scale intensities [0..255].
%
%  The format of the WriteFITSImage method is:
%
%      MagickBooleanType WriteFITSImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteFITSImage(const ImageInfo *image_info,
  Image *image)
{
  char
    header[FITSBlocksize],
    *fits_info;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  register const PixelPacket
    *p;

  size_t
    length;

  ssize_t
    count,
    offset,
    y;

  unsigned char
    *pixels;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  /*
    Allocate image memory.
  */
  fits_info=(char *) AcquireQuantumMemory(FITSBlocksize,sizeof(*fits_info));
  if (fits_info == (char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(fits_info,' ',FITSBlocksize*sizeof(*fits_info));
  /*
    Initialize image header.
  */
  image->depth=GetImageQuantumDepth(image,MagickFalse);
  quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  offset=0;
  (void) FormatLocaleString(header,FITSBlocksize,
    "SIMPLE  =                    T");
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) FormatLocaleString(header,FITSBlocksize,"BITPIX  =           %10ld",
    (long) (quantum_info->format == FloatingPointQuantumFormat ? -1 : 1)*
    image->depth);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) FormatLocaleString(header,FITSBlocksize,"NAXIS   =           %10lu",
    IsGrayImage(image,&image->exception) != MagickFalse ? 2UL : 3UL);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) FormatLocaleString(header,FITSBlocksize,"NAXIS1  =           %10lu",
    (unsigned long) image->columns);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) FormatLocaleString(header,FITSBlocksize,"NAXIS2  =           %10lu",
    (unsigned long) image->rows);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  if (IsGrayImage(image,&image->exception) == MagickFalse)
    {
      (void) FormatLocaleString(header,FITSBlocksize,
        "NAXIS3  =           %10lu",3UL);
      (void) strncpy(fits_info+offset,header,strlen(header));
      offset+=80;
    }
  (void) FormatLocaleString(header,FITSBlocksize,"BSCALE  =         %E",1.0);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) FormatLocaleString(header,FITSBlocksize,"BZERO   =         %E",
    image->depth > 8 ? GetFITSPixelRange(image->depth) : 0.0);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) FormatLocaleString(header,FITSBlocksize,"DATAMAX =         %E",
    1.0*((MagickOffsetType) GetQuantumRange(image->depth)));
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) FormatLocaleString(header,FITSBlocksize,"DATAMIN =         %E",0.0);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  if (image->endian == LSBEndian)
    {
      (void) FormatLocaleString(header,FITSBlocksize,"XENDIAN = 'SMALL'");
      (void) strncpy(fits_info+offset,header,strlen(header));
      offset+=80;
    }
  (void) FormatLocaleString(header,FITSBlocksize,"HISTORY %.72s",
    GetMagickVersion((size_t *) NULL));
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) strncpy(header,"END",FITSBlocksize);
  (void) strncpy(fits_info+offset,header,strlen(header));
  offset+=80;
  (void) WriteBlob(image,FITSBlocksize,(unsigned char *) fits_info);
  /*
    Convert image to fits scale PseudoColor class.
  */
  pixels=GetQuantumPixels(quantum_info);
  if (IsGrayImage(image,&image->exception) != MagickFalse)
    {
      length=GetQuantumExtent(image,quantum_info,GrayQuantum);
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        length=ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
          GrayQuantum,pixels,&image->exception);
        if (image->depth == 16)
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        if (((image->depth == 32) || (image->depth == 64)) &&
            (quantum_info->format != FloatingPointQuantumFormat))
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        count=WriteBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
    }
  else
    {
      length=GetQuantumExtent(image,quantum_info,RedQuantum);
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        length=ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
          RedQuantum,pixels,&image->exception);
        if (image->depth == 16)
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        if (((image->depth == 32) || (image->depth == 64)) &&
            (quantum_info->format != FloatingPointQuantumFormat))
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        count=WriteBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      length=GetQuantumExtent(image,quantum_info,GreenQuantum);
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        length=ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
          GreenQuantum,pixels,&image->exception);
        if (image->depth == 16)
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        if (((image->depth == 32) || (image->depth == 64)) &&
            (quantum_info->format != FloatingPointQuantumFormat))
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        count=WriteBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      length=GetQuantumExtent(image,quantum_info,BlueQuantum);
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        length=ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
          BlueQuantum,pixels,&image->exception);
        if (image->depth == 16)
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        if (((image->depth == 32) || (image->depth == 64)) &&
            (quantum_info->format != FloatingPointQuantumFormat))
          SetFITSUnsignedPixels(image->columns,image->depth,pixels);
        count=WriteBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
    }
  quantum_info=DestroyQuantumInfo(quantum_info);
  length=(size_t) (FITSBlocksize-TellBlob(image) % FITSBlocksize);
  if (length != 0)
    {
      (void) ResetMagickMemory(fits_info,0,length*sizeof(*fits_info));
      (void) WriteBlob(image,length,(unsigned char *) fits_info);
    }
  fits_info=DestroyString(fits_info);
  (void) CloseBlob(image);
  return(MagickTrue);
}

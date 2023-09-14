/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        M   M  IIIII  FFFFF  FFFFF                           %
%                        MM MM    I    F      F                               %
%                        M M M    I    FFF    FFF                             %
%                        M   M    I    F      F                               %
%                        M   M  IIIII  F      F                               %
%                                                                             %
%                                                                             %
%                      Read/Write MIFF Image Format                           %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#if defined(MAGICKCORE_BZLIB_DELEGATE)
#include "bzlib.h"
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
#include "lzma.h"
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include "zlib.h"
#endif

/*
  Define declarations.
*/
#if !defined(LZMA_OK)
#define LZMA_OK  0
#endif

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMIFFImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M I F F                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMIFF() returns MagickTrue if the image format type, identified by the
%  magick string, is MIFF.
%
%  The format of the IsMIFF method is:
%
%      MagickBooleanType IsMIFF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsMIFF(const unsigned char *magick,const size_t length)
{
  if (length < 14)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"id=ImageMagick",14) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M I F F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadMIFFImage() reads a MIFF image file and returns it.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the ReadMIFFImage method is:
%
%      Image *ReadMIFFImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  Decompression code contributed by Kyle Shorter.
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_BZLIB_DELEGATE) || defined(MAGICKCORE_LZMA_DELEGATE) || defined(MAGICKCORE_ZLIB_DELEGATE)
static void *AcquireCompressionMemory(void *context,const size_t items,
  const size_t size)
{
  size_t
    extent;

  (void) context;
  if (HeapOverflowSanityCheckGetSize(items,size,&extent) != MagickFalse)
    return((void *) NULL);
  if (extent > GetMaxMemoryRequest())
    return((void *) NULL);
  return(AcquireMagickMemory(extent));
}
#endif

#if defined(MAGICKCORE_BZLIB_DELEGATE)
static void *AcquireBZIPMemory(void *,int,int) magick_attribute((__malloc__));

static void *AcquireBZIPMemory(void *context,int items,int size)
{
  return(AcquireCompressionMemory(context,(size_t) items,(size_t) size));
}
#endif

#if defined(MAGICKCORE_LZMA_DELEGATE)
static void *AcquireLZMAMemory(void *,size_t,size_t)
  magick_attribute((__malloc__));

static void *AcquireLZMAMemory(void *context,size_t items,size_t size)
{
  return(AcquireCompressionMemory(context,items,size));
}
#endif

#if defined(MAGICKCORE_ZLIB_DELEGATE)
static voidpf AcquireZIPMemory(voidpf,unsigned int,unsigned int)
   magick_attribute((__malloc__));

static voidpf AcquireZIPMemory(voidpf context,unsigned int items,
  unsigned int size)
{
  return((voidpf) AcquireCompressionMemory(context,(size_t) items,
    (size_t) size));
}
#endif

static void PushRunlengthPacket(Image *image,const unsigned char *pixels,
  size_t *length,PixelInfo *pixel,ExceptionInfo *exception)
{
  const unsigned char
    *p;

  p=pixels;
  if (image->storage_class == PseudoClass)
    {
      pixel->index=0.0;
      switch (image->depth)
      {
        case 32:
        default:
        {
          pixel->index=(MagickRealType) ConstrainColormapIndex(image,(ssize_t)
            (((size_t) *p << 24) | ((size_t) *(p+1) << 16) |
            ((size_t) *(p+2) << 8) | (size_t) *(p+3)),exception);
          p+=4;
          break;
        }
        case 16:
        {
          pixel->index=(MagickRealType) ConstrainColormapIndex(image,(ssize_t)
            (((size_t) *p << 8) | (size_t) *(p+1)),exception);
          p+=2;
          break;
        }
        case 8:
        {
          pixel->index=(MagickRealType) ConstrainColormapIndex(image,
            (ssize_t) *p,exception);
          p++;
          break;
        }
      }
      switch (image->depth)
      {
        case 8:
        {
          unsigned char
            quantum;

          if (image->alpha_trait != UndefinedPixelTrait)
            {
              p=PushCharPixel(p,&quantum);
              pixel->alpha=(MagickRealType) ScaleCharToQuantum(quantum);
            }
          break;
        }
        case 16:
        {
          unsigned short
            quantum;

          if (image->alpha_trait != UndefinedPixelTrait)
            {
              p=PushShortPixel(MSBEndian,p,&quantum);
              pixel->alpha=(MagickRealType) ((size_t) quantum >> (image->depth-
                MAGICKCORE_QUANTUM_DEPTH));
            }
          break;
        }
        case 32:
        default:
        {
          unsigned int
            quantum;

          if (image->alpha_trait != UndefinedPixelTrait)
            {
              p=PushLongPixel(MSBEndian,p,&quantum);
              pixel->alpha=(MagickRealType) ((size_t) quantum >> (image->depth-
                MAGICKCORE_QUANTUM_DEPTH));
            }
          break;
        }
      }
      *length=((size_t) *p++)+1;
      return;
    }
  switch (image->depth)
  {
    case 8:
    {
      unsigned char
        quantum;

      p=PushCharPixel(p,&quantum);
      pixel->red=(MagickRealType) ScaleCharToQuantum(quantum);
      pixel->green=pixel->red;
      pixel->blue=pixel->red;
      if (IsGrayColorspace(image->colorspace) == MagickFalse)
        {
          p=PushCharPixel(p,&quantum);
          pixel->green=(MagickRealType) ScaleCharToQuantum(quantum);
          p=PushCharPixel(p,&quantum);
          pixel->blue=(MagickRealType) ScaleCharToQuantum(quantum);
        }
      if (image->colorspace == CMYKColorspace)
        {
          p=PushCharPixel(p,&quantum);
          pixel->black=(MagickRealType) ScaleCharToQuantum(quantum);
        }
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          p=PushCharPixel(p,&quantum);
          pixel->alpha=(MagickRealType) ScaleCharToQuantum(quantum);
        }
      break;
    }
    case 16:
    {
      unsigned short
        quantum;

      p=PushShortPixel(MSBEndian,p,&quantum);
      pixel->red=(MagickRealType) ((size_t) quantum >> (image->depth-
        MAGICKCORE_QUANTUM_DEPTH));
      pixel->green=pixel->red;
      pixel->blue=pixel->red;
      if (IsGrayColorspace(image->colorspace) == MagickFalse)
        {
          p=PushShortPixel(MSBEndian,p,&quantum);
          pixel->green=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
          p=PushShortPixel(MSBEndian,p,&quantum);
          pixel->blue=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      if (image->colorspace == CMYKColorspace)
        {
          p=PushShortPixel(MSBEndian,p,&quantum);
          pixel->black=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          p=PushShortPixel(MSBEndian,p,&quantum);
          pixel->alpha=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      break;
    }
    case 32:
    default:
    {
      unsigned int
        quantum;

      p=PushLongPixel(MSBEndian,p,&quantum);
      pixel->red=(MagickRealType) ((size_t) quantum >> (image->depth-
        MAGICKCORE_QUANTUM_DEPTH));
      pixel->green=pixel->red;
      pixel->blue=pixel->red;
      if (IsGrayColorspace(image->colorspace) == MagickFalse)
        {
          p=PushLongPixel(MSBEndian,p,&quantum);
          pixel->green=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
          p=PushLongPixel(MSBEndian,p,&quantum);
          pixel->blue=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      if (image->colorspace == CMYKColorspace)
        {
          p=PushLongPixel(MSBEndian,p,&quantum);
          pixel->black=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          p=PushLongPixel(MSBEndian,p,&quantum);
          pixel->alpha=(MagickRealType) ((size_t) quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      break;
    }
  }
  *length=(size_t) (*p++)+1;
}

#if defined(MAGICKCORE_BZLIB_DELEGATE)
static void RelinquishBZIPMemory(void *context,void *memory)
{
  (void) context;
  memory=RelinquishMagickMemory(memory);
}
#endif

#if defined(MAGICKCORE_LZMA_DELEGATE)
static void RelinquishLZMAMemory(void *context,void *memory)
{
  (void) context;
  memory=RelinquishMagickMemory(memory);
}
#endif

#if defined(MAGICKCORE_ZLIB_DELEGATE)
static void RelinquishZIPMemory(voidpf context,voidpf memory)
{
  (void) context;
  memory=RelinquishMagickMemory(memory);
}
#endif

static Image *ReadMIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define BZipMaxExtent(x)  ((x)+((x)/100)+600)
#define LZMAMaxExtent(x)  ((x)+((x)/3)+128)
#define ThrowMIFFException(exception,message) \
{ \
  if (quantum_info != (QuantumInfo *) NULL) \
    quantum_info=DestroyQuantumInfo(quantum_info); \
  if (compress_pixels != (unsigned char *) NULL) \
    compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels); \
  ThrowReaderException((exception),(message)); \
}
#define ZipMaxExtent(x)  ((x)+(((x)+7) >> 3)+(((x)+63) >> 6)+11)

#if defined(MAGICKCORE_BZLIB_DELEGATE)
  bz_stream
    bzip_info;
#endif

  char
    id[MagickPathExtent],
    keyword[MagickPathExtent],
    *options;

  double
    version;

  GeometryInfo
    geometry_info;

  Image
    *image;

  int
    c;

  LinkedListInfo
    *profiles;

#if defined(MAGICKCORE_LZMA_DELEGATE)
  lzma_stream
    initialize_lzma = LZMA_STREAM_INIT,
    lzma_info = LZMA_STREAM_INIT;

  lzma_allocator
    allocator;
#endif

  MagickBooleanType
    status;

  PixelInfo
    pixel;

  MagickStatusType
    flags;

  QuantumFormatType
    quantum_format;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  ssize_t
    i;

  size_t
    compress_extent,
    extent,
    length,
    packet_size;

  ssize_t
    count;

  unsigned char
    *compress_pixels,
    *pixels;

  size_t
    colors;

  ssize_t
    y;

#if defined(MAGICKCORE_ZLIB_DELEGATE)
  z_stream
    zip_info;
#endif

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
  /*
    Decode image header;  header terminates one character beyond a ':'.
  */
  c=ReadBlobByte(image);
  if (c == EOF)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  *id='\0';
  compress_pixels=(unsigned char *) NULL;
  quantum_info=(QuantumInfo *) NULL;
  (void) memset(keyword,0,sizeof(keyword));
  version=0.0;
  (void) version;
  do
  {
    /*
      Decode image header;  header terminates one character beyond a ':'.
    */
    SetGeometryInfo(&geometry_info);
    length=MagickPathExtent;
    options=AcquireString((char *) NULL);
    quantum_format=UndefinedQuantumFormat;
    profiles=(LinkedListInfo *) NULL;
    colors=0;
    image->depth=8UL;
    image->compression=NoCompression;
    while ((isgraph((int) ((unsigned char) c)) != 0) && (c != (int) ':'))
    {
      char
        *p;

      if (c == (int) '{')
        {
          char
            *comment;

          /*
            Read comment-- any text between { }.
          */
          length=MagickPathExtent;
          comment=AcquireString((char *) NULL);
          for (p=comment; comment != (char *) NULL; p++)
          {
            c=ReadBlobByte(image);
            if (c == (int) '\\')
              c=ReadBlobByte(image);
            else
              if ((c == EOF) || (c == (int) '}'))
                break;
            if ((size_t) (p-comment+1) >= length)
              {
                *p='\0';
                length<<=1;
                comment=(char *) ResizeQuantumMemory(comment,
                  OverAllocateMemory(length+MagickPathExtent),sizeof(*comment));
                if (comment == (char *) NULL)
                  break;
                p=comment+strlen(comment);
              }
            *p=(char) c;
          }
          if (comment == (char *) NULL)
            {
              options=DestroyString(options);
              ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
            }
          *p='\0';
          (void) SetImageProperty(image,"comment",comment,exception);
          comment=DestroyString(comment);
          c=ReadBlobByte(image);
        }
      else
        if (isalnum((int) ((unsigned char) c)) != MagickFalse)
          {
            /*
              Get the keyword.
            */
            length=MagickPathExtent-1;
            p=keyword;
            do
            {
              if (c == (int) '=')
                break;
              if ((size_t) (p-keyword) < (MagickPathExtent-1))
                *p++=(char) c;
              c=ReadBlobByte(image);
            } while (c != EOF);
            *p='\0';
            p=options;
            while ((isspace((int) ((unsigned char) c)) != 0) && (c != EOF))
              c=ReadBlobByte(image);
            if (c == (int) '=')
              {
                /*
                  Get the keyword value.
                */
                c=ReadBlobByte(image);
                while ((c != (int) '}') && (c != EOF))
                {
                  if ((size_t) (p-options+1) >= length)
                    {
                      *p='\0';
                      length<<=1;
                      options=(char *) ResizeQuantumMemory(options,length+
                        MagickPathExtent,sizeof(*options));
                      if (options == (char *) NULL)
                        break;
                      p=options+strlen(options);
                    }
                  *p++=(char) c;
                  c=ReadBlobByte(image);
                  if (c == '\\')
                    {
                      c=ReadBlobByte(image);
                      if (c == (int) '}')
                        {
                          *p++=(char) c;
                          c=ReadBlobByte(image);
                        }
                    }
                  if (*options != '{')
                    if (isspace((int) ((unsigned char) c)) != 0)
                      break;
                }
                if (options == (char *) NULL)
                  ThrowMIFFException(ResourceLimitError,
                    "MemoryAllocationFailed");
              }
            *p='\0';
            if (*options == '{')
              (void) CopyMagickString(options,options+1,strlen(options));
            /*
              Assign a value to the specified keyword.
            */
            switch (*keyword)
            {
              case 'a':
              case 'A':
              {
                if (LocaleCompare(keyword,"alpha-trait") == 0)
                  {
                    ssize_t
                      alpha_trait;

                    alpha_trait=ParseCommandOption(MagickPixelTraitOptions,
                      MagickFalse,options);
                    if (alpha_trait < 0)
                      break;
                    image->alpha_trait=(PixelTrait) alpha_trait;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'b':
              case 'B':
              {
                if (LocaleCompare(keyword,"background-color") == 0)
                  {
                    (void) QueryColorCompliance(options,AllCompliance,
                      &image->background_color,exception);
                    break;
                  }
                if (LocaleCompare(keyword,"blue-primary") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    image->chromaticity.blue_primary.x=geometry_info.rho;
                    image->chromaticity.blue_primary.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      image->chromaticity.blue_primary.y=
                        image->chromaticity.blue_primary.x;
                    break;
                  }
                if (LocaleCompare(keyword,"border-color") == 0)
                  {
                    (void) QueryColorCompliance(options,AllCompliance,
                      &image->border_color,exception);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'c':
              case 'C':
              {
                if (LocaleCompare(keyword,"channel-mask") == 0)
                  {
                    image->channel_mask=(ChannelType)
                      strtol(options,(char **) NULL,16);
                    break;
                  }
                if (LocaleCompare(keyword,"class") == 0)
                  {
                    ssize_t
                      storage_class;

                    storage_class=ParseCommandOption(MagickClassOptions,
                      MagickFalse,options);
                    if (storage_class < 0)
                      break;
                    image->storage_class=(ClassType) storage_class;
                    break;
                  }
                if (LocaleCompare(keyword,"colors") == 0)
                  {
                    colors=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"colorspace") == 0)
                  {
                    ssize_t
                      colorspace;

                    colorspace=ParseCommandOption(MagickColorspaceOptions,
                      MagickFalse,options);
                    if (colorspace < 0)
                      break;
                    image->colorspace=(ColorspaceType) colorspace;
                    break;
                  }
                if (LocaleCompare(keyword,"columns") == 0)
                  {
                    image->columns=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"compression") == 0)
                  {
                    ssize_t
                      compression;

                    compression=ParseCommandOption(MagickCompressOptions,
                      MagickFalse,options);
                    if (compression < 0)
                      break;
                    image->compression=(CompressionType) compression;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'd':
              case 'D':
              {
                if (LocaleCompare(keyword,"delay") == 0)
                  {
                    image->delay=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"depth") == 0)
                  {
                    image->depth=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"dispose") == 0)
                  {
                    ssize_t
                      dispose;

                    dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,
                      options);
                    if (dispose < 0)
                      break;
                    image->dispose=(DisposeType) dispose;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'e':
              case 'E':
              {
                if (LocaleCompare(keyword,"endian") == 0)
                  {
                    ssize_t
                      endian;

                    endian=ParseCommandOption(MagickEndianOptions,MagickFalse,
                      options);
                    if (endian < 0)
                      break;
                    image->endian=(EndianType) endian;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'g':
              case 'G':
              {
                if (LocaleCompare(keyword,"gamma") == 0)
                  {
                    image->gamma=StringToDouble(options,(char **) NULL);
                    break;
                  }
                if (LocaleCompare(keyword,"gravity") == 0)
                  {
                    ssize_t
                      gravity;

                    gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
                      options);
                    if (gravity < 0)
                      break;
                    image->gravity=(GravityType) gravity;
                    break;
                  }
                if (LocaleCompare(keyword,"green-primary") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    image->chromaticity.green_primary.x=geometry_info.rho;
                    image->chromaticity.green_primary.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      image->chromaticity.green_primary.y=
                        image->chromaticity.green_primary.x;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'i':
              case 'I':
              {
                if (LocaleCompare(keyword,"id") == 0)
                  {
                    if (*id != '\0')
                      {
                        options=DestroyString(options);
                        if (profiles != (LinkedListInfo *) NULL)
                          profiles=DestroyLinkedList(profiles,
                            RelinquishMagickMemory);
                        ThrowMIFFException(CorruptImageError,
                          "ImproperImageHeader");
                      }
                    (void) CopyMagickString(id,options,MagickPathExtent);
                    break;
                  }
                if (LocaleCompare(keyword,"iterations") == 0)
                  {
                    image->iterations=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'm':
              case 'M':
              {
                if (LocaleCompare(keyword,"matte") == 0)
                  {
                    ssize_t
                      matte;

                    matte=ParseCommandOption(MagickBooleanOptions,MagickFalse,
                      options);
                    if (matte < 0)
                      break;
                    image->alpha_trait=matte == 0 ? UndefinedPixelTrait :
                      BlendPixelTrait;
                    break;
                  }
                if (LocaleCompare(keyword,"mattecolor") == 0)
                  {
                    (void) QueryColorCompliance(options,AllCompliance,
                      &image->matte_color,exception);
                    break;
                  }
                if (LocaleCompare(keyword,"montage") == 0)
                  {
                    (void) CloneString(&image->montage,options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'n':
              case 'N':
              {
                if (LocaleCompare(keyword,"number-channels") == 0)
                  {
                    image->number_channels=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"number-meta-channels") == 0)
                  {
                    image->number_meta_channels=StringToUnsignedLong(options);
                    break;
                  }
                break;
              }
              case 'o':
              case 'O':
              {
                if (LocaleCompare(keyword,"orientation") == 0)
                  {
                    ssize_t
                      orientation;

                    orientation=ParseCommandOption(MagickOrientationOptions,
                      MagickFalse,options);
                    if (orientation < 0)
                      break;
                    image->orientation=(OrientationType) orientation;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'p':
              case 'P':
              {
                if (LocaleCompare(keyword,"page") == 0)
                  {
                    char
                      *geometry;

                    geometry=GetPageGeometry(options);
                    (void) ParseAbsoluteGeometry(geometry,&image->page);
                    geometry=DestroyString(geometry);
                    break;
                  }
                if (LocaleCompare(keyword,"pixel-intensity") == 0)
                  {
                    ssize_t
                      intensity;

                    intensity=ParseCommandOption(MagickPixelIntensityOptions,
                      MagickFalse,options);
                    if (intensity < 0)
                      break;
                    image->intensity=(PixelIntensityMethod) intensity;
                    break;
                  }
                if (LocaleCompare(keyword,"profile") == 0)
                  {
                    if (profiles == (LinkedListInfo *) NULL)
                      profiles=NewLinkedList(0);
                    (void) AppendValueToLinkedList(profiles,
                      AcquireString(options));
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'q':
              case 'Q':
              {
                if (LocaleCompare(keyword,"quality") == 0)
                  {
                    image->quality=StringToUnsignedLong(options);
                    break;
                  }
                if ((LocaleCompare(keyword,"quantum-format") == 0) ||
                    (LocaleCompare(keyword,"quantum:format") == 0))
                  {
                    ssize_t
                      format;

                    format=ParseCommandOption(MagickQuantumFormatOptions,
                      MagickFalse,options);
                    if (format < 0)
                      break;
                    quantum_format=(QuantumFormatType) format;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'r':
              case 'R':
              {
                if (LocaleCompare(keyword,"red-primary") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    image->chromaticity.red_primary.x=geometry_info.rho;
                    image->chromaticity.red_primary.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      image->chromaticity.red_primary.y=
                        image->chromaticity.red_primary.x;
                    break;
                  }
                if (LocaleCompare(keyword,"rendering-intent") == 0)
                  {
                    ssize_t
                      rendering_intent;

                    rendering_intent=ParseCommandOption(MagickIntentOptions,
                      MagickFalse,options);
                    if (rendering_intent < 0)
                      break;
                    image->rendering_intent=(RenderingIntent) rendering_intent;
                    break;
                  }
                if (LocaleCompare(keyword,"resolution") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    image->resolution.x=geometry_info.rho;
                    image->resolution.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      image->resolution.y=image->resolution.x;
                    break;
                  }
                if (LocaleCompare(keyword,"rows") == 0)
                  {
                    image->rows=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 's':
              case 'S':
              {
                if (LocaleCompare(keyword,"scene") == 0)
                  {
                    image->scene=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 't':
              case 'T':
              {
                if (LocaleCompare(keyword,"ticks-per-second") == 0)
                  {
                    image->ticks_per_second=(ssize_t) StringToLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"tile-offset") == 0)
                  {
                    char
                      *geometry;

                    geometry=GetPageGeometry(options);
                    (void) ParseAbsoluteGeometry(geometry,&image->tile_offset);
                    geometry=DestroyString(geometry);
                    break;
                  }
                if (LocaleCompare(keyword,"type") == 0)
                  {
                    ssize_t
                      type;

                    type=ParseCommandOption(MagickTypeOptions,MagickFalse,
                      options);
                    if (type < 0)
                      break;
                    image->type=(ImageType) type;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'u':
              case 'U':
              {
                if (LocaleCompare(keyword,"units") == 0)
                  {
                    ssize_t
                      units;

                    units=ParseCommandOption(MagickResolutionOptions,
                      MagickFalse,options);
                    if (units < 0)
                      break;
                    image->units=(ResolutionType) units;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'v':
              case 'V':
              {
                if (LocaleCompare(keyword,"version") == 0)
                  {
                    version=StringToDouble(options,(char **) NULL);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              case 'w':
              case 'W':
              {
                if (LocaleCompare(keyword,"white-point") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    image->chromaticity.white_point.x=geometry_info.rho;
                    image->chromaticity.white_point.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      image->chromaticity.white_point.y=
                        image->chromaticity.white_point.x;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
              default:
              {
                (void) SetImageProperty(image,keyword,options,exception);
                break;
              }
            }
          }
        else
          c=ReadBlobByte(image);
      while (isspace((int) ((unsigned char) c)) != 0)
        c=ReadBlobByte(image);
    }
    options=DestroyString(options);
    (void) ReadBlobByte(image);
    /*
      Verify that required image information is defined.
    */
    if ((LocaleCompare(id,"ImageMagick") != 0) ||
        (image->storage_class == UndefinedClass) ||
        (image->compression == UndefinedCompression) ||
        (image->colorspace == UndefinedColorspace) ||
        (image->columns == 0) || (image->rows == 0) ||
        (image->number_meta_channels > (size_t) (MaxPixelChannels-MetaPixelChannels)) ||
        ((image->number_channels+image->number_meta_channels) >= MaxPixelChannels) ||
        (image->depth == 0) || (image->depth > 64))
      {
        if (profiles != (LinkedListInfo *) NULL)
          profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
        if (image->previous == (Image *) NULL)
          ThrowMIFFException(CorruptImageError,"ImproperImageHeader");
        DeleteImageFromList(&image);
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"ImproperImageHeader","`%s'",image->filename);
        break;
      }
    *id='\0';
    if (image->montage != (char *) NULL)
      {
        char
          *p;

        /*
          Image directory.
        */
        extent=MagickPathExtent;
        image->directory=AcquireString((char *) NULL);
        p=image->directory;
        length=0;
        do
        {
          *p='\0';
          if ((length+MagickPathExtent) >= extent)
            {
              /*
                Allocate more memory for the image directory.
              */
              extent<<=1;
              image->directory=(char *) ResizeQuantumMemory(image->directory,
                extent+MagickPathExtent,sizeof(*image->directory));
              if (image->directory == (char *) NULL)
                ThrowMIFFException(CorruptImageError,"UnableToReadImageData");
              p=image->directory+length;
            }
          c=ReadBlobByte(image);
          if (c == EOF)
            break;
          *p++=(char) c;
          length++;
        } while (c != (int) '\0');
      }
    if (profiles != (LinkedListInfo *) NULL)
      {
        const char
          *name;

        StringInfo
          *profile;

        /*
          Read image profiles.
        */
        ResetLinkedListIterator(profiles);
        name=(const char *) GetNextValueInLinkedList(profiles);
        while (name != (const char *) NULL)
        {
          length=ReadBlobMSBLong(image);
          if ((length == 0) || ((MagickSizeType) length > GetBlobSize(image)))
            break;
          profile=AcquireStringInfo(length);
          if (profile == (StringInfo *) NULL)
            break;
          count=ReadBlob(image,length,GetStringInfoDatum(profile));
          if (count != (ssize_t) length)
            {
              profile=DestroyStringInfo(profile);
              break;
            }
          status=SetImageProfile(image,name,profile,exception);
          profile=DestroyStringInfo(profile);
          if (status == MagickFalse)
            break;
          name=(const char *) GetNextValueInLinkedList(profiles);
        }
        profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
      }
    image->depth=GetImageQuantumDepth(image,MagickFalse);
    if (image->storage_class == PseudoClass)
      {
        unsigned char
          *colormap;

        /*
          Create image colormap.
        */
        packet_size=(size_t) (3UL*image->depth/8UL);
        if ((MagickSizeType) colors > GetBlobSize(image))
          ThrowMIFFException(CorruptImageError,"InsufficientImageDataInFile");
        if (((MagickSizeType) packet_size*colors) > GetBlobSize(image))
          ThrowMIFFException(CorruptImageError,"InsufficientImageDataInFile");
        status=AcquireImageColormap(image,colors != 0 ? colors : 256,exception);
        if (status == MagickFalse)
          ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
        if (colors != 0)
          {
            const unsigned char
              *p;

            /*
              Read image colormap from file.
            */
            colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
              packet_size*sizeof(*colormap));
            if (colormap == (unsigned char *) NULL)
              ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
            count=ReadBlob(image,packet_size*image->colors,colormap);
            p=colormap;
            switch (image->depth)
            {
              case 8:
              {
                unsigned char
                  char_pixel;

                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  p=PushCharPixel(p,&char_pixel);
                  image->colormap[i].red=(MagickRealType)
                    ScaleCharToQuantum(char_pixel);
                  p=PushCharPixel(p,&char_pixel);
                  image->colormap[i].green=(MagickRealType)
                    ScaleCharToQuantum(char_pixel);
                  p=PushCharPixel(p,&char_pixel);
                  image->colormap[i].blue=(MagickRealType)
                    ScaleCharToQuantum(char_pixel);
                }
                break;
              }
              case 16:
              {
                unsigned short
                  short_pixel;

                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  p=PushShortPixel(MSBEndian,p,&short_pixel);
                  image->colormap[i].red=(MagickRealType)
                    ScaleShortToQuantum(short_pixel);
                  p=PushShortPixel(MSBEndian,p,&short_pixel);
                  image->colormap[i].green=(MagickRealType)
                    ScaleShortToQuantum(short_pixel);
                  p=PushShortPixel(MSBEndian,p,&short_pixel);
                  image->colormap[i].blue=(MagickRealType)
                    ScaleShortToQuantum(short_pixel);
                }
                break;
              }
              case 32:
              default:
              {
                unsigned int
                  long_pixel;

                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  p=PushLongPixel(MSBEndian,p,&long_pixel);
                  image->colormap[i].red=(MagickRealType)
                    ScaleLongToQuantum(long_pixel);
                  p=PushLongPixel(MSBEndian,p,&long_pixel);
                  image->colormap[i].green=(MagickRealType)
                    ScaleLongToQuantum(long_pixel);
                  p=PushLongPixel(MSBEndian,p,&long_pixel);
                  image->colormap[i].blue=(MagickRealType)
                    ScaleLongToQuantum(long_pixel);
                }
                break;
              }
            }
            colormap=(unsigned char *) RelinquishMagickMemory(colormap);
          }
      }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    status=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    /*
      Allocate image pixels.
    */
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
    if (quantum_format != UndefinedQuantumFormat)
      {
        status=SetQuantumFormat(image,quantum_info,quantum_format);
        if (status == MagickFalse)
          ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
      }
    packet_size=(size_t) (image->depth/8);
    if (image->storage_class == DirectClass)
      packet_size=(size_t) (3*image->depth/8);
    if (IsGrayColorspace(image->colorspace) != MagickFalse)
      packet_size=image->depth/8;
    if (image->alpha_trait != UndefinedPixelTrait)
      packet_size+=image->depth/8;
    if (image->colorspace == CMYKColorspace)
      packet_size+=image->depth/8;
    if (image->compression == RLECompression)
      packet_size++;
    if (image->number_meta_channels != 0)
      packet_size+=image->number_meta_channels*image->depth/8;
    compress_extent=MagickMax(MagickMax(BZipMaxExtent(packet_size*
      image->columns),LZMAMaxExtent(packet_size*image->columns)),
      ZipMaxExtent(packet_size*image->columns));
    if (compress_extent < (packet_size*image->columns))
      ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
    compress_pixels=(unsigned char *) AcquireQuantumMemory(compress_extent,
      sizeof(*compress_pixels));
    if (compress_pixels == (unsigned char *) NULL)
      ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
    /*
      Read image pixels.
    */
    quantum_type=RGBQuantum;
    if (image->alpha_trait != UndefinedPixelTrait)
      quantum_type=RGBAQuantum;
    if (image->colorspace == CMYKColorspace)
      {
        quantum_type=CMYKQuantum;
        if (image->alpha_trait != UndefinedPixelTrait)
          quantum_type=CMYKAQuantum;
      }
    if (IsGrayColorspace(image->colorspace) != MagickFalse)
      {
        quantum_type=GrayQuantum;
        if (image->alpha_trait != UndefinedPixelTrait)
          quantum_type=GrayAlphaQuantum;
      }
    if (image->storage_class == PseudoClass)
      {
        quantum_type=IndexQuantum;
        if (image->alpha_trait != UndefinedPixelTrait)
          quantum_type=IndexAlphaQuantum;
      }
    if (image->number_meta_channels != 0)
      quantum_type=MultispectralQuantum;
    status=MagickTrue;
    GetPixelInfo(image,&pixel);
#if defined(MAGICKCORE_BZLIB_DELEGATE)
   (void) memset(&bzip_info,0,sizeof(bzip_info));
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
    (void) memset(&allocator,0,sizeof(allocator));
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
    (void) memset(&zip_info,0,sizeof(zip_info));
#endif
    switch (image->compression)
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      case BZipCompression:
      {
        int
          code;

        bzip_info.bzalloc=AcquireBZIPMemory;
        bzip_info.bzfree=RelinquishBZIPMemory;
        bzip_info.opaque=(void *) image;
        code=BZ2_bzDecompressInit(&bzip_info,(int) image_info->verbose,
          MagickFalse);
        if (code != BZ_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
      case LZMACompression:
      {
        int
          code;

        allocator.alloc=AcquireLZMAMemory;
        allocator.free=RelinquishLZMAMemory;
        allocator.opaque=(void *) image;
        lzma_info=initialize_lzma;
        lzma_info.allocator=(&allocator);
        code=(int) lzma_auto_decoder(&lzma_info,(uint64_t) -1,0);
        if (code != LZMA_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      case LZWCompression:
      case ZipCompression:
      {
        int
          code;

        zip_info.zalloc=AcquireZIPMemory;
        zip_info.zfree=RelinquishZIPMemory;
        zip_info.opaque=(voidpf) image;
        code=inflateInit(&zip_info);
        if (code != Z_OK)
          status=MagickFalse;
        break;
      }
#endif
      case RLECompression:
        break;
      default:
        break;
    }
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    length=0;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      ssize_t
        x;

      Quantum
        *magick_restrict q;

      if (status == MagickFalse)
        break;
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      extent=0;
      switch (image->compression)
      {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
        case BZipCompression:
        {
          bzip_info.next_out=(char *) pixels;
          bzip_info.avail_out=(unsigned int) (packet_size*image->columns);
          do
          {
            int
              code;

            if (bzip_info.avail_in == 0)
              {
                bzip_info.next_in=(char *) compress_pixels;
                length=(size_t) BZipMaxExtent(packet_size*image->columns);
                if (version != 0.0)
                  length=(size_t) ReadBlobMSBLong(image);
                if (length <= compress_extent)
                  bzip_info.avail_in=(unsigned int) ReadBlob(image,length,
                    (unsigned char *) bzip_info.next_in);
                if ((length > compress_extent) ||
                    ((size_t) bzip_info.avail_in != length))
                  {
                    (void) BZ2_bzDecompressEnd(&bzip_info);
                    ThrowMIFFException(CorruptImageError,
                      "UnableToReadImageData");
                  }
              }
            code=BZ2_bzDecompress(&bzip_info);
            if ((code != BZ_OK) && (code != BZ_STREAM_END))
              {
                status=MagickFalse;
                break;
              }
            if (code == BZ_STREAM_END)
              break;
          } while (bzip_info.avail_out != 0);
          extent=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
        case LZMACompression:
        {
          lzma_info.next_out=pixels;
          lzma_info.avail_out=packet_size*image->columns;
          do
          {
            int
              code;

            if (lzma_info.avail_in == 0)
              {
                lzma_info.next_in=compress_pixels;
                length=(size_t) ReadBlobMSBLong(image);
                if (length <= compress_extent)
                  lzma_info.avail_in=(unsigned int) ReadBlob(image,length,
                    (unsigned char *) lzma_info.next_in);
                if ((length > compress_extent) ||
                    (lzma_info.avail_in != length))
                  {
                    lzma_end(&lzma_info);
                    ThrowMIFFException(CorruptImageError,
                      "UnableToReadImageData");
                  }
              }
            code=(int) lzma_code(&lzma_info,LZMA_RUN);
            if ((code != LZMA_OK) && (code != LZMA_STREAM_END))
              {
                status=MagickFalse;
                break;
              }
            if (code == LZMA_STREAM_END)
              break;
          } while (lzma_info.avail_out != 0);
          extent=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
        case LZWCompression:
        case ZipCompression:
        {
          zip_info.next_out=pixels;
          zip_info.avail_out=(uInt) (packet_size*image->columns);
          do
          {
            int
              code;

            if (zip_info.avail_in == 0)
              {
                zip_info.next_in=compress_pixels;
                length=(size_t) ZipMaxExtent(packet_size*image->columns);
                if (version != 0.0)
                  length=(size_t) ReadBlobMSBLong(image);
                if (length <= compress_extent)
                  zip_info.avail_in=(unsigned int) ReadBlob(image,length,
                    zip_info.next_in);
                if ((length > compress_extent) ||
                    ((size_t) zip_info.avail_in != length))
                  {
                    (void) inflateEnd(&zip_info);
                    ThrowMIFFException(CorruptImageError,
                      "UnableToReadImageData");
                  }
              }
            code=inflate(&zip_info,Z_SYNC_FLUSH);
            if ((code != Z_OK) && (code != Z_STREAM_END))
              {
                status=MagickFalse;
                break;
              }
            if (code == Z_STREAM_END)
              break;
          } while (zip_info.avail_out != 0);
          extent=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
        case RLECompression:
        {
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (length == 0)
              {
                count=ReadBlob(image,packet_size,pixels);
                if (count != (ssize_t) packet_size)
                  ThrowMIFFException(CorruptImageError,"UnableToReadImageData");
                PushRunlengthPacket(image,pixels,&length,&pixel,exception);
              }
            length--;
            if (image->storage_class == PseudoClass)
              SetPixelIndex(image,ClampToQuantum(pixel.index),q);
            else
              {
                SetPixelRed(image,ClampToQuantum(pixel.red),q);
                SetPixelGreen(image,ClampToQuantum(pixel.green),q);
                SetPixelBlue(image,ClampToQuantum(pixel.blue),q);
                if (image->colorspace == CMYKColorspace)
                  SetPixelBlack(image,ClampToQuantum(pixel.black),q);
              }
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ClampToQuantum(pixel.alpha),q);
            q+=GetPixelChannels(image);
          }
          extent=(size_t) x;
          break;
        }
        default:
        {
          const void
            *stream;

          stream=ReadBlobStream(image,packet_size*image->columns,pixels,&count);
          if (count != (ssize_t) (packet_size*image->columns))
            ThrowMIFFException(CorruptImageError,"UnableToReadImageData");
          extent=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,(unsigned char *) stream,exception);
          break;
        }
      }
      if (extent < image->columns)
        break;
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
    }
    SetQuantumImageType(image,quantum_type);
    switch (image->compression)
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      case BZipCompression:
      {
        int
          code;

        if (version == 0.0)
          {
            MagickOffsetType
              offset;

            offset=SeekBlob(image,-((MagickOffsetType) bzip_info.avail_in),
              SEEK_CUR);
            if (offset < 0)
              {
                (void) BZ2_bzDecompressEnd(&bzip_info);
                ThrowMIFFException(CorruptImageError,"ImproperImageHeader");
              }
          }
        code=BZ2_bzDecompressEnd(&bzip_info);
        if (code != BZ_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
      case LZMACompression:
      {
        int
          code;

        code=(int) lzma_code(&lzma_info,LZMA_FINISH);
        if ((code != LZMA_STREAM_END) && (code != LZMA_OK))
          status=MagickFalse;
        lzma_end(&lzma_info);
        break;
      }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      case LZWCompression:
      case ZipCompression:
      {
        int
          code;

        if (version == 0.0)
          {
            MagickOffsetType
              offset;

            offset=SeekBlob(image,-((MagickOffsetType) zip_info.avail_in),
              SEEK_CUR);
            if (offset < 0)
              {
                (void) inflateEnd(&zip_info);
                ThrowMIFFException(CorruptImageError,"ImproperImageHeader");
              }
          }
        code=inflateEnd(&zip_info);
        if (code != Z_OK)
          status=MagickFalse;
        break;
      }
#endif
      default:
        break;
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels);
    if (((y != (ssize_t) image->rows)) || (status == MagickFalse))
      {
        image=DestroyImageList(image);
        return((Image *) NULL);
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
    do
    {
      c=ReadBlobByte(image);
    } while ((isgraph((int) ((unsigned char) c)) == 0) && (c != EOF));
    if ((c != EOF) && ((c == 'i') || (c == 'I')))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  } while (c != EOF && ((c == 'i') || (c == 'I')));
  (void) CloseBlob(image);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M I F F I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMIFFImage() adds properties for the MIFF image format to the list of
%  supported formats.  The properties include the image format tag, a method to
%  read and/or write the format, whether the format supports the saving of more
%  than one frame to the same file or blob, whether the format supports native
%  in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterMIFFImage method is:
%
%      size_t RegisterMIFFImage(void)
%
*/
ModuleExport size_t RegisterMIFFImage(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(MagickImageCoderSignatureText)
  (void) CopyMagickString(version,MagickLibVersionText,MagickPathExtent);
#if defined(ZLIB_VERSION)
  (void) ConcatenateMagickString(version," with Zlib ",MagickPathExtent);
  (void) ConcatenateMagickString(version,ZLIB_VERSION,MagickPathExtent);
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  (void) ConcatenateMagickString(version," and BZlib",MagickPathExtent);
#endif
#endif
  entry=AcquireMagickInfo("MIFF","MIFF","Magick Image File Format");
  entry->decoder=(DecodeImageHandler *) ReadMIFFImage;
  entry->encoder=(EncodeImageHandler *) WriteMIFFImage;
  entry->magick=(IsImageFormatHandler *) IsMIFF;
  entry->flags|=CoderDecoderSeekableStreamFlag;
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
%   U n r e g i s t e r M I F F I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMIFFImage() removes format registrations made by the MIFF module
%  from the list of supported formats.
%
%  The format of the UnregisterMIFFImage method is:
%
%      UnregisterMIFFImage(void)
%
*/
ModuleExport void UnregisterMIFFImage(void)
{
  (void) UnregisterMagickInfo("MIFF");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMIFFImage() writes a MIFF image to a file.
%
%  The format of the WriteMIFFImage method is:
%
%      MagickBooleanType WriteMIFFImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  Compression code contributed by Kyle Shorter.
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

static unsigned char *PopRunlengthPacket(Image *image,unsigned char *pixels,
  size_t length,PixelInfo *pixel)
{
  if (image->storage_class != DirectClass)
    {
      unsigned int
        value;

      value=(unsigned int) ClampToQuantum(pixel->index);
      switch (image->depth)
      {
        case 32:
        default:
        {
          *pixels++=(unsigned char) (value >> 24);
          *pixels++=(unsigned char) (value >> 16);
          magick_fallthrough;
        }
        case 16:
        {
          *pixels++=(unsigned char) (value >> 8);
          magick_fallthrough;
        }
        case 8:
        {
          *pixels++=(unsigned char) value;
          break;
        }
      }
      switch (image->depth)
      {
        case 32:
        default:
        {
          unsigned int
            long_value;

          if (image->alpha_trait != UndefinedPixelTrait)
            {
              long_value=ScaleQuantumToLong(ClampToQuantum(pixel->alpha));
              pixels=PopLongPixel(MSBEndian,long_value,pixels);
            }
          break;
        }
        case 16:
        {
          unsigned short
            short_value;

          if (image->alpha_trait != UndefinedPixelTrait)
            {
              short_value=ScaleQuantumToShort(ClampToQuantum(pixel->alpha));
              pixels=PopShortPixel(MSBEndian,short_value,pixels);
            }
          break;
        }
        case 8:
        {
          unsigned char
            char_value;

          if (image->alpha_trait != UndefinedPixelTrait)
            {
              char_value=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
                pixel->alpha));
              pixels=PopCharPixel(char_value,pixels);
            }
          break;
        }
      }
      *pixels++=(unsigned char) length;
      return(pixels);
    }
  switch (image->depth)
  {
    case 32:
    default:
    {
      unsigned int
        value;

      value=ScaleQuantumToLong(ClampToQuantum(pixel->red));
      pixels=PopLongPixel(MSBEndian,value,pixels);
      if (IsGrayColorspace(image->colorspace) == MagickFalse)
        {
          value=ScaleQuantumToLong(ClampToQuantum(pixel->green));
          pixels=PopLongPixel(MSBEndian,value,pixels);
          value=ScaleQuantumToLong(ClampToQuantum(pixel->blue));
          pixels=PopLongPixel(MSBEndian,value,pixels);
        }
      if (image->colorspace == CMYKColorspace)
        {
          value=ScaleQuantumToLong(ClampToQuantum(pixel->black));
          pixels=PopLongPixel(MSBEndian,value,pixels);
        }
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          value=ScaleQuantumToLong(ClampToQuantum(pixel->alpha));
          pixels=PopLongPixel(MSBEndian,value,pixels);
        }
      break;
    }
    case 16:
    {
      unsigned short
        value;

      value=ScaleQuantumToShort(ClampToQuantum(pixel->red));
      pixels=PopShortPixel(MSBEndian,value,pixels);
      if (IsGrayColorspace(image->colorspace) == MagickFalse)
        {
          value=ScaleQuantumToShort(ClampToQuantum(pixel->green));
          pixels=PopShortPixel(MSBEndian,value,pixels);
          value=ScaleQuantumToShort(ClampToQuantum(pixel->blue));
          pixels=PopShortPixel(MSBEndian,value,pixels);
        }
      if (image->colorspace == CMYKColorspace)
        {
          value=ScaleQuantumToShort(ClampToQuantum(pixel->black));
          pixels=PopShortPixel(MSBEndian,value,pixels);
        }
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          value=ScaleQuantumToShort(ClampToQuantum(pixel->alpha));
          pixels=PopShortPixel(MSBEndian,value,pixels);
        }
      break;
    }
    case 8:
    {
      unsigned char
        value;

      value=(unsigned char) ScaleQuantumToChar(ClampToQuantum(pixel->red));
      pixels=PopCharPixel(value,pixels);
      if (IsGrayColorspace(image->colorspace) == MagickFalse)
        {
          value=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
            pixel->green));
          pixels=PopCharPixel(value,pixels);
          value=(unsigned char) ScaleQuantumToChar(ClampToQuantum(pixel->blue));
          pixels=PopCharPixel(value,pixels);
        }
      if (image->colorspace == CMYKColorspace)
        {
          value=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
            pixel->black));
          pixels=PopCharPixel(value,pixels);
        }
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          value=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
            pixel->alpha));
          pixels=PopCharPixel(value,pixels);
        }
      break;
    }
  }
  *pixels++=(unsigned char) length;
  return(pixels);
}

static MagickBooleanType WriteMIFFImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  bz_stream
    bzip_info;
#endif

  char
    buffer[MagickPathExtent];

  CompressionType
    compression;

  const char
    *property,
    *value;

#if defined(MAGICKCORE_LZMA_DELEGATE)
  lzma_allocator
    allocator;

  lzma_stream
    initialize_lzma = LZMA_STREAM_INIT,
    lzma_info;
#endif

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  PixelInfo
    pixel,
    target;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  size_t
    length,
    number_scenes,
    packet_size;

  ssize_t
    i,
    y;

  unsigned char
    *compress_pixels,
    *pixels,
    *q;

#if defined(MAGICKCORE_ZLIB_DELEGATE)
  z_stream
    zip_info;
#endif

  /*
    Open output image file.
  */
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
  scene=0;
  number_scenes=GetImageListLength(image);
  do
  {
    /*
      Allocate image pixels.
    */
    if ((image->storage_class == PseudoClass) &&
        (image->colors > (size_t) (GetQuantumRange(image->depth)+1)))
      (void) SetImageStorageClass(image,DirectClass,exception);
    image->depth=image->depth <= 8 ? 8UL : image->depth <= 16 ? 16UL : 32UL;
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    if ((image->storage_class != PseudoClass) && (image->depth >= 16) &&
        (quantum_info->format == UndefinedQuantumFormat) &&
        (IsHighDynamicRangeImage(image,exception) != MagickFalse))
      {
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          {
            quantum_info=DestroyQuantumInfo(quantum_info);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
      }
    else
      if (image->depth < 16)
        (void) DeleteImageProperty(image,"quantum:format");
    compression=UndefinedCompression;
    if (image_info->compression != UndefinedCompression)
      compression=image_info->compression;
    switch (compression)
    {
#if !defined(MAGICKCORE_LZMA_DELEGATE)
      case LZMACompression: compression=NoCompression; break;
#endif
#if !defined(MAGICKCORE_ZLIB_DELEGATE)
      case LZWCompression:
      case ZipCompression: compression=NoCompression; break;
#endif
#if !defined(MAGICKCORE_BZLIB_DELEGATE)
      case BZipCompression: compression=NoCompression; break;
#endif
      case RLECompression:
      {
        if (quantum_info->format == FloatingPointQuantumFormat)
          compression=NoCompression;
        GetPixelInfo(image,&target);
        break;
      }
      default:
        break;
    }
    packet_size=(size_t) (image->depth/8);
    if (image->storage_class == DirectClass)
      packet_size=(size_t) (3*image->depth/8);
    if (IsGrayColorspace(image->colorspace) != MagickFalse)
      packet_size=(size_t) (image->depth/8);
    if (image->alpha_trait != UndefinedPixelTrait)
      packet_size+=image->depth/8;
    if (image->colorspace == CMYKColorspace)
      packet_size+=image->depth/8;
    if (compression == RLECompression)
      packet_size++;
    if (image->number_meta_channels != 0)
      packet_size+=image->number_meta_channels*image->depth/8;
    length=MagickMax(BZipMaxExtent(packet_size*image->columns),ZipMaxExtent(
      packet_size*image->columns));
    if ((compression == BZipCompression) || (compression == ZipCompression))
      if (length != (size_t) ((unsigned int) length))
        compression=NoCompression;
    compress_pixels=(unsigned char *) AcquireQuantumMemory(length,
      sizeof(*compress_pixels));
    if (compress_pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    /*
      Write MIFF header.
    */
    (void) WriteBlobString(image,"id=ImageMagick version=1.0\n");
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "class=%s colors=%.20g alpha-trait=%s\n",CommandOptionToMnemonic(
      MagickClassOptions,image->storage_class),(double) image->colors,
      CommandOptionToMnemonic(MagickPixelTraitOptions,(ssize_t)
      image->alpha_trait));
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent, "number-channels=%.20g "
      "number-meta-channels=%.20g channel-mask=0x%016llx\n",
      (double) image->number_channels,(double) image->number_meta_channels,
      (MagickOffsetType) image->channel_mask);
    (void) WriteBlobString(image,buffer);
    if (image->alpha_trait != UndefinedPixelTrait)
      (void) WriteBlobString(image,"matte=True\n");
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "columns=%.20g rows=%.20g depth=%.20g\n",(double) image->columns,
      (double) image->rows,(double) image->depth);
    (void) WriteBlobString(image,buffer);
    if (image->type != UndefinedType)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"type=%s\n",
          CommandOptionToMnemonic(MagickTypeOptions,image->type));
        (void) WriteBlobString(image,buffer);
      }
    if (image->colorspace != UndefinedColorspace)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"colorspace=%s\n",
          CommandOptionToMnemonic(MagickColorspaceOptions,image->colorspace));
        (void) WriteBlobString(image,buffer);
      }
    if (image->intensity != UndefinedPixelIntensityMethod)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "pixel-intensity=%s\n",CommandOptionToMnemonic(
          MagickPixelIntensityOptions,image->intensity));
        (void) WriteBlobString(image,buffer);
      }
    if (image->endian != UndefinedEndian)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"endian=%s\n",
          CommandOptionToMnemonic(MagickEndianOptions,image->endian));
        (void) WriteBlobString(image,buffer);
      }
    if (compression != UndefinedCompression)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"compression=%s  "
          "quality=%.20g\n",CommandOptionToMnemonic(MagickCompressOptions,
          compression),(double) image->quality);
        (void) WriteBlobString(image,buffer);
      }
    if (image->units != UndefinedResolution)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"units=%s\n",
          CommandOptionToMnemonic(MagickResolutionOptions,image->units));
        (void) WriteBlobString(image,buffer);
      }
    if ((image->resolution.x != 0) || (image->resolution.y != 0))
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "resolution=%gx%g\n",image->resolution.x,image->resolution.y);
        (void) WriteBlobString(image,buffer);
      }
    if ((image->page.width != 0) || (image->page.height != 0))
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "page=%.20gx%.20g%+.20g%+.20g\n",(double) image->page.width,(double)
          image->page.height,(double) image->page.x,(double) image->page.y);
        (void) WriteBlobString(image,buffer);
      }
    else
      if ((image->page.x != 0) || (image->page.y != 0))
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"page=%+ld%+ld\n",
            (long) image->page.x,(long) image->page.y);
          (void) WriteBlobString(image,buffer);
        }
    if ((image->tile_offset.x != 0) || (image->tile_offset.y != 0))
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "tile-offset=%+ld%+ld\n",(long) image->tile_offset.x,(long)
          image->tile_offset.y);
        (void) WriteBlobString(image,buffer);
      }
    if ((GetNextImageInList(image) != (Image *) NULL) ||
        (GetPreviousImageInList(image) != (Image *) NULL))
      {
        if (image->scene == 0)
          (void) FormatLocaleString(buffer,MagickPathExtent,"iterations=%.20g "
            "delay=%.20g ticks-per-second=%.20g\n",(double) image->iterations,
            (double) image->delay,(double) image->ticks_per_second);
        else
          (void) FormatLocaleString(buffer,MagickPathExtent,"scene=%.20g  "
            "iterations=%.20g delay=%.20g ticks-per-second=%.20g\n",(double)
            image->scene,(double) image->iterations,(double) image->delay,
            (double) image->ticks_per_second);
        (void) WriteBlobString(image,buffer);
      }
    else
      {
        if (image->scene != 0)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,"scene=%.20g\n",
              (double) image->scene);
            (void) WriteBlobString(image,buffer);
          }
        if (image->iterations != 0)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "iterations=%.20g\n",(double) image->iterations);
            (void) WriteBlobString(image,buffer);
          }
        if (image->delay != 0)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,"delay=%.20g\n",
              (double) image->delay);
            (void) WriteBlobString(image,buffer);
          }
        if (image->ticks_per_second != UndefinedTicksPerSecond)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "ticks-per-second=%.20g\n",(double) image->ticks_per_second);
            (void) WriteBlobString(image,buffer);
          }
      }
    if (image->gravity != UndefinedGravity)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"gravity=%s\n",
          CommandOptionToMnemonic(MagickGravityOptions,image->gravity));
        (void) WriteBlobString(image,buffer);
      }
    if (image->dispose != UndefinedDispose)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"dispose=%s\n",
          CommandOptionToMnemonic(MagickDisposeOptions,image->dispose));
        (void) WriteBlobString(image,buffer);
      }
    if (image->rendering_intent != UndefinedIntent)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "rendering-intent=%s\n",CommandOptionToMnemonic(MagickIntentOptions,
          image->rendering_intent));
        (void) WriteBlobString(image,buffer);
      }
    if (image->gamma != 0.0)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"gamma=%g\n",
          image->gamma);
        (void) WriteBlobString(image,buffer);
      }
    if (image->chromaticity.white_point.x != 0.0)
      {
        /*
          Note chromaticity points.
        */
        (void) FormatLocaleString(buffer,MagickPathExtent,"red-primary=%g,%g "
          "green-primary=%g,%g blue-primary=%g,%g\n",
          image->chromaticity.red_primary.x,image->chromaticity.red_primary.y,
          image->chromaticity.green_primary.x,
          image->chromaticity.green_primary.y,
          image->chromaticity.blue_primary.x,
          image->chromaticity.blue_primary.y);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "white-point=%g,%g\n",image->chromaticity.white_point.x,
          image->chromaticity.white_point.y);
        (void) WriteBlobString(image,buffer);
      }
    if (image->orientation != UndefinedOrientation)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"orientation=%s\n",
          CommandOptionToMnemonic(MagickOrientationOptions,image->orientation));
        (void) WriteBlobString(image,buffer);
      }
    if (image->profiles != (void *) NULL)
      {
        const char
          *name;

        const StringInfo
          *profile;

        /*
          Write image profile names.
        */
        ResetImageProfileIterator(image);
        for (name=GetNextImageProfile(image); name != (const char *) NULL; )
        {
          profile=GetImageProfile(image,name);
          if (profile != (StringInfo *) NULL)
            {
              (void) FormatLocaleString(buffer,MagickPathExtent,"profile=%s\n",
                name);
              (void) WriteBlobString(image,buffer);
            }
          name=GetNextImageProfile(image);
        }
      }
    if (image->montage != (char *) NULL)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"montage=%s\n",
          image->montage);
        (void) WriteBlobString(image,buffer);
      }
    if (quantum_info->format == FloatingPointQuantumFormat)
      (void) SetImageProperty(image,"quantum:format","floating-point",
        exception);
    ResetImagePropertyIterator(image);
    property=GetNextImageProperty(image);
    while (property != (const char *) NULL)
    {
      (void) FormatLocaleString(buffer,MagickPathExtent,"%s=",property);
      (void) WriteBlobString(image,buffer);
      value=GetImageProperty(image,property,exception);
      if (value != (const char *) NULL)
        {
          length=strlen(value);
          for (i=0; i < (ssize_t) length; i++)
            if ((isspace((int) ((unsigned char) value[i])) != 0) ||
                (value[i] == '}'))
              break;
          if ((i == (ssize_t) length) && (i != 0))
            (void) WriteBlob(image,length,(const unsigned char *) value);
          else
            {
              (void) WriteBlobByte(image,'{');
              if (strchr(value,'}') == (char *) NULL)
                (void) WriteBlob(image,length,(const unsigned char *) value);
              else
                for (i=0; i < (ssize_t) length; i++)
                {
                  if (value[i] == (int) '}')
                    (void) WriteBlobByte(image,'\\');
                  (void) WriteBlobByte(image,(unsigned char) value[i]);
                }
              (void) WriteBlobByte(image,'}');
            }
        }
      (void) WriteBlobByte(image,'\n');
      property=GetNextImageProperty(image);
    }
    (void) WriteBlobString(image,"\f\n:\032");
    if (image->montage != (char *) NULL)
      {
        /*
          Write montage tile directory.
        */
        if (image->directory != (char *) NULL)
          (void) WriteBlob(image,strlen(image->directory),(unsigned char *)
            image->directory);
        (void) WriteBlobByte(image,'\0');
      }
    if (image->profiles != 0)
      {
        const char
          *name;

        const StringInfo
          *profile;

        /*
          Write image profile blob.
        */
        ResetImageProfileIterator(image);
        name=GetNextImageProfile(image);
        while (name != (const char *) NULL)
        {
          profile=GetImageProfile(image,name);
          (void) WriteBlobMSBLong(image,(unsigned int)
            GetStringInfoLength(profile));
          (void) WriteBlob(image,GetStringInfoLength(profile),
            GetStringInfoDatum(profile));
          name=GetNextImageProfile(image);
        }
      }
    if (image->storage_class == PseudoClass)
      {
        size_t
          colormap_size;

        unsigned char
          *colormap;

        /*
          Allocate colormap.
        */
        colormap_size=(size_t) (3*image->depth/8);
        colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          colormap_size*sizeof(*colormap));
        if (colormap == (unsigned char *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        /*
          Write colormap to file.
        */
        q=colormap;
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          switch (image->depth)
          {
            case 32:
            default:
            {
              unsigned int
                long_pixel;

              long_pixel=ScaleQuantumToLong((Quantum)
                image->colormap[i].red);
              q=PopLongPixel(MSBEndian,long_pixel,q);
              long_pixel=ScaleQuantumToLong((Quantum)
                image->colormap[i].green);
              q=PopLongPixel(MSBEndian,long_pixel,q);
              long_pixel=ScaleQuantumToLong((Quantum)
                image->colormap[i].blue);
              q=PopLongPixel(MSBEndian,long_pixel,q);
              break;
            }
            case 16:
            {
              unsigned short
                short_pixel;

              short_pixel=ScaleQuantumToShort((Quantum)
                image->colormap[i].red);
              q=PopShortPixel(MSBEndian,short_pixel,q);
              short_pixel=ScaleQuantumToShort((Quantum)
                image->colormap[i].green);
              q=PopShortPixel(MSBEndian,short_pixel,q);
              short_pixel=ScaleQuantumToShort((Quantum)
                image->colormap[i].blue);
              q=PopShortPixel(MSBEndian,short_pixel,q);
              break;
            }
            case 8:
            {
              unsigned char
                char_pixel;

              char_pixel=(unsigned char) ScaleQuantumToChar((Quantum)
                image->colormap[i].red);
              q=PopCharPixel(char_pixel,q);
              char_pixel=(unsigned char) ScaleQuantumToChar((Quantum)
                image->colormap[i].green);
              q=PopCharPixel(char_pixel,q);
              char_pixel=(unsigned char) ScaleQuantumToChar((Quantum)
                image->colormap[i].blue);
              q=PopCharPixel(char_pixel,q);
              break;
            }
          }
        }
        (void) WriteBlob(image,colormap_size*image->colors,colormap);
        colormap=(unsigned char *) RelinquishMagickMemory(colormap);
      }
    /*
      Write image pixels to file.
    */
    status=MagickTrue;
    switch (compression)
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      case BZipCompression:
      {
        int
          code;

        (void) memset(&bzip_info,0,sizeof(bzip_info));
        bzip_info.bzalloc=AcquireBZIPMemory;
        bzip_info.bzfree=RelinquishBZIPMemory;
        bzip_info.opaque=(void *) NULL;
        code=BZ2_bzCompressInit(&bzip_info,(int) (image->quality ==
          UndefinedCompressionQuality ? 7 : MagickMin(image->quality/10,9)),
          (int) image_info->verbose,0);
        if (code != BZ_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
      case LZMACompression:
      {
        int
          code;

        allocator.alloc=AcquireLZMAMemory;
        allocator.free=RelinquishLZMAMemory;
        allocator.opaque=(void *) NULL;
        lzma_info=initialize_lzma;
        lzma_info.allocator=&allocator;
        code=(int) lzma_easy_encoder(&lzma_info,(uint32_t) (image->quality/10),
          LZMA_CHECK_SHA256);
        if (code != LZMA_OK)
          status=MagickTrue;
        break;
      }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      case LZWCompression:
      case ZipCompression:
      {
        int
          code;

        zip_info.zalloc=AcquireZIPMemory;
        zip_info.zfree=RelinquishZIPMemory;
        zip_info.opaque=(void *) NULL;
        code=deflateInit(&zip_info,(int) (image->quality ==
          UndefinedCompressionQuality ? 7 : MagickMin(image->quality/10,9)));
        if (code != Z_OK)
          status=MagickFalse;
        break;
      }
#endif
      default:
        break;
    }
    quantum_type=GetQuantumType(image,exception);
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      const Quantum
        *magick_restrict p;

      ssize_t
        count,
        x;

      if (status == MagickFalse)
        break;
      count=0;
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      q=pixels;
      switch (compression)
      {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
        case BZipCompression:
        {
          bzip_info.next_in=(char *) pixels;
          bzip_info.avail_in=(unsigned int) (packet_size*image->columns);
          (void) ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          do
          {
            int
              code;

            bzip_info.next_out=(char *) compress_pixels;
            bzip_info.avail_out=(unsigned int) BZipMaxExtent(packet_size*
              image->columns);
            code=BZ2_bzCompress(&bzip_info,BZ_FLUSH);
            if (code < 0)
              status=MagickFalse;
            length=(size_t) (bzip_info.next_out-(char *) compress_pixels);
            count=0;
            if (length != 0)
              {
                (void) WriteBlobMSBLong(image,(unsigned int) length);
                count=WriteBlob(image,length,compress_pixels);
              }
          } while (bzip_info.avail_in != 0);
          break;
        }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
        case LZMACompression:
        {
          lzma_info.next_in=pixels;
          lzma_info.avail_in=packet_size*image->columns;
          (void) ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          do
          {
            int
              code;

            lzma_info.next_out=compress_pixels;
            lzma_info.avail_out=LZMAMaxExtent(packet_size*image->columns);
            code=(int) lzma_code(&lzma_info,LZMA_RUN);
            if (code != LZMA_OK)
              status=MagickFalse;
            length=(size_t) (lzma_info.next_out-compress_pixels);
            count=0;
            if (length != 0)
              {
                (void) WriteBlobMSBLong(image,(unsigned int) length);
                count=WriteBlob(image,length,compress_pixels);
              }
          } while (lzma_info.avail_in != 0);
          break;
        }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
        case LZWCompression:
        case ZipCompression:
        {
          zip_info.next_in=pixels;
          zip_info.avail_in=(uInt) (packet_size*image->columns);
          (void) ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          do
          {
            int
              code;

            zip_info.next_out=compress_pixels;
            zip_info.avail_out=(uInt) ZipMaxExtent(packet_size*image->columns);
            code=deflate(&zip_info,Z_SYNC_FLUSH);
            if (code != Z_OK)
              status=MagickFalse;
            length=(size_t) (zip_info.next_out-compress_pixels);
            count=0;
            if (length != 0)
              {
                (void) WriteBlobMSBLong(image,(unsigned int) length);
                count=WriteBlob(image,length,compress_pixels);
              }
          } while (zip_info.avail_in != 0);
          break;
        }
#endif
        case RLECompression:
        {
          length=0;
          GetPixelInfoPixel(image,p,&pixel);
          p+=GetPixelChannels(image);
          for (x=1; x < (ssize_t) image->columns; x++)
          {
            GetPixelInfoPixel(image,p,&target);
            if ((length < 255) &&
                (IsPixelInfoEquivalent(&pixel,&target) != MagickFalse))
              length++;
            else
              {
                q=PopRunlengthPacket(image,q,length,&pixel);
                length=0;
              }
            GetPixelInfoPixel(image,p,&pixel);
            p+=GetPixelChannels(image);
          }
          q=PopRunlengthPacket(image,q,length,&pixel);
          length=(size_t) (q-pixels);
          count=WriteBlob(image,length,pixels);
          break;
        }
        default:
        {
          (void) ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          length=packet_size*image->columns;
          count=WriteBlob(image,length,pixels);
          break;
        }
      }
      if (length != (size_t) count)
        {
          status=MagickFalse;
          break;
        }
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    switch (compression)
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      case BZipCompression:
      {
        int
          code;

        for ( ; ; )
        {
          if (status == MagickFalse)
            break;
          bzip_info.next_out=(char *) compress_pixels;
          bzip_info.avail_out=(unsigned int) BZipMaxExtent(packet_size*
            image->columns);
          code=BZ2_bzCompress(&bzip_info,BZ_FINISH);
          length=(size_t) (bzip_info.next_out-(char *) compress_pixels);
          if (length != 0)
            {
              (void) WriteBlobMSBLong(image,(unsigned int) length);
              (void) WriteBlob(image,length,compress_pixels);
            }
          if (code == BZ_STREAM_END)
            break;
        }
        code=BZ2_bzCompressEnd(&bzip_info);
        if (code != BZ_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
      case LZMACompression:
      {
        int
          code;

        for ( ; ; )
        {
          if (status == MagickFalse)
            break;
          lzma_info.next_out=compress_pixels;
          lzma_info.avail_out=packet_size*image->columns;
          code=(int) lzma_code(&lzma_info,LZMA_FINISH);
          length=(size_t) (lzma_info.next_out-compress_pixels);
          if (length > 6)
            {
              (void) WriteBlobMSBLong(image,(unsigned int) length);
              (void) WriteBlob(image,length,compress_pixels);
            }
          if (code == LZMA_STREAM_END)
            break;
        }
        lzma_end(&lzma_info);
        break;
      }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      case LZWCompression:
      case ZipCompression:
      {
        int
          code;

        for ( ; ; )
        {
          if (status == MagickFalse)
            break;
          zip_info.next_out=compress_pixels;
          zip_info.avail_out=(uInt) ZipMaxExtent(packet_size*image->columns);
          code=deflate(&zip_info,Z_FINISH);
          length=(size_t) (zip_info.next_out-compress_pixels);
          if (length > 6)
            {
              (void) WriteBlobMSBLong(image,(unsigned int) length);
              (void) WriteBlob(image,length,compress_pixels);
            }
          if (code == Z_STREAM_END)
            break;
        }
        code=deflateEnd(&zip_info);
        if (code != Z_OK)
          status=MagickFalse;
        break;
      }
#endif
      default:
        break;
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(status);
}

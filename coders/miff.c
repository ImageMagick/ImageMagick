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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/module.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include "zlib.h"
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
#include "bzlib.h"
#endif

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMIFFImage(const ImageInfo *,Image *);

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

#if defined(MAGICKCORE_BZLIB_DELEGATE)
static void *AcquireBZIPMemory(void *context,int items,int size)
{
  (void) context;
  return((void *) AcquireQuantumMemory((size_t) items,(size_t) size));
}
#endif

#if defined(MAGICKCORE_ZLIB_DELEGATE)
static voidpf AcquireZIPMemory(voidpf context,unsigned int items,
  unsigned int size)
{
  (void) context;
  return((voidpf) AcquireQuantumMemory(items,size));
}
#endif

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static void PushRunlengthPacket(Image *image,const unsigned char *pixels,
  size_t *length,PixelPacket *pixel,IndexPacket *index)
{
  const unsigned char
    *p;

  p=pixels;
  if (image->storage_class == PseudoClass)
    {
      *index=(IndexPacket) 0;
      switch (image->depth)
      {
        case 32:
        {
          *index=ConstrainColormapIndex(image,
            (*p << 24) | (*(p+1) << 16) | (*(p+2) << 8) | *(p+3));
          p+=4;
          break;
        }
        case 16:
        {
          *index=ConstrainColormapIndex(image,(*p << 8) | *(p+1));
          p+=2;
          break;
        }
        case 8:
        {
          *index=ConstrainColormapIndex(image,*p);
          p++;
          break;
        }
        default:
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            CorruptImageError,"ImageDepthNotSupported","`%s'",image->filename);
      }
      *pixel=image->colormap[(long) *index];
      switch (image->depth)
      {
        case 8:
        {
          unsigned char
            quantum;

          if (image->matte != MagickFalse)
            {
              p=PushCharPixel(p,&quantum);
              pixel->opacity=ScaleCharToQuantum(quantum);
            }
          break;
        }
        case 16:
        {
          unsigned short
            quantum;

          if (image->matte != MagickFalse)
            {
              p=PushShortPixel(MSBEndian,p,&quantum);
              pixel->opacity=(Quantum) (quantum >> (image->depth-
                MAGICKCORE_QUANTUM_DEPTH));
            }
          break;
        }
        case 32:
        {
          unsigned long
            quantum;

          if (image->matte != MagickFalse)
            {
              p=PushLongPixel(MSBEndian,p,&quantum);
              pixel->opacity=(Quantum) (quantum >> (image->depth-
                MAGICKCORE_QUANTUM_DEPTH));
            }
          break;
        }
        default:
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            CorruptImageError,"ImageDepthNotSupported","`%s'",image->filename);
      }
      *length=(size_t) (*p++)+1;
      return;
    }
  switch (image->depth)
  {
    case 8:
    {
      unsigned char
        quantum;

      p=PushCharPixel(p,&quantum);
      pixel->red=ScaleCharToQuantum(quantum);
      p=PushCharPixel(p,&quantum);
      pixel->green=ScaleCharToQuantum(quantum);
      p=PushCharPixel(p,&quantum);
      pixel->blue=ScaleCharToQuantum(quantum);
      if (image->matte != MagickFalse)
        {
          p=PushCharPixel(p,&quantum);
          pixel->opacity=ScaleCharToQuantum(quantum);
        }
      if (image->colorspace == CMYKColorspace)
        {
          p=PushCharPixel(p,&quantum);
          *index=ScaleCharToQuantum(quantum);
        }
      break;
    }
    case 16:
    {
      unsigned short
        quantum;

      p=PushShortPixel(MSBEndian,p,&quantum);
      pixel->red=(Quantum) (quantum >> (image->depth-MAGICKCORE_QUANTUM_DEPTH));
      p=PushShortPixel(MSBEndian,p,&quantum);
      pixel->green=(Quantum) (quantum >> (image->depth-
        MAGICKCORE_QUANTUM_DEPTH));
      p=PushShortPixel(MSBEndian,p,&quantum);
      pixel->blue=(Quantum) (quantum >> (image->depth-
        MAGICKCORE_QUANTUM_DEPTH));
      if (image->matte != MagickFalse)
        {
          p=PushShortPixel(MSBEndian,p,&quantum);
          pixel->opacity=(Quantum) (quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      if (image->colorspace == CMYKColorspace)
        {
          p=PushShortPixel(MSBEndian,p,&quantum);
          *index=(IndexPacket) (quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      break;
    }
    case 32:
    {
      unsigned long
        quantum;

      p=PushLongPixel(MSBEndian,p,&quantum);
      pixel->red=(Quantum) (quantum >> (image->depth-MAGICKCORE_QUANTUM_DEPTH));
      p=PushLongPixel(MSBEndian,p,&quantum);
      pixel->green=(Quantum) (quantum >> (image->depth-
        MAGICKCORE_QUANTUM_DEPTH));
      p=PushLongPixel(MSBEndian,p,&quantum);
      pixel->blue=(Quantum) (quantum >> (image->depth-
        MAGICKCORE_QUANTUM_DEPTH));
      if (image->matte != MagickFalse)
        {
          p=PushLongPixel(MSBEndian,p,&quantum);
          pixel->opacity=(Quantum) (quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      if (image->colorspace == CMYKColorspace)
        {
          p=PushLongPixel(MSBEndian,p,&quantum);
          *index=(IndexPacket) (quantum >> (image->depth-
            MAGICKCORE_QUANTUM_DEPTH));
        }
      break;
    }
    default:
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        CorruptImageError,"ImageDepthNotSupported","`%s'",image->filename);
  }
  *length=(size_t) (*p++)+1;
}

#if defined(MAGICKCORE_ZLIB_DELEGATE)
static void RelinquishZIPMemory(voidpf context,voidpf memory)
{
  (void) context;
  memory=RelinquishMagickMemory(memory);
}
#endif

#if defined(MAGICKCORE_BZLIB_DELEGATE)
static void RelinquishBZIPMemory(void *context,void *memory)
{
  (void) context;
  memory=RelinquishMagickMemory(memory);
}
#endif

static Image *ReadMIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define BZipMaxExtent(x)  ((x)+((x)/100)+600)
#define ZipMaxExtent(x)  ((x)+(((x)+7) >> 3)+(((x)+63) >> 6)+11)

#if defined(MAGICKCORE_BZLIB_DELEGATE)
  bz_stream
    bzip_info;
#endif

  char
    id[MaxTextExtent],
    keyword[MaxTextExtent],
    *options;

  const unsigned char
    *p;

  double
    version;

  GeometryInfo
    geometry_info;

  Image
    *image;

  IndexPacket
    index;

  int
    c,
    code;

  LinkedListInfo
    *profiles;

  long
    y;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  PixelPacket
    pixel;

  QuantumFormatType
    quantum_format;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register long
    i;

  size_t
    length,
    packet_size;

  ssize_t
    count;

  unsigned char
    *compress_pixels,
    *pixels;

  unsigned long
    colors;

#if defined(MAGICKCORE_ZLIB_DELEGATE)
  z_stream
    zip_info;
#endif

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
    Decode image header;  header terminates one character beyond a ':'.
  */
  c=ReadBlobByte(image);
  if (c == EOF)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  code=0;
  *id='\0';
  (void) ResetMagickMemory(keyword,0,sizeof(keyword));
  version=0.0;
  do
  {
    /*
      Decode image header;  header terminates one character beyond a ':'.
    */
    length=MaxTextExtent;
    options=AcquireString((char *) NULL);
    quantum_format=UndefinedQuantumFormat;
    profiles=(LinkedListInfo *) NULL;
    colors=0;
    image->depth=8UL;
    image->compression=NoCompression;
    while ((isgraph(c) != MagickFalse) && (c != (int) ':'))
    {
      register char
        *p;

      if (c == (int) '{')
        {
          char
            *comment;

          /*
            Read comment-- any text between { }.
          */
          length=MaxTextExtent;
          comment=AcquireString((char *) NULL);
          for (p=comment; comment != (char *) NULL; p++)
          {
            c=ReadBlobByte(image);
            if ((c == EOF) || (c == (int) '}'))
              break;
            if ((size_t) (p-comment+1) >= length)
              {
                *p='\0';
                length<<=1;
                comment=(char *) ResizeQuantumMemory(comment,length+
                  MaxTextExtent,sizeof(*comment));
                if (comment == (char *) NULL)
                  break;
                p=comment+strlen(comment);
              }
            *p=(char) c;
          }
          if (comment == (char *) NULL)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          *p='\0';
          (void) SetImageProperty(image,"comment",comment);
          comment=DestroyString(comment);
          c=ReadBlobByte(image);
        }
      else
        if (isalnum(c) != MagickFalse)
          {
            /*
              Get the keyword.
            */
            p=keyword;
            do
            {
              if (isspace((int) ((unsigned char) c)) != 0)
                break;
              if (c == (int) '=')
                break;
              if ((size_t) (p-keyword) < (MaxTextExtent-1))
                *p++=(char) c;
              c=ReadBlobByte(image);
            } while (c != EOF);
            *p='\0';
            p=options;
            while (isspace((int) ((unsigned char) c)) != 0)
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
                        MaxTextExtent,sizeof(*options));
                      if (options == (char *) NULL)
                        break;
                      p=options+strlen(options);
                    }
                  if (options == (char *) NULL)
                    ThrowReaderException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  *p++=(char) c;
                  c=ReadBlobByte(image);
                  if (*options != '{')
                    if (isspace((int) ((unsigned char) c)) != 0)
                      break;
                }
              }
            *p='\0';
            if (*options == '{')
              (void) CopyMagickString(options,options+1,MaxTextExtent);
            /*
              Assign a value to the specified keyword.
            */
            switch (*keyword)
            {
              case 'b':
              case 'B':
              {
                if (LocaleCompare(keyword,"background-color") == 0)
                  {
                    (void) QueryColorDatabase(options,&image->background_color,
                      exception);
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
                    (void) QueryColorDatabase(options,&image->border_color,
                      exception);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'c':
              case 'C':
              {
                if (LocaleCompare(keyword,"class") == 0)
                  {
                    long
                      storage_class;

                    storage_class=ParseMagickOption(MagickClassOptions,
                      MagickFalse,options);
                    if (storage_class < 0)
                      break;
                    image->storage_class=(ClassType) storage_class;
                    break;
                  }
                if (LocaleCompare(keyword,"colors") == 0)
                  {
                    colors=(unsigned long) atol(options);
                    break;
                  }
                if (LocaleCompare(keyword,"colorspace") == 0)
                  {
                    long
                      colorspace;

                    colorspace=ParseMagickOption(MagickColorspaceOptions,
                      MagickFalse,options);
                    if (colorspace < 0)
                      break;
                    image->colorspace=(ColorspaceType) colorspace;
                    break;
                  }
                if (LocaleCompare(keyword,"compression") == 0)
                  {
                    long
                      compression;

                    compression=ParseMagickOption(MagickCompressOptions,
                      MagickFalse,options);
                    if (compression < 0)
                      break;
                    image->compression=(CompressionType) compression;
                    break;
                  }
                if (LocaleCompare(keyword,"columns") == 0)
                  {
                    image->columns=(unsigned long) atol(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'd':
              case 'D':
              {
                if (LocaleCompare(keyword,"delay") == 0)
                  {
                    image->delay=(unsigned long) atol(options);
                    break;
                  }
                if (LocaleCompare(keyword,"depth") == 0)
                  {
                    image->depth=(unsigned long) atol(options);
                    break;
                  }
                if (LocaleCompare(keyword,"dispose") == 0)
                  {
                    long
                      dispose;

                    dispose=ParseMagickOption(MagickDisposeOptions,MagickFalse,
                      options);
                    if (dispose < 0)
                      break;
                    image->dispose=(DisposeType) dispose;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'e':
              case 'E':
              {
                if (LocaleCompare(keyword,"endian") == 0)
                  {
                    long
                      endian;

                    endian=ParseMagickOption(MagickEndianOptions,MagickFalse,
                      options);
                    if (endian < 0)
                      break;
                    image->endian=(EndianType) endian;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'g':
              case 'G':
              {
                if (LocaleCompare(keyword,"gamma") == 0)
                  {
                    image->gamma=atof(options);
                    break;
                  }
                if (LocaleCompare(keyword,"gravity") == 0)
                  {
                    long
                      gravity;

                    gravity=ParseMagickOption(MagickGravityOptions,MagickFalse,
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
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'i':
              case 'I':
              {
                if (LocaleCompare(keyword,"id") == 0)
                  {
                    (void) CopyMagickString(id,options,MaxTextExtent);
                    break;
                  }
                if (LocaleCompare(keyword,"iterations") == 0)
                  {
                    image->iterations=(unsigned long) atol(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'm':
              case 'M':
              {
                if (LocaleCompare(keyword,"matte") == 0)
                  {
                    long
                      matte;

                    matte=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                      options);
                    if (matte < 0)
                      break;
                    image->matte=(MagickBooleanType) matte;
                    break;
                  }
                if (LocaleCompare(keyword,"matte-color") == 0)
                  {
                    (void) QueryColorDatabase(options,&image->matte_color,
                      exception);
                    break;
                  }
                if (LocaleCompare(keyword,"montage") == 0)
                  {
                    (void) CloneString(&image->montage,options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'o':
              case 'O':
              {
                if (LocaleCompare(keyword,"opaque") == 0)
                  {
                    long
                      matte;

                    matte=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                      options);
                    if (matte < 0)
                      break;
                    image->matte=(MagickBooleanType) matte;
                    break;
                  }
                if (LocaleCompare(keyword,"orientation") == 0)
                  {
                    long
                      orientation;

                    orientation=ParseMagickOption(MagickOrientationOptions,
                      MagickFalse,options);
                    if (orientation < 0)
                      break;
                    image->orientation=(OrientationType) orientation;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
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
                if ((LocaleNCompare(keyword,"profile:",8) == 0) ||
                    (LocaleNCompare(keyword,"profile-",8) == 0))
                  {
                    StringInfo
                      *profile;

                    if (profiles == (LinkedListInfo *) NULL)
                      profiles=NewLinkedList(0);
                    (void) AppendValueToLinkedList(profiles,
                      AcquireString(keyword+8));
                    profile=AcquireStringInfo((size_t) atol(options));
                    (void) SetImageProfile(image,keyword+8,profile);
                    profile=DestroyStringInfo(profile);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'q':
              case 'Q':
              {
                if (LocaleCompare(keyword,"quality") == 0)
                  {
                    image->quality=(unsigned long) atol(options);
                    break;
                  }
                if ((LocaleCompare(keyword,"quantum-format") == 0) ||
                    (LocaleCompare(keyword,"quantum:format") == 0))
                  {
                    long
                      format;

                    format=ParseMagickOption(MagickQuantumFormatOptions,
                      MagickFalse,options);
                    if (format < 0)
                      break;
                    quantum_format=(QuantumFormatType) format;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
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
                    long
                      rendering_intent;

                    rendering_intent=ParseMagickOption(MagickIntentOptions,
                      MagickFalse,options);
                    if (rendering_intent < 0)
                      break;
                    image->rendering_intent=(RenderingIntent) rendering_intent;
                    break;
                  }
                if (LocaleCompare(keyword,"resolution") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    image->x_resolution=geometry_info.rho;
                    image->y_resolution=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      image->y_resolution=image->x_resolution;
                    break;
                  }
                if (LocaleCompare(keyword,"rows") == 0)
                  {
                    image->rows=(unsigned long) atol(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 's':
              case 'S':
              {
                if (LocaleCompare(keyword,"scene") == 0)
                  {
                    image->scene=(unsigned long) atol(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 't':
              case 'T':
              {
                if (LocaleCompare(keyword,"ticks-per-second") == 0)
                  {
                    image->ticks_per_second=atol(options);
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
                    long
                      type;

                    type=ParseMagickOption(MagickTypeOptions,MagickFalse,
                      options);
                    if (type < 0)
                      break;
                    image->type=(ImageType) type;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'u':
              case 'U':
              {
                if (LocaleCompare(keyword,"units") == 0)
                  {
                    long
                      units;

                    units=ParseMagickOption(MagickResolutionOptions,MagickFalse,
                      options);
                    if (units < 0)
                      break;
                    image->units=(ResolutionType) units;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'v':
              case 'V':
              {
                if (LocaleCompare(keyword,"version") == 0)
                  {
                    version=atof(options);
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              case 'w':
              case 'W':
              {
                if (LocaleCompare(keyword,"white-point") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    image->chromaticity.white_point.x=geometry_info.rho;
                    image->chromaticity.white_point.y=geometry_info.rho;
                    if ((flags & SigmaValue) != 0)
                      image->chromaticity.white_point.y=
                        image->chromaticity.white_point.x;
                    break;
                  }
                (void) SetImageProperty(image,keyword,options);
                break;
              }
              default:
              {
                (void) SetImageProperty(image,keyword,options);
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
        (image->columns == 0) || (image->rows == 0))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if (image->montage != (char *) NULL)
      {
        register char
          *p;

        /*
          Image directory.
        */
        length=MaxTextExtent;
        image->directory=AcquireString((char *) NULL);
        p=image->directory;
        do
        {
          *p='\0';
          if ((strlen(image->directory)+MaxTextExtent) >= length)
            {
              /*
                Allocate more memory for the image directory.
              */
              length<<=1;
              image->directory=(char *) ResizeQuantumMemory(image->directory,
                length+MaxTextExtent,sizeof(*image->directory));
              if (image->directory == (char *) NULL)
                ThrowReaderException(CorruptImageError,"UnableToReadImageData");
              p=image->directory+strlen(image->directory);
            }
          c=ReadBlobByte(image);
          *p++=(char) c;
        } while (c != (int) '\0');
      }
    if (profiles != (LinkedListInfo *) NULL)
      {
        const char
          *name;

        const StringInfo
          *profile;

        /*
          Read image profiles.
        */
        ResetLinkedListIterator(profiles);
        name=(const char *) GetNextValueInLinkedList(profiles);
        while (name != (const char *) NULL)
        {
          profile=GetImageProfile(image,name);
          if (profile != (StringInfo *) NULL)
            {
              register unsigned char
                *p;

              p=GetStringInfoDatum(profile);
              count=ReadBlob(image,GetStringInfoLength(profile),p);
            }
          name=(const char *) GetNextValueInLinkedList(profiles);
        }
        profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
      }
    image->depth=GetImageQuantumDepth(image,MagickFalse);
    if (image->storage_class == PseudoClass)
      {
        /*
          Create image colormap.
        */
        status=AcquireImageColormap(image,colors != 0 ? colors : 256);
        if (status == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        if (colors != 0)
          {
            size_t
              packet_size;

            unsigned char
              *colormap;

            /*
              Read image colormap from file.
            */
            packet_size=(size_t) (3UL*image->depth/8UL);
            colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
              packet_size*sizeof(*colormap));
            if (colormap == (unsigned char *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            count=ReadBlob(image,packet_size*image->colors,colormap);
            p=colormap;
            switch (image->depth)
            {
              default:
                ThrowReaderException(CorruptImageError,
                  "ImageDepthNotSupported");
              case 8:
              {
                unsigned char
                  pixel;

                for (i=0; i < (long) image->colors; i++)
                {
                  p=PushCharPixel(p,&pixel);
                  image->colormap[i].red=ScaleCharToQuantum(pixel);
                  p=PushCharPixel(p,&pixel);
                  image->colormap[i].green=ScaleCharToQuantum(pixel);
                  p=PushCharPixel(p,&pixel);
                  image->colormap[i].blue=ScaleCharToQuantum(pixel);
                }
                break;
              }
              case 16:
              {
                unsigned short
                  pixel;

                for (i=0; i < (long) image->colors; i++)
                {
                  p=PushShortPixel(MSBEndian,p,&pixel);
                  image->colormap[i].red=ScaleShortToQuantum(pixel);
                  p=PushShortPixel(MSBEndian,p,&pixel);
                  image->colormap[i].green=ScaleShortToQuantum(pixel);
                  p=PushShortPixel(MSBEndian,p,&pixel);
                  image->colormap[i].blue=ScaleShortToQuantum(pixel);
                }
                break;
              }
              case 32:
              {
                unsigned long
                  pixel;

                for (i=0; i < (long) image->colors; i++)
                {
                  p=PushLongPixel(MSBEndian,p,&pixel);
                  image->colormap[i].red=ScaleLongToQuantum(pixel);
                  p=PushLongPixel(MSBEndian,p,&pixel);
                  image->colormap[i].green=ScaleLongToQuantum(pixel);
                  p=PushLongPixel(MSBEndian,p,&pixel);
                  image->colormap[i].blue=ScaleLongToQuantum(pixel);
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
    /*
      Allocate image pixels.
    */
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    if (quantum_format != UndefinedQuantumFormat)
      {
        status=SetQuantumFormat(image,quantum_info,quantum_format);
        if (status == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    packet_size=(size_t) (quantum_info->depth/8);
    if (image->storage_class == DirectClass)
      packet_size=(size_t) (3*quantum_info->depth/8);
    if (image->matte != MagickFalse)
      packet_size+=quantum_info->depth/8;
    if (image->colorspace == CMYKColorspace)
      packet_size+=quantum_info->depth/8;
    if (image->compression == RLECompression)
      packet_size++;
    length=image->columns;
    length=MagickMax(BZipMaxExtent(packet_size*image->columns),ZipMaxExtent(
      packet_size*image->columns));
    compress_pixels=(unsigned char *) AcquireQuantumMemory(length,
      sizeof(*compress_pixels));
    if (compress_pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    /*
      Read image pixels.
    */
    quantum_type=RGBQuantum;
    if (image->matte != MagickFalse)
      quantum_type=RGBAQuantum;
    if (image->colorspace == CMYKColorspace)
      {
        quantum_type=CMYKQuantum;
        if (image->matte != MagickFalse)
          quantum_type=CMYKAQuantum;
      }
    if (image->storage_class == PseudoClass)
      {
        quantum_type=IndexQuantum;
        if (image->matte != MagickFalse)
          quantum_type=IndexAlphaQuantum;
      }
    if (image->colorspace == GRAYColorspace)
      {
        quantum_type=GrayQuantum;
        if (image->matte != MagickFalse)
          quantum_type=GrayAlphaQuantum;
      }
    pixels=GetQuantumPixels(quantum_info);
    index=(IndexPacket) 0;
    length=0;
    for (y=0; y < (long) image->rows; y++)
    {
      register IndexPacket
        *__restrict indexes;

      register long
        x;

      register PixelPacket
        *__restrict q;

      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        break;
      indexes=GetAuthenticIndexQueue(image);
      switch (image->compression)
      {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
        case LZWCompression:
        case ZipCompression:
        {
          if (y == 0)
            {
              zip_info.zalloc=AcquireZIPMemory;
              zip_info.zfree=RelinquishZIPMemory;
              zip_info.opaque=(voidpf) NULL;
              code=inflateInit(&zip_info);
              if (code >= 0)
                status=MagickTrue;
              zip_info.avail_in=0;
            }
          zip_info.next_out=pixels;
          zip_info.avail_out=(uInt) (packet_size*image->columns);
          do
          {
            if (zip_info.avail_in == 0)
              {
                zip_info.next_in=compress_pixels;
                length=(size_t) ZipMaxExtent(packet_size*image->columns);
                if (version != 0)
                  length=(size_t) ReadBlobMSBLong(image);
                zip_info.avail_in=(unsigned int) ReadBlob(image,length,
                  zip_info.next_in);
              }
            if (inflate(&zip_info,Z_SYNC_FLUSH) == Z_STREAM_END)
              break;
          } while (zip_info.avail_out != 0);
          if (y == (long) (image->rows-1))
            {
              if (version == 0)
                {
                  MagickOffsetType
                    offset;

                  offset=SeekBlob(image,-((MagickOffsetType) zip_info.avail_in),
                    SEEK_CUR);
                  if (offset < 0)
                    ThrowReaderException(CorruptImageError,
                      "ImproperImageHeader");
                }
              code=inflateEnd(&zip_info);
              if (code >= 0)
                status=MagickTrue;
            }
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
        case BZipCompression:
        {
          if (y == 0)
            {
              bzip_info.bzalloc=AcquireBZIPMemory;
              bzip_info.bzfree=RelinquishBZIPMemory;
              bzip_info.opaque=(void *) NULL;
              code=BZ2_bzDecompressInit(&bzip_info,(int) image_info->verbose,
                MagickFalse);
              if (code >= 0)
                status=MagickTrue;
              bzip_info.avail_in=0;
            }
          bzip_info.next_out=(char *) pixels;
          bzip_info.avail_out=(unsigned int) (packet_size*image->columns);
          do
          {
            if (bzip_info.avail_in == 0)
              {
                bzip_info.next_in=(char *) compress_pixels;
                length=(size_t) BZipMaxExtent(packet_size*image->columns);
                if (version != 0)
                  length=(size_t) ReadBlobMSBLong(image);
                bzip_info.avail_in=(unsigned int) ReadBlob(image,length,
                  (unsigned char *) bzip_info.next_in);
              }
            if (BZ2_bzDecompress(&bzip_info) == BZ_STREAM_END)
              break;
          } while (bzip_info.avail_out != 0);
          if (y == (long) (image->rows-1))
            {
              if (version == 0)
                {
                  MagickOffsetType
                    offset;

                  offset=SeekBlob(image,-((MagickOffsetType)
                    bzip_info.avail_in),SEEK_CUR);
                  if (offset < 0)
                    ThrowReaderException(CorruptImageError,
                      "ImproperImageHeader");
                }
              code=BZ2_bzDecompressEnd(&bzip_info);
              if (code >= 0)
                status=MagickTrue;
            }
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
        case RLECompression:
        {
          if (y == 0)
            {
              (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
              pixel.opacity=(Quantum) TransparentOpacity;
              index=(IndexPacket) 0;
            }
          for (x=0; x < (long) image->columns; x++)
          {
            if (length == 0)
              {
                count=ReadBlob(image,packet_size,pixels);
                PushRunlengthPacket(image,pixels,&length,&pixel,&index);
              }
            length--;
            if ((image->storage_class == PseudoClass) ||
                (image->colorspace == CMYKColorspace))
              indexes[x]=index;
            *q++=pixel;
          }
          break;
        }
        default:
        {
          count=ReadBlob(image,packet_size*image->columns,pixels);
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
    }
    SetQuantumImageType(image,quantum_type);
    quantum_info=DestroyQuantumInfo(quantum_info);
    compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels);
    if (((y != (long) image->rows)) || (status == MagickFalse))
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
    } while ((isgraph(c) == MagickFalse) && (c != EOF));
    if (c != EOF)
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
  } while (c != EOF);
  (void) CloseBlob(image);
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
%      unsigned long RegisterMIFFImage(void)
%
*/
ModuleExport unsigned long RegisterMIFFImage(void)
{
  char
    version[MaxTextExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(MagickImageCoderSignatureText)
  (void) CopyMagickString(version,MagickLibVersionText,MaxTextExtent);
#if defined(ZLIB_VERSION)
  (void) ConcatenateMagickString(version," with Zlib ",MaxTextExtent);
  (void) ConcatenateMagickString(version,ZLIB_VERSION,MaxTextExtent);
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  (void) ConcatenateMagickString(version," and BZlib",MaxTextExtent);
#endif
#endif
  entry=SetMagickInfo("MIFF");
  entry->decoder=(DecodeImageHandler *) ReadMIFFImage;
  entry->encoder=(EncodeImageHandler *) WriteMIFFImage;
  entry->magick=(IsImageFormatHandler *) IsMIFF;
  entry->description=ConstantString("Magick Image File Format");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("MIFF");
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
%        Image *image)
%
%  Compression code contributed by Kyle Shorter.
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
*/

static unsigned char *PopRunlengthPacket(Image *image,unsigned char *pixels,
  size_t length,PixelPacket pixel,IndexPacket index)
{
  if (image->storage_class != DirectClass)
    {
      switch (image->depth)
      {
        case 32:
        {
          *pixels++=(unsigned char) ((unsigned long) index >> 24);
          *pixels++=(unsigned char) ((unsigned long) index >> 16);
        }
        case 16:
          *pixels++=(unsigned char) ((unsigned long) index >> 8);
        case 8:
        {
          *pixels++=(unsigned char) index;
          break;
        }
        default:
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            CorruptImageError,"ImageDepthNotSupported","`%s'",image->filename);
      }
      switch (image->depth)
      {
        case 32:
        {
          unsigned long
            value;

          if (image->matte != MagickFalse)
            {
              value=ScaleQuantumToLong(pixel.opacity);
              pixels=PopLongPixel(MSBEndian,value,pixels);
            }
          break;
        }
        case 16:
        {
          unsigned short
            value;

          if (image->matte != MagickFalse)
            {
              value=ScaleQuantumToShort(pixel.opacity);
              pixels=PopShortPixel(MSBEndian,value,pixels);
            }
          break;
        }
        case 8:
        {
          unsigned char
            value;

          if (image->matte != MagickFalse)
            {
              value=(unsigned char) ScaleQuantumToChar(pixel.opacity);
              pixels=PopCharPixel(value,pixels);
            }
          break;
        }
        default:
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            CorruptImageError,"ImageDepthNotSupported","`%s'",image->filename);
      }
      *pixels++=(unsigned char) length;
      return(pixels);
    }
  switch (image->depth)
  {
    case 32:
    {
      unsigned long
        value;

      value=ScaleQuantumToLong(pixel.red);
      pixels=PopLongPixel(MSBEndian,value,pixels);
      value=ScaleQuantumToLong(pixel.green);
      pixels=PopLongPixel(MSBEndian,value,pixels);
      value=ScaleQuantumToLong(pixel.blue);
      pixels=PopLongPixel(MSBEndian,value,pixels);
      if (image->matte != MagickFalse)
        {
          value=ScaleQuantumToLong(pixel.opacity);
          pixels=PopLongPixel(MSBEndian,value,pixels);
        }
      if (image->colorspace == CMYKColorspace)
        {
          value=ScaleQuantumToLong(index);
          pixels=PopLongPixel(MSBEndian,value,pixels);
        }
      break;
    }
    case 16:
    {
      unsigned short
        value;

      value=ScaleQuantumToShort(pixel.red);
      pixels=PopShortPixel(MSBEndian,value,pixels);
      value=ScaleQuantumToShort(pixel.green);
      pixels=PopShortPixel(MSBEndian,value,pixels);
      value=ScaleQuantumToShort(pixel.blue);
      pixels=PopShortPixel(MSBEndian,value,pixels);
      if (image->matte != MagickFalse)
        {
          value=ScaleQuantumToShort(pixel.opacity);
          pixels=PopShortPixel(MSBEndian,value,pixels);
        }
      if (image->colorspace == CMYKColorspace)
        {
          value=ScaleQuantumToShort(index);
          pixels=PopShortPixel(MSBEndian,value,pixels);
        }
      break;
    }
    case 8:
    {
      unsigned char
        value;

      value=(unsigned char) ScaleQuantumToChar(pixel.red);
      pixels=PopCharPixel(value,pixels);
      value=(unsigned char) ScaleQuantumToChar(pixel.green);
      pixels=PopCharPixel(value,pixels);
      value=(unsigned char) ScaleQuantumToChar(pixel.blue);
      pixels=PopCharPixel(value,pixels);
      if (image->matte != MagickFalse)
        {
          value=(unsigned char) ScaleQuantumToChar(pixel.opacity);
          pixels=PopCharPixel(value,pixels);
        }
      if (image->colorspace == CMYKColorspace)
        {
          value=(unsigned char) ScaleQuantumToChar(index);
          pixels=PopCharPixel(value,pixels);
        }
      break;
    }
    default:
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        CorruptImageError,"ImageDepthNotSupported","`%s'",image->filename);
  }
  *pixels++=(unsigned char) length;
  return(pixels);
}

static MagickBooleanType WriteMIFFImage(const ImageInfo *image_info,
  Image *image)
{
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  bz_stream
    bzip_info;
#endif

  char
    buffer[MaxTextExtent];

  CompressionType
    compression;

  const char
    *property,
    *value;

  IndexPacket
    index;

  int
    code;

  long
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  PixelPacket
    pixel;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register long
    i;

  size_t
    length,
    packet_size;

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
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  code=0;
  scene=0;
  do
  {
    /*
      Allocate image pixels.
    */
    image->depth=image->depth <= 8 ? 8UL : image->depth <= 16 ? 16UL :
      image->depth <= 32 ? 32UL : 64UL;
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    if ((image->storage_class != PseudoClass) && (image->depth >= 32) &&
        (quantum_info->format == UndefinedQuantumFormat) &&
        (IsHighDynamicRangeImage(image,&image->exception) != MagickFalse))
      {
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      }
    if ((image->storage_class == PseudoClass) &&
        (image->colors > (unsigned long) (GetQuantumRange(image->depth)+1)))
      (void) SetImageStorageClass(image,DirectClass);
    if (IsGrayImage(image,&image->exception) != MagickFalse)
      {
        image->storage_class=DirectClass;
        (void) SetImageColorspace(image,GRAYColorspace);
      }
    compression=image->compression;
    if (image_info->compression != UndefinedCompression)
      compression=image_info->compression;
    switch (compression)
    {
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
        break;
      }
      default:
        break;
    }
    packet_size=(size_t) (quantum_info->depth/8);
    if (image->storage_class == DirectClass)
      packet_size=(size_t) (3*quantum_info->depth/8);
    if (image->matte != MagickFalse)
      packet_size+=quantum_info->depth/8;
    if (image->colorspace == CMYKColorspace)
      packet_size+=quantum_info->depth/8;
    if (compression == RLECompression)
      packet_size++;
    length=image->columns;
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
    (void) WriteBlobString(image,"id=ImageMagick  version=1.0\n");
    (void) FormatMagickString(buffer,MaxTextExtent,
      "class=%s  colors=%lu  matte=%s\n",MagickOptionToMnemonic(
      MagickClassOptions,image->storage_class),image->colors,
      MagickOptionToMnemonic(MagickBooleanOptions,(long) image->matte));
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"columns=%lu  rows=%lu  "
      "depth=%lu\n",image->columns,image->rows,image->depth);
    (void) WriteBlobString(image,buffer);
    if (image->type != UndefinedType)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"type=%s\n",
          MagickOptionToMnemonic(MagickTypeOptions,image->type));
        (void) WriteBlobString(image,buffer);
      }
    if (image->colorspace != UndefinedColorspace)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"colorspace=%s\n",
          MagickOptionToMnemonic(MagickColorspaceOptions,image->colorspace));
        (void) WriteBlobString(image,buffer);
      }
    if (compression != UndefinedCompression)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"compression=%s  "
          "quality=%lu\n",MagickOptionToMnemonic(MagickCompressOptions,
          compression),image->quality);
        (void) WriteBlobString(image,buffer);
      }
    if (image->units != UndefinedResolution)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"units=%s\n",
          MagickOptionToMnemonic(MagickResolutionOptions,image->units));
        (void) WriteBlobString(image,buffer);
      }
    if ((image->x_resolution != 0) || (image->y_resolution != 0))
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"resolution=%gx%g\n",
          image->x_resolution,image->y_resolution);
        (void) WriteBlobString(image,buffer);
      }
    if ((image->page.width != 0) || (image->page.height != 0))
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"page=%lux%lu%+ld%+ld\n",
          image->page.width,image->page.height,image->page.x,image->page.y);
        (void) WriteBlobString(image,buffer);
      }
    else
      if ((image->page.x != 0) || (image->page.y != 0))
        {
          (void) FormatMagickString(buffer,MaxTextExtent,"page=%+ld%+ld\n",
            image->page.x,image->page.y);
          (void) WriteBlobString(image,buffer);
        }
    if ((image->tile_offset.x != 0) || (image->tile_offset.y != 0))
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"tile-offset=%+ld%+ld\n",
          image->tile_offset.x,image->tile_offset.y);
        (void) WriteBlobString(image,buffer);
      }
    if ((GetNextImageInList(image) != (Image *) NULL) ||
        (GetPreviousImageInList(image) != (Image *) NULL))
      {
        if (image->scene == 0)
          (void) FormatMagickString(buffer,MaxTextExtent,"iterations=%lu  "
            "delay=%lu  ticks-per-second=%lu\n",image->iterations,image->delay,
             image->ticks_per_second);
        else
          (void) FormatMagickString(buffer,MaxTextExtent,"scene=%lu  "
            "iterations=%lu  delay=%lu  ticks-per-second=%lu\n",image->scene,
            image->iterations,image->delay,image->ticks_per_second);
        (void) WriteBlobString(image,buffer);
      }
    else
      {
        if (image->scene != 0)
          {
            (void) FormatMagickString(buffer,MaxTextExtent,"scene=%lu\n",
              image->scene);
            (void) WriteBlobString(image,buffer);
          }
        if (image->iterations != 0)
          {
            (void) FormatMagickString(buffer,MaxTextExtent,"iterations=%lu\n",
              image->iterations);
            (void) WriteBlobString(image,buffer);
          }
        if (image->delay != 0)
          {
            (void) FormatMagickString(buffer,MaxTextExtent,"delay=%lu\n",
              image->delay);
            (void) WriteBlobString(image,buffer);
          }
        if (image->ticks_per_second != UndefinedTicksPerSecond)
          {
            (void) FormatMagickString(buffer,MaxTextExtent,
              "ticks-per-second=%lu\n",image->ticks_per_second);
            (void) WriteBlobString(image,buffer);
          }
      }
    if (image->gravity != UndefinedGravity)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"gravity=%s\n",
          MagickOptionToMnemonic(MagickGravityOptions,image->gravity));
        (void) WriteBlobString(image,buffer);
      }
    if (image->dispose != UndefinedDispose)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"dispose=%s\n",
          MagickOptionToMnemonic(MagickDisposeOptions,image->dispose));
        (void) WriteBlobString(image,buffer);
      }
    if (image->rendering_intent != UndefinedIntent)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,
          "rendering-intent=%s\n",
           MagickOptionToMnemonic(MagickIntentOptions,image->rendering_intent));
        (void) WriteBlobString(image,buffer);
      }
    if (image->gamma != 0.0)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"gamma=%g\n",
          image->gamma);
        (void) WriteBlobString(image,buffer);
      }
    if (image->chromaticity.white_point.x != 0.0)
      {
        /*
          Note chomaticity points.
        */
        (void) FormatMagickString(buffer,MaxTextExtent,"red-primary=%g,%g  "
          "green-primary=%g,%g  blue-primary=%g,%g\n",
          image->chromaticity.red_primary.x,image->chromaticity.red_primary.y,
          image->chromaticity.green_primary.x,
          image->chromaticity.green_primary.y,
          image->chromaticity.blue_primary.x,
          image->chromaticity.blue_primary.y);
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"white-point=%g,%g\n",
          image->chromaticity.white_point.x,image->chromaticity.white_point.y);
        (void) WriteBlobString(image,buffer);
      }
    if (image->orientation != UndefinedOrientation)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"orientation=%s\n",
          MagickOptionToMnemonic(MagickOrientationOptions,image->orientation));
        (void) WriteBlobString(image,buffer);
      }
    if (image->profiles != (void *) NULL)
      {
        const char
          *name;

        const StringInfo
          *profile;

        /*
          Write image profiles.
        */
        ResetImageProfileIterator(image);
        name=GetNextImageProfile(image);
        while (name != (const char *) NULL)
        {
          profile=GetImageProfile(image,name);
          if (profile != (StringInfo *) NULL)
            {
              (void) FormatMagickString(buffer,MaxTextExtent,"profile:%s=%lu\n",
                name,(unsigned long) GetStringInfoLength(profile));
              (void) WriteBlobString(image,buffer);
            }
          name=GetNextImageProfile(image);
        }
      }
    if (image->montage != (char *) NULL)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"montage=%s\n",
          image->montage);
        (void) WriteBlobString(image,buffer);
      }
    if (quantum_info->format == FloatingPointQuantumFormat)
      SetImageProperty(image,"quantum:format","floating-point");
    ResetImagePropertyIterator(image);
    property=GetNextImageProperty(image);
    while (property != (const char *) NULL)
    {
      (void) FormatMagickString(buffer,MaxTextExtent,"%s=",property);
      (void) WriteBlobString(image,buffer);
      value=GetImageProperty(image,property);
      if (value != (const char *) NULL)
        {
          for (i=0; i < (long) strlen(value); i++)
            if (isspace((int) ((unsigned char) value[i])) != 0)
              break;
          if (i <= (long) strlen(value))
            (void) WriteBlobByte(image,'{');
          (void) WriteBlob(image,strlen(value),(const unsigned char *) value);
          if (i <= (long) strlen(value))
            (void) WriteBlobByte(image,'}');
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
    if (image->profiles != (void *) NULL)
      {
        const char
          *name;

        const StringInfo
          *profile;

        /*
          Generic profile.
        */
        ResetImageProfileIterator(image);
        name=GetNextImageProfile(image);
        while (name != (const char *) NULL)
        {
          profile=GetImageProfile(image,name);
          (void) WriteBlob(image,GetStringInfoLength(profile),
            GetStringInfoDatum(profile));
          name=GetNextImageProfile(image);
        }
      }
    if (image->storage_class == PseudoClass)
      {
        size_t
          packet_size;

        unsigned char
          *colormap,
          *q;

        /*
          Allocate colormap.
        */
        packet_size=(size_t) (3*quantum_info->depth/8);
        colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          packet_size*sizeof(*colormap));
        if (colormap == (unsigned char *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        /*
          Write colormap to file.
        */
        q=colormap;
        for (i=0; i < (long) image->colors; i++)
        {
          switch (quantum_info->depth)
          {
            default:
              ThrowWriterException(CorruptImageError,"ImageDepthNotSupported");
            case 32:
            {
              register unsigned long
                pixel;

              pixel=ScaleQuantumToLong(image->colormap[i].red);
              q=PopLongPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToLong(image->colormap[i].green);
              q=PopLongPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToLong(image->colormap[i].blue);
              q=PopLongPixel(MSBEndian,pixel,q);
              break;
            }
            case 16:
            {
              register unsigned short
                pixel;

              pixel=ScaleQuantumToShort(image->colormap[i].red);
              q=PopShortPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToShort(image->colormap[i].green);
              q=PopShortPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToShort(image->colormap[i].blue);
              q=PopShortPixel(MSBEndian,pixel,q);
              break;
            }
            case 8:
            {
              register unsigned char
                pixel;

              pixel=(unsigned char) ScaleQuantumToChar(image->colormap[i].red);
              q=PopCharPixel(pixel,q);
              pixel=(unsigned char) ScaleQuantumToChar(
                image->colormap[i].green);
              q=PopCharPixel(pixel,q);
              pixel=(unsigned char) ScaleQuantumToChar(image->colormap[i].blue);
              q=PopCharPixel(pixel,q);
              break;
            }
          }
        }
        (void) WriteBlob(image,packet_size*image->colors,colormap);
        colormap=(unsigned char *) RelinquishMagickMemory(colormap);
      }
    /*
      Write image pixels to file.
    */
    quantum_type=GetQuantumType(image,&image->exception);
    pixels=GetQuantumPixels(quantum_info);
    status=MagickTrue;
    for (y=0; y < (long) image->rows; y++)
    {
      register const IndexPacket
        *__restrict indexes;

      register const PixelPacket
        *__restrict p;

      register long
        x;

      p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      q=pixels;
      switch (compression)
      {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
        case LZWCompression:
        case ZipCompression:
        {
          if (y == 0)
            {
              zip_info.zalloc=AcquireZIPMemory;
              zip_info.zfree=RelinquishZIPMemory;
              zip_info.opaque=(voidpf) NULL;
              code=deflateInit(&zip_info,(int) (image->quality ==
                UndefinedCompressionQuality ? 7 : MagickMin(image->quality/10,
                9)));
              if (code >= 0)
                status=MagickTrue;
            }
          zip_info.next_in=pixels;
          zip_info.avail_in=(uInt) (packet_size*image->columns);
          (void) ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
            quantum_type,pixels,&image->exception);
          do
          {
            zip_info.next_out=compress_pixels;
            zip_info.avail_out=(uInt) ZipMaxExtent(packet_size*image->columns);
            code=deflate(&zip_info,Z_SYNC_FLUSH);
            if (code >= 0)
              status=MagickTrue;
            length=(size_t) (zip_info.next_out-compress_pixels);
            if (length != 0)
              {
                (void) WriteBlobMSBLong(image,(unsigned int) length);
                (void) WriteBlob(image,length,compress_pixels);
              }
          } while (zip_info.avail_in != 0);
          if (y == (long) (image->rows-1))
            {
              for ( ; ; )
              {
                zip_info.next_out=compress_pixels;
                zip_info.avail_out=(uInt) ZipMaxExtent(packet_size*
                  image->columns);
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
              status=deflateEnd(&zip_info) == 0 ? MagickTrue : MagickFalse;
            }
          break;
        }
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
        case BZipCompression:
        {
          if (y == 0)
            {
              bzip_info.bzalloc=AcquireBZIPMemory;
              bzip_info.bzfree=RelinquishBZIPMemory;
              bzip_info.opaque=(void *) NULL;
              code=BZ2_bzCompressInit(&bzip_info,(int) (image->quality ==
                UndefinedCompressionQuality ? 7 : MagickMin(image->quality/10,
                9)),
                (int) image_info->verbose,0);
              if (code >= 0)
                status=MagickTrue;
            }
          bzip_info.next_in=(char *) pixels;
          bzip_info.avail_in=(unsigned int) (packet_size*image->columns);
          (void) ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
            quantum_type,pixels,&image->exception);
          do
          {
            bzip_info.next_out=(char *) compress_pixels;
            bzip_info.avail_out=(unsigned int) BZipMaxExtent(packet_size*
              image->columns);
            code=BZ2_bzCompress(&bzip_info,BZ_FLUSH);
            if (code >= 0)
              status=MagickTrue;
            length=(size_t) (bzip_info.next_out-(char *) compress_pixels);
            if (length != 0)
              {
                (void) WriteBlobMSBLong(image,(unsigned int) length);
                (void) WriteBlob(image,length,compress_pixels);
              }
          } while (bzip_info.avail_in != 0);
          if (y == (long) (image->rows-1))
            {
              for ( ; ; )
              {
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
              status=BZ2_bzCompressEnd(&bzip_info) == 0 ? MagickTrue :
                MagickFalse;
            }
          break;
        }
#endif
        case RLECompression:
        {
          pixel=(*p);
          index=(IndexPacket) 0;
          if (indexes != (IndexPacket *) NULL)
            index=(*indexes);
          length=255;
          for (x=0; x < (long) image->columns; x++)
          {
            if ((length < 255) && (x < (long) (image->columns-1)) &&
                (IsColorEqual(p,&pixel) != MagickFalse) &&
                ((image->matte == MagickFalse) ||
                 (p->opacity == pixel.opacity)) &&
                ((indexes == (IndexPacket *) NULL) || (index == indexes[x])))
              length++;
            else
              {
                if (x > 0)
                  q=PopRunlengthPacket(image,q,length,pixel,index);
                length=0;
              }
            pixel=(*p);
            if (indexes != (IndexPacket *) NULL)
              index=indexes[x];
            p++;
          }
          q=PopRunlengthPacket(image,q,length,pixel,index);
          (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          break;
        }
        default:
        {
          (void) ExportQuantumPixels(image,(const CacheView *) NULL,
            quantum_info,quantum_type,pixels,&image->exception);
          (void) WriteBlob(image,packet_size*image->columns,pixels);
          break;
        }
      }
      if ((image->previous == (Image *) NULL) &&
          (SetImageProgress(image,SaveImageTag,y,image->rows) == MagickFalse))
        break;
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(status);
}

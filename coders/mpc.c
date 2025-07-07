/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            M   M  PPPP    CCCC                              %
%                            MM MM  P   P  C                                  %
%                            M M M  PPPP   C                                  %
%                            M   M  P      C                                  %
%                            M   M  P       CCCC                              %
%                                                                             %
%                                                                             %
%                 Read/Write Magick Pixel Cache Image Format                  %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2000                                  %
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
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
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
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/version-private.h"

/*
  Define declarations.
*/
#define MagickPixelCacheNonce  "MagickPixelCache"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMPCImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M P C                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMPC() returns MagickTrue if the image format type, identified by the
%  magick string, is an Magick Pixel Cache image.
%
%  The format of the IsMPC method is:
%
%      MagickBooleanType IsMPC(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsMPC(const unsigned char *magick,const size_t length)
{
  if (length < 19)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"id=MagickPixelCache",19) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C A C H E I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadMPCImage() reads an Magick Pixel Cache image file and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadMPCImage method is:
%
%      Image *ReadMPCImage(const ImageInfo *image_info,ExceptionInfo *exception)
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
static Image *ReadMPCImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    cache_filename[MagickPathExtent],
    id[MagickPathExtent],
    keyword[MagickPathExtent],
    *options;

  GeometryInfo
    geometry_info;

  Image
    *image;

  int
    c;

  LinkedListInfo
    *profiles;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  MagickStatusType
    flags;

  size_t
    depth,
    extent,
    length;

  ssize_t
    count,
    i;

  StringInfo
    *nonce;

  unsigned int
    signature;

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
  (void) CopyMagickString(cache_filename,image->filename,MagickPathExtent-6);
  AppendImageFormat("cache",cache_filename);
  c=ReadBlobByte(image);
  if (c == EOF)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  *id='\0';
  (void) memset(keyword,0,sizeof(keyword));
  offset=0;
  do
  {
    /*
      Decode image header;  header terminates one character beyond a ':'.
    */
    SetGeometryInfo(&geometry_info);
    profiles=(LinkedListInfo *) NULL;
    length=MagickPathExtent;
    options=AcquireString((char *) NULL);
    nonce=StringToStringInfo(MagickPixelCacheNonce);
    signature=GetMagickSignature(nonce);
    nonce=DestroyStringInfo(nonce);
    image->depth=8;
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
                comment=(char *) ResizeQuantumMemory(comment,length+
                  MagickPathExtent,sizeof(*comment));
                if (comment == (char *) NULL)
                  break;
                p=comment+strlen(comment);
              }
            *p=(char) c;
          }
          if (comment == (char *) NULL)
            {
              options=DestroyString(options);
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
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
            while (isspace((int) ((unsigned char) c)) != 0)
              c=ReadBlobByte(image);
            if (c == (int) '=')
              {
                /*
                  Get the keyword value.
                */
                c=ReadBlobByte(image);
                while ((c != (int) '{') && (c != (int) '}') && (c != EOF))
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
                      if ((c == (int) '{') || (c == (int) '}'))
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
                  ThrowReaderException(ResourceLimitError,
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
                    image->colors=StringToUnsignedLong(options);
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
                if (LocaleCompare(keyword,"columns") == 0)
                  {
                    image->columns=StringToUnsignedLong(options);
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
                if (LocaleCompare(keyword,"error") == 0)
                  {
                    image->error.mean_error_per_pixel=StringToDouble(options,
                      (char **) NULL);
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
                if (LocaleCompare(keyword,"magick-signature") == 0)
                  {
                    signature=(unsigned int) StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"mattecolor") == 0)
                  {
                    (void) QueryColorCompliance(options,AllCompliance,
                      &image->matte_color,exception);
                    break;
                  }
                if (LocaleCompare(keyword,"maximum-error") == 0)
                  {
                    image->error.normalized_maximum_error=StringToDouble(
                      options,(char **) NULL);
                    break;
                  }
                if (LocaleCompare(keyword,"mean-error") == 0)
                  {
                    image->error.normalized_mean_error=StringToDouble(options,
                      (char **) NULL);
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
                    if ((flags & SigmaValue) != 0)
                      image->chromaticity.red_primary.y=geometry_info.sigma;
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
    if ((LocaleCompare(id,"MagickPixelCache") != 0) ||
        (image->storage_class == UndefinedClass) ||
        (image->compression == UndefinedCompression) ||
        (image->columns == 0) || (image->rows == 0) ||
        (image->number_channels > MaxPixelChannels) ||
        (image->number_meta_channels > (size_t) (MaxPixelChannels-MetaPixelChannels)) ||
        ((image->number_channels+image->number_meta_channels) >= MaxPixelChannels) ||
        (image->depth == 0) || (image->depth > 64))
      {
        if (profiles != (LinkedListInfo *) NULL)
          profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    nonce=StringToStringInfo(MagickPixelCacheNonce);
    if (signature != GetMagickSignature(nonce))
      {
        nonce=DestroyStringInfo(nonce);
        if (profiles != (LinkedListInfo *) NULL)
          profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
        ThrowReaderException(CacheError,"IncompatibleAPI");
      }
    nonce=DestroyStringInfo(nonce);
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
                {
                  if (profiles != (LinkedListInfo *) NULL)
                    profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
                  ThrowReaderException(CorruptImageError,
                    "UnableToReadImageData");
                }
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
          Read image profile blobs.
        */
        ResetLinkedListIterator(profiles);
        name=(const char *) GetNextValueInLinkedList(profiles);
        while (name != (const char *) NULL)
        {
          length=ReadBlobMSBLong(image);
          if ((MagickSizeType) length > GetBlobSize(image))
            break;
          profile=AcquireProfileStringInfo(name,length,exception);
          if (profile == (StringInfo *) NULL)
            {
              count=(ssize_t) SeekBlob(image,(MagickOffsetType) length,SEEK_CUR);
              if (count != (ssize_t) length)
                break;
            }
          else
            {
              count=ReadBlob(image,length,GetStringInfoDatum(profile));
              if (count != (ssize_t) length)
                {
                  profile=DestroyStringInfo(profile);
                  break;
                }
              status=SetImageProfilePrivate(image,profile,exception);
            }
          if (status == MagickFalse)
            break;
          name=(const char *) GetNextValueInLinkedList(profiles);
        }
        profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
      }
    depth=GetImageQuantumDepth(image,MagickFalse);
    if (image->storage_class == PseudoClass)
      {
        const unsigned char
          *p;

        size_t
          packet_size;

        unsigned char
          *colormap;

        /*
          Create image colormap.
        */
        packet_size=(size_t) (3UL*depth/8UL);
        if ((MagickSizeType) (packet_size*image->colors) > GetBlobSize(image))
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        image->colormap=(PixelInfo *) AcquireQuantumMemory(image->colors+1,
          sizeof(*image->colormap));
        if (image->colormap == (PixelInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        if (image->colors != 0)
          {
            /*
              Read image colormap from file.
            */
            colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
              packet_size*sizeof(*colormap));
            if (colormap == (unsigned char *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            count=ReadBlob(image,packet_size*image->colors,colormap);
            if (count != (ssize_t) (packet_size*image->colors))
              {
                colormap=(unsigned char *) RelinquishMagickMemory(colormap);
                ThrowReaderException(CorruptImageError,
                  "InsufficientImageDataInFile");
              }
            p=colormap;
            switch (depth)
            {
              default:
                colormap=(unsigned char *) RelinquishMagickMemory(colormap);
                ThrowReaderException(CorruptImageError,
                  "ImageDepthNotSupported");
              case 8:
              {
                unsigned char
                  pixel;

                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  p=PushCharPixel(p,&pixel);
                  image->colormap[i].red=(MagickRealType)
                    ScaleCharToQuantum(pixel);
                  p=PushCharPixel(p,&pixel);
                  image->colormap[i].green=(MagickRealType)
                    ScaleCharToQuantum(pixel);
                  p=PushCharPixel(p,&pixel);
                  image->colormap[i].blue=(MagickRealType)
                    ScaleCharToQuantum(pixel);
                }
                break;
              }
              case 16:
              {
                unsigned short
                  pixel;

                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  p=PushShortPixel(MSBEndian,p,&pixel);
                  image->colormap[i].red=(MagickRealType)
                    ScaleShortToQuantum(pixel);
                  p=PushShortPixel(MSBEndian,p,&pixel);
                  image->colormap[i].green=(MagickRealType)
                    ScaleShortToQuantum(pixel);
                  p=PushShortPixel(MSBEndian,p,&pixel);
                  image->colormap[i].blue=(MagickRealType)
                    ScaleShortToQuantum(pixel);
                }
                break;
              }
              case 32:
              {
                unsigned int
                  pixel;

                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  p=PushLongPixel(MSBEndian,p,&pixel);
                  image->colormap[i].red=(MagickRealType)
                    ScaleLongToQuantum(pixel);
                  p=PushLongPixel(MSBEndian,p,&pixel);
                  image->colormap[i].green=(MagickRealType)
                    ScaleLongToQuantum(pixel);
                  p=PushLongPixel(MSBEndian,p,&pixel);
                  image->colormap[i].blue=(MagickRealType)
                    ScaleLongToQuantum(pixel);
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
    if ((AcquireMagickResource(WidthResource,image->columns) == MagickFalse) ||
        (AcquireMagickResource(HeightResource,image->rows) == MagickFalse))
      ThrowReaderException(ImageError,"WidthOrHeightExceedsLimit");
    /*
      Attach persistent pixel cache.
    */
    status=PersistPixelCache(image,cache_filename,MagickTrue,&offset,exception);
    if (status == MagickFalse)
      {
        status=SetImageExtent(image,image->columns,image->rows,exception);
        ThrowReaderException(CacheError,"UnableToPersistPixelCache");
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
  } while ((c != EOF) && ((c == 'i') || (c == 'I')));
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M P C I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMPCImage() adds properties for the Cache image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMPCImage method is:
%
%      size_t RegisterMPCImage(void)
%
*/
ModuleExport size_t RegisterMPCImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("MPC","CACHE","Magick Pixel Cache image format");
  entry->flags|=CoderStealthFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("MPC","MPC","Magick Pixel Cache image format");
  entry->decoder=(DecodeImageHandler *) ReadMPCImage;
  entry->encoder=(EncodeImageHandler *) WriteMPCImage;
  entry->magick=(IsImageFormatHandler *) IsMPC;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M P C I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMPCImage() removes format registrations made by the
%  MPC module from the list of supported formats.
%
%  The format of the UnregisterMPCImage method is:
%
%      UnregisterMPCImage(void)
%
*/
ModuleExport void UnregisterMPCImage(void)
{
  (void) UnregisterMagickInfo("CACHE");
  (void) UnregisterMagickInfo("MPC");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M P C I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMPCImage() writes an Magick Pixel Cache image to a file.
%
%  The format of the WriteMPCImage method is:
%
%      MagickBooleanType WriteMPCImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
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
static MagickBooleanType WriteMPCImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    cache_filename[MagickPathExtent];

  const char
    *property,
    *value;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    scene;

  size_t
    depth,
    number_scenes;

  ssize_t
    i;

  /*
    Open persistent cache.
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
  (void) CopyMagickString(cache_filename,image->filename,MagickPathExtent-6);
  AppendImageFormat("cache",cache_filename);
  scene=0;
  offset=0;
  number_scenes=GetImageListLength(image);
  do
  {
    StringInfo
      *nonce;

    /*
      Write cache meta-information.

      SetImageStorageClass() required to sync pixel cache.
    */
    (void) SetImageStorageClass(image,image->storage_class,exception);
    depth=GetImageQuantumDepth(image,MagickTrue);
    if ((image->storage_class == PseudoClass) &&
        (image->colors > (size_t) (GetQuantumRange(image->depth)+1)))
      (void) SetImageStorageClass(image,DirectClass,exception);
    (void) WriteBlobString(image,"id=MagickPixelCache\n");
    nonce=StringToStringInfo(MagickPixelCacheNonce);
    (void) FormatLocaleString(buffer,MagickPathExtent,"magick-signature=%u\n",
      GetMagickSignature(nonce));
    nonce=DestroyStringInfo(nonce);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "class=%s colors=%.20g alpha-trait=%s\n",CommandOptionToMnemonic(
      MagickClassOptions,image->storage_class),(double) image->colors,
      CommandOptionToMnemonic(MagickPixelTraitOptions,(ssize_t)
      image->alpha_trait));
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"number-channels=%.20g "
      "number-meta-channels=%.20g channel-mask=0x%016llx\n",
      (double) image->number_channels,(double) image->number_meta_channels,
      (MagickOffsetType) image->channel_mask);
    (void) WriteBlobString(image,buffer);
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
    (void) FormatLocaleString(buffer,MagickPathExtent,"colorspace=%s\n",
      CommandOptionToMnemonic(MagickColorspaceOptions,image->colorspace));
    (void) WriteBlobString(image,buffer);
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
    if (image->compression != UndefinedCompression)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "compression=%s quality=%.20g\n",CommandOptionToMnemonic(
          MagickCompressOptions,image->compression),(double) image->quality);
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
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "iterations=%.20g delay=%.20g  ticks-per-second=%.20g\n",(double)
            image->iterations,(double) image->delay,(double)
            image->ticks_per_second);
        else
          (void) FormatLocaleString(buffer,MagickPathExtent,"scene=%.20g  "
            "iterations=%.20g delay=%.20g  ticks-per-second=%.20g\n",
            (double) image->scene,(double) image->iterations,(double)
            image->delay,(double) image->ticks_per_second);
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
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "orientation=%s\n",CommandOptionToMnemonic(MagickOrientationOptions,
          image->orientation));
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
    ResetImagePropertyIterator(image);
    property=GetNextImageProperty(image);
    while (property != (const char *) NULL)
    {
      (void) FormatLocaleString(buffer,MagickPathExtent,"%s=",property);
      (void) WriteBlobString(image,buffer);
      value=GetImageProperty(image,property,exception);
      if (value != (const char *) NULL)
        {
          size_t
            length;

          length=strlen(value);
          for (i=0; i < (ssize_t) length; i++)
            if (isspace((int) ((unsigned char) value[i])) != 0)
              break;
          if ((i == (ssize_t) length) && (i != 0))
            (void) WriteBlob(image,length,(const unsigned char *) value);
          else
            {
              (void) WriteBlobByte(image,'{');
              if ((strchr(value,'{') == (char *) NULL) &&
                  (strchr(value,'}') == (char *) NULL))
                (void) WriteBlob(image,length,(const unsigned char *) value);
              else
                for (i=0; i < (ssize_t) length; i++)
                {
                  if ((value[i] == (int) '{') || (value[i] == (int) '}'))
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
          (void) WriteBlobString(image,image->directory);
        (void) WriteBlobByte(image,'\0');
      }
    if (image->profiles != 0)
      {
        const char
          *name;

        const StringInfo
          *profile;

        /*
          Write image profile blobs.
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
          packet_size;

        unsigned char
          *colormap,
          *q;

        /*
          Allocate colormap.
        */
        packet_size=(size_t) (3UL*depth/8UL);
        colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          packet_size*sizeof(*colormap));
        if (colormap == (unsigned char *) NULL)
          return(MagickFalse);
        /*
          Write colormap to file.
        */
        q=colormap;
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          switch (depth)
          {
            default:
            {
              colormap=(unsigned char *) RelinquishMagickMemory(colormap);
              ThrowWriterException(CorruptImageError,"ImageDepthNotSupported");
              break;
            }
            case 32:
            {
              unsigned int
                pixel;

              pixel=ScaleQuantumToLong(ClampToQuantum(image->colormap[i].red));
              q=PopLongPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToLong(ClampToQuantum(
                image->colormap[i].green));
              q=PopLongPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToLong(ClampToQuantum(image->colormap[i].blue));
              q=PopLongPixel(MSBEndian,pixel,q);
              break;
            }
            case 16:
            {
              unsigned short
                pixel;

              pixel=ScaleQuantumToShort(ClampToQuantum(image->colormap[i].red));
              q=PopShortPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToShort(ClampToQuantum(
                image->colormap[i].green));
              q=PopShortPixel(MSBEndian,pixel,q);
              pixel=ScaleQuantumToShort(ClampToQuantum(
                image->colormap[i].blue));
              q=PopShortPixel(MSBEndian,pixel,q);
              break;
            }
            case 8:
            {
              unsigned char
                pixel;

              pixel=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
                image->colormap[i].red));
              q=PopCharPixel(pixel,q);
              pixel=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
                image->colormap[i].green));
              q=PopCharPixel(pixel,q);
              pixel=(unsigned char) ScaleQuantumToChar(ClampToQuantum(
                image->colormap[i].blue));
              q=PopCharPixel(pixel,q);
              break;
            }
          }
        }
        (void) WriteBlob(image,packet_size*image->colors,colormap);
        colormap=(unsigned char *) RelinquishMagickMemory(colormap);
      }
    /*
      Initialize persistent pixel cache.
    */
    status=PersistPixelCache(image,cache_filename,MagickFalse,&offset,
      exception);
    if (status == MagickFalse)
      ThrowWriterException(CacheError,"UnableToPersistPixelCache");
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        status=image->progress_monitor(SaveImagesTag,scene,
          number_scenes,image->client_data);
        if (status == MagickFalse)
          break;
      }
    scene++;
  } while (image_info->adjoin != MagickFalse);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

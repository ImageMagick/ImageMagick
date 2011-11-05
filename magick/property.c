/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            PPPP    RRRR    OOO   PPPP   EEEEE  RRRR   TTTTT  Y   Y          %
%            P   P   R   R  O   O  P   P  E      R   R    T     Y Y           %
%            PPPP    RRRR   O   O  PPPP   EEE    RRRR     T      Y            %
%            P       R R    O   O  P      E      R R      T      Y            %
%            P       R  R    OOO   P      EEEEE  R  R     T      Y            %
%                                                                             %
%                                                                             %
%                         MagickCore Property Methods                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 March 2000                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/attribute.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/compare.h"
#include "magick/constitute.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fx.h"
#include "magick/fx-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/histogram.h"
#include "magick/image.h"
#include "magick/image.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/montage.h"
#include "magick/option.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum.h"
#include "magick/resource_.h"
#include "magick/splay-tree.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/version.h"
#include "magick/xml-tree.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e P r o p e r t i e s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageProperties() clones one or more image properties.
%
%  The format of the CloneImageProperties method is:
%
%      MagickBooleanType CloneImageProperties(Image *image,
%        const Image *clone_image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o clone_image: the clone image.
%
*/
MagickExport MagickBooleanType CloneImageProperties(Image *image,
  const Image *clone_image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(clone_image != (const Image *) NULL);
  assert(clone_image->signature == MagickSignature);
  if (clone_image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      clone_image->filename);
  (void) CopyMagickString(image->filename,clone_image->filename,MaxTextExtent);
  (void) CopyMagickString(image->magick_filename,clone_image->magick_filename,
    MaxTextExtent);
  image->compression=clone_image->compression;
  image->quality=clone_image->quality;
  image->depth=clone_image->depth;
  image->background_color=clone_image->background_color;
  image->border_color=clone_image->border_color;
  image->matte_color=clone_image->matte_color;
  image->transparent_color=clone_image->transparent_color;
  image->gamma=clone_image->gamma;
  image->chromaticity=clone_image->chromaticity;
  image->rendering_intent=clone_image->rendering_intent;
  image->black_point_compensation=clone_image->black_point_compensation;
  image->units=clone_image->units;
  image->montage=(char *) NULL;
  image->directory=(char *) NULL;
  (void) CloneString(&image->geometry,clone_image->geometry);
  image->offset=clone_image->offset;
  image->x_resolution=clone_image->x_resolution;
  image->y_resolution=clone_image->y_resolution;
  image->page=clone_image->page;
  image->tile_offset=clone_image->tile_offset;
  image->extract_info=clone_image->extract_info;
  image->bias=clone_image->bias;
  image->filter=clone_image->filter;
  image->blur=clone_image->blur;
  image->fuzz=clone_image->fuzz;
  image->interlace=clone_image->interlace;
  image->interpolate=clone_image->interpolate;
  image->endian=clone_image->endian;
  image->gravity=clone_image->gravity;
  image->compose=clone_image->compose;
  image->scene=clone_image->scene;
  image->orientation=clone_image->orientation;
  image->dispose=clone_image->dispose;
  image->delay=clone_image->delay;
  image->ticks_per_second=clone_image->ticks_per_second;
  image->iterations=clone_image->iterations;
  image->total_colors=clone_image->total_colors;
  image->taint=clone_image->taint;
  image->progress_monitor=clone_image->progress_monitor;
  image->client_data=clone_image->client_data;
  image->start_loop=clone_image->start_loop;
  image->error=clone_image->error;
  image->signature=clone_image->signature;
  if (clone_image->properties != (void *) NULL)
    {
      if (image->properties != (void *) NULL)
        DestroyImageProperties(image);
      image->properties=CloneSplayTree((SplayTreeInfo *)
        clone_image->properties,(void *(*)(void *)) ConstantString,
        (void *(*)(void *)) ConstantString);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e f i n e I m a g e P r o p e r t y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefineImageProperty() associates a key/value pair with an image property.
%
%  The format of the DefineImageProperty method is:
%
%      MagickBooleanType DefineImageProperty(Image *image,
%        const char *property)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o property: the image property.
%
*/
MagickExport MagickBooleanType DefineImageProperty(Image *image,
  const char *property)
{
  char
    key[MaxTextExtent],
    value[MaxTextExtent];

  register char
    *p;

  assert(image != (Image *) NULL);
  assert(property != (const char *) NULL);
  (void) CopyMagickString(key,property,MaxTextExtent-1);
  for (p=key; *p != '\0'; p++)
    if (*p == '=')
      break;
  *value='\0';
  if (*p == '=')
    (void) CopyMagickString(value,p+1,MaxTextExtent);
  *p='\0';
  return(SetImageProperty(image,key,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e P r o p e r t y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageProperty() deletes an image property.
%
%  The format of the DeleteImageProperty method is:
%
%      MagickBooleanType DeleteImageProperty(Image *image,const char *property)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o property: the image property.
%
*/
MagickExport MagickBooleanType DeleteImageProperty(Image *image,
  const char *property)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image->filename);
  if (image->properties == (void *) NULL)
    return(MagickFalse);
  return(DeleteNodeFromSplayTree((SplayTreeInfo *) image->properties,property));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e P r o p e r t i e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageProperties() releases memory associated with image property
%  values.
%
%  The format of the DestroyDefines method is:
%
%      void DestroyImageProperties(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DestroyImageProperties(Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image->filename);
  if (image->properties != (void *) NULL)
    image->properties=(void *) DestroySplayTree((SplayTreeInfo *)
      image->properties);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t I m a g e P r o p e r t y                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatImageProperty() permits formatted property/value pairs to be saved as
%  an image property.
%
%  The format of the FormatImageProperty method is:
%
%      MagickBooleanType FormatImageProperty(Image *image,const char *property,
%        const char *format,...)
%
%  A description of each parameter follows.
%
%   o  image:  The image.
%
%   o  property:  The attribute property.
%
%   o  format:  A string describing the format to use to write the remaining
%      arguments.
%
*/
MagickExport MagickBooleanType FormatImageProperty(Image *image,
  const char *property,const char *format,...)
{
  char
    value[MaxTextExtent];

  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=FormatLocaleStringList(value,MaxTextExtent,format,operands);
  (void) n;
  va_end(operands);
  return(SetImageProperty(image,property,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e P r o p e r t y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageProperty() gets a value associated with an image property.
%
%  The format of the GetImageProperty method is:
%
%      const char *GetImageProperty(const Image *image,const char *key)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o key: the key.
%
*/

static char
  *TracePSClippath(const unsigned char *,size_t,const size_t,
    const size_t),
  *TraceSVGClippath(const unsigned char *,size_t,const size_t,
    const size_t);

static MagickBooleanType GetIPTCProperty(const Image *image,const char *key)
{
  char
    *attribute,
    *message;

  const StringInfo
    *profile;

  long
    count,
    dataset,
    record;

  register ssize_t
    i;

  size_t
    length;

  profile=GetImageProfile(image,"iptc");
  if (profile == (StringInfo *) NULL)
    profile=GetImageProfile(image,"8bim");
  if (profile == (StringInfo *) NULL)
    return(MagickFalse);
  count=sscanf(key,"IPTC:%ld:%ld",&dataset,&record);
  if (count != 2)
    return(MagickFalse);
  attribute=(char *) NULL;
  for (i=0; i < (ssize_t) GetStringInfoLength(profile); i+=(ssize_t) length)
  {
    length=1;
    if ((ssize_t) GetStringInfoDatum(profile)[i] != 0x1c)
      continue;
    length=(size_t) (GetStringInfoDatum(profile)[i+3] << 8);
    length|=GetStringInfoDatum(profile)[i+4];
    if (((long) GetStringInfoDatum(profile)[i+1] == dataset) &&
        ((long) GetStringInfoDatum(profile)[i+2] == record))
      {
        message=(char *) NULL;
        if (~length >= 1)
          message=(char *) AcquireQuantumMemory(length+1UL,sizeof(*message));
        if (message != (char *) NULL)
          {
            (void) CopyMagickString(message,(char *) GetStringInfoDatum(
              profile)+i+5,length+1);
            (void) ConcatenateString(&attribute,message);
            (void) ConcatenateString(&attribute,";");
            message=DestroyString(message);
          }
      }
    i+=5;
  }
  if ((attribute == (char *) NULL) || (*attribute == ';'))
    {
      if (attribute != (char *) NULL)
        attribute=DestroyString(attribute);
      return(MagickFalse);
    }
  attribute[strlen(attribute)-1]='\0';
  (void) SetImageProperty((Image *) image,key,(const char *) attribute);
  attribute=DestroyString(attribute);
  return(MagickTrue);
}

static inline ssize_t MagickMax(const ssize_t x,const ssize_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline int ReadPropertyByte(const unsigned char **p,size_t *length)
{
  int
    c;

  if (*length < 1)
    return(EOF);
  c=(int) (*(*p)++);
  (*length)--;
  return(c);
}

static inline size_t ReadPropertyMSBLong(const unsigned char **p,
  size_t *length)
{
  int
    c;

  register ssize_t
    i;

  unsigned char
    buffer[4];

  size_t
    value;

  if (*length < 4)
    return(~0UL);
  for (i=0; i < 4; i++)
  {
    c=(int) (*(*p)++);
    (*length)--;
    buffer[i]=(unsigned char) c;
  }
  value=(size_t) (buffer[0] << 24);
  value|=buffer[1] << 16;
  value|=buffer[2] << 8;
  value|=buffer[3];
  return(value & 0xffffffff);
}

static inline unsigned short ReadPropertyMSBShort(const unsigned char **p,
  size_t *length)
{
  int
    c;

  register ssize_t
    i;

  unsigned char
    buffer[2];

  unsigned short
    value;

  if (*length < 2)
    return((unsigned short) ~0U);
  for (i=0; i < 2; i++)
  {
    c=(int) (*(*p)++);
    (*length)--;
    buffer[i]=(unsigned char) c;
  }
  value=(unsigned short) (buffer[0] << 8);
  value|=buffer[1];
  return((unsigned short) (value & 0xffff));
}

static MagickBooleanType Get8BIMProperty(const Image *image,const char *key)
{
  char
    *attribute,
    format[MaxTextExtent],
    name[MaxTextExtent],
    *resource;

  const StringInfo
    *profile;

  const unsigned char
    *info;

  long
    start,
    stop;

  MagickBooleanType
    status;

  register ssize_t
    i;

  ssize_t
    count,
    id,
    sub_number;

  size_t
    length;

  /*
    There are no newlines in path names, so it's safe as terminator.
  */
  profile=GetImageProfile(image,"8bim");
  if (profile == (StringInfo *) NULL)
    return(MagickFalse);
  count=(ssize_t) sscanf(key,"8BIM:%ld,%ld:%[^\n]\n%[^\n]",&start,&stop,name,
    format);
  if ((count != 2) && (count != 3) && (count != 4))
    return(MagickFalse);
  if (count < 4)
    (void) CopyMagickString(format,"SVG",MaxTextExtent);
  if (count < 3)
    *name='\0';
  sub_number=1;
  if (*name == '#')
    sub_number=(ssize_t) StringToLong(&name[1]);
  sub_number=MagickMax(sub_number,1L);
  resource=(char *) NULL;
  status=MagickFalse;
  length=GetStringInfoLength(profile);
  info=GetStringInfoDatum(profile);
  while ((length > 0) && (status == MagickFalse))
  {
    if (ReadPropertyByte(&info,&length) != (unsigned char) '8')
      continue;
    if (ReadPropertyByte(&info,&length) != (unsigned char) 'B')
      continue;
    if (ReadPropertyByte(&info,&length) != (unsigned char) 'I')
      continue;
    if (ReadPropertyByte(&info,&length) != (unsigned char) 'M')
      continue;
    id=(ssize_t) ReadPropertyMSBShort(&info,&length);
    if (id < (ssize_t) start)
      continue;
    if (id > (ssize_t) stop)
      continue;
    if (resource != (char *) NULL)
      resource=DestroyString(resource);
    count=(ssize_t) ReadPropertyByte(&info,&length);
    if ((count != 0) && ((size_t) count <= length))
      {
        resource=(char *) NULL;
        if (~(1UL*count) >= (MaxTextExtent-1))
          resource=(char *) AcquireQuantumMemory((size_t) count+MaxTextExtent,
            sizeof(*resource));
        if (resource != (char *) NULL)
          {
            for (i=0; i < (ssize_t) count; i++)
              resource[i]=(char) ReadPropertyByte(&info,&length);
            resource[count]='\0';
          }
      }
    if ((count & 0x01) == 0)
      (void) ReadPropertyByte(&info,&length);
    count=(ssize_t) ((int) ReadPropertyMSBLong(&info,&length));
    if ((*name != '\0') && (*name != '#'))
      if ((resource == (char *) NULL) || (LocaleCompare(name,resource) != 0))
        {
          /*
            No name match, scroll forward and try next.
          */
          info+=count;
          length-=count;
          continue;
        }
    if ((*name == '#') && (sub_number != 1))
      {
        /*
          No numbered match, scroll forward and try next.
        */
        sub_number--;
        info+=count;
        length-=count;
        continue;
      }
    /*
      We have the resource of interest.
    */
    attribute=(char *) NULL;
    if (~(1UL*count) >= (MaxTextExtent-1))
      attribute=(char *) AcquireQuantumMemory((size_t) count+MaxTextExtent,
        sizeof(*attribute));
    if (attribute != (char *) NULL)
      {
        (void) CopyMagickMemory(attribute,(char *) info,(size_t) count);
        attribute[count]='\0';
        info+=count;
        length-=count;
        if ((id <= 1999) || (id >= 2999))
          (void) SetImageProperty((Image *) image,key,(const char *)
            attribute);
        else
          {
            char
              *path;

            if (LocaleCompare(format,"svg") == 0)
              path=TraceSVGClippath((unsigned char *) attribute,(size_t) count,
                image->columns,image->rows);
            else
              path=TracePSClippath((unsigned char *) attribute,(size_t) count,
                image->columns,image->rows);
            (void) SetImageProperty((Image *) image,key,(const char *) path);
            path=DestroyString(path);
          }
        attribute=DestroyString(attribute);
        status=MagickTrue;
      }
  }
  if (resource != (char *) NULL)
    resource=DestroyString(resource);
  return(status);
}

static inline unsigned short ReadPropertyShort(const EndianType endian,
  const unsigned char *buffer)
{
  unsigned short
    value;

  if (endian == MSBEndian)
    {
      value=(unsigned short) ((((unsigned char *) buffer)[0] << 8) |
        ((unsigned char *) buffer)[1]);
      return((unsigned short) (value & 0xffff));
    }
  value=(unsigned short) ((buffer[1] << 8) | buffer[0]);
  return((unsigned short) (value & 0xffff));
}

static inline size_t ReadPropertyLong(const EndianType endian,
  const unsigned char *buffer)
{
  size_t
    value;

  if (endian == MSBEndian)
    {
      value=(size_t) ((buffer[0] << 24) | (buffer[1] << 16) |
        (buffer[2] << 8) | buffer[3]);
      return((size_t) (value & 0xffffffff));
    }
  value=(size_t) ((buffer[3] << 24) | (buffer[2] << 16) |
    (buffer[1] << 8 ) | (buffer[0]));
  return((size_t) (value & 0xffffffff));
}

static MagickBooleanType GetEXIFProperty(const Image *image,
  const char *property)
{
#define MaxDirectoryStack  16
#define EXIF_DELIMITER  "\n"
#define EXIF_NUM_FORMATS  12
#define EXIF_FMT_BYTE  1
#define EXIF_FMT_STRING  2
#define EXIF_FMT_USHORT  3
#define EXIF_FMT_ULONG  4
#define EXIF_FMT_URATIONAL  5
#define EXIF_FMT_SBYTE  6
#define EXIF_FMT_UNDEFINED  7
#define EXIF_FMT_SSHORT  8
#define EXIF_FMT_SLONG  9
#define EXIF_FMT_SRATIONAL  10
#define EXIF_FMT_SINGLE  11
#define EXIF_FMT_DOUBLE  12
#define TAG_EXIF_OFFSET  0x8769
#define TAG_GPS_OFFSET  0x8825
#define TAG_INTEROP_OFFSET  0xa005

#define EXIFMultipleValues(size, format, arg) \
{ \
   ssize_t \
     component; \
 \
   size_t \
     length; \
 \
   unsigned char \
     *p1; \
 \
   length=0; \
   p1=p; \
   for (component=0; component < components; component++) \
   { \
     length+=FormatLocaleString(buffer+length,MaxTextExtent-length, \
       format", ",arg); \
     if (length >= (MaxTextExtent-1)) \
       length=MaxTextExtent-1; \
     p1+=size; \
   } \
   if (length > 1) \
     buffer[length-2]='\0'; \
   value=AcquireString(buffer); \
}

#define EXIFMultipleFractions(size, format, arg1, arg2) \
{ \
   ssize_t \
     component; \
 \
   size_t \
     length; \
 \
   unsigned char \
     *p1; \
 \
   length=0; \
   p1=p; \
   for (component=0; component < components; component++) \
   { \
     length+=FormatLocaleString(buffer+length,MaxTextExtent-length, \
       format", ",arg1, arg2); \
     if (length >= (MaxTextExtent-1)) \
       length=MaxTextExtent-1; \
     p1+=size; \
   } \
   if (length > 1) \
     buffer[length-2]='\0'; \
   value=AcquireString(buffer); \
}

  typedef struct _DirectoryInfo
  {
    const unsigned char
      *directory;

    size_t
      entry,
      offset;
  } DirectoryInfo;

  typedef struct _TagInfo
  {
    size_t
      tag;

    const char
      *description;
  } TagInfo;

  static TagInfo
    EXIFTag[] =
    {
      {  0x001, "exif:InteroperabilityIndex" },
      {  0x002, "exif:InteroperabilityVersion" },
      {  0x100, "exif:ImageWidth" },
      {  0x101, "exif:ImageLength" },
      {  0x102, "exif:BitsPerSample" },
      {  0x103, "exif:Compression" },
      {  0x106, "exif:PhotometricInterpretation" },
      {  0x10a, "exif:FillOrder" },
      {  0x10d, "exif:DocumentName" },
      {  0x10e, "exif:ImageDescription" },
      {  0x10f, "exif:Make" },
      {  0x110, "exif:Model" },
      {  0x111, "exif:StripOffsets" },
      {  0x112, "exif:Orientation" },
      {  0x115, "exif:SamplesPerPixel" },
      {  0x116, "exif:RowsPerStrip" },
      {  0x117, "exif:StripByteCounts" },
      {  0x11a, "exif:XResolution" },
      {  0x11b, "exif:YResolution" },
      {  0x11c, "exif:PlanarConfiguration" },
      {  0x11d, "exif:PageName" },
      {  0x11e, "exif:XPosition" },
      {  0x11f, "exif:YPosition" },
      {  0x118, "exif:MinSampleValue" },
      {  0x119, "exif:MaxSampleValue" },
      {  0x120, "exif:FreeOffsets" },
      {  0x121, "exif:FreeByteCounts" },
      {  0x122, "exif:GrayResponseUnit" },
      {  0x123, "exif:GrayResponseCurve" },
      {  0x124, "exif:T4Options" },
      {  0x125, "exif:T6Options" },
      {  0x128, "exif:ResolutionUnit" },
      {  0x12d, "exif:TransferFunction" },
      {  0x131, "exif:Software" },
      {  0x132, "exif:DateTime" },
      {  0x13b, "exif:Artist" },
      {  0x13e, "exif:WhitePoint" },
      {  0x13f, "exif:PrimaryChromaticities" },
      {  0x140, "exif:ColorMap" },
      {  0x141, "exif:HalfToneHints" },
      {  0x142, "exif:TileWidth" },
      {  0x143, "exif:TileLength" },
      {  0x144, "exif:TileOffsets" },
      {  0x145, "exif:TileByteCounts" },
      {  0x14a, "exif:SubIFD" },
      {  0x14c, "exif:InkSet" },
      {  0x14d, "exif:InkNames" },
      {  0x14e, "exif:NumberOfInks" },
      {  0x150, "exif:DotRange" },
      {  0x151, "exif:TargetPrinter" },
      {  0x152, "exif:ExtraSample" },
      {  0x153, "exif:SampleFormat" },
      {  0x154, "exif:SMinSampleValue" },
      {  0x155, "exif:SMaxSampleValue" },
      {  0x156, "exif:TransferRange" },
      {  0x157, "exif:ClipPath" },
      {  0x158, "exif:XClipPathUnits" },
      {  0x159, "exif:YClipPathUnits" },
      {  0x15a, "exif:Indexed" },
      {  0x15b, "exif:JPEGTables" },
      {  0x15f, "exif:OPIProxy" },
      {  0x200, "exif:JPEGProc" },
      {  0x201, "exif:JPEGInterchangeFormat" },
      {  0x202, "exif:JPEGInterchangeFormatLength" },
      {  0x203, "exif:JPEGRestartInterval" },
      {  0x205, "exif:JPEGLosslessPredictors" },
      {  0x206, "exif:JPEGPointTransforms" },
      {  0x207, "exif:JPEGQTables" },
      {  0x208, "exif:JPEGDCTables" },
      {  0x209, "exif:JPEGACTables" },
      {  0x211, "exif:YCbCrCoefficients" },
      {  0x212, "exif:YCbCrSubSampling" },
      {  0x213, "exif:YCbCrPositioning" },
      {  0x214, "exif:ReferenceBlackWhite" },
      {  0x2bc, "exif:ExtensibleMetadataPlatform" },
      {  0x301, "exif:Gamma" },
      {  0x302, "exif:ICCProfileDescriptor" },
      {  0x303, "exif:SRGBRenderingIntent" },
      {  0x320, "exif:ImageTitle" },
      {  0x5001, "exif:ResolutionXUnit" },
      {  0x5002, "exif:ResolutionYUnit" },
      {  0x5003, "exif:ResolutionXLengthUnit" },
      {  0x5004, "exif:ResolutionYLengthUnit" },
      {  0x5005, "exif:PrintFlags" },
      {  0x5006, "exif:PrintFlagsVersion" },
      {  0x5007, "exif:PrintFlagsCrop" },
      {  0x5008, "exif:PrintFlagsBleedWidth" },
      {  0x5009, "exif:PrintFlagsBleedWidthScale" },
      {  0x500A, "exif:HalftoneLPI" },
      {  0x500B, "exif:HalftoneLPIUnit" },
      {  0x500C, "exif:HalftoneDegree" },
      {  0x500D, "exif:HalftoneShape" },
      {  0x500E, "exif:HalftoneMisc" },
      {  0x500F, "exif:HalftoneScreen" },
      {  0x5010, "exif:JPEGQuality" },
      {  0x5011, "exif:GridSize" },
      {  0x5012, "exif:ThumbnailFormat" },
      {  0x5013, "exif:ThumbnailWidth" },
      {  0x5014, "exif:ThumbnailHeight" },
      {  0x5015, "exif:ThumbnailColorDepth" },
      {  0x5016, "exif:ThumbnailPlanes" },
      {  0x5017, "exif:ThumbnailRawBytes" },
      {  0x5018, "exif:ThumbnailSize" },
      {  0x5019, "exif:ThumbnailCompressedSize" },
      {  0x501a, "exif:ColorTransferFunction" },
      {  0x501b, "exif:ThumbnailData" },
      {  0x5020, "exif:ThumbnailImageWidth" },
      {  0x5021, "exif:ThumbnailImageHeight" },
      {  0x5022, "exif:ThumbnailBitsPerSample" },
      {  0x5023, "exif:ThumbnailCompression" },
      {  0x5024, "exif:ThumbnailPhotometricInterp" },
      {  0x5025, "exif:ThumbnailImageDescription" },
      {  0x5026, "exif:ThumbnailEquipMake" },
      {  0x5027, "exif:ThumbnailEquipModel" },
      {  0x5028, "exif:ThumbnailStripOffsets" },
      {  0x5029, "exif:ThumbnailOrientation" },
      {  0x502a, "exif:ThumbnailSamplesPerPixel" },
      {  0x502b, "exif:ThumbnailRowsPerStrip" },
      {  0x502c, "exif:ThumbnailStripBytesCount" },
      {  0x502d, "exif:ThumbnailResolutionX" },
      {  0x502e, "exif:ThumbnailResolutionY" },
      {  0x502f, "exif:ThumbnailPlanarConfig" },
      {  0x5030, "exif:ThumbnailResolutionUnit" },
      {  0x5031, "exif:ThumbnailTransferFunction" },
      {  0x5032, "exif:ThumbnailSoftwareUsed" },
      {  0x5033, "exif:ThumbnailDateTime" },
      {  0x5034, "exif:ThumbnailArtist" },
      {  0x5035, "exif:ThumbnailWhitePoint" },
      {  0x5036, "exif:ThumbnailPrimaryChromaticities" },
      {  0x5037, "exif:ThumbnailYCbCrCoefficients" },
      {  0x5038, "exif:ThumbnailYCbCrSubsampling" },
      {  0x5039, "exif:ThumbnailYCbCrPositioning" },
      {  0x503A, "exif:ThumbnailRefBlackWhite" },
      {  0x503B, "exif:ThumbnailCopyRight" },
      {  0x5090, "exif:LuminanceTable" },
      {  0x5091, "exif:ChrominanceTable" },
      {  0x5100, "exif:FrameDelay" },
      {  0x5101, "exif:LoopCount" },
      {  0x5110, "exif:PixelUnit" },
      {  0x5111, "exif:PixelPerUnitX" },
      {  0x5112, "exif:PixelPerUnitY" },
      {  0x5113, "exif:PaletteHistogram" },
      {  0x1000, "exif:RelatedImageFileFormat" },
      {  0x1001, "exif:RelatedImageLength" },
      {  0x1002, "exif:RelatedImageWidth" },
      {  0x800d, "exif:ImageID" },
      {  0x80e3, "exif:Matteing" },
      {  0x80e4, "exif:DataType" },
      {  0x80e5, "exif:ImageDepth" },
      {  0x80e6, "exif:TileDepth" },
      {  0x828d, "exif:CFARepeatPatternDim" },
      {  0x828e, "exif:CFAPattern2" },
      {  0x828f, "exif:BatteryLevel" },
      {  0x8298, "exif:Copyright" },
      {  0x829a, "exif:ExposureTime" },
      {  0x829d, "exif:FNumber" },
      {  0x83bb, "exif:IPTC/NAA" },
      {  0x84e3, "exif:IT8RasterPadding" },
      {  0x84e5, "exif:IT8ColorTable" },
      {  0x8649, "exif:ImageResourceInformation" },
      {  0x8769, "exif:ExifOffset" },
      {  0x8773, "exif:InterColorProfile" },
      {  0x8822, "exif:ExposureProgram" },
      {  0x8824, "exif:SpectralSensitivity" },
      {  0x8825, "exif:GPSInfo" },
      {  0x8827, "exif:ISOSpeedRatings" },
      {  0x8828, "exif:OECF" },
      {  0x8829, "exif:Interlace" },
      {  0x882a, "exif:TimeZoneOffset" },
      {  0x882b, "exif:SelfTimerMode" },
      {  0x9000, "exif:ExifVersion" },
      {  0x9003, "exif:DateTimeOriginal" },
      {  0x9004, "exif:DateTimeDigitized" },
      {  0x9101, "exif:ComponentsConfiguration" },
      {  0x9102, "exif:CompressedBitsPerPixel" },
      {  0x9201, "exif:ShutterSpeedValue" },
      {  0x9202, "exif:ApertureValue" },
      {  0x9203, "exif:BrightnessValue" },
      {  0x9204, "exif:ExposureBiasValue" },
      {  0x9205, "exif:MaxApertureValue" },
      {  0x9206, "exif:SubjectDistance" },
      {  0x9207, "exif:MeteringMode" },
      {  0x9208, "exif:LightSource" },
      {  0x9209, "exif:Flash" },
      {  0x920a, "exif:FocalLength" },
      {  0x920b, "exif:FlashEnergy" },
      {  0x920c, "exif:SpatialFrequencyResponse" },
      {  0x920d, "exif:Noise" },
      {  0x9211, "exif:ImageNumber" },
      {  0x9212, "exif:SecurityClassification" },
      {  0x9213, "exif:ImageHistory" },
      {  0x9214, "exif:SubjectArea" },
      {  0x9215, "exif:ExposureIndex" },
      {  0x9216, "exif:TIFF-EPStandardID" },
      {  0x927c, "exif:MakerNote" },
      {  0x9C9b, "exif:WinXP-Title" },
      {  0x9C9c, "exif:WinXP-Comments" },
      {  0x9C9d, "exif:WinXP-Author" },
      {  0x9C9e, "exif:WinXP-Keywords" },
      {  0x9C9f, "exif:WinXP-Subject" },
      {  0x9286, "exif:UserComment" },
      {  0x9290, "exif:SubSecTime" },
      {  0x9291, "exif:SubSecTimeOriginal" },
      {  0x9292, "exif:SubSecTimeDigitized" },
      {  0xa000, "exif:FlashPixVersion" },
      {  0xa001, "exif:ColorSpace" },
      {  0xa002, "exif:ExifImageWidth" },
      {  0xa003, "exif:ExifImageLength" },
      {  0xa004, "exif:RelatedSoundFile" },
      {  0xa005, "exif:InteroperabilityOffset" },
      {  0xa20b, "exif:FlashEnergy" },
      {  0xa20c, "exif:SpatialFrequencyResponse" },
      {  0xa20d, "exif:Noise" },
      {  0xa20e, "exif:FocalPlaneXResolution" },
      {  0xa20f, "exif:FocalPlaneYResolution" },
      {  0xa210, "exif:FocalPlaneResolutionUnit" },
      {  0xa214, "exif:SubjectLocation" },
      {  0xa215, "exif:ExposureIndex" },
      {  0xa216, "exif:TIFF/EPStandardID" },
      {  0xa217, "exif:SensingMethod" },
      {  0xa300, "exif:FileSource" },
      {  0xa301, "exif:SceneType" },
      {  0xa302, "exif:CFAPattern" },
      {  0xa401, "exif:CustomRendered" },
      {  0xa402, "exif:ExposureMode" },
      {  0xa403, "exif:WhiteBalance" },
      {  0xa404, "exif:DigitalZoomRatio" },
      {  0xa405, "exif:FocalLengthIn35mmFilm" },
      {  0xa406, "exif:SceneCaptureType" },
      {  0xa407, "exif:GainControl" },
      {  0xa408, "exif:Contrast" },
      {  0xa409, "exif:Saturation" },
      {  0xa40a, "exif:Sharpness" },
      {  0xa40b, "exif:DeviceSettingDescription" },
      {  0xa40c, "exif:SubjectDistanceRange" },
      {  0xa420, "exif:ImageUniqueID" },
      {  0xc4a5, "exif:PrintImageMatching" },
      {  0xa500, "exif:Gamma" },
      {  0xc640, "exif:CR2Slice" },
      { 0x10000, "exif:GPSVersionID" },
      { 0x10001, "exif:GPSLatitudeRef" },
      { 0x10002, "exif:GPSLatitude" },
      { 0x10003, "exif:GPSLongitudeRef" },
      { 0x10004, "exif:GPSLongitude" },
      { 0x10005, "exif:GPSAltitudeRef" },
      { 0x10006, "exif:GPSAltitude" },
      { 0x10007, "exif:GPSTimeStamp" },
      { 0x10008, "exif:GPSSatellites" },
      { 0x10009, "exif:GPSStatus" },
      { 0x1000a, "exif:GPSMeasureMode" },
      { 0x1000b, "exif:GPSDop" },
      { 0x1000c, "exif:GPSSpeedRef" },
      { 0x1000d, "exif:GPSSpeed" },
      { 0x1000e, "exif:GPSTrackRef" },
      { 0x1000f, "exif:GPSTrack" },
      { 0x10010, "exif:GPSImgDirectionRef" },
      { 0x10011, "exif:GPSImgDirection" },
      { 0x10012, "exif:GPSMapDatum" },
      { 0x10013, "exif:GPSDestLatitudeRef" },
      { 0x10014, "exif:GPSDestLatitude" },
      { 0x10015, "exif:GPSDestLongitudeRef" },
      { 0x10016, "exif:GPSDestLongitude" },
      { 0x10017, "exif:GPSDestBearingRef" },
      { 0x10018, "exif:GPSDestBearing" },
      { 0x10019, "exif:GPSDestDistanceRef" },
      { 0x1001a, "exif:GPSDestDistance" },
      { 0x1001b, "exif:GPSProcessingMethod" },
      { 0x1001c, "exif:GPSAreaInformation" },
      { 0x1001d, "exif:GPSDateStamp" },
      { 0x1001e, "exif:GPSDifferential" },
      {  0x0000, NULL}
    };

  const StringInfo
    *profile;

  const unsigned char
    *directory,
    *exif;

  DirectoryInfo
    directory_stack[MaxDirectoryStack];

  EndianType
    endian;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    entry,
    length,
    number_entries,
    tag_offset,
    tag;

  ssize_t
    all,
    id,
    level,
    offset,
    tag_value;

  static int
    tag_bytes[] = {0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

  /*
    If EXIF data exists, then try to parse the request for a tag.
  */
  profile=GetImageProfile(image,"exif");
  if (profile == (StringInfo *) NULL)
    return(MagickFalse);
  if ((property == (const char *) NULL) || (*property == '\0'))
    return(MagickFalse);
  while (isspace((int) ((unsigned char) *property)) != 0)
    property++;
  all=0;
  tag=(~0UL);
  switch (*(property+5))
  {
    case '*':
    {
      /*
        Caller has asked for all the tags in the EXIF data.
      */
      tag=0;
      all=1; /* return the data in description=value format */
      break;
    }
    case '!':
    {
      tag=0;
      all=2; /* return the data in tagid=value format */
      break;
    }
    case '#':
    case '@':
    {
      int
        c;

      size_t
        n;

      /*
        Check for a hex based tag specification first.
      */
      tag=(*(property+5) == '@') ? 1UL : 0UL;
      property+=6;
      n=strlen(property);
      if (n != 4)
        return(MagickFalse);
      /*
        Parse tag specification as a hex number.
      */
      n/=4;
      do
      {
        for (i=(ssize_t) n-1L; i >= 0; i--)
        {
          c=(*property++);
          tag<<=4;
          if ((c >= '0') && (c <= '9'))
            tag|=(c-'0');
          else
            if ((c >= 'A') && (c <= 'F'))
              tag|=(c-('A'-10));
            else
              if ((c >= 'a') && (c <= 'f'))
                tag|=(c-('a'-10));
              else
                return(MagickFalse);
        }
      } while (*property != '\0');
      break;
    }
    default:
    {
      /*
        Try to match the text with a tag name instead.
      */
      for (i=0; ; i++)
      {
        if (EXIFTag[i].tag == 0)
          break;
        if (LocaleCompare(EXIFTag[i].description,property) == 0)
          {
            tag=(size_t) EXIFTag[i].tag;
            break;
          }
      }
      break;
    }
  }
  if (tag == (~0UL))
    return(MagickFalse);
  length=GetStringInfoLength(profile);
  exif=GetStringInfoDatum(profile);
  while (length != 0)
  {
    if (ReadPropertyByte(&exif,&length) != 0x45)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x78)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x69)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x66)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x00)
      continue;
    if (ReadPropertyByte(&exif,&length) != 0x00)
      continue;
    break;
  }
  if (length < 16)
    return(MagickFalse);
  id=(ssize_t) ReadPropertyShort(LSBEndian,exif);
  endian=LSBEndian;
  if (id == 0x4949)
    endian=LSBEndian;
  else
    if (id == 0x4D4D)
      endian=MSBEndian;
    else
      return(MagickFalse);
  if (ReadPropertyShort(endian,exif+2) != 0x002a)
    return(MagickFalse);
  /*
    This the offset to the first IFD.
  */
  offset=(ssize_t) ((int) ReadPropertyLong(endian,exif+4));
  if ((size_t) offset >= length)
    return(MagickFalse);
  /*
    Set the pointer to the first IFD and follow it were it leads.
  */
  status=MagickFalse;
  directory=exif+offset;
  level=0;
  entry=0;
  tag_offset=0;
  do
  {
    /*
      If there is anything on the stack then pop it off.
    */
    if (level > 0)
      {
        level--;
        directory=directory_stack[level].directory;
        entry=directory_stack[level].entry;
        tag_offset=directory_stack[level].offset;
      }
    /*
      Determine how many entries there are in the current IFD.
    */
    number_entries=ReadPropertyShort(endian,directory);
    for ( ; entry < number_entries; entry++)
    {
      register unsigned char
        *p,
        *q;

      size_t
        format,
        number_bytes;

      ssize_t
        components;

      q=(unsigned char *) (directory+2+(12*entry));
      tag_value=(ssize_t) (ReadPropertyShort(endian,q)+tag_offset);
      format=(size_t) ReadPropertyShort(endian,q+2);
      if (format >= (sizeof(tag_bytes)/sizeof(*tag_bytes)))
        break;
      components=(ssize_t) ((int) ReadPropertyLong(endian,q+4));
      number_bytes=(size_t) components*tag_bytes[format];
      if (number_bytes <= 4)
        p=q+8;
      else
        {
          ssize_t
            offset;

          /*
            The directory entry contains an offset.
          */
          offset=(ssize_t) ((int) ReadPropertyLong(endian,q+8));
          if ((size_t) (offset+number_bytes) > length)
            continue;
          p=(unsigned char *) (exif+offset);
        }
      if ((all != 0) || (tag == (size_t) tag_value))
        {
          char
            buffer[MaxTextExtent],
            *value;

          switch (format)
          {
            case EXIF_FMT_BYTE:
            case EXIF_FMT_UNDEFINED:
            {
              EXIFMultipleValues(1,"%.20g",(double) (*(unsigned char *) p1));
              break;
            }
            case EXIF_FMT_SBYTE:
            {
              EXIFMultipleValues(1,"%.20g",(double) (*(signed char *) p1));
              break;
            }
            case EXIF_FMT_SSHORT:
            {
              EXIFMultipleValues(2,"%hd",ReadPropertyShort(endian,p1));
              break;
            }
            case EXIF_FMT_USHORT:
            {
              EXIFMultipleValues(2,"%hu",ReadPropertyShort(endian,p1));
              break;
            }
            case EXIF_FMT_ULONG:
            {
              EXIFMultipleValues(4,"%.20g",(double)
                ReadPropertyLong(endian,p1));
              break;
            }
            case EXIF_FMT_SLONG:
            {
              EXIFMultipleValues(4,"%.20g",(double)
                ReadPropertyLong(endian,p1));
              break;
            }
            case EXIF_FMT_URATIONAL:
            {
              EXIFMultipleFractions(8,"%.20g/%.20g",(double)
                ReadPropertyLong(endian,p1),(double)
                ReadPropertyLong(endian,p1+4));
              break;
            }
            case EXIF_FMT_SRATIONAL:
            {
              EXIFMultipleFractions(8,"%.20g/%.20g",(double)
                ReadPropertyLong(endian,p1),(double)
                ReadPropertyLong(endian,p1+4));
              break;
            }
            case EXIF_FMT_SINGLE:
            {
              EXIFMultipleValues(4,"%f",(double) *(float *) p1);
              break;
            }
            case EXIF_FMT_DOUBLE:
            {
              EXIFMultipleValues(8,"%f",*(double *) p1);
              break;
            }
            default:
            case EXIF_FMT_STRING:
            {
              value=(char *) NULL;
              if (~(1UL*number_bytes) >= 1)
                value=(char *) AcquireQuantumMemory((size_t) number_bytes+1UL,
                  sizeof(*value));
              if (value != (char *) NULL)
                {
                  register ssize_t
                    i;

                  for (i=0; i < (ssize_t) number_bytes; i++)
                  {
                    value[i]='.';
                    if ((isprint((int) p[i]) != 0) || (p[i] == '\0'))
                      value[i]=(char) p[i];
                  }
                  value[i]='\0';
                }
              break;
            }
          }
          if (value != (char *) NULL)
            {
              char
                key[MaxTextExtent];

              register const char
                *p;

              (void) CopyMagickString(key,property,MaxTextExtent);
              switch (all)
              {
                case 1:
                {
                  const char
                    *description;

                  register ssize_t
                    i;

                  description="unknown";
                  for (i=0; ; i++)
                  {
                    if (EXIFTag[i].tag == 0)
                      break;
                    if ((ssize_t) EXIFTag[i].tag == tag_value)
                      {
                        description=EXIFTag[i].description;
                        break;
                      }
                  }
                  (void) FormatLocaleString(key,MaxTextExtent,"%s",
                    description);
                  break;
                }
                case 2:
                {
                  if (tag_value < 0x10000)
                    (void) FormatLocaleString(key,MaxTextExtent,"#%04lx",
                      (unsigned long) tag_value);
                  else
                    if (tag_value < 0x20000)
                      (void) FormatLocaleString(key,MaxTextExtent,"@%04lx",
                        (unsigned long) (tag_value & 0xffff));
                    else
                      (void) FormatLocaleString(key,MaxTextExtent,"unknown");
                  break;
                }
              }
              p=(const char *) NULL;
              if (image->properties != (void *) NULL)
                p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
                  image->properties,key);
              if (p == (const char *) NULL)
                (void) SetImageProperty((Image *) image,key,value);
              value=DestroyString(value);
              status=MagickTrue;
            }
        }
        if ((tag_value == TAG_EXIF_OFFSET) ||
            (tag_value == TAG_INTEROP_OFFSET) ||
            (tag_value == TAG_GPS_OFFSET))
          {
            size_t
              offset;

            offset=(size_t) ReadPropertyLong(endian,p);
            if ((offset < length) && (level < (MaxDirectoryStack-2)))
              {
                size_t
                  tag_offset1;

                tag_offset1=(tag_value == TAG_GPS_OFFSET) ? 0x10000UL : 0UL;
                directory_stack[level].directory=directory;
                entry++;
                directory_stack[level].entry=entry;
                directory_stack[level].offset=tag_offset;
                level++;
                directory_stack[level].directory=exif+offset;
                directory_stack[level].offset=tag_offset1;
                directory_stack[level].entry=0;
                level++;
                if ((directory+2+(12*number_entries)) > (exif+length))
                  break;
                offset=(size_t) ReadPropertyLong(endian,directory+2+(12*
                  number_entries));
                if ((offset != 0) && (offset < length) &&
                    (level < (MaxDirectoryStack-2)))
                  {
                    directory_stack[level].directory=exif+offset;
                    directory_stack[level].entry=0;
                    directory_stack[level].offset=tag_offset1;
                    level++;
                  }
              }
            break;
          }
    }
  } while (level > 0);
  return(status);
}

static MagickBooleanType GetXMPProperty(const Image *image,const char *property)
{
  char
    *xmp_profile;

  const StringInfo
    *profile;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  register const char
    *p;

  XMLTreeInfo
    *child,
    *description,
    *node,
    *rdf,
    *xmp;

  profile=GetImageProfile(image,"xmp");
  if (profile == (StringInfo *) NULL)
    return(MagickFalse);
  if ((property == (const char *) NULL) || (*property == '\0'))
    return(MagickFalse);
  xmp_profile=StringInfoToString(profile);
  if (xmp_profile == (char *) NULL)
    return(MagickFalse);
  for (p=xmp_profile; *p != '\0'; p++)
    if ((*p == '<') && (*(p+1) == 'x'))
      break;
  exception=AcquireExceptionInfo();
  xmp=NewXMLTree((char *) p,exception);
  xmp_profile=DestroyString(xmp_profile);
  exception=DestroyExceptionInfo(exception);
  if (xmp == (XMLTreeInfo *) NULL)
    return(MagickFalse);
  status=MagickFalse;
  rdf=GetXMLTreeChild(xmp,"rdf:RDF");
  if (rdf != (XMLTreeInfo *) NULL)
    {
      if (image->properties == (void *) NULL)
        ((Image *) image)->properties=NewSplayTree(CompareSplayTreeString,
          RelinquishMagickMemory,RelinquishMagickMemory);
      description=GetXMLTreeChild(rdf,"rdf:Description");
      while (description != (XMLTreeInfo *) NULL)
      {
        node=GetXMLTreeChild(description,(const char *) NULL);
        while (node != (XMLTreeInfo *) NULL)
        {
          child=GetXMLTreeChild(node,(const char *) NULL);
          if (child == (XMLTreeInfo *) NULL)
            (void) AddValueToSplayTree((SplayTreeInfo *) image->properties,
              ConstantString(GetXMLTreeTag(node)),
              ConstantString(GetXMLTreeContent(node)));
          while (child != (XMLTreeInfo *) NULL)
          {
            if (LocaleCompare(GetXMLTreeTag(child),"rdf:Seq") != 0)
              (void) AddValueToSplayTree((SplayTreeInfo *) image->properties,
                ConstantString(GetXMLTreeTag(child)),
                ConstantString(GetXMLTreeContent(child)));
            child=GetXMLTreeSibling(child);
          }
          node=GetXMLTreeSibling(node);
        }
        description=GetNextXMLTreeTag(description);
      }
    }
  xmp=DestroyXMLTree(xmp);
  return(status);
}

static char *TracePSClippath(const unsigned char *blob,size_t length,
  const size_t magick_unused(columns),
  const size_t magick_unused(rows))
{
  char
    *path,
    *message;

  MagickBooleanType
    in_subpath;

  PointInfo
    first[3],
    last[3],
    point[3];

  register ssize_t
    i,
    x;

  ssize_t
    knot_count,
    selector,
    y;

  path=AcquireString((char *) NULL);
  if (path == (char *) NULL)
    return((char *) NULL);
  message=AcquireString((char *) NULL);
  (void) FormatLocaleString(message,MaxTextExtent,"/ClipImage\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"{\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"  /c {curveto} bind def\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"  /l {lineto} bind def\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"  /m {moveto} bind def\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,
    "  /v {currentpoint 6 2 roll curveto} bind def\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,
    "  /y {2 copy curveto} bind def\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,
    "  /z {closepath} bind def\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"  newpath\n");
  (void) ConcatenateString(&path,message);
  /*
    The clipping path format is defined in "Adobe Photoshop File
    Formats Specification" version 6.0 downloadable from adobe.com.
  */
  (void) ResetMagickMemory(point,0,sizeof(point));
  (void) ResetMagickMemory(first,0,sizeof(first));
  (void) ResetMagickMemory(last,0,sizeof(last));
  knot_count=0;
  in_subpath=MagickFalse;
  while (length > 0)
  {
    selector=(ssize_t) ReadPropertyMSBShort(&blob,&length);
    switch (selector)
    {
      case 0:
      case 3:
      {
        if (knot_count != 0)
          {
            blob+=24;
            length-=24;
            break;
          }
        /*
          Expected subpath length record.
        */
        knot_count=(ssize_t) ReadPropertyMSBShort(&blob,&length);
        blob+=22;
        length-=22;
        break;
      }
      case 1:
      case 2:
      case 4:
      case 5:
      {
        if (knot_count == 0)
          {
            /*
              Unexpected subpath knot
            */
            blob+=24;
            length-=24;
            break;
          }
        /*
          Add sub-path knot
        */
        for (i=0; i < 3; i++)
        {
          size_t
            xx,
            yy;

          yy=ReadPropertyMSBLong(&blob,&length);
          xx=ReadPropertyMSBLong(&blob,&length);
          x=(ssize_t) xx;
          if (xx > 2147483647)
            x=(ssize_t) xx-4294967295U-1;
          y=(ssize_t) yy;
          if (yy > 2147483647)
            y=(ssize_t) yy-4294967295U-1;
          point[i].x=(double) x/4096/4096;
          point[i].y=1.0-(double) y/4096/4096;
        }
        if (in_subpath == MagickFalse)
          {
            (void) FormatLocaleString(message,MaxTextExtent,"  %g %g m\n",
              point[1].x,point[1].y);
            for (i=0; i < 3; i++)
            {
              first[i]=point[i];
              last[i]=point[i];
            }
          }
        else
          {
            /*
              Handle special cases when Bezier curves are used to describe
              corners and straight lines.
            */
            if ((last[1].x == last[2].x) && (last[1].y == last[2].y) &&
                (point[0].x == point[1].x) && (point[0].y == point[1].y))
              (void) FormatLocaleString(message,MaxTextExtent,
                "  %g %g l\n",point[1].x,point[1].y);
            else
              if ((last[1].x == last[2].x) && (last[1].y == last[2].y))
                (void) FormatLocaleString(message,MaxTextExtent,
                  "  %g %g %g %g v\n",point[0].x,point[0].y,
                  point[1].x,point[1].y);
              else
                if ((point[0].x == point[1].x) && (point[0].y == point[1].y))
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "  %g %g %g %g y\n",last[2].x,last[2].y,
                    point[1].x,point[1].y);
                else
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "  %g %g %g %g %g %g c\n",last[2].x,
                    last[2].y,point[0].x,point[0].y,point[1].x,point[1].y);
            for (i=0; i < 3; i++)
              last[i]=point[i];
          }
        (void) ConcatenateString(&path,message);
        in_subpath=MagickTrue;
        knot_count--;
        /*
          Close the subpath if there are no more knots.
        */
        if (knot_count == 0)
          {
            /*
              Same special handling as above except we compare to the
              first point in the path and close the path.
            */
            if ((last[1].x == last[2].x) && (last[1].y == last[2].y) &&
                (first[0].x == first[1].x) && (first[0].y == first[1].y))
              (void) FormatLocaleString(message,MaxTextExtent,
                "  %g %g l z\n",first[1].x,first[1].y);
            else
              if ((last[1].x == last[2].x) && (last[1].y == last[2].y))
                (void) FormatLocaleString(message,MaxTextExtent,
                  "  %g %g %g %g v z\n",first[0].x,first[0].y,
                  first[1].x,first[1].y);
              else
                if ((first[0].x == first[1].x) && (first[0].y == first[1].y))
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "  %g %g %g %g y z\n",last[2].x,last[2].y,
                    first[1].x,first[1].y);
                else
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "  %g %g %g %g %g %g c z\n",last[2].x,
                    last[2].y,first[0].x,first[0].y,first[1].x,first[1].y);
            (void) ConcatenateString(&path,message);
            in_subpath=MagickFalse;
          }
        break;
      }
      case 6:
      case 7:
      case 8:
      default:
      {
        blob+=24;
        length-=24;
        break;
      }
    }
  }
  /*
    Returns an empty PS path if the path has no knots.
  */
  (void) FormatLocaleString(message,MaxTextExtent,"  eoclip\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"} bind def");
  (void) ConcatenateString(&path,message);
  message=DestroyString(message);
  return(path);
}

static char *TraceSVGClippath(const unsigned char *blob,size_t length,
  const size_t columns,const size_t rows)
{
  char
    *path,
    *message;

  MagickBooleanType
    in_subpath;

  PointInfo
    first[3],
    last[3],
    point[3];

  register ssize_t
    i;

  ssize_t
    knot_count,
    selector,
    x,
    y;

  path=AcquireString((char *) NULL);
  if (path == (char *) NULL)
    return((char *) NULL);
  message=AcquireString((char *) NULL);
  (void) FormatLocaleString(message,MaxTextExtent,
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,
    "<svg width=\"%.20g\" height=\"%.20g\">\n",(double) columns,(double) rows);
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"<g>\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,
    "<path style=\"fill:#00000000;stroke:#00000000;");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,
    "stroke-width:0;stroke-antialiasing:false\" d=\"\n");
  (void) ConcatenateString(&path,message);
  (void) ResetMagickMemory(point,0,sizeof(point));
  (void) ResetMagickMemory(first,0,sizeof(first));
  (void) ResetMagickMemory(last,0,sizeof(last));
  knot_count=0;
  in_subpath=MagickFalse;
  while (length != 0)
  {
    selector=(ssize_t) ReadPropertyMSBShort(&blob,&length);
    switch (selector)
    {
      case 0:
      case 3:
      {
        if (knot_count != 0)
          {
            blob+=24;
            length-=24;
            break;
          }
        /*
          Expected subpath length record.
        */
        knot_count=(ssize_t) ReadPropertyMSBShort(&blob,&length);
        blob+=22;
        length-=22;
        break;
      }
      case 1:
      case 2:
      case 4:
      case 5:
      {
        if (knot_count == 0)
          {
            /*
              Unexpected subpath knot.
            */
            blob+=24;
            length-=24;
            break;
          }
        /*
          Add sub-path knot
        */
        for (i=0; i < 3; i++)
        {
          size_t
            xx,
            yy;

          yy=ReadPropertyMSBLong(&blob,&length);
          xx=ReadPropertyMSBLong(&blob,&length);
          x=(ssize_t) xx;
          if (xx > 2147483647)
            x=(ssize_t) xx-4294967295U-1;
          y=(ssize_t) yy;
          if (yy > 2147483647)
            y=(ssize_t) yy-4294967295U-1;
          point[i].x=(double) x*columns/4096/4096;
          point[i].y=(double) y*rows/4096/4096;
        }
        if (in_subpath == MagickFalse)
          {
            (void) FormatLocaleString(message,MaxTextExtent,"M %g,%g\n",
              point[1].x,point[1].y);
            for (i=0; i < 3; i++)
            {
              first[i]=point[i];
              last[i]=point[i];
            }
          }
        else
          {
            if ((last[1].x == last[2].x) && (last[1].y == last[2].y) &&
                (point[0].x == point[1].x) && (point[0].y == point[1].y))
              (void) FormatLocaleString(message,MaxTextExtent,"L %g,%g\n",
                point[1].x,point[1].y);
            else
              (void) FormatLocaleString(message,MaxTextExtent,
                "C %g,%g %g,%g %g,%g\n",last[2].x,last[2].y,
                point[0].x,point[0].y,point[1].x,point[1].y);
            for (i=0; i < 3; i++)
              last[i]=point[i];
          }
        (void) ConcatenateString(&path,message);
        in_subpath=MagickTrue;
        knot_count--;
        /*
          Close the subpath if there are no more knots.
        */
        if (knot_count == 0)
          {
            if ((last[1].x == last[2].x) && (last[1].y == last[2].y) &&
                (first[0].x == first[1].x) && (first[0].y == first[1].y))
              (void) FormatLocaleString(message,MaxTextExtent,
                "L %g,%g Z\n",first[1].x,first[1].y);
            else
              {
                (void) FormatLocaleString(message,MaxTextExtent,
                  "C %g,%g %g,%g %g,%g Z\n",last[2].x,
                  last[2].y,first[0].x,first[0].y,first[1].x,first[1].y);
                (void) ConcatenateString(&path,message);
              }
            in_subpath=MagickFalse;
          }
        break;
      }
      case 6:
      case 7:
      case 8:
      default:
      {
        blob+=24;
        length-=24;
        break;
      }
    }
  }
  /*
    Return an empty SVG image if the path does not have knots.
  */
  (void) FormatLocaleString(message,MaxTextExtent,"\"/>\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"</g>\n");
  (void) ConcatenateString(&path,message);
  (void) FormatLocaleString(message,MaxTextExtent,"</svg>\n");
  (void) ConcatenateString(&path,message);
  message=DestroyString(message);
  return(path);
}

MagickExport const char *GetImageProperty(const Image *image,
  const char *property)
{
  ExceptionInfo
    *exception;

  FxInfo
    *fx_info;

  MagickRealType
    alpha;

  MagickStatusType
    status;

  register const char
    *p;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  p=(const char *) NULL;
  if (image->properties != (void *) NULL)
    {
      if (property == (const char *) NULL)
        {
          ResetSplayTreeIterator((SplayTreeInfo *) image->properties);
          p=(const char *) GetNextValueInSplayTree((SplayTreeInfo *)
            image->properties);
          return(p);
        }
      if (LocaleNCompare("fx:",property,3) != 0)
        {
          p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
            image->properties,property);
          if (p != (const char *) NULL)
            return(p);
        }
    }
  if ((property == (const char *) NULL) ||
      (strchr(property,':') == (char *) NULL))
    return(p);
  exception=(&((Image *) image)->exception);
  switch (*property)
  {
    case '8':
    {
      if (LocaleNCompare("8bim:",property,5) == 0)
        {
          if ((Get8BIMProperty(image,property) != MagickFalse) &&
              (image->properties != (void *) NULL))
            {
              p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
                image->properties,property);
              return(p);
            }
        }
      break;
    }
    case 'E':
    case 'e':
    {
      if (LocaleNCompare("exif:",property,5) == 0)
        {
          if ((GetEXIFProperty(image,property) != MagickFalse) &&
              (image->properties != (void *) NULL))
            {
              p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
                image->properties,property);
              return(p);
            }
        }
      break;
    }
    case 'F':
    case 'f':
    {
      if (LocaleNCompare("fx:",property,3) == 0)
        {
          fx_info=AcquireFxInfo(image,property+3);
          status=FxEvaluateChannelExpression(fx_info,DefaultChannels,0,0,&alpha,
            exception);
          fx_info=DestroyFxInfo(fx_info);
          if (status != MagickFalse)
            {
              char
                value[MaxTextExtent];

              (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
                GetMagickPrecision(),(double) alpha);
              (void) SetImageProperty((Image *) image,property,value);
            }
          if (image->properties != (void *) NULL)
            {
              p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
                image->properties,property);
              return(p);
            }
        }
      break;
    }
    case 'I':
    case 'i':
    {
      if (LocaleNCompare("iptc:",property,5) == 0)
        {
          if ((GetIPTCProperty(image,property) != MagickFalse) &&
              (image->properties != (void *) NULL))
            {
              p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
                image->properties,property);
              return(p);
            }
        }
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleNCompare("pixel:",property,6) == 0)
        {
          MagickPixelPacket
            pixel;

          GetMagickPixelPacket(image,&pixel);
          fx_info=AcquireFxInfo(image,property+6);
          status=FxEvaluateChannelExpression(fx_info,RedChannel,0,0,&alpha,
            exception);
          pixel.red=(MagickRealType) QuantumRange*alpha;
          status|=FxEvaluateChannelExpression(fx_info,GreenChannel,0,0,&alpha,
            exception);
          pixel.green=(MagickRealType) QuantumRange*alpha;
          status|=FxEvaluateChannelExpression(fx_info,BlueChannel,0,0,&alpha,
            exception);
          pixel.blue=(MagickRealType) QuantumRange*alpha;
          status|=FxEvaluateChannelExpression(fx_info,OpacityChannel,0,0,&alpha,
            exception);
          pixel.opacity=(MagickRealType) QuantumRange*(1.0-alpha);
          if (image->colorspace == CMYKColorspace)
            {
              status|=FxEvaluateChannelExpression(fx_info,BlackChannel,0,0,
                &alpha,exception);
              pixel.index=(MagickRealType) QuantumRange*alpha;
            }
          fx_info=DestroyFxInfo(fx_info);
          if (status != MagickFalse)
            {
              char
                name[MaxTextExtent];

              (void) QueryMagickColorname(image,&pixel,SVGCompliance,name,
                exception);
              (void) SetImageProperty((Image *) image,property,name);
              return(GetImageProperty(image,property));
            }
        }
      break;
    }
    case 'X':
    case 'x':
    {
      if (LocaleNCompare("xmp:",property,4) == 0)
        {
          if ((GetXMPProperty(image,property) != MagickFalse) &&
              (image->properties != (void *) NULL))
            {
              p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
                image->properties,property);
              return(p);
            }
        }
      break;
    }
    default:
      break;
  }
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k P r o p e r t y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickProperty() gets a value associated with an image property.
%
%  The format of the GetMagickProperty method is:
%
%      const char *GetMagickProperty(const ImageInfo *image_info,Image *image,
%        const char *key)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o key: the key.
%
*/
MagickExport const char *GetMagickProperty(const ImageInfo *image_info,
  Image *image,const char *property)
{
  char
    value[MaxTextExtent],
    filename[MaxTextExtent];

  *value='\0';
  switch (*property)
  {
    case 'b':
    {
      if (LocaleNCompare("base",property,4) == 0)
        {
          GetPathComponent(image->magick_filename,BasePath,filename);
          (void) CopyMagickString(value,filename,MaxTextExtent);
          break;
        }
      break;
    }
    case 'c':
    {
      if (LocaleNCompare("channels",property,8) == 0)
        {
          /*
            Image channels.
          */
          (void) FormatLocaleString(value,MaxTextExtent,"%s",
            CommandOptionToMnemonic(MagickColorspaceOptions,(ssize_t)
            image->colorspace));
          LocaleLower(value);
          if (image->matte != MagickFalse)
            (void) ConcatenateMagickString(value,"a",MaxTextExtent);
          break;
        }
      if (LocaleNCompare("colorspace",property,10) == 0)
        {
          ColorspaceType
            colorspace;

          /*
            Image storage class and colorspace.
          */
          colorspace=image->colorspace;
          if (IsGrayImage(image,&image->exception) != MagickFalse)
            colorspace=GRAYColorspace;
          (void) FormatLocaleString(value,MaxTextExtent,"%s",
            CommandOptionToMnemonic(MagickColorspaceOptions,(ssize_t)
            colorspace));
          break;
        }
      if (LocaleNCompare("copyright",property,9) == 0)
        {
          (void) CopyMagickString(value,GetMagickCopyright(),MaxTextExtent);
          break;
        }
      break;
    }
    case 'd':
    {
      if (LocaleNCompare("depth",property,5) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            image->depth);
          break;
        }
      if (LocaleNCompare("directory",property,9) == 0)
        {
          GetPathComponent(image->magick_filename,HeadPath,filename);
          (void) CopyMagickString(value,filename,MaxTextExtent);
          break;
        }
      break;
    }
    case 'e':
    {
      if (LocaleNCompare("extension",property,9) == 0)
        {
          GetPathComponent(image->magick_filename,ExtensionPath,filename);
          (void) CopyMagickString(value,filename,MaxTextExtent);
          break;
        }
      break;
    }
    case 'g':
    {
      if (LocaleNCompare("group",property,5) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"0x%lx",
            (unsigned long) image_info->group);
          break;
        }
      break;
    }
    case 'h':
    {
      if (LocaleNCompare("height",property,6) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",
            image->magick_rows != 0 ? (double) image->magick_rows : 256.0);
          break;
        }
      break;
    }
    case 'i':
    {
      if (LocaleNCompare("input",property,5) == 0)
        {
          (void) CopyMagickString(value,image->filename,MaxTextExtent);
          break;
        }
      break;
    }
    case 'k':
    {
      if (LocaleNCompare("kurtosis",property,8) == 0)
        {
          double
            kurtosis,
            skewness;

          (void) GetImageChannelKurtosis(image,image_info->channel,&kurtosis,
            &skewness,&image->exception);
          (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
            GetMagickPrecision(),kurtosis);
          break;
        }
      break;
    }
    case 'm':
    {
      if (LocaleNCompare("magick",property,6) == 0)
        {
          (void) CopyMagickString(value,image->magick,MaxTextExtent);
          break;
        }
      if (LocaleNCompare("max",property,3) == 0)
        {
          double
            maximum,
            minimum;

          (void) GetImageChannelRange(image,image_info->channel,&minimum,
            &maximum,&image->exception);
          (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
            GetMagickPrecision(),maximum);
          break;
        }
      if (LocaleNCompare("mean",property,4) == 0)
        {
          double
            mean,
            standard_deviation;

          (void) GetImageChannelMean(image,image_info->channel,&mean,
            &standard_deviation,&image->exception);
          (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
            GetMagickPrecision(),mean);
          break;
        }
      if (LocaleNCompare("min",property,3) == 0)
        {
          double
            maximum,
            minimum;

          (void) GetImageChannelRange(image,image_info->channel,&minimum,
            &maximum,&image->exception);
          (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
            GetMagickPrecision(),minimum);
          break;
        }
      break;
    }
    case 'n':
    {
      if (LocaleNCompare("name",property,4) == 0)
        {
          (void) CopyMagickString(value,filename,MaxTextExtent);
          break;
        }
     break;
    }
    case 'o':
    {
      if (LocaleNCompare("opaque",property,6) == 0)
        {
          MagickBooleanType
            opaque;

          opaque=IsOpaqueImage(image,&image->exception);
          (void) CopyMagickString(value,opaque == MagickFalse ? "false" :
            "true",MaxTextExtent);
          break;
        }
      if (LocaleNCompare("output",property,6) == 0)
        {
          (void) CopyMagickString(value,image_info->filename,MaxTextExtent);
          break;
        }
     break;
    }
    case 'p':
    {
      if (LocaleNCompare("page",property,4) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            GetImageIndexInList(image)+1);
          break;
        }
      break;
    }
    case 's':
    {
      if (LocaleNCompare("size",property,4) == 0)
        {
          char
            format[MaxTextExtent];

          (void) FormatMagickSize(GetBlobSize(image),MagickFalse,format);
          (void) FormatLocaleString(value,MaxTextExtent,"%sB",format);
          break;
        }
      if (LocaleNCompare("scenes",property,6) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            GetImageListLength(image));
          break;
        }
      if (LocaleNCompare("scene",property,5) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            image->scene);
          if (image_info->number_scenes != 0)
            (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
              image_info->scene);
          break;
        }
      if (LocaleNCompare("skewness",property,8) == 0)
        {
          double
            kurtosis,
            skewness;

          (void) GetImageChannelKurtosis(image,image_info->channel,&kurtosis,
            &skewness,&image->exception);
          (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
            GetMagickPrecision(),skewness);
          break;
        }
      if ((LocaleNCompare("standard-deviation",property,18) == 0) ||
          (LocaleNCompare("standard_deviation",property,18) == 0))
        {
          double
            mean,
            standard_deviation;

          (void) GetImageChannelMean(image,image_info->channel,&mean,
            &standard_deviation,&image->exception);
          (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
            GetMagickPrecision(),standard_deviation);
          break;
        }
       break;
    }
    case 'u':
    {
      if (LocaleNCompare("unique",property,6) == 0)
        {
          (void) CopyMagickString(filename,image_info->unique,MaxTextExtent);
          (void) CopyMagickString(value,filename,MaxTextExtent);
          break;
        }
      break;
    }
    case 'v':
    {
      if (LocaleNCompare("version",property,7) == 0)
        {
          (void) CopyMagickString(value,GetMagickVersion((size_t *) NULL),
            MaxTextExtent);
          break;
        }
      break;
    }
    case 'w':
    {
      if (LocaleNCompare("width",property,5) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            (image->magick_columns != 0 ? image->magick_columns : 256));
          break;
        }
      break;
    }
    case 'x':
    {
      if (LocaleNCompare("xresolution",property,11) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%g",
            image->x_resolution);
          break;
        }
      break;
    }
    case 'y':
    {
      if (LocaleNCompare("yresolution",property,11) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%g",
            image->y_resolution);
          break;
        }
      break;
    }
    case 'z':
    {
      if (LocaleNCompare("zero",property,4) == 0)
        {
          (void) CopyMagickString(filename,image_info->zero,MaxTextExtent);
          (void) CopyMagickString(value,filename,MaxTextExtent);
          break;
        }
      break;
    }
  }
  if (*value != '\0')
   {
     if (image->properties == (void *) NULL)
       image->properties=NewSplayTree(CompareSplayTreeString,
         RelinquishMagickMemory,RelinquishMagickMemory);
     (void) AddValueToSplayTree((SplayTreeInfo *) image->properties,
       ConstantString(property),ConstantString(value));
   }
  return(GetImageProperty(image,property));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e P r o p e r t y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImageProperty() gets the next image property value.
%
%  The format of the GetNextImageProperty method is:
%
%      char *GetNextImageProperty(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport char *GetNextImageProperty(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image->filename);
  if (image->properties == (void *) NULL)
    return((char *) NULL);
  return((char *) GetNextKeyInSplayTree((SplayTreeInfo *) image->properties));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p r e t I m a g e P r o p e r t i e s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretImageProperties() replaces any embedded formatting characters with
%  the appropriate image property and returns the interpretted text.
%
%  The format of the InterpretImageProperties method is:
%
%      char *InterpretImageProperties(const ImageInfo *image_info,Image *image,
%        const char *embed_text)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o embed_text: the address of a character string containing the embedded
%      formatting characters.
%
*/
MagickExport char *InterpretImageProperties(const ImageInfo *image_info,
  Image *image,const char *embed_text)
{
  char
    filename[MaxTextExtent],
    *interpret_text,
    *text;

  const char
    *value;

  register char
    *q;

  register const char
    *p;

  register ssize_t
    i;

  size_t
    extent,
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((embed_text == (const char *) NULL) || (*embed_text == '\0'))
    return((char *) NULL);
  text=(char *) embed_text;
  if ((*text == '@') && ((*(text+1) == '-') ||
      (IsPathAccessible(text+1) != MagickFalse)))
    return(FileToString(embed_text+1,~0,&image->exception));
  /*
    Translate any embedded format characters.
  */
  interpret_text=AcquireString(text);
  extent=MaxTextExtent;
  p=text;
  for (q=interpret_text; *p != '\0'; p++)
  {
    *q='\0';
    if ((size_t) (q-interpret_text+MaxTextExtent) >= extent)
      {
        extent+=MaxTextExtent;
        interpret_text=(char *) ResizeQuantumMemory(interpret_text,extent+
          MaxTextExtent+1,sizeof(*interpret_text));
        if (interpret_text == (char *) NULL)
          break;
        q=interpret_text+strlen(interpret_text);
      }
    /*
      Process formatting characters in text.
    */
    if ((*p == '\\') && (*(p+1) == 'r'))
      {
        *q++='\r';
        p++;
        continue;
      }
    if ((*p == '\\') && (*(p+1) == 'n'))
      {
        *q++='\n';
        p++;
        continue;
      }
    if (*p == '\\')
      {
        p++;
        *q++=(*p);
        continue;
      }
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    switch (*p)
    {
      case 'b':
      {
        char
          format[MaxTextExtent];

        /*
          File size.
        */
        (void) FormatLocaleString(format,MaxTextExtent,"%.20g",(double)
          ((MagickOffsetType) image->extent));
        if (image->extent != (MagickSizeType) ((size_t) image->extent))
          (void) FormatMagickSize(image->extent,MagickFalse,format);
        q+=ConcatenateMagickString(q,format,extent);
        q+=ConcatenateMagickString(q,"B",extent);
        break;
      }
      case 'c':
      {
        /*
          Image comment.
        */
        value=GetImageProperty(image,"comment");
        if (value == (const char *) NULL)
          break;
        length=strlen(value);
        if ((size_t) (q-interpret_text+length+1) >= extent)
          {
            extent+=length;
            interpret_text=(char *) ResizeQuantumMemory(interpret_text,extent+
              MaxTextExtent,sizeof(*interpret_text));
            if (interpret_text == (char *) NULL)
              break;
            q=interpret_text+strlen(interpret_text);
          }
        (void) CopyMagickString(q,value,extent);
        q+=length;
        break;
      }
      case 'd':
      case 'e':
      case 'f':
      case 't':
      {
        /*
          Label segment is the base of the filename.
        */
        if (*image->magick_filename == '\0')
          break;
        switch (*p)
        {
          case 'd':
          {
            /*
              Directory.
            */
            GetPathComponent(image->magick_filename,HeadPath,filename);
            q+=CopyMagickString(q,filename,extent);
            break;
          }
          case 'e':
          {
            /*
              Filename extension.
            */
            GetPathComponent(image->magick_filename,ExtensionPath,filename);
            q+=CopyMagickString(q,filename,extent);
            break;
          }
          case 'f':
          {
            /*
              Filename.
            */
            GetPathComponent(image->magick_filename,TailPath,filename);
            q+=CopyMagickString(q,filename,extent);
            break;
          }
          case 't':
          {
            /*
              Base filename.
            */
            GetPathComponent(image->magick_filename,BasePath,filename);
            q+=CopyMagickString(q,filename,extent);
            break;
          }
        }
        break;
      }
      case 'g':
      {
        /*
          Image geometry.
        */
        q+=FormatLocaleString(q,extent,"%.20gx%.20g%+.20g%+.20g",(double)
          image->page.width,(double) image->page.height,(double) image->page.x,
          (double) image->page.y);
        break;
      }
      case 'h':
      {
        /*
          Image height.
        */
        q+=FormatLocaleString(q,extent,"%.20g",(double) (image->rows != 0 ?
          image->rows : image->magick_rows));
        break;
      }
      case 'i':
      {
        /*
          Image filename.
        */
        q+=CopyMagickString(q,image->filename,extent);
        break;
      }
      case 'k':
      {
        /*
          Number of unique colors.
        */
        q+=FormatLocaleString(q,extent,"%.20g",(double) GetNumberColors(image,
          (FILE *) NULL,&image->exception));
        break;
      }
      case 'l':
      {
        /*
          Image label.
        */
        value=GetImageProperty(image,"label");
        if (value == (const char *) NULL)
          break;
        length=strlen(value);
        if ((size_t) (q-interpret_text+length+1) >= extent)
          {
            extent+=length;
            interpret_text=(char *) ResizeQuantumMemory(interpret_text,extent+
              MaxTextExtent,sizeof(*interpret_text));
            if (interpret_text == (char *) NULL)
              break;
            q=interpret_text+strlen(interpret_text);
          }
        q+=CopyMagickString(q,value,extent);
        break;
      }
      case 'm':
      {
        /*
          Image format.
        */
        q+=CopyMagickString(q,image->magick,extent);
        break;
      }
      case 'M':
      {
        /*
          Image magick filename.
        */
        q+=CopyMagickString(q,image->magick_filename,extent);
        break;
      }
      case 'n':
      {
        /*
          Number of images in the list.
        */
        q+=FormatLocaleString(q,extent,"%.20g",(double)
          GetImageListLength(image));
        break;
      }
      case 'o':
      {
        /*
          Image output filename.
        */
        q+=CopyMagickString(q,image_info->filename,extent);
        break;
      }
      case 'p':
      {
        /*
          Image index in list.
        */
        q+=FormatLocaleString(q,extent,"%.20g",(double)
            GetImageIndexInList(image));
        break;
      }
      case 'q':
      {
        /*
          Image depth.
        */
        q+=FormatLocaleString(q,extent,"%.20g",(double)
          MAGICKCORE_QUANTUM_DEPTH);
        break;
      }
      case 'r':
      {
        ColorspaceType
          colorspace;

        /*
          Image storage class and colorspace.
        */
        colorspace=image->colorspace;
        if (IsGrayImage(image,&image->exception) != MagickFalse)
          colorspace=GRAYColorspace;
        q+=FormatLocaleString(q,extent,"%s%s%s",CommandOptionToMnemonic(
          MagickClassOptions,(ssize_t) image->storage_class),
          CommandOptionToMnemonic(MagickColorspaceOptions,(ssize_t) colorspace),
          image->matte != MagickFalse ? "Matte" : "");
        break;
      }
      case 's':
      {
        /*
          Image scene number.
        */
        if (image_info->number_scenes == 0)
          q+=FormatLocaleString(q,extent,"%.20g",(double) image->scene);
        else
          q+=FormatLocaleString(q,extent,"%.20g",(double) image_info->scene);
        break;
      }
      case 'u':
      {
        /*
          Unique filename.
        */
        (void) CopyMagickString(filename,image_info->unique,extent);
        q+=CopyMagickString(q,filename,extent);
        break;
      }
      case 'w':
      {
        /*
          Image width.
        */
        q+=FormatLocaleString(q,extent,"%.20g",(double) (image->columns != 0 ?
          image->columns : image->magick_columns));
        break;
      }
      case 'x':
      {
        /*
          Image horizontal resolution.
        */
        q+=FormatLocaleString(q,extent,"%g %s",image->x_resolution,
          CommandOptionToMnemonic(MagickResolutionOptions,(ssize_t)
            image->units));
        break;
      }
      case 'y':
      {
        /*
          Image vertical resolution.
        */
        q+=FormatLocaleString(q,extent,"%g %s",image->y_resolution,
          CommandOptionToMnemonic(MagickResolutionOptions,(ssize_t)
          image->units));
        break;
      }
      case 'z':
      {
        /*
          Image depth.
        */
        q+=FormatLocaleString(q,extent,"%.20g",(double) image->depth);
        break;
      }
      case 'A':
      {
        /*
          Image alpha channel.
        */
        q+=FormatLocaleString(q,extent,"%s",CommandOptionToMnemonic(
          MagickBooleanOptions,(ssize_t) image->matte));
        break;
      }
      case 'C':
      {
        /*
          Image compression method.
        */
        q+=FormatLocaleString(q,extent,"%s",CommandOptionToMnemonic(
          MagickCompressOptions,(ssize_t) image->compression));
        break;
      }
      case 'D':
      {
        /*
          Image dispose method.
        */
        q+=FormatLocaleString(q,extent,"%s",CommandOptionToMnemonic(
          MagickDisposeOptions,(ssize_t) image->dispose));
        break;
      }
      case 'G':
      {
        q+=FormatLocaleString(q,extent,"%.20gx%.20g",(double)
          image->magick_columns,(double) image->magick_rows);
        break;
      }
      case 'H':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) image->page.height);
        break;
      }
      case 'O':
      {
        q+=FormatLocaleString(q,extent,"%+ld%+ld",(long) image->page.x,(long)
          image->page.y);
        break;
      }
      case 'P':
      {
        q+=FormatLocaleString(q,extent,"%.20gx%.20g",(double) image->page.width,
          (double) image->page.height);
        break;
      }
      case 'Q':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) image->quality);
        break;
      }
      case 'S':
      {
        /*
          Image scenes.
        */
        if (image_info->number_scenes == 0)
          q+=CopyMagickString(q,"2147483647",extent);
        else
          q+=FormatLocaleString(q,extent,"%.20g",(double) (image_info->scene+
            image_info->number_scenes));
        break;
      }
      case 'T':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) image->delay);
        break;
      }
      case 'W':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) image->page.width);
        break;
      }
      case 'X':
      {
        q+=FormatLocaleString(q,extent,"%+.20g",(double) image->page.x);
        break;
      }
      case 'Y':
      {
        q+=FormatLocaleString(q,extent,"%+.20g",(double) image->page.y);
        break;
      }
      case 'Z':
      {
        /*
          Unique filename.
        */
        (void) CopyMagickString(filename,image_info->zero,extent);
        q+=CopyMagickString(q,filename,extent);
        break;
      }
      case '[':
      {
        char
          pattern[MaxTextExtent];

        const char
          *key,
          *value;

        ssize_t
          depth;

        /*
          Image value.
        */
        if (strchr(p,']') == (char *) NULL)
          break;
        depth=1;
        p++;
        for (i=0; (i < (MaxTextExtent-1L)) && (*p != '\0'); i++)
        {
          if (*p == '[')
            depth++;
          if (*p == ']')
            depth--;
          if (depth <= 0)
            break;
          pattern[i]=(*p++);
        }
        pattern[i]='\0';
        value=GetImageProperty(image,pattern);
        if (value != (const char *) NULL)
          {
            length=strlen(value);
            if ((size_t) (q-interpret_text+length+1) >= extent)
              {
                extent+=length;
                interpret_text=(char *) ResizeQuantumMemory(interpret_text,
                  extent+MaxTextExtent,sizeof(*interpret_text));
                if (interpret_text == (char *) NULL)
                  break;
                q=interpret_text+strlen(interpret_text);
              }
            (void) CopyMagickString(q,value,extent);
            q+=length;
            break;
          }
        else
          if (IsGlob(pattern) != MagickFalse)
            {
              /*
                Iterate over image properties.
              */
              ResetImagePropertyIterator(image);
              key=GetNextImageProperty(image);
              while (key != (const char *) NULL)
              {
                if (GlobExpression(key,pattern,MagickTrue) != MagickFalse)
                  {
                    value=GetImageProperty(image,key);
                    if (value != (const char *) NULL)
                      {
                        length=strlen(key)+strlen(value)+2;
                        if ((size_t) (q-interpret_text+length+1) >= extent)
                          {
                            extent+=length;
                            interpret_text=(char *) ResizeQuantumMemory(
                              interpret_text,extent+MaxTextExtent,
                              sizeof(*interpret_text));
                            if (interpret_text == (char *) NULL)
                              break;
                            q=interpret_text+strlen(interpret_text);
                          }
                        q+=FormatLocaleString(q,extent,"%s=%s\n",key,value);
                      }
                  }
                key=GetNextImageProperty(image);
              }
            }
        value=GetMagickProperty(image_info,image,pattern);
        if (value != (const char *) NULL)
          {
            length=strlen(value);
            if ((size_t) (q-interpret_text+length+1) >= extent)
              {
                extent+=length;
                interpret_text=(char *) ResizeQuantumMemory(interpret_text,
                  extent+MaxTextExtent,sizeof(*interpret_text));
                if (interpret_text == (char *) NULL)
                  break;
                q=interpret_text+strlen(interpret_text);
              }
            (void) CopyMagickString(q,value,extent);
            q+=length;
            break;
          }
        if (image_info == (ImageInfo *) NULL)
          break;
        value=GetImageOption(image_info,pattern);
        if (value != (char *) NULL)
          {
            length=strlen(value);
            if ((size_t) (q-interpret_text+length+1) >= extent)
              {
                extent+=length;
                interpret_text=(char *) ResizeQuantumMemory(interpret_text,
                  extent+MaxTextExtent,sizeof(*interpret_text));
                if (interpret_text == (char *) NULL)
                  break;
                q=interpret_text+strlen(interpret_text);
              }
            (void) CopyMagickString(q,value,extent);
            q+=length;
            break;
          }
        break;
      }
      case '@':
      {
        RectangleInfo
          page;

        /*
          Image bounding box.
        */
        page=GetImageBoundingBox(image,&image->exception);
        q+=FormatLocaleString(q,MaxTextExtent,"%.20gx%.20g%+.20g%+.20g",
          (double) page.width,(double) page.height,(double) page.x,(double)
          page.y);
        break;
      }
      case '#':
      {
        /*
          Image signature.
        */
        (void) SignatureImage(image);
        value=GetImageProperty(image,"signature");
        if (value == (const char *) NULL)
          break;
        q+=CopyMagickString(q,value,extent);
        break;
      }
      case '%':
      {
        *q++=(*p);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  *q='\0';
  if (text != (const char *) embed_text)
    text=DestroyString(text);
  (void) SubstituteString(&interpret_text,"&lt;","<");
  (void) SubstituteString(&interpret_text,"&gt;",">");
  return(interpret_text);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e I m a g e P r o p e r t y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveImageProperty() removes a property from the image and returns its
%  value.
%
%  The format of the RemoveImageProperty method is:
%
%      char *RemoveImageProperty(Image *image,const char *property)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o property: the image property.
%
*/
MagickExport char *RemoveImageProperty(Image *image,
  const char *property)
{
  char
    *value;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image->filename);
  if (image->properties == (void *) NULL)
    return((char *) NULL);
  value=(char *) RemoveNodeFromSplayTree((SplayTreeInfo *) image->properties,
    property);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e P r o p e r t y I t e r a t o r                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImagePropertyIterator() resets the image properties iterator.  Use it
%  in conjunction with GetNextImageProperty() to iterate over all the values
%  associated with an image property.
%
%  The format of the ResetImagePropertyIterator method is:
%
%      ResetImagePropertyIterator(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void ResetImagePropertyIterator(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image->filename);
  if (image->properties == (void *) NULL)
    return;
  ResetSplayTreeIterator((SplayTreeInfo *) image->properties);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e P r o p e r t y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageProperty() associates an value with an image property.
%
%  The format of the SetImageProperty method is:
%
%      MagickBooleanType SetImageProperty(Image *image,const char *property,
%        const char *value)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o property: the image property.
%
%    o values: the image property values.
%
*/
MagickExport MagickBooleanType SetImageProperty(Image *image,
  const char *property,const char *value)
{
  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image->filename);
  if (image->properties == (void *) NULL)
    image->properties=NewSplayTree(CompareSplayTreeString,
      RelinquishMagickMemory,RelinquishMagickMemory);
  if ((value == (const char *) NULL) || (*value == '\0'))
    return(DeleteImageProperty(image,property));
  status=MagickTrue;
  exception=(&image->exception);
  switch (*property)
  {
    case 'B':
    case 'b':
    {
      if (LocaleCompare(property,"background") == 0)
        {
          (void) QueryColorDatabase(value,&image->background_color,exception);
          break;
        }
      if (LocaleCompare(property,"bias") == 0)
        {
          image->bias=StringToDoubleInterval(value,(double) QuantumRange+1.0);
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'C':
    case 'c':
    {
      if (LocaleCompare(property,"colorspace") == 0)
        {
          ssize_t
            colorspace;

          colorspace=ParseCommandOption(MagickColorspaceOptions,MagickFalse,
            value);
          if (colorspace < 0)
            break;
          (void) SetImageColorspace(image,(ColorspaceType) colorspace);
          break;
        }
      if (LocaleCompare(property,"compose") == 0)
        {
          ssize_t
            compose;

          compose=ParseCommandOption(MagickComposeOptions,MagickFalse,value);
          if (compose < 0)
            break;
          image->compose=(CompositeOperator) compose;
          break;
        }
      if (LocaleCompare(property,"compress") == 0)
        {
          ssize_t
            compression;

          compression=ParseCommandOption(MagickCompressOptions,MagickFalse,
            value);
          if (compression < 0)
            break;
          image->compression=(CompressionType) compression;
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'D':
    case 'd':
    {
      if (LocaleCompare(property,"delay") == 0)
        {
          GeometryInfo
            geometry_info;

          flags=ParseGeometry(value,&geometry_info);
          if ((flags & GreaterValue) != 0)
            {
              if (image->delay > (size_t) floor(geometry_info.rho+0.5))
                image->delay=(size_t) floor(geometry_info.rho+0.5);
            }
          else
            if ((flags & LessValue) != 0)
              {
                if (image->delay < (size_t) floor(geometry_info.rho+0.5))
                  image->ticks_per_second=(ssize_t)
                    floor(geometry_info.sigma+0.5);
              }
            else
              image->delay=(size_t) floor(geometry_info.rho+0.5);
          if ((flags & SigmaValue) != 0)
            image->ticks_per_second=(ssize_t) floor(geometry_info.sigma+0.5);
          break;
        }
      if (LocaleCompare(property,"density") == 0)
        {
          GeometryInfo
            geometry_info;

          flags=ParseGeometry(value,&geometry_info);
          image->x_resolution=geometry_info.rho;
          image->y_resolution=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            image->y_resolution=image->x_resolution;
        }
      if (LocaleCompare(property,"depth") == 0)
        {
          image->depth=StringToUnsignedLong(value);
          break;
        }
      if (LocaleCompare(property,"dispose") == 0)
        {
          ssize_t
            dispose;

          dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,value);
          if (dispose < 0)
            break;
          image->dispose=(DisposeType) dispose;
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare(property,"gravity") == 0)
        {
          ssize_t
            gravity;

          gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,value);
          if (gravity < 0)
            break;
          image->gravity=(GravityType) gravity;
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'I':
    case 'i':
    {
      if (LocaleCompare(property,"intent") == 0)
        {
          ssize_t
            rendering_intent;

          rendering_intent=ParseCommandOption(MagickIntentOptions,MagickFalse,
            value);
          if (rendering_intent < 0)
            break;
          image->rendering_intent=(RenderingIntent) rendering_intent;
          break;
        }
      if (LocaleCompare(property,"interpolate") == 0)
        {
          ssize_t
            interpolate;

          interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
            value);
          if (interpolate < 0)
            break;
          image->interpolate=(InterpolatePixelMethod) interpolate;
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'L':
    case 'l':
    {
      if (LocaleCompare(property,"loop") == 0)
        {
          image->iterations=StringToUnsignedLong(value);
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare(property,"page") == 0)
        {
          char
            *geometry;

          geometry=GetPageGeometry(value);
          flags=ParseAbsoluteGeometry(geometry,&image->page);
          geometry=DestroyString(geometry);
          break;
        }
      if (LocaleCompare(property,"profile") == 0)
        {
          ImageInfo
            *image_info;

          StringInfo
            *profile;

          image_info=AcquireImageInfo();
          (void) CopyMagickString(image_info->filename,value,MaxTextExtent);
          (void) SetImageInfo(image_info,1,exception);
          profile=FileToStringInfo(image_info->filename,~0UL,exception);
          if (profile != (StringInfo *) NULL)
            status=SetImageProfile(image,image_info->magick,profile);
          image_info=DestroyImageInfo(image_info);
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'R':
    case 'r':
    {
      if (LocaleCompare(property,"rendering-intent") == 0)
        {
          ssize_t
            rendering_intent;

          rendering_intent=ParseCommandOption(MagickIntentOptions,MagickFalse,
            value);
          if (rendering_intent < 0)
            break;
          image->rendering_intent=(RenderingIntent) rendering_intent;
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'T':
    case 't':
    {
      if (LocaleCompare(property,"tile-offset") == 0)
        {
          char
            *geometry;

          geometry=GetPageGeometry(value);
          flags=ParseAbsoluteGeometry(geometry,&image->tile_offset);
          geometry=DestroyString(geometry);
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    case 'U':
    case 'u':
    {
      if (LocaleCompare(property,"units") == 0)
        {
          ssize_t
            units;

          units=ParseCommandOption(MagickResolutionOptions,MagickFalse,value);
          if (units < 0)
            break;
          image->units=(ResolutionType) units;
          break;
        }
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
    default:
    {
      status=AddValueToSplayTree((SplayTreeInfo *) image->properties,
        ConstantString(property),ConstantString(value));
      break;
    }
  }
  return(status);
}

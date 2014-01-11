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
%                                   Cristy                                    %
%                                 March 2000                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/artifact.h"
#include "magick/attribute.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/color.h"
#include "magick/colorspace-private.h"
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
#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(MAGICKCORE_HAVE_LCMS_LCMS2_H)
#include <lcms/lcms2.h>
#elif defined(MAGICKCORE_HAVE_LCMS2_H)
#include "lcms2.h"
#elif defined(MAGICKCORE_HAVE_LCMS_LCMS_H)
#include <lcms/lcms.h>
#else
#include "lcms.h"
#endif
#endif

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
  image->intensity=clone_image->intensity;
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

static inline ssize_t MagickMin(const ssize_t x,const ssize_t y)
{
  if (x < y)
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

static inline size_t ReadPropertyMSBLong(const unsigned char **p,size_t *length)
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
    return((unsigned short) ~0);
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
    id=(ssize_t) ((int) ReadPropertyMSBShort(&info,&length));
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
        if (~((size_t) count) >= (MaxTextExtent-1))
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
          length-=MagickMin(count,(ssize_t) length);
          continue;
        }
    if ((*name == '#') && (sub_number != 1))
      {
        /*
          No numbered match, scroll forward and try next.
        */
        sub_number--;
        info+=count;
        length-=MagickMin(count,(ssize_t) length);
        continue;
      }
    /*
      We have the resource of interest.
    */
    attribute=(char *) NULL;
    if (~((size_t) count) >= (MaxTextExtent-1))
      attribute=(char *) AcquireQuantumMemory((size_t) count+MaxTextExtent,
        sizeof(*attribute));
    if (attribute != (char *) NULL)
      {
        (void) CopyMagickMemory(attribute,(char *) info,(size_t) count);
        attribute[count]='\0';
        info+=count;
        length-=MagickMin(count,(ssize_t) length);
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

  if (endian == LSBEndian)
    {
      value=(unsigned short) ((buffer[1] << 8) | buffer[0]);
      return((unsigned short) (value & 0xffff));
    }
  value=(unsigned short) ((((unsigned char *) buffer)[0] << 8) |
    ((unsigned char *) buffer)[1]);
  return((unsigned short) (value & 0xffff));
}

static inline size_t ReadPropertyLong(const EndianType endian,
  const unsigned char *buffer)
{
  size_t
    value;

  if (endian == LSBEndian)
    {
      value=(size_t) ((buffer[3] << 24) | (buffer[2] << 16) |
        (buffer[1] << 8 ) | (buffer[0]));
      return((size_t) (value & 0xffffffff));
    }
  value=(size_t) ((buffer[0] << 24) | (buffer[1] << 16) |
    (buffer[2] << 8) | buffer[3]);
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

#define EXIFMultipleValues(size,format,arg) \
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

#define EXIFMultipleFractions(size,format,arg1,arg2) \
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
       format", ",arg1,arg2); \
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
      entry;

    ssize_t
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
      { 0x00000, NULL}
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
    tag;

  SplayTreeInfo
    *exif_resources;

  ssize_t
    all,
    id,
    level,
    offset,
    tag_offset,
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
  id=(ssize_t) ((int) ReadPropertyShort(LSBEndian,exif));
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
  if ((offset < 0) || (size_t) offset >= length)
    return(MagickFalse);
  /*
    Set the pointer to the first IFD and follow it were it leads.
  */
  status=MagickFalse;
  directory=exif+offset;
  level=0;
  entry=0;
  tag_offset=0;
  exif_resources=NewSplayTree((int (*)(const void *,const void *)) NULL,
    (void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
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
    number_entries=(size_t) ((int) ReadPropertyShort(endian,directory));
    for ( ; entry < number_entries; entry++)
    {
      register unsigned char
        *p,
        *q;

      size_t
        format;

      ssize_t
        number_bytes,
        components;

      q=(unsigned char *) (directory+(12*entry)+2);
      if (GetValueFromSplayTree(exif_resources,q) == q)
        break;
      (void) AddValueToSplayTree(exif_resources,q,q);
      tag_value=(ssize_t) ((int) ReadPropertyShort(endian,q)+tag_offset);
      format=(size_t) ((int) ReadPropertyShort(endian,q+2));
      if (format >= (sizeof(tag_bytes)/sizeof(*tag_bytes)))
        break;
      components=(ssize_t) ((int) ReadPropertyLong(endian,q+4));
      number_bytes=(size_t) components*tag_bytes[format];
      if (number_bytes < components)
        break;  /* prevent overflow */
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
          if ((ssize_t) (offset+number_bytes) < offset)
            continue;  /* prevent overflow */
          if ((size_t) (offset+number_bytes) > length)
            continue;
          p=(unsigned char *) (exif+offset);
        }
      if ((all != 0) || (tag == (size_t) tag_value))
        {
          char
            buffer[MaxTextExtent],
            *value;

          value=(char *) NULL;
          *buffer='\0';
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
                ((int) ReadPropertyLong(endian,p1)));
              break;
            }
            case EXIF_FMT_SLONG:
            {
              EXIFMultipleValues(4,"%.20g",(double)
                ((int) ReadPropertyLong(endian,p1)));
              break;
            }
            case EXIF_FMT_URATIONAL:
            {
              EXIFMultipleFractions(8,"%.20g/%.20g",(double)
                ((int) ReadPropertyLong(endian,p1)),(double)
                ((int) ReadPropertyLong(endian,p1+4)));
              break;
            }
            case EXIF_FMT_SRATIONAL:
            {
              EXIFMultipleFractions(8,"%.20g/%.20g",(double)
                ((int) ReadPropertyLong(endian,p1)),(double)
                ((int) ReadPropertyLong(endian,p1+4)));
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
              if (~((size_t) number_bytes) >= 1)
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
                *key;

              register const char
                *p;

              key=AcquireString(property);
              if (level == 2)
                (void) SubstituteString(&key,"exif:","exif:thumbnail:");
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
                  (void) FormatLocaleString(key,MaxTextExtent,"%s",description);
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
              key=DestroyString(key);
              status=MagickTrue;
            }
        }
        if ((tag_value == TAG_EXIF_OFFSET) ||
            (tag_value == TAG_INTEROP_OFFSET) || (tag_value == TAG_GPS_OFFSET))
          {
            ssize_t
              offset;

            offset=(ssize_t) ((int) ReadPropertyLong(endian,p));
            if (((size_t) offset < length) && (level < (MaxDirectoryStack-2)))
              {
                ssize_t
                  tag_offset1;

                tag_offset1=(ssize_t) ((tag_value == TAG_GPS_OFFSET) ? 0x10000 :
                  0);
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
                offset=(ssize_t) ((int) ReadPropertyLong(endian,directory+2+(12*
                  number_entries)));
                if ((offset != 0) && ((size_t) offset < length) &&
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
  exif_resources=DestroySplayTree(exif_resources);
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
  const size_t magick_unused(columns),const size_t magick_unused(rows))
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

  magick_unreferenced(columns);
  magick_unreferenced(rows);

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
    selector=(ssize_t) ((int) ReadPropertyMSBShort(&blob,&length));
    switch (selector)
    {
      case 0:
      case 3:
      {
        if (knot_count != 0)
          {
            blob+=24;
            length-=MagickMin(24,(ssize_t) length);
            break;
          }
        /*
          Expected subpath length record.
        */
        knot_count=(ssize_t) ((int) ReadPropertyMSBShort(&blob,&length));
        blob+=22;
        length-=MagickMin(22,(ssize_t) length);
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
            length-=MagickMin(24,(ssize_t) length);
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

          yy=(size_t) ((int) ReadPropertyMSBLong(&blob,&length));
          xx=(size_t) ((int) ReadPropertyMSBLong(&blob,&length));
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
        length-=MagickMin(24,(ssize_t) length);
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
    selector=(ssize_t) ((int) ReadPropertyMSBShort(&blob,&length));
    switch (selector)
    {
      case 0:
      case 3:
      {
        if (knot_count != 0)
          {
            blob+=24;
            length-=MagickMin(24,(ssize_t) length);
            break;
          }
        /*
          Expected subpath length record.
        */
        knot_count=(ssize_t) ((int) ReadPropertyMSBShort(&blob,&length));
        blob+=22;
        length-=MagickMin(22,(ssize_t) length);
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
            length-=MagickMin(24,(ssize_t) length);
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

          yy=(size_t) ((int) ReadPropertyMSBLong(&blob,&length));
          xx=(size_t) ((int) ReadPropertyMSBLong(&blob,&length));
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
            (void) FormatLocaleString(message,MaxTextExtent,"M %g %g\n",
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
                "L %g %g\n",point[1].x,point[1].y);
            else
              if ((last[1].x == last[2].x) && (last[1].y == last[2].y))
                (void) FormatLocaleString(message,MaxTextExtent,
                  "V %g %g %g %g\n",point[0].x,point[0].y,
                  point[1].x,point[1].y);
              else
                if ((point[0].x == point[1].x) && (point[0].y == point[1].y))
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "Y %g %g %g %g\n",last[2].x,last[2].y,
                    point[1].x,point[1].y);
                else
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "C %g %g %g %g %g %g\n",last[2].x,
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
                "L %g %g Z\n",first[1].x,first[1].y);
            else
              if ((last[1].x == last[2].x) && (last[1].y == last[2].y))
                (void) FormatLocaleString(message,MaxTextExtent,
                  "V %g %g %g %g Z\n",first[0].x,first[0].y,
                  first[1].x,first[1].y);
              else
                if ((first[0].x == first[1].x) && (first[0].y == first[1].y))
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "Y %g %g %g %g Z\n",last[2].x,last[2].y,
                    first[1].x,first[1].y);
                else
                  (void) FormatLocaleString(message,MaxTextExtent,
                    "C %g %g %g %g %g %g Z\n",last[2].x,
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
        length-=MagickMin(24,(ssize_t) length);
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
  double
    alpha;

  ExceptionInfo
    *exception;

  FxInfo
    *fx_info;

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
      if (LocaleNCompare("fx:",property,3) != 0) /* NOT fx: !!!! */
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
          status&=FxEvaluateChannelExpression(fx_info,GreenChannel,0,0,&alpha,
            exception);
          pixel.green=(MagickRealType) QuantumRange*alpha;
          status&=FxEvaluateChannelExpression(fx_info,BlueChannel,0,0,&alpha,
            exception);
          pixel.blue=(MagickRealType) QuantumRange*alpha;
          status&=FxEvaluateChannelExpression(fx_info,OpacityChannel,0,0,&alpha,
            exception);
          pixel.opacity=(MagickRealType) QuantumRange*(1.0-alpha);
          if (image->colorspace == CMYKColorspace)
            {
              status&=FxEvaluateChannelExpression(fx_info,BlackChannel,0,0,
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
%  GetMagickProperty() gets attributes or calculated values that is associated
%  with a fixed known property name, or single letter property.
%
%  This does not return, special profile or property expressions. Nor does it
%  return free-form property strings, unless referenced by a single letter
%  property name.
%
%  The returned string is stored as the image artifact 'get-property' (not as
%  another property), and as such should not be freed. Later calls however
%  will overwrite this value so if needed for a longer period a copy should be
%  made.  This artifact can be deleted when no longer required.
%
%  The format of the GetMagickProperty method is:
%
%      const char *GetMagickProperty(const ImageInfo *image_info,Image *image,
%        const char *property)
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
static const char *GetMagickPropertyLetter(const ImageInfo *image_info,
  Image *image,const char letter)
{
  char
    value[MaxTextExtent];

  const char
    *string;

  if ((image != (Image *) NULL) && (image->debug != MagickFalse))
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *value='\0';
  string=(char *)NULL;
  switch (letter)
  {
    case 'b':  /* image size read in - in bytes */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        ((MagickOffsetType) image->extent));
      if (image->extent != (MagickSizeType) ((size_t) image->extent))
        (void) FormatMagickSize(image->extent,MagickFalse,value);
      ConcatenateMagickString(value,"B",MaxTextExtent);
      break;
    }
    case 'c':  /* image comment property - empty string by default */
    {
      string=GetImageProperty(image,"comment");
      if (string == (const char *) NULL)
        string="";
      break;
    }
    case 'd':  /* Directory component of filename */
    {
      GetPathComponent(image->magick_filename,HeadPath,value);
      break;
    }
    case 'e': /* Filename extension (suffix) of image file */
    {
      GetPathComponent(image->magick_filename,ExtensionPath,value);
      break;
    }
    case 'f': /* Filename without directory component */
    {
      GetPathComponent(image->magick_filename,TailPath,value);
      break;
    }
    case 'g': /* Image geometry, canvas and offset  %Wx%H+%X+%Y */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20gx%.20g%+.20g%+.20g",
        (double) image->page.width,(double) image->page.height,
        (double) image->page.x,(double) image->page.y);
      break;
    }
    case 'h': /* Image height (current) */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        (image->rows != 0 ? image->rows : image->magick_rows));
      break;
    }
    case 'i': /* Filename last used for image (read or write) */
    {
      string=image->filename;
      break;
    }
    case 'k': /* Number of unique colors  */
    {
      /* FUTURE: ensure this does not generate the formated comment! */
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        GetNumberColors(image,(FILE *) NULL,&image->exception));
      break;
    }
    case 'l': /* Image label property - empty string by default */
    {
      string=GetImageProperty(image,"label");
      if (string == (const char *) NULL)
        string="";
      break;
    }
    case 'm': /* Image format (file magick) */
    {
      string=image->magick;
      break;
    }
    case 'n': /* Number of images in the list.  */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        GetImageListLength(image));
      break;
    }
    case 'o': /* Output Filename - for delegate use only */
    {
      string=image_info->filename;
      break;
    }
    case 'p': /* Image index in current image list -- As 'n' OBSOLETE */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        GetImageIndexInList(image));
      break;
    }
    case 'q': /* Quantum depth of image in memory */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        MAGICKCORE_QUANTUM_DEPTH);
      break;
    }
    case 'r': /* Image storage class and colorspace.  */
    {
      ColorspaceType
        colorspace;

      colorspace=image->colorspace;
      if (IsGrayImage(image,&image->exception) != MagickFalse)
        colorspace=GRAYColorspace;
      (void) FormatLocaleString(value,MaxTextExtent,"%s %s %s",
        CommandOptionToMnemonic(MagickClassOptions,(ssize_t) image->storage_class),
        CommandOptionToMnemonic(MagickColorspaceOptions,(ssize_t) colorspace),
        image->matte != MagickFalse ? "Matte" : "" );
      break;
    }
    case 's': /* Image scene number */
    {
      if (image_info->number_scenes != 0)
        (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
          image_info->scene);
      else
        (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
          image->scene);
      break;
    }
    case 't': /* Base filename without directory or extention */
    {
      GetPathComponent(image->magick_filename,BasePath,value);
      break;
    }
    case 'u': /* Unique filename */
    {
      string=image_info->unique;
      break;
    }
    case 'w': /* Image width (current) */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        (image->columns != 0 ? image->columns : image->magick_columns));
      break;
    }
    case 'x': /* Image horizontal resolution */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",
        fabs(image->x_resolution) > MagickEpsilon ? image->x_resolution : 72.0);
      break;
    }
    case 'y': /* Image vertical resolution */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",
        fabs(image->y_resolution) > MagickEpsilon ? image->y_resolution : 72.0);
      break;
    }
    case 'z': /* Image depth as read in */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        image->depth);
      break;
    }
    case 'A': /* Image alpha channel  */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%s",
         CommandOptionToMnemonic(MagickBooleanOptions,(ssize_t) image->matte));
      break;
    }
    case 'C': /* Image compression method.  */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%s",
        CommandOptionToMnemonic(MagickCompressOptions,(ssize_t)
          image->compression));
      break;
    }
    case 'D': /* Image dispose method.  */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%s",
        CommandOptionToMnemonic(MagickDisposeOptions,(ssize_t) image->dispose));
      break;
    }
    case 'G': /* Image size as geometry = "%wx%h" */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20gx%.20g",(double)
        image->magick_columns,(double) image->magick_rows);
      break;
    }
    case 'H': /* layer canvas height */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        image->page.height);
      break;
    }
    case 'M': /* Magick filename - filename given incl. coder & read mods */
    {
      string=image->magick_filename;
      break;
    }
    case 'O': /* layer canvas offset with sign = "+%X+%Y" */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%+ld%+ld",(long)
        image->page.x,(long) image->page.y);
      break;
    }
    case 'P': /* layer canvas page size = "%Wx%H" */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20gx%.20g",(double)
        image->page.width,(double) image->page.height);
      break;
    }
    case 'Q': /* image compression quality */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        (image->quality == 0 ? 92 : image->quality));
      break;
    }
    case 'S': /* Image scenes  ???? */
    {
      if (image_info->number_scenes == 0)
        string="2147483647";
      else
        (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
          image_info->scene+image_info->number_scenes);
      break;
    }
    case 'T': /* image time delay for animations */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        image->delay);
      break;
    }
    case 'U': /* Image resolution units. */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%s",
        CommandOptionToMnemonic(MagickResolutionOptions,(ssize_t)
          image->units));
      break;
    }
    case 'W': /* layer canvas width */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        image->page.width);
      break;
    }
    case 'X': /* layer canvas X offset */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%+.20g",(double)
        image->page.x);
      break;
    }
    case 'Y': /* layer canvas Y offset */
    {
      (void) FormatLocaleString(value,MaxTextExtent,"%+.20g",(double)
        image->page.y);
      break;
    }
    case 'Z': /* Zero filename ??? */
    {
      string=image_info->zero;
      break;
    }
    case '@': /* Trim bounding box, without Trimming! */
    {
      RectangleInfo
        page;

      page=GetImageBoundingBox(image,&image->exception);
      (void) FormatLocaleString(value,MaxTextExtent,"%.20gx%.20g%+.20g%+.20g",
        (double) page.width,(double) page.height,(double) page.x,(double)
        page.y);
      break;
    }
    case '#': /* Image signature */
    {
      (void) SignatureImage(image);
      string=GetImageProperty(image,"signature");
      break;
    }
    case '%': /* percent escaped */
    {
      string="%";
      break;
    }
  }
  if (*value != '\0')
    string=value;
  if (string != (char *) NULL)
    {
      (void) SetImageArtifact(image,"get-property",string);
      return(GetImageArtifact(image,"get-property"));
    }
  return((char *)NULL);
}

MagickExport const char *GetMagickProperty(const ImageInfo *image_info,
  Image *image,const char *property)
{
  char
    value[MaxTextExtent];

  const char
    *string;

  assert(property[0] != '\0');
  if (property[1] == '\0')  /* single letter property request */
    return(GetMagickPropertyLetter(image_info,image,*property));
  *value='\0';  /* formatted string */
  string=(char *) NULL;  /* constant string reference */
  switch (*property)
  {
    case 'b':
    {
      if ((LocaleCompare("base",property) == 0) ||
          (LocaleCompare("basename",property) == 0) )
        {
          GetPathComponent(image->magick_filename,BasePath,value);
          break;
        }
      if (LocaleCompare("bit-depth",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            GetImageDepth(image, &image->exception));
          break;
        }
      break;
    }
    case 'c':
    {
      if (LocaleCompare("channels",property) == 0)
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
      if (LocaleCompare("colorspace",property) == 0)
        {
          /*
            Image storage class and colorspace.
          */
          string=CommandOptionToMnemonic(MagickColorspaceOptions,(ssize_t)
            image->colorspace);
          break;
        }
      if (LocaleCompare("copyright",property) == 0)
        {
          (void) CopyMagickString(value,GetMagickCopyright(),MaxTextExtent);
          break;
        }
      break;
    }
    case 'd':
    {
      if (LocaleCompare("depth",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            image->depth);
          break;
        }
      if (LocaleCompare("directory",property) == 0)
        {
          GetPathComponent(image->magick_filename,HeadPath,value);
          break;
        }
      break;
    }
    case 'e':
    {
      if (LocaleCompare("extension",property) == 0)
        {
          GetPathComponent(image->magick_filename,ExtensionPath,value);
          break;
        }
      break;
    }
    case 'g':
    {
      if (LocaleCompare("gamma",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.*g",
            GetMagickPrecision(),image->gamma);
          break;
        }
      if ((image_info != (ImageInfo *) NULL) &&
          (LocaleCompare("group",property) == 0))
        {
          (void) FormatLocaleString(value,MaxTextExtent,"0x%lx",(unsigned long)
            image_info->group);
          break;
        }
      break;
    }
    case 'h':
    {
      if (LocaleCompare("height",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",
            image->magick_rows != 0 ? (double) image->magick_rows : 256.0);
          break;
        }
      break;
    }
    case 'i':
    {
      if (LocaleCompare("input",property) == 0)
        {
          string=image->filename;
          break;
        }
      break;
    }
    case 'k':
    {
      if (LocaleCompare("kurtosis",property) == 0)
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
      if (LocaleCompare("magick",property) == 0)
        {
          string=image->magick;
          break;
        }
      if ((LocaleCompare("max",property) == 0) ||
          (LocaleCompare("maxima",property) == 0))
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
      if (LocaleCompare("mean",property) == 0)
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
      if ((LocaleCompare("min",property) == 0) ||
          (LocaleCompare("minima",property) == 0))
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
    case 'o':
    {
      if (LocaleCompare("opaque",property) == 0)
        {
          MagickBooleanType
            opaque;

          opaque=IsOpaqueImage(image,&image->exception);
          (void) CopyMagickString(value,opaque != MagickFalse ? "true" :
            "false",MaxTextExtent);
          break;
        }
      if (LocaleCompare("orientation",property) == 0)
        {
          string=CommandOptionToMnemonic(MagickOrientationOptions,(ssize_t)
            image->orientation);
          break;
        }
      if ((image_info != (ImageInfo *) NULL) &&
          (LocaleCompare("output",property) == 0))
        {
          (void) CopyMagickString(value,image_info->filename,MaxTextExtent);
          break;
        }
     break;
    }
    case 'p':
    {
      if (LocaleCompare("page",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            GetImageIndexInList(image)+1);
          break;
        }
#if defined(MAGICKCORE_LCMS_DELEGATE)
      if (LocaleCompare("profile:icc",property) == 0 ||
          LocaleCompare("profile:icm",property) == 0)
        {
#if !defined(LCMS_VERSION) || (LCMS_VERSION < 2000)
#define cmsUInt32Number  DWORD
#endif

          const StringInfo
            *profile;

          cmsHPROFILE
            icc_profile;

          profile=GetImageProfile(image,property+8);
          if (profile == (StringInfo *) NULL)
            break;

          icc_profile=cmsOpenProfileFromMem(GetStringInfoDatum(profile),
            (cmsUInt32Number) GetStringInfoLength(profile));
          if (icc_profile != (cmsHPROFILE *) NULL)
            {
#if defined(LCMS_VERSION) && (LCMS_VERSION < 2000)
              string=cmsTakeProductName(icc_profile);
#else
              (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoDescription,
                "en","US",value,MaxTextExtent);
#endif
              (void) cmsCloseProfile(icc_profile);
            }
      }
#endif
      if (LocaleCompare("profiles",property) == 0)
        {
          const char
            *name;

          ResetImageProfileIterator(image);
          name=GetNextImageProfile(image);
          if (name != (char *) NULL)
            {
              (void) CopyMagickString(value,name,MaxTextExtent);
              name=GetNextImageProfile(image);
              while (name != (char *) NULL)
              {
                ConcatenateMagickString(value,",",MaxTextExtent);
                ConcatenateMagickString(value,name,MaxTextExtent);
                name=GetNextImageProfile(image);
              }
            }
          break;
        }
      break;
    }
    case 'r':
    {
      if (LocaleCompare("resolution.x",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%g",
            image->x_resolution);
          break;
        }
      if (LocaleCompare("resolution.y",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%g",
            image->y_resolution);
          break;
        }
      break;
    }
    case 's':
    {
      if (LocaleCompare("scene",property) == 0)
        {
          if ((image_info != (ImageInfo *) NULL) &&
              (image_info->number_scenes != 0))
            (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
              image_info->scene);
          else
            (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
              image->scene);
          break;
        }
      if (LocaleCompare("scenes",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            GetImageListLength(image));
          break;
        }
      if (LocaleCompare("size",property) == 0)
        {
          char
            format[MaxTextExtent];

          (void) FormatMagickSize(GetBlobSize(image),MagickFalse,format);
          (void) FormatLocaleString(value,MaxTextExtent,"%sB",format);
          break;
        }
      if (LocaleCompare("skewness",property) == 0)
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
      if ((LocaleCompare("standard-deviation",property) == 0) ||
          (LocaleCompare("standard_deviation",property) == 0))
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
    case 't':
    {
      if (LocaleCompare("type",property) == 0)
        {
          string=CommandOptionToMnemonic(MagickTypeOptions,(ssize_t)
            GetImageType(image,&image->exception));
          break;
        }
       break;
    }
    case 'u':
    {
      if ((image_info != (ImageInfo *) NULL) &&
          (LocaleCompare("unique",property) == 0))
        {
          string=image_info->unique;
          break;
        }
      if (LocaleCompare("units",property) == 0)
        {
          /*
            Image resolution units.
          */
          string=CommandOptionToMnemonic(MagickResolutionOptions,(ssize_t)
            image->units);
          break;
        }
      break;
    }
    case 'v':
    {
      if (LocaleCompare("version",property) == 0)
        {
          string=GetMagickVersion((size_t *) NULL);
          break;
        }
      break;
    }
    case 'w':
    {
      if (LocaleCompare("width",property) == 0)
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
            (image->magick_columns != 0 ? image->magick_columns : 256));
          break;
        }
      break;
    }
    case 'x': /* FUTURE: Obsolete X resolution */
    {
      if ((LocaleCompare("xresolution",property) == 0) ||
          (LocaleCompare("x-resolution",property) == 0) )
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",
            image->x_resolution);
          break;
        }
      break;
    }
    case 'y': /* FUTURE: Obsolete Y resolution */
    {
      if ((LocaleCompare("yresolution",property) == 0) ||
          (LocaleCompare("y-resolution",property) == 0) )
        {
          (void) FormatLocaleString(value,MaxTextExtent,"%.20g",
           image->y_resolution);
          break;
        }
      break;
    }
    case 'z':
    {
      if ((image_info != (ImageInfo *) NULL) &&
          (LocaleCompare("zero",property) == 0))
        {
          string=image_info->zero;
          break;
        }
      break;
    }
  }
  if (*value != '\0')
    string=value;
  if (string != (char *) NULL)
    {
      (void) SetImageArtifact(image,"get-property", string);
      return(GetImageArtifact(image,"get-property"));
    }
  return((char *)NULL);
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
%  GetNextImageProperty() gets the next free-form string property name.
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
%  the appropriate image property and returns the interpreted text.
%
%  This searches for and replaces
%     \n \r \%          replaced by newline, return, and percent resp.
%     &lt; &gt; &amp;   replaced by '<', '>', '&' resp.
%     %%                replaced by percent
%
%     %x %[x]       where 'x' is a single letter properity, case sensitive).
%     %[type:name]  where 'type' a is special and known prefix.
%     %[name]       where 'name' is a specifically known attribute, calculated
%                   value, or a per-image property string name, or a per-image
%                   'artifact' (as generated from a global option).
%                   It may contain ':' as long as the prefix is not special.
%
%  Single letter % substitutions will only happen if the character before the
%  percent is NOT a number. But braced substitutions will always be performed.
%  This prevents the typical usage of percent in a interpreted geometry
%  argument from being substituted when the percent is a geometry flag.
%
%  If 'glob-expresions' ('*' or '?' characters) is used for 'name' it may be
%  used as a search pattern to print multiple lines of "name=value\n" pairs of
%  the associacted set of properities.
%
%  The returned string must be freed using DestoryString() by the caller.
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

/* common inline code to expand the interpreted text string */
#define ExtendInterpretText(string_length)  do { \
DisableMSCWarning(4127) \
    size_t length=(string_length); \
    if ((size_t) (q-interpret_text+length+1) >= extent) \
     { extent+=length; \
       interpret_text=(char *) ResizeQuantumMemory(interpret_text, \
             extent+MaxTextExtent,sizeof(*interpret_text)); \
       if (interpret_text == (char *) NULL) \
         return((char *)NULL); \
       q=interpret_text+strlen(interpret_text); \
   } } while (0)  /* no trailing ; */ \
RestoreMSCWarning

/* same but append the given string */
#define AppendString2Text(string)  do { \
DisableMSCWarning(4127) \
    size_t length=strlen((string)); \
    if ((size_t) (q-interpret_text+length+1) >= extent) \
     { extent+=length; \
       interpret_text=(char *) ResizeQuantumMemory(interpret_text, \
             extent+MaxTextExtent,sizeof(*interpret_text)); \
       if (interpret_text == (char *) NULL) \
         return((char *)NULL); \
       q=interpret_text+strlen(interpret_text); \
      } \
     (void) CopyMagickString(q,(string),extent); \
     q+=length; \
   } while (0)  /* no trailing ; */ \
RestoreMSCWarning

/* same but append a 'key' and 'value' pair */
#define AppendKeyValue2Text(key,value)  do { \
DisableMSCWarning(4127) \
    size_t length=strlen(key)+strlen(value)+2; \
    if ((size_t) (q-interpret_text+length+1) >= extent) \
     { extent+=length; \
      interpret_text=(char *) ResizeQuantumMemory(interpret_text, \
              extent+MaxTextExtent,sizeof(*interpret_text)); \
      if (interpret_text == (char *) NULL) \
        return((char *)NULL); \
      q=interpret_text+strlen(interpret_text); \
     } \
     q+=FormatLocaleString(q,extent,"%s=%s\n",(key),(value)); \
   } while (0)  /* no trailing ; */ \
RestoreMSCWarning

MagickExport char *InterpretImageProperties(const ImageInfo *image_info,
  Image *image,const char *embed_text)
{
  char
    *interpret_text;

  register char
    *q;     /* current position in interpret_text */

  register const char
    *p;     /* position in embed_text string being expanded */

  size_t
    extent; /* allocated length of interpret_text */

  MagickBooleanType
    number;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  if (embed_text == (const char *) NULL)
    return((char *) NULL);
  p=embed_text;

  if (*p == '\0')
    return(ConstantString(""));

  /* handle a '@' replace string from file */
  if (*p == '@') {
     p++;
     if (*p != '-' && (IsPathAccessible(p) == MagickFalse) ) {
       (void) ThrowMagickException(&image->exception,GetMagickModule(),
           OptionError,"UnableToAccessPath","%s",p);
       return((char *) NULL);
     }
     return(FileToString(p,~0UL,&image->exception));
  }

  /*
    Translate any embedded format characters.
  */
  interpret_text=AcquireString(embed_text); /* new string with extra space */
  extent=MaxTextExtent;                     /* how many extra space */
  number=MagickFalse;                       /* is last char a number? */
  for (q=interpret_text; *p!='\0';
          number=(isdigit((int) ((unsigned char) *p))) ? MagickTrue : MagickFalse,p++)
  {
    *q='\0';
    ExtendInterpretText(MaxTextExtent);
    /*
      Look for the various escapes, (and handle other specials)
    */
    switch (*p) {
      case '\\':
        switch (*(p+1)) {
          case '\0':
            continue;
          case 'r':       /* convert to RETURN */
            *q++='\r';
            p++;
            continue;
          case 'n':       /* convert to NEWLINE */
            *q++='\n';
            p++;
            continue;
          case '\n':      /* EOL removal UNIX,MacOSX */
            p++;
            continue;
          case '\r':      /* EOL removal DOS,Windows */
            p++;
            if (*p == '\n') /* return-newline EOL */
              p++;
            continue;
          default:
            p++;
            *q++=(*p);
            continue;
        }
        continue; /* never reached! */
      case '&':
        if (LocaleNCompare("&lt;",p,4) == 0)
          *q++='<', p+=3;
        else if (LocaleNCompare("&gt;",p,4) == 0)
          *q++='>', p+=3;
        else if (LocaleNCompare("&amp;",p,5) == 0)
          *q++='&', p+=4;
        else
          *q++=(*p);
        continue;
      case '%':
        break;      /* continue to next set of handlers */
      default:
        *q++=(*p);  /* any thing else is 'as normal' */
        continue;
    }
    p++; /* advance beyond the percent */

    /*
      Doubled Percent - or percent at end of string
    */
    if ( *p == '\0' )
       p--;
    if ( *p == '%' ) {
        *q++='%';
        continue;
      }

    /*
      Single letter escapes  %c
    */
    if ( *p != '[' ) {
      const char
        *value;

      /* But only if not preceeded by a number! */
      if ( number != MagickFalse ) {
        *q++='%'; /* do NOT substitute the percent */
        p--;      /* back up one */
        continue;
      }
      value=GetMagickPropertyLetter(image_info,image,*p);
      if (value != (char *) NULL) {
        AppendString2Text(value);
        continue;
      }
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        OptionWarning,"UnknownImageProperty","\"%%%c\"",*p);
      continue;
    }

    /*
      Braced Percent Escape  %[...]
    */
    {
      char
        pattern[MaxTextExtent];

      const char
        *key,
        *value;

      register ssize_t
        len;

      ssize_t
        depth;

      /* get the string framed by the %[...] */
      p++;  /* advance p to just inside the opening brace */
      depth=1;
      if ( *p == ']' ) {
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
            OptionWarning,"UnknownImageProperty","\"%%[]\"");
        break;
      }
      for (len=0; len<(MaxTextExtent-1L) && (*p != '\0');)
      {
        /* skip escaped braces within braced pattern */
        if ( (*p == '\\') && (*(p+1) != '\0') ) {
          pattern[len++]=(*p++);
          pattern[len++]=(*p++);
          continue;
        }
        if (*p == '[')
          depth++;
        if (*p == ']')
          depth--;
        if (depth <= 0)
          break;
        pattern[len++]=(*p++);
      }
      pattern[len]='\0';
      /* Check for unmatched final ']' for "%[...]" */
      if ( depth != 0 ) {
        if (len >= 64) {  /* truncate string for error message */
          pattern[61] = '.';
          pattern[62] = '.';
          pattern[63] = '.';
          pattern[64] = '\0';
        }
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
            OptionError,"UnbalancedBraces","\"%%[%s\"",pattern);
        interpret_text=DestroyString(interpret_text);
        return((char *)NULL);
      }

      /*
        Special Lookup Prefixes %[prefix:...]
      */
      /* option - direct global option lookup (with globbing) */
      if (LocaleNCompare("option:",pattern,7) == 0)
      {
        if (image_info == (ImageInfo *) NULL)
          continue; /* no global options available */
        if (IsGlob(pattern+7) != MagickFalse)
        {
          ResetImageOptionIterator(image_info);
          while ((key=GetNextImageOption(image_info)) != (const char *) NULL)
            if( GlobExpression(key,pattern+7,MagickTrue) != MagickFalse)
              {
                value=GetImageOption(image_info,key);
                if (value != (const char *) NULL)
                  AppendKeyValue2Text(key,value);
                /* else - assertion failure? key but no value! */
              }
          continue;
        }
        value=GetImageOption(image_info,pattern+7);
        if (value != (char *) NULL)
          AppendString2Text(value);
        /* else - no global option of this specifc name */
        continue;
      }
      /* artifact - direct image artifact lookup (with glob) */
      if (LocaleNCompare("artifact:",pattern,9) == 0)
      {
        if (image == (Image *) NULL)
          continue; /* else no image to retrieve artifact */
        if (IsGlob(pattern+9) != MagickFalse)
        {
          ResetImageArtifactIterator(image);
          while ((key=GetNextImageArtifact(image)) != (const char *) NULL)
            if( GlobExpression(key,pattern+9,MagickTrue) != MagickFalse)
              {
                value=GetImageArtifact(image,key);
                if (value != (const char *) NULL)
                  AppendKeyValue2Text(key,value);
                /* else - assertion failure? key but no value! */
              }
          continue;
        }
        value=GetImageArtifact(image,pattern+9);
        if (value != (char *) NULL)
          AppendString2Text(value);
        /* else - no artifact of this specifc name */
        continue;
      }
      /* FUTURE: handle %[property:...] prefix - abort other lookups */

      /* handle special image properties */
      /* For example:  %[exif:...] %[fx:...] %[pixel:...] */
      value=GetImageProperty(image,pattern);
      if (value != (const char *) NULL)
        {
          AppendString2Text(value);
          continue;
        }
      /*
        Handle property 'glob' patterns
        Such as:  %[*]   %[user:array_??]  %[filename:e*]
      */
      if (IsGlob(pattern) != MagickFalse)
        {
          ResetImagePropertyIterator(image);
          while ((key=GetNextImageProperty(image)) != (const char *) NULL)
            if (GlobExpression(key,pattern,MagickTrue) != MagickFalse)
              {
                value=GetImageProperty(image,key);
                if (value != (const char *) NULL)
                  AppendKeyValue2Text(key,value);
                /* else - assertion failure? */
              }
          continue;
        }
      /*
        Look for a known property or image attribute
        Such as  %[basename]  %[denisty]  %[delay]
        Also handles a braced single letter:  %[b] %[G] %[g]
      */
      value=GetMagickProperty(image_info,image,pattern);
      if (value != (const char *) NULL)
        {
          AppendString2Text(value);
          continue;
        }
      /*
        Look for a per-image Artifact (user option, post-interpreted)
      */
      value=GetImageArtifact(image,pattern);
      if (value != (char *) NULL)
        {
          AppendString2Text(value);
          continue;
        }
      /*
        Look for user option of this name (should never match in CLI usage)
      */
      if (image_info != (ImageInfo *) NULL) {
        value=GetImageOption(image_info,pattern);
        if (value != (char *) NULL)
          {
            AppendString2Text(value);
            continue;
          }
        }
      /*
        Failed to find any match anywhere!
      */
      if (len >= 64) {  /* truncate string for error message */
        pattern[61] = '.';
        pattern[62] = '.';
        pattern[63] = '.';
        pattern[64] = '\0';
      }
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
          OptionWarning,"UnknownImageProperty","\"%%[%s]\"",pattern);
      /* continue */
    } /* Braced Percent Escape */

  } /* for each char in 'embed_text' */
  *q='\0';
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
%  In this case the ConstantString() value returned should be freed by the
%  caller when finished.
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
%  SetImageProperty() saves the given string value either to specific known
%  attribute or to a freeform property string.
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

  /* Create splay-tree */
  if (image->properties == (void *) NULL)
    image->properties=NewSplayTree(CompareSplayTreeString,
      RelinquishMagickMemory,RelinquishMagickMemory);

  /* Delete property if NULL --  empty string values are valid! */
  if (value == (const char *) NULL)
    return(DeleteImageProperty(image,property));

  /* FUTURE: These should produce 'illegal settings'
     + binary chars in p[roperty key
     + first letter must be a alphabetic
     + single letter property keys (read only)
     + known special prefix (read only, they don't get saved!)
  */

  status=MagickTrue;
  exception=(&image->exception);
  switch (*property)
  {
    case 'B':
    case 'b':
    {
      if (LocaleCompare("background",property) == 0)
        {
          (void) QueryColorDatabase(value,&image->background_color,exception);
          break;
        }
      if (LocaleCompare("bias",property) == 0)
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
      if (LocaleCompare("colorspace",property) == 0)
        {
          ssize_t
            colorspace;

          colorspace=ParseCommandOption(MagickColorspaceOptions,MagickFalse,
            value);
          if (colorspace < 0)
            break;
          status=SetImageColorspace(image,(ColorspaceType) colorspace);
          break;
        }
      if (LocaleCompare("compose",property) == 0)
        {
          ssize_t
            compose;

          compose=ParseCommandOption(MagickComposeOptions,MagickFalse,value);
          if (compose < 0)
            break;
          image->compose=(CompositeOperator) compose;
          break;
        }
      if (LocaleCompare("compress",property) == 0)
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
      if (LocaleCompare("delay",property) == 0)
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
      if (LocaleCompare("density",property) == 0)
        {
          GeometryInfo
            geometry_info;

          flags=ParseGeometry(value,&geometry_info);
          image->x_resolution=geometry_info.rho;
          image->y_resolution=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            image->y_resolution=image->x_resolution;
        }
      if (LocaleCompare("depth",property) == 0)
        {
          image->depth=StringToUnsignedLong(value);
          break;
        }
      if (LocaleCompare("dispose",property) == 0)
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
      if (LocaleCompare("gamma",property) == 0)
        {
          image->gamma=StringToDouble(value,(char **) NULL);
          break;
        }
      if (LocaleCompare("gravity",property) == 0)
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
      if (LocaleCompare("intensity",property) == 0)
        {
          ssize_t
            intensity;

          intensity=ParseCommandOption(MagickPixelIntensityOptions,MagickFalse,
            value);
          if (intensity < 0)
            break;
          image->intensity=(PixelIntensityMethod) intensity;
          break;
        }
      if (LocaleCompare("intent",property) == 0)
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
      if (LocaleCompare("interpolate",property) == 0)
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
      if (LocaleCompare("loop",property) == 0)
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
      if (LocaleCompare("page",property) == 0)
        {
          char
            *geometry;

          geometry=GetPageGeometry(value);
          flags=ParseAbsoluteGeometry(geometry,&image->page);
          geometry=DestroyString(geometry);
          break;
        }
      if (LocaleCompare("profile",property) == 0)
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
      if (LocaleCompare("rendering-intent",property) == 0)
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
      if (LocaleCompare("tile-offset",property) == 0)
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
      if (LocaleCompare("units",property) == 0)
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        JJJJJ  PPPP   EEEEE   GGGG                           %
%                          J    P   P  E      G                               %
%                          J    PPPP   EEE    G  GG                           %
%                        J J    P      E      G   G                           %
%                        JJJ    P      EEEEE   GGG                            %
%                                                                             %
%                                                                             %
%                       Read/Write JPEG Image Format                          %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
% This software is based in part on the work of the Independent JPEG Group.
% See ftp://ftp.uu.net/graphics/jpeg/jpegsrc.v6b.tar.gz for copyright and
% licensing restrictions.  Blob support contributed by Glenn Randers-Pehrson.
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/colormap-private.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/splay-tree.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"
#include <setjmp.h>
#if defined(MAGICKCORE_JPEG_DELEGATE)
#define JPEG_INTERNAL_OPTIONS
#if defined(__MINGW32__) || defined(__MINGW64__)
# define XMD_H 1  /* Avoid conflicting typedef for INT32 */
#endif
#undef HAVE_STDLIB_H
#include "jpeglib.h"
#include "jerror.h"
#endif

/*
  Define declarations.
*/
#define ICC_MARKER  (JPEG_APP0+2)
#define ICC_PROFILE  "ICC_PROFILE"
#define IPTC_MARKER  (JPEG_APP0+13)
#define XML_MARKER  (JPEG_APP0+1)
#define MaxBufferExtent  16384

/*
  Typedef declarations.
*/
#if defined(MAGICKCORE_JPEG_DELEGATE)
typedef struct _DestinationManager
{
  struct jpeg_destination_mgr
    manager;

  Image
    *image;

  JOCTET
    *buffer;
} DestinationManager;

typedef struct _ErrorManager
{
  Image
    *image;

  MagickBooleanType
    finished;

  StringInfo
    *profile;

  jmp_buf
    error_recovery;
} ErrorManager;

typedef struct _SourceManager
{
  struct jpeg_source_mgr
    manager;

  Image
    *image;

  JOCTET
    *buffer;

  boolean
    start_of_blob;
} SourceManager;
#endif

typedef struct _QuantizationTable
{
  char
    *slot,
    *description;

  size_t
    width,
    height;

  double
    divisor;

  unsigned int
    *levels;
} QuantizationTable;

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_JPEG_DELEGATE)
static MagickBooleanType
  WriteJPEGImage(const ImageInfo *,Image *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J P E G                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJPEG() returns MagickTrue if the image format type, identified by the
%  magick string, is JPEG.
%
%  The format of the IsJPEG  method is:
%
%      MagickBooleanType IsJPEG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsJPEG(const unsigned char *magick,const size_t length)
{
  if (length < 3)
    return(MagickFalse);
  if (memcmp(magick,"\377\330\377",3) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_JPEG_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J P E G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJPEGImage() reads a JPEG image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadJPEGImage method is:
%
%      Image *ReadJPEGImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static boolean FillInputBuffer(j_decompress_ptr cinfo)
{
  SourceManager
    *source;

  source=(SourceManager *) cinfo->src;
  source->manager.bytes_in_buffer=(size_t) ReadBlob(source->image,
    MaxBufferExtent,source->buffer);
  if (source->manager.bytes_in_buffer == 0)
    {
      if (source->start_of_blob != FALSE)
        ERREXIT(cinfo,JERR_INPUT_EMPTY);
      WARNMS(cinfo,JWRN_JPEG_EOF);
      source->buffer[0]=(JOCTET) 0xff;
      source->buffer[1]=(JOCTET) JPEG_EOI;
      source->manager.bytes_in_buffer=2;
    }
  source->manager.next_input_byte=source->buffer;
  source->start_of_blob=FALSE;
  return(TRUE);
}

static int GetCharacter(j_decompress_ptr jpeg_info)
{
  if (jpeg_info->src->bytes_in_buffer == 0)
    (void) (*jpeg_info->src->fill_input_buffer)(jpeg_info);
  jpeg_info->src->bytes_in_buffer--;
  return((int) GETJOCTET(*jpeg_info->src->next_input_byte++));
}

static void InitializeSource(j_decompress_ptr cinfo)
{
  SourceManager
    *source;

  source=(SourceManager *) cinfo->src;
  source->start_of_blob=TRUE;
}

static MagickBooleanType IsITUFaxImage(const Image *image)
{
  const StringInfo
    *profile;

  const unsigned char
    *datum;

  profile=GetImageProfile(image,"8bim");
  if (profile == (const StringInfo *) NULL)
    return(MagickFalse);
  if (GetStringInfoLength(profile) < 5)
    return(MagickFalse);
  datum=GetStringInfoDatum(profile);
  if ((datum[0] == 0x47) && (datum[1] == 0x33) && (datum[2] == 0x46) &&
      (datum[3] == 0x41) && (datum[4] == 0x58))
    return(MagickTrue);
  return(MagickFalse);
}

static void JPEGErrorHandler(j_common_ptr jpeg_info)
{
  char
    message[JMSG_LENGTH_MAX];

  ErrorManager
    *error_manager;

  Image
    *image;

  *message='\0';
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  (jpeg_info->err->format_message)(jpeg_info,message);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "[%s] JPEG Trace: \"%s\"",image->filename,message);
  if (error_manager->finished != MagickFalse)
    (void) ThrowMagickException(&image->exception,GetMagickModule(),
      CorruptImageWarning,(char *) message,"`%s'",image->filename);
  else
    (void) ThrowMagickException(&image->exception,GetMagickModule(),
      CorruptImageError,(char *) message,"`%s'",image->filename);
  longjmp(error_manager->error_recovery,1);
}

static MagickBooleanType JPEGWarningHandler(j_common_ptr jpeg_info,int level)
{
#define JPEGExcessiveWarnings  1000

  char
    message[JMSG_LENGTH_MAX];

  ErrorManager
    *error_manager;

  Image
    *image;

  *message='\0';
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  if (level < 0)
    {
      /*
        Process warning message.
      */
      (jpeg_info->err->format_message)(jpeg_info,message);
      if (jpeg_info->err->num_warnings++ > JPEGExcessiveWarnings)
        JPEGErrorHandler(jpeg_info);
      ThrowBinaryException(CorruptImageWarning,(char *) message,
        image->filename);
    }
  else
    if ((image->debug != MagickFalse) &&
        (level >= jpeg_info->err->trace_level))
      {
        /*
          Process trace message.
        */
        (jpeg_info->err->format_message)(jpeg_info,message);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "[%s] JPEG Trace: \"%s\"",image->filename,message);
      }
  return(MagickTrue);
}

static boolean ReadComment(j_decompress_ptr jpeg_info)
{
  ErrorManager
    *error_manager;

  Image
    *image;

  register unsigned char
    *p;

  register ssize_t
    i;

  size_t
    length;

  StringInfo
    *comment;

  /*
    Determine length of comment.
  */
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  length=(size_t) ((size_t) GetCharacter(jpeg_info) << 8);
  length+=GetCharacter(jpeg_info);
  length-=2;
  if (length <= 0)
    return(MagickTrue);
  comment=BlobToStringInfo((const void *) NULL,length);
  if (comment == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Read comment.
  */
  error_manager->profile=comment;
  p=GetStringInfoDatum(comment);
  for (i=0; i < (ssize_t) GetStringInfoLength(comment); i++)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  *p='\0';
  error_manager->profile=NULL;
  p=GetStringInfoDatum(comment);
  (void) SetImageProperty(image,"comment",(const char *) p);
  comment=DestroyStringInfo(comment);
  return(MagickTrue);
}

static boolean ReadICCProfile(j_decompress_ptr jpeg_info)
{
  char
    magick[12];

  ErrorManager
    *error_manager;

  Image
    *image;

  MagickBooleanType
    status;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    length;

  StringInfo
    *icc_profile,
    *profile;

  /*
    Read color profile.
  */
  length=(size_t) ((size_t) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  length-=2;
  if (length <= 14)
    {
      while (length-- > 0)
        (void) GetCharacter(jpeg_info);
      return(MagickTrue);
    }
  for (i=0; i < 12; i++)
    magick[i]=(char) GetCharacter(jpeg_info);
  if (LocaleCompare(magick,ICC_PROFILE) != 0)
    {
      /*
        Not a ICC profile, return.
      */
      for (i=0; i < (ssize_t) (length-12); i++)
        (void) GetCharacter(jpeg_info);
      return(MagickTrue);
    }
  (void) GetCharacter(jpeg_info);  /* id */
  (void) GetCharacter(jpeg_info);  /* markers */
  length-=14;
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  profile=BlobToStringInfo((const void *) NULL,length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  error_manager->profile=profile;
  p=GetStringInfoDatum(profile);
  for (i=(ssize_t) GetStringInfoLength(profile)-1; i >= 0; i--)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  error_manager->profile=NULL;
  icc_profile=(StringInfo *) GetImageProfile(image,"icc");
  if (icc_profile != (StringInfo *) NULL)
    {
      ConcatenateStringInfo(icc_profile,profile);
      profile=DestroyStringInfo(profile);
    }
  else
    {
      status=SetImageProfile(image,"icc",profile);
      profile=DestroyStringInfo(profile);
      if (status == MagickFalse)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: ICC, %.20g bytes",(double) length);
  return(MagickTrue);
}

static boolean ReadIPTCProfile(j_decompress_ptr jpeg_info)
{
  char
    magick[MaxTextExtent];

  ErrorManager
    *error_manager;

  Image
    *image;

  MagickBooleanType
    status;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    length;

  StringInfo
    *iptc_profile,
    *profile;

  /*
    Determine length of binary data stored here.
  */
  length=(size_t) ((size_t) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  length-=2;
  if (length <= 14)
    {
      while (length-- > 0)
        (void) GetCharacter(jpeg_info);
      return(MagickTrue);
    }
  /*
    Validate that this was written as a Photoshop resource format slug.
  */
  for (i=0; i < 10; i++)
    magick[i]=(char) GetCharacter(jpeg_info);
  magick[10]='\0';
  if (length <= 10)
    return(MagickTrue);
  length-=10;
  if (LocaleCompare(magick,"Photoshop ") != 0)
    {
      /*
        Not a IPTC profile, return.
      */
      for (i=0; i < (ssize_t) length; i++)
        (void) GetCharacter(jpeg_info);
      return(MagickTrue);
    }
  /*
    Remove the version number.
  */
  for (i=0; i < 4; i++)
    (void) GetCharacter(jpeg_info);
  if (length <= 4)
    return(MagickTrue);
  length-=4;
  if (length == 0)
    return(MagickTrue);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  profile=BlobToStringInfo((const void *) NULL,length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  error_manager->profile=profile;
  p=GetStringInfoDatum(profile);
  for (i=0;  i < (ssize_t) GetStringInfoLength(profile); i++)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  error_manager->profile=NULL;
  iptc_profile=(StringInfo *) GetImageProfile(image,"8bim");
  if (iptc_profile != (StringInfo *) NULL)
    {
      ConcatenateStringInfo(iptc_profile,profile);
      profile=DestroyStringInfo(profile);
    }
  else
    {
      status=SetImageProfile(image,"8bim",profile);
      profile=DestroyStringInfo(profile);
      if (status == MagickFalse)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: iptc, %.20g bytes",(double) length);
  return(MagickTrue);
}

static boolean ReadProfile(j_decompress_ptr jpeg_info)
{
  char
    name[MaxTextExtent];

  const StringInfo
    *previous_profile;

  ErrorManager
    *error_manager;

  Image
    *image;

  int
    marker;

  MagickBooleanType
    status;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    length;

  StringInfo
    *profile;

  /*
    Read generic profile.
  */
  length=(size_t) ((size_t) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  if (length <= 2)
    return(MagickTrue);
  length-=2;
  marker=jpeg_info->unread_marker-JPEG_APP0;
  (void) FormatLocaleString(name,MaxTextExtent,"APP%d",marker);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  profile=BlobToStringInfo((const void *) NULL,length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  error_manager->profile=profile;
  p=GetStringInfoDatum(profile);
  for (i=0; i < (ssize_t) GetStringInfoLength(profile); i++)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  error_manager->profile=NULL;
  if (marker == 1)
    {
      p=GetStringInfoDatum(profile);
      if ((length > 4) && (LocaleNCompare((char *) p,"exif",4) == 0))
        (void) CopyMagickString(name,"exif",MaxTextExtent);
      if ((length > 5) && (LocaleNCompare((char *) p,"http:",5) == 0))
        {
          ssize_t
            j;

          /*
            Extract namespace from XMP profile.
          */
          p=GetStringInfoDatum(profile);
          for (j=0; j < (ssize_t) GetStringInfoLength(profile); j++)
          {
            if (*p == '\0')
              break;
            p++;
          }
          if (j < (ssize_t) GetStringInfoLength(profile))
            (void) DestroyStringInfo(SplitStringInfo(profile,(size_t) (j+1)));
          (void) CopyMagickString(name,"xmp",MaxTextExtent);
        }
    }
  previous_profile=GetImageProfile(image,name);
  if (previous_profile != (const StringInfo *) NULL)
    {
      size_t
        length;

      length=GetStringInfoLength(profile);
      SetStringInfoLength(profile,GetStringInfoLength(profile)+
        GetStringInfoLength(previous_profile));
      (void) memmove(GetStringInfoDatum(profile)+
        GetStringInfoLength(previous_profile),GetStringInfoDatum(profile),
        length);
      (void) memcpy(GetStringInfoDatum(profile),
        GetStringInfoDatum(previous_profile),
        GetStringInfoLength(previous_profile));
    }
  status=SetImageProfile(image,name,profile);
  profile=DestroyStringInfo(profile);
  if (status == MagickFalse)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: %s, %.20g bytes",name,(double) length);
  return(MagickTrue);
}

static void SkipInputData(j_decompress_ptr cinfo,long number_bytes)
{
  SourceManager
    *source;

  if (number_bytes <= 0)
    return;
  source=(SourceManager *) cinfo->src;
  while (number_bytes > (long) source->manager.bytes_in_buffer)
  {
    number_bytes-=(long) source->manager.bytes_in_buffer;
    (void) FillInputBuffer(cinfo);
  }
  source->manager.next_input_byte+=number_bytes;
  source->manager.bytes_in_buffer-=number_bytes;
}

static void TerminateSource(j_decompress_ptr cinfo)
{
  (void) cinfo;
}

static void JPEGSourceManager(j_decompress_ptr cinfo,Image *image)
{
  SourceManager
    *source;

  cinfo->src=(struct jpeg_source_mgr *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,sizeof(SourceManager));
  source=(SourceManager *) cinfo->src;
  source->buffer=(JOCTET *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,MaxBufferExtent*sizeof(JOCTET));
  source=(SourceManager *) cinfo->src;
  source->manager.init_source=InitializeSource;
  source->manager.fill_input_buffer=FillInputBuffer;
  source->manager.skip_input_data=SkipInputData;
  source->manager.resync_to_restart=jpeg_resync_to_restart;
  source->manager.term_source=TerminateSource;
  source->manager.bytes_in_buffer=0;
  source->manager.next_input_byte=NULL;
  source->image=image;
}

static void JPEGSetImageQuality(struct jpeg_decompress_struct *jpeg_info,
  Image *image)
{
  image->quality=UndefinedCompressionQuality;
#if defined(D_PROGRESSIVE_SUPPORTED)
  if (image->compression == LosslessJPEGCompression)
    {
      image->quality=100;
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "Quality: 100 (lossless)");
    }
  else
#endif
  {
    ssize_t
      j,
      qvalue,
      sum;

    register ssize_t
      i;

    /*
      Determine the JPEG compression quality from the quantization tables.
    */
    sum=0;
    for (i=0; i < NUM_QUANT_TBLS; i++)
    {
      if (jpeg_info->quant_tbl_ptrs[i] != NULL)
        for (j=0; j < DCTSIZE2; j++)
          sum+=jpeg_info->quant_tbl_ptrs[i]->quantval[j];
    }
    if ((jpeg_info->quant_tbl_ptrs[0] != NULL) &&
        (jpeg_info->quant_tbl_ptrs[1] != NULL))
      {
        ssize_t
          hash[101] =
          {
            1020, 1015,  932,  848,  780,  735,  702,  679,  660,  645,
             632,  623,  613,  607,  600,  594,  589,  585,  581,  571,
             555,  542,  529,  514,  494,  474,  457,  439,  424,  410,
             397,  386,  373,  364,  351,  341,  334,  324,  317,  309,
             299,  294,  287,  279,  274,  267,  262,  257,  251,  247,
             243,  237,  232,  227,  222,  217,  213,  207,  202,  198,
             192,  188,  183,  177,  173,  168,  163,  157,  153,  148,
             143,  139,  132,  128,  125,  119,  115,  108,  104,   99,
              94,   90,   84,   79,   74,   70,   64,   59,   55,   49,
              45,   40,   34,   30,   25,   20,   15,   11,    6,    4,
               0
          },
          sums[101] =
          {
            32640, 32635, 32266, 31495, 30665, 29804, 29146, 28599, 28104,
            27670, 27225, 26725, 26210, 25716, 25240, 24789, 24373, 23946,
            23572, 22846, 21801, 20842, 19949, 19121, 18386, 17651, 16998,
            16349, 15800, 15247, 14783, 14321, 13859, 13535, 13081, 12702,
            12423, 12056, 11779, 11513, 11135, 10955, 10676, 10392, 10208,
             9928,  9747,  9564,  9369,  9193,  9017,  8822,  8639,  8458,
             8270,  8084,  7896,  7710,  7527,  7347,  7156,  6977,  6788,
             6607,  6422,  6236,  6054,  5867,  5684,  5495,  5305,  5128,
             4945,  4751,  4638,  4442,  4248,  4065,  3888,  3698,  3509,
             3326,  3139,  2957,  2775,  2586,  2405,  2216,  2037,  1846,
             1666,  1483,  1297,  1109,   927,   735,   554,   375,   201,
              128,     0
          };

        qvalue=(ssize_t) (jpeg_info->quant_tbl_ptrs[0]->quantval[2]+
          jpeg_info->quant_tbl_ptrs[0]->quantval[53]+
          jpeg_info->quant_tbl_ptrs[1]->quantval[0]+
          jpeg_info->quant_tbl_ptrs[1]->quantval[DCTSIZE2-1]);
        for (i=0; i < 100; i++)
        {
          if ((qvalue < hash[i]) && (sum < sums[i]))
            continue;
          if (((qvalue <= hash[i]) && (sum <= sums[i])) || (i >= 50))
            image->quality=(size_t) i+1;
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "Quality: %.20g (%s)",(double) i+1,(qvalue <= hash[i]) &&
              (sum <= sums[i]) ? "exact" : "approximate");
          break;
        }
      }
    else
      if (jpeg_info->quant_tbl_ptrs[0] != NULL)
        {
          ssize_t
            hash[101] =
            {
              510,  505,  422,  380,  355,  338,  326,  318,  311,  305,
              300,  297,  293,  291,  288,  286,  284,  283,  281,  280,
              279,  278,  277,  273,  262,  251,  243,  233,  225,  218,
              211,  205,  198,  193,  186,  181,  177,  172,  168,  164,
              158,  156,  152,  148,  145,  142,  139,  136,  133,  131,
              129,  126,  123,  120,  118,  115,  113,  110,  107,  105,
              102,  100,   97,   94,   92,   89,   87,   83,   81,   79,
               76,   74,   70,   68,   66,   63,   61,   57,   55,   52,
               50,   48,   44,   42,   39,   37,   34,   31,   29,   26,
               24,   21,   18,   16,   13,   11,    8,    6,    3,    2,
                0
            },
            sums[101] =
            {
              16320, 16315, 15946, 15277, 14655, 14073, 13623, 13230, 12859,
              12560, 12240, 11861, 11456, 11081, 10714, 10360, 10027,  9679,
               9368,  9056,  8680,  8331,  7995,  7668,  7376,  7084,  6823,
               6562,  6345,  6125,  5939,  5756,  5571,  5421,  5240,  5086,
               4976,  4829,  4719,  4616,  4463,  4393,  4280,  4166,  4092,
               3980,  3909,  3835,  3755,  3688,  3621,  3541,  3467,  3396,
               3323,  3247,  3170,  3096,  3021,  2952,  2874,  2804,  2727,
               2657,  2583,  2509,  2437,  2362,  2290,  2211,  2136,  2068,
               1996,  1915,  1858,  1773,  1692,  1620,  1552,  1477,  1398,
               1326,  1251,  1179,  1109,  1031,   961,   884,   814,   736,
                667,   592,   518,   441,   369,   292,   221,   151,    86,
                 64,     0
            };

          qvalue=(ssize_t) (jpeg_info->quant_tbl_ptrs[0]->quantval[2]+
            jpeg_info->quant_tbl_ptrs[0]->quantval[53]);
          for (i=0; i < 100; i++)
          {
            if ((qvalue < hash[i]) && (sum < sums[i]))
              continue;
            if (((qvalue <= hash[i]) && (sum <= sums[i])) || (i >= 50))
              image->quality=(size_t) i+1;
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "Quality: %.20g (%s)",(double) i+1,(qvalue <= hash[i]) &&
                (sum <= sums[i]) ? "exact" : "approximate");
            break;
          }
        }
  }
}

static void JPEGSetImageSamplingFactor(struct jpeg_decompress_struct *jpeg_info,  Image *image)
{
  char
    sampling_factor[MaxTextExtent];

  switch (jpeg_info->out_color_space)
  {
    case JCS_CMYK:
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Colorspace: CMYK");
      (void) FormatLocaleString(sampling_factor,MaxTextExtent,
        "%dx%d,%dx%d,%dx%d,%dx%d",jpeg_info->comp_info[0].h_samp_factor,
        jpeg_info->comp_info[0].v_samp_factor,
        jpeg_info->comp_info[1].h_samp_factor,
        jpeg_info->comp_info[1].v_samp_factor,
        jpeg_info->comp_info[2].h_samp_factor,
        jpeg_info->comp_info[2].v_samp_factor,
        jpeg_info->comp_info[3].h_samp_factor,
        jpeg_info->comp_info[3].v_samp_factor);
      break;
    }
    case JCS_GRAYSCALE:
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "Colorspace: GRAYSCALE");
      (void) FormatLocaleString(sampling_factor,MaxTextExtent,"%dx%d",
        jpeg_info->comp_info[0].h_samp_factor,
        jpeg_info->comp_info[0].v_samp_factor);
      break;
    }
    case JCS_RGB:
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Colorspace: RGB");
      (void) FormatLocaleString(sampling_factor,MaxTextExtent,
        "%dx%d,%dx%d,%dx%d",jpeg_info->comp_info[0].h_samp_factor,
        jpeg_info->comp_info[0].v_samp_factor,
        jpeg_info->comp_info[1].h_samp_factor,
        jpeg_info->comp_info[1].v_samp_factor,
        jpeg_info->comp_info[2].h_samp_factor,
        jpeg_info->comp_info[2].v_samp_factor);
      break;
    }
    default:
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Colorspace: %d",
        jpeg_info->out_color_space);
      (void) FormatLocaleString(sampling_factor,MaxTextExtent,
        "%dx%d,%dx%d,%dx%d,%dx%d",jpeg_info->comp_info[0].h_samp_factor,
        jpeg_info->comp_info[0].v_samp_factor,
        jpeg_info->comp_info[1].h_samp_factor,
        jpeg_info->comp_info[1].v_samp_factor,
        jpeg_info->comp_info[2].h_samp_factor,
        jpeg_info->comp_info[2].v_samp_factor,
        jpeg_info->comp_info[3].h_samp_factor,
        jpeg_info->comp_info[3].v_samp_factor);
      break;
    }
  }
  (void) SetImageProperty(image,"jpeg:sampling-factor",sampling_factor);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Sampling Factors: %s",
    sampling_factor);
}

static Image *ReadJPEGImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    value[MaxTextExtent];

  const char
    *option;

  ErrorManager
    error_manager;

  Image
    *image;

  IndexPacket
    index;

  JSAMPLE
    *volatile jpeg_pixels;

  JSAMPROW
    scanline[1];

  MagickBooleanType
    debug,
    status;

  MagickSizeType
    number_pixels;

  MemoryInfo
    *memory_info;

  register ssize_t
    i;

  struct jpeg_decompress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  register JSAMPLE
    *p;

  size_t
    units;

  ssize_t
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
  debug=IsEventLogging();
  (void) debug;
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Initialize JPEG parameters.
  */
  (void) ResetMagickMemory(&error_manager,0,sizeof(error_manager));
  (void) ResetMagickMemory(&jpeg_info,0,sizeof(jpeg_info));
  (void) ResetMagickMemory(&jpeg_error,0,sizeof(jpeg_error));
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=(void (*)(j_common_ptr,int)) JPEGWarningHandler;
  jpeg_info.err->error_exit=(void (*)(j_common_ptr)) JPEGErrorHandler;
  memory_info=(MemoryInfo *) NULL;
  error_manager.image=image;
  if (setjmp(error_manager.error_recovery) != 0)
    {
      jpeg_destroy_decompress(&jpeg_info);
      if (error_manager.profile != (StringInfo *) NULL)
        error_manager.profile=DestroyStringInfo(error_manager.profile);
      (void) CloseBlob(image);
      number_pixels=(MagickSizeType) image->columns*image->rows;
      if (number_pixels != 0)
        return(GetFirstImageInList(image));
      InheritException(exception,&image->exception);
      return(DestroyImage(image));
    }
  jpeg_info.client_data=(void *) &error_manager;
  jpeg_create_decompress(&jpeg_info);
  JPEGSourceManager(&jpeg_info,image);
  jpeg_set_marker_processor(&jpeg_info,JPEG_COM,ReadComment);
  jpeg_set_marker_processor(&jpeg_info,ICC_MARKER,ReadICCProfile);
  jpeg_set_marker_processor(&jpeg_info,IPTC_MARKER,ReadIPTCProfile);
  for (i=1; i < 16; i++)
    if ((i != 2) && (i != 13) && (i != 14))
      jpeg_set_marker_processor(&jpeg_info,(int) (JPEG_APP0+i),ReadProfile);
  i=(ssize_t) jpeg_read_header(&jpeg_info,MagickTrue);
  if ((image_info->colorspace == YCbCrColorspace) ||
      (image_info->colorspace == Rec601YCbCrColorspace) ||
      (image_info->colorspace == Rec709YCbCrColorspace))
    jpeg_info.out_color_space=JCS_YCbCr;
  /*
    Set image resolution.
  */
  units=0;
  if ((jpeg_info.saw_JFIF_marker != 0) && (jpeg_info.X_density != 1) &&
      (jpeg_info.Y_density != 1))
    {
      image->x_resolution=(double) jpeg_info.X_density;
      image->y_resolution=(double) jpeg_info.Y_density;
      units=(size_t) jpeg_info.density_unit;
    }
  if (units == 1)
    image->units=PixelsPerInchResolution;
  if (units == 2)
    image->units=PixelsPerCentimeterResolution;
  number_pixels=(MagickSizeType) image->columns*image->rows;
  option=GetImageOption(image_info,"jpeg:size");
  if (option != (const char *) NULL)
    {
      double
        scale_factor;

      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      /*
        Scale the image.
      */
      flags=ParseGeometry(option,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=geometry_info.rho;
      jpeg_calc_output_dimensions(&jpeg_info);
      image->magick_columns=jpeg_info.output_width;
      image->magick_rows=jpeg_info.output_height;
      scale_factor=1.0;
      if (geometry_info.rho != 0.0)
        scale_factor=jpeg_info.output_width/geometry_info.rho;
      if ((geometry_info.sigma != 0.0) &&
          (scale_factor > (jpeg_info.output_height/geometry_info.sigma)))
        scale_factor=jpeg_info.output_height/geometry_info.sigma;
      jpeg_info.scale_num=1U;
      jpeg_info.scale_denom=(unsigned int) scale_factor;
      jpeg_calc_output_dimensions(&jpeg_info);
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Scale factor: %.20g",(double) scale_factor);
    }
#if (JPEG_LIB_VERSION >= 61) && defined(D_PROGRESSIVE_SUPPORTED)
#if defined(D_LOSSLESS_SUPPORTED)
  image->interlace=jpeg_info.process == JPROC_PROGRESSIVE ?
    JPEGInterlace : NoInterlace;
  image->compression=jpeg_info.process == JPROC_LOSSLESS ?
    LosslessJPEGCompression : JPEGCompression;
  if (jpeg_info.data_precision > 8)
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "12-bit JPEG not supported. Reducing pixel data to 8 bits","`%s'",
      image->filename);
  if (jpeg_info.data_precision == 16)
    jpeg_info.data_precision=12;
#else
  image->interlace=jpeg_info.progressive_mode != 0 ? JPEGInterlace :
    NoInterlace;
  image->compression=JPEGCompression;
#endif
#else
  image->compression=JPEGCompression;
  image->interlace=JPEGInterlace;
#endif
  option=GetImageOption(image_info,"jpeg:colors");
  if (option != (const char *) NULL)
    {
      /*
        Let the JPEG library quantize for us.
      */
      jpeg_info.quantize_colors=MagickTrue;
      jpeg_info.desired_number_of_colors=(int) StringToUnsignedLong(option);
    }
  option=GetImageOption(image_info,"jpeg:block-smoothing");
  if (option != (const char *) NULL)
    jpeg_info.do_block_smoothing=IsStringTrue(option);
  jpeg_info.dct_method=JDCT_FLOAT;
  option=GetImageOption(image_info,"jpeg:dct-method");
  if (option != (const char *) NULL)
    switch (*option)
    {
      case 'D':
      case 'd':
      {
        if (LocaleCompare(option,"default") == 0)
          jpeg_info.dct_method=JDCT_DEFAULT;
        break;
      }
      case 'F':
      case 'f':
      {
        if (LocaleCompare(option,"fastest") == 0)
          jpeg_info.dct_method=JDCT_FASTEST;
        if (LocaleCompare(option,"float") == 0)
          jpeg_info.dct_method=JDCT_FLOAT;
        break;
      }
      case 'I':
      case 'i':
      {
        if (LocaleCompare(option,"ifast") == 0)
          jpeg_info.dct_method=JDCT_IFAST;
        if (LocaleCompare(option,"islow") == 0)
          jpeg_info.dct_method=JDCT_ISLOW;
        break;
      }
    }
  option=GetImageOption(image_info,"jpeg:fancy-upsampling");
  if (option != (const char *) NULL)
    jpeg_info.do_fancy_upsampling=IsStringTrue(option);
  (void) jpeg_start_decompress(&jpeg_info);
  image->columns=jpeg_info.output_width;
  image->rows=jpeg_info.output_height;
  image->depth=(size_t) jpeg_info.data_precision;
  switch (jpeg_info.out_color_space)
  {
    case JCS_RGB:
    default:
    {
      (void) SetImageColorspace(image,sRGBColorspace);
      break;
    }
    case JCS_GRAYSCALE:
    {
      (void) SetImageColorspace(image,GRAYColorspace);
      break;
    }
    case JCS_YCbCr:
    {
      (void) SetImageColorspace(image,YCbCrColorspace);
      break;
    }
    case JCS_CMYK:
    {
      (void) SetImageColorspace(image,CMYKColorspace);
      break;
    }
  }
  if (IsITUFaxImage(image) != MagickFalse)
    {
      (void) SetImageColorspace(image,LabColorspace);
      jpeg_info.out_color_space=JCS_YCbCr;
    }
  if (option != (const char *) NULL)
    if (AcquireImageColormap(image,StringToUnsignedLong(option)) == MagickFalse)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if ((jpeg_info.output_components == 1) &&
      (jpeg_info.quantize_colors == MagickFalse))
    {
      size_t
        colors;

      colors=(size_t) GetQuantumRange(image->depth)+1;
      if (AcquireImageColormap(image,colors) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  if (image->debug != MagickFalse)
    {
      if (image->interlace != NoInterlace)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Interlace: progressive");
      else
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Interlace: nonprogressive");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Data precision: %d",
        (int) jpeg_info.data_precision);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Geometry: %dx%d",
        (int) jpeg_info.output_width,(int) jpeg_info.output_height);
    }
  JPEGSetImageQuality(&jpeg_info,image);
  JPEGSetImageSamplingFactor(&jpeg_info,image);
  (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
    jpeg_info.out_color_space);
  (void) SetImageProperty(image,"jpeg:colorspace",value);
  if (image_info->ping != MagickFalse)
    {
      jpeg_destroy_decompress(&jpeg_info);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  memory_info=AcquireVirtualMemory((size_t) image->columns,
    jpeg_info.output_components*sizeof(*jpeg_pixels));
  if (memory_info == (MemoryInfo *) NULL)
    {
      jpeg_destroy_decompress(&jpeg_info);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  jpeg_pixels=(JSAMPLE *) GetVirtualMemoryBlob(memory_info);
  /*
    Convert JPEG pixels to pixel packets.
  */
  if (setjmp(error_manager.error_recovery) != 0)
    {
      if (memory_info != (MemoryInfo *) NULL)
        memory_info=RelinquishVirtualMemory(memory_info);
      jpeg_destroy_decompress(&jpeg_info);
      (void) CloseBlob(image);
      number_pixels=(MagickSizeType) image->columns*image->rows;
      if (number_pixels != 0)
        return(GetFirstImageInList(image));
      return(DestroyImage(image));
    }
  if (jpeg_info.quantize_colors != MagickFalse)
    {
      image->colors=(size_t) jpeg_info.actual_number_of_colors;
      if (jpeg_info.out_color_space == JCS_GRAYSCALE)
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          image->colormap[i].red=ScaleCharToQuantum(jpeg_info.colormap[0][i]);
          image->colormap[i].green=image->colormap[i].red;
          image->colormap[i].blue=image->colormap[i].red;
          image->colormap[i].opacity=OpaqueOpacity;
        }
      else
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          image->colormap[i].red=ScaleCharToQuantum(jpeg_info.colormap[0][i]);
          image->colormap[i].green=ScaleCharToQuantum(jpeg_info.colormap[1][i]);
          image->colormap[i].blue=ScaleCharToQuantum(jpeg_info.colormap[2][i]);
          image->colormap[i].opacity=OpaqueOpacity;
        }
    }
  scanline[0]=(JSAMPROW) jpeg_pixels;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (jpeg_read_scanlines(&jpeg_info,scanline,1) != 1)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageWarning,"SkipToSyncByte","`%s'",image->filename);
        continue;
      }
    p=jpeg_pixels;
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    if (jpeg_info.data_precision > 8)
      {
        unsigned short
          scale;

        scale=65535U/GetQuantumRange(jpeg_info.data_precision);
        if (jpeg_info.output_components == 1)
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            size_t
              pixel;

            pixel=(size_t) (scale*GETJSAMPLE(*p));
            index=ConstrainColormapIndex(image,pixel);
            SetPixelIndex(indexes+x,index);
            SetPixelRGBO(q,image->colormap+(ssize_t) index);
            p++;
            q++;
          }
        else
          if (image->colorspace != CMYKColorspace)
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelRed(q,ScaleShortToQuantum(scale*GETJSAMPLE(*p++)));
              SetPixelGreen(q,ScaleShortToQuantum(scale*GETJSAMPLE(*p++)));
              SetPixelBlue(q,ScaleShortToQuantum(scale*GETJSAMPLE(*p++)));
              SetPixelOpacity(q,OpaqueOpacity);
              q++;
            }
          else
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelCyan(q,QuantumRange-ScaleShortToQuantum(scale*
                GETJSAMPLE(*p++)));
              SetPixelMagenta(q,QuantumRange-ScaleShortToQuantum(scale*
                GETJSAMPLE(*p++)));
              SetPixelYellow(q,QuantumRange-ScaleShortToQuantum(scale*
                GETJSAMPLE(*p++)));
              SetPixelBlack(indexes+x,QuantumRange-ScaleShortToQuantum(scale*
                GETJSAMPLE(*p++)));
              SetPixelOpacity(q,OpaqueOpacity);
              q++;
            }
      }
    else
      if (jpeg_info.output_components == 1)
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          index=ConstrainColormapIndex(image,(size_t) GETJSAMPLE(*p));
          SetPixelIndex(indexes+x,index);
          SetPixelRGBO(q,image->colormap+(ssize_t) index);
          p++;
          q++;
        }
      else
        if (image->colorspace != CMYKColorspace)
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(q,ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++)));
            SetPixelGreen(q,ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++)));
            SetPixelBlue(q,ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++)));
            SetPixelOpacity(q,OpaqueOpacity);
            q++;
          }
        else
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelCyan(q,QuantumRange-ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++)));
            SetPixelMagenta(q,QuantumRange-ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++)));
            SetPixelYellow(q,QuantumRange-ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++)));
            SetPixelBlack(indexes+x,QuantumRange-ScaleCharToQuantum(
              (unsigned char) GETJSAMPLE(*p++)));
            SetPixelOpacity(q,OpaqueOpacity);
            q++;
          }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      {
        jpeg_abort_decompress(&jpeg_info);
        break;
      }
  }
  if (status != MagickFalse)
    {
      error_manager.finished=MagickTrue;
      if (setjmp(error_manager.error_recovery) == 0)
        (void) jpeg_finish_decompress(&jpeg_info);
    }
  /*
    Free jpeg resources.
  */
  jpeg_destroy_decompress(&jpeg_info);
  memory_info=RelinquishVirtualMemory(memory_info);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r J P E G I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJPEGImage() adds properties for the JPEG image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterJPEGImage method is:
%
%      size_t RegisterJPEGImage(void)
%
*/
ModuleExport size_t RegisterJPEGImage(void)
{
  char
    version[MaxTextExtent];

  MagickInfo
    *entry;

  static const char
    description[] = "Joint Photographic Experts Group JFIF format";

  *version='\0';
#if defined(JPEG_LIB_VERSION)
  (void) FormatLocaleString(version,MaxTextExtent,"%d",JPEG_LIB_VERSION);
#endif
  entry=SetMagickInfo("JPEG");
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JPEG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJPEGImage;
  entry->encoder=(EncodeImageHandler *) WriteJPEGImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsJPEG;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString(description);
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jpeg");
  entry->module=ConstantString("JPEG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("JPG");
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JPEG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJPEGImage;
  entry->encoder=(EncodeImageHandler *) WriteJPEGImage;
#endif
  entry->adjoin=MagickFalse;
  entry->description=ConstantString(description);
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jpeg");
  entry->module=ConstantString("JPEG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PJPEG");
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JPEG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJPEGImage;
  entry->encoder=(EncodeImageHandler *) WriteJPEGImage;
#endif
  entry->adjoin=MagickFalse;
  entry->description=ConstantString(description);
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->mime_type=ConstantString("image/jpeg");
  entry->module=ConstantString("JPEG");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r J P E G I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJPEGImage() removes format registrations made by the
%  JPEG module from the list of supported formats.
%
%  The format of the UnregisterJPEGImage method is:
%
%      UnregisterJPEGImage(void)
%
*/
ModuleExport void UnregisterJPEGImage(void)
{
  (void) UnregisterMagickInfo("PJPG");
  (void) UnregisterMagickInfo("JPEG");
  (void) UnregisterMagickInfo("JPG");
}

#if defined(MAGICKCORE_JPEG_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e J P E G I m a g e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJPEGImage() writes a JPEG image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the WriteJPEGImage method is:
%
%      MagickBooleanType WriteJPEGImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o jpeg_image:  The image.
%
%
*/

static QuantizationTable *DestroyQuantizationTable(QuantizationTable *table)
{
  assert(table != (QuantizationTable *) NULL);
  if (table->slot != (char *) NULL)
    table->slot=DestroyString(table->slot);
  if (table->description != (char *) NULL)
    table->description=DestroyString(table->description);
  if (table->levels != (unsigned int *) NULL)
    table->levels=(unsigned int *) RelinquishMagickMemory(table->levels);
  table=(QuantizationTable *) RelinquishMagickMemory(table);
  return(table);
}

static boolean EmptyOutputBuffer(j_compress_ptr cinfo)
{
  DestinationManager
    *destination;

  destination=(DestinationManager *) cinfo->dest;
  destination->manager.free_in_buffer=(size_t) WriteBlob(destination->image,
    MaxBufferExtent,destination->buffer);
  if (destination->manager.free_in_buffer != MaxBufferExtent)
    ERREXIT(cinfo,JERR_FILE_WRITE);
  destination->manager.next_output_byte=destination->buffer;
  return(TRUE);
}

static QuantizationTable *GetQuantizationTable(const char *filename,
  const char *slot,ExceptionInfo *exception)
{
  char
    *p,
    *xml;

  const char
    *attribute,
    *content;

  double
    value;

  register ssize_t
    i;

  QuantizationTable
    *table;

  size_t
    length;

  ssize_t
    j;

  XMLTreeInfo
    *description,
    *levels,
    *quantization_tables,
    *table_iterator;

  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading quantization tables \"%s\" ...",filename);
  table=(QuantizationTable *) NULL;
  xml=FileToString(filename,~0UL,exception);
  if (xml == (char *) NULL)
    return(table);
  quantization_tables=NewXMLTree(xml,exception);
  if (quantization_tables == (XMLTreeInfo *) NULL)
    {
      xml=DestroyString(xml);
      return(table);
    }
  for (table_iterator=GetXMLTreeChild(quantization_tables,"table");
       table_iterator != (XMLTreeInfo *) NULL;
       table_iterator=GetNextXMLTreeTag(table_iterator))
  {
    attribute=GetXMLTreeAttribute(table_iterator,"slot");
    if ((attribute != (char *) NULL) && (LocaleCompare(slot,attribute) == 0))
      break;
    attribute=GetXMLTreeAttribute(table_iterator,"alias");
    if ((attribute != (char *) NULL) && (LocaleCompare(slot,attribute) == 0))
      break;
  }
  if (table_iterator == (XMLTreeInfo *) NULL)
    {
      xml=DestroyString(xml);
      return(table);
    }
  description=GetXMLTreeChild(table_iterator,"description");
  if (description == (XMLTreeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<description>, slot \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      xml=DestroyString(xml);
      return(table);
    }
  levels=GetXMLTreeChild(table_iterator,"levels");
  if (levels == (XMLTreeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<levels>, slot \"%s\"", slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      xml=DestroyString(xml);
      return(table);
    }
  table=(QuantizationTable *) AcquireMagickMemory(sizeof(*table));
  if (table == (QuantizationTable *) NULL)
    ThrowFatalException(ResourceLimitFatalError,
      "UnableToAcquireQuantizationTable");
  table->slot=(char *) NULL;
  table->description=(char *) NULL;
  table->levels=(unsigned int *) NULL;
  attribute=GetXMLTreeAttribute(table_iterator,"slot");
  if (attribute != (char *) NULL)
    table->slot=ConstantString(attribute);
  content=GetXMLTreeContent(description);
  if (content != (char *) NULL)
    table->description=ConstantString(content);
  attribute=GetXMLTreeAttribute(levels,"width");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels width>, slot \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      table=DestroyQuantizationTable(table);
      xml=DestroyString(xml);
      return(table);
    }
  table->width=StringToUnsignedLong(attribute);
  if (table->width == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
       "XmlInvalidAttribute", "<levels width>, table \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      table=DestroyQuantizationTable(table);
      xml=DestroyString(xml);
      return(table);
    }
  attribute=GetXMLTreeAttribute(levels,"height");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels height>, table \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      table=DestroyQuantizationTable(table);
      xml=DestroyString(xml);
      return(table);
    }
  table->height=StringToUnsignedLong(attribute);
  if (table->height == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels height>, table \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      table=DestroyQuantizationTable(table);
      xml=DestroyString(xml);
      return(table);
    }
  attribute=GetXMLTreeAttribute(levels,"divisor");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels divisor>, table \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      table=DestroyQuantizationTable(table);
      xml=DestroyString(xml);
      return(table);
    }
  table->divisor=InterpretLocaleValue(attribute,(char **) NULL);
  if (table->divisor == 0.0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels divisor>, table \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      table=DestroyQuantizationTable(table);
      xml=DestroyString(xml);
      return(table);
    }
  content=GetXMLTreeContent(levels);
  if (content == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingContent", "<levels>, table \"%s\"",slot);
      quantization_tables=DestroyXMLTree(quantization_tables);
      table=DestroyQuantizationTable(table);
      xml=DestroyString(xml);
      return(table);
    }
  length=(size_t) table->width*table->height;
  if (length < 64)
    length=64;
  table->levels=(unsigned int *) AcquireQuantumMemory(length,
    sizeof(*table->levels));
  if (table->levels == (unsigned int *) NULL)
    ThrowFatalException(ResourceLimitFatalError,
      "UnableToAcquireQuantizationTable");
  for (i=0; i < (ssize_t) (table->width*table->height); i++)
  {
    table->levels[i]=(unsigned int) (InterpretLocaleValue(content,&p)/
      table->divisor+0.5);
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;
    if (*p == ',')
      p++;
    content=p;
  }
  value=InterpretLocaleValue(content,&p);
  (void) value;
  if (p != content)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidContent", "<level> too many values, table \"%s\"",slot);
     quantization_tables=DestroyXMLTree(quantization_tables);
     table=DestroyQuantizationTable(table);
     xml=DestroyString(xml);
     return(table);
   }
  for (j=i; j < 64; j++)
    table->levels[j]=table->levels[j-1];
  quantization_tables=DestroyXMLTree(quantization_tables);
  xml=DestroyString(xml);
  return(table);
}

static void InitializeDestination(j_compress_ptr cinfo)
{
  DestinationManager
    *destination;

  destination=(DestinationManager *) cinfo->dest;
  destination->buffer=(JOCTET *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,MaxBufferExtent*sizeof(JOCTET));
  destination->manager.next_output_byte=destination->buffer;
  destination->manager.free_in_buffer=MaxBufferExtent;
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static void TerminateDestination(j_compress_ptr cinfo)
{
  DestinationManager
    *destination;

  destination=(DestinationManager *) cinfo->dest;
  if ((MaxBufferExtent-(int) destination->manager.free_in_buffer) > 0)
    {
      ssize_t
        count;

      count=WriteBlob(destination->image,MaxBufferExtent-
        destination->manager.free_in_buffer,destination->buffer);
      if (count != (ssize_t)
          (MaxBufferExtent-destination->manager.free_in_buffer))
        ERREXIT(cinfo,JERR_FILE_WRITE);
    }
}

static void WriteProfile(j_compress_ptr jpeg_info,Image *image)
{
  const char
    *name;

  const StringInfo
    *profile;

  MagickBooleanType
    iptc;

  register ssize_t
    i;

  size_t
    length,
    tag_length;

  StringInfo
    *custom_profile;

  /*
    Save image profile as a APP marker.
  */
  iptc=MagickFalse;
  custom_profile=AcquireStringInfo(65535L);
  ResetImageProfileIterator(image);
  for (name=GetNextImageProfile(image); name != (const char *) NULL; )
  {
    register unsigned char
      *p;

    profile=GetImageProfile(image,name);
    p=GetStringInfoDatum(custom_profile);
    if (LocaleCompare(name,"EXIF") == 0)
      for (i=0; i < (ssize_t) GetStringInfoLength(profile); i+=65533L)
      {
        length=MagickMin(GetStringInfoLength(profile)-i,65533L);
        jpeg_write_marker(jpeg_info,XML_MARKER,GetStringInfoDatum(profile)+i,
          (unsigned int) length);
      }
    if (LocaleCompare(name,"ICC") == 0)
      {
        register unsigned char
          *p;

        tag_length=strlen(ICC_PROFILE);
        p=GetStringInfoDatum(custom_profile);
        (void) CopyMagickMemory(p,ICC_PROFILE,tag_length);
        p[tag_length]='\0';
        for (i=0; i < (ssize_t) GetStringInfoLength(profile); i+=65519L)
        {
          length=MagickMin(GetStringInfoLength(profile)-i,65519L);
          p[12]=(unsigned char) ((i/65519L)+1);
          p[13]=(unsigned char) (GetStringInfoLength(profile)/65519L+1);
          (void) CopyMagickMemory(p+tag_length+3,GetStringInfoDatum(profile)+i,
            length);
          jpeg_write_marker(jpeg_info,ICC_MARKER,GetStringInfoDatum(
            custom_profile),(unsigned int) (length+tag_length+3));
        }
      }
    if (((LocaleCompare(name,"IPTC") == 0) ||
        (LocaleCompare(name,"8BIM") == 0)) && (iptc == MagickFalse))
      {
        size_t
          roundup;

        iptc=MagickTrue;
        for (i=0; i < (ssize_t) GetStringInfoLength(profile); i+=65500L)
        {
          length=MagickMin(GetStringInfoLength(profile)-i,65500L);
          roundup=(size_t) (length & 0x01);
          if (LocaleNCompare((char *) GetStringInfoDatum(profile),"8BIM",4) == 0)
            {
              (void) memcpy(p,"Photoshop 3.0 ",14);
              tag_length=14;
            }
          else
            {
              (void) CopyMagickMemory(p,"Photoshop 3.0 8BIM\04\04\0\0\0\0",24);
              tag_length=26;
              p[24]=(unsigned char) (length >> 8);
              p[25]=(unsigned char) (length & 0xff);
            }
          p[13]=0x00;
          (void) memcpy(p+tag_length,GetStringInfoDatum(profile)+i,length);
          if (roundup != 0)
            p[length+tag_length]='\0';
          jpeg_write_marker(jpeg_info,IPTC_MARKER,GetStringInfoDatum(
            custom_profile),(unsigned int) (length+tag_length+roundup));
        }
      }
    if (LocaleCompare(name,"XMP") == 0)
      {
        StringInfo
          *xmp_profile;

        /*
          Add namespace to XMP profile.
        */
        xmp_profile=StringToStringInfo("http://ns.adobe.com/xap/1.0/ ");
        ConcatenateStringInfo(xmp_profile,profile);
        GetStringInfoDatum(xmp_profile)[28]='\0';
        for (i=0; i < (ssize_t) GetStringInfoLength(xmp_profile); i+=65533L)
        {
          length=MagickMin(GetStringInfoLength(xmp_profile)-i,65533L);
          jpeg_write_marker(jpeg_info,XML_MARKER,
            GetStringInfoDatum(xmp_profile)+i,(unsigned int) length);
        }
        xmp_profile=DestroyStringInfo(xmp_profile);
      }
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "%s profile: %.20g bytes",name,(double) GetStringInfoLength(profile));
    name=GetNextImageProfile(image);
  }
  custom_profile=DestroyStringInfo(custom_profile);
}

static void JPEGDestinationManager(j_compress_ptr cinfo,Image * image)
{
  DestinationManager
    *destination;

  cinfo->dest=(struct jpeg_destination_mgr *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,sizeof(DestinationManager));
  destination=(DestinationManager *) cinfo->dest;
  destination->manager.init_destination=InitializeDestination;
  destination->manager.empty_output_buffer=EmptyOutputBuffer;
  destination->manager.term_destination=TerminateDestination;
  destination->image=image;
}

static char **SamplingFactorToList(const char *text)
{
  char
    **textlist;

  register char
    *q;

  register const char
    *p;

  register ssize_t
    i;

  size_t
    lines;

  if (text == (char *) NULL)
    return((char **) NULL);
  /*
    Convert string to an ASCII list.
  */
  lines=1;
  for (p=text; *p != '\0'; p++)
    if (*p == ',')
      lines++;
  textlist=(char **) AcquireQuantumMemory((size_t) lines+MaxTextExtent,
    sizeof(*textlist));
  if (textlist == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConvertText");
  p=text;
  for (i=0; i < (ssize_t) lines; i++)
  {
    for (q=(char *) p; *q != '\0'; q++)
      if (*q == ',')
        break;
    textlist[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+MaxTextExtent,
      sizeof(*textlist[i]));
    if (textlist[i] == (char *) NULL)
      ThrowFatalException(ResourceLimitFatalError,"UnableToConvertText");
    (void) CopyMagickString(textlist[i],p,(size_t) (q-p+1));
    if (*q == '\r')
      q++;
    p=q+1;
  }
  textlist[i]=(char *) NULL;
  return(textlist);
}

static MagickBooleanType WriteJPEGImage(const ImageInfo *image_info,
  Image *image)
{
  const char
    *option,
    *sampling_factor,
    *value;

  ErrorManager
    error_manager;

  int
    colorspace,
    quality;

  JSAMPLE
    *volatile jpeg_pixels;

  JSAMPROW
    scanline[1];

  MagickBooleanType
    status;

  MemoryInfo
    *memory_info;

  register JSAMPLE
    *q;

  register ssize_t
    i;

  ssize_t
    y;

  struct jpeg_compress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  unsigned short
    scale;

  /*
    Open image file.
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
  /*
    Initialize JPEG parameters.
  */
  (void) ResetMagickMemory(&error_manager,0,sizeof(error_manager));
  (void) ResetMagickMemory(&jpeg_info,0,sizeof(jpeg_info));
  (void) ResetMagickMemory(&jpeg_error,0,sizeof(jpeg_error));
  jpeg_info.client_data=(void *) image;
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=(void (*)(j_common_ptr,int)) JPEGWarningHandler;
  jpeg_info.err->error_exit=(void (*)(j_common_ptr)) JPEGErrorHandler;
  error_manager.image=image;
  memory_info=(MemoryInfo *) NULL;
  if (setjmp(error_manager.error_recovery) != 0)
    {
      jpeg_destroy_compress(&jpeg_info);
      (void) CloseBlob(image);
      return(MagickFalse);
    }
  jpeg_info.client_data=(void *) &error_manager;
  jpeg_create_compress(&jpeg_info);
  JPEGDestinationManager(&jpeg_info,image);
  if ((image->columns != (unsigned int) image->columns) ||
      (image->rows != (unsigned int) image->rows))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  jpeg_info.image_width=(unsigned int) image->columns;
  jpeg_info.image_height=(unsigned int) image->rows;
  jpeg_info.input_components=3;
  jpeg_info.data_precision=8;
  jpeg_info.in_color_space=JCS_RGB;
  switch (image->colorspace)
  {
    case CMYKColorspace:
    {
      jpeg_info.input_components=4;
      jpeg_info.in_color_space=JCS_CMYK;
      break;
    }
    case YCbCrColorspace:
    case Rec601YCbCrColorspace:
    case Rec709YCbCrColorspace:
    {
      jpeg_info.in_color_space=JCS_YCbCr;
      break;
    }
    case GRAYColorspace:
    case Rec601LumaColorspace:
    case Rec709LumaColorspace:
    {
      jpeg_info.input_components=1;
      jpeg_info.in_color_space=JCS_GRAYSCALE;
      break;
    }
    default:
    {
      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
        (void) TransformImageColorspace(image,sRGBColorspace);
      break;
    }
  }
  if ((image_info->type != TrueColorType) &&
      (IsGrayImage(image,&image->exception) != MagickFalse))
    {
      jpeg_info.input_components=1;
      jpeg_info.in_color_space=JCS_GRAYSCALE;
    }
  jpeg_set_defaults(&jpeg_info);
  if (jpeg_info.in_color_space == JCS_CMYK)
    jpeg_set_colorspace(&jpeg_info,JCS_YCCK);
  if ((jpeg_info.data_precision != 12) && (image->depth <= 8))
    jpeg_info.data_precision=8;
  else
    jpeg_info.data_precision=BITS_IN_JSAMPLE;
  jpeg_info.density_unit=(UINT8) 1;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Image resolution: %.20g,%.20g",floor(image->x_resolution+0.5),
      floor(image->y_resolution+0.5));
  if ((image->x_resolution != 0.0) && (image->y_resolution != 0.0))
    {
      /*
        Set image resolution.
      */
      jpeg_info.write_JFIF_header=MagickTrue;
      jpeg_info.X_density=(UINT16) floor(image->x_resolution+0.5);
      jpeg_info.Y_density=(UINT16) floor(image->y_resolution+0.5);
      /*
        Set image resolution units.
      */
      jpeg_info.density_unit=(UINT8) 0;
      if (image->units == PixelsPerInchResolution)
        jpeg_info.density_unit=(UINT8) 1;
      if (image->units == PixelsPerCentimeterResolution)
        jpeg_info.density_unit=(UINT8) 2;
    }
  jpeg_info.dct_method=JDCT_FLOAT;
  option=GetImageOption(image_info,"jpeg:dct-method");
  if (option != (const char *) NULL)
    switch (*option)
    {
      case 'D':
      case 'd':
      {
        if (LocaleCompare(option,"default") == 0)
          jpeg_info.dct_method=JDCT_DEFAULT;
        break;
      }
      case 'F':
      case 'f':
      {
        if (LocaleCompare(option,"fastest") == 0)
          jpeg_info.dct_method=JDCT_FASTEST;
        if (LocaleCompare(option,"float") == 0)
          jpeg_info.dct_method=JDCT_FLOAT;
        break;
      }
      case 'I':
      case 'i':
      {
        if (LocaleCompare(option,"ifast") == 0)
          jpeg_info.dct_method=JDCT_IFAST;
        if (LocaleCompare(option,"islow") == 0)
          jpeg_info.dct_method=JDCT_ISLOW;
        break;
      }
    }
  option=GetImageOption(image_info,"jpeg:optimize-coding");
  if (option != (const char *) NULL)
    jpeg_info.optimize_coding=IsStringTrue(option);
  else
    {
      MagickSizeType
        length;

      length=(MagickSizeType) jpeg_info.input_components*image->columns*
        image->rows*sizeof(JSAMPLE);
      if (length == (MagickSizeType) ((size_t) length))
        {
          /*
            Perform optimization only if available memory resources permit it.
          */
          status=AcquireMagickResource(MemoryResource,length);
          RelinquishMagickResource(MemoryResource,length);
          jpeg_info.optimize_coding=status;
        }
    }
#if (JPEG_LIB_VERSION >= 61) && defined(C_PROGRESSIVE_SUPPORTED)
  if ((LocaleCompare(image_info->magick,"PJPEG") == 0) ||
      (image_info->interlace != NoInterlace))
    {
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Interlace: progressive");
      jpeg_simple_progression(&jpeg_info);
    }
  else
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "Interlace: non-progressive");
#else
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Interlace: nonprogressive");
#endif
  quality=92;
  option=GetImageOption(image_info,"jpeg:extent");
  if (option != (const char *) NULL)
    {
      Image
        *jpeg_image;

      ImageInfo
        *jpeg_info;

      jpeg_info=CloneImageInfo(image_info);
      jpeg_info->blob=NULL;
      jpeg_image=CloneImage(image,0,0,MagickTrue,&image->exception);
      if (jpeg_image != (Image *) NULL)
        {
          MagickSizeType
            extent;

          size_t
            maximum,
            minimum;

          /*
            Search for compression quality that does not exceed image extent.
          */
          jpeg_info->quality=0;
          extent=(MagickSizeType) SiPrefixToDoubleInterval(option,100.0);
          (void) DeleteImageOption(jpeg_info,"jpeg:extent");
          (void) DeleteImageArtifact(jpeg_image,"jpeg:extent");
          (void) AcquireUniqueFilename(jpeg_image->filename);
          maximum=101;
          for (minimum=2; minimum < maximum; )
          {
            jpeg_info->quality=minimum+(maximum-minimum+1)/2;
            status=WriteJPEGImage(jpeg_info,jpeg_image);
            if (GetBlobSize(jpeg_image) <= extent)
              minimum=jpeg_info->quality+1;
            else
              maximum=jpeg_info->quality-1;
          }
          (void) RelinquishUniqueFileResource(jpeg_image->filename);
          quality=minimum-1;
          jpeg_image=DestroyImage(jpeg_image);
        }
      jpeg_info=DestroyImageInfo(jpeg_info);
    }
  if ((image_info->compression != LosslessJPEGCompression) &&
      (image->quality <= 100))
    {
      if (image->quality != UndefinedCompressionQuality)
        quality=(int) image->quality;
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Quality: %.20g",
          (double) image->quality);
    }
  else
    {
#if !defined(C_LOSSLESS_SUPPORTED)
      quality=100;
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Quality: 100");
#else
      if (image->quality < 100)
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          CoderWarning,"LosslessToLossyJPEGConversion",image->filename);
      else
        {
          int
            point_transform,
            predictor;

          predictor=image->quality/100;  /* range 1-7 */
          point_transform=image->quality % 20;  /* range 0-15 */
          jpeg_simple_lossless(&jpeg_info,predictor,point_transform);
          if (image->debug != MagickFalse)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "Compression: lossless");
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "Predictor: %d",predictor);
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "Point Transform: %d",point_transform);
            }
        }
#endif
    }
  jpeg_set_quality(&jpeg_info,quality,MagickTrue);
#if (JPEG_LIB_VERSION >= 70)
  option=GetImageOption(image_info,"quality");
  if (option != (const char *) NULL)
    {
      GeometryInfo
        geometry_info;

      int
        flags;

      /*
        Set quality scaling for luminance and chrominance separately.
      */
      flags=ParseGeometry(option,&geometry_info);
      if (((flags & RhoValue) != 0) && ((flags & SigmaValue) != 0))
        {
          jpeg_info.q_scale_factor[0]=jpeg_quality_scaling((int)
            (geometry_info.rho+0.5));
          jpeg_info.q_scale_factor[1]=jpeg_quality_scaling((int)
            (geometry_info.sigma+0.5));
          jpeg_default_qtables(&jpeg_info,MagickTrue);
        }
    }
#endif
  colorspace=jpeg_info.in_color_space;
  value=GetImageOption(image_info,"jpeg:colorspace");
  if (value == (char *) NULL)
    value=GetImageProperty(image,"jpeg:colorspace");
  if (value != (char *) NULL)
    colorspace=StringToInteger(value);
  sampling_factor=(const char *) NULL;
  if (colorspace == jpeg_info.in_color_space)
    {
      value=GetImageOption(image_info,"jpeg:sampling-factor");
      if (value == (char *) NULL)
        value=GetImageProperty(image,"jpeg:sampling-factor");
      if (value != (char *) NULL)
        {
          sampling_factor=value;
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Input sampling-factors=%s",sampling_factor);
        }
    }
  if (image_info->sampling_factor != (char *) NULL)
    sampling_factor=image_info->sampling_factor;
  if (sampling_factor == (const char *) NULL)
    {
      if (image->quality >= 90)
        for (i=0; i < MAX_COMPONENTS; i++)
        {
          jpeg_info.comp_info[i].h_samp_factor=1;
          jpeg_info.comp_info[i].v_samp_factor=1;
        }
    }
  else
    {
      char
        **factors;

      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      /*
        Set sampling factor.
      */
      i=0;
      factors=SamplingFactorToList(sampling_factor);
      if (factors != (char **) NULL)
        {
          for (i=0; i < MAX_COMPONENTS; i++)
          {
            if (factors[i] == (char *) NULL)
              break;
            flags=ParseGeometry(factors[i],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=geometry_info.rho;
            jpeg_info.comp_info[i].h_samp_factor=(int) geometry_info.rho;
            jpeg_info.comp_info[i].v_samp_factor=(int) geometry_info.sigma;
            factors[i]=(char *) RelinquishMagickMemory(factors[i]);
          }
          factors=(char **) RelinquishMagickMemory(factors);
        }
      for ( ; i < MAX_COMPONENTS; i++)
      {
        jpeg_info.comp_info[i].h_samp_factor=1;
        jpeg_info.comp_info[i].v_samp_factor=1;
      }
    }
  option=GetImageOption(image_info,"jpeg:q-table");
  if (option != (const char *) NULL)
    {
      QuantizationTable
        *table;

      /*
        Custom quantization tables.
      */
      table=GetQuantizationTable(option,"0",&image->exception);
      if (table != (QuantizationTable *) NULL)
        {
          for (i=0; i < MAX_COMPONENTS; i++)
            jpeg_info.comp_info[i].quant_tbl_no=0;
          jpeg_add_quant_table(&jpeg_info,0,table->levels,
            jpeg_quality_scaling(quality),0);
          table=DestroyQuantizationTable(table);
        }
      table=GetQuantizationTable(option,"1",&image->exception);
      if (table != (QuantizationTable *) NULL)
        {
          for (i=1; i < MAX_COMPONENTS; i++)
            jpeg_info.comp_info[i].quant_tbl_no=1;
          jpeg_add_quant_table(&jpeg_info,1,table->levels,
            jpeg_quality_scaling(quality),0);
          table=DestroyQuantizationTable(table);
        }
      table=GetQuantizationTable(option,"2",&image->exception);
      if (table != (QuantizationTable *) NULL)
        {
          for (i=2; i < MAX_COMPONENTS; i++)
            jpeg_info.comp_info[i].quant_tbl_no=2;
          jpeg_add_quant_table(&jpeg_info,2,table->levels,
            jpeg_quality_scaling(quality),0);
          table=DestroyQuantizationTable(table);
        }
      table=GetQuantizationTable(option,"3",&image->exception);
      if (table != (QuantizationTable *) NULL)
        {
          for (i=3; i < MAX_COMPONENTS; i++)
            jpeg_info.comp_info[i].quant_tbl_no=3;
          jpeg_add_quant_table(&jpeg_info,3,table->levels,
            jpeg_quality_scaling(quality),0);
          table=DestroyQuantizationTable(table);
        }
    }
  jpeg_start_compress(&jpeg_info,MagickTrue);
  if (image->debug != MagickFalse)
    {
      if (image->storage_class == PseudoClass)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Storage class: PseudoClass");
      else
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Storage class: DirectClass");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Depth: %.20g",
        (double) image->depth);
      if (image->colors != 0)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Number of colors: %.20g",(double) image->colors);
      else
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Number of colors: unspecified");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "JPEG data precision: %d",(int) jpeg_info.data_precision);
      switch (image->colorspace)
      {
        case CMYKColorspace:
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Storage class: DirectClass");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Colorspace: CMYK");
          break;
        }
        case YCbCrColorspace:
        case Rec601YCbCrColorspace:
        case Rec709YCbCrColorspace:
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Colorspace: YCbCr");
          break;
        }
        default:
          break;
      }
      switch (image->colorspace)
      {
        case CMYKColorspace:
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Colorspace: CMYK");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Sampling factors: %dx%d,%dx%d,%dx%d,%dx%d",
            jpeg_info.comp_info[0].h_samp_factor,
            jpeg_info.comp_info[0].v_samp_factor,
            jpeg_info.comp_info[1].h_samp_factor,
            jpeg_info.comp_info[1].v_samp_factor,
            jpeg_info.comp_info[2].h_samp_factor,
            jpeg_info.comp_info[2].v_samp_factor,
            jpeg_info.comp_info[3].h_samp_factor,
            jpeg_info.comp_info[3].v_samp_factor);
          break;
        }
        case GRAYColorspace:
        case Rec601LumaColorspace:
        case Rec709LumaColorspace:
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Colorspace: GRAY");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Sampling factors: %dx%d",jpeg_info.comp_info[0].h_samp_factor,
            jpeg_info.comp_info[0].v_samp_factor);
          break;
        }
        case sRGBColorspace:
        case RGBColorspace:
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Image colorspace is RGB");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Sampling factors: %dx%d,%dx%d,%dx%d",
            jpeg_info.comp_info[0].h_samp_factor,
            jpeg_info.comp_info[0].v_samp_factor,
            jpeg_info.comp_info[1].h_samp_factor,
            jpeg_info.comp_info[1].v_samp_factor,
            jpeg_info.comp_info[2].h_samp_factor,
            jpeg_info.comp_info[2].v_samp_factor);
          break;
        }
        case YCbCrColorspace:
        case Rec601YCbCrColorspace:
        case Rec709YCbCrColorspace:
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Colorspace: YCbCr");
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Sampling factors: %dx%d,%dx%d,%dx%d",
            jpeg_info.comp_info[0].h_samp_factor,
            jpeg_info.comp_info[0].v_samp_factor,
            jpeg_info.comp_info[1].h_samp_factor,
            jpeg_info.comp_info[1].v_samp_factor,
            jpeg_info.comp_info[2].h_samp_factor,
            jpeg_info.comp_info[2].v_samp_factor);
          break;
        }
        default:
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Colorspace: %d",
            image->colorspace);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Sampling factors: %dx%d,%dx%d,%dx%d,%dx%d",
            jpeg_info.comp_info[0].h_samp_factor,
            jpeg_info.comp_info[0].v_samp_factor,
            jpeg_info.comp_info[1].h_samp_factor,
            jpeg_info.comp_info[1].v_samp_factor,
            jpeg_info.comp_info[2].h_samp_factor,
            jpeg_info.comp_info[2].v_samp_factor,
            jpeg_info.comp_info[3].h_samp_factor,
            jpeg_info.comp_info[3].v_samp_factor);
          break;
        }
      }
    }
  /*
    Write JPEG profiles.
  */
  value=GetImageProperty(image,"comment");
  if (value != (char *) NULL)
    for (i=0; i < (ssize_t) strlen(value); i+=65533L)
      jpeg_write_marker(&jpeg_info,JPEG_COM,(unsigned char *) value+i,
        (unsigned int) MagickMin((size_t) strlen(value+i),65533L));
  if (image->profiles != (void *) NULL)
    WriteProfile(&jpeg_info,image);
  /*
    Convert MIFF to JPEG raster pixels.
  */
  memory_info=AcquireVirtualMemory((size_t) image->columns,
    jpeg_info.input_components*sizeof(*jpeg_pixels));
  if (memory_info == (MemoryInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  jpeg_pixels=(JSAMPLE *) GetVirtualMemoryBlob(memory_info);
  if (setjmp(error_manager.error_recovery) != 0)
    {
      jpeg_destroy_compress(&jpeg_info);
      if (memory_info != (MemoryInfo *) NULL)
        memory_info=RelinquishVirtualMemory(memory_info);
      (void) CloseBlob(image);
      return(MagickFalse);
    }
  scanline[0]=(JSAMPROW) jpeg_pixels;
  scale=65535U/GetQuantumRange(jpeg_info.data_precision);
  if (scale == 0)
    scale=1;
  if (jpeg_info.data_precision <= 8)
    {
      if ((jpeg_info.in_color_space == JCS_RGB) ||
          (jpeg_info.in_color_space == JCS_YCbCr))
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const PixelPacket
            *p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=(JSAMPLE) ScaleQuantumToChar(GetPixelRed(p));
            *q++=(JSAMPLE) ScaleQuantumToChar(GetPixelGreen(p));
            *q++=(JSAMPLE) ScaleQuantumToChar(GetPixelBlue(p));
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
      else
        if (jpeg_info.in_color_space == JCS_GRAYSCALE)
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            register const PixelPacket
              *p;

            register ssize_t
              x;

            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            q=jpeg_pixels;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              *q++=(JSAMPLE) ScaleQuantumToChar(ClampToQuantum(
                GetPixelLuma(image,p)));
              p++;
            }
            (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
            status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
              image->rows);
            if (status == MagickFalse)
              break;
          }
        else
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            register const IndexPacket
              *indexes;

            register const PixelPacket
              *p;

            register ssize_t
              x;

            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            q=jpeg_pixels;
            indexes=GetVirtualIndexQueue(image);
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              /*
                Convert DirectClass packets to contiguous CMYK scanlines.
              */
              *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
                GetPixelCyan(p))));
              *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
                GetPixelMagenta(p))));
              *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
                GetPixelYellow(p))));
              *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
                GetPixelBlack(indexes+x))));
              p++;
            }
            (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
            status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
              image->rows);
            if (status == MagickFalse)
              break;
          }
    }
  else
    if (jpeg_info.in_color_space == JCS_GRAYSCALE)
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        register const PixelPacket
          *p;

        register ssize_t
          x;

        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        q=jpeg_pixels;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          *q++=(JSAMPLE) (ScaleQuantumToShort(ClampToQuantum(
            GetPixelLuma(image,p)))/scale);
          p++;
        }
        (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
    else
      if ((jpeg_info.in_color_space == JCS_RGB) ||
          (jpeg_info.in_color_space == JCS_YCbCr))
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const PixelPacket
            *p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=(JSAMPLE) (ScaleQuantumToShort(GetPixelRed(p))/scale);
            *q++=(JSAMPLE) (ScaleQuantumToShort(GetPixelGreen(p))/scale);
            *q++=(JSAMPLE) (ScaleQuantumToShort(GetPixelBlue(p))/scale);
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
      else
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const IndexPacket
            *indexes;

          register const PixelPacket
            *p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          indexes=GetVirtualIndexQueue(image);
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            /*
              Convert DirectClass packets to contiguous CMYK scanlines.
            */
            *q++=(JSAMPLE) (ScaleQuantumToShort(QuantumRange-GetPixelRed(p))/
              scale);
            *q++=(JSAMPLE) (ScaleQuantumToShort(QuantumRange-GetPixelGreen(p))/
              scale);
            *q++=(JSAMPLE) (ScaleQuantumToShort(QuantumRange-GetPixelBlue(p))/
              scale);
            *q++=(JSAMPLE) (ScaleQuantumToShort(QuantumRange-
              GetPixelIndex(indexes+x))/scale);
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
  if (y == (ssize_t) image->rows)
    jpeg_finish_compress(&jpeg_info);
  /*
    Relinquish resources.
  */
  jpeg_destroy_compress(&jpeg_info);
  memory_info=RelinquishVirtualMemory(memory_info);
  (void) CloseBlob(image);
  return(MagickTrue);
}
#endif

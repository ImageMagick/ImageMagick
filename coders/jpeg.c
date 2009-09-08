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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
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
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/splay-tree.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"
#include <setjmp.h>
#if defined(MAGICKCORE_JPEG_DELEGATE)
#define JPEG_INTERNAL_OPTIONS
#if defined(__MINGW32__)
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
#define MaxBufferExtent  8192

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

static MagickBooleanType EmitMessage(j_common_ptr jpeg_info,int level)
{
  char
    message[JMSG_LENGTH_MAX];

  ErrorManager
    *error_manager;

  Image
    *image;

  (jpeg_info->err->format_message)(jpeg_info,message);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  if (level < 0)
    {
      if ((jpeg_info->err->num_warnings == 0) ||
          (jpeg_info->err->trace_level >= 3))
        ThrowBinaryException(CorruptImageWarning,(char *) message,
          image->filename);
      jpeg_info->err->num_warnings++;
    }
  else
    if (jpeg_info->err->trace_level >= level)
      ThrowBinaryException(CoderError,(char *) message,image->filename);
  return(MagickTrue);
}

static boolean FillInputBuffer(j_decompress_ptr cinfo)
{
  SourceManager
    *source;

  source=(SourceManager *) cinfo->src;
  source->manager.bytes_in_buffer=(size_t)
    ReadBlob(source->image,MaxBufferExtent,source->buffer);
  if (source->manager.bytes_in_buffer == 0)
    {
      if (source->start_of_blob != 0)
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

static void JPEGErrorHandler(j_common_ptr jpeg_info)
{
  ErrorManager
    *error_manager;

  (void) EmitMessage(jpeg_info,0);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  longjmp(error_manager->error_recovery,1);
}

static boolean ReadComment(j_decompress_ptr jpeg_info)
{
  char
    *comment;

  ErrorManager
    *error_manager;

  Image
    *image;

  register char
    *p;

  register long
    i;

  size_t
    length;

  /*
    Determine length of comment.
  */
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
  length+=GetCharacter(jpeg_info);
  length-=2;
  if (length <= 0)
    return(MagickTrue);
  comment=(char *) NULL;
  if (~length >= MaxTextExtent)
    comment=(char *) AcquireQuantumMemory(length+MaxTextExtent,
      sizeof(*comment));
  if (comment == (char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Read comment.
  */
  i=(long) length-1;
  for (p=comment; i-- >= 0; p++)
    *p=(char) GetCharacter(jpeg_info);
  *p='\0';
  (void) SetImageProperty(image,"comment",comment);
  comment=DestroyString(comment);
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

  register long
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
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
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
      for (i=0; i < (long) (length-12); i++)
        (void) GetCharacter(jpeg_info);
      return(MagickTrue);
    }
  (void) GetCharacter(jpeg_info);  /* id */
  (void) GetCharacter(jpeg_info);  /* markers */
  length-=14;
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  profile=AcquireStringInfo(length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  p=GetStringInfoDatum(profile);
  for (i=(long) GetStringInfoLength(profile)-1; i >= 0; i--)
    *p++=(unsigned char) GetCharacter(jpeg_info);
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
      "Profile: ICC, %lu bytes",(unsigned long) length);
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

  register long
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
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
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
      for (i=0; i < (long) length; i++)
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
  profile=AcquireStringInfo(length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  p=GetStringInfoDatum(profile);
  for (i=(long) GetStringInfoLength(profile)-1; i >= 0; i--)
    *p++=(unsigned char) GetCharacter(jpeg_info);
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
      "Profile: iptc, %lu bytes",(unsigned long) length);
  return(MagickTrue);
}

static boolean ReadProfile(j_decompress_ptr jpeg_info)
{
  char
    name[MaxTextExtent];

  ErrorManager
    *error_manager;

  Image
    *image;

  int
    marker;

  MagickBooleanType
    status;

  register long
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
  length=(size_t) ((unsigned long) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  if (length <= 2)
    return(MagickTrue);
  length-=2;
  marker=jpeg_info->unread_marker-JPEG_APP0;
  (void) FormatMagickString(name,MaxTextExtent,"APP%d",marker);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  image=error_manager->image;
  profile=AcquireStringInfo(length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  p=GetStringInfoDatum(profile);
  for (i=(long) GetStringInfoLength(profile)-1; i >= 0; i--)
    *p++=(unsigned char) GetCharacter(jpeg_info);
  if (marker == 1)
    {
      p=GetStringInfoDatum(profile);
      if ((length > 4) && (LocaleNCompare((char *) p,"exif",4) == 0))
        (void) CopyMagickString(name,"exif",MaxTextExtent);
      if ((length > 5) && (LocaleNCompare((char *) p,"http:",5) == 0))
        {
          long
            j;

          /*
            Extract namespace from XMP profile.
          */
          p=GetStringInfoDatum(profile);
          for (j=0; j < (long) GetStringInfoLength(profile); j++)
          {
            if (*p == '\0')
              break;
            p++;
          }
          if (j < (long) GetStringInfoLength(profile))
            (void) DestroyStringInfo(SplitStringInfo(profile,(size_t) (j+1)));
          (void) CopyMagickString(name,"xmp",MaxTextExtent);
        }
    }
  status=SetImageProfile(image,name,profile);
  profile=DestroyStringInfo(profile);
  if (status == MagickFalse)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: %s, %lu bytes",name,(unsigned long) length);
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
  source->manager.next_input_byte+=(size_t) number_bytes;
  source->manager.bytes_in_buffer-=(size_t) number_bytes;
}

static void TerminateSource(j_decompress_ptr cinfo)
{
  cinfo=cinfo;
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
    long
      j,
      qvalue,
      sum;

    register long
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
         long
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

         qvalue=(long) (jpeg_info->quant_tbl_ptrs[0]->quantval[2]+
           jpeg_info->quant_tbl_ptrs[0]->quantval[53]+
           jpeg_info->quant_tbl_ptrs[1]->quantval[0]+
           jpeg_info->quant_tbl_ptrs[1]->quantval[DCTSIZE2-1]);
         for (i=0; i < 100; i++)
         {
           if ((qvalue < hash[i]) && (sum < sums[i]))
             continue;
           if ((qvalue <= hash[i]) && (sum <= sums[i]))
             image->quality=(unsigned long) i+1;
           if (image->debug != MagickFalse)
             {
               if (image->quality != UndefinedCompressionQuality)
                 (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "Quality: %ld",image->quality);
               else
                 (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "Quality: %ld (approximate)",i+1);
             }
           break;
         }
       }
     else
       if (jpeg_info->quant_tbl_ptrs[0] != NULL)
         {
           long
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

           qvalue=(long) (jpeg_info->quant_tbl_ptrs[0]->quantval[2]+
             jpeg_info->quant_tbl_ptrs[0]->quantval[53]);
           for (i=0; i < 100; i++)
           {
             if ((qvalue < hash[i]) && (sum < sums[i]))
               continue;
             image->quality=(unsigned long) i+1;
             if (image->debug != MagickFalse)
               {
                 if ((qvalue > hash[i]) || (sum > sums[i]))
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                     "Quality: %ld (approximate)",i+1);
                 else
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                     "Quality: %ld",i+1);
               }
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
      (void) FormatMagickString(sampling_factor,MaxTextExtent,
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
      (void) FormatMagickString(sampling_factor,MaxTextExtent,"%dx%d",
        jpeg_info->comp_info[0].h_samp_factor,
        jpeg_info->comp_info[0].v_samp_factor);
      break;
    }
    case JCS_RGB:
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Colorspace: RGB");
      (void) FormatMagickString(sampling_factor,MaxTextExtent,
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
      (void) FormatMagickString(sampling_factor,MaxTextExtent,
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
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "Sampling Factors: %s",sampling_factor);
}

static Image *ReadJPEGImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    value[MaxTextExtent];

  ErrorManager
    error_manager;

  IndexPacket
    jindex;

  Image
    *image;

  long
    y;

  JSAMPLE
    *jpeg_pixels;

  JSAMPROW
    scanline[1];

  MagickBooleanType
    debug,
    status;

  MagickSizeType
    number_pixels;

  register long
    i;

  struct jpeg_decompress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  register JSAMPLE
    *p;

  unsigned long
    precision,
    units;

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
  (void) ResetMagickMemory(&jpeg_info,0,sizeof(jpeg_info));
  (void) ResetMagickMemory(&jpeg_error,0,sizeof(jpeg_error));
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=(void (*)(j_common_ptr,int)) EmitMessage;
  jpeg_info.err->error_exit=(void (*)(j_common_ptr)) JPEGErrorHandler;
  jpeg_pixels=(JSAMPLE *) NULL;
  error_manager.image=image;
  if (setjmp(error_manager.error_recovery) != 0)
    {
      jpeg_destroy_decompress(&jpeg_info);
      (void) CloseBlob(image);
      number_pixels=(MagickSizeType) image->columns*image->rows;
      if (number_pixels != 0)
        return(GetFirstImageInList(image));
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
  i=jpeg_read_header(&jpeg_info,MagickTrue);
  if ((image->colorspace == YCbCrColorspace) ||
      (image->colorspace == Rec601YCbCrColorspace) ||
      (image->colorspace == Rec709YCbCrColorspace))
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
      units=(unsigned long) jpeg_info.density_unit;
    }
  if (units == 1)
    image->units=PixelsPerInchResolution;
  if (units == 2)
    image->units=PixelsPerCentimeterResolution;
  number_pixels=(MagickSizeType) image->columns*image->rows;
  if (image_info->size != (char *) NULL)
    {
      double
        scale_factor;

      /*
        Let the JPEG library subsample for us.
      */
      jpeg_calc_output_dimensions(&jpeg_info);
      image->magick_columns=jpeg_info.output_width;
      image->magick_rows=jpeg_info.output_height;
      scale_factor=(double) jpeg_info.output_width/image->columns;
      if (scale_factor > ((double) jpeg_info.output_height/image->rows))
        scale_factor=(double) jpeg_info.output_height/image->rows;
      jpeg_info.scale_num=1U;
      jpeg_info.scale_denom=(unsigned int) scale_factor;
      jpeg_calc_output_dimensions(&jpeg_info);
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Scale factor: %ld",
          (long) scale_factor);
    }
  precision=(unsigned long) jpeg_info.data_precision;
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
  if ((image_info->colors > 8) && (image_info->colors <= 256))
    {
      /*
        Let the JPEG library quantize for us.
      */
      jpeg_info.quantize_colors=MagickTrue;
      jpeg_info.desired_number_of_colors=(int) image_info->colors;
    }
  (void) jpeg_start_decompress(&jpeg_info);
  image->columns=jpeg_info.output_width;
  image->rows=jpeg_info.output_height;
  image->depth=(unsigned long) jpeg_info.data_precision;
  if (jpeg_info.out_color_space == JCS_YCbCr)
    image->colorspace=YCbCrColorspace;
  if (jpeg_info.out_color_space == JCS_CMYK)
    image->colorspace=CMYKColorspace;
  if ((image_info->colors != 0) && (image_info->colors <= 256))
    if (AcquireImageColormap(image,image_info->colors) == MagickFalse)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if ((jpeg_info.output_components == 1) &&
      (jpeg_info.quantize_colors == MagickFalse))
    {
      unsigned long
        colors;

      colors=(unsigned long) GetQuantumRange(image->depth)+1;
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
  (void) FormatMagickString(value,MaxTextExtent,"%ld",(long)
    jpeg_info.out_color_space);
  (void) SetImageProperty(image,"jpeg:colorspace",value);
  if (image_info->ping != MagickFalse)
    {
      jpeg_destroy_decompress(&jpeg_info);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  jpeg_pixels=(JSAMPLE *) AcquireQuantumMemory((size_t) image->columns,
    jpeg_info.output_components*sizeof(JSAMPLE));
  if (jpeg_pixels == (JSAMPLE *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Convert JPEG pixels to pixel packets.
  */
  if (setjmp(error_manager.error_recovery) != 0)
    {
      if (jpeg_pixels != (unsigned char *) NULL)
        jpeg_pixels=(unsigned char *) RelinquishMagickMemory(jpeg_pixels);
      jpeg_destroy_decompress(&jpeg_info);
      (void) CloseBlob(image);
      number_pixels=(MagickSizeType) image->columns*image->rows;
      if (number_pixels != 0)
        return(GetFirstImageInList(image));
      return(DestroyImage(image));
    }
  if (jpeg_info.quantize_colors != MagickFalse)
    {
      image->colors=(unsigned long) jpeg_info.actual_number_of_colors;
      if (jpeg_info.out_color_space == JCS_GRAYSCALE)
        for (i=0; i < (long) image->colors; i++)
        {
          image->colormap[i].red=ScaleCharToQuantum(jpeg_info.colormap[0][i]);
          image->colormap[i].green=image->colormap[i].red;
          image->colormap[i].blue=image->colormap[i].red;
          image->colormap[i].opacity=OpaqueOpacity;
        }
      else
        for (i=0; i < (long) image->colors; i++)
        {
          image->colormap[i].red=ScaleCharToQuantum(jpeg_info.colormap[0][i]);
          image->colormap[i].green=ScaleCharToQuantum(jpeg_info.colormap[1][i]);
          image->colormap[i].blue=ScaleCharToQuantum(jpeg_info.colormap[2][i]);
          image->colormap[i].opacity=OpaqueOpacity;
        }
    }
  scanline[0]=(JSAMPROW) jpeg_pixels;
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

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
        if (jpeg_info.output_components == 1)
          for (x=0; x < (long) image->columns; x++)
          {
            unsigned long
              pixel;

            if (precision != 16)
              pixel=(unsigned long) GETJSAMPLE(*p);
            else
              pixel=(unsigned long) ((GETJSAMPLE(*p) ^ 0x80) << 4);
            jindex=ConstrainColormapIndex(image,pixel);
            indexes[x]=jindex;
            *q++=image->colormap[(int) jindex];
            p++;
          }
        else
          if (image->colorspace != CMYKColorspace)
            for (x=0; x < (long) image->columns; x++)
            {
              q->red=ScaleShortToQuantum((unsigned char)
                (GETJSAMPLE(*p++) << 4));
              q->green=ScaleShortToQuantum((unsigned char)
                (GETJSAMPLE(*p++) << 4));
              q->blue=ScaleShortToQuantum((unsigned char)
                (GETJSAMPLE(*p++) << 4));
              q->opacity=OpaqueOpacity;
              q++;
            }
          else
            for (x=0; x < (long) image->columns; x++)
            {
              q->red=(Quantum) QuantumRange-ScaleShortToQuantum((unsigned char)
                (GETJSAMPLE(*p++) << 4));
              q->green=(Quantum) QuantumRange-ScaleShortToQuantum(
                (unsigned char) (GETJSAMPLE(*p++) << 4));
              q->blue=(Quantum) QuantumRange-ScaleShortToQuantum((unsigned char)
                (GETJSAMPLE(*p++) << 4));
              q->opacity=OpaqueOpacity;
              indexes[x]=(IndexPacket) QuantumRange-ScaleShortToQuantum(
                (unsigned char) (GETJSAMPLE(*p++) << 4));
              q++;
            }
      }
    else
      if (jpeg_info.output_components == 1)
        for (x=0; x < (long) image->columns; x++)
        {
          jindex=ConstrainColormapIndex(image,(unsigned long) GETJSAMPLE(*p));
          indexes[x]=(IndexPacket) jindex;
          *q++=image->colormap[(int) jindex];
          p++;
        }
      else
        if (image->colorspace != CMYKColorspace)
          for (x=0; x < (long) image->columns; x++)
          {
            q->red=ScaleCharToQuantum((unsigned char) GETJSAMPLE(*p++));
            q->green=ScaleCharToQuantum((unsigned char) GETJSAMPLE(*p++));
            q->blue=ScaleCharToQuantum((unsigned char) GETJSAMPLE(*p++));
            q->opacity=OpaqueOpacity;
            q++;
          }
        else
          for (x=0; x < (long) image->columns; x++)
          {
            q->red=(Quantum) QuantumRange-ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++));
            q->green=(Quantum) QuantumRange-ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++));
            q->blue=(Quantum) QuantumRange-ScaleCharToQuantum((unsigned char)
              GETJSAMPLE(*p++));
            q->opacity=OpaqueOpacity;
            indexes[x]=(IndexPacket) QuantumRange-ScaleCharToQuantum(
              (unsigned char) GETJSAMPLE(*p++));
            q++;
          }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  /*
    Free jpeg resources.
  */
  (void) jpeg_finish_decompress(&jpeg_info);
  jpeg_destroy_decompress(&jpeg_info);
  jpeg_pixels=(unsigned char *) RelinquishMagickMemory(jpeg_pixels);
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
%      unsigned long RegisterJPEGImage(void)
%
*/
ModuleExport unsigned long RegisterJPEGImage(void)
{
  char
    version[MaxTextExtent];

  MagickInfo
    *entry;

  static const char
    description[] = "Joint Photographic Experts Group JFIF format";

  *version='\0';
#if defined(JPEG_LIB_VERSION)
  (void) FormatMagickString(version,MaxTextExtent,"%d",JPEG_LIB_VERSION);
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
%      MagickBooleanType WriteJPEGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o jpeg_image:  The image.
%
%
*/

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

  register long
    i;

  size_t
    length;

  StringInfo
    *custom_profile;

  unsigned long
    tag_length;

  /*
    Save image profile as a APP marker.
  */
  iptc=MagickFalse;
  custom_profile=AcquireStringInfo(65535L);
  ResetImageProfileIterator(image);
  for (name=GetNextImageProfile(image); name != (const char *) NULL; )
  {
    profile=GetImageProfile(image,name);
    if (LocaleCompare(name,"EXIF") == 0)
      for (i=0; i < (long) GetStringInfoLength(profile); i+=65533L)
      {
        length=MagickMin(GetStringInfoLength(profile)-i,65533L);
        jpeg_write_marker(jpeg_info,XML_MARKER,GetStringInfoDatum(profile)+i,
          (unsigned int) length);
      }
    if (LocaleCompare(name,"ICC") == 0)
      {
        register unsigned char
          *p;

        tag_length=14;
        p=GetStringInfoDatum(custom_profile);
        (void) CopyMagickMemory(p,ICC_PROFILE,tag_length);
        for (i=0; i < (long) GetStringInfoLength(profile); i+=65519L)
        {
          length=MagickMin(GetStringInfoLength(profile)-i,65519L);
          p=GetStringInfoDatum(custom_profile);
          p[12]=(unsigned char) ((i/65519L)+1);
          p[13]=(unsigned char) (GetStringInfoLength(profile)/65519L+1);
          (void) CopyMagickMemory(p+tag_length,GetStringInfoDatum(profile)+i,
            length);
          jpeg_write_marker(jpeg_info,ICC_MARKER,GetStringInfoDatum(
            custom_profile),(unsigned int) (length+tag_length));
        }
      }
    if (((LocaleCompare(name,"IPTC") == 0) ||
        (LocaleCompare(name,"8BIM") == 0)) && (iptc == MagickFalse))
      {
        register unsigned char
          *p;

        unsigned long
          roundup;

        iptc=MagickTrue;
        p=GetStringInfoDatum(custom_profile);
        if (LocaleNCompare((char *) GetStringInfoDatum(profile),"8BIM",4) == 0)
          {
            (void) CopyMagickMemory(p,"Photoshop 3.0\0",14);
            tag_length=14;
          }
        else
          {
            (void) CopyMagickMemory(p,"Photoshop 3.0\08BIM\04\04\0\0\0\0",24);
            p[13]=0x00;
            p[24]=(unsigned char) (GetStringInfoLength(profile) >> 8);
            p[25]=(unsigned char) (GetStringInfoLength(profile) & 0xff);
            tag_length=26;
          }
        for (i=0; i < (long) GetStringInfoLength(profile); i+=65500L)
        {
          length=MagickMin(GetStringInfoLength(profile)-i,65500L);
          roundup=(unsigned long) (length & 0x01);
          (void) CopyMagickMemory(p+tag_length,GetStringInfoDatum(profile)+i,
            length);
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
        xmp_profile=StringToStringInfo("http://ns.adobe.com/xap/1.0/");
        ConcatenateStringInfo(xmp_profile,profile);
        GetStringInfoDatum(xmp_profile)[28]='\0';
        for (i=0; i < (long) GetStringInfoLength(xmp_profile); i+=65533L)
        {
          length=MagickMin(GetStringInfoLength(xmp_profile)-i,65533L);
          jpeg_write_marker(jpeg_info,XML_MARKER,
            GetStringInfoDatum(xmp_profile)+i,(unsigned int) length);
        }
        xmp_profile=DestroyStringInfo(xmp_profile);
      }
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s profile: %lu bytes",
      name,(unsigned long) GetStringInfoLength(profile));
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

  register long
    i;

  unsigned long
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
  for (i=0; i < (long) lines; i++)
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

  JSAMPLE
    *jpeg_pixels;

  JSAMPROW
    scanline[1];

  long
    y;

  MagickBooleanType
    status;

  register JSAMPLE
    *q;

  register long
    i;

  struct jpeg_compress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

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
  (void) ResetMagickMemory(&jpeg_info,0,sizeof(jpeg_info));
  (void) ResetMagickMemory(&jpeg_error,0,sizeof(jpeg_error));
  jpeg_info.client_data=(void *) image;
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=(void (*)(j_common_ptr,int)) EmitMessage;
  jpeg_info.err->error_exit=(void (*)(j_common_ptr)) JPEGErrorHandler;
  error_manager.image=image;
  jpeg_pixels=(JSAMPLE *) NULL;
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
      break;
  }
  if ((image_info->type != TrueColorType) &&
      (IsGrayImage(image,&image->exception) != MagickFalse))
    {
      jpeg_info.input_components=1;
      jpeg_info.in_color_space=JCS_GRAYSCALE;
    }
  jpeg_set_defaults(&jpeg_info);
  if ((jpeg_info.data_precision != 12) && (image->depth <= 8))
    jpeg_info.data_precision=8;
  else
    if (sizeof(JSAMPLE) > 1)
      jpeg_info.data_precision=12;
  jpeg_info.density_unit=(UINT8) 1;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Image resolution: %ld,%ld",(long) (image->x_resolution+0.5),
      (long) (image->y_resolution+0.5));
  if ((image->x_resolution != 0.0) && (image->y_resolution != 0.0))
    {
      /*
        Set image resolution.
      */
      jpeg_info.write_JFIF_header=MagickTrue;
      jpeg_info.X_density=(UINT16) image->x_resolution;
      jpeg_info.Y_density=(UINT16) image->y_resolution;
      if (image->units == PixelsPerInchResolution)
        jpeg_info.density_unit=(UINT8) 1;
      if (image->units == PixelsPerCentimeterResolution)
        jpeg_info.density_unit=(UINT8) 2;
    }
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
    {
      jpeg_info.optimize_coding=MagickFalse;
      if (IsMagickTrue(option) != MagickFalse)
        jpeg_info.optimize_coding=MagickTrue;
    }
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
          if (status != MagickFalse)
            jpeg_info.optimize_coding=MagickTrue;
          RelinquishMagickResource(MemoryResource,length);
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
  if ((image_info->compression != LosslessJPEGCompression) &&
      (image->quality <= 100))
    {
      if (image->quality == UndefinedCompressionQuality)
        jpeg_set_quality(&jpeg_info,92,MagickTrue);
      else
        jpeg_set_quality(&jpeg_info,(int) image->quality,MagickTrue);
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Quality: %lu",
          image->quality);
    }
  else
    {
#if !defined(C_LOSSLESS_SUPPORTED)
      jpeg_set_quality(&jpeg_info,100,MagickTrue);
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
  sampling_factor=(const char *) NULL;
  value=GetImageProperty(image,"jpeg:sampling-factor");
  if (value != (char *) NULL)
    {
      sampling_factor=value;
      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Input sampling-factors=%s",sampling_factor);
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
  if (jpeg_info.input_components == 1)
    for (i=0; i < MAX_COMPONENTS; i++)
    {
      jpeg_info.comp_info[i].h_samp_factor=1;
      jpeg_info.comp_info[i].v_samp_factor=1;
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
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Depth: %lu",
        image->depth);
      if (image->colors != 0)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Number of colors: %lu",image->colors);
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
    for (i=0; i < (long) strlen(value); i+=65533L)
      jpeg_write_marker(&jpeg_info,JPEG_COM,(unsigned char *) value+i,
        (unsigned int) MagickMin((size_t) strlen(value+i),65533L));
  if (image->profiles != (void *) NULL)
    WriteProfile(&jpeg_info,image);
  /*
    Convert MIFF to JPEG raster pixels.
  */
  jpeg_pixels=(JSAMPLE *) AcquireQuantumMemory((size_t) image->columns,
    jpeg_info.input_components*sizeof(*jpeg_pixels));
  if (jpeg_pixels == (JSAMPLE *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  if (setjmp(error_manager.error_recovery) != 0)
    {
      jpeg_destroy_compress(&jpeg_info);
      if (jpeg_pixels != (unsigned char *) NULL)
        jpeg_pixels=(unsigned char *) RelinquishMagickMemory(jpeg_pixels);
      (void) CloseBlob(image);
      return(MagickFalse);
    }
  scanline[0]=(JSAMPROW) jpeg_pixels;
  if (jpeg_info.data_precision > 8)
    {
      if (jpeg_info.in_color_space == JCS_GRAYSCALE)
        for (y=0; y < (long) image->rows; y++)
        {
          register const PixelPacket
            *p;

          register long
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          for (x=0; x < (long) image->columns; x++)
          {
            *q++=(JSAMPLE) (ScaleQuantumToShort(PixelIntensityToQuantum(p)) >>
              4);
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          if (SetImageProgress(image,SaveImageTag,y,image->rows) == MagickFalse)
            break;
        }
      else
        if ((jpeg_info.in_color_space == JCS_RGB) ||
            (jpeg_info.in_color_space == JCS_YCbCr))
          for (y=0; y < (long) image->rows; y++)
          {
            register const PixelPacket
              *p;

            register long
              x;

            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            q=jpeg_pixels;
            for (x=0; x < (long) image->columns; x++)
            {
              *q++=(JSAMPLE) (ScaleQuantumToShort(p->red) >> 4);
              *q++=(JSAMPLE) (ScaleQuantumToShort(p->green) >> 4);
              *q++=(JSAMPLE) (ScaleQuantumToShort(p->blue) >> 4);
              p++;
            }
            (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
            status=SetImageProgress(image,SaveImageTag,y,image->rows);
            if (status == MagickFalse)
              break;
          }
        else
          for (y=0; y < (long) image->rows; y++)
          {
            register const IndexPacket
              *indexes;

            register const PixelPacket
              *p;

            register long
              x;

            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            q=jpeg_pixels;
            indexes=GetVirtualIndexQueue(image);
            for (x=0; x < (long) image->columns; x++)
            {
              /*
                Convert DirectClass packets to contiguous CMYK scanlines.
              */
              *q++=(JSAMPLE) (4095-(ScaleQuantumToShort(p->red) >> 4));
              *q++=(JSAMPLE) (4095-(ScaleQuantumToShort(p->green) >> 4));
              *q++=(JSAMPLE) (4095-(ScaleQuantumToShort(p->blue) >> 4));
              *q++=(JSAMPLE) (4095-(ScaleQuantumToShort(indexes[x]) >> 4));
              p++;
            }
            (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
            status=SetImageProgress(image,SaveImageTag,y,image->rows);
            if (status == MagickFalse)
              break;
          }
    }
  else
    if (jpeg_info.in_color_space == JCS_GRAYSCALE)
      for (y=0; y < (long) image->rows; y++)
      {
        register const PixelPacket
          *p;

        register long
          x;

        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        q=jpeg_pixels;
        for (x=0; x < (long) image->columns; x++)
        {
          *q++=(JSAMPLE) ScaleQuantumToChar(PixelIntensityToQuantum(p));
          p++;
        }
        (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
        if (SetImageProgress(image,SaveImageTag,y,image->rows) == MagickFalse)
          break;
      }
    else
      if ((jpeg_info.in_color_space == JCS_RGB) ||
          (jpeg_info.in_color_space == JCS_YCbCr))
        for (y=0; y < (long) image->rows; y++)
        {
          register const PixelPacket
            *p;

          register long
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          for (x=0; x < (long) image->columns; x++)
          {
            *q++=(JSAMPLE) ScaleQuantumToChar(p->red);
            *q++=(JSAMPLE) ScaleQuantumToChar(p->green);
            *q++=(JSAMPLE) ScaleQuantumToChar(p->blue);
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          if (SetImageProgress(image,SaveImageTag,y,image->rows) == MagickFalse)
            break;
        }
      else
        for (y=0; y < (long) image->rows; y++)
        {
          register const IndexPacket
            *indexes;

          register const PixelPacket
            *p;

          register long
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          indexes=GetVirtualIndexQueue(image);
          for (x=0; x < (long) image->columns; x++)
          {
            /*
              Convert DirectClass packets to contiguous CMYK scanlines.
            */
            *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
              p->red)));
            *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
              p->green)));
            *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
              p->blue)));
            *q++=(JSAMPLE) (ScaleQuantumToChar((Quantum) (QuantumRange-
              indexes[x])));
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          if (SetImageProgress(image,SaveImageTag,y,image->rows) == MagickFalse)
            break;
        }
  if (y == (long) image->rows)
    jpeg_finish_compress(&jpeg_info);
  /*
    Relinquish resources.
  */
  jpeg_destroy_compress(&jpeg_info);
  jpeg_pixels=(unsigned char *) RelinquishMagickMemory(jpeg_pixels);
  (void) CloseBlob(image);
  return(MagickTrue);
}
#endif

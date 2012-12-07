/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              JJJ  PPPP    222                               %
%                               J   P   P  2   2                              %
%                               J   PPPP     22                               %
%                            J  J   P       2                                 %
%                             JJ    P      22222                              %
%                                                                             %
%                                                                             %
%                     Read/Write JPEG-2000 Image Format                       %
%                                                                             %
%                                John Cristy                                  %
%                                Nathan Brown                                 %
%                                 June 2001                                   %
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
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/profile.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/module.h"
#if defined(MAGICKCORE_JP2_DELEGATE)
#ifndef JAS_IMAGE_CM_GRAY
#define JAS_IMAGE_CM_GRAY JAS_IMAGE_CS_GRAY
#endif
#ifndef JAS_IMAGE_CM_RGB
#define JAS_IMAGE_CM_RGB JAS_IMAGE_CS_RGB
#endif
#if !defined(uchar)
#define uchar  unsigned char
#endif
#if !defined(ushort)
#define ushort  unsigned short
#endif
#if !defined(uint)
#define uint  unsigned int
#endif
#if !defined(ssize_tssize_t)
#define ssize_tssize_t  long long
#endif
#if !defined(ussize_tssize_t)
#define ussize_tssize_t  unsigned long long
#endif

#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include "jasper/jasper.h"
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#endif

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_JP2_DELEGATE)
static MagickBooleanType
  WriteJP2Image(const ImageInfo *,Image *);

static volatile MagickBooleanType
  instantiate_jp2 = MagickFalse;
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J P 2                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJP2() returns MagickTrue if the image format type, identified by the
%  magick string, is JP2.
%
%  The format of the IsJP2 method is:
%
%      MagickBooleanType IsJP2(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsJP2(const unsigned char *magick,const size_t length)
{
  if (length < 9)
    return(MagickFalse);
  if (memcmp(magick+4,"\152\120\040\040\015",5) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J P C                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJPC()() returns MagickTrue if the image format type, identified by the
%  magick string, is JPC.
%
%  The format of the IsJPC method is:
%
%      MagickBooleanType IsJPC(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsJPC(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if (memcmp(magick,"\377\117",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J P 2 I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJP2Image() reads a JPEG 2000 Image file (JP2) or JPEG 2000
%  codestream (JPC) image file and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image or set of images.
%
%  JP2 support is originally written by Nathan Brown, nathanbrown@letu.edu.
%
%  The format of the ReadJP2Image method is:
%
%      Image *ReadJP2Image(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
#if defined(MAGICKCORE_JP2_DELEGATE)

typedef struct _StreamManager
{
  jas_stream_t
    *stream;

  Image
    *image;
} StreamManager;

static int BlobRead(jas_stream_obj_t *object,char *buffer,const int length)
{
  ssize_t
    count;

  StreamManager
    *source;

  source=(StreamManager *) object;
  count=ReadBlob(source->image,(size_t) length,(unsigned char *) buffer);
  return((int) count);
}

static int BlobWrite(jas_stream_obj_t *object,char *buffer,const int length)
{
  ssize_t
    count;

  StreamManager
    *source;

  source=(StreamManager *) object;
  count=WriteBlob(source->image,(size_t) length,(unsigned char *) buffer);
  return((int) count);
}

static long BlobSeek(jas_stream_obj_t *object,long offset,int origin)
{
  StreamManager
    *source;

  source=(StreamManager *) object;
  return((long) SeekBlob(source->image,offset,origin));
}

static int BlobClose(jas_stream_obj_t *object)
{
  StreamManager
    *source;

  source=(StreamManager *) object;
  (void) CloseBlob(source->image);
  free(source);
  source=(StreamManager *) NULL;
  return(0);
}

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

static jas_stream_t *JP2StreamManager(Image *image)
{
  static jas_stream_ops_t
    StreamOperators =
    {
      BlobRead,
      BlobWrite,
      BlobSeek,
      BlobClose
    };

  jas_stream_t
    *stream;

  StreamManager
    *source;

  stream=(jas_stream_t *) jas_malloc(sizeof(*stream));
  if (stream == (jas_stream_t *) NULL)
    return((jas_stream_t *) NULL);
  (void) ResetMagickMemory(stream,0,sizeof(*stream));
  stream->rwlimit_=(-1);
  stream->obj_=(jas_stream_obj_t *) jas_malloc(sizeof(StreamManager));
  if (stream->obj_ == (jas_stream_obj_t *) NULL)
    return((jas_stream_t *) NULL);
  (void) ResetMagickMemory(stream->obj_,0,sizeof(StreamManager));
  stream->ops_=(&StreamOperators);
  stream->openmode_=JAS_STREAM_READ | JAS_STREAM_WRITE | JAS_STREAM_BINARY;
  stream->bufbase_=(unsigned char *) jas_malloc(JAS_STREAM_BUFSIZE+
    JAS_STREAM_MAXPUTBACK);
  if (stream->bufbase_ == (void *) NULL)
    {
      stream->bufbase_=stream->tinybuf_;
      stream->bufsize_=1;
    }
  else
    {
      stream->bufmode_=JAS_STREAM_FREEBUF | JAS_STREAM_BUFMODEMASK;
      stream->bufsize_=JAS_STREAM_BUFSIZE;
    }
  stream->bufstart_=(&stream->bufbase_[JAS_STREAM_MAXPUTBACK]);
  stream->ptr_=stream->bufstart_;
  stream->cnt_=0;
  source=(StreamManager *) stream->obj_;
  source->image=image;
  return(stream);
}

static Image *ReadJP2Image(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  jas_cmprof_t
    *cm_profile;

  jas_iccprof_t
    *icc_profile;

  jas_image_t
    *jp2_image;

  jas_matrix_t
    *pixels[4];

  jas_stream_t
    *jp2_stream;

  MagickBooleanType
    status;

  QuantumAny
    pixel,
    range[4];

  register PixelPacket
    *q;

  register ssize_t
    i,
    x;

  size_t
    maximum_component_depth,
    number_components,
    x_step[4],
    y_step[4];

  ssize_t
    components[4],
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
    Initialize JPEG 2000 API.
  */
  jp2_stream=JP2StreamManager(image);
  if (jp2_stream == (jas_stream_t *) NULL)
    ThrowReaderException(DelegateError,"UnableToManageJP2Stream");
  jp2_image=jas_image_decode(jp2_stream,-1,0);
  if (jp2_image == (jas_image_t *) NULL)
    {
      (void) jas_stream_close(jp2_stream);
      ThrowReaderException(DelegateError,"UnableToDecodeImageFile");
    }
  image->columns=jas_image_width(jp2_image);
  image->rows=jas_image_height(jp2_image);
  image->compression=JPEG2000Compression;
  switch (jas_clrspc_fam(jas_image_clrspc(jp2_image)))
  {
    case JAS_CLRSPC_FAM_RGB:
    {
      SetImageColorspace(image,RGBColorspace);
      components[0]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_RGB_R);
      components[1]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_RGB_G);
      components[2]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_RGB_B);
      if ((components[0] < 0) || (components[1] < 0) || (components[2] < 0))
        {
          (void) jas_stream_close(jp2_stream);
          jas_image_destroy(jp2_image);
          ThrowReaderException(CorruptImageError,"MissingImageChannel");
        }
      number_components=3;
      components[3]=jas_image_getcmptbytype(jp2_image,3);
      if (components[3] > 0)
        {
          image->matte=MagickTrue;
          number_components++;
        }
      break;
    }
    case JAS_CLRSPC_FAM_GRAY:
    {
      SetImageColorspace(image,GRAYColorspace);
      components[0]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_GRAY_Y);
      if (components[0] < 0)
        {
          (void) jas_stream_close(jp2_stream);
          jas_image_destroy(jp2_image);
          ThrowReaderException(CorruptImageError,"MissingImageChannel");
        }
      number_components=1;
      break;
    }
    case JAS_CLRSPC_FAM_YCBCR:
    {
      SetImageColorspace(image,YCbCrColorspace);
      components[0]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_YCBCR_Y);
      components[1]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_YCBCR_CB);
      components[2]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_YCBCR_CR);
      if ((components[0] < 0) || (components[1] < 0) || (components[2] < 0))
        {
          (void) jas_stream_close(jp2_stream);
          jas_image_destroy(jp2_image);
          ThrowReaderException(CorruptImageError,"MissingImageChannel");
        }
      number_components=3;
      components[3]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_UNKNOWN);
      if (components[3] > 0)
        {
          image->matte=MagickTrue;
          number_components++;
        }
      break;
    }
    case JAS_CLRSPC_FAM_XYZ:
    {
      SetImageColorspace(image,XYZColorspace);
      components[0]=jas_image_getcmptbytype(jp2_image,0);
      components[1]=jas_image_getcmptbytype(jp2_image,1);
      components[2]=jas_image_getcmptbytype(jp2_image,2);
      if ((components[0] < 0) || (components[1] < 0) || (components[2] < 0))
        {
          (void) jas_stream_close(jp2_stream);
          jas_image_destroy(jp2_image);
          ThrowReaderException(CorruptImageError,"MissingImageChannel");
        }
      number_components=3;
      components[3]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_UNKNOWN);
      if (components[3] > 0)
        {
          image->matte=MagickTrue;
          number_components++;
        }
      break;
    }
    case JAS_CLRSPC_FAM_LAB:
    {
      SetImageColorspace(image,YCbCrColorspace);
      components[0]=jas_image_getcmptbytype(jp2_image,0);
      components[1]=jas_image_getcmptbytype(jp2_image,1);
      components[2]=jas_image_getcmptbytype(jp2_image,2);
      if ((components[0] < 0) || (components[1] < 0) || (components[2] < 0))
        {
          (void) jas_stream_close(jp2_stream);
          jas_image_destroy(jp2_image);
          ThrowReaderException(CorruptImageError,"MissingImageChannel");
        }
      number_components=3;
      components[3]=jas_image_getcmptbytype(jp2_image,JAS_IMAGE_CT_UNKNOWN);
      if (components[3] > 0)
        {
          image->matte=MagickTrue;
          number_components++;
        }
      break;
    }
    default:
    {
      (void) jas_stream_close(jp2_stream);
      jas_image_destroy(jp2_image);
      ThrowReaderException(CoderError,"ColorspaceModelIsNotSupported");
    }
  }
  for (i=0; i < (ssize_t) number_components; i++)
  {
    size_t
      height,
      width;

    width=(size_t) (jas_image_cmptwidth(jp2_image,components[i])*
      jas_image_cmpthstep(jp2_image,components[i]));
    height=(size_t) (jas_image_cmptheight(jp2_image,components[i])*
      jas_image_cmptvstep(jp2_image,components[i]));
    x_step[i]=(unsigned int) jas_image_cmpthstep(jp2_image,components[i]);
    y_step[i]=(unsigned int) jas_image_cmptvstep(jp2_image,components[i]);
    if ((width != image->columns) || (height != image->rows) ||
        (jas_image_cmpttlx(jp2_image,components[i]) != 0) ||
        (jas_image_cmpttly(jp2_image,components[i]) != 0) ||
        (jas_image_cmptsgnd(jp2_image,components[i]) != MagickFalse))
      {
        (void) jas_stream_close(jp2_stream);
        jas_image_destroy(jp2_image);
        ThrowReaderException(CoderError,"IrregularChannelGeometryNotSupported");
      }
  }
  /*
    Convert JPEG 2000 pixels.
  */
  image->matte=number_components > 3 ? MagickTrue : MagickFalse;
  maximum_component_depth=0;
  for (i=0; i < (ssize_t) number_components; i++)
  {
    maximum_component_depth=(unsigned int) MagickMax((size_t)
      jas_image_cmptprec(jp2_image,components[i]),(size_t)
      maximum_component_depth);
    pixels[i]=jas_matrix_create(1,(int) (image->columns/x_step[i]));
    if (pixels[i] == (jas_matrix_t *) NULL)
      {
        for (--i; i >= 0; i--)
          jas_matrix_destroy(pixels[i]);
        jas_image_destroy(jp2_image);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
  }
  image->depth=maximum_component_depth;
  if (image_info->ping != MagickFalse)
    {
      (void) jas_stream_close(jp2_stream);
      jas_image_destroy(jp2_image);
      return(GetFirstImageInList(image));
    }
  for (i=0; i < (ssize_t) number_components; i++)
    range[i]=GetQuantumRange((size_t) jas_image_cmptprec(jp2_image,
      components[i]));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (i=0; i < (ssize_t) number_components; i++)
      (void) jas_image_readcmpt(jp2_image,(short) components[i],0,
        (jas_image_coord_t) (y/y_step[i]),(jas_image_coord_t) (image->columns/
        x_step[i]),1,pixels[i]);
    switch (number_components)
    {
      case 1:
      {
        /*
          Grayscale.
        */
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          pixel=(QuantumAny) jas_matrix_getv(pixels[0],x/x_step[0]);
          SetPixelRed(q,ScaleAnyToQuantum((QuantumAny) pixel,
            range[0]));
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
        break;
      }
      case 3:
      {
        /*
          RGB.
        */
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          pixel=(QuantumAny) jas_matrix_getv(pixels[0],x/x_step[0]);
          SetPixelRed(q,ScaleAnyToQuantum((QuantumAny) pixel,range[0]));
          pixel=(QuantumAny) jas_matrix_getv(pixels[1],x/x_step[1]);
          SetPixelGreen(q,ScaleAnyToQuantum((QuantumAny) pixel,range[1]));
          pixel=(QuantumAny) jas_matrix_getv(pixels[2],x/x_step[2]);
          SetPixelBlue(q,ScaleAnyToQuantum((QuantumAny) pixel,range[2]));
          q++;
        }
        break;
      }
      case 4:
      {
        /*
          RGBA.
        */
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          pixel=(QuantumAny) jas_matrix_getv(pixels[0],x/x_step[0]);
          SetPixelRed(q,ScaleAnyToQuantum((QuantumAny) pixel,range[0]));
          pixel=(QuantumAny) jas_matrix_getv(pixels[1],x/x_step[1]);
          SetPixelGreen(q,ScaleAnyToQuantum((QuantumAny) pixel,range[1]));
          pixel=(QuantumAny) jas_matrix_getv(pixels[2],x/x_step[2]);
          SetPixelBlue(q,ScaleAnyToQuantum((QuantumAny) pixel,range[2]));
          pixel=(QuantumAny) jas_matrix_getv(pixels[3],x/x_step[3]);
          SetPixelAlpha(q,ScaleAnyToQuantum((QuantumAny) pixel,range[3]));
          q++;
        }
        break;
      }
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  cm_profile=jas_image_cmprof(jp2_image);
  icc_profile=(jas_iccprof_t *) NULL;
  if (cm_profile != (jas_cmprof_t *) NULL)
    icc_profile=jas_iccprof_createfromcmprof(cm_profile);
  if (icc_profile != (jas_iccprof_t *) NULL)
    {
      jas_stream_t
        *icc_stream;

      icc_stream=jas_stream_memopen(NULL,0);
      if ((icc_stream != (jas_stream_t *) NULL) &&
          (jas_iccprof_save(icc_profile,icc_stream) == 0) &&
          (jas_stream_flush(icc_stream) == 0))
        {
          jas_stream_memobj_t
            *blob;

          StringInfo
            *icc_profile,
            *profile;

          /*
            Extract the icc profile, handle errors without much noise.
          */
          blob=(jas_stream_memobj_t *) icc_stream->obj_;
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "Profile: ICC, %.20g bytes",(double) blob->len_);
          profile=BlobToStringInfo(blob->buf_,blob->len_);
          if (profile == (StringInfo *) NULL)
            ThrowReaderException(CorruptImageError,"MemoryAllocationFailed");
          icc_profile=(StringInfo *) GetImageProfile(image,"icc");
          if (icc_profile == (StringInfo *) NULL)
            (void) SetImageProfile(image,"icc",profile);
          else
            (void) ConcatenateStringInfo(icc_profile,profile);
          profile=DestroyStringInfo(profile);
          (void) jas_stream_close(icc_stream);
        }
    }
  (void) jas_stream_close(jp2_stream);
  jas_image_destroy(jp2_image);
  for (i=0; i < (ssize_t) number_components; i++)
    jas_matrix_destroy(pixels[i]);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r J P 2 I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJP2Image() adds attributes for the JP2 image format to the list of
%  supported formats.  The attributes include the image format tag, a method
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterJP2Image method is:
%
%      size_t RegisterJP2Image(void)
%
*/
ModuleExport size_t RegisterJP2Image(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("JP2");
  entry->description=ConstantString("JPEG-2000 File Format Syntax");
  entry->module=ConstantString("JP2");
  entry->magick=(IsImageFormatHandler *) IsJP2;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("JPC");
  entry->description=ConstantString("JPEG-2000 Code Stream Syntax");
  entry->module=ConstantString("JP2");
  entry->magick=(IsImageFormatHandler *) IsJPC;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("J2C");
  entry->description=ConstantString("JPEG-2000 Code Stream Syntax");
  entry->module=ConstantString("JP2");
  entry->magick=(IsImageFormatHandler *) IsJPC;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("J2K");
  entry->description=ConstantString("JPEG-2000 Code Stream Syntax");
  entry->module=ConstantString("JP2");
  entry->magick=(IsImageFormatHandler *) IsJPC;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("JPX");
  entry->description=ConstantString("JPEG-2000 File Format Syntax");
  entry->module=ConstantString("JP2");
  entry->magick=(IsImageFormatHandler *) IsJPC;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
  entry->encoder=(EncodeImageHandler *) WriteJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PGX");
  entry->description=ConstantString("JPEG-2000 VM Format");
  entry->module=ConstantString("JP2");
  entry->magick=(IsImageFormatHandler *) IsJPC;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=NoThreadSupport;
#if defined(MAGICKCORE_JP2_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJP2Image;
#endif
  (void) RegisterMagickInfo(entry);
#if defined(MAGICKCORE_JP2_DELEGATE)
  if (instantiate_jp2 == MagickFalse)
    {
      jas_init();
      instantiate_jp2=MagickTrue;
    }
#endif
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r J P 2 I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJP2Image() removes format registrations made by the JP2 module
%  from the list of supported formats.
%
%  The format of the UnregisterJP2Image method is:
%
%      UnregisterJP2Image(void)
%
*/
ModuleExport void UnregisterJP2Image(void)
{
  (void) UnregisterMagickInfo("PGX");
  (void) UnregisterMagickInfo("J2K");
  (void) UnregisterMagickInfo("J2C");
  (void) UnregisterMagickInfo("JPC");
  (void) UnregisterMagickInfo("JP2");
#if defined(MAGICKCORE_JP2_DELEGATE)
  if (instantiate_jp2 != MagickFalse)
    {
      jas_cleanup();
      instantiate_jp2=MagickFalse;
    }
#endif
}

#if defined(MAGICKCORE_JP2_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e J P 2 I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJP2Image() writes an image in the JPEG 2000 image format.
%
%  JP2 support originally written by Nathan Brown, nathanbrown@letu.edu
%
%  The format of the WriteJP2Image method is:
%
%      MagickBooleanType WriteJP2Image(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteJP2Image(const ImageInfo *image_info,Image *image)
{
  char
    *key,
    magick[MaxTextExtent],
    *options;

  const char
    *option;

  jas_image_cmptparm_t
    component_info[4];

  jas_image_t
    *jp2_image;

  jas_matrix_t
    *pixels[4];

  jas_stream_t
    *jp2_stream;

  MagickBooleanType
    status;

  QuantumAny
    range;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  size_t
    number_components;

  ssize_t
    format,
    y;

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
    Initialize JPEG 2000 API.
  */
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  jp2_stream=JP2StreamManager(image);
  if (jp2_stream == (jas_stream_t *) NULL)
    ThrowWriterException(DelegateError,"UnableToManageJP2Stream");
  number_components=image->matte ? 4UL : 3UL;
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    number_components=1;
  if ((image->columns != (unsigned int) image->columns) ||
      (image->rows != (unsigned int) image->rows))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  (void) ResetMagickMemory(&component_info,0,sizeof(component_info));
  for (i=0; i < (ssize_t) number_components; i++)
  {
    component_info[i].tlx=0;
    component_info[i].tly=0;
    component_info[i].hstep=1;
    component_info[i].vstep=1;
    component_info[i].width=(unsigned int) image->columns;
    component_info[i].height=(unsigned int) image->rows;
    component_info[i].prec=(int) MagickMax(MagickMin(image->depth,16),2);
    component_info[i].sgnd=MagickFalse;
  }
  jp2_image=jas_image_create((int) number_components,component_info,
    JAS_CLRSPC_UNKNOWN);
  if (jp2_image == (jas_image_t *) NULL)
    ThrowWriterException(DelegateError,"UnableToCreateImage");
  switch (image->colorspace)
  {
    case RGBColorspace:
    case sRGBColorspace:
    {
      /*
        RGB colorspace.
      */
      jas_image_setclrspc(jp2_image,JAS_CLRSPC_SRGB);
      jas_image_setcmpttype(jp2_image,0,
        (jas_image_cmpttype_t) JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
      jas_image_setcmpttype(jp2_image,1,
        (jas_image_cmpttype_t) JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
      jas_image_setcmpttype(jp2_image,2,
        (jas_image_cmpttype_t) JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
      if (number_components == 4)
        jas_image_setcmpttype(jp2_image,3,JAS_IMAGE_CT_OPACITY);
      break;
    }
    case GRAYColorspace:
    {
      /*
        Grayscale colorspace.
      */
      jas_image_setclrspc(jp2_image,JAS_CLRSPC_SGRAY);
      jas_image_setcmpttype(jp2_image,0,
        JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
      break;
    }
    case YCbCrColorspace:
    {
      /*
        YCbCr colorspace.
      */
      jas_image_setclrspc(jp2_image,JAS_CLRSPC_SYCBCR);
      jas_image_setcmpttype(jp2_image,0,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(0));
      jas_image_setcmpttype(jp2_image,1,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(1));
      jas_image_setcmpttype(jp2_image,2,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(2));
      if (number_components == 4)
        jas_image_setcmpttype(jp2_image,3,JAS_IMAGE_CT_OPACITY);
      break;
    }
    case XYZColorspace:
    {
      /*
        XYZ colorspace.
      */
      jas_image_setclrspc(jp2_image,JAS_CLRSPC_CIEXYZ);
      jas_image_setcmpttype(jp2_image,0,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(0));
      jas_image_setcmpttype(jp2_image,1,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(1));
      jas_image_setcmpttype(jp2_image,2,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(2));
      if (number_components == 4)
        jas_image_setcmpttype(jp2_image,3,JAS_IMAGE_CT_OPACITY);
      break;
    }
    case LabColorspace:
    {
      /*
        Lab colorspace.
      */
      jas_image_setclrspc(jp2_image,JAS_CLRSPC_CIELAB);
      jas_image_setcmpttype(jp2_image,0,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(0));
      jas_image_setcmpttype(jp2_image,1,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(1));
      jas_image_setcmpttype(jp2_image,2,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(2));
      if (number_components == 4)
        jas_image_setcmpttype(jp2_image,3,JAS_IMAGE_CT_OPACITY);
      break;
    }
    default:
    {
      /*
        Unknow.
      */
      jas_image_setclrspc(jp2_image,JAS_CLRSPC_UNKNOWN);
      jas_image_setcmpttype(jp2_image,0,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(0));
      jas_image_setcmpttype(jp2_image,1,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(1));
      jas_image_setcmpttype(jp2_image,2,(jas_image_cmpttype_t)
        JAS_IMAGE_CT_COLOR(2));
      if (number_components == 4)
        jas_image_setcmpttype(jp2_image,3,JAS_IMAGE_CT_OPACITY);
      break;
    }
  }
  /*
    Convert to JPEG 2000 pixels.
  */
  for (i=0; i < (ssize_t) number_components; i++)
  {
    pixels[i]=jas_matrix_create(1,(int) image->columns);
    if (pixels[i] == (jas_matrix_t *) NULL)
      {
        for (x=0; x < i; x++)
          jas_matrix_destroy(pixels[x]);
        jas_image_destroy(jp2_image);
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      }
  }
  range=GetQuantumRange((size_t) component_info[0].prec);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (number_components == 1)
        jas_matrix_setv(pixels[0],x,(jas_seqent_t) ScaleQuantumToAny(
          PixelIntensityToQuantum(image,p),range));
      else
        {
          jas_matrix_setv(pixels[0],x,(jas_seqent_t)
            ScaleQuantumToAny(GetPixelRed(p),range));
          jas_matrix_setv(pixels[1],x,(jas_seqent_t)
            ScaleQuantumToAny(GetPixelGreen(p),range));
          jas_matrix_setv(pixels[2],x,(jas_seqent_t)
            ScaleQuantumToAny(GetPixelBlue(p),range));
          if (number_components > 3)
            jas_matrix_setv(pixels[3],x,(jas_seqent_t)
              ScaleQuantumToAny((Quantum) (GetPixelAlpha(p)),range));
        }
      p++;
    }
    for (i=0; i < (ssize_t) number_components; i++)
      (void) jas_image_writecmpt(jp2_image,(short) i,0,(unsigned int) y,
        (unsigned int) image->columns,1,pixels[i]);
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) CopyMagickString(magick,image_info->magick,MaxTextExtent);
  if (LocaleCompare(magick,"J2C") == 0)
    (void) CopyMagickString(magick,"JPC",MaxTextExtent);
  LocaleLower(magick);
  format=jas_image_strtofmt(magick);
  options=(char *) NULL;
  ResetImageOptionIterator(image_info);
  key=GetNextImageOption(image_info);
  for ( ; key != (char *) NULL; key=GetNextImageOption(image_info))
  {
    option=GetImageOption(image_info,key);
    if (option == (const char *) NULL)
      continue;
    if (LocaleNCompare(key,"jp2:",4) == 0)
      {
        (void) ConcatenateString(&options,key+4);
        if (*option != '\0')
          {
            (void) ConcatenateString(&options,"=");
            (void) ConcatenateString(&options,option);
          }
        (void) ConcatenateString(&options," ");
      }
  }
  option=GetImageOption(image_info,"jp2:rate");
  if ((option == (const char *) NULL) &&
      (image_info->compression != LosslessJPEGCompression) &&
      (image->quality != UndefinedCompressionQuality) &&
      ((double) image->quality <= 99.5) &&
      ((image->rows*image->columns) > 2500))
    {
      char
        option[MaxTextExtent];

      double
        alpha,
        header_size,
        number_pixels,
        rate,
        target_size;

      alpha=115.0-image->quality;
      rate=100.0/(alpha*alpha);
      header_size=550.0;
      header_size+=(number_components-1)*142;
      number_pixels=(double) image->rows*image->columns*number_components*
        (GetImageQuantumDepth(image,MagickTrue)/8);
      target_size=(number_pixels*rate)+header_size;
      rate=target_size/number_pixels;
      (void) FormatLocaleString(option,MaxTextExtent,"rate=%g",rate);
      (void) ConcatenateString(&options,option);
    }
  status=jas_image_encode(jp2_image,jp2_stream,format,options) != 0 ?
    MagickTrue : MagickFalse;
  if (options != (char *) NULL)
    options=DestroyString(options);
  (void) jas_stream_close(jp2_stream);
  for (i=0; i < (ssize_t) number_components; i++)
    jas_matrix_destroy(pixels[i]);
  jas_image_destroy(jp2_image);
  if (status != MagickFalse)
    ThrowWriterException(DelegateError,"UnableToEncodeImageFile");
  return(MagickTrue);
}
#endif

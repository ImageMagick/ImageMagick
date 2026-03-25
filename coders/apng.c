/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        A   PPPP   N   N   GGGG                              %
%                       A A  P   P  NN  N  G                                  %
%                      AAAAA PPPP   N N N  G  GG                              %
%                      A   A P      N  NN  G   G                              %
%                      A   A P      N   N   GGGG                              %
%                                                                             %
%                                                                             %
%              Read/Write Animated Portable Network Graphics                  %
%                                                                             %
%                              Software Design                                %
%                              ezgif.com, 2026                                %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/channel.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include <zlib.h>

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteAPNGImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
  Typedef declarations.
*/
typedef struct _APNGFrameInfo
{
  unsigned int
    width,
    height,
    x_offset,
    y_offset;

  unsigned short
    delay_num,
    delay_den;

  unsigned char
    dispose_op,
    blend_op;

  unsigned char
    *compressed_data;

  size_t
    compressed_size,
    compressed_alloc;
} APNGFrameInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d A P N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadAPNGImage() reads an Animated Portable Network Graphics (APNG) file
%  and returns composed frames as an image sequence.
%
%  The format of the ReadAPNGImage method is:
%
%      Image *ReadAPNGImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline unsigned int ReadAPNGInt(const unsigned char *p)
{
  return(((unsigned int) p[0] << 24) | ((unsigned int) p[1] << 16) |
         ((unsigned int) p[2] << 8) | (unsigned int) p[3]);
}

static inline void WriteAPNGInt(unsigned char *p,unsigned int v)
{
  p[0]=(unsigned char) ((v >> 24) & 0xff);
  p[1]=(unsigned char) ((v >> 16) & 0xff);
  p[2]=(unsigned char) ((v >> 8) & 0xff);
  p[3]=(unsigned char) (v & 0xff);
}

static inline unsigned char PaethPredictor(int a,int b,int c)
{
  int
    p,pa,pb,pc;

  p=a+b-c;
  pa=abs(p-a);
  pb=abs(p-b);
  pc=abs(p-c);
  if ((pa <= pb) && (pa <= pc))
    return((unsigned char) a);
  if (pb <= pc)
    return((unsigned char) b);
  return((unsigned char) c);
}

static int BppForColorType(unsigned char ct)
{
  switch (ct)
  {
    case 0: return(1);
    case 2: return(3);
    case 3: return(1);
    case 4: return(2);
    case 6: return(4);
    default: return(0);
  }
}

static MagickBooleanType InflateAPNGData(const unsigned char *in_data,
  size_t in_size,unsigned char **out_data,size_t *out_size)
{
  int
    ret;

  size_t
    alloc_size;

  unsigned char
    *buffer,
    *new_buffer;

  z_stream
    stream;

  alloc_size=in_size*4;
  if (alloc_size < 65536)
    alloc_size=65536;
  if (alloc_size > 256*1024*1024)
    alloc_size=256*1024*1024;
  buffer=(unsigned char *) AcquireQuantumMemory(alloc_size,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    return(MagickFalse);
  (void) memset(&stream,0,sizeof(stream));
  stream.next_in=(Bytef *) in_data;
  stream.avail_in=(uInt) in_size;
  if (inflateInit(&stream) != Z_OK)
    {
      buffer=(unsigned char *) RelinquishMagickMemory(buffer);
      return(MagickFalse);
    }
  *out_size=0;
  do
  {
    if (*out_size + 65536 > alloc_size)
      {
        alloc_size=alloc_size*2;
        if (alloc_size > (size_t) 1024*1024*1024)
          {
            (void) inflateEnd(&stream);
            buffer=(unsigned char *) RelinquishMagickMemory(buffer);
            return(MagickFalse);
          }
        new_buffer=(unsigned char *) ResizeMagickMemory(buffer,alloc_size);
        if (new_buffer == (unsigned char *) NULL)
          {
            (void) inflateEnd(&stream);
            buffer=(unsigned char *) RelinquishMagickMemory(buffer);
            return(MagickFalse);
          }
        buffer=new_buffer;
      }
    stream.next_out=buffer+*out_size;
    stream.avail_out=(uInt) (alloc_size-*out_size);
    ret=inflate(&stream,Z_NO_FLUSH);
    if ((ret == Z_STREAM_ERROR) || (ret == Z_DATA_ERROR) ||
        (ret == Z_MEM_ERROR))
      {
        (void) inflateEnd(&stream);
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
        return(MagickFalse);
      }
    *out_size=alloc_size-stream.avail_out;
  } while (ret != Z_STREAM_END);
  (void) inflateEnd(&stream);
  *out_data=buffer;
  return(MagickTrue);
}

static MagickBooleanType UnfilterAPNGRows(unsigned char *raw,size_t raw_size,
  unsigned int w,unsigned int h,int bpp,unsigned char **pixel_data)
{
  unsigned char
    *dst,
    ft,
    *out;

  const unsigned char
    *src;

  unsigned int
    stride,
    x,
    y;

  stride=w*(unsigned int) bpp;
  if (raw_size < (size_t) h*(1+stride))
    return(MagickFalse);
  out=(unsigned char *) AcquireQuantumMemory((size_t) h*stride,sizeof(*out));
  if (out == (unsigned char *) NULL)
    return(MagickFalse);
  for (y=0; y < h; y++)
  {
    unsigned char
      a_val,b_val,c_val;

    ft=raw[y*(1+stride)];
    src=&raw[y*(1+stride)+1];
    dst=&out[y*stride];
    for (x=0; x < stride; x++)
    {
      a_val=(x >= (unsigned int) bpp) ? dst[x-bpp] : 0;
      b_val=(y > 0) ? out[(y-1)*stride+x] : 0;
      c_val=((x >= (unsigned int) bpp) && (y > 0)) ?
        out[(y-1)*stride+x-bpp] : 0;
      switch (ft)
      {
        case 0: dst[x]=src[x]; break;
        case 1: dst[x]=(unsigned char) (src[x]+a_val); break;
        case 2: dst[x]=(unsigned char) (src[x]+b_val); break;
        case 3: dst[x]=(unsigned char) (src[x]+
          (unsigned char) (((int) a_val+(int) b_val)/2)); break;
        case 4: dst[x]=(unsigned char) (src[x]+
          PaethPredictor(a_val,b_val,c_val)); break;
        default:
          out=(unsigned char *) RelinquishMagickMemory(out);
          return(MagickFalse);
      }
    }
  }
  *pixel_data=out;
  return(MagickTrue);
}

static void ConvertToRGBA(const unsigned char *px,unsigned int w,
  unsigned int h,unsigned char ct,const unsigned char *pal,size_t pal_size,
  const unsigned char *trns,size_t trns_size,unsigned char *rgba)
{
  unsigned char
    a,b,g,r;

  unsigned int
    i,
    np;

  np=w*h;
  for (i=0; i < np; i++)
  {
    r=0;
    g=0;
    b=0;
    a=255;
    switch(ct)
    {
      case 0:
        r=g=b=px[i];
        if ((trns_size >= 2) && (px[i] == trns[1]))
          a=0;
        break;
      case 2:
        r=px[i*3]; g=px[i*3+1]; b=px[i*3+2];
        if ((trns_size >= 6) && (r == trns[1]) && (g == trns[3]) &&
            (b == trns[5]))
          a=0;
        break;
      case 3:
      {
        unsigned char
          idx;

        idx=px[i];
        if ((size_t) idx*3+2 < pal_size)
          {
            r=pal[idx*3];
            g=pal[idx*3+1];
            b=pal[idx*3+2];
          }
        a=((size_t) idx < trns_size) ? trns[idx] : 255;
        break;
      }
      case 4:
        r=g=b=px[i*2]; a=px[i*2+1];
        break;
      case 6:
        r=px[i*4]; g=px[i*4+1]; b=px[i*4+2]; a=px[i*4+3];
        break;
    }
    rgba[i*4]=r;
    rgba[i*4+1]=g;
    rgba[i*4+2]=b;
    rgba[i*4+3]=a;
  }
}

static inline void BlendOver(unsigned char *dst,const unsigned char *src)
{
  unsigned int
    da,ra,sa;

  sa=src[3];
  da=dst[3];
  if (sa == 255)
    {
      (void) memcpy(dst,src,4);
      return;
    }
  if (sa == 0)
    return;
  ra=sa*255+da*(255-sa);
  if (ra == 0)
    {
      (void) memset(dst,0,4);
      return;
    }
  dst[0]=(unsigned char) ((src[0]*sa*255+dst[0]*da*(255-sa))/ra);
  dst[1]=(unsigned char) ((src[1]*sa*255+dst[1]*da*(255-sa))/ra);
  dst[2]=(unsigned char) ((src[2]*sa*255+dst[2]*da*(255-sa))/ra);
  dst[3]=(unsigned char) ((ra+127)/255);
}

static void DestroyAPNGFrameInfo(APNGFrameInfo *frames,size_t count)
{
  size_t
    i;

  if (frames == (APNGFrameInfo *) NULL)
    return;
  for (i=0; i < count; i++)
    if (frames[i].compressed_data != (unsigned char *) NULL)
      frames[i].compressed_data=(unsigned char *)
        RelinquishMagickMemory(frames[i].compressed_data);
  frames=(APNGFrameInfo *) RelinquishMagickMemory(frames);
}

static MagickBooleanType AppendFrameData(APNGFrameInfo *frame,
  const unsigned char *data,size_t length)
{
  if (frame->compressed_data == (unsigned char *) NULL)
    {
      frame->compressed_alloc=length+4096;
      frame->compressed_data=(unsigned char *)
        AcquireQuantumMemory(frame->compressed_alloc,
          sizeof(*frame->compressed_data));
      if (frame->compressed_data == (unsigned char *) NULL)
        return(MagickFalse);
      frame->compressed_size=0;
    }
  if (frame->compressed_size+length > frame->compressed_alloc)
    {
      unsigned char
        *new_data;

      frame->compressed_alloc=(frame->compressed_size+length)*2;
      new_data=(unsigned char *) ResizeMagickMemory(frame->compressed_data,
        frame->compressed_alloc);
      if (new_data == (unsigned char *) NULL)
        {
          frame->compressed_data=(unsigned char *) NULL;
          return(MagickFalse);
        }
      frame->compressed_data=new_data;
    }
  (void) memcpy(frame->compressed_data+frame->compressed_size,data,length);
  frame->compressed_size+=length;
  return(MagickTrue);
}

static Image *ReadAPNGImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  APNGFrameInfo
    *cur,
    *frames;

  Image
    *image,
    *images;

  int
    bpp;

  MagickBooleanType
    first_fctl_before_idat,
    found_actl,
    seen_idat,
    status;

  size_t
    canvas_size,
    fi,
    filesize,
    frame_count,
    frames_alloc,
    pal_size,
    trns_size;

  ssize_t
    x,y;

  unsigned char
    bit_depth,
    *canvas,
    color_type,
    *data,
    *palette,
    *prev_canvas,
    *trns;

  unsigned int
    canvas_h,
    canvas_w;

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
    Read entire file into memory.
  */
  filesize=(size_t) GetBlobSize(image);
  if (filesize < 33)
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
  data=(unsigned char *) AcquireQuantumMemory(filesize,sizeof(*data));
  if (data == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (ReadBlob(image,filesize,data) != (ssize_t) filesize)
    {
      data=(unsigned char *) RelinquishMagickMemory(data);
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    }
  (void) CloseBlob(image);
  image=DestroyImageList(image);
  /*
    Verify PNG signature.
  */
  if (memcmp(data,"\211PNG\r\n\032\n",8) != 0)
    {
      data=(unsigned char *) RelinquishMagickMemory(data);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  /*
    Parse APNG chunks.
  */
  canvas_w=0;
  canvas_h=0;
  bit_depth=0;
  color_type=0;
  found_actl=MagickFalse;
  first_fctl_before_idat=MagickFalse;
  seen_idat=MagickFalse;
  palette=(unsigned char *) NULL;
  pal_size=0;
  trns=(unsigned char *) NULL;
  trns_size=0;
  frames=(APNGFrameInfo *) NULL;
  frame_count=0;
  frames_alloc=0;
  cur=(APNGFrameInfo *) NULL;
  {
    size_t
      pos;

    pos=8;
    while (pos+12 <= filesize)
    {
      unsigned int
        clen;

      const unsigned char
        *cd;

      clen=ReadAPNGInt(&data[pos]);
      cd=&data[pos+8];
      if (pos+12+(size_t) clen > filesize)
        break;
      if ((memcmp(&data[pos+4],"IHDR",4) == 0) && (clen >= 13))
        {
          canvas_w=ReadAPNGInt(cd);
          canvas_h=ReadAPNGInt(cd+4);
          if ((canvas_w == 0) || (canvas_h == 0) ||
              (canvas_w > 0x7FFFFFFF) || (canvas_h > 0x7FFFFFFF))
            {
              data=(unsigned char *) RelinquishMagickMemory(data);
              ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            }
          bit_depth=cd[8];
          color_type=cd[9];
        }
      else if (memcmp(&data[pos+4],"acTL",4) == 0)
        found_actl=MagickTrue;
      else if (memcmp(&data[pos+4],"PLTE",4) == 0)
        {
          palette=(unsigned char *) cd;
          pal_size=clen;
        }
      else if (memcmp(&data[pos+4],"tRNS",4) == 0)
        {
          trns=(unsigned char *) cd;
          trns_size=clen;
        }
      else if ((memcmp(&data[pos+4],"fcTL",4) == 0) && (clen >= 26))
        {
          APNGFrameInfo
            new_frame,
            *new_frames;

          if (seen_idat == MagickFalse)
            first_fctl_before_idat=MagickTrue;
          (void) memset(&new_frame,0,sizeof(new_frame));
          new_frame.width=ReadAPNGInt(cd+4);
          new_frame.height=ReadAPNGInt(cd+8);
          new_frame.x_offset=ReadAPNGInt(cd+12);
          new_frame.y_offset=ReadAPNGInt(cd+16);
          new_frame.delay_num=(unsigned short) ((cd[20] << 8) | cd[21]);
          new_frame.delay_den=(unsigned short) ((cd[22] << 8) | cd[23]);
          new_frame.dispose_op=cd[24];
          new_frame.blend_op=cd[25];
          /*
            Validate frame region fits within canvas.
          */
          if ((new_frame.width == 0) || (new_frame.height == 0) ||
              ((size_t) new_frame.x_offset+new_frame.width > canvas_w) ||
              ((size_t) new_frame.y_offset+new_frame.height > canvas_h))
            {
              pos+=12+clen;
              continue;
            }
          new_frame.compressed_data=(unsigned char *) NULL;
          new_frame.compressed_size=0;
          new_frame.compressed_alloc=0;
          /*
            Grow frames array.
          */
          if (frame_count >= frames_alloc)
            {
              frames_alloc=(frames_alloc == 0) ? 16 : frames_alloc*2;
              new_frames=(APNGFrameInfo *) ResizeMagickMemory(frames,
                frames_alloc*sizeof(*frames));
              if (new_frames == (APNGFrameInfo *) NULL)
                {
                  DestroyAPNGFrameInfo(frames,frame_count);
                  data=(unsigned char *) RelinquishMagickMemory(data);
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed");
                }
              frames=new_frames;
            }
          frames[frame_count]=new_frame;
          cur=&frames[frame_count];
          frame_count++;
        }
      else if (memcmp(&data[pos+4],"IDAT",4) == 0)
        {
          seen_idat=MagickTrue;
          if ((cur != (APNGFrameInfo *) NULL) &&
              (first_fctl_before_idat != MagickFalse) && (frame_count == 1))
            {
              if (AppendFrameData(cur,cd,clen) == MagickFalse)
                {
                  DestroyAPNGFrameInfo(frames,frame_count);
                  data=(unsigned char *) RelinquishMagickMemory(data);
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed");
                }
            }
        }
      else if ((memcmp(&data[pos+4],"fdAT",4) == 0) && (clen > 4))
        {
          if (cur != (APNGFrameInfo *) NULL)
            {
              if (AppendFrameData(cur,cd+4,clen-4) == MagickFalse)
                {
                  DestroyAPNGFrameInfo(frames,frame_count);
                  data=(unsigned char *) RelinquishMagickMemory(data);
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed");
                }
            }
        }
      else if (memcmp(&data[pos+4],"IEND",4) == 0)
        break;
      pos+=12+clen;
    }
  }
  /*
    Validate APNG structure.
  */
  if ((found_actl == MagickFalse) || (frame_count == 0))
    {
      DestroyAPNGFrameInfo(frames,frame_count);
      data=(unsigned char *) RelinquishMagickMemory(data);
      (void) ThrowMagickException(exception,GetMagickModule(),
        CoderError,"Not an animated PNG (no acTL chunk)","`%s'",
        image_info->filename);
      return((Image *) NULL);
    }
  if (bit_depth != 8)
    {
      DestroyAPNGFrameInfo(frames,frame_count);
      data=(unsigned char *) RelinquishMagickMemory(data);
      (void) ThrowMagickException(exception,GetMagickModule(),
        CoderError,"Only 8-bit depth APNG supported","`%s'",
        image_info->filename);
      return((Image *) NULL);
    }
  bpp=BppForColorType(color_type);
  if (bpp == 0)
    {
      DestroyAPNGFrameInfo(frames,frame_count);
      data=(unsigned char *) RelinquishMagickMemory(data);
      (void) ThrowMagickException(exception,GetMagickModule(),
        CoderError,"Unsupported APNG color type","`%s'",
        image_info->filename);
      return((Image *) NULL);
    }
  /*
    Per APNG spec: first frame dispose_op PREVIOUS -> treat as BACKGROUND.
  */
  if ((frame_count > 0) && (frames[0].dispose_op == 2))
    frames[0].dispose_op=1;
  /*
    Compose frames onto canvas.
  */
  if ((size_t) canvas_w > SIZE_MAX / ((size_t) canvas_h * 4))
    {
      DestroyAPNGFrameInfo(frames,frame_count);
      data=(unsigned char *) RelinquishMagickMemory(data);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  canvas_size=(size_t) canvas_w * (size_t) canvas_h * 4;
  canvas=(unsigned char *) AcquireQuantumMemory(canvas_size,sizeof(*canvas));
  prev_canvas=(unsigned char *) AcquireQuantumMemory(canvas_size,
    sizeof(*prev_canvas));
  if ((canvas == (unsigned char *) NULL) ||
      (prev_canvas == (unsigned char *) NULL))
    {
      if (canvas != (unsigned char *) NULL)
        canvas=(unsigned char *) RelinquishMagickMemory(canvas);
      if (prev_canvas != (unsigned char *) NULL)
        prev_canvas=(unsigned char *) RelinquishMagickMemory(prev_canvas);
      DestroyAPNGFrameInfo(frames,frame_count);
      data=(unsigned char *) RelinquishMagickMemory(data);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) memset(canvas,0,canvas_size);
  images=(Image *) NULL;
  for (fi=0; fi < frame_count; fi++)
  {
    APNGFrameInfo
      *f;

    double
      delay_seconds;

    Image
      *frame_image;

    Quantum
      *q;

    size_t
      idx,
      raw_size;

    unsigned char
      *frame_rgba,
      *pixel_data,
      *raw;

    unsigned int
      cx,cy,fx,fy;

    unsigned short
      dden,dnum;

    f=&frames[fi];
    /*
      Save canvas for DISPOSE_OP_PREVIOUS.
    */
    if (f->dispose_op == 2)
      (void) memcpy(prev_canvas,canvas,canvas_size);
    /*
      Decompress frame data.
    */
    raw=(unsigned char *) NULL;
    raw_size=0;
    if ((f->compressed_data == (unsigned char *) NULL) ||
        (f->compressed_size == 0) ||
        (InflateAPNGData(f->compressed_data,f->compressed_size,
          &raw,&raw_size) == MagickFalse))
      {
        if (canvas != (unsigned char *) NULL)
          canvas=(unsigned char *) RelinquishMagickMemory(canvas);
        if (prev_canvas != (unsigned char *) NULL)
          prev_canvas=(unsigned char *) RelinquishMagickMemory(prev_canvas);
        DestroyAPNGFrameInfo(frames,frame_count);
        data=(unsigned char *) RelinquishMagickMemory(data);
        if (images != (Image *) NULL)
          images=DestroyImageList(images);
        (void) ThrowMagickException(exception,GetMagickModule(),
          CoderError,"APNGDecompressFailed","`%s' frame %lu",
          image_info->filename,(unsigned long) fi);
        return((Image *) NULL);
      }
    /*
      Unfilter rows.
    */
    pixel_data=(unsigned char *) NULL;
    if (UnfilterAPNGRows(raw,raw_size,f->width,f->height,bpp,
        &pixel_data) == MagickFalse)
      {
        raw=(unsigned char *) RelinquishMagickMemory(raw);
        if (canvas != (unsigned char *) NULL)
          canvas=(unsigned char *) RelinquishMagickMemory(canvas);
        if (prev_canvas != (unsigned char *) NULL)
          prev_canvas=(unsigned char *) RelinquishMagickMemory(prev_canvas);
        DestroyAPNGFrameInfo(frames,frame_count);
        data=(unsigned char *) RelinquishMagickMemory(data);
        if (images != (Image *) NULL)
          images=DestroyImageList(images);
        (void) ThrowMagickException(exception,GetMagickModule(),
          CoderError,"APNGUnfilterFailed","`%s' frame %lu",
          image_info->filename,(unsigned long) fi);
        return((Image *) NULL);
      }
    raw=(unsigned char *) RelinquishMagickMemory(raw);
    /*
      Convert to RGBA.
    */
    frame_rgba=(unsigned char *) AcquireQuantumMemory(
      (size_t) f->width * (size_t) f->height,4);
    if (frame_rgba == (unsigned char *) NULL)
      {
        pixel_data=(unsigned char *) RelinquishMagickMemory(pixel_data);
        canvas=(unsigned char *) RelinquishMagickMemory(canvas);
        prev_canvas=(unsigned char *) RelinquishMagickMemory(prev_canvas);
        DestroyAPNGFrameInfo(frames,frame_count);
        data=(unsigned char *) RelinquishMagickMemory(data);
        if (images != (Image *) NULL)
          images=DestroyImageList(images);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    ConvertToRGBA(pixel_data,f->width,f->height,color_type,
      palette,pal_size,trns,trns_size,frame_rgba);
    pixel_data=(unsigned char *) RelinquishMagickMemory(pixel_data);
    /*
      Composite onto canvas.
    */
    for (fy=0; fy < f->height; fy++)
    {
      for (fx=0; fx < f->width; fx++)
      {
        cx=f->x_offset+fx;
        cy=f->y_offset+fy;
        if ((cx >= canvas_w) || (cy >= canvas_h))
          continue;
        if (f->blend_op == 0)
          (void) memcpy(&canvas[(cy*canvas_w+cx)*4],
            &frame_rgba[(fy*f->width+fx)*4],4);
        else
          BlendOver(&canvas[(cy*canvas_w+cx)*4],
            &frame_rgba[(fy*f->width+fx)*4]);
      }
    }
    frame_rgba=(unsigned char *) RelinquishMagickMemory(frame_rgba);
    /*
      Create ImageMagick Image from canvas.
    */
    frame_image=AcquireImage(image_info,exception);
    frame_image->columns=canvas_w;
    frame_image->rows=canvas_h;
    frame_image->alpha_trait=BlendPixelTrait;
    if (SetImageExtent(frame_image,canvas_w,canvas_h,exception) == MagickFalse)
      {
        frame_image=DestroyImage(frame_image);
        canvas=(unsigned char *) RelinquishMagickMemory(canvas);
        prev_canvas=(unsigned char *) RelinquishMagickMemory(prev_canvas);
        DestroyAPNGFrameInfo(frames,frame_count);
        data=(unsigned char *) RelinquishMagickMemory(data);
        if (images != (Image *) NULL)
          images=DestroyImageList(images);
        return((Image *) NULL);
      }
    for (y=0; y < (ssize_t) canvas_h; y++)
    {
      q=QueueAuthenticPixels(frame_image,0,y,canvas_w,1,exception);
      if (q == (Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) canvas_w; x++)
      {
        idx=((size_t) y*canvas_w+(size_t) x)*4;
        SetPixelRed(frame_image,ScaleCharToQuantum(canvas[idx]),q);
        SetPixelGreen(frame_image,ScaleCharToQuantum(canvas[idx+1]),q);
        SetPixelBlue(frame_image,ScaleCharToQuantum(canvas[idx+2]),q);
        SetPixelAlpha(frame_image,ScaleCharToQuantum(canvas[idx+3]),q);
        q+=GetPixelChannels(frame_image);
      }
      if (SyncAuthenticPixels(frame_image,exception) == MagickFalse)
        break;
    }
    /*
      Set frame timing.
    */
    dnum=f->delay_num;
    dden=f->delay_den;
    if (dden == 0)
      dden=100;
    delay_seconds=(double) dnum/(double) dden;
    frame_image->ticks_per_second=100;
    frame_image->delay=(size_t) (delay_seconds*100.0+0.5);
    frame_image->scene=fi;
    frame_image->dispose=BackgroundDispose;
    (void) CopyMagickString(frame_image->filename,image_info->filename,
      MagickPathExtent);
    (void) CopyMagickString(frame_image->magick,"APNG",MagickPathExtent);
    if (images == (Image *) NULL)
      images=frame_image;
    else
      AppendImageToList(&images,frame_image);
    /*
      Apply dispose operation.
    */
    if (f->dispose_op == 1)
      {
        /* DISPOSE_OP_BACKGROUND: clear frame region to transparent. */
        for (fy=0; fy < f->height; fy++)
          for (fx=0; fx < f->width; fx++)
          {
            cx=f->x_offset+fx;
            cy=f->y_offset+fy;
            if ((cx < canvas_w) && (cy < canvas_h))
              (void) memset(&canvas[(cy*canvas_w+cx)*4],0,4);
          }
      }
    else if (f->dispose_op == 2)
      {
        /* DISPOSE_OP_PREVIOUS: restore canvas. */
        (void) memcpy(canvas,prev_canvas,canvas_size);
      }
  }
  canvas=(unsigned char *) RelinquishMagickMemory(canvas);
  prev_canvas=(unsigned char *) RelinquishMagickMemory(prev_canvas);
  DestroyAPNGFrameInfo(frames,frame_count);
  data=(unsigned char *) RelinquishMagickMemory(data);
  return(GetFirstImageInList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r A P N G I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterAPNGImage() adds attributes for the APNG image format to the list
%  of supported formats.
%
%  The format of the RegisterAPNGImage method is:
%
%      size_t RegisterAPNGImage(void)
%
*/
ModuleExport size_t RegisterAPNGImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PNG","APNG","Animated Portable Network Graphics");
  entry->decoder=(DecodeImageHandler *) ReadAPNGImage;
  entry->encoder=(EncodeImageHandler *) WriteAPNGImage;
  entry->mime_type=ConstantString("image/apng");
  entry->flags^=CoderBlobSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A P N G I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterAPNGImage() removes format registrations made by the APNG module
%  from the list of supported formats.
%
%  The format of the UnregisterAPNGImage method is:
%
%      UnregisterAPNGImage(void)
%
*/
ModuleExport void UnregisterAPNGImage(void)
{
  (void) UnregisterMagickInfo("APNG");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e A P N G I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteAPNGImage() writes an image sequence as an Animated Portable Network
%  Graphics (APNG) file.
%
%  The format of the WriteAPNGImage method is:
%
%      MagickBooleanType WriteAPNGImage(const ImageInfo *image_info,
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

static MagickBooleanType WritePNGChunk(Image *image,const char *type,
  const unsigned char *chunk_data,unsigned int length,ExceptionInfo *exception)
{
  unsigned char
    header[8];

  unsigned int
    crc;

  (void) exception;
  WriteAPNGInt(header,length);
  (void) memcpy(header+4,type,4);
  if (WriteBlob(image,8,header) != 8)
    return(MagickFalse);
  crc=crc32(0,(const Bytef *) type,4);
  if (length > 0)
    {
      if (WriteBlob(image,length,chunk_data) != (ssize_t) length)
        return(MagickFalse);
      crc=crc32(crc,chunk_data,length);
    }
  WriteAPNGInt(header,crc);
  if (WriteBlob(image,4,header) != 4)
    return(MagickFalse);
  return(MagickTrue);
}

static unsigned char *FilterAPNGRows(const unsigned char *rgba,int w,int h,
  size_t *filtered_size)
{
  const unsigned char
    *prev,
    *row;

  int
    best_filter,
    ftype,
    stride,
    x,
    y;

  long
    best_sum,
    sum;

  size_t
    opos;

  unsigned char
    *out;

  stride=w*4;
  *filtered_size=(size_t) h*(1+stride);
  out=(unsigned char *) AcquireQuantumMemory(*filtered_size,sizeof(*out));
  if (out == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  opos=0;
  for (y=0; y < h; y++)
  {
    row=rgba+y*stride;
    prev=(y > 0) ? rgba+(y-1)*stride : (const unsigned char *) NULL;
    best_filter=0;
    best_sum=LONG_MAX;
    for (ftype=0; ftype < 5; ftype++)
    {
      sum=0;
      for (x=0; x < stride; x++)
      {
        int
          a,b_val,c_val,val;

        a=(x >= 4) ? (int) row[x-4] : 0;
        b_val=prev ? (int) prev[x] : 0;
        c_val=(prev && x >= 4) ? (int) prev[x-4] : 0;
        switch (ftype)
        {
          case 0: val=(int) row[x]; break;
          case 1: val=((int) row[x]-a) & 0xff; break;
          case 2: val=((int) row[x]-b_val) & 0xff; break;
          case 3: val=((int) row[x]-((a+b_val) >> 1)) & 0xff; break;
          case 4: val=((int) row[x]-(int) PaethPredictor(a,b_val,
            c_val)) & 0xff; break;
          default: val=(int) row[x]; break;
        }
        sum+=(val < 128) ? val : 256-val;
      }
      if (sum < best_sum)
        {
          best_sum=sum;
          best_filter=ftype;
        }
    }
    out[opos++]=(unsigned char) best_filter;
    for (x=0; x < stride; x++)
    {
      int
        a,b_val,c_val;

      a=(x >= 4) ? (int) row[x-4] : 0;
      b_val=prev ? (int) prev[x] : 0;
      c_val=(prev && x >= 4) ? (int) prev[x-4] : 0;
      switch (best_filter)
      {
        case 0: out[opos++]=row[x]; break;
        case 1: out[opos++]=(unsigned char) (((int) row[x]-a) & 0xff); break;
        case 2: out[opos++]=(unsigned char) (((int) row[x]-b_val) & 0xff);
          break;
        case 3: out[opos++]=(unsigned char) (((int) row[x]-
          ((a+b_val) >> 1)) & 0xff); break;
        case 4: out[opos++]=(unsigned char) (((int) row[x]-
          (int) PaethPredictor(a,b_val,c_val)) & 0xff); break;
      }
    }
  }
  return(out);
}

static MagickBooleanType CompressAPNGFrame(const unsigned char *rgba,int w,
  int h,unsigned char **comp_data,size_t *comp_size)
{
  int
    ret;

  size_t
    filtered_size;

  uLongf
    comp_len;

  unsigned char
    *compressed,
    *filtered;

  filtered=FilterAPNGRows(rgba,w,h,&filtered_size);
  if (filtered == (unsigned char *) NULL)
    return(MagickFalse);
  comp_len=compressBound((uLong) filtered_size);
  compressed=(unsigned char *) AcquireQuantumMemory(comp_len,
    sizeof(*compressed));
  if (compressed == (unsigned char *) NULL)
    {
      filtered=(unsigned char *) RelinquishMagickMemory(filtered);
      return(MagickFalse);
    }
  ret=compress2(compressed,&comp_len,filtered,(uLong) filtered_size,
    Z_DEFAULT_COMPRESSION);
  filtered=(unsigned char *) RelinquishMagickMemory(filtered);
  if (ret != Z_OK)
    {
      compressed=(unsigned char *) RelinquishMagickMemory(compressed);
      return(MagickFalse);
    }
  *comp_data=compressed;
  *comp_size=(size_t) comp_len;
  return(MagickTrue);
}

static unsigned char *GetImageRGBA(Image *image,ExceptionInfo *exception)
{
  const Quantum
    *p;

  size_t
    idx;

  ssize_t
    x,
    y;

  unsigned char
    *rgba;

  rgba=(unsigned char *) AcquireQuantumMemory((size_t) image->columns *
    (size_t) image->rows,4);
  if (rgba == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        rgba=(unsigned char *) RelinquishMagickMemory(rgba);
        return((unsigned char *) NULL);
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      idx=((size_t) y*image->columns+(size_t) x)*4;
      rgba[idx]=(unsigned char) ScaleQuantumToChar(GetPixelRed(image,p));
      rgba[idx+1]=(unsigned char) ScaleQuantumToChar(GetPixelGreen(image,p));
      rgba[idx+2]=(unsigned char) ScaleQuantumToChar(GetPixelBlue(image,p));
      rgba[idx+3]=(unsigned char) ScaleQuantumToChar(GetPixelAlpha(image,p));
      p+=GetPixelChannels(image);
    }
  }
  return(rgba);
}

static void CleanupFramePixels(unsigned char **frame_pixels,
  size_t frame_count)
{
  size_t
    i;

  for (i=0; i < frame_count; i++)
    if (frame_pixels[i] != (unsigned char *) NULL)
      frame_pixels[i]=(unsigned char *)
        RelinquishMagickMemory(frame_pixels[i]);
  frame_pixels=(unsigned char **) RelinquishMagickMemory(frame_pixels);
}

static MagickBooleanType WriteAPNGImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  Image
    *next;

  MagickBooleanType
    has_transparency,
    status;

  size_t
    fi,
    frame_count,
    np,
    px;

  unsigned char
    **frame_pixels;

  unsigned int
    gh,
    gw,
    seq_num;

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
  /*
    Count frames and get canvas size from first frame.
  */
  frame_count=GetImageListLength(image);
  if (frame_count == 0)
    {
      (void) CloseBlob(image);
      return(MagickFalse);
    }
  gw=(unsigned int) image->columns;
  gh=(unsigned int) image->rows;
  /*
    Convert all frames to RGBA and ensure consistent alpha channel.
  */
  frame_pixels=(unsigned char **) AcquireQuantumMemory(frame_count,
    sizeof(*frame_pixels));
  if (frame_pixels == (unsigned char **) NULL)
    {
      (void) CloseBlob(image);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) memset(frame_pixels,0,frame_count*sizeof(*frame_pixels));
  fi=0;
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->alpha_trait != BlendPixelTrait)
      (void) SetImageAlphaChannel(next,OpaqueAlphaChannel,exception);
    else
      (void) SetImageAlphaChannel(next,ActivateAlphaChannel,exception);
    (void) TransformImageColorspace(next,sRGBColorspace,exception);
    frame_pixels[fi]=GetImageRGBA(next,exception);
    if (frame_pixels[fi] == (unsigned char *) NULL)
      {
        CleanupFramePixels(frame_pixels,frame_count);
        (void) CloseBlob(image);
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      }
    fi++;
  }
  /*
    Check if any frame has transparent pixels.
  */
  has_transparency=MagickFalse;
  np=(size_t) gw*gh;
  for (fi=0; fi < frame_count; fi++)
  {
    if (has_transparency != MagickFalse)
      break;
    for (px=0; px < np; px++)
    {
      if (frame_pixels[fi][px*4+3] < 255)
        {
          has_transparency=MagickTrue;
          break;
        }
    }
  }
  /*
    Write PNG signature.
  */
  {
    const unsigned char
      png_sig[8]={137,80,78,71,13,10,26,10};

    (void) WriteBlob(image,8,png_sig);
  }
  /*
    Write IHDR chunk.
  */
  {
    unsigned char
      ihdr[13];

    WriteAPNGInt(ihdr,gw);
    WriteAPNGInt(ihdr+4,gh);
    ihdr[8]=8;   /* bit depth */
    ihdr[9]=6;   /* color type: RGBA */
    ihdr[10]=0;  /* compression */
    ihdr[11]=0;  /* filter */
    ihdr[12]=0;  /* interlace */
    (void) WritePNGChunk(image,"IHDR",ihdr,13,exception);
  }
  /*
    Write acTL chunk (animation control).
  */
  {
    unsigned char
      actl[8];

    unsigned int
      num_plays;

    WriteAPNGInt(actl,(unsigned int) frame_count);
    num_plays=(unsigned int) image->iterations;
    WriteAPNGInt(actl+4,num_plays);
    (void) WritePNGChunk(image,"acTL",actl,8,exception);
  }
  /*
    Write frames.
  */
  seq_num=0;
  fi=0;
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    double
      delay_sec;

    size_t
      comp_size;

    unsigned char
      *comp_data;

    unsigned short
      delay_den,
      delay_num;

    /*
      Calculate delay.
    */
    delay_sec=(double) next->delay/MagickMax(1.0,(double)
      next->ticks_per_second);
    delay_num=(unsigned short) MagickMin(
      (double) (delay_sec*1000.0+0.5),65535.0);
    delay_den=1000;
    /*
      Simplify fraction.
    */
    if (delay_num > 0)
      {
        unsigned short
          a_v,b_v,t_v;

        a_v=delay_num;
        b_v=delay_den;
        while (b_v != 0)
        {
          t_v=b_v;
          b_v=(unsigned short) (a_v % b_v);
          a_v=t_v;
        }
        delay_num=(unsigned short) (delay_num/a_v);
        delay_den=(unsigned short) (delay_den/a_v);
      }
    /*
      Write fcTL chunk.
    */
    {
      unsigned char
        fctl[26];

      WriteAPNGInt(fctl,seq_num);
      seq_num++;
      WriteAPNGInt(fctl+4,gw);
      WriteAPNGInt(fctl+8,gh);
      WriteAPNGInt(fctl+12,0);  /* x_offset */
      WriteAPNGInt(fctl+16,0);  /* y_offset */
      fctl[20]=(unsigned char) ((delay_num >> 8) & 0xff);
      fctl[21]=(unsigned char) (delay_num & 0xff);
      fctl[22]=(unsigned char) ((delay_den >> 8) & 0xff);
      fctl[23]=(unsigned char) (delay_den & 0xff);
      fctl[24]=has_transparency != MagickFalse ? 1 : 0;
      fctl[25]=0;  /* blend_op: SOURCE */
      (void) WritePNGChunk(image,"fcTL",fctl,26,exception);
    }
    /*
      Compress frame data.
    */
    comp_data=(unsigned char *) NULL;
    comp_size=0;
    if (CompressAPNGFrame(frame_pixels[fi],(int) gw,(int) gh,
        &comp_data,&comp_size) == MagickFalse)
      {
        CleanupFramePixels(frame_pixels,frame_count);
        (void) CloseBlob(image);
        ThrowWriterException(CoderError,"APNGCompressFailed");
      }
    if (fi == 0)
      {
        /*
          First frame: write as IDAT for backwards compatibility.
        */
        (void) WritePNGChunk(image,"IDAT",comp_data,
          (unsigned int) comp_size,exception);
      }
    else
      {
        /*
          Subsequent frames: write as fdAT with sequence number.
        */
        unsigned char
          *fdat;

        unsigned int
          fdat_len;

        fdat_len=4+(unsigned int) comp_size;
        fdat=(unsigned char *) AcquireQuantumMemory(fdat_len,sizeof(*fdat));
        if (fdat == (unsigned char *) NULL)
          {
            comp_data=(unsigned char *) RelinquishMagickMemory(comp_data);
            CleanupFramePixels(frame_pixels,frame_count);
            (void) CloseBlob(image);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        WriteAPNGInt(fdat,seq_num);
        seq_num++;
        (void) memcpy(fdat+4,comp_data,comp_size);
        (void) WritePNGChunk(image,"fdAT",fdat,fdat_len,exception);
        fdat=(unsigned char *) RelinquishMagickMemory(fdat);
      }
    comp_data=(unsigned char *) RelinquishMagickMemory(comp_data);
    /*
      Free frame pixels early.
    */
    frame_pixels[fi]=(unsigned char *)
      RelinquishMagickMemory(frame_pixels[fi]);
    fi++;
  }
  /*
    Write IEND chunk.
  */
  (void) WritePNGChunk(image,"IEND",(const unsigned char *) NULL,0,exception);
  (void) CloseBlob(image);
  /*
    Cleanup.
  */
  CleanupFramePixels(frame_pixels,frame_count);
  return(MagickTrue);
}

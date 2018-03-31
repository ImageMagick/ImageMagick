/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP    CCCC  DDDD                               %
%                            P   P  C      D   D                              %
%                            PPPP   C      D   D                              %
%                            P      C      D   D                              %
%                            P       CCCC  DDDD                               %
%                                                                             %
%                                                                             %
%                     Read/Write Photo CD Image Format                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/client.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/distort.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePCDImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecodeImage recovers the Huffman encoded luminance and chrominance
%  deltas.
%
%  The format of the DecodeImage method is:
%
%      MagickBooleanType DecodeImage(Image *image,unsigned char *luma,
%        unsigned char *chroma1,unsigned char *chroma2)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o luma: the address of a character buffer that contains the
%      luminance information.
%
%    o chroma1: the address of a character buffer that contains the
%      chrominance information.
%
%    o chroma2: the address of a character buffer that contains the
%      chrominance information.
%
*/
static MagickBooleanType DecodeImage(Image *image,unsigned char *luma,
  unsigned char *chroma1,unsigned char *chroma2,ExceptionInfo *exception)
{
#define IsSync(sum)  ((sum & 0xffffff00UL) == 0xfffffe00UL)
#define PCDGetBits(n) \
{  \
  sum=(sum << n) & 0xffffffff; \
  bits-=n; \
  while (bits <= 24) \
  { \
    if (p >= (buffer+0x800)) \
      { \
        count=ReadBlob(image,0x800,buffer); \
        p=buffer; \
      } \
    sum|=((unsigned int) (*p) << (24-bits)); \
    bits+=8; \
    p++; \
  } \
}

  typedef struct PCDTable
  {
    unsigned int
      length,
      sequence;

    MagickStatusType
      mask;

    unsigned char
      key;
  } PCDTable;

  PCDTable
    *pcd_table[3];

  register ssize_t
    i,
    j;

  register PCDTable
    *r;

  register unsigned char
    *p,
    *q;

  size_t
    bits,
    length,
    plane,
    pcd_length[3],
    row,
    sum;

  ssize_t
    count,
    quantum;

  unsigned char
    *buffer;

  /*
    Initialize Huffman tables.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(luma != (unsigned char *) NULL);
  assert(chroma1 != (unsigned char *) NULL);
  assert(chroma2 != (unsigned char *) NULL);
  buffer=(unsigned char *) AcquireQuantumMemory(0x800,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  sum=0;
  bits=32;
  p=buffer+0x800;
  for (i=0; i < 3; i++)
  {
    pcd_table[i]=(PCDTable *) NULL;
    pcd_length[i]=0;
  }
  for (i=0; i < (image->columns > 1536 ? 3 : 1); i++)
  {
    PCDGetBits(8);
    length=(sum & 0xff)+1;
    pcd_table[i]=(PCDTable *) AcquireQuantumMemory(length,
      sizeof(*pcd_table[i]));
    if (pcd_table[i] == (PCDTable *) NULL)
      {
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      }
    r=pcd_table[i];
    for (j=0; j < (ssize_t) length; j++)
    {
      PCDGetBits(8);
      r->length=(unsigned int) (sum & 0xff)+1;
      if (r->length > 16)
        {
          buffer=(unsigned char *) RelinquishMagickMemory(buffer);
          return(MagickFalse);
        }
      PCDGetBits(16);
      r->sequence=(unsigned int) (sum & 0xffff) << 16;
      PCDGetBits(8);
      r->key=(unsigned char) (sum & 0xff);
      r->mask=(~((1U << (32-r->length))-1));
      r++;
    }
    pcd_length[i]=(size_t) length;
  }
  /*
    Search for Sync byte.
  */
  for (i=0; i < 1; i++)
    PCDGetBits(16);
  for (i=0; i < 1; i++)
    PCDGetBits(16);
  while ((sum & 0x00fff000UL) != 0x00fff000UL)
    PCDGetBits(8);
  while (IsSync(sum) == 0)
    PCDGetBits(1);
  /*
    Recover the Huffman encoded luminance and chrominance deltas.
  */
  count=0;
  length=0;
  plane=0;
  row=0;
  q=luma;
  for ( ; ; )
  {
    if (IsSync(sum) != 0)
      {
        /*
          Determine plane and row number.
        */
        PCDGetBits(16);
        row=((sum >> 9) & 0x1fff);
        if (row == image->rows)
          break;
        PCDGetBits(8);
        plane=sum >> 30;
        PCDGetBits(16);
        switch (plane)
        {
          case 0:
          {
            q=luma+row*image->columns;
            count=(ssize_t) image->columns;
            break;
          }
          case 2:
          {
            q=chroma1+(row >> 1)*image->columns;
            count=(ssize_t) (image->columns >> 1);
            plane--;
            break;
          }
          case 3:
          {
            q=chroma2+(row >> 1)*image->columns;
            count=(ssize_t) (image->columns >> 1);
            plane--;
            break;
          }
          default:
          {
            ThrowBinaryException(CorruptImageError,"CorruptImage",
              image->filename);
          }
        }
        length=pcd_length[plane];
        continue;
      }
    /*
      Decode luminance or chrominance deltas.
    */
    r=pcd_table[plane];
    for (i=0; ((i < (ssize_t) length) && ((sum & r->mask) != r->sequence)); i++)
      r++;
    if ((row > image->rows) || (r == (PCDTable *) NULL))
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageWarning,"SkipToSyncByte","`%s'",image->filename);
        while ((sum & 0x00fff000) != 0x00fff000)
          PCDGetBits(8);
        while (IsSync(sum) == 0)
          PCDGetBits(1);
        continue;
      }
    if (r->key < 128)
      quantum=(ssize_t) (*q)+r->key;
    else
      quantum=(ssize_t) (*q)+r->key-256;
    *q=(unsigned char) ((quantum < 0) ? 0 : (quantum > 255) ? 255 : quantum);
    q++;
    PCDGetBits(r->length);
    count--;
  }
  /*
    Relinquish resources.
  */
  for (i=0; i < (image->columns > 1536 ? 3 : 1); i++)
    pcd_table[i]=(PCDTable *) RelinquishMagickMemory(pcd_table[i]);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P C D                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPCD() returns MagickTrue if the image format type, identified by the
%  magick string, is PCD.
%
%  The format of the IsPCD method is:
%
%      MagickBooleanType IsPCD(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPCD(const unsigned char *magick,const size_t length)
{
  if (length < 2052)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick+2048,"PCD_",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P C D I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPCDImage() reads a Photo CD image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.  Much of the PCD decoder was derived from
%  the program hpcdtoppm(1) by Hadmut Danisch.
%
%  The format of the ReadPCDImage method is:
%
%      image=ReadPCDImage(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *OverviewImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  Image
    *montage_image;

  MontageInfo
    *montage_info;

  register Image
    *p;

  /*
    Create the PCD Overview image.
  */
  for (p=image; p != (Image *) NULL; p=p->next)
  {
    (void) DeleteImageProperty(p,"label");
    (void) SetImageProperty(p,"label",DefaultTileLabel,exception);
  }
  montage_info=CloneMontageInfo(image_info,(MontageInfo *) NULL);
  (void) CopyMagickString(montage_info->filename,image_info->filename,
    MagickPathExtent);
  montage_image=MontageImageList(image_info,montage_info,image,exception);
  montage_info=DestroyMontageInfo(montage_info);
  if (montage_image == (Image *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  image=DestroyImageList(image);
  return(montage_image);
}

static void Upsample(const size_t width,const size_t height,
  const size_t scaled_width,unsigned char *pixels)
{
  register ssize_t
    x,
    y;

  register unsigned char
    *p,
    *q,
    *r;

  /*
    Create a new image that is a integral size greater than an existing one.
  */
  assert(pixels != (unsigned char *) NULL);
  for (y=0; y < (ssize_t) height; y++)
  {
    p=pixels+(height-1-y)*scaled_width+(width-1);
    q=pixels+((height-1-y) << 1)*scaled_width+((width-1) << 1);
    *q=(*p);
    *(q+1)=(*(p));
    for (x=1; x < (ssize_t) width; x++)
    {
      p--;
      q-=2;
      *q=(*p);
      *(q+1)=(unsigned char) ((((size_t) *p)+((size_t) *(p+1))+1) >> 1);
    }
  }
  for (y=0; y < (ssize_t) (height-1); y++)
  {
    p=pixels+((size_t) y << 1)*scaled_width;
    q=p+scaled_width;
    r=q+scaled_width;
    for (x=0; x < (ssize_t) (width-1); x++)
    {
      *q=(unsigned char) ((((size_t) *p)+((size_t) *r)+1) >> 1);
      *(q+1)=(unsigned char) ((((size_t) *p)+((size_t) *(p+2))+
        ((size_t) *r)+((size_t) *(r+2))+2) >> 2);
      q+=2;
      p+=2;
      r+=2;
    }
    *q++=(unsigned char) ((((size_t) *p++)+((size_t) *r++)+1) >> 1);
    *q++=(unsigned char) ((((size_t) *p++)+((size_t) *r++)+1) >> 1);
  }
  p=pixels+(2*height-2)*scaled_width;
  q=pixels+(2*height-1)*scaled_width;
  (void) memcpy(q,p,(size_t) (2*width));
}

static Image *ReadPCDImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define ThrowPCDException(exception,message) \
{ \
  if (header != (unsigned char *) NULL) \
    header=(unsigned char *) RelinquishMagickMemory(header); \
  if (luma != (unsigned char *) NULL) \
    luma=(unsigned char *) RelinquishMagickMemory(luma); \
  if (chroma2 != (unsigned char *) NULL) \
    chroma2=(unsigned char *) RelinquishMagickMemory(chroma2); \
  if (chroma1 != (unsigned char *) NULL) \
    chroma1=(unsigned char *) RelinquishMagickMemory(chroma1); \
  ThrowReaderException((exception),(message)); \
}

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  MagickSizeType
    number_pixels;

  register ssize_t
    i,
    y;

  register Quantum
    *q;

  register unsigned char
    *c1,
    *c2,
    *yy;

  size_t
    height,
    number_images,
    rotate,
    scene,
    width;

  ssize_t
    count,
    x;

  unsigned char
    *chroma1,
    *chroma2,
    *header,
    *luma;

  unsigned int
    overview;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Determine if this a PCD file.
  */
  header=(unsigned char *) AcquireQuantumMemory(0x800,3UL*sizeof(*header));
  if (header == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  chroma1=(unsigned char *) NULL;
  chroma2=(unsigned char *) NULL;
  luma=(unsigned char *) NULL;
  count=ReadBlob(image,3*0x800,header);
  if (count != (3*0x800))
    ThrowPCDException(CorruptImageError,"ImproperImageHeader");
  overview=LocaleNCompare((char *) header,"PCD_OPA",7) == 0;
  if ((LocaleNCompare((char *) header+0x800,"PCD",3) != 0) && (overview == 0))
    ThrowPCDException(CorruptImageError,"ImproperImageHeader");
  rotate=header[0x0e02] & 0x03;
  number_images=((header[10] << 8) | header[11]) & 0xffff;
  header=(unsigned char *) RelinquishMagickMemory(header);
  if ((overview != 0) &&
      (AcquireMagickResource(ListLengthResource,number_images) == MagickFalse))
    ThrowPCDException(ResourceLimitError,"ListLengthExceedsLimit");
  /*
    Determine resolution by scene specification.
  */
  if ((image->columns == 0) || (image->rows == 0))
    scene=3;
  else
    {
      width=192;
      height=128;
      for (scene=1; scene < 6; scene++)
      {
        if ((width >= image->columns) && (height >= image->rows))
          break;
        width<<=1;
        height<<=1;
      }
    }
  if (image_info->number_scenes != 0)
    scene=(size_t) MagickMin(image_info->scene,6);
  if (overview != 0)
    scene=1;
  /*
    Initialize image structure.
  */
  width=192;
  height=128;
  for (i=1; i < (ssize_t) MagickMin(scene,3); i++)
  {
    width<<=1;
    height<<=1;
  }
  image->columns=width;
  image->rows=height;
  image->depth=8;
  for ( ; i < (ssize_t) scene; i++)
  {
    image->columns<<=1;
    image->rows<<=1;
  }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Allocate luma and chroma memory.
  */
  number_pixels=(MagickSizeType) image->columns*image->rows;
  if (number_pixels != (size_t) number_pixels)
    ThrowPCDException(ResourceLimitError,"MemoryAllocationFailed");
  chroma1=(unsigned char *) AcquireQuantumMemory(image->columns+1UL,image->rows*
    10*sizeof(*chroma1));
  chroma2=(unsigned char *) AcquireQuantumMemory(image->columns+1UL,image->rows*
    10*sizeof(*chroma2));
  luma=(unsigned char *) AcquireQuantumMemory(image->columns+1UL,image->rows*
    10*sizeof(*luma));
  if ((chroma1 == (unsigned char *) NULL) ||
      (chroma2 == (unsigned char *) NULL) || (luma == (unsigned char *) NULL))
    ThrowPCDException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(chroma1,0,(image->columns+1UL)*image->rows*
    10*sizeof(*chroma1));
  (void) memset(chroma2,0,(image->columns+1UL)*image->rows*
    10*sizeof(*chroma2));
  (void) memset(luma,0,(image->columns+1UL)*image->rows*
    10*sizeof(*luma));
  /*
    Advance to image data.
  */
  offset=93;
  if (overview != 0)
    offset=2;
  else
    if (scene == 2)
      offset=20;
    else
      if (scene <= 1)
        offset=1;
  for (i=0; i < (ssize_t) (offset*0x800); i++)
    if (ReadBlobByte(image) == EOF)
      ThrowPCDException(CorruptImageError,"UnexpectedEndOfFile");
  if (overview != 0)
    {
      MagickProgressMonitor
        progress_monitor;

      register ssize_t
        j;

      /*
        Read thumbnails from overview image.
      */
      for (j=1; j <= (ssize_t) number_images; j++)
      {
        progress_monitor=SetImageProgressMonitor(image,
          (MagickProgressMonitor) NULL,image->client_data);
        (void) FormatLocaleString(image->filename,MagickPathExtent,
          "images/img%04ld.pcd",(long) j);
        (void) FormatLocaleString(image->magick_filename,MagickPathExtent,
          "images/img%04ld.pcd",(long) j);
        image->scene=(size_t) j;
        image->columns=width;
        image->rows=height;
        image->depth=8;
        yy=luma;
        c1=chroma1;
        c2=chroma2;
        for (y=0; y < (ssize_t) height; y+=2)
        {
          count=ReadBlob(image,width,yy);
          yy+=image->columns;
          count=ReadBlob(image,width,yy);
          yy+=image->columns;
          count=ReadBlob(image,width >> 1,c1);
          c1+=image->columns;
          count=ReadBlob(image,width >> 1,c2);
          c2+=image->columns;
          if (EOFBlob(image) != MagickFalse)
            ThrowPCDException(CorruptImageError,"UnexpectedEndOfFile");
        }
        Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma1);
        Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma2);
        /*
          Transfer luminance and chrominance channels.
        */
        yy=luma;
        c1=chroma1;
        c2=chroma2;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(image,ScaleCharToQuantum(*yy++),q);
            SetPixelGreen(image,ScaleCharToQuantum(*c1++),q);
            SetPixelBlue(image,ScaleCharToQuantum(*c2++),q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
        }
        image->colorspace=YCCColorspace;
        if (LocaleCompare(image_info->magick,"PCDS") == 0)
          (void) SetImageColorspace(image,sRGBColorspace,exception);
        if (EOFBlob(image) != MagickFalse)
          break;
        if (j < (ssize_t) number_images)
          {
            /*
              Allocate next image structure.
            */
            AcquireNextImage(image_info,image,exception);
            if (GetNextImageInList(image) == (Image *) NULL)
              {
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
            image=SyncNextImageInList(image);
          }
        (void) SetImageProgressMonitor(image,progress_monitor,
          image->client_data);
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,j-1,number_images);
            if (status == MagickFalse)
              break;
          }
      }
      chroma2=(unsigned char *) RelinquishMagickMemory(chroma2);
      chroma1=(unsigned char *) RelinquishMagickMemory(chroma1);
      luma=(unsigned char *) RelinquishMagickMemory(luma);
      image=GetFirstImageInList(image);
      return(OverviewImage(image_info,image,exception));
    }
  /*
    Read interleaved image.
  */
  yy=luma;
  c1=chroma1;
  c2=chroma2;
  for (y=0; y < (ssize_t) height; y+=2)
  {
    count=ReadBlob(image,width,yy);
    yy+=image->columns;
    count=ReadBlob(image,width,yy);
    yy+=image->columns;
    count=ReadBlob(image,width >> 1,c1);
    c1+=image->columns;
    count=ReadBlob(image,width >> 1,c2);
    c2+=image->columns;
    if (EOFBlob(image) != MagickFalse)
      ThrowPCDException(CorruptImageError,"UnexpectedEndOfFile");
  }
  if (scene >= 4)
    {
      /*
        Recover luminance deltas for 1536x1024 image.
      */
      Upsample(768,512,image->columns,luma);
      Upsample(384,256,image->columns,chroma1);
      Upsample(384,256,image->columns,chroma2);
      image->rows=1024;
      for (i=0; i < (4*0x800); i++)
        (void) ReadBlobByte(image);
      status=DecodeImage(image,luma,chroma1,chroma2,exception);
      if ((scene >= 5) && status)
        {
          /*
            Recover luminance deltas for 3072x2048 image.
          */
          Upsample(1536,1024,image->columns,luma);
          Upsample(768,512,image->columns,chroma1);
          Upsample(768,512,image->columns,chroma2);
          image->rows=2048;
          offset=TellBlob(image)/0x800+12;
          offset=SeekBlob(image,offset*0x800,SEEK_SET);
          status=DecodeImage(image,luma,chroma1,chroma2,exception);
          if ((scene >= 6) && (status != MagickFalse))
            {
              /*
                Recover luminance deltas for 6144x4096 image (vaporware).
              */
              Upsample(3072,2048,image->columns,luma);
              Upsample(1536,1024,image->columns,chroma1);
              Upsample(1536,1024,image->columns,chroma2);
              image->rows=4096;
            }
        }
    }
  Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma1);
  Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma2);
  /*
    Transfer luminance and chrominance channels.
  */
  yy=luma;
  c1=chroma1;
  c2=chroma2;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(image,ScaleCharToQuantum(*yy++),q);
      SetPixelGreen(image,ScaleCharToQuantum(*c1++),q);
      SetPixelBlue(image,ScaleCharToQuantum(*c2++),q);
      q+=GetPixelChannels(image);
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
  chroma2=(unsigned char *) RelinquishMagickMemory(chroma2);
  chroma1=(unsigned char *) RelinquishMagickMemory(chroma1);
  luma=(unsigned char *) RelinquishMagickMemory(luma);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  if (image_info->ping == MagickFalse)
    if ((rotate == 1) || (rotate == 3))
      {
        double
          degrees;

        Image
          *rotate_image;

        /*
          Rotate image.
        */
        degrees=rotate == 1 ? -90.0 : 90.0;
        rotate_image=RotateImage(image,degrees,exception);
        if (rotate_image != (Image *) NULL)
          {
            image=DestroyImage(image);
            image=rotate_image;
          }
      }
  /*
    Set CCIR 709 primaries with a D65 white point.
  */
  image->chromaticity.red_primary.x=0.6400f;
  image->chromaticity.red_primary.y=0.3300f;
  image->chromaticity.green_primary.x=0.3000f;
  image->chromaticity.green_primary.y=0.6000f;
  image->chromaticity.blue_primary.x=0.1500f;
  image->chromaticity.blue_primary.y=0.0600f;
  image->chromaticity.white_point.x=0.3127f;
  image->chromaticity.white_point.y=0.3290f;
  image->gamma=1.000f/2.200f;
  image->colorspace=YCCColorspace;
  if (LocaleCompare(image_info->magick,"PCDS") == 0)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P C D I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPCDImage() adds attributes for the PCD image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPCDImage method is:
%
%      size_t RegisterPCDImage(void)
%
*/
ModuleExport size_t RegisterPCDImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PCD","PCD","Photo CD");
  entry->decoder=(DecodeImageHandler *) ReadPCDImage;
  entry->encoder=(EncodeImageHandler *) WritePCDImage;
  entry->magick=(IsImageFormatHandler *) IsPCD;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PCD","PCDS","Photo CD");
  entry->decoder=(DecodeImageHandler *) ReadPCDImage;
  entry->encoder=(EncodeImageHandler *) WritePCDImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P C D I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPCDImage() removes format registrations made by the
%  PCD module from the list of supported formats.
%
%  The format of the UnregisterPCDImage method is:
%
%      UnregisterPCDImage(void)
%
*/
ModuleExport void UnregisterPCDImage(void)
{
  (void) UnregisterMagickInfo("PCD");
  (void) UnregisterMagickInfo("PCDS");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P C D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePCDImage() writes an image in the Photo CD encoded image format.
%
%  The format of the WritePCDImage method is:
%
%      MagickBooleanType WritePCDImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType WritePCDTile(Image *image,const char *page_geometry,
  const size_t tile_columns,const size_t tile_rows,ExceptionInfo *exception)
{
  GeometryInfo
    geometry_info;

  Image
    *downsample_image,
    *tile_image;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  RectangleInfo
    geometry;

  register const Quantum
    *p,
    *q;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  /*
    Scale image to tile size.
  */
  SetGeometry(image,&geometry);
  (void) ParseMetaGeometry(page_geometry,&geometry.x,&geometry.y,
    &geometry.width,&geometry.height);
  if ((geometry.width % 2) != 0)
    geometry.width--;
  if ((geometry.height % 2) != 0)
    geometry.height--;
  tile_image=ResizeImage(image,geometry.width,geometry.height,TriangleFilter,
    exception);
  if (tile_image == (Image *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(page_geometry,&geometry_info);
  geometry.width=(size_t) geometry_info.rho;
  geometry.height=(size_t) geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    geometry.height=geometry.width;
  if ((tile_image->columns != geometry.width) ||
      (tile_image->rows != geometry.height))
    {
      Image
        *bordered_image;

      RectangleInfo
        border_info;

      /*
        Put a border around the image.
      */
      border_info.width=(geometry.width-tile_image->columns+1) >> 1;
      border_info.height=(geometry.height-tile_image->rows+1) >> 1;
      bordered_image=BorderImage(tile_image,&border_info,image->compose,
        exception);
      if (bordered_image == (Image *) NULL)
        return(MagickFalse);
      tile_image=DestroyImage(tile_image);
      tile_image=bordered_image;
    }
  if ((tile_image->columns != tile_columns) || (tile_image->rows != tile_rows))
    {
      Image
        *resize_image;

      resize_image=ResizeImage(tile_image,tile_columns,tile_rows,
        tile_image->filter,exception);
      if (resize_image != (Image *) NULL)
        {
          tile_image=DestroyImage(tile_image);
          tile_image=resize_image;
        }
    }
  (void) TransformImageColorspace(tile_image,YCCColorspace,exception);
  downsample_image=ResizeImage(tile_image,tile_image->columns/2,
    tile_image->rows/2,TriangleFilter,exception);
  if (downsample_image == (Image *) NULL)
    return(MagickFalse);
  /*
    Write tile to PCD file.
  */
  for (y=0; y < (ssize_t) tile_image->rows; y+=2)
  {
    p=GetVirtualPixels(tile_image,0,y,tile_image->columns,2,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) (tile_image->columns << 1); x++)
    {
      (void) WriteBlobByte(image,ScaleQuantumToChar(GetPixelRed(tile_image,p)));
      p+=GetPixelChannels(tile_image);
    }
    q=GetVirtualPixels(downsample_image,0,y >> 1,downsample_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) downsample_image->columns; x++)
    {
      (void) WriteBlobByte(image,ScaleQuantumToChar(
        GetPixelGreen(tile_image,q)));
      q+=GetPixelChannels(tile_image);
    }
    q=GetVirtualPixels(downsample_image,0,y >> 1,downsample_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) downsample_image->columns; x++)
    {
      (void) WriteBlobByte(image,ScaleQuantumToChar(
        GetPixelBlue(tile_image,q)));
      q+=GetPixelChannels(tile_image);
    }
    status=SetImageProgress(image,SaveImageTag,y,tile_image->rows);
    if (status == MagickFalse)
      break;
  }
  for (i=0; i < 0x800; i++)
    (void) WriteBlobByte(image,'\0');
  downsample_image=DestroyImage(downsample_image);
  tile_image=DestroyImage(tile_image);
  return(MagickTrue);
}

static MagickBooleanType WritePCDImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  Image
    *pcd_image;

  MagickBooleanType
    status;

  register ssize_t
    i;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  pcd_image=image;
  if (image->columns < image->rows)
    {
      Image
        *rotate_image;

      /*
        Rotate portrait to landscape.
      */
      rotate_image=RotateImage(image,90.0,exception);
      if (rotate_image == (Image *) NULL)
        return(MagickFalse);
      pcd_image=rotate_image;
      DestroyBlob(rotate_image);
      pcd_image->blob=ReferenceBlob(image->blob);
    }
  /*
    Open output image file.
  */
  status=OpenBlob(image_info,pcd_image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      if (pcd_image != image)
        pcd_image=DestroyImage(pcd_image);
      return(status);
    }
  if (IssRGBCompatibleColorspace(pcd_image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(pcd_image,sRGBColorspace,exception);
  /*
    Write PCD image header.
  */
  for (i=0; i < 32; i++)
    (void) WriteBlobByte(pcd_image,0xff);
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x0e);
  for (i=0; i < 8; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x01);
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x05);
  for (i=0; i < 8; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x0A);
  for (i=0; i < 36; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x01);
  for (i=0; i < 1944; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  (void) WriteBlob(pcd_image,7,(const unsigned char *) "PCD_IPI");
  (void) WriteBlobByte(pcd_image,0x06);
  for (i=0; i < 1530; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  if (image->columns < image->rows)
    (void) WriteBlobByte(pcd_image,'\1');
  else
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < (3*0x800-1539); i++)
    (void) WriteBlobByte(pcd_image,'\0');
  /*
    Write PCD tiles.
  */
  status=WritePCDTile(pcd_image,"768x512>",192,128,exception);
  status=WritePCDTile(pcd_image,"768x512>",384,256,exception);
  status=WritePCDTile(pcd_image,"768x512>",768,512,exception);
  (void) CloseBlob(pcd_image);
  if (pcd_image != image)
    pcd_image=DestroyImage(pcd_image);
  return(status);
}

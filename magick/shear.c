/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      SSSSS  H   H  EEEEE   AAA    RRRR                      %
%                      SS     H   H  E      A   A   R   R                     %
%                       SSS   HHHHH  EEE    AAAAA   RRRR                      %
%                         SS  H   H  E      A   A   R R                       %
%                      SSSSS  H   H  EEEEE  A   A   R  R                      %
%                                                                             %
%                                                                             %
%    MagickCore Methods to Shear or Rotate an Image by an Arbitrary Angle     %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                  July 1992                                  %
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
%  The XShearImage() and YShearImage() methods are based on the paper "A Fast
%  Algorithm for General Raster Rotatation" by Alan W. Paeth, Graphics
%  Interface '86 (Vancouver).  ShearRotateImage() is adapted from a similar
%  method based on the Paeth paper written by Michael Halle of the Spatial
%  Imaging Group, MIT Media Lab.
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/attribute.h"
#include "magick/blob-private.h"
#include "magick/cache-private.h"
#include "magick/channel.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/decorate.h"
#include "magick/distort.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/memory_.h"
#include "magick/list.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/resource_.h"
#include "magick/shear.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/threshold.h"
#include "magick/transform.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C r o p T o F i t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CropToFitImage() crops the sheared image as determined by the bounding box
%  as defined by width and height and shearing angles.
%
%  The format of the CropToFitImage method is:
%
%      MagickBooleanType CropToFitImage(Image **image,
%        const MagickRealType x_shear,const MagickRealType x_shear,
%        const MagickRealType width,const MagickRealType height,
%        const MagickBooleanType rotate,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o x_shear, y_shear, width, height: Defines a region of the image to crop.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType CropToFitImage(Image **image,
  const MagickRealType x_shear,const MagickRealType y_shear,
  const MagickRealType width,const MagickRealType height,
  const MagickBooleanType rotate,ExceptionInfo *exception)
{
  Image
    *crop_image;

  PointInfo
    extent[4],
    min,
    max;

  RectangleInfo
    geometry,
    page;

  register ssize_t
    i;

  /*
    Calculate the rotated image size.
  */
  extent[0].x=(double) (-width/2.0);
  extent[0].y=(double) (-height/2.0);
  extent[1].x=(double) width/2.0;
  extent[1].y=(double) (-height/2.0);
  extent[2].x=(double) (-width/2.0);
  extent[2].y=(double) height/2.0;
  extent[3].x=(double) width/2.0;
  extent[3].y=(double) height/2.0;
  for (i=0; i < 4; i++)
  {
    extent[i].x+=x_shear*extent[i].y;
    extent[i].y+=y_shear*extent[i].x;
    if (rotate != MagickFalse)
      extent[i].x+=x_shear*extent[i].y;
    extent[i].x+=(double) (*image)->columns/2.0;
    extent[i].y+=(double) (*image)->rows/2.0;
  }
  min=extent[0];
  max=extent[0];
  for (i=1; i < 4; i++)
  {
    if (min.x > extent[i].x)
      min.x=extent[i].x;
    if (min.y > extent[i].y)
      min.y=extent[i].y;
    if (max.x < extent[i].x)
      max.x=extent[i].x;
    if (max.y < extent[i].y)
      max.y=extent[i].y;
  }
  geometry.x=(ssize_t) ceil(min.x-0.5);
  geometry.y=(ssize_t) ceil(min.y-0.5);
  geometry.width=(size_t) floor(max.x-min.x+0.5);
  geometry.height=(size_t) floor(max.y-min.y+0.5);
  page=(*image)->page;
  (void) ParseAbsoluteGeometry("0x0+0+0",&(*image)->page);
  crop_image=CropImage(*image,&geometry,exception);
  if (crop_image == (Image *) NULL)
    return(MagickFalse);
  crop_image->page=page;
  *image=DestroyImage(*image);
  *image=crop_image;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D e s k e w I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeskewImage() removes skew from the image.  Skew is an artifact that
%  occurs in scanned images because of the camera being misaligned,
%  imperfections in the scanning or surface, or simply because the paper was
%  not placed completely flat when scanned.
%
%  The amount of rotation calculated to deskew the image is saved in the
%  artifact "deskew:angle".
%
%  If the artifact "deskew:auto-crop" is given the image will be automatically
%  cropped of the excess background.  The value is the border width of all
%  pixels around the edge that will be used to determine an average border
%  color for the automatic trim.
%
%  The format of the DeskewImage method is:
%
%      Image *DeskewImage(const Image *image,const double threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: separate background from foreground.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _RadonInfo
{
  CacheType
    type;

  size_t
    width,
    height;

  MagickSizeType
    length;

  MagickBooleanType
    mapped;

  char
    path[MaxTextExtent];

  int
    file;

  unsigned short
    *cells;
} RadonInfo;

static RadonInfo *DestroyRadonInfo(RadonInfo *radon_info)
{
  assert(radon_info != (RadonInfo *) NULL);
  switch (radon_info->type)
  {
    case MemoryCache:
    {
      if (radon_info->mapped == MagickFalse)
        radon_info->cells=(unsigned short *) RelinquishMagickMemory(
          radon_info->cells);
      else
        {
          (void) UnmapBlob(radon_info->cells,(size_t) radon_info->length);
          radon_info->cells=(unsigned short *) NULL;
        }
      RelinquishMagickResource(MemoryResource,radon_info->length);
      break;
    }
    case MapCache:
    {
      (void) UnmapBlob(radon_info->cells,(size_t) radon_info->length);
      radon_info->cells=(unsigned short *) NULL;
      RelinquishMagickResource(MapResource,radon_info->length);
    }
    case DiskCache:
    {
      if (radon_info->file != -1)
        (void) close(radon_info->file);
      (void) RelinquishUniqueFileResource(radon_info->path);
      RelinquishMagickResource(DiskResource,radon_info->length);
      break;
    }
    default:
      break;
  }
  return((RadonInfo *) RelinquishMagickMemory(radon_info));
}

static MagickBooleanType ResetRadonCells(RadonInfo *radon_info)
{
  register ssize_t
    x;

  ssize_t
    count,
    y;

  unsigned short
    value;

  if (radon_info->type != DiskCache)
    {
      (void) ResetMagickMemory(radon_info->cells,0,(size_t) radon_info->length);
      return(MagickTrue);
    }
  value=0;
  (void) lseek(radon_info->file,0,SEEK_SET);
  for (y=0; y < (ssize_t) radon_info->height; y++)
  {
    for (x=0; x < (ssize_t) radon_info->width; x++)
    {
      count=write(radon_info->file,&value,sizeof(*radon_info->cells));
      if (count != (ssize_t) sizeof(*radon_info->cells))
        break;
    }
    if (x < (ssize_t) radon_info->width)
      break;
  }
  return(y < (ssize_t) radon_info->height ? MagickFalse : MagickTrue);
}

static RadonInfo *AcquireRadonInfo(const Image *image,const size_t width,
  const size_t height,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  RadonInfo
    *radon_info;

  radon_info=(RadonInfo *) AcquireMagickMemory(sizeof(*radon_info));
  if (radon_info == (RadonInfo *) NULL)
    return((RadonInfo *) NULL);
  (void) ResetMagickMemory(radon_info,0,sizeof(*radon_info));
  radon_info->width=width;
  radon_info->height=height;
  radon_info->length=(MagickSizeType) width*height*sizeof(*radon_info->cells);
  radon_info->type=MemoryCache;
  status=AcquireMagickResource(AreaResource,radon_info->length);
  if ((status != MagickFalse) &&
      (radon_info->length == (MagickSizeType) ((size_t) radon_info->length)))
    {
      status=AcquireMagickResource(MemoryResource,radon_info->length);
      if (status != MagickFalse)
        {
          radon_info->mapped=MagickFalse;
          radon_info->cells=(unsigned short *) AcquireMagickMemory((size_t)
            radon_info->length);
          if (radon_info->cells == (unsigned short *) NULL)
            {
              radon_info->mapped=MagickTrue;
              radon_info->cells=(unsigned short *) MapBlob(-1,IOMode,0,(size_t)
                radon_info->length);
            }
          if (radon_info->cells == (unsigned short *) NULL)
            RelinquishMagickResource(MemoryResource,radon_info->length);
        }
    }
  radon_info->file=(-1);
  if (radon_info->cells == (unsigned short *) NULL)
    {
      status=AcquireMagickResource(DiskResource,radon_info->length);
      if (status == MagickFalse)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
            "CacheResourcesExhausted","`%s'",image->filename);
          return(DestroyRadonInfo(radon_info));
        }
      radon_info->type=DiskCache;
      (void) AcquireMagickResource(MemoryResource,radon_info->length);
      radon_info->file=AcquireUniqueFileResource(radon_info->path);
      if (radon_info->file == -1)
        return(DestroyRadonInfo(radon_info));
      status=AcquireMagickResource(MapResource,radon_info->length);
      if (status != MagickFalse)
        {
          status=ResetRadonCells(radon_info);
          if (status != MagickFalse)
            {
              radon_info->cells=(unsigned short *) MapBlob(radon_info->file,
                IOMode,0,(size_t) radon_info->length);
              if (radon_info->cells != (unsigned short *) NULL)
                radon_info->type=MapCache;
              else
                RelinquishMagickResource(MapResource,radon_info->length);
            }
        }
    }
  return(radon_info);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static inline ssize_t ReadRadonCell(const RadonInfo *radon_info,
  const MagickOffsetType offset,const size_t length,unsigned char *buffer)
{
  register ssize_t
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PPREAD)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ReadRadonCell)
#endif
  {
    i=(-1);
    if (lseek(radon_info->file,offset,SEEK_SET) >= 0)
      {
#endif
        count=0;
        for (i=0; i < (ssize_t) length; i+=count)
        {
#if !defined(MAGICKCORE_HAVE_PPREAD)
          count=read(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX));
#else
          count=pread(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX),offset+i);
#endif
          if (count > 0)
            continue;
          count=0;
          if (errno != EINTR)
            {
              i=(-1);
              break;
            }
        }
#if !defined(MAGICKCORE_HAVE_PPREAD)
      }
  }
#endif
  return(i);
}

static inline ssize_t WriteRadonCell(const RadonInfo *radon_info,
  const MagickOffsetType offset,const size_t length,const unsigned char *buffer)
{
  register ssize_t
    i;

  ssize_t
    count;

  i=0;
#if !defined(MAGICKCORE_HAVE_PWRITE)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_WriteRadonCell)
#endif
  {
    if (lseek(radon_info->file,offset,SEEK_SET) >= 0)
      {
#endif
        count=0;
        for (i=0; i < (ssize_t) length; i+=count)
        {
#if !defined(MAGICKCORE_HAVE_PWRITE)
          count=write(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX));
#else
          count=pwrite(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX),offset+i);
#endif
          if (count > 0)
            continue;
          count=0;
          if (errno != EINTR)
            {
              i=(-1);
              break;
            }
        }
#if !defined(MAGICKCORE_HAVE_PWRITE)
      }
  }
#endif
  return(i);
}

static inline unsigned short GetRadonCell(const RadonInfo *radon_info,
  const ssize_t x,const ssize_t y)
{
  MagickOffsetType
    i;

  unsigned short
    value;

  i=(MagickOffsetType) radon_info->height*x+y;
  if ((i < 0) ||
      ((MagickSizeType) (i*sizeof(*radon_info->cells)) >= radon_info->length))
    return(0);
  if (radon_info->type != DiskCache)
    return(radon_info->cells[i]);
  value=0;
  (void) ReadRadonCell(radon_info,i*sizeof(*radon_info->cells),
    sizeof(*radon_info->cells),(unsigned char *) &value);
  return(value);
}

static inline MagickBooleanType SetRadonCell(const RadonInfo *radon_info,
  const ssize_t x,const ssize_t y,const unsigned short value)
{
  MagickOffsetType
    i;

  ssize_t
    count;

  i=(MagickOffsetType) radon_info->height*x+y;
  if ((i < 0) ||
      ((MagickSizeType) (i*sizeof(*radon_info->cells)) >= radon_info->length))
    return(MagickFalse);
  if (radon_info->type != DiskCache)
    {
      radon_info->cells[i]=value;
      return(MagickTrue);
    }
  count=WriteRadonCell(radon_info,i*sizeof(*radon_info->cells),
    sizeof(*radon_info->cells),(const unsigned char *) &value);
  if (count != (ssize_t) sizeof(*radon_info->cells))
    return(MagickFalse);
  return(MagickTrue);
}

static void RadonProjection(const Image *image,RadonInfo *source_cells,
  RadonInfo *destination_cells,const ssize_t sign,size_t *projection)
{
  RadonInfo
    *swap;

  register ssize_t
    x;

  register RadonInfo
    *p,
    *q;

  size_t
    step;

  p=source_cells;
  q=destination_cells;
  for (step=1; step < p->width; step*=2)
  {
    for (x=0; x < (ssize_t) p->width; x+=2*(ssize_t) step)
    {
      register ssize_t
        i;

      ssize_t
        y;

      unsigned short
        cell;

      for (i=0; i < (ssize_t) step; i++)
      {
        for (y=0; y < (ssize_t) (p->height-i-1); y++)
        {
          cell=GetRadonCell(p,x+i,y);
          (void) SetRadonCell(q,x+2*i,y,cell+GetRadonCell(p,x+i+(ssize_t)
            step,y+i));
          (void) SetRadonCell(q,x+2*i+1,y,cell+GetRadonCell(p,x+i+(ssize_t)
            step,y+i+1));
        }
        for ( ; y < (ssize_t) (p->height-i); y++)
        {
          cell=GetRadonCell(p,x+i,y);
          (void) SetRadonCell(q,x+2*i,y,cell+GetRadonCell(p,x+i+(ssize_t) step,
            y+i));
          (void) SetRadonCell(q,x+2*i+1,y,cell);
        }
        for ( ; y < (ssize_t) p->height; y++)
        {
          cell=GetRadonCell(p,x+i,y);
          (void) SetRadonCell(q,x+2*i,y,cell);
          (void) SetRadonCell(q,x+2*i+1,y,cell);
        }
      }
    }
    swap=p;
    p=q;
    q=swap;
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) \
    magick_threads(image,image,1,1)
#endif
  for (x=0; x < (ssize_t) p->width; x++)
  {
    register ssize_t
      y;

    size_t
      sum;

    sum=0;
    for (y=0; y < (ssize_t) (p->height-1); y++)
    {
      ssize_t
        delta;

      delta=GetRadonCell(p,x,y)-(ssize_t) GetRadonCell(p,x,y+1);
      sum+=delta*delta;
    }
    projection[p->width+sign*x-1]=sum;
  }
}

static MagickBooleanType RadonTransform(const Image *image,
  const double threshold,size_t *projection,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  RadonInfo
    *destination_cells,
    *source_cells;

  register ssize_t
    i;

  size_t
    count,
    width;

  ssize_t
    y;

  unsigned char
    byte;

  unsigned short
    bits[256];

  for (width=1; width < ((image->columns+7)/8); width<<=1) ;
  source_cells=AcquireRadonInfo(image,width,image->rows,exception);
  destination_cells=AcquireRadonInfo(image,width,image->rows,exception);
  if ((source_cells == (RadonInfo *) NULL) ||
      (destination_cells == (RadonInfo *) NULL))
    {
      if (destination_cells != (RadonInfo *) NULL)
        destination_cells=DestroyRadonInfo(destination_cells);
      if (source_cells != (RadonInfo *) NULL)
        source_cells=DestroyRadonInfo(source_cells);
      return(MagickFalse);
    }
  if (ResetRadonCells(source_cells) == MagickFalse)
    {
      destination_cells=DestroyRadonInfo(destination_cells);
      source_cells=DestroyRadonInfo(source_cells);
      return(MagickFalse);
    }
  for (i=0; i < 256; i++)
  {
    byte=(unsigned char) i;
    for (count=0; byte != 0; byte>>=1)
      count+=byte & 0x01;
    bits[i]=(unsigned short) count;
  }
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,1,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      i,
      x;

    size_t
      bit,
      byte;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    bit=0;
    byte=0;
    i=(ssize_t) (image->columns+7)/8;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      byte<<=1;
      if (((MagickRealType) GetPixelRed(p) < threshold) ||
          ((MagickRealType) GetPixelGreen(p) < threshold) ||
          ((MagickRealType) GetPixelBlue(p) < threshold))
        byte|=0x01;
      bit++;
      if (bit == 8)
        {
          (void) SetRadonCell(source_cells,--i,y,bits[byte]);
          bit=0;
          byte=0;
        }
      p++;
    }
    if (bit != 0)
      {
        byte<<=(8-bit);
        (void) SetRadonCell(source_cells,--i,y,bits[byte]);
      }
  }
  RadonProjection(image,source_cells,destination_cells,-1,projection);
  (void) ResetRadonCells(source_cells);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      i,
      x;

    size_t
      bit,
      byte;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    bit=0;
    byte=0;
    i=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      byte<<=1;
      if (((MagickRealType) GetPixelRed(p) < threshold) ||
          ((MagickRealType) GetPixelGreen(p) < threshold) ||
          ((MagickRealType) GetPixelBlue(p) < threshold))
        byte|=0x01;
      bit++;
      if (bit == 8)
        {
          (void) SetRadonCell(source_cells,i++,y,bits[byte]);
          bit=0;
          byte=0;
        }
      p++;
    }
    if (bit != 0)
      {
        byte<<=(8-bit);
        (void) SetRadonCell(source_cells,i++,y,bits[byte]);
      }
  }
  RadonProjection(image,source_cells,destination_cells,1,projection);
  image_view=DestroyCacheView(image_view);
  destination_cells=DestroyRadonInfo(destination_cells);
  source_cells=DestroyRadonInfo(source_cells);
  return(MagickTrue);
}

static void GetImageBackgroundColor(Image *image,const ssize_t offset,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickPixelPacket
    background;

  MagickRealType
    count;

  ssize_t
    y;

  /*
    Compute average background color.
  */
  if (offset <= 0)
    return;
  GetMagickPixelPacket(image,&background);
  count=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    if ((y >= offset) && (y < ((ssize_t) image->rows-offset)))
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      continue;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x >= offset) && (x < ((ssize_t) image->columns-offset)))
        continue;
      background.red+=QuantumScale*GetPixelRed(p);
      background.green+=QuantumScale*GetPixelGreen(p);
      background.blue+=QuantumScale*GetPixelBlue(p);
      background.opacity+=QuantumScale*GetPixelOpacity(p);
      count++;
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  image->background_color.red=ClampToQuantum((MagickRealType) QuantumRange*
    background.red/count);
  image->background_color.green=ClampToQuantum((MagickRealType) QuantumRange*
    background.green/count);
  image->background_color.blue=ClampToQuantum((MagickRealType) QuantumRange*
    background.blue/count);
  image->background_color.opacity=ClampToQuantum((MagickRealType) QuantumRange*
    background.opacity/count);
}

MagickExport Image *DeskewImage(const Image *image,const double threshold,
  ExceptionInfo *exception)
{
  AffineMatrix
    affine_matrix;

  const char
    *artifact;

  double
    degrees;

  Image
    *clone_image,
    *crop_image,
    *deskew_image,
    *median_image;

  MagickBooleanType
    status;

  RectangleInfo
    geometry;

  register ssize_t
    i;

  size_t
    max_projection,
    *projection,
    width;

  ssize_t
    skew;

  /*
    Compute deskew angle.
  */
  for (width=1; width < ((image->columns+7)/8); width<<=1) ;
  projection=(size_t *) AcquireQuantumMemory((size_t) (2*width-1),
    sizeof(*projection));
  if (projection == (size_t *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  status=RadonTransform(image,threshold,projection,exception);
  if (status == MagickFalse)
    {
      projection=(size_t *) RelinquishMagickMemory(projection);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  max_projection=0;
  skew=0;
  for (i=0; i < (ssize_t) (2*width-1); i++)
  {
    if (projection[i] > max_projection)
      {
        skew=i-(ssize_t) width+1;
        max_projection=projection[i];
      }
  }
  projection=(size_t *) RelinquishMagickMemory(projection);
  degrees=RadiansToDegrees(-atan((double) skew/width/8));
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TransformEvent,GetMagickModule(),
      "  Deskew angle: %g",degrees);
  /*
    Deskew image.
  */
  clone_image=CloneImage(image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return((Image *) NULL);
  {
    char
      angle[MaxTextExtent];

    (void) FormatLocaleString(angle,MaxTextExtent,"%g",degrees);
    (void) SetImageArtifact(clone_image,"deskew:angle",angle);
  }
  (void) SetImageVirtualPixelMethod(clone_image,BackgroundVirtualPixelMethod);
  affine_matrix.sx=cos(DegreesToRadians(fmod((double) degrees,360.0)));
  affine_matrix.rx=sin(DegreesToRadians(fmod((double) degrees,360.0)));
  affine_matrix.ry=(-sin(DegreesToRadians(fmod((double) degrees,360.0))));
  affine_matrix.sy=cos(DegreesToRadians(fmod((double) degrees,360.0)));
  affine_matrix.tx=0.0;
  affine_matrix.ty=0.0;
  artifact=GetImageArtifact(image,"deskew:auto-crop");
  if (artifact == (const char *) NULL)
    {
      deskew_image=AffineTransformImage(clone_image,&affine_matrix,exception);
      clone_image=DestroyImage(clone_image);
      return(deskew_image);
    }
  /*
    Auto-crop image.
  */
  GetImageBackgroundColor(clone_image,(ssize_t) StringToLong(artifact),
    exception);
  deskew_image=AffineTransformImage(clone_image,&affine_matrix,exception);
  clone_image=DestroyImage(clone_image);
  if (deskew_image == (Image *) NULL)
    return((Image *) NULL);
  median_image=StatisticImage(deskew_image,MedianStatistic,3,3,exception);
  if (median_image == (Image *) NULL)
    {
      deskew_image=DestroyImage(deskew_image);
      return((Image *) NULL);
    }
  geometry=GetImageBoundingBox(median_image,exception);
  median_image=DestroyImage(median_image);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TransformEvent,GetMagickModule(),"  Deskew geometry: "
      "%.20gx%.20g%+.20g%+.20g",(double) geometry.width,(double)
      geometry.height,(double) geometry.x,(double) geometry.y);
  crop_image=CropImage(deskew_image,&geometry,exception);
  deskew_image=DestroyImage(deskew_image);
  return(crop_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e g r a l R o t a t e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IntegralRotateImage() rotates the image an integral of 90 degrees.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the rotated image.
%
%  The format of the IntegralRotateImage method is:
%
%      Image *IntegralRotateImage(const Image *image,size_t rotations,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o rotations: Specifies the number of 90 degree rotations.
%
*/
MagickExport Image *IntegralRotateImage(const Image *image,size_t rotations,
  ExceptionInfo *exception)
{
#define RotateImageTag  "Rotate/Image"

  CacheView
    *image_view,
    *rotate_view;

  Image
    *rotate_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RectangleInfo
    page;

  ssize_t
    y;

  /*
    Initialize rotated image attributes.
  */
  assert(image != (Image *) NULL);
  page=image->page;
  rotations%=4;
  if (rotations == 0)
    return(CloneImage(image,0,0,MagickTrue,exception));
  if ((rotations == 1) || (rotations == 3))
    rotate_image=CloneImage(image,image->rows,image->columns,MagickTrue,
      exception);
  else
    rotate_image=CloneImage(image,image->columns,image->rows,MagickTrue,
      exception);
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Integral rotate the image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  rotate_view=AcquireAuthenticCacheView(rotate_image,exception);
  switch (rotations)
  {
    case 0:
    {
      /*
        Rotate 0 degrees.
      */
      break;
    }
    case 1:
    {
      size_t
        tile_height,
        tile_width;

      ssize_t
        tile_y;

      /*
        Rotate 90 degrees.
      */
      GetPixelCacheTileSize(image,&tile_width,&tile_height);
      tile_width=image->columns;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_threads(image,image,1,1)
#endif
      for (tile_y=0; tile_y < (ssize_t) image->rows; tile_y+=(ssize_t) tile_height)
      {
        register ssize_t
          tile_x;

        if (status == MagickFalse)
          continue;
        for (tile_x=0; tile_x < (ssize_t) image->columns; tile_x+=(ssize_t) tile_width)
        {
          MagickBooleanType
            sync;

          register const IndexPacket
            *restrict indexes;

          register const PixelPacket
            *restrict p;

          register IndexPacket
            *restrict rotate_indexes;

          register PixelPacket
            *restrict q;

          register ssize_t
            y;

          size_t
            height,
            width;

          width=tile_width;
          if ((tile_x+(ssize_t) tile_width) > (ssize_t) image->columns)
            width=(size_t) (tile_width-(tile_x+tile_width-image->columns));
          height=tile_height;
          if ((tile_y+(ssize_t) tile_height) > (ssize_t) image->rows)
            height=(size_t) (tile_height-(tile_y+tile_height-image->rows));
          p=GetCacheViewVirtualPixels(image_view,tile_x,tile_y,width,height,
            exception);
          if (p == (const PixelPacket *) NULL)
            {
              status=MagickFalse;
              break;
            }
          indexes=GetCacheViewVirtualIndexQueue(image_view);
          for (y=0; y < (ssize_t) width; y++)
          {
            register const PixelPacket
              *restrict tile_pixels;

            register ssize_t
              x;

            if (status == MagickFalse)
              continue;
            q=QueueCacheViewAuthenticPixels(rotate_view,(ssize_t)
              (rotate_image->columns-(tile_y+height)),y+tile_x,height,1,
              exception);
            if (q == (PixelPacket *) NULL)
              {
                status=MagickFalse;
                continue;
              }
            tile_pixels=p+(height-1)*width+y;
            for (x=0; x < (ssize_t) height; x++)
            {
              *q++=(*tile_pixels);
              tile_pixels-=width;
            }
            rotate_indexes=GetCacheViewAuthenticIndexQueue(rotate_view);
            if ((indexes != (IndexPacket *) NULL) &&
                (rotate_indexes != (IndexPacket *) NULL))
              {
                register const IndexPacket
                  *restrict tile_indexes;

                tile_indexes=indexes+(height-1)*width+y;
                for (x=0; x < (ssize_t) height; x++)
                {
                  *rotate_indexes++=(*tile_indexes);
                  tile_indexes-=width;
                }
              }
            sync=SyncCacheViewAuthenticPixels(rotate_view,exception);
            if (sync == MagickFalse)
              status=MagickFalse;
          }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_IntegralRotateImage)
#endif
            proceed=SetImageProgress(image,RotateImageTag,progress+=tile_height,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      (void) SetImageProgress(image,RotateImageTag,(MagickOffsetType)
        image->rows-1,image->rows);
      Swap(page.width,page.height);
      Swap(page.x,page.y);
      if (page.width != 0)
        page.x=(ssize_t) (page.width-rotate_image->columns-page.x);
      break;
    }
    case 2:
    {
      /*
        Rotate 180 degrees.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_threads(image,image,1,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const IndexPacket
          *restrict indexes;

        register const PixelPacket
          *restrict p;

        register IndexPacket
          *restrict rotate_indexes;

        register PixelPacket
          *restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,
          exception);
        q=QueueCacheViewAuthenticPixels(rotate_view,0,(ssize_t) (image->rows-y-
          1),image->columns,1,exception);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        indexes=GetCacheViewVirtualIndexQueue(image_view);
        rotate_indexes=GetCacheViewAuthenticIndexQueue(rotate_view);
        q+=image->columns;
        for (x=0; x < (ssize_t) image->columns; x++)
          *--q=(*p++);
        if ((indexes != (IndexPacket *) NULL) &&
            (rotate_indexes != (IndexPacket *) NULL))
          for (x=0; x < (ssize_t) image->columns; x++)
            SetPixelIndex(rotate_indexes+image->columns-x-1,
              GetPixelIndex(indexes+x));
        sync=SyncCacheViewAuthenticPixels(rotate_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_IntegralRotateImage)
#endif
            proceed=SetImageProgress(image,RotateImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      if (page.width != 0)
        page.x=(ssize_t) (page.width-rotate_image->columns-page.x);
      if (page.height != 0)
        page.y=(ssize_t) (page.height-rotate_image->rows-page.y);
      break;
    }
    case 3:
    {
      size_t
        tile_height,
        tile_width;

      ssize_t
        tile_y;

      /*
        Rotate 270 degrees.
      */
      GetPixelCacheTileSize(image,&tile_width,&tile_height);
      tile_width=image->columns;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_threads(image,image,1,1)
#endif
      for (tile_y=0; tile_y < (ssize_t) image->rows; tile_y+=(ssize_t) tile_height)
      {
        register ssize_t
          tile_x;

        if (status == MagickFalse)
          continue;
        for (tile_x=0; tile_x < (ssize_t) image->columns; tile_x+=(ssize_t) tile_width)
        {
          MagickBooleanType
            sync;

          register const IndexPacket
            *restrict indexes;

          register const PixelPacket
            *restrict p;

          register IndexPacket
            *restrict rotate_indexes;

          register PixelPacket
            *restrict q;

          register ssize_t
            y;

          size_t
            height,
            width;

          width=tile_width;
          if ((tile_x+(ssize_t) tile_width) > (ssize_t) image->columns)
            width=(size_t) (tile_width-(tile_x+tile_width-image->columns));
          height=tile_height;
          if ((tile_y+(ssize_t) tile_height) > (ssize_t) image->rows)
            height=(size_t) (tile_height-(tile_y+tile_height-image->rows));
          p=GetCacheViewVirtualPixels(image_view,tile_x,tile_y,width,height,
            exception);
          if (p == (const PixelPacket *) NULL)
            {
              status=MagickFalse;
              break;
            }
          indexes=GetCacheViewVirtualIndexQueue(image_view);
          for (y=0; y < (ssize_t) width; y++)
          {
            register const PixelPacket
              *restrict tile_pixels;

            register ssize_t
              x;

            if (status == MagickFalse)
              continue;
            q=QueueCacheViewAuthenticPixels(rotate_view,tile_y,(ssize_t) (y+
              rotate_image->rows-(tile_x+width)),height,1,exception);
            if (q == (PixelPacket *) NULL)
              {
                status=MagickFalse;
                continue;
              }
            tile_pixels=p+(width-1)-y;
            for (x=0; x < (ssize_t) height; x++)
            {
              *q++=(*tile_pixels);
              tile_pixels+=width;
            }
            rotate_indexes=GetCacheViewAuthenticIndexQueue(rotate_view);
            if ((indexes != (IndexPacket *) NULL) &&
                (rotate_indexes != (IndexPacket *) NULL))
              {
                register const IndexPacket
                  *restrict tile_indexes;

                tile_indexes=indexes+(width-1)-y;
                for (x=0; x < (ssize_t) height; x++)
                {
                  *rotate_indexes++=(*tile_indexes);
                  tile_indexes+=width;
                }
              }
            sync=SyncCacheViewAuthenticPixels(rotate_view,exception);
            if (sync == MagickFalse)
              status=MagickFalse;
          }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_IntegralRotateImage)
#endif
            proceed=SetImageProgress(image,RotateImageTag,progress+=tile_height,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      (void) SetImageProgress(image,RotateImageTag,(MagickOffsetType)
        image->rows-1,image->rows);
      Swap(page.width,page.height);
      Swap(page.x,page.y);
      if (page.height != 0)
        page.y=(ssize_t) (page.height-rotate_image->rows-page.y);
      break;
    }
  }
  rotate_view=DestroyCacheView(rotate_view);
  image_view=DestroyCacheView(image_view);
  rotate_image->type=image->type;
  rotate_image->page=page;
  if (status == MagickFalse)
    rotate_image=DestroyImage(rotate_image);
  return(rotate_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X S h e a r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XShearImage() shears the image in the X direction with a shear angle of
%  'degrees'.  Positive angles shear counter-clockwise (right-hand rule), and
%  negative angles shear clockwise.  Angles are measured relative to a vertical
%  Y-axis.  X shears will widen an image creating 'empty' triangles on the left
%  and right sides of the source image.
%
%  The format of the XShearImage method is:
%
%      MagickBooleanType XShearImage(Image *image,const MagickRealType degrees,
%        const size_t width,const size_t height,
%        const ssize_t x_offset,const ssize_t y_offset,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: A MagickRealType representing the shearing angle along the X
%      axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XShearImage(Image *image,const MagickRealType degrees,
  const size_t width,const size_t height,const ssize_t x_offset,
  const ssize_t y_offset,ExceptionInfo *exception)
{
#define XShearImageTag  "XShear/Image"

  typedef enum
  {
    LEFT,
    RIGHT
  } ShearDirection;

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    background;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  GetMagickPixelPacket(image,&background);
  SetMagickPixelPacket(image,&image->background_color,(IndexPacket *) NULL,
    &background);
  if (image->colorspace == CMYKColorspace)
    ConvertRGBToCMYK(&background);
  /*
    X shear image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,height,1)
#endif
  for (y=0; y < (ssize_t) height; y++)
  {
    MagickPixelPacket
      pixel,
      source,
      destination;

    MagickRealType
      area,
      displacement;

    register IndexPacket
      *restrict indexes,
      *restrict shear_indexes;

    register PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      i;

    ShearDirection
      direction;

    ssize_t
      step;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewAuthenticPixels(image_view,0,y_offset+y,image->columns,1,
      exception);
    if (p == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    p+=x_offset;
    indexes+=x_offset;
    displacement=degrees*(MagickRealType) (y-height/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=RIGHT;
    else
      {
        displacement*=(-1.0);
        direction=LEFT;
      }
    step=(ssize_t) floor((double) displacement);
    area=(MagickRealType) (displacement-step);
    step++;
    pixel=background;
    GetMagickPixelPacket(image,&source);
    GetMagickPixelPacket(image,&destination);
    switch (direction)
    {
      case LEFT:
      {
        /*
          Transfer pixels left-to-right.
        */
        if (step > x_offset)
          break;
        q=p-step;
        shear_indexes=indexes-step;
        for (i=0; i < (ssize_t) width; i++)
        {
          if ((x_offset+i) < step)
            {
              SetMagickPixelPacket(image,++p,++indexes,&pixel);
              q++;
              shear_indexes++;
              continue;
            }
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
            &source,(MagickRealType) GetPixelOpacity(p),area,&destination);
          SetPixelPacket(image,&destination,q++,shear_indexes++);
          SetMagickPixelPacket(image,p++,indexes++,&pixel);
        }
        MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
          &background,(MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,q++,shear_indexes++);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,q++,shear_indexes++);
        break;
      }
      case RIGHT:
      {
        /*
          Transfer pixels right-to-left.
        */
        p+=width;
        indexes+=width;
        q=p+step;
        shear_indexes=indexes+step;
        for (i=0; i < (ssize_t) width; i++)
        {
          p--;
          indexes--;
          q--;
          shear_indexes--;
          if ((size_t) (x_offset+width+step-i) >= image->columns)
            continue;
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
            &source,(MagickRealType) GetPixelOpacity(p),area,&destination);
          SetPixelPacket(image,&destination,q,shear_indexes);
          SetMagickPixelPacket(image,p,indexes,&pixel);
        }
        MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
          &background,(MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,--q,--shear_indexes);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,--q,--shear_indexes);
        break;
      }
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_XShearImage)
#endif
        proceed=SetImageProgress(image,XShearImageTag,progress++,height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   Y S h e a r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  YShearImage shears the image in the Y direction with a shear angle of
%  'degrees'.  Positive angles shear counter-clockwise (right-hand rule), and
%  negative angles shear clockwise.  Angles are measured relative to a
%  horizontal X-axis.  Y shears will increase the height of an image creating
%  'empty' triangles on the top and bottom of the source image.
%
%  The format of the YShearImage method is:
%
%      MagickBooleanType YShearImage(Image *image,const MagickRealType degrees,
%        const size_t width,const size_t height,
%        const ssize_t x_offset,const ssize_t y_offset,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: A MagickRealType representing the shearing angle along the Y
%      axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType YShearImage(Image *image,const MagickRealType degrees,
  const size_t width,const size_t height,const ssize_t x_offset,
  const ssize_t y_offset,ExceptionInfo *exception)
{
#define YShearImageTag  "YShear/Image"

  typedef enum
  {
    UP,
    DOWN
  } ShearDirection;

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    background;

  ssize_t
    x;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  GetMagickPixelPacket(image,&background);
  SetMagickPixelPacket(image,&image->background_color,(IndexPacket *) NULL,
    &background);
  if (image->colorspace == CMYKColorspace)
    ConvertRGBToCMYK(&background);
  /*
    Y Shear image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,width,1)
#endif
  for (x=0; x < (ssize_t) width; x++)
  {
    ssize_t
      step;

    MagickPixelPacket
      pixel,
      source,
      destination;

    MagickRealType
      area,
      displacement;

    register IndexPacket
      *restrict indexes,
      *restrict shear_indexes;

    register ssize_t
      i;

    register PixelPacket
      *restrict p,
      *restrict q;

    ShearDirection
      direction;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewAuthenticPixels(image_view,x_offset+x,0,1,image->rows,
      exception);
    if (p == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    p+=y_offset;
    indexes+=y_offset;
    displacement=degrees*(MagickRealType) (x-width/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=DOWN;
    else
      {
        displacement*=(-1.0);
        direction=UP;
      }
    step=(ssize_t) floor((double) displacement);
    area=(MagickRealType) (displacement-step);
    step++;
    pixel=background;
    GetMagickPixelPacket(image,&source);
    GetMagickPixelPacket(image,&destination);
    switch (direction)
    {
      case UP:
      {
        /*
          Transfer pixels top-to-bottom.
        */
        if (step > y_offset)
          break;
        q=p-step;
        shear_indexes=indexes-step;
        for (i=0; i < (ssize_t) height; i++)
        {
          if ((y_offset+i) < step)
            {
              SetMagickPixelPacket(image,++p,++indexes,&pixel);
              q++;
              shear_indexes++;
              continue;
            }
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
            &source,(MagickRealType) GetPixelOpacity(p),area,&destination);
          SetPixelPacket(image,&destination,q++,shear_indexes++);
          SetMagickPixelPacket(image,p++,indexes++,&pixel);
        }
        MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
          &background,(MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,q++,shear_indexes++);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,q++,shear_indexes++);
        break;
      }
      case DOWN:
      {
        /*
          Transfer pixels bottom-to-top.
        */
        p+=height;
        indexes+=height;
        q=p+step;
        shear_indexes=indexes+step;
        for (i=0; i < (ssize_t) height; i++)
        {
          p--;
          indexes--;
          q--;
          shear_indexes--;
          if ((size_t) (y_offset+height+step-i) >= image->rows)
            continue;
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
            &source,(MagickRealType) GetPixelOpacity(p),area,&destination);
          SetPixelPacket(image,&destination,q,shear_indexes);
          SetMagickPixelPacket(image,p,indexes,&pixel);
        }
        MagickPixelCompositeAreaBlend(&pixel,(MagickRealType) pixel.opacity,
          &background,(MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,--q,--shear_indexes);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,--q,--shear_indexes);
        break;
      }
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_YShearImage)
#endif
        proceed=SetImageProgress(image,YShearImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S h e a r I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShearImage() creates a new image that is a shear_image copy of an existing
%  one.  Shearing slides one edge of an image along the X or Y axis, creating
%  a parallelogram.  An X direction shear slides an edge along the X axis,
%  while a Y direction shear slides an edge along the Y axis.  The amount of
%  the shear is controlled by a shear angle.  For X direction shears, x_shear
%  is measured relative to the Y axis, and similarly, for Y direction shears
%  y_shear is measured relative to the X axis.  Empty triangles left over from
%  shearing the image are filled with the background color defined by member
%  'background_color' of the image..  ShearImage() allocates the memory
%  necessary for the new Image structure and returns a pointer to the new image.
%
%  ShearImage() is based on the paper "A Fast Algorithm for General Raster
%  Rotatation" by Alan W. Paeth.
%
%  The format of the ShearImage method is:
%
%      Image *ShearImage(const Image *image,const double x_shear,
%        const double y_shear,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o x_shear, y_shear: Specifies the number of degrees to shear the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ShearImage(const Image *image,const double x_shear,
  const double y_shear,ExceptionInfo *exception)
{
  Image
    *integral_image,
    *shear_image;

  ssize_t
    x_offset,
    y_offset;

  MagickBooleanType
    status;

  PointInfo
    shear;

  RectangleInfo
    border_info;

  size_t
    y_width;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((x_shear != 0.0) && (fmod(x_shear,90.0) == 0.0))
    ThrowImageException(ImageError,"AngleIsDiscontinuous");
  if ((y_shear != 0.0) && (fmod(y_shear,90.0) == 0.0))
    ThrowImageException(ImageError,"AngleIsDiscontinuous");
  /*
    Initialize shear angle.
  */
  integral_image=CloneImage(image,0,0,MagickTrue,exception);
  if (integral_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  shear.x=(-tan(DegreesToRadians(fmod(x_shear,360.0))));
  shear.y=tan(DegreesToRadians(fmod(y_shear,360.0)));
  if ((shear.x == 0.0) && (shear.y == 0.0))
    return(integral_image);
  if (SetImageStorageClass(integral_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&integral_image->exception);
      integral_image=DestroyImage(integral_image);
      return(integral_image);
    }
  if (integral_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(integral_image,OpaqueAlphaChannel);
  /*
    Compute image size.
  */
  y_width=image->columns+(ssize_t) floor(fabs(shear.x)*image->rows+0.5);
  x_offset=(ssize_t) ceil((double) image->columns+((fabs(shear.x)*image->rows)-
    image->columns)/2.0-0.5);
  y_offset=(ssize_t) ceil((double) image->rows+((fabs(shear.y)*y_width)-
    image->rows)/2.0-0.5);
  /*
    Surround image with border.
  */
  integral_image->border_color=integral_image->background_color;
  integral_image->compose=CopyCompositeOp;
  border_info.width=(size_t) x_offset;
  border_info.height=(size_t) y_offset;
  shear_image=BorderImage(integral_image,&border_info,exception);
  integral_image=DestroyImage(integral_image);
  if (shear_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Shear the image.
  */
  if (shear_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(shear_image,OpaqueAlphaChannel);
  status=XShearImage(shear_image,shear.x,image->columns,image->rows,x_offset,
    (ssize_t) (shear_image->rows-image->rows)/2,exception);
  if (status == MagickFalse)
    {
      shear_image=DestroyImage(shear_image);
      return((Image *) NULL);
    }
  status=YShearImage(shear_image,shear.y,y_width,image->rows,(ssize_t)
    (shear_image->columns-y_width)/2,y_offset,exception);
  if (status == MagickFalse)
    {
      shear_image=DestroyImage(shear_image);
      return((Image *) NULL);
    }
  status=CropToFitImage(&shear_image,shear.x,shear.y,(MagickRealType)
    image->columns,(MagickRealType) image->rows,MagickFalse,exception);
  shear_image->compose=image->compose;
  shear_image->page.width=0;
  shear_image->page.height=0;
  if (status == MagickFalse)
    shear_image=DestroyImage(shear_image);
  return(shear_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S h e a r R o t a t e I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShearRotateImage() creates a new image that is a rotated copy of an existing
%  one.  Positive angles rotate counter-clockwise (right-hand rule), while
%  negative angles rotate clockwise.  Rotated images are usually larger than
%  the originals and have 'empty' triangular corners.  X axis.  Empty
%  triangles left over from shearing the image are filled with the background
%  color defined by member 'background_color' of the image.  ShearRotateImage
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  ShearRotateImage() is based on the paper "A Fast Algorithm for General
%  Raster Rotatation" by Alan W. Paeth.  ShearRotateImage is adapted from a
%  similar method based on the Paeth paper written by Michael Halle of the
%  Spatial Imaging Group, MIT Media Lab.
%
%  The format of the ShearRotateImage method is:
%
%      Image *ShearRotateImage(const Image *image,const double degrees,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: Specifies the number of degrees to rotate the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ShearRotateImage(const Image *image,const double degrees,
  ExceptionInfo *exception)
{
  Image
    *integral_image,
    *rotate_image;

  ssize_t
    x_offset,
    y_offset;

  MagickBooleanType
    status;

  MagickRealType
    angle;

  PointInfo
    shear;

  RectangleInfo
    border_info;

  size_t
    height,
    rotations,
    width,
    y_width;

  /*
    Adjust rotation angle.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  angle=degrees;
  while (angle < -45.0)
    angle+=360.0;
  for (rotations=0; angle > 45.0; rotations++)
    angle-=90.0;
  rotations%=4;
  /*
    Calculate shear equations.
  */
  integral_image=IntegralRotateImage(image,rotations,exception);
  if (integral_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  shear.x=(-tan((double) DegreesToRadians(angle)/2.0));
  shear.y=sin((double) DegreesToRadians(angle));
  if ((shear.x == 0.0) && (shear.y == 0.0))
    return(integral_image);
  if (SetImageStorageClass(integral_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&integral_image->exception);
      integral_image=DestroyImage(integral_image);
      return(integral_image);
    }
  if (integral_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(integral_image,OpaqueAlphaChannel);
  /*
    Compute image size.
  */
  width=image->columns;
  height=image->rows;
  if ((rotations == 1) || (rotations == 3))
    {
      width=image->rows;
      height=image->columns;
    }
  y_width=width+(ssize_t) floor(fabs(shear.x)*height+0.5);
  x_offset=(ssize_t) ceil((double) width+((fabs(shear.y)*height)-width)/2.0-
    0.5);
  y_offset=(ssize_t) ceil((double) height+((fabs(shear.y)*y_width)-height)/2.0-
    0.5);
  /*
    Surround image with a border.
  */
  integral_image->border_color=integral_image->background_color;
  integral_image->compose=CopyCompositeOp;
  border_info.width=(size_t) x_offset;
  border_info.height=(size_t) y_offset;
  rotate_image=BorderImage(integral_image,&border_info,exception);
  integral_image=DestroyImage(integral_image);
  if (rotate_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Rotate the image.
  */
  status=XShearImage(rotate_image,shear.x,width,height,x_offset,(ssize_t)
    (rotate_image->rows-height)/2,exception);
  if (status == MagickFalse)
    {
      rotate_image=DestroyImage(rotate_image);
      return((Image *) NULL);
    }
  status=YShearImage(rotate_image,shear.y,y_width,height,(ssize_t)
    (rotate_image->columns-y_width)/2,y_offset,exception);
  if (status == MagickFalse)
    {
      rotate_image=DestroyImage(rotate_image);
      return((Image *) NULL);
    }
  status=XShearImage(rotate_image,shear.x,y_width,rotate_image->rows,(ssize_t)
    (rotate_image->columns-y_width)/2,0,exception);
  if (status == MagickFalse)
    {
      rotate_image=DestroyImage(rotate_image);
      return((Image *) NULL);
    }
  status=CropToFitImage(&rotate_image,shear.x,shear.y,(MagickRealType) width,
    (MagickRealType) height,MagickTrue,exception);
  rotate_image->compose=image->compose;
  rotate_image->page.width=0;
  rotate_image->page.height=0;
  if (status == MagickFalse)
    rotate_image=DestroyImage(rotate_image);
  return(rotate_image);
}

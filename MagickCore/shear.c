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
%  The XShearImage() and YShearImage() methods are based on the paper "A Fast
%  Algorithm for General Raster Rotation" by Alan W. Paeth, Graphics
%  Interface '86 (Vancouver).  ShearRotateImage() is adapted from a similar
%  method based on the Paeth paper written by Michael Halle of the Spatial
%  Imaging Group, MIT Media Lab.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/channel.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/decorate.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/matrix.h"
#include "MagickCore/memory_.h"
#include "MagickCore/list.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resource_.h"
#include "MagickCore/shear.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/transform.h"

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
%        const double x_shear,const double x_shear,
%        const double width,const double height,
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
  const double x_shear,const double y_shear,
  const double width,const double height,
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

  ssize_t
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
  for (i=3; i >= 0; i--)
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
  geometry.x=CastDoubleToSsizeT(ceil(min.x-0.5));
  geometry.y=CastDoubleToSsizeT(ceil(min.y-0.5));
  geometry.width=(size_t) CastDoubleToSsizeT(floor(max.x-min.x+0.5));
  geometry.height=(size_t) CastDoubleToSsizeT(floor(max.y-min.y+0.5));
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
%  The result will be auto-cropped if the artifact "deskew:auto-crop" is
%  defined, while the amount the image is to be deskewed, in degrees is also
%  saved as the artifact "deskew:angle".
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

static void RadonProjection(MatrixInfo *source_matrices,
  MatrixInfo *destination_matrices,const ssize_t sign,size_t *projection)
{
  MatrixInfo
    *p,
    *q,
    *swap;

  size_t
    step;

  ssize_t
    x;

  p=source_matrices;
  q=destination_matrices;
  for (step=1; step < GetMatrixColumns(p); step*=2)
  {
    for (x=0; x < (ssize_t) GetMatrixColumns(p); x+=2*(ssize_t) step)
    {
      ssize_t
        i,
        y;

      unsigned short
        element,
        neighbor;

      for (i=0; i < (ssize_t) step; i++)
      {
        for (y=0; y < ((ssize_t) GetMatrixRows(p)-i-1); y++)
        {
          if (GetMatrixElement(p,x+i,y,&element) == MagickFalse)
            continue;
          if (GetMatrixElement(p,x+i+(ssize_t) step,y+i,&neighbor) == MagickFalse)
            continue;
          neighbor+=element;
          if (SetMatrixElement(q,x+2*i,y,&neighbor) == MagickFalse)
            continue;
          if (GetMatrixElement(p,x+i+(ssize_t) step,y+i+1,&neighbor) == MagickFalse)
            continue;
          neighbor+=element;
          if (SetMatrixElement(q,x+2*i+1,y,&neighbor) == MagickFalse)
            continue;
        }
        for ( ; y < ((ssize_t) GetMatrixRows(p)-i); y++)
        {
          if (GetMatrixElement(p,x+i,y,&element) == MagickFalse)
            continue;
          if (GetMatrixElement(p,x+i+(ssize_t) step,y+i,&neighbor) == MagickFalse)
            continue;
          neighbor+=element;
          if (SetMatrixElement(q,x+2*i,y,&neighbor) == MagickFalse)
            continue;
          if (SetMatrixElement(q,x+2*i+1,y,&element) == MagickFalse)
            continue;
        }
        for ( ; y < (ssize_t) GetMatrixRows(p); y++)
        {
          if (GetMatrixElement(p,x+i,y,&element) == MagickFalse)
            continue;
          if (SetMatrixElement(q,x+2*i,y,&element) == MagickFalse)
            continue;
          if (SetMatrixElement(q,x+2*i+1,y,&element) == MagickFalse)
            continue;
        }
      }
    }
    swap=p;
    p=q;
    q=swap;
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) \
    num_threads((int) GetMagickResourceLimit(ThreadResource))
#endif
  for (x=0; x < (ssize_t) GetMatrixColumns(p); x++)
  {
    size_t
      sum;

    ssize_t
      y;

    sum=0;
    for (y=0; y < (ssize_t) (GetMatrixRows(p)-1); y++)
    {
      ssize_t
        delta;

      unsigned short
        element,
        neighbor;

      if (GetMatrixElement(p,x,y,&element) == MagickFalse)
        continue;
      if (GetMatrixElement(p,x,y+1,&neighbor) == MagickFalse)
        continue;
      delta=(ssize_t) element-(ssize_t) neighbor;
      sum+=(size_t) (delta*delta);
    }
    projection[(ssize_t) GetMatrixColumns(p)+sign*x-1]=sum;
  }
}

static MagickBooleanType RadonTransform(const Image *image,
  const double threshold,size_t *projection,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MatrixInfo
    *destination_matrices,
    *source_matrices;

  MagickBooleanType
    status;

  size_t
    count,
    width;

  ssize_t
    j,
    y;

  unsigned char
    c;

  unsigned short
    bits[256];

  for (width=1; width < ((image->columns+7)/8); width<<=1) ;
  source_matrices=AcquireMatrixInfo(width,image->rows,sizeof(unsigned short),
    exception);
  destination_matrices=AcquireMatrixInfo(width,image->rows,
    sizeof(unsigned short),exception);
  if ((source_matrices == (MatrixInfo *) NULL) ||
      (destination_matrices == (MatrixInfo *) NULL))
    {
      if (destination_matrices != (MatrixInfo *) NULL)
        destination_matrices=DestroyMatrixInfo(destination_matrices);
      if (source_matrices != (MatrixInfo *) NULL)
        source_matrices=DestroyMatrixInfo(source_matrices);
      return(MagickFalse);
    }
  if (NullMatrix(source_matrices) == MagickFalse)
    {
      destination_matrices=DestroyMatrixInfo(destination_matrices);
      source_matrices=DestroyMatrixInfo(source_matrices);
      return(MagickFalse);
    }
  for (j=0; j < 256; j++)
  {
    c=(unsigned char) j;
    for (count=0; c != 0; c>>=1)
      count+=c & 0x01;
    bits[j]=(unsigned short) count;
  }
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,2)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    size_t
      bit,
      byte;

    ssize_t
      i,
      x;

    unsigned short
      value;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
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
      if (((MagickRealType) GetPixelRed(image,p) < threshold) ||
          ((MagickRealType) GetPixelGreen(image,p) < threshold) ||
          ((MagickRealType) GetPixelBlue(image,p) < threshold))
        byte|=0x01;
      bit++;
      if (bit == 8)
        {
          value=bits[byte];
          (void) SetMatrixElement(source_matrices,--i,y,&value);
          bit=0;
          byte=0;
        }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (bit != 0)
      {
        byte<<=(8-bit);
        value=bits[byte];
        (void) SetMatrixElement(source_matrices,--i,y,&value);
      }
  }
  RadonProjection(source_matrices,destination_matrices,-1,projection);
  (void) NullMatrix(source_matrices);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,2)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    size_t
      bit,
      byte;

    ssize_t
      i,
      x;

    unsigned short
     value;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
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
      if (((MagickRealType) GetPixelRed(image,p) < threshold) ||
          ((MagickRealType) GetPixelGreen(image,p) < threshold) ||
          ((MagickRealType) GetPixelBlue(image,p) < threshold))
        byte|=0x01;
      bit++;
      if (bit == 8)
        {
          value=bits[byte];
          (void) SetMatrixElement(source_matrices,i++,y,&value);
          bit=0;
          byte=0;
        }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (bit != 0)
      {
        byte<<=(8-bit);
        value=bits[byte];
        (void) SetMatrixElement(source_matrices,i++,y,&value);
      }
  }
  RadonProjection(source_matrices,destination_matrices,1,projection);
  image_view=DestroyCacheView(image_view);
  destination_matrices=DestroyMatrixInfo(destination_matrices);
  source_matrices=DestroyMatrixInfo(source_matrices);
  return(MagickTrue);
}

static void GetImageBackgroundColor(Image *image,const ssize_t offset,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  double
    count;

  PixelInfo
    background;

  ssize_t
    y;

  /*
    Compute average background color.
  */
  if (offset <= 0)
    return;
  GetPixelInfo(image,&background);
  count=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    if ((y >= offset) && (y < ((ssize_t) image->rows-offset)))
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      continue;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x >= offset) && (x < ((ssize_t) image->columns-offset)))
        continue;
      background.red+=QuantumScale*(double) GetPixelRed(image,p);
      background.green+=QuantumScale*(double) GetPixelGreen(image,p);
      background.blue+=QuantumScale*(double) GetPixelBlue(image,p);
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        background.alpha+=QuantumScale*(double) GetPixelAlpha(image,p);
      count++;
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  image->background_color.red=(double) ClampToQuantum((double) QuantumRange*
    (double) background.red/count);
  image->background_color.green=(double) ClampToQuantum((double) QuantumRange*
    (double) background.green/count);
  image->background_color.blue=(double) ClampToQuantum((double) QuantumRange*
    (double) background.blue/count);
  if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
    image->background_color.alpha=(double) ClampToQuantum((double) QuantumRange*
      (double) background.alpha/count);
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

  size_t
    max_projection,
    *projection,
    width;

  ssize_t
    i,
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
      angle[MagickPathExtent];

    (void) FormatLocaleString(angle,MagickPathExtent,"%.20g",degrees);
    (void) SetImageArtifact(clone_image,"deskew:angle",angle);
  }
  (void) SetImageVirtualPixelMethod(clone_image,BackgroundVirtualPixelMethod,
    exception);
  affine_matrix.sx=cos(DegreesToRadians(fmod((double) degrees,360.0)));
  affine_matrix.rx=sin(DegreesToRadians(fmod((double) degrees,360.0)));
  affine_matrix.ry=(-sin(DegreesToRadians(fmod((double) degrees,360.0))));
  affine_matrix.sy=cos(DegreesToRadians(fmod((double) degrees,360.0)));
  affine_matrix.tx=0.0;
  affine_matrix.ty=0.0;
  artifact=GetImageArtifact(image,"deskew:auto-crop");
  if (IsStringTrue(artifact) == MagickFalse)
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

  /*
    Initialize rotated image attributes.
  */
  assert(image != (Image *) NULL);
  page=image->page;
  rotations%=4;
  switch (rotations)
  {
    case 0:
    default:
    {
      rotate_image=CloneImage(image,0,0,MagickTrue,exception);
      break;
    }
    case 2:
    {
      rotate_image=CloneImage(image,image->columns,image->rows,MagickTrue,
        exception);
      break;
    }
    case 1:
    case 3:
    {
      rotate_image=CloneImage(image,image->rows,image->columns,MagickTrue,
        exception);
      break;
    }
  }
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  if (rotations == 0)
    return(rotate_image);
  /*
    Integral rotate the image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  rotate_view=AcquireAuthenticCacheView(rotate_image,exception);
  switch (rotations)
  {
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
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,rotate_image,image->rows/tile_height,2)
#endif
      for (tile_y=0; tile_y < (ssize_t) image->rows; tile_y+=(ssize_t) tile_height)
      {
        ssize_t
          tile_x;

        if (status == MagickFalse)
          continue;
        tile_x=0;
        for ( ; tile_x < (ssize_t) image->columns; tile_x+=(ssize_t) tile_width)
        {
          const Quantum
            *magick_restrict p;

          MagickBooleanType
            sync;

          Quantum
            *magick_restrict q;

          size_t
            height,
            width;

          ssize_t
            y;

          width=tile_width;
          if ((tile_width+(size_t) tile_x) > image->columns)
            width=(size_t) ((ssize_t) tile_width-(tile_x+(ssize_t) tile_width-
              (ssize_t) image->columns));
          height=tile_height;
          if ((tile_height+(size_t) tile_y) > image->rows)
            height=(size_t) ((ssize_t) tile_height-(tile_y+(ssize_t)
              tile_height-(ssize_t) image->rows));
          p=GetCacheViewVirtualPixels(image_view,tile_x,tile_y,width,height,
            exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (y=0; y < (ssize_t) width; y++)
          {
            const Quantum
              *magick_restrict tile_pixels;

            ssize_t
              x;

            if (status == MagickFalse)
              continue;
            q=QueueCacheViewAuthenticPixels(rotate_view,(ssize_t)
              rotate_image->columns-(tile_y+(ssize_t) height),y+tile_x,height,
              1,exception);
            if (q == (Quantum *) NULL)
              {
                status=MagickFalse;
                continue;
              }
            tile_pixels=p+(((ssize_t) height-1)*(ssize_t) width+y)*(ssize_t)
              GetPixelChannels(image);
            for (x=0; x < (ssize_t) height; x++)
            {
              ssize_t
                i;

              for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
              {
                PixelChannel channel = GetPixelChannelChannel(image,i);
                PixelTrait traits = GetPixelChannelTraits(image,channel);
                PixelTrait rotate_traits = GetPixelChannelTraits(rotate_image,
                  channel);
                if ((traits == UndefinedPixelTrait) ||
                    (rotate_traits == UndefinedPixelTrait))
                  continue;
                SetPixelChannel(rotate_image,channel,tile_pixels[i],q);
              }
              tile_pixels-=width*GetPixelChannels(image);
              q+=(ptrdiff_t) GetPixelChannels(rotate_image);
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

            proceed=SetImageProgress(image,RotateImageTag,
              progress+=(MagickOffsetType) tile_height,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      (void) SetImageProgress(image,RotateImageTag,(MagickOffsetType)
        image->rows-1,image->rows);
      Swap(page.width,page.height);
      Swap(page.x,page.y);
      if (page.width != 0)
        page.x=(ssize_t) page.width-(ssize_t) rotate_image->columns-page.x;
      break;
    }
    case 2:
    {
      ssize_t
        y;

      /*
        Rotate 180 degrees.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,rotate_image,image->rows,2)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        const Quantum
          *magick_restrict p;

        MagickBooleanType
          sync;

        Quantum
          *magick_restrict q;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
        q=QueueCacheViewAuthenticPixels(rotate_view,0,(ssize_t) image->rows-y-1,
          image->columns,1,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        q+=(ptrdiff_t) GetPixelChannels(rotate_image)*image->columns;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          ssize_t
            i;

          q-=GetPixelChannels(rotate_image);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            PixelTrait rotate_traits = GetPixelChannelTraits(rotate_image,
              channel);
            if ((traits == UndefinedPixelTrait) ||
                (rotate_traits == UndefinedPixelTrait))
              continue;
            SetPixelChannel(rotate_image,channel,p[i],q);
          }
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(rotate_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

            proceed=SetImageProgress(image,RotateImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      (void) SetImageProgress(image,RotateImageTag,(MagickOffsetType)
        image->rows-1,image->rows);
      if (page.width != 0)
        page.x=(ssize_t) page.width-(ssize_t) rotate_image->columns-page.x;
      if (page.height != 0)
        page.y=(ssize_t) page.height-(ssize_t) rotate_image->rows-page.y;
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
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,rotate_image,image->rows/tile_height,2)
#endif
      for (tile_y=0; tile_y < (ssize_t) image->rows; tile_y+=(ssize_t) tile_height)
      {
        ssize_t
          tile_x;

        if (status == MagickFalse)
          continue;
        tile_x=0;
        for ( ; tile_x < (ssize_t) image->columns; tile_x+=(ssize_t) tile_width)
        {
          MagickBooleanType
            sync;

          const Quantum
            *magick_restrict p;

          Quantum
            *magick_restrict q;

          size_t
            height,
            width;

          ssize_t
            y;

          width=tile_width;
          if ((tile_width+(size_t) tile_x) > image->columns)
            width=(size_t) ((ssize_t) tile_width-(tile_x+(ssize_t) tile_width-
              (ssize_t) image->columns));
          height=tile_height;
          if ((tile_height+(size_t) tile_y) > image->rows)
            height=(size_t) ((ssize_t) tile_height-(tile_y+(ssize_t)
              tile_height-(ssize_t) image->rows));
          p=GetCacheViewVirtualPixels(image_view,tile_x,tile_y,width,height,
            exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (y=0; y < (ssize_t) width; y++)
          {
            const Quantum
              *magick_restrict tile_pixels;

            ssize_t
              x;

            if (status == MagickFalse)
              continue;
            q=QueueCacheViewAuthenticPixels(rotate_view,tile_y,y+(ssize_t)
              rotate_image->rows-(tile_x+(ssize_t) width),height,1,exception);
            if (q == (Quantum *) NULL)
              {
                status=MagickFalse;
                continue;
              }
            tile_pixels=p+(((ssize_t) width-1)-y)*(ssize_t)
              GetPixelChannels(image);
            for (x=0; x < (ssize_t) height; x++)
            {
              ssize_t
                i;

              for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
              {
                PixelChannel channel = GetPixelChannelChannel(image,i);
                PixelTrait traits = GetPixelChannelTraits(image,channel);
                PixelTrait rotate_traits = GetPixelChannelTraits(rotate_image,
                  channel);
                if ((traits == UndefinedPixelTrait) ||
                    (rotate_traits == UndefinedPixelTrait))
                  continue;
                SetPixelChannel(rotate_image,channel,tile_pixels[i],q);
              }
              tile_pixels+=width*GetPixelChannels(image);
              q+=(ptrdiff_t) GetPixelChannels(rotate_image);
            }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_IntegralRotateImage)
#endif
            sync=SyncCacheViewAuthenticPixels(rotate_view,exception);
            if (sync == MagickFalse)
              status=MagickFalse;
          }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

            proceed=SetImageProgress(image,RotateImageTag,
              progress+=(MagickOffsetType) tile_height,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      (void) SetImageProgress(image,RotateImageTag,(MagickOffsetType)
        image->rows-1,image->rows);
      Swap(page.width,page.height);
      Swap(page.x,page.y);
      if (page.height != 0)
        page.y=(ssize_t) page.height-(ssize_t) rotate_image->rows-page.y;
      break;
    }
    default:
      break;
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
%      MagickBooleanType XShearImage(Image *image,const double degrees,
%        const size_t width,const size_t height,
%        const ssize_t x_offset,const ssize_t y_offset,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: A double representing the shearing angle along the X
%      axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XShearImage(Image *image,const double degrees,
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

  PixelInfo
    background;

  ssize_t
    y;

  /*
    X shear image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  background=image->background_color;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,height,1)
#endif
  for (y=0; y < (ssize_t) height; y++)
  {
    double
      area,
      displacement;

    PixelInfo
      pixel,
      source,
      destination;

    Quantum
      *magick_restrict p,
      *magick_restrict q;

    ShearDirection
      direction;

    ssize_t
      i,
      step;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewAuthenticPixels(image_view,0,y_offset+y,image->columns,1,
      exception);
    if (p == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    p+=(ptrdiff_t) x_offset*(ssize_t) GetPixelChannels(image);
    displacement=degrees*(double) (y-height/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=RIGHT;
    else
      {
        displacement*=(-1.0);
        direction=LEFT;
      }
    step=CastDoubleToSsizeT(floor((double) displacement));
    area=(double) (displacement-step);
    step++;
    pixel=background;
    GetPixelInfo(image,&source);
    GetPixelInfo(image,&destination);
    switch (direction)
    {
      case LEFT:
      {
        /*
          Transfer pixels left-to-right.
        */
        if (step > x_offset)
          break;
        q=p-step*(ssize_t) GetPixelChannels(image);
        for (i=0; i < (ssize_t) width; i++)
        {
          if ((x_offset+i) < step)
            {
              p+=(ptrdiff_t) GetPixelChannels(image);
              GetPixelInfoPixel(image,p,&pixel);
              q+=(ptrdiff_t) GetPixelChannels(image);
              continue;
            }
          GetPixelInfoPixel(image,p,&source);
          CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
            &source,(double) GetPixelAlpha(image,p),area,&destination);
          SetPixelViaPixelInfo(image,&destination,q);
          GetPixelInfoPixel(image,p,&pixel);
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
          &background,(double) background.alpha,area,&destination);
        SetPixelViaPixelInfo(image,&destination,q);
        q+=(ptrdiff_t) GetPixelChannels(image);
        for (i=0; i < (step-1); i++)
        {
          SetPixelViaPixelInfo(image,&background,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        break;
      }
      case RIGHT:
      {
        /*
          Transfer pixels right-to-left.
        */
        p+=(ptrdiff_t) width*GetPixelChannels(image);
        q=p+step*(ssize_t) GetPixelChannels(image);
        for (i=0; i < (ssize_t) width; i++)
        {
          p-=(ptrdiff_t)GetPixelChannels(image);
          q-=GetPixelChannels(image);
          if ((size_t) (x_offset+(ssize_t) width+step-i) > image->columns)
            continue;
          GetPixelInfoPixel(image,p,&source);
          CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
            &source,(double) GetPixelAlpha(image,p),area,&destination);
          SetPixelViaPixelInfo(image,&destination,q);
          GetPixelInfoPixel(image,p,&pixel);
        }
        CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
          &background,(double) background.alpha,area,&destination);
        q-=GetPixelChannels(image);
        SetPixelViaPixelInfo(image,&destination,q);
        for (i=0; i < (step-1); i++)
        {
          q-=GetPixelChannels(image);
          SetPixelViaPixelInfo(image,&background,q);
        }
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
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,XShearImageTag,progress,height);
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
%      MagickBooleanType YShearImage(Image *image,const double degrees,
%        const size_t width,const size_t height,
%        const ssize_t x_offset,const ssize_t y_offset,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: A double representing the shearing angle along the Y
%      axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType YShearImage(Image *image,const double degrees,
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

  PixelInfo
    background;

  ssize_t
    x;

  /*
    Y Shear image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  progress=0;
  background=image->background_color;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,width,1)
#endif
  for (x=0; x < (ssize_t) width; x++)
  {
    double
      area,
      displacement;

    PixelInfo
      pixel,
      source,
      destination;

    Quantum
      *magick_restrict p,
      *magick_restrict q;

    ShearDirection
      direction;

    ssize_t
      i,
      step;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewAuthenticPixels(image_view,x_offset+x,0,1,image->rows,
      exception);
    if (p == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    p+=(ptrdiff_t) y_offset*(ssize_t) GetPixelChannels(image);
    displacement=degrees*(double) (x-width/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=DOWN;
    else
      {
        displacement*=(-1.0);
        direction=UP;
      }
    step=CastDoubleToSsizeT(floor((double) displacement));
    area=(double) (displacement-step);
    step++;
    pixel=background;
    GetPixelInfo(image,&source);
    GetPixelInfo(image,&destination);
    switch (direction)
    {
      case UP:
      {
        /*
          Transfer pixels top-to-bottom.
        */
        if (step > y_offset)
          break;
        q=p-step*(ssize_t) GetPixelChannels(image);
        for (i=0; i < (ssize_t) height; i++)
        {
          if ((y_offset+i) < step)
            {
              p+=(ptrdiff_t) GetPixelChannels(image);
              GetPixelInfoPixel(image,p,&pixel);
              q+=(ptrdiff_t) GetPixelChannels(image);
              continue;
            }
          GetPixelInfoPixel(image,p,&source);
          CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
            &source,(double) GetPixelAlpha(image,p),area,
            &destination);
          SetPixelViaPixelInfo(image,&destination,q);
          GetPixelInfoPixel(image,p,&pixel);
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
          &background,(double) background.alpha,area,&destination);
        SetPixelViaPixelInfo(image,&destination,q);
        q+=(ptrdiff_t) GetPixelChannels(image);
        for (i=0; i < (step-1); i++)
        {
          SetPixelViaPixelInfo(image,&background,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        break;
      }
      case DOWN:
      {
        /*
          Transfer pixels bottom-to-top.
        */
        p+=(ptrdiff_t) height*GetPixelChannels(image);
        q=p+step*(ssize_t) GetPixelChannels(image);
        for (i=0; i < (ssize_t) height; i++)
        {
          p-=(ptrdiff_t)GetPixelChannels(image);
          q-=GetPixelChannels(image);
          if ((size_t) (y_offset+(ssize_t) height+step-i) > image->rows)
            continue;
          GetPixelInfoPixel(image,p,&source);
          CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
            &source,(double) GetPixelAlpha(image,p),area,
            &destination);
          SetPixelViaPixelInfo(image,&destination,q);
          GetPixelInfoPixel(image,p,&pixel);
        }
        CompositePixelInfoAreaBlend(&pixel,(double) pixel.alpha,
          &background,(double) background.alpha,area,&destination);
        q-=GetPixelChannels(image);
        SetPixelViaPixelInfo(image,&destination,q);
        for (i=0; i < (step-1); i++)
        {
          q-=GetPixelChannels(image);
          SetPixelViaPixelInfo(image,&background,q);
        }
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
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,YShearImageTag,progress,image->rows);
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
%  Rotation" by Alan W. Paeth.
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

  MagickBooleanType
    status;

  PointInfo
    shear;

  RectangleInfo
    border_info,
    bounds;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
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
  if (SetImageStorageClass(integral_image,DirectClass,exception) == MagickFalse)
    {
      integral_image=DestroyImage(integral_image);
      return(integral_image);
    }
  if (integral_image->alpha_trait == UndefinedPixelTrait)
    (void) SetImageAlphaChannel(integral_image,OpaqueAlphaChannel,exception);
  /*
    Compute image size.
  */
  bounds.width=(size_t) ((ssize_t) image->columns+CastDoubleToSsizeT(floor(
    fabs(shear.x)*image->rows+0.5)));
  bounds.x=CastDoubleToSsizeT(ceil((double) image->columns+((fabs(shear.x)*
    image->rows)-image->columns)/2.0-0.5));
  bounds.y=CastDoubleToSsizeT(ceil((double) image->rows+((fabs(shear.y)*
    bounds.width)-image->rows)/2.0-0.5));
  /*
    Surround image with border.
  */
  integral_image->border_color=integral_image->background_color;
  integral_image->compose=CopyCompositeOp;
  border_info.width=(size_t) bounds.x;
  border_info.height=(size_t) bounds.y;
  shear_image=BorderImage(integral_image,&border_info,image->compose,exception);
  integral_image=DestroyImage(integral_image);
  if (shear_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Shear the image.
  */
  if (shear_image->alpha_trait == UndefinedPixelTrait)
    (void) SetImageAlphaChannel(shear_image,OpaqueAlphaChannel,exception);
  status=XShearImage(shear_image,shear.x,image->columns,image->rows,bounds.x,
    (ssize_t) (shear_image->rows-image->rows)/2,exception);
  if (status == MagickFalse)
    {
      shear_image=DestroyImage(shear_image);
      return((Image *) NULL);
    }
  status=YShearImage(shear_image,shear.y,bounds.width,image->rows,(ssize_t)
    (shear_image->columns-bounds.width)/2,bounds.y,exception);
  if (status == MagickFalse)
    {
      shear_image=DestroyImage(shear_image);
      return((Image *) NULL);
    }
  status=CropToFitImage(&shear_image,shear.x,shear.y,(MagickRealType)
    image->columns,(MagickRealType) image->rows,MagickFalse,exception);
  shear_image->alpha_trait=image->alpha_trait;
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
%  Raster Rotation" by Alan W. Paeth.  ShearRotateImage is adapted from a
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

  MagickBooleanType
    status;

  MagickRealType
    angle;

  PointInfo
    shear;

  RectangleInfo
    border_info,
    bounds;

  size_t
    height,
    rotations,
    shear_width,
    width;

  /*
    Adjust rotation angle.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  angle=fmod(degrees,360.0);
  if (angle < -45.0)
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
  if (SetImageStorageClass(integral_image,DirectClass,exception) == MagickFalse)
    {
      integral_image=DestroyImage(integral_image);
      return(integral_image);
    }
  if (integral_image->alpha_trait == UndefinedPixelTrait)
    (void) SetImageAlphaChannel(integral_image,OpaqueAlphaChannel,exception);
  /*
    Compute maximum bounds for 3 shear operations.
  */
  width=integral_image->columns;
  height=integral_image->rows;
  bounds.width=CastDoubleToSizeT(fabs((double) height*shear.x)+width+0.5);
  bounds.height=CastDoubleToSizeT(fabs((double) bounds.width*shear.y)+height+0.5);
  shear_width=CastDoubleToSizeT(fabs((double) bounds.height*shear.x)+
    bounds.width+0.5);
  bounds.x=CastDoubleToSsizeT(floor((double) ((shear_width > bounds.width) ?
    width : bounds.width-shear_width+2)/2.0+0.5));
  bounds.y=CastDoubleToSsizeT(floor(((double) bounds.height-height+2)/2.0+0.5));
  /*
    Surround image with a border.
  */
  integral_image->border_color=integral_image->background_color;
  integral_image->compose=CopyCompositeOp;
  border_info.width=(size_t) bounds.x;
  border_info.height=(size_t) bounds.y;
  rotate_image=BorderImage(integral_image,&border_info,image->compose,
    exception);
  integral_image=DestroyImage(integral_image);
  if (rotate_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Rotate the image.
  */
  status=XShearImage(rotate_image,shear.x,width,height,bounds.x,(ssize_t)
    (rotate_image->rows-height)/2,exception);
  if (status == MagickFalse)
    {
      rotate_image=DestroyImage(rotate_image);
      return((Image *) NULL);
    }
  status=YShearImage(rotate_image,shear.y,bounds.width,height,(ssize_t)
    (rotate_image->columns-bounds.width)/2,bounds.y,exception);
  if (status == MagickFalse)
    {
      rotate_image=DestroyImage(rotate_image);
      return((Image *) NULL);
    }
  status=XShearImage(rotate_image,shear.x,bounds.width,bounds.height,(ssize_t)
    (rotate_image->columns-bounds.width)/2,(ssize_t) (rotate_image->rows-
    bounds.height)/2,exception);
  if (status == MagickFalse)
    {
      rotate_image=DestroyImage(rotate_image);
      return((Image *) NULL);
    }
  status=CropToFitImage(&rotate_image,shear.x,shear.y,(MagickRealType) width,
    (MagickRealType) height,MagickTrue,exception);
  rotate_image->alpha_trait=image->alpha_trait;
  rotate_image->compose=image->compose;
  rotate_image->page.width=0;
  rotate_image->page.height=0;
  if (status == MagickFalse)
    rotate_image=DestroyImage(rotate_image);
  return(rotate_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           RRRR    EEEEE   SSSSS   AAA   M   M  PPPP   L      EEEEE          %
%           R   R   E       SS     A   A  MM MM  P   P  L      E              %
%           RRRR    EEE      SSS   AAAAA  M M M  PPPP   L      EEE            %
%           R R     E          SS  A   A  M   M  P      L      E              %
%           R  R    EEEEE   SSSSS  A   A  M   M  P      LLLLL  EEEEE          %
%                                                                             %
%                                                                             %
%                      MagickCore Pixel Resampling Methods                    %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                              Anthony Thyssen                                %
%                                August 2007                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/artifact.h"
#include "magick/color-private.h"
#include "magick/cache.h"
#include "magick/draw.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/resample.h"
#include "magick/resize.h"
#include "magick/resize-private.h"
#include "magick/transform.h"
#include "magick/signature-private.h"
/*
  Typedef declarations.
*/
#define WLUT_WIDTH 1024
struct _ResampleFilter
{
  CacheView
    *view;

  Image
    *image;

  ExceptionInfo
    *exception;

  MagickBooleanType
    debug;

  /* Information about image being resampled */
  long
    image_area;

  InterpolatePixelMethod
    interpolate;

  VirtualPixelMethod
    virtual_pixel;

  FilterTypes
    filter;

  /* processing settings needed */
  MagickBooleanType
    limit_reached,
    do_interpolate,
    average_defined;

  MagickPixelPacket
    average_pixel;

  /* current ellipitical area being resampled around center point */
  double
    A, B, C,
    sqrtA, sqrtC, sqrtU, slope;

  /* LUT of weights for filtered average in elliptical area */
  double
    filter_lut[WLUT_WIDTH],
    support;

  unsigned long
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e R e s a m p l e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireResampleFilter() initializes the information resample needs do to a
%  scaled lookup of a color from an image, using area sampling.
%
%  The algorithm is based on a Elliptical Weighted Average, where the pixels
%  found in a large elliptical area is averaged together according to a
%  weighting (filter) function.  For more details see "Fundamentals of Texture
%  Mapping and Image Warping" a master's thesis by Paul.S.Heckbert, June 17,
%  1989.  Available for free from, http://www.cs.cmu.edu/~ph/
%
%  As EWA resampling (or any sort of resampling) can require a lot of
%  calculations to produce a distorted scaling of the source image for each
%  output pixel, the ResampleFilter structure generated holds that information
%  between individual image resampling.
%
%  This function will make the appropriate AcquireCacheView() calls
%  to view the image, calling functions do not need to open a cache view.
%
%  Usage Example...
%      resample_filter=AcquireResampleFilter(image,exception);
%      for (y=0; y < (long) image->rows; y++) {
%        for (x=0; x < (long) image->columns; x++) {
%          X= ....;   Y= ....;
%          ScaleResampleFilter(resample_filter, ... scaling vectors ...);
%          (void) ResamplePixelColor(resample_filter,X,Y,&pixel);
%          ... assign resampled pixel value ...
%        }
%      }
%      DestroyResampleFilter(resample_filter);
%
%  The format of the AcquireResampleFilter method is:
%
%     ResampleFilter *AcquireResampleFilter(const Image *image,
%       ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ResampleFilter *AcquireResampleFilter(const Image *image,
  ExceptionInfo *exception)
{
  register ResampleFilter
    *resample_filter;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  resample_filter=(ResampleFilter *) AcquireMagickMemory(
    sizeof(*resample_filter));
  if (resample_filter == (ResampleFilter *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(resample_filter,0,sizeof(*resample_filter));

  resample_filter->image=ReferenceImage((Image *) image);
  resample_filter->view=AcquireCacheView(resample_filter->image);
  resample_filter->exception=exception;

  resample_filter->debug=IsEventLogging();
  resample_filter->signature=MagickSignature;

  resample_filter->image_area = (long) resample_filter->image->columns *
    resample_filter->image->rows;
  resample_filter->average_defined = MagickFalse;

  /* initialise the resampling filter settings */
  SetResampleFilter(resample_filter, resample_filter->image->filter,
    resample_filter->image->blur);
  resample_filter->interpolate = resample_filter->image->interpolate;
  resample_filter->virtual_pixel=GetImageVirtualPixelMethod(image);

  /* init scale to a default of a unit circle */
  ScaleResampleFilter(resample_filter, 1.0, 0.0, 0.0, 1.0);

  return(resample_filter);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y R e s a m p l e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyResampleFilter() finalizes and cleans up the resampling
%  resample_filter as returned by AcquireResampleFilter(), freeing any memory
%  or other information as needed.
%
%  The format of the DestroyResampleFilter method is:
%
%      ResampleFilter *DestroyResampleFilter(ResampleFilter *resample_filter)
%
%  A description of each parameter follows:
%
%    o resample_filter: resampling information structure
%
*/
MagickExport ResampleFilter *DestroyResampleFilter(
  ResampleFilter *resample_filter)
{
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);
  assert(resample_filter->image != (Image *) NULL);
  if (resample_filter->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      resample_filter->image->filename);
  resample_filter->view=DestroyCacheView(resample_filter->view);
  resample_filter->image=DestroyImage(resample_filter->image);
  resample_filter->signature=(~MagickSignature);
  resample_filter=(ResampleFilter *) RelinquishMagickMemory(resample_filter);
  return(resample_filter);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p o l a t e R e s a m p l e F i l t e r                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpolateResampleFilter() applies bi-linear or tri-linear interpolation
%  between a floating point coordinate and the pixels surrounding that
%  coordinate.  No pixel area resampling, or scaling of the result is
%  performed.
%
%  The format of the InterpolateResampleFilter method is:
%
%      MagickBooleanType InterpolateResampleFilter(
%        ResampleInfo *resample_filter,const InterpolatePixelMethod method,
%        const double x,const double y,MagickPixelPacket *pixel)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resample filter.
%
%    o method: the pixel clor interpolation method.
%
%    o x,y: A double representing the current (x,y) position of the pixel.
%
%    o pixel: return the interpolated pixel here.
%
*/

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static void BicubicInterpolate(const MagickPixelPacket *pixels,const double dx,
  MagickPixelPacket *pixel)
{
  MagickRealType
    dx2,
    p,
    q,
    r,
    s;

  dx2=dx*dx;
  p=(pixels[3].red-pixels[2].red)-(pixels[0].red-pixels[1].red);
  q=(pixels[0].red-pixels[1].red)-p;
  r=pixels[2].red-pixels[0].red;
  s=pixels[1].red;
  pixel->red=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].green-pixels[2].green)-(pixels[0].green-pixels[1].green);
  q=(pixels[0].green-pixels[1].green)-p;
  r=pixels[2].green-pixels[0].green;
  s=pixels[1].green;
  pixel->green=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].blue-pixels[2].blue)-(pixels[0].blue-pixels[1].blue);
  q=(pixels[0].blue-pixels[1].blue)-p;
  r=pixels[2].blue-pixels[0].blue;
  s=pixels[1].blue;
  pixel->blue=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].opacity-pixels[2].opacity)-(pixels[0].opacity-pixels[1].opacity);
  q=(pixels[0].opacity-pixels[1].opacity)-p;
  r=pixels[2].opacity-pixels[0].opacity;
  s=pixels[1].opacity;
  pixel->opacity=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  if (pixel->colorspace == CMYKColorspace)
    {
      p=(pixels[3].index-pixels[2].index)-(pixels[0].index-pixels[1].index);
      q=(pixels[0].index-pixels[1].index)-p;
      r=pixels[2].index-pixels[0].index;
      s=pixels[1].index;
      pixel->index=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
    }
}

static inline MagickRealType CubicWeightingFunction(const MagickRealType x)
{
  MagickRealType
    alpha,
    gamma;

  alpha=MagickMax(x+2.0,0.0);
  gamma=1.0*alpha*alpha*alpha;
  alpha=MagickMax(x+1.0,0.0);
  gamma-=4.0*alpha*alpha*alpha;
  alpha=MagickMax(x+0.0,0.0);
  gamma+=6.0*alpha*alpha*alpha;
  alpha=MagickMax(x-1.0,0.0);
  gamma-=4.0*alpha*alpha*alpha;
  return(gamma/6.0);
}

static inline double MeshInterpolate(const PointInfo *delta,const double p,
  const double x,const double y)
{
  return(delta->x*x+delta->y*y+(1.0-delta->x-delta->y)*p);
}

static inline long NearestNeighbor(MagickRealType x)
{
  if (x >= 0.0)
    return((long) (x+0.5));
  return((long) (x-0.5));
}

static MagickBooleanType InterpolateResampleFilter(
  ResampleFilter *resample_filter,const InterpolatePixelMethod method,
  const double x,const double y,MagickPixelPacket *pixel)
{
  MagickBooleanType
    status;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    i;

  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);
  status=MagickTrue;
  switch (method)
  {
    case AverageInterpolatePixel:
    {
      MagickPixelPacket
        pixels[16];

      MagickRealType
        alpha[16],
        gamma;

      p=GetCacheViewVirtualPixels(resample_filter->view,(long) floor(x)-1,(long)
        floor(y)-1,4,4,resample_filter->exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);
      for (i=0; i < 16L; i++)
      {
        GetMagickPixelPacket(resample_filter->image,pixels+i);
        SetMagickPixelPacket(resample_filter->image,p,indexes+i,pixels+i);
        alpha[i]=1.0;
        if (resample_filter->image->matte != MagickFalse)
          {
            alpha[i]=QuantumScale*((MagickRealType) GetAlphaPixelComponent(p));
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (resample_filter->image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        gamma=alpha[i];
        gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
        pixel->red+=gamma*0.0625*pixels[i].red;
        pixel->green+=gamma*0.0625*pixels[i].green;
        pixel->blue+=gamma*0.0625*pixels[i].blue;
        pixel->opacity+=0.0625*pixels[i].opacity;
        if (resample_filter->image->colorspace == CMYKColorspace)
          pixel->index+=gamma*0.0625*pixels[i].index;
        p++;
      }
      break;
    }
    case BicubicInterpolatePixel:
    {
      MagickPixelPacket
        pixels[16],
        u[4];

      MagickRealType
        alpha[16];

      PointInfo
        delta;

      p=GetCacheViewVirtualPixels(resample_filter->view,(long) floor(x)-1,(long)
        floor(y)-1,4,4,resample_filter->exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);
      for (i=0; i < 16L; i++)
      {
        GetMagickPixelPacket(resample_filter->image,pixels+i);
        SetMagickPixelPacket(resample_filter->image,p,indexes+i,pixels+i);
        alpha[i]=1.0;
        if (resample_filter->image->matte != MagickFalse)
          {
            alpha[i]=QuantumScale*((MagickRealType) GetAlphaPixelComponent(p));
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (resample_filter->image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        p++;
      }
      delta.x=x-floor(x);
      for (i=0; i < 4L; i++)
        BicubicInterpolate(pixels+4*i,delta.x,u+i);
      delta.y=y-floor(y);
      BicubicInterpolate(u,delta.y,pixel);
      break;
    }
    case BilinearInterpolatePixel:
    default:
    {
      MagickPixelPacket
        pixels[4];

      MagickRealType
        alpha[4],
        gamma;

      PointInfo
        delta,
        epsilon;

      p=GetCacheViewVirtualPixels(resample_filter->view,(long) floor(x),(long)
        floor(y),2,2,resample_filter->exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);
      for (i=0; i < 4L; i++)
      {
        pixels[i].red=(MagickRealType) p[i].red;
        pixels[i].green=(MagickRealType) p[i].green;
        pixels[i].blue=(MagickRealType) p[i].blue;
        pixels[i].opacity=(MagickRealType) p[i].opacity;
        alpha[i]=1.0;
      }
      if (resample_filter->image->matte != MagickFalse)
        for (i=0; i < 4L; i++)
        {
          alpha[i]=QuantumScale*((MagickRealType) QuantumRange-p[i].opacity);
          pixels[i].red*=alpha[i];
          pixels[i].green*=alpha[i];
          pixels[i].blue*=alpha[i];
        }
      if (indexes != (IndexPacket *) NULL)
        for (i=0; i < 4L; i++)
        {
          pixels[i].index=(MagickRealType) indexes[i];
          if (resample_filter->image->colorspace == CMYKColorspace)
            pixels[i].index*=alpha[i];
        }
      delta.x=x-floor(x);
      delta.y=y-floor(y);
      epsilon.x=1.0-delta.x;
      epsilon.y=1.0-delta.y;
      gamma=((epsilon.y*(epsilon.x*alpha[0]+delta.x*alpha[1])+delta.y*
        (epsilon.x*alpha[2]+delta.x*alpha[3])));
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
      pixel->red=gamma*(epsilon.y*(epsilon.x*pixels[0].red+delta.x*
        pixels[1].red)+delta.y*(epsilon.x*pixels[2].red+delta.x*pixels[3].red));
      pixel->green=gamma*(epsilon.y*(epsilon.x*pixels[0].green+delta.x*
        pixels[1].green)+delta.y*(epsilon.x*pixels[2].green+delta.x*
        pixels[3].green));
      pixel->blue=gamma*(epsilon.y*(epsilon.x*pixels[0].blue+delta.x*
        pixels[1].blue)+delta.y*(epsilon.x*pixels[2].blue+delta.x*
        pixels[3].blue));
      pixel->opacity=(epsilon.y*(epsilon.x*pixels[0].opacity+delta.x*
        pixels[1].opacity)+delta.y*(epsilon.x*pixels[2].opacity+delta.x*
        pixels[3].opacity));
      if (resample_filter->image->colorspace == CMYKColorspace)
        pixel->index=gamma*(epsilon.y*(epsilon.x*pixels[0].index+delta.x*
          pixels[1].index)+delta.y*(epsilon.x*pixels[2].index+delta.x*
          pixels[3].index));
      break;
    }
    case FilterInterpolatePixel:
    {
      Image
        *excerpt_image,
        *filter_image;

      MagickPixelPacket
        pixels[1];

      RectangleInfo
        geometry;

      CacheView
        *filter_view;

      geometry.width=4L;
      geometry.height=4L;
      geometry.x=(long) floor(x)-1L;
      geometry.y=(long) floor(y)-1L;
      excerpt_image=ExcerptImage(resample_filter->image,&geometry,
        resample_filter->exception);
      if (excerpt_image == (Image *) NULL)
        {
          status=MagickFalse;
          break;
        }
      filter_image=ResizeImage(excerpt_image,1,1,resample_filter->image->filter,
        resample_filter->image->blur,resample_filter->exception);
      excerpt_image=DestroyImage(excerpt_image);
      if (filter_image == (Image *) NULL)
        break;
      filter_view=AcquireCacheView(filter_image);
      p=GetCacheViewVirtualPixels(filter_view,0,0,1,1,
        resample_filter->exception);
      if (p != (const PixelPacket *) NULL)
        {
          indexes=GetVirtualIndexQueue(filter_image);
          GetMagickPixelPacket(resample_filter->image,pixels);
          SetMagickPixelPacket(resample_filter->image,p,indexes,pixel);
        }
      filter_view=DestroyCacheView(filter_view);
      filter_image=DestroyImage(filter_image);
      break;
    }
    case IntegerInterpolatePixel:
    {
      MagickPixelPacket
        pixels[1];

      p=GetCacheViewVirtualPixels(resample_filter->view,(long) floor(x),(long)
        floor(y),1,1,resample_filter->exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);
      GetMagickPixelPacket(resample_filter->image,pixels);
      SetMagickPixelPacket(resample_filter->image,p,indexes,pixel);
      break;
    }
    case MeshInterpolatePixel:
    {
      MagickPixelPacket
        pixels[4];

      MagickRealType
        alpha[4],
        gamma;

      PointInfo
        delta,
        luminance;

      p=GetCacheViewVirtualPixels(resample_filter->view,(long) floor(x),(long)
        floor(y),2,2,resample_filter->exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);
      for (i=0; i < 4L; i++)
      {
        GetMagickPixelPacket(resample_filter->image,pixels+i);
        SetMagickPixelPacket(resample_filter->image,p,indexes+i,pixels+i);
        alpha[i]=1.0;
        if (resample_filter->image->matte != MagickFalse)
          {
            alpha[i]=QuantumScale*((MagickRealType) GetAlphaPixelComponent(p));
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (resample_filter->image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        p++;
      }
      delta.x=x-floor(x);
      delta.y=y-floor(y);
      luminance.x=MagickPixelLuminance(pixels+0)-MagickPixelLuminance(pixels+3);
      luminance.y=MagickPixelLuminance(pixels+1)-MagickPixelLuminance(pixels+2);
      if (fabs(luminance.x) < fabs(luminance.y))
        {
          /*
            Diagonal 0-3 NW-SE.
          */
          if (delta.x <= delta.y)
            {
              /*
                Bottom-left triangle  (pixel:2, diagonal: 0-3).
              */
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[2],alpha[3],alpha[0]);
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[2].red,
                pixels[3].red,pixels[0].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[2].green,
                pixels[3].green,pixels[0].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[2].blue,
                pixels[3].blue,pixels[0].blue);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[2].opacity,
                pixels[3].opacity,pixels[0].opacity);
              if (resample_filter->image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[2].index,
                  pixels[3].index,pixels[0].index);
            }
          else
            {
              /*
                Top-right triangle (pixel:1, diagonal: 0-3).
              */
              delta.x=1.0-delta.x;
              gamma=MeshInterpolate(&delta,alpha[1],alpha[0],alpha[3]);
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[1].red,
                pixels[0].red,pixels[3].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[1].green,
                pixels[0].green,pixels[3].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[1].blue,
                pixels[0].blue,pixels[3].blue);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[1].opacity,
                pixels[0].opacity,pixels[3].opacity);
              if (resample_filter->image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[1].index,
                  pixels[0].index,pixels[3].index);
            }
        }
      else
        {
          /*
            Diagonal 1-2 NE-SW.
          */
          if (delta.x <= (1.0-delta.y))
            {
              /*
                Top-left triangle (pixel 0, diagonal: 1-2).
              */
              gamma=MeshInterpolate(&delta,alpha[0],alpha[1],alpha[2]);
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[0].red,
                pixels[1].red,pixels[2].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[0].green,
                pixels[1].green,pixels[2].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[0].blue,
                pixels[1].blue,pixels[2].blue);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[0].opacity,
                pixels[1].opacity,pixels[2].opacity);
              if (resample_filter->image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[0].index,
                  pixels[1].index,pixels[2].index);
            }
          else
            {
              /*
                Bottom-right triangle (pixel: 3, diagonal: 1-2).
              */
              delta.x=1.0-delta.x;
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[3],alpha[2],alpha[1]);
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
              pixel->red=gamma*MeshInterpolate(&delta,pixels[3].red,
                pixels[2].red,pixels[1].red);
              pixel->green=gamma*MeshInterpolate(&delta,pixels[3].green,
                pixels[2].green,pixels[1].green);
              pixel->blue=gamma*MeshInterpolate(&delta,pixels[3].blue,
                pixels[2].blue,pixels[1].blue);
              pixel->opacity=gamma*MeshInterpolate(&delta,pixels[3].opacity,
                pixels[2].opacity,pixels[1].opacity);
              if (resample_filter->image->colorspace == CMYKColorspace)
                pixel->index=gamma*MeshInterpolate(&delta,pixels[3].index,
                  pixels[2].index,pixels[1].index);
            }
        }
      break;
    }
    case NearestNeighborInterpolatePixel:
    {
      MagickPixelPacket
        pixels[1];

      p=GetCacheViewVirtualPixels(resample_filter->view,NearestNeighbor(x),
        NearestNeighbor(y),1,1,resample_filter->exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);
      GetMagickPixelPacket(resample_filter->image,pixels);
      SetMagickPixelPacket(resample_filter->image,p,indexes,pixel);
      break;
    }
    case SplineInterpolatePixel:
    {
      long
        j,
        n;

      MagickPixelPacket
        pixels[16];

      MagickRealType
        alpha[16],
        dx,
        dy,
        gamma;

      PointInfo
        delta;

      p=GetCacheViewVirtualPixels(resample_filter->view,(long) floor(x)-1,(long)
        floor(y)-1,4,4,resample_filter->exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);
      n=0;
      delta.x=x-floor(x);
      delta.y=y-floor(y);
      for (i=(-1); i < 3L; i++)
      {
        dy=CubicWeightingFunction((MagickRealType) i-delta.y);
        for (j=(-1); j < 3L; j++)
        {
          GetMagickPixelPacket(resample_filter->image,pixels+n);
          SetMagickPixelPacket(resample_filter->image,p,indexes+n,pixels+n);
          alpha[n]=1.0;
          if (resample_filter->image->matte != MagickFalse)
            {
              alpha[n]=QuantumScale*((MagickRealType) GetAlphaPixelComponent(p));
              pixels[n].red*=alpha[n];
              pixels[n].green*=alpha[n];
              pixels[n].blue*=alpha[n];
              if (resample_filter->image->colorspace == CMYKColorspace)
                pixels[n].index*=alpha[n];
            }
          dx=CubicWeightingFunction(delta.x-(MagickRealType) j);
          gamma=alpha[n];
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          pixel->red+=gamma*dx*dy*pixels[n].red;
          pixel->green+=gamma*dx*dy*pixels[n].green;
          pixel->blue+=gamma*dx*dy*pixels[n].blue;
          if (resample_filter->image->matte != MagickFalse)
            pixel->opacity+=dx*dy*pixels[n].opacity;
          if (resample_filter->image->colorspace == CMYKColorspace)
            pixel->index+=gamma*dx*dy*pixels[n].index;
          n++;
          p++;
        }
      }
      break;
    }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s a m p l e P i x e l C o l o r                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResamplePixelColor() samples the pixel values surrounding the location
%  given using an elliptical weighted average, at the scale previously
%  calculated, and in the most efficent manner possible for the
%  VirtualPixelMethod setting.
%
%  The format of the ResamplePixelColor method is:
%
%     MagickBooleanType ResamplePixelColor(ResampleFilter *resample_filter,
%       const double u0,const double v0,MagickPixelPacket *pixel)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resample filter.
%
%    o u0,v0: A double representing the center of the area to resample,
%        The distortion transformed transformed x,y coordinate.
%
%    o pixel: the resampled pixel is returned here.
%
*/
MagickExport MagickBooleanType ResamplePixelColor(
  ResampleFilter *resample_filter,const double u0,const double v0,
  MagickPixelPacket *pixel)
{
  MagickBooleanType
    status;

  long u,v, uw,v1,v2, hit;
  double u1;
  double U,V,Q,DQ,DDQ;
  double divisor_c,divisor_m;
  register double weight;
  register const PixelPacket *pixels;
  register const IndexPacket *indexes;
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);

  status=MagickTrue;
  GetMagickPixelPacket(resample_filter->image,pixel);
  if ( resample_filter->do_interpolate ) {
    status=InterpolateResampleFilter(resample_filter,
      resample_filter->interpolate,u0,v0,pixel);
    return(status);
  }

  /*
    Does resample area Miss the image?
    And is that area a simple solid color - then return that color
  */
  hit = 0;
  switch ( resample_filter->virtual_pixel ) {
    case BackgroundVirtualPixelMethod:
    case ConstantVirtualPixelMethod:
    case TransparentVirtualPixelMethod:
    case BlackVirtualPixelMethod:
    case GrayVirtualPixelMethod:
    case WhiteVirtualPixelMethod:
    case MaskVirtualPixelMethod:
      if ( resample_filter->limit_reached
           || u0 + resample_filter->sqrtC < 0.0
           || u0 - resample_filter->sqrtC > (double) resample_filter->image->columns
           || v0 + resample_filter->sqrtA < 0.0
           || v0 - resample_filter->sqrtA > (double) resample_filter->image->rows
           )
        hit++;
      break;

    case UndefinedVirtualPixelMethod:
    case EdgeVirtualPixelMethod:
      if (    ( u0 + resample_filter->sqrtC < 0.0 && v0 + resample_filter->sqrtA < 0.0 )
           || ( u0 + resample_filter->sqrtC < 0.0
                && v0 - resample_filter->sqrtA > (double) resample_filter->image->rows )
           || ( u0 - resample_filter->sqrtC > (double) resample_filter->image->columns
                && v0 + resample_filter->sqrtA < 0.0 )
           || ( u0 - resample_filter->sqrtC > (double) resample_filter->image->columns
                && v0 - resample_filter->sqrtA > (double) resample_filter->image->rows )
           )
        hit++;
      break;
    case HorizontalTileVirtualPixelMethod:
      if (    v0 + resample_filter->sqrtA < 0.0
           || v0 - resample_filter->sqrtA > (double) resample_filter->image->rows
           )
        hit++;  /* outside the horizontally tiled images. */
      break;
    case VerticalTileVirtualPixelMethod:
      if (    u0 + resample_filter->sqrtC < 0.0
           || u0 - resample_filter->sqrtC > (double) resample_filter->image->columns
           )
        hit++;  /* outside the vertically tiled images. */
      break;
    case DitherVirtualPixelMethod:
      if (    ( u0 + resample_filter->sqrtC < -32.0 && v0 + resample_filter->sqrtA < -32.0 )
           || ( u0 + resample_filter->sqrtC < -32.0
                && v0 - resample_filter->sqrtA > (double) resample_filter->image->rows+32.0 )
           || ( u0 - resample_filter->sqrtC > (double) resample_filter->image->columns+32.0
                && v0 + resample_filter->sqrtA < -32.0 )
           || ( u0 - resample_filter->sqrtC > (double) resample_filter->image->columns+32.0
                && v0 - resample_filter->sqrtA > (double) resample_filter->image->rows+32.0 )
           )
        hit++;
      break;
    case TileVirtualPixelMethod:
    case MirrorVirtualPixelMethod:
    case RandomVirtualPixelMethod:
    case HorizontalTileEdgeVirtualPixelMethod:
    case VerticalTileEdgeVirtualPixelMethod:
    case CheckerTileVirtualPixelMethod:
      /* resampling of area is always needed - no VP limits */
      break;
  }
  if ( hit ) {
    /* whole area is a solid color -- just return that color */
    status=InterpolateResampleFilter(resample_filter,IntegerInterpolatePixel,
      u0,v0,pixel);
    return(status);
  }

  /*
    Scaling limits reached, return an 'averaged' result.
  */
  if ( resample_filter->limit_reached ) {
    switch ( resample_filter->virtual_pixel ) {
      /*  This is always handled by the above, so no need.
        case BackgroundVirtualPixelMethod:
        case ConstantVirtualPixelMethod:
        case TransparentVirtualPixelMethod:
        case GrayVirtualPixelMethod,
        case WhiteVirtualPixelMethod
        case MaskVirtualPixelMethod:
      */
      case UndefinedVirtualPixelMethod:
      case EdgeVirtualPixelMethod:
      case DitherVirtualPixelMethod:
      case HorizontalTileEdgeVirtualPixelMethod:
      case VerticalTileEdgeVirtualPixelMethod:
        /* We need an average edge pixel, for the right edge!
           How should I calculate an average edge color?
           Just returning an averaged neighbourhood,
           works well in general, but falls down for TileEdge methods.
           This needs to be done properly!!!!!!
        */
        status=InterpolateResampleFilter(resample_filter,
          AverageInterpolatePixel,u0,v0,pixel);
        break;
      case HorizontalTileVirtualPixelMethod:
      case VerticalTileVirtualPixelMethod:
        /* just return the background pixel - Is there more direct way? */
        status=InterpolateResampleFilter(resample_filter,
           IntegerInterpolatePixel,(double)-1,(double)-1,pixel);
        break;
      case TileVirtualPixelMethod:
      case MirrorVirtualPixelMethod:
      case RandomVirtualPixelMethod:
      case CheckerTileVirtualPixelMethod:
      default:
        /* generate a average color of the WHOLE image */
        if ( resample_filter->average_defined == MagickFalse ) {
          Image
            *average_image;

          CacheView
            *average_view;

          GetMagickPixelPacket(resample_filter->image,
                (MagickPixelPacket *)&(resample_filter->average_pixel));
          resample_filter->average_defined = MagickTrue;

          /* Try to get an averaged pixel color of whole image */
          average_image=ResizeImage(resample_filter->image,1,1,BoxFilter,1.0,
           resample_filter->exception);
          if (average_image == (Image *) NULL)
            {
              *pixel=resample_filter->average_pixel; /* FAILED */
              break;
            }
          average_view=AcquireCacheView(average_image);
          pixels=(PixelPacket *)GetCacheViewVirtualPixels(average_view,0,0,1,1,
            resample_filter->exception);
          if (pixels == (const PixelPacket *) NULL) {
            average_view=DestroyCacheView(average_view);
            average_image=DestroyImage(average_image);
            *pixel=resample_filter->average_pixel; /* FAILED */
            break;
          }
          indexes=(IndexPacket *) GetCacheViewAuthenticIndexQueue(average_view);
          SetMagickPixelPacket(resample_filter->image,pixels,indexes,
            &(resample_filter->average_pixel));
          average_view=DestroyCacheView(average_view);
          average_image=DestroyImage(average_image);
#if 0
          /* CheckerTile should average the image with background color */
          //if ( resample_filter->virtual_pixel == CheckerTileVirtualPixelMethod ) {
#if 0
            resample_filter->average_pixel.red =
                      ( resample_filter->average_pixel.red +
                          resample_filter->image->background_color.red ) /2;
            resample_filter->average_pixel.green =
                      ( resample_filter->average_pixel.green +
                          resample_filter->image->background_color.green ) /2;
            resample_filter->average_pixel.blue =
                      ( resample_filter->average_pixel.blue +
                          resample_filter->image->background_color.blue ) /2;
            resample_filter->average_pixel.matte =
                      ( resample_filter->average_pixel.matte +
                          resample_filter->image->background_color.matte ) /2;
            resample_filter->average_pixel.black =
                      ( resample_filter->average_pixel.black +
                          resample_filter->image->background_color.black ) /2;
#else
            resample_filter->average_pixel =
                          resample_filter->image->background_color;
#endif
          }
#endif
        }
        *pixel=resample_filter->average_pixel;
        break;
    }
    return(status);
  }

  /*
    Initialize weighted average data collection
  */
  hit = 0;
  divisor_c = 0.0;
  divisor_m = 0.0;
  pixel->red = pixel->green = pixel->blue = 0.0;
  if (resample_filter->image->matte != MagickFalse) pixel->opacity = 0.0;
  if (resample_filter->image->colorspace == CMYKColorspace) pixel->index = 0.0;

  /*
    Determine the parellelogram bounding box fitted to the ellipse
    centered at u0,v0.  This area is bounding by the lines...
        v = +/- sqrt(A)
        u = -By/2A  +/- sqrt(F/A)
    Which has been pre-calculated above.
  */
  v1 = (long)(v0 - resample_filter->sqrtA);               /* range of scan lines */
  v2 = (long)(v0 + resample_filter->sqrtA + 1);

  u1 = u0 + (v1-v0)*resample_filter->slope - resample_filter->sqrtU; /* start of scanline for v=v1 */
  uw = (long)(2*resample_filter->sqrtU)+1;       /* width of parallelogram */

  /*
    Do weighted resampling of all pixels,  within the scaled ellipse,
    bound by a Parellelogram fitted to the ellipse.
  */
  DDQ = 2*resample_filter->A;
  for( v=v1; v<=v2;  v++, u1+=resample_filter->slope ) {
    u = (long)u1;       /* first pixel in scanline  ( floor(u1) ) */
    U = (double)u-u0;   /* location of that pixel, relative to u0,v0 */
    V = (double)v-v0;

    /* Q = ellipse quotent ( if Q<F then pixel is inside ellipse) */
    Q = U*(resample_filter->A*U + resample_filter->B*V) + resample_filter->C*V*V;
    DQ = resample_filter->A*(2.0*U+1) + resample_filter->B*V;

    /* get the scanline of pixels for this v */
    pixels=GetCacheViewVirtualPixels(resample_filter->view,u,v,(unsigned long) uw,
      1,resample_filter->exception);
    if (pixels == (const PixelPacket *) NULL)
      return(MagickFalse);
    indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);

    /* count up the weighted pixel colors */
    for( u=0; u<uw; u++ ) {
      /* Note that the ellipse has been pre-scaled so F = WLUT_WIDTH */
      if ( Q < (double)WLUT_WIDTH ) {
        weight = resample_filter->filter_lut[(int)Q];

        pixel->opacity  += weight*pixels->opacity;
        divisor_m += weight;

        if (resample_filter->image->matte != MagickFalse)
          weight *= QuantumScale*((MagickRealType)(QuantumRange-pixels->opacity));
        pixel->red   += weight*pixels->red;
        pixel->green += weight*pixels->green;
        pixel->blue  += weight*pixels->blue;
        if (resample_filter->image->colorspace == CMYKColorspace)
          pixel->index += weight*(*indexes);
        divisor_c += weight;

        hit++;
      }
      pixels++;
      indexes++;
      Q += DQ;
      DQ += DDQ;
    }
  }

  /*
    Result sanity check -- this should NOT happen
  */
  if ( hit < 4 || divisor_c < 1.0 ) {
    /* not enough pixels in resampling, resort to direct interpolation */
    status=InterpolateResampleFilter(resample_filter,
      resample_filter->interpolate,u0,v0,pixel);
    return status;
  }

  /*
    Finialize results of resampling
  */
  divisor_m = 1.0/divisor_m;
  pixel->opacity = (MagickRealType) ClampToQuantum(divisor_m*pixel->opacity);
  divisor_c = 1.0/divisor_c;
  pixel->red   = (MagickRealType) ClampToQuantum(divisor_c*pixel->red);
  pixel->green = (MagickRealType) ClampToQuantum(divisor_c*pixel->green);
  pixel->blue  = (MagickRealType) ClampToQuantum(divisor_c*pixel->blue);
  if (resample_filter->image->colorspace == CMYKColorspace)
    pixel->index = (MagickRealType) ClampToQuantum(divisor_c*pixel->index);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S c a l e R e s a m p l e F i l t e r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ScaleResampleFilter() does all the calculations needed to resample an image
%  at a specific scale, defined by two scaling vectors.  This not using
%  a orthogonal scaling, but two distorted scaling vectors, to allow the
%  generation of a angled ellipse.
%
%  As only two deritive scaling vectors are used the center of the ellipse
%  must be the center of the lookup.  That is any curvature that the
%  distortion may produce is discounted.
%
%  The input vectors are produced by either finding the derivitives of the
%  distortion function, or the partial derivitives from a distortion mapping.
%  They do not need to be the orthogonal dx,dy scaling vectors, but can be
%  calculated from other derivatives.  For example you could use  dr,da/r
%  polar coordinate vector scaling vectors
%
%  If   u,v =  DistortEquation(x,y)
%  Then the scaling vectors dx,dy (in u,v space) are the derivitives...
%      du/dx, dv/dx     and    du/dy, dv/dy
%  If the scaling is only othogonally aligned then...
%      dv/dx = 0   and   du/dy  =  0
%  Producing an othogonally alligned ellipse for the area to be resampled.
%
%  Note that scaling vectors are different to argument order.  Argument order
%  is the general order the deritives are extracted from the distortion
%  equations, EG: U(x,y), V(x,y).  Caution is advised if you are trying to
%  define the ellipse directly from scaling vectors.
%
%  The format of the ScaleResampleFilter method is:
%
%     void ScaleResampleFilter(const ResampleFilter *resample_filter,
%       const double dux,const double duy,const double dvx,const double dvy)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resampling resample_filterrmation defining the
%      image being resampled
%
%    o dux,duy,dvx,dvy:
%         The partial derivitives or scaling vectors for resampling.
%           dx = du/dx, dv/dx    and  dy = du/dy, dv/dy
%
%         The values are used to define the size and angle of the
%         elliptical resampling area, centered on the lookup point.
%
*/
MagickExport void ScaleResampleFilter(ResampleFilter *resample_filter,
  const double dux,const double duy,const double dvx,const double dvy)
{
  double A,B,C,F, area;

  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);

  resample_filter->limit_reached = MagickFalse;
  resample_filter->do_interpolate = MagickFalse;

  /* A 'point' filter forces use of interpolation instead of area sampling */
  if ( resample_filter->filter == PointFilter ) {
    resample_filter->do_interpolate = MagickTrue;
    return;
  }

  /* Find Ellipse Coefficents such that
        A*u^2 + B*u*v + C*v^2 = F
     With u,v relative to point around which we are resampling.
     And the given scaling dx,dy vectors in u,v space
         du/dx,dv/dx   and  du/dy,dv/dy
  */
#if 0
  /* Direct conversions of derivatives to elliptical coefficients
     No scaling will result in F == 1.0 and a unit circle.
  */
  A = dvx*dvx+dvy*dvy;
  B = (dux*dvx+duy*dvy)*-2.0;
  C = dux*dux+duy*duy;
  F = dux*dvy+duy*dvx;
  F *= F;
#define F_UNITY 1.0
#else
  /* This Paul Heckbert's recomended "Higher Quality EWA" formula, from page
     60 in his thesis, which adds a unit circle to the elliptical area so are
     to do both Reconstruction and Prefiltering of the pixels in the
     resampling.  It also means it is likely to have at least 4 pixels within
     the area of the ellipse, for weighted averaging.
     No scaling will result if F == 4.0 and a circle of radius 2.0
  */
  A = dvx*dvx+dvy*dvy+1;
  B = (dux*dvx+duy*dvy)*-2.0;
  C = dux*dux+duy*duy+1;
  F = A*C - B*B/4;
#define F_UNITY 4.0
#endif

/* DEBUGGING OUTPUT */
#if 0
  fprintf(stderr, "dux=%lf; dvx=%lf;   duy=%lf; dvy%lf;\n",
       dux, dvx, duy, dvy);
  fprintf(stderr, "A=%lf; B=%lf; C=%lf; F=%lf\n", A,B,C,F);
#endif

#if 0
  /* Figure out the Ellipses Major and Minor Axis, and other info.
     This information currently not needed at this time, but may be
     needed later for better limit determination.
  */
  { double alpha, beta, gamma, Major, Minor;
    double Eccentricity, Ellipse_Area, Ellipse_angle;
    double max_horizontal_cross_section, max_vertical_cross_section;
    alpha = A+C;
    beta  = A-C;
    gamma = sqrt(beta*beta + B*B );

    if ( alpha - gamma <= MagickEpsilon )
      Major = MagickHuge;
    else
      Major = sqrt(2*F/(alpha - gamma));
    Minor = sqrt(2*F/(alpha + gamma));

    fprintf(stderr, "\tMajor=%lf; Minor=%lf\n",
         Major, Minor );

    /* other information about ellipse include... */
    Eccentricity = Major/Minor;
    Ellipse_Area = MagickPI*Major*Minor;
    Ellipse_angle =  atan2(B, A-C);

    fprintf(stderr, "\tAngle=%lf Area=%lf\n",
         RadiansToDegrees(Ellipse_angle), Ellipse_Area );

    /* Ellipse limits */

    /* orthogonal rectangle - improved ellipse */
    max_horizontal_orthogonal = sqrt(A); /* = sqrt(4*A*F/(4*A*C-B*B)) */
    max_vertical_orthogonal   = sqrt(C); /* = sqrt(4*C*F/(4*A*C-B*B)) */

    /* parallelogram bounds -- what we are using */
    max_horizontal_cross_section = sqrt(F/A);
    max_vertical_cross_section   = sqrt(F/C);
  }
#endif

  /* Is default elliptical area, too small? Image being magnified?
     Switch to doing pure 'point' interpolation of the pixel.
     That is turn off  EWA Resampling.
  */
  if ( F <= F_UNITY ) {
    resample_filter->do_interpolate = MagickTrue;
    return;
  }


  /* If F is impossibly large, we may as well not bother doing any
   * form of resampling, as you risk an infinite resampled area.
  */
  if ( F > MagickHuge ) {
    resample_filter->limit_reached = MagickTrue;
    return;
  }

  /* Othogonal bounds of the ellipse */
  resample_filter->sqrtA = sqrt(A)+1.0;     /* Vertical Orthogonal Limit */
  resample_filter->sqrtC = sqrt(C)+1.0;     /* Horizontal Orthogonal Limit */

  /* Horizontally aligned Parallelogram fitted to ellipse */
  resample_filter->sqrtU = sqrt(F/A)+1.0;   /* Parallelogram Width */
  resample_filter->slope = -B/(2*A);        /* Slope of the parallelogram */

  /* The size of the area of the parallelogram we will be sampling */
  area = 4 * resample_filter->sqrtA * resample_filter->sqrtU;

  /* Absolute limit on the area to be resampled
   * This limit needs more work, as it gets too slow for
   * larger images involved with tiled views of the horizon. */
  if ( area > 20.0*resample_filter->image_area ) {
    resample_filter->limit_reached = MagickTrue;
    return;
  }

  /* Scale ellipse formula to directly fit the Filter Lookup Table */
  { register double scale;
    scale = (double)WLUT_WIDTH/F;
    resample_filter->A = A*scale;
    resample_filter->B = B*scale;
    resample_filter->C = C*scale;
    /* ..ple_filter->F = WLUT_WIDTH; -- hardcoded */
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R e s a m p l e F i l t e r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetResampleFilter() set the resampling filter lookup table based on a
%  specific filter.  Note that the filter is used as a radial filter not as a
%  two pass othogonally aligned resampling filter.
%
%  The default Filter, is Gaussian, which is the standard filter used by the
%  original paper on the Elliptical Weighted Everage Algorithm. However other
%  filters can also be used.
%
%  The format of the SetResampleFilter method is:
%
%    void SetResampleFilter(ResampleFilter *resample_filter,
%      const FilterTypes filter,const double blur)
%
%  A description of each parameter follows:
%
%    o resample_filter: resampling resample_filterrmation structure
%
%    o filter: the resize filter for elliptical weighting LUT
%
%    o blur: filter blur factor (radial scaling) for elliptical weighting LUT
%
*/
MagickExport void SetResampleFilter(ResampleFilter *resample_filter,
  const FilterTypes filter,const double blur)
{
  register int
     Q;

  double
     r_scale;

  ResizeFilter
     *resize_filter;

  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);

  resample_filter->filter = filter;

  /* Scale radius so it equals 1.0, at edge of ellipse when a
     default blurring factor of 1.0 is used.

     Note that these filters are being used as a radial filter, not as
     an othoginally alligned filter. How this effects results is still
     being worked out.

     Future: Direct use of teh resize filters in "resize.c" to set the lookup
     table, based on the filters working support window.
  */
  r_scale = sqrt(1.0/(double)WLUT_WIDTH)/blur;
  r_scale *= 2; /* for 2 pixel radius of Improved Elliptical Formula */

  switch ( filter ) {
  case PointFilter:
    /* This equivelent to turning off the EWA algroithm.
       Only Interpolated lookup will be used.  */
    break;
  default:
    /*
      Fill the LUT with a 1D resize filter function
      But make the Sinc/Bessel tapered window 2.0
      I also normalize the result so the filter is 1.0
    */
    resize_filter = AcquireResizeFilter(resample_filter->image,filter,
         (MagickRealType)1.0,MagickTrue,resample_filter->exception);
    if (resize_filter != (ResizeFilter *) NULL) {
      resample_filter->support = GetResizeFilterSupport(resize_filter);
      resample_filter->support /= blur; /* taken into account above */
      resample_filter->support *= resample_filter->support;
      resample_filter->support *= (double)WLUT_WIDTH/4;
      if ( resample_filter->support >= (double)WLUT_WIDTH )
           resample_filter->support = (double)WLUT_WIDTH;  /* hack */
      for(Q=0; Q<WLUT_WIDTH; Q++)
        if ( (double) Q < resample_filter->support )
          resample_filter->filter_lut[Q] = (double)
               GetResizeFilterWeight(resize_filter,sqrt((double)Q)*r_scale);
        else
          resample_filter->filter_lut[Q] = 0.0;
      resize_filter = DestroyResizeFilter(resize_filter);
      break;
    }
    else {
      (void) ThrowMagickException(resample_filter->exception,GetMagickModule(),
           ModuleError, "UnableToSetFilteringValue",
           "Fall back to default EWA gaussian filter");
    }
    /* FALLTHRU - on exception */
  /*case GaussianFilter:*/
  case UndefinedFilter:
    /*
      Create Normal Gaussian 2D Filter Weighted Lookup Table.
      A normal EWA guassual lookup would use   exp(Q*ALPHA)
      where  Q = distantce squared from 0.0 (center) to 1.0 (edge)
      and    ALPHA = -4.0*ln(2.0)  ==>  -2.77258872223978123767
      However the table is of length 1024, and equates to a radius of 2px
      thus needs to be scaled by  ALPHA*4/1024 and any blur factor squared
    */
    /*r_scale = -2.77258872223978123767*4/WLUT_WIDTH/blur/blur;*/
    r_scale = -2.77258872223978123767/WLUT_WIDTH/blur/blur;
    for(Q=0; Q<WLUT_WIDTH; Q++)
      resample_filter->filter_lut[Q] = exp((double)Q*r_scale);
    resample_filter->support = WLUT_WIDTH;
    break;
  }
  if (GetImageArtifact(resample_filter->image,"resample:verbose")
        != (const char *) NULL)
    /* Debug output of the filter weighting LUT
      Gnuplot the LUT with hoizontal adjusted to 'r' using...
        plot [0:2][-.2:1] "lut.dat" using (sqrt($0/1024)*2):1 with lines
      THe filter values is normalized for comparision
    */
    for(Q=0; Q<WLUT_WIDTH; Q++)
      printf("%lf\n", resample_filter->filter_lut[Q]
                        /resample_filter->filter_lut[0] );
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R e s a m p l e F i l t e r I n t e r p o l a t e M e t h o d       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetResampleFilterInterpolateMethod() changes the interpolation method
%  associated with the specified resample filter.
%
%  The format of the SetResampleFilterInterpolateMethod method is:
%
%      MagickBooleanType SetResampleFilterInterpolateMethod(
%        ResampleFilter *resample_filter,const InterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resample filter.
%
%    o method: the interpolation method.
%
*/
MagickExport MagickBooleanType SetResampleFilterInterpolateMethod(
  ResampleFilter *resample_filter,const InterpolatePixelMethod method)
{
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);
  assert(resample_filter->image != (Image *) NULL);
  if (resample_filter->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      resample_filter->image->filename);
  resample_filter->interpolate=method;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R e s a m p l e F i l t e r V i r t u a l P i x e l M e t h o d     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetResampleFilterVirtualPixelMethod() changes the virtual pixel method
%  associated with the specified resample filter.
%
%  The format of the SetResampleFilterVirtualPixelMethod method is:
%
%      MagickBooleanType SetResampleFilterVirtualPixelMethod(
%        ResampleFilter *resample_filter,const VirtualPixelMethod method)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resample filter.
%
%    o method: the virtual pixel method.
%
*/
MagickExport MagickBooleanType SetResampleFilterVirtualPixelMethod(
  ResampleFilter *resample_filter,const VirtualPixelMethod method)
{
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);
  assert(resample_filter->image != (Image *) NULL);
  if (resample_filter->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      resample_filter->image->filename);
  resample_filter->virtual_pixel=method;
  (void) SetCacheViewVirtualPixelMethod(resample_filter->view,method);
  return(MagickTrue);
}

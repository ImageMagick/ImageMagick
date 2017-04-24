/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               FFFFF  EEEEE   AAA   TTTTT  U   U  RRRR   EEEEE               %
%               F      E      A   A    T    U   U  R   R  E                   %
%               FFF    EEE    AAAAA    T    U   U  RRRR   EEE                 %
%               F      E      A   A    T    U   U  R R    E                   %
%               F      EEEEE  A   A    T     UUU   R  R   EEEEE               %
%                                                                             %
%                                                                             %
%                      MagickCore Image Feature Methods                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/animate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/compress.h"
#include "MagickCore/constitute.h"
#include "MagickCore/display.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/feature.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/list.h"
#include "MagickCore/image-private.h"
#include "MagickCore/magic.h"
#include "MagickCore/magick.h"
#include "MagickCore/matrix.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/morphology-private.h"
#include "MagickCore/option.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/timer.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C a n n y E d g e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CannyEdgeImage() uses a multi-stage algorithm to detect a wide range of
%  edges in images.
%
%  The format of the CannyEdgeImage method is:
%
%      Image *CannyEdgeImage(const Image *image,const double radius,
%        const double sigma,const double lower_percent,
%        const double upper_percent,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the gaussian smoothing filter.
%
%    o sigma: the sigma of the gaussian smoothing filter.
%
%    o lower_precent: percentage of edge pixels in the lower threshold.
%
%    o upper_percent: percentage of edge pixels in the upper threshold.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _CannyInfo
{
  double
    magnitude,
    intensity;

  int
    orientation;

  ssize_t
    x,
    y;
} CannyInfo;

static inline MagickBooleanType IsAuthenticPixel(const Image *image,
  const ssize_t x,const ssize_t y)
{
  if ((x < 0) || (x >= (ssize_t) image->columns))
    return(MagickFalse);
  if ((y < 0) || (y >= (ssize_t) image->rows))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType TraceEdges(Image *edge_image,CacheView *edge_view,
  MatrixInfo *canny_cache,const ssize_t x,const ssize_t y,
  const double lower_threshold,ExceptionInfo *exception)
{
  CannyInfo
    edge,
    pixel;

  MagickBooleanType
    status;

  register Quantum
    *q;

  register ssize_t
    i;

  q=GetCacheViewAuthenticPixels(edge_view,x,y,1,1,exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  *q=QuantumRange;
  status=SyncCacheViewAuthenticPixels(edge_view,exception);
  if (status == MagickFalse)
    return(MagickFalse);;
  if (GetMatrixElement(canny_cache,0,0,&edge) == MagickFalse)
    return(MagickFalse);
  edge.x=x;
  edge.y=y;
  if (SetMatrixElement(canny_cache,0,0,&edge) == MagickFalse)
    return(MagickFalse);
  for (i=1; i != 0; )
  {
    ssize_t
      v;

    i--;
    status=GetMatrixElement(canny_cache,i,0,&edge);
    if (status == MagickFalse)
      return(MagickFalse);
    for (v=(-1); v <= 1; v++)
    {
      ssize_t
        u;

      for (u=(-1); u <= 1; u++)
      {
        if ((u == 0) && (v == 0))
          continue;
        if (IsAuthenticPixel(edge_image,edge.x+u,edge.y+v) == MagickFalse)
          continue;
        /*
          Not an edge if gradient value is below the lower threshold.
        */
        q=GetCacheViewAuthenticPixels(edge_view,edge.x+u,edge.y+v,1,1,
          exception);
        if (q == (Quantum *) NULL)
          return(MagickFalse);
        status=GetMatrixElement(canny_cache,edge.x+u,edge.y+v,&pixel);
        if (status == MagickFalse)
          return(MagickFalse);
        if ((GetPixelIntensity(edge_image,q) == 0.0) &&
            (pixel.intensity >= lower_threshold))
          {
            *q=QuantumRange;
            status=SyncCacheViewAuthenticPixels(edge_view,exception);
            if (status == MagickFalse)
              return(MagickFalse);
            edge.x+=u;
            edge.y+=v;
            status=SetMatrixElement(canny_cache,i,0,&edge);
            if (status == MagickFalse)
              return(MagickFalse);
            i++;
          }
      }
    }
  }
  return(MagickTrue);
}

MagickExport Image *CannyEdgeImage(const Image *image,const double radius,
  const double sigma,const double lower_percent,const double upper_percent,
  ExceptionInfo *exception)
{
#define CannyEdgeImageTag  "CannyEdge/Image"

  CacheView
    *edge_view;

  CannyInfo
    element;

  char
    geometry[MagickPathExtent];

  double
    lower_threshold,
    max,
    min,
    upper_threshold;

  Image
    *edge_image;

  KernelInfo
    *kernel_info;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MatrixInfo
    *canny_cache;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  /*
    Filter out noise.
  */
  (void) FormatLocaleString(geometry,MagickPathExtent,
    "blur:%.20gx%.20g;blur:%.20gx%.20g+90",radius,sigma,radius,sigma);
  kernel_info=AcquireKernelInfo(geometry,exception);
  if (kernel_info == (KernelInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  edge_image=ConvolveImage(image, kernel_info, exception);
  kernel_info=DestroyKernelInfo(kernel_info);
  if (edge_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageColorspace(edge_image,GRAYColorspace,exception) == MagickFalse)
    {
      edge_image=DestroyImage(edge_image);
      return((Image *) NULL);
    }
  (void) SetImageAlphaChannel(edge_image,OffAlphaChannel,exception);
  /*
    Find the intensity gradient of the image.
  */
  canny_cache=AcquireMatrixInfo(edge_image->columns,edge_image->rows,
    sizeof(CannyInfo),exception);
  if (canny_cache == (MatrixInfo *) NULL)
    {
      edge_image=DestroyImage(edge_image);
      return((Image *) NULL);
    }
  status=MagickTrue;
  edge_view=AcquireVirtualCacheView(edge_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(edge_image,edge_image,edge_image->rows,1)
#endif
  for (y=0; y < (ssize_t) edge_image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns+1,2,
      exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) edge_image->columns; x++)
    {
      CannyInfo
        pixel;

      double
        dx,
        dy;

      register const Quantum
        *magick_restrict kernel_pixels;

      ssize_t
        v;

      static double
        Gx[2][2] =
        {
          { -1.0,  +1.0 },
          { -1.0,  +1.0 }
        },
        Gy[2][2] =
        {
          { +1.0, +1.0 },
          { -1.0, -1.0 }
        };

      (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
      dx=0.0;
      dy=0.0;
      kernel_pixels=p;
      for (v=0; v < 2; v++)
      {
        ssize_t
          u;

        for (u=0; u < 2; u++)
        {
          double
            intensity;

          intensity=GetPixelIntensity(edge_image,kernel_pixels+u);
          dx+=0.5*Gx[v][u]*intensity;
          dy+=0.5*Gy[v][u]*intensity;
        }
        kernel_pixels+=edge_image->columns+1;
      }
      pixel.magnitude=hypot(dx,dy);
      pixel.orientation=0;
      if (fabs(dx) > MagickEpsilon)
        {
          double
            slope;

          slope=dy/dx;
          if (slope < 0.0)
            {
              if (slope < -2.41421356237)
                pixel.orientation=0;
              else
                if (slope < -0.414213562373)
                  pixel.orientation=1;
                else
                  pixel.orientation=2;
            }
          else
            {
              if (slope > 2.41421356237)
                pixel.orientation=0;
              else
                if (slope > 0.414213562373)
                  pixel.orientation=3;
                else
                  pixel.orientation=2;
            }
        }
      if (SetMatrixElement(canny_cache,x,y,&pixel) == MagickFalse)
        continue;
      p+=GetPixelChannels(edge_image);
    }
  }
  edge_view=DestroyCacheView(edge_view);
  /*
    Non-maxima suppression, remove pixels that are not considered to be part
    of an edge.
  */
  progress=0;
  (void) GetMatrixElement(canny_cache,0,0,&element);
  max=element.intensity;
  min=element.intensity;
  edge_view=AcquireAuthenticCacheView(edge_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(edge_image,edge_image,edge_image->rows,1)
#endif
  for (y=0; y < (ssize_t) edge_image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(edge_view,0,y,edge_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) edge_image->columns; x++)
    {
      CannyInfo
        alpha_pixel,
        beta_pixel,
        pixel;

      (void) GetMatrixElement(canny_cache,x,y,&pixel);
      switch (pixel.orientation)
      {
        case 0:
        default:
        {
          /*
            0 degrees, north and south.
          */
          (void) GetMatrixElement(canny_cache,x,y-1,&alpha_pixel);
          (void) GetMatrixElement(canny_cache,x,y+1,&beta_pixel);
          break;
        }
        case 1:
        {
          /*
            45 degrees, northwest and southeast.
          */
          (void) GetMatrixElement(canny_cache,x-1,y-1,&alpha_pixel);
          (void) GetMatrixElement(canny_cache,x+1,y+1,&beta_pixel);
          break;
        }
        case 2:
        {
          /*
            90 degrees, east and west.
          */
          (void) GetMatrixElement(canny_cache,x-1,y,&alpha_pixel);
          (void) GetMatrixElement(canny_cache,x+1,y,&beta_pixel);
          break;
        }
        case 3:
        {
          /*
            135 degrees, northeast and southwest.
          */
          (void) GetMatrixElement(canny_cache,x+1,y-1,&beta_pixel);
          (void) GetMatrixElement(canny_cache,x-1,y+1,&alpha_pixel);
          break;
        }
      }
      pixel.intensity=pixel.magnitude;
      if ((pixel.magnitude < alpha_pixel.magnitude) ||
          (pixel.magnitude < beta_pixel.magnitude))
        pixel.intensity=0;
      (void) SetMatrixElement(canny_cache,x,y,&pixel);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp critical (MagickCore_CannyEdgeImage)
#endif
      {
        if (pixel.intensity < min)
          min=pixel.intensity;
        if (pixel.intensity > max)
          max=pixel.intensity;
      }
      *q=0;
      q+=GetPixelChannels(edge_image);
    }
    if (SyncCacheViewAuthenticPixels(edge_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  edge_view=DestroyCacheView(edge_view);
  /*
    Estimate hysteresis threshold.
  */
  lower_threshold=lower_percent*(max-min)+min;
  upper_threshold=upper_percent*(max-min)+min;
  /*
    Hysteresis threshold.
  */
  edge_view=AcquireAuthenticCacheView(edge_image,exception);
  for (y=0; y < (ssize_t) edge_image->rows; y++)
  {
    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    for (x=0; x < (ssize_t) edge_image->columns; x++)
    {
      CannyInfo
        pixel;

      register const Quantum
        *magick_restrict p;

      /*
        Edge if pixel gradient higher than upper threshold.
      */
      p=GetCacheViewVirtualPixels(edge_view,x,y,1,1,exception);
      if (p == (const Quantum *) NULL)
        continue;
      status=GetMatrixElement(canny_cache,x,y,&pixel);
      if (status == MagickFalse)
        continue;
      if ((GetPixelIntensity(edge_image,p) == 0.0) &&
          (pixel.intensity >= upper_threshold))
        status=TraceEdges(edge_image,edge_view,canny_cache,x,y,lower_threshold,
          exception);
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_CannyEdgeImage)
#endif
        proceed=SetImageProgress(image,CannyEdgeImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  edge_view=DestroyCacheView(edge_view);
  /*
    Free resources.
  */
  canny_cache=DestroyMatrixInfo(canny_cache);
  return(edge_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e F e a t u r e s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageFeatures() returns features for each channel in the image in
%  each of four directions (horizontal, vertical, left and right diagonals)
%  for the specified distance.  The features include the angular second
%  moment, contrast, correlation, sum of squares: variance, inverse difference
%  moment, sum average, sum varience, sum entropy, entropy, difference variance,%  difference entropy, information measures of correlation 1, information
%  measures of correlation 2, and maximum correlation coefficient.  You can
%  access the red channel contrast, for example, like this:
%
%      channel_features=GetImageFeatures(image,1,exception);
%      contrast=channel_features[RedPixelChannel].contrast[0];
%
%  Use MagickRelinquishMemory() to free the features buffer.
%
%  The format of the GetImageFeatures method is:
%
%      ChannelFeatures *GetImageFeatures(const Image *image,
%        const size_t distance,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o distance: the distance.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickLog10(const double x)
{
#define Log10Epsilon  (1.0e-11)

 if (fabs(x) < Log10Epsilon)
   return(log10(Log10Epsilon));
 return(log10(fabs(x)));
}

MagickExport ChannelFeatures *GetImageFeatures(const Image *image,
  const size_t distance,ExceptionInfo *exception)
{
  typedef struct _ChannelStatistics
  {
    PixelInfo
      direction[4];  /* horizontal, vertical, left and right diagonals */
  } ChannelStatistics;

  CacheView
    *image_view;

  ChannelFeatures
    *channel_features;

  ChannelStatistics
    **cooccurrence,
    correlation,
    *density_x,
    *density_xy,
    *density_y,
    entropy_x,
    entropy_xy,
    entropy_xy1,
    entropy_xy2,
    entropy_y,
    mean,
    **Q,
    *sum,
    sum_squares,
    variance;

  PixelPacket
    gray,
    *grays;

  MagickBooleanType
    status;

  register ssize_t
    i,
    r;

  size_t
    length;

  unsigned int
    number_grays;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns < (distance+1)) || (image->rows < (distance+1)))
    return((ChannelFeatures *) NULL);
  length=MaxPixelChannels+1UL;
  channel_features=(ChannelFeatures *) AcquireQuantumMemory(length,
    sizeof(*channel_features));
  if (channel_features == (ChannelFeatures *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_features,0,length*
    sizeof(*channel_features));
  /*
    Form grays.
  */
  grays=(PixelPacket *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*grays));
  if (grays == (PixelPacket *) NULL)
    {
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    grays[i].red=(~0U);
    grays[i].green=(~0U);
    grays[i].blue=(~0U);
    grays[i].alpha=(~0U);
    grays[i].black=(~0U);
  }
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (r=0; r < (ssize_t) image->rows; r++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,r,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      grays[ScaleQuantumToMap(GetPixelRed(image,p))].red=
        ScaleQuantumToMap(GetPixelRed(image,p));
      grays[ScaleQuantumToMap(GetPixelGreen(image,p))].green=
        ScaleQuantumToMap(GetPixelGreen(image,p));
      grays[ScaleQuantumToMap(GetPixelBlue(image,p))].blue=
        ScaleQuantumToMap(GetPixelBlue(image,p));
      if (image->colorspace == CMYKColorspace)
        grays[ScaleQuantumToMap(GetPixelBlack(image,p))].black=
          ScaleQuantumToMap(GetPixelBlack(image,p));
      if (image->alpha_trait != UndefinedPixelTrait)
        grays[ScaleQuantumToMap(GetPixelAlpha(image,p))].alpha=
          ScaleQuantumToMap(GetPixelAlpha(image,p));
      p+=GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      grays=(PixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  (void) ResetMagickMemory(&gray,0,sizeof(gray));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if (grays[i].red != ~0U)
      grays[gray.red++].red=grays[i].red;
    if (grays[i].green != ~0U)
      grays[gray.green++].green=grays[i].green;
    if (grays[i].blue != ~0U)
      grays[gray.blue++].blue=grays[i].blue;
    if (image->colorspace == CMYKColorspace)
      if (grays[i].black != ~0U)
        grays[gray.black++].black=grays[i].black;
    if (image->alpha_trait != UndefinedPixelTrait)
      if (grays[i].alpha != ~0U)
        grays[gray.alpha++].alpha=grays[i].alpha;
  }
  /*
    Allocate spatial dependence matrix.
  */
  number_grays=gray.red;
  if (gray.green > number_grays)
    number_grays=gray.green;
  if (gray.blue > number_grays)
    number_grays=gray.blue;
  if (image->colorspace == CMYKColorspace)
    if (gray.black > number_grays)
      number_grays=gray.black;
  if (image->alpha_trait != UndefinedPixelTrait)
    if (gray.alpha > number_grays)
      number_grays=gray.alpha;
  cooccurrence=(ChannelStatistics **) AcquireQuantumMemory(number_grays,
    sizeof(*cooccurrence));
  density_x=(ChannelStatistics *) AcquireQuantumMemory(2*(number_grays+1),
    sizeof(*density_x));
  density_xy=(ChannelStatistics *) AcquireQuantumMemory(2*(number_grays+1),
    sizeof(*density_xy));
  density_y=(ChannelStatistics *) AcquireQuantumMemory(2*(number_grays+1),
    sizeof(*density_y));
  Q=(ChannelStatistics **) AcquireQuantumMemory(number_grays,sizeof(*Q));
  sum=(ChannelStatistics *) AcquireQuantumMemory(number_grays,sizeof(*sum));
  if ((cooccurrence == (ChannelStatistics **) NULL) ||
      (density_x == (ChannelStatistics *) NULL) ||
      (density_xy == (ChannelStatistics *) NULL) ||
      (density_y == (ChannelStatistics *) NULL) ||
      (Q == (ChannelStatistics **) NULL) ||
      (sum == (ChannelStatistics *) NULL))
    {
      if (Q != (ChannelStatistics **) NULL)
        {
          for (i=0; i < (ssize_t) number_grays; i++)
            Q[i]=(ChannelStatistics *) RelinquishMagickMemory(Q[i]);
          Q=(ChannelStatistics **) RelinquishMagickMemory(Q);
        }
      if (sum != (ChannelStatistics *) NULL)
        sum=(ChannelStatistics *) RelinquishMagickMemory(sum);
      if (density_y != (ChannelStatistics *) NULL)
        density_y=(ChannelStatistics *) RelinquishMagickMemory(density_y);
      if (density_xy != (ChannelStatistics *) NULL)
        density_xy=(ChannelStatistics *) RelinquishMagickMemory(density_xy);
      if (density_x != (ChannelStatistics *) NULL)
        density_x=(ChannelStatistics *) RelinquishMagickMemory(density_x);
      if (cooccurrence != (ChannelStatistics **) NULL)
        {
          for (i=0; i < (ssize_t) number_grays; i++)
            cooccurrence[i]=(ChannelStatistics *)
              RelinquishMagickMemory(cooccurrence[i]);
          cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(
            cooccurrence);
        }
      grays=(PixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  (void) ResetMagickMemory(&correlation,0,sizeof(correlation));
  (void) ResetMagickMemory(density_x,0,2*(number_grays+1)*sizeof(*density_x));
  (void) ResetMagickMemory(density_xy,0,2*(number_grays+1)*sizeof(*density_xy));
  (void) ResetMagickMemory(density_y,0,2*(number_grays+1)*sizeof(*density_y));
  (void) ResetMagickMemory(&mean,0,sizeof(mean));
  (void) ResetMagickMemory(sum,0,number_grays*sizeof(*sum));
  (void) ResetMagickMemory(&sum_squares,0,sizeof(sum_squares));
  (void) ResetMagickMemory(density_xy,0,2*number_grays*sizeof(*density_xy));
  (void) ResetMagickMemory(&entropy_x,0,sizeof(entropy_x));
  (void) ResetMagickMemory(&entropy_xy,0,sizeof(entropy_xy));
  (void) ResetMagickMemory(&entropy_xy1,0,sizeof(entropy_xy1));
  (void) ResetMagickMemory(&entropy_xy2,0,sizeof(entropy_xy2));
  (void) ResetMagickMemory(&entropy_y,0,sizeof(entropy_y));
  (void) ResetMagickMemory(&variance,0,sizeof(variance));
  for (i=0; i < (ssize_t) number_grays; i++)
  {
    cooccurrence[i]=(ChannelStatistics *) AcquireQuantumMemory(number_grays,
      sizeof(**cooccurrence));
    Q[i]=(ChannelStatistics *) AcquireQuantumMemory(number_grays,sizeof(**Q));
    if ((cooccurrence[i] == (ChannelStatistics *) NULL) ||
        (Q[i] == (ChannelStatistics *) NULL))
      break;
    (void) ResetMagickMemory(cooccurrence[i],0,number_grays*
      sizeof(**cooccurrence));
    (void) ResetMagickMemory(Q[i],0,number_grays*sizeof(**Q));
  }
  if (i < (ssize_t) number_grays)
    {
      for (i--; i >= 0; i--)
      {
        if (Q[i] != (ChannelStatistics *) NULL)
          Q[i]=(ChannelStatistics *) RelinquishMagickMemory(Q[i]);
        if (cooccurrence[i] != (ChannelStatistics *) NULL)
          cooccurrence[i]=(ChannelStatistics *)
            RelinquishMagickMemory(cooccurrence[i]);
      }
      Q=(ChannelStatistics **) RelinquishMagickMemory(Q);
      cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
      sum=(ChannelStatistics *) RelinquishMagickMemory(sum);
      density_y=(ChannelStatistics *) RelinquishMagickMemory(density_y);
      density_xy=(ChannelStatistics *) RelinquishMagickMemory(density_xy);
      density_x=(ChannelStatistics *) RelinquishMagickMemory(density_x);
      grays=(PixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  /*
    Initialize spatial dependence matrix.
  */
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  for (r=0; r < (ssize_t) image->rows; r++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    ssize_t
      offset,
      u,
      v;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-(ssize_t) distance,r,image->columns+
      2*distance,distance+2,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    p+=distance*GetPixelChannels(image);;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      for (i=0; i < 4; i++)
      {
        switch (i)
        {
          case 0:
          default:
          {
            /*
              Horizontal adjacency.
            */
            offset=(ssize_t) distance;
            break;
          }
          case 1:
          {
            /*
              Vertical adjacency.
            */
            offset=(ssize_t) (image->columns+2*distance);
            break;
          }
          case 2:
          {
            /*
              Right diagonal adjacency.
            */
            offset=(ssize_t) ((image->columns+2*distance)-distance);
            break;
          }
          case 3:
          {
            /*
              Left diagonal adjacency.
            */
            offset=(ssize_t) ((image->columns+2*distance)+distance);
            break;
          }
        }
        u=0;
        v=0;
        while (grays[u].red != ScaleQuantumToMap(GetPixelRed(image,p)))
          u++;
        while (grays[v].red != ScaleQuantumToMap(GetPixelRed(image,p+offset*GetPixelChannels(image))))
          v++;
        cooccurrence[u][v].direction[i].red++;
        cooccurrence[v][u].direction[i].red++;
        u=0;
        v=0;
        while (grays[u].green != ScaleQuantumToMap(GetPixelGreen(image,p)))
          u++;
        while (grays[v].green != ScaleQuantumToMap(GetPixelGreen(image,p+offset*GetPixelChannels(image))))
          v++;
        cooccurrence[u][v].direction[i].green++;
        cooccurrence[v][u].direction[i].green++;
        u=0;
        v=0;
        while (grays[u].blue != ScaleQuantumToMap(GetPixelBlue(image,p)))
          u++;
        while (grays[v].blue != ScaleQuantumToMap(GetPixelBlue(image,p+offset*GetPixelChannels(image))))
          v++;
        cooccurrence[u][v].direction[i].blue++;
        cooccurrence[v][u].direction[i].blue++;
        if (image->colorspace == CMYKColorspace)
          {
            u=0;
            v=0;
            while (grays[u].black != ScaleQuantumToMap(GetPixelBlack(image,p)))
              u++;
            while (grays[v].black != ScaleQuantumToMap(GetPixelBlack(image,p+offset*GetPixelChannels(image))))
              v++;
            cooccurrence[u][v].direction[i].black++;
            cooccurrence[v][u].direction[i].black++;
          }
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            u=0;
            v=0;
            while (grays[u].alpha != ScaleQuantumToMap(GetPixelAlpha(image,p)))
              u++;
            while (grays[v].alpha != ScaleQuantumToMap(GetPixelAlpha(image,p+offset*GetPixelChannels(image))))
              v++;
            cooccurrence[u][v].direction[i].alpha++;
            cooccurrence[v][u].direction[i].alpha++;
          }
      }
      p+=GetPixelChannels(image);
    }
  }
  grays=(PixelPacket *) RelinquishMagickMemory(grays);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      for (i=0; i < (ssize_t) number_grays; i++)
        cooccurrence[i]=(ChannelStatistics *)
          RelinquishMagickMemory(cooccurrence[i]);
      cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  /*
    Normalize spatial dependence matrix.
  */
  for (i=0; i < 4; i++)
  {
    double
      normalize;

    register ssize_t
      y;

    switch (i)
    {
      case 0:
      default:
      {
        /*
          Horizontal adjacency.
        */
        normalize=2.0*image->rows*(image->columns-distance);
        break;
      }
      case 1:
      {
        /*
          Vertical adjacency.
        */
        normalize=2.0*(image->rows-distance)*image->columns;
        break;
      }
      case 2:
      {
        /*
          Right diagonal adjacency.
        */
        normalize=2.0*(image->rows-distance)*(image->columns-distance);
        break;
      }
      case 3:
      {
        /*
          Left diagonal adjacency.
        */
        normalize=2.0*(image->rows-distance)*(image->columns-distance);
        break;
      }
    }
    normalize=PerceptibleReciprocal(normalize);
    for (y=0; y < (ssize_t) number_grays; y++)
    {
      register ssize_t
        x;

      for (x=0; x < (ssize_t) number_grays; x++)
      {
        cooccurrence[x][y].direction[i].red*=normalize;
        cooccurrence[x][y].direction[i].green*=normalize;
        cooccurrence[x][y].direction[i].blue*=normalize;
        if (image->colorspace == CMYKColorspace)
          cooccurrence[x][y].direction[i].black*=normalize;
        if (image->alpha_trait != UndefinedPixelTrait)
          cooccurrence[x][y].direction[i].alpha*=normalize;
      }
    }
  }
  /*
    Compute texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      y;

    for (y=0; y < (ssize_t) number_grays; y++)
    {
      register ssize_t
        x;

      for (x=0; x < (ssize_t) number_grays; x++)
      {
        /*
          Angular second moment:  measure of homogeneity of the image.
        */
        channel_features[RedPixelChannel].angular_second_moment[i]+=
          cooccurrence[x][y].direction[i].red*
          cooccurrence[x][y].direction[i].red;
        channel_features[GreenPixelChannel].angular_second_moment[i]+=
          cooccurrence[x][y].direction[i].green*
          cooccurrence[x][y].direction[i].green;
        channel_features[BluePixelChannel].angular_second_moment[i]+=
          cooccurrence[x][y].direction[i].blue*
          cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          channel_features[BlackPixelChannel].angular_second_moment[i]+=
            cooccurrence[x][y].direction[i].black*
            cooccurrence[x][y].direction[i].black;
        if (image->alpha_trait != UndefinedPixelTrait)
          channel_features[AlphaPixelChannel].angular_second_moment[i]+=
            cooccurrence[x][y].direction[i].alpha*
            cooccurrence[x][y].direction[i].alpha;
        /*
          Correlation: measure of linear-dependencies in the image.
        */
        sum[y].direction[i].red+=cooccurrence[x][y].direction[i].red;
        sum[y].direction[i].green+=cooccurrence[x][y].direction[i].green;
        sum[y].direction[i].blue+=cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          sum[y].direction[i].black+=cooccurrence[x][y].direction[i].black;
        if (image->alpha_trait != UndefinedPixelTrait)
          sum[y].direction[i].alpha+=cooccurrence[x][y].direction[i].alpha;
        correlation.direction[i].red+=x*y*cooccurrence[x][y].direction[i].red;
        correlation.direction[i].green+=x*y*
          cooccurrence[x][y].direction[i].green;
        correlation.direction[i].blue+=x*y*
          cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          correlation.direction[i].black+=x*y*
            cooccurrence[x][y].direction[i].black;
        if (image->alpha_trait != UndefinedPixelTrait)
          correlation.direction[i].alpha+=x*y*
            cooccurrence[x][y].direction[i].alpha;
        /*
          Inverse Difference Moment.
        */
        channel_features[RedPixelChannel].inverse_difference_moment[i]+=
          cooccurrence[x][y].direction[i].red/((y-x)*(y-x)+1);
        channel_features[GreenPixelChannel].inverse_difference_moment[i]+=
          cooccurrence[x][y].direction[i].green/((y-x)*(y-x)+1);
        channel_features[BluePixelChannel].inverse_difference_moment[i]+=
          cooccurrence[x][y].direction[i].blue/((y-x)*(y-x)+1);
        if (image->colorspace == CMYKColorspace)
          channel_features[BlackPixelChannel].inverse_difference_moment[i]+=
            cooccurrence[x][y].direction[i].black/((y-x)*(y-x)+1);
        if (image->alpha_trait != UndefinedPixelTrait)
          channel_features[AlphaPixelChannel].inverse_difference_moment[i]+=
            cooccurrence[x][y].direction[i].alpha/((y-x)*(y-x)+1);
        /*
          Sum average.
        */
        density_xy[y+x+2].direction[i].red+=
          cooccurrence[x][y].direction[i].red;
        density_xy[y+x+2].direction[i].green+=
          cooccurrence[x][y].direction[i].green;
        density_xy[y+x+2].direction[i].blue+=
          cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          density_xy[y+x+2].direction[i].black+=
            cooccurrence[x][y].direction[i].black;
        if (image->alpha_trait != UndefinedPixelTrait)
          density_xy[y+x+2].direction[i].alpha+=
            cooccurrence[x][y].direction[i].alpha;
        /*
          Entropy.
        */
        channel_features[RedPixelChannel].entropy[i]-=
          cooccurrence[x][y].direction[i].red*
          MagickLog10(cooccurrence[x][y].direction[i].red);
        channel_features[GreenPixelChannel].entropy[i]-=
          cooccurrence[x][y].direction[i].green*
          MagickLog10(cooccurrence[x][y].direction[i].green);
        channel_features[BluePixelChannel].entropy[i]-=
          cooccurrence[x][y].direction[i].blue*
          MagickLog10(cooccurrence[x][y].direction[i].blue);
        if (image->colorspace == CMYKColorspace)
          channel_features[BlackPixelChannel].entropy[i]-=
            cooccurrence[x][y].direction[i].black*
            MagickLog10(cooccurrence[x][y].direction[i].black);
        if (image->alpha_trait != UndefinedPixelTrait)
          channel_features[AlphaPixelChannel].entropy[i]-=
            cooccurrence[x][y].direction[i].alpha*
            MagickLog10(cooccurrence[x][y].direction[i].alpha);
        /*
          Information Measures of Correlation.
        */
        density_x[x].direction[i].red+=cooccurrence[x][y].direction[i].red;
        density_x[x].direction[i].green+=cooccurrence[x][y].direction[i].green;
        density_x[x].direction[i].blue+=cooccurrence[x][y].direction[i].blue;
        if (image->alpha_trait != UndefinedPixelTrait)
          density_x[x].direction[i].alpha+=
            cooccurrence[x][y].direction[i].alpha;
        if (image->colorspace == CMYKColorspace)
          density_x[x].direction[i].black+=
            cooccurrence[x][y].direction[i].black;
        density_y[y].direction[i].red+=cooccurrence[x][y].direction[i].red;
        density_y[y].direction[i].green+=cooccurrence[x][y].direction[i].green;
        density_y[y].direction[i].blue+=cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          density_y[y].direction[i].black+=
            cooccurrence[x][y].direction[i].black;
        if (image->alpha_trait != UndefinedPixelTrait)
          density_y[y].direction[i].alpha+=
            cooccurrence[x][y].direction[i].alpha;
      }
      mean.direction[i].red+=y*sum[y].direction[i].red;
      sum_squares.direction[i].red+=y*y*sum[y].direction[i].red;
      mean.direction[i].green+=y*sum[y].direction[i].green;
      sum_squares.direction[i].green+=y*y*sum[y].direction[i].green;
      mean.direction[i].blue+=y*sum[y].direction[i].blue;
      sum_squares.direction[i].blue+=y*y*sum[y].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        {
          mean.direction[i].black+=y*sum[y].direction[i].black;
          sum_squares.direction[i].black+=y*y*sum[y].direction[i].black;
        }
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          mean.direction[i].alpha+=y*sum[y].direction[i].alpha;
          sum_squares.direction[i].alpha+=y*y*sum[y].direction[i].alpha;
        }
    }
    /*
      Correlation: measure of linear-dependencies in the image.
    */
    channel_features[RedPixelChannel].correlation[i]=
      (correlation.direction[i].red-mean.direction[i].red*
      mean.direction[i].red)/(sqrt(sum_squares.direction[i].red-
      (mean.direction[i].red*mean.direction[i].red))*sqrt(
      sum_squares.direction[i].red-(mean.direction[i].red*
      mean.direction[i].red)));
    channel_features[GreenPixelChannel].correlation[i]=
      (correlation.direction[i].green-mean.direction[i].green*
      mean.direction[i].green)/(sqrt(sum_squares.direction[i].green-
      (mean.direction[i].green*mean.direction[i].green))*sqrt(
      sum_squares.direction[i].green-(mean.direction[i].green*
      mean.direction[i].green)));
    channel_features[BluePixelChannel].correlation[i]=
      (correlation.direction[i].blue-mean.direction[i].blue*
      mean.direction[i].blue)/(sqrt(sum_squares.direction[i].blue-
      (mean.direction[i].blue*mean.direction[i].blue))*sqrt(
      sum_squares.direction[i].blue-(mean.direction[i].blue*
      mean.direction[i].blue)));
    if (image->colorspace == CMYKColorspace)
      channel_features[BlackPixelChannel].correlation[i]=
        (correlation.direction[i].black-mean.direction[i].black*
        mean.direction[i].black)/(sqrt(sum_squares.direction[i].black-
        (mean.direction[i].black*mean.direction[i].black))*sqrt(
        sum_squares.direction[i].black-(mean.direction[i].black*
        mean.direction[i].black)));
    if (image->alpha_trait != UndefinedPixelTrait)
      channel_features[AlphaPixelChannel].correlation[i]=
        (correlation.direction[i].alpha-mean.direction[i].alpha*
        mean.direction[i].alpha)/(sqrt(sum_squares.direction[i].alpha-
        (mean.direction[i].alpha*mean.direction[i].alpha))*sqrt(
        sum_squares.direction[i].alpha-(mean.direction[i].alpha*
        mean.direction[i].alpha)));
  }
  /*
    Compute more texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      x;

    for (x=2; x < (ssize_t) (2*number_grays); x++)
    {
      /*
        Sum average.
      */
      channel_features[RedPixelChannel].sum_average[i]+=
        x*density_xy[x].direction[i].red;
      channel_features[GreenPixelChannel].sum_average[i]+=
        x*density_xy[x].direction[i].green;
      channel_features[BluePixelChannel].sum_average[i]+=
        x*density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackPixelChannel].sum_average[i]+=
          x*density_xy[x].direction[i].black;
      if (image->alpha_trait != UndefinedPixelTrait)
        channel_features[AlphaPixelChannel].sum_average[i]+=
          x*density_xy[x].direction[i].alpha;
      /*
        Sum entropy.
      */
      channel_features[RedPixelChannel].sum_entropy[i]-=
        density_xy[x].direction[i].red*
        MagickLog10(density_xy[x].direction[i].red);
      channel_features[GreenPixelChannel].sum_entropy[i]-=
        density_xy[x].direction[i].green*
        MagickLog10(density_xy[x].direction[i].green);
      channel_features[BluePixelChannel].sum_entropy[i]-=
        density_xy[x].direction[i].blue*
        MagickLog10(density_xy[x].direction[i].blue);
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackPixelChannel].sum_entropy[i]-=
          density_xy[x].direction[i].black*
          MagickLog10(density_xy[x].direction[i].black);
      if (image->alpha_trait != UndefinedPixelTrait)
        channel_features[AlphaPixelChannel].sum_entropy[i]-=
          density_xy[x].direction[i].alpha*
          MagickLog10(density_xy[x].direction[i].alpha);
      /*
        Sum variance.
      */
      channel_features[RedPixelChannel].sum_variance[i]+=
        (x-channel_features[RedPixelChannel].sum_entropy[i])*
        (x-channel_features[RedPixelChannel].sum_entropy[i])*
        density_xy[x].direction[i].red;
      channel_features[GreenPixelChannel].sum_variance[i]+=
        (x-channel_features[GreenPixelChannel].sum_entropy[i])*
        (x-channel_features[GreenPixelChannel].sum_entropy[i])*
        density_xy[x].direction[i].green;
      channel_features[BluePixelChannel].sum_variance[i]+=
        (x-channel_features[BluePixelChannel].sum_entropy[i])*
        (x-channel_features[BluePixelChannel].sum_entropy[i])*
        density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackPixelChannel].sum_variance[i]+=
          (x-channel_features[BlackPixelChannel].sum_entropy[i])*
          (x-channel_features[BlackPixelChannel].sum_entropy[i])*
          density_xy[x].direction[i].black;
      if (image->alpha_trait != UndefinedPixelTrait)
        channel_features[AlphaPixelChannel].sum_variance[i]+=
          (x-channel_features[AlphaPixelChannel].sum_entropy[i])*
          (x-channel_features[AlphaPixelChannel].sum_entropy[i])*
          density_xy[x].direction[i].alpha;
    }
  }
  /*
    Compute more texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      y;

    for (y=0; y < (ssize_t) number_grays; y++)
    {
      register ssize_t
        x;

      for (x=0; x < (ssize_t) number_grays; x++)
      {
        /*
          Sum of Squares: Variance
        */
        variance.direction[i].red+=(y-mean.direction[i].red+1)*
          (y-mean.direction[i].red+1)*cooccurrence[x][y].direction[i].red;
        variance.direction[i].green+=(y-mean.direction[i].green+1)*
          (y-mean.direction[i].green+1)*cooccurrence[x][y].direction[i].green;
        variance.direction[i].blue+=(y-mean.direction[i].blue+1)*
          (y-mean.direction[i].blue+1)*cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          variance.direction[i].black+=(y-mean.direction[i].black+1)*
            (y-mean.direction[i].black+1)*cooccurrence[x][y].direction[i].black;
        if (image->alpha_trait != UndefinedPixelTrait)
          variance.direction[i].alpha+=(y-mean.direction[i].alpha+1)*
            (y-mean.direction[i].alpha+1)*
            cooccurrence[x][y].direction[i].alpha;
        /*
          Sum average / Difference Variance.
        */
        density_xy[MagickAbsoluteValue(y-x)].direction[i].red+=
          cooccurrence[x][y].direction[i].red;
        density_xy[MagickAbsoluteValue(y-x)].direction[i].green+=
          cooccurrence[x][y].direction[i].green;
        density_xy[MagickAbsoluteValue(y-x)].direction[i].blue+=
          cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          density_xy[MagickAbsoluteValue(y-x)].direction[i].black+=
            cooccurrence[x][y].direction[i].black;
        if (image->alpha_trait != UndefinedPixelTrait)
          density_xy[MagickAbsoluteValue(y-x)].direction[i].alpha+=
            cooccurrence[x][y].direction[i].alpha;
        /*
          Information Measures of Correlation.
        */
        entropy_xy.direction[i].red-=cooccurrence[x][y].direction[i].red*
          MagickLog10(cooccurrence[x][y].direction[i].red);
        entropy_xy.direction[i].green-=cooccurrence[x][y].direction[i].green*
          MagickLog10(cooccurrence[x][y].direction[i].green);
        entropy_xy.direction[i].blue-=cooccurrence[x][y].direction[i].blue*
          MagickLog10(cooccurrence[x][y].direction[i].blue);
        if (image->colorspace == CMYKColorspace)
          entropy_xy.direction[i].black-=cooccurrence[x][y].direction[i].black*
            MagickLog10(cooccurrence[x][y].direction[i].black);
        if (image->alpha_trait != UndefinedPixelTrait)
          entropy_xy.direction[i].alpha-=
            cooccurrence[x][y].direction[i].alpha*MagickLog10(
            cooccurrence[x][y].direction[i].alpha);
        entropy_xy1.direction[i].red-=(cooccurrence[x][y].direction[i].red*
          MagickLog10(density_x[x].direction[i].red*density_y[y].direction[i].red));
        entropy_xy1.direction[i].green-=(cooccurrence[x][y].direction[i].green*
          MagickLog10(density_x[x].direction[i].green*
          density_y[y].direction[i].green));
        entropy_xy1.direction[i].blue-=(cooccurrence[x][y].direction[i].blue*
          MagickLog10(density_x[x].direction[i].blue*density_y[y].direction[i].blue));
        if (image->colorspace == CMYKColorspace)
          entropy_xy1.direction[i].black-=(
            cooccurrence[x][y].direction[i].black*MagickLog10(
            density_x[x].direction[i].black*density_y[y].direction[i].black));
        if (image->alpha_trait != UndefinedPixelTrait)
          entropy_xy1.direction[i].alpha-=(
            cooccurrence[x][y].direction[i].alpha*MagickLog10(
            density_x[x].direction[i].alpha*density_y[y].direction[i].alpha));
        entropy_xy2.direction[i].red-=(density_x[x].direction[i].red*
          density_y[y].direction[i].red*MagickLog10(density_x[x].direction[i].red*
          density_y[y].direction[i].red));
        entropy_xy2.direction[i].green-=(density_x[x].direction[i].green*
          density_y[y].direction[i].green*MagickLog10(density_x[x].direction[i].green*
          density_y[y].direction[i].green));
        entropy_xy2.direction[i].blue-=(density_x[x].direction[i].blue*
          density_y[y].direction[i].blue*MagickLog10(density_x[x].direction[i].blue*
          density_y[y].direction[i].blue));
        if (image->colorspace == CMYKColorspace)
          entropy_xy2.direction[i].black-=(density_x[x].direction[i].black*
            density_y[y].direction[i].black*MagickLog10(
            density_x[x].direction[i].black*density_y[y].direction[i].black));
        if (image->alpha_trait != UndefinedPixelTrait)
          entropy_xy2.direction[i].alpha-=(density_x[x].direction[i].alpha*
            density_y[y].direction[i].alpha*MagickLog10(
            density_x[x].direction[i].alpha*density_y[y].direction[i].alpha));
      }
    }
    channel_features[RedPixelChannel].variance_sum_of_squares[i]=
      variance.direction[i].red;
    channel_features[GreenPixelChannel].variance_sum_of_squares[i]=
      variance.direction[i].green;
    channel_features[BluePixelChannel].variance_sum_of_squares[i]=
      variance.direction[i].blue;
    if (image->colorspace == CMYKColorspace)
      channel_features[BlackPixelChannel].variance_sum_of_squares[i]=
        variance.direction[i].black;
    if (image->alpha_trait != UndefinedPixelTrait)
      channel_features[AlphaPixelChannel].variance_sum_of_squares[i]=
        variance.direction[i].alpha;
  }
  /*
    Compute more texture features.
  */
  (void) ResetMagickMemory(&variance,0,sizeof(variance));
  (void) ResetMagickMemory(&sum_squares,0,sizeof(sum_squares));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      x;

    for (x=0; x < (ssize_t) number_grays; x++)
    {
      /*
        Difference variance.
      */
      variance.direction[i].red+=density_xy[x].direction[i].red;
      variance.direction[i].green+=density_xy[x].direction[i].green;
      variance.direction[i].blue+=density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        variance.direction[i].black+=density_xy[x].direction[i].black;
      if (image->alpha_trait != UndefinedPixelTrait)
        variance.direction[i].alpha+=density_xy[x].direction[i].alpha;
      sum_squares.direction[i].red+=density_xy[x].direction[i].red*
        density_xy[x].direction[i].red;
      sum_squares.direction[i].green+=density_xy[x].direction[i].green*
        density_xy[x].direction[i].green;
      sum_squares.direction[i].blue+=density_xy[x].direction[i].blue*
        density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        sum_squares.direction[i].black+=density_xy[x].direction[i].black*
          density_xy[x].direction[i].black;
      if (image->alpha_trait != UndefinedPixelTrait)
        sum_squares.direction[i].alpha+=density_xy[x].direction[i].alpha*
          density_xy[x].direction[i].alpha;
      /*
        Difference entropy.
      */
      channel_features[RedPixelChannel].difference_entropy[i]-=
        density_xy[x].direction[i].red*
        MagickLog10(density_xy[x].direction[i].red);
      channel_features[GreenPixelChannel].difference_entropy[i]-=
        density_xy[x].direction[i].green*
        MagickLog10(density_xy[x].direction[i].green);
      channel_features[BluePixelChannel].difference_entropy[i]-=
        density_xy[x].direction[i].blue*
        MagickLog10(density_xy[x].direction[i].blue);
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackPixelChannel].difference_entropy[i]-=
          density_xy[x].direction[i].black*
          MagickLog10(density_xy[x].direction[i].black);
      if (image->alpha_trait != UndefinedPixelTrait)
        channel_features[AlphaPixelChannel].difference_entropy[i]-=
          density_xy[x].direction[i].alpha*
          MagickLog10(density_xy[x].direction[i].alpha);
      /*
        Information Measures of Correlation.
      */
      entropy_x.direction[i].red-=(density_x[x].direction[i].red*
        MagickLog10(density_x[x].direction[i].red));
      entropy_x.direction[i].green-=(density_x[x].direction[i].green*
        MagickLog10(density_x[x].direction[i].green));
      entropy_x.direction[i].blue-=(density_x[x].direction[i].blue*
        MagickLog10(density_x[x].direction[i].blue));
      if (image->colorspace == CMYKColorspace)
        entropy_x.direction[i].black-=(density_x[x].direction[i].black*
          MagickLog10(density_x[x].direction[i].black));
      if (image->alpha_trait != UndefinedPixelTrait)
        entropy_x.direction[i].alpha-=(density_x[x].direction[i].alpha*
          MagickLog10(density_x[x].direction[i].alpha));
      entropy_y.direction[i].red-=(density_y[x].direction[i].red*
        MagickLog10(density_y[x].direction[i].red));
      entropy_y.direction[i].green-=(density_y[x].direction[i].green*
        MagickLog10(density_y[x].direction[i].green));
      entropy_y.direction[i].blue-=(density_y[x].direction[i].blue*
        MagickLog10(density_y[x].direction[i].blue));
      if (image->colorspace == CMYKColorspace)
        entropy_y.direction[i].black-=(density_y[x].direction[i].black*
          MagickLog10(density_y[x].direction[i].black));
      if (image->alpha_trait != UndefinedPixelTrait)
        entropy_y.direction[i].alpha-=(density_y[x].direction[i].alpha*
          MagickLog10(density_y[x].direction[i].alpha));
    }
    /*
      Difference variance.
    */
    channel_features[RedPixelChannel].difference_variance[i]=
      (((double) number_grays*number_grays*sum_squares.direction[i].red)-
      (variance.direction[i].red*variance.direction[i].red))/
      ((double) number_grays*number_grays*number_grays*number_grays);
    channel_features[GreenPixelChannel].difference_variance[i]=
      (((double) number_grays*number_grays*sum_squares.direction[i].green)-
      (variance.direction[i].green*variance.direction[i].green))/
      ((double) number_grays*number_grays*number_grays*number_grays);
    channel_features[BluePixelChannel].difference_variance[i]=
      (((double) number_grays*number_grays*sum_squares.direction[i].blue)-
      (variance.direction[i].blue*variance.direction[i].blue))/
      ((double) number_grays*number_grays*number_grays*number_grays);
    if (image->colorspace == CMYKColorspace)
      channel_features[BlackPixelChannel].difference_variance[i]=
        (((double) number_grays*number_grays*sum_squares.direction[i].black)-
        (variance.direction[i].black*variance.direction[i].black))/
        ((double) number_grays*number_grays*number_grays*number_grays);
    if (image->alpha_trait != UndefinedPixelTrait)
      channel_features[AlphaPixelChannel].difference_variance[i]=
        (((double) number_grays*number_grays*sum_squares.direction[i].alpha)-
        (variance.direction[i].alpha*variance.direction[i].alpha))/
        ((double) number_grays*number_grays*number_grays*number_grays);
    /*
      Information Measures of Correlation.
    */
    channel_features[RedPixelChannel].measure_of_correlation_1[i]=
      (entropy_xy.direction[i].red-entropy_xy1.direction[i].red)/
      (entropy_x.direction[i].red > entropy_y.direction[i].red ?
       entropy_x.direction[i].red : entropy_y.direction[i].red);
    channel_features[GreenPixelChannel].measure_of_correlation_1[i]=
      (entropy_xy.direction[i].green-entropy_xy1.direction[i].green)/
      (entropy_x.direction[i].green > entropy_y.direction[i].green ?
       entropy_x.direction[i].green : entropy_y.direction[i].green);
    channel_features[BluePixelChannel].measure_of_correlation_1[i]=
      (entropy_xy.direction[i].blue-entropy_xy1.direction[i].blue)/
      (entropy_x.direction[i].blue > entropy_y.direction[i].blue ?
       entropy_x.direction[i].blue : entropy_y.direction[i].blue);
    if (image->colorspace == CMYKColorspace)
      channel_features[BlackPixelChannel].measure_of_correlation_1[i]=
        (entropy_xy.direction[i].black-entropy_xy1.direction[i].black)/
        (entropy_x.direction[i].black > entropy_y.direction[i].black ?
         entropy_x.direction[i].black : entropy_y.direction[i].black);
    if (image->alpha_trait != UndefinedPixelTrait)
      channel_features[AlphaPixelChannel].measure_of_correlation_1[i]=
        (entropy_xy.direction[i].alpha-entropy_xy1.direction[i].alpha)/
        (entropy_x.direction[i].alpha > entropy_y.direction[i].alpha ?
         entropy_x.direction[i].alpha : entropy_y.direction[i].alpha);
    channel_features[RedPixelChannel].measure_of_correlation_2[i]=
      (sqrt(fabs(1.0-exp(-2.0*(double) (entropy_xy2.direction[i].red-
      entropy_xy.direction[i].red)))));
    channel_features[GreenPixelChannel].measure_of_correlation_2[i]=
      (sqrt(fabs(1.0-exp(-2.0*(double) (entropy_xy2.direction[i].green-
      entropy_xy.direction[i].green)))));
    channel_features[BluePixelChannel].measure_of_correlation_2[i]=
      (sqrt(fabs(1.0-exp(-2.0*(double) (entropy_xy2.direction[i].blue-
      entropy_xy.direction[i].blue)))));
    if (image->colorspace == CMYKColorspace)
      channel_features[BlackPixelChannel].measure_of_correlation_2[i]=
        (sqrt(fabs(1.0-exp(-2.0*(double) (entropy_xy2.direction[i].black-
        entropy_xy.direction[i].black)))));
    if (image->alpha_trait != UndefinedPixelTrait)
      channel_features[AlphaPixelChannel].measure_of_correlation_2[i]=
        (sqrt(fabs(1.0-exp(-2.0*(double) (entropy_xy2.direction[i].alpha-
        entropy_xy.direction[i].alpha)))));
  }
  /*
    Compute more texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    ssize_t
      z;

    for (z=0; z < (ssize_t) number_grays; z++)
    {
      register ssize_t
        y;

      ChannelStatistics
        pixel;

      (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
      for (y=0; y < (ssize_t) number_grays; y++)
      {
        register ssize_t
          x;

        for (x=0; x < (ssize_t) number_grays; x++)
        {
          /*
            Contrast:  amount of local variations present in an image.
          */
          if (((y-x) == z) || ((x-y) == z))
            {
              pixel.direction[i].red+=cooccurrence[x][y].direction[i].red;
              pixel.direction[i].green+=cooccurrence[x][y].direction[i].green;
              pixel.direction[i].blue+=cooccurrence[x][y].direction[i].blue;
              if (image->colorspace == CMYKColorspace)
                pixel.direction[i].black+=cooccurrence[x][y].direction[i].black;
              if (image->alpha_trait != UndefinedPixelTrait)
                pixel.direction[i].alpha+=
                  cooccurrence[x][y].direction[i].alpha;
            }
          /*
            Maximum Correlation Coefficient.
          */
          Q[z][y].direction[i].red+=cooccurrence[z][x].direction[i].red*
            cooccurrence[y][x].direction[i].red/density_x[z].direction[i].red/
            density_y[x].direction[i].red;
          Q[z][y].direction[i].green+=cooccurrence[z][x].direction[i].green*
            cooccurrence[y][x].direction[i].green/
            density_x[z].direction[i].green/density_y[x].direction[i].red;
          Q[z][y].direction[i].blue+=cooccurrence[z][x].direction[i].blue*
            cooccurrence[y][x].direction[i].blue/density_x[z].direction[i].blue/
            density_y[x].direction[i].blue;
          if (image->colorspace == CMYKColorspace)
            Q[z][y].direction[i].black+=cooccurrence[z][x].direction[i].black*
              cooccurrence[y][x].direction[i].black/
              density_x[z].direction[i].black/density_y[x].direction[i].black;
          if (image->alpha_trait != UndefinedPixelTrait)
            Q[z][y].direction[i].alpha+=
              cooccurrence[z][x].direction[i].alpha*
              cooccurrence[y][x].direction[i].alpha/
              density_x[z].direction[i].alpha/
              density_y[x].direction[i].alpha;
        }
      }
      channel_features[RedPixelChannel].contrast[i]+=z*z*
        pixel.direction[i].red;
      channel_features[GreenPixelChannel].contrast[i]+=z*z*
        pixel.direction[i].green;
      channel_features[BluePixelChannel].contrast[i]+=z*z*
        pixel.direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackPixelChannel].contrast[i]+=z*z*
          pixel.direction[i].black;
      if (image->alpha_trait != UndefinedPixelTrait)
        channel_features[AlphaPixelChannel].contrast[i]+=z*z*
          pixel.direction[i].alpha;
    }
    /*
      Maximum Correlation Coefficient.
      Future: return second largest eigenvalue of Q.
    */
    channel_features[RedPixelChannel].maximum_correlation_coefficient[i]=
      sqrt((double) -1.0);
    channel_features[GreenPixelChannel].maximum_correlation_coefficient[i]=
      sqrt((double) -1.0);
    channel_features[BluePixelChannel].maximum_correlation_coefficient[i]=
      sqrt((double) -1.0);
    if (image->colorspace == CMYKColorspace)
      channel_features[BlackPixelChannel].maximum_correlation_coefficient[i]=
        sqrt((double) -1.0);
    if (image->alpha_trait != UndefinedPixelTrait)
      channel_features[AlphaPixelChannel].maximum_correlation_coefficient[i]=
        sqrt((double) -1.0);
  }
  /*
    Relinquish resources.
  */
  sum=(ChannelStatistics *) RelinquishMagickMemory(sum);
  for (i=0; i < (ssize_t) number_grays; i++)
    Q[i]=(ChannelStatistics *) RelinquishMagickMemory(Q[i]);
  Q=(ChannelStatistics **) RelinquishMagickMemory(Q);
  density_y=(ChannelStatistics *) RelinquishMagickMemory(density_y);
  density_xy=(ChannelStatistics *) RelinquishMagickMemory(density_xy);
  density_x=(ChannelStatistics *) RelinquishMagickMemory(density_x);
  for (i=0; i < (ssize_t) number_grays; i++)
    cooccurrence[i]=(ChannelStatistics *)
      RelinquishMagickMemory(cooccurrence[i]);
  cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
  return(channel_features);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     H o u g h L i n e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Use HoughLineImage() in conjunction with any binary edge extracted image (we
%  recommand Canny) to identify lines in the image.  The algorithm accumulates
%  counts for every white pixel for every possible orientation (for angles from
%  0 to 179 in 1 degree increments) and distance from the center of the image to
%  the corner (in 1 px increments) and stores the counts in an accumulator matrix
%  of angle vs distance. The size of the accumulator is 180x(diagonal/2). Next
%  it searches this space for peaks in counts and converts the locations of the
%  peaks to slope and intercept in the normal x,y input image space. Use the
%  slope/intercepts to find the endpoints clipped to the bounds of the image. The
%  lines are then drawn. The counts are a measure of the length of the lines
%
%  The format of the HoughLineImage method is:
%
%      Image *HoughLineImage(const Image *image,const size_t width,
%        const size_t height,const size_t threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width, height: find line pairs as local maxima in this neighborhood.
%
%    o threshold: the line count threshold.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickRound(double x)
{
  /*
    Round the fraction to nearest integer.
  */
  if ((x-floor(x)) < (ceil(x)-x))
    return(floor(x));
  return(ceil(x));
}

static Image *RenderHoughLines(const ImageInfo *image_info,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
{
#define BoundingBox  "viewbox"

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  /*
    Open image.
  */
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  image->columns=columns;
  image->rows=rows;
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  draw_info->affine.sx=image->resolution.x == 0.0 ? 1.0 : image->resolution.x/
    DefaultResolution;
  draw_info->affine.sy=image->resolution.y == 0.0 ? 1.0 : image->resolution.y/
    DefaultResolution;
  image->columns=(size_t) (draw_info->affine.sx*image->columns);
  image->rows=(size_t) (draw_info->affine.sy*image->rows);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Render drawing.
  */
  if (GetBlobStreamData(image) == (unsigned char *) NULL)
    draw_info->primitive=FileToString(image->filename,~0UL,exception);
  else
    {
      draw_info->primitive=(char *) AcquireMagickMemory((size_t)
        GetBlobSize(image)+1);
      if (draw_info->primitive != (char *) NULL)
        {
          (void) CopyMagickMemory(draw_info->primitive,GetBlobStreamData(image),
            (size_t) GetBlobSize(image));
          draw_info->primitive[GetBlobSize(image)]='\0';
        }
     }
  (void) DrawImage(image,draw_info,exception);
  draw_info=DestroyDrawInfo(draw_info);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

MagickExport Image *HoughLineImage(const Image *image,const size_t width,
  const size_t height,const size_t threshold,ExceptionInfo *exception)
{
#define HoughLineImageTag  "HoughLine/Image"

  CacheView
    *image_view;

  char
    message[MagickPathExtent],
    path[MagickPathExtent];

  const char
    *artifact;

  double
    hough_height;

  Image
    *lines_image = NULL;

  ImageInfo
    *image_info;

  int
    file;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MatrixInfo
    *accumulator;

  PointInfo
    center;

  register ssize_t
    y;

  size_t
    accumulator_height,
    accumulator_width,
    line_count;

  /*
    Create the accumulator.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  accumulator_width=180;
  hough_height=((sqrt(2.0)*(double) (image->rows > image->columns ?
    image->rows : image->columns))/2.0);
  accumulator_height=(size_t) (2.0*hough_height);
  accumulator=AcquireMatrixInfo(accumulator_width,accumulator_height,
    sizeof(double),exception);
  if (accumulator == (MatrixInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  if (NullMatrix(accumulator) == MagickFalse)
    {
      accumulator=DestroyMatrixInfo(accumulator);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Populate the accumulator.
  */
  status=MagickTrue;
  progress=0;
  center.x=(double) image->columns/2.0;
  center.y=(double) image->rows/2.0;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelIntensity(image,p) > (QuantumRange/2.0))
        {
          register ssize_t
            i;

          for (i=0; i < 180; i++)
          {
            double
              count,
              radius;

            radius=(((double) x-center.x)*cos(DegreesToRadians((double) i)))+
              (((double) y-center.y)*sin(DegreesToRadians((double) i)));
            (void) GetMatrixElement(accumulator,i,(ssize_t)
              MagickRound(radius+hough_height),&count);
            count++;
            (void) SetMatrixElement(accumulator,i,(ssize_t)
              MagickRound(radius+hough_height),&count);
          }
        }
      p+=GetPixelChannels(image);
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_CannyEdgeImage)
#endif
        proceed=SetImageProgress(image,CannyEdgeImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      accumulator=DestroyMatrixInfo(accumulator);
      return((Image *) NULL);
    }
  /*
    Generate line segments from accumulator.
  */
  file=AcquireUniqueFileResource(path);
  if (file == -1)
    {
      accumulator=DestroyMatrixInfo(accumulator);
      return((Image *) NULL);
    }
  (void) FormatLocaleString(message,MagickPathExtent,
    "# Hough line transform: %.20gx%.20g%+.20g\n",(double) width,
    (double) height,(double) threshold);
  if (write(file,message,strlen(message)) != (ssize_t) strlen(message))
    status=MagickFalse;
  (void) FormatLocaleString(message,MagickPathExtent,
    "viewbox 0 0 %.20g %.20g\n",(double) image->columns,(double) image->rows);
  if (write(file,message,strlen(message)) != (ssize_t) strlen(message))
    status=MagickFalse;
  line_count=image->columns > image->rows ? image->columns/4 : image->rows/4;
  if (threshold != 0)
    line_count=threshold;
  for (y=0; y < (ssize_t) accumulator_height; y++)
  {
    register ssize_t
      x;

    for (x=0; x < (ssize_t) accumulator_width; x++)
    {
      double
        count;

      (void) GetMatrixElement(accumulator,x,y,&count);
      if (count >= (double) line_count)
        {
          double
            maxima;

          SegmentInfo
            line;

          ssize_t
            v;

          /*
            Is point a local maxima?
          */
          maxima=count;
          for (v=(-((ssize_t) height/2)); v <= (((ssize_t) height/2)); v++)
          {
            ssize_t
              u;

            for (u=(-((ssize_t) width/2)); u <= (((ssize_t) width/2)); u++)
            {
              if ((u != 0) || (v !=0))
                {
                  (void) GetMatrixElement(accumulator,x+u,y+v,&count);
                  if (count > maxima)
                    {
                      maxima=count;
                      break;
                    }
                }
            }
            if (u < (ssize_t) (width/2))
              break;
          }
          (void) GetMatrixElement(accumulator,x,y,&count);
          if (maxima > count)
            continue;
          if ((x >= 45) && (x <= 135))
            {
              /*
                y = (r-x cos(t))/sin(t)
              */
              line.x1=0.0;
              line.y1=((double) (y-(accumulator_height/2.0))-((line.x1-
                (image->columns/2.0))*cos(DegreesToRadians((double) x))))/
                sin(DegreesToRadians((double) x))+(image->rows/2.0);
              line.x2=(double) image->columns;
              line.y2=((double) (y-(accumulator_height/2.0))-((line.x2-
                (image->columns/2.0))*cos(DegreesToRadians((double) x))))/
                sin(DegreesToRadians((double) x))+(image->rows/2.0);
            }
          else
            {
              /*
                x = (r-y cos(t))/sin(t)
              */
              line.y1=0.0;
              line.x1=((double) (y-(accumulator_height/2.0))-((line.y1-
                (image->rows/2.0))*sin(DegreesToRadians((double) x))))/
                cos(DegreesToRadians((double) x))+(image->columns/2.0);
              line.y2=(double) image->rows;
              line.x2=((double) (y-(accumulator_height/2.0))-((line.y2-
                (image->rows/2.0))*sin(DegreesToRadians((double) x))))/
                cos(DegreesToRadians((double) x))+(image->columns/2.0);
            }
          (void) FormatLocaleString(message,MagickPathExtent,
            "line %g,%g %g,%g  # %g\n",line.x1,line.y1,line.x2,line.y2,maxima);
          if (write(file,message,strlen(message)) != (ssize_t) strlen(message))
            status=MagickFalse;
        }
    }
  }
  (void) close(file);
  /*
    Render lines to image canvas.
  */
  image_info=AcquireImageInfo();
  image_info->background_color=image->background_color;
  (void) FormatLocaleString(image_info->filename,MagickPathExtent,"%s",path);
  artifact=GetImageArtifact(image,"background");
  if (artifact != (const char *) NULL)
    (void) SetImageOption(image_info,"background",artifact);
  artifact=GetImageArtifact(image,"fill");
  if (artifact != (const char *) NULL)
    (void) SetImageOption(image_info,"fill",artifact);
  artifact=GetImageArtifact(image,"stroke");
  if (artifact != (const char *) NULL)
    (void) SetImageOption(image_info,"stroke",artifact);
  artifact=GetImageArtifact(image,"strokewidth");
  if (artifact != (const char *) NULL)
    (void) SetImageOption(image_info,"strokewidth",artifact);
  lines_image=RenderHoughLines(image_info,image->columns,image->rows,exception);
  artifact=GetImageArtifact(image,"hough-lines:accumulator");
  if ((lines_image != (Image *) NULL) &&
      (IsStringTrue(artifact) != MagickFalse))
    {
      Image
        *accumulator_image;

      accumulator_image=MatrixToImage(accumulator,exception);
      if (accumulator_image != (Image *) NULL)
        AppendImageToList(&lines_image,accumulator_image);
    }
  /*
    Free resources.
  */
  accumulator=DestroyMatrixInfo(accumulator);
  image_info=DestroyImageInfo(image_info);
  (void) RelinquishUniqueFileResource(path);
  return(GetFirstImageInList(lines_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M e a n S h i f t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MeanShiftImage() delineate arbitrarily shaped clusters in the image. For
%  each pixel, it visits all the pixels in the neighborhood specified by
%  the window centered at the pixel and excludes those that are outside the
%  radius=(window-1)/2 surrounding the pixel. From those pixels, it finds those
%  that are within the specified color distance from the current mean, and
%  computes a new x,y centroid from those coordinates and a new mean. This new
%  x,y centroid is used as the center for a new window. This process iterates
%  until it converges and the final mean is replaces the (original window
%  center) pixel value. It repeats this process for the next pixel, etc.,
%  until it processes all pixels in the image. Results are typically better with
%  colorspaces other than sRGB. We recommend YIQ, YUV or YCbCr.
%
%  The format of the MeanShiftImage method is:
%
%      Image *MeanShiftImage(const Image *image,const size_t width,
%        const size_t height,const double color_distance,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width, height: find pixels in this neighborhood.
%
%    o color_distance: the color distance.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MeanShiftImage(const Image *image,const size_t width,
  const size_t height,const double color_distance,ExceptionInfo *exception)
{
#define MaxMeanShiftIterations  100
#define MeanShiftImageTag  "MeanShift/Image"

  CacheView
    *image_view,
    *mean_view,
    *pixel_view;

  Image
    *mean_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  mean_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (mean_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(mean_image,DirectClass,exception) == MagickFalse)
    {
      mean_image=DestroyImage(mean_image);
      return((Image *) NULL);
    }
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  pixel_view=AcquireVirtualCacheView(image,exception);
  mean_view=AcquireAuthenticCacheView(mean_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status,progress) \
    magick_threads(mean_image,mean_image,mean_image->rows,1)
#endif
  for (y=0; y < (ssize_t) mean_image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(mean_view,0,y,mean_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) mean_image->columns; x++)
    {
      PixelInfo
        mean_pixel,
        previous_pixel;

      PointInfo
        mean_location,
        previous_location;

      register ssize_t
        i;

      GetPixelInfo(image,&mean_pixel);
      GetPixelInfoPixel(image,p,&mean_pixel);
      mean_location.x=(double) x;
      mean_location.y=(double) y;
      for (i=0; i < MaxMeanShiftIterations; i++)
      {
        double
          distance,
          gamma;

        PixelInfo
          sum_pixel;

        PointInfo
          sum_location;

        ssize_t
          count,
          v;

        sum_location.x=0.0;
        sum_location.y=0.0;
        GetPixelInfo(image,&sum_pixel);
        previous_location=mean_location;
        previous_pixel=mean_pixel;
        count=0;
        for (v=(-((ssize_t) height/2)); v <= (((ssize_t) height/2)); v++)
        {
          ssize_t
            u;

          for (u=(-((ssize_t) width/2)); u <= (((ssize_t) width/2)); u++)
          {
            if ((v*v+u*u) <= (ssize_t) ((width/2)*(height/2)))
              {
                PixelInfo
                  pixel;

                status=GetOneCacheViewVirtualPixelInfo(pixel_view,(ssize_t)
                  MagickRound(mean_location.x+u),(ssize_t) MagickRound(
                  mean_location.y+v),&pixel,exception);
                distance=(mean_pixel.red-pixel.red)*(mean_pixel.red-pixel.red)+
                  (mean_pixel.green-pixel.green)*(mean_pixel.green-pixel.green)+
                  (mean_pixel.blue-pixel.blue)*(mean_pixel.blue-pixel.blue);
                if (distance <= (color_distance*color_distance))
                  {
                    sum_location.x+=mean_location.x+u;
                    sum_location.y+=mean_location.y+v;
                    sum_pixel.red+=pixel.red;
                    sum_pixel.green+=pixel.green;
                    sum_pixel.blue+=pixel.blue;
                    sum_pixel.alpha+=pixel.alpha;
                    count++;
                  }
              }
          }
        }
        gamma=1.0/count;
        mean_location.x=gamma*sum_location.x;
        mean_location.y=gamma*sum_location.y;
        mean_pixel.red=gamma*sum_pixel.red;
        mean_pixel.green=gamma*sum_pixel.green;
        mean_pixel.blue=gamma*sum_pixel.blue;
        mean_pixel.alpha=gamma*sum_pixel.alpha;
        distance=(mean_location.x-previous_location.x)*
          (mean_location.x-previous_location.x)+
          (mean_location.y-previous_location.y)*
          (mean_location.y-previous_location.y)+
          255.0*QuantumScale*(mean_pixel.red-previous_pixel.red)*
          255.0*QuantumScale*(mean_pixel.red-previous_pixel.red)+
          255.0*QuantumScale*(mean_pixel.green-previous_pixel.green)*
          255.0*QuantumScale*(mean_pixel.green-previous_pixel.green)+
          255.0*QuantumScale*(mean_pixel.blue-previous_pixel.blue)*
          255.0*QuantumScale*(mean_pixel.blue-previous_pixel.blue);
        if (distance <= 3.0)
          break;
      }
      SetPixelRed(mean_image,ClampToQuantum(mean_pixel.red),q);
      SetPixelGreen(mean_image,ClampToQuantum(mean_pixel.green),q);
      SetPixelBlue(mean_image,ClampToQuantum(mean_pixel.blue),q);
      SetPixelAlpha(mean_image,ClampToQuantum(mean_pixel.alpha),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(mean_image);
    }
    if (SyncCacheViewAuthenticPixels(mean_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_MeanShiftImage)
#endif
        proceed=SetImageProgress(image,MeanShiftImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  mean_view=DestroyCacheView(mean_view);
  pixel_view=DestroyCacheView(pixel_view);
  image_view=DestroyCacheView(image_view);
  return(mean_image);
}

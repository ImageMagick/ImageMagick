/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               FFFFF   OOO   U   U  RRRR   IIIII  EEEEE  RRRR                %
%               F      O   O  U   U  R   R    I    E      R   R               %
%               FFF    O   O  U   U  RRRR     I    EEE    RRRR                %
%               F      O   O  U   U  R R      I    E      R R                 %
%               F       OOO    UUU   R  R   IIIII  EEEEE  R  R                %
%                                                                             %
%                                                                             %
%                MagickCore Discrete Fourier Transform Methods                %
%                                                                             %
%                              Software Design                                %
%                                Sean Burke                                   %
%                               Fred Weinhaus                                 %
%                                John Cristy                                  %
%                                 July 2009                                   %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/cache.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/fourier.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/property.h"
#include "magick/thread-private.h"
#if defined(MAGICKCORE_FFTW_DELEGATE)
#include <complex.h>
#include <fftw3.h>
#endif

/*
  Typedef declarations.
*/
typedef struct _FourierInfo
{
  ChannelType
    channel;

  MagickBooleanType
    modulus;

  unsigned long
    width,
    height;

  long
    center;
} FourierInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     F o r w a r d F o u r i e r T r a n s f o r m I m a g e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ForwardFourierTransformImage() implements the discrete Fourier transform
%  (DFT) of the image either as a magnitude / phase or real / imaginary image
%  pair.
%
%  The format of the ForwadFourierTransformImage method is:
%
%      Image *ForwardFourierTransformImage(const Image *image,
%        const MagickBooleanType modulus,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o modulus: if true, return as transform as a magnitude / phase pair
%      otherwise a real / imaginary image pair.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_FFTW_DELEGATE)

static MagickBooleanType RollFourier(const unsigned long width,
  const unsigned long height,const long x_offset,const long y_offset,
  double *fourier)
{
  double
    *roll;

  long
    u,
    v,
    y;

  register long
    i,
    x;

  /*
    Move the zero frequency (DC) from (0,0) to (width/2,height/2).
  */
  roll=(double *) AcquireQuantumMemory((size_t) width,height*sizeof(*roll));
  if (roll == (double *) NULL)
    return(MagickFalse);
  i=0L;
  for (y=0L; y < (long) height; y++)
  {
    if (y_offset < 0L)
      v=((y+y_offset) < 0L) ? y+y_offset+(long) height : y+y_offset;
    else
      v=((y+y_offset) > ((long) height-1L)) ? y+y_offset-(long) height :
        y+y_offset;
    for (x=0L; x < (long) width; x++)
    {
      if (x_offset < 0L)
        u=((x+x_offset) < 0L) ? x+x_offset+(long) width : x+x_offset;
      else
        u=((x+x_offset) > ((long) width-1L)) ? x+x_offset-(long) width :
          x+x_offset;
      roll[v*width+u]=fourier[i++];
   }
  }
  (void) CopyMagickMemory(fourier,roll,width*height*sizeof(*roll));
  roll=(double *) RelinquishMagickMemory(roll);
  return(MagickTrue);
}

static MagickBooleanType ForwardQuadrantSwap(const unsigned long width,
  const unsigned long height,double *source,double *destination)
{
  long
    center,
    y;

  MagickBooleanType
    status;

  register long
    x;

  /*
    Swap quadrants.
  */
  center=(long) floor((double) width/2L)+1L;
  status=RollFourier((unsigned long) center,height,0L,(long) height/2L,source);
  if (status == MagickFalse)
    return(MagickFalse);
  for (y=0L; y < (long) height; y++)
    for (x=0L; x < (long) (width/2L-1L); x++)
      destination[width*y+x+width/2L]=source[center*y+x];
  for (y=1; y < (long) height; y++)
    for (x=0L; x < (long) (width/2L-1L); x++)
      destination[width*(height-y)+width/2L-x-1L]=source[center*y+x+1L];
  for (x=0L; x < (long) (width/2L); x++)
    destination[-x+width/2L-1L]=destination[x+width/2L+1L];
  return(MagickTrue);
}

static void CorrectPhaseLHS(const unsigned long width,
  const unsigned long height,double *fourier)
{
  long
    y;

  register long
    x;

  for (y=0L; y < (long) height; y++)
    for (x=0L; x < (long) (width/2L); x++)
      fourier[y*width+x]*=(-1.0);
}

static MagickBooleanType ForwardFourier(const FourierInfo *fourier_info,
  Image *image,double *magnitude,double *phase,ExceptionInfo *exception)
{
  CacheView
    *magnitude_view,
    *phase_view;

  double
    *magnitude_source,
    *phase_source;

  Image
    *magnitude_image,
    *phase_image;

  long
    i,
    y;

  MagickBooleanType
    status;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  magnitude_image=GetFirstImageInList(image);
  phase_image=GetNextImageInList(image);
  if (phase_image == (Image *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ImageSequenceRequired","`%s'",image->filename);
      return(MagickFalse);
    }
  /*
    Create "Fourier Transform" image from constituent arrays.
  */
  magnitude_source=(double *) AcquireQuantumMemory((size_t)
    fourier_info->height,fourier_info->width*sizeof(*magnitude_source));
  if (magnitude_source == (double *) NULL)
    return(MagickFalse);
  (void) ResetMagickMemory(magnitude_source,0,fourier_info->width*
    fourier_info->height*sizeof(*magnitude_source));
  phase_source=(double *) AcquireQuantumMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*phase_source));
  if (magnitude_source == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      magnitude_source=(double *) RelinquishMagickMemory(magnitude_source);
      return(MagickFalse);
    }
  status=ForwardQuadrantSwap(fourier_info->height,fourier_info->height,
    magnitude,magnitude_source);
  if (status != MagickFalse)
    status=ForwardQuadrantSwap(fourier_info->height,fourier_info->height,phase,
      phase_source);
  CorrectPhaseLHS(fourier_info->height,fourier_info->height,phase_source);
  if (fourier_info->modulus != MagickFalse)
    {
      i=0L;
      for (y=0L; y < (long) fourier_info->height; y++)
        for (x=0L; x < (long) fourier_info->width; x++)
        {
          phase_source[i]/=(2.0*MagickPI);
          phase_source[i]+=0.5;
          i++;
        }
    }
  magnitude_view=AcquireCacheView(magnitude_image);
  phase_view=AcquireCacheView(phase_image);
  i=0L;
  for (y=0L; y < (long) fourier_info->height; y++)
  {
    q=GetCacheViewAuthenticPixels(magnitude_view,0L,y,fourier_info->height,1UL,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(magnitude_view);
    for (x=0L; x < (long) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          q->red=ClampToQuantum(QuantumRange*magnitude_source[i]);
          break;
        }
        case GreenChannel:
        {
          q->green=ClampToQuantum(QuantumRange*magnitude_source[i]);
          break;
        }
        case BlueChannel:
        {
          q->blue=ClampToQuantum(QuantumRange*magnitude_source[i]);
          break;
        }
        case OpacityChannel:
        {
          q->opacity=ClampToQuantum(QuantumRange*magnitude_source[i]);
          break;
        }
        case IndexChannel:
        {
          indexes[x]=ClampToQuantum(QuantumRange*magnitude_source[i]);
          break;
        }
        case GrayChannels:
        {
          q->red=ClampToQuantum(QuantumRange*magnitude_source[i]);
          q->green=q->red;
          q->blue=q->red;
          break;
        }
      }
      i++;
      q++;
    }
    status=SyncCacheViewAuthenticPixels(magnitude_view,exception);
    if (status == MagickFalse)
      break;
  }
  i=0L;
  for (y=0L; y < (long) fourier_info->height; y++)
  {
    q=GetCacheViewAuthenticPixels(phase_view,0L,y,fourier_info->height,1UL,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(phase_view);
    for (x=0L; x < (long) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          q->red=ClampToQuantum(QuantumRange*phase_source[i]);
          break;
        }
        case GreenChannel:
        {
          q->green=ClampToQuantum(QuantumRange*phase_source[i]);
          break;
        }
        case BlueChannel:
        {
          q->blue=ClampToQuantum(QuantumRange*phase_source[i]);
          break;
        }
        case OpacityChannel:
        {
          q->opacity=ClampToQuantum(QuantumRange*phase_source[i]);
          break;
        }
        case IndexChannel:
        {
          indexes[x]=ClampToQuantum(QuantumRange*phase_source[i]);
          break;
        }
        case GrayChannels:
        {
          q->red=ClampToQuantum(QuantumRange*phase_source[i]);
          q->green=q->red;
          q->blue=q->red;
          break;
        }
      }
      i++;
      q++;
    }
    status=SyncCacheViewAuthenticPixels(phase_view,exception);
    if (status == MagickFalse)
      break;
   }
  phase_view=DestroyCacheView(phase_view);
  magnitude_view=DestroyCacheView(magnitude_view);
  phase_source=(double *) RelinquishMagickMemory(phase_source);
  magnitude_source=(double *) RelinquishMagickMemory(magnitude_source);
  return(status);
}

static MagickBooleanType ForwardFourierTransform(FourierInfo *fourier_info,
  const Image *image,double *magnitude,double *phase,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  double
    n,
    *source;

  fftw_complex
    *fourier;

  fftw_plan
    fftw_r2c_plan;

  long
    y;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  /*
    Generate the forward Fourier transform.
  */
  source=(double *) AcquireQuantumMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*source));
  if (source == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  ResetMagickMemory(source,0,fourier_info->width*fourier_info->height*
    sizeof(*source));
  i=0L;
  image_view=AcquireCacheView(image);
  for (y=0L; y < (long) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0L,y,fourier_info->width,1UL,
      exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0L; x < (long) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          source[i]=QuantumScale*GetRedPixelComponent(p);
          break;
        }
        case GreenChannel:
        {
          source[i]=QuantumScale*GetGreenPixelComponent(p);
          break;
        }
        case BlueChannel:
        {
          source[i]=QuantumScale*GetBluePixelComponent(p);
          break;
        }
        case OpacityChannel:
        {
          source[i]=QuantumScale*GetOpacityPixelComponent(p);
          break;
        }
        case IndexChannel:
        {
          source[i]=QuantumScale*indexes[x];
          break;
        }
        case GrayChannels:
        {
          source[i]=QuantumScale*GetRedPixelComponent(p);
          break;
        }
      }
      i++;
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  fourier=(fftw_complex *) AcquireAlignedMemory((size_t) fourier_info->height,
    fourier_info->center*sizeof(*fourier));
  if (fourier == (fftw_complex *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      source=(double *) RelinquishMagickMemory(source);
      return(MagickFalse);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ForwardFourierTransform)
#endif
  fftw_r2c_plan=fftw_plan_dft_r2c_2d(fourier_info->width,fourier_info->width,
    source,fourier,FFTW_ESTIMATE);
  fftw_execute(fftw_r2c_plan);
  fftw_destroy_plan(fftw_r2c_plan);
  source=(double *) RelinquishMagickMemory(source);
  /*
    Normalize Fourier transform.
  */
  n=(double) fourier_info->width*(double) fourier_info->width;
  i=0L;
  for (y=0L; y < (long) fourier_info->height; y++)
    for (x=0L; x < (long) fourier_info->center; x++)
      fourier[i++]/=n;
  /*
    Generate magnitude and phase (or real and imaginary).
  */
  i=0L;
  if (fourier_info->modulus != MagickFalse)
    for (y=0L; y < (long) fourier_info->height; y++)
      for (x=0L; x < (long) fourier_info->center; x++)
      {
        magnitude[i]=cabs(fourier[i]);
        phase[i]=carg(fourier[i]);
        i++;
      }
  else
    for (y=0L; y < (long) fourier_info->height; y++)
      for (x=0L; x < (long) fourier_info->center; x++)
      {
        magnitude[i]=creal(fourier[i]);
        phase[i]=cimag(fourier[i]);
        i++;
      }
  fourier=(fftw_complex *) RelinquishAlignedMemory(fourier);
  return(MagickTrue);
}

static MagickBooleanType ForwardFourierTransformChannel(const Image *image,
  const ChannelType channel,const MagickBooleanType modulus,
  Image *fourier_image,ExceptionInfo *exception)
{
  double
    *magnitude,
    *phase;

  fftw_complex
    *fourier;

  MagickBooleanType
    status;

  FourierInfo
    fourier_info;

  size_t
    extent;

  fourier_info.width=image->columns;
  if ((image->columns != image->rows) || ((image->columns % 2) != 0) ||
      ((image->rows % 2) != 0))
    {
      extent=image->columns < image->rows ? image->rows : image->columns;
      fourier_info.width=(extent & 0x01) == 1 ? extent+1UL : extent;
    }
  fourier_info.height=fourier_info.width;
  fourier_info.center=(long) floor((double) fourier_info.width/2.0)+1L;
  fourier_info.channel=channel;
  fourier_info.modulus=modulus;
  magnitude=(double *) AcquireQuantumMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*magnitude));
  if (magnitude == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  phase=(double *) AcquireQuantumMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*phase));
  if (phase == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      magnitude=(double *) RelinquishMagickMemory(magnitude);
      return(MagickFalse);
    }
  fourier=(fftw_complex *) AcquireAlignedMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*fourier));
  if (fourier == (fftw_complex *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      phase=(double *) RelinquishMagickMemory(phase);
      magnitude=(double *) RelinquishMagickMemory(magnitude);
      return(MagickFalse);
    }
  status=ForwardFourierTransform(&fourier_info,image,magnitude,phase,exception);
  if (status != MagickFalse)
    status=ForwardFourier(&fourier_info,fourier_image,magnitude,phase,
      exception);
  fourier=(fftw_complex *) RelinquishAlignedMemory(fourier);
  phase=(double *) RelinquishMagickMemory(phase);
  magnitude=(double *) RelinquishMagickMemory(magnitude);
  return(status);
}
#endif

MagickExport Image *ForwardFourierTransformImage(const Image *image,
  const MagickBooleanType modulus,ExceptionInfo *exception)
{
  Image
    *fourier_image;

  fourier_image=NewImageList();
#if !defined(MAGICKCORE_FFTW_DELEGATE)
  (void) modulus;
  (void) ThrowMagickException(exception,GetMagickModule(),
    MissingDelegateWarning,"DelegateLibrarySupportNotBuiltIn","`%s' (FFTW)",
    image->filename);
#else
  {
    Image
      *magnitude_image;

    unsigned long
      extent,
      width;

    width=image->columns;
    if ((image->columns != image->rows) || ((image->columns % 2) != 0) ||
        ((image->rows % 2) != 0))
      {
        extent=image->columns < image->rows ? image->rows : image->columns;
        width=(extent & 0x01) == 1 ? extent+1UL : extent;
      }
    magnitude_image=CloneImage(image,width,width,MagickFalse,exception);
    if (magnitude_image != (Image *) NULL)
      {
        Image
          *phase_image;

        magnitude_image->storage_class=DirectClass;
        magnitude_image->depth=32UL;
        phase_image=CloneImage(image,width,width,MagickFalse,exception);
        if (phase_image == (Image *) NULL)
          magnitude_image=DestroyImage(magnitude_image);
        else
          {
            MagickBooleanType
              is_gray,
              status;

            register long
              i;

            phase_image->storage_class=DirectClass;
            phase_image->depth=32UL;
            AppendImageToList(&fourier_image,magnitude_image);
            AppendImageToList(&fourier_image,phase_image);
            status=MagickTrue;
            is_gray=IsGrayImage(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
            for (i=0L; i < 5L; i++)
            {
              MagickBooleanType
                thread_status;

              thread_status=MagickTrue;
              switch (i)
              {
                case 0:
                {
                  if (is_gray != MagickFalse)
                    {
                      thread_status=ForwardFourierTransformChannel(image,
                        GrayChannels,modulus,fourier_image,exception);
                      break;
                    }
                  thread_status=ForwardFourierTransformChannel(image,RedChannel,
                    modulus,fourier_image,exception);
                  break;
                }
                case 1:
                {
                  if (is_gray == MagickFalse)
                    thread_status=ForwardFourierTransformChannel(image,
                      GreenChannel,modulus,fourier_image,exception);
                  break;
                }
                case 2:
                {
                  if (is_gray == MagickFalse)
                    thread_status=ForwardFourierTransformChannel(image,
                      BlueChannel,modulus,fourier_image,exception);
                  break;
                }
                case 4:
                {
                  if (image->matte != MagickFalse)
                    thread_status=ForwardFourierTransformChannel(image,
                      OpacityChannel,modulus,fourier_image,exception);
                  break;
                }
                case 5:
                {
                  if (image->colorspace == CMYKColorspace)
                    thread_status=ForwardFourierTransformChannel(image,
                      IndexChannel,modulus,fourier_image,exception);
                  break;
                }
              }
              if (thread_status == MagickFalse)
                status=thread_status;
            }
            if (status == MagickFalse)
              fourier_image=DestroyImageList(fourier_image);
            fftw_cleanup();
          }
      }
  }
#endif
  return(fourier_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I n v e r s e F o u r i e r T r a n s f o r m I m a g e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InverseFourierTransformImage() implements the inverse discrete Fourier
%  transform (DFT) of the image either as a magnitude / phase or real /
%  imaginary image pair.
%
%  The format of the InverseFourierTransformImage method is:
%
%      Image *InverseFourierTransformImage(const Image *magnitude_image,
%        const Image *phase_image,const MagickBooleanType modulus,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o magnitude_image: the magnitude or real image.
%
%    o phase_image: the phase or imaginary image.
%
%    o modulus: if true, return transform as a magnitude / phase pair
%      otherwise a real / imaginary image pair.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_FFTW_DELEGATE)
static MagickBooleanType InverseQuadrantSwap(const unsigned long width,
  const unsigned long height,const double *source,double *destination)
{
  long
    center,
    y;

  register long
    x;

  /*
    Swap quadrants.
  */
  center=(long) floor((double) width/2.0)+1L;
  for (y=1L; y < (long) height; y++)
    for (x=0L; x < (long) (width/2L+1L); x++)
      destination[center*(height-y)-x+width/2L]=source[y*width+x];
  for (y=0L; y < (long) height; y++)
    destination[center*y]=source[y*width+width/2L];
  for (x=0L; x < center; x++)
    destination[x]=source[center-x-1L];
  return(RollFourier(center,height,0L,(long) height/-2L,destination));
}

static MagickBooleanType InverseFourier(FourierInfo *fourier_info,
  const Image *magnitude_image,const Image *phase_image,fftw_complex *fourier,
  ExceptionInfo *exception)
{
  CacheView
    *magnitude_view,
    *phase_view;

  double
    *magnitude,
    *phase,
    *magnitude_source,
    *phase_source;

  long
    y;

  MagickBooleanType
    status;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  /*
    Inverse fourier - read image and break down into a double array.
  */
  magnitude_source=(double *) AcquireQuantumMemory((size_t)
    fourier_info->height,fourier_info->width*sizeof(*magnitude_source));
  if (magnitude_source == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      return(MagickFalse);
    }
  phase_source=(double *) AcquireQuantumMemory((size_t) fourier_info->height,
    fourier_info->height*sizeof(*phase_source));
  if (phase_source == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      magnitude_source=(double *) RelinquishMagickMemory(magnitude_source);
      return(MagickFalse);
    }
  i=0L;
  magnitude_view=AcquireCacheView(magnitude_image);
  for (y=0L; y < (long) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(magnitude_view,0L,y,fourier_info->width,1UL,
      exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(magnitude_view);
    for (x=0L; x < (long) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          magnitude_source[i]=QuantumScale*GetRedPixelComponent(p);
          break;
        }
        case GreenChannel:
        {
          magnitude_source[i]=QuantumScale*GetGreenPixelComponent(p);
          break;
        }
        case BlueChannel:
        {
          magnitude_source[i]=QuantumScale*GetBluePixelComponent(p);
          break;
        }
        case OpacityChannel:
        {
          magnitude_source[i]=QuantumScale*GetOpacityPixelComponent(p);
          break;
        }
        case IndexChannel:
        {
          magnitude_source[i]=QuantumScale*indexes[x];
          break;
        }
        case GrayChannels:
        {
          magnitude_source[i]=QuantumScale*GetRedPixelComponent(p);
          break;
        }
      }
      i++;
      p++;
    }
  }
  i=0L;
  phase_view=AcquireCacheView(phase_image);
  for (y=0L; y < (long) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(phase_view,0,y,fourier_info->width,1,
      exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(phase_view);
    for (x=0L; x < (long) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          phase_source[i]=QuantumScale*GetRedPixelComponent(p);
          break;
        }
        case GreenChannel:
        {
          phase_source[i]=QuantumScale*GetGreenPixelComponent(p);
          break;
        }
        case BlueChannel:
        {
          phase_source[i]=QuantumScale*GetBluePixelComponent(p);
          break;
        }
        case OpacityChannel:
        {
          phase_source[i]=QuantumScale*GetOpacityPixelComponent(p);
          break;
        }
        case IndexChannel:
        {
          phase_source[i]=QuantumScale*indexes[x];
          break;
        }
        case GrayChannels:
        {
          phase_source[i]=QuantumScale*GetRedPixelComponent(p);
          break;
        }
      }
      i++;
      p++;
    }
  }
  if (fourier_info->modulus != MagickFalse)
    {
      i=0L;
      for (y=0L; y < (long) fourier_info->height; y++)
        for (x=0L; x < (long) fourier_info->width; x++)
        {
          phase_source[i]-=0.5;
          phase_source[i]*=(2.0*MagickPI);
          i++;
        }
    }
  magnitude_view=DestroyCacheView(magnitude_view);
  phase_view=DestroyCacheView(phase_view);
  magnitude=(double *) AcquireQuantumMemory((size_t) fourier_info->height,
    fourier_info->center*sizeof(*magnitude));
  if (magnitude == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      magnitude_source=(double *) RelinquishMagickMemory(magnitude_source);
      phase_source=(double *) RelinquishMagickMemory(phase_source);
      return(MagickFalse);
    }
  status=InverseQuadrantSwap(fourier_info->width,fourier_info->height,
    magnitude_source,magnitude);
  magnitude_source=(double *) RelinquishMagickMemory(magnitude_source);
  phase=(double *) AcquireQuantumMemory((size_t) fourier_info->width,
    fourier_info->height*sizeof(*phase));
  if (phase == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      phase_source=(double *) RelinquishMagickMemory(phase_source);
      return(MagickFalse);
    }
  CorrectPhaseLHS(fourier_info->width,fourier_info->width,phase_source);
  if (status != MagickFalse)
    status=InverseQuadrantSwap(fourier_info->width,fourier_info->height,
      phase_source,phase);
  phase_source=(double *) RelinquishMagickMemory(phase_source);
  /*
    Merge two sets.
  */
  i=0L;
  if (fourier_info->modulus != MagickFalse)
    for (y=0L; y < (long) fourier_info->height; y++)
       for (x=0L; x < (long) fourier_info->center; x++)
       {
         fourier[i]=magnitude[i]*cos(phase[i])+I*magnitude[i]*sin(phase[i]);
         i++;
      }
  else
    for (y=0L; y < (long) fourier_info->height; y++)
      for (x=0L; x < (long) fourier_info->center; x++)
      {
        fourier[i]=magnitude[i]+I*phase[i];
        i++;
      }
  phase=(double *) RelinquishMagickMemory(phase);
  magnitude=(double *) RelinquishMagickMemory(magnitude);
  return(status);
}

static MagickBooleanType InverseFourierTransform(FourierInfo *fourier_info,
  fftw_complex *fourier,Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  double
    *source;

  fftw_plan
    fftw_c2r_plan;

  long
    y;

  register IndexPacket
    *indexes;

  register long
    i,
    x;

  register PixelPacket
    *q;

  source=(double *) AcquireQuantumMemory((size_t) fourier_info->width,
    fourier_info->height*sizeof(double));
  if (source == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_InverseFourierTransform)
#endif
  fftw_c2r_plan=fftw_plan_dft_c2r_2d(fourier_info->width,fourier_info->height,
    fourier,source,FFTW_ESTIMATE);
  fftw_execute(fftw_c2r_plan);
  fftw_destroy_plan(fftw_c2r_plan);
  i=0L;
  image_view=AcquireCacheView(image);
  for (y=0L; y < (long) fourier_info->height; y++)
  {
    q=GetCacheViewAuthenticPixels(image_view,0L,y,fourier_info->width,1UL,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0L; x < (long) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          q->red=ClampToQuantum(QuantumRange*source[i]);
          break;
        }
        case GreenChannel:
        {
          q->green=ClampToQuantum(QuantumRange*source[i]);
          break;
        }
        case BlueChannel:
        {
          q->blue=ClampToQuantum(QuantumRange*source[i]);
          break;
        }
        case OpacityChannel:
        {
          q->opacity=ClampToQuantum(QuantumRange*source[i]);
          break;
        }
        case IndexChannel:
        {
          indexes[x]=ClampToQuantum(QuantumRange*source[i]);
          break;
        }
        case GrayChannels:
        {
          q->red=ClampToQuantum(QuantumRange*source[i]);
          q->green=q->red;
          q->blue=q->red;
          break;
        }
      }
      i++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  source=(double *) RelinquishMagickMemory(source);
  return(MagickTrue);
}

static MagickBooleanType InverseFourierTransformChannel(
  const Image *magnitude_image,const Image *phase_image,
  const ChannelType channel,const MagickBooleanType modulus,
  Image *fourier_image,ExceptionInfo *exception)
{
  double
    *magnitude,
    *phase;

  fftw_complex
    *fourier;

  FourierInfo
    fourier_info;

  MagickBooleanType
    status;

  size_t
    extent;

  fourier_info.width=magnitude_image->columns;
  if ((magnitude_image->columns != magnitude_image->rows) ||
      ((magnitude_image->columns % 2) != 0) ||
      ((magnitude_image->rows % 2) != 0))
    {
      extent=magnitude_image->columns < magnitude_image->rows ?
        magnitude_image->rows : magnitude_image->columns;
      fourier_info.width=(extent & 0x01) == 1 ? extent+1UL : extent;
    }
  fourier_info.height=fourier_info.width;
  fourier_info.center=(long) floor((double) fourier_info.width/2.0)+1L;
  fourier_info.channel=channel;
  fourier_info.modulus=modulus;
  magnitude=(double *) AcquireQuantumMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(double));
  if (magnitude == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      return(MagickFalse);
    }
  phase=(double *) AcquireQuantumMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(double));
  if (phase == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      magnitude=(double *) RelinquishMagickMemory(magnitude);
      return(MagickFalse);
    }
  fourier=(fftw_complex *) AcquireAlignedMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*fourier));
  if (fourier == (fftw_complex *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      phase=(double *) RelinquishMagickMemory(phase);
      magnitude=(double *) RelinquishMagickMemory(magnitude);
      return(MagickFalse);
    }
  status=InverseFourier(&fourier_info,magnitude_image,phase_image,fourier,
   exception);
  if (status != MagickFalse)
    status=InverseFourierTransform(&fourier_info,fourier,fourier_image,
      exception);
  fourier=(fftw_complex *) RelinquishAlignedMemory(fourier);
  phase=(double *) RelinquishMagickMemory(phase);
  magnitude=(double *) RelinquishMagickMemory(magnitude);
  return(status);
}
#endif

MagickExport Image *InverseFourierTransformImage(const Image *magnitude_image,
  const Image *phase_image,const MagickBooleanType modulus,
  ExceptionInfo *exception)
{
  Image
    *fourier_image;

  assert(magnitude_image != (Image *) NULL);
  assert(magnitude_image->signature == MagickSignature);
  if (magnitude_image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      magnitude_image->filename);
  if (phase_image == (Image *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ImageSequenceRequired","`%s'",magnitude_image->filename);
      return((Image *) NULL);
    }
#if !defined(MAGICKCORE_FFTW_DELEGATE)
  fourier_image=(Image *) NULL;
  (void) modulus;
  (void) ThrowMagickException(exception,GetMagickModule(),
    MissingDelegateWarning,"DelegateLibrarySupportNotBuiltIn","`%s' (FFTW)",
    magnitude_image->filename);
#else
  {
    fourier_image=CloneImage(magnitude_image,magnitude_image->columns,
      magnitude_image->rows,MagickFalse,exception);
    if (fourier_image != (Image *) NULL)
      {
        MagickBooleanType
          is_gray,
          status;

        register long
          i;

        status=MagickTrue;
        is_gray=IsGrayImage(magnitude_image,exception);
        if (is_gray != MagickFalse)
          is_gray=IsGrayImage(phase_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
        for (i=0L; i < 5L; i++)
        {
          MagickBooleanType
            thread_status;

          thread_status=MagickTrue;
          switch (i)
          {
            case 0:
            {
              if (is_gray != MagickFalse)
                {
                  thread_status=InverseFourierTransformChannel(magnitude_image,
                    phase_image,GrayChannels,modulus,fourier_image,exception);
                  break;
                }
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,RedChannel,modulus,fourier_image,exception);
              break;
            }
            case 1:
            {
              if (is_gray == MagickFalse)
                thread_status=InverseFourierTransformChannel(magnitude_image,
                  phase_image,GreenChannel,modulus,fourier_image,exception);
              break;
            }
            case 2:
            {
              if (is_gray == MagickFalse)
                thread_status=InverseFourierTransformChannel(magnitude_image,
                  phase_image,BlueChannel,modulus,fourier_image,exception);
              break;
            }
            case 3:
            {
              if (magnitude_image->matte != MagickFalse)
                thread_status=InverseFourierTransformChannel(magnitude_image,
                  phase_image,OpacityChannel,modulus,fourier_image,exception);
              break;
            }
            case 4:
            {
              if (magnitude_image->colorspace == CMYKColorspace)
                thread_status=InverseFourierTransformChannel(magnitude_image,
                  phase_image,IndexChannel,modulus,fourier_image,exception);
              break;
            }
          }
          if (thread_status == MagickFalse)
            status=thread_status;
        }
        if (status == MagickFalse)
          fourier_image=DestroyImage(fourier_image);
      }
      fftw_cleanup();
  }
#endif
  return(fourier_image);
}

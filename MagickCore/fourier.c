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
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/cache.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/fourier.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/thread-private.h"
#if defined(MAGICKCORE_FFTW_DELEGATE)
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
#include <complex.h>
#endif
#include <fftw3.h>
#if !defined(MAGICKCORE_HAVE_CABS)
#define cabs(z)  (sqrt(z[0]*z[0]+z[1]*z[1]))
#endif
#if !defined(MAGICKCORE_HAVE_CARG)
#define carg(z)  (atan2(cimag(z),creal(z)))
#endif
#if !defined(MAGICKCORE_HAVE_CIMAG)
#define cimag(z)  (z[1])
#endif
#if !defined(MAGICKCORE_HAVE_CREAL)
#define creal(z)  (z[0])
#endif
#endif

/*
  Typedef declarations.
*/
typedef struct _FourierInfo
{
  PixelChannel
    channel;

  MagickBooleanType
    modulus;

  size_t
    width,
    height;

  ssize_t
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

static MagickBooleanType RollFourier(const size_t width,const size_t height,
  const ssize_t x_offset,const ssize_t y_offset,double *fourier)
{
  double
    *roll;

  register ssize_t
    i,
    x;

  ssize_t
    u,
    v,
    y;

  /*
    Move zero frequency (DC, average color) from (0,0) to (width/2,height/2).
  */
  roll=(double *) AcquireQuantumMemory((size_t) height,width*sizeof(*roll));
  if (roll == (double *) NULL)
    return(MagickFalse);
  i=0L;
  for (y=0L; y < (ssize_t) height; y++)
  {
    if (y_offset < 0L)
      v=((y+y_offset) < 0L) ? y+y_offset+(ssize_t) height : y+y_offset;
    else
      v=((y+y_offset) > ((ssize_t) height-1L)) ? y+y_offset-(ssize_t) height :
        y+y_offset;
    for (x=0L; x < (ssize_t) width; x++)
    {
      if (x_offset < 0L)
        u=((x+x_offset) < 0L) ? x+x_offset+(ssize_t) width : x+x_offset;
      else
        u=((x+x_offset) > ((ssize_t) width-1L)) ? x+x_offset-(ssize_t) width :
          x+x_offset;
      roll[v*width+u]=fourier[i++];
    }
  }
  (void) CopyMagickMemory(fourier,roll,height*width*sizeof(*roll));
  roll=(double *) RelinquishMagickMemory(roll);
  return(MagickTrue);
}

static MagickBooleanType ForwardQuadrantSwap(const size_t width,
  const size_t height,double *source,double *destination)
{
  MagickBooleanType
    status;

  register ssize_t
    x;

  ssize_t
    center,
    y;

  /*
    Swap quadrants.
  */
  center=(ssize_t) floor((double) width/2L)+1L;
  status=RollFourier((size_t) center,height,0L,(ssize_t) height/2L,source);
  if (status == MagickFalse)
    return(MagickFalse);
  for (y=0L; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L-1L); x++)
      destination[width*y+x+width/2L]=source[center*y+x];
  for (y=1; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L-1L); x++)
      destination[width*(height-y)+width/2L-x-1L]=source[center*y+x+1L];
  for (x=0L; x < (ssize_t) (width/2L); x++)
    destination[-x+width/2L-1L]=destination[x+width/2L+1L];
  return(MagickTrue);
}

static void CorrectPhaseLHS(const size_t width,const size_t height,
  double *fourier)
{
  register ssize_t
    x;

  ssize_t
    y;

  for (y=0L; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L); x++)
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

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  ssize_t
    i,
    y;

  magnitude_image=GetFirstImageInList(image);
  phase_image=GetNextImageInList(image);
  if (phase_image == (Image *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "TwoOrMoreImagesRequired","`%s'",image->filename);
      return(MagickFalse);
    }
  /*
    Create "Fourier Transform" image from constituent arrays.
  */
  magnitude_source=(double *) AcquireQuantumMemory((size_t)
    fourier_info->height,fourier_info->width*sizeof(*magnitude_source));
  if (magnitude_source == (double *) NULL)
    return(MagickFalse);
  (void) ResetMagickMemory(magnitude_source,0,fourier_info->height*
    fourier_info->width*sizeof(*magnitude_source));
  phase_source=(double *) AcquireQuantumMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*phase_source));
  if (phase_source == (double *) NULL)
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
      for (y=0L; y < (ssize_t) fourier_info->height; y++)
        for (x=0L; x < (ssize_t) fourier_info->width; x++)
        {
          phase_source[i]/=(2.0*MagickPI);
          phase_source[i]+=0.5;
          i++;
        }
    }
  magnitude_view=AcquireAuthenticCacheView(magnitude_image,exception);
  i=0L;
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    q=GetCacheViewAuthenticPixels(magnitude_view,0L,y,fourier_info->height,1UL,
      exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedPixelChannel:
        default:
        {
          SetPixelRed(magnitude_image,ClampToQuantum(QuantumRange*
            magnitude_source[i]),q);
          break;
        }
        case GreenPixelChannel:
        {
          SetPixelGreen(magnitude_image,ClampToQuantum(QuantumRange*
            magnitude_source[i]),q);
          break;
        }
        case BluePixelChannel:
        {
          SetPixelBlue(magnitude_image,ClampToQuantum(QuantumRange*
            magnitude_source[i]),q);
          break;
        }
        case BlackPixelChannel:
        {
          SetPixelBlack(magnitude_image,ClampToQuantum(QuantumRange*
            magnitude_source[i]),q);
          break;
        }
        case AlphaPixelChannel:
        {
          SetPixelAlpha(magnitude_image,ClampToQuantum(QuantumRange*
            magnitude_source[i]),q);
          break;
        }
      }
      i++;
      q+=GetPixelChannels(magnitude_image);
    }
    status=SyncCacheViewAuthenticPixels(magnitude_view,exception);
    if (status == MagickFalse)
      break;
  }
  magnitude_view=DestroyCacheView(magnitude_view);
  i=0L;
  phase_view=AcquireAuthenticCacheView(phase_image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    q=GetCacheViewAuthenticPixels(phase_view,0L,y,fourier_info->height,1UL,
      exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedPixelChannel:
        default:
        {
          SetPixelRed(phase_image,ClampToQuantum(QuantumRange*
            phase_source[i]),q);
          break;
        }
        case GreenPixelChannel:
        {
          SetPixelGreen(phase_image,ClampToQuantum(QuantumRange*
            phase_source[i]),q);
          break;
        }
        case BluePixelChannel:
        {
          SetPixelBlue(phase_image,ClampToQuantum(QuantumRange*
            phase_source[i]),q);
          break;
        }
        case BlackPixelChannel:
        {
          SetPixelBlack(phase_image,ClampToQuantum(QuantumRange*
            phase_source[i]),q);
          break;
        }
        case AlphaPixelChannel:
        {
          SetPixelAlpha(phase_image,ClampToQuantum(QuantumRange*
            phase_source[i]),q);
          break;
        }
      }
      i++;
      q+=GetPixelChannels(phase_image);
    }
    status=SyncCacheViewAuthenticPixels(phase_view,exception);
    if (status == MagickFalse)
      break;
   }
  phase_view=DestroyCacheView(phase_view);
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

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    y;

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
  ResetMagickMemory(source,0,fourier_info->height*fourier_info->width*
    sizeof(*source));
  i=0L;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0L,y,fourier_info->width,1UL,
      exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedPixelChannel:
        default:
        {
          source[i]=QuantumScale*GetPixelRed(image,p);
          break;
        }
        case GreenPixelChannel:
        {
          source[i]=QuantumScale*GetPixelGreen(image,p);
          break;
        }
        case BluePixelChannel:
        {
          source[i]=QuantumScale*GetPixelBlue(image,p);
          break;
        }
        case BlackPixelChannel:
        {
          source[i]=QuantumScale*GetPixelBlack(image,p);
          break;
        }
        case AlphaPixelChannel:
        {
          source[i]=QuantumScale*GetPixelAlpha(image,p);
          break;
        }
      }
      i++;
      p+=GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  fourier=(fftw_complex *) AcquireQuantumMemory((size_t) fourier_info->height,
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
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
    for (x=0L; x < (ssize_t) fourier_info->center; x++)
    {
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
      fourier[i]/=n;
#else
      fourier[i][0]/=n;
      fourier[i][1]/=n;
#endif
      i++;
    }
  /*
    Generate magnitude and phase (or real and imaginary).
  */
  i=0L;
  if (fourier_info->modulus != MagickFalse)
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
      for (x=0L; x < (ssize_t) fourier_info->center; x++)
      {
        magnitude[i]=cabs(fourier[i]);
        phase[i]=carg(fourier[i]);
        i++;
      }
  else
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
      for (x=0L; x < (ssize_t) fourier_info->center; x++)
      {
        magnitude[i]=creal(fourier[i]);
        phase[i]=cimag(fourier[i]);
        i++;
      }
  fourier=(fftw_complex *) RelinquishMagickMemory(fourier);
  return(MagickTrue);
}

static MagickBooleanType ForwardFourierTransformChannel(const Image *image,
  const PixelChannel channel,const MagickBooleanType modulus,
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

  fourier_info.width=image->columns;
  if ((image->columns != image->rows) || ((image->columns % 2) != 0) ||
      ((image->rows % 2) != 0))
    {
      extent=image->columns < image->rows ? image->rows : image->columns;
      fourier_info.width=(extent & 0x01) == 1 ? extent+1UL : extent;
    }
  fourier_info.height=fourier_info.width;
  fourier_info.center=(ssize_t) floor((double) fourier_info.width/2.0)+1L;
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
  fourier=(fftw_complex *) AcquireQuantumMemory((size_t) fourier_info.height,
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
  fourier=(fftw_complex *) RelinquishMagickMemory(fourier);
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

    size_t
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

            phase_image->storage_class=DirectClass;
            phase_image->depth=32UL;
            AppendImageToList(&fourier_image,magnitude_image);
            AppendImageToList(&fourier_image,phase_image);
            status=MagickTrue;
            is_gray=IsImageGray(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp parallel sections
#endif
            {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
              #pragma omp section
#endif
              {
                MagickBooleanType
                  thread_status;

                if (is_gray != MagickFalse)
                  thread_status=ForwardFourierTransformChannel(image,
                    GrayPixelChannel,modulus,fourier_image,exception);
                else
                  thread_status=ForwardFourierTransformChannel(image,
                    RedPixelChannel,modulus,fourier_image,exception);
                if (thread_status == MagickFalse)
                  status=thread_status;
              }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
              #pragma omp section
#endif
              {
                MagickBooleanType
                  thread_status;

                thread_status=MagickTrue;
                if (is_gray == MagickFalse)
                  thread_status=ForwardFourierTransformChannel(image,
                    GreenPixelChannel,modulus,fourier_image,exception);
                if (thread_status == MagickFalse)
                  status=thread_status;
              }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
              #pragma omp section
#endif
              {
                MagickBooleanType
                  thread_status;

                thread_status=MagickTrue;
                if (is_gray == MagickFalse)
                  thread_status=ForwardFourierTransformChannel(image,
                    BluePixelChannel,modulus,fourier_image,exception);
                if (thread_status == MagickFalse)
                  status=thread_status;
              }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
              #pragma omp section
#endif
              {
                MagickBooleanType
                  thread_status;

                thread_status=MagickTrue;
                if (image->colorspace == CMYKColorspace)
                  thread_status=ForwardFourierTransformChannel(image,
                    BlackPixelChannel,modulus,fourier_image,exception);
                if (thread_status == MagickFalse)
                  status=thread_status;
              }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
              #pragma omp section
#endif
              {
                MagickBooleanType
                  thread_status;

                thread_status=MagickTrue;
                if (image->matte != MagickFalse)
                  thread_status=ForwardFourierTransformChannel(image,
                    AlphaPixelChannel,modulus,fourier_image,exception);
                if (thread_status == MagickFalse)
                  status=thread_status;
              }
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
static MagickBooleanType InverseQuadrantSwap(const size_t width,
  const size_t height,const double *source,double *destination)
{
  register ssize_t
    x;

  ssize_t
    center,
    y;

  /*
    Swap quadrants.
  */
  center=(ssize_t) floor((double) width/2.0)+1L;
  for (y=1L; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L+1L); x++)
      destination[center*(height-y)-x+width/2L]=source[y*width+x];
  for (y=0L; y < (ssize_t) height; y++)
    destination[center*y]=source[y*width+width/2L];
  for (x=0L; x < center; x++)
    destination[x]=source[center-x-1L];
  return(RollFourier(center,height,0L,(ssize_t) height/-2L,destination));
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

  MagickBooleanType
    status;

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    y;

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
    fourier_info->width*sizeof(*phase_source));
  if (phase_source == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      magnitude_source=(double *) RelinquishMagickMemory(magnitude_source);
      return(MagickFalse);
    }
  i=0L;
  magnitude_view=AcquireVirtualCacheView(magnitude_image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(magnitude_view,0L,y,fourier_info->width,1UL,
      exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedPixelChannel:
        default:
        {
          magnitude_source[i]=QuantumScale*GetPixelRed(magnitude_image,p);
          break;
        }
        case GreenPixelChannel:
        {
          magnitude_source[i]=QuantumScale*GetPixelGreen(magnitude_image,p);
          break;
        }
        case BluePixelChannel:
        {
          magnitude_source[i]=QuantumScale*GetPixelBlue(magnitude_image,p);
          break;
        }
        case BlackPixelChannel:
        {
          magnitude_source[i]=QuantumScale*GetPixelBlack(magnitude_image,p);
          break;
        }
        case AlphaPixelChannel:
        {
          magnitude_source[i]=QuantumScale*GetPixelAlpha(magnitude_image,p);
          break;
        }
      }
      i++;
      p+=GetPixelChannels(magnitude_image);
    }
  }
  i=0L;
  phase_view=AcquireVirtualCacheView(phase_image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(phase_view,0,y,fourier_info->width,1,
      exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedPixelChannel:
        default:
        {
          phase_source[i]=QuantumScale*GetPixelRed(phase_image,p);
          break;
        }
        case GreenPixelChannel:
        {
          phase_source[i]=QuantumScale*GetPixelGreen(phase_image,p);
          break;
        }
        case BluePixelChannel:
        {
          phase_source[i]=QuantumScale*GetPixelBlue(phase_image,p);
          break;
        }
        case BlackPixelChannel:
        {
          phase_source[i]=QuantumScale*GetPixelBlack(phase_image,p);
          break;
        }
        case AlphaPixelChannel:
        {
          phase_source[i]=QuantumScale*GetPixelAlpha(phase_image,p);
          break;
        }
      }
      i++;
      p+=GetPixelChannels(phase_image);
    }
  }
  if (fourier_info->modulus != MagickFalse)
    {
      i=0L;
      for (y=0L; y < (ssize_t) fourier_info->height; y++)
        for (x=0L; x < (ssize_t) fourier_info->width; x++)
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
  phase=(double *) AcquireQuantumMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*phase));
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
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
       for (x=0L; x < (ssize_t) fourier_info->center; x++)
       {
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
         fourier[i]=magnitude[i]*cos(phase[i])+I*magnitude[i]*sin(phase[i]);
#else
         fourier[i][0]=magnitude[i]*cos(phase[i]);
         fourier[i][1]=magnitude[i]*sin(phase[i]);
#endif
         i++;
      }
  else
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
      for (x=0L; x < (ssize_t) fourier_info->center; x++)
      {
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
        fourier[i]=magnitude[i]+I*phase[i];
#else
        fourier[i][0]=magnitude[i];
        fourier[i][1]=phase[i];
#endif
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

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  source=(double *) AcquireQuantumMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*source));
  if (source == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_InverseFourierTransform)
#endif
  {
    fftw_c2r_plan=fftw_plan_dft_c2r_2d(fourier_info->width,fourier_info->height,
      fourier,source,FFTW_ESTIMATE);
    fftw_execute(fftw_c2r_plan);
    fftw_destroy_plan(fftw_c2r_plan);
  }
  i=0L;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    if (y >= (ssize_t) image->rows)
      break;
    q=GetCacheViewAuthenticPixels(image_view,0L,y,fourier_info->width >
      image->columns ? image->columns : fourier_info->width,1UL,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedPixelChannel:
        default:
        {
          SetPixelRed(image,ClampToQuantum(QuantumRange*source[i]),q);
          break;
        }
        case GreenPixelChannel:
        {
          SetPixelGreen(image,ClampToQuantum(QuantumRange*source[i]),q);
          break;
        }
        case BluePixelChannel:
        {
          SetPixelBlue(image,ClampToQuantum(QuantumRange*source[i]),q);
          break;
        }
        case BlackPixelChannel:
        {
          SetPixelBlack(image,ClampToQuantum(QuantumRange*source[i]),q);
          break;
        }
        case AlphaPixelChannel:
        {
          SetPixelAlpha(image,ClampToQuantum(QuantumRange*source[i]),q);
          break;
        }
      }
      i++;
      q+=GetPixelChannels(image);
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
  const PixelChannel channel,const MagickBooleanType modulus,
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
  fourier_info.center=(ssize_t) floor((double) fourier_info.width/2.0)+1L;
  fourier_info.channel=channel;
  fourier_info.modulus=modulus;
  magnitude=(double *) AcquireQuantumMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*magnitude));
  if (magnitude == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      return(MagickFalse);
    }
  phase=(double *) AcquireQuantumMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*phase));
  if (phase == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      magnitude=(double *) RelinquishMagickMemory(magnitude);
      return(MagickFalse);
    }
  fourier=(fftw_complex *) AcquireQuantumMemory((size_t) fourier_info.height,
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
  fourier=(fftw_complex *) RelinquishMagickMemory(fourier);
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
        "TwoOrMoreImagesRequired","`%s'",magnitude_image->filename);
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

        status=MagickTrue;
        is_gray=IsImageGray(magnitude_image,exception);
        if (is_gray != MagickFalse)
          is_gray=IsImageGray(phase_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp parallel sections
#endif
        {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp section
#endif
          {
            MagickBooleanType
              thread_status;

            if (is_gray != MagickFalse)
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,GrayPixelChannel,modulus,fourier_image,exception);
            else
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,RedPixelChannel,modulus,fourier_image,exception);
            if (thread_status == MagickFalse)
              status=thread_status;
          }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp section
#endif
          {
            MagickBooleanType
              thread_status;

            thread_status=MagickTrue;
            if (is_gray == MagickFalse)
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,GreenPixelChannel,modulus,fourier_image,exception);
            if (thread_status == MagickFalse)
              status=thread_status;
          }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp section
#endif
          {
            MagickBooleanType
              thread_status;

            thread_status=MagickTrue;
            if (is_gray == MagickFalse)
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,BluePixelChannel,modulus,fourier_image,exception);
            if (thread_status == MagickFalse)
              status=thread_status;
          }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp section
#endif
          {
            MagickBooleanType
              thread_status;

            thread_status=MagickTrue;
            if (magnitude_image->colorspace == CMYKColorspace)
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,BlackPixelChannel,modulus,fourier_image,exception);
            if (thread_status == MagickFalse)
              status=thread_status;
          }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp section
#endif
          {
            MagickBooleanType
              thread_status;

            thread_status=MagickTrue;
            if (magnitude_image->matte != MagickFalse)
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,AlphaPixelChannel,modulus,fourier_image,exception);
            if (thread_status == MagickFalse)
              status=thread_status;
          }
        }
        if (status == MagickFalse)
          fourier_image=DestroyImage(fourier_image);
      }
    fftw_cleanup();
  }
#endif
  return(fourier_image);
}

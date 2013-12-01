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
%                                   Cristy                                    %
%                                 July 2009                                   %
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
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/cache.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/fourier.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
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
  ChannelType
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
%     C o m p l e x I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ComplexImages() performs complex mathematics on an image sequence.
%
%  The format of the ComplexImages method is:
%
%      MagickBooleanType ComplexImages(Image *images,
%        const ComplexOperator op,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o op: A complex operator.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ComplexImages(const Image *images,const ComplexOperator op,
  ExceptionInfo *exception)
{
#define ComplexImageTag  "Complex/Image"

  CacheView
    *Ai_view,
    *Ar_view,
    *Bi_view,
    *Br_view,
    *Ci_view,
    *Cr_view;

  const char
    *artifact;

  const Image
    *Ai_image,
    *Ar_image,
    *Bi_image,
    *Br_image;

  double
    snr;

  Image
    *Ci_image,
    *complex_images,
    *Cr_image,
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (images->next == (Image *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ImageSequenceRequired","`%s'",images->filename);
      return((Image *) NULL);
    }
  image=CloneImage(images,images->columns,images->rows,MagickTrue,exception);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  image->storage_class=DirectClass;
  image->depth=32UL;
  complex_images=NewImageList();
  AppendImageToList(&complex_images,image);
  image=CloneImage(images,images->columns,images->rows,MagickTrue,exception);
  if (image == (Image *) NULL)
    {
      complex_images=DestroyImageList(complex_images);
      return(complex_images);
    }
  image->storage_class=DirectClass;
  image->depth=32UL;
  AppendImageToList(&complex_images,image);
  /*
    Apply complex mathematics to image pixels.
  */
  artifact=GetImageArtifact(image,"complex:snr");
  snr=0.0;
  if (artifact != (const char *) NULL)
    snr=StringToDouble(artifact,(char **) NULL);
  Ar_image=images;
  Ai_image=images->next;
  Br_image=images;
  Bi_image=images->next;
  if ((images->next->next != (Image *) NULL) &&
      (images->next->next->next != (Image *) NULL))
    {
      Br_image=images->next->next;
      Bi_image=images->next->next->next;
    }
  Cr_image=complex_images;
  Ci_image=complex_images->next;
  Ar_view=AcquireVirtualCacheView(Ar_image,exception);
  Ai_view=AcquireVirtualCacheView(Ai_image,exception);
  Br_view=AcquireVirtualCacheView(Br_image,exception);
  Bi_view=AcquireVirtualCacheView(Bi_image,exception);
  Cr_view=AcquireAuthenticCacheView(Cr_image,exception);
  Ci_view=AcquireAuthenticCacheView(Ci_image,exception);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp parallel for schedule(static,4) shared(progress,status) \
      magick_threads(images,complex_images,images->rows,1)
#endif
  for (y=0; y < (ssize_t) images->rows; y++)
  {
    register const PixelPacket
      *restrict Ai,
      *restrict Ar,
      *restrict Bi,
      *restrict Br;

    register PixelPacket
      *restrict Ci,
      *restrict Cr;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    Ar=GetCacheViewVirtualPixels(Ar_view,0,y,images->columns,1,exception);
    Ai=GetCacheViewVirtualPixels(Ai_view,0,y,images->columns,1,exception);
    Br=GetCacheViewVirtualPixels(Br_view,0,y,images->columns,1,exception);
    Bi=GetCacheViewVirtualPixels(Bi_view,0,y,images->columns,1,exception);
    Cr=QueueCacheViewAuthenticPixels(Cr_view,0,y,images->columns,1,exception);
    Ci=QueueCacheViewAuthenticPixels(Ci_view,0,y,images->columns,1,exception);
    if ((Ar == (const PixelPacket *) NULL) ||
        (Ai == (const PixelPacket *) NULL) ||
        (Br == (const PixelPacket *) NULL) ||
        (Bi == (const PixelPacket *) NULL) ||
        (Cr == (PixelPacket *) NULL) || (Ci == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) images->columns; x++)
    {
      switch (op)
      {
        case AddComplexOperator:
        {
          Cr->red=Ar->red+Br->red;
          Ci->red=Ai->red+Bi->red;
          Cr->green=Ar->green+Br->green;
          Ci->green=Ai->green+Bi->green;
          Cr->blue=Ar->blue+Br->blue;
          Ci->blue=Ai->blue+Bi->blue;
          if (images->matte != MagickFalse)
            {
              Cr->opacity=Ar->opacity+Br->opacity;
              Ci->opacity=Ai->opacity+Bi->opacity;
            }
          break;
        }
        case ConjugateComplexOperator:
        default:
        {
          Cr->red=Ar->red;
          Ci->red=(-Bi->red);
          Cr->green=Ar->green;
          Ci->green=(-Bi->green);
          Cr->blue=Ar->blue;
          Ci->blue=(-Bi->blue);
          if (images->matte != MagickFalse)
            {
              Cr->opacity=Ar->opacity;
              Ci->opacity=(-Bi->opacity);
            }
          break;
        }
        case DivideComplexOperator:
        {
          double
            gamma;

          gamma=PerceptibleReciprocal(Br->red*Br->red+Bi->red*Bi->red+snr);
          Cr->red=gamma*(Ar->red*Br->red+Ai->red*Bi->red);
          Ci->red=gamma*(Ai->red*Br->red-Ar->red*Bi->red);
          gamma=PerceptibleReciprocal(Br->green*Br->green+Bi->green*Bi->green+
            snr);
          Cr->green=gamma*(Ar->green*Br->green+Ai->green*Bi->green);
          Ci->green=gamma*(Ai->green*Br->green-Ar->green*Bi->green);
          gamma=PerceptibleReciprocal(Br->blue*Br->blue+Bi->blue*Bi->blue+snr);
          Cr->blue=gamma*(Ar->blue*Br->blue+Ai->blue*Bi->blue);
          Ci->blue=gamma*(Ai->blue*Br->blue-Ar->blue*Bi->blue);
          if (images->matte != MagickFalse)
            {
              gamma=PerceptibleReciprocal(Br->opacity*Br->opacity+Bi->opacity*
                Bi->opacity+snr);
              Cr->opacity=gamma*(Ar->opacity*Br->opacity+Ai->opacity*
                Bi->opacity);
              Ci->opacity=gamma*(Ai->opacity*Br->opacity-Ar->opacity*
                Bi->opacity);
            }
          break;
        }
        case MagnitudePhaseComplexOperator:
        {
          Cr->red=sqrt(Ar->red*Ar->red+Ai->red*Ai->red);
          Ci->red=atan2(Ai->red,Ar->red)/(2.0*MagickPI)+0.5;
          Cr->green=sqrt(Ar->green*Ar->green+Ai->green*Ai->green);
          Ci->green=atan2(Ai->green,Ar->green)/(2.0*MagickPI)+0.5;
          Cr->blue=sqrt(Ar->blue*Ar->blue+Ai->blue*Ai->blue);
          Ci->blue=atan2(Ai->blue,Ar->blue)/(2.0*MagickPI)+0.5;
          if (images->matte != MagickFalse)
            {
              Cr->opacity=sqrt(Ar->opacity*Ar->opacity+Ai->opacity*Ai->opacity);
              Ci->opacity=atan2(Ai->opacity,Ar->opacity)/(2.0*MagickPI)+0.5;
            }
          break;
        }
        case MultiplyComplexOperator:
        {
          Cr->red=QuantumScale*(Ar->red*Br->red-Ai->red*Bi->red);
          Ci->red=QuantumScale*(Ai->red*Br->red+Ar->red*Bi->red);
          Cr->green=QuantumScale*(Ar->green*Br->green-Ai->green*Bi->green);
          Ci->green=QuantumScale*(Ai->green*Br->green+Ar->green*Bi->green);
          Cr->blue=QuantumScale*(Ar->blue*Br->blue-Ai->blue*Bi->blue);
          Ci->blue=QuantumScale*(Ai->blue*Br->blue+Ar->blue*Bi->blue);
          if (images->matte != MagickFalse)
            {
              Cr->opacity=QuantumScale*(Ar->opacity*Br->opacity-Ai->opacity*
                Bi->opacity);
              Ci->opacity=QuantumScale*(Ai->opacity*Br->opacity+Ar->opacity*
                Bi->opacity);
            }
          break;
        }
        case RealImaginaryComplexOperator:
        {
          Cr->red=Ar->red*cos(2.0*MagickPI*(Ai->red-0.5));
          Ci->red=Ar->red*sin(2.0*MagickPI*(Ai->red-0.5));
          Cr->green=Ar->green*cos(2.0*MagickPI*(Ai->green-0.5));
          Ci->green=Ar->green*sin(2.0*MagickPI*(Ai->green-0.5));
          Cr->blue=Ar->blue*cos(2.0*MagickPI*(Ai->blue-0.5));
          Ci->blue=Ar->blue*sin(2.0*MagickPI*(Ai->blue-0.5));
          if (images->matte != MagickFalse)
            {
               Cr->opacity=Ar->opacity*cos(2.0*MagickPI*(Ai->opacity-0.5));
               Ci->opacity=Ar->opacity*sin(2.0*MagickPI*(Ai->opacity-0.5));
            }
          break;
        }
        case SubtractComplexOperator:
        {
          Cr->red=Ar->red-Br->red;
          Ci->red=Ai->red-Bi->red;
          Cr->green=Ar->green-Br->green;
          Ci->green=Ai->green-Bi->green;
          Cr->blue=Ar->blue-Br->blue;
          Ci->blue=Ai->blue-Bi->blue;
          if (images->matte != MagickFalse)
            {
              Cr->opacity=Ar->opacity-Br->opacity;
              Ci->opacity=Ai->opacity-Bi->opacity;
            }
          break;
        }
      }
      Ar++;
      Ai++;
      Br++;
      Bi++;
      Cr++;
      Ci++;
    }
    if (SyncCacheViewAuthenticPixels(Ci_view,exception) == MagickFalse)
      status=MagickFalse;
    if (SyncCacheViewAuthenticPixels(Cr_view,exception) == MagickFalse)
      status=MagickFalse;
    if (images->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_ComplexImages)
#endif
        proceed=SetImageProgress(images,ComplexImageTag,progress++,
          images->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  Cr_view=DestroyCacheView(Cr_view);
  Ci_view=DestroyCacheView(Ci_view);
  Br_view=DestroyCacheView(Br_view);
  Bi_view=DestroyCacheView(Bi_view);
  Ar_view=DestroyCacheView(Ar_view);
  Ai_view=DestroyCacheView(Ai_view);
  if (status == MagickFalse)
    complex_images=DestroyImageList(complex_images);
  return(complex_images);
}

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
  const ssize_t x_offset,const ssize_t y_offset,double *roll_pixels)
{
  double
    *source_pixels;

  MemoryInfo
    *source_info;

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
  source_info=AcquireVirtualMemory(height,width*sizeof(*source_pixels));
  if (source_info == (MemoryInfo *) NULL)
    return(MagickFalse);
  source_pixels=(double *) GetVirtualMemoryBlob(source_info);
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
      source_pixels[v*width+u]=roll_pixels[i++];
    }
  }
  (void) CopyMagickMemory(roll_pixels,source_pixels,height*width*
    sizeof(*source_pixels));
  source_info=RelinquishVirtualMemory(source_info);
  return(MagickTrue);
}

static MagickBooleanType ForwardQuadrantSwap(const size_t width,
  const size_t height,double *source_pixels,double *forward_pixels)
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
  status=RollFourier((size_t) center,height,0L,(ssize_t) height/2L,
    source_pixels);
  if (status == MagickFalse)
    return(MagickFalse);
  for (y=0L; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L-1L); x++)
      forward_pixels[y*width+x+width/2L]=source_pixels[y*center+x];
  for (y=1; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L-1L); x++)
      forward_pixels[(height-y)*width+width/2L-x-1L]=
        source_pixels[y*center+x+1L];
  for (x=0L; x < (ssize_t) (width/2L); x++)
    forward_pixels[-x+width/2L-1L]=forward_pixels[x+width/2L+1L];
  return(MagickTrue);
}

static void CorrectPhaseLHS(const size_t width,const size_t height,
  double *fourier_pixels)
{
  register ssize_t
    x;

  ssize_t
    y;

  for (y=0L; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L); x++)
      fourier_pixels[y*width+x]*=(-1.0);
}

static MagickBooleanType ForwardFourier(const FourierInfo *fourier_info,
  Image *image,double *magnitude,double *phase,ExceptionInfo *exception)
{
  CacheView
    *magnitude_view,
    *phase_view;

  double
    *magnitude_pixels,
    *phase_pixels;

  Image
    *magnitude_image,
    *phase_image;

  MagickBooleanType
    status;

  MemoryInfo
    *magnitude_info,
    *phase_info;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    x;

  ssize_t
    i,
    y;

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
  magnitude_info=AcquireVirtualMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*magnitude_pixels));
  phase_info=AcquireVirtualMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*phase_pixels));
  if ((magnitude_info == (MemoryInfo *) NULL) ||
      (phase_info == (MemoryInfo *) NULL))
    {
      if (phase_info != (MemoryInfo *) NULL)
        phase_info=RelinquishVirtualMemory(phase_info);
      if (magnitude_info != (MemoryInfo *) NULL)
        magnitude_info=RelinquishVirtualMemory(magnitude_info);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  magnitude_pixels=(double *) GetVirtualMemoryBlob(magnitude_info);
  (void) ResetMagickMemory(magnitude_pixels,0,fourier_info->height*
    fourier_info->width*sizeof(*magnitude_pixels));
  phase_pixels=(double *) GetVirtualMemoryBlob(phase_info);
  (void) ResetMagickMemory(phase_pixels,0,fourier_info->height*
    fourier_info->width*sizeof(*phase_pixels));
  status=ForwardQuadrantSwap(fourier_info->width,fourier_info->height,
    magnitude,magnitude_pixels);
  if (status != MagickFalse)
    status=ForwardQuadrantSwap(fourier_info->width,fourier_info->height,phase,
      phase_pixels);
  CorrectPhaseLHS(fourier_info->width,fourier_info->height,phase_pixels);
  if (fourier_info->modulus != MagickFalse)
    {
      i=0L;
      for (y=0L; y < (ssize_t) fourier_info->height; y++)
        for (x=0L; x < (ssize_t) fourier_info->width; x++)
        {
          phase_pixels[i]/=(2.0*MagickPI);
          phase_pixels[i]+=0.5;
          i++;
        }
    }
  magnitude_view=AcquireAuthenticCacheView(magnitude_image,exception);
  i=0L;
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    q=GetCacheViewAuthenticPixels(magnitude_view,0L,y,fourier_info->height,1UL,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(magnitude_view);
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          SetPixelRed(q,ClampToQuantum(QuantumRange*magnitude_pixels[i]));
          break;
        }
        case GreenChannel:
        {
          SetPixelGreen(q,ClampToQuantum(QuantumRange*magnitude_pixels[i]));
          break;
        }
        case BlueChannel:
        {
          SetPixelBlue(q,ClampToQuantum(QuantumRange*magnitude_pixels[i]));
          break;
        }
        case OpacityChannel:
        {
          SetPixelOpacity(q,ClampToQuantum(QuantumRange*magnitude_pixels[i]));
          break;
        }
        case IndexChannel:
        {
          SetPixelIndex(indexes+x,ClampToQuantum(QuantumRange*
            magnitude_pixels[i]));
          break;
        }
        case GrayChannels:
        {
          SetPixelGray(q,ClampToQuantum(QuantumRange*magnitude_pixels[i]));
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
  magnitude_view=DestroyCacheView(magnitude_view);
  i=0L;
  phase_view=AcquireAuthenticCacheView(phase_image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    q=GetCacheViewAuthenticPixels(phase_view,0L,y,fourier_info->height,1UL,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(phase_view);
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          SetPixelRed(q,ClampToQuantum(QuantumRange*phase_pixels[i]));
          break;
        }
        case GreenChannel:
        {
          SetPixelGreen(q,ClampToQuantum(QuantumRange*phase_pixels[i]));
          break;
        }
        case BlueChannel:
        {
          SetPixelBlue(q,ClampToQuantum(QuantumRange*phase_pixels[i]));
          break;
        }
        case OpacityChannel:
        {
          SetPixelOpacity(q,ClampToQuantum(QuantumRange*phase_pixels[i]));
          break;
        }
        case IndexChannel:
        {
          SetPixelIndex(indexes+x,ClampToQuantum(QuantumRange*phase_pixels[i]));
          break;
        }
        case GrayChannels:
        {
          SetPixelGray(q,ClampToQuantum(QuantumRange*phase_pixels[i]));
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
  phase_info=RelinquishVirtualMemory(phase_info);
  magnitude_info=RelinquishVirtualMemory(magnitude_info);
  return(status);
}

static MagickBooleanType ForwardFourierTransform(FourierInfo *fourier_info,
  const Image *image,double *magnitude_pixels,double *phase_pixels,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  const char
    *value;

  double
    *source_pixels;

  fftw_complex
    *forward_pixels;

  fftw_plan
    fftw_r2c_plan;

  MemoryInfo
    *forward_info,
    *source_info;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  /*
    Generate the forward Fourier transform.
  */
  source_info=AcquireVirtualMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*source_pixels));
  if (source_info == (MemoryInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  source_pixels=(double *) GetVirtualMemoryBlob(source_info);
  ResetMagickMemory(source_pixels,0,fourier_info->height*fourier_info->width*
    sizeof(*source_pixels));
  i=0L;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0L,y,fourier_info->width,1UL,
      exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          source_pixels[i]=QuantumScale*GetPixelRed(p);
          break;
        }
        case GreenChannel:
        {
          source_pixels[i]=QuantumScale*GetPixelGreen(p);
          break;
        }
        case BlueChannel:
        {
          source_pixels[i]=QuantumScale*GetPixelBlue(p);
          break;
        }
        case OpacityChannel:
        {
          source_pixels[i]=QuantumScale*GetPixelOpacity(p);
          break;
        }
        case IndexChannel:
        {
          source_pixels[i]=QuantumScale*GetPixelIndex(indexes+x);
          break;
        }
        case GrayChannels:
        {
          source_pixels[i]=QuantumScale*GetPixelGray(p);
          break;
        }
      }
      i++;
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  forward_info=AcquireVirtualMemory((size_t) fourier_info->height,
    fourier_info->center*sizeof(*forward_pixels));
  if (forward_info == (MemoryInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      source_info=(MemoryInfo *) RelinquishVirtualMemory(source_info);
      return(MagickFalse);
    }
  forward_pixels=(fftw_complex *) GetVirtualMemoryBlob(forward_info);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ForwardFourierTransform)
#endif
  fftw_r2c_plan=fftw_plan_dft_r2c_2d(fourier_info->width,fourier_info->height,
    source_pixels,forward_pixels,FFTW_ESTIMATE);
  fftw_execute(fftw_r2c_plan);
  fftw_destroy_plan(fftw_r2c_plan);
  source_info=(MemoryInfo *) RelinquishVirtualMemory(source_info);
  value=GetImageArtifact(image,"fourier:normalize");
  if ((value == (const char *) NULL) || (LocaleCompare(value,"forward") == 0))
    {
      double
        gamma;

      /*
        Normalize inverse transform.
      */
      i=0L;
      gamma=PerceptibleReciprocal((double) fourier_info->width*
        fourier_info->height);
      for (y=0L; y < (ssize_t) fourier_info->height; y++)
        for (x=0L; x < (ssize_t) fourier_info->center; x++)
        {
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
          forward_pixels[i]*=gamma;
#else
          forward_pixels[i][0]*=gamma;
          forward_pixels[i][1]*=gamma;
#endif
          i++;
        }
    }
  /*
    Generate magnitude and phase (or real and imaginary).
  */
  i=0L;
  if (fourier_info->modulus != MagickFalse)
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
      for (x=0L; x < (ssize_t) fourier_info->center; x++)
      {
        magnitude_pixels[i]=cabs(forward_pixels[i]);
        phase_pixels[i]=carg(forward_pixels[i]);
        i++;
      }
  else
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
      for (x=0L; x < (ssize_t) fourier_info->center; x++)
      {
        magnitude_pixels[i]=creal(forward_pixels[i]);
        phase_pixels[i]=cimag(forward_pixels[i]);
        i++;
      }
  forward_info=(MemoryInfo *) RelinquishVirtualMemory(forward_info);
  return(MagickTrue);
}

static MagickBooleanType ForwardFourierTransformChannel(const Image *image,
  const ChannelType channel,const MagickBooleanType modulus,
  Image *fourier_image,ExceptionInfo *exception)
{
  double
    *magnitude_pixels,
    *phase_pixels;

  FourierInfo
    fourier_info;

  MagickBooleanType
    status;

  MemoryInfo
    *magnitude_info,
    *phase_info;

  size_t
    extent;

  fourier_info.width=image->columns;
  fourier_info.height=image->rows;
  if ((image->columns != image->rows) || ((image->columns % 2) != 0) ||
      ((image->rows % 2) != 0))
    {
      extent=image->columns < image->rows ? image->rows : image->columns;
      fourier_info.width=(extent & 0x01) == 1 ? extent+1UL : extent;
    }
  fourier_info.height=fourier_info.width;
  fourier_info.center=(ssize_t) floor((double) fourier_info.width/2L)+1L;
  fourier_info.channel=channel;
  fourier_info.modulus=modulus;
  magnitude_info=AcquireVirtualMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*magnitude_pixels));
  phase_info=AcquireVirtualMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*phase_pixels));
  if ((magnitude_info == (MemoryInfo *) NULL) ||
      (phase_info == (MemoryInfo *) NULL))
    {
      if (phase_info != (MemoryInfo *) NULL)
        phase_info=RelinquishVirtualMemory(phase_info);
      if (magnitude_info == (MemoryInfo *) NULL)
        magnitude_info=RelinquishVirtualMemory(magnitude_info);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  magnitude_pixels=(double *) GetVirtualMemoryBlob(magnitude_info);
  phase_pixels=(double *) GetVirtualMemoryBlob(phase_info);
  status=ForwardFourierTransform(&fourier_info,image,magnitude_pixels,
    phase_pixels,exception);
  if (status != MagickFalse)
    status=ForwardFourier(&fourier_info,fourier_image,magnitude_pixels,
      phase_pixels,exception);
  phase_info=RelinquishVirtualMemory(phase_info);
  magnitude_info=RelinquishVirtualMemory(magnitude_info);
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
      height,
      width;

    width=image->columns;
    height=image->rows;
    if ((image->columns != image->rows) || ((image->columns % 2) != 0) ||
        ((image->rows % 2) != 0))
      {
        extent=image->columns < image->rows ? image->rows : image->columns;
        width=(extent & 0x01) == 1 ? extent+1UL : extent;
      }
    height=width;
    magnitude_image=CloneImage(image,width,height,MagickTrue,exception);
    if (magnitude_image != (Image *) NULL)
      {
        Image
          *phase_image;

        magnitude_image->storage_class=DirectClass;
        magnitude_image->depth=32UL;
        phase_image=CloneImage(image,width,height,MagickTrue,exception);
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
            is_gray=IsGrayImage(image,exception);
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
                    GrayChannels,modulus,fourier_image,exception);
                else
                  thread_status=ForwardFourierTransformChannel(image,RedChannel,
                    modulus,fourier_image,exception);
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
                    GreenChannel,modulus,fourier_image,exception);
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
                    BlueChannel,modulus,fourier_image,exception);
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
                    OpacityChannel,modulus,fourier_image,exception);
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
                    IndexChannel,modulus,fourier_image,exception);
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
  center=(ssize_t) floor((double) width/2L)+1L;
  for (y=1L; y < (ssize_t) height; y++)
    for (x=0L; x < (ssize_t) (width/2L+1L); x++)
      destination[(height-y)*center-x+width/2L]=source[y*width+x];
  for (y=0L; y < (ssize_t) height; y++)
    destination[y*center]=source[y*width+width/2L];
  for (x=0L; x < center; x++)
    destination[x]=source[center-x-1L];
  return(RollFourier(center,height,0L,(ssize_t) height/-2L,destination));
}

static MagickBooleanType InverseFourier(FourierInfo *fourier_info,
  const Image *magnitude_image,const Image *phase_image,
  fftw_complex *fourier_pixels,ExceptionInfo *exception)
{
  CacheView
    *magnitude_view,
    *phase_view;

  double
    *inverse_pixels,
    *magnitude_pixels,
    *phase_pixels;

  MagickBooleanType
    status;

  MemoryInfo
    *inverse_info,
    *magnitude_info,
    *phase_info;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  /*
    Inverse fourier - read image and break down into a double array.
  */
  magnitude_info=AcquireVirtualMemory((size_t)fourier_info->height,
    fourier_info->width*sizeof(*magnitude_pixels));
  phase_info=AcquireVirtualMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*phase_pixels));
  inverse_info=AcquireVirtualMemory((size_t) fourier_info->height,
    fourier_info->center*sizeof(*inverse_pixels));
  if ((magnitude_info == (MemoryInfo *) NULL) ||
      (phase_info == (MemoryInfo *) NULL) ||
      (inverse_info == (MemoryInfo *) NULL))
    {
      if (magnitude_info != (MemoryInfo *) NULL)
        magnitude_info=RelinquishVirtualMemory(magnitude_info);
      if (phase_info != (MemoryInfo *) NULL)
        phase_info=RelinquishVirtualMemory(phase_info);
      if (inverse_info != (MemoryInfo *) NULL)
        inverse_info=RelinquishVirtualMemory(inverse_info);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      return(MagickFalse);
    }
  magnitude_pixels=(double *) GetVirtualMemoryBlob(magnitude_info);
  phase_pixels=(double *) GetVirtualMemoryBlob(phase_info);
  inverse_pixels=(double *) GetVirtualMemoryBlob(inverse_info);
  i=0L;
  magnitude_view=AcquireVirtualCacheView(magnitude_image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(magnitude_view,0L,y,fourier_info->width,1UL,
      exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(magnitude_view);
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          magnitude_pixels[i]=QuantumScale*GetPixelRed(p);
          break;
        }
        case GreenChannel:
        {
          magnitude_pixels[i]=QuantumScale*GetPixelGreen(p);
          break;
        }
        case BlueChannel:
        {
          magnitude_pixels[i]=QuantumScale*GetPixelBlue(p);
          break;
        }
        case OpacityChannel:
        {
          magnitude_pixels[i]=QuantumScale*GetPixelOpacity(p);
          break;
        }
        case IndexChannel:
        {
          magnitude_pixels[i]=QuantumScale*GetPixelIndex(indexes+x);
          break;
        }
        case GrayChannels:
        {
          magnitude_pixels[i]=QuantumScale*GetPixelGray(p);
          break;
        }
      }
      i++;
      p++;
    }
  }
  magnitude_view=DestroyCacheView(magnitude_view);
  status=InverseQuadrantSwap(fourier_info->width,fourier_info->height,
    magnitude_pixels,inverse_pixels);
  (void) CopyMagickMemory(magnitude_pixels,inverse_pixels,fourier_info->height*
    fourier_info->center*sizeof(*magnitude_pixels));
  i=0L;
  phase_view=AcquireVirtualCacheView(phase_image,exception);
  for (y=0L; y < (ssize_t) fourier_info->height; y++)
  {
    p=GetCacheViewVirtualPixels(phase_view,0,y,fourier_info->width,1,
      exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(phase_view);
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      switch (fourier_info->channel)
      {
        case RedChannel:
        default:
        {
          phase_pixels[i]=QuantumScale*GetPixelRed(p);
          break;
        }
        case GreenChannel:
        {
          phase_pixels[i]=QuantumScale*GetPixelGreen(p);
          break;
        }
        case BlueChannel:
        {
          phase_pixels[i]=QuantumScale*GetPixelBlue(p);
          break;
        }
        case OpacityChannel:
        {
          phase_pixels[i]=QuantumScale*GetPixelOpacity(p);
          break;
        }
        case IndexChannel:
        {
          phase_pixels[i]=QuantumScale*GetPixelIndex(indexes+x);
          break;
        }
        case GrayChannels:
        {
          phase_pixels[i]=QuantumScale*GetPixelGray(p);
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
      for (y=0L; y < (ssize_t) fourier_info->height; y++)
        for (x=0L; x < (ssize_t) fourier_info->width; x++)
        {
          phase_pixels[i]-=0.5;
          phase_pixels[i]*=(2.0*MagickPI);
          i++;
        }
    }
  phase_view=DestroyCacheView(phase_view);
  CorrectPhaseLHS(fourier_info->width,fourier_info->height,phase_pixels);
  if (status != MagickFalse)
    status=InverseQuadrantSwap(fourier_info->width,fourier_info->height,
      phase_pixels,inverse_pixels);
  (void) CopyMagickMemory(phase_pixels,inverse_pixels,fourier_info->height*
    fourier_info->center*sizeof(*phase_pixels));
  inverse_info=RelinquishVirtualMemory(inverse_info);
  /*
    Merge two sets.
  */
  i=0L;
  if (fourier_info->modulus != MagickFalse)
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
       for (x=0L; x < (ssize_t) fourier_info->center; x++)
       {
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
         fourier_pixels[i]=magnitude_pixels[i]*cos(phase_pixels[i])+I*
           magnitude_pixels[i]*sin(phase_pixels[i]);
#else
         fourier_pixels[i][0]=magnitude_pixels[i]*cos(phase_pixels[i]);
         fourier_pixels[i][1]=magnitude_pixels[i]*sin(phase_pixels[i]);
#endif
         i++;
      }
  else
    for (y=0L; y < (ssize_t) fourier_info->height; y++)
      for (x=0L; x < (ssize_t) fourier_info->center; x++)
      {
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
        fourier_pixels[i]=magnitude_pixels[i]+I*phase_pixels[i];
#else
        fourier_pixels[i][0]=magnitude_pixels[i];
        fourier_pixels[i][1]=phase_pixels[i];
#endif
        i++;
      }
  magnitude_info=RelinquishVirtualMemory(magnitude_info);
  phase_info=RelinquishVirtualMemory(phase_info);
  return(status);
}

static MagickBooleanType InverseFourierTransform(FourierInfo *fourier_info,
  fftw_complex *fourier_pixels,Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  double
    *source_pixels;

  const char
    *value;

  fftw_plan
    fftw_c2r_plan;

  MemoryInfo
    *source_info;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  source_info=AcquireVirtualMemory((size_t) fourier_info->height,
    fourier_info->width*sizeof(*source_pixels));
  if (source_info == (MemoryInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  source_pixels=(double *) GetVirtualMemoryBlob(source_info);
  value=GetImageArtifact(image,"fourier:normalize");
  if (LocaleCompare(value,"inverse") == 0)
    {
      double
        gamma;

      /*
        Normalize Fourier transform.
      */
      i=0L;
      gamma=PerceptibleReciprocal((double) fourier_info->width*
        fourier_info->height);
      for (y=0L; y < (ssize_t) fourier_info->height; y++)
        for (x=0L; x < (ssize_t) fourier_info->center; x++)
        {
#if defined(MAGICKCORE_HAVE_COMPLEX_H)
          fourier_pixels[i]*=gamma;
#else
          fourier_pixels[i][0]*=gamma;
          fourier_pixels[i][1]*=gamma;
#endif
          i++;
        }
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_InverseFourierTransform)
#endif
  {
    fftw_c2r_plan=fftw_plan_dft_c2r_2d(fourier_info->width,fourier_info->height,
      fourier_pixels,source_pixels,FFTW_ESTIMATE);
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
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0L; x < (ssize_t) fourier_info->width; x++)
    {
      if (x < (ssize_t) image->columns)
        switch (fourier_info->channel)
        {
          case RedChannel:
          default:
          {
            SetPixelRed(q,ClampToQuantum(QuantumRange*source_pixels[i]));
            break;
          }
          case GreenChannel:
          {
            SetPixelGreen(q,ClampToQuantum(QuantumRange*source_pixels[i]));
            break;
          }
          case BlueChannel:
          {
            SetPixelBlue(q,ClampToQuantum(QuantumRange*source_pixels[i]));
            break;
          }
          case OpacityChannel:
          {
            SetPixelOpacity(q,ClampToQuantum(QuantumRange*source_pixels[i]));
            break;
          }
          case IndexChannel:
          {
            SetPixelIndex(indexes+x,ClampToQuantum(QuantumRange*
              source_pixels[i]));
            break;
          }
          case GrayChannels:
          {
            SetPixelGray(q,ClampToQuantum(QuantumRange*source_pixels[i]));
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
  source_info=RelinquishVirtualMemory(source_info);
  return(MagickTrue);
}

static MagickBooleanType InverseFourierTransformChannel(
  const Image *magnitude_image,const Image *phase_image,
  const ChannelType channel,const MagickBooleanType modulus,
  Image *fourier_image,ExceptionInfo *exception)
{
  fftw_complex
    *inverse_pixels;

  FourierInfo
    fourier_info;

  MagickBooleanType
    status;

  MemoryInfo
    *inverse_info;

  size_t
    extent;

  fourier_info.width=magnitude_image->columns;
  fourier_info.height=magnitude_image->rows;
  if ((magnitude_image->columns != magnitude_image->rows) ||
      ((magnitude_image->columns % 2) != 0) ||
      ((magnitude_image->rows % 2) != 0))
    {
      extent=magnitude_image->columns < magnitude_image->rows ?
        magnitude_image->rows : magnitude_image->columns;
      fourier_info.width=(extent & 0x01) == 1 ? extent+1UL : extent;
    }
  fourier_info.height=fourier_info.width;
  fourier_info.center=(ssize_t) floor((double) fourier_info.width/2L)+1L;
  fourier_info.channel=channel;
  fourier_info.modulus=modulus;
  inverse_info=AcquireVirtualMemory((size_t) fourier_info.height,
    fourier_info.center*sizeof(*inverse_pixels));
  if (inverse_info == (MemoryInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        magnitude_image->filename);
      return(MagickFalse);
    }
  inverse_pixels=(fftw_complex *) GetVirtualMemoryBlob(inverse_info);
  status=InverseFourier(&fourier_info,magnitude_image,phase_image,
    inverse_pixels,exception);
  if (status != MagickFalse)
    status=InverseFourierTransform(&fourier_info,inverse_pixels,fourier_image,
      exception);
  inverse_info=RelinquishVirtualMemory(inverse_info);
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
      magnitude_image->rows,MagickTrue,exception);
    if (fourier_image != (Image *) NULL)
      {
        MagickBooleanType
          is_gray,
          status;

        status=MagickTrue;
        is_gray=IsGrayImage(magnitude_image,exception);
        if (is_gray != MagickFalse)
          is_gray=IsGrayImage(phase_image,exception);
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
                phase_image,GrayChannels,modulus,fourier_image,exception);
            else
              thread_status=InverseFourierTransformChannel(magnitude_image,
                phase_image,RedChannel,modulus,fourier_image,exception);
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
                phase_image,GreenChannel,modulus,fourier_image,exception);
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
                phase_image,BlueChannel,modulus,fourier_image,exception);
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
                phase_image,OpacityChannel,modulus,fourier_image,exception);
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
                phase_image,IndexChannel,modulus,fourier_image,exception);
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

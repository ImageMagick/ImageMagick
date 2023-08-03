/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                AAA   N   N   AAA   L      Y   Y  ZZZZZ  EEEEE               %
%               A   A  NN  N  A   A  L       Y Y      ZZ  E                   %
%               AAAAA  N N N  AAAAA  L        Y     ZZZ   EEE                 %
%               A   A  N  NN  A   A  L        Y    ZZ     E                   %
%               A   A  N   N  A   A  LLLLL    Y    ZZZZZ  EEEEE               %
%                                                                             %
%                             Analyze An Image                                %
%                                                                             %
%                             Software Design                                 %
%                               Bill Corbis                                   %
%                              December 1998                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999 ImageMagick Studio LLC, a non-profit organization           %
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
*/

/*
  Include declarations.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "MagickCore/studio.h"
#include "MagickCore/MagickCore.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   a n a l y z e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  analyzeImage() computes the brightness and saturation mean,  standard
%  deviation, kurtosis and skewness and stores these values as attributes 
%  of the image.
%
%  The format of the analyzeImage method is:
%
%      size_t analyzeImage(Image *images,const int argc,char **argv,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _StatisticsInfo
{
  double
    area,
    brightness,
    mean,
    standard_deviation,
    sum[5],
    kurtosis,
    skewness;
} StatisticsInfo;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
static inline int GetMagickNumberThreads(const Image *source,
  const Image *destination,const size_t chunk,int multithreaded)
{
#define MagickMax(x,y)  (((x) > (y)) ? (x) : (y))
#define MagickMin(x,y)  (((x) < (y)) ? (x) : (y))

  /*
    Number of threads bounded by the amount of work and any thread resource
    limit.  The limit is 2 if the pixel cache type is not memory or
    memory-mapped.
  */
  if (multithreaded == 0)
    return(1);
  if (((GetImagePixelCacheType(source) != MemoryCache) &&
       (GetImagePixelCacheType(source) != MapCache)) ||
      ((GetImagePixelCacheType(destination) != MemoryCache) &&
       (GetImagePixelCacheType(destination) != MapCache)))
    return(MagickMax(MagickMin(GetMagickResourceLimit(ThreadResource),2),1));
  return(MagickMax(MagickMin((int) GetMagickResourceLimit(ThreadResource),
    (int) (chunk)/64),1));
}
#endif

ModuleExport size_t analyzeImage(Image **images,const int argc,
  const char **argv,ExceptionInfo *exception)
{
#define AnalyzeImageFilterTag  "Filter/Analyze"

  char
    text[MagickPathExtent];

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  assert(images != (Image **) NULL);
  assert(*images != (Image *) NULL);
  assert((*images)->signature == MagickCoreSignature);
  (void) argc;
  (void) argv;
  image=(*images);
  status=MagickTrue;
  progress=0;
  for ( ; image != (Image *) NULL; image=GetNextImageInList(image))
  {
    CacheView
      *image_view;

    double
      area;

    ssize_t
      y;

    StatisticsInfo
      brightness,
      saturation;

    if (status == MagickFalse)
      continue;
    (void) memset(&brightness,0,sizeof(brightness));
    (void) memset(&saturation,0,sizeof(saturation));
    status=MagickTrue;
    image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) \
    shared(progress,status,brightness,saturation) \
    num_threads(GetMagickNumberThreads(image,image,image->rows,1))
#endif
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      const Quantum
        *p;

      ssize_t
        i,
        x;

      StatisticsInfo
        local_brightness,
        local_saturation;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      (void) memset(&local_brightness,0,sizeof(local_brightness));
      (void) memset(&local_saturation,0,sizeof(local_saturation));
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        double
          b,
          h,
          s;

        ConvertRGBToHSL(GetPixelRed(image,p),GetPixelGreen(image,p),
          GetPixelBlue(image,p),&h,&s,&b);
        b*=(double) QuantumRange;
        for (i=1; i <= 4; i++)
          local_brightness.sum[i]+=pow(b,(double) i);
        s*=(double) QuantumRange;
        for (i=1; i <= 4; i++)
          local_saturation.sum[i]+=pow(s,(double) i);
        p+=GetPixelChannels(image);
      }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp critical (analyzeImage)
#endif
      for (i=1; i <= 4; i++)
      {
        brightness.sum[i]+=local_brightness.sum[i];
        saturation.sum[i]+=local_saturation.sum[i];
      }
    }
    image_view=DestroyCacheView(image_view);
    area=(double) image->columns*image->rows;
    brightness.mean=brightness.sum[1]/area;
    (void) FormatLocaleString(text,MagickPathExtent,"%g",brightness.mean);
    (void) SetImageProperty(image,"filter:brightness:mean",text,exception);
    brightness.standard_deviation=sqrt(brightness.sum[2]/area-
      (brightness.sum[1]/area*brightness.sum[1]/area));
    (void) FormatLocaleString(text,MagickPathExtent,"%g",
      brightness.standard_deviation);
    (void) SetImageProperty(image,"filter:brightness:standard-deviation",text,
      exception);
    if (fabs(brightness.standard_deviation) >= MagickEpsilon)
      brightness.kurtosis=(brightness.sum[4]/area-4.0*brightness.mean*
        brightness.sum[3]/area+6.0*brightness.mean*brightness.mean*
        brightness.sum[2]/area-3.0*brightness.mean*brightness.mean*
        brightness.mean*brightness.mean)/(brightness.standard_deviation*
        brightness.standard_deviation*brightness.standard_deviation*
        brightness.standard_deviation)-3.0;
    (void) FormatLocaleString(text,MagickPathExtent,"%g",brightness.kurtosis);
    (void) SetImageProperty(image,"filter:brightness:kurtosis",text,exception);
    if (brightness.standard_deviation != 0)
      brightness.skewness=(brightness.sum[3]/area-3.0*brightness.mean*
        brightness.sum[2]/area+2.0*brightness.mean*brightness.mean*
        brightness.mean)/(brightness.standard_deviation*
        brightness.standard_deviation*brightness.standard_deviation);
    (void) FormatLocaleString(text,MagickPathExtent,"%g",brightness.skewness);
    (void) SetImageProperty(image,"filter:brightness:skewness",text,exception);
    saturation.mean=saturation.sum[1]/area;
    (void) FormatLocaleString(text,MagickPathExtent,"%g",saturation.mean);
    (void) SetImageProperty(image,"filter:saturation:mean",text,exception);
    saturation.standard_deviation=sqrt(saturation.sum[2]/area-
      (saturation.sum[1]/area*saturation.sum[1]/area));
    (void) FormatLocaleString(text,MagickPathExtent,"%g",
      saturation.standard_deviation);
    (void) SetImageProperty(image,"filter:saturation:standard-deviation",text,
      exception);
    if (fabs(saturation.standard_deviation) >= MagickEpsilon)
      saturation.kurtosis=(saturation.sum[4]/area-4.0*saturation.mean*
        saturation.sum[3]/area+6.0*saturation.mean*saturation.mean*
        saturation.sum[2]/area-3.0*saturation.mean*saturation.mean*
        saturation.mean*saturation.mean)/(saturation.standard_deviation*
        saturation.standard_deviation*saturation.standard_deviation*
        saturation.standard_deviation)-3.0;
    (void) FormatLocaleString(text,MagickPathExtent,"%g",saturation.kurtosis);
    (void) SetImageProperty(image,"filter:saturation:kurtosis",text,exception);
    if (fabs(saturation.standard_deviation) >= MagickEpsilon)
      saturation.skewness=(saturation.sum[3]/area-3.0*saturation.mean*
        saturation.sum[2]/area+2.0*saturation.mean*saturation.mean*
        saturation.mean)/(saturation.standard_deviation*
        saturation.standard_deviation*saturation.standard_deviation);
    (void) FormatLocaleString(text,MagickPathExtent,"%g",saturation.skewness);
    (void) SetImageProperty(image,"filter:saturation:skewness",text,exception);
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,AnalyzeImageFilterTag,progress,
          GetImageListLength(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(MagickImageFilterSignature);
}

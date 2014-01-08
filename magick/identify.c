/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           IIIII  DDDD   EEEEE  N   N  TTTTT  IIIII  FFFFF  Y   Y            %
%             I    D   D  E      NN  N    T      I    F       Y Y             %
%             I    D   D  EEE    N N N    T      I    FFF      Y              %
%             I    D   D  E      N  NN    T      I    F        Y              %
%           IIIII  DDDD   EEEEE  N   N    T    IIIII  F        Y              %
%                                                                             %
%                                                                             %
%               Identify an Image Format and Characteristics.                 %
%                                                                             %
%                           Software Design                                   %
%                                Cristy                                       %
%                            September 1994                                   %
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
%  Identify describes the format and characteristics of one or more image
%  files.  It will also report if an image is incomplete or corrupt.
%
%
*/


/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/annotate.h"
#include "magick/artifact.h"
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/cache.h"
#include "magick/client.h"
#include "magick/coder.h"
#include "magick/color.h"
#include "magick/configure.h"
#include "magick/constitute.h"
#include "magick/decorate.h"
#include "magick/delegate.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/feature.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/histogram.h"
#include "magick/identify.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/locale_.h"
#include "magick/log.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/montage.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/prepress.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/registry.h"
#include "magick/resize.h"
#include "magick/resource_.h"
#include "magick/signature.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/timer.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/version.h"
#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(MAGICKCORE_HAVE_LCMS_LCMS2_H)
#include <lcms/lcms2.h>
#elif defined(MAGICKCORE_HAVE_LCMS2_H)
#include "lcms2.h"
#elif defined(MAGICKCORE_HAVE_LCMS_LCMS_H)
#include <lcms/lcms.h>
#else
#include "lcms.h"
#endif
#endif


/*
  Define declarations.
*/
#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(LCMS_VERSION) && (LCMS_VERSION < 2000)
#define cmsUInt32Number  DWORD
#endif
#endif


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I d e n t i f y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyImage() identifies an image by printing its attributes to the file.
%  Attributes include the image width, height, size, and others.
%
%  The format of the IdentifyImage method is:
%
%      MagickBooleanType IdentifyImage(Image *image,FILE *file,
%        const MagickBooleanType verbose)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o file: the file, typically stdout.
%
%    o verbose: A value other than zero prints more detailed information
%      about the image.
%
*/

static ChannelStatistics *GetLocationStatistics(const Image *image,
  const StatisticType type,ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  register ssize_t
    i;

  size_t
    length;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=CompositeChannels+1UL;
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(length,
    sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_statistics,0,length*
    sizeof(*channel_statistics));
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
  {
    switch (type)
    {
      case MaximumStatistic:
      default:
      {
        channel_statistics[i].maxima=(-MagickHuge);
        break;
      }
      case MinimumStatistic:
      {
        channel_statistics[i].minima=MagickHuge;
        break;
      }
    }
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      switch (type)
      {
        case MaximumStatistic:
        default:
        {
          if ((double) GetPixelRed(p) > channel_statistics[RedChannel].maxima)
            channel_statistics[RedChannel].maxima=(double) GetPixelRed(p);
          if ((double) GetPixelGreen(p) > channel_statistics[GreenChannel].maxima)
            channel_statistics[GreenChannel].maxima=(double) GetPixelGreen(p);
          if ((double) GetPixelBlue(p) > channel_statistics[BlueChannel].maxima)
            channel_statistics[BlueChannel].maxima=(double) GetPixelBlue(p);
          if ((image->matte != MagickFalse) &&
              ((double) GetPixelOpacity(p) > channel_statistics[OpacityChannel].maxima))
            channel_statistics[OpacityChannel].maxima=(double)
              GetPixelOpacity(p);
          if ((image->colorspace == CMYKColorspace) &&
              ((double) GetPixelIndex(indexes+x) > channel_statistics[BlackChannel].maxima))
            channel_statistics[BlackChannel].maxima=(double)
              GetPixelIndex(indexes+x);
          break;
        }
        case MinimumStatistic:
        {
          if ((double) GetPixelRed(p) < channel_statistics[RedChannel].minima)
            channel_statistics[RedChannel].minima=(double) GetPixelRed(p);
          if ((double) GetPixelGreen(p) < channel_statistics[GreenChannel].minima)
            channel_statistics[GreenChannel].minima=(double) GetPixelGreen(p);
          if ((double) GetPixelBlue(p) < channel_statistics[BlueChannel].minima)
            channel_statistics[BlueChannel].minima=(double) GetPixelBlue(p);
          if ((image->matte != MagickFalse) &&
              ((double) GetPixelOpacity(p) < channel_statistics[OpacityChannel].minima))
            channel_statistics[OpacityChannel].minima=(double)
              GetPixelOpacity(p);
          if ((image->colorspace == CMYKColorspace) &&
              ((double) GetPixelIndex(indexes+x) < channel_statistics[BlackChannel].minima))
            channel_statistics[BlackChannel].minima=(double)
              GetPixelIndex(indexes+x);
          break;
        }
      }
      p++;
    }
  }
  return(channel_statistics);
}

static ssize_t PrintChannelFeatures(FILE *file,const ChannelType channel,
  const char *name,const ChannelFeatures *channel_features)
{
#define PrintFeature(feature) \
  GetMagickPrecision(),(feature)[0], \
  GetMagickPrecision(),(feature)[1], \
  GetMagickPrecision(),(feature)[2], \
  GetMagickPrecision(),(feature)[3], \
  GetMagickPrecision(),((feature)[0]+(feature)[1]+(feature)[2]+(feature)[3])/4.0 \

#define FeaturesFormat "    %s:\n" \
  "      Angular Second Moment:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Contrast:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Correlation:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Sum of Squares: Variance:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Inverse Difference Moment:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Sum Average:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Sum Variance:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Sum Entropy:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Entropy:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Difference Variance:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Difference Entropy:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Information Measure of Correlation 1:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Information Measure of Correlation 2:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n" \
  "      Maximum Correlation Coefficient:\n" \
  "        %.*g, %.*g, %.*g, %.*g, %.*g\n"

  ssize_t
    n;

  n=FormatLocaleFile(file,FeaturesFormat,name,
    PrintFeature(channel_features[channel].angular_second_moment),
    PrintFeature(channel_features[channel].contrast),
    PrintFeature(channel_features[channel].correlation),
    PrintFeature(channel_features[channel].variance_sum_of_squares),
    PrintFeature(channel_features[channel].inverse_difference_moment),
    PrintFeature(channel_features[channel].sum_average),
    PrintFeature(channel_features[channel].sum_variance),
    PrintFeature(channel_features[channel].sum_entropy),
    PrintFeature(channel_features[channel].entropy),
    PrintFeature(channel_features[channel].difference_variance),
    PrintFeature(channel_features[channel].difference_entropy),
    PrintFeature(channel_features[channel].measure_of_correlation_1),
    PrintFeature(channel_features[channel].measure_of_correlation_2),
    PrintFeature(channel_features[channel].maximum_correlation_coefficient));
  return(n);
}

static ssize_t PrintChannelLocations(FILE *file,const Image *image,
  const ChannelType channel,const char *name,const StatisticType type,
  const size_t max_locations,const ChannelStatistics *channel_statistics)
{
  double
    target;

  ExceptionInfo
    *exception;

  ssize_t
    n,
    y;
  
  switch (type)
  {
    case MaximumStatistic:
    default:
    {
      target=channel_statistics[channel].maxima;
      break;
    }
    case MeanStatistic:
    {
      target=channel_statistics[channel].mean;
      break;
    }
    case MinimumStatistic:
    {
      target=channel_statistics[channel].minima;
      break;
    }
  }
  (void) FormatLocaleFile(file,"  %s: %.*g (%.*g)",name,GetMagickPrecision(),
    target,GetMagickPrecision(),QuantumScale*target);
  exception=AcquireExceptionInfo();
  n=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *p;

    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickBooleanType
        match;

      match=MagickFalse;
      switch (channel)
      {
        case RedChannel:
        {
          match=fabs((double) p->red-target) < 0.5 ? MagickTrue : MagickFalse;
          break;
        }
        case GreenChannel:
        {
          match=fabs((double) p->green-target) < 0.5 ? MagickTrue : MagickFalse;
          break;
        }
        case BlueChannel:
        {
          match=fabs((double) p->blue-target) < 0.5 ? MagickTrue : MagickFalse;
          break;
        }
        case AlphaChannel:
        {
          match=fabs((double) p->opacity-target) < 0.5 ? MagickTrue :
            MagickFalse;
          break;
        }
        default:
          break;
      }
      if (match != MagickFalse)
        {
          if ((max_locations != 0) && (n >= (ssize_t) max_locations))
            break;
          (void) FormatLocaleFile(file," %.20g,%.20g",(double) x,(double) y);
          n++;
        }
      p++;
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelMoments(FILE *file,const ChannelType channel,
  const char *name,const ChannelMoments *channel_moments)
{
  register ssize_t
    i;

  ssize_t
    n;

  n=FormatLocaleFile(file,"    %s:\n",name);
  n+=FormatLocaleFile(file,"      Centroid: %.*g,%.*g\n",
    GetMagickPrecision(),channel_moments[channel].centroid.x,
    GetMagickPrecision(),channel_moments[channel].centroid.y);
  n+=FormatLocaleFile(file,"      Ellipse Semi-Major/Minor axis: %.*g,%.*g\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_axis.x,
    GetMagickPrecision(),channel_moments[channel].ellipse_axis.y);
  n+=FormatLocaleFile(file,"      Ellipse angle: %.*g\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_angle);
  n+=FormatLocaleFile(file,"      Ellipse eccentricity: %.*g\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_eccentricity);
  n+=FormatLocaleFile(file,"      Ellipse intensity: %.*g\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_intensity);
  for (i=0; i < 8; i++)
    n+=FormatLocaleFile(file,"      I%.20g: %.*g\n",i+1.0,GetMagickPrecision(),
      channel_moments[channel].I[i]);
  return(n);
}

static ssize_t PrintChannelStatistics(FILE *file,const ChannelType channel,
  const char *name,const double scale,
  const ChannelStatistics *channel_statistics)
{
#define StatisticsFormat "    %s:\n      min: " QuantumFormat  \
  " (%g)\n      max: " QuantumFormat " (%g)\n"  \
  "      mean: %g (%g)\n      standard deviation: %g (%g)\n"  \
  "      kurtosis: %g\n      skewness: %g\n"

  ssize_t
    n;

  if (channel == AlphaChannel)
    {
      n=FormatLocaleFile(file,StatisticsFormat,name,ClampToQuantum(scale*
        (QuantumRange-channel_statistics[channel].maxima)),
        (QuantumRange-channel_statistics[channel].maxima)/(double) QuantumRange,
        ClampToQuantum(scale*(QuantumRange-channel_statistics[channel].minima)),
        (QuantumRange-channel_statistics[channel].minima)/(double) QuantumRange,
        scale*(QuantumRange-channel_statistics[channel].mean),(QuantumRange-
        channel_statistics[channel].mean)/(double) QuantumRange,scale*
        channel_statistics[channel].standard_deviation,
        channel_statistics[channel].standard_deviation/(double) QuantumRange,
        channel_statistics[channel].kurtosis,
        channel_statistics[channel].skewness);
      return(n);
    }
  n=FormatLocaleFile(file,StatisticsFormat,name,ClampToQuantum(scale*
    channel_statistics[channel].minima),channel_statistics[channel].minima/
    (double) QuantumRange,ClampToQuantum(scale*
    channel_statistics[channel].maxima),channel_statistics[channel].maxima/
    (double) QuantumRange,scale*channel_statistics[channel].mean,
    channel_statistics[channel].mean/(double) QuantumRange,scale*
    channel_statistics[channel].standard_deviation,
    channel_statistics[channel].standard_deviation/(double) QuantumRange,
    channel_statistics[channel].kurtosis,channel_statistics[channel].skewness);
  return(n);
}

MagickExport MagickBooleanType IdentifyImage(Image *image,FILE *file,
  const MagickBooleanType verbose)
{
  char
    color[MaxTextExtent],
    format[MaxTextExtent],
    key[MaxTextExtent];

  ChannelFeatures
    *channel_features;

  ChannelMoments
    *channel_moments;

  ChannelStatistics
    *channel_statistics;

  ColorspaceType
    colorspace;

  const char
    *artifact,
    *locate,
    *name,
    *property,
    *registry,
    *value;

  const MagickInfo
    *magick_info;

  const PixelPacket
    *pixels;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  ImageType
    type;

  MagickBooleanType
    ping;

  register ssize_t
    i,
    x;

  size_t
    distance,
    scale;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (file == (FILE *) NULL)
    file=stdout;
  exception=AcquireExceptionInfo();
  locate=GetImageArtifact(image,"identify:locate");
  if (locate != (const char *) NULL)
    {
      const char
        *limit;

      size_t
        max_locations;

      StatisticType
        type;

      /*
        Display minimum, maximum, or mean pixel locations.
      */
      type=(StatisticType) ParseCommandOption(MagickStatisticOptions,
        MagickFalse,locate);
      limit=GetImageArtifact(image,"identify:limit");
      max_locations=0;
      if (limit != (const char *) NULL)
        max_locations=StringToUnsignedLong(limit);
      channel_statistics=GetLocationStatistics(image,type,exception);
      if (channel_statistics == (ChannelStatistics *) NULL)
        return(MagickFalse);
      colorspace=image->colorspace;
      if (IsGrayImage(image,exception) != MagickFalse)
        colorspace=GRAYColorspace;
      (void) FormatLocaleFile(file,"Channel %s locations:\n",locate);
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelLocations(file,image,RedChannel,"Red",
            type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,GreenChannel,"Green",
            type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,BlueChannel,"Blue",
            type,max_locations,channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelLocations(file,image,CyanChannel,"Cyan",
            type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,MagentaChannel,"Magenta",
            type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,YellowChannel,"Yellow",
            type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,BlackChannel,"Black",
            type,max_locations,channel_statistics);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelLocations(file,image,GrayChannel,"Gray",
            type,max_locations,channel_statistics);
          break;
        }
      }
      if (image->matte != MagickFalse)
        (void) PrintChannelLocations(file,image,AlphaChannel,"Alpha",
          type,max_locations,channel_statistics);
      channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
        channel_statistics);
      return(ferror(file) != 0 ? MagickFalse : MagickTrue);
    }
  *format='\0';
  elapsed_time=GetElapsedTime(&image->timer);
  user_time=GetUserTime(&image->timer);
  GetTimerInfo(&image->timer);
  if (verbose == MagickFalse)
    {
      /*
        Display summary info about the image.
      */
      if (*image->magick_filename != '\0')
        if (LocaleCompare(image->magick_filename,image->filename) != 0)
          (void) FormatLocaleFile(file,"%s=>",image->magick_filename);
       if ((GetPreviousImageInList(image) == (Image *) NULL) &&
           (GetNextImageInList(image) == (Image *) NULL) &&
           (image->scene == 0))
        (void) FormatLocaleFile(file,"%s ",image->filename);
      else
        (void) FormatLocaleFile(file,"%s[%.20g] ",image->filename,(double)
          image->scene);
      (void) FormatLocaleFile(file,"%s ",image->magick);
      if ((image->magick_columns != 0) || (image->magick_rows != 0))
        if ((image->magick_columns != image->columns) ||
            (image->magick_rows != image->rows))
          (void) FormatLocaleFile(file,"%.20gx%.20g=>",(double)
            image->magick_columns,(double) image->magick_rows);
      (void) FormatLocaleFile(file,"%.20gx%.20g ",(double) image->columns,
        (double) image->rows);
      if ((image->page.width != 0) || (image->page.height != 0) ||
          (image->page.x != 0) || (image->page.y != 0))
        (void) FormatLocaleFile(file,"%.20gx%.20g%+.20g%+.20g ",(double)
          image->page.width,(double) image->page.height,(double) image->page.x,
          (double) image->page.y);
      (void) FormatLocaleFile(file,"%.20g-bit ",(double) image->depth);
      if (image->type != UndefinedType)
        (void) FormatLocaleFile(file,"%s ",CommandOptionToMnemonic(
          MagickTypeOptions,(ssize_t) image->type));
      if (image->colorspace != UndefinedColorspace)
        (void) FormatLocaleFile(file,"%s ",CommandOptionToMnemonic(
          MagickColorspaceOptions,(ssize_t) image->colorspace));
      if (image->storage_class == DirectClass)
        {
          if (image->total_colors != 0)
            {
              (void) FormatMagickSize(image->total_colors,MagickFalse,format);
              (void) FormatLocaleFile(file,"%s ",format);
            }
        }
      else
        if (image->total_colors <= image->colors)
          (void) FormatLocaleFile(file,"%.20gc ",(double)
            image->colors);
        else
          (void) FormatLocaleFile(file,"%.20g=>%.20gc ",(double)
            image->total_colors,(double) image->colors);
      if (image->error.mean_error_per_pixel != 0.0)
        (void) FormatLocaleFile(file,"%.20g/%f/%fdb ",(double)
          (image->error.mean_error_per_pixel+0.5),
          image->error.normalized_mean_error,
          image->error.normalized_maximum_error);
      if (GetBlobSize(image) != 0)
        {
          (void) FormatMagickSize(GetBlobSize(image),MagickFalse,format);
          (void) FormatLocaleFile(file,"%s ",format);
        }
      (void) FormatLocaleFile(file,"%0.3fu %lu:%02lu.%03lu",user_time,
        (unsigned long) (elapsed_time/60.0),(unsigned long) floor(fmod(
        elapsed_time,60.0)),(unsigned long) (1000.0*(elapsed_time-
        floor(elapsed_time))));
      (void) FormatLocaleFile(file,"\n");
      (void) fflush(file);
      return(ferror(file) != 0 ? MagickFalse : MagickTrue);
    }
  /*
    Display verbose info about the image.
  */
  pixels=GetVirtualPixels(image,0,0,1,1,exception);
  exception=DestroyExceptionInfo(exception);
  ping=pixels == (const PixelPacket *) NULL ? MagickTrue : MagickFalse;
  type=GetImageType(image,&image->exception);
  (void) SignatureImage(image);
  (void) FormatLocaleFile(file,"Image: %s\n",image->filename);
  if (*image->magick_filename != '\0')
    if (LocaleCompare(image->magick_filename,image->filename) != 0)
      {
        char
          filename[MaxTextExtent];

        GetPathComponent(image->magick_filename,TailPath,filename);
        (void) FormatLocaleFile(file,"  Base filename: %s\n",filename);
      }
  magick_info=GetMagickInfo(image->magick,&image->exception);
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetMagickDescription(magick_info) == (const char *) NULL))
    (void) FormatLocaleFile(file,"  Format: %s\n",image->magick);
  else
    (void) FormatLocaleFile(file,"  Format: %s (%s)\n",image->magick,
      GetMagickDescription(magick_info));
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetMagickMimeType(magick_info) != (const char *) NULL))
    (void) FormatLocaleFile(file,"  Mime type: %s\n",GetMagickMimeType(
      magick_info));
  (void) FormatLocaleFile(file,"  Class: %s\n",CommandOptionToMnemonic(
    MagickClassOptions,(ssize_t) image->storage_class));
  (void) FormatLocaleFile(file,"  Geometry: %.20gx%.20g%+.20g%+.20g\n",(double)
    image->columns,(double) image->rows,(double) image->tile_offset.x,(double)
    image->tile_offset.y);
  if ((image->magick_columns != 0) || (image->magick_rows != 0))
    if ((image->magick_columns != image->columns) ||
        (image->magick_rows != image->rows))
      (void) FormatLocaleFile(file,"  Base geometry: %.20gx%.20g\n",(double)
        image->magick_columns,(double) image->magick_rows);
  if ((image->x_resolution != 0.0) && (image->y_resolution != 0.0))
    {
      (void) FormatLocaleFile(file,"  Resolution: %gx%g\n",image->x_resolution,
        image->y_resolution);
      (void) FormatLocaleFile(file,"  Print size: %gx%g\n",(double)
        image->columns/image->x_resolution,(double) image->rows/
        image->y_resolution);
    }
  (void) FormatLocaleFile(file,"  Units: %s\n",CommandOptionToMnemonic(
    MagickResolutionOptions,(ssize_t) image->units));
  (void) FormatLocaleFile(file,"  Type: %s\n",CommandOptionToMnemonic(
    MagickTypeOptions,(ssize_t) type));
  if (image->type != UndefinedType)
    (void) FormatLocaleFile(file,"  Base type: %s\n",CommandOptionToMnemonic(
      MagickTypeOptions,(ssize_t) image->type));
  (void) FormatLocaleFile(file,"  Endianess: %s\n",CommandOptionToMnemonic(
    MagickEndianOptions,(ssize_t) image->endian));
  /*
    Detail channel depth and extrema.
  */
  (void) FormatLocaleFile(file,"  Colorspace: %s\n",CommandOptionToMnemonic(
    MagickColorspaceOptions,(ssize_t) image->colorspace));
  channel_statistics=(ChannelStatistics *) NULL;
  channel_moments=(ChannelMoments *) NULL;
  channel_features=(ChannelFeatures *) NULL;
  colorspace=image->colorspace;
  scale=1;
  if (ping == MagickFalse)
    {
      size_t
        depth;

      channel_statistics=GetImageChannelStatistics(image,exception);
      if (channel_statistics == (ChannelStatistics *) NULL)
        return(MagickFalse);
      artifact=GetImageArtifact(image,"identify:moments");
      if (artifact != (const char *) NULL)
        channel_moments=GetImageChannelMoments(image,exception);
      artifact=GetImageArtifact(image,"identify:features");
      if (artifact != (const char *) NULL)
        {
          distance=StringToUnsignedLong(artifact);
          channel_features=GetImageChannelFeatures(image,distance,exception);
        }
      depth=GetImageDepth(image,&image->exception);
      if (image->depth == depth)
        (void) FormatLocaleFile(file,"  Depth: %.20g-bit\n",(double)
          image->depth);
      else
        (void) FormatLocaleFile(file,"  Depth: %.20g/%.20g-bit\n",(double)
          image->depth,(double) depth);
      (void) FormatLocaleFile(file,"  Channel depth:\n");
      if (IsGrayImage(image,&image->exception) != MagickFalse)
        colorspace=GRAYColorspace;
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) FormatLocaleFile(file,"    red: %.20g-bit\n",(double)
            channel_statistics[RedChannel].depth);
          (void) FormatLocaleFile(file,"    green: %.20g-bit\n",(double)
            channel_statistics[GreenChannel].depth);
          (void) FormatLocaleFile(file,"    blue: %.20g-bit\n",(double)
            channel_statistics[BlueChannel].depth);
          break;
        }
        case CMYKColorspace:
        {
          (void) FormatLocaleFile(file,"    cyan: %.20g-bit\n",(double)
            channel_statistics[CyanChannel].depth);
          (void) FormatLocaleFile(file,"    magenta: %.20g-bit\n",(double)
            channel_statistics[MagentaChannel].depth);
          (void) FormatLocaleFile(file,"    yellow: %.20g-bit\n",(double)
            channel_statistics[YellowChannel].depth);
          (void) FormatLocaleFile(file,"    black: %.20g-bit\n",(double)
            channel_statistics[BlackChannel].depth);
          break;
        }
        case GRAYColorspace:
        {
          (void) FormatLocaleFile(file,"    gray: %.20g-bit\n",(double)
            channel_statistics[GrayChannel].depth);
          break;
        }
      }
      if (image->matte != MagickFalse)
        (void) FormatLocaleFile(file,"    alpha: %.20g-bit\n",(double)
          channel_statistics[OpacityChannel].depth);
      scale=1;
      if (image->depth <= MAGICKCORE_QUANTUM_DEPTH)
        scale=QuantumRange/((size_t) QuantumRange >> ((size_t)
          MAGICKCORE_QUANTUM_DEPTH-image->depth));
    }
  if (channel_statistics != (ChannelStatistics *) NULL)
    {
      (void) FormatLocaleFile(file,"  Channel statistics:\n");
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelStatistics(file,RedChannel,"Red",1.0/scale,
            channel_statistics);
          (void) PrintChannelStatistics(file,GreenChannel,"Green",1.0/scale,
            channel_statistics);
          (void) PrintChannelStatistics(file,BlueChannel,"Blue",1.0/scale,
            channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelStatistics(file,CyanChannel,"Cyan",1.0/scale,
            channel_statistics);
          (void) PrintChannelStatistics(file,MagentaChannel,"Magenta",1.0/scale,
            channel_statistics);
          (void) PrintChannelStatistics(file,YellowChannel,"Yellow",1.0/scale,
            channel_statistics);
          (void) PrintChannelStatistics(file,BlackChannel,"Black",1.0/scale,
            channel_statistics);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelStatistics(file,GrayChannel,"Gray",1.0/scale,
            channel_statistics);
          break;
        }
      }
      if (image->matte != MagickFalse)
        (void) PrintChannelStatistics(file,AlphaChannel,"Alpha",1.0/scale,
          channel_statistics);
      if (colorspace != GRAYColorspace)
        {
          (void) FormatLocaleFile(file,"  Image statistics:\n");
          (void) PrintChannelStatistics(file,CompositeChannels,"Overall",1.0/
            scale,channel_statistics);
        }
      channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
        channel_statistics);
    }
  if (channel_moments != (ChannelMoments *) NULL)
    {
      (void) FormatLocaleFile(file,"  Channel moments:\n");
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelMoments(file,RedChannel,"Red",channel_moments);
          (void) PrintChannelMoments(file,GreenChannel,"Green",channel_moments);
          (void) PrintChannelMoments(file,BlueChannel,"Blue",channel_moments);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelMoments(file,CyanChannel,"Cyan",channel_moments);
          (void) PrintChannelMoments(file,MagentaChannel,"Magenta",
            channel_moments);
          (void) PrintChannelMoments(file,YellowChannel,"Yellow",
            channel_moments);
          (void) PrintChannelMoments(file,BlackChannel,"Black",channel_moments);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelMoments(file,GrayChannel,"Gray",channel_moments);
          break;
        }
      }
      if (image->matte != MagickFalse)
        (void) PrintChannelMoments(file,AlphaChannel,"Alpha",channel_moments);
      channel_moments=(ChannelMoments *) RelinquishMagickMemory(
        channel_moments);
    }
  if (channel_features != (ChannelFeatures *) NULL)
    {
      (void) FormatLocaleFile(file,"  Channel features (horizontal, vertical, "
        "left and right diagonals, average):\n");
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelFeatures(file,RedChannel,"Red",channel_features);
          (void) PrintChannelFeatures(file,GreenChannel,"Green",
            channel_features);
          (void) PrintChannelFeatures(file,BlueChannel,"Blue",channel_features);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelFeatures(file,CyanChannel,"Cyan",channel_features);
          (void) PrintChannelFeatures(file,MagentaChannel,"Magenta",
            channel_features);
          (void) PrintChannelFeatures(file,YellowChannel,"Yellow",
            channel_features);
          (void) PrintChannelFeatures(file,BlackChannel,"Black",
            channel_features);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelFeatures(file,GrayChannel,"Gray",channel_features);
          break;
        }
      }
      if (image->matte != MagickFalse)
        (void) PrintChannelFeatures(file,AlphaChannel,"Alpha",channel_features);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
    }
  if (ping == MagickFalse)
    {
      if (image->colorspace == CMYKColorspace)
        (void) FormatLocaleFile(file,"  Total ink density: %.0f%%\n",100.0*
          GetImageTotalInkDensity(image)/(double) QuantumRange);
      x=0;
      if (image->matte != MagickFalse)
        {
          register const IndexPacket
            *indexes;

          register const PixelPacket
            *p;

          p=(PixelPacket *) NULL;
          indexes=(IndexPacket *) NULL;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);
            if (p == (const PixelPacket *) NULL)
              break;
            indexes=GetVirtualIndexQueue(image);
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelOpacity(p) == (Quantum) TransparentOpacity)
                break;
              p++;
            }
            if (x < (ssize_t) image->columns)
              break;
          }
          if ((x < (ssize_t) image->columns) || (y < (ssize_t) image->rows))
            {
              char
                tuple[MaxTextExtent];

              MagickPixelPacket
                pixel;

              GetMagickPixelPacket(image,&pixel);
              SetMagickPixelPacket(image,p,indexes+x,&pixel);
              (void) QueryMagickColorname(image,&pixel,SVGCompliance,tuple,
                &image->exception);
              (void) FormatLocaleFile(file,"  Alpha: %s ",tuple);
              GetColorTuple(&pixel,MagickTrue,tuple);
              (void) FormatLocaleFile(file,"  %s\n",tuple);
            }
        }
      artifact=GetImageArtifact(image,"identify:unique-colors");
      if (IsHistogramImage(image,&image->exception) != MagickFalse)
        {
          (void) FormatLocaleFile(file,"  Colors: %.20g\n",(double)
            GetNumberColors(image,(FILE *) NULL,&image->exception));
          (void) FormatLocaleFile(file,"  Histogram:\n");
          (void) GetNumberColors(image,file,&image->exception);
        }
      else
        if ((artifact != (const char *) NULL) &&
            (IsMagickTrue(artifact) != MagickFalse))
          (void) FormatLocaleFile(file,"  Colors: %.20g\n",(double)
            GetNumberColors(image,(FILE *) NULL,&image->exception));
    }
  if (image->storage_class == PseudoClass)
    {
      (void) FormatLocaleFile(file,"  Colormap entries: %.20g\n",(double)
        image->colors);
      (void) FormatLocaleFile(file,"  Colormap:\n");
      if (image->colors <= 1024)
        {
          char
            color[MaxTextExtent],
            hex[MaxTextExtent],
            tuple[MaxTextExtent];

          MagickPixelPacket
            pixel;

          register PixelPacket
            *restrict p;

          GetMagickPixelPacket(image,&pixel);
          p=image->colormap;
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            SetMagickPixelPacket(image,p,(IndexPacket *) NULL,&pixel);
            (void) CopyMagickString(tuple,"(",MaxTextExtent);
            ConcatenateColorComponent(&pixel,RedChannel,X11Compliance,tuple);
            (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
            ConcatenateColorComponent(&pixel,GreenChannel,X11Compliance,tuple);
            (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
            ConcatenateColorComponent(&pixel,BlueChannel,X11Compliance,tuple);
            if (pixel.colorspace == CMYKColorspace)
              {
                (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
                ConcatenateColorComponent(&pixel,IndexChannel,X11Compliance,
                  tuple);
              }
            if (pixel.matte != MagickFalse)
              {
                (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
                ConcatenateColorComponent(&pixel,AlphaChannel,X11Compliance,
                  tuple);
              }
            (void) ConcatenateMagickString(tuple,")",MaxTextExtent);
            (void) QueryMagickColorname(image,&pixel,SVGCompliance,color,
              &image->exception);
            GetColorTuple(&pixel,MagickTrue,hex);
            (void) FormatLocaleFile(file,"  %8ld: %s %s %s\n",(long) i,tuple,
              hex,color);
            p++;
          }
        }
    }
  if (image->error.mean_error_per_pixel != 0.0)
    (void) FormatLocaleFile(file,"  Mean error per pixel: %g\n",
      image->error.mean_error_per_pixel);
  if (image->error.normalized_mean_error != 0.0)
    (void) FormatLocaleFile(file,"  Normalized mean error: %g\n",
      image->error.normalized_mean_error);
  if (image->error.normalized_maximum_error != 0.0)
    (void) FormatLocaleFile(file,"  Normalized maximum error: %g\n",
      image->error.normalized_maximum_error);
  (void) FormatLocaleFile(file,"  Rendering intent: %s\n",
    CommandOptionToMnemonic(MagickIntentOptions,(ssize_t)
    image->rendering_intent));
  if (image->gamma != 0.0)
    (void) FormatLocaleFile(file,"  Gamma: %g\n",image->gamma);
  if ((image->chromaticity.red_primary.x != 0.0) ||
      (image->chromaticity.green_primary.x != 0.0) ||
      (image->chromaticity.blue_primary.x != 0.0) ||
      (image->chromaticity.white_point.x != 0.0))
    {
      /*
        Display image chromaticity.
      */
      (void) FormatLocaleFile(file,"  Chromaticity:\n");
      (void) FormatLocaleFile(file,"    red primary: (%g,%g)\n",
        image->chromaticity.red_primary.x,image->chromaticity.red_primary.y);
      (void) FormatLocaleFile(file,"    green primary: (%g,%g)\n",
        image->chromaticity.green_primary.x,
        image->chromaticity.green_primary.y);
      (void) FormatLocaleFile(file,"    blue primary: (%g,%g)\n",
        image->chromaticity.blue_primary.x,image->chromaticity.blue_primary.y);
      (void) FormatLocaleFile(file,"    white point: (%g,%g)\n",
        image->chromaticity.white_point.x,image->chromaticity.white_point.y);
    }
  if ((image->extract_info.width*image->extract_info.height) != 0)
    (void) FormatLocaleFile(file,"  Tile geometry: %.20gx%.20g%+.20g%+.20g\n",
      (double) image->extract_info.width,(double) image->extract_info.height,
      (double) image->extract_info.x,(double) image->extract_info.y);
  (void) QueryColorname(image,&image->background_color,SVGCompliance,color,
    &image->exception);
  (void) FormatLocaleFile(file,"  Background color: %s\n",color);
  (void) QueryColorname(image,&image->border_color,SVGCompliance,color,
    &image->exception);
  (void) FormatLocaleFile(file,"  Border color: %s\n",color);
  (void) QueryColorname(image,&image->matte_color,SVGCompliance,color,
    &image->exception);
  (void) FormatLocaleFile(file,"  Matte color: %s\n",color);
  (void) QueryColorname(image,&image->transparent_color,SVGCompliance,color,
    &image->exception);
  (void) FormatLocaleFile(file,"  Transparent color: %s\n",color);
  (void) FormatLocaleFile(file,"  Interlace: %s\n",CommandOptionToMnemonic(
    MagickInterlaceOptions,(ssize_t) image->interlace));
  (void) FormatLocaleFile(file,"  Intensity: %s\n",CommandOptionToMnemonic(
    MagickPixelIntensityOptions,(ssize_t) image->intensity));
  (void) FormatLocaleFile(file,"  Compose: %s\n",CommandOptionToMnemonic(
    MagickComposeOptions,(ssize_t) image->compose));
  if ((image->page.width != 0) || (image->page.height != 0) ||
      (image->page.x != 0) || (image->page.y != 0))
    (void) FormatLocaleFile(file,"  Page geometry: %.20gx%.20g%+.20g%+.20g\n",
      (double) image->page.width,(double) image->page.height,(double)
      image->page.x,(double) image->page.y);
  if ((image->page.x != 0) || (image->page.y != 0))
    (void) FormatLocaleFile(file,"  Origin geometry: %+.20g%+.20g\n",(double)
      image->page.x,(double) image->page.y);
  (void) FormatLocaleFile(file,"  Dispose: %s\n",CommandOptionToMnemonic(
    MagickDisposeOptions,(ssize_t) image->dispose));
  if (image->delay != 0)
    (void) FormatLocaleFile(file,"  Delay: %.20gx%.20g\n",(double) image->delay,
      (double) image->ticks_per_second);
  if (image->iterations != 1)
    (void) FormatLocaleFile(file,"  Iterations: %.20g\n",(double)
      image->iterations);
  if ((image->next != (Image *) NULL) || (image->previous != (Image *) NULL))
    (void) FormatLocaleFile(file,"  Scene: %.20g of %.20g\n",(double)
      image->scene,(double) GetImageListLength(image));
  else
    if (image->scene != 0)
      (void) FormatLocaleFile(file,"  Scene: %.20g\n",(double) image->scene);
  (void) FormatLocaleFile(file,"  Compression: %s\n",CommandOptionToMnemonic(
    MagickCompressOptions,(ssize_t) image->compression));
  if (image->quality != UndefinedCompressionQuality)
    (void) FormatLocaleFile(file,"  Quality: %.20g\n",(double) image->quality);
  (void) FormatLocaleFile(file,"  Orientation: %s\n",CommandOptionToMnemonic(
    MagickOrientationOptions,(ssize_t) image->orientation));
  if (image->montage != (char *) NULL)
    (void) FormatLocaleFile(file,"  Montage: %s\n",image->montage);
  if (image->directory != (char *) NULL)
    {
      Image
        *tile;

      ImageInfo
        *image_info;

      register char
        *p,
        *q;

      WarningHandler
        handler;

      /*
        Display visual image directory.
      */
      image_info=AcquireImageInfo();
      (void) CloneString(&image_info->size,"64x64");
      (void) FormatLocaleFile(file,"  Directory:\n");
      for (p=image->directory; *p != '\0'; p++)
      {
        q=p;
        while ((*q != '\n') && (*q != '\0'))
          q++;
        (void) CopyMagickString(image_info->filename,p,(size_t) (q-p+1));
        p=q;
        (void) FormatLocaleFile(file,"    %s",image_info->filename);
        handler=SetWarningHandler((WarningHandler) NULL);
        tile=ReadImage(image_info,&image->exception);
        (void) SetWarningHandler(handler);
        if (tile == (Image *) NULL)
          {
            (void) FormatLocaleFile(file,"\n");
            continue;
          }
        (void) FormatLocaleFile(file," %.20gx%.20g %s\n",(double)
          tile->magick_columns,(double) tile->magick_rows,tile->magick);
        (void) SignatureImage(tile);
        ResetImagePropertyIterator(tile);
        property=GetNextImageProperty(tile);
        while (property != (const char *) NULL)
        {
          (void) FormatLocaleFile(file,"  %s:\n",property);
          value=GetImageProperty(tile,property);
          if (value != (const char *) NULL)
            (void) FormatLocaleFile(file,"%s\n",value);
          property=GetNextImageProperty(tile);
        }
        tile=DestroyImage(tile);
      }
      image_info=DestroyImageInfo(image_info);
    }
  (void) GetImageProperty(image,"exif:*");
  ResetImagePropertyIterator(image);
  property=GetNextImageProperty(image);
  if (property != (const char *) NULL)
    {
      /*
        Display image properties.
      */
      (void) FormatLocaleFile(file,"  Properties:\n");
      while (property != (const char *) NULL)
      {
        (void) FormatLocaleFile(file,"    %s: ",property);
        value=GetImageProperty(image,property);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"%s\n",value);
        property=GetNextImageProperty(image);
      }
    }
  (void) FormatLocaleString(key,MaxTextExtent,"8BIM:1999,2998:#1");
  value=GetImageProperty(image,key);
  if (value != (const char *) NULL)
    {
      /*
        Display clipping path.
      */
      (void) FormatLocaleFile(file,"  Clipping path: ");
      if (strlen(value) > 80)
        (void) fputc('\n',file);
      (void) FormatLocaleFile(file,"%s\n",value);
    }
  ResetImageProfileIterator(image);
  name=GetNextImageProfile(image);
  if (name != (char *) NULL)
    {
      const StringInfo
        *profile;

      /*
        Identify image profiles.
      */
      (void) FormatLocaleFile(file,"  Profiles:\n");
      while (name != (char *) NULL)
      {
        profile=GetImageProfile(image,name);
        if (profile == (StringInfo *) NULL)
          continue;
        (void) FormatLocaleFile(file,"    Profile-%s: %.20g bytes\n",name,
          (double) GetStringInfoLength(profile));
#if defined(MAGICKCORE_LCMS_DELEGATE)
        if ((LocaleCompare(name,"icc") == 0) ||
            (LocaleCompare(name,"icm") == 0))
          {
            cmsHPROFILE
              icc_profile;

            icc_profile=cmsOpenProfileFromMem(GetStringInfoDatum(profile),
              (cmsUInt32Number) GetStringInfoLength(profile));
            if (icc_profile != (cmsHPROFILE *) NULL)
              {
#if defined(LCMS_VERSION) && (LCMS_VERSION < 2000)
                const char
                  *name;

                name=cmsTakeProductName(icc_profile);
                if (name != (const char *) NULL)
                  (void) FormatLocaleFile(file,"      %s\n",name);
#else
                char
                  info[MaxTextExtent];

                (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoDescription,
                  "en","US",info,MaxTextExtent);
                (void) FormatLocaleFile(file,"      Description: %s\n",info);
                (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoManufacturer,
                  "en","US",info,MaxTextExtent);
                (void) FormatLocaleFile(file,"      Manufacturer: %s\n",info);
                (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoModel,"en",
                  "US",info,MaxTextExtent);
                (void) FormatLocaleFile(file,"      Model: %s\n",info);
                (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoCopyright,
                  "en","US",info,MaxTextExtent);
                (void) FormatLocaleFile(file,"      Copyright: %s\n",info);
#endif
                (void) cmsCloseProfile(icc_profile);
              }
          }
#endif
        if (LocaleCompare(name,"iptc") == 0)
          {
            char
              *attribute,
              **attribute_list;

            const char
              *tag;

            long
              dataset,
              record,
              sentinel;

            register ssize_t
              j;

            size_t
              length,
              profile_length;

            profile_length=GetStringInfoLength(profile);
            for (i=0; i < (ssize_t) profile_length; i+=(ssize_t) length)
            {
              length=1;
              sentinel=GetStringInfoDatum(profile)[i++];
              if (sentinel != 0x1c)
                continue;
              dataset=GetStringInfoDatum(profile)[i++];
              record=GetStringInfoDatum(profile)[i++];
              switch (record)
              {
                case 5: tag="Image Name"; break;
                case 7: tag="Edit Status"; break;
                case 10: tag="Priority"; break;
                case 15: tag="Category"; break;
                case 20: tag="Supplemental Category"; break;
                case 22: tag="Fixture Identifier"; break;
                case 25: tag="Keyword"; break;
                case 30: tag="Release Date"; break;
                case 35: tag="Release Time"; break;
                case 40: tag="Special Instructions"; break;
                case 45: tag="Reference Service"; break;
                case 47: tag="Reference Date"; break;
                case 50: tag="Reference Number"; break;
                case 55: tag="Created Date"; break;
                case 60: tag="Created Time"; break;
                case 65: tag="Originating Program"; break;
                case 70: tag="Program Version"; break;
                case 75: tag="Object Cycle"; break;
                case 80: tag="Byline"; break;
                case 85: tag="Byline Title"; break;
                case 90: tag="City"; break;
                case 95: tag="Province State"; break;
                case 100: tag="Country Code"; break;
                case 101: tag="Country"; break;
                case 103: tag="Original Transmission Reference"; break;
                case 105: tag="Headline"; break;
                case 110: tag="Credit"; break;
                case 115: tag="Src"; break;
                case 116: tag="Copyright String"; break;
                case 120: tag="Caption"; break;
                case 121: tag="Local Caption"; break;
                case 122: tag="Caption Writer"; break;
                case 200: tag="Custom Field 1"; break;
                case 201: tag="Custom Field 2"; break;
                case 202: tag="Custom Field 3"; break;
                case 203: tag="Custom Field 4"; break;
                case 204: tag="Custom Field 5"; break;
                case 205: tag="Custom Field 6"; break;
                case 206: tag="Custom Field 7"; break;
                case 207: tag="Custom Field 8"; break;
                case 208: tag="Custom Field 9"; break;
                case 209: tag="Custom Field 10"; break;
                case 210: tag="Custom Field 11"; break;
                case 211: tag="Custom Field 12"; break;
                case 212: tag="Custom Field 13"; break;
                case 213: tag="Custom Field 14"; break;
                case 214: tag="Custom Field 15"; break;
                case 215: tag="Custom Field 16"; break;
                case 216: tag="Custom Field 17"; break;
                case 217: tag="Custom Field 18"; break;
                case 218: tag="Custom Field 19"; break;
                case 219: tag="Custom Field 20"; break;
                default: tag="unknown"; break;
              }
              (void) FormatLocaleFile(file,"      %s[%.20g,%.20g]: ",tag,
                (double) dataset,(double) record);
              length=(size_t) (GetStringInfoDatum(profile)[i++] << 8);
              length|=GetStringInfoDatum(profile)[i++];
              attribute=(char *) NULL;
              if (~length >= (MaxTextExtent-1))
                attribute=(char *) AcquireQuantumMemory(length+
                  MaxTextExtent,sizeof(*attribute));
              if (attribute != (char *) NULL)
                {
                  (void) CopyMagickString(attribute,(char *)
                    GetStringInfoDatum(profile)+i,length+1);
                  attribute_list=StringToList(attribute);
                  if (attribute_list != (char **) NULL)
                    {
                      for (j=0; attribute_list[j] != (char *) NULL; j++)
                      {
                        (void) fputs(attribute_list[j],file);
                        (void) fputs("\n",file);
                        attribute_list[j]=(char *) RelinquishMagickMemory(
                          attribute_list[j]);
                      }
                      attribute_list=(char **) RelinquishMagickMemory(
                        attribute_list);
                    }
                  attribute=DestroyString(attribute);
                }
            }
          }
        if (image->debug != MagickFalse)
          PrintStringInfo(file,name,profile);
        name=GetNextImageProfile(image);
      }
    }
  ResetImageArtifactIterator(image);
  artifact=GetNextImageArtifact(image);
  if (artifact != (const char *) NULL)
    {
      /*
        Display image artifacts.
      */
      (void) FormatLocaleFile(file,"  Artifacts:\n");
      while (artifact != (const char *) NULL)
      {
        (void) FormatLocaleFile(file,"    %s: ",artifact);
        value=GetImageArtifact(image,artifact);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"%s\n",value);
        artifact=GetNextImageArtifact(image);
      }
    }
  ResetImageRegistryIterator();
  registry=GetNextImageRegistry();
  if (registry != (const char *) NULL)
    {
      /*
        Display image registry.
      */
      (void) FormatLocaleFile(file,"  Registry:\n");
      while (registry != (const char *) NULL)
      {
        (void) FormatLocaleFile(file,"    %s: ",registry);
        value=(const char *) GetImageRegistry(StringRegistryType,registry,
          &image->exception);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"%s\n",value);
        registry=GetNextImageRegistry();
      }
    }
  (void) FormatLocaleFile(file,"  Tainted: %s\n",CommandOptionToMnemonic(
    MagickBooleanOptions,(ssize_t) image->taint));
  (void) FormatMagickSize(GetBlobSize(image),MagickFalse,format);
  (void) FormatLocaleFile(file,"  Filesize: %s\n",format);
  (void) FormatMagickSize((MagickSizeType) image->columns*image->rows,
     MagickFalse,format);
  if (strlen(format) > 1)
    format[strlen(format)-1]='\0';
  (void) FormatLocaleFile(file,"  Number pixels: %s\n",format);
  (void) FormatMagickSize((MagickSizeType) ((double) image->columns*image->rows/
    elapsed_time+0.5),MagickFalse,format);
  (void) FormatLocaleFile(file,"  Pixels per second: %s\n",format);
  (void) FormatLocaleFile(file,"  User time: %0.3fu\n",user_time);
  (void) FormatLocaleFile(file,"  Elapsed time: %lu:%02lu.%03lu\n",
    (unsigned long) (elapsed_time/60.0),(unsigned long) ceil(fmod(
    elapsed_time,60.0)),(unsigned long) (1000.0*(elapsed_time-floor(
    elapsed_time))));
  (void) FormatLocaleFile(file,"  Version: %s\n",GetMagickVersion((size_t *)
    NULL));
  (void) fflush(file);
  return(ferror(file) != 0 ? MagickFalse : MagickTrue);
}

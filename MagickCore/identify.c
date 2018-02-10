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
%  Identify describes the format and characteristics of one or more image
%  files.  It will also report if an image is incomplete or corrupt.
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache.h"
#include "MagickCore/client.h"
#include "MagickCore/coder.h"
#include "MagickCore/color.h"
#include "MagickCore/configure.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/delegate.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/feature.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/identify.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/magic.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/montage.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/prepress.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/random_.h"
#include "MagickCore/registry.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/signature.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/timer.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"

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
%        const MagickBooleanType verbose,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/

static ChannelStatistics *GetLocationStatistics(const Image *image,
  const StatisticType type,ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  register ssize_t
    i;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(
    MaxPixelChannels+1,sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_statistics,0,(MaxPixelChannels+1)*
    sizeof(*channel_statistics));
  for (i=0; i <= (ssize_t) MaxPixelChannels; i++)
  {
    switch (type)
    {
      case MaximumStatistic:
      default:
      {
        channel_statistics[i].maxima=(-MagickMaximumValue);
        break;
      }
      case MinimumStatistic:
      {
        channel_statistics[i].minima=MagickMaximumValue;
        break;
      }
    }
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelReadMask(image,p) <= (QuantumRange/2))
        {
          p+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        switch (type)
        {
          case MaximumStatistic:
          default:
          {
            if ((double) p[i] > channel_statistics[channel].maxima)
              channel_statistics[channel].maxima=(double) p[i];
            break;
          }
          case MinimumStatistic:
          {
            if ((double) p[i] < channel_statistics[channel].minima)
              channel_statistics[channel].minima=(double) p[i];
            break;
          }
        }
      }
      p+=GetPixelChannels(image);
    }
  }
  return(channel_statistics);
}

static ssize_t PrintChannelFeatures(FILE *file,const PixelChannel channel,
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
  "      Sum of Squares Variance:\n" \
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
  const PixelChannel channel,const char *name,const StatisticType type,
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
    register const Quantum
      *p;

    ssize_t
      offset,
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickBooleanType
        match;

      PixelTrait traits = GetPixelChannelTraits(image,channel);
      if (traits == UndefinedPixelTrait)
        continue;
      offset=GetPixelChannelOffset(image,channel);
      match=fabs((double) (p[offset]-target)) < 0.5 ? MagickTrue : MagickFalse;
      if (match != MagickFalse)
        {
          if ((max_locations != 0) && (n >= (ssize_t) max_locations))
            break;
          (void) FormatLocaleFile(file," %.20g,%.20g",(double) x,(double) y);
          n++;
        }
      p+=GetPixelChannels(image);
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelMoments(FILE *file,const PixelChannel channel,
  const char *name,const double scale,const ChannelMoments *channel_moments)
{
  double
    powers[MaximumNumberOfImageMoments] =
      { 1.0, 2.0, 3.0, 3.0, 6.0, 4.0, 6.0, 4.0 };

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
  n+=FormatLocaleFile(file,"      Ellipse intensity: %.*g (%.*g)\n",
    GetMagickPrecision(),pow(scale,powers[0])*
    channel_moments[channel].ellipse_intensity,GetMagickPrecision(),
    channel_moments[channel].ellipse_intensity);
  for (i=0; i < MaximumNumberOfImageMoments; i++)
    n+=FormatLocaleFile(file,"      I%.20g: %.*g (%.*g)\n",i+1.0,
      GetMagickPrecision(),channel_moments[channel].invariant[i]/pow(scale,
      powers[i]),GetMagickPrecision(),channel_moments[channel].invariant[i]);
  return(n);
}

static ssize_t PrintChannelPerceptualHash(Image *image,FILE *file,
  const ChannelPerceptualHash *channel_phash)
{
  register ssize_t
    i;

  ssize_t
    n;

  (void) FormatLocaleFile(file,"  Channel perceptual hash: ");
  for (i=0; i < (ssize_t) channel_phash[0].number_colorspaces; i++)
  {
    (void) FormatLocaleFile(file,"%s",CommandOptionToMnemonic(
      MagickColorspaceOptions,(ssize_t) channel_phash[0].colorspace[i]));
    if (i < (ssize_t) (channel_phash[0].number_colorspaces-1))
      (void) FormatLocaleFile(file,", ");
  }
  (void) FormatLocaleFile(file,"\n");
  n=0;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel
      channel;

    PixelTrait
      traits;

    register ssize_t
      j;

    channel=GetPixelChannelChannel(image,i);
    if (channel == IndexPixelChannel)
      continue;
    traits=GetPixelChannelTraits(image,channel);
    if (traits == UndefinedPixelTrait)
      continue;
    n=FormatLocaleFile(file,"    Channel %.20g:\n",(double) channel);
    for (j=0; j < MaximumNumberOfPerceptualHashes; j++)
    {
      register ssize_t
        k;

      n+=FormatLocaleFile(file,"      PH%.20g: ",(double) j+1);
      for (k=0; k < (ssize_t) channel_phash[0].number_colorspaces; k++)
      {
        n+=FormatLocaleFile(file,"%.*g",GetMagickPrecision(),
          channel_phash[channel].phash[k][j]);
        if (k < (ssize_t) (channel_phash[0].number_colorspaces-1))
          n+=FormatLocaleFile(file,", ");
      }
      n+=FormatLocaleFile(file,"\n");
    }
  }
  return(n);
}

static ssize_t PrintChannelStatistics(FILE *file,const PixelChannel channel,
  const char *name,const double scale,
  const ChannelStatistics *channel_statistics)
{
#define StatisticsFormat "    %s:\n      min: %.*g  (%.*g)\n      " \
  "max: %.*g (%.*g)\n      mean: %.*g (%.*g)\n      " \
  "standard deviation: %.*g (%.*g)\n      kurtosis: %.*g\n      " \
  "skewness: %.*g\n      entropy: %.*g\n"

  ssize_t
    n;

  n=FormatLocaleFile(file,StatisticsFormat,name,GetMagickPrecision(),
    (double) ClampToQuantum((MagickRealType) (scale*
    channel_statistics[channel].minima)),GetMagickPrecision(),
    channel_statistics[channel].minima/(double) QuantumRange,
    GetMagickPrecision(),(double) ClampToQuantum((MagickRealType) (scale*
    channel_statistics[channel].maxima)),GetMagickPrecision(),
    channel_statistics[channel].maxima/(double) QuantumRange,
    GetMagickPrecision(),scale*channel_statistics[channel].mean,
    GetMagickPrecision(),channel_statistics[channel].mean/(double) QuantumRange,
    GetMagickPrecision(),scale*channel_statistics[channel].standard_deviation,
    GetMagickPrecision(),channel_statistics[channel].standard_deviation/
    (double) QuantumRange,GetMagickPrecision(),
    channel_statistics[channel].kurtosis,GetMagickPrecision(),
    channel_statistics[channel].skewness,GetMagickPrecision(),
    channel_statistics[channel].entropy);
  return(n);
}

MagickExport MagickBooleanType IdentifyImage(Image *image,FILE *file,
  const MagickBooleanType verbose,ExceptionInfo *exception)
{
  char
    color[MagickPathExtent],
    format[MagickPathExtent],
    key[MagickPathExtent];

  ChannelFeatures
    *channel_features;

  ChannelMoments
    *channel_moments;

  ChannelPerceptualHash
    *channel_phash;

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

  double
    elapsed_time,
    scale,
    user_time;

  ImageType
    type;

  MagickBooleanType
    ping;

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  size_t
    distance;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (file == (FILE *) NULL)
    file=stdout;
  colorspace=image->colorspace;
  locate=GetImageArtifact(image,"identify:locate");
  if (locate != (const char *) NULL)
    {
      const char
        *limit;

      size_t
        max_locations;

      StatisticType
        statistic_type;

      /*
        Display minimum, maximum, or mean pixel locations.
      */
      statistic_type=(StatisticType) ParseCommandOption(MagickStatisticOptions,
        MagickFalse,locate);
      limit=GetImageArtifact(image,"identify:limit");
      max_locations=0;
      if (limit != (const char *) NULL)
        max_locations=StringToUnsignedLong(limit);
      channel_statistics=GetLocationStatistics(image,statistic_type,exception);
      if (channel_statistics == (ChannelStatistics *) NULL)
        return(MagickFalse);
      (void) FormatLocaleFile(file,"Channel %s locations:\n",locate);
      switch (colorspace)
      {
        case RGBColorspace:
        case sRGBColorspace:
        {
          (void) PrintChannelLocations(file,image,RedPixelChannel,"Red",
            statistic_type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,GreenPixelChannel,"Green",
            statistic_type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,BluePixelChannel,"Blue",
            statistic_type,max_locations,channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelLocations(file,image,CyanPixelChannel,"Cyan",
            statistic_type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,MagentaPixelChannel,
            "Magenta",statistic_type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,YellowPixelChannel,"Yellow",
            statistic_type,max_locations,channel_statistics);
          (void) PrintChannelLocations(file,image,BlackPixelChannel,"Black",
            statistic_type,max_locations,channel_statistics);
          break;
        }
        case LinearGRAYColorspace:
        case GRAYColorspace:
        {
          (void) PrintChannelLocations(file,image,GrayPixelChannel,"Gray",
            statistic_type,max_locations,channel_statistics);
          break;
        }
        default:
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
            (void) PrintChannelLocations(file,image,(PixelChannel) i,"Gray",
              statistic_type,max_locations,channel_statistics);
          break;
        }
      }
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelLocations(file,image,AlphaPixelChannel,"Alpha",
          statistic_type,max_locations,channel_statistics);
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
      if (colorspace != UndefinedColorspace)
        (void) FormatLocaleFile(file,"%s ",CommandOptionToMnemonic(
          MagickColorspaceOptions,(ssize_t) colorspace));
      if (image->storage_class == DirectClass)
        {
          if (image->total_colors != 0)
            {
              (void) FormatMagickSize(image->total_colors,MagickFalse,"B",
                MagickPathExtent,format);
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
          (void) FormatMagickSize(GetBlobSize(image),MagickTrue,"B",
            MagickPathExtent,format);
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
  p=GetVirtualPixels(image,0,0,1,1,exception);
  ping=p == (const Quantum *) NULL ? MagickTrue : MagickFalse;
  (void) SignatureImage(image,exception);
  (void) FormatLocaleFile(file,"Image: %s\n",image->filename);
  if (*image->magick_filename != '\0')
    if (LocaleCompare(image->magick_filename,image->filename) != 0)
      {
        char
          filename[MagickPathExtent];

        GetPathComponent(image->magick_filename,TailPath,filename);
        (void) FormatLocaleFile(file,"  Base filename: %s\n",filename);
      }
  magick_info=GetMagickInfo(image->magick,exception);
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetMagickDescription(magick_info) == (const char *) NULL))
    (void) FormatLocaleFile(file,"  Format: %s\n",image->magick);
  else
    (void) FormatLocaleFile(file,"  Format: %s (%s)\n",image->magick,
      GetMagickDescription(magick_info));
  if ((magick_info != (const MagickInfo *) NULL) &&
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
  if ((image->resolution.x != 0.0) && (image->resolution.y != 0.0))
    {
      (void) FormatLocaleFile(file,"  Resolution: %gx%g\n",image->resolution.x,
        image->resolution.y);
      (void) FormatLocaleFile(file,"  Print size: %gx%g\n",(double)
        image->columns/image->resolution.x,(double) image->rows/
        image->resolution.y);
    }
  (void) FormatLocaleFile(file,"  Units: %s\n",CommandOptionToMnemonic(
    MagickResolutionOptions,(ssize_t) image->units));
  (void) FormatLocaleFile(file,"  Colorspace: %s\n",CommandOptionToMnemonic(
    MagickColorspaceOptions,(ssize_t) colorspace));
  type=IdentifyImageType(image,exception);
  (void) FormatLocaleFile(file,"  Type: %s\n",CommandOptionToMnemonic(
    MagickTypeOptions,(ssize_t) type));
  if (image->type != type)
    (void) FormatLocaleFile(file,"  Base type: %s\n",CommandOptionToMnemonic(
      MagickTypeOptions,(ssize_t) image->type));
  (void) FormatLocaleFile(file,"  Endianess: %s\n",CommandOptionToMnemonic(
    MagickEndianOptions,(ssize_t) image->endian));
  /*
    Detail channel depth and extrema.
  */
  channel_statistics=(ChannelStatistics *) NULL;
  channel_moments=(ChannelMoments *) NULL;
  channel_phash=(ChannelPerceptualHash *) NULL;
  channel_features=(ChannelFeatures *) NULL;
  scale=1.0;
  if (ping == MagickFalse)
    {
      size_t
        depth;

      channel_statistics=GetImageStatistics(image,exception);
      if (channel_statistics == (ChannelStatistics *) NULL)
        return(MagickFalse);
      artifact=GetImageArtifact(image,"identify:moments");
      if (artifact != (const char *) NULL)
        {
          channel_moments=GetImageMoments(image,exception);
          channel_phash=GetImagePerceptualHash(image,exception);
        }
      artifact=GetImageArtifact(image,"identify:features");
      if (artifact != (const char *) NULL)
        {
          distance=StringToUnsignedLong(artifact);
          channel_features=GetImageFeatures(image,distance,exception);
        }
      depth=GetImageDepth(image,exception);
      if (image->depth == depth)
        (void) FormatLocaleFile(file,"  Depth: %.20g-bit\n",(double)
          image->depth);
      else
        (void) FormatLocaleFile(file,"  Depth: %.20g/%.20g-bit\n",(double)
          image->depth,(double) depth);
      (void) FormatLocaleFile(file,"  Channel depth:\n");
      switch (colorspace)
      {
        case RGBColorspace:
        case sRGBColorspace:
        {
          (void) FormatLocaleFile(file,"    Red: %.20g-bit\n",(double)
            channel_statistics[RedPixelChannel].depth);
          (void) FormatLocaleFile(file,"    Green: %.20g-bit\n",(double)
            channel_statistics[GreenPixelChannel].depth);
          (void) FormatLocaleFile(file,"    Blue: %.20g-bit\n",(double)
            channel_statistics[BluePixelChannel].depth);
          break;
        }
        case CMYKColorspace:
        {
          (void) FormatLocaleFile(file,"    Cyan: %.20g-bit\n",(double)
            channel_statistics[CyanPixelChannel].depth);
          (void) FormatLocaleFile(file,"    Magenta: %.20g-bit\n",(double)
            channel_statistics[MagentaPixelChannel].depth);
          (void) FormatLocaleFile(file,"    Yellow: %.20g-bit\n",(double)
            channel_statistics[YellowPixelChannel].depth);
          (void) FormatLocaleFile(file,"    Black: %.20g-bit\n",(double)
            channel_statistics[BlackPixelChannel].depth);
          break;
        }
        case LinearGRAYColorspace:
        case GRAYColorspace:
        {
          (void) FormatLocaleFile(file,"    Gray: %.20g-bit\n",(double)
            channel_statistics[GrayPixelChannel].depth);
          break;
        }
        default:
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
            (void) FormatLocaleFile(file,"    Channel %.20g: %.20g-bit\n",
              (double) i,(double) channel_statistics[i].depth);
          break;
        }
      }
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) FormatLocaleFile(file,"    Alpha: %.20g-bit\n",(double)
          channel_statistics[AlphaPixelChannel].depth);
      scale=1.0;
      if (image->depth <= MAGICKCORE_QUANTUM_DEPTH)
        scale=(double) (QuantumRange/((size_t) QuantumRange >> ((size_t)
          MAGICKCORE_QUANTUM_DEPTH-image->depth)));
    }
  if (channel_statistics != (ChannelStatistics *) NULL)
    {
      (void) FormatLocaleFile(file,"  Channel statistics:\n");
      (void) FormatLocaleFile(file,"    Pixels: %.20g\n",(double)
        image->columns*image->rows);
      switch (colorspace)
      {
        case RGBColorspace:
        case sRGBColorspace:
        {
          (void) PrintChannelStatistics(file,RedPixelChannel,"Red",1.0/
            scale,channel_statistics);
          (void) PrintChannelStatistics(file,GreenPixelChannel,"Green",1.0/
            scale,channel_statistics);
          (void) PrintChannelStatistics(file,BluePixelChannel,"Blue",1.0/
            scale,channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelStatistics(file,CyanPixelChannel,"Cyan",1.0/
            scale,channel_statistics);
          (void) PrintChannelStatistics(file,MagentaPixelChannel,"Magenta",1.0/
            scale,channel_statistics);
          (void) PrintChannelStatistics(file,YellowPixelChannel,"Yellow",1.0/
            scale,channel_statistics);
          (void) PrintChannelStatistics(file,BlackPixelChannel,"Black",1.0/
            scale,channel_statistics);
          break;
        }
        case LinearGRAYColorspace:
        case GRAYColorspace:
        {
          (void) PrintChannelStatistics(file,GrayPixelChannel,"Gray",1.0/
            scale,channel_statistics);
          break;
        }
        default:
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            char
              channel[MagickPathExtent];

            (void) FormatLocaleString(channel,MagickPathExtent,"Channel %.20g",
              (double) i);
            (void) PrintChannelStatistics(file,(PixelChannel) i,channel,1.0/
              scale,channel_statistics);
          }
          break;
        }
      }
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelStatistics(file,AlphaPixelChannel,"Alpha",1.0/
          scale,channel_statistics);
      if ((colorspace != LinearGRAYColorspace) && (colorspace != GRAYColorspace))
        {
          (void) FormatLocaleFile(file,"  Image statistics:\n");
          (void) PrintChannelStatistics(file,CompositePixelChannel,"Overall",
            1.0/scale,channel_statistics);
        }
      channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
        channel_statistics);
    }
  if (channel_moments != (ChannelMoments *) NULL)
    {
      scale=(double) ((1UL << image->depth)-1);
      (void) FormatLocaleFile(file,"  Channel moments:\n");
      switch (colorspace)
      {
        case RGBColorspace:
        case sRGBColorspace:
        {
          (void) PrintChannelMoments(file,RedPixelChannel,"Red",scale,
            channel_moments);
          (void) PrintChannelMoments(file,GreenPixelChannel,"Green",scale,
            channel_moments);
          (void) PrintChannelMoments(file,BluePixelChannel,"Blue",scale,
            channel_moments);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelMoments(file,CyanPixelChannel,"Cyan",scale,
            channel_moments);
          (void) PrintChannelMoments(file,MagentaPixelChannel,"Magenta",scale,
            channel_moments);
          (void) PrintChannelMoments(file,YellowPixelChannel,"Yellow",scale,
            channel_moments);
          (void) PrintChannelMoments(file,BlackPixelChannel,"Black",scale,
            channel_moments);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelMoments(file,GrayPixelChannel,"Gray",scale,
            channel_moments);
          break;
        }
        default:
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            char
              channel[MagickPathExtent];

            (void) FormatLocaleString(channel,MagickPathExtent,"Channel %.20g",
              (double) i);
            (void) PrintChannelMoments(file,(PixelChannel) i,"channel",scale,
              channel_moments);
          }
          break;
        }
      }
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelMoments(file,AlphaPixelChannel,"Alpha",scale,
          channel_moments);
      if (colorspace != GRAYColorspace)
        {
          (void) FormatLocaleFile(file,"  Image moments:\n");
          (void) PrintChannelMoments(file,CompositePixelChannel,"Overall",scale,
            channel_moments);
        }
      channel_moments=(ChannelMoments *) RelinquishMagickMemory(
        channel_moments);
    }
  if (channel_phash != (ChannelPerceptualHash *) NULL)
    {
      (void) PrintChannelPerceptualHash(image,file,channel_phash);
      channel_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(
        channel_phash);
    }
  if (channel_features != (ChannelFeatures *) NULL)
    {
      (void) FormatLocaleFile(file,"  Channel features (horizontal, vertical, "
        "left and right diagonals, average):\n");
      switch (colorspace)
      {
        case RGBColorspace:
        case sRGBColorspace:
        {
          (void) PrintChannelFeatures(file,RedPixelChannel,"Red",
            channel_features);
          (void) PrintChannelFeatures(file,GreenPixelChannel,"Green",
            channel_features);
          (void) PrintChannelFeatures(file,BluePixelChannel,"Blue",
            channel_features);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelFeatures(file,CyanPixelChannel,"Cyan",
            channel_features);
          (void) PrintChannelFeatures(file,MagentaPixelChannel,"Magenta",
            channel_features);
          (void) PrintChannelFeatures(file,YellowPixelChannel,"Yellow",
            channel_features);
          (void) PrintChannelFeatures(file,BlackPixelChannel,"Black",
            channel_features);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelFeatures(file,GrayPixelChannel,"Gray",
            channel_features);
          break;
        }
        default:
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            char
              channel[MagickPathExtent];

            (void) FormatLocaleString(channel,MagickPathExtent,"Channel %.20g",
              (double) i);
            (void) PrintChannelFeatures(file,(PixelChannel) i,channel,
              channel_features);
          }
          break;
        }
      }
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelFeatures(file,AlphaPixelChannel,"Alpha",
          channel_features);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
    }
  if (ping == MagickFalse)
    {
      if (colorspace == CMYKColorspace)
        (void) FormatLocaleFile(file,"  Total ink density: %.*g%%\n",
          GetMagickPrecision(),100.0*GetImageTotalInkDensity(image,exception)/
          (double) QuantumRange);
      x=0;
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          MagickBooleanType
            found = MagickFalse;

          p=(const Quantum *) NULL;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelAlpha(image,p) == (Quantum) TransparentAlpha)
                {
                  found=MagickTrue;
                  break;
                }
              p+=GetPixelChannels(image);
            }
            if (found != MagickFalse)
              break;
          }
          if (found != MagickFalse)
            {
              char
                tuple[MagickPathExtent];

              PixelInfo
                pixel;

              GetPixelInfo(image,&pixel);
              GetPixelInfoPixel(image,p,&pixel);
              (void) QueryColorname(image,&pixel,SVGCompliance,tuple,
                exception);
              (void) FormatLocaleFile(file,"  Alpha: %s ",tuple);
              GetColorTuple(&pixel,MagickTrue,tuple);
              (void) FormatLocaleFile(file,"  %s\n",tuple);
            }
        }
      if (IsHistogramImage(image,exception) != MagickFalse)
        {
          (void) FormatLocaleFile(file,"  Colors: %.20g\n",(double)
            GetNumberColors(image,(FILE *) NULL,exception));
          (void) FormatLocaleFile(file,"  Histogram:\n");
          (void) GetNumberColors(image,file,exception);
        }
      else
        {
          artifact=GetImageArtifact(image,"identify:unique-colors");
          if (IsStringTrue(artifact) != MagickFalse)
            (void) FormatLocaleFile(file,"  Colors: %.20g\n",(double)
              GetNumberColors(image,(FILE *) NULL,exception));
        }
    }
  if (image->storage_class == PseudoClass)
    {
      (void) FormatLocaleFile(file,"  Colormap entries: %.20g\n",(double)
        image->colors);
      (void) FormatLocaleFile(file,"  Colormap:\n");
      if (image->colors <= 1024)
        {
          char
            hex[MagickPathExtent],
            tuple[MagickPathExtent];

          PixelInfo
            pixel;

          register PixelInfo
            *magick_restrict c;

          GetPixelInfo(image,&pixel);
          c=image->colormap;
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            pixel=(*c);
            (void) CopyMagickString(tuple,"(",MagickPathExtent);
            ConcatenateColorComponent(&pixel,RedPixelChannel,X11Compliance,
              tuple);
            (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
            ConcatenateColorComponent(&pixel,GreenPixelChannel,X11Compliance,
              tuple);
            (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
            ConcatenateColorComponent(&pixel,BluePixelChannel,X11Compliance,
              tuple);
            if (pixel.colorspace == CMYKColorspace)
              {
                (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
                ConcatenateColorComponent(&pixel,BlackPixelChannel,
                  X11Compliance,tuple);
              }
            if (pixel.alpha_trait != UndefinedPixelTrait)
              {
                (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
                ConcatenateColorComponent(&pixel,AlphaPixelChannel,
                  X11Compliance,tuple);
              }
            (void) ConcatenateMagickString(tuple,")",MagickPathExtent);
            (void) QueryColorname(image,&pixel,SVGCompliance,color,
              exception);
            GetColorTuple(&pixel,MagickTrue,hex);
            (void) FormatLocaleFile(file,"  %8ld: %s %s %s\n",(long) i,tuple,
              hex,color);
            c++;
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
  (void) QueryColorname(image,&image->matte_color,SVGCompliance,color,
    exception);
  (void) FormatLocaleFile(file,"  Matte color: %s\n",color);
  (void) QueryColorname(image,&image->background_color,SVGCompliance,color,
    exception);
  (void) FormatLocaleFile(file,"  Background color: %s\n",color);
  (void) QueryColorname(image,&image->border_color,SVGCompliance,color,
    exception);
  (void) FormatLocaleFile(file,"  Border color: %s\n",color);
  (void) QueryColorname(image,&image->transparent_color,SVGCompliance,color,
    exception);
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
  if (image->duration != 0)
    (void) FormatLocaleFile(file,"  Duration: %.20g\n",(double)
      image->duration);
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
        *d,
        *q;

      WarningHandler
        handler;

      /*
        Display visual image directory.
      */
      image_info=AcquireImageInfo();
      (void) CloneString(&image_info->size,"64x64");
      (void) FormatLocaleFile(file,"  Directory:\n");
      for (d=image->directory; *d != '\0'; d++)
      {
        q=d;
        while ((*q != '\377') && (*q != '\0') &&
               ((size_t) (q-d) < sizeof(image_info->filename)))
          q++;
        (void) CopyMagickString(image_info->filename,d,(size_t) (q-d+1));
        d=q;
        (void) FormatLocaleFile(file,"    %s",image_info->filename);
        handler=SetWarningHandler((WarningHandler) NULL);
        tile=ReadImage(image_info,exception);
        (void) SetWarningHandler(handler);
        if (tile == (Image *) NULL)
          {
            (void) FormatLocaleFile(file,"\n");
            continue;
          }
        (void) FormatLocaleFile(file," %.20gx%.20g %s\n",(double)
          tile->magick_columns,(double) tile->magick_rows,tile->magick);
        (void) SignatureImage(tile,exception);
        ResetImagePropertyIterator(tile);
        property=GetNextImageProperty(tile);
        while (property != (const char *) NULL)
        {
          (void) FormatLocaleFile(file,"  %s:\n",property);
          value=GetImageProperty(tile,property,exception);
          if (value != (const char *) NULL)
            (void) FormatLocaleFile(file,"%s\n",value);
          property=GetNextImageProperty(tile);
        }
        tile=DestroyImage(tile);
      }
      image_info=DestroyImageInfo(image_info);
    }
  (void) GetImageProperty(image,"exif:*",exception);
  (void) GetImageProperty(image,"icc:*",exception);
  (void) GetImageProperty(image,"iptc:*",exception);
  (void) GetImageProperty(image,"xmp:*",exception);
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
        value=GetImageProperty(image,property,exception);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"%s\n",value);
        property=GetNextImageProperty(image);
      }
    }
  (void) FormatLocaleString(key,MagickPathExtent,"8BIM:1999,2998:#1");
  value=GetImageProperty(image,key,exception);
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
            for (i=0; i < (ssize_t) profile_length-5; i+=(ssize_t) length)
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
                case 92: tag="Sub-Location"; break;
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
              length=MagickMin(length,profile_length-i);
              attribute=(char *) NULL;
              if (~length >= (MagickPathExtent-1))
                attribute=(char *) AcquireQuantumMemory(length+MagickPathExtent,
                  sizeof(*attribute));
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
          exception);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"%s\n",value);
        registry=GetNextImageRegistry();
      }
    }
  (void) FormatLocaleFile(file,"  Tainted: %s\n",CommandOptionToMnemonic(
    MagickBooleanOptions,(ssize_t) image->taint));
  (void) FormatMagickSize(GetBlobSize(image),MagickTrue,"B",MagickPathExtent,
    format);
  (void) FormatLocaleFile(file,"  Filesize: %s\n",format);
  (void) FormatMagickSize((MagickSizeType) image->columns*image->rows,
    MagickFalse,"P",MagickPathExtent,format);
  if (strlen(format) > 1)
    format[strlen(format)-1]='\0';
  (void) FormatLocaleFile(file,"  Number pixels: %s\n",format);
  if (elapsed_time > MagickEpsilon)
    {
      (void) FormatMagickSize((MagickSizeType) ((double) image->columns*
        image->rows/elapsed_time+0.5),MagickFalse,"B",MagickPathExtent,format);
      (void) FormatLocaleFile(file,"  Pixels per second: %s\n",format);
    }
  (void) FormatLocaleFile(file,"  User time: %0.3fu\n",user_time);
  (void) FormatLocaleFile(file,"  Elapsed time: %lu:%02lu.%03lu\n",
    (unsigned long) (elapsed_time/60.0),(unsigned long) ceil(fmod(elapsed_time,
    60.0)),(unsigned long) (1000.0*(elapsed_time-floor(elapsed_time))));
  (void) FormatLocaleFile(file,"  Version: %s\n",GetMagickVersion((size_t *)
    NULL));
  (void) fflush(file);
  return(ferror(file) != 0 ? MagickFalse : MagickTrue);
}

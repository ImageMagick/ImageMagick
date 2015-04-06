/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        JJJJJ  SSSSS   OOO   N   N                           %
%                          J    SS     O   O  NN  N                           %
%                          J     SSS   O   O  N N N                           %
%                        J J       SS  O   O  N  NN                           %
%                        JJJ    SSSSS   OOO   N   N                           %
%                                                                             %
%                                                                             %
%                  Write Info About the Image in JSON Format.                 %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                January 2014                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/feature.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/prepress.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/registry.h"
#include "magick/signature.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/utility.h"
#include "magick/version.h"
#include "magick/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteJSONImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r J S O N I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJSONImage() adds attributes for the JSON image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterJSONImage method is:
%
%      size_t RegisterJSONImage(void)
%
*/
ModuleExport size_t RegisterJSONImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("JSON");
  entry->encoder=(EncodeImageHandler *) WriteJSONImage;
  entry->blob_support=MagickFalse;
  entry->description=ConstantString("The image format and characteristics");
  entry->module=ConstantString("JSON");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r J S O N I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJSONImage() removes format registrations made by the
%  JSON module from the list of supported formats.
%
%  The format of the UnregisterJSONImage method is:
%
%      UnregisterJSONImage(void)
%
*/
ModuleExport void UnregisterJSONImage(void)
{
  (void) UnregisterMagickInfo("JSON");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e J S O N I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJSONImage writes the pixel values as text numbers.
%
%  The format of the WriteJSONImage method is:
%
%      MagickBooleanType WriteJSONImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
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
  const char *name,const MagickBooleanType separator,
  const ChannelFeatures *channel_features)
{
#define PrintFeature(feature) \
  GetMagickPrecision(),(feature)[0], \
  GetMagickPrecision(),(feature)[1], \
  GetMagickPrecision(),(feature)[2], \
  GetMagickPrecision(),(feature)[3], \
  GetMagickPrecision(),((feature)[0]+(feature)[1]+(feature)[2]+(feature)[3])/4.0 \

#define FeaturesFormat "      \"%s\": {\n" \
  "        \"angularSecondMoment\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"contrast\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"correlation\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"sumOfSquaresVariance\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"inverseDifferenceMoment\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"sumAverage\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"sumVariance\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"sumEntropy\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"entropy\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"differenceVariance\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"differenceEntropy\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"informationMeasureOfCorrelation1\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"informationMeasureOfCorrelation2\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        },\n" \
  "        \"maximumCorrelationCoefficient\": {\n" \
  "          \"horizontal\": \"%.*g\",\n" \
  "          \"vertical\": \"%.*g\",\n" \
  "          \"leftDiagonal\": \"%.*g\",\n" \
  "          \"rightDiagonal\": \"%.*g\",\n" \
  "          \"average\": \"%.*g\"\n" \
  "        }\n"

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
  (void) FormatLocaleFile(file,"      }");
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelLocations(FILE *file,const Image *image,
  const ChannelType channel,const char *name,const StatisticType type,
  const size_t max_locations,const MagickBooleanType separator,
  const ChannelStatistics *channel_statistics)
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
  (void) FormatLocaleFile(file,"      \"%s\": {\n        \"intensity\": "
    "\"%.*g\",\n",name,GetMagickPrecision(),QuantumScale*target);
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
          match=fabs((double) (p->red-target)) < 0.5 ? MagickTrue : MagickFalse;
          break;
        }
        case GreenChannel:
        {
          match=fabs((double) (p->green-target)) < 0.5 ? MagickTrue :
            MagickFalse;
          break;
        }
        case BlueChannel:
        {
          match=fabs((double) (p->blue-target)) < 0.5 ? MagickTrue :
            MagickFalse;
          break;
        }
        case AlphaChannel:
        {
          match=fabs((double) (p->opacity-target)) < 0.5 ? MagickTrue :
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
          if (n != 0)
            (void) FormatLocaleFile(file,",\n");
          (void) FormatLocaleFile(file,"        \"location%.20g\": {\n"
            "          \"x\": \"%.20g\",\n          \"y\": \"%.20g\"\n"
            "        }",(double) n,(double) x,(double) y);
          n++;
        }
      p++;
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  (void) FormatLocaleFile(file,"\n      }");
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelMoments(FILE *file,const ChannelType channel,
  const char *name,const MagickBooleanType separator,
  const ChannelMoments *channel_moments)
{
  register ssize_t
    i;

  ssize_t
    n;

  n=FormatLocaleFile(file,"      \"%s\": {\n",name);
  n+=FormatLocaleFile(file,"        \"centroid\": {\n "
    "          \"x\": \"%.*g\",\n"
    "           \"y\": \"%.*g\"\n        },\n",
    GetMagickPrecision(),channel_moments[channel].centroid.x,
    GetMagickPrecision(),channel_moments[channel].centroid.y);
  n+=FormatLocaleFile(file,"        \"ellipseSemiMajorMinorAxis\": {\n"
    "          \"x\": \"%.*g\",\n"
    "          \"y\": \"%.*g\"\n        },\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_axis.x,
    GetMagickPrecision(),channel_moments[channel].ellipse_axis.y);
  n+=FormatLocaleFile(file,"        \"ellipseAngle\": \"%.*g\",\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_angle);
  n+=FormatLocaleFile(file,"        \"ellipseEccentricity\": \"%.*g\",\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_eccentricity);
  n+=FormatLocaleFile(file,"        \"ellipseIntensity\": \"%.*g\",\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_intensity);
  for (i=0; i < 7; i++)
    n+=FormatLocaleFile(file,"        \"I%.20g\": \"%.*g\",\n",i+1.0,
      GetMagickPrecision(),channel_moments[channel].I[i]);
  n+=FormatLocaleFile(file,"        \"I%.20g\": \"%.*g\"\n",i+1.0,
    GetMagickPrecision(),channel_moments[channel].I[i]);
  (void) FormatLocaleFile(file,"      }");
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelPerceptualHash(FILE *file,const ChannelType channel,
  const char *name,const MagickBooleanType separator,
  const ChannelPerceptualHash *channel_phash)
{
  register ssize_t
    i;

  ssize_t
    n;

  n=FormatLocaleFile(file,"      \"%s\": {\n",name);
  for (i=0; i < 6; i++)
    n+=FormatLocaleFile(file,
      "        \"PH%.20g\": [ \"%.*g\", \"%.*g\" ],\n",i+1.0,
      GetMagickPrecision(),channel_phash[channel].P[i],
      GetMagickPrecision(),channel_phash[channel].Q[i]);
  n+=FormatLocaleFile(file,
    "        \"PH%.20g\": [ \"%.*g\", \"%.*g\" ]\n",i+1.0,
    GetMagickPrecision(),channel_phash[channel].P[i],
    GetMagickPrecision(),channel_phash[channel].Q[i]);
  (void) FormatLocaleFile(file,"      }");
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelStatistics(FILE *file,const ChannelType channel,
  const char *name,const double scale,const MagickBooleanType separator,
  const ChannelStatistics *channel_statistics)
{
#define StatisticsFormat "      \"%s\": {\n        \"min\": \"" QuantumFormat  \
  "\",\n        \"max\": \"" QuantumFormat "\",\n"  \
  "        \"mean\": \"%g\",\n        \"standardDeviation\": "  \
  "\"%g\",\n        \"kurtosis\": \"%g\",\n        \"skewness\": " \
  "\"%g\"\n      }"

  ssize_t
    n;

  if (channel == AlphaChannel)
    {
      n=FormatLocaleFile(file,StatisticsFormat,name,ClampToQuantum(scale*
        (QuantumRange-channel_statistics[channel].maxima)),
        ClampToQuantum(scale*(QuantumRange-channel_statistics[channel].minima)),
        scale*(QuantumRange-channel_statistics[channel].mean),scale*
        channel_statistics[channel].standard_deviation,
        channel_statistics[channel].kurtosis,
        channel_statistics[channel].skewness);
      if (separator != MagickFalse)
        (void) FormatLocaleFile(file,",");
      (void) FormatLocaleFile(file,"\n");
      return(n);
    }
  n=FormatLocaleFile(file,StatisticsFormat,name,ClampToQuantum(scale*
    channel_statistics[channel].minima),ClampToQuantum(scale*
    channel_statistics[channel].maxima),scale*channel_statistics[channel].mean,
    scale*channel_statistics[channel].standard_deviation,
    channel_statistics[channel].kurtosis,channel_statistics[channel].skewness);
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static MagickBooleanType EncodeImageAttributes(Image *image,FILE *file)
{
  char
    color[MaxTextExtent],
    format[MaxTextExtent],
    key[MaxTextExtent];

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
    user_time;

  ExceptionInfo
    *exception;

  ImageType
    type;

  register ssize_t
    i,
    x;

  size_t
    depth,
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
  *format='\0';
  elapsed_time=GetElapsedTime(&image->timer);
  user_time=GetUserTime(&image->timer);
  GetTimerInfo(&image->timer);
  (void) GetVirtualPixels(image,0,0,1,1,exception);
  exception=DestroyExceptionInfo(exception);
  exception=(&image->exception);
  type=GetImageType(image,exception);
  (void) SignatureImage(image);
  (void) FormatLocaleFile(file,"{\n  \"image\": {\n    \"name\": \"%s\",\n",
    image->filename);
  if (*image->magick_filename != '\0')
    if (LocaleCompare(image->magick_filename,image->filename) != 0)
      {
        char
          filename[MaxTextExtent];

        GetPathComponent(image->magick_filename,TailPath,filename);
        (void) FormatLocaleFile(file,"    \"baseName\": \"%s\",\n",filename);
      }
  magick_info=GetMagickInfo(image->magick,exception);
  (void) FormatLocaleFile(file,"    \"format\": \"%s\",\n",image->magick);
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickDescription(magick_info) != (const char *) NULL))
    (void) FormatLocaleFile(file,"    \"formatDescription\": \"%s\",\n",
      GetMagickDescription(magick_info));
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickMimeType(magick_info) != (const char *) NULL))
    (void) FormatLocaleFile(file,"    \"mimeType\": \"%s\",\n",
      GetMagickMimeType(magick_info));
  (void) FormatLocaleFile(file,"    \"class\": \"%s\",\n",
    CommandOptionToMnemonic(MagickClassOptions,(ssize_t) image->storage_class));
  (void) FormatLocaleFile(file,"    \"geometry\": {\n"
    "      \"width\": \"%.20g\",\n      \"height\": \"%.20g\",\n"
    "      \"x\": \"%.20g\",\n      \"y\": \"%.20g\"\n    },\n",
    (double) image->columns,(double) image->rows,(double) image->tile_offset.x,
    (double) image->tile_offset.y);
  if ((image->magick_columns != 0) || (image->magick_rows != 0))
    if ((image->magick_columns != image->columns) ||
        (image->magick_rows != image->rows))
      (void) FormatLocaleFile(file,"    \"baseGeometry\": {\n"
        "      \"width\": \"%.20g\",\n      \"height\": \"%.20g\"\n    },\n",
        (double) image->magick_columns,(double) image->magick_rows);
  if ((image->x_resolution != 0.0) && (image->y_resolution != 0.0))
    {
      (void) FormatLocaleFile(file,"    \"resolution\": {\n"
        "      \"x\": \"%.20g\",\n      \"y\": \"%.20g\"\n    },\n",
        image->x_resolution,image->y_resolution);
      (void) FormatLocaleFile(file,"    \"printSize\": {\n"
        "      \"x\": \"%.20g\",\n      \"y\": \"%.20g\"\n    },\n",
        image->columns/image->x_resolution,(double) image->rows/
        image->y_resolution);
    }
  (void) FormatLocaleFile(file,"    \"units\": \"%s\",\n",
    CommandOptionToMnemonic(MagickResolutionOptions,(ssize_t) image->units));
  (void) FormatLocaleFile(file,"    \"type\": \"%s\",\n",
    CommandOptionToMnemonic(MagickTypeOptions,(ssize_t) type));
  if (image->type != UndefinedType)
    (void) FormatLocaleFile(file,"    \"baseType\": \"%s\",\n",
      CommandOptionToMnemonic(MagickTypeOptions,(ssize_t) image->type));
  (void) FormatLocaleFile(file,"    \"endianess\": \"%s\",\n",
    CommandOptionToMnemonic(MagickEndianOptions,(ssize_t) image->endian));
  locate=GetImageArtifact(image,"identify:locate");
  if (locate == (const char *) NULL)
    locate=GetImageArtifact(image,"json:locate");
  if (locate != (const char *) NULL)
    {
      char
        target[MaxTextExtent];

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
      max_locations=0;
      limit=GetImageArtifact(image,"identify:limit");
      if (limit == (const char *) NULL)
        limit=GetImageArtifact(image,"json:limit");
      if (limit != (const char *) NULL)
        max_locations=StringToUnsignedLong(limit);
      channel_statistics=GetLocationStatistics(image,type,exception);
      if (channel_statistics == (ChannelStatistics *) NULL)
        return(MagickFalse);
      colorspace=image->colorspace;
      if (SetImageGray(image,exception) != MagickFalse)
        colorspace=GRAYColorspace;
      (void) CopyMagickString(target,locate,MaxTextExtent);
      *target=(char) toupper((int) ((unsigned char) *target));
      (void) FormatLocaleFile(file,"    \"channel%s\": {\n",target);
      if (image->matte != MagickFalse)
        (void) PrintChannelLocations(file,image,AlphaChannel,"alpha",
          type,max_locations,MagickTrue,channel_statistics);
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelLocations(file,image,RedChannel,"red",
            type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,GreenChannel,"Green",
            type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,BlueChannel,"blue",
            type,max_locations,MagickFalse,channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelLocations(file,image,CyanChannel,"cyan",
            type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,MagentaChannel,"magenta",
            type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,YellowChannel,"yellow",
            type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,BlackChannel,"black",
            type,max_locations,MagickFalse,channel_statistics);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelLocations(file,image,GrayChannel,"gray",
            type,max_locations,MagickFalse,channel_statistics);
          break;
        }
      }
      (void) FormatLocaleFile(file,"    },\n");
      channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
        channel_statistics);
    }
  /*
    Detail channel depth and extrema.
  */
  (void) FormatLocaleFile(file,"    \"colorspace\": \"%s\",\n",
    CommandOptionToMnemonic(MagickColorspaceOptions,(ssize_t)
    image->colorspace));
  channel_statistics=(ChannelStatistics *) NULL;
  channel_moments=(ChannelMoments *) NULL;
  channel_phash=(ChannelPerceptualHash *) NULL;
  channel_features=(ChannelFeatures *) NULL;
  colorspace=image->colorspace;
  scale=1;
  channel_statistics=GetImageChannelStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  artifact=GetImageArtifact(image,"identify:moments");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"json:moments");
  if (artifact != (const char *) NULL)
    {
      channel_moments=GetImageChannelMoments(image,exception);
      channel_phash=GetImageChannelPerceptualHash(image,exception);
    }
  artifact=GetImageArtifact(image,"identify:features");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"json:features");
  if (artifact != (const char *) NULL)
    {
      distance=StringToUnsignedLong(artifact);
      channel_features=GetImageChannelFeatures(image,distance,exception);
    }
  depth=GetImageDepth(image,exception);
  (void) FormatLocaleFile(file,"    \"depth\": \"%.20g\",\n",(double) depth);
  (void) FormatLocaleFile(file,"    \"baseDepth\": \"%.20g\",\n",(double)
    image->depth);
  (void) FormatLocaleFile(file,"    \"channelDepth\": {\n");
  if (SetImageGray(image,exception) != MagickFalse)
    colorspace=GRAYColorspace;
  if (image->matte != MagickFalse)
    (void) FormatLocaleFile(file,"      \"alpha\": \"%.20g\",\n",(double)
      channel_statistics[OpacityChannel].depth);
  switch (colorspace)
  {
    case RGBColorspace:
    default:
    {
      (void) FormatLocaleFile(file,"      \"red\": \"%.20g\",\n",(double)
        channel_statistics[RedChannel].depth);
      (void) FormatLocaleFile(file,"      \"green\": \"%.20g\",\n",(double)
        channel_statistics[GreenChannel].depth);
      (void) FormatLocaleFile(file,"      \"blue\": \"%.20g\"\n",(double)
        channel_statistics[BlueChannel].depth);
      break;
    }
    case CMYKColorspace:
    {
      (void) FormatLocaleFile(file,"      \"cyan\": \"%.20g\",\n",(double)
        channel_statistics[CyanChannel].depth);
      (void) FormatLocaleFile(file,"      \"magenta\": \"%.20g\",\n",(double)
        channel_statistics[MagentaChannel].depth);
      (void) FormatLocaleFile(file,"      \"yellow\": \"%.20g\",\n",(double)
        channel_statistics[YellowChannel].depth);
      (void) FormatLocaleFile(file,"      \"black\": \"%.20g\"\n",(double)
        channel_statistics[BlackChannel].depth);
      break;
    }
    case GRAYColorspace:
    {
      (void) FormatLocaleFile(file,"      \"gray\": \"%.20g\"\n",(double)
        channel_statistics[GrayChannel].depth);
      break;
    }
  }
  (void) FormatLocaleFile(file,"    },\n");
  scale=1;
  if (image->depth <= MAGICKCORE_QUANTUM_DEPTH)
    scale=QuantumRange/((size_t) QuantumRange >> ((size_t)
      MAGICKCORE_QUANTUM_DEPTH-image->depth));
  if (channel_statistics != (ChannelStatistics *) NULL)
    {
      if (colorspace != GRAYColorspace)
        {
          (void) FormatLocaleFile(file,"    \"imageStatistics\": {\n");
          (void) PrintChannelStatistics(file,CompositeChannels,"all",1.0/
            scale,MagickFalse,channel_statistics);
          (void) FormatLocaleFile(file,"    },\n");
        }
      (void) FormatLocaleFile(file,"    \"channelStatistics\": {\n");
      if (image->matte != MagickFalse)
        (void) PrintChannelStatistics(file,AlphaChannel,"alpha",1.0/scale,
          MagickTrue,channel_statistics);
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelStatistics(file,RedChannel,"red",1.0/scale,
            MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,GreenChannel,"green",1.0/scale,
            MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,BlueChannel,"blue",1.0/scale,
            MagickFalse,channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelStatistics(file,CyanChannel,"cyan",1.0/scale,
            MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,MagentaChannel,"magenta",1.0/scale,
            MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,YellowChannel,"yellow",1.0/scale,
            MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,BlackChannel,"black",1.0/scale,
            MagickFalse,channel_statistics);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelStatistics(file,GrayChannel,"gray",1.0/scale,
            MagickFalse,channel_statistics);
          break;
        }
      }
      (void) FormatLocaleFile(file,"    },\n");
      channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
        channel_statistics);
    }
  if (channel_moments != (ChannelMoments *) NULL)
    {
      (void) FormatLocaleFile(file,"    \"channelMoments\": {\n");
      if (image->matte != MagickFalse)
        (void) PrintChannelMoments(file,AlphaChannel,"alpha",MagickTrue,
          channel_moments);
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelMoments(file,RedChannel,"red",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,GreenChannel,"green",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,BlueChannel,"blue",MagickFalse,
            channel_moments);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelMoments(file,CyanChannel,"cyan",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,MagentaChannel,"magenta",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,YellowChannel,"yellow",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,BlackChannel,"black",MagickFalse,
            channel_moments);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelMoments(file,GrayChannel,"gray",MagickFalse,
            channel_moments);
          break;
        }
      }
      (void) FormatLocaleFile(file,"    },\n");
      channel_moments=(ChannelMoments *) RelinquishMagickMemory(
        channel_moments);
    }
  if (channel_phash != (ChannelPerceptualHash *) NULL)
    {
      (void) FormatLocaleFile(file,"    \"channelPerceptualHash\": {\n");
      if (image->matte != MagickFalse)
        (void) PrintChannelPerceptualHash(file,AlphaChannel,"alphaAlpha",
          MagickTrue,channel_phash);
      (void) PrintChannelPerceptualHash(file,RedChannel,"redHue",MagickTrue,
        channel_phash);
      (void) PrintChannelPerceptualHash(file,GreenChannel,"greenChroma",
        MagickTrue,channel_phash);
      (void) PrintChannelPerceptualHash(file,BlueChannel,"blueLuma",MagickFalse,
        channel_phash);
      (void) FormatLocaleFile(file,"    },\n");
      channel_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(
        channel_phash);
    }
  if (channel_features != (ChannelFeatures *) NULL)
    {
      (void) FormatLocaleFile(file,"    \"channelFeatures\": {\n");
      if (image->matte != MagickFalse)
        (void) PrintChannelFeatures(file,AlphaChannel,"alpha",MagickTrue,
          channel_features);
      switch (colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelFeatures(file,RedChannel,"red",MagickTrue,
            channel_features);
          (void) PrintChannelFeatures(file,GreenChannel,"green",MagickTrue,
            channel_features);
          (void) PrintChannelFeatures(file,BlueChannel,"blue",MagickFalse,
            channel_features);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelFeatures(file,CyanChannel,"cyan",MagickTrue,
            channel_features);
          (void) PrintChannelFeatures(file,MagentaChannel,"magenta",MagickTrue,
            channel_features);
          (void) PrintChannelFeatures(file,YellowChannel,"yellow",MagickTrue,
            channel_features);
          (void) PrintChannelFeatures(file,BlackChannel,"black",MagickFalse,
            channel_features);
          break;
        }
        case GRAYColorspace:
        {
          (void) PrintChannelFeatures(file,GrayChannel,"gray",MagickFalse,
            channel_features);
          break;
        }
      }
      (void) FormatLocaleFile(file,"    },\n");
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
    }
  if (image->colorspace == CMYKColorspace)
    (void) FormatLocaleFile(file,"    \"totalInkDensity\": \"%.*g%%\",\n",
      GetMagickPrecision(),100.0*GetImageTotalInkDensity(image)/(double)
      QuantumRange);
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
            exception);
          (void) FormatLocaleFile(file,"    \"alpha\": \"%s\",\n",tuple);
        }
    }
  if (image->storage_class == PseudoClass)
    {
      char
        tuple[MaxTextExtent];

      MagickPixelPacket
        pixel;

      register PixelPacket
        *restrict p;

      (void) FormatLocaleFile(file,"    \"colormapEntries\": \"%.20g\",\n",
        (double) image->colors);
      (void) FormatLocaleFile(file,"    \"colormap\": [\n        ");
      GetMagickPixelPacket(image,&pixel);
      p=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        SetMagickPixelPacket(image,p,(IndexPacket *) NULL,&pixel);
        *tuple='\0';
        ConcatenateColorComponent(&pixel,RedChannel,X11Compliance,tuple);
        (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
        ConcatenateColorComponent(&pixel,GreenChannel,X11Compliance,tuple);
        (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
        ConcatenateColorComponent(&pixel,BlueChannel,X11Compliance,tuple);
        if (pixel.colorspace == CMYKColorspace)
          {
            (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
            ConcatenateColorComponent(&pixel,IndexChannel,X11Compliance,tuple);
          }
        if (pixel.matte != MagickFalse)
          {
            (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
            ConcatenateColorComponent(&pixel,AlphaChannel,X11Compliance,tuple);
          }
        (void) FormatLocaleFile(file,"%s",tuple);
        if (i < (ssize_t) (image->colors-1))
          (void) FormatLocaleFile(file,",");
        if (((i+1) % 5) == 0)
          (void) FormatLocaleFile(file,"\n        ");
        p++;
      }
      (void) FormatLocaleFile(file,"\n    ],\n");
    }
  if (image->error.mean_error_per_pixel != 0.0)
    (void) FormatLocaleFile(file,"    \"meanErrorPerPixel\": \"%g\",\n",
      image->error.mean_error_per_pixel);
  if (image->error.normalized_mean_error != 0.0)
    (void) FormatLocaleFile(file,"    \"normalizedMeanError\": \"%g\",\n",
      image->error.normalized_mean_error);
  if (image->error.normalized_maximum_error != 0.0)
    (void) FormatLocaleFile(file,"    \"normalizedMaximumError\": \"%g\",\n",
      image->error.normalized_maximum_error);
  (void) FormatLocaleFile(file,"    \"renderingIntent\": \"%s\",\n",
    CommandOptionToMnemonic(MagickIntentOptions,(ssize_t)
    image->rendering_intent));
  if (image->gamma != 0.0)
    (void) FormatLocaleFile(file,"    \"gamma\": \"%g\",\n",image->gamma);
  if ((image->chromaticity.red_primary.x != 0.0) ||
      (image->chromaticity.green_primary.x != 0.0) ||
      (image->chromaticity.blue_primary.x != 0.0) ||
      (image->chromaticity.white_point.x != 0.0))
    {
      /*
        Display image chromaticity.
      */
      (void) FormatLocaleFile(file,"    \"chromaticity\": {\n");
      (void) FormatLocaleFile(file,"      \"redPrimary\": {\n"
        "        \"x\": \"%g\",\n        \"y\": \"%g\"\n      },\n",
        image->chromaticity.red_primary.x,image->chromaticity.red_primary.y);
      (void) FormatLocaleFile(file,"      \"greenPrimary\": {\n"
        "        \"x\": \"%g\",\n        \"y\": \"%g\"\n      },\n",
        image->chromaticity.green_primary.x,
        image->chromaticity.green_primary.y);
      (void) FormatLocaleFile(file,"      \"bluePrimary\": {\n"
        "        \"x\": \"%g\",\n        \"y\": \"%g\"\n      },\n",
        image->chromaticity.blue_primary.x,image->chromaticity.blue_primary.y);
      (void) FormatLocaleFile(file,"      \"whitePrimary\": {\n"
        "        \"x\": \"%g\",\n        \"y\": \"%g\"\n      }\n",
        image->chromaticity.white_point.x,image->chromaticity.white_point.y);
      (void) FormatLocaleFile(file,"    },\n");
    }
  if ((image->extract_info.width*image->extract_info.height) != 0)
    (void) FormatLocaleFile(file,"    \"tileGeometry\": {\n"
      "      \"width\": \"%.20g\",\n      \"height\": \"%.20g\",\n"
      "      \"x\": \"%.20g\",\n      \"y\": \"%.20g\"\n    },\n",
      (double) image->extract_info.width,(double) image->extract_info.height,
      (double) image->extract_info.x,(double) image->extract_info.y);
  (void) QueryColorname(image,&image->background_color,SVGCompliance,color,
    exception);
  (void) FormatLocaleFile(file,"    \"backgroundColor\": \"%s\",\n",color);
  (void) QueryColorname(image,&image->border_color,SVGCompliance,color,
    exception);
  (void) FormatLocaleFile(file,"    \"borderColor\": \"%s\",\n",color);
  (void) QueryColorname(image,&image->matte_color,SVGCompliance,color,
    exception);
  (void) FormatLocaleFile(file,"    \"matteColor\": \"%s\",\n",color);
  (void) QueryColorname(image,&image->transparent_color,SVGCompliance,color,
    exception);
  (void) FormatLocaleFile(file,"    \"transparentColor\": \"%s\",\n",color);
  (void) FormatLocaleFile(file,"    \"interlace\": \"%s\",\n",
    CommandOptionToMnemonic(MagickInterlaceOptions,(ssize_t) image->interlace));
  (void) FormatLocaleFile(file,"    \"intensity\": \"%s\",\n",
    CommandOptionToMnemonic(MagickPixelIntensityOptions,(ssize_t)
    image->intensity));
  (void) FormatLocaleFile(file,"    \"compose\": \"%s\",\n",
    CommandOptionToMnemonic(MagickComposeOptions,(ssize_t) image->compose));
  if ((image->page.width != 0) || (image->page.height != 0) ||
      (image->page.x != 0) || (image->page.y != 0))
    (void) FormatLocaleFile(file,"    \"pageGeometry\": {\n"
      "      \"width\": \"%.20g\",\n      \"height\": \"%.20g\",\n"
      "      \"x\": \"%.20g\",\n      \"y\": \"%.20g\"\n    },\n",
      (double) image->page.width,(double) image->page.height,
      (double) image->page.x,(double) image->page.y);
  if ((image->page.x != 0) || (image->page.y != 0))
    (void) FormatLocaleFile(file,"    \"origingeometry\": %+.20g%+.20g\n",
      (double) image->page.x,(double) image->page.y);
  (void) FormatLocaleFile(file,"    \"dispose\": \"%s\",\n",
    CommandOptionToMnemonic(MagickDisposeOptions,(ssize_t) image->dispose));
  if (image->delay != 0)
    (void) FormatLocaleFile(file,"    \"delay\": %.20gx%.20g\n",
      (double) image->delay,(double) image->ticks_per_second);
  if (image->iterations != 1)
    (void) FormatLocaleFile(file,"    \"iterations\": \"%.20g\",\n",(double)
      image->iterations);
  if ((image->next != (Image *) NULL) || (image->previous != (Image *) NULL))
    (void) FormatLocaleFile(file,"    \"scene\": %.20g of %.20g\n",(double)
      image->scene,(double) GetImageListLength(image));
  else
    if (image->scene != 0)
      (void) FormatLocaleFile(file,"    \"scene\": \"%.20g\",\n",(double)
        image->scene);
  (void) FormatLocaleFile(file,"    \"compression\": \"%s\",\n",
    CommandOptionToMnemonic(MagickCompressOptions,(ssize_t)
    image->compression));
  if (image->quality != UndefinedCompressionQuality)
    (void) FormatLocaleFile(file,"    \"quality\": \"%.20g\",\n",(double)
      image->quality);
  (void) FormatLocaleFile(file,"    \"orientation\": \"%s\",\n",
    CommandOptionToMnemonic(MagickOrientationOptions,(ssize_t)
    image->orientation));
  if (image->montage != (char *) NULL)
    (void) FormatLocaleFile(file,"    \"montage\": \"%s\",\n",image->montage);
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
      (void) FormatLocaleFile(file,"    Directory:\n");
      for (p=image->directory; *p != '\0'; p++)
      {
        q=p;
        while ((*q != '\n') && (*q != '\0'))
          q++;
        (void) CopyMagickString(image_info->filename,p,(size_t) (q-p+1));
        p=q;
        (void) FormatLocaleFile(file,"      %s",image_info->filename);
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
        (void) SignatureImage(tile);
        ResetImagePropertyIterator(tile);
        property=GetNextImageProperty(tile);
        while (property != (const char *) NULL)
        {
          (void) FormatLocaleFile(file,"    %s:\n",property);
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
  (void) GetImageProperty(image,"icc:*");
  (void) GetImageProperty(image,"iptc:*");
  (void) GetImageProperty(image,"xmp:*");
  ResetImagePropertyIterator(image);
  property=GetNextImageProperty(image);
  if (property != (const char *) NULL)
    {
      size_t
        n;

      /*
        Display image properties.
      */
      n=0;
      (void) FormatLocaleFile(file,"    \"properties\": {\n");
      while (property != (const char *) NULL)
      {
        if (n++ != 0)
          (void) FormatLocaleFile(file,",\n");
        (void) FormatLocaleFile(file,"      \"%s\": ",property);
        value=GetImageProperty(image,property);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"\"%s\"",value);
        property=GetNextImageProperty(image);
      }
      (void) FormatLocaleFile(file,"\n    },\n");
    }
  (void) FormatLocaleString(key,MaxTextExtent,"8BIM:1999,2998:#1");
  value=GetImageProperty(image,key);
  if (value != (const char *) NULL)
    {
      /*
        Display clipping path.
      */
      (void) FormatLocaleFile(file,"    \"clipping path\": {");
      if (strlen(value) > 80)
        (void) fputc('\n',file);
      (void) FormatLocaleFile(file,"%s\n",value);
      (void) FormatLocaleFile(file,"    },\n");
    }
  ResetImageProfileIterator(image);
  name=GetNextImageProfile(image);
  if (name != (char *) NULL)
    {
      const StringInfo
        *profile;

      size_t
        n;

      /*
        Identify image profiles.
      */
      n=0;
      (void) FormatLocaleFile(file,"    \"profiles\": {\n");
      while (name != (char *) NULL)
      {
        profile=GetImageProfile(image,name);
        if (profile == (StringInfo *) NULL)
          continue;
        if (n++ != 0)
          (void) FormatLocaleFile(file,",\n");
        (void) FormatLocaleFile(file,"      \"%s\": {\n",name);
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
              (void) FormatLocaleFile(file,"        \"%s[%.20g,%.20g]\": ",
                tag,(double) dataset,(double) record);
              length=(size_t) (GetStringInfoDatum(profile)[i++] << 8);
              length|=GetStringInfoDatum(profile)[i++];
              attribute=(char *) NULL;
              if (~length >= (MaxTextExtent-1))
                attribute=(char *) AcquireQuantumMemory(length+MaxTextExtent,
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
                        (void) fputs("\"",file);
                        (void) fputs(attribute_list[j],file);
                        (void) fputs("\",\n",file);
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
        (void) FormatLocaleFile(file,"        \"length\": \"%.20g\"",(double)
          GetStringInfoLength(profile));
        (void) FormatLocaleFile(file,"\n      }");
        name=GetNextImageProfile(image);
      }
      (void) FormatLocaleFile(file,"\n    },\n");
    }
  ResetImageArtifactIterator(image);
  artifact=GetNextImageArtifact(image);
  if (artifact != (const char *) NULL)
    {
      ssize_t
        n;

      /*
        Display image artifacts.
      */
      n=0;
      (void) FormatLocaleFile(file,"    \"artifacts\": {\n");
      while (artifact != (const char *) NULL)
      {
        if (n++ != 0)
          (void) FormatLocaleFile(file,",\n");
        (void) FormatLocaleFile(file,"      \"%s\": ",artifact);
        value=GetImageArtifact(image,artifact);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"\"%s\"",value);
        artifact=GetNextImageArtifact(image);
      }
      (void) FormatLocaleFile(file,"\n    },\n");
    }
  ResetImageRegistryIterator();
  registry=GetNextImageRegistry();
  if (registry != (const char *) NULL)
    {
      /*
        Display image registry.
      */
      (void) FormatLocaleFile(file,"    \"registry\": {\n");
      while (registry != (const char *) NULL)
      {
        (void) FormatLocaleFile(file,"      %s: ",registry);
        value=(const char *) GetImageRegistry(StringRegistryType,registry,
          exception);
        if (value != (const char *) NULL)
          (void) FormatLocaleFile(file,"%s\n",value);
        registry=GetNextImageRegistry();
      }
      (void) FormatLocaleFile(file,"    },\n");
    }
  (void) FormatLocaleFile(file,"    \"tainted\": \"%s\",\n",
    CommandOptionToMnemonic(MagickBooleanOptions,(ssize_t) image->taint));
  (void) FormatMagickSize(GetBlobSize(image),MagickFalse,format);
  (void) FormatLocaleFile(file,"    \"filesize\": \"%s\",\n",format);
  (void) FormatMagickSize((MagickSizeType) image->columns*image->rows,
     MagickFalse,format);
  if (strlen(format) > 1)
    format[strlen(format)-1]='\0';
  (void) FormatLocaleFile(file,"    \"numberPixels\": \"%s\",\n",format);
  (void) FormatMagickSize((MagickSizeType) ((double) image->columns*image->rows/
    elapsed_time+0.5),MagickFalse,format);
  (void) FormatLocaleFile(file,"    \"pixelsPerSecond\": \"%s\",\n",format);
  (void) FormatLocaleFile(file,"    \"userTime\": \"%0.3fu\",\n",user_time);
  (void) FormatLocaleFile(file,"    \"elapsedTime\": \"%lu:%02lu.%03lu\",\n",
    (unsigned long) (elapsed_time/60.0),(unsigned long) ceil(fmod(
    elapsed_time,60.0)),(unsigned long) (1000.0*(elapsed_time-floor(
    elapsed_time))));
  (void) FormatLocaleFile(file,"    \"version\": \"%s\"\n",
    GetMagickVersion((size_t *) NULL));
  (void) FormatLocaleFile(file,"  }\n}\n");
  (void) fflush(file);
  return(ferror(file) != 0 ? MagickFalse : MagickTrue);
}

static MagickBooleanType WriteJSONImage(const ImageInfo *image_info,
  Image *image)
{
  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  do
  {
    (void) CopyMagickString(image->filename,image->magick_filename,
      MaxTextExtent);
    image->magick_columns=image->columns;
    image->magick_rows=image->rows;
    (void) EncodeImageAttributes(image,GetBlobFileHandle(image));
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

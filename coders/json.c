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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/feature.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/locale-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/prepress.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/registry.h"
#include "MagickCore/signature.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"
#include "MagickCore/module.h"
#include "coders/coders-private.h"

/*
  Typedef declarations.
*/
typedef struct _IPTCInfo
{
  long
    dataset,
    record;

  size_t
    values_length;

  char
    tag[32],
    ***values;
} IPTCInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteJSONImage(const ImageInfo *,Image *,ExceptionInfo *);

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

  entry=AcquireMagickInfo("JSON","JSON","The image format and characteristics");
  entry->encoder=(EncodeImageHandler *) WriteJSONImage;
  entry->mime_type=ConstantString("application/json");
  entry->flags|=CoderEndianSupportFlag;
  entry->flags^=CoderBlobSupportFlag;
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
%  WriteJSONImage writes the image attributes in the JSON format.
%
%  The format of the WriteJSONImage method is:
%
%      MagickBooleanType WriteJSONImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void JSONFormatLocaleFile(FILE *file,const char *format,
  const char *value)
{
  char
    *escaped_json;

  char
    *q;

  const char
    *p;

  size_t
    length;

  assert(format != (const char *) NULL);
  if ((value == (char *) NULL) || (*value == '\0'))
    {
      (void) FormatLocaleFile(file,format,"null");
      return;
    }
  length=strlen(value)+2;
  /*
    Find all the chars that need escaping and increase the dest length counter.
  */
  for (p=value; *p != '\0'; p++)
  {
    switch (*p)
    {
      case '"':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
      case '\\':
      {
        if (~length < 1)
          return;
        length++;
        break;
      }
      default:
      {
        if (((int) *p >= 0x00) && ((int) *p <= 0x1f))
          length+=6;
        break;
      }
    }
  }
  escaped_json=(char *) NULL;
  if (~length >= (MagickPathExtent-1))
    escaped_json=(char *) AcquireQuantumMemory(length+MagickPathExtent,
      sizeof(*escaped_json));
  if (escaped_json == (char *) NULL)
    {
      (void) FormatLocaleFile(file,format,"null");
      return;
    }
  q=escaped_json;
  *q++='"';
  for (p=value; *p != '\0'; p++)
  {
    switch (*p)
    {
      case '"':
      {
        *q++='\\';
        *q++=(*p);
        break;
      }
      case '\b':
      {
        *q++='\\';
        *q++='b';
        break;
      }
      case '\f':
      {
        *q++='\\';
        *q++='f';
        break;
      }
      case '\n':
      {
        *q++='\\';
        *q++='n';
        break;
      }
      case '\r':
      {
        *q++='\\';
        *q++='r';
        break;
      }
      case '\t':
      {
        *q++='\\';
        *q++='t';
        break;
      }
      case '\\':
      {
        *q++='\\';
        *q++='\\';
        break;
      }
      default:
      {
        if (((int) *p >= 0x00) && ((int) *p <= 0x1f))
          {
            (void) FormatLocaleString(q,7,"\\u%04X",(int) *p);
            q+=(ptrdiff_t) 6;
            break;
          }
        *q++=(*p);
        break;
      }
    }
  }
  *q++='"';
  *q='\0';
  (void) FormatLocaleFile(file,format,escaped_json);
  (void) DestroyString(escaped_json);
}

static ChannelStatistics *GetLocationStatistics(const Image *image,
  const StatisticType type,ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  ssize_t
    i;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(
    MaxPixelChannels+1,sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(channel_statistics,0,(MaxPixelChannels+1)*
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
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelReadMask(image,p) <= (QuantumRange/2))
        {
          p+=(ptrdiff_t) GetPixelChannels(image);
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
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  return(channel_statistics);
}

static ssize_t PrintChannelFeatures(FILE *file,const PixelChannel channel,
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
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"contrast\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"correlation\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"sumOfSquaresVariance\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"inverseDifferenceMoment\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"sumAverage\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"sumVariance\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"sumEntropy\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"entropy\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"differenceVariance\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"differenceEntropy\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"informationMeasureOfCorrelation1\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"informationMeasureOfCorrelation2\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        },\n" \
  "        \"maximumCorrelationCoefficient\": {\n" \
  "          \"horizontal\": %.*g,\n" \
  "          \"vertical\": %.*g,\n" \
  "          \"leftDiagonal\": %.*g,\n" \
  "          \"rightDiagonal\": %.*g,\n" \
  "          \"average\": %.*g\n" \
  "        }\n"

  char
    *buffer;

  ssize_t
    n;

  buffer=AcquireString((char *) NULL);
  n=FormatLocaleString(buffer,MagickPathExtent,FeaturesFormat,name,
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
  (void) SubstituteString(&buffer,": -inf",": null");
  (void) SubstituteString(&buffer,": inf",": null");
  (void) SubstituteString(&buffer,": -nan",": null");
  (void) SubstituteString(&buffer,": nan",": null");
  n=FormatLocaleFile(file,"%s",buffer);
  buffer=DestroyString(buffer);
  (void) FormatLocaleFile(file,"      }");
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelLocations(FILE *file,const Image *image,
  const PixelChannel channel,const char *name,const StatisticType type,
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
    case MinimumStatistic:
    {
      target=channel_statistics[channel].minima;
      break;
    }
  }
  (void) FormatLocaleFile(file,"      \"%s\": {\n        \"intensity\": "
    "%.*g,\n",name,GetMagickPrecision(),QuantumScale*target);
  exception=AcquireExceptionInfo();
  n=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
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
      match=fabs((double) p[offset]-target) < 0.5 ? MagickTrue : MagickFalse;
      if (match != MagickFalse)
        {
          if ((max_locations != 0) && (n >= (ssize_t) max_locations))
            break;
          if (n != 0)
            (void) FormatLocaleFile(file,",\n");
          (void) FormatLocaleFile(file,"        \"location%.20g\": {\n"
            "          \"x\": %.20g,\n          \"y\": %.20g\n"
            "        }",(double) n,(double) x,(double) y);
          n++;
        }
      p+=(ptrdiff_t) GetPixelChannels(image);
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

static ssize_t PrintChannelMoments(FILE *file,const PixelChannel channel,
  const char *name,const MagickBooleanType separator,
  const ChannelMoments *channel_moments)
{
  ssize_t
    i;

  ssize_t
    n;

  n=FormatLocaleFile(file,"      \"%s\": {\n",name);
  n+=FormatLocaleFile(file,"        \"centroid\": {\n "
    "          \"x\": %.*g,\n"
    "           \"y\": %.*g\n        },\n",
    GetMagickPrecision(),channel_moments[channel].centroid.x,
    GetMagickPrecision(),channel_moments[channel].centroid.y);
  n+=FormatLocaleFile(file,"        \"ellipseSemiMajorMinorAxis\": {\n"
    "          \"x\": %.*g,\n"
    "          \"y\": %.*g\n        },\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_axis.x,
    GetMagickPrecision(),channel_moments[channel].ellipse_axis.y);
  n+=FormatLocaleFile(file,"        \"ellipseAngle\": %.*g,\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_angle);
  n+=FormatLocaleFile(file,"        \"ellipseEccentricity\": %.*g,\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_eccentricity);
  n+=FormatLocaleFile(file,"        \"ellipseIntensity\": %.*g,\n",
    GetMagickPrecision(),channel_moments[channel].ellipse_intensity);
  for (i=0; i < 7; i++)
    n+=FormatLocaleFile(file,"        \"I%.20g\": %.*g,\n",i+1.0,
      GetMagickPrecision(),channel_moments[channel].invariant[i]);
  n+=FormatLocaleFile(file,"        \"I%.20g\": %.*g\n",i+1.0,
    GetMagickPrecision(),channel_moments[channel].invariant[i]);
  (void) FormatLocaleFile(file,"      }");
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static ssize_t PrintChannelPerceptualHash(Image *image,FILE *file,
  const ChannelPerceptualHash *channel_phash)
{
  ssize_t
    i;

  ssize_t
    n = 0;

  (void) FormatLocaleFile(file,"      \"colorspaces\": [ ");
  for (i=0; i < (ssize_t) channel_phash[0].number_colorspaces; i++)
  {
    (void) FormatLocaleFile(file,"\"%s\"",CommandOptionToMnemonic(
      MagickColorspaceOptions,(ssize_t) channel_phash[0].colorspace[i]));
    if (i < (ssize_t) (channel_phash[0].number_colorspaces-1))
      (void) FormatLocaleFile(file,", ");
  }
  (void) FormatLocaleFile(file,"],\n");
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    ssize_t
      j;

    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if (traits == UndefinedPixelTrait)
      continue;
    n=FormatLocaleFile(file,"      \"Channel%.20g\": {\n",(double) channel);
    for (j=0; j < MaximumNumberOfPerceptualHashes; j++)
    {
      ssize_t
        k;

      n+=FormatLocaleFile(file,"        \"PH%.20g\": [",(double) j+1);
      for (k=0; k < (ssize_t) channel_phash[0].number_colorspaces; k++)
      {
        n+=FormatLocaleFile(file,"%.*g",GetMagickPrecision(),
          channel_phash[channel].phash[k][j]);
        if (k < (ssize_t) (channel_phash[0].number_colorspaces-1))
          n+=FormatLocaleFile(file,", ");
      }
      n+=FormatLocaleFile(file,"]");
      if (j < (MaximumNumberOfPerceptualHashes-1))
        n+=FormatLocaleFile(file,",\n");
    }
    if (i < (ssize_t) (GetPixelChannels(image)-1))
      n+=FormatLocaleFile(file,"\n      },\n");
  }
  n+=FormatLocaleFile(file,"\n      }\n");
  return(n);
}

static ssize_t PrintChannelStatistics(FILE *file,const PixelChannel channel,
  const char *name,const double scale,const MagickBooleanType separator,
  const ChannelStatistics *channel_statistics)
{
#define StatisticsFormat "      \"%s\": {\n        \"min\": %.*g,\n"  \
  "        \"max\": %.*g,\n        \"mean\": %.*g,\n        \"median\": %.*g,\n        "  \
  "\"standardDeviation\": %.*g,\n        \"kurtosis\": %.*g,\n        "\
  "\"skewness\": %.*g,\n        \"entropy\": %.*g\n      }"

  char
    *buffer;

  ssize_t
    n;

  buffer=AcquireString((char *) NULL);
  n=FormatLocaleString(buffer,MagickPathExtent,StatisticsFormat,name,
    GetMagickPrecision(),
    channel_statistics[channel].minima == MagickMaximumValue ? 0.0 : (double)
    ClampToQuantum(scale*channel_statistics[channel].minima),
    GetMagickPrecision(),
    channel_statistics[channel].maxima == -MagickMaximumValue ? 0.0 :
    (double) ClampToQuantum(scale*channel_statistics[channel].maxima),
    GetMagickPrecision(),scale*channel_statistics[channel].mean,
    GetMagickPrecision(),scale*channel_statistics[channel].median,
    GetMagickPrecision(),
    IsNaN(channel_statistics[channel].standard_deviation) != 0 ? MagickEpsilon :
    scale*channel_statistics[channel].standard_deviation,GetMagickPrecision(),
    channel_statistics[channel].kurtosis,GetMagickPrecision(),
    channel_statistics[channel].skewness,GetMagickPrecision(),
    channel_statistics[channel].entropy);
  (void) SubstituteString(&buffer,": -inf",": null");
  (void) SubstituteString(&buffer,": inf",": null");
  (void) SubstituteString(&buffer,": -nan",": null");
  (void) SubstituteString(&buffer,": nan",": null");
  n=FormatLocaleFile(file,"%s",buffer);
  buffer=DestroyString(buffer);
  if (separator != MagickFalse)
    (void) FormatLocaleFile(file,",");
  (void) FormatLocaleFile(file,"\n");
  return(n);
}

static void EncodeIptcProfile(FILE *file,const StringInfo *profile)
{
  char
    *attribute,
    **attribute_list;

  const char
    *tag;

  IPTCInfo
    *value,
    **values;

  long
    dataset,
    record,
    sentinel;

  ssize_t
    i,
    j,
    k;

  size_t
    count,
    length,
    profile_length;

  values=(IPTCInfo **) NULL;
  count=0;
  profile_length=GetStringInfoLength(profile);
  for (i=0; i < (ssize_t) profile_length; i+=(ssize_t) length)
  {
    length=1;
    sentinel=GetStringInfoDatum(profile)[i++];
    if (sentinel != 0x1c)
      continue;
    dataset=GetStringInfoDatum(profile)[i++];
    record=GetStringInfoDatum(profile)[i++];
    value=(IPTCInfo *) NULL;
    for (j=0; j < (ssize_t) count; j++)
    {
      if ((values[j]->record == record) && (values[j]->dataset == dataset))
        value=values[j];
    }
    if (value == (IPTCInfo *) NULL)
      {
        values=(IPTCInfo **) ResizeQuantumMemory(values,count+1,
          sizeof(*values));
        if (values == (IPTCInfo **) NULL)
          break;
        value=(IPTCInfo *) AcquireMagickMemory(sizeof(*value));
        if (value == (IPTCInfo *) NULL)
          break;
        /* Check the tag length in IPTCInfo when a new tag is added */
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
          default: tag="Unknown"; break;
        }
        (void) CopyMagickString(value->tag,tag,strlen(tag)+1);
        value->record=record;
        value->dataset=dataset;
        value->values=(char ***) NULL;
        value->values_length=0;
        values[count++]=value;
      }
    length=((size_t) GetStringInfoDatum(profile)[i++] << 8);
    length|=GetStringInfoDatum(profile)[i++];
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
            value->values=(char ***) ResizeQuantumMemory(value->values,
              value->values_length+1,
              sizeof(*value->values));
            if (value->values == (char ***) NULL)
              break;
            value->values[value->values_length++]=attribute_list;
          }
        attribute=DestroyString(attribute);
      }
  }
  if (values != (IPTCInfo **) NULL)
    {
      for (i=0; i < (ssize_t) count; i++)
      {
        value=values[i];
        (void) FormatLocaleFile(file,"        \"%s[%.20g,%.20g]\": ",
          value->tag,(double) value->dataset,(double) value->record);
        if (value->values_length == 0)
          (void) FormatLocaleFile(file,"null,");
        else
          {
            (void) FormatLocaleFile(file,"[");
            for (j=0; j < (ssize_t) value->values_length; j++)
            {
              for (k=0; value->values[j][k] != (char *) NULL; k++)
              {
                if (j > 0 || k > 0)
                  (void) FormatLocaleFile(file,",");
                JSONFormatLocaleFile(file,"%s",value->values[j][k]);
                value->values[j][k]=(char *) RelinquishMagickMemory(
                  value->values[j][k]);
              }
              value->values[j]=(char **) RelinquishMagickMemory(
                value->values[j]);
            }
            value->values=(char ***) RelinquishMagickMemory(value->values);
            (void) FormatLocaleFile(file,"],\n");
          }
        values[i]=(IPTCInfo *) RelinquishMagickMemory(values[i]);
      }
      values=(IPTCInfo **) RelinquishMagickMemory(values);
    }
}

static MagickBooleanType EncodeImageAttributes(Image *image,FILE *file,
  ExceptionInfo *exception)
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
    user_time,
    version;

  ImageType
    type;

  MagickBooleanType
    ping;

  size_t
    depth,
    distance;

  ssize_t
    i,
    x,
    y;

  struct stat
    properties;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *format='\0';
  elapsed_time=GetElapsedTime(&image->timer);
  user_time=GetUserTime(&image->timer);
  GetTimerInfo(&image->timer);
  ping=MagickTrue;
  if (GetVirtualPixels(image,0,0,1,1,exception) != (const Quantum *) NULL)
    ping=MagickFalse;
  (void) ping;
  (void) SignatureImage(image,exception);
  (void) FormatLocaleFile(file,"{\n");
  version=1.0;
  artifact=GetImageArtifact(image,"json:version");
  if (artifact != (const char *) NULL)
    version=StringToDouble(artifact,(char **) NULL);
  if (version >= 1.0)
    (void) FormatLocaleFile(file,"  \"version\": \"%.1f\",\n",version);
  if (*image->magick_filename == '\0')
    JSONFormatLocaleFile(file,"  \"image\": {\n    \"name\": %s,\n",
      image->filename);
  else
    {
      JSONFormatLocaleFile(file,"  \"image\": {\n    \"name\": %s,\n",
        image->magick_filename);
      if (LocaleCompare(image->magick_filename,image->filename) != 0)
        {   
          char
            filename[MagickPathExtent];
          
          GetPathComponent(image->magick_filename,TailPath,filename);
          JSONFormatLocaleFile(file,"    \"baseName\": %s,\n",filename);
        }
    }
  properties=(*GetBlobProperties(image));
  if (properties.st_mode != 0)
    (void) FormatLocaleFile(file,"    \"permissions\": %d%d%d,\n",
      (properties.st_mode >> 6) & 0x07,(properties.st_mode >> 3) & 0x07,
      (properties.st_mode >> 0) & 0x07);
  JSONFormatLocaleFile(file,"    \"format\": %s,\n",image->magick);
  magick_info=GetMagickInfo(image->magick,exception);
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickDescription(magick_info) != (const char *) NULL))
    JSONFormatLocaleFile(file,"    \"formatDescription\": %s,\n",
      GetMagickDescription(magick_info));
  if ((magick_info != (const MagickInfo *) NULL) &&
      (GetMagickMimeType(magick_info) != (const char *) NULL))
    JSONFormatLocaleFile(file,"    \"mimeType\": %s,\n",GetMagickMimeType(
      magick_info));
  JSONFormatLocaleFile(file,"    \"class\": %s,\n",CommandOptionToMnemonic(
    MagickClassOptions,(ssize_t) image->storage_class));
  (void) FormatLocaleFile(file,"    \"geometry\": {\n"
    "      \"width\": %g,\n      \"height\": %g,\n"
    "      \"x\": %g,\n      \"y\": %g\n    },\n",
    (double) image->columns,(double) image->rows,(double) image->tile_offset.x,
    (double) image->tile_offset.y);
  if ((image->magick_columns != 0) || (image->magick_rows != 0))
    if ((image->magick_columns != image->columns) ||
        (image->magick_rows != image->rows))
      (void) FormatLocaleFile(file,"    \"baseGeometry\": {\n"
        "      \"width\": %g,\n      \"height\": %g\n    },\n",(double)
        image->magick_columns,(double) image->magick_rows);
  if ((image->resolution.x != 0.0) && (image->resolution.y != 0.0))
    {
      (void) FormatLocaleFile(file,"    \"resolution\": {\n"
        "      \"x\": %g,\n      \"y\": %g\n    },\n",image->resolution.x,
        image->resolution.y);
      (void) FormatLocaleFile(file,"    \"printSize\": {\n"
        "      \"x\": %.*g,\n      \"y\": %.*g\n    },\n",GetMagickPrecision(),
        image->columns/image->resolution.x,GetMagickPrecision(),(double)
        image->rows/image->resolution.y);
    }
  JSONFormatLocaleFile(file,"    \"units\": %s,\n",CommandOptionToMnemonic(
    MagickResolutionOptions,(ssize_t) image->units));
  type=IdentifyImageCoderType(image,exception);
  JSONFormatLocaleFile(file,"    \"type\": %s,\n",CommandOptionToMnemonic(
    MagickTypeOptions,(ssize_t) type));
  if (image->type != type)
    JSONFormatLocaleFile(file,"    \"baseType\": %s,\n",
      CommandOptionToMnemonic(MagickTypeOptions,(ssize_t) image->type));
  if (version < 1.0)
    JSONFormatLocaleFile(file,"    \"endianess\": %s,\n",
      CommandOptionToMnemonic(MagickEndianOptions,(ssize_t) image->endian));
  else
    JSONFormatLocaleFile(file,"    \"endianness\": %s,\n",
      CommandOptionToMnemonic(MagickEndianOptions,(ssize_t) image->endian));
  locate=GetImageArtifact(image,"identify:locate");
  if (locate == (const char *) NULL)
    locate=GetImageArtifact(image,"json:locate");
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
      if (limit == (const char *) NULL)
        limit=GetImageArtifact(image,"json:limit");
      max_locations=0;
      if (limit != (const char *) NULL)
        max_locations=StringToUnsignedLong(limit);
      channel_statistics=GetLocationStatistics(image,statistic_type,exception);
      if (channel_statistics == (ChannelStatistics *) NULL)
        return(MagickFalse);
      (void) FormatLocaleFile(file,"    \"channel%s\": {\n",locate);
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelLocations(file,image,AlphaPixelChannel,"alpha",
          statistic_type,max_locations,MagickTrue,channel_statistics);
      switch (image->colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelLocations(file,image,RedPixelChannel,"red",
            statistic_type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,GreenPixelChannel,"green",
            statistic_type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,BluePixelChannel,"blue",
            statistic_type,max_locations,MagickFalse,channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelLocations(file,image,CyanPixelChannel,"cyan",
            statistic_type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,MagentaPixelChannel,
            "magenta",statistic_type,max_locations,MagickTrue,
            channel_statistics);
          (void) PrintChannelLocations(file,image,YellowPixelChannel,"yellow",
            statistic_type,max_locations,MagickTrue,channel_statistics);
          (void) PrintChannelLocations(file,image,BlackPixelChannel,"black",
            statistic_type,max_locations,MagickFalse,channel_statistics);
          break;
        }
        case LinearGRAYColorspace:
        case GRAYColorspace:
        {
          (void) PrintChannelLocations(file,image,GrayPixelChannel,"gray",
            statistic_type,max_locations,MagickFalse,channel_statistics);
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
  JSONFormatLocaleFile(file,"    \"colorspace\": %s,\n",
    CommandOptionToMnemonic(MagickColorspaceOptions,(ssize_t)
    image->colorspace));
  channel_statistics=(ChannelStatistics *) NULL;
  channel_moments=(ChannelMoments *) NULL;
  channel_phash=(ChannelPerceptualHash *) NULL;
  channel_features=(ChannelFeatures *) NULL;
  channel_statistics=GetImageStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  artifact=GetImageArtifact(image,"identify:moments");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"json:moments");
  if (artifact != (const char *) NULL)
    {
      channel_moments=GetImageMoments(image,exception);
      channel_phash=GetImagePerceptualHash(image,exception);
    }
  artifact=GetImageArtifact(image,"identify:features");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"json:features");
  if (artifact != (const char *) NULL)
    {
      distance=StringToUnsignedLong(artifact);
      channel_features=GetImageFeatures(image,distance,exception);
    }
  depth=GetImageDepth(image,exception);
  (void) FormatLocaleFile(file,"    \"depth\": %g,\n",(double) depth);
  (void) FormatLocaleFile(file,"    \"baseDepth\": %g,\n",(double)
    image->depth);
  (void) FormatLocaleFile(file,"    \"channelDepth\": {\n");
  if (image->alpha_trait != UndefinedPixelTrait)
    (void) FormatLocaleFile(file,"      \"alpha\": %.20g,\n",(double)
      channel_statistics[AlphaPixelChannel].depth);
  switch (image->colorspace)
  {
    case RGBColorspace:
    default:
    {
      (void) FormatLocaleFile(file,"      \"red\": %.20g,\n",(double)
        channel_statistics[RedChannel].depth);
      (void) FormatLocaleFile(file,"      \"green\": %.20g,\n",(double)
        channel_statistics[GreenChannel].depth);
      (void) FormatLocaleFile(file,"      \"blue\": %.20g\n",(double)
        channel_statistics[BlueChannel].depth);
      break;
    }
    case CMYKColorspace:
    {
      (void) FormatLocaleFile(file,"      \"cyan\": %.20g,\n",(double)
        channel_statistics[CyanChannel].depth);
      (void) FormatLocaleFile(file,"      \"magenta\": %.20g,\n",(double)
        channel_statistics[MagentaChannel].depth);
      (void) FormatLocaleFile(file,"      \"yellow\": %.20g,\n",(double)
        channel_statistics[YellowChannel].depth);
      (void) FormatLocaleFile(file,"      \"black\": %.20g\n",(double)
        channel_statistics[BlackChannel].depth);
      break;
    }
    case LinearGRAYColorspace:
    case GRAYColorspace:
    {
      (void) FormatLocaleFile(file,"      \"gray\": %.20g\n",(double)
        channel_statistics[GrayChannel].depth);
      break;
    }
  }
  (void) FormatLocaleFile(file,"    },\n");
  scale=1;
  if (image->depth <= MAGICKCORE_QUANTUM_DEPTH)
    scale=(double) (QuantumRange/((size_t) QuantumRange >> ((size_t)
      MAGICKCORE_QUANTUM_DEPTH-image->depth)));
  if (channel_statistics != (ChannelStatistics *) NULL)
    {
      (void) FormatLocaleFile(file,"    \"pixels\": %.20g,\n",
        channel_statistics[CompositePixelChannel].area);
      if ((image->colorspace != LinearGRAYColorspace) &&
          (image->colorspace != GRAYColorspace))
        {
          (void) FormatLocaleFile(file,"    \"imageStatistics\": {\n");
          (void) PrintChannelStatistics(file,(PixelChannel) MaxPixelChannels,
            "Overall",1.0/scale,MagickFalse,channel_statistics);
          (void) FormatLocaleFile(file,"    },\n");
        }
      (void) FormatLocaleFile(file,"    \"channelStatistics\": {\n");
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelStatistics(file,AlphaPixelChannel,"alpha",1.0/scale,
          MagickTrue,channel_statistics);
      switch (image->colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelStatistics(file,RedPixelChannel,"red",1.0/scale,
            MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,GreenPixelChannel,"green",1.0/
            scale,MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,BluePixelChannel,"blue",1.0/scale,
            MagickFalse,channel_statistics);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelStatistics(file,CyanPixelChannel,"cyan",1.0/scale,
            MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,MagentaPixelChannel,"magenta",1.0/
            scale,MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,YellowPixelChannel,"yellow",1.0/
            scale,MagickTrue,channel_statistics);
          (void) PrintChannelStatistics(file,BlackPixelChannel,"black",1.0/
            scale,MagickFalse,channel_statistics);
          break;
        }
        case LinearGRAYColorspace:
        case GRAYColorspace:
        {
          (void) PrintChannelStatistics(file,GrayPixelChannel,"gray",1.0/scale,
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
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelMoments(file,AlphaPixelChannel,"alpha",MagickTrue,
          channel_moments);
      switch (image->colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelMoments(file,RedPixelChannel,"red",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,GreenPixelChannel,"green",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,BluePixelChannel,"blue",MagickFalse,
            channel_moments);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelMoments(file,CyanPixelChannel,"cyan",MagickTrue,
            channel_moments);
          (void) PrintChannelMoments(file,MagentaPixelChannel,"magenta",
            MagickTrue,channel_moments);
          (void) PrintChannelMoments(file,YellowPixelChannel,"yellow",
            MagickTrue,channel_moments);
          (void) PrintChannelMoments(file,BlackPixelChannel,"black",
            MagickFalse,channel_moments);
          break;
        }
        case LinearGRAYColorspace:
        case GRAYColorspace:
        {
          (void) PrintChannelMoments(file,GrayPixelChannel,"gray",MagickFalse,
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
      (void) PrintChannelPerceptualHash(image,file,channel_phash);
      (void) FormatLocaleFile(file,"    },\n");
      channel_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(
        channel_phash);
    }
  if (channel_features != (ChannelFeatures *) NULL)
    {
      (void) FormatLocaleFile(file,"    \"channelFeatures\": {\n");
      if (image->alpha_trait != UndefinedPixelTrait)
        (void) PrintChannelFeatures(file,AlphaPixelChannel,"alpha",MagickTrue,
          channel_features);
      switch (image->colorspace)
      {
        case RGBColorspace:
        default:
        {
          (void) PrintChannelFeatures(file,RedPixelChannel,"red",MagickTrue,
            channel_features);
          (void) PrintChannelFeatures(file,GreenPixelChannel,"green",
            MagickTrue,channel_features);
          (void) PrintChannelFeatures(file,BluePixelChannel,"blue",MagickFalse,
            channel_features);
          break;
        }
        case CMYKColorspace:
        {
          (void) PrintChannelFeatures(file,CyanPixelChannel,"cyan",MagickTrue,
            channel_features);
          (void) PrintChannelFeatures(file,MagentaPixelChannel,"magenta",
            MagickTrue,channel_features);
          (void) PrintChannelFeatures(file,YellowPixelChannel,"yellow",
            MagickTrue,channel_features);
          (void) PrintChannelFeatures(file,BlackPixelChannel,"black",
            MagickFalse,channel_features);
          break;
        }
        case LinearGRAYColorspace:
        case GRAYColorspace:
        {
          (void) PrintChannelFeatures(file,GrayPixelChannel,"gray",MagickFalse,
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
        GetMagickPrecision(),100.0*GetImageTotalInkDensity(image,exception)/
        (double) QuantumRange);
    x=0;
    if (image->alpha_trait != UndefinedPixelTrait)
      {
        const Quantum
          *p;

        p=(const Quantum *) NULL;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (GetPixelAlpha(image,p) == (Quantum) TransparentAlpha)
              break;
            p+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (x < (ssize_t) image->columns)
            break;
        }
        if ((x < (ssize_t) image->columns) || (y < (ssize_t) image->rows))
          {
            PixelInfo
              pixel;

            GetPixelInfo(image,&pixel);
            GetPixelInfoPixel(image,p,&pixel);
            GetColorTuple(&pixel,MagickTrue,color);
            (void) FormatLocaleFile(file,"    \"alpha\": \"%s\",\n",color);
          }
      }
  if (image->storage_class == PseudoClass)
    {
      PixelInfo
        *magick_restrict p;

      (void) FormatLocaleFile(file,"    \"colormapEntries\": %.20g,\n",
        (double) image->colors);
      (void) FormatLocaleFile(file,"    \"colormap\": [\n      ");
      p=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        GetColorTuple(p,MagickTrue,color);
        (void) FormatLocaleFile(file,"\"%s\"",color);
        if (i < (ssize_t) (image->colors-1))
          (void) FormatLocaleFile(file,",");
        if (((i+1) % 5) == 0)
          (void) FormatLocaleFile(file,"\n      ");
        p++;
      }
      (void) FormatLocaleFile(file,"\n    ],\n");
    }
  if (image->error.mean_error_per_pixel != 0.0)
    (void) FormatLocaleFile(file,"    \"meanErrorPerPixel\": %g,\n",
      image->error.mean_error_per_pixel);
  if (image->error.normalized_mean_error != 0.0)
    (void) FormatLocaleFile(file,"    \"normalizedMeanError\": %g,\n",
      image->error.normalized_mean_error);
  if (image->error.normalized_maximum_error != 0.0)
    (void) FormatLocaleFile(file,"    \"normalizedMaximumError\": %g,\n",
      image->error.normalized_maximum_error);
  JSONFormatLocaleFile(file,"    \"renderingIntent\": %s,\n",
    CommandOptionToMnemonic(MagickIntentOptions,(ssize_t)
    image->rendering_intent));
  if (image->gamma != 0.0)
    (void) FormatLocaleFile(file,"    \"gamma\": %g,\n",image->gamma);
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
        "        \"x\": %g,\n        \"y\": %g\n      },\n",
        image->chromaticity.red_primary.x,image->chromaticity.red_primary.y);
      (void) FormatLocaleFile(file,"      \"greenPrimary\": {\n"
        "        \"x\": %g,\n        \"y\": %g\n      },\n",
        image->chromaticity.green_primary.x,
        image->chromaticity.green_primary.y);
      (void) FormatLocaleFile(file,"      \"bluePrimary\": {\n"
        "        \"x\": %g,\n        \"y\": %g\n      },\n",
        image->chromaticity.blue_primary.x,image->chromaticity.blue_primary.y);
      (void) FormatLocaleFile(file,"      \"whitePrimary\": {\n"
        "        \"x\": %g,\n        \"y\": %g\n      }\n",
        image->chromaticity.white_point.x,image->chromaticity.white_point.y);
      (void) FormatLocaleFile(file,"    },\n");
    }
  if ((image->extract_info.width*image->extract_info.height) != 0)
    (void) FormatLocaleFile(file,"    \"tileGeometry\": {\n"
      "      \"width\": %.20g,\n      \"height\": %.20g,\n"
      "      \"x\": %.20g,\n      \"y\": %.20g\n    },\n",
      (double) image->extract_info.width,(double) image->extract_info.height,
      (double) image->extract_info.x,(double) image->extract_info.y);
  GetColorTuple(&image->matte_color,MagickTrue,color);
  (void) FormatLocaleFile(file,"    \"matteColor\": \"%s\",\n",color);
  GetColorTuple(&image->background_color,MagickTrue,color);
  (void) FormatLocaleFile(file,"    \"backgroundColor\": \"%s\",\n",color);
  GetColorTuple(&image->border_color,MagickTrue,color);
  (void) FormatLocaleFile(file,"    \"borderColor\": \"%s\",\n",color);
  GetColorTuple(&image->transparent_color,MagickTrue,color);
  (void) FormatLocaleFile(file,"    \"transparentColor\": \"%s\",\n",color);
  JSONFormatLocaleFile(file,"    \"interlace\": %s,\n",CommandOptionToMnemonic(
    MagickInterlaceOptions,(ssize_t) image->interlace));
  JSONFormatLocaleFile(file,"    \"intensity\": %s,\n",CommandOptionToMnemonic(
    MagickPixelIntensityOptions,(ssize_t) image->intensity));
  JSONFormatLocaleFile(file,"    \"compose\": %s,\n",
    CommandOptionToMnemonic(MagickComposeOptions,(ssize_t) image->compose));
  if ((image->page.width != 0) || (image->page.height != 0) ||
      (image->page.x != 0) || (image->page.y != 0))
    (void) FormatLocaleFile(file,"    \"pageGeometry\": {\n"
      "      \"width\": %.20g,\n      \"height\": %.20g,\n"
      "      \"x\": %.20g,\n      \"y\": %.20g\n    },\n",
      (double) image->page.width,(double) image->page.height,
      (double) image->page.x,(double) image->page.y);
  if ((image->page.x != 0) || (image->page.y != 0))
    (void) FormatLocaleFile(file,"    \"originGeometry\": \"%+.20g%+.20g\",\n",
      (double) image->page.x,(double) image->page.y);
  JSONFormatLocaleFile(file,"    \"dispose\": %s,\n",
    CommandOptionToMnemonic(MagickDisposeOptions,(ssize_t) image->dispose));
  if (image->delay != 0)
    (void) FormatLocaleFile(file,"    \"delay\": \"%.20gx%.20g\",\n",
      (double) image->delay,(double) image->ticks_per_second);
  if (image->iterations != 1)
    (void) FormatLocaleFile(file,"    \"iterations\": %.20g,\n",(double)
      image->iterations);
  if ((image->next != (Image *) NULL) || (image->previous != (Image *) NULL))
    (void) FormatLocaleFile(file,"    \"scene\": %.20g,\n    \"scenes\": "
      "%.20g,\n",(double) image->scene,(double) GetImageListLength(image));
  else
    if (image->scene != 0)
      (void) FormatLocaleFile(file,"    \"scene\": %.20g,\n",(double)
        image->scene);
  JSONFormatLocaleFile(file,"    \"compression\": %s,\n",
    CommandOptionToMnemonic(MagickCompressOptions,(ssize_t)
    image->compression));
  if (image->quality != UndefinedCompressionQuality)
    (void) FormatLocaleFile(file,"    \"quality\": %.20g,\n",(double)
      image->quality);
  JSONFormatLocaleFile(file,"    \"orientation\": %s,\n",
    CommandOptionToMnemonic(MagickOrientationOptions,(ssize_t)
    image->orientation));
  if (image->montage != (char *) NULL)
    JSONFormatLocaleFile(file,"    \"montage\": \"%s\",\n",image->montage);
  if (image->directory != (char *) NULL)
    {
      Image
        *tile;

      ImageInfo
        *image_info;

      char
        *p,
        *q;

      WarningHandler
        handler;

      /*
        Display visual image directory.
      */
      image_info=AcquireImageInfo();
      (void) CloneString(&image_info->size,"64x64");
      (void) FormatLocaleFile(file,"    \"montageDirectory\": [");
      p=image->directory;
      while (*p != '\0')
      {
        q=p;
        while ((*q != '\xff') && (*q != '\0'))
          q++;
        (void) CopyMagickString(image_info->filename,p,(size_t) (q-p+1));
        p=q+1;
        JSONFormatLocaleFile(file,"{\n       \"name\": %s",image_info->filename);
        handler=SetWarningHandler((WarningHandler) NULL);
        tile=ReadImage(image_info,exception);
        (void) SetWarningHandler(handler);
        if (tile == (Image *) NULL)
          {
            (void) FormatLocaleFile(file,"    }");
            continue;
          }
        (void) FormatLocaleFile(file,",\n       \"info\": \"%.20gx%.20g %s\"",
          (double) tile->magick_columns,(double) tile->magick_rows,tile->magick);
        (void) SignatureImage(tile,exception);
        ResetImagePropertyIterator(tile);
        property=GetNextImageProperty(tile);
        while (property != (const char *) NULL)
        {
          JSONFormatLocaleFile(file,",\n       %s: ",property);
          value=GetImageProperty(tile,property,exception);
          JSONFormatLocaleFile(file,"%s",value);
          property=GetNextImageProperty(tile);
        }
        tile=DestroyImageList(tile);
        if (*p != '\0')
          (void) FormatLocaleFile(file,"\n    },");
        else
          (void) FormatLocaleFile(file,"\n    }");
      }
      (void) FormatLocaleFile(file,"],\n");
      image_info=DestroyImageInfo(image_info);
    }
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
        JSONFormatLocaleFile(file,"      %s: ",property);
        value=GetImageProperty(image,property,exception);
        JSONFormatLocaleFile(file,"%s",value);
        property=GetNextImageProperty(image);
      }
      (void) FormatLocaleFile(file,"\n    },\n");
    }
  (void) FormatLocaleString(key,MagickPathExtent,"8BIM:1999,2998:#1");
  value=GetImageProperty(image,key,exception);
  if (value != (const char *) NULL)
    {
      /*
        Display clipping path.
      */
      JSONFormatLocaleFile(file,"    \"clipping path\": %s,\n",value);
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
        JSONFormatLocaleFile(file,"      %s: {\n",name);
        if (LocaleCompare(name,"iptc") == 0)
          EncodeIptcProfile(file,profile);
        (void) FormatLocaleFile(file,"        \"length\": %.20g",(double)
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
        JSONFormatLocaleFile(file,"      %s: ",artifact);
        value=GetImageArtifact(image,artifact);
        JSONFormatLocaleFile(file,"%s",value);
        artifact=GetNextImageArtifact(image);
      }
      (void) FormatLocaleFile(file,"\n    },\n");
    }
  ResetImageRegistryIterator();
  registry=GetNextImageRegistry();
  if (registry != (const char *) NULL)
    {
      ssize_t
        n;

      /*
        Display image registry.
      */
      (void) FormatLocaleFile(file,"    \"registry\": {\n");
      n=0;
      while (registry != (const char *) NULL)
      {
        if (n++ != 0)
          (void) FormatLocaleFile(file,",\n");
        JSONFormatLocaleFile(file,"      %s: ",registry);
        value=(const char *) GetImageRegistry(StringRegistryType,registry,
          exception);
        JSONFormatLocaleFile(file,"%s",value);
        registry=GetNextImageRegistry();
      }
      (void) FormatLocaleFile(file,"    },\n");
    }
  (void) FormatLocaleFile(file,"    \"tainted\": %s,\n",
    image->taint != MagickFalse ? "true" : "false");
  (void) FormatMagickSize(image->extent,MagickFalse,"B",MagickPathExtent,
    format);
  JSONFormatLocaleFile(file,"    \"filesize\": %s,\n",format);
  (void) FormatMagickSize((MagickSizeType) image->columns*image->rows,
    MagickFalse,"B",MagickPathExtent,format);
  if (strlen(format) > 1)
    format[strlen(format)-1]='\0';
  JSONFormatLocaleFile(file,"    \"numberPixels\": %s,\n",format);
  (void) FormatMagickSize((MagickSizeType) ((double) image->columns*image->rows/
    elapsed_time+0.5),MagickFalse,"B",MagickPathExtent,format);
  JSONFormatLocaleFile(file,"    \"pixelsPerSecond\": %s,\n",format);
  (void) FormatLocaleFile(file,"    \"userTime\": \"%0.3fu\",\n",user_time);
  (void) FormatLocaleFile(file,"    \"elapsedTime\": \"%lu:%02lu.%03lu\",\n",
    (unsigned long) (elapsed_time/60.0),(unsigned long) ceil(fmod(
    elapsed_time,60.0)),(unsigned long) (1000.0*(elapsed_time-floor(
    elapsed_time))));
  JSONFormatLocaleFile(file,"    \"version\": %s\n",GetMagickVersion(
    (size_t *) NULL));
  (void) FormatLocaleFile(file,"  }\n}");
  (void) fflush(file);
  return(ferror(file) != 0 ? MagickFalse : MagickTrue);
}

static MagickBooleanType WriteJSONImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  FILE
    *file;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  size_t
    number_scenes;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  file=GetBlobFileHandle(image);
  if (file == (FILE *) NULL)
    file=stdout;
  scene=0;
  number_scenes=GetImageListLength(image);
  do
  {
    if (scene == 0)
      (void) WriteBlobString(image,"[");
    image->magick_columns=image->columns;
    image->magick_rows=image->rows;
    status=EncodeImageAttributes(image,file,exception);
    if (status == MagickFalse)
      break;
    if (GetNextImageInList(image) == (Image *) NULL)
      {
        (void) WriteBlobString(image,"]");
        break;
      }
    (void) WriteBlobString(image,",\n");
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

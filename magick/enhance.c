/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              EEEEE  N   N  H   H   AAA   N   N   CCCC  EEEEE                %
%              E      NN  N  H   H  A   A  NN  N  C      E                    %
%              EEE    N N N  HHHHH  AAAAA  N N N  C      EEE                  %
%              E      N  NN  H   H  A   A  N  NN  C      E                    %
%              EEEEE  N   N  H   H  A   A  N   N   CCCC  EEEEE                %
%                                                                             %
%                                                                             %
%                    MagickCore Image Enhancement Methods                     %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/cache.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/composite-private.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/histogram.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/quantum.h"
#include "magick/quantum-private.h"
#include "magick/resample.h"
#include "magick/resample-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/token.h"
#include "magick/xml-tree.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A u t o G a m m a I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AutoGammaImage() extract the 'mean' from the image and adjust the image
%  to try make set its gamma appropriatally.
%
%  The format of the LevelImage method is:
%
%      MagickBooleanType AutoGammaImage(Image *image)
%      MagickBooleanType AutoGammaImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o channel: The channels to auto-level.  If the special 'SyncChannels'
%      flag is set all given channels is adjusted in the same way using the
%      mean average of those channels.
%
*/

MagickExport MagickBooleanType AutoGammaImage(Image *image)
{
  return(AutoGammaImageChannel(image,DefaultChannels));
}

MagickExport MagickBooleanType AutoGammaImageChannel(Image *image,
  const ChannelType channel)
{
  MagickStatusType
    status;

  double
    mean,junk,gamma,logmean;

  logmean=log(0.5);

  if ((channel & SyncChannels) != 0 )
    {
      /*
        Apply gamma correction equally accross all given channels
      */
      GetImageChannelMean(image, channel, &mean, &junk, &image->exception);
      gamma = log(mean*QuantumScale)/logmean;
      //return GammaImageChannel(image, channel, gamma);
      return LevelImageChannel(image, channel,
                               0.0, (double)QuantumRange, gamma);
    }

  /*
    auto-gamma each channel separateally
  */
  status = MagickTrue;
  if ((channel & RedChannel) != 0)
    {
      GetImageChannelMean(image, RedChannel, &mean, &junk, &image->exception);
      gamma = log(mean*QuantumScale)/logmean;
      //status = status && GammaImageChannel(image, RedChannel, gamma);
      status = status && LevelImageChannel(image, RedChannel,
                               0.0, (double)QuantumRange, gamma);
    }
  if ((channel & GreenChannel) != 0)
    {
      GetImageChannelMean(image, GreenChannel, &mean, &junk, &image->exception);
      gamma = log(mean*QuantumScale)/logmean;
      //status = status && GammaImageChannel(image, GreenChannel, gamma);
      status = status && LevelImageChannel(image, GreenChannel,
                               0.0, (double)QuantumRange, gamma);
    }
  if ((channel & BlueChannel) != 0)
    {
      GetImageChannelMean(image, BlueChannel, &mean, &junk, &image->exception);
      gamma = log(mean*QuantumScale)/logmean;
      //status = status && GammaImageChannel(image, BlueChannel, gamma);
      status = status && LevelImageChannel(image, BlueChannel,
                               0.0, (double)QuantumRange, gamma);
    }
  if (((channel & OpacityChannel) != 0) &&
      (image->matte == MagickTrue))
    {
      GetImageChannelMean(image, OpacityChannel, &mean, &junk, &image->exception);
      gamma = log(mean*QuantumScale)/logmean;
      //status = status && GammaImageChannel(image, OpacityChannel, gamma);
      status = status && LevelImageChannel(image, OpacityChannel,
                               0.0, (double)QuantumRange, gamma);
    }
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    {
      GetImageChannelMean(image, IndexChannel, &mean, &junk, &image->exception);
      gamma = log(mean*QuantumScale)/logmean;
      //status = status && GammaImageChannel(image, IndexChannel, gamma);
      status = status && LevelImageChannel(image, IndexChannel,
                               0.0, (double)QuantumRange, gamma);
    }
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A u t o L e v e l I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AutoLevelImage() adjusts the levels of a particular image channel by
%  scaling the minimum and maximum values to the full quantum range.
%
%  The format of the LevelImage method is:
%
%      MagickBooleanType AutoLevelImage(Image *image)
%      MagickBooleanType AutoLevelImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o channel: The channels to auto-level.  If the special 'SyncChannels'
%      flag is set the min/max/mean value of all given channels is used for
%      all given channels, to all channels in the same way.
%
*/

MagickExport MagickBooleanType AutoLevelImage(Image *image)
{
  return(AutoLevelImageChannel(image,DefaultChannels));
}

MagickExport MagickBooleanType AutoLevelImageChannel(Image *image,
  const ChannelType channel)
{
  /*
    This is simply a convenience function around a Min/Max Histogram Stretch
  */
  return MinMaxStretchImage(image, channel, 0.0, 0.0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o l o r D e c i s i o n L i s t I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorDecisionListImage() accepts a lightweight Color Correction Collection
%  (CCC) file which solely contains one or more color corrections and applies
%  the correction to the image.  Here is a sample CCC file:
%
%    <ColorCorrectionCollection xmlns="urn:ASC:CDL:v1.2">
%          <ColorCorrection id="cc03345">
%                <SOPNode>
%                     <Slope> 0.9 1.2 0.5 </Slope>
%                     <Offset> 0.4 -0.5 0.6 </Offset>
%                     <Power> 1.0 0.8 1.5 </Power>
%                </SOPNode>
%                <SATNode>
%                     <Saturation> 0.85 </Saturation>
%                </SATNode>
%          </ColorCorrection>
%    </ColorCorrectionCollection>
%
%  which includes the slop, offset, and power for each of the RGB channels
%  as well as the saturation.
%
%  The format of the ColorDecisionListImage method is:
%
%      MagickBooleanType ColorDecisionListImage(Image *image,
%        const char *color_correction_collection)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o color_correction_collection: the color correction collection in XML.
%
*/
MagickExport MagickBooleanType ColorDecisionListImage(Image *image,
  const char *color_correction_collection)
{
#define ColorDecisionListCorrectImageTag  "ColorDecisionList/Image"

  typedef struct _Correction
  {
    double
      slope,
      offset,
      power;
  } Correction;

  typedef struct _ColorCorrection
  {
    Correction
      red,
      green,
      blue;

    double
      saturation;
  } ColorCorrection;

  char
    token[MaxTextExtent];

  ColorCorrection
    color_correction;

  const char
    *content,
    *p;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  PixelPacket
    *cdl_map;

  register long
    i;

  XMLTreeInfo
    *cc,
    *ccc,
    *sat,
    *sop;

  CacheView
    *image_view;

  /*
    Allocate and initialize cdl maps.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (color_correction_collection == (const char *) NULL)
    return(MagickFalse);
  ccc=NewXMLTree((const char *) color_correction_collection,&image->exception);
  if (ccc == (XMLTreeInfo *) NULL)
    return(MagickFalse);
  cc=GetXMLTreeChild(ccc,"ColorCorrection");
  if (cc == (XMLTreeInfo *) NULL)
    {
      ccc=DestroyXMLTree(ccc);
      return(MagickFalse);
    }
  color_correction.red.slope=1.0;
  color_correction.red.offset=0.0;
  color_correction.red.power=1.0;
  color_correction.green.slope=1.0;
  color_correction.green.offset=0.0;
  color_correction.green.power=1.0;
  color_correction.blue.slope=1.0;
  color_correction.blue.offset=0.0;
  color_correction.blue.power=1.0;
  color_correction.saturation=0.0;
  sop=GetXMLTreeChild(cc,"SOPNode");
  if (sop != (XMLTreeInfo *) NULL)
    {
      XMLTreeInfo
        *offset,
        *power,
        *slope;

      slope=GetXMLTreeChild(sop,"Slope");
      if (slope != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(slope);
          p=(const char *) content;
          for (i=0; (*p != '\0') && (i < 3); i++)
          {
            GetMagickToken(p,&p,token);
            if (*token == ',')
              GetMagickToken(p,&p,token);
            switch (i)
            {
              case 0: color_correction.red.slope=atof(token); break;
              case 1: color_correction.green.slope=atof(token); break;
              case 2: color_correction.blue.slope=atof(token); break;
            }
          }
        }
      offset=GetXMLTreeChild(sop,"Offset");
      if (offset != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(offset);
          p=(const char *) content;
          for (i=0; (*p != '\0') && (i < 3); i++)
          {
            GetMagickToken(p,&p,token);
            if (*token == ',')
              GetMagickToken(p,&p,token);
            switch (i)
            {
              case 0: color_correction.red.offset=atof(token); break;
              case 1: color_correction.green.offset=atof(token); break;
              case 2: color_correction.blue.offset=atof(token); break;
            }
          }
        }
      power=GetXMLTreeChild(sop,"Power");
      if (power != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(power);
          p=(const char *) content;
          for (i=0; (*p != '\0') && (i < 3); i++)
          {
            GetMagickToken(p,&p,token);
            if (*token == ',')
              GetMagickToken(p,&p,token);
            switch (i)
            {
              case 0: color_correction.red.power=atof(token); break;
              case 1: color_correction.green.power=atof(token); break;
              case 2: color_correction.blue.power=atof(token); break;
            }
          }
        }
    }
  sat=GetXMLTreeChild(cc,"SATNode");
  if (sat != (XMLTreeInfo *) NULL)
    {
      XMLTreeInfo
        *saturation;

      saturation=GetXMLTreeChild(sat,"Saturation");
      if (saturation != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(saturation);
          p=(const char *) content;
          GetMagickToken(p,&p,token);
          color_correction.saturation=atof(token);
        }
    }
  ccc=DestroyXMLTree(ccc);
  if (image->debug != MagickFalse)
    {
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  Color Correction Collection:");
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.red.slope: %g",color_correction.red.slope);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.red.offset: %g",color_correction.red.offset);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.red.power: %g",color_correction.red.power);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.green.slope: %g",color_correction.green.slope);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.green.offset: %g",color_correction.green.offset);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.green.power: %g",color_correction.green.power);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.blue.slope: %g",color_correction.blue.slope);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.blue.offset: %g",color_correction.blue.offset);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.blue.power: %g",color_correction.blue.power);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.saturation: %g",color_correction.saturation);
    }
  cdl_map=(PixelPacket *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*cdl_map));
  if (cdl_map == (PixelPacket *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1)
#endif
  for (i=0; i <= (long) MaxMap; i++)
  {
    cdl_map[i].red=RoundToQuantum((MagickRealType) ScaleMapToQuantum((
      MagickRealType) (MaxMap*(pow(color_correction.red.slope*i/MaxMap+
      color_correction.red.offset,color_correction.red.power)))));
    cdl_map[i].green=RoundToQuantum((MagickRealType) ScaleMapToQuantum((
      MagickRealType) (MaxMap*(pow(color_correction.green.slope*i/MaxMap+
      color_correction.green.offset,color_correction.green.power)))));
    cdl_map[i].blue=RoundToQuantum((MagickRealType) ScaleMapToQuantum((
      MagickRealType) (MaxMap*(pow(color_correction.blue.slope*i/MaxMap+
      color_correction.blue.offset,color_correction.blue.power)))));
  }
  if (image->storage_class == PseudoClass)
    {
      /*
        Apply transfer function to colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        double
          luma;

        luma=0.2126*image->colormap[i].red+0.7152*image->colormap[i].green+
          0.0722*image->colormap[i].blue;
        image->colormap[i].red=RoundToQuantum(luma+color_correction.saturation*
          cdl_map[ScaleQuantumToMap(image->colormap[i].red)].red-luma);
        image->colormap[i].green=RoundToQuantum(luma+
          color_correction.saturation*cdl_map[ScaleQuantumToMap(
          image->colormap[i].green)].green-luma);
        image->colormap[i].blue=RoundToQuantum(luma+color_correction.saturation*
          cdl_map[ScaleQuantumToMap(image->colormap[i].blue)].blue-luma);
      }
    }
  /*
    Apply transfer function to image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    double
      luma;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      luma=0.2126*q->red+0.7152*q->green+0.0722*q->blue;
      q->red=RoundToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(q->red)].red-luma));
      q->green=RoundToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(q->green)].green-luma));
      q->blue=RoundToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(q->blue)].blue-luma));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ColorDecisionListImageChannel)
#endif
        proceed=SetImageProgress(image,ColorDecisionListCorrectImageTag,
          progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  cdl_map=(PixelPacket *) RelinquishMagickMemory(cdl_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C l u t I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClutImage() replaces each color value in the given image, by using it as an
%  index to lookup a replacement color value in a Color Look UP Table in the
%  form of an image.  The values are extracted along a diagonal of the CLUT
%  image so either a horizontal or vertial gradient image can be used.
%
%  Typically this is used to either re-color a gray-scale image according to a
%  color gradient in the CLUT image, or to perform a freeform histogram
%  (level) adjustment according to the (typically gray-scale) gradient in the
%  CLUT image.
%
%  When the 'channel' mask includes the matte/alpha transparency channel but
%  one image has no such channel it is assumed that that image is a simple
%  gray-scale image that will effect the alpha channel values, either for
%  gray-scale coloring (with transparent or semi-transparent colors), or
%  a histogram adjustment of existing alpha channel values.   If both images
%  have matte channels, direct and normal indexing is applied, which is rarely
%  used.
%
%  The format of the ClutImage method is:
%
%      MagickBooleanType ClutImage(Image *image,Image *clut_image)
%      MagickBooleanType ClutImageChannel(Image *image,
%        const ChannelType channel,Image *clut_image)
%
%  A description of each parameter follows:
%
%    o image: the image, which is replaced by indexed CLUT values
%
%    o clut_image: the color lookup table image for replacement color values.
%
%    o channel: the channel.
%
*/

MagickExport MagickBooleanType ClutImage(Image *image,const Image *clut_image)
{
  return(ClutImageChannel(image,DefaultChannels,clut_image));
}

MagickExport MagickBooleanType ClutImageChannel(Image *image,
  const ChannelType channel,const Image *clut_image)
{
#define ClutImageTag  "Clut/Image"

  ExceptionInfo
    *exception;

  long
    adjust,
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  ResampleFilter
    **resample_filter;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(clut_image != (Image *) NULL);
  assert(clut_image->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  /*
    Clut image.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(clut_image,&zero);
  adjust=clut_image->interpolate == IntegerInterpolatePixel ? 0 : 1;
  exception=(&image->exception);
  resample_filter=AcquireResampleFilterThreadSet(clut_image,MagickTrue,
    exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register IndexPacket
      *__restrict indexes;

    register long
      id,
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    pixel=zero;
    id=GetOpenMPThreadId();
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        PROGRAMMERS WARNING:

        Apply OpacityChannel BEFORE the color channels.  Do not re-order.

        The handling special case 2 (coloring gray-scale), requires access to
        the unmodified colors of the original image to determine the index
        value.  As such alpha/matte channel handling must be performed BEFORE,
        any of the color channels are modified.

      */
      if ((channel & OpacityChannel) != 0)
        {
          if (clut_image->matte == MagickFalse)
            {
              /*
                A gray-scale LUT replacement for an image alpha channel.
              */
              (void) ResamplePixelColor(resample_filter[id],QuantumScale*
                (QuantumRange-q->opacity)*(clut_image->columns+adjust),
                QuantumScale*(QuantumRange-q->opacity)*(clut_image->rows+
                adjust),&pixel);
              q->opacity=(Quantum) (QuantumRange-MagickPixelIntensityToQuantum(
                &pixel));
            }
          else
            if (image->matte == MagickFalse)
              {
                /*
                  A greyscale image being colored by a LUT with transparency.
                */
                (void) ResamplePixelColor(resample_filter[id],QuantumScale*
                  PixelIntensity(q)*(clut_image->columns-adjust),QuantumScale*
                  PixelIntensity(q)*(clut_image->rows-adjust),&pixel);
                q->opacity=RoundToQuantum(pixel.opacity);
              }
            else
              {
                /*
                  Direct alpha channel lookup.
                */
                (void) ResamplePixelColor(resample_filter[id],QuantumScale*
                  q->opacity*(clut_image->columns-adjust),QuantumScale*
                  q->opacity* (clut_image->rows-adjust),&pixel);
                q->opacity=RoundToQuantum(pixel.opacity);
              }
        }
      if ((channel & RedChannel) != 0)
        {
          (void) ResamplePixelColor(resample_filter[id],QuantumScale*q->red*
            (clut_image->columns-adjust),QuantumScale*q->red*
            (clut_image->rows-adjust),&pixel);
          q->red=RoundToQuantum(pixel.red);
        }
      if ((channel & GreenChannel) != 0)
        {
          (void) ResamplePixelColor(resample_filter[id],QuantumScale*q->green*
            (clut_image->columns-adjust),QuantumScale*q->green*
            (clut_image->rows-adjust),&pixel);
          q->green=RoundToQuantum(pixel.green);
        }
      if ((channel & BlueChannel) != 0)
        {
          (void) ResamplePixelColor(resample_filter[id],QuantumScale*q->blue*
            (clut_image->columns-adjust),QuantumScale*q->blue*
            (clut_image->rows-adjust),&pixel);
          q->blue=RoundToQuantum(pixel.blue);
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          (void) ResamplePixelColor(resample_filter[id],QuantumScale*indexes[x]*
            (clut_image->columns-adjust),QuantumScale*indexes[x]*
            (clut_image->rows-adjust),&pixel);
          indexes[x]=RoundToQuantum(pixel.index);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ClutImageChannel)
#endif
        proceed=SetImageProgress(image,ClutImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  resample_filter=DestroyResampleFilterThreadSet(resample_filter);
  /*
    Enable alpha channel if CLUT image could enable it.
  */
  if ((clut_image->matte != MagickFalse) && ((channel & OpacityChannel) != 0))
    (void) SetImageAlphaChannel(image,ActivateAlphaChannel);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n t r a s t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ContrastImage() enhances the intensity differences between the lighter and
%  darker elements of the image.  Set sharpen to a MagickTrue to increase the
%  image contrast otherwise the contrast is reduced.
%
%  The format of the ContrastImage method is:
%
%      MagickBooleanType ContrastImage(Image *image,
%        const MagickBooleanType sharpen)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o sharpen: Increase or decrease image contrast.
%
*/

static void Contrast(const int sign,Quantum *red,Quantum *green,Quantum *blue)
{
  double
    brightness,
    hue,
    saturation;

  /*
    Enhance contrast: dark color become darker, light color become lighter.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  hue=0.0;
  saturation=0.0;
  brightness=0.0;
  ConvertRGBToHSB(*red,*green,*blue,&hue,&saturation,&brightness);
  brightness+=0.5*sign*(0.5*(sin(MagickPI*(brightness-0.5))+1.0)-brightness);
  if (brightness > 1.0)
    brightness=1.0;
  else
    if (brightness < 0.0)
      brightness=0.0;
  ConvertHSBToRGB(hue,saturation,brightness,red,green,blue);
}

MagickExport MagickBooleanType ContrastImage(Image *image,
  const MagickBooleanType sharpen)
{
#define ContrastImageTag  "Contrast/Image"

  ExceptionInfo
    *exception;

  int
    sign;

  long
    progress,
    y;

  MagickBooleanType
    status;

  register long
    i;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  sign=sharpen != MagickFalse ? 1 : -1;
  if (image->storage_class == PseudoClass)
    {
      /*
        Contrast enhance colormap.
      */
      for (i=0; i < (long) image->colors; i++)
        Contrast(sign,&image->colormap[i].red,&image->colormap[i].green,
          &image->colormap[i].blue);
    }
  /*
    Contrast enhance image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      Contrast(sign,&q->red,&q->green,&q->blue);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ContrastImage)
#endif
        proceed=SetImageProgress(image,ContrastImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n t r a s t S t r e t c h I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The ContrastStretchImage() is a simple image enhancement technique that
%  attempts to improve the contrast in an image by `stretching' the range of
%  intensity values it contains to span a desired range of values. It differs
%  from the more sophisticated histogram equalization in that it can only
%  apply %  a linear scaling function to the image pixel values.  As a result
%  the `enhancement' is less harsh.
%
%  The format of the ContrastStretchImage method is:
%
%      MagickBooleanType ContrastStretchImage(Image *image,
%        const char *levels)
%      MagickBooleanType ContrastStretchImageChannel(Image *image,
%        const unsigned long channel,const double black_point,
%        const double white_point)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o black_point: the black point.
%
%    o white_point: the white point.
%
%    o levels: Specify the levels where the black and white points have the
%      range of 0 to number-of-pixels (e.g. 1%, 10x90%, etc.).
%
*/

MagickExport MagickBooleanType ContrastStretchImage(Image *image,
  const char *levels)
{
  double
    black_point,
    white_point;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  /*
    Parse levels.
  */
  if (levels == (char *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(levels,&geometry_info);
  black_point=geometry_info.rho;
  white_point=(double) image->columns*image->rows;
  if ((flags & SigmaValue) != 0)
    white_point=geometry_info.sigma;
  if ((flags & PercentValue) != 0)
    {
      black_point*=(double) QuantumRange/100.0;
      white_point*=(double) QuantumRange/100.0;
    }
  if ((flags & SigmaValue) == 0)
    white_point=(double) image->columns*image->rows-black_point;
  status=ContrastStretchImageChannel(image,DefaultChannels,black_point,
    white_point);
  return(status);
}

MagickExport MagickBooleanType ContrastStretchImageChannel(Image *image,
  const ChannelType channel,const double black_point,const double white_point)
{
#define MaxRange(color)  ((MagickRealType) ScaleQuantumToMap((Quantum) (color)))
#define ContrastStretchImageTag  "ContrastStretch/Image"

  double
    intensity;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    black,
    *histogram,
    *stretch_map,
    white;

  register long
    i;

  CacheView
    *image_view;

  /*
    Allocate histogram and stretch map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  histogram=(MagickPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*histogram));
  stretch_map=(MagickPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*stretch_map));
  if ((histogram == (MagickPixelPacket *) NULL) ||
      (stretch_map == (MagickPixelPacket *) NULL))
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Form histogram.
  */
  status=MagickTrue;
  exception=(&image->exception);
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));
  image_view=AcquireCacheView(image);
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *__restrict p;

    register IndexPacket
      *__restrict indexes;

    register long
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    if (channel == DefaultChannels)
      for (x=0; x < (long) image->columns; x++)
      {
        Quantum
          intensity;

        intensity=PixelIntensityToQuantum(p);
        histogram[ScaleQuantumToMap(intensity)].red++;
        histogram[ScaleQuantumToMap(intensity)].green++;
        histogram[ScaleQuantumToMap(intensity)].blue++;
        histogram[ScaleQuantumToMap(intensity)].index++;
        p++;
      }
    else
      for (x=0; x < (long) image->columns; x++)
      {
        if ((channel & RedChannel) != 0)
          histogram[ScaleQuantumToMap(p->red)].red++;
        if ((channel & GreenChannel) != 0)
          histogram[ScaleQuantumToMap(p->green)].green++;
        if ((channel & BlueChannel) != 0)
          histogram[ScaleQuantumToMap(p->blue)].blue++;
        if ((channel & OpacityChannel) != 0)
          histogram[ScaleQuantumToMap(p->opacity)].opacity++;
        if (((channel & IndexChannel) != 0) &&
            (image->colorspace == CMYKColorspace))
          histogram[ScaleQuantumToMap(indexes[x])].index++;
        p++;
      }
  }
  /*
    Find the histogram boundaries by locating the black/white levels.
  */
  black.red=0.0;
  white.red=MaxRange(QuantumRange);
  if ((channel & RedChannel) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (long) MaxMap; i++)
      {
        intensity+=histogram[i].red;
        if (intensity > black_point)
          break;
      }
      black.red=(MagickRealType) i;
      intensity=0.0;
      for (i=(long) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].red;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.red=(MagickRealType) i;
    }
  black.green=0.0;
  white.green=MaxRange(QuantumRange);
  if ((channel & GreenChannel) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (long) MaxMap; i++)
      {
        intensity+=histogram[i].green;
        if (intensity > black_point)
          break;
      }
      black.green=(MagickRealType) i;
      intensity=0.0;
      for (i=(long) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].green;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.green=(MagickRealType) i;
    }
  black.blue=0.0;
  white.blue=MaxRange(QuantumRange);
  if ((channel & BlueChannel) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (long) MaxMap; i++)
      {
        intensity+=histogram[i].blue;
        if (intensity > black_point)
          break;
      }
      black.blue=(MagickRealType) i;
      intensity=0.0;
      for (i=(long) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].blue;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.blue=(MagickRealType) i;
    }
  black.opacity=0.0;
  white.opacity=MaxRange(QuantumRange);
  if ((channel & OpacityChannel) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (long) MaxMap; i++)
      {
        intensity+=histogram[i].opacity;
        if (intensity > black_point)
          break;
      }
      black.opacity=(MagickRealType) i;
      intensity=0.0;
      for (i=(long) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].opacity;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.opacity=(MagickRealType) i;
    }
  black.index=0.0;
  white.index=MaxRange(QuantumRange);
  if (((channel & IndexChannel) != 0) && (image->colorspace == CMYKColorspace))
    {
      intensity=0.0;
      for (i=0; i <= (long) MaxMap; i++)
      {
        intensity+=histogram[i].index;
        if (intensity > black_point)
          break;
      }
      black.index=(MagickRealType) i;
      intensity=0.0;
      for (i=(long) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].index;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.index=(MagickRealType) i;
    }
  histogram=(MagickPixelPacket *) RelinquishMagickMemory(histogram);
  /*
    Stretch the histogram to create the stretched image mapping.
  */
  (void) ResetMagickMemory(stretch_map,0,(MaxMap+1)*sizeof(*stretch_map));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (i=0; i <= (long) MaxMap; i++)
  {
    if ((channel & RedChannel) != 0)
      {
        if (i < (long) black.red)
          stretch_map[i].red=0.0;
        else
          if (i > (long) white.red)
            stretch_map[i].red=(MagickRealType) QuantumRange;
          else
            if (black.red != white.red)
              stretch_map[i].red=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.red)/(white.red-black.red)));
      }
    if ((channel & GreenChannel) != 0)
      {
        if (i < (long) black.green)
          stretch_map[i].green=0.0;
        else
          if (i > (long) white.green)
            stretch_map[i].green=(MagickRealType) QuantumRange;
          else
            if (black.green != white.green)
              stretch_map[i].green=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.green)/(white.green-
                black.green)));
      }
    if ((channel & BlueChannel) != 0)
      {
        if (i < (long) black.blue)
          stretch_map[i].blue=0.0;
        else
          if (i > (long) white.blue)
            stretch_map[i].blue=(MagickRealType) QuantumRange;
          else
            if (black.blue != white.blue)
              stretch_map[i].blue=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.blue)/(white.blue-
                black.blue)));
      }
    if ((channel & OpacityChannel) != 0)
      {
        if (i < (long) black.opacity)
          stretch_map[i].opacity=0.0;
        else
          if (i > (long) white.opacity)
            stretch_map[i].opacity=(MagickRealType) QuantumRange;
          else
            if (black.opacity != white.opacity)
              stretch_map[i].opacity=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.opacity)/(white.opacity-
                black.opacity)));
      }
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace))
      {
        if (i < (long) black.index)
          stretch_map[i].index=0.0;
        else
          if (i > (long) white.index)
            stretch_map[i].index=(MagickRealType) QuantumRange;
          else
            if (black.index != white.index)
              stretch_map[i].index=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.index)/(white.index-
                black.index)));
      }
  }
  /*
    Stretch the image.
  */
  if (((channel & OpacityChannel) != 0) || (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace)))
    image->storage_class=DirectClass;
  if (image->storage_class == PseudoClass)
    {
      /*
        Stretch colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        if ((channel & RedChannel) != 0)
          {
            if (black.red != white.red)
              image->colormap[i].red=RoundToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].red)].red);
          }
        if ((channel & GreenChannel) != 0)
          {
            if (black.green != white.green)
              image->colormap[i].green=RoundToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].green)].green);
          }
        if ((channel & BlueChannel) != 0)
          {
            if (black.blue != white.blue)
              image->colormap[i].blue=RoundToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].blue)].blue);
          }
        if ((channel & OpacityChannel) != 0)
          {
            if (black.opacity != white.opacity)
              image->colormap[i].opacity=RoundToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].opacity)].opacity);
          }
      }
    }
  /*
    Stretch image.
  */
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          if (black.red != white.red)
            q->red=RoundToQuantum(stretch_map[ScaleQuantumToMap(q->red)].red);
        }
      if ((channel & GreenChannel) != 0)
        {
          if (black.green != white.green)
            q->green=RoundToQuantum(stretch_map[ScaleQuantumToMap(
              q->green)].green);
        }
      if ((channel & BlueChannel) != 0)
        {
          if (black.blue != white.blue)
            q->blue=RoundToQuantum(stretch_map[ScaleQuantumToMap(
              q->blue)].blue);
        }
      if ((channel & OpacityChannel) != 0)
        {
          if (black.opacity != white.opacity)
            q->opacity=RoundToQuantum(stretch_map[ScaleQuantumToMap(
              q->opacity)].opacity);
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if (black.index != white.index)
            indexes[x]=(IndexPacket) RoundToQuantum(stretch_map[
              ScaleQuantumToMap(indexes[x])].index);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ContrastStretchImageChannel)
#endif
        proceed=SetImageProgress(image,ContrastStretchImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  stretch_map=(MagickPixelPacket *) RelinquishMagickMemory(stretch_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E n h a n c e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EnhanceImage() applies a digital filter that improves the quality of a
%  noisy image.
%
%  The format of the EnhanceImage method is:
%
%      Image *EnhanceImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *EnhanceImage(const Image *image,ExceptionInfo *exception)
{
#define Enhance(weight) \
  mean=((MagickRealType) r->red+pixel.red)/2; \
  distance=(MagickRealType) r->red-(MagickRealType) pixel.red; \
  distance_squared=QuantumScale*(2.0*((MagickRealType) QuantumRange+1.0)+ \
     mean)*distance*distance; \
  mean=((MagickRealType) r->green+pixel.green)/2; \
  distance=(MagickRealType) r->green-(MagickRealType) pixel.green; \
  distance_squared+=4.0*distance*distance; \
  mean=((MagickRealType) r->blue+pixel.blue)/2; \
  distance=(MagickRealType) r->blue-(MagickRealType) pixel.blue; \
  distance_squared+=QuantumScale*(3.0*((MagickRealType) \
    QuantumRange+1.0)-1.0-mean)*distance*distance; \
  mean=((MagickRealType) r->opacity+pixel.opacity)/2; \
  distance=(MagickRealType) r->opacity-(MagickRealType) pixel.opacity; \
  distance_squared+=QuantumScale*(3.0*((MagickRealType) \
    QuantumRange+1.0)-1.0-mean)*distance*distance; \
  if (distance_squared < ((MagickRealType) QuantumRange*(MagickRealType) \
      QuantumRange/25.0f)) \
    { \
      aggregate.red+=(weight)*r->red; \
      aggregate.green+=(weight)*r->green; \
      aggregate.blue+=(weight)*r->blue; \
      aggregate.opacity+=(weight)*r->opacity; \
      total_weight+=(weight); \
    } \
  r++;
#define EnhanceImageTag  "Enhance/Image"

  Image
    *enhance_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  CacheView
    *enhance_view,
    *image_view;

  /*
    Initialize enhanced image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((image->columns < 5) || (image->rows < 5))
    return((Image *) NULL);
  enhance_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (enhance_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(enhance_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&enhance_image->exception);
      enhance_image=DestroyImage(enhance_image);
      return((Image *) NULL);
    }
  /*
    Enhance image.
  */
  status=MagickTrue;
  progress=0;
  (void) ResetMagickMemory(&zero,0,sizeof(zero));
  image_view=AcquireCacheView(image);
  enhance_view=AcquireCacheView(enhance_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *__restrict p;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Read another scan line.
    */
    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-2,y-2,image->columns+4,5,exception);
    q=QueueCacheViewAuthenticPixels(enhance_view,0,y,enhance_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      MagickPixelPacket
        aggregate;

      MagickRealType
        distance,
        distance_squared,
        mean,
        total_weight;

      PixelPacket
        pixel;

      register const PixelPacket
        *__restrict r;

      /*
        Compute weighted average of target pixel color components.
      */
      aggregate=zero;
      total_weight=0.0;
      r=p+2*(image->columns+4)+2;
      pixel=(*r);
      r=p;
      Enhance(5.0); Enhance(8.0); Enhance(10.0); Enhance(8.0); Enhance(5.0);
      r=p+(image->columns+4);
      Enhance(8.0); Enhance(20.0); Enhance(40.0); Enhance(20.0); Enhance(8.0);
      r=p+2*(image->columns+4);
      Enhance(10.0); Enhance(40.0); Enhance(80.0); Enhance(40.0); Enhance(10.0);
      r=p+3*(image->columns+4);
      Enhance(8.0); Enhance(20.0); Enhance(40.0); Enhance(20.0); Enhance(8.0);
      r=p+4*(image->columns+4);
      Enhance(5.0); Enhance(8.0); Enhance(10.0); Enhance(8.0); Enhance(5.0);
      q->red=(Quantum) ((aggregate.red+(total_weight/2)-1)/total_weight);
      q->green=(Quantum) ((aggregate.green+(total_weight/2)-1)/total_weight);
      q->blue=(Quantum) ((aggregate.blue+(total_weight/2)-1)/total_weight);
      q->opacity=(Quantum) ((aggregate.opacity+(total_weight/2)-1)/
        total_weight);
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(enhance_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_EnhanceImage)
#endif
        proceed=SetImageProgress(image,EnhanceImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  enhance_view=DestroyCacheView(enhance_view);
  image_view=DestroyCacheView(image_view);
  return(enhance_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E q u a l i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EqualizeImage() applies a histogram equalization to the image.
%
%  The format of the EqualizeImage method is:
%
%      MagickBooleanType EqualizeImage(Image *image)
%      MagickBooleanType EqualizeImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
*/

MagickExport MagickBooleanType EqualizeImage(Image *image)
{
  return(EqualizeImageChannel(image,DefaultChannels));
}

MagickExport MagickBooleanType EqualizeImageChannel(Image *image,
  const ChannelType channel)
{
#define EqualizeImageTag  "Equalize/Image"

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    black,
    *equalize_map,
    *histogram,
    intensity,
    *map,
    white;

  register long
    i;

  CacheView
    *image_view;

  /*
    Allocate and initialize histogram arrays.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  equalize_map=(MagickPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*equalize_map));
  histogram=(MagickPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*histogram));
  map=(MagickPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*map));
  if ((equalize_map == (MagickPixelPacket *) NULL) ||
      (histogram == (MagickPixelPacket *) NULL) ||
      (map == (MagickPixelPacket *) NULL))
    {
      if (map != (MagickPixelPacket *) NULL)
        map=(MagickPixelPacket *) RelinquishMagickMemory(map);
      if (histogram != (MagickPixelPacket *) NULL)
        histogram=(MagickPixelPacket *) RelinquishMagickMemory(histogram);
      if (equalize_map != (MagickPixelPacket *) NULL)
        equalize_map=(MagickPixelPacket *) RelinquishMagickMemory(equalize_map);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Form histogram.
  */
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));
  exception=(&image->exception);
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        histogram[ScaleQuantumToMap(p->red)].red++;
      if ((channel & GreenChannel) != 0)
        histogram[ScaleQuantumToMap(p->green)].green++;
      if ((channel & BlueChannel) != 0)
        histogram[ScaleQuantumToMap(p->blue)].blue++;
      if ((channel & OpacityChannel) != 0)
        histogram[ScaleQuantumToMap(p->opacity)].opacity++;
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        histogram[ScaleQuantumToMap(indexes[x])].index++;
      p++;
    }
  }
  /*
    Integrate the histogram to get the equalization map.
  */
  (void) ResetMagickMemory(&intensity,0,sizeof(intensity));
  for (i=0; i <= (long) MaxMap; i++)
  {
    if ((channel & RedChannel) != 0)
      intensity.red+=histogram[i].red;
    if ((channel & GreenChannel) != 0)
      intensity.green+=histogram[i].green;
    if ((channel & BlueChannel) != 0)
      intensity.blue+=histogram[i].blue;
    if ((channel & OpacityChannel) != 0)
      intensity.opacity+=histogram[i].opacity;
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace))
      intensity.index+=histogram[i].index;
    map[i]=intensity;
  }
  black=map[0];
  white=map[(int) MaxMap];
  (void) ResetMagickMemory(equalize_map,0,(MaxMap+1)*sizeof(*equalize_map));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (i=0; i <= (long) MaxMap; i++)
  {
    if (((channel & RedChannel) != 0) && (white.red != black.red))
      equalize_map[i].red=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].red-black.red))/(white.red-black.red)));
    if (((channel & GreenChannel) != 0) && (white.green != black.green))
      equalize_map[i].green=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].green-black.green))/(white.green-black.green)));
    if (((channel & BlueChannel) != 0) && (white.blue != black.blue))
      equalize_map[i].blue=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].blue-black.blue))/(white.blue-black.blue)));
    if (((channel & OpacityChannel) != 0) && (white.opacity != black.opacity))
      equalize_map[i].opacity=(MagickRealType) ScaleMapToQuantum(
        (MagickRealType) ((MaxMap*(map[i].opacity-black.opacity))/
        (white.opacity-black.opacity)));
    if ((((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace)) &&
        (white.index != black.index))
      equalize_map[i].index=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].index-black.index))/(white.index-black.index)));
  }
  histogram=(MagickPixelPacket *) RelinquishMagickMemory(histogram);
  map=(MagickPixelPacket *) RelinquishMagickMemory(map);
  if (image->storage_class == PseudoClass)
    {
      /*
        Equalize colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        if (((channel & RedChannel) != 0) && (white.red != black.red))
          image->colormap[i].red=RoundToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].red)].red);
        if (((channel & GreenChannel) != 0) && (white.green != black.green))
          image->colormap[i].green=RoundToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].green)].green);
        if (((channel & BlueChannel) != 0) && (white.blue != black.blue))
          image->colormap[i].blue=RoundToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].blue)].blue);
        if (((channel & OpacityChannel) != 0) &&
            (white.opacity != black.opacity))
          image->colormap[i].opacity=RoundToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].opacity)].opacity);
      }
    }
  /*
    Equalize image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if (((channel & RedChannel) != 0) && (white.red != black.red))
        q->red=RoundToQuantum(equalize_map[ScaleQuantumToMap(q->red)].red);
      if (((channel & GreenChannel) != 0) && (white.green != black.green))
        q->green=RoundToQuantum(equalize_map[ScaleQuantumToMap(
          q->green)].green);
      if (((channel & BlueChannel) != 0) && (white.blue != black.blue))
        q->blue=RoundToQuantum(equalize_map[ScaleQuantumToMap(q->blue)].blue);
      if (((channel & OpacityChannel) != 0) && (white.opacity != black.opacity))
        q->opacity=RoundToQuantum(equalize_map[ScaleQuantumToMap(
          q->opacity)].opacity);
      if ((((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace)) &&
          (white.index != black.index))
        indexes[x]=RoundToQuantum(equalize_map[ScaleQuantumToMap(
          indexes[x])].index);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_EqualizeImageChannel)
#endif
        proceed=SetImageProgress(image,EqualizeImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  equalize_map=(MagickPixelPacket *) RelinquishMagickMemory(equalize_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     G a m m a I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GammaImage() gamma-corrects a particular image channel.  The same
%  image viewed on different devices will have perceptual differences in the
%  way the image's intensities are represented on the screen.  Specify
%  individual gamma levels for the red, green, and blue channels, or adjust
%  all three with the gamma parameter.  Values typically range from 0.8 to 2.3.
%
%  You can also reduce the influence of a particular channel with a gamma
%  value of 0.
%
%  The format of the GammaImage method is:
%
%      MagickBooleanType GammaImage(Image *image,const double gamma)
%      MagickBooleanType GammaImageChannel(Image *image,
%        const ChannelType channel,const double gamma)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o gamma: the image gamma.
%
*/
MagickExport MagickBooleanType GammaImage(Image *image,const char *level)
{
  GeometryInfo
    geometry_info;

  MagickPixelPacket
    gamma;

  MagickStatusType
    flags,
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (level == (char *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(level,&geometry_info);
  gamma.red=geometry_info.rho;
  gamma.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    gamma.green=gamma.red;
  gamma.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    gamma.blue=gamma.red;
  if ((gamma.red == 1.0) && (gamma.green == 1.0) && (gamma.blue == 1.0))
    return(MagickTrue);
  if ((gamma.red == gamma.green) && (gamma.green == gamma.blue))
    status=GammaImageChannel(image,(const ChannelType) (RedChannel |
      GreenChannel | BlueChannel),(double) gamma.red);
  else
    {
      status=GammaImageChannel(image,RedChannel,(double) gamma.red);
      status|=GammaImageChannel(image,GreenChannel,(double) gamma.green);
      status|=GammaImageChannel(image,BlueChannel,(double) gamma.blue);
    }
  return(status != 0 ? MagickTrue : MagickFalse);
}

MagickExport MagickBooleanType GammaImageChannel(Image *image,
  const ChannelType channel,const double gamma)
{
#define GammaCorrectImageTag  "GammaCorrect/Image"

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  Quantum
    *gamma_map;

  register long
    i;

  CacheView
    *image_view;

  /*
    Allocate and initialize gamma maps.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (gamma == 1.0)
    return(MagickTrue);
  gamma_map=(Quantum *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*gamma_map));
  if (gamma_map == (Quantum *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  (void) ResetMagickMemory(gamma_map,0,(MaxMap+1)*sizeof(*gamma_map));
  if (gamma != 0.0)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1)
#endif
    for (i=0; i <= (long) MaxMap; i++)
      gamma_map[i]=RoundToQuantum((MagickRealType) ScaleMapToQuantum((
        MagickRealType) (MaxMap*pow((double) i/MaxMap,1.0/gamma))));
  if (image->storage_class == PseudoClass)
    {
      /*
        Gamma-correct colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        if ((channel & RedChannel) != 0)
          image->colormap[i].red=gamma_map[
            ScaleQuantumToMap(image->colormap[i].red)];
        if ((channel & GreenChannel) != 0)
          image->colormap[i].green=gamma_map[
            ScaleQuantumToMap(image->colormap[i].green)];
        if ((channel & BlueChannel) != 0)
          image->colormap[i].blue=gamma_map[
            ScaleQuantumToMap(image->colormap[i].blue)];
        if ((channel & OpacityChannel) != 0)
          {
            if (image->matte == MagickFalse)
              image->colormap[i].opacity=gamma_map[
                ScaleQuantumToMap(image->colormap[i].opacity)];
            else
              image->colormap[i].opacity=(Quantum) QuantumRange-
                gamma_map[ScaleQuantumToMap((Quantum) (QuantumRange-
                image->colormap[i].opacity))];
          }
      }
    }
  /*
    Gamma-correct image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=gamma_map[ScaleQuantumToMap(q->red)];
      if ((channel & GreenChannel) != 0)
        q->green=gamma_map[ScaleQuantumToMap(q->green)];
      if ((channel & BlueChannel) != 0)
        q->blue=gamma_map[ScaleQuantumToMap(q->blue)];
      if ((channel & OpacityChannel) != 0)
        {
          if (image->matte == MagickFalse)
            q->opacity=gamma_map[ScaleQuantumToMap(q->opacity)];
          else
            q->opacity=(Quantum) QuantumRange-gamma_map[
              ScaleQuantumToMap((Quantum) (QuantumRange-q->opacity))];
        }
      q++;
    }
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace))
      for (x=0; x < (long) image->columns; x++)
        indexes[x]=gamma_map[ScaleQuantumToMap(indexes[x])];
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_GammaImageChannel)
#endif
        proceed=SetImageProgress(image,GammaCorrectImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  gamma_map=(Quantum *) RelinquishMagickMemory(gamma_map);
  if (image->gamma != 0.0)
    image->gamma*=gamma;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     H a l d C l u t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HaldClutImage() applies a Hald color lookup table to the image.  A Hald
%  color lookup table is a 3-dimensional color cube mapped to 2 dimensions.
%  Create it with the HALD coder.  You can apply any color transformation to
%  the Hald image and then use this method to apply the transform to the
%  image.
%
%  The format of the HaldClutImage method is:
%
%      MagickBooleanType HaldClutImage(Image *image,Image *hald_image)
%      MagickBooleanType HaldClutImageChannel(Image *image,
%        const ChannelType channel,Image *hald_image)
%
%  A description of each parameter follows:
%
%    o image: the image, which is replaced by indexed CLUT values
%
%    o hald_image: the color lookup table image for replacement color values.
%
%    o channel: the channel.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport MagickBooleanType HaldClutImage(Image *image,
  const Image *hald_image)
{
  return(HaldClutImageChannel(image,DefaultChannels,hald_image));
}

MagickExport MagickBooleanType HaldClutImageChannel(Image *image,
  const ChannelType channel,const Image *hald_image)
{
#define HaldClutImageTag  "Clut/Image"

  typedef struct _HaldInfo
  {
    MagickRealType
      x,
      y,
      z;
  } HaldInfo;

  double
    width;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  ResampleFilter
    **resample_filter;

  size_t
    cube_size,
    length,
    level;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(hald_image != (Image *) NULL);
  assert(hald_image->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  /*
    Hald clut image.
  */
  status=MagickTrue;
  progress=0;
  length=MagickMin(hald_image->columns,hald_image->rows);
  for (level=2; (level*level*level) < length; level++) ;
  level*=level;
  cube_size=level*level;
  width=(double) hald_image->columns;
  GetMagickPixelPacket(hald_image,&zero);
  exception=(&image->exception);
  resample_filter=AcquireResampleFilterThreadSet(hald_image,MagickTrue,
    exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    double
      offset;

    HaldInfo
      point;

    MagickPixelPacket
      pixel,
      pixel1,
      pixel2,
      pixel3,
      pixel4;

    register IndexPacket
      *__restrict indexes;

    register long
      id,
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    pixel=zero;
    pixel1=zero;
    pixel2=zero;
    pixel3=zero;
    pixel4=zero;
    id=GetOpenMPThreadId();
    for (x=0; x < (long) image->columns; x++)
    {
      point.x=QuantumScale*(level-1.0)*q->red;
      point.y=QuantumScale*(level-1.0)*q->green;
      point.z=QuantumScale*(level-1.0)*q->blue;
      offset=point.x+level*floor(point.y)+cube_size*floor(point.z);
      point.x-=floor(point.x);
      point.y-=floor(point.y);
      point.z-=floor(point.z);
      (void) ResamplePixelColor(resample_filter[id],fmod(offset,width),
        floor(offset/width),&pixel1);
      (void) ResamplePixelColor(resample_filter[id],fmod(offset+level,width),
        floor((offset+level)/width),&pixel2);
      MagickPixelCompositeAreaBlend(&pixel1,pixel1.opacity,&pixel2,
        pixel2.opacity,point.y,&pixel3);
      offset+=cube_size;
      (void) ResamplePixelColor(resample_filter[id],fmod(offset,width),
        floor(offset/width),&pixel1);
      (void) ResamplePixelColor(resample_filter[id],fmod(offset+level,width),
        floor((offset+level)/width),&pixel2);
      MagickPixelCompositeAreaBlend(&pixel1,pixel1.opacity,&pixel2,
        pixel2.opacity,point.y,&pixel4);
      MagickPixelCompositeAreaBlend(&pixel3,pixel3.opacity,&pixel4,
        pixel4.opacity,point.z,&pixel);
      if ((channel & RedChannel) != 0)
        q->red=RoundToQuantum(pixel.red);
      if ((channel & GreenChannel) != 0)
        q->green=RoundToQuantum(pixel.green);
      if ((channel & BlueChannel) != 0)
        q->blue=RoundToQuantum(pixel.blue);
      if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse))
        q->opacity=RoundToQuantum(pixel.opacity);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=RoundToQuantum(pixel.index);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_HaldClutImageChannel)
#endif
        proceed=SetImageProgress(image,HaldClutImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  resample_filter=DestroyResampleFilterThreadSet(resample_filter);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelImage() adjusts the levels of a particular image channel by
%  scaling the colors falling between specified white and black points to
%  the full available quantum range.
%
%  The parameters provided represent the black, and white points.  The black
%  point specifies the darkest color in the image. Colors darker than the
%  black point are set to zero.  White point specifies the lightest color in
%  the image.  Colors brighter than the white point are set to the maximum
%  quantum value.
%
%  If a '!' flag is given, map black and white colors to the given levels
%  rather than mapping those levels to black and white.  See
%  LevelizeImageChannel() and LevelizeImageChannel(), below.
%
%  Gamma specifies a gamma correction to apply to the image.
%
%  The format of the LevelImage method is:
%
%      MagickBooleanType LevelImage(Image *image,const char *levels)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o levels: Specify the levels where the black and white points have the
%      range of 0-QuantumRange, and gamma has the range 0-10 (e.g. 10x90%+2).
%      A '!' flag inverts the re-mapping.
%
*/

MagickExport MagickBooleanType LevelImage(Image *image,const char *levels)
{
  double
    black_point,
    gamma,
    white_point;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  /*
    Parse levels.
  */
  if (levels == (char *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(levels,&geometry_info);
  black_point=geometry_info.rho;
  white_point=(double) QuantumRange;
  if ((flags & SigmaValue) != 0)
    white_point=geometry_info.sigma;
  gamma=1.0;
  if ((flags & XiValue) != 0)
    gamma=geometry_info.xi;
  if ((flags & PercentValue) != 0)
    {
      black_point*=(double) image->columns*image->rows/100.0;
      white_point*=(double) image->columns*image->rows/100.0;
    }
  if ((flags & SigmaValue) == 0)
    white_point=(double) QuantumRange-black_point;
  if ((flags & AspectValue ) == 0)
    status=LevelImageChannel(image,DefaultChannels,black_point,white_point,
      gamma);
  else
    status=LevelizeImageChannel(image,DefaultChannels,black_point,white_point,
      gamma);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l I m a g e C h a n n e l                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelImageChannel() applies the normal LevelImage() operation to just the
%  Specific channels specified, spreading out the values between the black and
%  white points over the entire range of values.  Gamma correction is also
%  applied after the values has been mapped.
%
%  It is typically used to improve image contrast, or to provide a controlled
%  linear threshold for the image. If the black and white points are set to
%  the minimum and maximum values found in the image, the image can be
%  normalized.  or by swapping black and white values, negate the image.
%
%  The format of the LevelizeImageChannel method is:
%
%      MagickBooleanType LevelImageChannel(Image *image,
%        const ChannelType channel,black_point,white_point,gamma)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o black_point: The level which is to be mapped to zero (black)
%
%    o white_point: The level which is to be mapped to QuantiumRange (white)
%
%    o gamma: adjust gamma by this factor before mapping values.
%             use 1.0 for purely linear stretching of image color values
%
*/
MagickExport MagickBooleanType LevelImageChannel(Image *image,
  const ChannelType channel,const double black_point,const double white_point,
  const double gamma)
{
#define LevelImageTag  "Level/Image"
#define LevelValue(x) (RoundToQuantum((MagickRealType) QuantumRange* \
  pow(((double) (x)-black_point)/(white_point-black_point),1.0/gamma)))

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  register long
    i;

  CacheView
    *image_view;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
    for (i=0; i < (long) image->colors; i++)
    {
      /*
        Level colormap.
      */
      if ((channel & RedChannel) != 0)
        image->colormap[i].red=LevelValue(image->colormap[i].red);
      if ((channel & GreenChannel) != 0)
        image->colormap[i].green=LevelValue(image->colormap[i].green);
      if ((channel & BlueChannel) != 0)
        image->colormap[i].blue=LevelValue(image->colormap[i].blue);
      if ((channel & OpacityChannel) != 0)
        image->colormap[i].opacity=LevelValue(image->colormap[i].opacity);
      }
  /*
    Level image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=LevelValue(q->red);
      if ((channel & GreenChannel) != 0)
        q->green=LevelValue(q->green);
      if ((channel & BlueChannel) != 0)
        q->blue=LevelValue(q->blue);
      if (((channel & OpacityChannel) != 0) &&
          (image->matte == MagickTrue))
        q->opacity=LevelValue(q->opacity);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=LevelValue(indexes[x]);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_LevelImageChannel)
#endif
        proceed=SetImageProgress(image,LevelImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l i z e I m a g e C h a n n e l                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelizeImageChannel() applies the reversed LevelImage() operation to just
%  the specific channels specified.  It compresses the full range of color
%  values, so that they lie between the given black and white points. Gamma is
%  applied before the values are mapped.
%
%  LevelizeImageChannel() can be called with by using a +level command line
%  API option, or using a '!' on a -level or LevelImage() geometry string.
%
%  It can be used for example de-contrast a greyscale image to the exact
%  levels specified.  Or by using specific levels for each channel of an image
%  you can convert a gray-scale image to any linear color gradient, according
%  to those levels.
%
%  The format of the LevelizeImageChannel method is:
%
%      MagickBooleanType LevelizeImageChannel(Image *image,
%        const ChannelType channel,const char *levels)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o black_point: The level to map zero (black) to.
%
%    o white_point: The level to map QuantiumRange (white) to.
%
%    o gamma: adjust gamma by this factor before mapping values.
%
*/
MagickExport MagickBooleanType LevelizeImageChannel(Image *image,
  const ChannelType channel,const double black_point,const double white_point,
  const double gamma)
{
#define LevelizeImageTag  "Levelize/Image"
#define LevelizeValue(x) (RoundToQuantum(((MagickRealType) \
  pow((double)(QuantumScale*(x)),1.0/gamma))*(white_point-black_point)+ \
  black_point))

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  register long
    i;

  CacheView
    *image_view;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
    for (i=0; i < (long) image->colors; i++)
    {
      /*
        Level colormap.
      */
      if ((channel & RedChannel) != 0)
        image->colormap[i].red=LevelizeValue(image->colormap[i].red);
      if ((channel & GreenChannel) != 0)
        image->colormap[i].green=LevelizeValue(image->colormap[i].green);
      if ((channel & BlueChannel) != 0)
        image->colormap[i].blue=LevelizeValue(image->colormap[i].blue);
      if ((channel & OpacityChannel) != 0)
        image->colormap[i].opacity=LevelizeValue(image->colormap[i].opacity);
    }
  /*
    Level image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=LevelizeValue(q->red);
      if ((channel & GreenChannel) != 0)
        q->green=LevelizeValue(q->green);
      if ((channel & BlueChannel) != 0)
        q->blue=LevelizeValue(q->blue);
      if (((channel & OpacityChannel) != 0) &&
          (image->matte == MagickTrue))
        q->opacity=LevelizeValue(q->opacity);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=LevelizeValue(indexes[x]);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_LevelizeImageChannel)
#endif
        proceed=SetImageProgress(image,LevelizeImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l I m a g e C o l o r s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelImageColor() will map the given color to "black" and "white"
%  values, limearly spreading out the colors, and level values on a channel by
%  channel bases, as per LevelImage().  The given colors allows you to specify
%  different level ranges for each of the color channels seperatally.
%
%  If the boolean 'invert' is set true the image values will modifyed in the
%  reverse direction. That is any existing "black" and "white" colors in the
%  image will become the color values given, with all other values compressed
%  appropriatally.  This effectivally maps a greyscale gradient into the given
%  color gradient.
%
%  The format of the LevelImageColors method is:
%
%  MagickBooleanType LevelImageColors(Image *image,const ChannelType channel,
%    const MagickPixelPacket *black_color,const MagickPixelPacket *white_color,
%    const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o black_color: The color to map black to/from
%
%    o white_point: The color to map white to/from
%
%    o invert: if true map the colors (levelize), rather than from (level)
%
*/
MagickBooleanType LevelImageColors(Image *image,const ChannelType channel,
  const MagickPixelPacket *black_color,const MagickPixelPacket *white_color,
  const MagickBooleanType invert)
{
#define LevelColorImageTag  "LevelColor/Image"

  MagickStatusType
    status;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickFalse;
  if (invert == MagickFalse)
    {
      if ((channel & RedChannel) != 0)
        status|=LevelImageChannel(image,RedChannel,
          black_color->red,white_color->red,(double) 1.0);
      if ((channel & GreenChannel) != 0)
        status|=LevelImageChannel(image,GreenChannel,
          black_color->green,white_color->green,(double) 1.0);
      if ((channel & BlueChannel) != 0)
        status|=LevelImageChannel(image,BlueChannel,
          black_color->blue,white_color->blue,(double) 1.0);
      if (((channel & OpacityChannel) != 0) &&
          (image->matte == MagickTrue))
        status|=LevelImageChannel(image,OpacityChannel,
          black_color->opacity,white_color->opacity,(double) 1.0);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        status|=LevelImageChannel(image,IndexChannel,
          black_color->index,white_color->index,(double) 1.0);
    }
  else
    {
      if ((channel & RedChannel) != 0)
        status|=LevelizeImageChannel(image,RedChannel,
          black_color->red,white_color->red,(double) 1.0);
      if ((channel & GreenChannel) != 0)
        status|=LevelizeImageChannel(image,GreenChannel,
          black_color->green,white_color->green,(double) 1.0);
      if ((channel & BlueChannel) != 0)
        status|=LevelizeImageChannel(image,BlueChannel,
          black_color->blue,white_color->blue,(double) 1.0);
      if (((channel & OpacityChannel) != 0) &&
          (image->matte == MagickTrue))
        status|=LevelizeImageChannel(image,OpacityChannel,
          black_color->opacity,white_color->opacity,(double) 1.0);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        status|=LevelizeImageChannel(image,IndexChannel,
          black_color->index,white_color->index,(double) 1.0);
    }
  return(status == 0 ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L i n e a r S t r e t c h I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The LinearStretchImage() discards any pixels below the black point and
%  above the white point and levels the remaining pixels.
%
%  The format of the LinearStretchImage method is:
%
%      MagickBooleanType LinearStretchImage(Image *image,
%        const double black_point,const double white_point)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_point: the black point.
%
%    o white_point: the white point.
%
*/
MagickExport MagickBooleanType LinearStretchImage(Image *image,
  const double black_point,const double white_point)
{
#define LinearStretchImageTag  "LinearStretch/Image"

  ExceptionInfo
    *exception;

  long
    black,
    white,
    y;

  MagickBooleanType
    status;

  MagickRealType
    *histogram,
    intensity;

  MagickSizeType
    number_pixels;

  /*
    Allocate histogram and linear map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  histogram=(MagickRealType *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*histogram));
  if (histogram == (MagickRealType *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Form histogram.
  */
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));
  exception=(&image->exception);
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *__restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=(long) image->columns-1; x >= 0; x--)
    {
      histogram[ScaleQuantumToMap(PixelIntensityToQuantum(p))]++;
      p++;
    }
  }
  /*
    Find the histogram boundaries by locating the black and white point levels.
  */
  number_pixels=(MagickSizeType) image->columns*image->rows;
  intensity=0.0;
  for (black=0; black < (long) MaxMap; black++)
  {
    intensity+=histogram[black];
    if (intensity >= black_point)
      break;
  }
  intensity=0.0;
  for (white=(long) MaxMap; white != 0; white--)
  {
    intensity+=histogram[white];
    if (intensity >= white_point)
      break;
  }
  histogram=(MagickRealType *) RelinquishMagickMemory(histogram);
  status=LevelImageChannel(image,DefaultChannels,(double) black,(double) white,
    1.0);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o d u l a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModulateImage() lets you control the brightness, saturation, and hue
%  of an image.  Modulate represents the brightness, saturation, and hue
%  as one parameter (e.g. 90,150,100).  If the image colorspace is HSL, the
%  modulation is lightness, saturation, and hue.  And if the colorspace is
%  HWB, use blackness, whiteness, and hue.
%
%  The format of the ModulateImage method is:
%
%      MagickBooleanType ModulateImage(Image *image,const char *modulate)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o modulate: Define the percent change in brightness, saturation, and
%      hue.
%
*/

static void ModulateHSB(const double percent_hue,
  const double percent_saturation,const double percent_brightness,
  Quantum *red,Quantum *green,Quantum *blue)
{
  double
    brightness,
    hue,
    saturation;

  /*
    Increase or decrease color brightness, saturation, or hue.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  ConvertRGBToHSB(*red,*green,*blue,&hue,&saturation,&brightness);
  hue+=0.5*(0.01*percent_hue-1.0);
  while (hue < 0.0)
    hue+=1.0;
  while (hue > 1.0)
    hue-=1.0;
  saturation*=0.01*percent_saturation;
  brightness*=0.01*percent_brightness;
  ConvertHSBToRGB(hue,saturation,brightness,red,green,blue);
}

static void ModulateHSL(const double percent_hue,
  const double percent_saturation,const double percent_lightness,
  Quantum *red,Quantum *green,Quantum *blue)
{
  double
    hue,
    lightness,
    saturation;

  /*
    Increase or decrease color lightness, saturation, or hue.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  ConvertRGBToHSL(*red,*green,*blue,&hue,&saturation,&lightness);
  hue+=0.5*(0.01*percent_hue-1.0);
  while (hue < 0.0)
    hue+=1.0;
  while (hue > 1.0)
    hue-=1.0;
  saturation*=0.01*percent_saturation;
  lightness*=0.01*percent_lightness;
  ConvertHSLToRGB(hue,saturation,lightness,red,green,blue);
}

static void ModulateHWB(const double percent_hue,const double percent_whiteness,  const double percent_blackness,Quantum *red,Quantum *green,Quantum *blue)
{
  double
    blackness,
    hue,
    whiteness;

  /*
    Increase or decrease color blackness, whiteness, or hue.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  ConvertRGBToHWB(*red,*green,*blue,&hue,&whiteness,&blackness);
  hue+=0.5*(0.01*percent_hue-1.0);
  while (hue < 0.0)
    hue+=1.0;
  while (hue > 1.0)
    hue-=1.0;
  blackness*=0.01*percent_blackness;
  whiteness*=0.01*percent_whiteness;
  ConvertHWBToRGB(hue,whiteness,blackness,red,green,blue);
}

MagickExport MagickBooleanType ModulateImage(Image *image,const char *modulate)
{
#define ModulateImageTag  "Modulate/Image"

  ColorspaceType
    colorspace;

  const char
    *artifact;

  double
    percent_brightness,
    percent_hue,
    percent_saturation;

  ExceptionInfo
    *exception;

  GeometryInfo
    geometry_info;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  register long
    i;

  CacheView
    *image_view;

  /*
    Initialize gamma table.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (modulate == (char *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(modulate,&geometry_info);
  percent_brightness=geometry_info.rho;
  percent_saturation=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    percent_saturation=100.0;
  percent_hue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    percent_hue=100.0;
  colorspace=UndefinedColorspace;
  artifact=GetImageArtifact(image,"modulate:colorspace");
  if (artifact != (const char *) NULL)
    colorspace=(ColorspaceType) ParseMagickOption(MagickColorspaceOptions,
      MagickFalse,artifact);
  if (image->storage_class == PseudoClass)
    {
      /*
        Modulate colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (i=0; i < (long) image->colors; i++)
        switch (colorspace)
        {
          case HSBColorspace:
          {
            ModulateHSB(percent_hue,percent_saturation,percent_brightness,
              &image->colormap[i].red,&image->colormap[i].green,
              &image->colormap[i].blue);
            break;
          }
          case HSLColorspace:
          default:
          {
            ModulateHSL(percent_hue,percent_saturation,percent_brightness,
              &image->colormap[i].red,&image->colormap[i].green,
              &image->colormap[i].blue);
            break;
          }
          case HWBColorspace:
          {
            ModulateHWB(percent_hue,percent_saturation,percent_brightness,
              &image->colormap[i].red,&image->colormap[i].green,
              &image->colormap[i].blue);
            break;
          }
        }
    }
  /*
    Modulate image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (long) image->columns; x++)
    {
      switch (colorspace)
      {
        case HSBColorspace:
        {
          ModulateHSB(percent_hue,percent_saturation,percent_brightness,
            &q->red,&q->green,&q->blue);
          break;
        }
        case HSLColorspace:
        default:
        {
          ModulateHSL(percent_hue,percent_saturation,percent_brightness,
            &q->red,&q->green,&q->blue);
          break;
        }
        case HWBColorspace:
        {
          ModulateHWB(percent_hue,percent_saturation,percent_brightness,
            &q->red,&q->green,&q->blue);
          break;
        }
      }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ModulateImage)
#endif
        proceed=SetImageProgress(image,ModulateImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     N e g a t e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NegateImage() negates the colors in the reference image.  The grayscale
%  option means that only grayscale values within the image are negated.
%
%  The format of the NegateImageChannel method is:
%
%      MagickBooleanType NegateImage(Image *image,
%        const MagickBooleanType grayscale)
%      MagickBooleanType NegateImageChannel(Image *image,
%        const ChannelType channel,const MagickBooleanType grayscale)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o grayscale: If MagickTrue, only negate grayscale pixels within the image.
%
*/

MagickExport MagickBooleanType NegateImage(Image *image,
  const MagickBooleanType grayscale)
{
  MagickBooleanType
    status;

  status=NegateImageChannel(image,DefaultChannels,grayscale);
  return(status);
}

MagickExport MagickBooleanType NegateImageChannel(Image *image,
  const ChannelType channel,const MagickBooleanType grayscale)
{
#define NegateImageTag  "Negate/Image"

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  register long
    i;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      /*
        Negate colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        if (grayscale != MagickFalse)
          if ((image->colormap[i].red != image->colormap[i].green) ||
              (image->colormap[i].green != image->colormap[i].blue))
            continue;
        if ((channel & RedChannel) != 0)
          image->colormap[i].red=(Quantum) QuantumRange-
            image->colormap[i].red;
        if ((channel & GreenChannel) != 0)
          image->colormap[i].green=(Quantum) QuantumRange-
            image->colormap[i].green;
        if ((channel & BlueChannel) != 0)
          image->colormap[i].blue=(Quantum) QuantumRange-
            image->colormap[i].blue;
      }
    }
  /*
    Negate image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
  if (grayscale != MagickFalse)
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register IndexPacket
          *__restrict indexes;

        register long
          x;

        register PixelPacket
          *__restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        indexes=GetCacheViewAuthenticIndexQueue(image_view);
        for (x=0; x < (long) image->columns; x++)
        {
          if ((q->red != q->green) || (q->green != q->blue))
            {
              q++;
              continue;
            }
          if ((channel & RedChannel) != 0)
            q->red=(Quantum) QuantumRange-q->red;
          if ((channel & GreenChannel) != 0)
            q->green=(Quantum) QuantumRange-q->green;
          if ((channel & BlueChannel) != 0)
            q->blue=(Quantum) QuantumRange-q->blue;
          if ((channel & OpacityChannel) != 0)
            q->opacity=(Quantum) QuantumRange-q->opacity;
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            indexes[x]=(IndexPacket) QuantumRange-indexes[x];
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_NegateImageChannel)
#endif
            proceed=SetImageProgress(image,NegateImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      image_view=DestroyCacheView(image_view);
      return(MagickTrue);
    }
  /*
    Negate image.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=(Quantum) QuantumRange-q->red;
      if ((channel & GreenChannel) != 0)
        q->green=(Quantum) QuantumRange-q->green;
      if ((channel & BlueChannel) != 0)
        q->blue=(Quantum) QuantumRange-q->blue;
      if ((channel & OpacityChannel) != 0)
        q->opacity=(Quantum) QuantumRange-q->opacity;
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=(IndexPacket) QuantumRange-indexes[x];
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_NegateImageChannel)
#endif
        proceed=SetImageProgress(image,NegateImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     N o r m a l i z e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The NormalizeImage() method enhances the contrast of a color image by
%  mapping the darkest 2 percent of all pixel to black and the brightest
%  1 percent to white.
%
%  The format of the NormalizeImage method is:
%
%      MagickBooleanType NormalizeImage(Image *image)
%      MagickBooleanType NormalizeImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
*/

MagickExport MagickBooleanType NormalizeImage(Image *image)
{
  MagickBooleanType
    status;

  status=NormalizeImageChannel(image,DefaultChannels);
  return(status);
}

MagickExport MagickBooleanType NormalizeImageChannel(Image *image,
  const ChannelType channel)
{
  double
    black_point,
    white_point;

  black_point=(double) image->columns*image->rows*0.02;
  white_point=(double) image->columns*image->rows*0.99;
  return(ContrastStretchImageChannel(image,channel,black_point,white_point));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S i g m o i d a l C o n t r a s t I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SigmoidalContrastImage() adjusts the contrast of an image with a non-linear
%  sigmoidal contrast algorithm.  Increase the contrast of the image using a
%  sigmoidal transfer function without saturating highlights or shadows.
%  Contrast indicates how much to increase the contrast (0 is none; 3 is
%  typical; 20 is pushing it); mid-point indicates where midtones fall in the
%  resultant image (0 is white; 50% is middle-gray; 100% is black).  Set
%  sharpen to MagickTrue to increase the image contrast otherwise the contrast
%  is reduced.
%
%  The format of the SigmoidalContrastImage method is:
%
%      MagickBooleanType SigmoidalContrastImage(Image *image,
%        const MagickBooleanType sharpen,const char *levels)
%      MagickBooleanType SigmoidalContrastImageChannel(Image *image,
%        const ChannelType channel,const MagickBooleanType sharpen,
%        const double contrast,const double midpoint)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o sharpen: Increase or decrease image contrast.
%
%    o contrast: control the "shoulder" of the contast curve.
%
%    o midpoint: control the "toe" of the contast curve.
%
*/

MagickExport MagickBooleanType SigmoidalContrastImage(Image *image,
  const MagickBooleanType sharpen,const char *levels)
{
  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickStatusType
    flags;

  flags=ParseGeometry(levels,&geometry_info);
  if ((flags & SigmaValue) == 0)
    geometry_info.sigma=1.0*QuantumRange/2.0;
  if ((flags & PercentValue) != 0)
    geometry_info.sigma=1.0*QuantumRange*geometry_info.sigma/100.0;
  status=SigmoidalContrastImageChannel(image,DefaultChannels,sharpen,
    geometry_info.rho,geometry_info.sigma);
  return(status);
}

MagickExport MagickBooleanType SigmoidalContrastImageChannel(Image *image,
  const ChannelType channel,const MagickBooleanType sharpen,
  const double contrast,const double midpoint)
{
#define SigmoidalContrastImageTag  "SigmoidalContrast/Image"

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickRealType
    *sigmoidal_map;

  register long
    i;

  CacheView
    *image_view;

  /*
    Allocate and initialize sigmoidal maps.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  sigmoidal_map=(MagickRealType *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*sigmoidal_map));
  if (sigmoidal_map == (MagickRealType *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  (void) ResetMagickMemory(sigmoidal_map,0,(MaxMap+1)*sizeof(*sigmoidal_map));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (i=0; i <= (long) MaxMap; i++)
  {
    if (sharpen != MagickFalse)
      {
        sigmoidal_map[i]=(MagickRealType) ScaleMapToQuantum((MagickRealType)
          (MaxMap*((1.0/(1.0+exp(contrast*(midpoint/(double) QuantumRange-
          (double) i/MaxMap))))-(1.0/(1.0+exp(contrast*(midpoint/
          (double) QuantumRange)))))/((1.0/(1.0+exp(contrast*(midpoint/
          (double) QuantumRange-1.0))))-(1.0/(1.0+exp(contrast*(midpoint/
          (double) QuantumRange)))))+0.5));
        continue;
      }
    sigmoidal_map[i]=(MagickRealType) ScaleMapToQuantum((MagickRealType)
      (MaxMap*(QuantumScale*midpoint-log((1.0-(1.0/(1.0+exp(midpoint/
      (double) QuantumRange*contrast))+((double) i/MaxMap)*((1.0/
      (1.0+exp(contrast*(midpoint/(double) QuantumRange-1.0))))-(1.0/
      (1.0+exp(midpoint/(double) QuantumRange*contrast))))))/
      (1.0/(1.0+exp(midpoint/(double) QuantumRange*contrast))+
      ((double) i/MaxMap)*((1.0/(1.0+exp(contrast*(midpoint/
      (double) QuantumRange-1.0))))-(1.0/(1.0+exp(midpoint/
      (double) QuantumRange*contrast))))))/contrast)));
  }
  if (image->storage_class == PseudoClass)
    {
      /*
        Sigmoidal-contrast enhance colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        if ((channel & RedChannel) != 0)
          image->colormap[i].red=RoundToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].red)]);
        if ((channel & GreenChannel) != 0)
          image->colormap[i].green=RoundToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].green)]);
        if ((channel & BlueChannel) != 0)
          image->colormap[i].blue=RoundToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].blue)]);
        if ((channel & OpacityChannel) != 0)
          image->colormap[i].opacity=RoundToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].opacity)]);
      }
    }
  /*
    Sigmoidal-contrast enhance image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=RoundToQuantum(sigmoidal_map[ScaleQuantumToMap(q->red)]);
      if ((channel & GreenChannel) != 0)
        q->green=RoundToQuantum(sigmoidal_map[ScaleQuantumToMap(q->green)]);
      if ((channel & BlueChannel) != 0)
        q->blue=RoundToQuantum(sigmoidal_map[ScaleQuantumToMap(q->blue)]);
      if ((channel & OpacityChannel) != 0)
        q->opacity=RoundToQuantum(sigmoidal_map[ScaleQuantumToMap(q->opacity)]);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=(IndexPacket) RoundToQuantum(sigmoidal_map[
          ScaleQuantumToMap(indexes[x])]);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SigmoidalContrastImageChannel)
#endif
        proceed=SetImageProgress(image,SigmoidalContrastImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  sigmoidal_map=(MagickRealType *) RelinquishMagickMemory(sigmoidal_map);
  return(status);
}

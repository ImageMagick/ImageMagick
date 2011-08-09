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
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resample-private.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"
#include "MagickCore/xml-tree.h"

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
%  The format of the AutoGammaImage method is:
%
%      MagickBooleanType AutoGammaImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType AutoGammaImage(Image *image,
  ExceptionInfo *exception)
{
  double
    gamma,
    log_mean,
    mean,
    sans;

  MagickStatusType
    status;

  log_mean=log(0.5);
  if (image->sync != MagickFalse)
    {
      /*
        Apply gamma correction equally accross all given channels.
      */
      (void) GetImageMean(image,&mean,&sans,exception);
      gamma=log(mean*QuantumScale)/log_mean;
      return(LevelImage(image,0.0,(double) QuantumRange,gamma));
    }
  /*
    Auto-gamma each channel separately.
  */
  status=MagickTrue;
  if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
    {
      PushPixelChannelMap(image,RedChannel);
      (void) GetImageMean(image,&mean,&sans,exception);
      gamma=log(mean*QuantumScale)/log_mean;
      status=status && LevelImage(image,0.0,(double) QuantumRange,gamma);
      PopPixelChannelMap(image);
    }
  if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
    {
      PushPixelChannelMap(image,GreenChannel);
      (void) GetImageMean(image,&mean,&sans,exception);
      gamma=log(mean*QuantumScale)/log_mean;
      status=status && LevelImage(image,0.0,(double) QuantumRange,gamma);
      PopPixelChannelMap(image);
    }
  if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
    {
      PushPixelChannelMap(image,BlueChannel);
      (void) GetImageMean(image,&mean,&sans,exception);
      gamma=log(mean*QuantumScale)/log_mean;
      status=status && LevelImage(image,0.0,(double) QuantumRange,gamma);
      PopPixelChannelMap(image);
    }
  if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
      (image->colorspace == CMYKColorspace))
    {
      PushPixelChannelMap(image,BlackChannel);
      (void) GetImageMean(image,&mean,&sans,exception);
      gamma=log(mean*QuantumScale)/log_mean;
      status=status && LevelImage(image,0.0,(double) QuantumRange,gamma);
      PopPixelChannelMap(image);
    }
  if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
      (image->matte == MagickTrue))
    {
      PushPixelChannelMap(image,AlphaChannel);
      (void) GetImageMean(image,&mean,&sans,exception);
      gamma=log(mean*QuantumScale)/log_mean;
      status=status && LevelImage(image,0.0,(double) QuantumRange,gamma);
      PopPixelChannelMap(image);
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
%      MagickBooleanType AutoLevelImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType AutoLevelImage(Image *image,
  ExceptionInfo *exception)
{
  return(MinMaxStretchImage(image,0.0,0.0,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B r i g h t n e s s C o n t r a s t I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BrightnessContrastImage() changes the brightness and/or contrast of an
%  image.  It converts the brightness and contrast parameters into slope and
%  intercept and calls a polynomical function to apply to the image.
%
%  The format of the BrightnessContrastImage method is:
%
%      MagickBooleanType BrightnessContrastImage(Image *image,
%        const double brightness,const double contrast)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o brightness: the brightness percent (-100 .. 100).
%
%    o contrast: the contrast percent (-100 .. 100).
%
*/
MagickExport MagickBooleanType BrightnessContrastImage(Image *image,
  const double brightness,const double contrast)
{
#define BrightnessContastImageTag  "BrightnessContast/Image"

  double
    alpha,
    intercept,
    coefficients[2],
    slope;

  MagickBooleanType
    status;

  /*
    Compute slope and intercept.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  alpha=contrast;
  slope=tan((double) (MagickPI*(alpha/100.0+1.0)/4.0));
  if (slope < 0.0)
    slope=0.0;
  intercept=brightness/100.0+((100-brightness)/200.0)*(1.0-slope);
  coefficients[0]=slope;
  coefficients[1]=intercept;
  status=FunctionImage(image,PolynomialFunction,2,coefficients,
    &image->exception);
  return(status);
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

  CacheView
    *image_view;

  char
    token[MaxTextExtent];

  ColorCorrection
    color_correction;

  const char
    *content,
    *p;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelPacket
    *cdl_map;

  register ssize_t
    i;

  ssize_t
    y;

  XMLTreeInfo
    *cc,
    *ccc,
    *sat,
    *sop;

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
              case 0:
              {
                color_correction.red.slope=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
              case 1:
              {
                color_correction.green.slope=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
              case 2:
              {
                color_correction.blue.slope=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
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
              case 0:
              {
                color_correction.red.offset=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
              case 1:
              {
                color_correction.green.offset=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
              case 2:
              {
                color_correction.blue.offset=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
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
              case 0:
              {
                color_correction.red.power=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
              case 1:
              {
                color_correction.green.power=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
              case 2:
              {
                color_correction.blue.power=InterpretLocaleValue(token,
                  (char **) NULL);
                break;
              }
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
          color_correction.saturation=InterpretLocaleValue(token,
            (char **) NULL);
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
  #pragma omp parallel for schedule(dynamic,4)
#endif
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    cdl_map[i].red=ClampToQuantum((MagickRealType) ScaleMapToQuantum((
      MagickRealType) (MaxMap*(pow(color_correction.red.slope*i/MaxMap+
      color_correction.red.offset,color_correction.red.power)))));
    cdl_map[i].green=ClampToQuantum((MagickRealType) ScaleMapToQuantum((
      MagickRealType) (MaxMap*(pow(color_correction.green.slope*i/MaxMap+
      color_correction.green.offset,color_correction.green.power)))));
    cdl_map[i].blue=ClampToQuantum((MagickRealType) ScaleMapToQuantum((
      MagickRealType) (MaxMap*(pow(color_correction.blue.slope*i/MaxMap+
      color_correction.blue.offset,color_correction.blue.power)))));
  }
  if (image->storage_class == PseudoClass)
    {
      /*
        Apply transfer function to colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        double
          luma;

        luma=0.2126*image->colormap[i].red+0.7152*image->colormap[i].green+
          0.0722*image->colormap[i].blue;
        image->colormap[i].red=ClampToQuantum(luma+color_correction.saturation*
          cdl_map[ScaleQuantumToMap(image->colormap[i].red)].red-luma);
        image->colormap[i].green=ClampToQuantum(luma+
          color_correction.saturation*cdl_map[ScaleQuantumToMap(
          image->colormap[i].green)].green-luma);
        image->colormap[i].blue=ClampToQuantum(luma+color_correction.saturation*
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      luma;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      luma=0.2126*GetPixelRed(image,q)+0.7152*GetPixelGreen(image,q)+0.0722*
        GetPixelBlue(image,q);
      SetPixelRed(image,ClampToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(GetPixelRed(image,q))].red-luma)),q);
      SetPixelGreen(image,ClampToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(GetPixelGreen(image,q))].green-luma)),q);
      SetPixelBlue(image,ClampToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(GetPixelBlue(image,q))].blue-luma)),q);
      q+=GetPixelChannels(image);
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
#define ClampAlphaPixelChannel(pixel) ClampToQuantum((pixel)->alpha)
#define ClampBlackPixelChannel(pixel) ClampToQuantum((pixel)->black)
#define ClampBluePixelChannel(pixel) ClampToQuantum((pixel)->blue)
#define ClampGreenPixelChannel(pixel) ClampToQuantum((pixel)->green)
#define ClampRedPixelChannel(pixel) ClampToQuantum((pixel)->red)
#define ClutImageTag  "Clut/Image"

  CacheView
    *clut_view,
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    *clut_map;

  register ssize_t
    i;

  ssize_t
    adjust,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(clut_image != (Image *) NULL);
  assert(clut_image->signature == MagickSignature);
  exception=(&image->exception);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  clut_map=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*clut_map));
  if (clut_map == (PixelInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Clut image.
  */
  status=MagickTrue;
  progress=0;
  adjust=(ssize_t) (clut_image->interpolate == IntegerInterpolatePixel ? 0 : 1);
  clut_view=AcquireCacheView(clut_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    GetPixelInfo(clut_image,clut_map+i);
    (void) InterpolatePixelInfo(clut_image,clut_view,
      UndefinedInterpolatePixel,QuantumScale*i*(clut_image->columns-adjust),
      QuantumScale*i*(clut_image->rows-adjust),clut_map+i,exception);
  }
  clut_view=DestroyCacheView(clut_view);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    GetPixelInfo(image,&pixel);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelInfo(image,q,&pixel);
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        SetPixelRed(image,ClampRedPixelChannel(clut_map+
          ScaleQuantumToMap(GetPixelRed(image,q))),q);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        SetPixelGreen(image,ClampGreenPixelChannel(clut_map+
          ScaleQuantumToMap(GetPixelGreen(image,q))),q);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        SetPixelBlue(image,ClampBluePixelChannel(clut_map+
          ScaleQuantumToMap(GetPixelBlue(image,q))),q);
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,ClampBlackPixelChannel(clut_map+
          ScaleQuantumToMap(GetPixelBlack(image,q))),q);
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        {
          if (clut_image->matte == MagickFalse)
            SetPixelAlpha(image,GetPixelInfoIntensity(clut_map+
              ScaleQuantumToMap((Quantum) GetPixelAlpha(image,q))),q);
          else
            if (image->matte == MagickFalse)
              SetPixelAlpha(image,ClampAlphaPixelChannel(clut_map+
                ScaleQuantumToMap((Quantum) GetPixelInfoIntensity(&pixel))),q);
            else
              SetPixelAlpha(image,ClampAlphaPixelChannel(clut_map+
                ScaleQuantumToMap(GetPixelAlpha(image,q))),q);
        }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ClutImage)
#endif
        proceed=SetImageProgress(image,ClutImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  clut_map=(PixelInfo *) RelinquishMagickMemory(clut_map);
  if ((clut_image->matte != MagickFalse) &&
      ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0))
    (void) SetImageAlphaChannel(image,ActivateAlphaChannel,exception);
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
  brightness+=0.5*sign*(0.5*(sin((double) (MagickPI*(brightness-0.5)))+1.0)-
    brightness);
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

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  int
    sign;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

  ssize_t
    y;

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
      for (i=0; i < (ssize_t) image->colors; i++)
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      blue,
      green,
      red;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      red=GetPixelRed(image,q);
      green=GetPixelGreen(image,q);
      blue=GetPixelBlue(image,q);
      Contrast(sign,&red,&green,&blue);
      SetPixelRed(image,red,q);
      SetPixelGreen(image,green,q);
      SetPixelBlue(image,blue,q);
      q+=GetPixelChannels(image);
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
%  ContrastStretchImage() is a simple image enhancement technique that attempts
%  to improve the contrast in an image by `stretching' the range of intensity
%  values it contains to span a desired range of values. It differs from the
%  more sophisticated histogram equalization in that it can only apply a
%  linear scaling function to the image pixel values.  As a result the
%  `enhancement' is less harsh.
%
%  The format of the ContrastStretchImage method is:
%
%      MagickBooleanType ContrastStretchImage(Image *image,
%        const char *levels)
%
%  A description of each parameter follows:
%
%    o image: the image.
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
  const double black_point,const double white_point)
{
#define MaxRange(color)  ((MagickRealType) ScaleQuantumToMap((Quantum) (color)))
#define ContrastStretchImageTag  "ContrastStretch/Image"

  CacheView
    *image_view;

  double
    intensity;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    black,
    *histogram,
    *stretch_map,
    white;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate histogram and stretch map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  histogram=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*histogram));
  stretch_map=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*stretch_map));
  if ((histogram == (PixelInfo *) NULL) ||
      (stretch_map == (PixelInfo *) NULL))
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Form histogram.
  */
  status=MagickTrue;
  exception=(&image->exception);
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));
  image_view=AcquireCacheView(image);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelRed(image,p))].red++;
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelGreen(image,p))].green++;
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelBlue(image,p))].blue++;
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        histogram[ScaleQuantumToMap(GetPixelBlack(image,p))].black++;
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelAlpha(image,p))].alpha++;
      p+=GetPixelChannels(image);
    }
  }
  /*
    Find the histogram boundaries by locating the black/white levels.
  */
  black.red=0.0;
  white.red=MaxRange(QuantumRange);
  if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        intensity+=histogram[i].red;
        if (intensity > black_point)
          break;
      }
      black.red=(MagickRealType) i;
      intensity=0.0;
      for (i=(ssize_t) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].red;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.red=(MagickRealType) i;
    }
  black.green=0.0;
  white.green=MaxRange(QuantumRange);
  if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        intensity+=histogram[i].green;
        if (intensity > black_point)
          break;
      }
      black.green=(MagickRealType) i;
      intensity=0.0;
      for (i=(ssize_t) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].green;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.green=(MagickRealType) i;
    }
  black.blue=0.0;
  white.blue=MaxRange(QuantumRange);
  if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        intensity+=histogram[i].blue;
        if (intensity > black_point)
          break;
      }
      black.blue=(MagickRealType) i;
      intensity=0.0;
      for (i=(ssize_t) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].blue;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.blue=(MagickRealType) i;
    }
  black.alpha=0.0;
  white.alpha=MaxRange(QuantumRange);
  if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
    {
      intensity=0.0;
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        intensity+=histogram[i].alpha;
        if (intensity > black_point)
          break;
      }
      black.alpha=(MagickRealType) i;
      intensity=0.0;
      for (i=(ssize_t) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].alpha;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.alpha=(MagickRealType) i;
    }
  black.black=0.0;
  white.black=MaxRange(QuantumRange);
  if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) && (image->colorspace == CMYKColorspace))
    {
      intensity=0.0;
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        intensity+=histogram[i].black;
        if (intensity > black_point)
          break;
      }
      black.black=(MagickRealType) i;
      intensity=0.0;
      for (i=(ssize_t) MaxMap; i != 0; i--)
      {
        intensity+=histogram[i].black;
        if (intensity > ((double) image->columns*image->rows-white_point))
          break;
      }
      white.black=(MagickRealType) i;
    }
  histogram=(PixelInfo *) RelinquishMagickMemory(histogram);
  /*
    Stretch the histogram to create the stretched image mapping.
  */
  (void) ResetMagickMemory(stretch_map,0,(MaxMap+1)*sizeof(*stretch_map));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
      {
        if (i < (ssize_t) black.red)
          stretch_map[i].red=0.0;
        else
          if (i > (ssize_t) white.red)
            stretch_map[i].red=(MagickRealType) QuantumRange;
          else
            if (black.red != white.red)
              stretch_map[i].red=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.red)/(white.red-black.red)));
      }
    if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
      {
        if (i < (ssize_t) black.green)
          stretch_map[i].green=0.0;
        else
          if (i > (ssize_t) white.green)
            stretch_map[i].green=(MagickRealType) QuantumRange;
          else
            if (black.green != white.green)
              stretch_map[i].green=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.green)/(white.green-
                black.green)));
      }
    if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
      {
        if (i < (ssize_t) black.blue)
          stretch_map[i].blue=0.0;
        else
          if (i > (ssize_t) white.blue)
            stretch_map[i].blue=(MagickRealType) QuantumRange;
          else
            if (black.blue != white.blue)
              stretch_map[i].blue=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.blue)/(white.blue-
                black.blue)));
      }
    if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
      {
        if (i < (ssize_t) black.alpha)
          stretch_map[i].alpha=0.0;
        else
          if (i > (ssize_t) white.alpha)
            stretch_map[i].alpha=(MagickRealType) QuantumRange;
          else
            if (black.alpha != white.alpha)
              stretch_map[i].alpha=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.alpha)/(white.alpha-
                black.alpha)));
      }
    if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
        (image->colorspace == CMYKColorspace))
      {
        if (i < (ssize_t) black.black)
          stretch_map[i].black=0.0;
        else
          if (i > (ssize_t) white.black)
            stretch_map[i].black=(MagickRealType) QuantumRange;
          else
            if (black.black != white.black)
              stretch_map[i].black=(MagickRealType) ScaleMapToQuantum(
                (MagickRealType) (MaxMap*(i-black.black)/(white.black-
                black.black)));
      }
  }
  /*
    Stretch the image.
  */
  if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) || (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
      (image->colorspace == CMYKColorspace)))
    image->storage_class=DirectClass;
  if (image->storage_class == PseudoClass)
    {
      /*
        Stretch colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          {
            if (black.red != white.red)
              image->colormap[i].red=ClampToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].red)].red);
          }
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          {
            if (black.green != white.green)
              image->colormap[i].green=ClampToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].green)].green);
          }
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          {
            if (black.blue != white.blue)
              image->colormap[i].blue=ClampToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].blue)].blue);
          }
        if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
          {
            if (black.alpha != white.alpha)
              image->colormap[i].alpha=ClampToQuantum(stretch_map[
                ScaleQuantumToMap(image->colormap[i].alpha)].alpha);
          }
      }
    }
  /*
    Stretch image.
  */
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        {
          if (black.red != white.red)
            SetPixelRed(image,ClampToQuantum(stretch_map[ScaleQuantumToMap(
              GetPixelRed(image,q))].red),q);
        }
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        {
          if (black.green != white.green)
            SetPixelGreen(image,ClampToQuantum(stretch_map[ScaleQuantumToMap(
              GetPixelGreen(image,q))].green),q);
        }
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        {
          if (black.blue != white.blue)
            SetPixelBlue(image,ClampToQuantum(stretch_map[ScaleQuantumToMap(
              GetPixelBlue(image,q))].blue),q);
        }
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if (black.black != white.black)
            SetPixelBlack(image,ClampToQuantum(stretch_map[ScaleQuantumToMap(
              GetPixelBlack(image,q))].black),q);
        }
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        {
          if (black.alpha != white.alpha)
            SetPixelAlpha(image,ClampToQuantum(stretch_map[ScaleQuantumToMap(
              GetPixelAlpha(image,q))].alpha),q);
        }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ContrastStretchImage)
#endif
        proceed=SetImageProgress(image,ContrastStretchImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  stretch_map=(PixelInfo *) RelinquishMagickMemory(stretch_map);
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
  mean=((MagickRealType) GetPixelRed(image,r)+pixel.red)/2; \
  distance=(MagickRealType) GetPixelRed(image,r)-(MagickRealType) pixel.red; \
  distance_squared=QuantumScale*(2.0*((MagickRealType) QuantumRange+1.0)+ \
     mean)*distance*distance; \
  mean=((MagickRealType) GetPixelGreen(image,r)+pixel.green)/2; \
  distance=(MagickRealType) GetPixelGreen(image,r)- \
    (MagickRealType) pixel.green; \
  distance_squared+=4.0*distance*distance; \
  mean=((MagickRealType) GetPixelBlue(image,r)+pixel.blue)/2; \
  distance=(MagickRealType) GetPixelBlue(image,r)- \
    (MagickRealType) pixel.blue; \
  distance_squared+=QuantumScale*(3.0*((MagickRealType) \
    QuantumRange+1.0)-1.0-mean)*distance*distance; \
  mean=((MagickRealType) GetPixelAlpha(image,r)+pixel.alpha)/2; \
  distance=(MagickRealType) GetPixelAlpha(image,r)-(MagickRealType) pixel.alpha; \
  distance_squared+=QuantumScale*(3.0*((MagickRealType) \
    QuantumRange+1.0)-1.0-mean)*distance*distance; \
  if (distance_squared < ((MagickRealType) QuantumRange*(MagickRealType) \
      QuantumRange/25.0f)) \
    { \
      aggregate.red+=(weight)*GetPixelRed(image,r); \
      aggregate.green+=(weight)*GetPixelGreen(image,r); \
      aggregate.blue+=(weight)*GetPixelBlue(image,r); \
      aggregate.alpha+=(weight)*GetPixelAlpha(image,r); \
      total_weight+=(weight); \
    } \
  r++;
#define EnhanceImageTag  "Enhance/Image"

  CacheView
    *enhance_view,
    *image_view;

  Image
    *enhance_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    zero;

  ssize_t
    y;

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
  if (SetImageStorageClass(enhance_image,DirectClass,exception) == MagickFalse)
    {
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    /*
      Read another scan line.
    */
    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-2,y-2,image->columns+4,5,exception);
    q=QueueCacheViewAuthenticPixels(enhance_view,0,y,enhance_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelInfo
        aggregate;

      MagickRealType
        distance,
        distance_squared,
        mean,
        total_weight;

      PixelPacket
        pixel;

      register const Quantum
        *restrict r;

      /*
        Compute weighted average of target pixel color components.
      */
      aggregate=zero;
      total_weight=0.0;
      r=p+2*(image->columns+4)+2;
      GetPixelPacket(image,r,&pixel);
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
      SetPixelRed(enhance_image,(Quantum) ((aggregate.red+
        (total_weight/2)-1)/total_weight),q);
      SetPixelGreen(enhance_image,(Quantum) ((aggregate.green+
        (total_weight/2)-1)/total_weight),q);
      SetPixelBlue(enhance_image,(Quantum) ((aggregate.blue+
        (total_weight/2)-1)/total_weight),q);
      SetPixelAlpha(enhance_image,(Quantum) ((aggregate.alpha+
        (total_weight/2)-1)/total_weight),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(enhance_image);
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
#define EqualizeImageTag  "Equalize/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    black,
    *equalize_map,
    *histogram,
    intensity,
    *map,
    white;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate and initialize histogram arrays.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  equalize_map=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*equalize_map));
  histogram=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*histogram));
  map=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*map));
  if ((equalize_map == (PixelInfo *) NULL) ||
      (histogram == (PixelInfo *) NULL) ||
      (map == (PixelInfo *) NULL))
    {
      if (map != (PixelInfo *) NULL)
        map=(PixelInfo *) RelinquishMagickMemory(map);
      if (histogram != (PixelInfo *) NULL)
        histogram=(PixelInfo *) RelinquishMagickMemory(histogram);
      if (equalize_map != (PixelInfo *) NULL)
        equalize_map=(PixelInfo *) RelinquishMagickMemory(equalize_map);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Form histogram.
  */
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));
  exception=(&image->exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelRed(image,p))].red++;
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelGreen(image,p))].green++;
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelBlue(image,p))].blue++;
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        histogram[ScaleQuantumToMap(GetPixelBlack(image,p))].black++;
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToMap(GetPixelAlpha(image,p))].alpha++;
      p+=GetPixelChannels(image);
    }
  }
  /*
    Integrate the histogram to get the equalization map.
  */
  (void) ResetMagickMemory(&intensity,0,sizeof(intensity));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
      intensity.red+=histogram[i].red;
    if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
      intensity.green+=histogram[i].green;
    if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
      intensity.blue+=histogram[i].blue;
    if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
        (image->colorspace == CMYKColorspace))
      intensity.black+=histogram[i].black;
    if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
      intensity.alpha+=histogram[i].alpha;
    map[i]=intensity;
  }
  black=map[0];
  white=map[(int) MaxMap];
  (void) ResetMagickMemory(equalize_map,0,(MaxMap+1)*sizeof(*equalize_map));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if (((GetPixelRedTraits(image) & UpdatePixelTrait) != 0) &&
        (white.red != black.red))
      equalize_map[i].red=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].red-black.red))/(white.red-black.red)));
    if (((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0) &&
        (white.green != black.green))
      equalize_map[i].green=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].green-black.green))/(white.green-black.green)));
    if (((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0) &&
        (white.blue != black.blue))
      equalize_map[i].blue=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].blue-black.blue))/(white.blue-black.blue)));
    if ((((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
        (image->colorspace == CMYKColorspace)) &&
        (white.black != black.black))
      equalize_map[i].black=(MagickRealType) ScaleMapToQuantum((MagickRealType)
        ((MaxMap*(map[i].black-black.black))/(white.black-black.black)));
    if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
        (white.alpha != black.alpha))
      equalize_map[i].alpha=(MagickRealType) ScaleMapToQuantum(
        (MagickRealType) ((MaxMap*(map[i].alpha-black.alpha))/
        (white.alpha-black.alpha)));
  }
  histogram=(PixelInfo *) RelinquishMagickMemory(histogram);
  map=(PixelInfo *) RelinquishMagickMemory(map);
  if (image->storage_class == PseudoClass)
    {
      /*
        Equalize colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if (((GetPixelRedTraits(image) & UpdatePixelTrait) != 0) &&
            (white.red != black.red))
          image->colormap[i].red=ClampToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].red)].red);
        if (((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0) &&
            (white.green != black.green))
          image->colormap[i].green=ClampToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].green)].green);
        if (((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0) &&
            (white.blue != black.blue))
          image->colormap[i].blue=ClampToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].blue)].blue);
        if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
            (white.alpha != black.alpha))
          image->colormap[i].alpha=ClampToQuantum(equalize_map[
            ScaleQuantumToMap(image->colormap[i].alpha)].alpha);
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (((GetPixelRedTraits(image) & UpdatePixelTrait) != 0) &&
          (white.red != black.red))
        SetPixelRed(image,ClampToQuantum(equalize_map[
          ScaleQuantumToMap(GetPixelRed(image,q))].red),q);
      if (((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0) &&
          (white.green != black.green))
        SetPixelGreen(image,ClampToQuantum(equalize_map[
          ScaleQuantumToMap(GetPixelGreen(image,q))].green),q);
      if (((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0) &&
          (white.blue != black.blue))
        SetPixelBlue(image,ClampToQuantum(equalize_map[
          ScaleQuantumToMap(GetPixelBlue(image,q))].blue),q);
      if ((((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace)) &&
          (white.black != black.black))
        SetPixelBlack(image,ClampToQuantum(equalize_map[
          ScaleQuantumToMap(GetPixelBlack(image,q))].black),q);
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (white.alpha != black.alpha))
        SetPixelAlpha(image,ClampToQuantum(equalize_map[
          ScaleQuantumToMap(GetPixelAlpha(image,q))].alpha),q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_EqualizeImage)
#endif
        proceed=SetImageProgress(image,EqualizeImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  equalize_map=(PixelInfo *) RelinquishMagickMemory(equalize_map);
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
%      MagickBooleanType GammaImage(Image *image,const double gamma,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o level: the image gamma as a string (e.g. 1.6,1.2,1.0).
%
%    o gamma: the image gamma.
%
*/
MagickExport MagickBooleanType GammaImage(Image *image,const double gamma,
  ExceptionInfo *exception)
{
#define GammaCorrectImageTag  "GammaCorrect/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  Quantum
    *gamma_map;

  register ssize_t
    i;

  ssize_t
    y;

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
#if defined(MAGICKCORE_OPENMP_SUPPORT) && (MaxMap > 256)
  #pragma omp parallel for
#endif
    for (i=0; i <= (ssize_t) MaxMap; i++)
      gamma_map[i]=ClampToQuantum((MagickRealType) ScaleMapToQuantum((
        MagickRealType) (MaxMap*pow((double) i/MaxMap,1.0/gamma))));
  if (image->storage_class == PseudoClass)
    {
      /*
        Gamma-correct colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].red=gamma_map[
            ScaleQuantumToMap(image->colormap[i].red)];
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].green=gamma_map[
            ScaleQuantumToMap(image->colormap[i].green)];
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].blue=gamma_map[
            ScaleQuantumToMap(image->colormap[i].blue)];
        if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].alpha=gamma_map[
            ScaleQuantumToMap(image->colormap[i].alpha)];
      }
    }
  /*
    Gamma-correct image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelTrait
          traits;

        traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
        if ((traits & UpdatePixelTrait) != 0)
          q[i]=gamma_map[ScaleQuantumToMap(q[i])];
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_GammaImage)
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
%
%  A description of each parameter follows:
%
%    o image: the image, which is replaced by indexed CLUT values
%
%    o hald_image: the color lookup table image for replacement color values.
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
#define HaldClutImageTag  "Clut/Image"

  typedef struct _HaldInfo
  {
    MagickRealType
      x,
      y,
      z;
  } HaldInfo;

  CacheView
    *hald_view,
    *image_view;

  double
    width;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    zero;

  size_t
    cube_size,
    length,
    level;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(hald_image != (Image *) NULL);
  assert(hald_image->signature == MagickSignature);
  exception=(&image->exception);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
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
  GetPixelInfo(hald_image,&zero);
  image_view=AcquireCacheView(image);
  hald_view=AcquireCacheView(hald_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      offset;

    HaldInfo
      point;

    PixelInfo
      pixel,
      pixel1,
      pixel2,
      pixel3,
      pixel4;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    pixel=zero;
    pixel1=zero;
    pixel2=zero;
    pixel3=zero;
    pixel4=zero;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      point.x=QuantumScale*(level-1.0)*GetPixelRed(image,q);
      point.y=QuantumScale*(level-1.0)*GetPixelGreen(image,q);
      point.z=QuantumScale*(level-1.0)*GetPixelBlue(image,q);
      offset=point.x+level*floor(point.y)+cube_size*floor(point.z);
      point.x-=floor(point.x);
      point.y-=floor(point.y);
      point.z-=floor(point.z);
      (void) InterpolatePixelInfo(image,hald_view,
        UndefinedInterpolatePixel,fmod(offset,width),floor(offset/width),
        &pixel1,exception);
      (void) InterpolatePixelInfo(image,hald_view,
        UndefinedInterpolatePixel,fmod(offset+level,width),floor((offset+level)/
        width),&pixel2,exception);
      CompositePixelInfoAreaBlend(&pixel1,pixel1.alpha,&pixel2,
        pixel2.alpha,point.y,&pixel3);
      offset+=cube_size;
      (void) InterpolatePixelInfo(image,hald_view,
        UndefinedInterpolatePixel,fmod(offset,width),floor(offset/width),
        &pixel1,exception);
      (void) InterpolatePixelInfo(image,hald_view,
        UndefinedInterpolatePixel,fmod(offset+level,width),floor((offset+level)/
        width),&pixel2,exception);
      CompositePixelInfoAreaBlend(&pixel1,pixel1.alpha,&pixel2,
        pixel2.alpha,point.y,&pixel4);
      CompositePixelInfoAreaBlend(&pixel3,pixel3.alpha,&pixel4,
        pixel4.alpha,point.z,&pixel);
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        SetPixelRed(image,
          ClampToQuantum(pixel.red),q);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        SetPixelGreen(image,
          ClampToQuantum(pixel.green),q);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        SetPixelBlue(image,
          ClampToQuantum(pixel.blue),q);
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,
          ClampToQuantum(pixel.black),q);
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) && (image->matte != MagickFalse))
        SetPixelAlpha(image,
          ClampToQuantum(pixel.alpha),q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_HaldClutImage)
#endif
        proceed=SetImageProgress(image,HaldClutImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  hald_view=DestroyCacheView(hald_view);
  image_view=DestroyCacheView(image_view);
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
%  LevelizeImage() below.
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
MagickExport MagickBooleanType LevelImage(Image *image,
  const double black_point,const double white_point,const double gamma)
{
#define LevelImageTag  "Level/Image"
#define LevelQuantum(x) (ClampToQuantum((MagickRealType) QuantumRange* \
  pow(scale*((double) (x)-black_point),1.0/gamma)))

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register double
    scale;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  scale=(white_point != black_point) ? 1.0/(white_point-black_point) : 1.0;
  if (image->storage_class == PseudoClass)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      /*
        Level colormap.
      */
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].red=LevelQuantum(image->colormap[i].red);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].green=LevelQuantum(image->colormap[i].green);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].blue=LevelQuantum(image->colormap[i].blue);
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].alpha=LevelQuantum(image->colormap[i].alpha);
      }
  /*
    Level image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        SetPixelRed(image,LevelQuantum(
          GetPixelRed(image,q)),q);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        SetPixelGreen(image,
          LevelQuantum(GetPixelGreen(image,q)),q);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        SetPixelBlue(image,
          LevelQuantum(GetPixelBlue(image,q)),q);
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (image->matte == MagickTrue))
        SetPixelAlpha(image,
          LevelQuantum(GetPixelAlpha(image,q)),q);
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,
          LevelQuantum(GetPixelBlack(image,q)),q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_LevelImage)
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
%  LevelizeImage() applies the reversed LevelImage() operation to just
%  the specific channels specified.  It compresses the full range of color
%  values, so that they lie between the given black and white points. Gamma is
%  applied before the values are mapped.
%
%  LevelizeImage() can be called with by using a +level command line
%  API option, or using a '!' on a -level or LevelImage() geometry string.
%
%  It can be used for example de-contrast a greyscale image to the exact
%  levels specified.  Or by using specific levels for each channel of an image
%  you can convert a gray-scale image to any linear color gradient, according
%  to those levels.
%
%  The format of the LevelizeImage method is:
%
%      MagickBooleanType LevelizeImage(Image *image,const double black_point,
%        const double white_point,const double gamma)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_point: The level to map zero (black) to.
%
%    o white_point: The level to map QuantiumRange (white) to.
%
%    o gamma: adjust gamma by this factor before mapping values.
%
*/
MagickExport MagickBooleanType LevelizeImage(Image *image,
  const double black_point,const double white_point,const double gamma)
{
#define LevelizeImageTag  "Levelize/Image"
#define LevelizeValue(x) (ClampToQuantum(((MagickRealType) \
  pow((double) (QuantumScale*(x)),1.0/gamma))*(white_point-black_point)+ \
  black_point))

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      /*
        Level colormap.
      */
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].red=LevelizeValue(image->colormap[i].red);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].green=LevelizeValue(image->colormap[i].green);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].blue=LevelizeValue(image->colormap[i].blue);
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].alpha=LevelizeValue(image->colormap[i].alpha);
    }
  /*
    Level image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        SetPixelRed(image,LevelizeValue(GetPixelRed(image,q)),q);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        SetPixelGreen(image,LevelizeValue(GetPixelGreen(image,q)),q);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        SetPixelBlue(image,LevelizeValue(GetPixelBlue(image,q)),q);
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,LevelizeValue(GetPixelBlack(image,q)),q);
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (image->matte == MagickTrue))
        SetPixelAlpha(image,LevelizeValue(GetPixelAlpha(image,q)),q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_LevelizeImage)
#endif
        proceed=SetImageProgress(image,LevelizeImageTag,progress++,image->rows);
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
%     L e v e l I m a g e C o l o r s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelImageColors() maps the given color to "black" and "white" values,
%  linearly spreading out the colors, and level values on a channel by channel
%  bases, as per LevelImage().  The given colors allows you to specify
%  different level ranges for each of the color channels separately.
%
%  If the boolean 'invert' is set true the image values will modifyed in the
%  reverse direction. That is any existing "black" and "white" colors in the
%  image will become the color values given, with all other values compressed
%  appropriatally.  This effectivally maps a greyscale gradient into the given
%  color gradient.
%
%  The format of the LevelImageColors method is:
%
%    MagickBooleanType LevelImageColors(Image *image,
%      const PixelInfo *black_color,const PixelInfo *white_color,
%      const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_color: The color to map black to/from
%
%    o white_point: The color to map white to/from
%
%    o invert: if true map the colors (levelize), rather than from (level)
%
*/
MagickExport MagickBooleanType LevelImageColors(Image *image,
  const PixelInfo *black_color,const PixelInfo *white_color,
  const MagickBooleanType invert)
{
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
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        {
          PushPixelChannelMap(image,RedChannel);
          status|=LevelImage(image,black_color->red,white_color->red,1.0);
          PopPixelChannelMap(image);
        }
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        {
          PushPixelChannelMap(image,GreenChannel);
          status|=LevelImage(image,black_color->green,white_color->green,1.0);
          PopPixelChannelMap(image);
        }
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        {
          PushPixelChannelMap(image,BlueChannel);
          status|=LevelImage(image,black_color->blue,white_color->blue,1.0);
          PopPixelChannelMap(image);
        }
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          PushPixelChannelMap(image,BlackChannel);
          status|=LevelImage(image,black_color->black,white_color->black,1.0);
          PopPixelChannelMap(image);
        }
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (image->matte == MagickTrue))
        {
          PushPixelChannelMap(image,AlphaChannel);
          status|=LevelImage(image,black_color->alpha,white_color->alpha,1.0);
          PopPixelChannelMap(image);
        }
    }
  else
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        {
          PushPixelChannelMap(image,RedChannel);
          status|=LevelizeImage(image,black_color->red,white_color->red,1.0);
          PopPixelChannelMap(image);
        }
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        {
          PushPixelChannelMap(image,GreenChannel);
          status|=LevelizeImage(image,black_color->green,white_color->green,
            1.0);
          PopPixelChannelMap(image);
        }
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        {
          PushPixelChannelMap(image,BlueChannel);
          status|=LevelizeImage(image,black_color->blue,white_color->blue,1.0);
          PopPixelChannelMap(image);
        }
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          PushPixelChannelMap(image,BlackChannel);
          status|=LevelizeImage(image,black_color->black,white_color->black,
            1.0);
          PopPixelChannelMap(image);
        }
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (image->matte == MagickTrue))
        {
          PushPixelChannelMap(image,AlphaChannel);
          status|=LevelizeImage(image,black_color->alpha,white_color->alpha,
            1.0);
          PopPixelChannelMap(image);
        }
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
%  LinearStretchImage() discards any pixels below the black point and above
%  the white point and levels the remaining pixels.
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

  MagickBooleanType
    status;

  MagickRealType
    *histogram,
    intensity;

  ssize_t
    black,
    white,
    y;

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
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=(ssize_t) image->columns-1; x >= 0; x--)
    {
      histogram[ScaleQuantumToMap(GetPixelIntensity(image,p))]++;
      p+=GetPixelChannels(image);
    }
  }
  /*
    Find the histogram boundaries by locating the black and white point levels.
  */
  intensity=0.0;
  for (black=0; black < (ssize_t) MaxMap; black++)
  {
    intensity+=histogram[black];
    if (intensity >= black_point)
      break;
  }
  intensity=0.0;
  for (white=(ssize_t) MaxMap; white != 0; white--)
  {
    intensity+=histogram[white];
    if (intensity >= white_point)
      break;
  }
  histogram=(MagickRealType *) RelinquishMagickMemory(histogram);
  status=LevelImage(image,(double) black,(double) white,1.0);
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

  CacheView
    *image_view;

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

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickStatusType
    flags;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Initialize modulate table.
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
    colorspace=(ColorspaceType) ParseCommandOption(MagickColorspaceOptions,
      MagickFalse,artifact);
  if (image->storage_class == PseudoClass)
    {
      /*
        Modulate colormap.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      blue,
      green,
      red;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      red=GetPixelRed(image,q);
      green=GetPixelGreen(image,q);
      blue=GetPixelBlue(image,q);
      switch (colorspace)
      {
        case HSBColorspace:
        {
          ModulateHSB(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSLColorspace:
        default:
        {
          ModulateHSL(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HWBColorspace:
        {
          ModulateHWB(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
      }
      SetPixelRed(image,red,q);
      SetPixelGreen(image,green,q);
      SetPixelBlue(image,blue,q);
      q+=GetPixelChannels(image);
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
%  The format of the NegateImage method is:
%
%      MagickBooleanType NegateImage(Image *image,
%        const MagickBooleanType grayscale,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o grayscale: If MagickTrue, only negate grayscale pixels within the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType NegateImage(Image *image,
  const MagickBooleanType grayscale,ExceptionInfo *exception)
{
#define NegateImageTag  "Negate/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

  ssize_t
    y;

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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if (grayscale != MagickFalse)
          if ((image->colormap[i].red != image->colormap[i].green) ||
              (image->colormap[i].green != image->colormap[i].blue))
            continue;
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].red=(Quantum) QuantumRange-
            image->colormap[i].red;
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].green=(Quantum) QuantumRange-
            image->colormap[i].green;
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].blue=(Quantum) QuantumRange-
            image->colormap[i].blue;
      }
    }
  /*
    Negate image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  if (grayscale != MagickFalse)
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register Quantum
          *restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register ssize_t
            i;

          if (IsPixelGray(image,q) != MagickFalse)
            {
              q+=GetPixelChannels(image);
              continue;
            }
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelTrait 
              traits;

            traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
            if ((traits & UpdatePixelTrait) != 0)
              q[i]=QuantumRange-q[i];
          }
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_NegateImage)
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelTrait 
          traits;

        traits=GetPixelChannelMapTraits(image,(PixelChannel) i);
        if ((traits & UpdatePixelTrait) != 0)
          q[i]=QuantumRange-q[i];
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_NegateImage)
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
%  NormalizeImage() enhances the contrast of a color image by mapping the
%  darkest 2 percent of all pixel to black and the brightest 1 percent to white.
%
%  The format of the NormalizeImage method is:
%
%      MagickBooleanType NormalizeImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType NormalizeImage(Image *image)
{
  double
    black_point,
    white_point;

  black_point=(double) image->columns*image->rows*0.0015;
  white_point=(double) image->columns*image->rows*0.9995;
  return(ContrastStretchImage(image,black_point,white_point));
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
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o sharpen: Increase or decrease image contrast.
%
%    o alpha: strength of the contrast, the larger the number the more
%      'threshold-like' it becomes.
%
%    o beta: midpoint of the function as a color value 0 to QuantumRange.
%
*/
MagickExport MagickBooleanType SigmoidalContrastImage(Image *image,
  const MagickBooleanType sharpen,const double contrast,const double midpoint)
{
#define SigmoidalContrastImageTag  "SigmoidalContrast/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    *sigmoidal_map;

  register ssize_t
    i;

  ssize_t
    y;

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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (i=0; i <= (ssize_t) MaxMap; i++)
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].red=ClampToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].red)]);
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].green=ClampToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].green)]);
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].blue=ClampToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].blue)]);
        if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].alpha=ClampToQuantum(sigmoidal_map[
            ScaleQuantumToMap(image->colormap[i].alpha)]);
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        SetPixelRed(image,ClampToQuantum(sigmoidal_map[ScaleQuantumToMap(
          GetPixelRed(image,q))]),q);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        SetPixelGreen(image,ClampToQuantum(sigmoidal_map[ScaleQuantumToMap(
          GetPixelGreen(image,q))]),q);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        SetPixelBlue(image,ClampToQuantum(sigmoidal_map[ScaleQuantumToMap(
          GetPixelBlue(image,q))]),q);
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,ClampToQuantum(sigmoidal_map[ScaleQuantumToMap(
          GetPixelBlack(image,q))]),q);
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        SetPixelAlpha(image,ClampToQuantum(sigmoidal_map[ScaleQuantumToMap(
          GetPixelAlpha(image,q))]),q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SigmoidalContrastImage)
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     CCCC   OOO   L       OOO   RRRR   SSSSS  PPPP    AAA    CCCC  EEEEE     %
%    C      O   O  L      O   O  R   R  SS     P   P  A   A  C      E         %
%    C      O   O  L      O   O  RRRR    SSS   PPPP   AAAAA  C      EEE       %
%    C      O   O  L      O   O  R R       SS  P      A   A  C      E         %
%     CCCC   OOO   LLLLL   OOO   R  R   SSSSS  P      A   A   CCCC  EEEEE     %
%                                                                             %
%                                                                             %
%                     MagickCore Image Colorspace Methods                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/property.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/enhance.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility.h"

/*
  Typedef declarations.
*/
typedef struct _TransformPacket
{
  MagickRealType
    x,
    y,
    z;
} TransformPacket;

/*
  Forward declarations.
*/
static MagickBooleanType
  TransformsRGBImage(Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C o l o r s p a c e T y p e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageColorspaceType() returns the potential type of image:
%  sRGBColorspaceType, RGBColorspaceType, GRAYColorspaceType, etc.
%
%  To ensure the image type matches its potential, use SetImageColorspaceType():
%
%    (void) SetImageColorspaceType(image,GetImageColorspaceType(image),
%      exception);
%
%  The format of the GetImageColorspaceType method is:
%
%      ColorspaceType GetImageColorspaceType(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ColorspaceType GetImageColorspaceType(const Image *image,
  ExceptionInfo *exception)
{
  ColorspaceType
    colorspace;

  ImageType
    type;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  colorspace=image->colorspace;
  type=IdentifyImageType(image,exception);
  if ((type == BilevelType) || (type == GrayscaleType) ||
      (type == GrayscaleAlphaType))
    colorspace=GRAYColorspace;
  return(colorspace);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     s R G B T r a n s f o r m I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  sRGBTransformImage() converts the reference image from sRGB to an alternate
%  colorspace.  The transformation matrices are not the standard ones: the
%  weights are rescaled to normalized the range of the transformed values to
%  be [0..QuantumRange].
%
%  The format of the sRGBTransformImage method is:
%
%      MagickBooleanType sRGBTransformImage(Image *image,
%        const ColorspaceType colorspace,EsceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colorspace: the colorspace to transform the image to.
%
%   o exception: return any errors or warnings in this structure.
%
*/

static inline void ConvertRGBToCMY(const double red,const double green,
  const double blue,double *cyan,double *magenta,double *yellow)
{
  *cyan=QuantumScale*(QuantumRange-red);
  *magenta=QuantumScale*(QuantumRange-green);
  *yellow=QuantumScale*(QuantumRange-blue);
}

static inline void ConvertXYZToLMS(const double x,const double y,
  const double z,double *L,double *M,double *S)
{
  *L=0.7328*x+0.4296*y-0.1624*z;
  *M=(-0.7036*x+1.6975*y+0.0061*z);
  *S=0.0030*x+0.0136*y+0.9834*z;
}

static void ConvertRGBToLMS(const double red,const double green,
  const double blue,double *L,double *M,double *S)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLMS(X,Y,Z,L,M,S);
}

static void ConvertRGBToLab(const double red,const double green,
  const double blue,double *L,double *a,double *b)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLab(X,Y,Z,L,a,b);
}

static void ConvertRGBToLuv(const double red,const double green,
  const double blue,double *L,double *u,double *v)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLuv(X,Y,Z,L,u,v);
}

static void ConvertRGBToxyY(const double red,const double green,
  const double blue,double *low_x,double *low_y,double *cap_Y)
{
  double
    gamma,
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  gamma=PerceptibleReciprocal(X+Y+Z);
  *low_x=gamma*X;
  *low_y=gamma*Y;
  *cap_Y=Y;
}

static void ConvertRGBToYDbDr(const double red,const double green,
  const double blue,double *Y,double *Db,double *Dr)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *Db=QuantumScale*(-0.450*red-0.883*green+1.333*blue)+0.5;
  *Dr=QuantumScale*(-1.333*red+1.116*green+0.217*blue)+0.5;
}

static void ConvertRGBToYIQ(const double red,const double green,
  const double blue,double *Y,double *I,double *Q)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *I=QuantumScale*(0.595716*red-0.274453*green-0.321263*blue)+0.5;
  *Q=QuantumScale*(0.211456*red-0.522591*green+0.311135*blue)+0.5;
}

static void ConvertRGBToYPbPr(const double red,const double green,
  const double blue,double *Y,double *Pb,double *Pr)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *Pb=QuantumScale*((-0.1687367)*red-0.331264*green+0.5*blue)+0.5;
  *Pr=QuantumScale*(0.5*red-0.418688*green-0.081312*blue)+0.5;
}

static void ConvertRGBToYCbCr(const double red,const double green,
  const double blue,double *Y,double *Cb,double *Cr)
{
  ConvertRGBToYPbPr(red,green,blue,Y,Cb,Cr);
}

static void ConvertRGBToYUV(const double red,const double green,
  const double blue,double *Y,double *U,double *V)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *U=QuantumScale*((-0.147)*red-0.289*green+0.436*blue)+0.5;
  *V=QuantumScale*(0.615*red-0.515*green-0.100*blue)+0.5;
}

static MagickBooleanType sRGBTransformImage(Image *image,
  const ColorspaceType colorspace,ExceptionInfo *exception)
{
#define sRGBTransformImageTag  "RGBTransform/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PrimaryInfo
    primary_info;

  register ssize_t
    i;

  ssize_t
    y;

  TransformPacket
    *x_map,
    *y_map,
    *z_map;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(colorspace != sRGBColorspace);
  assert(colorspace != TransparentColorspace);
  assert(colorspace != UndefinedColorspace);
  status=MagickTrue;
  progress=0;
  switch (colorspace)
  {
    case CMYKColorspace:
    {
      PixelInfo
        zero;

      /*
        Convert RGB to CMYK colorspace.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      if (SetImageColorspace(image,colorspace,exception) == MagickFalse)
        return(MagickFalse);
      GetPixelInfo(image,&zero);
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        PixelInfo
          pixel;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixel=zero;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          GetPixelInfoPixel(image,q,&pixel);
          ConvertRGBToCMYK(&pixel);
          SetPixelViaPixelInfo(image,&pixel,q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      image->type=image->alpha_trait == UndefinedPixelTrait ?
        ColorSeparationType : ColorSeparationAlphaType;
      if (SetImageColorspace(image,colorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case LinearGRAYColorspace:
    case GRAYColorspace:
    {
      /*
        Transform image from sRGB to GRAY.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelGray(image,ClampToQuantum(GetPixelIntensity(image,q)),q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,colorspace,exception) == MagickFalse)
        return(MagickFalse);
      image->type=GrayscaleType;
      return(status);
    }
    case CMYColorspace:
    case HCLColorspace:
    case HCLpColorspace:
    case HSBColorspace:
    case HSIColorspace:
    case HSLColorspace:
    case HSVColorspace:
    case HWBColorspace:
    case LabColorspace:
    case LCHColorspace:
    case LCHabColorspace:
    case LCHuvColorspace:
    case LMSColorspace:
    case LuvColorspace:
    case xyYColorspace:
    case XYZColorspace:
    case YCbCrColorspace:
    case YDbDrColorspace:
    case YIQColorspace:
    case YPbPrColorspace:
    case YUVColorspace:
    {
      /*
        Transform image from sRGB to target colorspace.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          double
            blue,
            green,
            red,
            X,
            Y,
            Z;

          red=(double) GetPixelRed(image,q);
          green=(double) GetPixelGreen(image,q);
          blue=(double) GetPixelBlue(image,q);
          switch (colorspace)
          {
            case CMYColorspace:
            {
              ConvertRGBToCMY(red,green,blue,&X,&Y,&Z);
              break;
            }
            case HCLColorspace:
            {
              ConvertRGBToHCL(red,green,blue,&X,&Y,&Z);
              break;
            }
            case HCLpColorspace:
            {
              ConvertRGBToHCLp(red,green,blue,&X,&Y,&Z);
              break;
            }
            case HSBColorspace:
            {
              ConvertRGBToHSB(red,green,blue,&X,&Y,&Z);
              break;
            }
            case HSIColorspace:
            {
              ConvertRGBToHSI(red,green,blue,&X,&Y,&Z);
              break;
            }
            case HSLColorspace:
            {
              ConvertRGBToHSL(red,green,blue,&X,&Y,&Z);
              break;
            }
            case HSVColorspace:
            {
              ConvertRGBToHSV(red,green,blue,&X,&Y,&Z);
              break;
            }
            case HWBColorspace:
            {
              ConvertRGBToHWB(red,green,blue,&X,&Y,&Z);
              break;
            }
            case LabColorspace:
            {
              ConvertRGBToLab(red,green,blue,&X,&Y,&Z);
              break;
            }
            case LCHColorspace:
            case LCHabColorspace:
            {
              ConvertRGBToLCHab(red,green,blue,&X,&Y,&Z);
              break;
            }
            case LCHuvColorspace:
            {
              ConvertRGBToLCHuv(red,green,blue,&X,&Y,&Z);
              break;
            }
            case LMSColorspace:
            {
              ConvertRGBToLMS(red,green,blue,&X,&Y,&Z);
              break;
            }
            case LuvColorspace:
            {
              ConvertRGBToLuv(red,green,blue,&X,&Y,&Z);
              break;
            }
            case xyYColorspace:
            {
              ConvertRGBToxyY(red,green,blue,&X,&Y,&Z);
              break;
            }
            case XYZColorspace:
            {
              ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
              break;
            }
            case YCbCrColorspace:
            {
              ConvertRGBToYCbCr(red,green,blue,&X,&Y,&Z);
              break;
            }
            case YDbDrColorspace:
            {
              ConvertRGBToYDbDr(red,green,blue,&X,&Y,&Z);
              break;
            }
            case YIQColorspace:
            {
              ConvertRGBToYIQ(red,green,blue,&X,&Y,&Z);
              break;
            }
            case YPbPrColorspace:
            {
              ConvertRGBToYPbPr(red,green,blue,&X,&Y,&Z);
              break;
            }
            case YUVColorspace:
            {
              ConvertRGBToYUV(red,green,blue,&X,&Y,&Z);
              break;
            }
            default:
            {
              X=QuantumScale*red;
              Y=QuantumScale*green;
              Z=QuantumScale*blue;
              break;
            }
          }
          SetPixelRed(image,ClampToQuantum(QuantumRange*X),q);
          SetPixelGreen(image,ClampToQuantum(QuantumRange*Y),q);
          SetPixelBlue(image,ClampToQuantum(QuantumRange*Z),q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,colorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case LogColorspace:
    {
#define DisplayGamma  (1.0/1.7)
#define FilmGamma  0.6
#define ReferenceBlack  95.0
#define ReferenceWhite  685.0

      const char
        *value;

      double
        black,
        density,
        film_gamma,
        gamma,
        reference_black,
        reference_white;

      Quantum
        *logmap;

      /*
        Transform RGB to Log colorspace.
      */
      density=DisplayGamma;
      gamma=DisplayGamma;
      value=GetImageProperty(image,"gamma",exception);
      if (value != (const char *) NULL)
        gamma=PerceptibleReciprocal(StringToDouble(value,(char **) NULL));
      film_gamma=FilmGamma;
      value=GetImageProperty(image,"film-gamma",exception);
      if (value != (const char *) NULL)
        film_gamma=StringToDouble(value,(char **) NULL);
      reference_black=ReferenceBlack;
      value=GetImageProperty(image,"reference-black",exception);
      if (value != (const char *) NULL)
        reference_black=StringToDouble(value,(char **) NULL);
      reference_white=ReferenceWhite;
      value=GetImageProperty(image,"reference-white",exception);
      if (value != (const char *) NULL)
        reference_white=StringToDouble(value,(char **) NULL);
      logmap=(Quantum *) AcquireQuantumMemory((size_t) MaxMap+1UL,
        sizeof(*logmap));
      if (logmap == (Quantum *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      black=pow(10.0,(reference_black-reference_white)*(gamma/density)*0.002/
        film_gamma);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
        logmap[i]=ScaleMapToQuantum((double) (MaxMap*(reference_white+
          log10(black+(1.0*i/MaxMap)*(1.0-black))/((gamma/density)*0.002/
          film_gamma))/1024.0));
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=(ssize_t) image->columns; x != 0; x--)
        {
          double
            blue,
            green,
            red;

          red=(double) DecodePixelGamma((MagickRealType)
            GetPixelRed(image,q));
          green=(double) DecodePixelGamma((MagickRealType)
            GetPixelGreen(image,q));
          blue=(double) DecodePixelGamma((MagickRealType)
            GetPixelBlue(image,q));
          SetPixelRed(image,logmap[ScaleQuantumToMap(ClampToQuantum(red))],q);
          SetPixelGreen(image,logmap[ScaleQuantumToMap(ClampToQuantum(green))],
            q);
          SetPixelBlue(image,logmap[ScaleQuantumToMap(ClampToQuantum(blue))],q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      logmap=(Quantum *) RelinquishMagickMemory(logmap);
      if (SetImageColorspace(image,colorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case RGBColorspace:
    case scRGBColorspace:
    {
      /*
        Transform image from sRGB to linear RGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          double
            blue,
            green,
            red;

          red=DecodePixelGamma((MagickRealType) GetPixelRed(image,q));
          green=DecodePixelGamma((MagickRealType) GetPixelGreen(image,q));
          blue=DecodePixelGamma((MagickRealType) GetPixelBlue(image,q));
          SetPixelRed(image,ClampToQuantum(red),q);
          SetPixelGreen(image,ClampToQuantum(green),q);
          SetPixelBlue(image,ClampToQuantum(blue),q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,colorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    default:
      break;
  }
  /*
    Allocate the tables.
  */
  x_map=(TransformPacket *) AcquireQuantumMemory((size_t) MaxMap+1UL,
    sizeof(*x_map));
  y_map=(TransformPacket *) AcquireQuantumMemory((size_t) MaxMap+1UL,
    sizeof(*y_map));
  z_map=(TransformPacket *) AcquireQuantumMemory((size_t) MaxMap+1UL,
    sizeof(*z_map));
  if ((x_map == (TransformPacket *) NULL) ||
      (y_map == (TransformPacket *) NULL) ||
      (z_map == (TransformPacket *) NULL))
    {
      if (x_map != (TransformPacket *) NULL)
        x_map=(TransformPacket *) RelinquishMagickMemory(x_map);
      if (y_map != (TransformPacket *) NULL)
        y_map=(TransformPacket *) RelinquishMagickMemory(y_map);
      if (z_map != (TransformPacket *) NULL)
        z_map=(TransformPacket *) RelinquishMagickMemory(z_map);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  (void) ResetMagickMemory(&primary_info,0,sizeof(primary_info));
  switch (colorspace)
  {
    case OHTAColorspace:
    {
      /*
        Initialize OHTA tables:

          I1 = 0.33333*R+0.33334*G+0.33333*B
          I2 = 0.50000*R+0.00000*G-0.50000*B
          I3 =-0.25000*R+0.50000*G-0.25000*B

        I and Q, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (0.33333*(double) i);
        y_map[i].x=(MagickRealType) (0.33334*(double) i);
        z_map[i].x=(MagickRealType) (0.33333*(double) i);
        x_map[i].y=(MagickRealType) (0.50000*(double) i);
        y_map[i].y=(MagickRealType) (0.00000*(double) i);
        z_map[i].y=(MagickRealType) (-0.50000*(double) i);
        x_map[i].z=(MagickRealType) (-0.25000*(double) i);
        y_map[i].z=(MagickRealType) (0.50000*(double) i);
        z_map[i].z=(MagickRealType) (-0.25000*(double) i);
      }
      break;
    }
    case Rec601YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables (ITU-R BT.601):

          Y =  0.2988390*R+0.5868110*G+0.1143500*B
          Cb= -0.1687367*R-0.3312640*G+0.5000000*B
          Cr=  0.5000000*R-0.4186880*G-0.0813120*B

        Cb and Cr, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (0.298839*(double) i);
        y_map[i].x=(MagickRealType) (0.586811*(double) i);
        z_map[i].x=(MagickRealType) (0.114350*(double) i);
        x_map[i].y=(MagickRealType) (-0.1687367*(double) i);
        y_map[i].y=(MagickRealType) (-0.331264*(double) i);
        z_map[i].y=(MagickRealType) (0.500000*(double) i);
        x_map[i].z=(MagickRealType) (0.500000*(double) i);
        y_map[i].z=(MagickRealType) (-0.418688*(double) i);
        z_map[i].z=(MagickRealType) (-0.081312*(double) i);
      }
      break;
    }
    case Rec709YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables (ITU-R BT.709):

          Y =  0.212656*R+0.715158*G+0.072186*B
          Cb= -0.114572*R-0.385428*G+0.500000*B
          Cr=  0.500000*R-0.454153*G-0.045847*B

        Cb and Cr, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (0.212656*(double) i);
        y_map[i].x=(MagickRealType) (0.715158*(double) i);
        z_map[i].x=(MagickRealType) (0.072186*(double) i);
        x_map[i].y=(MagickRealType) (-0.114572*(double) i);
        y_map[i].y=(MagickRealType) (-0.385428*(double) i);
        z_map[i].y=(MagickRealType) (0.500000*(double) i);
        x_map[i].z=(MagickRealType) (0.500000*(double) i);
        y_map[i].z=(MagickRealType) (-0.454153*(double) i);
        z_map[i].z=(MagickRealType) (-0.045847*(double) i);
      }
      break;
    }
    case YCCColorspace:
    {
      /*
        Initialize YCC tables:

          Y =  0.298839*R+0.586811*G+0.114350*B
          C1= -0.298839*R-0.586811*G+0.88600*B
          C2=  0.70100*R-0.586811*G-0.114350*B

        YCC is scaled by 1.3584.  C1 zero is 156 and C2 is at 137.
      */
      primary_info.y=(double) ScaleQuantumToMap(ScaleCharToQuantum(156));
      primary_info.z=(double) ScaleQuantumToMap(ScaleCharToQuantum(137));
      for (i=0; i <= (ssize_t) (0.018*MaxMap); i++)
      {
        x_map[i].x=0.005382*i;
        y_map[i].x=0.010566*i;
        z_map[i].x=0.002052*i;
        x_map[i].y=(-0.003296)*i;
        y_map[i].y=(-0.006471)*i;
        z_map[i].y=0.009768*i;
        x_map[i].z=0.009410*i;
        y_map[i].z=(-0.007880)*i;
        z_map[i].z=(-0.001530)*i;
      }
      for ( ; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=0.298839*(1.099*i-0.099);
        y_map[i].x=0.586811*(1.099*i-0.099);
        z_map[i].x=0.114350*(1.099*i-0.099);
        x_map[i].y=(-0.298839)*(1.099*i-0.099);
        y_map[i].y=(-0.586811)*(1.099*i-0.099);
        z_map[i].y=0.88600*(1.099*i-0.099);
        x_map[i].z=0.70100*(1.099*i-0.099);
        y_map[i].z=(-0.586811)*(1.099*i-0.099);
        z_map[i].z=(-0.114350)*(1.099*i-0.099);
      }
      break;
    }
    default:
    {
      /*
        Linear conversion tables.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (1.0*(double) i);
        y_map[i].x=(MagickRealType) 0.0;
        z_map[i].x=(MagickRealType) 0.0;
        x_map[i].y=(MagickRealType) 0.0;
        y_map[i].y=(MagickRealType) (1.0*(double) i);
        z_map[i].y=(MagickRealType) 0.0;
        x_map[i].z=(MagickRealType) 0.0;
        y_map[i].z=(MagickRealType) 0.0;
        z_map[i].z=(MagickRealType) (1.0*(double) i);
      }
      break;
    }
  }
  /*
    Convert from sRGB.
  */
  switch (image->storage_class)
  {
    case DirectClass:
    default:
    {
      /*
        Convert DirectClass image.
      */
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        PixelInfo
          pixel;

        register Quantum
          *magick_restrict q;

        register ssize_t
          x;

        register unsigned int
          blue,
          green,
          red;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          red=ScaleQuantumToMap(ClampToQuantum((MagickRealType)
            GetPixelRed(image,q)));
          green=ScaleQuantumToMap(ClampToQuantum((MagickRealType)
            GetPixelGreen(image,q)));
          blue=ScaleQuantumToMap(ClampToQuantum((MagickRealType)
            GetPixelBlue(image,q)));
          pixel.red=(x_map[red].x+y_map[green].x+z_map[blue].x)+
            primary_info.x;
          pixel.green=(x_map[red].y+y_map[green].y+z_map[blue].y)+
            primary_info.y;
          pixel.blue=(x_map[red].z+y_map[green].z+z_map[blue].z)+
            primary_info.z;
          SetPixelRed(image,ScaleMapToQuantum(pixel.red),q);
          SetPixelGreen(image,ScaleMapToQuantum(pixel.green),q);
          SetPixelBlue(image,ScaleMapToQuantum(pixel.blue),q);
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
            #pragma omp critical (MagickCore_sRGBTransformImage)
#endif
            proceed=SetImageProgress(image,sRGBTransformImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      image_view=DestroyCacheView(image_view);
      break;
    }
    case PseudoClass:
    {
      register unsigned int
        blue,
        green,
        red;

      /*
        Convert PseudoClass image.
      */
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        PixelInfo
          pixel;

        red=ScaleQuantumToMap(ClampToQuantum(image->colormap[i].red));
        green=ScaleQuantumToMap(ClampToQuantum(image->colormap[i].green));
        blue=ScaleQuantumToMap(ClampToQuantum(image->colormap[i].blue));
        pixel.red=x_map[red].x+y_map[green].x+z_map[blue].x+primary_info.x;
        pixel.green=x_map[red].y+y_map[green].y+z_map[blue].y+primary_info.y;
        pixel.blue=x_map[red].z+y_map[green].z+z_map[blue].z+primary_info.z;
        image->colormap[i].red=(double) ScaleMapToQuantum(pixel.red);
        image->colormap[i].green=(double) ScaleMapToQuantum(pixel.green);
        image->colormap[i].blue=(double) ScaleMapToQuantum(pixel.blue);
      }
      (void) SyncImage(image,exception);
      break;
    }
  }
  /*
    Relinquish resources.
  */
  z_map=(TransformPacket *) RelinquishMagickMemory(z_map);
  y_map=(TransformPacket *) RelinquishMagickMemory(y_map);
  x_map=(TransformPacket *) RelinquishMagickMemory(x_map);
  if (SetImageColorspace(image,colorspace,exception) == MagickFalse)
    return(MagickFalse);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e C o l o r s p a c e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageColorspace() sets the colorspace member of the Image structure.
%
%  The format of the SetImageColorspace method is:
%
%      MagickBooleanType SetImageColorspace(Image *image,
%        const ColorspaceType colorspace,ExceptiionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colorspace: the colorspace.
%
%   o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageColorspace(Image *image,
  const ColorspaceType colorspace,ExceptionInfo *exception)
{
  ImageType
    type;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (image->colorspace == colorspace)
    return(MagickTrue);
  image->colorspace=colorspace;
  image->rendering_intent=UndefinedIntent;
  image->gamma=1.000/2.200;
  (void) ResetMagickMemory(&image->chromaticity,0,sizeof(image->chromaticity));
  type=image->type;
  if (IsGrayColorspace(colorspace) != MagickFalse)
    {
      if (colorspace == LinearGRAYColorspace)
        image->gamma=1.000;
      type=GrayscaleType;
    }
  else
    if ((IsRGBColorspace(colorspace) != MagickFalse) ||
        (colorspace == XYZColorspace) || (colorspace == xyYColorspace))
      image->gamma=1.000;
    else
      {
        image->rendering_intent=PerceptualIntent;
        image->chromaticity.red_primary.x=0.6400;
        image->chromaticity.red_primary.y=0.3300;
        image->chromaticity.red_primary.z=0.0300;
        image->chromaticity.green_primary.x=0.3000;
        image->chromaticity.green_primary.y=0.6000;
        image->chromaticity.green_primary.z=0.1000;
        image->chromaticity.blue_primary.x=0.1500;
        image->chromaticity.blue_primary.y=0.0600;
        image->chromaticity.blue_primary.z=0.7900;
        image->chromaticity.white_point.x=0.3127;
        image->chromaticity.white_point.y=0.3290;
        image->chromaticity.white_point.z=0.3583;
      }
  status=SyncImagePixelCache(image,exception);
  image->type=type;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e t I m a g e G r a y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageGray() returns MagickTrue if all the pixels in the image have the
%  same red, green, and blue intensities and changes the type of the image to
%  bi-level or grayscale.
%
%  The format of the SetImageGray method is:
%
%      MagickBooleanType SetImageGray(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageGray(Image *image,
  ExceptionInfo *exception)
{
  const char
    *value;

  ImageType
    type;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (IsImageGray(image))
    return(MagickTrue);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    return(MagickFalse);
  value=GetImageProperty(image,"colorspace:auto-grayscale",exception);
  if (IsStringFalse(value) != MagickFalse)
    return(MagickFalse);
  type=IdentifyImageGray(image,exception);
  if (type == UndefinedType)
    return(MagickFalse);
  image->colorspace=GRAYColorspace;
  if (SyncImagePixelCache((Image *) image,exception) == MagickFalse)
    return(MagickFalse);
  image->type=type;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e M o n o c h r o m e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageMonochrome() returns MagickTrue if all the pixels in the image have
%  the same red, green, and blue intensities and the intensity is either
%  0 or QuantumRange and changes the type of the image to bi-level.
%
%  The format of the SetImageMonochrome method is:
%
%      MagickBooleanType SetImageMonochrome(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageMonochrome(Image *image,
  ExceptionInfo *exception)
{
  const char
    *value;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->type == BilevelType)
    return(MagickTrue);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    return(MagickFalse);
  value=GetImageProperty(image,"colorspace:auto-grayscale",exception);
  if (IsStringFalse(value) != MagickFalse)
    return(MagickFalse);
  if (IdentifyImageMonochrome(image,exception) == MagickFalse)
    return(MagickFalse);
  image->colorspace=GRAYColorspace;
  if (SyncImagePixelCache((Image *) image,exception) == MagickFalse)
    return(MagickFalse);
  image->type=BilevelType;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s f o r m I m a g e C o l o r s p a c e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformImageColorspace() transforms an image colorspace, changing the
%  image data to reflect the new colorspace.
%
%  The format of the TransformImageColorspace method is:
%
%      MagickBooleanType TransformImageColorspace(Image *image,
%        const ColorspaceType colorspace,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colorspace: the colorspace.
%
%   o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType TransformImageColorspace(Image *image,
  const ColorspaceType colorspace,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->colorspace == colorspace)
    return(SetImageColorspace(image,colorspace,exception));
  (void) DeleteImageProfile(image,"icc");
  (void) DeleteImageProfile(image,"icm");
  if (colorspace == LinearGRAYColorspace)
    return(GrayscaleImage(image,Rec709LuminancePixelIntensityMethod,exception));
  if (colorspace == GRAYColorspace)
    return(GrayscaleImage(image,Rec709LumaPixelIntensityMethod,exception));
  if (colorspace == UndefinedColorspace)
    return(SetImageColorspace(image,colorspace,exception));
  /*
    Convert the reference image from an alternate colorspace to sRGB.
  */
  if (IssRGBColorspace(colorspace) != MagickFalse)
    return(TransformsRGBImage(image,exception));
  status=MagickTrue;
  if (IssRGBColorspace(image->colorspace) == MagickFalse)
    status=TransformsRGBImage(image,exception);
  if (status == MagickFalse)
    return(status);
  /*
    Convert the reference image from sRGB to an alternate colorspace.
  */
  if (sRGBTransformImage(image,colorspace,exception) == MagickFalse)
    status=MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     T r a n s f o r m s R G B I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformsRGBImage() converts the reference image from an alternate
%  colorspace to sRGB.  The transformation matrices are not the standard ones:
%  the weights are rescaled to normalize the range of the transformed values
%  to be [0..QuantumRange].
%
%  The format of the TransformsRGBImage method is:
%
%      MagickBooleanType TransformsRGBImage(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%   o exception: return any errors or warnings in this structure.
%
*/

static inline void ConvertCMYToRGB(const double cyan,const double magenta,
  const double yellow,double *red,double *green,double *blue)
{
  *red=QuantumRange*(1.0-cyan);
  *green=QuantumRange*(1.0-magenta);
  *blue=QuantumRange*(1.0-yellow);
}

static inline void ConvertLMSToXYZ(const double L,const double M,const double S,
  double *X,double *Y,double *Z)
{
  *X=1.096123820835514*L-0.278869000218287*M+0.182745179382773*S;
  *Y=0.454369041975359*L+0.473533154307412*M+0.072097803717229*S;
  *Z=(-0.009627608738429)*L-0.005698031216113*M+1.015325639954543*S;
}

static inline void ConvertLMSToRGB(const double L,const double M,
  const double S,double *red,double *green,double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertLMSToXYZ(L,M,S,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertLuvToRGB(const double L,const double u,
  const double v,double *red,double *green,double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertLuvToXYZ(100.0*L,354.0*u-134.0,262.0*v-140.0,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline ssize_t RoundToYCC(const double value)
{
  if (value <= 0.0)
    return(0);
  if (value >= 1388.0)
    return(1388);
  return((ssize_t) (value+0.5));
}

static inline void ConvertLabToRGB(const double L,const double a,
  const double b,double *red,double *green,double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertLabToXYZ(100.0*L,255.0*(a-0.5),255.0*(b-0.5),&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertxyYToRGB(const double low_x,const double low_y,
  const double cap_Y,double *red,double *green,double *blue)
{
  double
    gamma,
    X,
    Y,
    Z;

  gamma=PerceptibleReciprocal(low_y);
  X=gamma*cap_Y*low_x;
  Y=cap_Y;
  Z=gamma*cap_Y*(1.0-low_x-low_y);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static void ConvertYPbPrToRGB(const double Y,const double Pb,const double Pr,
  double *red,double *green,double *blue)
{
  *red=QuantumRange*(0.99999999999914679361*Y-1.2188941887145875e-06*(Pb-0.5)+
    1.4019995886561440468*(Pr-0.5));
  *green=QuantumRange*(0.99999975910502514331*Y-0.34413567816504303521*(Pb-0.5)-
    0.71413649331646789076*(Pr-0.5));
  *blue=QuantumRange*(1.00000124040004623180*Y+1.77200006607230409200*(Pb-0.5)+
    2.1453384174593273e-06*(Pr-0.5));
}

static void ConvertYCbCrToRGB(const double Y,const double Cb,
  const double Cr,double *red,double *green,double *blue)
{
  ConvertYPbPrToRGB(Y,Cb,Cr,red,green,blue);
}

static void ConvertYIQToRGB(const double Y,const double I,const double Q,
  double *red,double *green,double *blue)
{
  *red=QuantumRange*(Y+0.9562957197589482261*(I-0.5)+0.6210244164652610754*
    (Q-0.5));
  *green=QuantumRange*(Y-0.2721220993185104464*(I-0.5)-0.6473805968256950427*
    (Q-0.5));
  *blue=QuantumRange*(Y-1.1069890167364901945*(I-0.5)+1.7046149983646481374*
    (Q-0.5));
}

static void ConvertYDbDrToRGB(const double Y,const double Db,const double Dr,
  double *red,double *green,double *blue)
{
  *red=QuantumRange*(Y+9.2303716147657e-05*(Db-0.5)-
    0.52591263066186533*(Dr-0.5));
  *green=QuantumRange*(Y-0.12913289889050927*(Db-0.5)+
    0.26789932820759876*(Dr-0.5));
  *blue=QuantumRange*(Y+0.66467905997895482*(Db-0.5)-
    7.9202543533108e-05*(Dr-0.5));
}

static void ConvertYUVToRGB(const double Y,const double U,const double V,
  double *red,double *green,double *blue)
{
  *red=QuantumRange*(Y-3.945707070708279e-05*(U-0.5)+1.1398279671717170825*
    (V-0.5));
  *green=QuantumRange*(Y-0.3946101641414141437*(U-0.5)-0.5805003156565656797*
    (V-0.5));
  *blue=QuantumRange*(Y+2.0319996843434342537*(U-0.5)-4.813762626262513e-04*
    (V-0.5));
}

static MagickBooleanType TransformsRGBImage(Image *image,
  ExceptionInfo *exception)
{
#define TransformsRGBImageTag  "Transform/Image"

  static const float
    YCCMap[1389] =
    {
      0.000000f, 0.000720f, 0.001441f, 0.002161f, 0.002882f, 0.003602f,
      0.004323f, 0.005043f, 0.005764f, 0.006484f, 0.007205f, 0.007925f,
      0.008646f, 0.009366f, 0.010086f, 0.010807f, 0.011527f, 0.012248f,
      0.012968f, 0.013689f, 0.014409f, 0.015130f, 0.015850f, 0.016571f,
      0.017291f, 0.018012f, 0.018732f, 0.019452f, 0.020173f, 0.020893f,
      0.021614f, 0.022334f, 0.023055f, 0.023775f, 0.024496f, 0.025216f,
      0.025937f, 0.026657f, 0.027378f, 0.028098f, 0.028818f, 0.029539f,
      0.030259f, 0.030980f, 0.031700f, 0.032421f, 0.033141f, 0.033862f,
      0.034582f, 0.035303f, 0.036023f, 0.036744f, 0.037464f, 0.038184f,
      0.038905f, 0.039625f, 0.040346f, 0.041066f, 0.041787f, 0.042507f,
      0.043228f, 0.043948f, 0.044669f, 0.045389f, 0.046110f, 0.046830f,
      0.047550f, 0.048271f, 0.048991f, 0.049712f, 0.050432f, 0.051153f,
      0.051873f, 0.052594f, 0.053314f, 0.054035f, 0.054755f, 0.055476f,
      0.056196f, 0.056916f, 0.057637f, 0.058357f, 0.059078f, 0.059798f,
      0.060519f, 0.061239f, 0.061960f, 0.062680f, 0.063401f, 0.064121f,
      0.064842f, 0.065562f, 0.066282f, 0.067003f, 0.067723f, 0.068444f,
      0.069164f, 0.069885f, 0.070605f, 0.071326f, 0.072046f, 0.072767f,
      0.073487f, 0.074207f, 0.074928f, 0.075648f, 0.076369f, 0.077089f,
      0.077810f, 0.078530f, 0.079251f, 0.079971f, 0.080692f, 0.081412f,
      0.082133f, 0.082853f, 0.083573f, 0.084294f, 0.085014f, 0.085735f,
      0.086455f, 0.087176f, 0.087896f, 0.088617f, 0.089337f, 0.090058f,
      0.090778f, 0.091499f, 0.092219f, 0.092939f, 0.093660f, 0.094380f,
      0.095101f, 0.095821f, 0.096542f, 0.097262f, 0.097983f, 0.098703f,
      0.099424f, 0.100144f, 0.100865f, 0.101585f, 0.102305f, 0.103026f,
      0.103746f, 0.104467f, 0.105187f, 0.105908f, 0.106628f, 0.107349f,
      0.108069f, 0.108790f, 0.109510f, 0.110231f, 0.110951f, 0.111671f,
      0.112392f, 0.113112f, 0.113833f, 0.114553f, 0.115274f, 0.115994f,
      0.116715f, 0.117435f, 0.118156f, 0.118876f, 0.119597f, 0.120317f,
      0.121037f, 0.121758f, 0.122478f, 0.123199f, 0.123919f, 0.124640f,
      0.125360f, 0.126081f, 0.126801f, 0.127522f, 0.128242f, 0.128963f,
      0.129683f, 0.130403f, 0.131124f, 0.131844f, 0.132565f, 0.133285f,
      0.134006f, 0.134726f, 0.135447f, 0.136167f, 0.136888f, 0.137608f,
      0.138329f, 0.139049f, 0.139769f, 0.140490f, 0.141210f, 0.141931f,
      0.142651f, 0.143372f, 0.144092f, 0.144813f, 0.145533f, 0.146254f,
      0.146974f, 0.147695f, 0.148415f, 0.149135f, 0.149856f, 0.150576f,
      0.151297f, 0.152017f, 0.152738f, 0.153458f, 0.154179f, 0.154899f,
      0.155620f, 0.156340f, 0.157061f, 0.157781f, 0.158501f, 0.159222f,
      0.159942f, 0.160663f, 0.161383f, 0.162104f, 0.162824f, 0.163545f,
      0.164265f, 0.164986f, 0.165706f, 0.166427f, 0.167147f, 0.167867f,
      0.168588f, 0.169308f, 0.170029f, 0.170749f, 0.171470f, 0.172190f,
      0.172911f, 0.173631f, 0.174352f, 0.175072f, 0.175793f, 0.176513f,
      0.177233f, 0.177954f, 0.178674f, 0.179395f, 0.180115f, 0.180836f,
      0.181556f, 0.182277f, 0.182997f, 0.183718f, 0.184438f, 0.185159f,
      0.185879f, 0.186599f, 0.187320f, 0.188040f, 0.188761f, 0.189481f,
      0.190202f, 0.190922f, 0.191643f, 0.192363f, 0.193084f, 0.193804f,
      0.194524f, 0.195245f, 0.195965f, 0.196686f, 0.197406f, 0.198127f,
      0.198847f, 0.199568f, 0.200288f, 0.201009f, 0.201729f, 0.202450f,
      0.203170f, 0.203890f, 0.204611f, 0.205331f, 0.206052f, 0.206772f,
      0.207493f, 0.208213f, 0.208934f, 0.209654f, 0.210375f, 0.211095f,
      0.211816f, 0.212536f, 0.213256f, 0.213977f, 0.214697f, 0.215418f,
      0.216138f, 0.216859f, 0.217579f, 0.218300f, 0.219020f, 0.219741f,
      0.220461f, 0.221182f, 0.221902f, 0.222622f, 0.223343f, 0.224063f,
      0.224784f, 0.225504f, 0.226225f, 0.226945f, 0.227666f, 0.228386f,
      0.229107f, 0.229827f, 0.230548f, 0.231268f, 0.231988f, 0.232709f,
      0.233429f, 0.234150f, 0.234870f, 0.235591f, 0.236311f, 0.237032f,
      0.237752f, 0.238473f, 0.239193f, 0.239914f, 0.240634f, 0.241354f,
      0.242075f, 0.242795f, 0.243516f, 0.244236f, 0.244957f, 0.245677f,
      0.246398f, 0.247118f, 0.247839f, 0.248559f, 0.249280f, 0.250000f,
      0.250720f, 0.251441f, 0.252161f, 0.252882f, 0.253602f, 0.254323f,
      0.255043f, 0.255764f, 0.256484f, 0.257205f, 0.257925f, 0.258646f,
      0.259366f, 0.260086f, 0.260807f, 0.261527f, 0.262248f, 0.262968f,
      0.263689f, 0.264409f, 0.265130f, 0.265850f, 0.266571f, 0.267291f,
      0.268012f, 0.268732f, 0.269452f, 0.270173f, 0.270893f, 0.271614f,
      0.272334f, 0.273055f, 0.273775f, 0.274496f, 0.275216f, 0.275937f,
      0.276657f, 0.277378f, 0.278098f, 0.278818f, 0.279539f, 0.280259f,
      0.280980f, 0.281700f, 0.282421f, 0.283141f, 0.283862f, 0.284582f,
      0.285303f, 0.286023f, 0.286744f, 0.287464f, 0.288184f, 0.288905f,
      0.289625f, 0.290346f, 0.291066f, 0.291787f, 0.292507f, 0.293228f,
      0.293948f, 0.294669f, 0.295389f, 0.296109f, 0.296830f, 0.297550f,
      0.298271f, 0.298991f, 0.299712f, 0.300432f, 0.301153f, 0.301873f,
      0.302594f, 0.303314f, 0.304035f, 0.304755f, 0.305476f, 0.306196f,
      0.306916f, 0.307637f, 0.308357f, 0.309078f, 0.309798f, 0.310519f,
      0.311239f, 0.311960f, 0.312680f, 0.313401f, 0.314121f, 0.314842f,
      0.315562f, 0.316282f, 0.317003f, 0.317723f, 0.318444f, 0.319164f,
      0.319885f, 0.320605f, 0.321326f, 0.322046f, 0.322767f, 0.323487f,
      0.324207f, 0.324928f, 0.325648f, 0.326369f, 0.327089f, 0.327810f,
      0.328530f, 0.329251f, 0.329971f, 0.330692f, 0.331412f, 0.332133f,
      0.332853f, 0.333573f, 0.334294f, 0.335014f, 0.335735f, 0.336455f,
      0.337176f, 0.337896f, 0.338617f, 0.339337f, 0.340058f, 0.340778f,
      0.341499f, 0.342219f, 0.342939f, 0.343660f, 0.344380f, 0.345101f,
      0.345821f, 0.346542f, 0.347262f, 0.347983f, 0.348703f, 0.349424f,
      0.350144f, 0.350865f, 0.351585f, 0.352305f, 0.353026f, 0.353746f,
      0.354467f, 0.355187f, 0.355908f, 0.356628f, 0.357349f, 0.358069f,
      0.358790f, 0.359510f, 0.360231f, 0.360951f, 0.361671f, 0.362392f,
      0.363112f, 0.363833f, 0.364553f, 0.365274f, 0.365994f, 0.366715f,
      0.367435f, 0.368156f, 0.368876f, 0.369597f, 0.370317f, 0.371037f,
      0.371758f, 0.372478f, 0.373199f, 0.373919f, 0.374640f, 0.375360f,
      0.376081f, 0.376801f, 0.377522f, 0.378242f, 0.378963f, 0.379683f,
      0.380403f, 0.381124f, 0.381844f, 0.382565f, 0.383285f, 0.384006f,
      0.384726f, 0.385447f, 0.386167f, 0.386888f, 0.387608f, 0.388329f,
      0.389049f, 0.389769f, 0.390490f, 0.391210f, 0.391931f, 0.392651f,
      0.393372f, 0.394092f, 0.394813f, 0.395533f, 0.396254f, 0.396974f,
      0.397695f, 0.398415f, 0.399135f, 0.399856f, 0.400576f, 0.401297f,
      0.402017f, 0.402738f, 0.403458f, 0.404179f, 0.404899f, 0.405620f,
      0.406340f, 0.407061f, 0.407781f, 0.408501f, 0.409222f, 0.409942f,
      0.410663f, 0.411383f, 0.412104f, 0.412824f, 0.413545f, 0.414265f,
      0.414986f, 0.415706f, 0.416427f, 0.417147f, 0.417867f, 0.418588f,
      0.419308f, 0.420029f, 0.420749f, 0.421470f, 0.422190f, 0.422911f,
      0.423631f, 0.424352f, 0.425072f, 0.425793f, 0.426513f, 0.427233f,
      0.427954f, 0.428674f, 0.429395f, 0.430115f, 0.430836f, 0.431556f,
      0.432277f, 0.432997f, 0.433718f, 0.434438f, 0.435158f, 0.435879f,
      0.436599f, 0.437320f, 0.438040f, 0.438761f, 0.439481f, 0.440202f,
      0.440922f, 0.441643f, 0.442363f, 0.443084f, 0.443804f, 0.444524f,
      0.445245f, 0.445965f, 0.446686f, 0.447406f, 0.448127f, 0.448847f,
      0.449568f, 0.450288f, 0.451009f, 0.451729f, 0.452450f, 0.453170f,
      0.453891f, 0.454611f, 0.455331f, 0.456052f, 0.456772f, 0.457493f,
      0.458213f, 0.458934f, 0.459654f, 0.460375f, 0.461095f, 0.461816f,
      0.462536f, 0.463256f, 0.463977f, 0.464697f, 0.465418f, 0.466138f,
      0.466859f, 0.467579f, 0.468300f, 0.469020f, 0.469741f, 0.470461f,
      0.471182f, 0.471902f, 0.472622f, 0.473343f, 0.474063f, 0.474784f,
      0.475504f, 0.476225f, 0.476945f, 0.477666f, 0.478386f, 0.479107f,
      0.479827f, 0.480548f, 0.481268f, 0.481988f, 0.482709f, 0.483429f,
      0.484150f, 0.484870f, 0.485591f, 0.486311f, 0.487032f, 0.487752f,
      0.488473f, 0.489193f, 0.489914f, 0.490634f, 0.491354f, 0.492075f,
      0.492795f, 0.493516f, 0.494236f, 0.494957f, 0.495677f, 0.496398f,
      0.497118f, 0.497839f, 0.498559f, 0.499280f, 0.500000f, 0.500720f,
      0.501441f, 0.502161f, 0.502882f, 0.503602f, 0.504323f, 0.505043f,
      0.505764f, 0.506484f, 0.507205f, 0.507925f, 0.508646f, 0.509366f,
      0.510086f, 0.510807f, 0.511527f, 0.512248f, 0.512968f, 0.513689f,
      0.514409f, 0.515130f, 0.515850f, 0.516571f, 0.517291f, 0.518012f,
      0.518732f, 0.519452f, 0.520173f, 0.520893f, 0.521614f, 0.522334f,
      0.523055f, 0.523775f, 0.524496f, 0.525216f, 0.525937f, 0.526657f,
      0.527378f, 0.528098f, 0.528818f, 0.529539f, 0.530259f, 0.530980f,
      0.531700f, 0.532421f, 0.533141f, 0.533862f, 0.534582f, 0.535303f,
      0.536023f, 0.536744f, 0.537464f, 0.538184f, 0.538905f, 0.539625f,
      0.540346f, 0.541066f, 0.541787f, 0.542507f, 0.543228f, 0.543948f,
      0.544669f, 0.545389f, 0.546109f, 0.546830f, 0.547550f, 0.548271f,
      0.548991f, 0.549712f, 0.550432f, 0.551153f, 0.551873f, 0.552594f,
      0.553314f, 0.554035f, 0.554755f, 0.555476f, 0.556196f, 0.556916f,
      0.557637f, 0.558357f, 0.559078f, 0.559798f, 0.560519f, 0.561239f,
      0.561960f, 0.562680f, 0.563401f, 0.564121f, 0.564842f, 0.565562f,
      0.566282f, 0.567003f, 0.567723f, 0.568444f, 0.569164f, 0.569885f,
      0.570605f, 0.571326f, 0.572046f, 0.572767f, 0.573487f, 0.574207f,
      0.574928f, 0.575648f, 0.576369f, 0.577089f, 0.577810f, 0.578530f,
      0.579251f, 0.579971f, 0.580692f, 0.581412f, 0.582133f, 0.582853f,
      0.583573f, 0.584294f, 0.585014f, 0.585735f, 0.586455f, 0.587176f,
      0.587896f, 0.588617f, 0.589337f, 0.590058f, 0.590778f, 0.591499f,
      0.592219f, 0.592939f, 0.593660f, 0.594380f, 0.595101f, 0.595821f,
      0.596542f, 0.597262f, 0.597983f, 0.598703f, 0.599424f, 0.600144f,
      0.600865f, 0.601585f, 0.602305f, 0.603026f, 0.603746f, 0.604467f,
      0.605187f, 0.605908f, 0.606628f, 0.607349f, 0.608069f, 0.608790f,
      0.609510f, 0.610231f, 0.610951f, 0.611671f, 0.612392f, 0.613112f,
      0.613833f, 0.614553f, 0.615274f, 0.615994f, 0.616715f, 0.617435f,
      0.618156f, 0.618876f, 0.619597f, 0.620317f, 0.621037f, 0.621758f,
      0.622478f, 0.623199f, 0.623919f, 0.624640f, 0.625360f, 0.626081f,
      0.626801f, 0.627522f, 0.628242f, 0.628963f, 0.629683f, 0.630403f,
      0.631124f, 0.631844f, 0.632565f, 0.633285f, 0.634006f, 0.634726f,
      0.635447f, 0.636167f, 0.636888f, 0.637608f, 0.638329f, 0.639049f,
      0.639769f, 0.640490f, 0.641210f, 0.641931f, 0.642651f, 0.643372f,
      0.644092f, 0.644813f, 0.645533f, 0.646254f, 0.646974f, 0.647695f,
      0.648415f, 0.649135f, 0.649856f, 0.650576f, 0.651297f, 0.652017f,
      0.652738f, 0.653458f, 0.654179f, 0.654899f, 0.655620f, 0.656340f,
      0.657061f, 0.657781f, 0.658501f, 0.659222f, 0.659942f, 0.660663f,
      0.661383f, 0.662104f, 0.662824f, 0.663545f, 0.664265f, 0.664986f,
      0.665706f, 0.666427f, 0.667147f, 0.667867f, 0.668588f, 0.669308f,
      0.670029f, 0.670749f, 0.671470f, 0.672190f, 0.672911f, 0.673631f,
      0.674352f, 0.675072f, 0.675793f, 0.676513f, 0.677233f, 0.677954f,
      0.678674f, 0.679395f, 0.680115f, 0.680836f, 0.681556f, 0.682277f,
      0.682997f, 0.683718f, 0.684438f, 0.685158f, 0.685879f, 0.686599f,
      0.687320f, 0.688040f, 0.688761f, 0.689481f, 0.690202f, 0.690922f,
      0.691643f, 0.692363f, 0.693084f, 0.693804f, 0.694524f, 0.695245f,
      0.695965f, 0.696686f, 0.697406f, 0.698127f, 0.698847f, 0.699568f,
      0.700288f, 0.701009f, 0.701729f, 0.702450f, 0.703170f, 0.703891f,
      0.704611f, 0.705331f, 0.706052f, 0.706772f, 0.707493f, 0.708213f,
      0.708934f, 0.709654f, 0.710375f, 0.711095f, 0.711816f, 0.712536f,
      0.713256f, 0.713977f, 0.714697f, 0.715418f, 0.716138f, 0.716859f,
      0.717579f, 0.718300f, 0.719020f, 0.719741f, 0.720461f, 0.721182f,
      0.721902f, 0.722622f, 0.723343f, 0.724063f, 0.724784f, 0.725504f,
      0.726225f, 0.726945f, 0.727666f, 0.728386f, 0.729107f, 0.729827f,
      0.730548f, 0.731268f, 0.731988f, 0.732709f, 0.733429f, 0.734150f,
      0.734870f, 0.735591f, 0.736311f, 0.737032f, 0.737752f, 0.738473f,
      0.739193f, 0.739914f, 0.740634f, 0.741354f, 0.742075f, 0.742795f,
      0.743516f, 0.744236f, 0.744957f, 0.745677f, 0.746398f, 0.747118f,
      0.747839f, 0.748559f, 0.749280f, 0.750000f, 0.750720f, 0.751441f,
      0.752161f, 0.752882f, 0.753602f, 0.754323f, 0.755043f, 0.755764f,
      0.756484f, 0.757205f, 0.757925f, 0.758646f, 0.759366f, 0.760086f,
      0.760807f, 0.761527f, 0.762248f, 0.762968f, 0.763689f, 0.764409f,
      0.765130f, 0.765850f, 0.766571f, 0.767291f, 0.768012f, 0.768732f,
      0.769452f, 0.770173f, 0.770893f, 0.771614f, 0.772334f, 0.773055f,
      0.773775f, 0.774496f, 0.775216f, 0.775937f, 0.776657f, 0.777378f,
      0.778098f, 0.778818f, 0.779539f, 0.780259f, 0.780980f, 0.781700f,
      0.782421f, 0.783141f, 0.783862f, 0.784582f, 0.785303f, 0.786023f,
      0.786744f, 0.787464f, 0.788184f, 0.788905f, 0.789625f, 0.790346f,
      0.791066f, 0.791787f, 0.792507f, 0.793228f, 0.793948f, 0.794669f,
      0.795389f, 0.796109f, 0.796830f, 0.797550f, 0.798271f, 0.798991f,
      0.799712f, 0.800432f, 0.801153f, 0.801873f, 0.802594f, 0.803314f,
      0.804035f, 0.804755f, 0.805476f, 0.806196f, 0.806916f, 0.807637f,
      0.808357f, 0.809078f, 0.809798f, 0.810519f, 0.811239f, 0.811960f,
      0.812680f, 0.813401f, 0.814121f, 0.814842f, 0.815562f, 0.816282f,
      0.817003f, 0.817723f, 0.818444f, 0.819164f, 0.819885f, 0.820605f,
      0.821326f, 0.822046f, 0.822767f, 0.823487f, 0.824207f, 0.824928f,
      0.825648f, 0.826369f, 0.827089f, 0.827810f, 0.828530f, 0.829251f,
      0.829971f, 0.830692f, 0.831412f, 0.832133f, 0.832853f, 0.833573f,
      0.834294f, 0.835014f, 0.835735f, 0.836455f, 0.837176f, 0.837896f,
      0.838617f, 0.839337f, 0.840058f, 0.840778f, 0.841499f, 0.842219f,
      0.842939f, 0.843660f, 0.844380f, 0.845101f, 0.845821f, 0.846542f,
      0.847262f, 0.847983f, 0.848703f, 0.849424f, 0.850144f, 0.850865f,
      0.851585f, 0.852305f, 0.853026f, 0.853746f, 0.854467f, 0.855187f,
      0.855908f, 0.856628f, 0.857349f, 0.858069f, 0.858790f, 0.859510f,
      0.860231f, 0.860951f, 0.861671f, 0.862392f, 0.863112f, 0.863833f,
      0.864553f, 0.865274f, 0.865994f, 0.866715f, 0.867435f, 0.868156f,
      0.868876f, 0.869597f, 0.870317f, 0.871037f, 0.871758f, 0.872478f,
      0.873199f, 0.873919f, 0.874640f, 0.875360f, 0.876081f, 0.876801f,
      0.877522f, 0.878242f, 0.878963f, 0.879683f, 0.880403f, 0.881124f,
      0.881844f, 0.882565f, 0.883285f, 0.884006f, 0.884726f, 0.885447f,
      0.886167f, 0.886888f, 0.887608f, 0.888329f, 0.889049f, 0.889769f,
      0.890490f, 0.891210f, 0.891931f, 0.892651f, 0.893372f, 0.894092f,
      0.894813f, 0.895533f, 0.896254f, 0.896974f, 0.897695f, 0.898415f,
      0.899135f, 0.899856f, 0.900576f, 0.901297f, 0.902017f, 0.902738f,
      0.903458f, 0.904179f, 0.904899f, 0.905620f, 0.906340f, 0.907061f,
      0.907781f, 0.908501f, 0.909222f, 0.909942f, 0.910663f, 0.911383f,
      0.912104f, 0.912824f, 0.913545f, 0.914265f, 0.914986f, 0.915706f,
      0.916427f, 0.917147f, 0.917867f, 0.918588f, 0.919308f, 0.920029f,
      0.920749f, 0.921470f, 0.922190f, 0.922911f, 0.923631f, 0.924352f,
      0.925072f, 0.925793f, 0.926513f, 0.927233f, 0.927954f, 0.928674f,
      0.929395f, 0.930115f, 0.930836f, 0.931556f, 0.932277f, 0.932997f,
      0.933718f, 0.934438f, 0.935158f, 0.935879f, 0.936599f, 0.937320f,
      0.938040f, 0.938761f, 0.939481f, 0.940202f, 0.940922f, 0.941643f,
      0.942363f, 0.943084f, 0.943804f, 0.944524f, 0.945245f, 0.945965f,
      0.946686f, 0.947406f, 0.948127f, 0.948847f, 0.949568f, 0.950288f,
      0.951009f, 0.951729f, 0.952450f, 0.953170f, 0.953891f, 0.954611f,
      0.955331f, 0.956052f, 0.956772f, 0.957493f, 0.958213f, 0.958934f,
      0.959654f, 0.960375f, 0.961095f, 0.961816f, 0.962536f, 0.963256f,
      0.963977f, 0.964697f, 0.965418f, 0.966138f, 0.966859f, 0.967579f,
      0.968300f, 0.969020f, 0.969741f, 0.970461f, 0.971182f, 0.971902f,
      0.972622f, 0.973343f, 0.974063f, 0.974784f, 0.975504f, 0.976225f,
      0.976945f, 0.977666f, 0.978386f, 0.979107f, 0.979827f, 0.980548f,
      0.981268f, 0.981988f, 0.982709f, 0.983429f, 0.984150f, 0.984870f,
      0.985591f, 0.986311f, 0.987032f, 0.987752f, 0.988473f, 0.989193f,
      0.989914f, 0.990634f, 0.991354f, 0.992075f, 0.992795f, 0.993516f,
      0.994236f, 0.994957f, 0.995677f, 0.996398f, 0.997118f, 0.997839f,
      0.998559f, 0.999280f, 1.000000f
    };

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

  TransformPacket
    *y_map,
    *x_map,
    *z_map;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  progress=0;
  switch (image->colorspace)
  {
    case CMYKColorspace:
    {
      PixelInfo
        zero;

      /*
        Transform image from CMYK to sRGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      GetPixelInfo(image,&zero);
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        PixelInfo
          pixel;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixel=zero;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          GetPixelInfoPixel(image,q,&pixel);
          ConvertCMYKToRGB(&pixel);
          SetPixelViaPixelInfo(image,&pixel,q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,sRGBColorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case LinearGRAYColorspace:
    case GRAYColorspace:
    {
      /*
        Transform linear GRAY to sRGB colorspace.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      if (SetImageColorspace(image,sRGBColorspace,exception) == MagickFalse)
        return(MagickFalse);
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=(ssize_t) image->columns; x != 0; x--)
        {
          MagickRealType
            gray;

          gray=(MagickRealType) GetPixelGray(image,q);
          if ((image->intensity == Rec601LuminancePixelIntensityMethod) ||
              (image->intensity == Rec709LuminancePixelIntensityMethod))
            gray=EncodePixelGamma(gray);
          SetPixelRed(image,ClampToQuantum(gray),q);
          SetPixelGreen(image,ClampToQuantum(gray),q);
          SetPixelBlue(image,ClampToQuantum(gray),q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,sRGBColorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case CMYColorspace:
    case HCLColorspace:
    case HCLpColorspace:
    case HSBColorspace:
    case HSIColorspace:
    case HSLColorspace:
    case HSVColorspace:
    case HWBColorspace:
    case LabColorspace:
    case LCHColorspace:
    case LCHabColorspace:
    case LCHuvColorspace:
    case LMSColorspace:
    case LuvColorspace:
    case xyYColorspace:
    case XYZColorspace:
    case YCbCrColorspace:
    case YDbDrColorspace:
    case YIQColorspace:
    case YPbPrColorspace:
    case YUVColorspace:
    {
      /*
        Transform image from source colorspace to sRGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          double
            blue,
            green,
            red,
            X,
            Y,
            Z;

          X=QuantumScale*GetPixelRed(image,q);
          Y=QuantumScale*GetPixelGreen(image,q);
          Z=QuantumScale*GetPixelBlue(image,q);
          switch (image->colorspace)
          {
            case CMYColorspace:
            {
              ConvertCMYToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case HCLColorspace:
            {
              ConvertHCLToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case HCLpColorspace:
            {
              ConvertHCLpToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case HSBColorspace:
            {
              ConvertHSBToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case HSIColorspace:
            {
              ConvertHSIToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case HSLColorspace:
            {
              ConvertHSLToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case HSVColorspace:
            {
              ConvertHSVToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case HWBColorspace:
            {
              ConvertHWBToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case LabColorspace:
            {
              ConvertLabToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case LCHColorspace:
            case LCHabColorspace:
            {
              ConvertLCHabToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case LCHuvColorspace:
            {
              ConvertLCHuvToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case LMSColorspace:
            {
              ConvertLMSToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case LuvColorspace:
            {
              ConvertLuvToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case xyYColorspace:
            {
              ConvertxyYToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case XYZColorspace:
            {
              ConvertXYZToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case YCbCrColorspace:
            {
              ConvertYCbCrToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case YDbDrColorspace:
            {
              ConvertYDbDrToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case YIQColorspace:
            {
              ConvertYIQToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case YPbPrColorspace:
            {
              ConvertYPbPrToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            case YUVColorspace:
            {
              ConvertYUVToRGB(X,Y,Z,&red,&green,&blue);
              break;
            }
            default:
            {
              red=QuantumRange*X;
              green=QuantumRange*Y;
              blue=QuantumRange*Z;
              break;
            }
          }
          SetPixelRed(image,ClampToQuantum(red),q);
          SetPixelGreen(image,ClampToQuantum(green),q);
          SetPixelBlue(image,ClampToQuantum(blue),q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,sRGBColorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case LogColorspace:
    {
      const char
        *value;

      double
        black,
        density,
        film_gamma,
        gamma,
        reference_black,
        reference_white;

      Quantum
        *logmap;

      /*
        Transform Log to sRGB colorspace.
      */
      density=DisplayGamma;
      gamma=DisplayGamma;
      value=GetImageProperty(image,"gamma",exception);
      if (value != (const char *) NULL)
        gamma=PerceptibleReciprocal(StringToDouble(value,(char **) NULL));
      film_gamma=FilmGamma;
      value=GetImageProperty(image,"film-gamma",exception);
      if (value != (const char *) NULL)
        film_gamma=StringToDouble(value,(char **) NULL);
      reference_black=ReferenceBlack;
      value=GetImageProperty(image,"reference-black",exception);
      if (value != (const char *) NULL)
        reference_black=StringToDouble(value,(char **) NULL);
      reference_white=ReferenceWhite;
      value=GetImageProperty(image,"reference-white",exception);
      if (value != (const char *) NULL)
        reference_white=StringToDouble(value,(char **) NULL);
      logmap=(Quantum *) AcquireQuantumMemory((size_t) MaxMap+1UL,
        sizeof(*logmap));
      if (logmap == (Quantum *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      black=pow(10.0,(reference_black-reference_white)*(gamma/density)*0.002/
        film_gamma);
      for (i=0; i <= (ssize_t) (reference_black*MaxMap/1024.0); i++)
        logmap[i]=(Quantum) 0;
      for ( ; i < (ssize_t) (reference_white*MaxMap/1024.0); i++)
        logmap[i]=ClampToQuantum(QuantumRange/(1.0-black)*
          (pow(10.0,(1024.0*i/MaxMap-reference_white)*(gamma/density)*0.002/
          film_gamma)-black));
      for ( ; i <= (ssize_t) MaxMap; i++)
        logmap[i]=QuantumRange;
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=(ssize_t) image->columns; x != 0; x--)
        {
          double
            blue,
            green,
            red;

          red=(double) logmap[ScaleQuantumToMap(GetPixelRed(image,q))];
          green=(double) logmap[ScaleQuantumToMap(GetPixelGreen(image,q))];
          blue=(double) logmap[ScaleQuantumToMap(GetPixelBlue(image,q))];
          SetPixelRed(image,ClampToQuantum(EncodePixelGamma((MagickRealType)
            red)),q);
          SetPixelGreen(image,ClampToQuantum(EncodePixelGamma((MagickRealType)
            green)),q);
          SetPixelBlue(image,ClampToQuantum(EncodePixelGamma((MagickRealType)
            blue)),q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      logmap=(Quantum *) RelinquishMagickMemory(logmap);
      if (SetImageColorspace(image,sRGBColorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case RGBColorspace:
    case scRGBColorspace:
    {
      /*
        Transform linear RGB to sRGB colorspace.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image,exception) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=(ssize_t) image->columns; x != 0; x--)
        {
          double
            blue,
            green,
            red;

          red=EncodePixelGamma((MagickRealType) GetPixelRed(image,q));
          green=EncodePixelGamma((MagickRealType) GetPixelGreen(image,q));
          blue=EncodePixelGamma((MagickRealType) GetPixelBlue(image,q));
          SetPixelRed(image,ClampToQuantum(red),q);
          SetPixelGreen(image,ClampToQuantum(green),q);
          SetPixelBlue(image,ClampToQuantum(blue),q);
          q+=GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,sRGBColorspace,exception) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    default:
      break;
  }
  /*
    Allocate the tables.
  */
  x_map=(TransformPacket *) AcquireQuantumMemory((size_t) MaxMap+1UL,
    sizeof(*x_map));
  y_map=(TransformPacket *) AcquireQuantumMemory((size_t) MaxMap+1UL,
    sizeof(*y_map));
  z_map=(TransformPacket *) AcquireQuantumMemory((size_t) MaxMap+1UL,
    sizeof(*z_map));
  if ((x_map == (TransformPacket *) NULL) ||
      (y_map == (TransformPacket *) NULL) ||
      (z_map == (TransformPacket *) NULL))
    {
      if (z_map != (TransformPacket *) NULL)
        z_map=(TransformPacket *) RelinquishMagickMemory(z_map);
      if (y_map != (TransformPacket *) NULL)
        y_map=(TransformPacket *) RelinquishMagickMemory(y_map);
      if (x_map != (TransformPacket *) NULL)
        x_map=(TransformPacket *) RelinquishMagickMemory(x_map);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  switch (image->colorspace)
  {
    case OHTAColorspace:
    {
      /*
        Initialize OHTA tables:

          I1 = 0.33333*R+0.33334*G+0.33333*B
          I2 = 0.50000*R+0.00000*G-0.50000*B
          I3 =-0.25000*R+0.50000*G-0.25000*B
          R = I1+1.00000*I2-0.66668*I3
          G = I1+0.00000*I2+1.33333*I3
          B = I1-1.00000*I2-0.66668*I3

        I and Q, normally -0.5 through 0.5, must be normalized to the range 0
        through QuantumRange.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (1.0*(double) i);
        y_map[i].x=(MagickRealType) (0.5*1.00000*(2.0*(double) i-MaxMap));
        z_map[i].x=(MagickRealType) (-0.5*0.66668*(2.0*(double) i-MaxMap));
        x_map[i].y=(MagickRealType) (1.0*(double) i);
        y_map[i].y=(MagickRealType) (0.5*0.00000*(2.0*(double) i-MaxMap));
        z_map[i].y=(MagickRealType) (0.5*1.33333*(2.0*(double) i-MaxMap));
        x_map[i].z=(MagickRealType) (1.0*(double) i);
        y_map[i].z=(MagickRealType) (-0.5*1.00000*(2.0*(double) i-MaxMap));
        z_map[i].z=(MagickRealType) (-0.5*0.66668*(2.0*(double) i-MaxMap));
      }
      break;
    }
    case Rec601YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables:

          R = Y            +1.402000*Cr
          G = Y-0.344136*Cb-0.714136*Cr
          B = Y+1.772000*Cb

        Cb and Cr, normally -0.5 through 0.5, must be normalized to the range 0
        through QuantumRange.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=0.99999999999914679361*(double) i;
        y_map[i].x=0.5*(-1.2188941887145875e-06)*(2.00*(double) i-MaxMap);
        z_map[i].x=0.5*1.4019995886561440468*(2.00*(double) i-MaxMap);
        x_map[i].y=0.99999975910502514331*(double) i;
        y_map[i].y=0.5*(-0.34413567816504303521)*(2.00*(double) i-MaxMap);
        z_map[i].y=0.5*(-0.71413649331646789076)*(2.00*(double) i-MaxMap);
        x_map[i].z=1.00000124040004623180*(double) i;
        y_map[i].z=0.5*1.77200006607230409200*(2.00*(double) i-MaxMap);
        z_map[i].z=0.5*2.1453384174593273e-06*(2.00*(double) i-MaxMap);
      }
      break;
    }
    case Rec709YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables:

          R = Y            +1.574800*Cr
          G = Y-0.187324*Cb-0.468124*Cr
          B = Y+1.855600*Cb

        Cb and Cr, normally -0.5 through 0.5, must be normalized to the range 0
        through QuantumRange.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (1.0*i);
        y_map[i].x=(MagickRealType) (0.5*0.000000*(2.0*i-MaxMap));
        z_map[i].x=(MagickRealType) (0.5*1.574800*(2.0*i-MaxMap));
        x_map[i].y=(MagickRealType) (1.0*i);
        y_map[i].y=(MagickRealType) (0.5*(-0.187324)*(2.0*i-MaxMap));
        z_map[i].y=(MagickRealType) (0.5*(-0.468124)*(2.0*i-MaxMap));
        x_map[i].z=(MagickRealType) (1.0*i);
        y_map[i].z=(MagickRealType) (0.5*1.855600*(2.0*i-MaxMap));
        z_map[i].z=(MagickRealType) (0.5*0.000000*(2.0*i-MaxMap));
      }
      break;
    }
    case YCCColorspace:
    {
      /*
        Initialize YCC tables:

          R = Y            +1.340762*C2
          G = Y-0.317038*C1-0.682243*C2
          B = Y+1.632639*C1

        YCC is scaled by 1.3584.  C1 zero is 156 and C2 is at 137.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (1.3584000*(double) i);
        y_map[i].x=(MagickRealType) 0.0000000;
        z_map[i].x=(MagickRealType) (1.8215000*(1.0*(double) i-(double)
          ScaleQuantumToMap(ScaleCharToQuantum(137))));
        x_map[i].y=(MagickRealType) (1.3584000*(double) i);
        y_map[i].y=(MagickRealType) (-0.4302726*(1.0*(double) i-(double)
          ScaleQuantumToMap(ScaleCharToQuantum(156))));
        z_map[i].y=(MagickRealType) (-0.9271435*(1.0*(double) i-(double)
          ScaleQuantumToMap(ScaleCharToQuantum(137))));
        x_map[i].z=(MagickRealType) (1.3584000*(double) i);
        y_map[i].z=(MagickRealType) (2.2179000*(1.0*(double) i-(double)
          ScaleQuantumToMap(ScaleCharToQuantum(156))));
        z_map[i].z=(MagickRealType) 0.0000000;
      }
      break;
    }
    default:
    {
      /*
        Linear conversion tables.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) (1.0*(double) i);
        y_map[i].x=(MagickRealType) 0.0;
        z_map[i].x=(MagickRealType) 0.0;
        x_map[i].y=(MagickRealType) 0.0;
        y_map[i].y=(MagickRealType) (1.0*(double) i);
        z_map[i].y=(MagickRealType) 0.0;
        x_map[i].z=(MagickRealType) 0.0;
        y_map[i].z=(MagickRealType) 0.0;
        z_map[i].z=(MagickRealType) (1.0*(double) i);
      }
      break;
    }
  }
  /*
    Convert to sRGB.
  */
  switch (image->storage_class)
  {
    case DirectClass:
    default:
    {
      /*
        Convert DirectClass image.
      */
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        PixelInfo
          pixel;

        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register size_t
            blue,
            green,
            red;

          red=ScaleQuantumToMap(GetPixelRed(image,q));
          green=ScaleQuantumToMap(GetPixelGreen(image,q));
          blue=ScaleQuantumToMap(GetPixelBlue(image,q));
          pixel.red=x_map[red].x+y_map[green].x+z_map[blue].x;
          pixel.green=x_map[red].y+y_map[green].y+z_map[blue].y;
          pixel.blue=x_map[red].z+y_map[green].z+z_map[blue].z;
          if (image->colorspace == YCCColorspace)
            {
              pixel.red=QuantumRange*YCCMap[RoundToYCC(1024.0*pixel.red/
                (double) MaxMap)];
              pixel.green=QuantumRange*YCCMap[RoundToYCC(1024.0*pixel.green/
                (double) MaxMap)];
              pixel.blue=QuantumRange*YCCMap[RoundToYCC(1024.0*pixel.blue/
                (double) MaxMap)];
            }
          else
            {
              pixel.red=(MagickRealType) ScaleMapToQuantum(pixel.red);
              pixel.green=(MagickRealType) ScaleMapToQuantum(pixel.green);
              pixel.blue=(MagickRealType) ScaleMapToQuantum(pixel.blue);
            }
          SetPixelRed(image,ClampToQuantum(pixel.red),q);
          SetPixelGreen(image,ClampToQuantum(pixel.green),q);
          SetPixelBlue(image,ClampToQuantum(pixel.blue),q);
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
            #pragma omp critical (MagickCore_TransformsRGBImage)
#endif
            proceed=SetImageProgress(image,TransformsRGBImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      image_view=DestroyCacheView(image_view);
      break;
    }
    case PseudoClass:
    {
      /*
        Convert PseudoClass image.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        PixelInfo
          pixel;

        register size_t
          blue,
          green,
          red;

        red=ScaleQuantumToMap(ClampToQuantum(image->colormap[i].red));
        green=ScaleQuantumToMap(ClampToQuantum(image->colormap[i].green));
        blue=ScaleQuantumToMap(ClampToQuantum(image->colormap[i].blue));
        pixel.red=x_map[red].x+y_map[green].x+z_map[blue].x;
        pixel.green=x_map[red].y+y_map[green].y+z_map[blue].y;
        pixel.blue=x_map[red].z+y_map[green].z+z_map[blue].z;
        if (image->colorspace == YCCColorspace)
          {
            pixel.red=QuantumRange*YCCMap[RoundToYCC(1024.0*pixel.red/
              (double) MaxMap)];
            pixel.green=QuantumRange*YCCMap[RoundToYCC(1024.0*pixel.green/
              (double) MaxMap)];
            pixel.blue=QuantumRange*YCCMap[RoundToYCC(1024.0*pixel.blue/
              (double) MaxMap)];
          }
        else
          {
            pixel.red=(MagickRealType) ScaleMapToQuantum(pixel.red);
            pixel.green=(MagickRealType) ScaleMapToQuantum(pixel.green);
            pixel.blue=(MagickRealType) ScaleMapToQuantum(pixel.blue);
          }
        image->colormap[i].red=(double) ClampToQuantum(pixel.red);
        image->colormap[i].green=(double) ClampToQuantum(pixel.green);
        image->colormap[i].blue=(double) ClampToQuantum(pixel.blue);
      }
      (void) SyncImage(image,exception);
      break;
    }
  }
  /*
    Relinquish resources.
  */
  z_map=(TransformPacket *) RelinquishMagickMemory(z_map);
  y_map=(TransformPacket *) RelinquishMagickMemory(y_map);
  x_map=(TransformPacket *) RelinquishMagickMemory(x_map);
  if (SetImageColorspace(image,sRGBColorspace,exception) == MagickFalse)
    return(MagickFalse);
  return(MagickTrue);
}

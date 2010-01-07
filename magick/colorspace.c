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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/property.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/gem.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/utility.h"

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
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     R G B T r a n s f o r m I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RGBTransformImage() converts the reference image from RGB to an alternate
%  colorspace.  The transformation matrices are not the standard ones: the
%  weights are rescaled to normalized the range of the transformed values to
%  be [0..QuantumRange].
%
%  The format of the RGBTransformImage method is:
%
%      MagickBooleanType RGBTransformImage(Image *image,
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colorspace: the colorspace to transform the image to.
%
*/

static inline void ConvertRGBToXYZ(const Quantum red,const Quantum green,
  const Quantum blue,double *X,double *Y,double *Z)
{
  double
    b,
    g,
    r;

  assert(X != (double *) NULL);
  assert(Y != (double *) NULL);
  assert(Z != (double *) NULL);
  r=QuantumScale*red;
  g=QuantumScale*green;
  b=QuantumScale*blue;
  *X=0.4124240*r+0.3575790*g+0.1804640*b;
  *Y=0.2126560*r+0.7151580*g+0.0721856*b;
  *Z=0.0193324*r+0.1191930*g+0.9504440*b;
}

static inline void ConvertXYZToLab(const double X,const double Y,const double Z,
  double *L,double *a,double *b)
{
  double
    x,
    y,
    z;

  assert(L != (double *) NULL);
  assert(a != (double *) NULL);
  assert(b != (double *) NULL);
  x=X/0.9504559271;
  if (x > (216/24389.0))
    x=pow(x,1.0/3.0);
  else
    x=(7.787*x)+(16.0/116.0);
  y=Y/1.00000;
  if (y > (216/24389.0))
    y=pow(y,1.0/3.0);
  else
    y=(7.787*y)+(16.0/116.0);
  z=Z/1.0890577508;
  if (z > (216/24389.0))
    z=pow(z,1.0/3.0);
  else
    z=(7.787*z)+(16.0/116.0);
  *L=0.5*((1.160*y)-0.160+1.0);
  *a=0.5*(5.000*(x-y)+1.0);
  *b=0.5*(2.000*(y-z)+1.0);
}

MagickExport MagickBooleanType RGBTransformImage(Image *image,
  const ColorspaceType colorspace)
{
#define RGBTransformImageTag  "RGBTransform/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status,
    sync;

  PrimaryInfo
    primary_info;

  register long
    i;

  TransformPacket
    *x_map,
    *y_map,
    *z_map;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(colorspace != RGBColorspace);
  assert(colorspace != TransparentColorspace);
  assert(colorspace != UndefinedColorspace);
  switch (image->colorspace)
  {
    case GRAYColorspace:
    case Rec601LumaColorspace:
    case Rec709LumaColorspace:
    case RGBColorspace:
    case TransparentColorspace:
      break;
    default:
    {
      (void) TransformImageColorspace(image,image->colorspace);
      break;
    }
  }
  if (SetImageColorspace(image,colorspace) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  switch (colorspace)
  {
    case CMYColorspace:
    {
      /*
        Convert RGB to CMY colorspace.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum((MagickRealType) (QuantumRange-q->red));
          q->green=ClampToQuantum((MagickRealType) (QuantumRange-q->green));
          q->blue=ClampToQuantum((MagickRealType) (QuantumRange-q->blue));
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      image->type=image->matte == MagickFalse ? ColorSeparationType :
        ColorSeparationMatteType;
      return(status);
    }
    case CMYKColorspace:
    {
      MagickPixelPacket
        zero;

      /*
        Convert RGB to CMYK colorspace.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      GetMagickPixelPacket(image,&zero);
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickPixelPacket
          pixel;

        register IndexPacket
          *restrict indexes;

        register long
          x;

        register PixelPacket
          *restrict q;

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
        pixel=zero;
        for (x=0; x < (long) image->columns; x++)
        {
          SetMagickPixelPacket(image,q,indexes+x,&pixel);
          ConvertRGBToCMYK(&pixel);
          SetPixelPacket(image,&pixel,q,indexes+x);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      image->type=image->matte == MagickFalse ? ColorSeparationType :
        ColorSeparationMatteType;
      return(status);
    }
    case HSBColorspace:
    {
      /*
        Transform image from RGB to HSB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          brightness,
          hue,
          saturation;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        hue=0.0;
        saturation=0.0;
        brightness=0.0;
        for (x=0; x < (long) image->columns; x++)
        {
          ConvertRGBToHSB(q->red,q->green,q->blue,&hue,&saturation,&brightness);
          q->red=ClampToQuantum((MagickRealType) QuantumRange*hue);
          q->green=ClampToQuantum((MagickRealType) QuantumRange*saturation);
          q->blue=ClampToQuantum((MagickRealType) QuantumRange*brightness);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case HSLColorspace:
    {
      /*
        Transform image from RGB to HSL.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          hue,
          lightness,
          saturation;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        hue=0.0;
        saturation=0.0;
        lightness=0.0;
        for (x=0; x < (long) image->columns; x++)
        {
          ConvertRGBToHSL(q->red,q->green,q->blue,&hue,&saturation,&lightness);
          q->red=ClampToQuantum((MagickRealType) QuantumRange*hue);
          q->green=ClampToQuantum((MagickRealType) QuantumRange*saturation);
          q->blue=ClampToQuantum((MagickRealType) QuantumRange*lightness);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case HWBColorspace:
    {
      /*
        Transform image from RGB to HWB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          blackness,
          hue,
          whiteness;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        hue=0.0;
        whiteness=0.0;
        blackness=0.0;
        for (x=0; x < (long) image->columns; x++)
        {
          ConvertRGBToHWB(q->red,q->green,q->blue,&hue,&whiteness,&blackness);
          q->red=ClampToQuantum((MagickRealType) QuantumRange*hue);
          q->green=ClampToQuantum((MagickRealType) QuantumRange*whiteness);
          q->blue=ClampToQuantum((MagickRealType) QuantumRange*blackness);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case LabColorspace:
    {
      /*
        Transform image from RGB to Lab.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          a,
          b,
          L,
          X,
          Y,
          Z;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        L=0.0;
        a=0.0;
        b=0.0;
        X=0.0;
        Y=0.0;
        Z=0.0;
        for (x=0; x < (long) image->columns; x++)
        {
          ConvertRGBToXYZ(q->red,q->green,q->blue,&X,&Y,&Z);
          ConvertXYZToLab(X,Y,Z,&L,&a,&b);
          q->red=ClampToQuantum((MagickRealType) QuantumRange*L);
          q->green=ClampToQuantum((MagickRealType) QuantumRange*a);
          q->blue=ClampToQuantum((MagickRealType) QuantumRange*b);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case LogColorspace:
    {
#define ReferenceBlack  95.0
#define ReferenceWhite  685.0
#define DisplayGamma  (1.0/1.7)

      const char
        *value;

      double
        black,
        density,
        gamma,
        reference_black,
        reference_white;

      Quantum
        *logmap;

      /*
        Transform RGB to Log colorspace.
      */
      density=2.03728;
      gamma=DisplayGamma;
      value=GetImageProperty(image,"gamma");
      if (value != (const char *) NULL)
        gamma=1.0/StringToDouble(value) != 0.0 ? StringToDouble(value) : 1.0;
      reference_black=ReferenceBlack;
      value=GetImageProperty(image,"reference-black");
      if (value != (const char *) NULL)
        reference_black=StringToDouble(value);
      reference_white=ReferenceWhite;
      value=GetImageProperty(image,"reference-white");
      if (value != (const char *) NULL)
        reference_white=StringToDouble(value);
      logmap=(Quantum *) AcquireQuantumMemory((size_t) MaxMap+1UL,
        sizeof(*logmap));
      if (logmap == (Quantum *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      black=pow(10.0,(reference_black-reference_white)*(gamma/density)*
        0.002/0.6);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
        logmap[i]=ScaleMapToQuantum((MagickRealType) (MaxMap*(reference_white+
          log10(black+((MagickRealType) i/MaxMap)*(1.0-black))/((gamma/density)*
          0.002/0.6))/1024.0+0.5));
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=(long) image->columns; x != 0; x--)
        {
          q->red=logmap[ScaleQuantumToMap(q->red)];
          q->green=logmap[ScaleQuantumToMap(q->green)];
          q->blue=logmap[ScaleQuantumToMap(q->blue)];
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      logmap=(Quantum *) RelinquishMagickMemory(logmap);
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
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
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
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.33333f*(MagickRealType) i;
        y_map[i].x=0.33334f*(MagickRealType) i;
        z_map[i].x=0.33333f*(MagickRealType) i;
        x_map[i].y=0.50000f*(MagickRealType) i;
        y_map[i].y=0.00000f*(MagickRealType) i;
        z_map[i].y=(-0.50000f)*(MagickRealType) i;
        x_map[i].z=(-0.25000f)*(MagickRealType) i;
        y_map[i].z=0.50000f*(MagickRealType) i;
        z_map[i].z=(-0.25000f)*(MagickRealType) i;
      }
      break;
    }
    case Rec601LumaColorspace:
    case GRAYColorspace:
    {
      /*
        Initialize Rec601 luma tables:

          G = 0.29900*R+0.58700*G+0.11400*B
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.29900f*(MagickRealType) i;
        y_map[i].x=0.58700f*(MagickRealType) i;
        z_map[i].x=0.11400f*(MagickRealType) i;
        x_map[i].y=0.29900f*(MagickRealType) i;
        y_map[i].y=0.58700f*(MagickRealType) i;
        z_map[i].y=0.11400f*(MagickRealType) i;
        x_map[i].z=0.29900f*(MagickRealType) i;
        y_map[i].z=0.58700f*(MagickRealType) i;
        z_map[i].z=0.11400f*(MagickRealType) i;
      }
      image->type=GrayscaleType;
      break;
    }
    case Rec601YCbCrColorspace:
    case YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables (ITU-R BT.601):

          Y =  0.299000*R+0.587000*G+0.114000*B
          Cb= -0.168736*R-0.331264*G+0.500000*B
          Cr=  0.500000*R-0.418688*G-0.081312*B

        Cb and Cr, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.299000f*(MagickRealType) i;
        y_map[i].x=0.587000f*(MagickRealType) i;
        z_map[i].x=0.114000f*(MagickRealType) i;
        x_map[i].y=(-0.168730f)*(MagickRealType) i;
        y_map[i].y=(-0.331264f)*(MagickRealType) i;
        z_map[i].y=0.500000f*(MagickRealType) i;
        x_map[i].z=0.500000f*(MagickRealType) i;
        y_map[i].z=(-0.418688f)*(MagickRealType) i;
        z_map[i].z=(-0.081312f)*(MagickRealType) i;
      }
      break;
    }
    case Rec709LumaColorspace:
    {
      /*
        Initialize Rec709 luma tables:

          G = 0.21260*R+0.71520*G+0.07220*B
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.21260f*(MagickRealType) i;
        y_map[i].x=0.71520f*(MagickRealType) i;
        z_map[i].x=0.07220f*(MagickRealType) i;
        x_map[i].y=0.21260f*(MagickRealType) i;
        y_map[i].y=0.71520f*(MagickRealType) i;
        z_map[i].y=0.07220f*(MagickRealType) i;
        x_map[i].z=0.21260f*(MagickRealType) i;
        y_map[i].z=0.71520f*(MagickRealType) i;
        z_map[i].z=0.07220f*(MagickRealType) i;
      }
      break;
    }
    case Rec709YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables (ITU-R BT.709):

          Y =  0.212600*R+0.715200*G+0.072200*B
          Cb= -0.114572*R-0.385428*G+0.500000*B
          Cr=  0.500000*R-0.454153*G-0.045847*B

        Cb and Cr, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.212600f*(MagickRealType) i;
        y_map[i].x=0.715200f*(MagickRealType) i;
        z_map[i].x=0.072200f*(MagickRealType) i;
        x_map[i].y=(-0.114572f)*(MagickRealType) i;
        y_map[i].y=(-0.385428f)*(MagickRealType) i;
        z_map[i].y=0.500000f*(MagickRealType) i;
        x_map[i].z=0.500000f*(MagickRealType) i;
        y_map[i].z=(-0.454153f)*(MagickRealType) i;
        z_map[i].z=(-0.045847f)*(MagickRealType) i;
      }
      break;
    }
    case sRGBColorspace:
    {
      /*
        Linear RGB to nonlinear sRGB (http://www.w3.org/Graphics/Color/sRGB):

          R = 1.0*R+0.0*G+0.0*B
          G = 0.0*R+0.1*G+0.0*B
          B = 0.0*R+0.0*G+1.0*B
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        MagickRealType
          v;

        v=(MagickRealType) i/(MagickRealType) MaxMap;
        if (((MagickRealType) i/(MagickRealType) MaxMap) <= 0.03928f)
          v/=12.92f;
        else
          v=(MagickRealType) MaxMap*pow((((double) i/MaxMap)+0.055)/1.055,2.4);
        x_map[i].x=1.0f*v;
        y_map[i].x=0.0f*v;
        z_map[i].x=0.0f*v;
        x_map[i].y=0.0f*v;
        y_map[i].y=1.0f*v;
        z_map[i].y=0.0f*v;
        x_map[i].z=0.0f*v;
        y_map[i].z=0.0f*v;
        z_map[i].z=1.0f*v;
      }
      break;
    }
    case XYZColorspace:
    {
      /*
        Initialize CIE XYZ tables (ITU-R 709 RGB):

          X = 0.4124564*R+0.3575761*G+0.1804375*B
          Y = 0.2126729*R+0.7151522*G+0.0721750*B
          Z = 0.0193339*R+0.1191920*G+0.9503041*B
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.4124564f*(MagickRealType) i;
        y_map[i].x=0.3575761f*(MagickRealType) i;
        z_map[i].x=0.1804375f*(MagickRealType) i;
        x_map[i].y=0.2126729f*(MagickRealType) i;
        y_map[i].y=0.7151522f*(MagickRealType) i;
        z_map[i].y=0.0721750f*(MagickRealType) i;
        x_map[i].z=0.0193339f*(MagickRealType) i;
        y_map[i].z=0.1191920f*(MagickRealType) i;
        z_map[i].z=0.9503041f*(MagickRealType) i;
      }
      break;
    }
    case YCCColorspace:
    {
      /*
        Initialize YCC tables:

          Y =  0.29900*R+0.58700*G+0.11400*B
          C1= -0.29900*R-0.58700*G+0.88600*B
          C2=  0.70100*R-0.58700*G-0.11400*B

        YCC is scaled by 1.3584.  C1 zero is 156 and C2 is at 137.
      */
      primary_info.y=(double) ScaleQuantumToMap(ScaleCharToQuantum(156));
      primary_info.z=(double) ScaleQuantumToMap(ScaleCharToQuantum(137));
      for (i=0; i <= (long) (0.018*MaxMap); i++)
      {
        x_map[i].x=0.003962014134275617f*(MagickRealType) i;
        y_map[i].x=0.007778268551236748f*(MagickRealType) i;
        z_map[i].x=0.001510600706713781f*(MagickRealType) i;
        x_map[i].y=(-0.002426619775463276f)*(MagickRealType) i;
        y_map[i].y=(-0.004763965913702149f)*(MagickRealType) i;
        z_map[i].y=0.007190585689165425f*(MagickRealType) i;
        x_map[i].z=0.006927257754597858f*(MagickRealType) i;
        y_map[i].z=(-0.005800713697502058f)*(MagickRealType) i;
        z_map[i].z=(-0.0011265440570958f)*(MagickRealType) i;
      }
      for ( ; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.2201118963486454*(1.099f*(MagickRealType) i-0.099f);
        y_map[i].x=0.4321260306242638*(1.099f*(MagickRealType) i-0.099f);
        z_map[i].x=0.08392226148409894*(1.099f*(MagickRealType) i-0.099f);
        x_map[i].y=(-0.1348122097479598)*(1.099f*(MagickRealType) i-0.099f);
        y_map[i].y=(-0.2646647729834528)*(1.099f*(MagickRealType) i-0.099f);
        z_map[i].y=0.3994769827314126*(1.099f*(MagickRealType) i-0.099f);
        x_map[i].z=0.3848476530332144*(1.099f*(MagickRealType) i-0.099f);
        y_map[i].z=(-0.3222618720834477)*(1.099f*(MagickRealType) i-0.099f);
        z_map[i].z=(-0.06258578094976668)*(1.099f*(MagickRealType) i-0.099f);
      }
      break;
    }
    case YIQColorspace:
    {
      /*
        Initialize YIQ tables:

          Y = 0.29900*R+0.58700*G+0.11400*B
          I = 0.59600*R-0.27400*G-0.32200*B
          Q = 0.21100*R-0.52300*G+0.31200*B

        I and Q, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.29900f*(MagickRealType) i;
        y_map[i].x=0.58700f*(MagickRealType) i;
        z_map[i].x=0.11400f*(MagickRealType) i;
        x_map[i].y=0.59600f*(MagickRealType) i;
        y_map[i].y=(-0.27400f)*(MagickRealType) i;
        z_map[i].y=(-0.32200f)*(MagickRealType) i;
        x_map[i].z=0.21100f*(MagickRealType) i;
        y_map[i].z=(-0.52300f)*(MagickRealType) i;
        z_map[i].z=0.31200f*(MagickRealType) i;
      }
      break;
    }
    case YPbPrColorspace:
    {
      /*
        Initialize YPbPr tables (ITU-R BT.601):

          Y =  0.299000*R+0.587000*G+0.114000*B
          Pb= -0.168736*R-0.331264*G+0.500000*B
          Pr=  0.500000*R-0.418688*G-0.081312*B

        Pb and Pr, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.299000f*(MagickRealType) i;
        y_map[i].x=0.587000f*(MagickRealType) i;
        z_map[i].x=0.114000f*(MagickRealType) i;
        x_map[i].y=(-0.168736f)*(MagickRealType) i;
        y_map[i].y=(-0.331264f)*(MagickRealType) i;
        z_map[i].y=0.500000f*(MagickRealType) i;
        x_map[i].z=0.500000f*(MagickRealType) i;
        y_map[i].z=(-0.418688f)*(MagickRealType) i;
        z_map[i].z=(-0.081312f)*(MagickRealType) i;
      }
      break;
    }
    case YUVColorspace:
    default:
    {
      /*
        Initialize YUV tables:

          Y =  0.29900*R+0.58700*G+0.11400*B
          U = -0.14740*R-0.28950*G+0.43690*B
          V =  0.61500*R-0.51500*G-0.10000*B

        U and V, normally -0.5 through 0.5, are normalized to the range 0
        through QuantumRange.  Note that U = 0.493*(B-Y), V = 0.877*(R-Y).
      */
      primary_info.y=(double) (MaxMap+1.0)/2.0;
      primary_info.z=(double) (MaxMap+1.0)/2.0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=0.29900f*(MagickRealType) i;
        y_map[i].x=0.58700f*(MagickRealType) i;
        z_map[i].x=0.11400f*(MagickRealType) i;
        x_map[i].y=(-0.14740f)*(MagickRealType) i;
        y_map[i].y=(-0.28950f)*(MagickRealType) i;
        z_map[i].y=0.43690f*(MagickRealType) i;
        x_map[i].z=0.61500f*(MagickRealType) i;
        y_map[i].z=(-0.51500f)*(MagickRealType) i;
        z_map[i].z=(-0.10000f)*(MagickRealType) i;
      }
      break;
    }
  }
  /*
    Convert from RGB.
  */
  switch (image->storage_class)
  {
    case DirectClass:
    default:
    {
      /*
        Convert DirectClass image.
      */
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickPixelPacket
          pixel;

        register long
          x;

        register PixelPacket
          *restrict q;

        register unsigned long
          blue,
          green,
          red;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) image->columns; x++)
        {
          red=ScaleQuantumToMap(q->red);
          green=ScaleQuantumToMap(q->green);
          blue=ScaleQuantumToMap(q->blue);
          pixel.red=(x_map[red].x+y_map[green].x+z_map[blue].x)+
            (MagickRealType) primary_info.x;
          pixel.green=(x_map[red].y+y_map[green].y+z_map[blue].y)+
            (MagickRealType) primary_info.y;
          pixel.blue=(x_map[red].z+y_map[green].z+z_map[blue].z)+
            (MagickRealType) primary_info.z;
          q->red=ScaleMapToQuantum(pixel.red);
          q->green=ScaleMapToQuantum(pixel.green);
          q->blue=ScaleMapToQuantum(pixel.blue);
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
  #pragma omp critical (MagickCore_RGBTransformImage)
#endif
            proceed=SetImageProgress(image,RGBTransformImageTag,progress++,
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
      register unsigned long
        blue,
        green,
        red;

      /*
        Convert PseudoClass image.
      */
      image_view=AcquireCacheView(image);
      for (i=0; i < (long) image->colors; i++)
      {
        MagickPixelPacket
          pixel;

        red=ScaleQuantumToMap(image->colormap[i].red);
        green=ScaleQuantumToMap(image->colormap[i].green);
        blue=ScaleQuantumToMap(image->colormap[i].blue);
        pixel.red=x_map[red].x+y_map[green].x+z_map[blue].x+primary_info.x;
        pixel.green=x_map[red].y+y_map[green].y+z_map[blue].y+primary_info.y;
        pixel.blue=x_map[red].z+y_map[green].z+z_map[blue].z+primary_info.z;
        image->colormap[i].red=ScaleMapToQuantum(pixel.red);
        image->colormap[i].green=ScaleMapToQuantum(pixel.green);
        image->colormap[i].blue=ScaleMapToQuantum(pixel.blue);
      }
      image_view=DestroyCacheView(image_view);
      (void) SyncImage(image);
      break;
    }
  }
  /*
    Relinquish resources.
  */
  z_map=(TransformPacket *) RelinquishMagickMemory(z_map);
  y_map=(TransformPacket *) RelinquishMagickMemory(y_map);
  x_map=(TransformPacket *) RelinquishMagickMemory(x_map);
  if (SetImageColorspace(image,colorspace) == MagickFalse)
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
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colorspace: the colorspace.
%
*/
MagickExport MagickBooleanType SetImageColorspace(Image *image,
  const ColorspaceType colorspace)
{
  Cache
    cache;

  if (image->colorspace == colorspace)
    return(MagickTrue);
  image->colorspace=colorspace;
  cache=GetImagePixelCache(image,MagickTrue,&image->exception);
  image->colorspace=colorspace;  /* GRAY colorspace might get reset to RGB */
  return(cache == (Cache) NULL ? MagickFalse: MagickTrue);
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
%  TransformImageColorspace() transforms an image colorspace.
%
%  The format of the TransformImageColorspace method is:
%
%      MagickBooleanType TransformImageColorspace(Image *image,
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colorspace: the colorspace.
%
*/
MagickExport MagickBooleanType TransformImageColorspace(Image *image,
  const ColorspaceType colorspace)
{
  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (colorspace == UndefinedColorspace)
    {
      if (SetImageColorspace(image,colorspace) == MagickFalse)
        return(MagickFalse);
      return(MagickTrue);
    }
  if (image->colorspace == colorspace)
    return(MagickTrue);
  if ((colorspace == RGBColorspace) || (colorspace == TransparentColorspace))
    return(TransformRGBImage(image,image->colorspace));
  status=MagickTrue;
  if ((image->colorspace != RGBColorspace) &&
      (image->colorspace != TransparentColorspace) &&
      (image->colorspace != GRAYColorspace))
    status=TransformRGBImage(image,image->colorspace);
  if (RGBTransformImage(image,colorspace) == MagickFalse)
    status=MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     T r a n s f o r m R G B I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformRGBImage() converts the reference image from an alternate
%  colorspace to RGB.  The transformation matrices are not the standard ones:
%  the weights are rescaled to normalize the range of the transformed values to
%  be [0..QuantumRange].
%
%  The format of the TransformRGBImage method is:
%
%      MagickBooleanType TransformRGBImage(Image *image,
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colorspace: the colorspace to transform the image to.
%
*/

static inline void ConvertLabToXYZ(const double L,const double a,const double b,
  double *X,double *Y,double *Z)
{
  double
    x,
    y,
    z;

  assert(X != (double *) NULL);
  assert(Y != (double *) NULL);
  assert(Z != (double *) NULL);
  y=((2.0*L-1.0)+0.160)/1.160;
  x=(2.0*a-1.0)/5.000+y;
  z=y-(2.0*b-1.0)/2.000;
  if ((x*x*x) > (216.0/24389.0))
    x=x*x*x;
  else
    x=(x-16.0/116.0)/7.787;
  if ((y*y*y) > (216.0/24389.0))
    y=y*y*y;
  else
    y=(y-16.0/116.0)/7.787;
  if ((z*z*z) > (216.0/24389.0))
    z=z*z*z;
  else
    z=(z-16.0/116.0)/7.787;
  *X=0.9504559271*x;
  *Y=1.0000000000*y;
  *Z=1.0890577508*z;
}

static inline unsigned short RoundToYCC(const MagickRealType value)
{
  if (value <= 0.0)
    return(0UL);
  if (value >= 350.0)
    return(350);
  return((unsigned short) (value+0.5));
}

static inline void ConvertXYZToRGB(const double x,const double y,const double z,
  Quantum *red,Quantum *green,Quantum *blue)
{
  double
    b,
    g,
    r;

  /*
    Convert XYZ to RGB colorspace.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  r=3.2407100*x-1.5372600*y-0.4985710*z;
  g=(-0.9692580*x+1.8759900*y+0.0415557*z);
  b=0.0556352*x-0.2039960*y+1.0570700*z;
  *red=ClampToQuantum((MagickRealType) QuantumRange*r);
  *green=ClampToQuantum((MagickRealType) QuantumRange*g);
  *blue=ClampToQuantum((MagickRealType) QuantumRange*b);
}

static inline void ConvertCMYKToRGB(MagickPixelPacket *pixel)
{
  pixel->red=(MagickRealType) QuantumRange-(QuantumScale*pixel->red*
    (QuantumRange-pixel->index)+pixel->index);
  pixel->green=(MagickRealType) QuantumRange-(QuantumScale*pixel->green*
    (QuantumRange-pixel->index)+pixel->index);
  pixel->blue=(MagickRealType) QuantumRange-(QuantumScale*pixel->blue*
    (QuantumRange-pixel->index)+pixel->index);
}

MagickExport MagickBooleanType TransformRGBImage(Image *image,
  const ColorspaceType colorspace)
{
#define D50X  (0.9642)
#define D50Y  (1.0)
#define D50Z  (0.8249)
#define TransformRGBImageTag  "Transform/Image"

#if !defined(MAGICKCORE_HDRI_SUPPORT)
  static const unsigned char
    YCCMap[351] =  /* Photo CD information beyond 100% white, Gamma 2.2 */
    {
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,
       14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
       28,  29,  30,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,
       43,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  56,  57,  58,
       59,  60,  61,  62,  63,  64,  66,  67,  68,  69,  70,  71,  72,  73,
       74,  76,  77,  78,  79,  80,  81,  82,  83,  84,  86,  87,  88,  89,
       90,  91,  92,  93,  94,  95,  97,  98,  99, 100, 101, 102, 103, 104,
      105, 106, 107, 108, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
      120, 121, 122, 123, 124, 125, 126, 127, 129, 130, 131, 132, 133, 134,
      135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
      149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162,
      163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
      176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
      190, 191, 192, 193, 193, 194, 195, 196, 197, 198, 199, 200, 201, 201,
      202, 203, 204, 205, 206, 207, 207, 208, 209, 210, 211, 211, 212, 213,
      214, 215, 215, 216, 217, 218, 218, 219, 220, 221, 221, 222, 223, 224,
      224, 225, 226, 226, 227, 228, 228, 229, 230, 230, 231, 232, 232, 233,
      234, 234, 235, 236, 236, 237, 237, 238, 238, 239, 240, 240, 241, 241,
      242, 242, 243, 243, 244, 244, 245, 245, 245, 246, 246, 247, 247, 247,
      248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 251, 251, 251,
      251, 251, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255
    };
#endif

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  register long
    i;

  TransformPacket
    *y_map,
    *x_map,
    *z_map;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  switch (colorspace)
  {
    case GRAYColorspace:
    case Rec601LumaColorspace:
    case Rec709LumaColorspace:
    case RGBColorspace:
    case TransparentColorspace:
    case UndefinedColorspace:
      return(MagickTrue);
    default:
      break;
  }
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  switch (colorspace)
  {
    case CMYColorspace:
    {
      /*
        Transform image from CMY to RGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ClampToQuantum((MagickRealType) (QuantumRange-q->red));
          q->green=ClampToQuantum((MagickRealType) (QuantumRange-q->green));
          q->blue=ClampToQuantum((MagickRealType) (QuantumRange-q->blue));
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case CMYKColorspace:
    {
      MagickPixelPacket
        zero;

      /*
        Transform image from CMYK to RGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      GetMagickPixelPacket(image,&zero);
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickBooleanType
          sync;

        MagickPixelPacket
          pixel;

        register IndexPacket
          *restrict indexes;

        register long
          x;

        register PixelPacket
          *restrict q;

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
        pixel=zero;
        for (x=0; x < (long) image->columns; x++)
        {
          SetMagickPixelPacket(image,q,indexes+x,&pixel);
          ConvertCMYKToRGB(&pixel);
          SetPixelPacket(image,&pixel,q,indexes+x);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case HSBColorspace:
    {
      /*
        Transform image from HSB to RGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          brightness,
          hue,
          saturation;

        MagickBooleanType
          sync;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) image->columns; x++)
        {
          hue=(double) (QuantumScale*q->red);
          saturation=(double) (QuantumScale*q->green);
          brightness=(double) (QuantumScale*q->blue);
          ConvertHSBToRGB(hue,saturation,brightness,&q->red,&q->green,&q->blue);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case HSLColorspace:
    {
      /*
        Transform image from HSL to RGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          hue,
          lightness,
          saturation;

        MagickBooleanType
          sync;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) image->columns; x++)
        {
          hue=(double) (QuantumScale*q->red);
          saturation=(double) (QuantumScale*q->green);
          lightness=(double) (QuantumScale*q->blue);
          ConvertHSLToRGB(hue,saturation,lightness,&q->red,&q->green,&q->blue);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case HWBColorspace:
    {
      /*
        Transform image from HWB to RGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          blackness,
          hue,
          whiteness;

        MagickBooleanType
          sync;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) image->columns; x++)
        {
          hue=(double) (QuantumScale*q->red);
          whiteness=(double) (QuantumScale*q->green);
          blackness=(double) (QuantumScale*q->blue);
          ConvertHWBToRGB(hue,whiteness,blackness,&q->red,&q->green,&q->blue);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
        return(MagickFalse);
      return(status);
    }
    case LabColorspace:
    {
      /*
        Transform image from Lab to RGB.
      */
      if (image->storage_class == PseudoClass)
        {
          if (SyncImage(image) == MagickFalse)
            return(MagickFalse);
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            return(MagickFalse);
        }
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        double
          a,
          b,
          L,
          X,
          Y,
          Z;

        MagickBooleanType
          sync;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        X=0.0;
        Y=0.0;
        Z=0.0;
        for (x=0; x < (long) image->columns; x++)
        {
          L=QuantumScale*q->red;
          a=QuantumScale*q->green;
          b=QuantumScale*q->blue;
          ConvertLabToXYZ(L,a,b,&X,&Y,&Z);
          ConvertXYZToRGB(X,Y,Z,&q->red,&q->green,&q->blue);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
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
        gamma,
        reference_black,
        reference_white;

      Quantum
        *logmap;

      /*
        Transform Log to RGB colorspace.
      */
      density=2.03728;
      gamma=DisplayGamma;
      value=GetImageProperty(image,"gamma");
      if (value != (const char *) NULL)
        gamma=1.0/StringToDouble(value) != 0.0 ? StringToDouble(value) : 1.0;
      reference_black=ReferenceBlack;
      value=GetImageProperty(image,"reference-black");
      if (value != (const char *) NULL)
        reference_black=StringToDouble(value);
      reference_white=ReferenceWhite;
      value=GetImageProperty(image,"reference-white");
      if (value != (const char *) NULL)
        reference_white=StringToDouble(value);
      logmap=(Quantum *) AcquireQuantumMemory((size_t) MaxMap+1UL,
        sizeof(*logmap));
      if (logmap == (Quantum *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      black=pow(10.0,(reference_black-reference_white)*(gamma/density)*
        0.002/0.6);
      for (i=0; i <= (long) (reference_black*MaxMap/1024.0); i++)
        logmap[i]=(Quantum) 0;
      for ( ; i < (long) (reference_white*MaxMap/1024.0); i++)
        logmap[i]=ClampToQuantum((MagickRealType) QuantumRange/(1.0-black)*
          (pow(10.0,(1024.0*i/MaxMap-reference_white)*
          (gamma/density)*0.002/0.6)-black));
      for ( ; i <= (long) MaxMap; i++)
        logmap[i]=(Quantum) QuantumRange;
      if (SetImageStorageClass(image,DirectClass) == MagickFalse)
        return(MagickFalse);
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=(long) image->columns; x != 0; x--)
        {
          q->red=logmap[ScaleQuantumToMap(q->red)];
          q->green=logmap[ScaleQuantumToMap(q->green)];
          q->blue=logmap[ScaleQuantumToMap(q->blue)];
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      logmap=(Quantum *) RelinquishMagickMemory(logmap);
      if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
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
  switch (colorspace)
  {
    case OHTAColorspace:
    {
      /*
        Initialize OHTA tables:

          R = I1+1.00000*I2-0.66668*I3
          G = I1+0.00000*I2+1.33333*I3
          B = I1-1.00000*I2-0.66668*I3

        I and Q, normally -0.5 through 0.5, must be normalized to the range 0
        through QuantumRange.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) i;
        y_map[i].x=0.500000f*(2.000000*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].x=(-0.333340f)*(2.000000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].y=(MagickRealType) i;
        y_map[i].y=0.000000f;
        z_map[i].y=0.666665f*(2.000000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].z=(MagickRealType) i;
        y_map[i].z=(-0.500000f)*(2.000000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].z=(-0.333340f)*(2.000000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
      }
      break;
    }
    case Rec601YCbCrColorspace:
    case YCbCrColorspace:
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
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) i;
        y_map[i].x=0.000000f;
        z_map[i].x=(1.402000f*0.500000f)*(2.000000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        x_map[i].y=(MagickRealType) i;
        y_map[i].y=(-0.344136f*0.500000f)*(2.000000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        z_map[i].y=(-0.714136f*0.500000f)*(2.000000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        x_map[i].z=(MagickRealType) i;
        y_map[i].z=(1.772000f*0.500000f)*(2.000000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        z_map[i].z=0.000000f;
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
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) i;
        y_map[i].x=0.000000f;
        z_map[i].x=(1.574800f*0.50000f)*(2.00000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        x_map[i].y=(MagickRealType) i;
        y_map[i].y=(-0.187324f*0.50000f)*(2.00000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        z_map[i].y=(-0.468124f*0.50000f)*(2.00000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        x_map[i].z=(MagickRealType) i;
        y_map[i].z=(1.855600f*0.50000f)*(2.00000f*(MagickRealType) i-
          (MagickRealType) MaxMap);
        z_map[i].z=0.00000f;
      }
      break;
    }
    case sRGBColorspace:
    {
      /*
        Nonlinear sRGB to linear RGB.

          R = 1.0*R+0.0*G+0.0*B
          G = 0.0*R+1.0*G+0.0*B
          B = 0.0*R+0.0*G+1.0*B
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=1.0f*(MagickRealType) i;
        y_map[i].x=0.0f*(MagickRealType) i;
        z_map[i].x=0.0f*(MagickRealType) i;
        x_map[i].y=0.0f*(MagickRealType) i;
        y_map[i].y=1.0f*(MagickRealType) i;
        z_map[i].y=0.0f*(MagickRealType) i;
        x_map[i].z=0.0f*(MagickRealType) i;
        y_map[i].z=0.0f*(MagickRealType) i;
        z_map[i].z=1.0f*(MagickRealType) i;
      }
      break;
    }
    case XYZColorspace:
    {
      /*
        Initialize CIE XYZ tables (ITU R-709 RGB):

          R =  3.2404542*X-1.5371385*Y-0.4985314*Z
          G = -0.9692660*X+1.8760108*Y+0.0415560*Z
          B =  0.0556434*X-0.2040259*Y+1.057225*Z
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=3.2404542f*(MagickRealType) i;
        x_map[i].y=(-0.9692660f)*(MagickRealType) i;
        x_map[i].z=0.0556434f*(MagickRealType) i;
        y_map[i].x=(-1.5371385f)*(MagickRealType) i;
        y_map[i].y=1.8760108f*(MagickRealType) i;
        y_map[i].z=(-0.2040259f)*(MagickRealType) i;
        z_map[i].x=(-0.4985314f)*(MagickRealType) i;
        z_map[i].y=0.0415560f*(MagickRealType) i;
        z_map[i].z=1.0572252f*(MagickRealType) i;
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
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=1.3584000f*(MagickRealType) i;
        y_map[i].x=0.0000000f;
        z_map[i].x=1.8215000f*((MagickRealType) i-(MagickRealType)
          ScaleQuantumToMap(ScaleCharToQuantum(137)));
        x_map[i].y=1.3584000f*(MagickRealType) i;
        y_map[i].y=(-0.4302726f)*((MagickRealType) i-(MagickRealType)
          ScaleQuantumToMap(ScaleCharToQuantum(156)));
        z_map[i].y=(-0.9271435f)*((MagickRealType) i-(MagickRealType)
          ScaleQuantumToMap(ScaleCharToQuantum(137)));
        x_map[i].z=1.3584000f*(MagickRealType) i;
        y_map[i].z=2.2179000f*((MagickRealType) i-(MagickRealType)
          ScaleQuantumToMap(ScaleCharToQuantum(156)));
        z_map[i].z=0.0000000f;
      }
      break;
    }
    case YIQColorspace:
    {
      /*
        Initialize YIQ tables:

          R = Y+0.95620*I+0.62140*Q
          G = Y-0.27270*I-0.64680*Q
          B = Y-1.10370*I+1.70060*Q

        I and Q, normally -0.5 through 0.5, must be normalized to the range 0
        through QuantumRange.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) i;
        y_map[i].x=0.47810f*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].x=0.31070f*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].y=(MagickRealType) i;
        y_map[i].y=(-0.13635f)*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].y=(-0.32340f)*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].z=(MagickRealType) i;
        y_map[i].z=(-0.55185f)*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].z=0.85030f*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
      }
      break;
    }
    case YPbPrColorspace:
    {
      /*
        Initialize YPbPr tables:

          R = Y            +1.402000*C2
          G = Y-0.344136*C1+0.714136*C2
          B = Y+1.772000*C1

        Pb and Pr, normally -0.5 through 0.5, must be normalized to the range 0
        through QuantumRange.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) i;
        y_map[i].x=0.000000f;
        z_map[i].x=0.701000f*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].y=(MagickRealType) i;
        y_map[i].y=(-0.172068f)*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].y=0.357068f*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].z=(MagickRealType) i;
        y_map[i].z=0.88600f*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].z=0.00000f;
      }
      break;
    }
    case YUVColorspace:
    default:
    {
      /*
        Initialize YUV tables:

          R = Y          +1.13980*V
          G = Y-0.39380*U-0.58050*V
          B = Y+2.02790*U

        U and V, normally -0.5 through 0.5, must be normalized to the range 0
        through QuantumRange.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4)
#endif
      for (i=0; i <= (long) MaxMap; i++)
      {
        x_map[i].x=(MagickRealType) i;
        y_map[i].x=0.00000f;
        z_map[i].x=0.56990f*(2.0000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].y=(MagickRealType) i;
        y_map[i].y=(-0.19690f)*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].y=(-0.29025f)*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        x_map[i].z=(MagickRealType) i;
        y_map[i].z=1.01395f*(2.00000f*(MagickRealType) i-(MagickRealType)
          MaxMap);
        z_map[i].z=0.00000f;
      }
      break;
    }
  }
  /*
    Convert to RGB.
  */
  switch (image->storage_class)
  {
    case DirectClass:
    default:
    {
      /*
        Convert DirectClass image.
      */
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickBooleanType
          sync;

        MagickPixelPacket
          pixel;

        register long
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (long) image->columns; x++)
        {
          register unsigned long
            blue,
            green,
            red;

          red=ScaleQuantumToMap(q->red);
          green=ScaleQuantumToMap(q->green);
          blue=ScaleQuantumToMap(q->blue);
          pixel.red=x_map[red].x+y_map[green].x+z_map[blue].x;
          pixel.green=x_map[red].y+y_map[green].y+z_map[blue].y;
          pixel.blue=x_map[red].z+y_map[green].z+z_map[blue].z;
          switch (colorspace)
          {
            case YCCColorspace:
            {
#if !defined(MAGICKCORE_HDRI_SUPPORT)
              pixel.red=(MagickRealType) ScaleCharToQuantum(YCCMap[RoundToYCC(
                255.0*QuantumScale*pixel.red)]);
              pixel.green=(MagickRealType) ScaleCharToQuantum(YCCMap[RoundToYCC(
                255.0*QuantumScale*pixel.green)]);
              pixel.blue=(MagickRealType) ScaleCharToQuantum(YCCMap[RoundToYCC(
                255.0*QuantumScale*pixel.blue)]);
#endif
              break;
            }
            case sRGBColorspace:
            {
              if ((QuantumScale*pixel.red) <= 0.0031308)
                pixel.red*=12.92f;
              else
                pixel.red=(MagickRealType) QuantumRange*(1.055*
                  pow(QuantumScale*pixel.red,(1.0/2.4))-0.055);
              if ((QuantumScale*pixel.green) <= 0.0031308)
                pixel.green*=12.92f;
              else
                pixel.green=(MagickRealType) QuantumRange*(1.055*
                  pow(QuantumScale*pixel.green,(1.0/2.4))-0.055);
              if ((QuantumScale*pixel.blue) <= 0.0031308)
                pixel.blue*=12.92f;
              else
                pixel.blue=(MagickRealType) QuantumRange*(1.055*
                  pow(QuantumScale*pixel.blue,(1.0/2.4))-0.055);
              break;
            }
            default:
              break;
          }
          q->red=ScaleMapToQuantum((MagickRealType) MaxMap*QuantumScale*
            pixel.red);
          q->green=ScaleMapToQuantum((MagickRealType) MaxMap*QuantumScale*
            pixel.green);
          q->blue=ScaleMapToQuantum((MagickRealType) MaxMap*QuantumScale*
            pixel.blue);
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
  #pragma omp critical (MagickCore_TransformRGBImage)
#endif
            proceed=SetImageProgress(image,TransformRGBImageTag,progress++,
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
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        MagickPixelPacket
          pixel;

        register unsigned long
          blue,
          green,
          red;

        red=ScaleQuantumToMap(image->colormap[i].red);
        green=ScaleQuantumToMap(image->colormap[i].green);
        blue=ScaleQuantumToMap(image->colormap[i].blue);
        pixel.red=x_map[red].x+y_map[green].x+z_map[blue].x;
        pixel.green=x_map[red].y+y_map[green].y+z_map[blue].y;
        pixel.blue=x_map[red].z+y_map[green].z+z_map[blue].z;
        switch (colorspace)
        {
          case YCCColorspace:
          {
#if !defined(MAGICKCORE_HDRI_SUPPORT)
            image->colormap[i].red=ScaleCharToQuantum(YCCMap[RoundToYCC(
              255.0*QuantumScale*pixel.red)]);
            image->colormap[i].green=ScaleCharToQuantum(YCCMap[RoundToYCC(
              255.0*QuantumScale*pixel.green)]);
            image->colormap[i].blue=ScaleCharToQuantum(YCCMap[RoundToYCC(
              255.0*QuantumScale*pixel.blue)]);
#endif
            break;
          }
          case sRGBColorspace:
          {
            if ((QuantumScale*pixel.red) <= 0.0031308)
              pixel.red*=12.92f;
            else
              pixel.red=(MagickRealType) QuantumRange*(1.055*pow(QuantumScale*
                pixel.red,(1.0/2.4))-0.055);
            if ((QuantumScale*pixel.green) <= 0.0031308)
              pixel.green*=12.92f;
            else
              pixel.green=(MagickRealType) QuantumRange*(1.055*pow(QuantumScale*
                pixel.green,(1.0/2.4))-0.055);
            if ((QuantumScale*pixel.blue) <= 0.0031308)
              pixel.blue*=12.92f;
            else
              pixel.blue=(MagickRealType) QuantumRange*(1.055*pow(QuantumScale*
                pixel.blue,(1.0/2.4))-0.055);
          }
          default:
          {
            image->colormap[i].red=ScaleMapToQuantum((MagickRealType) MaxMap*
              QuantumScale*pixel.red);
            image->colormap[i].green=ScaleMapToQuantum((MagickRealType) MaxMap*
              QuantumScale*pixel.green);
            image->colormap[i].blue=ScaleMapToQuantum((MagickRealType) MaxMap*
              QuantumScale*pixel.blue);
            break;
          }
        }
      }
      image_view=DestroyCacheView(image_view);
      (void) SyncImage(image);
      break;
    }
  }
  /*
    Relinquish resources.
  */
  z_map=(TransformPacket *) RelinquishMagickMemory(z_map);
  y_map=(TransformPacket *) RelinquishMagickMemory(y_map);
  x_map=(TransformPacket *) RelinquishMagickMemory(x_map);
  if (SetImageColorspace(image,RGBColorspace) == MagickFalse)
    return(MagickFalse);
  return(MagickTrue);
}

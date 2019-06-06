/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private kernels for accelerated functions.
*/

#ifndef MAGICKCORE_ACCELERATE_KERNELS_PRIVATE_H
#define MAGICKCORE_ACCELERATE_KERNELS_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_OPENCL_SUPPORT)

/*
  Define declarations.
*/
#define OPENCL_DEFINE(VAR,...)	"\n #""define " #VAR " " #__VA_ARGS__ " \n"
#define OPENCL_ELIF(...)	"\n #""elif " #__VA_ARGS__ " \n"
#define OPENCL_ELSE()		"\n #""else " " \n"
#define OPENCL_ENDIF()		"\n #""endif " " \n"
#define OPENCL_IF(...)		"\n #""if " #__VA_ARGS__ " \n"
#define STRINGIFY(...) #__VA_ARGS__ "\n"

const char *accelerateKernels =

/*
  Define declarations.
*/
  OPENCL_DEFINE(SigmaUniform, (attenuate*0.015625f))
  OPENCL_DEFINE(SigmaGaussian, (attenuate*0.015625f))
  OPENCL_DEFINE(SigmaImpulse, (attenuate*0.1f))
  OPENCL_DEFINE(SigmaLaplacian, (attenuate*0.0390625f))
  OPENCL_DEFINE(SigmaMultiplicativeGaussian, (attenuate*0.5f))
  OPENCL_DEFINE(SigmaPoisson, (attenuate*12.5f))
  OPENCL_DEFINE(SigmaRandom, (attenuate))
  OPENCL_DEFINE(TauGaussian, (attenuate*0.078125f))
  OPENCL_DEFINE(MagickMax(x,y), (((x) > (y)) ? (x) : (y)))
  OPENCL_DEFINE(MagickMin(x,y), (((x) < (y)) ? (x) : (y)))

/*
  Typedef declarations.
*/
  STRINGIFY(
    typedef enum
    {
      UndefinedColorspace,
      CMYColorspace,           /* negated linear RGB colorspace */
      CMYKColorspace,          /* CMY with Black separation */
      GRAYColorspace,          /* Single Channel greyscale (non-linear) image */
      HCLColorspace,
      HCLpColorspace,
      HSBColorspace,
      HSIColorspace,
      HSLColorspace,
      HSVColorspace,           /* alias for HSB */
      HWBColorspace,
      LabColorspace,
      LCHColorspace,           /* alias for LCHuv */
      LCHabColorspace,         /* Cylindrical (Polar) Lab */
      LCHuvColorspace,         /* Cylindrical (Polar) Luv */
      LogColorspace,
      LMSColorspace,
      LuvColorspace,
      OHTAColorspace,
      Rec601YCbCrColorspace,
      Rec709YCbCrColorspace,
      RGBColorspace,           /* Linear RGB colorspace */
      scRGBColorspace,         /* ??? */
      sRGBColorspace,          /* Default: non-linear sRGB colorspace */
      TransparentColorspace,
      xyYColorspace,
      XYZColorspace,           /* IEEE Color Reference colorspace */
      YCbCrColorspace,
      YCCColorspace,
      YDbDrColorspace,
      YIQColorspace,
      YPbPrColorspace,
      YUVColorspace,
      LinearGRAYColorspace     /* Single Channel greyscale (linear) image */
    } ColorspaceType;
  )

  STRINGIFY(
    typedef enum
    {
      UndefinedCompositeOp,
      AlphaCompositeOp,
      AtopCompositeOp,
      BlendCompositeOp,
      BlurCompositeOp,
      BumpmapCompositeOp,
      ChangeMaskCompositeOp,
      ClearCompositeOp,
      ColorBurnCompositeOp,
      ColorDodgeCompositeOp,
      ColorizeCompositeOp,
      CopyBlackCompositeOp,
      CopyBlueCompositeOp,
      CopyCompositeOp,
      CopyCyanCompositeOp,
      CopyGreenCompositeOp,
      CopyMagentaCompositeOp,
      CopyAlphaCompositeOp,
      CopyRedCompositeOp,
      CopyYellowCompositeOp,
      DarkenCompositeOp,
      DarkenIntensityCompositeOp,
      DifferenceCompositeOp,
      DisplaceCompositeOp,
      DissolveCompositeOp,
      DistortCompositeOp,
      DivideDstCompositeOp,
      DivideSrcCompositeOp,
      DstAtopCompositeOp,
      DstCompositeOp,
      DstInCompositeOp,
      DstOutCompositeOp,
      DstOverCompositeOp,
      ExclusionCompositeOp,
      HardLightCompositeOp,
      HardMixCompositeOp,
      HueCompositeOp,
      InCompositeOp,
      IntensityCompositeOp,
      LightenCompositeOp,
      LightenIntensityCompositeOp,
      LinearBurnCompositeOp,
      LinearDodgeCompositeOp,
      LinearLightCompositeOp,
      LuminizeCompositeOp,
      MathematicsCompositeOp,
      MinusDstCompositeOp,
      MinusSrcCompositeOp,
      ModulateCompositeOp,
      ModulusAddCompositeOp,
      ModulusSubtractCompositeOp,
      MultiplyCompositeOp,
      NoCompositeOp,
      OutCompositeOp,
      OverCompositeOp,
      OverlayCompositeOp,
      PegtopLightCompositeOp,
      PinLightCompositeOp,
      PlusCompositeOp,
      ReplaceCompositeOp,
      SaturateCompositeOp,
      ScreenCompositeOp,
      SoftLightCompositeOp,
      SrcAtopCompositeOp,
      SrcCompositeOp,
      SrcInCompositeOp,
      SrcOutCompositeOp,
      SrcOverCompositeOp,
      ThresholdCompositeOp,
      VividLightCompositeOp,
      XorCompositeOp,
      StereoCompositeOp
    } CompositeOperator;
  )

  STRINGIFY(
    typedef enum
    {
      UndefinedFunction,
      ArcsinFunction,
      ArctanFunction,
      PolynomialFunction,
      SinusoidFunction
    } MagickFunction;
  )

  STRINGIFY(
    typedef enum
    {
      UndefinedNoise,
      UniformNoise,
      GaussianNoise,
      MultiplicativeGaussianNoise,
      ImpulseNoise,
      LaplacianNoise,
      PoissonNoise,
      RandomNoise
    } NoiseType;
  )

  STRINGIFY(
    typedef enum
    {
      UndefinedPixelIntensityMethod = 0,
      AveragePixelIntensityMethod,
      BrightnessPixelIntensityMethod,
      LightnessPixelIntensityMethod,
      MSPixelIntensityMethod,
      Rec601LumaPixelIntensityMethod,
      Rec601LuminancePixelIntensityMethod,
      Rec709LumaPixelIntensityMethod,
      Rec709LuminancePixelIntensityMethod,
      RMSPixelIntensityMethod
    } PixelIntensityMethod;
  )

  STRINGIFY(
    typedef enum
    {
      BoxWeightingFunction = 0,
      TriangleWeightingFunction,
      CubicBCWeightingFunction,
      HannWeightingFunction,
      HammingWeightingFunction,
      BlackmanWeightingFunction,
      GaussianWeightingFunction,
      QuadraticWeightingFunction,
      JincWeightingFunction,
      SincWeightingFunction,
      SincFastWeightingFunction,
      KaiserWeightingFunction,
      WelchWeightingFunction,
      BohmanWeightingFunction,
      LagrangeWeightingFunction,
      CosineWeightingFunction
    } ResizeWeightingFunctionType;
  )

  STRINGIFY(
    typedef enum
    {
      UndefinedChannel = 0x0000,
      RedChannel = 0x0001,
      GrayChannel = 0x0001,
      CyanChannel = 0x0001,
      GreenChannel = 0x0002,
      MagentaChannel = 0x0002,
      BlueChannel = 0x0004,
      YellowChannel = 0x0004,
      BlackChannel = 0x0008,
      AlphaChannel = 0x0010,
      OpacityChannel = 0x0010,
      IndexChannel = 0x0020,             /* Color Index Table? */
      ReadMaskChannel = 0x0040,          /* Pixel is Not Readable? */
      WriteMaskChannel = 0x0080,         /* Pixel is Write Protected? */
      MetaChannel = 0x0100,              /* ???? */
      CompositeChannels = 0x001F,
      AllChannels = 0x7ffffff,
      /*
        Special purpose channel types.
        FUTURE: are these needed any more - they are more like hacks
        SyncChannels for example is NOT a real channel but a 'flag'
        It really says -- "User has not defined channels"
        Though it does have extra meaning in the "-auto-level" operator
      */
      TrueAlphaChannel = 0x0100, /* extract actual alpha channel from opacity */
      RGBChannels = 0x0200,      /* set alpha from grayscale mask in RGB */
      GrayChannels = 0x0400,
      SyncChannels = 0x20000,    /* channels modified as a single unit */
      DefaultChannels = AllChannels
    } ChannelType;  /* must correspond to PixelChannel */
  )

/*
  Helper functions.
*/

OPENCL_IF((MAGICKCORE_QUANTUM_DEPTH == 8))

  STRINGIFY(
    static inline CLQuantum ScaleCharToQuantum(const unsigned char value)
    {
      return((CLQuantum) value);
    }
  )

OPENCL_ELIF((MAGICKCORE_QUANTUM_DEPTH == 16))

  STRINGIFY(
    static inline CLQuantum ScaleCharToQuantum(const unsigned char value)
    {
      return((CLQuantum) (257.0f*value));
    }
  )

OPENCL_ELIF((MAGICKCORE_QUANTUM_DEPTH == 32))

  STRINGIFY(
    static inline CLQuantum ScaleCharToQuantum(const unsigned char value)
    {
      return((CLQuantum) (16843009.0*value));
    }
  )

OPENCL_ENDIF()

OPENCL_IF((MAGICKCORE_HDRI_SUPPORT == 1))

  STRINGIFY(
    static inline CLQuantum ClampToQuantum(const float value)
      {
        return (CLQuantum) clamp(value, 0.0f, QuantumRange);
      }
  )

OPENCL_ELSE()

  STRINGIFY(
    static inline CLQuantum ClampToQuantum(const float value)
      {
        return (CLQuantum) (clamp(value, 0.0f, QuantumRange) + 0.5f);
      }
  )

OPENCL_ENDIF()

  STRINGIFY(
    static inline int ClampToCanvas(const int offset,const int range)
      {
        return clamp(offset, (int)0, range-1);
      }
  )

  STRINGIFY(
    static inline uint ScaleQuantumToMap(CLQuantum value)
      {
        if (value >= (CLQuantum) MaxMap)
          return ((uint)MaxMap);
        else 
          return ((uint)value);
      }
  )

  STRINGIFY(
    static inline float PerceptibleReciprocal(const float x)
    {
      float sign = x < (float) 0.0 ? (float) -1.0 : (float) 1.0;
      return((sign*x) >= MagickEpsilon ? (float) 1.0/x : sign*((float) 1.0/MagickEpsilon));
    }
  )

  STRINGIFY(
  static inline float RoundToUnity(const float value)
   {
     return clamp(value,0.0f,1.0f);
   }
  )

  STRINGIFY(

  static inline unsigned int getPixelIndex(const unsigned int number_channels,
    const unsigned int columns, const unsigned int x, const unsigned int y)
  {
    return (x * number_channels) + (y * columns * number_channels);
  }

  static inline float getPixelRed(const __global CLQuantum *p)   { return (float)*p; }
  static inline float getPixelGreen(const __global CLQuantum *p) { return (float)*(p+1); }
  static inline float getPixelBlue(const __global CLQuantum *p)  { return (float)*(p+2); }
  static inline float getPixelAlpha(const __global CLQuantum *p,const unsigned int number_channels) { return (float)*(p+number_channels-1); }

  static inline void setPixelRed(__global CLQuantum *p,const CLQuantum value)   { *p=value; }
  static inline void setPixelGreen(__global CLQuantum *p,const CLQuantum value) { *(p+1)=value; }
  static inline void setPixelBlue(__global CLQuantum *p,const CLQuantum value)  { *(p+2)=value; }
  static inline void setPixelAlpha(__global CLQuantum *p,const unsigned int number_channels,const CLQuantum value) { *(p+number_channels-1)=value; }

  static inline CLQuantum getBlue(CLPixelType p)               { return p.x; }
  static inline void setBlue(CLPixelType* p, CLQuantum value)  { (*p).x = value; }
  static inline float getBlueF4(float4 p)                      { return p.x; }
  static inline void setBlueF4(float4* p, float value)         { (*p).x = value; }

  static inline CLQuantum getGreen(CLPixelType p)              { return p.y; }
  static inline void setGreen(CLPixelType* p, CLQuantum value) { (*p).y = value; }
  static inline float getGreenF4(float4 p)                     { return p.y; }
  static inline void setGreenF4(float4* p, float value)        { (*p).y = value; }

  static inline CLQuantum getRed(CLPixelType p)                { return p.z; }
  static inline void setRed(CLPixelType* p, CLQuantum value)   { (*p).z = value; }
  static inline float getRedF4(float4 p)                       { return p.z; }
  static inline void setRedF4(float4* p, float value)          { (*p).z = value; }

  static inline CLQuantum getAlpha(CLPixelType p)              { return p.w; }
  static inline void setAlpha(CLPixelType* p, CLQuantum value) { (*p).w = value; }
  static inline float getAlphaF4(float4 p)                     { return p.w; }
  static inline void setAlphaF4(float4* p, float value)        { (*p).w = value; }

  static inline void ReadChannels(const __global CLQuantum *p, const unsigned int number_channels,
    const ChannelType channel, float *red, float *green, float *blue, float *alpha)
  {
    if ((channel & RedChannel) != 0)
      *red=getPixelRed(p);

    if (number_channels > 2)
      {
        if ((channel & GreenChannel) != 0)
          *green=getPixelGreen(p);

        if ((channel & BlueChannel) != 0)
          *blue=getPixelBlue(p);
      }

    if (((number_channels == 4) || (number_channels == 2)) &&
        ((channel & AlphaChannel) != 0))
      *alpha=getPixelAlpha(p,number_channels);
  }

  static inline float4 ReadAllChannels(const __global CLQuantum *image, const unsigned int number_channels,
    const unsigned int columns, const unsigned int x, const unsigned int y)
  {
    const __global CLQuantum *p = image + getPixelIndex(number_channels, columns, x, y);

    float4 pixel;

    pixel.x=getPixelRed(p);

    if (number_channels > 2)
      {
        pixel.y=getPixelGreen(p);
        pixel.z=getPixelBlue(p);
      }

    if ((number_channels == 4) || (number_channels == 2))
      pixel.w=getPixelAlpha(p,number_channels);
    return(pixel);
  }

  static inline float4 ReadFloat4(const __global CLQuantum *image, const unsigned int number_channels,
    const unsigned int columns, const unsigned int x, const unsigned int y, const ChannelType channel)
  {
    const __global CLQuantum *p = image + getPixelIndex(number_channels, columns, x, y);

    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float alpha = 0.0f;

    ReadChannels(p, number_channels, channel, &red, &green, &blue, &alpha);
    return (float4)(red, green, blue, alpha);
  }

  static inline void WriteChannels(__global CLQuantum *p, const unsigned int number_channels,
    const ChannelType channel, float red, float green, float blue, float alpha)
  {
    if ((channel & RedChannel) != 0)
      setPixelRed(p,ClampToQuantum(red));

    if (number_channels > 2)
      {
        if ((channel & GreenChannel) != 0)
          setPixelGreen(p,ClampToQuantum(green));

        if ((channel & BlueChannel) != 0)
          setPixelBlue(p,ClampToQuantum(blue));
      }

    if (((number_channels == 4) || (number_channels == 2)) &&
        ((channel & AlphaChannel) != 0))
      setPixelAlpha(p,number_channels,ClampToQuantum(alpha));
  }

  static inline void WriteAllChannels(__global CLQuantum *image, const unsigned int number_channels,
    const unsigned int columns, const unsigned int x, const unsigned int y, float4 pixel)
  {
    __global CLQuantum *p = image + getPixelIndex(number_channels, columns, x, y);

    setPixelRed(p,ClampToQuantum(pixel.x));

    if (number_channels > 2)
      {
        setPixelGreen(p,ClampToQuantum(pixel.y));
        setPixelBlue(p,ClampToQuantum(pixel.z));
      }

    if ((number_channels == 4) || (number_channels == 2))
      setPixelAlpha(p,number_channels,ClampToQuantum(pixel.w));
  }

  static inline void WriteFloat4(__global CLQuantum *image, const unsigned int number_channels,
    const unsigned int columns, const unsigned int x, const unsigned int y, const ChannelType channel,
    float4 pixel)
  {
    __global CLQuantum *p = image + getPixelIndex(number_channels, columns, x, y);
    WriteChannels(p, number_channels, channel, pixel.x, pixel.y, pixel.z, pixel.w);
  }

  static inline float GetPixelIntensity(const unsigned int colorspace,
    const unsigned int method,float red,float green,float blue)
  {
    float intensity;

    if ((colorspace == GRAYColorspace) || (colorspace == LinearGRAYColorspace))
      return red;

    switch (method)
    {
      case AveragePixelIntensityMethod:
        {
          intensity=(red+green+blue)/3.0;
          break;
        }
      case BrightnessPixelIntensityMethod:
        {
          intensity=MagickMax(MagickMax(red,green),blue);
          break;
        }
      case LightnessPixelIntensityMethod:
        {
          intensity=(MagickMin(MagickMin(red,green),blue)+
              MagickMax(MagickMax(red,green),blue))/2.0;
          break;
        }
      case MSPixelIntensityMethod:
        {
          intensity=(float) (((float) red*red+green*green+blue*blue)/
              (3.0*QuantumRange));
          break;
        }
      case Rec601LumaPixelIntensityMethod:
        {
          /*
          if (image->colorspace == RGBColorspace)
          {
            red=EncodePixelGamma(red);
            green=EncodePixelGamma(green);
            blue=EncodePixelGamma(blue);
          }
          */
          intensity=0.298839*red+0.586811*green+0.114350*blue;
          break;
        }
      case Rec601LuminancePixelIntensityMethod:
        {
          /*
          if (image->colorspace == sRGBColorspace)
          {
            red=DecodePixelGamma(red);
            green=DecodePixelGamma(green);
            blue=DecodePixelGamma(blue);
          }
          */
          intensity=0.298839*red+0.586811*green+0.114350*blue;
          break;
        }
      case Rec709LumaPixelIntensityMethod:
      default:
        {
          /*
          if (image->colorspace == RGBColorspace)
          {
            red=EncodePixelGamma(red);
            green=EncodePixelGamma(green);
            blue=EncodePixelGamma(blue);
          }
          */
          intensity=0.212656*red+0.715158*green+0.072186*blue;
          break;
        }
      case Rec709LuminancePixelIntensityMethod:
        {
          /*
          if (image->colorspace == sRGBColorspace)
          {
            red=DecodePixelGamma(red);
            green=DecodePixelGamma(green);
            blue=DecodePixelGamma(blue);
          }
          */
          intensity=0.212656*red+0.715158*green+0.072186*blue;
          break;
        }
      case RMSPixelIntensityMethod:
        {
          intensity=(float) (sqrt((float) red*red+green*green+blue*blue)/
              sqrt(3.0));
          break;
        }
    }

    return intensity;
  }

  static inline int mirrorBottom(int value)
  {
      return (value < 0) ? - (value) : value;
  }

  static inline int mirrorTop(int value, int width)
  {
      return (value >= width) ? (2 * width - value - 1) : value;
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A d d N o i s e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
  /*
  Part of MWC64X by David Thomas, dt10@imperial.ac.uk
  This is provided under BSD, full license is with the main package.
  See http://www.doc.ic.ac.uk/~dt10/research
  */

  // Pre: a<M, b<M
  // Post: r=(a+b) mod M
  ulong MWC_AddMod64(ulong a, ulong b, ulong M)
  {
    ulong v=a+b;
    //if( (v>=M) || (v<a) )
    if( (v>=M) || (convert_float(v) < convert_float(a)) ) // workaround for what appears to be an optimizer bug.
      v=v-M;
    return v;
  }

  // Pre: a<M,b<M
  // Post: r=(a*b) mod M
  // This could be done more efficently, but it is portable, and should
  // be easy to understand. It can be replaced with any of the better
  // modular multiplication algorithms (for example if you know you have
  // double precision available or something).
  ulong MWC_MulMod64(ulong a, ulong b, ulong M)
  {
    ulong r=0;
    while(a!=0){
      if(a&1)
        r=MWC_AddMod64(r,b,M);
      b=MWC_AddMod64(b,b,M);
      a=a>>1;
    }
    return r;
  }

  // Pre: a<M, e>=0
  // Post: r=(a^b) mod M
  // This takes at most ~64^2 modular additions, so probably about 2^15 or so instructions on
  // most architectures
  ulong MWC_PowMod64(ulong a, ulong e, ulong M)
  {
    ulong sqr=a, acc=1;
    while(e!=0){
      if(e&1)
        acc=MWC_MulMod64(acc,sqr,M);
        sqr=MWC_MulMod64(sqr,sqr,M);
      e=e>>1;
    }
    return acc;
  }

  uint2 MWC_SkipImpl_Mod64(uint2 curr, ulong A, ulong M, ulong distance)
  {
    ulong m=MWC_PowMod64(A, distance, M);
    ulong x=curr.x*(ulong)A+curr.y;
    x=MWC_MulMod64(x, m, M);
    return (uint2)((uint)(x/A), (uint)(x%A));
  }

  uint2 MWC_SeedImpl_Mod64(ulong A, ulong M, uint vecSize, uint vecOffset, ulong streamBase, ulong streamGap)
  {
    // This is an arbitrary constant for starting LCG jumping from. I didn't
    // want to start from 1, as then you end up with the two or three first values
    // being a bit poor in ones - once you've decided that, one constant is as
    // good as any another. There is no deep mathematical reason for it, I just
    // generated a random number.
    enum{ MWC_BASEID = 4077358422479273989UL };

    ulong dist=streamBase + (get_global_id(0)*vecSize+vecOffset)*streamGap;
    ulong m=MWC_PowMod64(A, dist, M);

    ulong x=MWC_MulMod64(MWC_BASEID, m, M);
    return (uint2)((uint)(x/A), (uint)(x%A));
  }

  //! Represents the state of a particular generator
  typedef struct{ uint x; uint c; uint seed0; ulong seed1; } mwc64x_state_t;

  void MWC64X_Step(mwc64x_state_t *s)
  {
    uint X=s->x, C=s->c;

    uint Xn=s->seed0*X+C;
    uint carry=(uint)(Xn<C); // The (Xn<C) will be zero or one for scalar
    uint Cn=mad_hi(s->seed0,X,carry);

    s->x=Xn;
    s->c=Cn;
  }

  void MWC64X_Skip(mwc64x_state_t *s, ulong distance)
  {
    uint2 tmp=MWC_SkipImpl_Mod64((uint2)(s->x,s->c), s->seed0, s->seed1, distance);
    s->x=tmp.x;
    s->c=tmp.y;
  }

  void MWC64X_SeedStreams(mwc64x_state_t *s, ulong baseOffset, ulong perStreamOffset)
  {
    uint2 tmp=MWC_SeedImpl_Mod64(s->seed0, s->seed1, 1, 0, baseOffset, perStreamOffset);
    s->x=tmp.x;
    s->c=tmp.y;
  }

  //! Return a 32-bit integer in the range [0..2^32)
  uint MWC64X_NextUint(mwc64x_state_t *s)
  {
    uint res=s->x ^ s->c;
    MWC64X_Step(s);
    return res;
  }

  //
  // End of MWC64X excerpt
  //

  float mwcReadPseudoRandomValue(mwc64x_state_t* rng)
  {
    return (1.0f * MWC64X_NextUint(rng)) / (float)(0xffffffff); // normalized to 1.0
  }

  float mwcGenerateDifferentialNoise(mwc64x_state_t* r, float pixel, NoiseType noise_type, float attenuate)
  {
    float
      alpha,
      beta,
      noise,
      sigma;

    noise = 0.0f;
    alpha=mwcReadPseudoRandomValue(r);
    switch(noise_type)
    {
      case UniformNoise:
      default:
        {
          noise=(pixel+QuantumRange*SigmaUniform*(alpha-0.5f));
          break;
        }
      case GaussianNoise:
        {
          float
            gamma,
            tau;

          if (alpha == 0.0f)
            alpha=1.0f;
          beta=mwcReadPseudoRandomValue(r);
          gamma=sqrt(-2.0f*log(alpha));
          sigma=gamma*cospi((2.0f*beta));
          tau=gamma*sinpi((2.0f*beta));
          noise=pixel+sqrt(pixel)*SigmaGaussian*sigma+QuantumRange*TauGaussian*tau;
          break;
        }
      case ImpulseNoise:
      {
        if (alpha < (SigmaImpulse/2.0f))
          noise=0.0f;
        else
          if (alpha >= (1.0f-(SigmaImpulse/2.0f)))
            noise=QuantumRange;
          else
            noise=pixel;
        break;
      }
      case LaplacianNoise:
      {
        if (alpha <= 0.5f)
          {
            if (alpha <= MagickEpsilon)
              noise=(pixel-QuantumRange);
            else
              noise=(pixel+QuantumRange*SigmaLaplacian*log(2.0f*alpha)+
                0.5f);
            break;
          }
        beta=1.0f-alpha;
        if (beta <= (0.5f*MagickEpsilon))
          noise=(pixel+QuantumRange);
        else
          noise=(pixel-QuantumRange*SigmaLaplacian*log(2.0f*beta)+0.5f);
        break;
      }
      case MultiplicativeGaussianNoise:
      {
        sigma=1.0f;
        if (alpha > MagickEpsilon)
          sigma=sqrt(-2.0f*log(alpha));
        beta=mwcReadPseudoRandomValue(r);
        noise=(pixel+pixel*SigmaMultiplicativeGaussian*sigma*
          cospi((2.0f*beta))/2.0f);
        break;
      }
      case PoissonNoise:
      {
        float 
          poisson;
        unsigned int i;
        poisson=exp(-SigmaPoisson*QuantumScale*pixel);
        for (i=0; alpha > poisson; i++)
        {
          beta=mwcReadPseudoRandomValue(r);
          alpha*=beta;
        }
        noise=(QuantumRange*i/SigmaPoisson);
        break;
      }
      case RandomNoise:
      {
        noise=(QuantumRange*SigmaRandom*alpha);
        break;
      }
    }
    return noise;
  }

  __kernel
  void AddNoise(const __global CLQuantum *image,
    const unsigned int number_channels,const ChannelType channel,
    const unsigned int length,const unsigned int pixelsPerWorkItem,
    const NoiseType noise_type,const float attenuate,const unsigned int seed0,
    const unsigned int seed1,const unsigned int numRandomNumbersPerPixel,
    __global CLQuantum *filteredImage)
  {
    mwc64x_state_t rng;
    rng.seed0 = seed0;
    rng.seed1 = seed1;

    uint span = pixelsPerWorkItem * numRandomNumbersPerPixel; // length of RNG substream each workitem will use
    uint offset = span * get_local_size(0) * get_group_id(0); // offset of this workgroup's RNG substream (in master stream);
    MWC64X_SeedStreams(&rng, offset, span); // Seed the RNG streams

    uint pos = get_group_id(0) * get_local_size(0) * pixelsPerWorkItem * number_channels + (get_local_id(0) * number_channels);
    uint count = pixelsPerWorkItem;

    while (count > 0 && pos < length)
    {
      const __global CLQuantum *p = image + pos;
      __global CLQuantum *q = filteredImage + pos;

      float red;
      float green;
      float blue;
      float alpha;

      ReadChannels(p, number_channels, channel, &red, &green, &blue, &alpha);

      if ((channel & RedChannel) != 0)
        red=mwcGenerateDifferentialNoise(&rng,red,noise_type,attenuate);

      if (number_channels > 2)
      {
        if ((channel & GreenChannel) != 0)
          green=mwcGenerateDifferentialNoise(&rng,green,noise_type,attenuate);

        if ((channel & BlueChannel) != 0)
          blue=mwcGenerateDifferentialNoise(&rng,blue,noise_type,attenuate);
      }

      if (((number_channels == 4) || (number_channels == 2)) &&
          ((channel & AlphaChannel) != 0))
        alpha=mwcGenerateDifferentialNoise(&rng,alpha,noise_type,attenuate);

      WriteChannels(q, number_channels, channel, red, green, blue, alpha);

      pos += (get_local_size(0) * number_channels);
      count--;
    }
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    B l u r                                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
  /*
  Reduce image noise and reduce detail levels by row
  */
  __kernel void BlurRow(const __global CLQuantum *image,
    const unsigned int number_channels,const ChannelType channel,
    __constant float *filter,const unsigned int width,
    const unsigned int imageColumns,const unsigned int imageRows,
    __local float4 *temp,__global float4 *tempImage)
  {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    const int columns = imageColumns;

    const unsigned int radius = (width-1)/2;
    const int wsize = get_local_size(0);
    const unsigned int loadSize = wsize+width;

    //group coordinate
    const int groupX=get_local_size(0)*get_group_id(0);

    //parallel load and clamp
    for (int i=get_local_id(0); i < loadSize; i=i+get_local_size(0))
    {
      int cx = ClampToCanvas(i + groupX - radius, columns);
      temp[i] = ReadFloat4(image, number_channels, columns, cx, y, channel);
    }

    // barrier
    barrier(CLK_LOCAL_MEM_FENCE);

    // only do the work if this is not a patched item
    if (get_global_id(0) < columns)
    {
      // compute
      float4 result = (float4) 0;

      int i = 0;

      for ( ; i+7 < width; )
      {
        for (int j=0; j < 8; j++)
          result+=filter[i+j]*temp[i+j+get_local_id(0)];
        i+=8;
      }

      for ( ; i < width; i++)
        result+=filter[i]*temp[i+get_local_id(0)];

      // write back to global
      tempImage[y*columns+x] = result;
    }
  }
  )

  STRINGIFY(
  /*
  Reduce image noise and reduce detail levels by line
  */
  __kernel void BlurColumn(const __global float4 *blurRowData,
    const unsigned int number_channels,const ChannelType channel,
    __constant float *filter,const unsigned int width,
    const unsigned int imageColumns,const unsigned int imageRows,
    __local float4 *temp,__global CLQuantum *filteredImage)
  {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    const int columns = imageColumns;
    const int rows = imageRows;

    unsigned int radius = (width-1)/2;
    const int wsize = get_local_size(1);
    const unsigned int loadSize = wsize+width;

    //group coordinate
    const int groupX=get_local_size(0)*get_group_id(0);
    const int groupY=get_local_size(1)*get_group_id(1);
    //notice that get_local_size(0) is 1, so
    //groupX=get_group_id(0);

    //parallel load and clamp
    for (int i = get_local_id(1); i < loadSize; i=i+get_local_size(1))
      temp[i] = blurRowData[ClampToCanvas(i+groupY-radius, rows) * columns + groupX];

    // barrier
    barrier(CLK_LOCAL_MEM_FENCE);

    // only do the work if this is not a patched item
    if (get_global_id(1) < rows)
    {
      // compute
      float4 result = (float4) 0;

      int i = 0;

      for ( ; i+7 < width; )
      {
        for (int j=0; j < 8; j++)
          result+=filter[i+j]*temp[i+j+get_local_id(1)];
        i+=8;
      }

      for ( ; i < width; i++)
        result+=filter[i]*temp[i+get_local_id(1)];

      // write back to global
      WriteFloat4(filteredImage, number_channels, columns, x, y, channel, result);
    }
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    C o n t r a s t                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(

  static inline float4 ConvertRGBToHSB(const float4 pixel)
  {
    float4 result=0.0f;
    result.w=pixel.w;
    float tmax=MagickMax(MagickMax(pixel.x,pixel.y),pixel.z);
    if (tmax != 0.0f)
    {
      float tmin=MagickMin(MagickMin(pixel.x,pixel.y),pixel.z);
      float delta=tmax-tmin;

      result.y=delta/tmax;
      result.z=QuantumScale*tmax;
      if (delta != 0.0f)
      {
        result.x =((pixel.x == tmax) ? 0.0f : ((pixel.y == tmax) ? 2.0f : 4.0f));
        result.x+=((pixel.x == tmax) ? (pixel.y-pixel.z) : ((pixel.y == tmax) ?
          (pixel.z-pixel.x) : (pixel.x-pixel.y)))/delta;
        result.x/=6.0f;
        result.x+=(result.x < 0.0f) ? 0.0f : 1.0f;
      }
    }
    return(result);
  }

  static inline float4 ConvertHSBToRGB(const float4 pixel)
  {
    float hue=pixel.x;
    float saturation=pixel.y;
    float brightness=pixel.z;

    float4 result=pixel;

    if (saturation == 0.0f)
    {
      result.x=result.y=result.z=ClampToQuantum(QuantumRange*brightness);
    }
    else
    {
      float h=6.0f*(hue-floor(hue));
      float f=h-floor(h);
      float p=brightness*(1.0f-saturation);
      float q=brightness*(1.0f-saturation*f);
      float t=brightness*(1.0f-(saturation*(1.0f-f)));
      int ih = (int)h;

      if (ih == 1)
      {
        result.x=ClampToQuantum(QuantumRange*q);
        result.y=ClampToQuantum(QuantumRange*brightness);
        result.z=ClampToQuantum(QuantumRange*p);
      }
      else if (ih == 2)
      {
        result.x=ClampToQuantum(QuantumRange*p);
        result.y=ClampToQuantum(QuantumRange*brightness);
        result.z=ClampToQuantum(QuantumRange*t);
      }
      else if (ih == 3)
      {
        result.x=ClampToQuantum(QuantumRange*p);
        result.y=ClampToQuantum(QuantumRange*q);
        result.z=ClampToQuantum(QuantumRange*brightness);
      }
      else if (ih == 4)
      {
        result.x=ClampToQuantum(QuantumRange*t);
        result.y=ClampToQuantum(QuantumRange*p);
        result.z=ClampToQuantum(QuantumRange*brightness);
      }
      else if (ih == 5)
      {
        result.x=ClampToQuantum(QuantumRange*brightness);
        result.y=ClampToQuantum(QuantumRange*p);
        result.z=ClampToQuantum(QuantumRange*q);
      }
      else
      {
        result.x=ClampToQuantum(QuantumRange*brightness);
        result.y=ClampToQuantum(QuantumRange*t);
        result.z=ClampToQuantum(QuantumRange*p);
      }
    }
    return(result);
  }

  __kernel void Contrast(__global CLQuantum *image,
    const unsigned int number_channels,const int sign)
  {
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const unsigned int columns=get_global_size(0);

    float4 pixel=ReadAllChannels(image,number_channels,columns,x,y);
    if (number_channels < 3)
      pixel.y=pixel.z=pixel.x;

    pixel=ConvertRGBToHSB(pixel);
    float brightness=pixel.z;
    brightness+=0.5f*sign*(0.5f*(sinpi(brightness-0.5f)+1.0f)-brightness);
    brightness=clamp(brightness,0.0f,1.0f);
    pixel.z=brightness;
    pixel=ConvertHSBToRGB(pixel);

    WriteAllChannels(image,number_channels,columns,x,y,pixel);
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    C o n t r a s t S t r e t c h                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

    STRINGIFY(
    /*
    */
    __kernel void Histogram(__global CLPixelType * restrict im,
      const ChannelType channel, 
      const unsigned int colorspace,
      const unsigned int method,
      __global uint4 * restrict histogram)
      {
        const int x = get_global_id(0);  
        const int y = get_global_id(1);  
        const int columns = get_global_size(0);  
        const int c = x + y * columns;
        if ((channel & SyncChannels) != 0)
        {
          float red=(float)getRed(im[c]);
          float green=(float)getGreen(im[c]);
          float blue=(float)getBlue(im[c]);

          float intensity = GetPixelIntensity(colorspace, method, red, green, blue);
          uint pos = ScaleQuantumToMap(ClampToQuantum(intensity));
          atomic_inc((__global uint *)(&(histogram[pos]))+2); //red position
        }
        else
        {
          // for equalizing, we always need all channels?
          // otherwise something more
        }
      }
    )

    STRINGIFY(
    /*
    */
    __kernel void ContrastStretch(__global CLPixelType * restrict im,
      const ChannelType channel,  
      __global CLPixelType * restrict stretch_map,
      const float4 white, const float4 black)
      {
        const int x = get_global_id(0);  
        const int y = get_global_id(1);  
        const int columns = get_global_size(0);  
        const int c = x + y * columns;

        uint ePos;
        CLPixelType oValue, eValue;
        CLQuantum red, green, blue, alpha;

        //read from global
        oValue=im[c];

        if ((channel & RedChannel) != 0)
        {
          if (getRedF4(white) != getRedF4(black))
          {
            ePos = ScaleQuantumToMap(getRed(oValue)); 
            eValue = stretch_map[ePos];
            red = getRed(eValue);
          }
        }

        if ((channel & GreenChannel) != 0)
        {
          if (getGreenF4(white) != getGreenF4(black))
          {
            ePos = ScaleQuantumToMap(getGreen(oValue)); 
            eValue = stretch_map[ePos];
            green = getGreen(eValue);
          }
        }

        if ((channel & BlueChannel) != 0)
        {
          if (getBlueF4(white) != getBlueF4(black))
          {
            ePos = ScaleQuantumToMap(getBlue(oValue)); 
            eValue = stretch_map[ePos];
            blue = getBlue(eValue);
          }
        }

        if ((channel & AlphaChannel) != 0)
        {
          if (getAlphaF4(white) != getAlphaF4(black))
          {
            ePos = ScaleQuantumToMap(getAlpha(oValue)); 
            eValue = stretch_map[ePos];
            alpha = getAlpha(eValue);
          }
        }

        //write back
        im[c]=(CLPixelType)(blue, green, red, alpha);

      }
    )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    C o n v o l v e                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
    __kernel 
    void ConvolveOptimized(const __global CLPixelType *input, __global CLPixelType *output,
    const unsigned int imageWidth, const unsigned int imageHeight,
    __constant float *filter, const unsigned int filterWidth, const unsigned int filterHeight,
    const uint matte, const ChannelType channel, __local CLPixelType *pixelLocalCache, __local float* filterCache) {

      int2 blockID;
      blockID.x = get_global_id(0) / get_local_size(0);
      blockID.y = get_global_id(1) / get_local_size(1);

      // image area processed by this workgroup
      int2 imageAreaOrg;
      imageAreaOrg.x = blockID.x * get_local_size(0);
      imageAreaOrg.y = blockID.y * get_local_size(1);

      int2 midFilterDimen;
      midFilterDimen.x = (filterWidth-1)/2;
      midFilterDimen.y = (filterHeight-1)/2;

      int2 cachedAreaOrg = imageAreaOrg - midFilterDimen;

      // dimension of the local cache
      int2 cachedAreaDimen;
      cachedAreaDimen.x = get_local_size(0) + filterWidth - 1;
      cachedAreaDimen.y = get_local_size(1) + filterHeight - 1;

      // cache the pixels accessed by this workgroup in local memory
      int localID = get_local_id(1)*get_local_size(0)+get_local_id(0);
      int cachedAreaNumPixels = cachedAreaDimen.x * cachedAreaDimen.y;
      int groupSize = get_local_size(0) * get_local_size(1);
      for (int i = localID; i < cachedAreaNumPixels; i+=groupSize) {

        int2 cachedAreaIndex;
        cachedAreaIndex.x = i % cachedAreaDimen.x;
        cachedAreaIndex.y = i / cachedAreaDimen.x;

        int2 imagePixelIndex;
        imagePixelIndex = cachedAreaOrg + cachedAreaIndex;

        // only support EdgeVirtualPixelMethod through ClampToCanvas
        // TODO: implement other virtual pixel method
        imagePixelIndex.x = ClampToCanvas(imagePixelIndex.x, imageWidth);
        imagePixelIndex.y = ClampToCanvas(imagePixelIndex.y, imageHeight);

        pixelLocalCache[i] = input[imagePixelIndex.y * imageWidth + imagePixelIndex.x];
      }

      // cache the filter
      for (int i = localID; i < filterHeight*filterWidth; i+=groupSize) {
        filterCache[i] = filter[i];
      }
      barrier(CLK_LOCAL_MEM_FENCE);


      int2 imageIndex;
      imageIndex.x = imageAreaOrg.x + get_local_id(0);
      imageIndex.y = imageAreaOrg.y + get_local_id(1);

      // if out-of-range, stops here and quit
      if (imageIndex.x >= imageWidth
        || imageIndex.y >= imageHeight) {
          return;
      }

      int filterIndex = 0;
      float4 sum = (float4)0.0f;
      float gamma = 0.0f;
      if (((channel & AlphaChannel) == 0) || (matte == 0)) {
        int cacheIndexY = get_local_id(1);
        for (int j = 0; j < filterHeight; j++) {
          int cacheIndexX = get_local_id(0);
          for (int i = 0; i < filterWidth; i++) {
            CLPixelType p = pixelLocalCache[cacheIndexY*cachedAreaDimen.x + cacheIndexX];
            float f = filterCache[filterIndex];

            sum.x += f * p.x;
            sum.y += f * p.y;
            sum.z += f * p.z; 
            sum.w += f * p.w;

            gamma += f;
            filterIndex++;
            cacheIndexX++;
          }
          cacheIndexY++;
        }
      }
      else {
        int cacheIndexY = get_local_id(1);
        for (int j = 0; j < filterHeight; j++) {
          int cacheIndexX = get_local_id(0);
          for (int i = 0; i < filterWidth; i++) {

            CLPixelType p = pixelLocalCache[cacheIndexY*cachedAreaDimen.x + cacheIndexX];
            float alpha = QuantumScale*p.w;
            float f = filterCache[filterIndex];
            float g = alpha * f;

            sum.x += g*p.x;
            sum.y += g*p.y;
            sum.z += g*p.z;
            sum.w += f*p.w;

            gamma += g;
            filterIndex++;
            cacheIndexX++;
          }
          cacheIndexY++;
        }
        gamma = PerceptibleReciprocal(gamma);
        sum.xyz = gamma*sum.xyz;
      }
      CLPixelType outputPixel;
      outputPixel.x = ClampToQuantum(sum.x);
      outputPixel.y = ClampToQuantum(sum.y);
      outputPixel.z = ClampToQuantum(sum.z);
      outputPixel.w = ((channel & AlphaChannel)!=0)?ClampToQuantum(sum.w):input[imageIndex.y * imageWidth + imageIndex.x].w;

      output[imageIndex.y * imageWidth + imageIndex.x] = outputPixel;
    }
  )

  STRINGIFY(
    __kernel 
    void Convolve(const __global CLPixelType *input, __global CLPixelType *output,
                  const uint imageWidth, const uint imageHeight,
                  __constant float *filter, const unsigned int filterWidth, const unsigned int filterHeight,
                  const uint matte, const ChannelType channel) {

      int2 imageIndex;
      imageIndex.x = get_global_id(0);
      imageIndex.y = get_global_id(1);

      /*
      unsigned int imageWidth = get_global_size(0);
      unsigned int imageHeight = get_global_size(1);
      */
      if (imageIndex.x >= imageWidth
          || imageIndex.y >= imageHeight)
          return;

      int2 midFilterDimen;
      midFilterDimen.x = (filterWidth-1)/2;
      midFilterDimen.y = (filterHeight-1)/2;

      int filterIndex = 0;
      float4 sum = (float4)0.0f;
      float gamma = 0.0f;
      if (((channel & AlphaChannel) == 0) || (matte == 0)) {
        for (int j = 0; j < filterHeight; j++) {
          int2 inputPixelIndex;
          inputPixelIndex.y = imageIndex.y - midFilterDimen.y + j;
          inputPixelIndex.y = ClampToCanvas(inputPixelIndex.y, imageHeight);
          for (int i = 0; i < filterWidth; i++) {
            inputPixelIndex.x = imageIndex.x - midFilterDimen.x + i;
            inputPixelIndex.x = ClampToCanvas(inputPixelIndex.x, imageWidth);
        
            CLPixelType p = input[inputPixelIndex.y * imageWidth + inputPixelIndex.x];
            float f = filter[filterIndex];

            sum.x += f * p.x;
            sum.y += f * p.y;
            sum.z += f * p.z; 
            sum.w += f * p.w;

            gamma += f;

            filterIndex++;
          }
        }
      }
      else {

        for (int j = 0; j < filterHeight; j++) {
          int2 inputPixelIndex;
          inputPixelIndex.y = imageIndex.y - midFilterDimen.y + j;
          inputPixelIndex.y = ClampToCanvas(inputPixelIndex.y, imageHeight);
          for (int i = 0; i < filterWidth; i++) {
            inputPixelIndex.x = imageIndex.x - midFilterDimen.x + i;
            inputPixelIndex.x = ClampToCanvas(inputPixelIndex.x, imageWidth);
        
            CLPixelType p = input[inputPixelIndex.y * imageWidth + inputPixelIndex.x];
            float alpha = QuantumScale*p.w;
            float f = filter[filterIndex];
            float g = alpha * f;

            sum.x += g*p.x;
            sum.y += g*p.y;
            sum.z += g*p.z;
            sum.w += f*p.w;

            gamma += g;


            filterIndex++;
          }
        }
        gamma = PerceptibleReciprocal(gamma);
        sum.xyz = gamma*sum.xyz;
      }

      CLPixelType outputPixel;
      outputPixel.x = ClampToQuantum(sum.x);
      outputPixel.y = ClampToQuantum(sum.y);
      outputPixel.z = ClampToQuantum(sum.z);
      outputPixel.w = ((channel & AlphaChannel)!=0)?ClampToQuantum(sum.w):input[imageIndex.y * imageWidth + imageIndex.x].w;

      output[imageIndex.y * imageWidth + imageIndex.x] = outputPixel;
    }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D e s p e c k l e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(

  __kernel void HullPass1(const __global CLPixelType *inputImage, __global CLPixelType *outputImage
  , const unsigned int imageWidth, const unsigned int imageHeight
  , const int2 offset, const int polarity, const int matte) {

    int x = get_global_id(0);
    int y = get_global_id(1);

    CLPixelType v = inputImage[y*imageWidth+x];

    int2 neighbor;
    neighbor.y = y + offset.y;
    neighbor.x = x + offset.x;

    int2 clampedNeighbor;
    clampedNeighbor.x = ClampToCanvas(neighbor.x, imageWidth);
    clampedNeighbor.y = ClampToCanvas(neighbor.y, imageHeight);

    CLPixelType r = (clampedNeighbor.x == neighbor.x
                     && clampedNeighbor.y == neighbor.y)?inputImage[clampedNeighbor.y*imageWidth+clampedNeighbor.x]
    :(CLPixelType)0;

    int sv[4];
    sv[0] = (int)v.x;
    sv[1] = (int)v.y;
    sv[2] = (int)v.z;
    sv[3] = (int)v.w;

    int sr[4];
    sr[0] = (int)r.x;
    sr[1] = (int)r.y;
    sr[2] = (int)r.z;
    sr[3] = (int)r.w;

    if (polarity > 0) {
      \n #pragma unroll 4\n
      for (unsigned int i = 0; i < 4; i++) {
        sv[i] = (sr[i] >= (sv[i]+ScaleCharToQuantum(2)))?(sv[i]+ScaleCharToQuantum(1)):sv[i];
      }
    }
    else {
      \n #pragma unroll 4\n
      for (unsigned int i = 0; i < 4; i++) {
        sv[i] = (sr[i] <= (sv[i]-ScaleCharToQuantum(2)))?(sv[i]-ScaleCharToQuantum(1)):sv[i];
      }

    }

    v.x = (CLQuantum)sv[0];
    v.y = (CLQuantum)sv[1];
    v.z = (CLQuantum)sv[2];

    if (matte!=0)
      v.w = (CLQuantum)sv[3];

    outputImage[y*imageWidth+x] = v;

    }


  )

  STRINGIFY(

  __kernel void HullPass2(const __global CLPixelType *inputImage, __global CLPixelType *outputImage
  , const unsigned int imageWidth, const unsigned int imageHeight
  , const int2 offset, const int polarity, const int matte) {

    int x = get_global_id(0);
    int y = get_global_id(1);

    CLPixelType v = inputImage[y*imageWidth+x];

    int2 neighbor, clampedNeighbor;

    neighbor.y = y + offset.y;
    neighbor.x = x + offset.x;
    clampedNeighbor.x = ClampToCanvas(neighbor.x, imageWidth);
    clampedNeighbor.y = ClampToCanvas(neighbor.y, imageHeight);

    CLPixelType r = (clampedNeighbor.x == neighbor.x
      && clampedNeighbor.y == neighbor.y)?inputImage[clampedNeighbor.y*imageWidth+clampedNeighbor.x]
    :(CLPixelType)0;


    neighbor.y = y - offset.y;
    neighbor.x = x - offset.x;
    clampedNeighbor.x = ClampToCanvas(neighbor.x, imageWidth);
    clampedNeighbor.y = ClampToCanvas(neighbor.y, imageHeight);

    CLPixelType s = (clampedNeighbor.x == neighbor.x
      && clampedNeighbor.y == neighbor.y)?inputImage[clampedNeighbor.y*imageWidth+clampedNeighbor.x]
    :(CLPixelType)0;


    int sv[4];
    sv[0] = (int)v.x;
    sv[1] = (int)v.y;
    sv[2] = (int)v.z;
    sv[3] = (int)v.w;

    int sr[4];
    sr[0] = (int)r.x;
    sr[1] = (int)r.y;
    sr[2] = (int)r.z;
    sr[3] = (int)r.w;

    int ss[4];
    ss[0] = (int)s.x;
    ss[1] = (int)s.y;
    ss[2] = (int)s.z;
    ss[3] = (int)s.w;

    if (polarity > 0) {
      \n #pragma unroll 4\n
      for (unsigned int i = 0; i < 4; i++) {
        //sv[i] = (ss[i] >= (sv[i]+ScaleCharToQuantum(2)) && sr[i] > sv[i] )   ? (sv[i]+ScaleCharToQuantum(1)):sv[i];
        //
        //sv[i] =(!( (int)(ss[i] >= (sv[i]+ScaleCharToQuantum(2))) && (int) (sr[i] > sv[i] ) ))  ? sv[i]:(sv[i]+ScaleCharToQuantum(1));
        //sv[i] =(( (int)( ss[i] < (sv[i]+ScaleCharToQuantum(2))) || (int) ( sr[i] <= sv[i] ) ))  ? sv[i]:(sv[i]+ScaleCharToQuantum(1));
        sv[i] =(( (int)( ss[i] < (sv[i]+ScaleCharToQuantum(2))) + (int) ( sr[i] <= sv[i] ) ) !=0)  ? sv[i]:(sv[i]+ScaleCharToQuantum(1));
      }
    }
    else {
      \n #pragma unroll 4\n
      for (unsigned int i = 0; i < 4; i++) {
        //sv[i] = (ss[i] <= (sv[i]-ScaleCharToQuantum(2)) && sr[i] < sv[i] )   ? (sv[i]-ScaleCharToQuantum(1)):sv[i];
        //
        //sv[i] = ( (int)(ss[i] <= (sv[i]-ScaleCharToQuantum(2)) ) + (int)( sr[i] < sv[i] ) ==0)   ? sv[i]:(sv[i]-ScaleCharToQuantum(1));
        sv[i] = (( (int)(ss[i] > (sv[i]-ScaleCharToQuantum(2))) + (int)( sr[i] >= sv[i] )) !=0)   ? sv[i]:(sv[i]-ScaleCharToQuantum(1));
      }
    }

    v.x = (CLQuantum)sv[0];
    v.y = (CLQuantum)sv[1];
    v.z = (CLQuantum)sv[2];

    if (matte!=0)
      v.w = (CLQuantum)sv[3];

    outputImage[y*imageWidth+x] = v;

    }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E q u a l i z e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

    STRINGIFY(
    /*
    */
    __kernel void Equalize(__global CLPixelType * restrict im,
      const ChannelType channel,  
      __global CLPixelType * restrict equalize_map,
      const float4 white, const float4 black)
      {
        const int x = get_global_id(0);  
        const int y = get_global_id(1);  
        const int columns = get_global_size(0);  
        const int c = x + y * columns;

        uint ePos;
        CLPixelType oValue, eValue;
        CLQuantum red, green, blue, alpha;

        //read from global
        oValue=im[c];

        if ((channel & SyncChannels) != 0)
        {
          if (getRedF4(white) != getRedF4(black))
          {
            ePos = ScaleQuantumToMap(getRed(oValue)); 
            eValue = equalize_map[ePos];
            red = getRed(eValue);
            ePos = ScaleQuantumToMap(getGreen(oValue)); 
            eValue = equalize_map[ePos];
            green = getRed(eValue);
            ePos = ScaleQuantumToMap(getBlue(oValue)); 
            eValue = equalize_map[ePos];
            blue = getRed(eValue);
            ePos = ScaleQuantumToMap(getAlpha(oValue)); 
            eValue = equalize_map[ePos];
            alpha = getRed(eValue);
 
            //write back
            im[c]=(CLPixelType)(blue, green, red, alpha);
          }

        }

        // for equalizing, we always need all channels?
        // otherwise something more

     }
    )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     F u n c t i o n                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
  /*
  apply FunctionImageChannel(braightness-contrast)
  */
  CLQuantum ApplyFunction(float pixel,const MagickFunction function,
    const unsigned int number_parameters,__constant float *parameters)
  {
    float result = 0.0f;

    switch (function)
    {
    case PolynomialFunction:
      {
        for (unsigned int i=0; i < number_parameters; i++)
          result = result*QuantumScale*pixel + parameters[i];
        result *= QuantumRange;
        break;
      }
    case SinusoidFunction:
      {
        float  freq,phase,ampl,bias;
        freq  = ( number_parameters >= 1 ) ? parameters[0] : 1.0f;
        phase = ( number_parameters >= 2 ) ? parameters[1] : 0.0f;
        ampl  = ( number_parameters >= 3 ) ? parameters[2] : 0.5f;
        bias  = ( number_parameters >= 4 ) ? parameters[3] : 0.5f;
        result = QuantumRange*(ampl*sin(2.0f*MagickPI*
          (freq*QuantumScale*pixel + phase/360.0f)) + bias);
        break;
      }
    case ArcsinFunction:
      {
        float  width,range,center,bias;
        width  = ( number_parameters >= 1 ) ? parameters[0] : 1.0f;
        center = ( number_parameters >= 2 ) ? parameters[1] : 0.5f;
        range  = ( number_parameters >= 3 ) ? parameters[2] : 1.0f;
        bias   = ( number_parameters >= 4 ) ? parameters[3] : 0.5f;

        result = 2.0f/width*(QuantumScale*pixel - center);
        result = range/MagickPI*asin(result)+bias;
        result = ( result <= -1.0f ) ? bias - range/2.0f : result;
        result = ( result >= 1.0f ) ? bias + range/2.0f : result;
        result *= QuantumRange;
        break;
      }
    case ArctanFunction:
      {
        float slope,range,center,bias;
        slope  = ( number_parameters >= 1 ) ? parameters[0] : 1.0f;
        center = ( number_parameters >= 2 ) ? parameters[1] : 0.5f;
        range  = ( number_parameters >= 3 ) ? parameters[2] : 1.0f;
        bias   = ( number_parameters >= 4 ) ? parameters[3] : 0.5f;
        result = MagickPI*slope*(QuantumScale*pixel-center);
        result = QuantumRange*(range/MagickPI*atan(result) + bias);
        break;
      }
    case UndefinedFunction:
      break;
    }
    return(ClampToQuantum(result));
  }
  )

  STRINGIFY(
  /*
  Improve brightness / contrast of the image
  channel : define which channel is improved
  function : the function called to enchance the brightness contrast
  number_parameters : numbers of parameters 
  parameters : the parameter
  */
  __kernel void ComputeFunction(__global CLQuantum *image,const unsigned int number_channels,
    const ChannelType channel,const MagickFunction function,const unsigned int number_parameters,
    __constant float *parameters)
  {
    const unsigned int x = get_global_id(0);
    const unsigned int y = get_global_id(1);
    const unsigned int columns = get_global_size(0);
    __global CLQuantum *p = image + getPixelIndex(number_channels, columns, x, y);

    float red;
    float green;
    float blue;
    float alpha;

    ReadChannels(p, number_channels, channel, &red, &green, &blue, &alpha);

    if ((channel & RedChannel) != 0)
      red=ApplyFunction(red, function, number_parameters, parameters);

    if (number_channels > 2)
      {
        if ((channel & GreenChannel) != 0)
          green=ApplyFunction(green, function, number_parameters, parameters);

        if ((channel & BlueChannel) != 0)
          blue=ApplyFunction(blue, function, number_parameters, parameters);
      }

    if (((number_channels == 4) || (number_channels == 2)) &&
        ((channel & AlphaChannel) != 0))
      alpha=ApplyFunction(alpha, function, number_parameters, parameters);

    WriteChannels(p, number_channels, channel, red, green, blue, alpha);
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     G r a y s c a l e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
  __kernel void Grayscale(__global CLQuantum *image,const int number_channels,
    const unsigned int colorspace,const unsigned int method)
  {
    const unsigned int x = get_global_id(0);
    const unsigned int y = get_global_id(1);
    const unsigned int columns = get_global_size(0);
    __global CLQuantum *p = image + getPixelIndex(number_channels, columns, x, y);

    float
      blue,
      green,
      red;

    red=getPixelRed(p);
    green=getPixelGreen(p);
    blue=getPixelBlue(p);

    CLQuantum intensity=ClampToQuantum(GetPixelIntensity(colorspace, method, red, green, blue));

    setPixelRed(p,intensity);
    setPixelGreen(p,intensity);
    setPixelBlue(p,intensity);
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L o c a l C o n t r a s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

    STRINGIFY(

      __kernel void LocalContrastBlurRow(__global CLPixelType *srcImage, __global CLPixelType *dstImage, __global float *tmpImage,
          const int radius, 
          const int imageWidth,
          const int imageHeight)
      {
        const float4 RGB = ((float4)(0.2126f, 0.7152f, 0.0722f, 0.0f));

        int x = get_local_id(0);
        int y = get_global_id(1);

        if ((x >= imageWidth) || (y >= imageHeight))
          return;

        global CLPixelType *src = srcImage + y * imageWidth;

        for (int i = x; i < imageWidth; i += get_local_size(0)) {
            float sum = 0.0f;
            float weight = 1.0f;

            int j = i - radius;
            while ((j + 7) < i) {
                for (int k = 0; k < 8; ++k) // Unroll 8x
                    sum += (weight + k) * dot(RGB, convert_float4(src[mirrorBottom(j+k)]));
                weight += 8.0f;
                j+=8;
            }
            while (j < i) {
                sum += weight * dot(RGB, convert_float4(src[mirrorBottom(j)]));
                weight += 1.0f;
                ++j;
            }

            while ((j + 7) < radius + i) {
                for (int k = 0; k < 8; ++k) // Unroll 8x
                    sum += (weight - k) * dot(RGB, convert_float4(src[mirrorTop(j + k, imageWidth)]));
                weight -= 8.0f;
                j+=8;
            }
            while (j < radius + i) {
                sum += weight * dot(RGB, convert_float4(src[mirrorTop(j, imageWidth)]));
                weight -= 1.0f;
                ++j;
            }

            tmpImage[i + y * imageWidth] = sum / ((radius + 1) * (radius + 1));
        }
      }
    )

    STRINGIFY(
      __kernel void LocalContrastBlurApplyColumn(__global CLPixelType *srcImage, __global CLPixelType *dstImage, __global float *blurImage,
          const int radius, 
          const float strength,
          const int imageWidth,
          const int imageHeight)
      {
        const float4 RGB = (float4)(0.2126f, 0.7152f, 0.0722f, 0.0f);

        int x = get_global_id(0);
        int y = get_global_id(1);

        if ((x >= imageWidth) || (y >= imageHeight))
                return;

        global float *src = blurImage + x;

        float sum = 0.0f;
        float weight = 1.0f;

        int j = y - radius;
        while ((j + 7) < y) {
            for (int k = 0; k < 8; ++k) // Unroll 8x
                sum += (weight + k) * src[mirrorBottom(j+k) * imageWidth];
            weight += 8.0f;
            j+=8;
        }
        while (j < y) {
            sum += weight * src[mirrorBottom(j) * imageWidth];
            weight += 1.0f;
            ++j;
        }

        while ((j + 7) < radius + y) {
            for (int k = 0; k < 8; ++k) // Unroll 8x
                sum += (weight - k) * src[mirrorTop(j + k, imageHeight) * imageWidth];
            weight -= 8.0f;
            j+=8;
        }
        while (j < radius + y) {
            sum += weight * src[mirrorTop(j, imageHeight) * imageWidth];
            weight -= 1.0f;
            ++j;
        }

        CLPixelType pixel = srcImage[x + y * imageWidth];
        float srcVal = dot(RGB, convert_float4(pixel));
        float mult = (srcVal - (sum / ((radius + 1) * (radius + 1)))) * (strength / 100.0f);
        mult = (srcVal + mult) / srcVal;

        pixel.x = ClampToQuantum(pixel.x * mult);
        pixel.y = ClampToQuantum(pixel.y * mult);
        pixel.z = ClampToQuantum(pixel.z * mult);

        dstImage[x + y * imageWidth] = pixel;
      }
    )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o d u l a t e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(

  static inline void ConvertRGBToHSL(const CLQuantum red,const CLQuantum green, const CLQuantum blue,
    float *hue, float *saturation, float *lightness)
  {
  float
    c,
    tmax,
    tmin;

  /*
     Convert RGB to HSL colorspace.
     */
  tmax=MagickMax(QuantumScale*red,MagickMax(QuantumScale*green, QuantumScale*blue));
  tmin=MagickMin(QuantumScale*red,MagickMin(QuantumScale*green, QuantumScale*blue));

  c=tmax-tmin;

  *lightness=(tmax+tmin)/2.0;
  if (c <= 0.0)
  {
    *hue=0.0;
    *saturation=0.0;
    return;
  }

  if (tmax == (QuantumScale*red))
  {
    *hue=(QuantumScale*green-QuantumScale*blue)/c;
    if ((QuantumScale*green) < (QuantumScale*blue))
      *hue+=6.0;
  }
  else
    if (tmax == (QuantumScale*green))
      *hue=2.0+(QuantumScale*blue-QuantumScale*red)/c;
    else
      *hue=4.0+(QuantumScale*red-QuantumScale*green)/c;

  *hue*=60.0/360.0;
  if (*lightness <= 0.5)
    *saturation=c/(2.0*(*lightness));
  else
    *saturation=c/(2.0-2.0*(*lightness));
  }

  static inline void ConvertHSLToRGB(const float hue,const float saturation, const float lightness,
      CLQuantum *red,CLQuantum *green,CLQuantum *blue)
  {
    float
      b,
      c,
      g,
      h,
      tmin,
      r,
      x;

    /*
       Convert HSL to RGB colorspace.
       */
    h=hue*360.0;
    if (lightness <= 0.5)
      c=2.0*lightness*saturation;
    else
      c=(2.0-2.0*lightness)*saturation;
    tmin=lightness-0.5*c;
    h-=360.0*floor(h/360.0);
    h/=60.0;
    x=c*(1.0-fabs(h-2.0*floor(h/2.0)-1.0));
    switch ((int) floor(h) % 6)
    {
      case 0:
        {
          r=tmin+c;
          g=tmin+x;
          b=tmin;
          break;
        }
      case 1:
        {
          r=tmin+x;
          g=tmin+c;
          b=tmin;
          break;
        }
      case 2:
        {
          r=tmin;
          g=tmin+c;
          b=tmin+x;
          break;
        }
      case 3:
        {
          r=tmin;
          g=tmin+x;
          b=tmin+c;
          break;
        }
      case 4:
        {
          r=tmin+x;
          g=tmin;
          b=tmin+c;
          break;
        }
      case 5:
        {
          r=tmin+c;
          g=tmin;
          b=tmin+x;
          break;
        }
      default:
        {
          r=0.0;
          g=0.0;
          b=0.0;
        }
    }
    *red=ClampToQuantum(QuantumRange*r);
    *green=ClampToQuantum(QuantumRange*g);
    *blue=ClampToQuantum(QuantumRange*b);
  }

  static inline void ModulateHSL(const float percent_hue, const float percent_saturation,const float percent_lightness, 
    CLQuantum *red,CLQuantum *green,CLQuantum *blue)
  {
    float
      hue,
      lightness,
      saturation;

    /*
    Increase or decrease color lightness, saturation, or hue.
    */
    ConvertRGBToHSL(*red,*green,*blue,&hue,&saturation,&lightness);
    hue+=0.5*(0.01*percent_hue-1.0);
    while (hue < 0.0)
      hue+=1.0;
    while (hue >= 1.0)
      hue-=1.0;
    saturation*=0.01*percent_saturation;
    lightness*=0.01*percent_lightness;
    ConvertHSLToRGB(hue,saturation,lightness,red,green,blue);
  }

  __kernel void Modulate(__global CLPixelType *im, 
    const float percent_brightness, 
    const float percent_hue, 
    const float percent_saturation, 
    const int colorspace)
  {

    const int x = get_global_id(0);  
    const int y = get_global_id(1);
    const int columns = get_global_size(0);
    const int c = x + y * columns;

    CLPixelType pixel = im[c];

    CLQuantum
        blue,
        green,
        red;

    red=getRed(pixel);
    green=getGreen(pixel);
    blue=getBlue(pixel);

    switch (colorspace)
    {
      case HSLColorspace:
      default:
        {
          ModulateHSL(percent_hue, percent_saturation, percent_brightness, 
              &red, &green, &blue);
        }

    }

    CLPixelType filteredPixel;
   
    setRed(&filteredPixel, red);
    setGreen(&filteredPixel, green);
    setBlue(&filteredPixel, blue);
    filteredPixel.w = pixel.w;

    im[c] = filteredPixel;
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o t i o n B l u r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
    __kernel 
    void MotionBlur(const __global CLPixelType *input, __global CLPixelType *output,
                    const unsigned int imageWidth, const unsigned int imageHeight,
                    const __global float *filter, const unsigned int width, const __global int2* offset,
                    const float4 bias,
                    const ChannelType channel, const unsigned int matte) {

      int2 currentPixel;
      currentPixel.x = get_global_id(0);
      currentPixel.y = get_global_id(1);

      if (currentPixel.x >= imageWidth
          || currentPixel.y >= imageHeight)
          return;

      float4 pixel;
      pixel.x = (float)bias.x;
      pixel.y = (float)bias.y;
      pixel.z = (float)bias.z;
      pixel.w = (float)bias.w;

      if (((channel & AlphaChannel) == 0) || (matte == 0)) {
        
        for (int i = 0; i < width; i++) {
          // only support EdgeVirtualPixelMethod through ClampToCanvas
          // TODO: implement other virtual pixel method
          int2 samplePixel = currentPixel + offset[i];
          samplePixel.x = ClampToCanvas(samplePixel.x, imageWidth);
          samplePixel.y = ClampToCanvas(samplePixel.y, imageHeight);
          CLPixelType samplePixelValue = input[ samplePixel.y * imageWidth + samplePixel.x];

          pixel.x += (filter[i] * (float)samplePixelValue.x);
          pixel.y += (filter[i] * (float)samplePixelValue.y);
          pixel.z += (filter[i] * (float)samplePixelValue.z);
          pixel.w += (filter[i] * (float)samplePixelValue.w);
        }

        CLPixelType outputPixel;
        outputPixel.x = ClampToQuantum(pixel.x);
        outputPixel.y = ClampToQuantum(pixel.y);
        outputPixel.z = ClampToQuantum(pixel.z);
        outputPixel.w = ClampToQuantum(pixel.w);
        output[currentPixel.y * imageWidth + currentPixel.x] = outputPixel;
      }
      else {

        float gamma = 0.0f;
        for (int i = 0; i < width; i++) {
          // only support EdgeVirtualPixelMethod through ClampToCanvas
          // TODO: implement other virtual pixel method
          int2 samplePixel = currentPixel + offset[i];
          samplePixel.x = ClampToCanvas(samplePixel.x, imageWidth);
          samplePixel.y = ClampToCanvas(samplePixel.y, imageHeight);

          CLPixelType samplePixelValue = input[ samplePixel.y * imageWidth + samplePixel.x];

          float alpha = QuantumScale*samplePixelValue.w;
          float k = filter[i];
          pixel.x = pixel.x + k * alpha * samplePixelValue.x;
          pixel.y = pixel.y + k * alpha * samplePixelValue.y;
          pixel.z = pixel.z + k * alpha * samplePixelValue.z;

          pixel.w += k * alpha * samplePixelValue.w;

          gamma+=k*alpha;
        }
        gamma = PerceptibleReciprocal(gamma);
        pixel.xyz = gamma*pixel.xyz;

        CLPixelType outputPixel;
        outputPixel.x = ClampToQuantum(pixel.x);
        outputPixel.y = ClampToQuantum(pixel.y);
        outputPixel.z = ClampToQuantum(pixel.z);
        outputPixel.w = ClampToQuantum(pixel.w);
        output[currentPixel.y * imageWidth + currentPixel.x] = outputPixel;
      }
    }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R e s i z e                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
  // Based on Box from resize.c
  float BoxResizeFilter(const float x)
  {
    return 1.0f;
  }
  )
    
  STRINGIFY(
  // Based on CubicBC from resize.c
  float CubicBC(const float x,const __global float* resizeFilterCoefficients)
  {
    /*
    Cubic Filters using B,C determined values:
    Mitchell-Netravali  B = 1/3 C = 1/3  "Balanced" cubic spline filter
    Catmull-Rom         B = 0   C = 1/2  Interpolatory and exact on linears
    Spline              B = 1   C = 0    B-Spline Gaussian approximation
    Hermite             B = 0   C = 0    B-Spline interpolator

    See paper by Mitchell and Netravali, Reconstruction Filters in Computer
    Graphics Computer Graphics, Volume 22, Number 4, August 1988
    http://www.cs.utexas.edu/users/fussell/courses/cs384g/lectures/mitchell/
    Mitchell.pdf.

    Coefficents are determined from B,C values:
    P0 = (  6 - 2*B       )/6 = coeff[0]
    P1 =         0
    P2 = (-18 +12*B + 6*C )/6 = coeff[1]
    P3 = ( 12 - 9*B - 6*C )/6 = coeff[2]
    Q0 = (      8*B +24*C )/6 = coeff[3]
    Q1 = (    -12*B -48*C )/6 = coeff[4]
    Q2 = (      6*B +30*C )/6 = coeff[5]
    Q3 = (    - 1*B - 6*C )/6 = coeff[6]

    which are used to define the filter:

    P0 + P1*x + P2*x^2 + P3*x^3      0 <= x < 1
    Q0 + Q1*x + Q2*x^2 + Q3*x^3      1 <= x < 2

    which ensures function is continuous in value and derivative (slope).
    */
    if (x < 1.0)
      return(resizeFilterCoefficients[0]+x*(x*
      (resizeFilterCoefficients[1]+x*resizeFilterCoefficients[2])));
    if (x < 2.0)
      return(resizeFilterCoefficients[3]+x*(resizeFilterCoefficients[4]+x*
      (resizeFilterCoefficients[5]+x*resizeFilterCoefficients[6])));
    return(0.0);
  }
  )

  STRINGIFY(
  float Sinc(const float x)
  {
    if (x != 0.0f)
    {
      const float alpha=(float) (MagickPI*x);
      return sinpi(x)/alpha;
    }
    return(1.0f);
  }
  )

  STRINGIFY(
  float Triangle(const float x)
  {
    /*
    1st order (linear) B-Spline, bilinear interpolation, Tent 1D filter, or
    a Bartlett 2D Cone filter.  Also used as a Bartlett Windowing function
    for Sinc().
    */
    return ((x<1.0f)?(1.0f-x):0.0f);
  }
  )


  STRINGIFY(
  float Hann(const float x)
  {
    /*
    Cosine window function:
      0.5+0.5*cos(pi*x).
    */
    const float cosine=cos((MagickPI*x));
    return(0.5f+0.5f*cosine);
  }
  )

  STRINGIFY(
  float Hamming(const float x)
  {
    /*
      Offset cosine window function:
       .54 + .46 cos(pi x).
    */
    const float cosine=cos((MagickPI*x));
    return(0.54f+0.46f*cosine);
  }
  )

  STRINGIFY(
  float Blackman(const float x)
  {
    /*
      Blackman: 2nd order cosine windowing function:
        0.42 + 0.5 cos(pi x) + 0.08 cos(2pi x)

      Refactored by Chantal Racette and Nicolas Robidoux to one trig call and
      five flops.
    */
    const float cosine=cos((MagickPI*x));
    return(0.34f+cosine*(0.5f+cosine*0.16f));
  }
  )

  STRINGIFY(
  static inline float applyResizeFilter(const float x, const ResizeWeightingFunctionType filterType, const __global float* filterCoefficients)
  {
    switch (filterType)
    {
    /* Call Sinc even for SincFast to get better precision on GPU 
       and to avoid thread divergence.  Sinc is pretty fast on GPU anyway...*/
    case SincWeightingFunction:
    case SincFastWeightingFunction:
      return Sinc(x);
    case CubicBCWeightingFunction:
      return CubicBC(x,filterCoefficients);
    case BoxWeightingFunction:
      return BoxResizeFilter(x);
    case TriangleWeightingFunction:
      return Triangle(x);
    case HannWeightingFunction:
      return Hann(x);
    case HammingWeightingFunction:
      return Hamming(x);
    case BlackmanWeightingFunction:
      return Blackman(x);

    default:
      return 0.0f;
    }
  }
  )


  STRINGIFY(
  static inline float getResizeFilterWeight(const __global float* resizeFilterCubicCoefficients, const ResizeWeightingFunctionType resizeFilterType
           , const ResizeWeightingFunctionType resizeWindowType
           , const float resizeFilterScale, const float resizeWindowSupport, const float resizeFilterBlur, const float x)
  {
    float scale;
    float xBlur = fabs(x/resizeFilterBlur);
    if (resizeWindowSupport < MagickEpsilon
        || resizeWindowType == BoxWeightingFunction)
    {
      scale = 1.0f;
    }
    else
    {
      scale = resizeFilterScale;
      scale = applyResizeFilter(xBlur*scale, resizeWindowType, resizeFilterCubicCoefficients);
    }
    float weight = scale * applyResizeFilter(xBlur, resizeFilterType, resizeFilterCubicCoefficients);
    return weight;
  }

  )

  ;
  const char *accelerateKernels2 =

  STRINGIFY(

  static inline unsigned int getNumWorkItemsPerPixel(const unsigned int pixelPerWorkgroup, const unsigned int numWorkItems) {
    return (numWorkItems/pixelPerWorkgroup);
  }

  // returns the index of the pixel for the current workitem to compute.
  // returns -1 if this workitem doesn't need to participate in any computation
  static inline int pixelToCompute(const unsigned itemID, const unsigned int pixelPerWorkgroup, const unsigned int numWorkItems) {
    const unsigned int numWorkItemsPerPixel = getNumWorkItemsPerPixel(pixelPerWorkgroup, numWorkItems);
    int pixelIndex = itemID/numWorkItemsPerPixel;
    pixelIndex = (pixelIndex<pixelPerWorkgroup)?pixelIndex:-1;
    return pixelIndex;
  }
 
  )

  STRINGIFY(
  __kernel __attribute__((reqd_work_group_size(256, 1, 1)))
    void ResizeHorizontalFilter(const __global CLQuantum *inputImage, const unsigned int number_channels,
      const unsigned int inputColumns, const unsigned int inputRows, __global CLQuantum *filteredImage,
      const unsigned int filteredColumns, const unsigned int filteredRows, const float xFactor,
      const int resizeFilterType, const int resizeWindowType, const __global float *resizeFilterCubicCoefficients,
      const float resizeFilterScale, const float resizeFilterSupport, const float resizeFilterWindowSupport,
      const float resizeFilterBlur, __local CLQuantum *inputImageCache, const int numCachedPixels,
      const unsigned int pixelPerWorkgroup, const unsigned int pixelChunkSize,
      __local float4 *outputPixelCache, __local float *densityCache, __local float *gammaCache)
  {
    // calculate the range of resized image pixels computed by this workgroup
    const unsigned int startX = get_group_id(0)*pixelPerWorkgroup;
    const unsigned int stopX = MagickMin(startX + pixelPerWorkgroup,filteredColumns);
    const unsigned int actualNumPixelToCompute = stopX - startX;

    // calculate the range of input image pixels to cache
    float scale = MagickMax(1.0f/xFactor+MagickEpsilon ,1.0f);
    const float support = MagickMax(scale*resizeFilterSupport,0.5f);
    scale = PerceptibleReciprocal(scale);

    const int cacheRangeStartX = MagickMax((int)((startX+0.5f)/xFactor+MagickEpsilon-support+0.5f),(int)(0));
    const int cacheRangeEndX = MagickMin((int)(cacheRangeStartX + numCachedPixels), (int)inputColumns);

    // cache the input pixels into local memory
    const unsigned int y = get_global_id(1);
    const unsigned int pos = getPixelIndex(number_channels, inputColumns, cacheRangeStartX, y);
    const unsigned int num_elements = (cacheRangeEndX - cacheRangeStartX) * number_channels;
    event_t e = async_work_group_copy(inputImageCache, inputImage + pos, num_elements, 0);
    wait_group_events(1, &e);

    unsigned int alpha_index = (number_channels == 4) || (number_channels == 2) ? number_channels - 1 : 0;
    unsigned int totalNumChunks = (actualNumPixelToCompute+pixelChunkSize-1)/pixelChunkSize;
    for (unsigned int chunk = 0; chunk < totalNumChunks; chunk++)
    {
      const unsigned int chunkStartX = startX + chunk*pixelChunkSize;
      const unsigned int chunkStopX = MagickMin(chunkStartX + pixelChunkSize, stopX);
      const unsigned int actualNumPixelInThisChunk = chunkStopX - chunkStartX;

      // determine which resized pixel computed by this workitem
      const unsigned int itemID = get_local_id(0);
      const unsigned int numItems = getNumWorkItemsPerPixel(actualNumPixelInThisChunk, get_local_size(0));

      const int pixelIndex = pixelToCompute(itemID, actualNumPixelInThisChunk, get_local_size(0));

      float4 filteredPixel = (float4)0.0f;
      float density = 0.0f;
      float gamma = 0.0f;
      // -1 means this workitem doesn't participate in the computation
      if (pixelIndex != -1)
      {
        // x coordinated of the resized pixel computed by this workitem
        const int x = chunkStartX + pixelIndex;

        // calculate how many steps required for this pixel
        const float bisect = (x+0.5)/xFactor+MagickEpsilon;
        const unsigned int start = (unsigned int)MagickMax(bisect-support+0.5f,0.0f);
        const unsigned int stop  = (unsigned int)MagickMin(bisect+support+0.5f,(float)inputColumns);
        const unsigned int n = stop - start;

        // calculate how many steps this workitem will contribute
        unsigned int numStepsPerWorkItem = n / numItems;
        numStepsPerWorkItem += ((numItems*numStepsPerWorkItem)==n?0:1);

        const unsigned int startStep = (itemID%numItems)*numStepsPerWorkItem;
        if (startStep < n)
        {
          const unsigned int stopStep = MagickMin(startStep+numStepsPerWorkItem, n);

          unsigned int cacheIndex = start+startStep-cacheRangeStartX;
          for (unsigned int i = startStep; i < stopStep; i++, cacheIndex++)
          {
            float weight = getResizeFilterWeight(resizeFilterCubicCoefficients,
              (ResizeWeightingFunctionType) resizeFilterType,
              (ResizeWeightingFunctionType) resizeWindowType,
              resizeFilterScale, resizeFilterWindowSupport,
              resizeFilterBlur, scale*(start + i - bisect + 0.5));

            float4 cp = (float4)0.0f;

            __local CLQuantum *p = inputImageCache + (cacheIndex*number_channels);
            cp.x = (float) *(p);
            if (number_channels > 2)
            {
              cp.y = (float) *(p + 1);
              cp.z = (float) *(p + 2);
            }

            if (alpha_index != 0)
            {
              cp.w = (float) *(p + alpha_index);

              float alpha = weight * QuantumScale * cp.w;

              filteredPixel.x += alpha * cp.x;
              filteredPixel.y += alpha * cp.y;
              filteredPixel.z += alpha * cp.z;
              filteredPixel.w += weight * cp.w;
              gamma += alpha;
            }
            else
              filteredPixel += ((float4) weight)*cp;

            density += weight;
          }
        }
      }

      // initialize the accumulators to zero
      if (itemID < actualNumPixelInThisChunk) {
        outputPixelCache[itemID] = (float4)0.0f;
        densityCache[itemID] = 0.0f;
        if (alpha_index != 0)
          gammaCache[itemID] = 0.0f;
      }
      barrier(CLK_LOCAL_MEM_FENCE);

      // accumulatte the filtered pixel value and the density
      for (unsigned int i = 0; i < numItems; i++) {
        if (pixelIndex != -1) {
          if (itemID%numItems == i) {
            outputPixelCache[pixelIndex]+=filteredPixel;
            densityCache[pixelIndex]+=density;
            if (alpha_index != 0)
              gammaCache[pixelIndex]+=gamma;
          }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
      }

      if (itemID < actualNumPixelInThisChunk)
      {
        float4 filteredPixel = outputPixelCache[itemID];

        float gamma = 0.0f;
        if (alpha_index != 0)
          gamma = gammaCache[itemID];

        float density = densityCache[itemID];
        if ((density != 0.0f) && (density != 1.0f))
        {
          density = PerceptibleReciprocal(density);
          filteredPixel *= (float4) density;
          if (alpha_index != 0)
            gamma *= density;
        }

        if (alpha_index != 0)
        {
          gamma = PerceptibleReciprocal(gamma);
          filteredPixel.x *= gamma;
          filteredPixel.y *= gamma;
          filteredPixel.z *= gamma;
        }

        WriteAllChannels(filteredImage, number_channels, filteredColumns, chunkStartX + itemID, y, filteredPixel);
      }
    }
  }
  )


  STRINGIFY(
 __kernel __attribute__((reqd_work_group_size(1, 256, 1)))
    void ResizeVerticalFilter(const __global CLQuantum *inputImage, const unsigned int number_channels,
      const unsigned int inputColumns, const unsigned int inputRows, __global CLQuantum *filteredImage,
      const unsigned int filteredColumns, const unsigned int filteredRows, const float yFactor,
      const int resizeFilterType, const int resizeWindowType, const __global float *resizeFilterCubicCoefficients,
      const float resizeFilterScale, const float resizeFilterSupport, const float resizeFilterWindowSupport,
      const float resizeFilterBlur, __local CLQuantum *inputImageCache, const int numCachedPixels,
      const unsigned int pixelPerWorkgroup, const unsigned int pixelChunkSize,
      __local float4 *outputPixelCache, __local float *densityCache, __local float *gammaCache)
  {
    // calculate the range of resized image pixels computed by this workgroup
    const unsigned int startY = get_group_id(1)*pixelPerWorkgroup;
    const unsigned int stopY = MagickMin(startY + pixelPerWorkgroup,filteredRows);
    const unsigned int actualNumPixelToCompute = stopY - startY;

    // calculate the range of input image pixels to cache
    float scale = MagickMax(1.0f/yFactor+MagickEpsilon ,1.0f);
    const float support = MagickMax(scale*resizeFilterSupport,0.5f);
    scale = PerceptibleReciprocal(scale);

    const int cacheRangeStartY = MagickMax((int)((startY+0.5f)/yFactor+MagickEpsilon-support+0.5f),(int)(0));
    const int cacheRangeEndY = MagickMin((int)(cacheRangeStartY + numCachedPixels), (int)inputRows);

    // cache the input pixels into local memory
    const unsigned int x = get_global_id(0);
    unsigned int pos = getPixelIndex(number_channels, inputColumns, x, cacheRangeStartY);
    unsigned int rangeLength = cacheRangeEndY-cacheRangeStartY;
    unsigned int stride = inputColumns * number_channels;
    for (unsigned int i = 0; i < number_channels; i++)
    {
      event_t e = async_work_group_strided_copy(inputImageCache + (rangeLength*i), inputImage+pos+i, rangeLength, stride, 0);
      wait_group_events(1,&e);
    }

    unsigned int alpha_index = (number_channels == 4) || (number_channels == 2) ? number_channels - 1 : 0;
    unsigned int totalNumChunks = (actualNumPixelToCompute+pixelChunkSize-1)/pixelChunkSize;
    for (unsigned int chunk = 0; chunk < totalNumChunks; chunk++)
    {
      const unsigned int chunkStartY = startY + chunk*pixelChunkSize;
      const unsigned int chunkStopY = MagickMin(chunkStartY + pixelChunkSize, stopY);
      const unsigned int actualNumPixelInThisChunk = chunkStopY - chunkStartY;

      // determine which resized pixel computed by this workitem
      const unsigned int itemID = get_local_id(1);
      const unsigned int numItems = getNumWorkItemsPerPixel(actualNumPixelInThisChunk, get_local_size(1));

      const int pixelIndex = pixelToCompute(itemID, actualNumPixelInThisChunk, get_local_size(1));

      float4 filteredPixel = (float4)0.0f;
      float density = 0.0f;
      float gamma = 0.0f;
      // -1 means this workitem doesn't participate in the computation
      if (pixelIndex != -1)
      {
        // x coordinated of the resized pixel computed by this workitem
        const int y = chunkStartY + pixelIndex;

        // calculate how many steps required for this pixel
        const float bisect = (y+0.5)/yFactor+MagickEpsilon;
        const unsigned int start = (unsigned int)MagickMax(bisect-support+0.5f,0.0f);
        const unsigned int stop  = (unsigned int)MagickMin(bisect+support+0.5f,(float)inputRows);
        const unsigned int n = stop - start;

        // calculate how many steps this workitem will contribute
        unsigned int numStepsPerWorkItem = n / numItems;
        numStepsPerWorkItem += ((numItems*numStepsPerWorkItem)==n?0:1);

        const unsigned int startStep = (itemID%numItems)*numStepsPerWorkItem;
        if (startStep < n)
        {
          const unsigned int stopStep = MagickMin(startStep+numStepsPerWorkItem, n);

          unsigned int cacheIndex = start+startStep-cacheRangeStartY;
          for (unsigned int i = startStep; i < stopStep; i++, cacheIndex++)
          {
            float weight = getResizeFilterWeight(resizeFilterCubicCoefficients,
              (ResizeWeightingFunctionType) resizeFilterType,
              (ResizeWeightingFunctionType) resizeWindowType,
              resizeFilterScale, resizeFilterWindowSupport,
              resizeFilterBlur, scale*(start + i - bisect + 0.5));

            float4 cp = (float4)0.0f;

            __local CLQuantum *p = inputImageCache + cacheIndex;
            cp.x = (float) *(p);
            if (number_channels > 2)
            {
              cp.y = (float) *(p + rangeLength);
              cp.z = (float) *(p + (rangeLength * 2));
            }

            if (alpha_index != 0)
            {
              cp.w = (float) *(p + (rangeLength * alpha_index));

              float alpha = weight * QuantumScale * cp.w;

              filteredPixel.x += alpha * cp.x;
              filteredPixel.y += alpha * cp.y;
              filteredPixel.z += alpha * cp.z;
              filteredPixel.w += weight * cp.w;
              gamma += alpha;
            }
            else
              filteredPixel += ((float4) weight)*cp;

            density += weight;
          }
        }
      }

      // initialize the accumulators to zero
      if (itemID < actualNumPixelInThisChunk) {
        outputPixelCache[itemID] = (float4)0.0f;
        densityCache[itemID] = 0.0f;
        if (alpha_index != 0)
          gammaCache[itemID] = 0.0f;
      }
      barrier(CLK_LOCAL_MEM_FENCE);

      // accumulatte the filtered pixel value and the density
      for (unsigned int i = 0; i < numItems; i++) {
        if (pixelIndex != -1) {
          if (itemID%numItems == i) {
            outputPixelCache[pixelIndex]+=filteredPixel;
            densityCache[pixelIndex]+=density;
            if (alpha_index != 0)
              gammaCache[pixelIndex]+=gamma;
          }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
      }

      if (itemID < actualNumPixelInThisChunk)
      {
        float4 filteredPixel = outputPixelCache[itemID];

        float gamma = 0.0f;
        if (alpha_index != 0)
          gamma = gammaCache[itemID];

        float density = densityCache[itemID];
        if ((density != 0.0f) && (density != 1.0f))
        {
          density = PerceptibleReciprocal(density);
          filteredPixel *= (float4) density;
          if (alpha_index != 0)
            gamma *= density;
        }

        if (alpha_index != 0)
        {
          gamma = PerceptibleReciprocal(gamma);
          filteredPixel.x *= gamma;
          filteredPixel.y *= gamma;
          filteredPixel.z *= gamma;
        }

        WriteAllChannels(filteredImage, number_channels, filteredColumns, x, chunkStartY + itemID, filteredPixel);
      }
    }
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R o t a t i o n a l B l u r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
  __kernel void RotationalBlur(const __global CLQuantum *image,
    const unsigned int number_channels,const unsigned int channel,
    const float2 blurCenter,__constant float *cos_theta,
    __constant float *sin_theta,const unsigned int cossin_theta_size,
    __global CLQuantum *filteredImage)
  {
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int columns = get_global_size(0);
    const int rows = get_global_size(1);
    unsigned int step = 1;
    float center_x = (float) x - blurCenter.x;
    float center_y = (float) y - blurCenter.y;
    float radius = hypot(center_x, center_y);

    float blur_radius = hypot(blurCenter.x, blurCenter.y);

    if (radius > MagickEpsilon)
    {
      step = (unsigned int) (blur_radius / radius);
      if (step == 0)
        step = 1;
      if (step >= cossin_theta_size)
        step = cossin_theta_size-1;
    }

    float4 result = 0.0f;
    float normalize = 0.0f;
    float gamma = 0.0f;

    for (unsigned int i=0; i<cossin_theta_size; i+=step)
    {
      int cx = ClampToCanvas(blurCenter.x+center_x*cos_theta[i]-center_y*sin_theta[i]+0.5f,columns);
      int cy = ClampToCanvas(blurCenter.y+center_x*sin_theta[i]+center_y*cos_theta[i]+0.5f,rows);

      float4 pixel = ReadAllChannels(image, number_channels, columns, cx, cy);

      if ((number_channels == 4) || (number_channels == 2))
      {
        float alpha = (float)(QuantumScale*pixel.w);

        gamma += alpha;

        result.x += alpha * pixel.x;
        result.y += alpha * pixel.y;
        result.z += alpha * pixel.z;
        result.w += pixel.w;
      }
      else
        result += pixel;

      normalize += 1.0f;
    }

    normalize = PerceptibleReciprocal(normalize);

    if ((number_channels == 4) || (number_channels == 2))
    {
      gamma = PerceptibleReciprocal(gamma);
      result.x *= gamma;
      result.y *= gamma;
      result.z *= gamma;
      result.w *= normalize;
    }
    else
      result *= normalize;

    WriteFloat4(filteredImage, number_channels, columns, x, y, channel, result);
  }
  )

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     U n s h a r p M a s k                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

  STRINGIFY(
  __kernel void UnsharpMaskBlurColumn(const __global CLQuantum* image,
    const __global float4 *blurRowData,const unsigned int number_channels,
    const ChannelType channel,const unsigned int columns,
    const unsigned int rows,__local float4* cachedData,
    __local float* cachedFilter,const __global float *filter,
    const unsigned int width,const float gain, const float threshold,
    __global CLQuantum *filteredImage)
  {
    const unsigned int radius = (width-1)/2;

    // cache the pixel shared by the workgroup
    const int groupX = get_group_id(0);
    const int groupStartY = get_group_id(1)*get_local_size(1) - radius;
    const int groupStopY = (get_group_id(1)+1)*get_local_size(1) + radius;

    if ((groupStartY >= 0) && (groupStopY < rows))
    {
      event_t e = async_work_group_strided_copy(cachedData,
        blurRowData+groupStartY*columns+groupX,groupStopY-groupStartY,columns,0);
      wait_group_events(1,&e);
    }
    else
    {
      for (int i = get_local_id(1); i < (groupStopY - groupStartY); i+=get_local_size(1))
        cachedData[i] = blurRowData[ClampToCanvas(groupStartY+i,rows)*columns + groupX];

      barrier(CLK_LOCAL_MEM_FENCE);
    }
    // cache the filter as well
    event_t e = async_work_group_copy(cachedFilter,filter,width,0);
    wait_group_events(1,&e);

    // only do the work if this is not a patched item
    const int cy = get_global_id(1);

    if (cy < rows)
    {
      float4 blurredPixel = (float4) 0.0f;

      int i = 0;

      for ( ; i+7 < width; )
      {
        for (int j=0; j < 8; j++, i++)
          blurredPixel+=cachedFilter[i+j]*cachedData[i+j+get_local_id(1)];
      }

      for ( ; i < width; i++)
        blurredPixel+=cachedFilter[i]*cachedData[i+get_local_id(1)];

      float4 inputImagePixel = ReadFloat4(image,number_channels,columns,groupX,cy,channel);
      float4 outputPixel = inputImagePixel - blurredPixel;

      float quantumThreshold = QuantumRange*threshold;

      int4 mask = isless(fabs(2.0f*outputPixel), (float4)quantumThreshold);
      outputPixel = select(inputImagePixel + outputPixel * gain, inputImagePixel, mask);

      //write back
      WriteFloat4(filteredImage,number_channels,columns,groupX,cy,channel,outputPixel);
    }
  }
  )

  STRINGIFY(
  __kernel void UnsharpMask(const __global CLQuantum *image,const unsigned int number_channels,
    const ChannelType channel,__constant float *filter,const unsigned int width,
    const unsigned int columns,const unsigned int rows,__local float4 *pixels,
    const float gain,const float threshold,__global CLQuantum *filteredImage)
  {
    const unsigned int x = get_global_id(0);
    const unsigned int y = get_global_id(1);

    const unsigned int radius = (width - 1) / 2;

    int row = y - radius;
    int baseRow = get_group_id(1) * get_local_size(1) - radius;
    int endRow = (get_group_id(1) + 1) * get_local_size(1) + radius;

    while (row < endRow) {
      int srcy = (row < 0) ? -row : row; // mirror pad
      srcy = (srcy >= rows) ? (2 * rows - srcy - 1) : srcy;

      float4 value = 0.0f;

      int ix = x - radius;
      int i = 0;

      while (i + 7 < width) {
        for (int j = 0; j < 8; ++j) { // unrolled
          int srcx = ix + j;
          srcx = (srcx < 0) ? -srcx : srcx;
          srcx = (srcx >= columns) ? (2 * columns - srcx - 1) : srcx;
          value += filter[i + j] * ReadFloat4(image, number_channels, columns, srcx, srcy, channel);
        }
        ix += 8;
        i += 8;
      }

      while (i < width) {
        int srcx = (ix < 0) ? -ix : ix; // mirror pad
        srcx = (srcx >= columns) ? (2 * columns - srcx - 1) : srcx;
        value += filter[i] * ReadFloat4(image, number_channels, columns, srcx, srcy, channel);
        ++i;
        ++ix;
      }
      pixels[(row - baseRow) * get_local_size(0) + get_local_id(0)] = value;
      row += get_local_size(1);
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    const int px = get_local_id(0);
    const int py = get_local_id(1);
    const int prp = get_local_size(0);
    float4 value = (float4)(0.0f);

    int i = 0;
    while (i + 7 < width) {
      for (int j = 0; j < 8; ++j) // unrolled
        value += (float4)(filter[i]) * pixels[px + (py + i + j) * prp];
      i += 8;
    }
    while (i < width) {
      value += (float4)(filter[i]) * pixels[px + (py + i) * prp];
      ++i;
    }

    if ((x < columns) && (y < rows)) {
      float4 srcPixel = ReadFloat4(image, number_channels, columns, x, y, channel);
      float4 diff = srcPixel - value;

      float quantumThreshold = QuantumRange*threshold;

      int4 mask = isless(fabs(2.0f * diff), (float4)quantumThreshold);
      value = select(srcPixel + diff * gain, srcPixel, mask);

      WriteFloat4(filteredImage, number_channels, columns, x, y, channel, value);
    }
  }
  )

  STRINGIFY(
    __kernel __attribute__((reqd_work_group_size(64, 4, 1)))
    void WaveletDenoise(__global CLQuantum *srcImage,__global CLQuantum *dstImage,
      const unsigned int number_channels,const unsigned int max_channels,
      const float threshold,const int passes,const unsigned int imageWidth,
      const unsigned int imageHeight)
  {
    const int pad = (1 << (passes - 1));
    const int tileSize = 64;
    const int tileRowPixels = 64;
    const float noise[] = { 0.8002, 0.2735, 0.1202, 0.0585, 0.0291, 0.0152, 0.0080, 0.0044 };

    CLQuantum stage[48]; // 16 * 3 (we only need 3 channels)

    local float buffer[64 * 64];

    int srcx = (get_group_id(0) + get_global_offset(0) / tileSize) * (tileSize - 2 * pad) - pad + get_local_id(0);
    int srcy = (get_group_id(1) + get_global_offset(1) / 4) * (tileSize - 2 * pad) - pad;

    for (int i = get_local_id(1); i < tileSize; i += get_local_size(1)) {
      int pos = (mirrorTop(mirrorBottom(srcx), imageWidth) * number_channels) +
                (mirrorTop(mirrorBottom(srcy + i), imageHeight)) * imageWidth * number_channels;

      for (int channel = 0; channel < max_channels; ++channel)
        stage[(i / 4) + (16 * channel)] = srcImage[pos + channel];
    }

    for (int channel = 0; channel < max_channels; ++channel) {
      // Load LDS
      for (int i = get_local_id(1); i < tileSize; i += get_local_size(1))
        buffer[get_local_id(0) + i * tileRowPixels] = convert_float(stage[(i / 4) + (16 * channel)]);

      // Process

      float tmp[16];
      float accum[16];
      float pixel;

      for (int i = 0; i < 16; i++)
        accum[i]=0.0f;

      for (int pass = 0; pass < passes; ++pass) {
        const int radius = 1 << pass;
        const int x = get_local_id(0);
        const float thresh = threshold * noise[pass];

        // Apply horizontal hat
        for (int i = get_local_id(1); i < tileSize; i += get_local_size(1)) {
          const int offset = i * tileRowPixels;
          if (pass == 0)
            tmp[i / 4] = buffer[x + offset]; // snapshot input on first pass
          pixel = 0.5f * tmp[i / 4] + 0.25 * (buffer[mirrorBottom(x - radius) + offset] + buffer[mirrorTop(x + radius, tileSize) + offset]);
          barrier(CLK_LOCAL_MEM_FENCE);
          buffer[x + offset] = pixel;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // Apply vertical hat
        for (int i = get_local_id(1); i < tileSize; i += get_local_size(1)) {
          pixel = 0.5f * buffer[x + i * tileRowPixels] + 0.25 * (buffer[x + mirrorBottom(i - radius) * tileRowPixels] + buffer[x + mirrorTop(i + radius, tileRowPixels) * tileRowPixels]);
          float delta = tmp[i / 4] - pixel;
          tmp[i / 4] = pixel; // hold output in tmp until all workitems are done
          if (delta < -thresh)
            delta += thresh;
          else if (delta > thresh)
            delta -= thresh;
          else
            delta = 0;
          accum[i / 4] += delta;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        if (pass < passes - 1)
          for (int i = get_local_id(1); i < tileSize; i += get_local_size(1))
            buffer[x + i * tileRowPixels] = tmp[i / 4]; // store lowpass for next pass
        else  // last pass
          for (int i = get_local_id(1); i < tileSize; i += get_local_size(1))
            accum[i / 4] += tmp[i / 4]; // add the lowpass signal back to output
        barrier(CLK_LOCAL_MEM_FENCE);
      }

      for (int i = get_local_id(1); i < tileSize; i += get_local_size(1))
        stage[(i / 4) + (16 * channel)] = ClampToQuantum(accum[i / 4]);

      barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Write from stage to output

    if ((get_local_id(0) >= pad) && (get_local_id(0) < tileSize - pad) && (srcx >= 0) && (srcx < imageWidth)) {
      for (int i = get_local_id(1); i < tileSize; i += get_local_size(1)) {
        if ((i >= pad) && (i < tileSize - pad) && (srcy + i >= 0) && (srcy + i < imageHeight)) {
          int pos = (srcx * number_channels) + ((srcy + i) * (imageWidth * number_channels));
          for (int channel = 0; channel < max_channels; ++channel) {
            dstImage[pos + channel] = stage[(i / 4) + (16 * channel)];
          }
        }
      }
    }
  }
  )

  ;

#endif // MAGICKCORE_OPENCL_SUPPORT

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif // MAGICKCORE_ACCELERATE_KERNELS_PRIVATE_H

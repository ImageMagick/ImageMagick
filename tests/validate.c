/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%           V   V   AAA   L      IIIII  DDDD    AAA   TTTTT  EEEEE            %
%           V   V  A   A  L        I    D   D  A   A    T    E                %
%           V   V  AAAAA  L        I    D   D  AAAAA    T    EEE              %
%            V V   A   A  L        I    D   D  A   A    T    E                %
%             V    A   A  LLLLL  IIIII  DDDD   A   A    T    EEEEE            %
%                                                                             %
%                                                                             %
%                        ImageMagick Validation Suite                         %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               March 2001                                    %
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
%  see the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string-private.h"
#include "validate.h"

/*
  Define declarations.
*/
#if defined(__APPLE__)
  #include "TargetConditionals.h"
  #if TARGET_OS_IOS || TARGET_OS_WATCH || TARGET_OS_TV
    #define system(s) ((s)==NULL ? 0 : -1)
  #endif // end iOS
#elif defined(__ANDROID__)
  #define system(s) ((s)==NULL ? 0 : -1)
#endif
#define CIEEpsilon  (216.0/24389.0)
#define CIEK  (24389.0/27.0)
#define D65X  0.95047
#define D65Y  1.0
#define D65Z  1.08883
#define ReferenceEpsilon  ((double) QuantumRange*1.0e-2)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e C o l o r s p a c e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateColorspaces() validates the ImageMagick colorspaces and returns the
%  number of validation tests that passed and failed.
%
%  The format of the ValidateColorspaces method is:
%
%      size_t ValidateColorspaces(size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o fails: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType ValidateHSIToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  HSIToRGB");
  ConvertHSIToRGB(111.244375/360.0,0.295985,0.658734,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToHSI()
{
  double
    h,
    i,
    s;

  (void) FormatLocaleFile(stdout,"  RGBToHSI");
  ConvertRGBToHSI(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&h,&s,&i);
  if ((fabs(h-111.244374/360.0) >= ReferenceEpsilon) ||
      (fabs(s-0.295985) >= ReferenceEpsilon) ||
      (fabs(i-0.658734) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateHSLToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  HSLToRGB");
  ConvertHSLToRGB(110.200859/360.0,0.882623,0.715163,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToHSL()
{
  double
    h,
    l,
    s;

  (void) FormatLocaleFile(stdout,"  RGBToHSL");
  ConvertRGBToHSL(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&h,&s,&l);
  if ((fabs(h-110.200859/360.0) >= ReferenceEpsilon) ||
      (fabs(s-0.882623) >= ReferenceEpsilon) ||
      (fabs(l-0.715163) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateHSVToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  HSVToRGB");
  ConvertHSVToRGB(110.200859/360.0,0.520200,0.966567,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToHSV()
{
  double
    h,
    s,
    v;

  (void) FormatLocaleFile(stdout,"  RGBToHSV");
  ConvertRGBToHSV(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&h,&s,&v);
  if ((fabs(h-110.200859/360.0) >= ReferenceEpsilon) ||
      (fabs(s-0.520200) >= ReferenceEpsilon) ||
      (fabs(v-0.966567) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToJPEGYCbCr()
{
  double
    Cb,
    Cr,
    Y;

  (void) FormatLocaleFile(stdout,"  RGBToJPEGYCbCr");
  ConvertRGBToYCbCr(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&Y,&Cb,&Cr);
  if ((fabs(Y-0.783460) >= ReferenceEpsilon) ||
      (fabs(Cb-0.319581) >= ReferenceEpsilon) ||
      (fabs(Cr-0.330539) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateJPEGYCbCrToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  JPEGYCbCrToRGB");
  ConvertYCbCrToRGB(0.783460,0.319581,0.330539,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateLabToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  LabToRGB");
  ConvertLabToRGB(88.456154/100.0,-54.671483/255+0.5,51.662818/255.0+0.5,
    D65Illuminant,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToLab()
{
  double
    a,
    b,
    L;

  (void) FormatLocaleFile(stdout,"  RGBToLab");
  ConvertRGBToLab(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,D65Illuminant,&L,&a,&b);
  if ((fabs(L-(88.456154/100.0)) >= ReferenceEpsilon) ||
      (fabs(a-(-54.671483/255.0+0.5)) >= ReferenceEpsilon) ||
      (fabs((double) b-(51.662818/255.0+0.5)) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateLchToRGB()
{
  double
    b,
    g,
    r;

  (void) FormatLocaleFile(stdout,"  LchToRGB");
  ConvertLCHabToRGB(88.456154/100.0,75.219797/255.0+0.5,136.620717/360.0,
    D65Illuminant,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToLch()
{
  double
    c,
    h,
    L;

  (void) FormatLocaleFile(stdout,"  RGBToLch");
  ConvertRGBToLCHab(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,D65Illuminant,&L,&c,&h);
  if ((fabs(L-88.456154/100.0) >= ReferenceEpsilon) ||
      (fabs(c-(75.219797/255.0+0.5)) >= ReferenceEpsilon) ||
      (fabs(h-(136.620717/255.0+0.5)) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToLMS()
{
  double
    L,
    M,
    S;

  (void) FormatLocaleFile(stdout,"  RGBToLMS");
  ConvertRGBToLMS(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&L,&M,&S);
  if ((fabs(L-0.611749) >= ReferenceEpsilon) ||
      (fabs(M-0.910088) >= ReferenceEpsilon) ||
      (fabs(S-0.294880) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateLMSToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  LMSToRGB");
  ConvertLMSToRGB(0.611749,0.910088,0.294880,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToLuv()
{
  double
    l,
    u,
    v;

  (void) FormatLocaleFile(stdout,"  RGBToLuv");
  ConvertRGBToLuv(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,D65Illuminant,&l,&u,&v);
  if ((fabs(l-88.456154/262.0) >= ReferenceEpsilon) ||
      (fabs(u-(-51.330414+134.0)/354.0) >= ReferenceEpsilon) ||
      (fabs(v-(76.405526+140.0)/262.0) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateLuvToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  LuvToRGB");
  ConvertLuvToRGB(88.456154/100.0,(-51.330414+134.0)/354.0,
    (76.405526+140.0)/262.0,D65Illuminant,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToXYZ()
{
  double
    x,
    y,
    z;

  (void) FormatLocaleFile(stdout,"  RGBToXYZ");
  ConvertRGBToXYZ(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&x,&y,&z);
  if ((fabs(x-0.470646) >= ReferenceEpsilon) ||
      (fabs(y-0.730178) >= ReferenceEpsilon) ||
      (fabs(z-0.288324) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateXYZToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  XYZToRGB");
  ConvertXYZToRGB(0.470646,0.730178,0.288324,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateYDbDrToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  YDbDrToRGB");
  ConvertYDbDrToRGB(0.783460,-0.480932+0.5,0.451670+0.5,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToYDbDr()
{
  double
    Db,
    Dr,
    Y;

  (void) FormatLocaleFile(stdout,"  RGBToYDbDr");
  ConvertRGBToYDbDr(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&Y,&Db,&Dr);
  if ((fabs(Y-0.783460) >= ReferenceEpsilon) ||
      (fabs(Db-(-0.480932)) >= ReferenceEpsilon) ||
      (fabs(Dr-0.451670) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToYIQ()
{
  double
    i,
    q,
    y;

  (void) FormatLocaleFile(stdout,"  RGBToYIQ");
  ConvertRGBToYIQ(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&y,&i,&q);
  if ((fabs(y-0.783460) >= ReferenceEpsilon) ||
      (fabs(i-(-0.089078)) >= ReferenceEpsilon) ||
      (fabs(q-(-0.245399)) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateYIQToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  YIQToRGB");
  ConvertYIQToRGB(0.783460,-0.089078+0.5,-0.245399+0.5,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToYPbPr()
{
  double
    Pb,
    Pr,
    y;

  (void) FormatLocaleFile(stdout,"  RGBToYPbPr");
  ConvertRGBToYPbPr(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&y,&Pb,&Pr);
  if ((fabs(y-0.783460) >= ReferenceEpsilon) ||
      (fabs(Pb-(-0.180419)) >= ReferenceEpsilon) ||
      (fabs(Pr-(-0.169461)) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateYPbPrToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  YPbPrToRGB");
  ConvertYPbPrToRGB(0.783460,-0.180419+0.5,-0.169461+0.5,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateRGBToYUV()
{
  double
    U,
    V,
    Y;

  (void) FormatLocaleFile(stdout,"  RGBToYUV");
  ConvertRGBToYUV(0.545877*(double) QuantumRange,0.966567*(double)
    QuantumRange,0.463759*(double) QuantumRange,&Y,&U,&V);
  if ((fabs(Y-0.783460) >= ReferenceEpsilon) ||
      (fabs(U-(-0.157383)) >= ReferenceEpsilon) ||
      (fabs(V-(-0.208443)) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ValidateYUVToRGB()
{
  double
    r,
    g,
    b;

  (void) FormatLocaleFile(stdout,"  YUVToRGB");
  ConvertYUVToRGB(0.783460,-0.157383+0.5,-0.208443+0.5,&r,&g,&b);
  if ((fabs((double) r-0.545877*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) g-0.966567*(double) QuantumRange) >= ReferenceEpsilon) ||
      (fabs((double) b-0.463759*(double) QuantumRange) >= ReferenceEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static size_t ValidateColorspaces(size_t *fails,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  size_t
    fail,
    test;

  /*
     Reference: https://code.google.com/p/chroma.

     Illuminant =  D65
     Observer   =  2° (1931)

     XYZ            0.470645,   0.730177,   0.288323
     sRGB           0.545877,   0.966567,   0.463759
     CAT02 LMS      0.611749,   0.910088,   0.294880
     Y'DbDr         0.783460,  -0.480932,   0.451670
     Y'IQ           0.783460,  -0.089078,  -0.245399
     Y'PbPr         0.783460,  -0.180419,  -0.169461
     Y'UV           0.783460,  -0.157383,  -0.208443
     JPEG-Y'CbCr    0.783460,   0.319581,   0.330539
     L*u*v*        88.456154, -51.330414,  76.405526
     L*a*b*        88.456154, -54.671483,  51.662818
     L*C*H*        88.456154,  75.219797, 136.620717
     HSV          110.200859,   0.520200,   0.966567
     HSL          110.200859,   0.882623,   0.715163
     HSI          111.244375,   0.295985,   0.658734
     Y'CbCr       187.577791,  87.586330,  90.040886
  */
  (void) FormatLocaleFile(stdout,"validate colorspaces:\n");
  fail=0;
  for (test=0; test < 26; test++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: ",(double) test);
    switch (test)
    {
      case  0: status=ValidateHSIToRGB(); break;
      case  1: status=ValidateRGBToHSI(); break;
      case  2: status=ValidateHSLToRGB(); break;
      case  3: status=ValidateRGBToHSL(); break;
      case  4: status=ValidateHSVToRGB(); break;
      case  5: status=ValidateRGBToHSV(); break;
      case  6: status=ValidateJPEGYCbCrToRGB(); break;
      case  7: status=ValidateRGBToJPEGYCbCr(); break;
      case  8: status=ValidateLabToRGB(); break;
      case  9: status=ValidateRGBToLab(); break;
      case 10: status=ValidateLchToRGB(); break;
      case 11: status=ValidateRGBToLch(); break;
      case 12: status=ValidateLMSToRGB(); break;
      case 13: status=ValidateRGBToLMS(); break;
      case 14: status=ValidateLuvToRGB(); break;
      case 15: status=ValidateRGBToLuv(); break;
      case 16: status=ValidateXYZToRGB(); break;
      case 17: status=ValidateRGBToXYZ(); break;
      case 18: status=ValidateYDbDrToRGB(); break;
      case 19: status=ValidateRGBToYDbDr(); break;
      case 20: status=ValidateYIQToRGB(); break;
      case 21: status=ValidateRGBToYIQ(); break;
      case 22: status=ValidateYPbPrToRGB(); break;
      case 23: status=ValidateRGBToYPbPr(); break;
      case 24: status=ValidateYUVToRGB(); break;
      case 25: status=ValidateRGBToYUV(); break;
      default: status=MagickFalse;
    }
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e C o m p a r e C o m m a n d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateCompareCommand() validates the ImageMagick compare command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateCompareCommand method is:
%
%      size_t ValidateCompareCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateCompareCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MagickPathExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate compare command line program:\n");
  for (i=0; compare_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      compare_options[i]);
    (void) FormatLocaleString(command,MagickPathExtent,"%s %s %s %s",
      compare_options[i],reference_filename,reference_filename,output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (void) LogMagickEvent(ExceptionEvent,GetMagickModule(),"%s",
          exception->reason);
        fail++;
        continue;
      }
    status=CompareImagesCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (void) LogMagickEvent(ExceptionEvent,GetMagickModule(),"%s",
          exception->reason);
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e C o m p o s i t e C o m m a n d                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateCompositeCommand() validates the ImageMagick composite command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateCompositeCommand method is:
%
%      size_t ValidateCompositeCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateCompositeCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MagickPathExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate composite command line program:\n");
  for (i=0; composite_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      composite_options[i]);
    (void) FormatLocaleString(command,MagickPathExtent,"%s %s %s %s",
      reference_filename,composite_options[i],reference_filename,
      output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    status=CompositeImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e C o n v e r t C o m m a n d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateConvertCommand() validates the ImageMagick convert command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateConvertCommand method is:
%
%      size_t ValidateConvertCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateConvertCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MagickPathExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate convert command line program:\n");
  for (i=0; convert_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) test++,
      convert_options[i]);
    (void) FormatLocaleString(command,MagickPathExtent,"%s %s %s %s",
      reference_filename,convert_options[i],reference_filename,output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    status=ConvertImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I d e n t i f y C o m m a n d                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateIdentifyCommand() validates the ImageMagick identify command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateIdentifyCommand method is:
%
%      size_t ValidateIdentifyCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateIdentifyCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MagickPathExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  (void) output_filename;
  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate identify command line program:\n");
  for (i=0; identify_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) test++,
      identify_options[i]);
    (void) FormatLocaleString(command,MagickPathExtent,"%s %s",
      identify_options[i],reference_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    status=IdentifyImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I m a g e F o r m a t s I n M e m o r y                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateImageFormatsInMemory() validates the ImageMagick image formats in
%  memory and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateImageFormatsInMemory method is:
%
%      size_t ValidateImageFormatsInMemory(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/

/*
  Enable this to count remaining $TMPDIR/magick-* files.  Note that the count
  includes any files left over from other runs.
*/
#undef MagickCountTempFiles

static size_t ValidateImageFormatsInMemory(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
#ifdef MagickCountTempFiles
    path[MagickPathExtent],
    SystemCommand[MagickPathExtent],
#endif
    size[MagickPathExtent];

  const MagickInfo
    *magick_info;

  double
    distortion,
    fuzz;

  Image
    *difference_image,
    *ping_image,
    *reconstruct_image,
    *reference_image;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    length,
    test;

  unsigned char
    *blob;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate image formats in memory:\n");

#ifdef MagickCountTempFiles
  (void)GetPathTemplate(path);
  /* Remove file template except for the leading "/path/to/magick-" */
  path[strlen(path)-17]='\0';
  (void) FormatLocaleFile(stdout," tmp path is '%s*'\n",path);
#endif

  for (i=0; reference_formats[i].magick != (char *) NULL; i++)
  {
    magick_info=GetMagickInfo(reference_formats[i].magick,exception);
    if ((magick_info == (const MagickInfo *) NULL) ||
        (magick_info->decoder == (DecodeImageHandler *) NULL) ||
        (magick_info->encoder == (EncodeImageHandler *) NULL))
      continue;
    for (j=0; reference_types[j].type != UndefinedType; j++)
    {
      /*
        Generate reference image.
      */
      CatchException(exception);
      (void) FormatLocaleFile(stdout,"  test %.20g: %s/%s/%s/%.20g-bits",
        (double) (test++),reference_formats[i].magick,CommandOptionToMnemonic(
        MagickCompressOptions,reference_formats[i].compression),
        CommandOptionToMnemonic(MagickTypeOptions,reference_types[j].type),
        (double) reference_types[j].depth);
      (void) CopyMagickString(image_info->filename,reference_filename,
        MagickPathExtent);
      reference_image=ReadImage(image_info,exception);
      if ((reference_image == (Image *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(size,MagickPathExtent,"%.20gx%.20g",
        (double) reference_image->columns,(double) reference_image->rows);
      (void) CloneString(&image_info->size,size);
      image_info->depth=reference_types[j].depth;
      (void) FormatLocaleString(reference_image->filename,MagickPathExtent,
        "%s:%s",reference_formats[i].magick,output_filename);
      status=SetImageType(reference_image,reference_types[j].type,exception);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      status=SetImageDepth(reference_image,reference_types[j].depth,exception);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      reference_image->compression=reference_formats[i].compression;
      status=WriteImage(image_info,reference_image,exception);
      reference_image=DestroyImage(reference_image);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          continue;
        }
      /*
        Ping reference image.
      */
      (void) FormatLocaleString(image_info->filename,MagickPathExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      ping_image=PingImage(image_info,exception);
      if (ping_image == (Image *) NULL ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          continue;
        }
      ping_image=DestroyImage(ping_image);
      /*
        Read reference image.
      */
      reference_image=ReadImage(image_info,exception);
      if ((reference_image == (Image *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(reference_image->filename,MagickPathExtent,
        "%s:%s",reference_formats[i].magick,output_filename);
      (void) CopyMagickString(image_info->magick,reference_formats[i].magick,
        MagickPathExtent);
      reference_image->depth=reference_types[j].depth;
      reference_image->compression=reference_formats[i].compression;
      length=8192;
      blob=(unsigned char *) ImageToBlob(image_info,reference_image,&length,
        exception);
      if ((blob == (unsigned char *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Ping reference blob.
      */
      ping_image=PingBlob(image_info,blob,length,exception);
      if (ping_image == (Image *) NULL ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          continue;
        }
      ping_image=DestroyImage(ping_image);
      /*
        Read reconstruct image.
      */
      (void) FormatLocaleString(image_info->filename,MagickPathExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reconstruct_image=BlobToImage(image_info,blob,length,exception);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      if (reconstruct_image == (Image *) NULL ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Compare reference to reconstruct image.
      */
      fuzz=0.003;  /* grayscale */
      if (reference_formats[i].fuzz != 0.0)
        fuzz=reference_formats[i].fuzz;
      difference_image=CompareImages(reference_image,reconstruct_image,
        RootMeanSquaredErrorMetric,&distortion,exception);
      reconstruct_image=DestroyImage(reconstruct_image);
      reference_image=DestroyImage(reference_image);
      if ((difference_image == (Image *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          continue;
        }
      difference_image=DestroyImage(difference_image);
      if (MagickSafeSignificantError(QuantumScale*distortion,fuzz) != MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail (with distortion %g).\n",
            QuantumScale*distortion);
          fail++;
          continue;
        }
#ifdef MagickCountTempFiles
      (void) FormatLocaleFile(stdout,"... pass, ");
      (void) fflush(stdout);
      SystemCommand[0]='\0';
      (void) strncat(SystemCommand,"echo `ls ",9);
      (void) strncat(SystemCommand,path,MagickPathExtent-31);
      (void) strncat(SystemCommand,"* | wc -w` tmp files.",20);
      (void) system(SystemCommand);
      (void) fflush(stdout);
#else
      (void) FormatLocaleFile(stdout,"... pass\n");
#endif
    }
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I m a g e F o r m a t s O n D i s k                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateImageFormatsOnDisk() validates the ImageMagick image formats on disk
%  and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateImageFormatsOnDisk method is:
%
%      size_t ValidateImageFormatsOnDisk(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateImageFormatsOnDisk(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    size[MagickPathExtent];

  const MagickInfo
    *magick_info;

  double
    distortion,
    fuzz;

  Image
    *difference_image,
    *reference_image,
    *reconstruct_image;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate image formats on disk:\n");
  for (i=0; reference_formats[i].magick != (char *) NULL; i++)
  {
    magick_info=GetMagickInfo(reference_formats[i].magick,exception);
    if ((magick_info == (const MagickInfo *) NULL) ||
        (magick_info->decoder == (DecodeImageHandler *) NULL) ||
        (magick_info->encoder == (EncodeImageHandler *) NULL))
      continue;
    for (j=0; reference_types[j].type != UndefinedType; j++)
    {
      /*
        Generate reference image.
      */
      CatchException(exception);
      (void) FormatLocaleFile(stdout,"  test %.20g: %s/%s/%s/%.20g-bits",
        (double) (test++),reference_formats[i].magick,CommandOptionToMnemonic(
        MagickCompressOptions,reference_formats[i].compression),
        CommandOptionToMnemonic(MagickTypeOptions,reference_types[j].type),
        (double) reference_types[j].depth);
      (void) CopyMagickString(image_info->filename,reference_filename,
        MagickPathExtent);
      reference_image=ReadImage(image_info,exception);
      if ((reference_image == (Image *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(size,MagickPathExtent,"%.20gx%.20g",
        (double) reference_image->columns,(double) reference_image->rows);
      (void) CloneString(&image_info->size,size);
      image_info->depth=reference_types[j].depth;
      (void) FormatLocaleString(reference_image->filename,MagickPathExtent,
        "%s:%s",reference_formats[i].magick,output_filename);
      status=SetImageType(reference_image,reference_types[j].type,exception);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      status=SetImageDepth(reference_image,reference_types[j].depth,exception);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      reference_image->compression=reference_formats[i].compression;
      status=WriteImage(image_info,reference_image,exception);
      reference_image=DestroyImage(reference_image);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          if (exception->reason != (char *) NULL)
            (void) FormatLocaleFile(stdout,"    reason:%s\n",exception->reason);
          CatchException(exception);
          fail++;
          continue;
        }
      /*
        Read reference image.
      */
      (void) FormatLocaleString(image_info->filename,MagickPathExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reference_image=ReadImage(image_info,exception);
      if ((reference_image == (Image *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(reference_image->filename,MagickPathExtent,
        "%s:%s",reference_formats[i].magick,output_filename);
      reference_image->depth=reference_types[j].depth;
      reference_image->compression=reference_formats[i].compression;
      status=WriteImage(image_info,reference_image,exception);
      if (status == MagickFalse ||exception->severity >= ErrorException)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Read reconstruct image.
      */
      (void) FormatLocaleString(image_info->filename,MagickPathExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reconstruct_image=ReadImage(image_info,exception);
      if (reconstruct_image == (Image *) NULL ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Compare reference to reconstruct image.
      */
      fuzz=0.003;  /* grayscale */
      if (reference_formats[i].fuzz != 0.0)
        fuzz=reference_formats[i].fuzz;
      difference_image=CompareImages(reference_image,reconstruct_image,
        RootMeanSquaredErrorMetric,&distortion,exception);
      reconstruct_image=DestroyImage(reconstruct_image);
      reference_image=DestroyImage(reference_image);
      if (difference_image == (Image *) NULL ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          continue;
        }
      difference_image=DestroyImage(difference_image);
      if (MagickSafeSignificantError(QuantumScale*distortion,fuzz) != MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail (with distortion %g).\n",
            QuantumScale*distortion);
          fail++;
          continue;
        }
      (void) FormatLocaleFile(stdout,"... pass.\n");
    }
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I m p o r t E x p o r t P i x e l s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateImportExportPixels() validates the pixel import and export methods.
%  It returns the number of validation tests that passed and failed.
%
%  The format of the ValidateImportExportPixels method is:
%
%      size_t ValidateImportExportPixels(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateImportExportPixels(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  double
    distortion;

  Image
    *difference_image,
    *reference_image,
    *reconstruct_image;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    length;

  unsigned char
    *pixels;

  size_t
    fail,
    test;

  (void) output_filename;
  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,
    "validate the import and export of image pixels:\n");
  for (i=0; reference_map[i] != (char *) NULL; i++)
  {
    for (j=0; reference_storage[j].type != UndefinedPixel; j++)
    {
      /*
        Generate reference image.
      */
      CatchException(exception);
      (void) FormatLocaleFile(stdout,"  test %.20g: %s/%s",(double) (test++),
        reference_map[i],CommandOptionToMnemonic(MagickStorageOptions,
        reference_storage[j].type));
      (void) CopyMagickString(image_info->filename,reference_filename,
        MagickPathExtent);
      reference_image=ReadImage(image_info,exception);
      if ((reference_image == (Image *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          continue;
        }
      if (LocaleNCompare(reference_map[i],"cmy",3) == 0)
        (void) SetImageColorspace(reference_image,CMYKColorspace,exception);
      length=strlen(reference_map[i])*reference_image->columns*
        reference_image->rows*reference_storage[j].quantum;
      pixels=(unsigned char *) AcquireQuantumMemory(length,sizeof(*pixels));
      if ((pixels == (unsigned char *) NULL) ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      (void) memset(pixels,0,length*sizeof(*pixels));
      status=ExportImagePixels(reference_image,0,0,reference_image->columns,
        reference_image->rows,reference_map[i],reference_storage[j].type,pixels,
        exception);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          reference_image=DestroyImage(reference_image);
          continue;
        }
      (void) SetImageBackgroundColor(reference_image,exception);
      status=ImportImagePixels(reference_image,0,0,reference_image->columns,
        reference_image->rows,reference_map[i],reference_storage[j].type,
        pixels,exception);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
           pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Read reconstruct image.
      */
      reconstruct_image=AcquireImage(image_info,exception);
      (void) SetImageExtent(reconstruct_image,reference_image->columns,
        reference_image->rows,exception);
      (void) SetImageColorspace(reconstruct_image,reference_image->colorspace,
        exception);
      (void) SetImageBackgroundColor(reconstruct_image,exception);
      status=ImportImagePixels(reconstruct_image,0,0,reconstruct_image->columns,
        reconstruct_image->rows,reference_map[i],reference_storage[j].type,
        pixels,exception);
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      if (status == MagickFalse || (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Compare reference to reconstruct image.
      */
      difference_image=CompareImages(reference_image,reconstruct_image,
        RootMeanSquaredErrorMetric,&distortion,exception);
      reconstruct_image=DestroyImage(reconstruct_image);
      reference_image=DestroyImage(reference_image);
      if (difference_image == (Image *) NULL ||
          (exception->severity >= ErrorException))
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          CatchException(exception);
          fail++;
          continue;
        }
      difference_image=DestroyImage(difference_image);
      if ((QuantumScale*distortion) > 0.0)
        {
          (void) FormatLocaleFile(stdout,"... fail (with distortion %g).\n",
            QuantumScale*distortion);
          fail++;
          continue;
        }
      (void) FormatLocaleFile(stdout,"... pass.\n");
    }
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e M a g i c k C o m m a n d                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateMagickCommand() validates the ImageMagick magick command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateMagickCommand method is:
%
%      size_t ValidateMagickCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateMagickCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MagickPathExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate magick command line program:\n");
  for (i=0; convert_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) test++,
      convert_options[i]);
    (void) FormatLocaleString(command,MagickPathExtent,"%s %s %s %s",
      reference_filename,convert_options[i],reference_filename,output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    status=MagickImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e M o n t a g e C o m m a n d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateMontageCommand() validates the ImageMagick montage command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateMontageCommand method is:
%
%      size_t ValidateMontageCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateMontageCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MagickPathExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate montage command line program:\n");
  for (i=0; montage_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      montage_options[i]);
    (void) FormatLocaleString(command,MagickPathExtent,"%s %s %s %s",
      reference_filename,montage_options[i],reference_filename,
      output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
        fail++;
        continue;
      }
    status=MontageImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e S t r e a m C o m m a n d                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateStreamCommand() validates the ImageMagick stream command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateStreamCommand method is:
%
%      size_t ValidateStreamCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fails,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateStreamCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fails,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MagickPathExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i,
    j;

  size_t
    fail,
    test;

  fail=0;
  test=0;
  (void) FormatLocaleFile(stdout,"validate stream command line program:\n");
  for (i=0; stream_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      stream_options[i]);
    (void) FormatLocaleString(command,MagickPathExtent,"%s %s %s",
      stream_options[i],reference_filename,output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    status=StreamImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status == MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        fail++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-fail),(double) fail);
  *fails+=fail;
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M a i n                                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

static MagickBooleanType ValidateUsage(void)
{
  const char
    **p;

  static const char
    *miscellaneous[]=
    {
      "-debug events        display copious debugging information",
      "-help                print program options",
      "-log format          format of debugging information",
      "-validate type       validation type",
      "-version             print version information",
      (char *) NULL
    },
    *settings[]=
    {
      "-regard-warnings     pay attention to warning messages",
      "-verbose             print detailed information about the image",
      (char *) NULL
    };

  (void) printf("Version: %s\n",GetMagickVersion((size_t *) NULL));
  (void) printf("Copyright: %s\n\n",GetMagickCopyright());
  (void) printf("Features: %s\n",GetMagickFeatures());
  (void) printf("Usage: %s [options ...] reference-file\n",GetClientName());
  (void) printf("\nValidate Settings:\n");
  for (p=settings; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nMiscellaneous Options:\n");
  for (p=miscellaneous; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  return(MagickTrue);
}

int main(int argc,char **argv)
{
#define DestroyValidate() \
{ \
  image_info=DestroyImageInfo(image_info); \
  exception=DestroyExceptionInfo(exception); \
}
#define ThrowValidateException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  CatchException(exception); \
  DestroyValidate(); \
  return(MagickFalse); \
}

  char
    output_filename[MagickPathExtent],
    reference_filename[MagickPathExtent],
    *option;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  Image
    *reference_image;

  ImageInfo
    *image_info;

  MagickBooleanType
    regard_warnings,
    status;

  MagickSizeType
    memory_resource,
    map_resource;

  ssize_t
    i;

  TimerInfo
    *timer;

  size_t
    fail,
    iterations,
    tests;

  ValidateType
    type;

  /*
    Validate the ImageMagick image processing suite.
  */
  MagickCoreGenesis(*argv,MagickTrue);
  (void) setlocale(LC_ALL,"");
  (void) setlocale(LC_NUMERIC,"C");
  iterations=1;
  status=MagickFalse;
  type=AllValidate;
  regard_warnings=MagickFalse;
  (void) regard_warnings;
  exception=AcquireExceptionInfo();
  image_info=AcquireImageInfo();
  (void) CopyMagickString(image_info->filename,ReferenceFilename,
    MagickPathExtent);
  for (i=1; i < (ssize_t) argc; i++)
  {
    option=argv[i];
    if (IsCommandOption(option) == MagickFalse)
      {
        (void) CopyMagickString(image_info->filename,option,MagickPathExtent);
        continue;
      }
    switch (*(option+1))
    {
      case 'b':
      {
        if (LocaleCompare("bench",option+1) == 0)
          {
            iterations=StringToUnsignedLong(argv[++i]);
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'd':
      {
        if (LocaleCompare("debug",option+1) == 0)
          {
            (void) SetLogEventMask(argv[++i]);
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'h':
      {
        if (LocaleCompare("help",option+1) == 0)
          {
            (void) ValidateUsage();
            return(0);
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'l':
      {
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option != '+')
              (void) SetLogFormat(argv[i+1]);
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'r':
      {
        if (LocaleCompare("regard-warnings",option+1) == 0)
          {
            regard_warnings=MagickTrue;
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'v':
      {
        if (LocaleCompare("validate",option+1) == 0)
          {
            ssize_t
              validate;

            if (*option == '+')
              break;
            i++;
            if (i >= (ssize_t) argc)
              ThrowValidateException(OptionError,"MissingArgument",option);
            validate=ParseCommandOption(MagickValidateOptions,MagickFalse,
              argv[i]);
            if (validate < 0)
              ThrowValidateException(OptionError,"UnrecognizedValidateType",
                argv[i]);
            type=(ValidateType) validate;
            break;
          }
        if ((LocaleCompare("version",option+1) == 0) ||
            (LocaleCompare("-version",option+1) == 0))
          {
            (void) FormatLocaleFile(stdout,"Version: %s\n",
              GetMagickVersion((size_t *) NULL));
            (void) FormatLocaleFile(stdout,"Copyright: %s\n\n",
              GetMagickCopyright());
            (void) FormatLocaleFile(stdout,"Features: %s\n\n",
              GetMagickFeatures());
            return(0);
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      default:
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
    }
  }
  timer=(TimerInfo *) NULL;
  if (iterations > 1)
    timer=AcquireTimerInfo();
  reference_image=ReadImage(image_info,exception);
  tests=0;
  fail=0;
  if (reference_image == (Image *) NULL)
    fail++;
  else
    {
      if (LocaleCompare(image_info->filename,ReferenceFilename) == 0)
        (void) CopyMagickString(reference_image->magick,ReferenceImageFormat,
          MagickPathExtent);
      (void) AcquireUniqueFilename(reference_filename);
      (void) AcquireUniqueFilename(output_filename);
      (void) CopyMagickString(reference_image->filename,reference_filename,
        MagickPathExtent);
      status=WriteImage(image_info,reference_image,exception);
      reference_image=DestroyImage(reference_image);
      if (status == MagickFalse)
        fail++;
      else
        {
          (void) FormatLocaleFile(stdout,"Version: %s\n",
            GetMagickVersion((size_t *) NULL));
          (void) FormatLocaleFile(stdout,"Copyright: %s\n\n",
            GetMagickCopyright());
          (void) FormatLocaleFile(stdout,
            "ImageMagick Validation Suite (%s)\n\n",CommandOptionToMnemonic(
            MagickValidateOptions,(ssize_t) type));
          if ((type & ColorspaceValidate) != 0)
            tests+=ValidateColorspaces(&fail,exception);
          if ((type & CompareValidate) != 0)
            tests+=ValidateCompareCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & CompositeValidate) != 0)
            tests+=ValidateCompositeCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & ConvertValidate) != 0)
            tests+=ValidateConvertCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & FormatsDiskValidate) != 0)
            {
              memory_resource=SetMagickResourceLimit(MemoryResource,0);
              map_resource=SetMagickResourceLimit(MapResource,0);
              (void) FormatLocaleFile(stdout,"[pixel-cache: disk] ");
              tests+=ValidateImageFormatsInMemory(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) FormatLocaleFile(stdout,"[pixel-cache: disk] ");
              tests+=ValidateImageFormatsOnDisk(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) SetMagickResourceLimit(MemoryResource,memory_resource);
              (void) SetMagickResourceLimit(MapResource,map_resource);
            }
          if ((type & FormatsMapValidate) != 0)
            {
              memory_resource=SetMagickResourceLimit(MemoryResource,0);
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory-mapped] ");
              tests+=ValidateImageFormatsInMemory(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory-mapped] ");
              tests+=ValidateImageFormatsOnDisk(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) SetMagickResourceLimit(MemoryResource,memory_resource);
            }
          if ((type & FormatsMemoryValidate) != 0)
            {
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory] ");
              tests+=ValidateImageFormatsInMemory(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory] ");
              tests+=ValidateImageFormatsOnDisk(image_info,reference_filename,
                output_filename,&fail,exception);
            }
          if ((type & IdentifyValidate) != 0)
            tests+=ValidateIdentifyCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & ImportExportValidate) != 0)
            tests+=ValidateImportExportPixels(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & MagickValidate) != 0)
            tests+=ValidateMagickCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & MontageValidate) != 0)
            tests+=ValidateMontageCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & StreamValidate) != 0)
            tests+=ValidateStreamCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          (void) FormatLocaleFile(stdout,
            "validation suite: %.20g tests; %.20g passed; %.20g failed.\n",
            (double) tests,(double) (tests-fail),(double) fail);
        }
      (void) RelinquishUniqueFileResource(output_filename);
      (void) ConcatenateMagickString(output_filename,"-0",MagickPathExtent);
      (void) RelinquishUniqueFileResource(output_filename);
      (void) RelinquishUniqueFileResource(reference_filename);
    }
  if (exception->severity != UndefinedException)
    CatchException(exception);
  if (iterations > 1)
    {
      elapsed_time=GetElapsedTime(timer);
      user_time=GetUserTime(timer);
      (void) FormatLocaleFile(stderr,
        "Performance: %.20gi %.3fips %0.6fu %ld:%02ld.%03ld\n",(double)
        iterations,1.0*iterations/elapsed_time,user_time,(long)
        (elapsed_time/60.0),(long) ceil(fmod(elapsed_time,60.0)),
        (long) (1000.0*(elapsed_time-floor(elapsed_time))));
      timer=DestroyTimerInfo(timer);
    }
  DestroyValidate();
  MagickCoreTerminus();
  return(fail == 0 ? 0 : 1);
}

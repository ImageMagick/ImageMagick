/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                 FFFFF  X   X                                %
%                                 F       X X                                 %
%                                 FFF      X                                  %
%                                 F       X X                                 %
%                                 F      X   X                                %
%                                                                             %
%                                                                             %
%                   MagickCore Image Special Effects Methods                  %
%                                                                             %
%                               Software Design                               %
%                             snibgo (Alan Gibson)                            %
%                                 January 2022                                %
%                                                                             %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/accelerate-private.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/decorate.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx.h"
#include "MagickCore/fx-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/policy.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resample-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/transform-private.h"
#include "MagickCore/utility.h"


#define MaxTokenLen 100
#define RpnInit 100
#define TableExtend 0.1
#define InitNumOprStack 50
#define MinValStackSize 100
#define InitNumUserSymbols 50

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#define __j0 _j0
#define __j1 _j1
#else
#define __j0 j0
#define __j1 j1
#endif

#define SECONDS_ERR -FLT_MAX

typedef long double fxFltType;

typedef enum {
  oAddEq,
  oSubtractEq,
  oMultiplyEq,
  oDivideEq,
  oPlusPlus,
  oSubSub,
  oAdd,
  oSubtract,
  oMultiply,
  oDivide,
  oModulus,
  oUnaryPlus,
  oUnaryMinus,
  oLshift,
  oRshift,
  oEq,
  oNotEq,
  oLtEq,
  oGtEq,
  oLt,
  oGt,
  oLogAnd,
  oLogOr,
  oLogNot,
  oBitAnd,
  oBitOr,
  oBitNot,
  oPow,
  oQuery,
  oColon,
  oOpenParen,
  oCloseParen,
  oOpenBracket,
  oCloseBracket,
  oOpenBrace,
  oCloseBrace,
  oAssign,
  oNull
} OperatorE;

typedef struct {
  OperatorE
    op;

  const char *
    str;

  int
    precedence, /* Higher number is higher precedence */
    number_args;
} OperatorT;

static const OperatorT Operators[] = {
  {oAddEq,       "+=",    12, 1},
  {oSubtractEq,  "-=",    12, 1},
  {oMultiplyEq,  "*=",    13, 1},
  {oDivideEq,    "/=",    13, 1},
  {oPlusPlus,    "++",    12, 0},
  {oSubSub,      "--",    12, 0},
  {oAdd,         "+",     12, 2},
  {oSubtract,    "-",     12, 2},
  {oMultiply,    "*",     13, 2},
  {oDivide,      "/",     13, 2},
  {oModulus,     "%",     13, 2},
  {oUnaryPlus,   "+",     14, 1},
  {oUnaryMinus,  "-",     14, 1},
  {oLshift,      "<<",    11, 2},
  {oRshift,      ">>",    11, 2},
  {oEq,          "==",     9, 2},
  {oNotEq,       "!=",     9, 2},
  {oLtEq,        "<=",    10, 2},
  {oGtEq,        ">=",    10, 2},
  {oLt,          "<",     10, 2},
  {oGt,          ">",     10, 2},
  {oLogAnd,      "&&",     6, 2},
  {oLogOr,       "||",     5, 2},
  {oLogNot,      "!",     16, 1},
  {oBitAnd,      "&",      8, 2},
  {oBitOr,       "|",      7, 2},
  {oBitNot,      "~",     16, 1},
  {oPow,         "^",     15, 2},
  {oQuery,       "?",      4, 1},
  {oColon,       ":",      4, 1},
  {oOpenParen,   "(",      0, 0},
  {oCloseParen,  ")",      0, 0},
  {oOpenBracket, "[",      0, 0},
  {oCloseBracket,"]",      0, 0},
  {oOpenBrace,   "{",      0, 0},
  {oCloseBrace,  "}",      0, 0},
  {oAssign,      "=",      3, 1},
  {oNull,        "onull",  17, 0}
};

typedef enum {
  cEpsilon,
  cE,
  cOpaque,
  cPhi,
  cPi,
  cQuantumRange,
  cQuantumScale,
  cTransparent,
  cMaxRgb,
  cNull
} ConstantE;

typedef struct {
  ConstantE
    cons;

  fxFltType
    val;

  const char
    *str;
} ConstantT;

static const ConstantT Constants[] = {
  {cEpsilon,      MagickEpsilon,         "epsilon"},
  {cE,            2.7182818284590452354, "e"},
  {cOpaque,       1.0,                   "opaque"},
  {cPhi,          MagickPHI,             "phi"},
  {cPi,           MagickPI,              "pi"},
  {cQuantumRange, QuantumRange,          "quantumrange"},
  {cQuantumScale, QuantumScale,          "quantumscale"},
  {cTransparent,  0.0,                   "transparent"},
  {cMaxRgb,       QuantumRange,          "MaxRGB"},
  {cNull,         0.0,                   "cnull"}
};

#define FirstFunc ((FunctionE) (oNull+1))

typedef enum {
  fAbs = oNull+1,
#if defined(MAGICKCORE_HAVE_ACOSH)
  fAcosh,
#endif
  fAcos,
#if defined(MAGICKCORE_HAVE_J1)
  fAiry,
#endif
  fAlt,
#if defined(MAGICKCORE_HAVE_ASINH)
  fAsinh,
#endif
  fAsin,
#if defined(MAGICKCORE_HAVE_ATANH)
  fAtanh,
#endif
  fAtan2,
  fAtan,
  fCeil,
  fChannel,
  fClamp,
  fCosh,
  fCos,
  fDebug,
  fDrc,
#if defined(MAGICKCORE_HAVE_ERF)
  fErf,
#endif
  fEpoch,
  fExp,
  fFloor,
  fGauss,
  fGcd,
  fHypot,
  fInt,
  fIsnan,
#if defined(MAGICKCORE_HAVE_J0)
  fJ0,
#endif
#if defined(MAGICKCORE_HAVE_J1)
  fJ1,
#endif
#if defined(MAGICKCORE_HAVE_J1)
  fJinc,
#endif
  fLn,
  fLogtwo,
  fLog,
  fMagickTime,
  fMax,
  fMin,
  fMod,
  fNot,
  fPow,
  fRand,
  fRound,
  fSign,
  fSinc,
  fSinh,
  fSin,
  fSqrt,
  fSquish,
  fTanh,
  fTan,
  fTrunc,
  fDo,
  fFor,
  fIf,
  fWhile,
  fU,
  fU0,
  fUP,
  fS,
  fV,
  fP,
  fSP,
  fVP,

  fNull
} FunctionE;

typedef struct {
  FunctionE
    func;

  const char
    *str;

  int
    number_args;
} FunctionT;

static const FunctionT Functions[] = {
  {fAbs,     "abs"   , 1},
#if defined(MAGICKCORE_HAVE_ACOSH)
  {fAcosh,   "acosh" , 1},
#endif
  {fAcos,    "acos"  , 1},
#if defined(MAGICKCORE_HAVE_J1)
  {fAiry,    "airy"  , 1},
#endif
  {fAlt,     "alt"   , 1},
#if defined(MAGICKCORE_HAVE_ASINH)
  {fAsinh,   "asinh" , 1},
#endif
  {fAsin,    "asin"  , 1},
#if defined(MAGICKCORE_HAVE_ATANH)
  {fAtanh,   "atanh" , 1},
#endif
  {fAtan2,   "atan2" , 2},
  {fAtan,    "atan"  , 1},
  {fCeil,    "ceil"  , 1},
  {fChannel, "channel", 5}, /* Special case: allow zero to five arguments. */
  {fClamp,   "clamp" , 1},
  {fCosh,    "cosh"  , 1},
  {fCos,     "cos"   , 1},
  {fDebug,   "debug" , 1},
  {fDrc,     "drc"   , 2},
#if defined(MAGICKCORE_HAVE_ERF)
  {fErf,     "erf"   , 1},
#endif
  {fEpoch,   "epoch" , 1}, /* Special case: needs a string date from a property eg %[date:modify] */
  {fExp,     "exp"   , 1},
  {fFloor,   "floor" , 1},
  {fGauss,   "gauss" , 1},
  {fGcd,     "gcd"   , 2},
  {fHypot,   "hypot" , 2},
  {fInt,     "int"   , 1},
  {fIsnan,   "isnan" , 1},
#if defined(MAGICKCORE_HAVE_J0)
  {fJ0,      "j0"    , 1},
#endif
#if defined(MAGICKCORE_HAVE_J1)
  {fJ1,      "j1"    , 1},
#endif
#if defined(MAGICKCORE_HAVE_J1)
  {fJinc,    "jinc"  , 1},
#endif
  {fLn,      "ln"    , 1},
  {fLogtwo,  "logtwo", 1},
  {fLog,     "log"   , 1},
  {fMagickTime,"magicktime", 0},
  {fMax,     "max"   , 2},
  {fMin,     "min"   , 2},
  {fMod,     "mod"   , 2},
  {fNot,     "not"   , 1},
  {fPow,     "pow"   , 2},
  {fRand,    "rand"  , 0},
  {fRound,   "round" , 1},
  {fSign,    "sign"  , 1},
  {fSinc,    "sinc"  , 1},
  {fSinh,    "sinh"  , 1},
  {fSin,     "sin"   , 1},
  {fSqrt,    "sqrt"  , 1},
  {fSquish,  "squish", 1},
  {fTanh,    "tanh"  , 1},
  {fTan,     "tan"   , 1},
  {fTrunc,   "trunc" , 1},
  {fDo,      "do",     2},
  {fFor,     "for",    3},
  {fIf,      "if",     3},
  {fWhile,   "while",  2},
  {fU,       "u",      1},
  {fU0,      "u0",     0},
  {fUP,      "up",     3},
  {fS,       "s",      0},
  {fV,       "v",      0},
  {fP,       "p",      2},
  {fSP,      "sp",     2},
  {fVP,      "vp",     2},

  {fNull,    "fnull" , 0}
};

#define FirstImgAttr ((ImgAttrE) (fNull+1))

typedef enum {
  aDepth = fNull+1,
  aExtent,
  aKurtosis,
  aMaxima,
  aMean,
  aMedian,
  aMinima,
  aPage,
  aPageX,
  aPageY,
  aPageWid,
  aPageHt,
  aPrintsize,
  aPrintsizeX,
  aPrintsizeY,
  aQuality,
  aRes,
  aResX,
  aResY,
  aSkewness,
  aStdDev,
  aH,
  aN,
  aT,
  aW,
  aZ,
  aNull
} ImgAttrE;

typedef struct {
  ImgAttrE
    attr;

  const char
    *str;

  MagickBooleanType
    need_stats;
} ImgAttrT;

static const ImgAttrT ImgAttrs[] = {
  {aDepth,      "depth",              MagickTrue},
  {aExtent,     "extent",             MagickFalse},
  {aKurtosis,   "kurtosis",           MagickTrue},
  {aMaxima,     "maxima",             MagickTrue},
  {aMean,       "mean",               MagickTrue},
  {aMedian,     "median",             MagickTrue},
  {aMinima,     "minima",             MagickTrue},
  {aPage,       "page",               MagickFalse},
  {aPageX,      "page.x",             MagickFalse},
  {aPageY,      "page.y",             MagickFalse},
  {aPageWid,    "page.width",         MagickFalse},
  {aPageHt,     "page.height",        MagickFalse},
  {aPrintsize,  "printsize",          MagickFalse},
  {aPrintsizeX, "printsize.x",        MagickFalse},
  {aPrintsizeY, "printsize.y",        MagickFalse},
  {aQuality,    "quality",            MagickFalse},
  {aRes,        "resolution",         MagickFalse},
  {aResX,       "resolution.x",       MagickFalse},
  {aResY,       "resolution.y",       MagickFalse},
  {aSkewness,   "skewness",           MagickTrue},
  {aStdDev,     "standard_deviation", MagickTrue},
  {aH,          "h",                  MagickFalse},
  {aN,          "n",                  MagickFalse},
  {aT,          "t",                  MagickFalse},
  {aW,          "w",                  MagickFalse},
  {aZ,          "z",                  MagickFalse},
  {aNull,       "anull",              MagickFalse},
  {aNull,       "anull",              MagickFalse},
  {aNull,       "anull",              MagickFalse},
  {aNull,       "anull",              MagickFalse}
};

#define FirstSym ((SymbolE) (aNull+1))

typedef enum {
  sHue = aNull+1,
  sIntensity,
  sLightness,
  sLuma,
  sLuminance,
  sSaturation,
  sA,
  sB,
  sC,
  sG,
  sI,
  sJ,
  sK,
  sM,
  sO,
  sR,
  sY,
  sNull
} SymbolE;

typedef struct {
  SymbolE
    sym;

  const char
    *str;
} SymbolT;

static const SymbolT Symbols[] = {
  {sHue,         "hue"},
  {sIntensity,   "intensity"},
  {sLightness,   "lightness"},
  {sLuma,        "luma"},
  {sLuminance,   "luminance"},
  {sSaturation,  "saturation"},
  {sA,           "a"},
  {sB,           "b"},
  {sC,           "c"},
  {sG,           "g"},
  {sI,           "i"},
  {sJ,           "j"},
  {sK,           "k"},
  {sM,           "m"},
  {sO,           "o"},
  {sR,           "r"},
  {sY,           "y"},
  {sNull,        "snull"}
};
/*
   There is no way to access new value of pixels. This might be a future enhancement, eg "q".
   fP, oU and oV can have channel qualifier such as "u.r".
   For meta channels, we might also allow numbered channels eg "u.2" or "u.16".
   ... or have extra argument to p[].
*/

#define FirstCont (sNull+1)

/* Run-time controls are in the RPN, not explicitly in the input string. */
typedef enum {
  rGoto = FirstCont,
  rGotoChk,
  rIfZeroGoto,
  rIfNotZeroGoto,
  rCopyFrom,
  rCopyTo,
  rZerStk,
  rNull
} ControlE;

typedef struct {
  ControlE
    cont;

  const char
    *str;

  int
    number_args;
} ControlT;

static const ControlT Controls[] = {
  {rGoto,          "goto",          0},
  {rGotoChk,       "gotochk",       0},
  {rIfZeroGoto,    "ifzerogoto",    1},
  {rIfNotZeroGoto, "ifnotzerogoto", 1},
  {rCopyFrom,      "copyfrom",      0},
  {rCopyTo,        "copyto",        1},
  {rZerStk,        "zerstk",        0},
  {rNull,          "rnull",         0}
};

#define NULL_ADDRESS -2

typedef struct {
  int
    addr_query,
    addr_colon;
} TernaryT;

typedef struct {
  const char
    *str;

  PixelChannel
    pixel_channel;
} ChannelT;

#define NO_CHAN_QUAL      ((PixelChannel) (-1))
#define THIS_CHANNEL      ((PixelChannel) (-2))
#define HUE_CHANNEL       ((PixelChannel) (-3))
#define SAT_CHANNEL       ((PixelChannel) (-4))
#define LIGHT_CHANNEL     ((PixelChannel) (-5))
#define INTENSITY_CHANNEL ((PixelChannel) (-6))

static const ChannelT Channels[] = {
  {"r",          RedPixelChannel},
  {"g",          GreenPixelChannel},
  {"b",          BluePixelChannel},
  {"c",          CyanPixelChannel},
  {"m",          MagentaPixelChannel},
  {"y",          YellowPixelChannel},
  {"k",          BlackPixelChannel},
  {"a",          AlphaPixelChannel},
  {"o",          AlphaPixelChannel},
  {"hue",        HUE_CHANNEL},
  {"saturation", SAT_CHANNEL},
  {"lightness",  LIGHT_CHANNEL},
  {"intensity",  INTENSITY_CHANNEL},
  {"all",        CompositePixelChannel},
  {"this",       THIS_CHANNEL},
  {"",           NO_CHAN_QUAL}
};

/* The index into UserSymbols is also the index into run-time UserSymVals.
*/
typedef struct {
  char
    *pex;

  size_t
    len;
} UserSymbolT;

typedef enum {
  etOperator,
  etConstant,
  etFunction,
  etImgAttr,
  etSymbol,
  etColourConstant,
  etControl
} ElementTypeE;

static const char * sElementTypes[] = {
  "Operator",
  "Constant",
  "Function",
  "ImgAttr",
  "Symbol",
  "ColConst",
  "Control"
};

typedef struct {
  char
    *exp_start;

  ElementTypeE
    type;

  fxFltType
    val,
    val1,
    val2;

  ImgAttrE
    img_attr_qual;

  int
    element_index,
    number_args,
    number_dest, /* Number of Elements that "goto" this element */
    operator_index;

  MagickBooleanType
    do_push,
    is_relative;

  PixelChannel
    channel_qual;

  size_t
    exp_len;
} ElementT;

typedef enum {
  rtUnknown,
  rtEntireImage,
  rtCornerOnly
} RunTypeE;

typedef struct {
  CacheView *View;
  /* Other per-image metadata could go here. */
} ImgT;

typedef struct {
  RandomInfo * magick_restrict random_info;
  int numValStack;
  int usedValStack;
  fxFltType * ValStack;
  fxFltType * UserSymVals;
  Quantum * thisPixel;
} fxRtT;

struct _FxInfo {
  Image * image;
  size_t ImgListLen;
  ssize_t ImgNum;
  MagickBooleanType NeedStats;
  MagickBooleanType GotStats;
  MagickBooleanType NeedHsl;
  MagickBooleanType DebugOpt;       /* Whether "-debug" option is in effect */
  MagickBooleanType ContainsDebug;  /* Whether expression contains "debug ()" function */
  char * expression;
  char * pex;
  char ShortExp[MagickPathExtent]; /* for reporting */
  int teDepth;
  char token[MagickPathExtent];
  size_t lenToken;
  int numElements;
  int usedElements;
  ElementT * Elements;  /* Elements is read-only at runtime. */
  int numUserSymbols;
  int usedUserSymbols;
  UserSymbolT * UserSymbols;
  int numOprStack;
  int usedOprStack;
  int maxUsedOprStack;
  OperatorE * OperatorStack;
  ChannelStatistics ** statistics;
  int precision;
  RunTypeE runType;

  RandomInfo
    **magick_restrict random_infos;

  ImgT * Imgs;
  Image ** Images;

  ExceptionInfo * exception;

  fxRtT * fxrts;
};

/* Forward declarations for recursion.
*/
static MagickBooleanType TranslateStatementList
  (FxInfo * pfx, const char * strLimit, char * chLimit);

static MagickBooleanType TranslateExpression
  (FxInfo * pfx, const char * strLimit, char * chLimit, MagickBooleanType * needPopAll);

static MagickBooleanType GetFunction (FxInfo * pfx, FunctionE fe);

static inline MagickBooleanType ChanIsVirtual (PixelChannel pc)
{
  if (pc==HUE_CHANNEL || pc==SAT_CHANNEL || pc==LIGHT_CHANNEL || pc==INTENSITY_CHANNEL)
    return MagickTrue;

  return MagickFalse;
}

static MagickBooleanType InitFx (FxInfo * pfx, const Image * img,
  MagickBooleanType CalcAllStats, ExceptionInfo *exception)
{
  ssize_t i=0;
  const Image * next;

  pfx->ImgListLen = GetImageListLength (img);
  pfx->ImgNum = GetImageIndexInList (img);
  pfx->image = (Image *)img;

  pfx->NeedStats = MagickFalse;
  pfx->GotStats = MagickFalse;
  pfx->NeedHsl = MagickFalse;
  pfx->DebugOpt = IsStringTrue (GetImageArtifact (img, "fx:debug"));
  pfx->statistics = NULL;
  pfx->Imgs = NULL;
  pfx->Images = NULL;
  pfx->exception = exception;
  pfx->precision = GetMagickPrecision ();
  pfx->random_infos = AcquireRandomInfoTLS ();
  pfx->ContainsDebug = MagickFalse;
  pfx->runType = (CalcAllStats) ? rtEntireImage : rtCornerOnly;
  pfx->Imgs = (ImgT *)AcquireQuantumMemory (pfx->ImgListLen, sizeof (ImgT));
  if (!pfx->Imgs) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "Imgs", "%lu",
      (unsigned long) pfx->ImgListLen);
    return MagickFalse;
  }

  next = GetFirstImageInList (img);
  for ( ; next != (Image *) NULL; next=next->next)
  {
    ImgT * pimg = &pfx->Imgs[i];
    pimg->View = AcquireVirtualCacheView (next, pfx->exception);
    if (!pimg->View) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), ResourceLimitFatalError,
        "View", "[%li]",
        (long) i);
      /* dealloc any done so far, and Imgs */
      for ( ; i > 0; i--) {
        pimg = &pfx->Imgs[i-1];
        pimg->View = DestroyCacheView (pimg->View);
      }
      pfx->Imgs=(ImgT *) RelinquishMagickMemory (pfx->Imgs);
      return MagickFalse;
    }
    i++;
  }

  pfx->Images = ImageListToArray (img, pfx->exception);

  return MagickTrue;
}

static MagickBooleanType DeInitFx (FxInfo * pfx)
{
  ssize_t i;

  if (pfx->Images) pfx->Images = (Image**) RelinquishMagickMemory (pfx->Images);

  if (pfx->Imgs) {
    for (i = (ssize_t)GetImageListLength(pfx->image); i > 0; i--) {
      ImgT * pimg = &pfx->Imgs[i-1];
      pimg->View = DestroyCacheView (pimg->View);
    }
    pfx->Imgs=(ImgT *) RelinquishMagickMemory (pfx->Imgs);
  }
  pfx->random_infos = DestroyRandomInfoTLS (pfx->random_infos);

  if (pfx->statistics) {
    for (i = (ssize_t)GetImageListLength(pfx->image); i > 0; i--) {
      pfx->statistics[i-1]=(ChannelStatistics *) RelinquishMagickMemory (pfx->statistics[i-1]);
    }

    pfx->statistics = (ChannelStatistics**) RelinquishMagickMemory(pfx->statistics);
  }

  return MagickTrue;
}

static ElementTypeE TypeOfOpr (int op)
{
  if (op <  oNull) return etOperator;
  if (op == oNull) return etConstant;
  if (op <= fNull) return etFunction;
  if (op <= aNull) return etImgAttr;
  if (op <= sNull) return etSymbol;
  if (op <= rNull) return etControl;

  return (ElementTypeE) 0;
}

static char * SetPtrShortExp (FxInfo * pfx, char * pExp, size_t len)
{
  #define MaxLen 20

  size_t slen;
  char * p;

  *pfx->ShortExp = '\0';

  if (pExp && len) {
    slen = CopyMagickString (pfx->ShortExp, pExp, len);
    if (slen > MaxLen) {
      (void) CopyMagickString (pfx->ShortExp+MaxLen, "...", 4);
    }
    p = strchr (pfx->ShortExp, '\n');
    if (p) (void) CopyMagickString (p, "...", 4);
    p = strchr (pfx->ShortExp, '\r');
    if (p) (void) CopyMagickString (p, "...", 4);
  }
  return pfx->ShortExp;
}

static char * SetShortExp (FxInfo * pfx)
{
  return SetPtrShortExp (pfx, pfx->pex, MaxTokenLen-1);
}

static int FindUserSymbol (FxInfo * pfx, char * name)
/* returns index into pfx->UserSymbols, and thus into pfxrt->UserSymVals,
   or NULL_ADDRESS if not found.
*/
{
  int i;
  size_t lenName;
  lenName = strlen (name);
  for (i=0; i < pfx->usedUserSymbols; i++) {
    UserSymbolT *pus = &pfx->UserSymbols[i];
    if (lenName == pus->len && LocaleNCompare (name, pus->pex, lenName)==0) break;
  }
  if (i == pfx->usedUserSymbols) return NULL_ADDRESS;
  return i;
}

static MagickBooleanType ExtendUserSymbols (FxInfo * pfx)
{
  pfx->numUserSymbols = (int) ceil (pfx->numUserSymbols * (1 + TableExtend));
  pfx->UserSymbols = (UserSymbolT*) ResizeMagickMemory (pfx->UserSymbols, (size_t) pfx->numUserSymbols * sizeof(UserSymbolT));
  if (!pfx->UserSymbols) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "UserSymbols", "%i",
      pfx->numUserSymbols);
    return MagickFalse;
  }

  return MagickTrue;
}

static int AddUserSymbol (FxInfo * pfx, char * pex, size_t len)
{
  UserSymbolT *pus;
  if (++pfx->usedUserSymbols >= pfx->numUserSymbols) {
    if (!ExtendUserSymbols (pfx)) return -1;
  }
  pus = &pfx->UserSymbols[pfx->usedUserSymbols-1];
  pus->pex = pex;
  pus->len = len;

  return pfx->usedUserSymbols-1;
}

static void DumpTables (FILE * fh)
{

  int i;
  for (i=0; i <= rNull; i++) {
    const char * str = "";
    if (                     i < oNull) str = Operators[i].str;
    if (i >= (int) FirstFunc    && i < fNull) str = Functions[i-(int) FirstFunc].str;
    if (i >= (int) FirstImgAttr && i < aNull) str = ImgAttrs[i-(int) FirstImgAttr].str;
    if (i >= (int) FirstSym     && i < sNull) str = Symbols[i-(int) FirstSym].str;
    if (i >= (int) FirstCont    && i < rNull) str = Controls[i-(int) FirstCont].str;
    if      (i==0    ) fprintf (stderr, "Operators:\n ");
    else if (i==oNull) fprintf (stderr, "\nFunctions:\n ");
    else if (i==fNull) fprintf (stderr, "\nImage attributes:\n ");
    else if (i==aNull) fprintf (stderr, "\nSymbols:\n ");
    else if (i==sNull) fprintf (stderr, "\nControls:\n ");
    fprintf (fh, " %s", str);
  }
  fprintf (fh, "\n");
}

static char * NameOfUserSym (FxInfo * pfx, int ndx, char * buf)
{
  UserSymbolT * pus;
  assert (ndx >= 0 && ndx < pfx->usedUserSymbols);
  pus = &pfx->UserSymbols[ndx];
  (void) CopyMagickString (buf, pus->pex, pus->len+1);
  return buf;
}

static void DumpUserSymbols (FxInfo * pfx, FILE * fh)
{
  char UserSym[MagickPathExtent];
  int i;
  fprintf (fh, "UserSymbols (%i)\n", pfx->usedUserSymbols);
  for (i=0; i < pfx->usedUserSymbols; i++) {
    fprintf (fh, "  %i: '%s'\n", i, NameOfUserSym (pfx, i, UserSym));
  }
}

static MagickBooleanType BuildRPN (FxInfo * pfx)
{
  pfx->numUserSymbols = InitNumUserSymbols;
  pfx->usedUserSymbols = 0;
  pfx->UserSymbols = (UserSymbolT*) AcquireMagickMemory ((size_t) pfx->numUserSymbols * sizeof(UserSymbolT));
  if (!pfx->UserSymbols) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "UserSymbols", "%i",
      pfx->numUserSymbols);
    return MagickFalse;
  }

  pfx->numElements = RpnInit;
  pfx->usedElements = 0;
  pfx->Elements = NULL;

  pfx->Elements = (ElementT*) AcquireMagickMemory ((size_t) pfx->numElements * sizeof(ElementT));

  if (!pfx->Elements) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "Elements", "%i",
      pfx->numElements);
    return MagickFalse;
  }

  pfx->usedOprStack = 0;
  pfx->maxUsedOprStack = 0;
  pfx->numOprStack = InitNumOprStack;
  pfx->OperatorStack = (OperatorE*) AcquireMagickMemory ((size_t) pfx->numOprStack * sizeof(OperatorE));
  if (!pfx->OperatorStack) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "OperatorStack", "%i",
      pfx->numOprStack);
    return MagickFalse;
  }

  return MagickTrue;
}

static MagickBooleanType AllocFxRt (FxInfo * pfx, fxRtT * pfxrt)
{
  int nRnd;
  int i;
  pfxrt->random_info = AcquireRandomInfo ();
  pfxrt->thisPixel = NULL;

  nRnd = 20 + 10 * (int) GetPseudoRandomValue (pfxrt->random_info);
  for (i=0; i < nRnd; i++) (void) GetPseudoRandomValue (pfxrt->random_info);;

  pfxrt->usedValStack = 0;
  pfxrt->numValStack = 2 * pfx->maxUsedOprStack;
  if (pfxrt->numValStack < MinValStackSize) pfxrt->numValStack = MinValStackSize;
  pfxrt->ValStack = (fxFltType*) AcquireMagickMemory ((size_t) pfxrt->numValStack * sizeof(fxFltType));
  if (!pfxrt->ValStack) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "ValStack", "%i",
      pfxrt->numValStack);
    return MagickFalse;
  }

  pfxrt->UserSymVals = NULL;

  if (pfx->usedUserSymbols) {
    pfxrt->UserSymVals = (fxFltType*) AcquireMagickMemory ((size_t) pfx->usedUserSymbols * sizeof(fxFltType));
    if (!pfxrt->UserSymVals) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), ResourceLimitFatalError,
        "UserSymVals", "%i",
        pfx->usedUserSymbols);
      return MagickFalse;
    }
    for (i = 0; i < pfx->usedUserSymbols; i++) pfxrt->UserSymVals[i] = (fxFltType) 0;
  }

  return MagickTrue;
}

static MagickBooleanType ExtendRPN (FxInfo * pfx)
{
  pfx->numElements = (int) ceil (pfx->numElements * (1 + TableExtend));
  pfx->Elements = (ElementT*) ResizeMagickMemory (pfx->Elements, (size_t) pfx->numElements * sizeof(ElementT));
  if (!pfx->Elements) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "Elements", "%i",
      pfx->numElements);
    return MagickFalse;
  }
  return MagickTrue;
}

static inline MagickBooleanType OprInPlace (int op)
{
  return (op >= oAddEq && op <= oSubSub ? MagickTrue : MagickFalse);
}

static const char * OprStr (int oprNum)
{
  const char * str;
  if      (oprNum < 0) str = "bad OprStr";
  else if (oprNum <= oNull) str = Operators[oprNum].str;
  else if (oprNum <= fNull) str = Functions[oprNum-(int) FirstFunc].str;
  else if (oprNum <= aNull) str = ImgAttrs[oprNum-(int) FirstImgAttr].str;
  else if (oprNum <= sNull) str = Symbols[oprNum-(int) FirstSym].str;
  else if (oprNum <= rNull) str = Controls[oprNum-(int) FirstCont].str;
  else {
    str = "bad OprStr";
  }
  return str;
}

static MagickBooleanType DumpRPN (FxInfo * pfx, FILE * fh)
{
  int i;

  fprintf (fh, "DumpRPN:");
  fprintf (fh, "  numElements=%i", pfx->numElements);
  fprintf (fh, "  usedElements=%i", pfx->usedElements);
  fprintf (fh, "  maxUsedOprStack=%i", pfx->maxUsedOprStack);
  fprintf (fh, "  ImgListLen=%g", (double) pfx->ImgListLen);
  fprintf (fh, "  NeedStats=%s", pfx->NeedStats ? "yes" : "no");
  fprintf (fh, "  GotStats=%s", pfx->GotStats ? "yes" : "no");
  fprintf (fh, "  NeedHsl=%s\n", pfx->NeedHsl ? "yes" : "no");
  if      (pfx->runType==rtEntireImage) fprintf (stderr, "EntireImage");
  else if (pfx->runType==rtCornerOnly)  fprintf (stderr, "CornerOnly");
  fprintf (fh, "\n");


  for (i=0; i < pfx->usedElements; i++) {
    ElementT * pel = &pfx->Elements[i];
    pel->number_dest = 0;
  }
  for (i=0; i < pfx->usedElements; i++) {
    ElementT * pel = &pfx->Elements[i];
    if (pel->operator_index == rGoto || pel->operator_index == rGotoChk || pel->operator_index == rIfZeroGoto || pel->operator_index == rIfNotZeroGoto) {
      if (pel->element_index >= 0 && pel->element_index < pfx->numElements) {
        ElementT * pelDest = &pfx->Elements[pel->element_index];
        pelDest->number_dest++;
      }
    }
  }
  for (i=0; i < pfx->usedElements; i++) {
    char UserSym[MagickPathExtent];

    ElementT * pel = &pfx->Elements[i];
    const char * str = OprStr (pel->operator_index);
    const char *sRelAbs = "";

    if (pel->operator_index == fP || pel->operator_index == fUP || pel->operator_index == fVP || pel->operator_index == fSP)
      sRelAbs = pel->is_relative ? "[]" : "{}";

    if (pel->type == etColourConstant)
      fprintf (fh, "  %i: %s vals=%.*Lg,%.*Lg,%.*Lg '%s%s' nArgs=%i ndx=%i  %s",
               i, sElementTypes[pel->type],
               pfx->precision, pel->val, pfx->precision, pel->val1, pfx->precision, pel->val2,
               str, sRelAbs, pel->number_args, pel->element_index,
               pel->do_push ? "push" : "NO push");
    else
      fprintf (fh, "  %i: %s val=%.*Lg '%s%s' nArgs=%i ndx=%i  %s",
               i, sElementTypes[pel->type], pfx->precision, pel->val, str, sRelAbs,
               pel->number_args, pel->element_index,
               pel->do_push ? "push" : "NO push");

    if (pel->img_attr_qual != aNull)
      fprintf (fh, " ia=%s", OprStr((int) pel->img_attr_qual));

    if (pel->channel_qual != NO_CHAN_QUAL) {
      if (pel->channel_qual == THIS_CHANNEL) fprintf (stderr, "  ch=this");
      else fprintf (stderr, "  ch=%i", pel->channel_qual);
    }

    if (pel->operator_index == rCopyTo) {
      fprintf (fh, "  CopyTo ==> %s", NameOfUserSym (pfx, pel->element_index, UserSym));
    } else if (pel->operator_index == rCopyFrom) {
      fprintf (fh, "  CopyFrom <== %s", NameOfUserSym (pfx, pel->element_index, UserSym));
    } else if (OprInPlace (pel->operator_index)) {
      fprintf (fh, "  <==> %s", NameOfUserSym (pfx, pel->element_index, UserSym));
    }
    if (pel->number_dest > 0)  fprintf (fh, "  <==dest(%i)", pel->number_dest);
    fprintf (fh, "\n");
  }
  return MagickTrue;
}

static void DestroyRPN (FxInfo * pfx)
{
  pfx->numOprStack = 0;
  pfx->usedOprStack = 0;
  if (pfx->OperatorStack) pfx->OperatorStack = (OperatorE*) RelinquishMagickMemory (pfx->OperatorStack);

  pfx->numElements = 0;
  pfx->usedElements = 0;
  if (pfx->Elements) pfx->Elements = (ElementT*) RelinquishMagickMemory (pfx->Elements);

  pfx->usedUserSymbols = 0;
  if (pfx->UserSymbols) pfx->UserSymbols = (UserSymbolT*) RelinquishMagickMemory (pfx->UserSymbols);
}

static void DestroyFxRt (fxRtT * pfxrt)
{
  pfxrt->usedValStack = 0;
  if (pfxrt->ValStack) pfxrt->ValStack = (fxFltType*) RelinquishMagickMemory (pfxrt->ValStack);
  if (pfxrt->UserSymVals) pfxrt->UserSymVals = (fxFltType*) RelinquishMagickMemory (pfxrt->UserSymVals);

  pfxrt->random_info = DestroyRandomInfo (pfxrt->random_info);
}

static size_t GetToken (FxInfo * pfx)
/* Returns length of token that starts with an alpha,
     or 0 if it isn't a token that starts with an alpha.
   j0 and j1 have trailing digit.
   Also colours like "gray47" have more trailing digits.
   After initial alpha(s) also allow single "_", eg "standard_deviation".
   Does not advance pfx->pex.
   This splits "mean.r" etc.
*/
{

  char * p = pfx->pex;
  size_t len = 0;
  *pfx->token = '\0';
  pfx->lenToken = 0;
  if (!isalpha((int)*p)) return 0;

  /* Regard strings that start "icc-" or "device-",
     followed by any number of alphas,
     as a token.
  */

  if (LocaleNCompare (p, "icc-", 4) == 0) {
    len = 4;
    p += 4;
    while (isalpha ((int)*p)) { len++; p++; }
  } else if (LocaleNCompare (p, "device-", 7) == 0) {
    len = 7;
    p += 7;
    while (isalpha ((int)*p)) { len++; p++; }
  } else {
    while (isalpha ((int)*p)) { len++; p++; }
    if (*p == '_')            { len++; p++; }
    while (isalpha ((int)*p)) { len++; p++; }
    while (isdigit ((int)*p)) { len++; p++; }
  }
  if (len >= MaxTokenLen) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "GetToken: too long", "%g at '%s'",
      (double) len, SetShortExp(pfx));
    len = MaxTokenLen;
  }
  if (len) {
    (void) CopyMagickString (pfx->token, pfx->pex, (len+1<MaxTokenLen)?len+1:MaxTokenLen);
  }

  pfx->lenToken = strlen (pfx->token);
  return len;
}

static MagickBooleanType TokenMaybeUserSymbol (FxInfo * pfx)
{
  char * p = pfx->token;
  int i = 0;
  while (*p) {
    if (!isalpha ((int)*p++)) return MagickFalse;
    i++;
  }
  if (i < 2) return MagickFalse;
  return MagickTrue;
}

static MagickBooleanType AddElement (FxInfo * pfx, fxFltType val, int oprNum)
{
  ElementT * pel;

  assert (oprNum <= rNull);

  if (++pfx->usedElements >= pfx->numElements) {
    if (!ExtendRPN (pfx)) return MagickFalse;
  }

  pel = &pfx->Elements[pfx->usedElements-1];
  pel->type = TypeOfOpr (oprNum);
  pel->val = val;
  pel->val1 = (fxFltType) 0;
  pel->val2 = (fxFltType) 0;
  pel->operator_index = oprNum;
  pel->do_push = MagickTrue;
  pel->element_index = 0;
  pel->channel_qual = NO_CHAN_QUAL;
  pel->img_attr_qual = aNull;
  pel->number_dest = 0;
  pel->exp_start = NULL;
  pel->exp_len = 0;

  if (oprNum <= oNull) pel->number_args = Operators[oprNum].number_args;
  else if (oprNum <= fNull) pel->number_args = Functions[oprNum-(int) FirstFunc].number_args;
  else if (oprNum <= aNull) pel->number_args = 0;
  else if (oprNum <= sNull) pel->number_args = 0;
  else                      pel->number_args = Controls[oprNum-(int) FirstCont].number_args;

  return MagickTrue;
}

static MagickBooleanType AddAddressingElement (FxInfo * pfx, int oprNum, int EleNdx)
{
  ElementT * pel;
  if (!AddElement (pfx, (fxFltType) 0, oprNum)) return MagickFalse;
  pel = &pfx->Elements[pfx->usedElements-1];
  pel->element_index = EleNdx;
  if (oprNum == rGoto || oprNum == rGotoChk || oprNum == rIfZeroGoto || oprNum == rIfNotZeroGoto
   || oprNum == rZerStk)
  {
    pel->do_push = MagickFalse;
  }

  /* Note: for() may or may not need pushing,
     depending on whether the value is needed, eg "for(...)+2" or debug(for(...)).
  */

  return MagickTrue;
}

static MagickBooleanType AddColourElement (FxInfo * pfx, fxFltType val0, fxFltType val1, fxFltType val2)
{
  ElementT * pel;
  if (!AddElement (pfx, val0, oNull)) return MagickFalse;
  pel = &pfx->Elements[pfx->usedElements-1];
  pel->val1 = val1;
  pel->val2 = val2;
  pel->type = etColourConstant;
  return MagickTrue;
}

static inline void SkipSpaces (FxInfo * pfx)
{
  while (isspace ((int)*pfx->pex)) pfx->pex++;
}

static inline char PeekChar (FxInfo * pfx)
{
  SkipSpaces (pfx);
  return *pfx->pex;
}

static inline MagickBooleanType PeekStr (FxInfo * pfx, const char * str)
{
  SkipSpaces (pfx);

  return (LocaleNCompare (pfx->pex, str, strlen(str))==0 ? MagickTrue : MagickFalse);
}

static MagickBooleanType ExpectChar (FxInfo * pfx, char c)
{
  if (PeekChar (pfx) != c) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Expected char", "'%c' at '%s'", c, SetShortExp (pfx));
    return MagickFalse;
  }
  pfx->pex++;
  return MagickTrue;
}

static int MaybeXYWH (FxInfo * pfx, ImgAttrE * pop)
/* If ".x" or ".y" or ".width" or ".height" increments *pop and returns 1 to 4 .
   Otherwise returns 0.
*/
{
  int ret=0;

  if (*pop != aPage && *pop != aPrintsize && *pop != aRes) return 0;

  if (PeekChar (pfx) != '.') return 0;

  if (!ExpectChar (pfx, '.')) return 0;

  (void) GetToken (pfx);
  if (LocaleCompare ("x", pfx->token)==0) ret=1;
  else if (LocaleCompare ("y", pfx->token)==0) ret=2;
  else if (LocaleCompare ("width", pfx->token)==0) ret=3;
  else if (LocaleCompare ("height", pfx->token)==0) ret=4;

  if (!ret)
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Invalid 'x' or 'y' or 'width' or 'height' token=", "'%s' at '%s'",
      pfx->token, SetShortExp(pfx));

  if (*pop == aPage) (*pop) = (ImgAttrE) ((int) *pop + ret);
  else {
    if (ret > 2) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Invalid 'width' or 'height' token=", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
    } else {
      (*pop) = (ImgAttrE) ((int) *pop + ret);
    }
  }
  pfx->pex+=pfx->lenToken;

  return ret;
}

static MagickBooleanType ExtendOperatorStack (FxInfo * pfx)
{
  pfx->numOprStack = (int) ceil (pfx->numOprStack * (1 + TableExtend));
  pfx->OperatorStack = (OperatorE*) ResizeMagickMemory (pfx->OperatorStack, (size_t) pfx->numOprStack * sizeof(OperatorE));
  if (!pfx->OperatorStack) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "OprStack", "%i",
      pfx->numOprStack);
    return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType PushOperatorStack (FxInfo * pfx, int op)
{
  if (++pfx->usedOprStack >= pfx->numOprStack) {
    if (!ExtendOperatorStack (pfx))
      return MagickFalse;
  }
  pfx->OperatorStack[pfx->usedOprStack-1] = (OperatorE) op;

  if (pfx->maxUsedOprStack < pfx->usedOprStack)
    pfx->maxUsedOprStack = pfx->usedOprStack;
  return MagickTrue;
}

static OperatorE GetLeadingOp (FxInfo * pfx)
{
  OperatorE op = oNull;

  if      (*pfx->pex == '-') op = oUnaryMinus;
  else if (*pfx->pex == '+') op = oUnaryPlus;
  else if (*pfx->pex == '~') op = oBitNot;
  else if (*pfx->pex == '!') op = oLogNot;
  else if (*pfx->pex == '(') op = oOpenParen;

  return op;
}

static inline MagickBooleanType OprIsUnaryPrefix (OperatorE op)
{
  return (op == oUnaryMinus || op == oUnaryPlus || op == oBitNot || op == oLogNot ? MagickTrue : MagickFalse);
}

static MagickBooleanType TopOprIsUnaryPrefix (FxInfo * pfx)
{
  if (!pfx->usedOprStack) return MagickFalse;

  return OprIsUnaryPrefix (pfx->OperatorStack[pfx->usedOprStack-1]);
}

static MagickBooleanType PopOprOpenParen (FxInfo * pfx, OperatorE op)
{

  if (!pfx->usedOprStack) return MagickFalse;

  if (pfx->OperatorStack[pfx->usedOprStack-1] != op) return MagickFalse;

  pfx->usedOprStack--;

  return MagickTrue;
}

static int GetCoordQualifier (FxInfo * pfx, int op)
/* Returns -1 if invalid CoordQualifier, +1 if valid and appropriate.
*/
{
  if (op != fU && op != fV && op != fS) return -1;

  (void) GetToken (pfx);

  if (pfx->lenToken != 1) {
    return -1;
  }
  if (*pfx->token != 'p' && *pfx->token != 'P') return -1;
  if (!GetFunction (pfx, fP)) return -1;

  return 1;
}

static PixelChannel GetChannelQualifier (FxInfo * pfx, int op)
{
  if (op == fU || op == fV || op == fP ||
      op == fUP || op == fVP ||
      op == fS || (op >= (int) FirstImgAttr && op <= aNull)
     )
  {
    const ChannelT * pch = &Channels[0];
    (void) GetToken (pfx);

    while (*pch->str) {
      if (LocaleCompare (pch->str, pfx->token)==0) {

        if (op >= (int) FirstImgAttr && op <= (int) ((OperatorE)aNull) &&
              ChanIsVirtual (pch->pixel_channel)
           )
        {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Can't have image attribute with channel qualifier at", "'%s' at '%s'",
            pfx->token, SetShortExp(pfx));
          return NO_CHAN_QUAL;
        }

        pfx->pex += pfx->lenToken;
        return pch->pixel_channel;
      }
      pch++;
    }
  }
  return NO_CHAN_QUAL;
}

static ImgAttrE GetImgAttrToken (FxInfo * pfx)
{
  ImgAttrE ia = aNull;
  const char * iaStr;
  for (ia = FirstImgAttr; ia < aNull; ia=(ImgAttrE) (ia+1)) {
    iaStr = ImgAttrs[ia-(int) FirstImgAttr].str;
    if (LocaleCompare (iaStr, pfx->token)==0) {
      pfx->pex += strlen(pfx->token);
      if (ImgAttrs[ia-(int) FirstImgAttr].need_stats != MagickFalse) pfx->NeedStats = MagickTrue;
      MaybeXYWH (pfx, &ia);
      break;
    }
  }

  if (ia == aPage || ia == aPrintsize || ia == aRes) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Attribute", "'%s' needs qualifier at '%s'",
      iaStr, SetShortExp(pfx));
  }

  return ia;
}

static ImgAttrE GetImgAttrQualifier (FxInfo * pfx, int op)
{
  ImgAttrE ia = aNull;
  if (op == (OperatorE)fU || op == (OperatorE)fV || op == (OperatorE)fP || op == (OperatorE)fS) {
    (void) GetToken (pfx);
    if (pfx->lenToken == 0) {
      return aNull;
    }
    ia = GetImgAttrToken (pfx);
  }
  return ia;
}

static MagickBooleanType IsQualifier (FxInfo * pfx)
{
  if (PeekChar (pfx) == '.') {
    pfx->pex++;
    return MagickTrue;
  }
  return MagickFalse;
}

static MagickBooleanType ParseISO860(const char* text,struct tm* tp)
{
  int
    year,
    month,
    day,
    hour,
    min,
    sec;

  memset(tp,0,sizeof(struct tm));
  if (MagickSscanf(text,"%d-%d-%dT%d:%d:%d",&year,&month,&day,&hour,&min,&sec) != 6)
    return(MagickFalse);
  tp->tm_year=year-1900;
  tp->tm_mon=month-1;
  tp->tm_mday=day;
  tp->tm_hour=hour;
  tp->tm_min=min;
  tp->tm_sec=sec;
  tp->tm_isdst=-1;
  return(MagickTrue);
}

static ssize_t GetProperty (FxInfo * pfx, fxFltType *val, fxFltType *seconds)
/* Returns number of characters to swallow.
   Returns "-1" means invalid input.
   Returns "0" means no relevant input (don't swallow, but not an error).
   If *seconds is not null, sets that from assumed date-time, or SECONDS_ERR if error.
*/
{
  if (seconds != NULL) *seconds = SECONDS_ERR;

  if (PeekStr (pfx, "%[")) {
    int level = 0;
    size_t len;
    char sProperty [MagickPathExtent];
    char * p = pfx->pex + 2;

    while (*p) {

      if (*p == '[') level++;
      else if (*p == ']') {
        if (level == 0) break;
        level--;
      }
      p++;
    }
    if (!*p || level != 0) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "After '%[' expected ']' at", "'%s'",
        SetShortExp(pfx));
      return -1;
    }

    len = (size_t) (p - pfx->pex + 1);
    if (len > MaxTokenLen) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Too much text between '%[' and ']' at", "'%s'",
        SetShortExp(pfx));
      return -1;
    }

    (void) CopyMagickString (sProperty, pfx->pex, len+1);
    sProperty[len] = '\0';
    {
      char * tailptr;
      char * text;
      text = InterpretImageProperties (pfx->image->image_info, pfx->image,
         sProperty, pfx->exception);
      if (!text || !*text) {
        text = DestroyString(text);
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Unknown property", "'%s' at '%s'",
          sProperty, SetShortExp(pfx));
        return -1;
      }

      if (seconds != NULL) {
        struct tm tp;
        if (ParseISO860(text,&tp) == MagickFalse) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Function 'epoch' expected date property, found ", "'%s' at '%s'",
            text, SetShortExp(pfx));
          text = DestroyString(text);
          *seconds = SECONDS_ERR;
          return -1;
        }
        *seconds = (fxFltType)mktime (&tp);
        *val = *seconds;
      } else {
        *val = strtold (text, &tailptr);
        if (text == tailptr) {
          text = DestroyString(text);
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Property", "'%s' text '%s' is not a number at '%s'",
            sProperty, text, SetShortExp(pfx));
          text = DestroyString(text);
          return -1;
        }
      }
      text = DestroyString(text);
    }
    return ((ssize_t) len);
  }

  return 0;
}

static inline ssize_t GetConstantColour (FxInfo * pfx, fxFltType *v0, fxFltType *v1, fxFltType *v2)
/* Finds named colour such as "blue" and colorspace function such as "lab(10,20,30)".
   Returns number of characters to swallow.
   Return -1 means apparently a constant colour, but with an error.
   Return 0 means not a constant colour, but not an error.
*/
{
  PixelInfo
    colour;

  ExceptionInfo
    *dummy_exception = AcquireExceptionInfo ();

  char
    *p;

  MagickBooleanType
    IsGray,
    IsIcc,
    IsDev;

  char ColSp[MagickPathExtent];
  (void) CopyMagickString (ColSp, pfx->token, MaxTokenLen);
  p = ColSp + pfx->lenToken - 1;
  if (*p == 'a' || *p == 'A') *p = '\0';

  (void) GetPixelInfo (pfx->image, &colour);

  /* "gray" is both a colorspace and a named colour. */

  IsGray = (LocaleCompare (ColSp, "gray") == 0) ? MagickTrue : MagickFalse;
  IsIcc = (LocaleCompare (ColSp, "icc-color") == 0) ? MagickTrue : MagickFalse;
  IsDev = (LocaleNCompare (ColSp, "device-", 7) == 0) ? MagickTrue : MagickFalse;

  /* QueryColorCompliance will raise a warning if it isn't a colour, so we discard any exceptions.
  */
  if (!QueryColorCompliance (pfx->token, AllCompliance, &colour, dummy_exception) || IsGray) {
    ssize_t type = ParseCommandOption (MagickColorspaceOptions, MagickFalse, ColSp);
    if (type >= 0 || IsIcc || IsDev) {
      char * q = pfx->pex + pfx->lenToken;
      while (isspace((int) ((unsigned char) *q))) q++;
      if (*q == '(') {
        size_t lenfun;
        char sFunc[MagickPathExtent];
        while (*q && *q != ')') q++;
        if (!*q) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "constant color missing ')'", "at '%s'",
            SetShortExp(pfx));
          dummy_exception = DestroyExceptionInfo (dummy_exception);
          return -1;
        }
        lenfun = (size_t) (q - pfx->pex + 1);
        if (lenfun > MaxTokenLen) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "lenfun too long", "'%lu' at '%s'",
            (unsigned long) lenfun, SetShortExp(pfx));
          dummy_exception = DestroyExceptionInfo (dummy_exception);
          return -1;
        }
        (void) CopyMagickString (sFunc, pfx->pex, lenfun+1);
        if (QueryColorCompliance (sFunc, AllCompliance, &colour, dummy_exception)) {
          *v0 = QuantumScale*colour.red;
          *v1 = QuantumScale*colour.green;
          *v2 = QuantumScale*colour.blue;
          dummy_exception = DestroyExceptionInfo (dummy_exception);
          return (ssize_t)lenfun;
        }
      } else {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "colorspace but not a valid color with '(...)' at", "'%s'",
          SetShortExp(pfx));
        dummy_exception = DestroyExceptionInfo (dummy_exception);
        return -1;
      }
    }
    if (!IsGray) {
      dummy_exception = DestroyExceptionInfo (dummy_exception);
      return 0;
    }
  }

  *v0 = QuantumScale*colour.red;
  *v1 = QuantumScale*colour.green;
  *v2 = QuantumScale*colour.blue;

  dummy_exception = DestroyExceptionInfo (dummy_exception);
  return (ssize_t)strlen (pfx->token);
}

static inline ssize_t GetHexColour (FxInfo * pfx, fxFltType *v0, fxFltType *v1, fxFltType *v2)
/* Returns number of characters to swallow.
   Negative return means it starts with '#', but invalid hex number.
*/
{
  char * p;
  size_t len;
  PixelInfo colour;

  if (*pfx->pex != '#') return 0;

  /* find end of hex digits. */
  p = pfx->pex + 1;
  while (isxdigit ((int)*p)) p++;
  if (isalpha ((int)*p)) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Bad hex number at", "'%s'",
      SetShortExp(pfx));
    return -1;
  }

  len = (size_t) (p - pfx->pex);
  if (len < 1) return 0;
  if (len >= MaxTokenLen) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Hex colour too long at", "'%s'",
      SetShortExp(pfx));
    return -1;
  }
  (void) CopyMagickString (pfx->token, pfx->pex, len+1);

  (void) GetPixelInfo (pfx->image, &colour);

  if (!QueryColorCompliance (pfx->token, AllCompliance, &colour, pfx->exception)) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "QueryColorCompliance rejected", "'%s' at '%s'",
      pfx->token, SetShortExp(pfx));
    return -1;
  }

  *v0 = QuantumScale*colour.red;
  *v1 = QuantumScale*colour.green;
  *v2 = QuantumScale*colour.blue;

  return (ssize_t) len;
}

static MagickBooleanType GetFunction (FxInfo * pfx, FunctionE fe)
{
  /* A function, so get open-parens, n args, close-parens
  */
  const char * funStr = Functions[fe-(int) FirstFunc].str;
  int nArgs = Functions[fe-(int) FirstFunc].number_args;
  char chLimit = ')';
  char expChLimit = ')';
  const char *strLimit = ",)";
  OperatorE pushOp = oOpenParen;

  char * pExpStart;

  size_t lenExp = 0;

  int FndArgs = 0;
  int ndx0 = NULL_ADDRESS, ndx1 = NULL_ADDRESS, ndx2 = NULL_ADDRESS, ndx3 = NULL_ADDRESS;

  MagickBooleanType coordQual = MagickFalse;
  PixelChannel chQual = NO_CHAN_QUAL;
  ImgAttrE iaQual = aNull;

  pfx->pex += pfx->lenToken;

  if (fe == fP) {
    char p = PeekChar (pfx);
    if (p=='{') {
      (void) ExpectChar (pfx, '{');
      pushOp = oOpenBrace;
      strLimit = ",}";
      chLimit = '}';
      expChLimit = '}';
    } else if (p=='[') {
      (void) ExpectChar (pfx, '[');
      pushOp = oOpenBracket;
      strLimit = ",]";
      chLimit = ']';
      expChLimit = ']';
    } else {
      nArgs = 0;
      chLimit = ']';
      expChLimit = ']';
    }
  } else if (fe == fU) {
    char p = PeekChar (pfx);
    if (p=='[') {
      (void) ExpectChar (pfx, '[');
      pushOp = oOpenBracket;
      strLimit = ",]";
      chLimit = ']';
      expChLimit = ']';
    } else {
      nArgs = 0;
      chLimit = ']';
      expChLimit = ']';
    }
  } else if (fe == fV || fe == fS) {
      nArgs = 0;
      pushOp = oOpenBracket;
      chLimit = ']';
      expChLimit = ']';
  } else {
    if (!ExpectChar (pfx, '(')) return MagickFalse;
  }
  if (!PushOperatorStack (pfx, (int) pushOp)) return MagickFalse;

  pExpStart = pfx->pex;
  ndx0 = pfx->usedElements;
  if (fe==fDo) {
    (void) AddAddressingElement (pfx, rGoto, NULL_ADDRESS); /* address will be ndx1+1 */
  }
  if (fe==fEpoch) {
    fxFltType
      val,
      seconds;
    ssize_t
      lenOptArt = GetProperty (pfx, &val, &seconds);
    if (seconds == SECONDS_ERR) {
      /* Exception may not have been raised. */
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Function 'epoch' expected date property", "at '%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    if (lenOptArt < 0) return MagickFalse;
    if (lenOptArt > 0) {
      (void) AddElement (pfx, seconds, oNull);
      pfx->pex += lenOptArt;
      if (!ExpectChar (pfx, ')')) return MagickFalse;
      if (!PopOprOpenParen (pfx, pushOp)) return MagickFalse;
      return MagickTrue;
    }
  }

  while (nArgs > 0) {
    int FndOne = 0;
    if (TranslateStatementList (pfx, strLimit, &chLimit)) {
      FndOne = 1;
    } else {
      if (!*pfx->pex) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "For function", "'%s' expected ')' at '%s'",
          funStr, SetShortExp(pfx));
        return MagickFalse;
      }
      /* Maybe don't break because other expressions may be not empty. */
      if (!chLimit) break;
      if (fe == fP || fe == fS|| fe == fIf) {
        (void) AddElement (pfx, (fxFltType) 0, oNull);
        FndOne = 1;
      }
    }

    if (strchr (strLimit, chLimit)==NULL) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s' expected one of '%s' after expression but found '%c' at '%s'",
        funStr, strLimit, chLimit ? chLimit : ' ', SetShortExp(pfx));
      return MagickFalse;
    }
    if (FndOne) {
      FndArgs++;
      nArgs--;
    }
    switch (FndArgs) {
      case 1:
        if (ndx1 != NULL_ADDRESS) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "For function", "'%s' required argument is missing at '%s'",
            funStr, SetShortExp(pfx));
          return MagickFalse;
        }
        ndx1 = pfx->usedElements;
        if (fe==fWhile || fe==fIf) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fDo) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].do_push = MagickFalse;
        }
        break;
      case 2:
        if (ndx2 != NULL_ADDRESS) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "For function", "'%s' required argument is missing at '%s'",
            funStr, SetShortExp(pfx));
          return MagickFalse;
        }
        ndx2 = pfx->usedElements;
        if (fe==fWhile) {
          pfx->Elements[pfx->usedElements-1].do_push = MagickFalse;
          (void) AddAddressingElement (pfx, rGotoChk, ndx0);
        } else if (fe==fDo) {
          pfx->Elements[pfx->usedElements-1].do_push = MagickFalse;
          (void) AddAddressingElement (pfx, rGotoChk, ndx0 + 1);
        } else if (fe==fFor) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx3 */
          pfx->Elements[pfx->usedElements-1].do_push = MagickTrue; /* we may need return from for() */
          (void) AddAddressingElement (pfx, rZerStk, NULL_ADDRESS);
        } else if (fe==fIf) {
          (void) AddAddressingElement (pfx, rGoto, NULL_ADDRESS); /* address will be ndx3 */
        }
        break;
      case 3:
        if (ndx3 != NULL_ADDRESS) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "For function", "'%s' required argument is missing at '%s'",
            funStr, SetShortExp(pfx));
          return MagickFalse;
        }
        if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].do_push = MagickFalse;
          (void) AddAddressingElement (pfx, rGotoChk, ndx1);
        }
        ndx3 = pfx->usedElements;
        break;
      default:
        break;
    }
    if (chLimit == expChLimit) {
      lenExp = (size_t) (pfx->pex - pExpStart - 1);
      break;
    }
  } /* end while args of a function */
  if (chLimit && chLimit != expChLimit && chLimit != ',' ) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected '%c', found '%c' at '%s'",
      funStr, expChLimit, chLimit ? chLimit : ' ', SetShortExp(pfx));
    return MagickFalse;
  }

  if (fe == fP || fe == fS || fe == fU || fe == fChannel) {
    while (FndArgs < Functions[fe-(int) FirstFunc].number_args) {
      (void) AddElement (pfx, (fxFltType) 0, oNull);
      FndArgs++;
    }
  }

  if (FndArgs > Functions[fe-(int) FirstFunc].number_args)
  {
    if (fe==fChannel) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s' expected up to %i arguments, found '%i' at '%s'",
        funStr, Functions[fe-(int) FirstFunc].number_args, FndArgs, SetShortExp(pfx));
    } else {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s' expected %i arguments, found '%i' at '%s'",
        funStr, Functions[fe-(int) FirstFunc].number_args, FndArgs, SetShortExp(pfx));
    }
    return MagickFalse;
  }
  if (FndArgs < Functions[fe-(int) FirstFunc].number_args) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected %i arguments, found too few (%i) at '%s'",
      funStr, Functions[fe-(int) FirstFunc].number_args, FndArgs, SetShortExp(pfx));
    return MagickFalse;
  }
  if (fe != fS && fe != fV && FndArgs == 0 && Functions[fe-(int) FirstFunc].number_args == 0) {
    /* This is for "rand()" and similar. */
    chLimit = expChLimit;
    if (!ExpectChar (pfx, ')')) return MagickFalse;
  }

  if (chLimit != expChLimit) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "For function", "'%s', arguments don't end with '%c' at '%s'",
      funStr, expChLimit, SetShortExp(pfx));
    return MagickFalse;
  }
  if (!PopOprOpenParen (pfx, pushOp)) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Bug: For function", "'%s' tos not '%s' at '%s'",
      funStr, Operators[pushOp].str, SetShortExp(pfx));
    return MagickFalse;
  }

  if (IsQualifier (pfx)) {

    if (fe == fU || fe == fV || fe == fS) {

      coordQual = (GetCoordQualifier (pfx, (int) fe) == 1) ? MagickTrue : MagickFalse;

      if (coordQual) {

        /* Remove last element, which should be fP */
        ElementT * pel = &pfx->Elements[pfx->usedElements-1];
        if (pel->operator_index != fP) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Bug: For function", "'%s' last element not 'p' at '%s'",
            funStr, SetShortExp(pfx));
          return MagickFalse;
        }
        chQual = pel->channel_qual;
        expChLimit = (pel->is_relative) ? ']' : '}';
        pfx->usedElements--;
        if (fe == fU) fe = fUP;
        else if (fe == fV) fe = fVP;
        else if (fe == fS) fe = fSP;
        funStr = Functions[fe-(int) FirstFunc].str;
      }
    }

    if ( chQual == NO_CHAN_QUAL &&
         (fe == fP || fe == fS || fe == fSP || fe == fU || fe == fUP || fe == fV || fe == fVP)
       )
    {
      chQual = GetChannelQualifier (pfx, (int) fe);
    }

    if (chQual == NO_CHAN_QUAL && (fe == fU || fe == fV || fe == fS)) {
      /* Note: we don't allow "p.mean" etc. */
      iaQual = GetImgAttrQualifier (pfx, (int) fe);
    }
    if (IsQualifier (pfx) && chQual == NO_CHAN_QUAL && iaQual != aNull) {
      chQual = GetChannelQualifier (pfx, (int) fe);
    }
    if (coordQual && iaQual != aNull) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s', can't have qualifiers 'p' and image attribute '%s' at '%s'",
        funStr, pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!coordQual && chQual == NO_CHAN_QUAL && iaQual == aNull) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s', bad qualifier '%s' at '%s'",
        funStr, pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!coordQual && chQual == CompositePixelChannel && iaQual == aNull) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s', bad composite qualifier '%s' at '%s'",
        funStr, pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }

    if (chQual == HUE_CHANNEL || chQual == SAT_CHANNEL || chQual == LIGHT_CHANNEL) {
      pfx->NeedHsl = MagickTrue;

      if (iaQual >= FirstImgAttr && iaQual < aNull) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Can't have image attribute with HLS qualifier at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
    }
  }

  if (iaQual != aNull && chQual != NO_CHAN_QUAL) {
    if (ImgAttrs[iaQual-(int) FirstImgAttr].need_stats == MagickFalse) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Can't have image attribute ", "'%s' with channel qualifier '%s' at '%s'",
        ImgAttrs[iaQual-(int) FirstImgAttr].str,
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    } else {
      if (ChanIsVirtual (chQual)) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Can't have statistical image attribute ", "'%s' with virtual channel qualifier '%s' at '%s'",
          ImgAttrs[iaQual-(int) FirstImgAttr].str,
          pfx->token, SetShortExp(pfx));
        return MagickFalse;
      }
    }
  }

  if (fe==fWhile) {
    pfx->Elements[ndx1].element_index = ndx2+1;
  } else if (fe==fDo) {
    pfx->Elements[ndx0].element_index = ndx1+1;
    pfx->Elements[ndx1].element_index = ndx2+1;
  } else if (fe==fFor) {
    pfx->Elements[ndx2].element_index = ndx3;
  } else if (fe==fIf) {
    pfx->Elements[ndx1].element_index = ndx2 + 1;
    pfx->Elements[ndx2].element_index = ndx3;
  } else {
    if (fe == fU && iaQual == aNull) {
      ElementT * pel = &pfx->Elements[pfx->usedElements-1];
      if (pel->type == etConstant && pel->val == 0.0) {
        pfx->usedElements--;
        fe = fU0;
      }
    }
    (void) AddElement (pfx, (fxFltType) 0, (int) fe);
    if (fe == fP || fe == fU  || fe == fU0 || fe == fUP ||
        fe == fV || fe == fVP || fe == fS || fe == fSP)
    {
      ElementT * pel = &pfx->Elements[pfx->usedElements-1];
      pel->is_relative = (expChLimit == ']' ? MagickTrue : MagickFalse);
      if (chQual >= 0) pel->channel_qual = chQual;
      if (iaQual != aNull && (fe == fU || fe == fV || fe == fS)) {
        /* Note: we don't allow "p[2,3].mean" or "p.mean" etc. */
        pel->img_attr_qual = iaQual;
      }
    }
  }

  if (pExpStart && lenExp) {
    ElementT * pel = &pfx->Elements[pfx->usedElements-1];
    pel->exp_start = pExpStart;
    pel->exp_len = lenExp;
  }

  if (fe == fDebug)
    pfx->ContainsDebug = MagickTrue;

  return MagickTrue;
}

static MagickBooleanType IsStealth (int op)
{
  return (op == fU0 || op == fUP || op == fSP || op == fVP ||
           (op >= FirstCont && op <= rNull) ? MagickTrue : MagickFalse
         );
}

static MagickBooleanType GetOperand (
  FxInfo * pfx, MagickBooleanType * UserSymbol, MagickBooleanType * NewUserSymbol, int * UserSymNdx,
  MagickBooleanType * needPopAll)
{

  *NewUserSymbol = *UserSymbol = MagickFalse;
  *UserSymNdx = NULL_ADDRESS;

  SkipSpaces (pfx);
  if (!*pfx->pex) return MagickFalse;
  (void) GetToken (pfx);

  if (pfx->lenToken==0) {

    /* Try '(' or unary prefix
    */
    OperatorE op = GetLeadingOp (pfx);
    if (op==oOpenParen) {
      char chLimit = '\0';
      if (!PushOperatorStack (pfx, (int) op)) return MagickFalse;
      pfx->pex++;
      if (!TranslateExpression (pfx, ")", &chLimit, needPopAll)) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Empty expression in parentheses at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
      if (chLimit != ')') {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "'(' but no ')' at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
      /* Top of opr stack should be '('. */
      if (!PopOprOpenParen (pfx, oOpenParen)) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Bug: tos not '(' at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
      return MagickTrue;
    } else if (OprIsUnaryPrefix (op)) {
      if (!PushOperatorStack (pfx, (int) op)) return MagickFalse;
      pfx->pex++;
      SkipSpaces (pfx);
      if (!*pfx->pex) return MagickFalse;

      if (!GetOperand (pfx, UserSymbol, NewUserSymbol, UserSymNdx, needPopAll)) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "After unary, bad operand at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }

      if (*NewUserSymbol) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "After unary, NewUserSymbol at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }

      if (*UserSymbol) {
        (void) AddAddressingElement (pfx, rCopyFrom, *UserSymNdx);
        *UserSymNdx = NULL_ADDRESS;

        *UserSymbol = MagickFalse;
        *NewUserSymbol = MagickFalse;
      }

      (void) GetToken (pfx);
      return MagickTrue;
    } else if (*pfx->pex == '#') {
      fxFltType v0=0, v1=0, v2=0;
      ssize_t lenToken = GetHexColour (pfx, &v0, &v1, &v2);
      if (lenToken < 0) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Bad hex number at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      } else if (lenToken > 0) {
        (void) AddColourElement (pfx, v0, v1, v2);
        pfx->pex+=lenToken;
      }
      return MagickTrue;
    }

    /* Try a constant number.
    */
    {
      char * tailptr;
      ssize_t lenOptArt;
      fxFltType val = strtold (pfx->pex, &tailptr);
      if (pfx->pex != tailptr) {
        pfx->pex = tailptr;
        if (*tailptr) {
          /* Could have "prefix" K, Ki, M etc.
             See https://en.wikipedia.org/wiki/Metric_prefix
             and https://en.wikipedia.org/wiki/Binary_prefix
          */
          double Pow = 0.0;
          const char Prefixes[] = "yzafpnum.kMGTPEZY";
          const char * pSi = strchr (Prefixes, *tailptr);
          if (pSi && *pSi != '.') Pow = (double) ((pSi - Prefixes) * 3 - 24);
          else if (*tailptr == 'c') Pow = -2;
          else if (*tailptr == 'h') Pow =  2;
          else if (*tailptr == 'k') Pow =  3;
          if (Pow != 0.0) {
            if (*(++pfx->pex) == 'i') {
              val *= pow (2.0, Pow/0.3);
              pfx->pex++;
            } else {
              val *= pow (10.0, Pow);
            }
          }
        }
        (void) AddElement (pfx, val, oNull);
        return MagickTrue;
      }

      val = (fxFltType) 0;
      lenOptArt = GetProperty (pfx, &val, NULL);
      if (lenOptArt < 0) return MagickFalse;
      if (lenOptArt > 0) {
        (void) AddElement (pfx, val, oNull);
        pfx->pex += lenOptArt;
        return MagickTrue;
      }
    }

  } /* end of lenToken==0 */

  if (pfx->lenToken > 0) {
    /* Try a constant
    */
    {
      ConstantE ce;
      for (ce = (ConstantE)0; ce < cNull; ce=(ConstantE) (ce+1)) {
        const char * ceStr = Constants[ce].str;
        if (LocaleCompare (ceStr, pfx->token)==0) {
          break;
        }
      }

      if (ce != cNull) {
        (void) AddElement (pfx, Constants[ce].val, oNull);
        pfx->pex += pfx->lenToken;
        return MagickTrue;
      }
    }

    /* Try a function
    */
    {
      FunctionE fe;
      for (fe = FirstFunc; fe < fNull; fe=(FunctionE) (fe+1)) {
        const char * feStr = Functions[fe-(int) FirstFunc].str;
        if (LocaleCompare (feStr, pfx->token)==0) {
          break;
        }
      }

      if (fe == fV && pfx->ImgListLen < 2) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Symbol 'v' but fewer than two images at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }

      if (IsStealth ((int) fe)) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Function", "'%s' not permitted at '%s'",
          pfx->token, SetShortExp(pfx));
      }

      if (fe == fDo || fe == fFor || fe == fIf || fe == fWhile) {
        *needPopAll = MagickTrue;
      }

      if (fe != fNull) return (GetFunction (pfx, fe));
    }

    /* Try image attribute
    */
    {
      ImgAttrE ia = GetImgAttrToken (pfx);
      if (ia != aNull) {
        fxFltType val = 0;
        (void) AddElement (pfx, val, (int) ia);

        if (ImgAttrs[ia-(int) FirstImgAttr].need_stats != MagickFalse) {
          if (IsQualifier (pfx)) {
            PixelChannel chQual = GetChannelQualifier (pfx, (int) ia);
            ElementT * pel;
            if (chQual == NO_CHAN_QUAL) {
              (void) ThrowMagickException (
                pfx->exception, GetMagickModule(), OptionError,
                "Bad channel qualifier at", "'%s'",
                SetShortExp(pfx));
              return MagickFalse;
            }
            /* Adjust the element */
            pel = &pfx->Elements[pfx->usedElements-1];
            pel->channel_qual = chQual;
          }
        }
        return MagickTrue;
      }
    }

    /* Try symbol
    */
    {
      SymbolE se;
      for (se = FirstSym; se < sNull; se=(SymbolE) (se+1)) {
        const char * seStr = Symbols[se-(int) FirstSym].str;
        if (LocaleCompare (seStr, pfx->token)==0) {
          break;
        }
      }
      if (se != sNull) {
        fxFltType val = 0;
        (void) AddElement (pfx, val, (int) se);
        pfx->pex += pfx->lenToken;

        if (se==sHue || se==sSaturation || se==sLightness) pfx->NeedHsl = MagickTrue;
        return MagickTrue;
      }
    }

    /* Try constant colour.
    */
    {
      fxFltType v0, v1, v2;
      ssize_t ColLen = GetConstantColour (pfx, &v0, &v1, &v2);
      if (ColLen < 0) return MagickFalse;
      if (ColLen > 0) {
        (void) AddColourElement (pfx, v0, v1, v2);
        pfx->pex+=ColLen;
        return MagickTrue;
      }
    }

    /* Try image artifact.
    */
    {
      const char *artifact;
      artifact = GetImageArtifact (pfx->image, pfx->token);
      if (artifact != (const char *) NULL) {
        char * tailptr;
        fxFltType val = strtold (artifact, &tailptr);
        if (pfx->token == tailptr) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Artifact", "'%s' has value '%s', not a number, at '%s'",
            pfx->token, artifact, SetShortExp(pfx));
          return MagickFalse;
        }
        (void) AddElement (pfx, val, oNull);
        pfx->pex+=pfx->lenToken;
        return MagickTrue;
      }
    }

    /* Try user symbols. If it is, don't AddElement yet.
    */
    if (TokenMaybeUserSymbol (pfx)) {
      *UserSymbol = MagickTrue;
      *UserSymNdx = FindUserSymbol (pfx, pfx->token);
      if (*UserSymNdx == NULL_ADDRESS) {
        *UserSymNdx = AddUserSymbol (pfx, pfx->pex, pfx->lenToken);
        *NewUserSymbol = MagickTrue;
      } else {
      }
      pfx->pex += pfx->lenToken;

      return MagickTrue;
    }
  }

  (void) ThrowMagickException (
    pfx->exception, GetMagickModule(), OptionError,
    "Expected operand at", "'%s'",
    SetShortExp(pfx));

  return MagickFalse;
}

static inline MagickBooleanType IsRealOperator (OperatorE op)
{
  return (op < oOpenParen || op > oCloseBrace) ? MagickTrue : MagickFalse;
}

static inline MagickBooleanType ProcessTernaryOpr (FxInfo * pfx, TernaryT * ptern)
/* Ternary operator "... ? ... : ..."
   returns false iff we have exception
*/
{
  if (pfx->usedOprStack == 0)
    return MagickFalse;
  if (pfx->OperatorStack[pfx->usedOprStack-1] == oQuery) {
    if (ptern->addr_query != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have '?' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    if (ptern->addr_colon != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    ptern->addr_query = pfx->usedElements;
    (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS);
    /* address will be one after the Colon address. */
  }
  else if (pfx->OperatorStack[pfx->usedOprStack-1] == oColon) {
    if (ptern->addr_query == NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Need '?' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    if (ptern->addr_colon != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    ptern->addr_colon = pfx->usedElements;
    pfx->Elements[pfx->usedElements-1].do_push = MagickTrue;
    (void) AddAddressingElement (pfx, rGoto, NULL_ADDRESS);
    /* address will be after the subexpression */
  }
  return MagickTrue;
}

static MagickBooleanType GetOperator (
  FxInfo * pfx,
  MagickBooleanType * Assign, MagickBooleanType * Update, MagickBooleanType * IncrDecr)
{
  OperatorE op;
  size_t len = 0;
  MagickBooleanType DoneIt = MagickFalse;
  SkipSpaces (pfx);
  for (op = (OperatorE)0; op != oNull; op=(OperatorE) (op+1)) {
    const char * opStr = Operators[op].str;
    len = strlen(opStr);
    if (LocaleNCompare (opStr, pfx->pex, len)==0) {
      break;
    }
  }

  if (!IsRealOperator (op)) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Not a real operator at", "'%s'",
      SetShortExp(pfx));
    return MagickFalse;
  }

  if (op==oNull) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Expected operator at", "'%s'",
      SetShortExp(pfx));
    return MagickFalse;
  }

  *Assign = (op==oAssign) ? MagickTrue : MagickFalse;
  *Update = OprInPlace ((int) op);
  *IncrDecr = (op == oPlusPlus || op == oSubSub) ? MagickTrue : MagickFalse;

  /* while top of OperatorStack is not empty and is not open-parens or assign,
       and top of OperatorStack is higher precedence than new op,
     then move top of OperatorStack to Element list.
  */

  while (pfx->usedOprStack > 0) {
    OperatorE top = pfx->OperatorStack[pfx->usedOprStack-1];
    int precTop, precNew;
    if (top == oOpenParen || top == oAssign || OprInPlace ((int) top)) break;
    precTop = Operators[top].precedence;
    precNew = Operators[op].precedence;
    /* Assume left associativity.
       If right assoc, this would be "<=".
    */
    if (precTop < precNew) break;
    (void) AddElement (pfx, (fxFltType) 0, (int) top);
    pfx->usedOprStack--;
  }

  /* If new op is close paren, and stack top is open paren,
     remove stack top.
  */
  if (op==oCloseParen) {
    if (pfx->usedOprStack == 0) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Found ')' but nothing on stack at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }

    if (pfx->OperatorStack[pfx->usedOprStack-1] != oOpenParen) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Found ')' but no '(' on stack at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    DoneIt = MagickTrue;
  }

  if (!DoneIt) {
    if (!PushOperatorStack (pfx, (int) op)) return MagickFalse;
  }

  pfx->pex += len;

  return MagickTrue;
}

static MagickBooleanType ResolveTernaryAddresses (FxInfo * pfx, TernaryT * ptern)
{
  if (ptern->addr_query == NULL_ADDRESS && ptern->addr_colon == NULL_ADDRESS)
    return MagickTrue;

  if (ptern->addr_query != NULL_ADDRESS && ptern->addr_colon != NULL_ADDRESS) {
    pfx->Elements[ptern->addr_query].element_index = ptern->addr_colon + 1;
    pfx->Elements[ptern->addr_colon].element_index = pfx->usedElements;
    ptern->addr_query = NULL_ADDRESS;
    ptern->addr_colon = NULL_ADDRESS;
  } else if (ptern->addr_query != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "'?' with no corresponding ':'", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
  } else if (ptern->addr_colon != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "':' with no corresponding '?'", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType TranslateExpression (
  FxInfo * pfx, const char * strLimit, char * chLimit, MagickBooleanType * needPopAll)
{
  /* There should be only one New per expression (oAssign), but can be many Old.
  */
  MagickBooleanType UserSymbol, NewUserSymbol;
  int UserSymNdx0, UserSymNdx1;

  MagickBooleanType
    Assign = MagickFalse,
    Update = MagickFalse,
    IncrDecr = MagickFalse;

  int StartEleNdx;

  TernaryT ternary;
  ternary.addr_query = NULL_ADDRESS;
  ternary.addr_colon = NULL_ADDRESS;

  pfx->teDepth++;

  *chLimit = '\0';

  StartEleNdx = pfx->usedElements-1;
  if (StartEleNdx < 0) StartEleNdx = 0;

  SkipSpaces (pfx);

  if (!*pfx->pex) {
    pfx->teDepth--;
    return MagickFalse;
  }

  if (strchr(strLimit,*pfx->pex)!=NULL) {
    *chLimit = *pfx->pex;
    pfx->pex++;
    pfx->teDepth--;

    return MagickFalse;
  }

  if (!GetOperand (pfx, &UserSymbol, &NewUserSymbol, &UserSymNdx0, needPopAll)) return MagickFalse;
  SkipSpaces (pfx);

  /* Loop through Operator, Operand, Operator, Operand, ...
  */
  while (*pfx->pex && (!*strLimit || (strchr(strLimit,*pfx->pex)==NULL))) {
    if (!GetOperator (pfx, &Assign, &Update, &IncrDecr)) return MagickFalse;
    SkipSpaces (pfx);
    if (NewUserSymbol && !Assign) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Expected assignment after new UserSymbol", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!UserSymbol && Assign) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Attempted assignment to non-UserSymbol", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!UserSymbol && Update) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Attempted update to non-UserSymbol", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (UserSymbol && (Assign || Update) && !IncrDecr) {

      if (!TranslateExpression (pfx, strLimit, chLimit, needPopAll)) return MagickFalse;
      if (!*pfx->pex) break;
      if (!*strLimit) break;
      if (strchr(strLimit,*chLimit)!=NULL) break;
    }
    if (UserSymbol && !Assign && !Update && UserSymNdx0 != NULL_ADDRESS) {
      ElementT * pel;
      (void) AddAddressingElement (pfx, rCopyFrom, UserSymNdx0);
      UserSymNdx0 = NULL_ADDRESS;
      pel = &pfx->Elements[pfx->usedElements-1];
      pel->do_push = MagickTrue;
    }

    if (UserSymbol) {
      while (TopOprIsUnaryPrefix (pfx)) {
        OperatorE op = pfx->OperatorStack[pfx->usedOprStack-1];
        (void) AddElement (pfx, (fxFltType) 0, (int) op);
        pfx->usedOprStack--;
      }
    }

    if (!ProcessTernaryOpr (pfx, &ternary)) return MagickFalse;

    if (ternary.addr_colon != NULL_ADDRESS) {
      if (!TranslateExpression (pfx, ",);", chLimit, needPopAll)) return MagickFalse;
      break;
    }

    UserSymbol = NewUserSymbol = MagickFalse;

    if ( (!*pfx->pex) || (*strLimit && (strchr(strLimit,*pfx->pex)!=NULL) ) )
    {
      if (IncrDecr) break;

      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Expected operand after operator", "at '%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }

    if (IncrDecr) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "'++' and '--' must be the final operators in an expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }

    if (!GetOperand (pfx, &UserSymbol, &NewUserSymbol, &UserSymNdx1, needPopAll)) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Expected operand at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    SkipSpaces (pfx);
    if (NewUserSymbol && !Assign) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "NewUserSymbol", "'%s' after non-assignment operator at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (UserSymbol && !NewUserSymbol) {
      (void) AddAddressingElement (pfx, rCopyFrom, UserSymNdx1);
      UserSymNdx1 = NULL_ADDRESS;
    }
    UserSymNdx0 = UserSymNdx1;
  }

  if (UserSymbol && !Assign && !Update && UserSymNdx0 != NULL_ADDRESS) {
    ElementT * pel;
    if (NewUserSymbol) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "NewUserSymbol", "'%s' needs assignment operator at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    (void) AddAddressingElement (pfx, rCopyFrom, UserSymNdx0);
    pel = &pfx->Elements[pfx->usedElements-1];
    pel->do_push = MagickTrue;
  }

  if (*pfx->pex && !*chLimit && (strchr(strLimit,*pfx->pex)!=NULL)) {
    *chLimit = *pfx->pex;
    pfx->pex++;
  }
  while (pfx->usedOprStack) {
    OperatorE op = pfx->OperatorStack[pfx->usedOprStack-1];
    if (op == oOpenParen || op == oOpenBracket || op == oOpenBrace) {
      break;
    }
    if ( (op==oAssign && !Assign) || (OprInPlace((int) op) && !Update) ) {
      break;
    }
    pfx->usedOprStack--;
    (void) AddElement (pfx, (fxFltType) 0, (int) op);
    if (op == oAssign) {
      if (UserSymNdx0 < 0) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Assignment to unknown user symbol at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
      /* Adjust last element, by deletion and add.
      */
      pfx->usedElements--;
      (void) AddAddressingElement (pfx, rCopyTo, UserSymNdx0);
      break;
    } else if (OprInPlace ((int) op)) {
      if (UserSymNdx0 < 0) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Operator-in-place to unknown user symbol at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
      /* Modify latest element.
      */
      pfx->Elements[pfx->usedElements-1].element_index = UserSymNdx0;
      break;
    }
  }

  if (ternary.addr_query != NULL_ADDRESS) *needPopAll = MagickTrue;

  (void) ResolveTernaryAddresses (pfx, &ternary);

  pfx->teDepth--;

  if (!pfx->teDepth && *needPopAll) {
    (void) AddAddressingElement (pfx, rZerStk, NULL_ADDRESS);
    *needPopAll = MagickFalse;
  }

  if (pfx->exception->severity >= ErrorException)
    return MagickFalse;

  return MagickTrue;
}


static MagickBooleanType TranslateStatement (FxInfo * pfx, char * strLimit, char * chLimit)
{
  MagickBooleanType NeedPopAll = MagickFalse;

  SkipSpaces (pfx);

  if (!*pfx->pex) return MagickFalse;

  if (!TranslateExpression (pfx, strLimit, chLimit, &NeedPopAll)) {
    return MagickFalse;
  }
  if (pfx->usedElements && *chLimit==';') {
    /* FIXME: not necessarily the last element,
       but the last _executed_ element, eg "goto" in a "for()".,
       Pending a fix, we will use rZerStk.
    */
    ElementT * pel = &pfx->Elements[pfx->usedElements-1];
    if (pel->do_push) pel->do_push = MagickFalse;
  }

  return MagickTrue;
}

static MagickBooleanType TranslateStatementList (FxInfo * pfx, const char * strLimit, char * chLimit)
{
#define MAX_SLIMIT 10
  char sLimits[MAX_SLIMIT];
  SkipSpaces (pfx);

  if (!*pfx->pex) return MagickFalse;
  (void) CopyMagickString (sLimits, strLimit, MAX_SLIMIT-1);

  if (strchr(strLimit,';')==NULL)
    (void) ConcatenateMagickString (sLimits, ";", MAX_SLIMIT);

  for (;;) {
    if (!TranslateStatement (pfx, sLimits, chLimit)) return MagickFalse;

    if (!*pfx->pex) break;

    if (*chLimit != ';') {
      break;
    }
  }

  if (pfx->exception->severity >= ErrorException)
    return MagickFalse;

  return MagickTrue;
}

/*--------------------------------------------------------------------
   Run-time
*/

static ChannelStatistics *CollectOneImgStats (FxInfo * pfx, Image * img)
{
  int ch;
  ChannelStatistics * cs = GetImageStatistics (img, pfx->exception);
  /* Use RelinquishMagickMemory() somewhere. */

  if (cs == (ChannelStatistics *) NULL)
    return((ChannelStatistics *) NULL);

  for (ch=0; ch <= (int) MaxPixelChannels; ch++) {
    cs[ch].mean *= QuantumScale;
    cs[ch].median *= QuantumScale;
    cs[ch].maxima *= QuantumScale;
    cs[ch].minima *= QuantumScale;
    cs[ch].standard_deviation *= QuantumScale;
  }

  return cs;
}

static MagickBooleanType CollectStatistics (FxInfo * pfx)
{
  Image * img = GetFirstImageInList (pfx->image);

  size_t imgNum=0;

  pfx->statistics = (ChannelStatistics**) AcquireMagickMemory ((size_t) pfx->ImgListLen * sizeof (ChannelStatistics *));
  if (!pfx->statistics) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), ResourceLimitFatalError,
      "Statistics", "%lu",
      (unsigned long) pfx->ImgListLen);
    return MagickFalse;
  }

  for (;;) {
    pfx->statistics[imgNum] = CollectOneImgStats (pfx, img);

    if (++imgNum == pfx->ImgListLen) break;
    img = GetNextImageInList (img);
    assert (img != (Image *) NULL);
  }
  pfx->GotStats = MagickTrue;

  return MagickTrue;
}

static inline MagickBooleanType PushVal (FxInfo * pfx, fxRtT * pfxrt, fxFltType val, int addr)
{
  if (pfxrt->usedValStack >=pfxrt->numValStack) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "ValStack overflow at addr=", "%i",
      addr);
    return MagickFalse;
  }

  pfxrt->ValStack[pfxrt->usedValStack++] = val;
  return MagickTrue;
}

static inline fxFltType PopVal (FxInfo * pfx, fxRtT * pfxrt, int addr)
{
  if (pfxrt->usedValStack <= 0) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "ValStack underflow at addr=", "%i",
      addr);
    return (fxFltType) 0;
  }

  return pfxrt->ValStack[--pfxrt->usedValStack];
}

static inline fxFltType ImageStat (
  FxInfo * pfx, ssize_t ImgNum, PixelChannel channel, ImgAttrE ia)
{
  ChannelStatistics * cs = NULL;
  fxFltType ret = 0;
  MagickBooleanType NeedRelinq = MagickFalse;

  if (ImgNum < 0)
    {
      (void) ThrowMagickException(pfx->exception,GetMagickModule(),
        OptionError,"NoSuchImage","%lu",(unsigned long) ImgNum);
      ImgNum=0;
    }

  if (pfx->GotStats) {
    if ((channel < 0) || (channel > MaxPixelChannels))
      {
        (void) ThrowMagickException(pfx->exception,GetMagickModule(),
          OptionError,"NoSuchImageChannel","%i",channel);
        channel=(PixelChannel) 0;
      }
    cs = pfx->statistics[ImgNum];
  } else if (pfx->NeedStats) {
    /* If we need more than one statistic per pixel, this is inefficient. */
    if ((channel < 0) || (channel > MaxPixelChannels))
      {
        (void) ThrowMagickException(pfx->exception,GetMagickModule(),
          OptionError,"NoSuchImageChannel","%i",channel);
        channel=(PixelChannel) 0;
      }
    cs = CollectOneImgStats (pfx, pfx->Images[ImgNum]);
    NeedRelinq = MagickTrue;
  }

  switch (ia) {
    case aDepth:
      ret = (fxFltType) GetImageDepth (pfx->Images[ImgNum], pfx->exception);
      break;
    case aExtent:
      ret = (fxFltType) GetBlobSize (pfx->image);
      break;
    case aKurtosis:
      if ((cs != (ChannelStatistics *) NULL) && (channel >= 0))
        ret = cs[channel].kurtosis;
      break;
    case aMaxima:
      if ((cs != (ChannelStatistics *) NULL) && (channel >= 0))
        ret = cs[channel].maxima;
      break;
    case aMean:
      if ((cs != (ChannelStatistics *) NULL) && (channel >= 0))
        ret = cs[channel].mean;
      break;
    case aMedian:
      if ((cs != (ChannelStatistics *) NULL) && (channel >= 0))
        ret = cs[channel].median;
      break;
    case aMinima:
      if ((cs != (ChannelStatistics *) NULL) && (channel >= 0))
        ret = cs[channel].minima;
      break;
    case aPage:
      /* Do nothing */
      break;
    case aPageX:
      ret = (fxFltType) pfx->Images[ImgNum]->page.x;
      break;
    case aPageY:
      ret = (fxFltType) pfx->Images[ImgNum]->page.y;
      break;
    case aPageWid:
      ret = (fxFltType) pfx->Images[ImgNum]->page.width;
      break;
    case aPageHt:
      ret = (fxFltType) pfx->Images[ImgNum]->page.height;
      break;
    case aPrintsize:
      /* Do nothing */
      break;
    case aPrintsizeX:
      ret = (fxFltType) MagickSafeReciprocal (pfx->Images[ImgNum]->resolution.x)
                        * pfx->Images[ImgNum]->columns;
      break;
    case aPrintsizeY:
      ret = (fxFltType) MagickSafeReciprocal (pfx->Images[ImgNum]->resolution.y)
                        * pfx->Images[ImgNum]->rows;
      break;
    case aQuality:
      ret = (fxFltType) pfx->Images[ImgNum]->quality;
      break;
    case aRes:
      /* Do nothing */
      break;
    case aResX:
      ret = pfx->Images[ImgNum]->resolution.x;
      break;
    case aResY:
      ret = pfx->Images[ImgNum]->resolution.y;
      break;
    case aSkewness:
      if ((cs != (ChannelStatistics *) NULL) && (channel >= 0))
        ret = cs[channel].skewness;
      break;
    case aStdDev:
      if ((cs != (ChannelStatistics *) NULL) && (channel >= 0))
        ret = cs[channel].standard_deviation;
      break;
    case aH:
      ret = (fxFltType) pfx->Images[ImgNum]->rows;
      break;
    case aN:
      ret = (fxFltType) pfx->ImgListLen;
      break;
    case aT: /* image index in list */
      ret = (fxFltType) ImgNum;
      break;
    case aW:
      ret = (fxFltType) pfx->Images[ImgNum]->columns;
      break;
    case aZ:
      ret = (fxFltType) GetImageDepth (pfx->Images[ImgNum], pfx->exception);
      break;
    default:
      (void) ThrowMagickException (pfx->exception,GetMagickModule(),OptionError,
        "Unknown ia=","%i",ia);
  }
  if (NeedRelinq) cs = (ChannelStatistics *)RelinquishMagickMemory (cs);

  return ret;
}

static inline fxFltType FxGcd (fxFltType x, fxFltType y, const size_t depth)
{
#define FxMaxFunctionDepth  200

  if (x < y)
    return (FxGcd (y, x, depth+1));
  if ((fabs((double) y) < 0.001) || (depth >= FxMaxFunctionDepth))
    return (x);
  return (FxGcd (y, x-y*floor((double) (x/y)), depth+1));
}

static inline ssize_t ChkImgNum (FxInfo * pfx, fxFltType f)
/* Returns -1 if f is too large. */
{
  ssize_t i = (ssize_t) floor ((double) f + 0.5);
  if (i < 0) i += (ssize_t) pfx->ImgListLen;
  if (i < 0 || i >= (ssize_t) pfx->ImgListLen) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "ImgNum", "%lu bad for ImgListLen %lu",
      (unsigned long) i, (unsigned long) pfx->ImgListLen);
    i = -1;
  }
  return i;
}

#define WHICH_ATTR_CHAN \
  (pel->channel_qual == NO_CHAN_QUAL) ? CompositePixelChannel : \
  (pel->channel_qual == THIS_CHANNEL) ? channel : pel->channel_qual

#define WHICH_NON_ATTR_CHAN \
  (pel->channel_qual == NO_CHAN_QUAL || \
   pel->channel_qual == THIS_CHANNEL || \
   pel->channel_qual == CompositePixelChannel \
  ) ? (channel == CompositePixelChannel ? RedPixelChannel: channel) \
    : pel->channel_qual

static fxFltType GetHslFlt (FxInfo * pfx, ssize_t ImgNum, const fxFltType fx, const fxFltType fy,
  PixelChannel channel)
{
  Image * img = pfx->Images[ImgNum];

  double red, green, blue;
  double hue=0, saturation=0, lightness=0;

  MagickBooleanType okay = MagickTrue;
  if(!InterpolatePixelChannel (img, pfx->Imgs[ImgNum].View, RedPixelChannel, img->interpolate,
    (double) fx, (double) fy, &red, pfx->exception)) okay = MagickFalse;
  if(!InterpolatePixelChannel (img, pfx->Imgs[ImgNum].View, GreenPixelChannel, img->interpolate,
    (double) fx, (double) fy, &green, pfx->exception)) okay = MagickFalse;
  if(!InterpolatePixelChannel (img, pfx->Imgs[ImgNum].View, BluePixelChannel, img->interpolate,
    (double) fx, (double) fy, &blue, pfx->exception)) okay = MagickFalse;

  if (!okay)
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "GetHslFlt failure", "%lu %g,%g %i", (unsigned long) ImgNum,
      (double) fx, (double) fy, channel);

  ConvertRGBToHSL (
    red, green, blue,
    &hue, &saturation, &lightness);

  if (channel == HUE_CHANNEL)   return hue;
  if (channel == SAT_CHANNEL)   return saturation;
  if (channel == LIGHT_CHANNEL) return lightness;

  return 0.0;
}

static fxFltType GetHslInt (FxInfo * pfx, ssize_t ImgNum, const ssize_t imgx, const ssize_t imgy, PixelChannel channel)
{
  Image * img = pfx->Images[ImgNum];

  double hue=0, saturation=0, lightness=0;

  const Quantum * p = GetCacheViewVirtualPixels (pfx->Imgs[ImgNum].View, imgx, imgy, 1, 1, pfx->exception);
  if (p == (const Quantum *) NULL)
    {
      (void) ThrowMagickException (pfx->exception,GetMagickModule(),
        OptionError,"GetHslInt failure","%lu %li,%li %i",(unsigned long) ImgNum,
        (long) imgx,(long) imgy,channel);
      return(0.0);
    }

  ConvertRGBToHSL (
    GetPixelRed (img, p), GetPixelGreen (img, p), GetPixelBlue (img, p),
    &hue, &saturation, &lightness);

  if (channel == HUE_CHANNEL)   return hue;
  if (channel == SAT_CHANNEL)   return saturation;
  if (channel == LIGHT_CHANNEL) return lightness;

  return 0.0;
}

static inline fxFltType GetIntensity (FxInfo * pfx, ssize_t ImgNum, const fxFltType fx, const fxFltType fy)
{
  Quantum
    quantum_pixel[MaxPixelChannels];

  PixelInfo
    pixelinf;

  Image * img = pfx->Images[ImgNum];

  (void) GetPixelInfo (img, &pixelinf);

  if (!InterpolatePixelInfo (img, pfx->Imgs[pfx->ImgNum].View, img->interpolate,
              (double) fx, (double) fy, &pixelinf, pfx->exception))
  {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "GetIntensity failure", "%lu %g,%g", (unsigned long) ImgNum,
      (double) fx, (double) fy);
  }

  SetPixelViaPixelInfo (img, &pixelinf, quantum_pixel);
  return QuantumScale * GetPixelIntensity (img, quantum_pixel);
}

static MagickBooleanType ExecuteRPN (FxInfo * pfx, fxRtT * pfxrt, fxFltType *result,
  const PixelChannel channel, const ssize_t imgx, const ssize_t imgy)
{
  const Quantum * p = pfxrt->thisPixel;
  fxFltType regA=0, regB=0, regC=0, regD=0, regE=0;
  Image * img = pfx->image;
  ChannelStatistics * cs = NULL;
  MagickBooleanType NeedRelinq = MagickFalse;
  double hue=0, saturation=0, lightness=0;
  int i;

  /* For -fx, this sets p to ImgNum 0.
     for %[fx:...], this sets p to the current image.
     Similarly img.
  */
  if (!p) p = GetCacheViewVirtualPixels (
    pfx->Imgs[pfx->ImgNum].View, imgx, imgy, 1, 1, pfx->exception);

  if (p == (const Quantum *) NULL)
    {
      (void) ThrowMagickException (pfx->exception,GetMagickModule(),
        OptionError,"Can't get virtual pixels","%lu %li,%li",(unsigned long)
        pfx->ImgNum,(long) imgx,(long) imgy);
      return(MagickFalse);
    }

  if (pfx->GotStats) {
    cs = pfx->statistics[pfx->ImgNum];
  } else if (pfx->NeedStats) {
    cs = CollectOneImgStats (pfx, pfx->Images[pfx->ImgNum]);
    NeedRelinq = MagickTrue;
  }

  /*  Following is only for expressions like "saturation", with no image specifier.
  */
  if (pfx->NeedHsl) {
    ConvertRGBToHSL (
      GetPixelRed (img, p), GetPixelGreen (img, p), GetPixelBlue (img, p),
      &hue, &saturation, &lightness);
  }

  for (i=0; i < pfx->usedElements; i++) {
    ElementT
      *pel;

    if (i < 0) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Bad run-time address", "%i", i);
    }
    pel=&pfx->Elements[i];
    switch (pel->number_args) {
        case 0:
          break;
        case 1:
          regA = PopVal (pfx, pfxrt, i);
          break;
        case 2:
          regB = PopVal (pfx, pfxrt, i);
          regA = PopVal (pfx, pfxrt, i);
          break;
        case 3:
          regC = PopVal (pfx, pfxrt, i);
          regB = PopVal (pfx, pfxrt, i);
          regA = PopVal (pfx, pfxrt, i);
          break;
        case 4:
          regD = PopVal (pfx, pfxrt, i);
          regC = PopVal (pfx, pfxrt, i);
          regB = PopVal (pfx, pfxrt, i);
          regA = PopVal (pfx, pfxrt, i);
          break;
        case 5:
          regE = PopVal (pfx, pfxrt, i);
          regD = PopVal (pfx, pfxrt, i);
          regC = PopVal (pfx, pfxrt, i);
          regB = PopVal (pfx, pfxrt, i);
          regA = PopVal (pfx, pfxrt, i);
          break;
        default:
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Too many args:", "%i", pel->number_args);
          break;
      }

      switch (pel->operator_index) {
        case oAddEq:
          regA = (pfxrt->UserSymVals[pel->element_index] += regA);
          break;
        case oSubtractEq:
          regA = (pfxrt->UserSymVals[pel->element_index] -= regA);
          break;
        case oMultiplyEq:
          regA = (pfxrt->UserSymVals[pel->element_index] *= regA);
          break;
        case oDivideEq:
          regA = (pfxrt->UserSymVals[pel->element_index] /= regA);
          break;
        case oPlusPlus:
          regA = pfxrt->UserSymVals[pel->element_index]++;
          break;
        case oSubSub:
          regA = pfxrt->UserSymVals[pel->element_index]--;
          break;
        case oAdd:
          regA += regB;
          break;
        case oSubtract:
          regA -= regB;
          break;
        case oMultiply:
          regA *= regB;
          break;
        case oDivide:
          regA /= regB;
          break;
        case oModulus:
          regA = fmod ((double) regA, fabs(floor((double) regB+0.5)));
          break;
        case oUnaryPlus:
          /* Do nothing. */
          break;
        case oUnaryMinus:
          regA = -regA;
          break;
        case oLshift:
          if ((size_t) (regB+0.5) >= (8*sizeof(size_t)))
            {
              (void) ThrowMagickException ( pfx->exception, GetMagickModule(),
                OptionError, "undefined shift", "%g", (double) regB);
              regA = (fxFltType) 0.0;
              break;
            }
          regA = (fxFltType) ((size_t)(regA+0.5) << (size_t)(regB+0.5));
          break;
        case oRshift:
          if ((size_t) (regB+0.5) >= (8*sizeof(size_t)))
            {
              (void) ThrowMagickException ( pfx->exception, GetMagickModule(),
                OptionError, "undefined shift", "%g", (double) regB);
              regA = (fxFltType) 0.0;
              break;
            }
          regA = (fxFltType) ((size_t)(regA+0.5) >> (size_t)(regB+0.5));
          break;
        case oEq:
          regA = fabs((double) (regA-regB)) < MagickEpsilon ? 1.0 : 0.0;
          break;
        case oNotEq:
          regA = fabs((double) (regA-regB)) >= MagickEpsilon ? 1.0 : 0.0;
          break;
        case oLtEq:
          regA = (regA <= regB) ? 1.0 : 0.0;
          break;
        case oGtEq:
          regA = (regA >= regB) ? 1.0 : 0.0;
          break;
        case oLt:
          regA = (regA < regB) ? 1.0 : 0.0;
          break;
        case oGt:
          regA = (regA > regB) ? 1.0 : 0.0;
          break;
        case oLogAnd:
          regA = (regA<=0) ? 0.0 : (regB > 0) ? 1.0 : 0.0;
          break;
        case oLogOr:
          regA = (regA>0) ? 1.0 : (regB > 0.0) ? 1.0 : 0.0;
          break;
        case oLogNot:
          regA = (regA==0) ? 1.0 : 0.0;
          break;
        case oBitAnd:
          regA = (fxFltType) ((size_t)(regA+0.5) & (size_t)(regB+0.5));
          break;
        case oBitOr:
          regA = (fxFltType) ((size_t)(regA+0.5) | (size_t)(regB+0.5));
          break;
        case oBitNot:
          /* Old fx doesn't add 0.5. */
          regA = (fxFltType) (~(size_t)(regA+0.5));
          break;
        case oPow:
          regA = pow ((double) regA, (double) regB);
          break;
        case oQuery:
        case oColon:
          break;
        case oOpenParen:
        case oCloseParen:
        case oOpenBracket:
        case oCloseBracket:
        case oOpenBrace:
        case oCloseBrace:
          break;
        case oAssign:
          pel->val = regA;
          break;
        case oNull: {
          if (pel->type == etColourConstant) {
            switch (channel) { default:
              case (PixelChannel) 0:
                regA = pel->val;
                break;
              case (PixelChannel) 1:
                regA = pel->val1;
                break;
              case (PixelChannel) 2:
                regA = pel->val2;
                break;
            }
          } else {
            regA = pel->val;
          }
          break;
        }
        case fAbs:
          regA = fabs ((double) regA);
          break;
#if defined(MAGICKCORE_HAVE_ACOSH)
        case fAcosh:
          regA = acosh ((double) regA);
          break;
#endif
        case fAcos:
          regA = acos ((double) regA);
          break;
#if defined(MAGICKCORE_HAVE_J1)
        case fAiry:
          if (regA==0) regA = 1.0;
          else {
            fxFltType gamma = 2.0 * __j1((double) (MagickPI*regA)) / (MagickPI*regA);
            regA = gamma * gamma;
          }
          break;
#endif
        case fAlt:
          regA = (fxFltType) (((ssize_t) regA) & 0x01 ? -1.0 : 1.0);
          break;
#if defined(MAGICKCORE_HAVE_ASINH)
        case fAsinh:
          regA = asinh ((double) regA);
          break;
#endif
        case fAsin:
          regA = asin ((double) regA);
          break;
#if defined(MAGICKCORE_HAVE_ATANH)
        case fAtanh:
          regA = atanh ((double) regA);
          break;
#endif
        case fAtan2:
          regA = atan2 ((double) regA, (double) regB);
          break;
        case fAtan:
          regA = atan ((double) regA);
          break;
        case fCeil:
          regA = ceil ((double) regA);
          break;
        case fChannel:
          switch (channel) {
            case (PixelChannel) 0: break;
            case (PixelChannel) 1: regA = regB; break;
            case (PixelChannel) 2: regA = regC; break;
            case (PixelChannel) 3: regA = regD; break;
            case (PixelChannel) 4: regA = regE; break;
            default: regA = 0.0;
          }
          break;
        case fClamp:
          if (regA < 0) regA = 0.0;
          else if (regA > 1.0) regA = 1.0;
          break;
        case fCosh:
          regA = cosh ((double) regA);
          break;
        case fCos:
          regA = cos ((double) regA);
          break;
        case fDebug:
          /* FIXME: debug() should give channel name. */

          (void) fprintf (stderr, "%s[%g,%g].[%i]: %s=%.*g\n",
                   img->filename, (double) imgx, (double) imgy,
                   channel, SetPtrShortExp (pfx, pel->exp_start, (size_t) (pel->exp_len+1)),
                   pfx->precision, (double) regA);
          break;
        case fDrc:
          regA = regA / (regB*(regA-1.0) + 1.0);
          break;
#if defined(MAGICKCORE_HAVE_ERF)
        case fErf:
          regA = erf ((double) regA);
          break;
#endif
        case fEpoch:
          /* Do nothing. */
          break;
        case fExp:
          regA = exp ((double) regA);
          break;
        case fFloor:
          regA = floor ((double) regA);
          break;
        case fGauss:
          regA = exp((double) (-regA*regA/2.0))/sqrt(2.0*MagickPI);
          break;
        case fGcd:
          if (!IsNaN((double) regA))
            regA = FxGcd (regA, regB, 0);
          break;
        case fHypot:
          regA = hypot ((double) regA, (double) regB);
          break;
        case fInt:
          regA = floor ((double) regA);
          break;
        case fIsnan:
          regA = (fxFltType) (!!IsNaN ((double) regA));
          break;
#if defined(MAGICKCORE_HAVE_J0)
        case fJ0:
          regA = __j0((double) regA);
          break;
#endif
#if defined(MAGICKCORE_HAVE_J1)
        case fJ1:
          regA = __j1((double) regA);
          break;
#endif
#if defined(MAGICKCORE_HAVE_J1)
        case fJinc:
          if (regA==0) regA = 1.0;
          else regA = 2.0 * __j1((double) (MagickPI*regA))/(MagickPI*regA);
          break;
#endif
        case fLn:
          regA = log ((double) regA);
          break;
        case fLogtwo:
          regA = log10((double) regA) / log10(2.0);
          break;
        case fLog:
          regA = log10 ((double) regA);
          break;
        case fMagickTime:
          regA = (fxFltType) GetMagickTime();
          break;
        case fMax:
          regA = (regA > regB) ? regA : regB;
          break;
        case fMin:
          regA = (regA < regB) ? regA : regB;
          break;
        case fMod:
          if (regB == 0) {
            regA = 0;
          } else {
            regA = regA - floor((double) (regA/regB))*regB;
          }
          break;
        case fNot:
          regA = (fxFltType) (regA < MagickEpsilon);
          break;
        case fPow:
          regA = pow ((double) regA, (double) regB);
          break;
        case fRand: {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp critical (MagickCore_ExecuteRPN)
#endif
          regA = GetPseudoRandomValue (pfxrt->random_info);
          break;
        }
        case fRound:
          regA = floor ((double) regA + 0.5);
          break;
        case fSign:
          regA = (regA < 0) ? -1.0 : 1.0;
          break;
        case fSinc:
          regA = sin ((double) (MagickPI*regA)) / (MagickPI*regA);
          break;
        case fSinh:
          regA = sinh ((double) regA);
          break;
        case fSin:
          regA = sin ((double) regA);
          break;
        case fSqrt:
          regA = sqrt ((double) regA);
          break;
        case fSquish:
          regA = 1.0 / (1.0 + exp ((double) -regA));
          break;
        case fTanh:
          regA = tanh ((double) regA);
          break;
        case fTan:
          regA = tan ((double) regA);
          break;
        case fTrunc:
          if (regA >= 0) regA = floor ((double) regA);
          else regA = ceil ((double) regA);
          break;

        case fDo:
        case fFor:
        case fIf:
        case fWhile:
          break;
        case fU: {
          /* Note: 1 value is available, index into image list.
             May have ImgAttr qualifier or channel qualifier or both.
          */
          ssize_t ImgNum = ChkImgNum (pfx, regA);
          if (ImgNum < 0) break;
          regA = (fxFltType) 0;
          if (ImgNum == 0) {
            Image * pimg = pfx->Images[0];
            if (pel->img_attr_qual == aNull) {
              if ((int) pel->channel_qual < 0) {
                if (pel->channel_qual == NO_CHAN_QUAL || pel->channel_qual == THIS_CHANNEL) {
                  if (pfx->ImgNum==0) {
                    regA = QuantumScale * (double) p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                  } else {
                    const Quantum * pv = GetCacheViewVirtualPixels (
                                   pfx->Imgs[0].View, imgx, imgy, 1,1, pfx->exception);
                    if (!pv) {
                      (void) ThrowMagickException (
                        pfx->exception, GetMagickModule(), OptionError,
                        "fU can't get cache", "%lu", (unsigned long) ImgNum);
                      break;
                    }
                    regA = QuantumScale * (double) pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                  }
                } else if (pel->channel_qual == HUE_CHANNEL || pel->channel_qual == SAT_CHANNEL ||
                    pel->channel_qual == LIGHT_CHANNEL) {
                  regA = GetHslInt (pfx, ImgNum, imgx, imgy, pel->channel_qual);
                  break;
                } else if (pel->channel_qual == INTENSITY_CHANNEL) {
                  regA = GetIntensity (pfx, 0, (double) imgx, (double) imgy);
                  break;
                }
              } else {
                if (pfx->ImgNum==0) {
                  regA = QuantumScale * (double) p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                } else {
                  const Quantum * pv = GetCacheViewVirtualPixels (
                                 pfx->Imgs[0].View, imgx, imgy, 1,1, pfx->exception);
                  if (!pv) {
                    (void) ThrowMagickException (
                      pfx->exception, GetMagickModule(), OptionError,
                      "fU can't get cache", "%lu", (unsigned long) ImgNum);
                    break;
                  }
                  regA = QuantumScale * (double) pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                }
              }
            } else {
              /* we have an image attribute */
              regA = ImageStat (pfx, 0, WHICH_ATTR_CHAN, pel->img_attr_qual);
            }
          } else {
            /* We have non-zero ImgNum. */
            if (pel->img_attr_qual == aNull) {
              const Quantum * pv;
              if ((int) pel->channel_qual < 0) {
                if (pel->channel_qual == HUE_CHANNEL || pel->channel_qual == SAT_CHANNEL ||
                    pel->channel_qual == LIGHT_CHANNEL)
                {
                  regA = GetHslInt (pfx, ImgNum, imgx, imgy, pel->channel_qual);
                  break;
                } else if (pel->channel_qual == INTENSITY_CHANNEL)
                {
                  regA = GetIntensity (pfx, ImgNum, (fxFltType) imgx, (fxFltType) imgy);
                  break;
                }
              }

              pv = GetCacheViewVirtualPixels (
                     pfx->Imgs[ImgNum].View, imgx, imgy, 1,1, pfx->exception);
              if (!pv) {
                (void) ThrowMagickException (
                  pfx->exception, GetMagickModule(), OptionError,
                  "fU can't get cache", "%lu", (unsigned long) ImgNum);
                break;
              }
              regA = QuantumScale * (double)
                pv[pfx->Images[ImgNum]->channel_map[WHICH_NON_ATTR_CHAN].offset];
            } else {
              regA = ImageStat (pfx, ImgNum, WHICH_ATTR_CHAN, pel->img_attr_qual);
            }
          }
          break;
        }
        case fU0: {
          /* No args. No image attribute. We may have a ChannelQual.
             If called from %[fx:...], ChannelQual will be CompositePixelChannel.
          */
          Image * pimg = pfx->Images[0];
          if ((int) pel->channel_qual < 0) {
            if (pel->channel_qual == NO_CHAN_QUAL || pel->channel_qual == THIS_CHANNEL) {

              if (pfx->ImgNum==0) {
                regA = QuantumScale * (double) p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
              } else {
                const Quantum * pv = GetCacheViewVirtualPixels (
                               pfx->Imgs[0].View, imgx, imgy, 1,1, pfx->exception);
                if (!pv) {
                  (void) ThrowMagickException (
                    pfx->exception, GetMagickModule(), OptionError,
                    "fU0 can't get cache", "%i", 0);
                  break;
                }
                regA = QuantumScale * (double) pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
              }

            } else if (pel->channel_qual == HUE_CHANNEL || pel->channel_qual == SAT_CHANNEL ||
                       pel->channel_qual == LIGHT_CHANNEL) {
              regA = GetHslInt (pfx, 0, imgx, imgy, pel->channel_qual);
              break;
            } else if (pel->channel_qual == INTENSITY_CHANNEL) {
              regA = GetIntensity (pfx, 0, (fxFltType) imgx, (fxFltType) imgy);
            }
          } else {
            if (pfx->ImgNum==0) {
              regA = QuantumScale * (double) p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
            } else {
              const Quantum * pv = GetCacheViewVirtualPixels (
                                   pfx->Imgs[0].View, imgx, imgy, 1,1, pfx->exception);
              if (!pv) {
                (void) ThrowMagickException (
                  pfx->exception, GetMagickModule(), OptionError,
                  "fU0 can't get cache", "%i", 0);
                break;
              }
              regA = QuantumScale * (double) pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
            }
          }
          break;
        }
        case fUP: {
          /* 3 args are: ImgNum, x, y */
          ssize_t ImgNum = ChkImgNum (pfx, regA);
          fxFltType fx, fy;

          if (ImgNum < 0) break;

          if (pel->is_relative) {
            fx = imgx + regB;
            fy = imgy + regC;
          } else {
            fx = regB;
            fy = regC;
          }

          if ((int) pel->channel_qual < 0) {
            if (pel->channel_qual == HUE_CHANNEL || pel->channel_qual == SAT_CHANNEL
             || pel->channel_qual == LIGHT_CHANNEL) {
              regA = GetHslFlt (pfx, ImgNum, fx, fy, pel->channel_qual);
              break;
            } else if (pel->channel_qual == INTENSITY_CHANNEL) {
              regA = GetIntensity (pfx, ImgNum, fx, fy);
              break;
            }
          }

          {
            double v;
            Image * imUP = pfx->Images[ImgNum];
            if (! InterpolatePixelChannel (imUP, pfx->Imgs[ImgNum].View, WHICH_NON_ATTR_CHAN,
                    imUP->interpolate, (double) fx, (double) fy, &v, pfx->exception))
            {
              (void) ThrowMagickException (
                pfx->exception, GetMagickModule(), OptionError,
                "fUP can't get interpolate", "%lu", (unsigned long) ImgNum);
              break;
            }
            regA = v * QuantumScale;
          }

          break;
        }
        case fS:
        case fV: {
          /* No args. */
          ssize_t ImgNum = 1;
          if (pel->operator_index == fS) ImgNum = pfx->ImgNum;

          if (pel->img_attr_qual == aNull) {
            const Quantum * pv = GetCacheViewVirtualPixels (
                                   pfx->Imgs[ImgNum].View, imgx, imgy, 1,1, pfx->exception);
            if (!pv) {
              (void) ThrowMagickException (
                pfx->exception, GetMagickModule(), OptionError,
                "fV can't get cache", "%lu", (unsigned long) ImgNum);
              break;
            }

            if ((int) pel->channel_qual < 0) {
              if (pel->channel_qual == HUE_CHANNEL || pel->channel_qual == SAT_CHANNEL ||
                  pel->channel_qual == LIGHT_CHANNEL) {
                regA = GetHslInt (pfx, ImgNum, imgx, imgy, pel->channel_qual);
                break;
              } else if (pel->channel_qual == INTENSITY_CHANNEL) {
                regA = GetIntensity (pfx, ImgNum, (double) imgx, (double) imgy);
                break;
              }
            }

            regA = QuantumScale * (double)
              pv[pfx->Images[ImgNum]->channel_map[WHICH_NON_ATTR_CHAN].offset];
          } else {
            regA = ImageStat (pfx, ImgNum, WHICH_ATTR_CHAN, pel->img_attr_qual);
          }

          break;
        }
        case fP:
        case fSP:
        case fVP: {
          /* 2 args are: x, y */
          fxFltType fx, fy;
          ssize_t ImgNum = pfx->ImgNum;
          if (pel->operator_index == fVP) ImgNum = 1;
          if (pel->is_relative) {
            fx = imgx + regA;
            fy = imgy + regB;
          } else {
            fx = regA;
            fy = regB;
          }
          if ((int) pel->channel_qual < 0) {
            if (pel->channel_qual == HUE_CHANNEL || pel->channel_qual == SAT_CHANNEL ||
                pel->channel_qual == LIGHT_CHANNEL) {
              regA = GetHslFlt (pfx, ImgNum, fx, fy, pel->channel_qual);
              break;
            } else if (pel->channel_qual == INTENSITY_CHANNEL) {
              regA = GetIntensity (pfx, ImgNum, fx, fy);
              break;
            }
          }

          {
            double v;

            if (! InterpolatePixelChannel (pfx->Images[ImgNum], pfx->Imgs[ImgNum].View,
                                           WHICH_NON_ATTR_CHAN, pfx->Images[ImgNum]->interpolate,
                                           (double) fx, (double) fy, &v, pfx->exception)
                                          )
            {
              (void) ThrowMagickException (
                pfx->exception, GetMagickModule(), OptionError,
                "fSP or fVP can't get interp", "%lu", (unsigned long) ImgNum);
              break;
            }
            regA = v * (fxFltType)QuantumScale;
          }

          break;
        }
        case fNull:
          break;
        case aDepth:
          regA = (fxFltType) GetImageDepth (img, pfx->exception);
          break;
        case aExtent:
          regA = (fxFltType) img->extent;
          break;
        case aKurtosis:
          if ((cs != (ChannelStatistics *) NULL) && (channel > 0))
            regA = cs[WHICH_ATTR_CHAN].kurtosis;
          break;
        case aMaxima:
          if ((cs != (ChannelStatistics *) NULL) && (channel > 0))
            regA = cs[WHICH_ATTR_CHAN].maxima;
          break;
        case aMean:
          if ((cs != (ChannelStatistics *) NULL) && (channel > 0))
            regA = cs[WHICH_ATTR_CHAN].mean;
          break;
        case aMedian:
          if ((cs != (ChannelStatistics *) NULL) && (channel > 0))
            regA = cs[WHICH_ATTR_CHAN].median;
          break;
        case aMinima:
          if ((cs != (ChannelStatistics *) NULL) && (channel > 0))
            regA = cs[WHICH_ATTR_CHAN].minima;
          break;
        case aPage:
          break;
        case aPageX:
          regA = (fxFltType) img->page.x;
          break;
        case aPageY:
          regA = (fxFltType) img->page.y;
          break;
        case aPageWid:
          regA = (fxFltType) img->page.width;
          break;
        case aPageHt:
          regA = (fxFltType) img->page.height;
          break;
        case aPrintsize:
          break;
        case aPrintsizeX:
          regA = (fxFltType) MagickSafeReciprocal (img->resolution.x) * img->columns;
          break;
        case aPrintsizeY:
          regA = (fxFltType) MagickSafeReciprocal (img->resolution.y) * img->rows;
          break;
        case aQuality:
          regA = (fxFltType) img->quality;
          break;
        case aRes:
          break;
        case aResX:
          regA = (fxFltType) img->resolution.x;
          break;
        case aResY:
          regA = (fxFltType) img->resolution.y;
          break;
        case aSkewness:
          if ((cs != (ChannelStatistics *) NULL) && (channel > 0))
            regA = cs[WHICH_ATTR_CHAN].skewness;
          break;
        case aStdDev:
          if ((cs != (ChannelStatistics *) NULL) && (channel > 0))
            regA = cs[WHICH_ATTR_CHAN].standard_deviation;
          break;
        case aH: /* image->rows */
          regA = (fxFltType) img->rows;
          break;
        case aN: /* image list length */
          regA = (fxFltType) pfx->ImgListLen;
          break;
        case aT: /* image index in list */
          regA = (fxFltType) pfx->ImgNum;
          break;
        case aW: /* image->columns */
          regA = (fxFltType) img->columns;
          break;
        case aZ: /* image depth */
          regA = (fxFltType) GetImageDepth (img, pfx->exception);
          break;
        case aNull:
          break;
        case sHue: /* of conversion to HSL */
          regA = hue;
          break;
        case sIntensity:
          regA = GetIntensity (pfx, pfx->ImgNum, (double) imgx, (double) imgy);
          break;
        case sLightness: /* of conversion to HSL */
          regA = lightness;
          break;
        case sLuma: /* calculation */
        case sLuminance: /* as Luma */
          regA = QuantumScale * (0.212656 * (double) GetPixelRed (img,p) +
                                 0.715158 * (double) GetPixelGreen (img,p) +
                                 0.072186 * (double) GetPixelBlue (img,p));
          break;
        case sSaturation: /* from conversion to HSL */
          regA = saturation;
          break;
        case sA: /* alpha */
          regA = QuantumScale * (double) GetPixelAlpha (img, p);
          break;
        case sB: /* blue */
          regA = QuantumScale * (double) GetPixelBlue (img, p);
          break;
        case sC: /* red (ie cyan) */
          regA = QuantumScale * (double) GetPixelCyan (img, p);
          break;
        case sG: /* green */
          regA = QuantumScale * (double) GetPixelGreen (img, p);
          break;
        case sI: /* current x-coordinate */
          regA = (fxFltType) imgx;
          break;
        case sJ: /* current y-coordinate */
          regA = (fxFltType) imgy;
          break;
        case sK: /* black of CMYK */
          regA = QuantumScale * (double) GetPixelBlack (img, p);
          break;
        case sM: /* green (ie magenta) */
          regA = QuantumScale * (double) GetPixelGreen (img, p);
          break;
        case sO: /* alpha */
          regA = QuantumScale * (double) GetPixelAlpha (img, p);
          break;
        case sR:
          regA = QuantumScale * (double) GetPixelRed (img, p);
          break;
        case sY:
          regA = QuantumScale * (double) GetPixelYellow (img, p);
          break;
        case sNull:
          break;

        case rGoto:
          assert (pel->element_index >= 0);
          i = pel->element_index-1; /* -1 because 'for' loop will increment. */
          break;
        case rGotoChk:
          assert (pel->element_index >= 0);
          i = pel->element_index-1; /* -1 because 'for' loop will increment. */
          if (IsImageTTLExpired(img) != MagickFalse) {
            i = pfx->usedElements-1; /* Do no more opcodes. */
            (void) ThrowMagickException (pfx->exception, GetMagickModule(),
              ResourceLimitFatalError, "TimeLimitExceeded", "`%s'", img->filename);
          }
          break;
        case rIfZeroGoto:
          assert (pel->element_index >= 0);
          if (fabs((double) regA) < MagickEpsilon) i = pel->element_index-1;
          break;
        case rIfNotZeroGoto:
          assert (pel->element_index >= 0);
          if (fabs((double) regA) > MagickEpsilon) i = pel->element_index-1;
          break;
        case rCopyFrom:
          assert (pel->element_index >= 0);
          regA = pfxrt->UserSymVals[pel->element_index];
          break;
        case rCopyTo:
          assert (pel->element_index >= 0);
          pfxrt->UserSymVals[pel->element_index] = regA;
          break;
        case rZerStk:
          pfxrt->usedValStack = 0;
          break;
        case rNull:
          break;

        default:
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "pel->oprNum", "%i '%s' not yet implemented",
            (int)pel->operator_index, OprStr(pel->operator_index));
          break;
    }
    if (pel->do_push)
      if (!PushVal (pfx, pfxrt, regA, i)) break;
  }

  if (pfxrt->usedValStack > 0) regA = PopVal (pfx, pfxrt, 9999);

  *result = regA;

  if (NeedRelinq) cs = (ChannelStatistics *)RelinquishMagickMemory (cs);

  if (pfx->exception->severity >= ErrorException)
    return MagickFalse;

  if (pfxrt->usedValStack != 0) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "ValStack not empty", "(%i)", pfxrt->usedValStack);
    return MagickFalse;
  }

  return MagickTrue;
}

/* Following is substitute for FxEvaluateChannelExpression().
*/
MagickPrivate MagickBooleanType FxEvaluateChannelExpression (
  FxInfo *pfx,
  const PixelChannel channel, const ssize_t x, const ssize_t y,
  double *result, ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  fxFltType ret;

  assert (pfx != NULL);
  assert (pfx->image != NULL);
  assert (pfx->Images != NULL);
  assert (pfx->Imgs != NULL);
  assert (pfx->fxrts != NULL);

  pfx->fxrts[id].thisPixel = NULL;

  if (!ExecuteRPN (pfx, &pfx->fxrts[id], &ret, channel, x, y)) {
    (void) ThrowMagickException (
      exception, GetMagickModule(), OptionError,
      "ExecuteRPN failed", " ");
    return MagickFalse;
  }

  *result = (double) ret;

  return MagickTrue;
}

static FxInfo *AcquireFxInfoPrivate (const Image * images, const char * expression,
  MagickBooleanType CalcAllStats, ExceptionInfo *exception)
{
  char chLimit;

  FxInfo * pfx = (FxInfo*) AcquireCriticalMemory (sizeof (*pfx));

  memset (pfx, 0, sizeof (*pfx));

  if (!InitFx (pfx, images, CalcAllStats, exception)) {
    pfx = (FxInfo*) RelinquishMagickMemory(pfx);
    return NULL;
  }

  if (!BuildRPN (pfx)) {
    (void) DeInitFx (pfx);
    pfx = (FxInfo*) RelinquishMagickMemory(pfx);
    return((FxInfo *) NULL);
  }

  if ((*expression == '@') && (strlen(expression) > 1))
    pfx->expression=FileToString(expression,~0UL,exception);
  if (pfx->expression == (char *) NULL)
    pfx->expression=ConstantString(expression);
  pfx->pex = (char *) pfx->expression;

  pfx->teDepth = 0;
  if (!TranslateStatementList (pfx, ";", &chLimit)) {
    (void) DestroyRPN (pfx);
    pfx->expression = DestroyString (pfx->expression);
    pfx->pex = NULL;
    (void) DeInitFx (pfx);
    pfx = (FxInfo*) RelinquishMagickMemory(pfx);
    return NULL;
  }

  if (pfx->teDepth) {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Translate expression depth", "(%i) not 0",
      pfx->teDepth);

    (void) DestroyRPN (pfx);
    pfx->expression = DestroyString (pfx->expression);
    pfx->pex = NULL;
    (void) DeInitFx (pfx);
    pfx = (FxInfo*) RelinquishMagickMemory(pfx);
    return NULL;
  }

  if (chLimit != '\0' && chLimit != ';') {
    (void) ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "AcquireFxInfo: TranslateExpression did not exhaust input", "(chLimit=%i) at'%s'",
      (int)chLimit, pfx->pex);

    (void) DestroyRPN (pfx);
    pfx->expression = DestroyString (pfx->expression);
    pfx->pex = NULL;
    (void) DeInitFx (pfx);
    pfx = (FxInfo*) RelinquishMagickMemory(pfx);
    return NULL;
  }

  if (pfx->NeedStats && pfx->runType == rtEntireImage && !pfx->statistics) {
    if (!CollectStatistics (pfx)) {
      (void) DestroyRPN (pfx);
      pfx->expression = DestroyString (pfx->expression);
      pfx->pex = NULL;
      (void) DeInitFx (pfx);
      pfx = (FxInfo*) RelinquishMagickMemory(pfx);
      return NULL;
    }
  }

  if (pfx->DebugOpt) {
    DumpTables (stderr);
    DumpUserSymbols (pfx, stderr);
    (void) DumpRPN (pfx, stderr);
  }

  {
    size_t number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
    ssize_t t;

    pfx->fxrts = (fxRtT *)AcquireQuantumMemory (number_threads, sizeof(fxRtT));
    if (!pfx->fxrts) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), ResourceLimitFatalError,
        "fxrts", "%lu",
        (unsigned long) number_threads);
      (void) DestroyRPN (pfx);
      pfx->expression = DestroyString (pfx->expression);
      pfx->pex = NULL;
      (void) DeInitFx (pfx);
      pfx = (FxInfo*) RelinquishMagickMemory(pfx);
      return NULL;
    }
    for (t=0; t < (ssize_t) number_threads; t++) {
      if (!AllocFxRt (pfx, &pfx->fxrts[t])) {
        (void) ThrowMagickException (
          pfx->exception, GetMagickModule(), ResourceLimitFatalError,
          "AllocFxRt t=", "%g",
          (double) t);
        {
          ssize_t t2;
          for (t2 = t-1; t2 >= 0; t2--) {
            DestroyFxRt (&pfx->fxrts[t]);
          }
        }
        pfx->fxrts = (fxRtT *) RelinquishMagickMemory (pfx->fxrts);
        (void) DestroyRPN (pfx);
        pfx->expression = DestroyString (pfx->expression);
        pfx->pex = NULL;
        (void) DeInitFx (pfx);
        pfx = (FxInfo*) RelinquishMagickMemory(pfx);
        return NULL;
      }
    }
  }
  return pfx;
}

FxInfo *AcquireFxInfo (const Image * images, const char * expression, ExceptionInfo *exception)
{
  return AcquireFxInfoPrivate (images, expression, MagickFalse, exception);
}

FxInfo *DestroyFxInfo (FxInfo * pfx)
{
  ssize_t t;

  assert (pfx != NULL);
  assert (pfx->image != NULL);
  assert (pfx->Images != NULL);
  assert (pfx->Imgs != NULL);
  assert (pfx->fxrts != NULL);

  for (t=0; t < (ssize_t) GetMagickResourceLimit(ThreadResource); t++) {
    DestroyFxRt (&pfx->fxrts[t]);
  }
  pfx->fxrts = (fxRtT *) RelinquishMagickMemory (pfx->fxrts);

  DestroyRPN (pfx);

  pfx->expression = DestroyString (pfx->expression);
  pfx->pex = NULL;

  (void) DeInitFx (pfx);

  pfx = (FxInfo*) RelinquishMagickMemory(pfx);

  return NULL;
}

/* Following is substitute for FxImage().
*/
MagickExport Image *FxImage(const Image *image,const char *expression,
  ExceptionInfo *exception)
{
#define FxImageTag  "FxNew/Image"

  CacheView
    *fx_view,
    *image_view;

  Image
    *fx_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  FxInfo
    *pfx;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (expression == (const char *) NULL)
    return(CloneImage(image,0,0,MagickTrue,exception));
  fx_image=CloneImage(image,0,0,MagickTrue,exception);
  if (!fx_image) return NULL;
  if (SetImageStorageClass(fx_image,DirectClass,exception) == MagickFalse) {
    fx_image=DestroyImage(fx_image);
    return NULL;
  }

  pfx = AcquireFxInfoPrivate (image, expression, MagickTrue, exception);

  if (!pfx) {
    fx_image=DestroyImage(fx_image);
    return NULL;
  }

  assert (pfx->image != NULL);
  assert (pfx->Images != NULL);
  assert (pfx->Imgs != NULL);
  assert (pfx->fxrts != NULL);

  status=MagickTrue;
  progress=0;
  image_view = AcquireVirtualCacheView (image, pfx->exception);
  fx_view = AcquireAuthenticCacheView (fx_image, pfx->exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic) shared(progress,status) \
    magick_number_threads(image,fx_image,fx_image->rows, \
      pfx->ContainsDebug ? 0 : 1)
#endif
  for (y=0; y < (ssize_t) fx_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    fxFltType
      result = 0.0;

    if (status == MagickFalse)
      continue;
    p = GetCacheViewVirtualPixels (image_view, 0, y, image->columns, 1, pfx->exception);
    q = QueueCacheViewAuthenticPixels (fx_view, 0, y, fx_image->columns, 1, pfx->exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL)) {
        status=MagickFalse;
        continue;
    }
    for (x=0; x < (ssize_t) fx_image->columns; x++) {
      ssize_t i;

      pfx->fxrts[id].thisPixel = (Quantum *)p;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel (image, i);
        PixelTrait traits = GetPixelChannelTraits (image, channel);
        PixelTrait fx_traits = GetPixelChannelTraits (fx_image, channel);
        if ((traits == UndefinedPixelTrait) ||
            (fx_traits == UndefinedPixelTrait))
          continue;
        if ((fx_traits & CopyPixelTrait) != 0) {
            SetPixelChannel (fx_image, channel, p[i], q);
            continue;
        }

        if (!ExecuteRPN (pfx, &pfx->fxrts[id], &result, channel, x, y)) {
          status=MagickFalse;
          break;
        }

        q[i] = ClampToQuantum ((MagickRealType) (QuantumRange*result));
      }
      p+=(ptrdiff_t) GetPixelChannels (image);
      q+=(ptrdiff_t) GetPixelChannels (fx_image);
    }
    if (SyncCacheViewAuthenticPixels(fx_view, pfx->exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed = SetImageProgress (image, FxImageTag, progress, image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }

  fx_view = DestroyCacheView (fx_view);
  image_view = DestroyCacheView (image_view);

  /* Before destroying the user symbol values, dump them to stderr.
  */
  if (pfx->DebugOpt && pfx->usedUserSymbols) {
    int t, i;
    char UserSym[MagickPathExtent];
    fprintf (stderr, "User symbols (%i):\n", pfx->usedUserSymbols);
    for (t=0; t < (int) GetMagickResourceLimit(ThreadResource); t++) {
      for (i = 0; i < (int) pfx->usedUserSymbols; i++) {
        fprintf (stderr, "th=%i us=%i '%s': %.*Lg\n",
                 t, i, NameOfUserSym (pfx, i, UserSym), pfx->precision, pfx->fxrts[t].UserSymVals[i]);
      }
    }
  }

  if ((status == MagickFalse) || (pfx->exception->severity >= ErrorException))
    fx_image=DestroyImage(fx_image);

  pfx=DestroyFxInfo(pfx);

  return(fx_image);
}

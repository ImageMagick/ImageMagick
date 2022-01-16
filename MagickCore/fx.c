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
%  Copyright 1999-2022 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
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
  OperatorE op;
  const char * str;
  int precedence;
  int nArgs;
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
  {oLogAnd,      "&&",    10, 2},
  {oLogOr,       "||",     5, 2},
  {oLogNot,      "!",      5, 1},
  {oBitAnd,      "&",      6, 2},
  {oBitOr,       "|",      7, 2},
  {oBitNot,      "~",      8, 1},
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
  ConstantE cons;
  fxFltType val;
  const char * str;
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

#define FirstFunc fAbs

typedef enum {
  fAbs = (oNull+1),
#if defined(MAGICKCORE_HAVE_ACOSH)
  fAcosh,
#endif
  fAcos,
  fAiry,
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
  FunctionE func;
  const char * str;
  int nArgs;
} FunctionT;

static const FunctionT Functions[] = {
  {fAbs,     "abs"   , 1},
#if defined(MAGICKCORE_HAVE_ACOSH)
  {fAcosh,   "acosh" , 1},
#endif
  {fAcos,    "acos"  , 1},
  {fAiry,    "airy"  , 1},
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
  {fChannel, "channel"  , 5},
  {fClamp,   "clamp" , 1},
  {fCosh,    "cosh"  , 1},
  {fCos,     "cos"   , 1},
  {fDebug,   "debug" , 1},
  {fDrc,     "drc"   , 2},
#if defined(MAGICKCORE_HAVE_ERF)
  {fErf,     "erf"   , 1},
#endif
  {fExp,     "exp"   , 1},
  {fFloor,   "floor" , 1},
  {fGauss,   "gauss" , 2},
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

#define FirstImgAttr aDepth

typedef enum {
  aDepth = fNull+1,
  aExtent,
  aKurtosis,
  aMaxima,
  aMean,
  aMedian,
  aMinima,
  aPage,
  aPageWid,
  aPageHt,
  aPageX,
  aPageY,
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
  ImgAttrE attr;
  const char * str;
  int NeedStats;
} ImgAttrT;

static const ImgAttrT ImgAttrs[] = {
  {aDepth,      "depth",              1},
  {aExtent,     "extent",             0},
  {aKurtosis,   "kurtosis",           1},
  {aMaxima,     "maxima",             1},
  {aMean,       "mean",               1},
  {aMedian,     "median",             1},
  {aMinima,     "minima",             1},
  {aPage,       "page",               0},
  {aPageX,      "page.x",             0},
  {aPageY,      "page.y",             0},
  {aPageWid,    "page.width",         0},
  {aPageHt,     "page.height",        0},
  {aPrintsize,  "printsize",          0},
  {aPrintsizeX, "printsize.x",        0},
  {aPrintsizeY, "printsize.y",        0},
  {aQuality,    "quality",            0},
  {aRes,        "resolution",         0},
  {aResX,       "resolution.x",       0},
  {aResY,       "resolution.y",       0},
  {aSkewness,   "skewness",           1},
  {aStdDev,     "standard_deviation", 1},
  {aH,          "h", 0},
  {aN,          "n", 0},
  {aT,          "t", 0},
  {aW,          "w", 0},
  {aZ,          "z", 0},

  {aNull,       "anull", 0}
};

#define FirstSym sHue

typedef enum {
  sHue = (aNull+1),
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
  SymbolE sym;
  const char * str;
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
  rIfZeroGoto,
  rIfNotZeroGoto,
  rCopyFrom,
  rCopyTo,
  rZerStk,
  rNull
} ControlE;

typedef struct {
  ControlE cont;
  const char * str;
  int nArgs;
} ControlT;

static const ControlT Controls[] = {
  {rGoto,          "goto",          0},
  {rIfZeroGoto,    "ifzerogoto",    1},
  {rIfNotZeroGoto, "ifnotzerogoto", 1},
  {rCopyFrom,      "copyfrom",      0},
  {rCopyTo,        "copyto",        1},
  {rZerStk,        "zerstk",        0},
  {rNull,          "rnull",         0}
};

#define NULL_ADDRESS -2

typedef struct {
  int addrQuery;
  int addrColon;
} TernaryT;

typedef struct {
  const char * str;
  PixelChannel pixChan;
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
  char * pex;
  size_t len;
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
  ElementTypeE type;
  fxFltType
    val, val1, val2;
  int oprNum;
  int nArgs;
  MagickBooleanType IsRelative;
  MagickBooleanType DoPush;
  int EleNdx;
  int nDest; /* Number of Elements that "goto" this element */
  PixelChannel ChannelQual;
  ImgAttrE ImgAttrQual;
  char * pExpStart;
  size_t lenExp;
} ElementT;

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
  int ImgListLen;
  int ImgNum;
  MagickBooleanType NeedStats;
  MagickBooleanType NeedHsl;
  MagickBooleanType DebugOpt;       /* Whether "-debug" option is in effect */
  MagickBooleanType ContainsDebug;  /* Whether expression contains "debug ()" function */
  char * expression;
  char * pex;
  char ShortExp[MaxTokenLen]; /* for reporting */
  int teDepth;
  char token[MaxTokenLen];
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

  RandomInfo
    **magick_restrict random_infos;

  CacheView ** Views;
  Image ** Images;

  ExceptionInfo * exception;

  fxRtT * fxrts;
};

/* Forward declarations for recursion.
*/
static MagickBooleanType TranslateStatementList
  (FxInfo * fx_info, char * strLimit, char * chLimit);

static MagickBooleanType TranslateExpression
  (FxInfo * fx_info, char * strLimit, char * chLimit, MagickBooleanType * needPopAll);

static MagickBooleanType GetFunction (FxInfo * fx_info, FunctionE fe);

static MagickBooleanType InitFx (FxInfo * fx_info, const Image * img, ExceptionInfo *exception)
{
  fx_info->ImgListLen = (int) GetImageListLength (img);
  fx_info->ImgNum = (int) GetImageIndexInList (img);
  fx_info->image = (Image *)img;

  fx_info->NeedStats = MagickFalse;
  fx_info->NeedHsl = MagickFalse;
  fx_info->DebugOpt = IsStringTrue (GetImageArtifact (img, "fx:debug"));
  fx_info->statistics = NULL;
  fx_info->Views = NULL;
  fx_info->Images = NULL;
  fx_info->exception = exception;
  fx_info->precision = GetMagickPrecision ();
  fx_info->random_infos = AcquireRandomInfoThreadSet ();
  fx_info->ContainsDebug = MagickFalse;

  fx_info->Views =
    (CacheView **) AcquireQuantumMemory (GetImageListLength (img), sizeof (fx_info->Views));
  if (!fx_info->Views) ThrowFatalException(ResourceLimitFatalError, "Views oom");
  ssize_t i=0;
  const Image * next = GetFirstImageInList (img);
  for ( ; next != (Image *) NULL; next=next->next)
  {
    fx_info->Views[i] = AcquireVirtualCacheView (next, fx_info->exception);
    if (!fx_info->Views[i]) ThrowFatalException(ResourceLimitFatalError, "Views[i] oom");
    i++;
  }

  fx_info->Images = ImageListToArray (img, fx_info->exception);

  return MagickTrue;
}

static MagickBooleanType DeInitFx (FxInfo * fx_info)
{
  ssize_t i;
  if (fx_info->Images) RelinquishMagickMemory (fx_info->Images);

  if (fx_info->Views) {
    for (i=(ssize_t) GetImageListLength(fx_info->image)-1; i >= 0; i--)
      fx_info->Views[i] = DestroyCacheView (fx_info->Views[i]);
    fx_info->Views=(CacheView **) RelinquishMagickMemory (fx_info->Views);
  }

  fx_info->random_infos = DestroyRandomInfoThreadSet (fx_info->random_infos);

  if (fx_info->statistics) {
    for (i=(ssize_t) GetImageListLength(fx_info->image)-1; i >= 0; i--) {
      RelinquishMagickMemory (fx_info->statistics[i]);
    }

    fx_info->statistics = (ChannelStatistics **) RelinquishMagickMemory(fx_info->statistics);
  }

  return MagickTrue;
}

static ElementTypeE TypeOfOpr (FxInfo * fx_info, int op)
{
  (void) fx_info;
  if (op <  oNull) return etOperator;
  if (op == oNull) return etConstant;
  if (op <= fNull) return etFunction;
  if (op <= aNull) return etImgAttr;
  if (op <= sNull) return etSymbol;
  if (op <= rNull) return etControl;

  return (ElementTypeE) 0;
}

static char * SetPtrShortExp (FxInfo * fx_info, char * pExp, size_t len)
{
  #define MaxLen 20

  *fx_info->ShortExp = '\0';

  if (pExp && len) {
    size_t slen = CopyMagickString (fx_info->ShortExp, pExp, len);
    if (slen > MaxLen) { 
      CopyMagickString (fx_info->ShortExp+MaxLen, "...", 4);
    }
    char * p = strchr (fx_info->ShortExp, '\n');
    if (p) CopyMagickString (p, "...", 4);
    p = strchr (fx_info->ShortExp, '\r');
    if (p) CopyMagickString (p, "...", 4);
  }
  return fx_info->ShortExp;
}

static char * SetShortExp (FxInfo * fx_info)
{
  return SetPtrShortExp (fx_info, fx_info->pex, MaxTokenLen-1);
}

static int FindUserSymbol (FxInfo * fx_info, char * name)
/* returns index into fx_info->UserSymbols, and thus into fx_infort->UserSymVals,
   or NULL_ADDRESS if not found.
*/
{
  ssize_t i;
  size_t lenName = strlen (name);
  for (i=0; i < fx_info->usedUserSymbols; i++) {
    UserSymbolT *pus = &fx_info->UserSymbols[i];
    if (lenName == pus->len && LocaleNCompare (name, pus->pex, lenName)==0) break;
  }
  if (i == fx_info->usedUserSymbols) return NULL_ADDRESS;
  return i;
}

static MagickBooleanType ExtendUserSymbols (FxInfo * fx_info)
{
  fx_info->numUserSymbols = ceil (fx_info->numUserSymbols * (1 + TableExtend));
  fx_info->UserSymbols = (UserSymbolT *) ResizeMagickMemory (fx_info->UserSymbols, fx_info->numUserSymbols * sizeof(UserSymbolT));
  if (!fx_info->UserSymbols) {
    (void) ThrowMagickException(fx_info->exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",
      fx_info->image->filename);
    return MagickFalse;
  }
  return MagickTrue;
}

static int AddUserSymbol (FxInfo * fx_info, char * pex, size_t len)
{
  if (++fx_info->usedUserSymbols >= fx_info->numUserSymbols) {
    if (!ExtendUserSymbols (fx_info)) return -1;
  }
  UserSymbolT *pus = &fx_info->UserSymbols[fx_info->usedUserSymbols-1];
  pus->pex = pex;
  pus->len = len;

  return fx_info->usedUserSymbols-1;
}

static void DumpTables (FxInfo * fx_info, FILE * fh)
{

  ssize_t i;
  (void) fx_info;
  for (i=0; i <= rNull; i++) {
    const char * str = "";
    if (                     i < oNull) str = Operators[i].str;
    if (i >= FirstFunc    && i < fNull) str = Functions[i-FirstFunc].str;
    if (i >= FirstImgAttr && i < aNull) str = ImgAttrs[i-FirstImgAttr].str;
    if (i >= FirstSym     && i < sNull) str = Symbols[i-FirstSym].str;
    if (i >= FirstCont    && i < rNull) str = Controls[i-FirstCont].str;
    if      (i==0    ) fprintf (stderr, "Operators:\n ");
    else if (i==oNull) fprintf (stderr, "\nFunctions:\n ");
    else if (i==fNull) fprintf (stderr, "\nImage attributes:\n ");
    else if (i==aNull) fprintf (stderr, "\nSymbols:\n ");
    else if (i==sNull) fprintf (stderr, "\nControls:\n ");
    fprintf (fh, " %s", str);
  }
  fprintf (fh, "\n");
}

static char * NameOfUserSym (FxInfo * fx_info, int ndx, char * buf)
{
  assert (ndx >= 0 && ndx < fx_info->usedUserSymbols);
  UserSymbolT * pus = &fx_info->UserSymbols[ndx];
  CopyMagickString (buf, pus->pex, pus->len+1);
  return buf;
}

static void DumpUserSymbols (FxInfo * fx_info, FILE * fh)
{
  fprintf (fh, "UserSymbols (%i)\n", fx_info->usedUserSymbols);
  char UserSym[MaxTokenLen];
  ssize_t i;
  for (i=0; i < fx_info->usedUserSymbols; i++) {
    fprintf (fh, "  %g: '%s'\n", (double) i, NameOfUserSym (fx_info, i, UserSym));
  }
}

static MagickBooleanType BuildRPN (FxInfo * fx_info)
{
  fx_info->numUserSymbols = InitNumUserSymbols;
  fx_info->usedUserSymbols = 0;
  fx_info->UserSymbols = (UserSymbolT *) AcquireMagickMemory (fx_info->numUserSymbols * sizeof(UserSymbolT));
  if (!fx_info->UserSymbols) ThrowFatalException(ResourceLimitFatalError, "UserSymbols oom");

  fx_info->numElements = RpnInit;
  fx_info->usedElements = 0;
  fx_info->Elements = NULL;

  fx_info->Elements = (ElementT *) AcquireMagickMemory (fx_info->numElements * sizeof(ElementT));
  if (!fx_info->Elements) ThrowFatalException(ResourceLimitFatalError, "Elements oom");

  fx_info->usedOprStack = 0;
  fx_info->maxUsedOprStack = 0;
  fx_info->numOprStack = InitNumOprStack;
  fx_info->OperatorStack = (OperatorE *) AcquireMagickMemory (fx_info->numOprStack * sizeof(OperatorE));
  if (!fx_info->OperatorStack) ThrowFatalException(ResourceLimitFatalError, "OperatorStack oom");

  return MagickTrue;
}

static MagickBooleanType AllocFxRt (FxInfo * fx_info, fxRtT * fx_infort)
{
  fx_infort->random_info = AcquireRandomInfo ();
  fx_infort->thisPixel = NULL;

  int nRnd = 20 + 10 * GetPseudoRandomValue (fx_infort->random_info);
  ssize_t i;
  for (i=0; i < nRnd; i++) GetPseudoRandomValue (fx_infort->random_info);;

  fx_infort->usedValStack = 0;
  fx_infort->numValStack = 2 * fx_info->maxUsedOprStack;
  if (fx_infort->numValStack < MinValStackSize) fx_infort->numValStack = MinValStackSize;
  fx_infort->ValStack = (fxFltType *) AcquireMagickMemory (fx_infort->numValStack * sizeof(fxFltType));
  if (!fx_infort->ValStack) ThrowFatalException(ResourceLimitFatalError, "ValStack oom");

  fx_infort->UserSymVals = NULL;

  if (fx_info->usedUserSymbols) {
    fx_infort->UserSymVals = (fxFltType *) AcquireMagickMemory (fx_info->usedUserSymbols * sizeof(fxFltType));
    if (!fx_infort->UserSymVals) ThrowFatalException(ResourceLimitFatalError, "UserSymVals oom");
    for (i = 0; i < fx_info->usedUserSymbols; i++) fx_infort->UserSymVals[i] = 0;
  }
  return MagickTrue;
}

static MagickBooleanType ExtendRPN (FxInfo * fx_info)
{
  fx_info->numElements = ceil (fx_info->numElements * (1 + TableExtend));
  fx_info->Elements = (ElementT *) ResizeMagickMemory (fx_info->Elements, fx_info->numElements * sizeof(ElementT));
  if (!fx_info->Elements)
    {
      (void) ThrowMagickException(fx_info->exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        fx_info->image->filename);
    return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType OprInPlace (FxInfo * fx_info, int op)
{
  (void) fx_info;
  return (op >= oAddEq && op <= oSubSub ? MagickTrue : MagickFalse);
}

static const char * OprStr (FxInfo * fx_info, int oprNum)
{
  const char
    *str;

  (void) fx_info;
  if      (oprNum < 0) str = "bad OprStr";
  else if (oprNum <= oNull) str = Operators[oprNum].str;
  else if (oprNum <= fNull) str = Functions[oprNum-FirstFunc].str;
  else if (oprNum <= aNull) str = ImgAttrs[oprNum-FirstImgAttr].str;
  else if (oprNum <= sNull) str = Symbols[oprNum-FirstSym].str;
  else if (oprNum <= rNull) str = Controls[oprNum-FirstCont].str;
  else {
    str = "bad OprStr";
  }
  return str;
}

static MagickBooleanType DumpRPN (FxInfo * fx_info, FILE * fh)
{
  fprintf (fh, "DumpRPN:");
  fprintf (fh, "  numElements=%i", fx_info->numElements);
  fprintf (fh, "  usedElements=%i", fx_info->usedElements);
  fprintf (fh, "  maxUsedOprStack=%i", fx_info->maxUsedOprStack);
  fprintf (fh, "  ImgListLen=%i", fx_info->ImgListLen);
  fprintf (fh, "  NeedStats=%s", fx_info->NeedStats ? "yes" : "no");
  fprintf (fh, "  NeedHsl=%s\n", fx_info->NeedHsl ? "yes" : "no");

  ssize_t i;
  for (i=0; i < fx_info->usedElements; i++) {
    ElementT * pel = &fx_info->Elements[i];
    pel->nDest = 0;
  }
  for (i=0; i < fx_info->usedElements; i++) {
    ElementT * pel = &fx_info->Elements[i];
    if (pel->oprNum == rGoto || pel->oprNum == rIfZeroGoto || pel->oprNum == rIfNotZeroGoto) {
      if (pel->EleNdx >= 0 && pel->EleNdx < fx_info->numElements) {
        ElementT * pelDest = &fx_info->Elements[pel->EleNdx];
        pelDest->nDest++;
      }
    }
  }
  for (i=0; i < fx_info->usedElements; i++) {
    ElementT * pel = &fx_info->Elements[i];
    const char * str = OprStr (fx_info, pel->oprNum);
    const char *sRelAbs = "";
    if (pel->oprNum == fP || pel->oprNum == fUP || pel->oprNum == fVP || pel->oprNum == fSP)
      sRelAbs = pel->IsRelative ? "[]" : "{}";

    if (pel->type == etColourConstant)
      fprintf (fh, "  %g: %s vals=%.*Lg,%.*Lg,%.*Lg '%s%s' nArgs=%i ndx=%i  %s",
               (double) i, sElementTypes[pel->type],
               fx_info->precision, pel->val, fx_info->precision, pel->val1, fx_info->precision, pel->val2,
               str, sRelAbs, pel->nArgs, pel->EleNdx,
               pel->DoPush ? "push" : "NO push");
    else
      fprintf (fh, "  %g: %s val=%.*Lg '%s%s' nArgs=%i ndx=%i  %s",
               (double) i, sElementTypes[pel->type], fx_info->precision, pel->val, str, sRelAbs,
               pel->nArgs, pel->EleNdx,
               pel->DoPush ? "push" : "NO push");

    if (pel->ImgAttrQual != aNull)
      fprintf (fh, " ia=%s", OprStr(fx_info, pel->ImgAttrQual));

    if (pel->ChannelQual != NO_CHAN_QUAL) {
      if (pel->ChannelQual == THIS_CHANNEL) fprintf (stderr, "  ch=this");
      else fprintf (stderr, "  ch=%i", pel->ChannelQual);
    }

    char UserSym[MaxTokenLen];

    if (pel->oprNum == rCopyTo) {
      fprintf (fh, "  CopyTo ==> %s", NameOfUserSym (fx_info, pel->EleNdx, UserSym));
    } else if (pel->oprNum == rCopyFrom) {
      fprintf (fh, "  CopyFrom <== %s", NameOfUserSym (fx_info, pel->EleNdx, UserSym));
    } else if (OprInPlace (fx_info, pel->oprNum)) {
      fprintf (fh, "  <==> %s", NameOfUserSym (fx_info, pel->EleNdx, UserSym));
    }
    if (pel->nDest > 0)  fprintf (fh, "  <==dest(%i)", pel->nDest);
    fprintf (fh, "\n");
  }
  return MagickTrue;
}

static void DestroyRPN (FxInfo * fx_info)
{
  fx_info->numOprStack = 0;
  fx_info->usedOprStack = 0;
  if (fx_info->OperatorStack) fx_info->OperatorStack = (OperatorE *) RelinquishMagickMemory (fx_info->OperatorStack);

  fx_info->numElements = 0;
  fx_info->usedElements = 0;
  if (fx_info->Elements) fx_info->Elements = (ElementT *) RelinquishMagickMemory (fx_info->Elements);

  fx_info->usedUserSymbols = 0;
  if (fx_info->UserSymbols) fx_info->UserSymbols = (UserSymbolT *) RelinquishMagickMemory (fx_info->UserSymbols);
}

static void DestroyFxRt (fxRtT * fx_infort)
{
  fx_infort->usedValStack = 0;
  if (fx_infort->ValStack) fx_infort->ValStack = (fxFltType *) RelinquishMagickMemory (fx_infort->ValStack);
  if (fx_infort->UserSymVals) fx_infort->UserSymVals = (fxFltType *) RelinquishMagickMemory (fx_infort->UserSymVals);

  fx_infort->random_info = DestroyRandomInfo (fx_infort->random_info);
}

static size_t GetToken(FxInfo * fx_info)
/* Returns length of token that starts with an alpha,
     or 0 if it isn't a token that starts with an alpha.
   j0 and j1 have trailing digit.
   Also colours like "gray47" have more trailing digits.
   After intial alpha(s) also allow single "_", eg "standard_deviation".
   Does not advance fx_info->pex.
   This splits "mean.r" etc.
*/
{

  *fx_info->token = '\0';
  fx_info->lenToken = 0;
  char * p = fx_info->pex;
  if (!isalpha((int)*p)) return 0;
  size_t len = 0;

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
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "GetToken: too long", "%g at '%s'",
      (double) len, SetShortExp(fx_info));
    len = MaxTokenLen;
  }
  if (len) {
    CopyMagickString (fx_info->token, fx_info->pex, (len+1<MaxTokenLen)?len+1:MaxTokenLen);
  }

  fx_info->lenToken = strlen (fx_info->token);
  return len;
}

static MagickBooleanType TokenMaybeUserSymbol (FxInfo * fx_info)
{
  char * p = fx_info->token;
  ssize_t i = 0;
  while (*p) {
    if (!isalpha ((int)*p++)) return MagickFalse;
    i++;
  }
  if (i < 2) return MagickFalse;
  return MagickTrue;
}

static MagickBooleanType AddElement (FxInfo * fx_info, fxFltType val, int oprNum)
{
  assert (oprNum <= rNull);

  if (++fx_info->usedElements >= fx_info->numElements) {
    if (!ExtendRPN (fx_info)) return MagickFalse;
  }

  ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
  pel->type = TypeOfOpr (fx_info, oprNum);
  pel->val = val;
  pel->val1 = 0;
  pel->val2 = 0;
  pel->oprNum = oprNum;
  pel->DoPush = MagickTrue;
  pel->EleNdx = 0;
  pel->ChannelQual = NO_CHAN_QUAL;
  pel->ImgAttrQual = aNull;
  pel->nDest = 0;
  pel->pExpStart = NULL;
  pel->lenExp = 0;

  if (oprNum <= oNull) pel->nArgs = Operators[oprNum].nArgs;
  else if (oprNum <= fNull) pel->nArgs = Functions[oprNum-FirstFunc].nArgs;
  else if (oprNum <= aNull) pel->nArgs = 0;
  else if (oprNum <= sNull) pel->nArgs = 0;
  else                      pel->nArgs = Controls[oprNum-FirstCont].nArgs;

  return MagickTrue;
}

static MagickBooleanType AddAddressingElement (FxInfo * fx_info, int oprNum, int EleNdx)
{
  if (!AddElement (fx_info, 0, oprNum)) return MagickFalse;
  ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
  pel->EleNdx = EleNdx;
  if (oprNum == rGoto || oprNum == rIfZeroGoto || oprNum == rIfNotZeroGoto 
   || oprNum == rZerStk)
  {
    pel->DoPush = MagickFalse;
  }

  /* Note: for() may or may not need pushing,
     depending on whether the value is needed, eg "for(...)+2" or debug(for(...)).
  */

  return MagickTrue;
}

static MagickBooleanType AddColourElement (FxInfo * fx_info, ElementTypeE type, fxFltType val0, fxFltType val1, fxFltType val2)
{
  (void) type;
  if (!AddElement (fx_info, val0, oNull)) return MagickFalse;
  ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
  pel->val1 = val1;
  pel->val2 = val2;
  pel->type = etColourConstant;
  return MagickTrue;
}

static void inline SkipSpaces (FxInfo * fx_info)
{
  while (isspace ((int)*fx_info->pex)) fx_info->pex++;
}

static char inline PeekChar (FxInfo * fx_info)
{
  SkipSpaces (fx_info);
  return *fx_info->pex;
}

static MagickBooleanType inline PeekStr (FxInfo * fx_info, const char * str)
{
  SkipSpaces (fx_info);
  
  return (LocaleNCompare (fx_info->pex, str, strlen(str))==0 ? MagickTrue : MagickFalse);
}

static MagickBooleanType ExpectChar (FxInfo * fx_info, char c)
{
  if (PeekChar (fx_info) != c) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Expected char", "'%c' at '%s'", c, SetShortExp (fx_info));
    return MagickFalse;
  }
  fx_info->pex++;
  return MagickTrue;
}

static int MaybeXYWH (FxInfo * fx_info, ImgAttrE * pop)
/* If ".x" or ".y" or ".width" or ".height" increments *pop and returns 1 to 4 .
   Otherwise returns 0.
*/
{

  if (*pop != aPage && *pop != aPrintsize && *pop != aRes) return 0;

  if (PeekChar (fx_info) != '.') return 0;

  if (!ExpectChar (fx_info, '.')) return 0;

  int ret=0;
  GetToken (fx_info);
  if (LocaleCompare ("x", fx_info->token)==0) ret=1;
  else if (LocaleCompare ("y", fx_info->token)==0) ret=2;
  else if (LocaleCompare ("width", fx_info->token)==0) ret=3;
  else if (LocaleCompare ("height", fx_info->token)==0) ret=4;

  if (!ret)
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Invalid 'x' or 'y' or 'width' or 'height' token=", "'%s' at '%s'",
      fx_info->token, SetShortExp(fx_info));

  if (*pop == aPage) (*pop) = (ImgAttrE) (*pop+ret);
  else {
    if (ret > 2) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Invalid 'width' or 'height' token=", "'%s' at '%s'",
        fx_info->token, SetShortExp(fx_info));
    } else {
      (*pop) = (ImgAttrE) (*pop+ret);
    }
  }
  fx_info->pex+=fx_info->lenToken;
  return ret;
}

static MagickBooleanType ExtendOperatorStack (FxInfo * fx_info)
{
  fx_info->numOprStack = ceil (fx_info->numOprStack * (1 + TableExtend));
  fx_info->OperatorStack = (OperatorE *) ResizeMagickMemory (fx_info->OperatorStack, fx_info->numOprStack * sizeof(OperatorE));
  if (!fx_info->OperatorStack) {
    (void) ThrowMagickException(fx_info->exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",
      fx_info->image->filename);
    return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType PushOperatorStack (FxInfo * fx_info, int op)
{
  if (++fx_info->usedOprStack >= fx_info->numOprStack) {
    if (!ExtendOperatorStack (fx_info))
      return MagickFalse;
  }
  fx_info->OperatorStack[fx_info->usedOprStack-1] = (OperatorE) op;

  if (fx_info->maxUsedOprStack < fx_info->usedOprStack)
    fx_info->maxUsedOprStack = fx_info->usedOprStack;
  return MagickTrue;
}

static OperatorE GetLeadingOp (FxInfo * fx_info)
{
  OperatorE op = oNull;

  if      (*fx_info->pex == '-') op = oUnaryMinus;
  else if (*fx_info->pex == '+') op = oUnaryPlus;
  else if (*fx_info->pex == '~') op = oBitNot;
  else if (*fx_info->pex == '!') op = oLogNot;
  else if (*fx_info->pex == '(') op = oOpenParen;

  return op;
}

static MagickBooleanType OprIsUnaryPrefix (FxInfo * fx_info, OperatorE op)
{
  (void) fx_info;
  return (op == oUnaryMinus || op == oUnaryPlus || op == oBitNot || op == oLogNot ? MagickTrue : MagickFalse);
}

static MagickBooleanType TopOprIsUnaryPrefix (FxInfo * fx_info)
{
  if (!fx_info->usedOprStack) return MagickFalse;

  return OprIsUnaryPrefix (fx_info, fx_info->OperatorStack[fx_info->usedOprStack-1]);
}

static MagickBooleanType PopOprOpenParen (FxInfo * fx_info, OperatorE op)
{

  if (!fx_info->usedOprStack) return MagickFalse;

  if (fx_info->OperatorStack[fx_info->usedOprStack-1] != op) return MagickFalse;

  fx_info->usedOprStack--;

  return MagickTrue;
}

static MagickBooleanType GetCoordQualifier(FxInfo * fx_info,OperatorE op)
/* Returns -1 if invalid CoordQualifier, +1 if valid and appropriate.
*/
{
  if (op != (OperatorE)fU && op != (OperatorE)fV && op != (OperatorE)fS) return MagickFalse;

  GetToken (fx_info);

  if (fx_info->lenToken != 1) {
    return MagickFalse;
  }
  if (*fx_info->token != 'p' && *fx_info->token != 'P') return MagickFalse;
  if (!GetFunction (fx_info, fP)) return MagickFalse;

  return MagickTrue;
}

static PixelChannel GetChannelQualifier (FxInfo * fx_info, OperatorE op)
{
  if (op == (OperatorE)fU || op == (OperatorE)fV || op == (OperatorE)fP || 
      op == (OperatorE)fUP || op == (OperatorE)fVP ||
      op == (OperatorE)fS || (op >= (OperatorE)FirstImgAttr && op <= (OperatorE)aNull)
     )
  {
    GetToken (fx_info);

    const ChannelT * pch = &Channels[0];
    while (*pch->str) {
      if (LocaleCompare (pch->str, fx_info->token)==0) {

        if (op >= (OperatorE)FirstImgAttr && op <= (OperatorE)aNull &&
              (pch->pixChan == HUE_CHANNEL ||
               pch->pixChan == SAT_CHANNEL ||
               pch->pixChan == LIGHT_CHANNEL)
           )
        {
          ThrowMagickException (
            fx_info->exception, GetMagickModule(), OptionError,
            "Can't have image attribute with HLS qualifier at", "'%s'",
            SetShortExp(fx_info));
          return NO_CHAN_QUAL;
        }

        fx_info->pex += fx_info->lenToken;
        return pch->pixChan;
      }
      pch++;
    }
  }
  return NO_CHAN_QUAL;
}

static ImgAttrE GetImgAttrToken (FxInfo * fx_info)
{
  ImgAttrE ia = aNull;
  const char * iaStr;
  for (ia = FirstImgAttr; ia < aNull; ia=(ImgAttrE) (ia+1)) {
    iaStr = ImgAttrs[ia-FirstImgAttr].str;
    if (LocaleCompare (iaStr, fx_info->token)==0) {
      fx_info->pex += strlen(fx_info->token);
      if (ImgAttrs[ia-FirstImgAttr].NeedStats == 1) fx_info->NeedStats = MagickTrue;
      MaybeXYWH (fx_info, &ia);
      break;
    }
  }

  if (ia == aPage || ia == aPrintsize || ia == aRes) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Attribute", "'%s' needs qualifier at '%s'",
      iaStr, SetShortExp(fx_info));
  }

  return ia;
}

static ImgAttrE GetImgAttrQualifier (FxInfo * fx_info, OperatorE op)
{
  ImgAttrE ia = aNull;
  if (op == (OperatorE)fU || op == (OperatorE)fV || op == (OperatorE)fP || op == (OperatorE)fS) {
    GetToken (fx_info);
    if (fx_info->lenToken == 0) {
      return aNull;
    }
    ia = GetImgAttrToken (fx_info);
  }
  return ia;
}

static MagickBooleanType IsQualifier (FxInfo * fx_info)
{
  if (PeekChar (fx_info) == '.') {
    fx_info->pex++;
    return MagickTrue;
  }
  return MagickFalse;
}

static ssize_t GetProperty(FxInfo * fx_info, fxFltType *val)
/* returns number of character to swallow.
   "-1" means invalid input
   "0" means no relevant input (don't swallow, but not an error)
*/
{

  if (PeekStr (fx_info, "%[")) {

    int level = 0;
    char * p = fx_info->pex + 2;
    while (*p) {
      if (*p == '[') level++;
      else if (*p == ']') {
        if (level == 0) break;
        level--;
      }
      p++;
    }
    if (!*p || level != 0) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "After '%[' expected ']' at", "'%s'",
        SetShortExp(fx_info));
      return -1;
    }

    size_t len = p - fx_info->pex + 1;
    if (len > MaxTokenLen) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Too much text between '%[' and ']' at", "'%s'",
        SetShortExp(fx_info));
      return -1;
    }

    char sProperty [MaxTokenLen];
    CopyMagickString (sProperty, fx_info->pex, len+1);
    sProperty[len] = '\0';

    char * text = InterpretImageProperties (fx_info->image->image_info, fx_info->image,
       sProperty, fx_info->exception);

    if (!text) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Unknown property", "'%s' at '%s'",
        sProperty, SetShortExp(fx_info));
      return -1;
    }

    char * tailptr;
    *val = strtold (text, &tailptr);
    if (text == tailptr) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Property", "'%s' is not a number at '%s'",
        text, SetShortExp(fx_info));
      return -1;
    }

    text = DestroyString(text);
    return ((ssize_t) len);
  }

  return 0;
}

static ssize_t inline GetConstantColour (FxInfo * fx_info, fxFltType *v0, fxFltType *v1, fxFltType *v2)
/* Finds named colour such as "blue" and colorspace function such as "lab(10,20,30)".
   Returns number of characters to swallow.
*/
{
  PixelInfo
    colour;

  ExceptionInfo
    *dummy_exception = AcquireExceptionInfo ();

  char ColSp[MaxTokenLen];
  CopyMagickString (ColSp, fx_info->token, MaxTokenLen);

  char * p = ColSp + fx_info->lenToken - 1;
  if (*p == 'a' || *p == 'A') *p = '\0';

  GetPixelInfo (fx_info->image, &colour);

  /* "gray" is both a colorspace and a named colour. */

  MagickBooleanType IsGray = (LocaleCompare (ColSp, "gray") == 0 ? MagickTrue : MagickFalse);
  MagickBooleanType IsIcc = (LocaleCompare (ColSp, "icc-color") == 0) ? MagickTrue : MagickFalse;
  MagickBooleanType IsDev = (LocaleNCompare (ColSp, "device-", 7) == 0) ? MagickTrue : MagickFalse;

  /* QueryColorCompliance will raise a warning if it isn't a colour, so we discard any exceptions.
  */
  if (!QueryColorCompliance (fx_info->token, AllCompliance, &colour, dummy_exception) || IsGray) {
    int type = ParseCommandOption (MagickColorspaceOptions, MagickFalse, ColSp);
    if (type >= 0 || IsIcc || IsDev) {
      char * q = fx_info->pex + fx_info->lenToken;
      while (isspace ((int)*q)) q++;
      if (*q == '(') {
        while (*q && *q != ')') q++;
        size_t lenfun = q - fx_info->pex + 1;
        if (lenfun > MaxTokenLen) {
          ThrowMagickException (
            fx_info->exception, GetMagickModule(), OptionError,
            "lenfun too long", "'%g' at '%s'",
            (double) lenfun, SetShortExp(fx_info));
          dummy_exception = DestroyExceptionInfo (dummy_exception);
          return 0;
        }
        char sFunc[MaxTokenLen];
        CopyMagickString (sFunc, fx_info->pex, lenfun+1);
        if (QueryColorCompliance (sFunc, AllCompliance, &colour, dummy_exception)) {
          *v0 = colour.red   / QuantumRange;
          *v1 = colour.green / QuantumRange;
          *v2 = colour.blue  / QuantumRange;
          dummy_exception = DestroyExceptionInfo (dummy_exception);
          return lenfun;
        }
      }
    }
    if (!IsGray) {
      dummy_exception = DestroyExceptionInfo (dummy_exception);
      return 0;
    }
  }

  *v0 = colour.red   / QuantumRange;
  *v1 = colour.green / QuantumRange;
  *v2 = colour.blue  / QuantumRange;

  dummy_exception = DestroyExceptionInfo (dummy_exception);

  return (ssize_t) strlen (fx_info->token);
}

static ssize_t inline GetHexColour (FxInfo * fx_info, fxFltType *v0, fxFltType *v1, fxFltType *v2)
/* Returns number of characters to swallow.
   Negative return means it starts with '#', but invalid hex number.
*/
{

  if (*fx_info->pex != '#') return 0;

  /* find end of hex digits. */
  char * p = fx_info->pex + 1;
  while (isxdigit ((int)*p)) p++;
  if (isalpha ((int)*p)) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Bad hex number at", "'%s'",
      SetShortExp(fx_info));
    return -1;
  }

  size_t len = p - fx_info->pex;
  if (len < 1) return 0;
  if (len >= MaxTokenLen) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Hex colour too long at", "'%s'",
      SetShortExp(fx_info));
    return -1;
  }
  CopyMagickString (fx_info->token, fx_info->pex, len+1);

  PixelInfo colour;

  GetPixelInfo (fx_info->image, &colour);

  if (!QueryColorCompliance (fx_info->token, AllCompliance, &colour, fx_info->exception)) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "QueryColorCompliance rejected", "'%s' at '%s'",
      fx_info->token, SetShortExp(fx_info));
    return -1;
  }

  *v0 = colour.red   / QuantumRange;
  *v1 = colour.green / QuantumRange;
  *v2 = colour.blue  / QuantumRange;

  return len;
}

static MagickBooleanType GetFunction (FxInfo * fx_info, FunctionE fe)
{
  /* A function, so get open-parens, n args, close-parens
  */
  const char * funStr = Functions[fe-FirstFunc].str;
  fx_info->pex += fx_info->lenToken;

  int nArgs = Functions[fe-FirstFunc].nArgs;
  char chLimit = ')';
  char expChLimit = ')';
  char *strLimit = (char *) ",)";
  OperatorE pushOp = oOpenParen;

  if (fe == fP) {
    char p = PeekChar (fx_info);
    if (p=='{') {
      ExpectChar (fx_info, '{');
      pushOp = oOpenBrace;
      strLimit = (char *) ",}";
      chLimit = '}';
      expChLimit = '}';
    } else if (p=='[') {
      ExpectChar (fx_info, '[');
      pushOp = oOpenBracket;
      strLimit = (char *) ",]";
      chLimit = ']';
      expChLimit = ']';
    } else {
      nArgs = 0;
      chLimit = ']';
      expChLimit = ']';
    }
  } else if (fe == fU) {
    char p = PeekChar (fx_info);
    if (p=='[') {
      ExpectChar (fx_info, '[');
      pushOp = oOpenBracket;
      strLimit = (char *) ",]";
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
    if (!ExpectChar (fx_info, '(')) return MagickFalse;
  }
  if (!PushOperatorStack (fx_info, pushOp)) return MagickFalse;

  char * pExpStart = fx_info->pex;
  size_t lenExp = 0;

  int FndArgs = 0;
  int ndx0 = NULL_ADDRESS, ndx1 = NULL_ADDRESS, ndx2 = NULL_ADDRESS, ndx3 = NULL_ADDRESS;
  ndx0 = fx_info->usedElements;
  if (fe==fDo) {
    AddAddressingElement (fx_info, rGoto, NULL_ADDRESS); /* address will be ndx1+1 */
  }
  while (nArgs > 0) {
    int FndOne = 0;
    if (TranslateStatementList (fx_info, strLimit, &chLimit)) {
      FndOne = 1;
    } else {
      /* Maybe don't break because other expressions may be not empty. */
      if (!chLimit) break;
      if (fe == fP || fe == fS|| fe == fIf) {
        AddElement (fx_info, 0, oNull);
        FndOne = 1;
      } 
    }

    if (strchr (strLimit, chLimit)==NULL) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "For function", "'%s' expected one of '%s' after expression but found '%c' at '%s'",
        funStr, strLimit, chLimit ? chLimit : ' ', SetShortExp(fx_info));
      return MagickFalse;
    }
    if (FndOne) {
      FndArgs++;
      nArgs--;
    }
    switch (FndArgs) {
      case 1:
        ndx1 = fx_info->usedElements;
        if (fe==fWhile) {
          AddAddressingElement (fx_info, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fDo) {
          AddAddressingElement (fx_info, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fFor) {
          fx_info->Elements[fx_info->usedElements-1].DoPush = MagickFalse;
        } else if (fe==fIf) {
          AddAddressingElement (fx_info, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2 + 1 */
          fx_info->Elements[fx_info->usedElements-1].DoPush = MagickTrue; /* we may need return from if() */
        }
        break;
      case 2:
        ndx2 = fx_info->usedElements;
        if (fe==fWhile) {
          fx_info->Elements[fx_info->usedElements-1].DoPush = MagickFalse;
          AddAddressingElement (fx_info, rGoto, ndx0);
        } else if (fe==fDo) {
          fx_info->Elements[fx_info->usedElements-1].DoPush = MagickFalse;
          AddAddressingElement (fx_info, rGoto, ndx0 + 1);
        } else if (fe==fFor) {
          AddAddressingElement (fx_info, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx3 */
          fx_info->Elements[fx_info->usedElements-1].DoPush = MagickTrue; /* we may need return from for() */
          AddAddressingElement (fx_info, rZerStk, NULL_ADDRESS);
        } else if (fe==fIf) {
          AddAddressingElement (fx_info, rGoto, NULL_ADDRESS); /* address will be ndx3 */
        }
        break;
      case 3:
        if (fe==fFor) {
          fx_info->Elements[fx_info->usedElements-1].DoPush = MagickFalse;
          AddAddressingElement (fx_info, rGoto, ndx1);
        }
        ndx3 = fx_info->usedElements;
        break;
      default:
        break;
    }
    if (chLimit == expChLimit) {
      lenExp = fx_info->pex - pExpStart - 1;
      break;
    }
  } /* end while args of a function */
  if (chLimit && chLimit != expChLimit && chLimit != ',' ) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected '%c', found '%c' at '%s'",
      funStr, expChLimit, chLimit ? chLimit : ' ', SetShortExp(fx_info));
    return MagickFalse;
  }

  if (fe == fP || fe == fS || fe == fU) {
    while (FndArgs < Functions[fe-FirstFunc].nArgs) {
      AddElement (fx_info, 0, oNull);
      FndArgs++;
    }
  }

  if (FndArgs > Functions[fe-FirstFunc].nArgs) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected %i arguments, found '%i' at '%s'",
      funStr, Functions[fe-FirstFunc].nArgs, FndArgs, SetShortExp(fx_info));
    return MagickFalse;
  }
  if (FndArgs < Functions[fe-FirstFunc].nArgs) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected %i arguments, found too few (%i) at '%s'",
      funStr, Functions[fe-FirstFunc].nArgs, FndArgs, SetShortExp(fx_info));
    return MagickFalse;
  }
  if (fe != fS && fe != fV && FndArgs == 0 && Functions[fe-FirstFunc].nArgs == 0) {
    /* This is for "rand()" and similar. */
    chLimit = expChLimit;
    if (!ExpectChar (fx_info, ')')) return MagickFalse;
  }

  if (chLimit != expChLimit) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "For function", "'%s', arguments don't end with '%c' at '%s'",
      funStr, expChLimit, SetShortExp(fx_info));
    return MagickFalse;
  }
  if (!PopOprOpenParen (fx_info, pushOp)) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Bug: For function", "'%s' tos not '%s' at '%s'",
      funStr, Operators[pushOp].str, SetShortExp(fx_info));
    return MagickFalse;
  }

  MagickBooleanType coordQual = MagickFalse;
  PixelChannel chQual = NO_CHAN_QUAL;
  ImgAttrE iaQual = aNull;

  if (IsQualifier (fx_info)) {

    if (fe == fU || fe == fV || fe == fS) {

      coordQual = GetCoordQualifier (fx_info, (OperatorE) fe);

      if (coordQual) {

        /* Remove last element, which should be fP */
        ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
        if (pel->oprNum != fP) {
          ThrowMagickException (
            fx_info->exception, GetMagickModule(), OptionError,
            "Bug: For function", "'%s' last element not 'p' at '%s'",
            funStr, SetShortExp(fx_info));
          return MagickFalse;
        }
        chQual = pel->ChannelQual;
        expChLimit = (pel->IsRelative) ? ']' : '}';
        fx_info->usedElements--;
        if (fe == fU) fe = fUP;
        else if (fe == fV) fe = fVP;
        else if (fe == fS) fe = fSP;
        funStr = Functions[fe-FirstFunc].str;
      }
    }

    if ( chQual == NO_CHAN_QUAL &&
         (fe == fP || fe == fS || fe == fSP || fe == fU || fe == fUP || fe == fV || fe == fVP)
       )
    {
      chQual = GetChannelQualifier (fx_info, (OperatorE) fe);
    }

    if (chQual == NO_CHAN_QUAL && (fe == fU || fe == fV || fe == fS)) {
      /* Note: we don't allow "p.mean" etc. */
      iaQual = GetImgAttrQualifier (fx_info, (OperatorE) fe);
    }
    if (IsQualifier (fx_info) && chQual == NO_CHAN_QUAL && iaQual != aNull) {
      chQual = GetChannelQualifier (fx_info, (OperatorE) fe);
    }
    if (coordQual && iaQual != aNull) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "For function", "'%s', can't have qualifiers 'p' and image attribute '%s' at '%s'",
        funStr, fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }
    if (!coordQual && chQual == NO_CHAN_QUAL && iaQual == aNull) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "For function", "'%s', bad qualifier '%s' at '%s'",
        funStr, fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }
    if (!coordQual && chQual == CompositePixelChannel && iaQual == aNull) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "For function", "'%s', bad compsite qualifier '%s' at '%s'",
        funStr, fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }

    if (chQual == HUE_CHANNEL || chQual == SAT_CHANNEL || chQual == LIGHT_CHANNEL) {
      fx_info->NeedHsl = MagickTrue;

      if (iaQual >= FirstImgAttr && iaQual < aNull) {
        ThrowMagickException (
          fx_info->exception, GetMagickModule(), OptionError,
          "Can't have image attribute with HLS qualifier at", "'%s'",
          SetShortExp(fx_info));
        return MagickFalse;
      }
    }
  }

  if (fe==fWhile) {
    fx_info->Elements[ndx1].EleNdx = ndx2+1;
  } else if (fe==fDo) {
    fx_info->Elements[ndx0].EleNdx = ndx1+1;
    fx_info->Elements[ndx1].EleNdx = ndx2+1;
  } else if (fe==fFor) {
    fx_info->Elements[ndx2].EleNdx = ndx3;
  } else if (fe==fIf) {
    fx_info->Elements[ndx1].EleNdx = ndx2 + 1;
    fx_info->Elements[ndx2].EleNdx = ndx3;
  } else {
    if (fe == fU && iaQual == aNull) {
      ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
      if (pel->type == etConstant && pel->val == 0.0) {
        fx_info->usedElements--;
        fe = fU0;
      }
    }
    AddElement (fx_info, 0, fe);
    if (fe == fP || fe == fU  || fe == fU0 || fe == fUP ||
        fe == fV || fe == fVP || fe == fS || fe == fSP)
    {
      ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
      pel->IsRelative = (expChLimit == ']') ? MagickTrue : MagickFalse;
      if (chQual >= 0) pel->ChannelQual = chQual;
      if (iaQual != aNull && (fe == fU || fe == fV || fe == fS)) {
        /* Note: we don't allow "p[2,3].mean" or "p.mean" etc. */
        pel->ImgAttrQual = iaQual;
      }
    }
  }

  if (pExpStart && lenExp) {
    ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
    pel->pExpStart = pExpStart;
    pel->lenExp = lenExp;
  }

  if (fe == fDebug)
    fx_info->ContainsDebug = MagickTrue;

  return MagickTrue;
}

static MagickBooleanType IsStealth (int op)
{
  return (op == fU0 || op == fUP || op == fSP || op == fVP ||
           (op >= FirstCont && op <= rNull)
         ) ? MagickTrue : MagickFalse;
}

static MagickBooleanType GetOperand (
  FxInfo * fx_info, MagickBooleanType * UserSymbol, MagickBooleanType * NewUserSymbol, int * UserSymNdx,
  MagickBooleanType * needPopAll)
{

  *NewUserSymbol = *UserSymbol = MagickFalse;
  *UserSymNdx = NULL_ADDRESS;

  SkipSpaces (fx_info);
  if (!*fx_info->pex) return MagickFalse;
  GetToken (fx_info);

  if (fx_info->lenToken==0) {

    /* Try '(' or unary prefix
    */
    OperatorE op = GetLeadingOp (fx_info);
    if (op==oOpenParen) {
      if (!PushOperatorStack (fx_info, op)) return MagickFalse;
      fx_info->pex++;
      char chLimit = '\0';
      if (!TranslateExpression (fx_info, (char *) ")", &chLimit, needPopAll)) {
        ThrowMagickException (
          fx_info->exception, GetMagickModule(), OptionError,
          "Empty expression in parentheses at", "'%s'",
          SetShortExp(fx_info));
      }
      if (chLimit != ')') {
        ThrowMagickException (
          fx_info->exception, GetMagickModule(), OptionError,
          "'(' but no ')' at", "'%s'",
          SetShortExp(fx_info));
        return MagickFalse;
      }
      /* Top of opr stack should be '('. */
      if (!PopOprOpenParen (fx_info, oOpenParen)) {
        ThrowMagickException (
          fx_info->exception, GetMagickModule(), OptionError,
          "Bug: tos not '(' at", "'%s'",
          SetShortExp(fx_info));
        return MagickFalse;
      }
      return MagickTrue;
    } else if (OprIsUnaryPrefix (fx_info, op)) {
      if (!PushOperatorStack (fx_info, op)) return MagickFalse;
      fx_info->pex++;
      SkipSpaces (fx_info);
      if (!*fx_info->pex) return MagickFalse;

      if (!GetOperand (fx_info, UserSymbol, NewUserSymbol, UserSymNdx, needPopAll)) {
        ThrowMagickException (
          fx_info->exception, GetMagickModule(), OptionError,
          "After unary, bad operand at", "'%s'",
          SetShortExp(fx_info));
        return MagickFalse;
      }

      if (*NewUserSymbol) {
        ThrowMagickException (
          fx_info->exception, GetMagickModule(), OptionError,
          "After unary, NewUserSymbol at", "'%s'",
          SetShortExp(fx_info));
        return MagickFalse;
      }

      if (*UserSymbol) {
        AddAddressingElement (fx_info, rCopyFrom, *UserSymNdx);
        *UserSymNdx = NULL_ADDRESS;

        *UserSymbol = MagickFalse;
        *NewUserSymbol = MagickFalse;
      }

      GetToken (fx_info);
      return MagickTrue;
    } else if (*fx_info->pex == '#') {
      fxFltType v0=0, v1=0, v2=0;
      size_t lenToken = GetHexColour (fx_info, &v0, &v1, &v2);
      if (lenToken < 0) {
        ThrowMagickException (
          fx_info->exception, GetMagickModule(), OptionError,
          "Bad hex number at", "'%s'",
          SetShortExp(fx_info));
        return MagickFalse;
      } else if (lenToken > 0) {
        AddColourElement (fx_info, etColourConstant, v0, v1, v2);
        fx_info->pex+=lenToken;
      }
      return MagickTrue;
    }

    /* Try a constant number.
    */
    char * tailptr;
    fxFltType val = strtold (fx_info->pex, &tailptr);
    if (fx_info->pex != tailptr) {
      fx_info->pex = tailptr;
      if (*tailptr) {
        /* Could have "prefix" K, Ki, M etc.
           See https://en.wikipedia.org/wiki/Metric_prefix
           and https://en.wikipedia.org/wiki/Binary_prefix
        */
        int Pow = 0.0;
        const char Prefices[] = "yzafpnum.kMGTPEZY";
        const char * pSi = strchr (Prefices, *tailptr);
        if (pSi && *pSi != '.') Pow = (pSi - Prefices) * 3 - 24;
        else if (*tailptr == 'c') Pow = -2;
        else if (*tailptr == 'h') Pow =  2;
        else if (*tailptr == 'k') Pow =  3;
        if (Pow != 0.0) {
          if (*(++fx_info->pex) == 'i') {
            val *= pow (2.0, Pow/0.3);
            fx_info->pex++;
          } else {
            val *= pow (10.0, Pow);
          }
        }
      }
      AddElement (fx_info, val, oNull);
      return MagickTrue;
    }

    val = 0;
    ssize_t lenOptArt = GetProperty (fx_info, &val);
    if (lenOptArt < 0) return MagickFalse;
    if (lenOptArt > 0) {
      AddElement (fx_info, val, oNull);
      fx_info->pex += lenOptArt;
      return MagickTrue;
    }

  } /* end of lenToken==0 */

  if (fx_info->lenToken > 0) {
    ConstantE ce;
    for (ce = (ConstantE)0; ce < cNull; ce=(ConstantE) (ce+1)) {
      const char * ceStr = Constants[ce].str;
      if (LocaleCompare (ceStr, fx_info->token)==0) {
        break;
      }
    }

    if (ce != cNull) {
      AddElement (fx_info, Constants[ce].val, oNull);
      fx_info->pex += fx_info->lenToken;
      return MagickTrue;
    }

    FunctionE fe;
    for (fe = FirstFunc; fe < fNull; fe=(FunctionE) (fe+1)) {
      const char * feStr = Functions[fe-FirstFunc].str;
      if (LocaleCompare (feStr, fx_info->token)==0) {
        break;
      }
    }

    if (fe == fV && fx_info->ImgListLen < 2) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Symbol 'v' but fewer than two images at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }

    if (IsStealth (fe)) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Function", "'%s' not permitted at '%s'",
        fx_info->token, SetShortExp(fx_info));
    }

    if (fe == fDo || fe == fFor || fe == fIf || fe == fWhile) {
      *needPopAll = MagickTrue;
    }

    if (fe != fNull) return (GetFunction (fx_info, fe));

    /* Try image attribute
    */
    ImgAttrE ia = GetImgAttrToken (fx_info);
    if (ia != aNull) {
      fxFltType val = 0;
      AddElement (fx_info, val, ia);

      if (ImgAttrs[ia-FirstImgAttr].NeedStats==1) {
        if (IsQualifier (fx_info)) {
          PixelChannel chQual = GetChannelQualifier (fx_info, (OperatorE) ia);
          if (chQual == NO_CHAN_QUAL) {
            ThrowMagickException (
              fx_info->exception, GetMagickModule(), OptionError,
              "Bad channel qualifier at", "'%s'",
              SetShortExp(fx_info));
            return MagickFalse;
          }
          /* Adjust the element */
          ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
          pel->ChannelQual = chQual;
        }
      }
      return MagickTrue;
    }

    /* Try symbol
    */
    SymbolE se;
    for (se = FirstSym; se < sNull; se=(SymbolE) (se+1)) {
      const char * seStr = Symbols[se-FirstSym].str;
      if (LocaleCompare (seStr, fx_info->token)==0) {
        break;
      }
    }
    if (se != sNull) {
      fxFltType val = 0;
      AddElement (fx_info, val, se);
      fx_info->pex += fx_info->lenToken;

      if (se==sHue || se==sSaturation || se==sLightness) fx_info->NeedHsl = MagickTrue;
      return MagickTrue;
    }

    /* Try constant colour.
    */
    fxFltType v0, v1, v2;
    int ColLen = GetConstantColour (fx_info, &v0, &v1, &v2);
    if (ColLen > 0) {
      AddColourElement (fx_info, etColourConstant, v0, v1, v2);
      fx_info->pex+=ColLen;
      return MagickTrue;
    }

    /* Try user symbols. If it is, don't AddElement yet.
    */
    if (TokenMaybeUserSymbol (fx_info)) {
      *UserSymbol = MagickTrue;
      *UserSymNdx = FindUserSymbol (fx_info, fx_info->token);
      if (*UserSymNdx == NULL_ADDRESS) {
        *UserSymNdx = AddUserSymbol (fx_info, fx_info->pex, fx_info->lenToken); /* so future "CopyFrom" and "CopyTo" works. */
        *NewUserSymbol = MagickTrue;
      } else {
      }
      fx_info->pex += fx_info->lenToken;

      return MagickTrue;
    }
  }

  ThrowMagickException (
    fx_info->exception, GetMagickModule(), OptionError,
    "Expected operand at", "'%s'",
    SetShortExp(fx_info));

  return MagickFalse;
}

static MagickBooleanType inline IsRealOperator (OperatorE op)
{
  return (op < oOpenParen || op > oCloseBrace ? MagickTrue : MagickFalse);
}

static MagickBooleanType inline ProcessTernaryOpr (FxInfo * fx_info, TernaryT * ptern)
/* Ternary operator "... ? ... : ..."
   returns false if we have exception
*/
{

  if (fx_info->OperatorStack[fx_info->usedOprStack-1] == oQuery) {
    if (ptern->addrQuery != NULL_ADDRESS) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Already have '?' in sub-expression at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }
    fx_info->usedOprStack--;
    ptern->addrQuery = fx_info->usedElements;
    AddAddressingElement (fx_info, rIfZeroGoto, NULL_ADDRESS);
    /* address will be one after the Colon address. */
  }
  if (fx_info->OperatorStack[fx_info->usedOprStack-1] == oColon) {
    if (ptern->addrQuery == NULL_ADDRESS) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Need '?' in sub-expression at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }
    fx_info->usedOprStack--;
    ptern->addrColon = fx_info->usedElements;
    fx_info->Elements[fx_info->usedElements-1].DoPush = MagickTrue;
    AddAddressingElement (fx_info, rGoto, NULL_ADDRESS);
    /* address will be after the subexpression */
  }
  return MagickTrue;
}

static MagickBooleanType GetOperator (
  FxInfo * fx_info,
  MagickBooleanType * Assign, MagickBooleanType * Update, MagickBooleanType * IncrDecr,
  TernaryT * ptern)
{
  (void) ptern;
  SkipSpaces (fx_info);

  OperatorE op;
  size_t len = 0;
  for (op = (OperatorE)0; op != oNull; op=(OperatorE) (op+1)) {
    const char * opStr = Operators[op].str;
    len = strlen(opStr);
    if (LocaleNCompare (opStr, fx_info->pex, len)==0) {
      break;
    }
  }

  if (!IsRealOperator (op)) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Not a real operator at", "'%s'",
      SetShortExp(fx_info));
    return MagickFalse;
  }

  if (op==oNull) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Expected operator at", "'%s'",
      SetShortExp(fx_info));
    return MagickFalse;
  }

  *Assign = (op==oAssign) ? MagickTrue : MagickFalse;
  *Update = OprInPlace (fx_info, op);
  *IncrDecr = (op == oPlusPlus || op == oSubSub) ? MagickTrue : MagickFalse;

  /* while top of OperatorStack is not empty and is not open-parens or assign,
       and top of OperatorStack is higher precedence than new op,
     then move top of OperatorStack to Element list.
  */

  while (fx_info->usedOprStack > 0) {
    OperatorE top = fx_info->OperatorStack[fx_info->usedOprStack-1];
    if (top == oOpenParen || top == oAssign || OprInPlace (fx_info, top)) break;
    int precTop = Operators[top].precedence;
    int precNew = Operators[op].precedence;
    /* Assume left associativity.
       If right assoc, this would be "<=".
    */
    if (precTop < precNew) break;
    AddElement (fx_info, 0, top);
    fx_info->usedOprStack--;
  }

  MagickBooleanType DoneIt = MagickFalse;

  /* If new op is close paren, and stack top is open paren,
     remove stack top.
  */
  if (op==oCloseParen) {
    if (fx_info->usedOprStack == 0) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Found ')' but nothing on stack at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }

    if (fx_info->OperatorStack[fx_info->usedOprStack-1] != oOpenParen) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Found ')' but no '(' on stack at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }
    fx_info->usedOprStack--;
    DoneIt = MagickTrue;
  }

  if (!DoneIt) {
    if (!PushOperatorStack (fx_info, op)) return MagickFalse;
  }

  fx_info->pex += len;

  return MagickTrue;
}

static MagickBooleanType ResolveTernaryAddresses (FxInfo * fx_info, TernaryT * ptern)
{
  if (ptern->addrQuery == NULL_ADDRESS && ptern->addrColon == NULL_ADDRESS)
    return MagickTrue;

  if (ptern->addrQuery != NULL_ADDRESS && ptern->addrColon != NULL_ADDRESS) {
    fx_info->Elements[ptern->addrQuery].EleNdx = ptern->addrColon + 1;
    fx_info->Elements[ptern->addrColon].EleNdx = fx_info->usedElements;
    ptern->addrQuery = NULL_ADDRESS;
    ptern->addrColon = NULL_ADDRESS;
  } else if (ptern->addrQuery != NULL_ADDRESS) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "'?' with no corresponding ':'", "'%s' at '%s'",
        fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
  } else if (ptern->addrColon != NULL_ADDRESS) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "':' with no corresponding '?'", "'%s' at '%s'",
        fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType TranslateExpression (
  FxInfo * fx_info, char * strLimit, char * chLimit, MagickBooleanType * needPopAll)
{
  fx_info->teDepth++;

  *chLimit = '\0';

  TernaryT ternary;
  ternary.addrQuery = NULL_ADDRESS;
  ternary.addrColon = NULL_ADDRESS;

  int StartEleNdx = fx_info->usedElements-1;
  if (StartEleNdx < 0) StartEleNdx = 0;

  SkipSpaces (fx_info);

  if (!*fx_info->pex) {
    fx_info->teDepth--;
    return MagickFalse;
  }

  if (strchr(strLimit,*fx_info->pex)!=NULL) {
    *chLimit = *fx_info->pex;
    fx_info->pex++;
    fx_info->teDepth--;

    return MagickFalse;
  }

  /* There should be only one New per expression (oAssign), but can be many Old.
  */
  MagickBooleanType UserSymbol, NewUserSymbol;
  int UserSymNdx0, UserSymNdx1;

  if (!GetOperand (fx_info, &UserSymbol, &NewUserSymbol, &UserSymNdx0, needPopAll)) return MagickFalse;
  SkipSpaces (fx_info);

  MagickBooleanType
    Assign = MagickFalse,
    Update = MagickFalse,
    IncrDecr = MagickFalse;

  /* Loop through Operator, Operand, Operator, Operand, ...
  */
  while (*fx_info->pex && (!*strLimit || (strchr(strLimit,*fx_info->pex)==NULL))) {
    if (!GetOperator (fx_info, &Assign, &Update, &IncrDecr, &ternary)) return MagickFalse;
    SkipSpaces (fx_info);
    if (NewUserSymbol && !Assign) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Expected assignment after new UserSymbol", "'%s' at '%s'",
        fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }
    if (!UserSymbol && Assign) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Attempted assignment to non-UserSymbol", "'%s' at '%s'",
        fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }
    if (!UserSymbol && Update) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Attempted update to non-UserSymbol", "'%s' at '%s'",
        fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }
    if (UserSymbol && (Assign || Update) && !IncrDecr) {

      if (!TranslateExpression (fx_info, strLimit, chLimit, needPopAll)) return MagickFalse;
      if (!*fx_info->pex) break;
      if (!*strLimit) break;
      if (strchr(strLimit,*chLimit)!=NULL) break;
    }
    if (UserSymbol && !Assign && !Update && UserSymNdx0 != NULL_ADDRESS) {
      AddAddressingElement (fx_info, rCopyFrom, UserSymNdx0);
      UserSymNdx0 = NULL_ADDRESS;
      ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
      pel->DoPush = MagickTrue;
    }

    if (UserSymbol) {
      while (TopOprIsUnaryPrefix (fx_info)) {
        OperatorE op = fx_info->OperatorStack[fx_info->usedOprStack-1];
        AddElement (fx_info, 0, op);
        fx_info->usedOprStack--;
      }
    }

    if (!ProcessTernaryOpr (fx_info, &ternary)) return MagickFalse;

    if (ternary.addrColon != NULL_ADDRESS) {
      if (!TranslateExpression (fx_info, (char *) ",);", chLimit, needPopAll)) return MagickFalse;
      break;
    }

    UserSymbol = NewUserSymbol = MagickFalse;

    if (!*fx_info->pex) break;
    if (*strLimit && (strchr(strLimit,*fx_info->pex)!=NULL) ) break;

    if (IncrDecr) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "'++' and '--' must be the final operators in an expression at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }

    if (!GetOperand (fx_info, &UserSymbol, &NewUserSymbol, &UserSymNdx1, needPopAll)) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Expected operand at", "'%s'",
        SetShortExp(fx_info));
      return MagickFalse;
    }
    SkipSpaces (fx_info);
    if (NewUserSymbol && !Assign) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "NewUserSymbol", "'%s' after non-assignment operator at '%s'",
        fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }
    if (UserSymbol && !NewUserSymbol) {
      AddAddressingElement (fx_info, rCopyFrom, UserSymNdx1);
      UserSymNdx1 = NULL_ADDRESS;
    }
    UserSymNdx0 = UserSymNdx1;
  }

  if (UserSymbol && !Assign && !Update && UserSymNdx0 != NULL_ADDRESS) {
    if (NewUserSymbol) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "NewUserSymbol", "'%s' needs assignment operator at '%s'",
        fx_info->token, SetShortExp(fx_info));
      return MagickFalse;
    }
    AddAddressingElement (fx_info, rCopyFrom, UserSymNdx0);
    ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
    pel->DoPush = MagickTrue;
  }

  if (*fx_info->pex && !*chLimit && (strchr(strLimit,*fx_info->pex)!=NULL)) {
    *chLimit = *fx_info->pex;
    fx_info->pex++;
  }
  while (fx_info->usedOprStack) {
    OperatorE op = fx_info->OperatorStack[fx_info->usedOprStack-1];
    if (op == oOpenParen || op == oOpenBracket || op == oOpenBrace) {
      break;
    }
    if ( (op==oAssign && !Assign) || (OprInPlace(fx_info, op) && !Update) ) {
      break;
    }
    fx_info->usedOprStack--;
    AddElement (fx_info, 0, op);
    if (op == oAssign) {
      /* Adjust last element, by deletion and add.
      */
      fx_info->usedElements--;
      int addr = UserSymNdx0;
      AddAddressingElement (fx_info, rCopyTo, addr);
      break;
    } else if (OprInPlace (fx_info, op)) {
      int addr = UserSymNdx0;
      /* Modify latest element.
      */
      fx_info->Elements[fx_info->usedElements-1].EleNdx = addr;
      break;
    }
  }

  ResolveTernaryAddresses (fx_info, &ternary);

  fx_info->teDepth--;

  if (!fx_info->teDepth && *needPopAll) {
    AddAddressingElement (fx_info, rZerStk, NULL_ADDRESS);
    *needPopAll = MagickFalse;
  }

  return MagickTrue;
}


static MagickBooleanType TranslateStatement (FxInfo * fx_info, char * strLimit, char * chLimit)
{
  SkipSpaces (fx_info);

  if (!*fx_info->pex) return MagickFalse;

  MagickBooleanType NeedPopAll = MagickFalse;

  if (!TranslateExpression (fx_info, strLimit, chLimit, &NeedPopAll)) {
    return MagickFalse;
  }

  if (fx_info->usedElements && *chLimit==';') {
    /* FIXME: not necessarily the last element,
       but the last _executed_ element, eg "goto" in a "for()"., 
       Pending a fix, we will use rZerStk.
    */
    ElementT * pel = &fx_info->Elements[fx_info->usedElements-1];
    if (pel->DoPush) pel->DoPush = MagickFalse;
  }

  return MagickTrue;
}

static MagickBooleanType TranslateStatementList (FxInfo * fx_info, char * strLimit, char * chLimit)
{
  SkipSpaces (fx_info);

  if (!*fx_info->pex) return MagickFalse;

#define MAX_SLIMIT 10
  char sLimits[MAX_SLIMIT];
  CopyMagickString (sLimits, strLimit, MAX_SLIMIT-1);

  if (strchr(strLimit,';')==NULL)
    ConcatenateMagickString (sLimits, ";", MAX_SLIMIT);

  for (;;) {
    if (!TranslateStatement (fx_info, sLimits, chLimit)) return MagickFalse;

    if (!*fx_info->pex) break;

    if (*chLimit != ';') {
      break;
    }
  }

  return MagickTrue;
}

/*--------------------------------------------------------------------
   Run-time
*/

static MagickBooleanType CollectStatistics (FxInfo * fx_info)
{

  fx_info->statistics = (ChannelStatistics **) AcquireMagickMemory (fx_info->ImgListLen * sizeof (ChannelStatistics *));
  if (!fx_info->statistics) {
    (void) ThrowMagickException(fx_info->exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",
      fx_info->image->filename);
    return MagickFalse;
  }

  Image * img = GetFirstImageInList (fx_info->image);

  ssize_t imgNum=0;
  for (;;) {
    ChannelStatistics * cs = GetImageStatistics (img, fx_info->exception);
    fx_info->statistics[imgNum] = cs;
    int ch;
    for (ch=0; ch <= (ssize_t) MaxPixelChannels; ch++) {
      cs[ch].mean *= QuantumScale;
      cs[ch].median *= QuantumScale;
      cs[ch].maxima *= QuantumScale;
      cs[ch].minima *= QuantumScale;
      cs[ch].standard_deviation *= QuantumScale;
      cs[ch].kurtosis *= QuantumScale;
      cs[ch].skewness *= QuantumScale;
      cs[ch].entropy *= QuantumScale;
    }

    if (++imgNum == fx_info->ImgListLen) break;
    img = GetNextImageInList (img);
    assert (img);
  }
  return MagickTrue;
}

static MagickBooleanType inline PushVal (FxInfo * fx_info, fxRtT * fx_infort, fxFltType val, int addr)
{
  if (fx_infort->usedValStack >=fx_infort->numValStack) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "ValStack overflow at addr=", "%i",
      addr);
    return MagickFalse;
  }

  fx_infort->ValStack[fx_infort->usedValStack++] = val;
  return MagickTrue;
}

static inline fxFltType PopVal (FxInfo * fx_info, fxRtT * fx_infort, int addr)
{
  if (fx_infort->usedValStack <= 0) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "ValStack underflow at addr=", "%i",
      addr);
    return 0;
  }

  return fx_infort->ValStack[--fx_infort->usedValStack];
}

static fxFltType inline ImageStat (
  FxInfo * fx_info, int ImgNum, PixelChannel channel, ImgAttrE ia)
{
  assert (channel >= 0 && channel <= MaxPixelChannels);

  ChannelStatistics * cs = NULL;

  if (fx_info->NeedStats) cs = fx_info->statistics[ImgNum];

  switch (ia) {
    case aDepth:
      return GetImageDepth (fx_info->Images[ImgNum], fx_info->exception);
      break;
    case aExtent:
      return GetBlobSize (fx_info->image);
      break;
    case aKurtosis:
      return cs[channel].kurtosis;
      break;
    case aMaxima:
      return cs[channel].maxima;
      break;
    case aMean:
      return cs[channel].mean;
      break;
    case aMedian:
      return cs[channel].median;
      break;
    case aMinima:
      return cs[channel].minima;
      break;
    case aPage:
      /* Do nothing */
      break;
    case aPageWid:
      return fx_info->Images[ImgNum]->page.width;
    case aPageHt:
      return fx_info->Images[ImgNum]->page.height;
    case aPageX:
      return fx_info->Images[ImgNum]->page.x;
    case aPageY:
      return fx_info->Images[ImgNum]->page.y;
    case aPrintsize:
      /* Do nothing */
      break;
    case aPrintsizeX:
      return PerceptibleReciprocal (fx_info->Images[ImgNum]->resolution.x) * fx_info->Images[ImgNum]->columns;
    case aPrintsizeY:
      return PerceptibleReciprocal (fx_info->Images[ImgNum]->resolution.y) * fx_info->Images[ImgNum]->rows;
    case aQuality:
      return fx_info->Images[ImgNum]->quality;
    case aRes:
      /* Do nothing */
      break;
    case aResX:
      return fx_info->Images[ImgNum]->resolution.x;
    case aResY:
      return fx_info->Images[ImgNum]->resolution.y;
    case aSkewness:
      return cs[channel].skewness;
    case aStdDev:
      return cs[channel].standard_deviation;
    case aH:
      return fx_info->Images[ImgNum]->rows;
    case aN:
      return fx_info->ImgListLen;
    case aT: /* image index in list */
      return ImgNum;
    case aW:
      return fx_info->Images[ImgNum]->columns;
    case aZ:
      return GetImageDepth (fx_info->Images[ImgNum], fx_info->exception);
      break;
    default:
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Unknown ia=", "%i",
        ia);
  }
  return -99;
}

static MagickOffsetType inline FxGcd(MagickOffsetType x, MagickOffsetType y)
{
  if (y != 0) return (FxGcd (y, x % y));
  return (x);
}

static ssize_t inline ChkImgNum (FxInfo * fx_info, fxFltType f)
{
  ssize_t i = floor (f + 0.5);
  if (i < 0) i += fx_info->ImgListLen;
  if (i < 0 || i >= fx_info->ImgListLen) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "ImgNum", "%g bad for ImgListLen %g",
      (double) i, (double) fx_info->ImgListLen);
    
  }
  return i;
}

#define WHICH_ATTR_CHAN \
  (pel->ChannelQual == NO_CHAN_QUAL) ? CompositePixelChannel : \
  (pel->ChannelQual == THIS_CHANNEL) ? channel : pel->ChannelQual

#define WHICH_NON_ATTR_CHAN \
  (pel->ChannelQual == NO_CHAN_QUAL || \
   pel->ChannelQual == THIS_CHANNEL || \
   pel->ChannelQual == CompositePixelChannel \
  ) ? (channel == CompositePixelChannel ? RedPixelChannel: channel) \
    : pel->ChannelQual

static fxFltType GetHslFlt (FxInfo * fx_info, int ImgNum, const fxFltType fx, const fxFltType fy,
  int channel)
{
  Image * img = fx_info->Images[ImgNum];

  double red, green, blue;

  MagickBooleanType okay = MagickTrue;
  if(!InterpolatePixelChannel (img, fx_info->Views[ImgNum], RedPixelChannel, img->interpolate,
    fx, fy, &red, fx_info->exception)) okay = MagickFalse;
  if(!InterpolatePixelChannel (img, fx_info->Views[ImgNum], GreenPixelChannel, img->interpolate,
    fx, fy, &green, fx_info->exception)) okay = MagickFalse;
  if(!InterpolatePixelChannel (img, fx_info->Views[ImgNum], BluePixelChannel, img->interpolate,
    fx, fy, &blue, fx_info->exception)) okay = MagickFalse;

  if (!okay)
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "GetHslFlt failure", "%i %Lg,%Lg %i", ImgNum, fx, fy, channel);

  double hue=0, saturation=0, lightness=0;

  ConvertRGBToHSL (
    red, green, blue,
    &hue, &saturation, &lightness);

  if (channel == HUE_CHANNEL)   return hue;
  if (channel == SAT_CHANNEL)   return saturation;
  if (channel == LIGHT_CHANNEL) return lightness;

  return 0;
}

static fxFltType GetHslInt (FxInfo * fx_info, int ImgNum, const ssize_t imgx, const ssize_t imgy, int channel)
{
  Image * img = fx_info->Images[ImgNum];

  const Quantum * p = GetCacheViewVirtualPixels (fx_info->Views[ImgNum], imgx, imgy, 1, 1, fx_info->exception);
  if (!p)
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "GetHslInt failure", "%i %g,%g %i", ImgNum, (double) imgx, (double) imgy, channel);

  double hue=0, saturation=0, lightness=0;

  ConvertRGBToHSL (
    GetPixelRed (img, p), GetPixelGreen (img, p), GetPixelBlue (img, p),
    &hue, &saturation, &lightness);

  if (channel == HUE_CHANNEL)   return hue;
  if (channel == SAT_CHANNEL)   return saturation;
  if (channel == LIGHT_CHANNEL) return lightness;

  return 0;
}

static fxFltType inline GetIntensity (FxInfo * fx_info, int ImgNum, const ssize_t imgx, const ssize_t imgy)
{
  Quantum
    quantum_pixel[MaxPixelChannels];

  PixelInfo
    pixelinf;

  Image * img = fx_info->Images[ImgNum];

  GetPixelInfo (img, &pixelinf);

  if (!InterpolatePixelInfo (img, fx_info->Views[fx_info->ImgNum], img->interpolate,
              imgx, imgy, &pixelinf, fx_info->exception))
  {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "GetIntensity failure", "%i %g,%g", ImgNum, (double) imgx, (double) imgy);
  }

  SetPixelViaPixelInfo (img, &pixelinf, quantum_pixel);
  return QuantumScale * GetPixelIntensity (img, quantum_pixel);
}

static MagickBooleanType ExecuteRPN (FxInfo * fx_info, fxRtT * fx_infort, FILE * fh, fxFltType *result,
  const PixelChannel channel, const ssize_t imgx, const ssize_t imgy)
{

  const Quantum * p = fx_infort->thisPixel;

  /* For -fx, this sets p to ImgNum 0.
     for %[fx:...], this sets p to the currrent image.
     Similarly img.
  */
  (void) fh;
  if (!p) p = GetCacheViewVirtualPixels (
    fx_info->Views[fx_info->ImgNum], imgx, imgy, 1, 1, fx_info->exception);

  fxFltType regA=0, regB=0, regC=0, regD=0, regE=0;
  Image * img = fx_info->image;
  ChannelStatistics * cs = NULL;
  if (fx_info->NeedStats) {
    cs = fx_info->statistics[fx_info->ImgNum];
  }

  /*  Folllowing is only for expressions like "saturation", with no image specifier.
  */
  double hue=0, saturation=0, lightness=0;
  if (fx_info->NeedHsl) {
    ConvertRGBToHSL (
      GetPixelRed (img, p), GetPixelGreen (img, p), GetPixelBlue (img, p),
      &hue, &saturation, &lightness);
  }

  ssize_t i;
  for (i=0; i < fx_info->usedElements; i++) {
    ElementT *pel = &fx_info->Elements[i];
      switch (pel->nArgs) {
        case 0:
          break;
        case 1:
          regA = PopVal (fx_info, fx_infort, i);
          break;
        case 2:
          regB = PopVal (fx_info, fx_infort, i);
          regA = PopVal (fx_info, fx_infort, i);
          break;
        case 3:
          regC = PopVal (fx_info, fx_infort, i);
          regB = PopVal (fx_info, fx_infort, i);
          regA = PopVal (fx_info, fx_infort, i);
          break;
        case 4:
          regD = PopVal (fx_info, fx_infort, i);
          regC = PopVal (fx_info, fx_infort, i);
          regB = PopVal (fx_info, fx_infort, i);
          regA = PopVal (fx_info, fx_infort, i);
          break;
        case 5:
          regE = PopVal (fx_info, fx_infort, i);
          regD = PopVal (fx_info, fx_infort, i);
          regC = PopVal (fx_info, fx_infort, i);
          regB = PopVal (fx_info, fx_infort, i);
          regA = PopVal (fx_info, fx_infort, i);
          break;
        default:
          ThrowMagickException (
            fx_info->exception, GetMagickModule(), OptionError,
            "Too many args:", "%i", pel->nArgs);
          break;
      }

      switch (pel->oprNum) {
        case oAddEq:
          regA = (fx_infort->UserSymVals[pel->EleNdx] += regA);
          break;
        case oSubtractEq:
          regA = (fx_infort->UserSymVals[pel->EleNdx] -= regA);
          break;
        case oMultiplyEq:
          regA = (fx_infort->UserSymVals[pel->EleNdx] *= regA);
          break;
        case oDivideEq:
          if (regA != 0) regA = (fx_infort->UserSymVals[pel->EleNdx] /= regA);
          break;
        case oPlusPlus:
          regA = fx_infort->UserSymVals[pel->EleNdx]++;
          break;
        case oSubSub:
          regA = fx_infort->UserSymVals[pel->EleNdx]--;
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
          if (regB != 0) regA /= regB;
          break;
        case oModulus:
          regA = fmod (regA, fabs(floor(regB+0.5)));
          break;
        case oUnaryPlus:
          /* Do nothing. */
          break;
        case oUnaryMinus:
          regA = -regA;
          break;
        case oLshift:
          regA = (size_t)(regA+0.5) << (size_t)(regB+0.5);
          break;
        case oRshift:
          regA = (size_t)(regA+0.5) >> (size_t)(regB+0.5);
          break;
        case oEq:
          regA = fabs(regA-regB) < MagickEpsilon ? 1.0 : 0.0;
          break;
        case oNotEq:
          regA = fabs(regA-regB) >= MagickEpsilon ? 1.0 : 0.0;
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
          regA = (regA<=0) ? 0 : (regB > 0) ? 1 : 0;
          break;
        case oLogOr:
          regA = (regA>0) ? 1 : (regB > 0) ? 1 : 0;
          break;
        case oLogNot:
          regA = (regA==0) ? 1 : 0;
          break;
        case oBitAnd:
          regA = (size_t)(regA+0.5) & (size_t)(regB+0.5);
          break;
        case oBitOr:
          regA = (size_t)(regA+0.5) | (size_t)(regB+0.5);
          break;
        case oBitNot:
          /* Old fx doesn't add 0.5. */
          regA = ~(size_t)(regA+0.5);
          break;
        case oPow:
          regA = pow (regA, regB);
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
            switch (channel) {
              default:
              case 0:
                regA = pel->val;
                break;
              case 1:
                regA = pel->val1;
                break;
              case 2:
                regA = pel->val2;
                break;
            }
          } else {
            regA = pel->val;
          }
          break;
        }
        case fAbs:
          regA = abs (regA);
          break;
#if defined(MAGICKCORE_HAVE_ACOSH)
        case fAcosh:
          regA = acosh (regA);
          break;
#endif
        case fAcos:
          regA = acos (regA);
          break;
#if defined(MAGICKCORE_HAVE_J1)
        case fAiry:
          if (regA==0) regA = 1.0;
          else {
            fxFltType gamma = 2.0 * j1 ((MagickPI*regA)) / (MagickPI*regA);
            regA = gamma * gamma;
          }
          break;
#endif
        case fAlt:
          regA = ((ssize_t) regA) & 0x01 ? -1.0 : 1.0;
          break;
#if defined(MAGICKCORE_HAVE_ASINH)
        case fAsinh:
          regA = asinh (regA);
          break;
#endif
        case fAsin:
          regA = asin (regA);
          break;
#if defined(MAGICKCORE_HAVE_ATANH)
        case fAtanh:
          regA = atanh (regA);
          break;
#endif
        case fAtan2:
          regA = atan2 (regA, regB);
          break;
        case fAtan:
          regA = atan (regA);
          break;
        case fCeil:
          regA = ceil (regA);
          break;
        case fChannel:
          switch (channel) {
            case 0: break;
            case 1: regA = regB; break;
            case 2: regA = regC; break;
            case 3: regA = regD; break;
            case 4: regA = regE; break;
            default: regA = 0;
          }
          break;
        case fClamp:
          if (regA < 0) regA = 0;
          else if (regA > 1.0) regA = 1.0;
          break;
        case fCosh:
          regA = cosh (regA);
          break;
        case fCos:
          regA = cos (regA);
          break;
        case fDebug:
          /* FIXME: debug() should give channel name. */

          fprintf (stderr, "%s[%g,%g].%g: %s=%.*Lg\n",
                   img->filename, (double) imgx, (double) imgy,
                   (double) channel, SetPtrShortExp (fx_info, pel->pExpStart, pel->lenExp+1),
                   fx_info->precision, regA);
          break;
        case fDrc:
          regA = regA / (regB*(regA-1.0) + 1.0);
          break;
#if defined(MAGICKCORE_HAVE_ERF)
        case fErf:
          regA = erf (regA);
          break;
#endif
        case fExp:
          regA = exp (regA);
          break;
        case fFloor:
          regA = floor (regA);
          break;
        case fGauss:
          regA = exp((-regA*regA/2.0))/sqrt(2.0*MagickPI);
          break;
        case fGcd:
          regA = FxGcd ((MagickOffsetType) (regA+0.5),(MagickOffsetType) (regB+0.5));
          break;
        case fHypot:
          regA = hypot (regA, regB);
          break;
        case fInt:
          regA = floor (regA);
          break;
        case fIsnan:
          regA = !!IsNaN (regA);
          break;
#if defined(MAGICKCORE_HAVE_J0)
        case fJ0:
          regA = j0 (regA);
          break;
#endif
#if defined(MAGICKCORE_HAVE_J1)
        case fJ1:
          regA = j1 (regA);
          break;
#endif
#if defined(MAGICKCORE_HAVE_J1)
        case fJinc:
          if (regA==0) regA = 1.0;
          else regA = 2.0 * j1 ((MagickPI*regA))/(MagickPI*regA);
          break;
#endif
        case fLn:
          regA = log (regA);
          break;
        case fLogtwo:
          regA = log10(regA) / log10(2.0);
          break;
        case fLog:
          regA = log10 (regA);
          break;
        case fMax:
          regA = (regA > regB) ? regA : regB;
          break;
        case fMin:
          regA = (regA < regB) ? regA : regB;
          break;
        case fMod:
          regA = regA - floor((regA*PerceptibleReciprocal(regB)))*regB;
          break;
        case fNot:
          regA = (regA < MagickEpsilon);
          break;
        case fPow:
          regA = pow (regA, regB);
          break;
        case fRand: {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp critical (MagickCore_ExecuteRPN)
#endif
          regA = GetPseudoRandomValue (fx_infort->random_info);
          break;
        }
        case fRound:
          regA = floor (regA + 0.5);
          break;
        case fSign:
          regA = (regA < 0) ? -1.0 : 1.0;
          break;
        case fSinc:
          regA = sin ((MagickPI*regA)) / (MagickPI*regA);
          break;
        case fSinh:
          regA = sinh (regA);
          break;
        case fSin:
          regA = sin (regA);
          break;
        case fSqrt:
          regA = sqrt (regA);
          break;
        case fSquish:
          regA = 1.0 / (1.0 + exp (-regA));
          break;
        case fTanh:
          regA = tanh (regA);
          break;
        case fTan:
          regA = tan (regA);
          break;
        case fTrunc:
          if (regA >= 0) regA = floor (regA);
          else regA = ceil (regA);
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
          int ImgNum = ChkImgNum (fx_info, regA);
          regA = 0;
          if (ImgNum == 0) {
            Image * pimg = fx_info->Images[0];
            int pech = (int)pel->ChannelQual;
            if (pel->ImgAttrQual == aNull) {
              if (pech < 0) {
                if (pech == NO_CHAN_QUAL || pech == THIS_CHANNEL) {
                  if (fx_info->ImgNum==0) {
                    regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                  } else {
                    const Quantum * pv = GetCacheViewVirtualPixels (
                                   fx_info->Views[0], imgx, imgy, 1,1, fx_info->exception);
                    if (!pv) {
                      ThrowMagickException (
                        fx_info->exception, GetMagickModule(), OptionError,
                        "fU can't get cache", "%i", ImgNum);
                      break;
                    }
                    regA = QuantumScale * pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                  }
                } else if (pech == HUE_CHANNEL || pech == SAT_CHANNEL ||
                    pech == LIGHT_CHANNEL) {
                  regA = GetHslInt (fx_info, ImgNum, imgx, imgy, pech);
                  break;
                } else if (pech == INTENSITY_CHANNEL) {
                  regA = GetIntensity (fx_info, 0, imgx, imgy);
                }
              } else {
                if (fx_info->ImgNum==0) {
                  regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                } else {
                  const Quantum * pv = GetCacheViewVirtualPixels (
                                 fx_info->Views[0], imgx, imgy, 1,1, fx_info->exception);
                  if (!pv) {
                    ThrowMagickException (
                      fx_info->exception, GetMagickModule(), OptionError,
                      "fU can't get cache", "%i", ImgNum);
                    break;
                  }
                  regA = QuantumScale * pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                }
              }
            } else {
              /* we have an image atttribute */
              regA = ImageStat (fx_info, 0, WHICH_ATTR_CHAN, pel->ImgAttrQual);
            }
          } else {
            /* We have non-zero ImgNum. */
            if (pel->ImgAttrQual == aNull) {
              if ((int)pel->ChannelQual < 0) {
                if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL ||
                    pel->ChannelQual == LIGHT_CHANNEL)
                {
                  regA = GetHslInt (fx_info, ImgNum, imgx, imgy, pel->ChannelQual);
                  break;
                } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
                  regA = GetIntensity (fx_info, ImgNum, imgx, imgy);
                  break;
                }
              }

              const Quantum * pv = GetCacheViewVirtualPixels (
                                   fx_info->Views[ImgNum], imgx, imgy, 1,1, fx_info->exception);
              if (!pv) {
                ThrowMagickException (
                  fx_info->exception, GetMagickModule(), OptionError,
                  "fU can't get cache", "%i", ImgNum);
                break;
              }
              regA = QuantumScale *
         pv[fx_info->Images[ImgNum]->channel_map[WHICH_NON_ATTR_CHAN].offset];
            } else {
              regA = ImageStat (fx_info, ImgNum, WHICH_ATTR_CHAN, pel->ImgAttrQual);
            }
          }
          break;
        }
        case fU0: {
          /* No args. No image attribute. We may have a ChannelQual.
             If called from %[fx:...], ChannelQual will be CompositePixelChannel.
          */
          Image * pimg = fx_info->Images[0];
          int pech = (int)pel->ChannelQual;
          if (pech < 0) {
            if (pech == NO_CHAN_QUAL || pech == THIS_CHANNEL) {

              if (fx_info->ImgNum==0) {
                regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
              } else {
                const Quantum * pv = GetCacheViewVirtualPixels (
                               fx_info->Views[0], imgx, imgy, 1,1, fx_info->exception);
                if (!pv) {
                  ThrowMagickException (
                    fx_info->exception, GetMagickModule(), OptionError,
                    "fU0 can't get cache", "%i", 0);
                  break;
                }
                regA = QuantumScale * pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
              }

            } else if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL ||
                       pel->ChannelQual == LIGHT_CHANNEL) {
              regA = GetHslInt (fx_info, 0, imgx, imgy, pel->ChannelQual);
              break;
            } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
              regA = GetIntensity (fx_info, 0, imgx, imgy);
            }
          } else {
            if (fx_info->ImgNum==0) {
              regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
            } else {
              const Quantum * pv = GetCacheViewVirtualPixels (
                                   fx_info->Views[0], imgx, imgy, 1,1, fx_info->exception);
              if (!pv) {
                ThrowMagickException (
                  fx_info->exception, GetMagickModule(), OptionError,
                  "fU0 can't get cache", "%i", 0);
                break;
              }
              regA = QuantumScale * pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
            }
          }
          break;
        }
        case fUP: {
          /* 3 args are: ImgNum, x, y */
          int ImgNum = ChkImgNum (fx_info, regA);

          fxFltType fx, fy;
          if (pel->IsRelative) {
            fx = imgx + regB;
            fy = imgy + regC;
          } else {
            fx = regB;
            fy = regC;
          }

          if ((int)pel->ChannelQual < 0) {
            if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL
             || pel->ChannelQual == LIGHT_CHANNEL) {
              regA = GetHslFlt (fx_info, ImgNum, fx, fy, pel->ChannelQual);
              break;
            } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
              regA = GetIntensity (fx_info, ImgNum, fx, fy);
              break;
            }
          }

          double v;
          Image * imUP = fx_info->Images[ImgNum];
          if (! InterpolatePixelChannel (imUP, fx_info->Views[ImgNum], WHICH_NON_ATTR_CHAN,
                  imUP->interpolate, fx, fy, &v, fx_info->exception))
          {
            ThrowMagickException (
              fx_info->exception, GetMagickModule(), OptionError,
              "fUP can't get interpolate", "%i", ImgNum);
            break;
          }
          regA = v * QuantumScale;

          break;
        }
        case fS:
        case fV: {
          /* No args. */
          int ImgNum = 1;
          if (pel->oprNum == fS) ImgNum = fx_info->ImgNum;

          if (pel->ImgAttrQual == aNull) {
            const Quantum * pv = GetCacheViewVirtualPixels (
                                   fx_info->Views[ImgNum], imgx, imgy, 1,1, fx_info->exception);
            if (!pv) {
              ThrowMagickException (
                fx_info->exception, GetMagickModule(), OptionError,
                "fV can't get cache", "%i", ImgNum);
              break;
            }

            if ((int)pel->ChannelQual < 0) {
              if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL ||
                  pel->ChannelQual == LIGHT_CHANNEL) {
                regA = GetHslInt (fx_info, ImgNum, imgx, imgy, pel->ChannelQual);
                break;
              } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
                regA = GetIntensity (fx_info, ImgNum, imgx, imgy);
                break;
              }
            }

            regA = QuantumScale *
         pv[fx_info->Images[ImgNum]->channel_map[WHICH_NON_ATTR_CHAN].offset];
          } else {
            regA = ImageStat (fx_info, ImgNum, WHICH_ATTR_CHAN, pel->ImgAttrQual);
          }

          break;
        }
        case fP:
        case fSP:
        case fVP: {
          /* 2 args are: x, y */
          int ImgNum = fx_info->ImgNum;
          if (pel->oprNum == fVP) ImgNum = 1;
          fxFltType fx, fy;
          if (pel->IsRelative) {
            fx = imgx + regA;
            fy = imgy + regB;
          } else {
            fx = regA;
            fy = regB;
          }
          if ((int)pel->ChannelQual < 0) {
            if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL ||
                pel->ChannelQual == LIGHT_CHANNEL) {
              regA = GetHslFlt (fx_info, ImgNum, fx, fy, pel->ChannelQual);
              break;
            } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
              regA = GetIntensity (fx_info, ImgNum, fx, fy);
            }
          }

          double v;

          if (! InterpolatePixelChannel (fx_info->Images[ImgNum], fx_info->Views[ImgNum],
                                         WHICH_NON_ATTR_CHAN, fx_info->Images[ImgNum]->interpolate,
                                         fx, fy, &v, fx_info->exception)
                                        )
          {
            ThrowMagickException (
              fx_info->exception, GetMagickModule(), OptionError,
              "fSP or fVP can't get interp", "%i", ImgNum);
            break;
          }
          regA = v * (fxFltType)QuantumScale;

          break;
        }
        case fNull:
          break;
        case aDepth:
          regA = GetImageDepth (img, fx_info->exception) / QuantumRange;
          break;
        case aExtent:
          regA = img->extent;
          break;
        case aKurtosis:
          regA = cs[WHICH_ATTR_CHAN].kurtosis;
          break;
        case aMaxima:
          regA = cs[WHICH_ATTR_CHAN].maxima;
          break;
        case aMean:
          regA = cs[WHICH_ATTR_CHAN].mean;
          break;
        case aMedian:
          regA = cs[WHICH_ATTR_CHAN].median;
          break;
        case aMinima:
          regA = cs[WHICH_ATTR_CHAN].minima;
          break;
        case aPage:
          break;
        case aPageWid:
          regA = img->page.width;
          break;
        case aPageHt:
          regA = img->page.height;
          break;
        case aPageX:
          regA = img->page.x;
          break;
        case aPageY:
          regA = img->page.y;
          break;
        case aPrintsize:
          break;
        case aPrintsizeX:
          regA = PerceptibleReciprocal (img->resolution.x) * img->columns;
          break;
        case aPrintsizeY:
          regA = PerceptibleReciprocal (img->resolution.y) * img->rows;
          break;
        case aQuality:
          regA = img->quality;
          break;
        case aRes:
          break;
        case aResX:
          regA = img->resolution.x;
          break;
        case aResY:
          regA = img->resolution.y;
          break;
        case aSkewness:
          regA = cs[WHICH_ATTR_CHAN].skewness;
          break;
        case aStdDev:
          regA = cs[WHICH_ATTR_CHAN].standard_deviation;
          break;
        case aH: /* image->rows */
          regA = img->rows;
          break;
        case aN: /* image list length */
          regA = fx_info->ImgListLen;
          break;
        case aT: /* image index in list */
          regA = fx_info->ImgNum;
          break;
        case aW: /* image->columns */
          regA = img->columns;
          break;
        case aZ: /* image depth */
          regA = GetImageDepth (img, fx_info->exception);
          break;
        case aNull:
          break;
        case sHue: /* of conversion to HSL */
          regA = hue;
          break;
        case sIntensity:
          regA = GetIntensity (fx_info, fx_info->ImgNum, imgx, imgy);
          break;
        case sLightness: /* of conversion to HSL */
          regA = lightness;
          break;
        case sLuma: /* calculation */
        case sLuminance: /* as Luma */
          regA = QuantumScale * (0.212656 * GetPixelRed (img,p) +
                                 0.715158 * GetPixelGreen (img,p) +
                                 0.072186 * GetPixelBlue (img,p));
          break;
        case sSaturation: /* from conversion to HSL */
          regA = saturation;
          break;
        case sA: /* alpha */
          regA = QuantumScale * GetPixelAlpha (img, p);
          break;
        case sB: /* blue */
          regA = QuantumScale * GetPixelBlue (img, p);
          break;
        case sC: /* red (ie cyan) */
          regA = QuantumScale * GetPixelCyan (img, p);
          break;
        case sG: /* green */
          regA = QuantumScale * GetPixelGreen (img, p);
          break;
        case sI: /* current x-coordinate */
          regA = imgx;
          break;
        case sJ: /* current y-coordinate */
          regA = imgy;
          break;
        case sK: /* black of CMYK */
          regA = QuantumScale * GetPixelBlack (img, p);
          break;
        case sM: /* green (ie magenta) */
          regA = QuantumScale * GetPixelGreen (img, p);
          break;
        case sO: /* alpha */
          regA = QuantumScale * GetPixelAlpha (img, p);
          break;
        case sR:
          regA = QuantumScale * GetPixelRed (img, p);
          break;
        case sY:
          regA = QuantumScale * GetPixelYellow (img, p);
          break;
        case sNull:
          break;

        case rGoto:
          i = pel->EleNdx-1; /* -1 because 'for' loop will increment. */
          break;
        case rIfZeroGoto:
          if (fabs(regA) < MagickEpsilon) i = pel->EleNdx-1;
          break;
        case rIfNotZeroGoto:
          if (fabs(regA) > MagickEpsilon) i = pel->EleNdx-1;
          break;
        case rCopyFrom:
          regA = fx_infort->UserSymVals[pel->EleNdx];
          break;
        case rCopyTo:
          fx_infort->UserSymVals[pel->EleNdx] = regA;
          break;
        case rZerStk:
          fx_infort->usedValStack = 0;
          break;
        case rNull:
          break;

        default:
          ThrowMagickException (
            fx_info->exception, GetMagickModule(), OptionError,
            "pel->oprNum", "%i '%s' not yet implemented",
            (int)pel->oprNum, OprStr(fx_info, pel->oprNum));
          break;
    }
    if (i < 0) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "Bad run-time address", "%g", (double) i);
    }
    if (pel->DoPush) 
      if (!PushVal (fx_info, fx_infort, regA, i)) break;
  }

  if (fx_infort->usedValStack > 0) regA = PopVal (fx_info, fx_infort, 9999);

  *result = regA;

  if (fx_infort->usedValStack != 0) {
      ThrowMagickException (
        fx_info->exception, GetMagickModule(), OptionError,
        "ValStack not empty", "%i",  fx_infort->usedValStack);
    return MagickFalse;
  }

  return MagickTrue;
}

/* Following is substitute for FxEvaluateChannelExpression().
*/
MagickPrivate MagickBooleanType FxEvaluateChannelExpression (
  FxInfo *fx_info,
  const PixelChannel channel, const ssize_t x, const ssize_t y,
  double *result, ExceptionInfo *exception)
{
  assert (fx_info != NULL);
  assert (fx_info->image != NULL);
  assert (fx_info->Images != NULL);
  assert (fx_info->Views != NULL);
  assert (fx_info->fxrts != NULL);

  const int
    id = GetOpenMPThreadId();

  fx_info->fxrts[id].thisPixel = NULL;

  fxFltType ret;

  if (!ExecuteRPN (fx_info, &fx_info->fxrts[id], stderr, &ret, channel, x, y)) {
    ThrowMagickException (exception, GetMagickModule(), OptionError,
      "ExecuteRPN failed", "'%s'",fx_info->pex);
    return MagickFalse;
  }

  *result = ret;

  return MagickTrue;
}

MagickPrivate FxInfo *AcquireFxInfo (const Image * images, const char * expression, ExceptionInfo *exception)
{
  FxInfo * fx_info = (FxInfo*) AcquireCriticalMemory (sizeof (*fx_info));

  memset (fx_info, 0, sizeof (*fx_info));

  if (!InitFx (fx_info, images, exception)) return NULL;

  if (!BuildRPN (fx_info)) return NULL;

  fx_info->pex = (char *)expression;

  if (*expression == '@')
    fx_info->expression = FileToString (expression+1, ~0UL, exception);
  else
    fx_info->expression = ConstantString (expression);

  fx_info->pex = fx_info->expression;

  fx_info->teDepth = 0;
  char chLimit;
  if (!TranslateStatementList (fx_info, (char *) ";", &chLimit)) return NULL;

  if (fx_info->teDepth) {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "Translate expression depth", "(%i) not 0",
      fx_info->teDepth);
    return NULL;
  }

  if (chLimit != '\0' && chLimit != ';') {
    ThrowMagickException (
      fx_info->exception, GetMagickModule(), OptionError,
      "AcquireFxInfo: TranslateExpression did not exhaust input at", "'%s'",
      fx_info->pex);
    return NULL;
  }

  if (fx_info->NeedStats && !fx_info->statistics) {
    if (!CollectStatistics (fx_info)) return (FxInfo *) NULL;
  }

  if (fx_info->DebugOpt) {
    DumpTables (fx_info, stderr);
    DumpUserSymbols (fx_info, stderr);
    DumpRPN (fx_info, stderr);
  }

  size_t number_threads=(size_t) GetMagickResourceLimit(ThreadResource);

  fx_info->fxrts = (fxRtT *)AcquireQuantumMemory (number_threads, sizeof(fxRtT));
  if (!fx_info->fxrts) {
    (void) ThrowMagickException(exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
    return NULL;
  }
  int t;
  for (t=0; t < (int) number_threads; t++) {
    if (!AllocFxRt (fx_info, &fx_info->fxrts[t])) {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return NULL;
    }
  }

  return fx_info;
}

FxInfo *DestroyFxInfo (FxInfo * fx_info)
{
  assert (fx_info != NULL);
  assert (fx_info->image != NULL);
  assert (fx_info->Images != NULL);
  assert (fx_info->Views != NULL);
  assert (fx_info->fxrts != NULL);

  int t;
  for (t=0; t < (int) GetMagickResourceLimit(ThreadResource); t++) {
    DestroyFxRt (&fx_info->fxrts[t]);
  }
  fx_info->fxrts = (fxRtT *) RelinquishMagickMemory (fx_info->fxrts);

  DestroyRPN (fx_info);

  fx_info->expression = DestroyString (fx_info->expression);
  fx_info->pex = NULL;

  DeInitFx (fx_info);

  fx_info = (FxInfo *) RelinquishMagickMemory(fx_info);

  return NULL;
}

/* Following is substitute for FxImage().
*/
MagickExport Image *FxImage (const Image *image, const char *expression,
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

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (expression == (const char *) NULL)
    return(CloneImage(image,0,0,MagickTrue,exception));
  fx_image=CloneImage(image,0,0,MagickTrue,exception);
  if (!fx_image) return NULL;
  if (SetImageStorageClass(fx_image,DirectClass,exception) == MagickFalse) {
    fx_image=DestroyImage(fx_image);
    return NULL;
  }

  FxInfo * fx_info = AcquireFxInfo (image, expression, exception);

  if (!fx_info) return NULL;
  assert (fx_info->image != NULL);
  assert (fx_info->Images != NULL);
  assert (fx_info->Views != NULL);
  assert (fx_info->fxrts != NULL);

  status=MagickTrue;
  progress=0;
  image_view = AcquireVirtualCacheView (image, fx_info->exception);
  fx_view = AcquireAuthenticCacheView (fx_image, fx_info->exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic) shared(progress,status) \
    magick_number_threads(image,fx_image,fx_image->rows, \
      fx_info->ContainsDebug ? 0 : 1)
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
    p = GetCacheViewVirtualPixels (image_view, 0, y, image->columns, 1, fx_info->exception);
    q = QueueCacheViewAuthenticPixels (fx_view, 0, y, fx_image->columns, 1, fx_info->exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL)) {
        status=MagickFalse;
        continue;
    }
    for (x=0; x < (ssize_t) fx_image->columns; x++) {
      ssize_t i;

      fx_info->fxrts[id].thisPixel = (Quantum *)p;

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

        if (!ExecuteRPN (fx_info, &fx_info->fxrts[id], stderr, &result, channel, x, y)) {
          status=MagickFalse;
          continue;
        }

        q[i] = ClampToQuantum (QuantumRange*result);
      }
      p+=GetPixelChannels (image);
      q+=GetPixelChannels (fx_image);
    }
    if (SyncCacheViewAuthenticPixels(fx_view, fx_info->exception) == MagickFalse)
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
  if (fx_info->DebugOpt && fx_info->usedUserSymbols) {
    fprintf (stderr, "User symbols (%i):\n", fx_info->usedUserSymbols);
    int t, i;
    char UserSym[MaxTokenLen];
    for (t=0; t < (int) GetMagickResourceLimit(ThreadResource); t++) {
      for (i = 0; i < fx_info->usedUserSymbols; i++) {
        fprintf (stderr, "th=%i us=%i '%s': %.*Lg\n",
                 t, i, NameOfUserSym (fx_info, i, UserSym), fx_info->precision, fx_info->fxrts[t].UserSymVals[i]);
      }
    }
  }

  if (fx_info->exception->severity != UndefinedException) {
    status = MagickFalse;
  }

  if (status == MagickFalse)
    fx_image = DestroyImage (fx_image);

  fx_info = DestroyFxInfo (fx_info);

  return(fx_image);
}

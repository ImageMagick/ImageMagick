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
  char * str;
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
  char * str;
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

#define FirstFunc (oNull+1)

typedef enum {
  fAbs = FirstFunc,
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
  char * str;
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

#define FirstImgAttr (fNull+1)

typedef enum {
  aDepth = FirstImgAttr,
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
  char * str;
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

#define FirstSym (aNull+1)

typedef enum {
  sHue = FirstSym,
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
  char * str;
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
  char * str;
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
  char * str;
  PixelChannel pixChan;
} ChannelT;

#define NO_CHAN_QUAL      (-1)
#define THIS_CHANNEL      (-2)
#define HUE_CHANNEL       (-3)
#define SAT_CHANNEL       (-4)
#define LIGHT_CHANNEL     (-5)
#define INTENSITY_CHANNEL (-6)

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
  int len;
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
  int lenExp;
} ElementT;

typedef struct {
  RandomInfo * magick_restrict random_info;
  int numValStack;
  int usedValStack;
  fxFltType * ValStack;
  fxFltType * UserSymVals;
  Quantum * thisPixel;
} fxRtT;

typedef struct _FxInfo {
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
  int lenToken;
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
} fxT;

/* Forward declarations for recursion.
*/
static MagickBooleanType TranslateStatementList
  (fxT * pfx, char * strLimit, char * chLimit);

static MagickBooleanType TranslateExpression
  (fxT * pfx, char * strLimit, char * chLimit, MagickBooleanType * needPopAll);

static MagickBooleanType GetFunction (fxT * pfx, FunctionE fe);

static MagickBooleanType InitFx (fxT * pfx, const Image * img, ExceptionInfo *exception)
{
  pfx->ImgListLen = (int) GetImageListLength (img);
  pfx->ImgNum = (int) GetImageIndexInList (img);
  pfx->image = (Image *)img;

  pfx->NeedStats = MagickFalse;
  pfx->NeedHsl = MagickFalse;
  pfx->DebugOpt = IsStringTrue (GetImageArtifact (img, "fx:debug"));
  pfx->statistics = NULL;
  pfx->Views = NULL;
  pfx->Images = NULL;
  pfx->exception = exception;
  pfx->precision = GetMagickPrecision ();
  pfx->random_infos = AcquireRandomInfoThreadSet ();
  pfx->ContainsDebug = MagickFalse;

  pfx->Views =
    (CacheView **) AcquireQuantumMemory (GetImageListLength (img), sizeof (pfx->Views));
  if (!pfx->Views) ThrowFatalException(ResourceLimitFatalError, "Views oom");
  int i=0;
  const Image * next = GetFirstImageInList (img);
  for ( ; next != (Image *) NULL; next=next->next)
  {
    pfx->Views[i] = AcquireVirtualCacheView (next, pfx->exception);
    if (!pfx->Views[i]) ThrowFatalException(ResourceLimitFatalError, "Views[i] oom");
    i++;
  }

  pfx->Images = ImageListToArray (img, pfx->exception);

  return MagickTrue;
}

static MagickBooleanType DeInitFx (fxT * pfx)
{
  if (pfx->Images) RelinquishMagickMemory (pfx->Images);

  int i;
  if (pfx->Views) {
    for (i=(ssize_t) GetImageListLength(pfx->image)-1; i >= 0; i--)
      pfx->Views[i] = DestroyCacheView (pfx->Views[i]);
    pfx->Views=(CacheView **) RelinquishMagickMemory (pfx->Views);
  }

  pfx->random_infos = DestroyRandomInfoThreadSet (pfx->random_infos);

  if (pfx->statistics) {
    for (i=(ssize_t) GetImageListLength(pfx->image)-1; i >= 0; i--) {
      RelinquishMagickMemory (pfx->statistics[i]);
    }

    pfx->statistics = RelinquishMagickMemory(pfx->statistics);
  }

  return MagickTrue;
}

static ElementTypeE TypeOfOpr (fxT * pfx, int op)
{
  if (op <  oNull) return etOperator;
  if (op == oNull) return etConstant;
  if (op <= fNull) return etFunction;
  if (op <= aNull) return etImgAttr;
  if (op <= sNull) return etSymbol;
  if (op <= rNull) return etControl;

  return 0;
}

static char * SetPtrShortExp (fxT * pfx, char * pExp, int len)
{
  #define MaxLen 20

  *pfx->ShortExp = '\0';

  if (pExp && len) {
    int slen = CopyMagickString (pfx->ShortExp, pExp, len);
    if (slen > MaxLen) { 
      CopyMagickString (pfx->ShortExp+MaxLen, "...", 4);
    }
    char * p = strchr (pfx->ShortExp, '\n');
    if (p) CopyMagickString (p, "...", 4);
    p = strchr (pfx->ShortExp, '\r');
    if (p) CopyMagickString (p, "...", 4);
  }
  return pfx->ShortExp;
}

static char * SetShortExp (fxT * pfx)
{
  return SetPtrShortExp (pfx, pfx->pex, MaxTokenLen-1);
}

static int FindUserSymbol (fxT * pfx, char * name)
/* returns index into pfx->UserSymbols, and thus into pfxrt->UserSymVals,
   or NULL_ADDRESS if not found.
*/
{
  int i;
  int lenName = strlen (name);
  for (i=0; i < pfx->usedUserSymbols; i++) {
    UserSymbolT *pus = &pfx->UserSymbols[i];
    if (lenName == pus->len && LocaleNCompare (name, pus->pex, lenName)==0) break;
  }
  if (i == pfx->usedUserSymbols) return NULL_ADDRESS;
  return i;
}

static MagickBooleanType ExtendUserSymbols (fxT * pfx)
{
  pfx->numUserSymbols = ceil (pfx->numUserSymbols * (1 + TableExtend));
  pfx->UserSymbols = ResizeMagickMemory (pfx->UserSymbols, pfx->numUserSymbols * sizeof(UserSymbolT));
  if (!pfx->UserSymbols) {
    ThrowFatalException(ResourceLimitFatalError, "ExtendUserSymbols oom");
    return MagickFalse;
  }
  return MagickTrue;
}

static int AddUserSymbol (fxT * pfx, char * pex, int len)
{
  if (++pfx->usedUserSymbols >= pfx->numUserSymbols) {
    if (!ExtendUserSymbols (pfx)) return -1;
  }
  UserSymbolT *pus = &pfx->UserSymbols[pfx->usedUserSymbols-1];
  pus->pex = pex;
  pus->len = len;

  return pfx->usedUserSymbols-1;
}

static void DumpTables (fxT * pfx, FILE * fh)
{

  int i;
  for (i=0; i <= rNull; i++) {
    char * str = "";
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

static char * NameOfUserSym (fxT * pfx, int ndx, char * buf)
{
  assert (ndx >= 0 && ndx < pfx->usedUserSymbols);
  UserSymbolT * pus = &pfx->UserSymbols[ndx];
  CopyMagickString (buf, pus->pex, pus->len+1);
  return buf;
}

static void DumpUserSymbols (fxT * pfx, FILE * fh)
{
  fprintf (fh, "UserSymbols (%i)\n", pfx->usedUserSymbols);
  char UserSym[MaxTokenLen];
  int i;
  for (i=0; i < pfx->usedUserSymbols; i++) {
    fprintf (fh, "  %i: '%s'\n", i, NameOfUserSym (pfx, i, UserSym));
  }
}

static MagickBooleanType BuildRPN (fxT * pfx)
{
  pfx->numUserSymbols = InitNumUserSymbols;
  pfx->usedUserSymbols = 0;
  pfx->UserSymbols = AcquireMagickMemory (pfx->numUserSymbols * sizeof(UserSymbolT));
  if (!pfx->UserSymbols) ThrowFatalException(ResourceLimitFatalError, "UserSymbols oom");

  pfx->numElements = RpnInit;
  pfx->usedElements = 0;
  pfx->Elements = NULL;

  pfx->Elements = AcquireMagickMemory (pfx->numElements * sizeof(ElementT));
  if (!pfx->Elements) ThrowFatalException(ResourceLimitFatalError, "Elements oom");

  pfx->usedOprStack = 0;
  pfx->maxUsedOprStack = 0;
  pfx->numOprStack = InitNumOprStack;
  pfx->OperatorStack = AcquireMagickMemory (pfx->numOprStack * sizeof(OperatorE));
  if (!pfx->OperatorStack) ThrowFatalException(ResourceLimitFatalError, "OperatorStack oom");

  return MagickTrue;
}

static MagickBooleanType AllocFxRt (fxT * pfx, fxRtT * pfxrt)
{
  pfxrt->random_info = AcquireRandomInfo ();
  pfxrt->thisPixel = NULL;

  int nRnd = 20 + 10 * GetPseudoRandomValue (pfxrt->random_info);
  int i;
  for (i=0; i < nRnd; i++) GetPseudoRandomValue (pfxrt->random_info);;

  pfxrt->usedValStack = 0;
  pfxrt->numValStack = 2 * pfx->maxUsedOprStack;
  if (pfxrt->numValStack < MinValStackSize) pfxrt->numValStack = MinValStackSize;
  pfxrt->ValStack = AcquireMagickMemory (pfxrt->numValStack * sizeof(fxFltType));
  if (!pfxrt->ValStack) ThrowFatalException(ResourceLimitFatalError, "ValStack oom");

  pfxrt->UserSymVals = NULL;

  if (pfx->usedUserSymbols) {
    pfxrt->UserSymVals = AcquireMagickMemory (pfx->usedUserSymbols * sizeof(fxFltType));
    if (!pfxrt->UserSymVals) ThrowFatalException(ResourceLimitFatalError, "UserSymVals oom");
    for (i = 0; i < pfx->usedUserSymbols; i++) pfxrt->UserSymVals[i] = 0;
  }
  return MagickTrue;
}

static MagickBooleanType ExtendRPN (fxT * pfx)
{
  pfx->numElements = ceil (pfx->numElements * (1 + TableExtend));
  pfx->Elements = ResizeMagickMemory (pfx->Elements, pfx->numElements * sizeof(ElementT));
  if (!pfx->Elements) {
    ThrowFatalException(ResourceLimitFatalError, "Extend Elements oom");
    return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType OprInPlace (fxT * pfx, int op)
{
  return (op >= oAddEq && op <= oSubSub);
}

static char * OprStr (fxT * pfx, int oprNum)
{
  char * str;
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

static MagickBooleanType DumpRPN (fxT * pfx, FILE * fh)
{
  fprintf (fh, "DumpRPN:");
  fprintf (fh, "  numElements=%i", pfx->numElements);
  fprintf (fh, "  usedElements=%i", pfx->usedElements);
  fprintf (fh, "  maxUsedOprStack=%i", pfx->maxUsedOprStack);
  fprintf (fh, "  ImgListLen=%i", pfx->ImgListLen);
  fprintf (fh, "  NeedStats=%s", pfx->NeedStats ? "yes" : "no");
  fprintf (fh, "  NeedHsl=%s\n", pfx->NeedHsl ? "yes" : "no");

  int i;
  for (i=0; i < pfx->usedElements; i++) {
    ElementT * pel = &pfx->Elements[i];
    pel->nDest = 0;
  }
  for (i=0; i < pfx->usedElements; i++) {
    ElementT * pel = &pfx->Elements[i];
    if (pel->oprNum == rGoto || pel->oprNum == rIfZeroGoto || pel->oprNum == rIfNotZeroGoto) {
      if (pel->EleNdx >= 0 && pel->EleNdx < pfx->numElements) {
        ElementT * pelDest = &pfx->Elements[pel->EleNdx];
        pelDest->nDest++;
      }
    }
  }
  for (i=0; i < pfx->usedElements; i++) {
    ElementT * pel = &pfx->Elements[i];
    const char * str = OprStr (pfx, pel->oprNum);
    const char *sRelAbs = "";
    if (pel->oprNum == fP || pel->oprNum == fUP || pel->oprNum == fVP || pel->oprNum == fSP)
      sRelAbs = pel->IsRelative ? "[]" : "{}";

    if (pel->type == etColourConstant)
      fprintf (fh, "  %i: %s vals=%.*Lg,%.*Lg,%.*Lg '%s%s' nArgs=%i ndx=%i  %s",
               i, sElementTypes[pel->type],
               pfx->precision, pel->val, pfx->precision, pel->val1, pfx->precision, pel->val2,
               str, sRelAbs, pel->nArgs, pel->EleNdx,
               pel->DoPush ? "push" : "NO push");
    else
      fprintf (fh, "  %i: %s val=%.*Lg '%s%s' nArgs=%i ndx=%i  %s",
               i, sElementTypes[pel->type], pfx->precision, pel->val, str, sRelAbs,
               pel->nArgs, pel->EleNdx,
               pel->DoPush ? "push" : "NO push");

    if (pel->ImgAttrQual != aNull)
      fprintf (fh, " ia=%s", OprStr(pfx, pel->ImgAttrQual));

    if (pel->ChannelQual != NO_CHAN_QUAL) {
      if (pel->ChannelQual == THIS_CHANNEL) fprintf (stderr, "  ch=this");
      else fprintf (stderr, "  ch=%i", pel->ChannelQual);
    }

    char UserSym[MaxTokenLen];

    if (pel->oprNum == rCopyTo) {
      fprintf (fh, "  CopyTo ==> %s", NameOfUserSym (pfx, pel->EleNdx, UserSym));
    } else if (pel->oprNum == rCopyFrom) {
      fprintf (fh, "  CopyFrom <== %s", NameOfUserSym (pfx, pel->EleNdx, UserSym));
    } else if (OprInPlace (pfx, pel->oprNum)) {
      fprintf (fh, "  <==> %s", NameOfUserSym (pfx, pel->EleNdx, UserSym));
    }
    if (pel->nDest > 0)  fprintf (fh, "  <==dest(%i)", pel->nDest);
    fprintf (fh, "\n");
  }
  return MagickTrue;
}

static void DestroyRPN (fxT * pfx)
{
  pfx->numOprStack = 0;
  pfx->usedOprStack = 0;
  if (pfx->OperatorStack) pfx->OperatorStack = RelinquishMagickMemory (pfx->OperatorStack);

  pfx->numElements = 0;
  pfx->usedElements = 0;
  if (pfx->Elements) pfx->Elements = RelinquishMagickMemory (pfx->Elements);

  pfx->usedUserSymbols = 0;
  if (pfx->UserSymbols) pfx->UserSymbols = RelinquishMagickMemory (pfx->UserSymbols);
}

static void DestroyFxRt (fxRtT * pfxrt)
{
  pfxrt->usedValStack = 0;
  if (pfxrt->ValStack) pfxrt->ValStack = RelinquishMagickMemory (pfxrt->ValStack);
  if (pfxrt->UserSymVals) pfxrt->UserSymVals = RelinquishMagickMemory (pfxrt->UserSymVals);

  pfxrt->random_info = DestroyRandomInfo (pfxrt->random_info);
}

static int GetToken (fxT * pfx)
/* Returns length of token that starts with an alpha,
     or 0 if it isn't a token that starts with an alpha.
   j0 and j1 have trailing digit.
   Also colours like "gray47" have more trailing digits.
   After intial alpha(s) also allow single "_", eg "standard_deviation".
   Does not advance pfx->pex.
   This splits "mean.r" etc.
*/
{

  *pfx->token = '\0';
  pfx->lenToken = 0;
  char * p = pfx->pex;
  if (!isalpha((int)*p)) return 0;
  int len = 0;

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
      pfx->exception, GetMagickModule(), OptionError,
      "GetToken: too long", "%i at '%s'",
      len, SetShortExp(pfx));
    len = MaxTokenLen;
  }
  if (len) {
    CopyMagickString (pfx->token, pfx->pex, (len+1<MaxTokenLen)?len+1:MaxTokenLen);
  }

  pfx->lenToken = strlen (pfx->token);
  return len;
}

static MagickBooleanType TokenMaybeUserSymbol (fxT * pfx)
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

static MagickBooleanType AddElement (fxT * pfx, fxFltType val, int oprNum)
{
  assert (oprNum <= rNull);

  if (++pfx->usedElements >= pfx->numElements) {
    if (!ExtendRPN (pfx)) return MagickFalse;
  }

  ElementT * pel = &pfx->Elements[pfx->usedElements-1];
  pel->type = TypeOfOpr (pfx, oprNum);
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

static MagickBooleanType AddAddressingElement (fxT * pfx, int oprNum, int EleNdx)
{
  if (!AddElement (pfx, 0, oprNum)) return MagickFalse;
  ElementT * pel = &pfx->Elements[pfx->usedElements-1];
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

static MagickBooleanType AddColourElement (fxT * pfx, ElementTypeE type, fxFltType val0, fxFltType val1, fxFltType val2)
{
  if (!AddElement (pfx, val0, oNull)) return MagickFalse;
  ElementT * pel = &pfx->Elements[pfx->usedElements-1];
  pel->val1 = val1;
  pel->val2 = val2;
  pel->type = etColourConstant;
  return MagickTrue;
}

static void inline SkipSpaces (fxT * pfx)
{
  while (isspace ((int)*pfx->pex)) pfx->pex++;
}

static char inline PeekChar (fxT * pfx)
{
  SkipSpaces (pfx);
  return *pfx->pex;
}

static MagickBooleanType inline PeekStr (fxT * pfx, const char * str)
{
  SkipSpaces (pfx);
  
  return (LocaleNCompare (pfx->pex, str, strlen(str))==0);
}

static MagickBooleanType ExpectChar (fxT * pfx, char c)
{
  if (PeekChar (pfx) != c) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Expected char", "'%c' at '%s'", c, SetShortExp (pfx));
    return MagickFalse;
  }
  pfx->pex++;
  return MagickTrue;
}

static int MaybeXYWH (fxT * pfx, ImgAttrE * pop)
/* If ".x" or ".y" or ".width" or ".height" increments *pop and returns 1 to 4 .
   Otherwise returns 0.
*/
{

  if (*pop != aPage && *pop != aPrintsize && *pop != aRes) return 0;

  if (PeekChar (pfx) != '.') return 0;

  if (!ExpectChar (pfx, '.')) return 0;

  int ret=0;
  GetToken (pfx);
  if (LocaleCompare ("x", pfx->token)==0) ret=1;
  else if (LocaleCompare ("y", pfx->token)==0) ret=2;
  else if (LocaleCompare ("width", pfx->token)==0) ret=3;
  else if (LocaleCompare ("height", pfx->token)==0) ret=4;

  if (!ret)
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Invalid 'x' or 'y' or 'width' or 'height' token=", "'%s' at '%s'",
      pfx->token, SetShortExp(pfx));

  if (*pop == aPage) (*pop) += ret;
  else {
    if (ret > 2) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Invalid 'width' or 'height' token=", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
    } else {
      (*pop) += ret;
    }
  }
  pfx->pex+=pfx->lenToken;
  return ret;
}

static MagickBooleanType ExtendOperatorStack (fxT * pfx)
{
  pfx->numOprStack = ceil (pfx->numOprStack * (1 + TableExtend));
  pfx->OperatorStack = ResizeMagickMemory (pfx->OperatorStack, pfx->numOprStack * sizeof(OperatorE));
  if (!pfx->OperatorStack) {
    ThrowFatalException(ResourceLimitFatalError, "ExtendOperatorStack oom");
    return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType PushOperatorStack (fxT * pfx, int op)
{
  if (++pfx->usedOprStack >= pfx->numOprStack) {
    if (!ExtendOperatorStack (pfx))
      return MagickFalse;
  }
  pfx->OperatorStack[pfx->usedOprStack-1] = op;

  if (pfx->maxUsedOprStack < pfx->usedOprStack)
    pfx->maxUsedOprStack = pfx->usedOprStack;
  return MagickTrue;
}

static OperatorE GetLeadingOp (fxT * pfx)
{
  OperatorE op = oNull;

  if      (*pfx->pex == '-') op = oUnaryMinus;
  else if (*pfx->pex == '+') op = oUnaryPlus;
  else if (*pfx->pex == '~') op = oBitNot;
  else if (*pfx->pex == '!') op = oLogNot;
  else if (*pfx->pex == '(') op = oOpenParen;

  return op;
}

static MagickBooleanType OprIsUnaryPrefix (fxT * pfx, OperatorE op)
{
  return (op == oUnaryMinus || op == oUnaryPlus || op == oBitNot || op == oLogNot);
}

static MagickBooleanType TopOprIsUnaryPrefix (fxT * pfx)
{
  if (!pfx->usedOprStack) return MagickFalse;

  return OprIsUnaryPrefix (pfx, pfx->OperatorStack[pfx->usedOprStack-1]);
}

static MagickBooleanType PopOprOpenParen (fxT * pfx, OperatorE op)
{

  if (!pfx->usedOprStack) return MagickFalse;

  if (pfx->OperatorStack[pfx->usedOprStack-1] != op) return MagickFalse;

  pfx->usedOprStack--;

  return MagickTrue;
}

static int GetCoordQualifier (fxT * pfx, OperatorE op)
/* Returns -1 if invalid CoordQualifier, +1 if valid and appropriate.
*/
{
  if (op != (OperatorE)fU && op != (OperatorE)fV && op != (OperatorE)fS) return -1;

  GetToken (pfx);

  if (pfx->lenToken != 1) {
    return -1;
  }
  if (*pfx->token != 'p' && *pfx->token != 'P') return -1;
  if (!GetFunction (pfx, fP)) return -1;

  return 1;
}

static PixelChannel GetChannelQualifier (fxT * pfx, OperatorE op)
{
  if (op == (OperatorE)fU || op == (OperatorE)fV || op == (OperatorE)fP || 
      op == (OperatorE)fUP || op == (OperatorE)fVP ||
      op == (OperatorE)fS || (op >= FirstImgAttr && op <= (OperatorE)aNull)
     )
  {
    GetToken (pfx);

    const ChannelT * pch = &Channels[0];
    while (*pch->str) {
      if (LocaleCompare (pch->str, pfx->token)==0) {

        if (op >= FirstImgAttr && op <= (OperatorE)aNull &&
              (pch->pixChan == HUE_CHANNEL ||
               pch->pixChan == SAT_CHANNEL ||
               pch->pixChan == LIGHT_CHANNEL)
           )
        {
          ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Can't have image attribute with HLS qualifier at", "'%s'",
            SetShortExp(pfx));
          return NO_CHAN_QUAL;
        }

        pfx->pex += pfx->lenToken;
        return pch->pixChan;
      }
      pch++;
    }
  }
  return NO_CHAN_QUAL;
}

static ImgAttrE GetImgAttrToken (fxT * pfx)
{
  ImgAttrE ia = aNull;
  const char * iaStr;
  for (ia = FirstImgAttr; ia < aNull; ia++) {
    iaStr = ImgAttrs[ia-FirstImgAttr].str;
    if (LocaleCompare (iaStr, pfx->token)==0) {
      pfx->pex += strlen(pfx->token);
      if (ImgAttrs[ia-FirstImgAttr].NeedStats == 1) pfx->NeedStats = MagickTrue;
      MaybeXYWH (pfx, &ia);
      break;
    }
  }

  if (ia == aPage || ia == aPrintsize || ia == aRes) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Attribute", "'%s' needs qualifier at '%s'",
      iaStr, SetShortExp(pfx));
  }

  return ia;
}

static ImgAttrE GetImgAttrQualifier (fxT * pfx, OperatorE op)
{
  ImgAttrE ia = aNull;
  if (op == (OperatorE)fU || op == (OperatorE)fV || op == (OperatorE)fP || op == (OperatorE)fS) {
    GetToken (pfx);
    if (pfx->lenToken == 0) {
      return aNull;
    }
    ia = GetImgAttrToken (pfx);
  }
  return ia;
}

static MagickBooleanType IsQualifier (fxT * pfx)
{
  if (PeekChar (pfx) == '.') {
    pfx->pex++;
    return MagickTrue;
  }
  return MagickFalse;
}

static int GetProperty (fxT * pfx, fxFltType *val)
/* returns number of character to swallow.
   "-1" means invalid input
   "0" means no relevant input (don't swallow, but not an error)
*/
{

  if (PeekStr (pfx, "%[")) {

    int level = 0;
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
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "After '%[' expected ']' at", "'%s'",
        SetShortExp(pfx));
      return -1;
    }

    int len = p - pfx->pex + 1;
    if (len > MaxTokenLen) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Too much text between '%[' and ']' at", "'%s'",
        SetShortExp(pfx));
      return -1;
    }

    char sProperty [MaxTokenLen];
    CopyMagickString (sProperty, pfx->pex, len+1);
    sProperty[len] = '\0';

    char * text = InterpretImageProperties (pfx->image->image_info, pfx->image,
       sProperty, pfx->exception);

    if (!text) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Unknown property", "'%s' at '%s'",
        sProperty, SetShortExp(pfx));
      return -1;
    }

    char * tailptr;
    *val = strtold (text, &tailptr);
    if (text == tailptr) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Property", "'%s' is not a number at '%s'",
        text, SetShortExp(pfx));
      return -1;
    }

    text = DestroyString(text);
    return (len);
  }

  return 0;
}

static int inline GetConstantColour (fxT * pfx, fxFltType *v0, fxFltType *v1, fxFltType *v2)
/* Finds named colour such as "blue" and colorspace function such as "lab(10,20,30)".
   Returns number of characters to swallow.
*/
{
  PixelInfo
    colour;

  ExceptionInfo
    *dummy_exception = AcquireExceptionInfo ();

  char ColSp[MaxTokenLen];
  CopyMagickString (ColSp, pfx->token, MaxTokenLen);

  char * p = ColSp + pfx->lenToken - 1;
  if (*p == 'a' || *p == 'A') *p = '\0';

  GetPixelInfo (pfx->image, &colour);

  /* "gray" is both a colorspace and a named colour. */

  MagickBooleanType IsGray = (LocaleCompare (ColSp, "gray") == 0);
  MagickBooleanType IsIcc = (LocaleCompare (ColSp, "icc-color") == 0);
  MagickBooleanType IsDev = (LocaleNCompare (ColSp, "device-", 7) == 0);

  /* QueryColorCompliance will raise a warning if it isn't a colour, so we discard any exceptions.
  */
  if (!QueryColorCompliance (pfx->token, AllCompliance, &colour, dummy_exception) || IsGray) {
    int type = ParseCommandOption (MagickColorspaceOptions, MagickFalse, ColSp);
    if (type >= 0 || IsIcc || IsDev) {
      char * p = pfx->pex + pfx->lenToken;
      while (isspace ((int)*p)) p++;
      if (*p == '(') {
        while (*p && *p != ')') p++;
        int lenfun = p - pfx->pex + 1;
        if (lenfun > MaxTokenLen) {
          ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "lenfun too long", "'%i' at '%s'",
            lenfun, SetShortExp(pfx));
          dummy_exception = DestroyExceptionInfo (dummy_exception);
          return 0;
        }
        char sFunc[MaxTokenLen];
        CopyMagickString (sFunc, pfx->pex, lenfun+1);
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

  return strlen (pfx->token);
}

static int inline GetHexColour (fxT * pfx, fxFltType *v0, fxFltType *v1, fxFltType *v2)
/* Returns number of characters to swallow.
   Negative return means it starts with '#', but invalid hex number.
*/
{

  if (*pfx->pex != '#') return 0;

  /* find end of hex digits. */
  char * p = pfx->pex + 1;
  while (isxdigit ((int)*p)) p++;
  if (isalpha ((int)*p)) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Bad hex number at", "'%s'",
      SetShortExp(pfx));
    return -1;
  }

  int len = p - pfx->pex;
  if (len < 1) return 0;
  if (len >= MaxTokenLen) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Hex colour too long at", "'%s'",
      SetShortExp(pfx));
    return -1;
  }
  CopyMagickString (pfx->token, pfx->pex, len+1);

  PixelInfo colour;

  GetPixelInfo (pfx->image, &colour);

  if (!QueryColorCompliance (pfx->token, AllCompliance, &colour, pfx->exception)) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "QueryColorCompliance rejected", "'%s' at '%s'",
      pfx->token, SetShortExp(pfx));
    return -1;
  }

  *v0 = colour.red   / QuantumRange;
  *v1 = colour.green / QuantumRange;
  *v2 = colour.blue  / QuantumRange;

  return len;
}

static MagickBooleanType GetFunction (fxT * pfx, FunctionE fe)
{
  /* A function, so get open-parens, n args, close-parens
  */
  char * funStr = Functions[fe-FirstFunc].str;
  pfx->pex += pfx->lenToken;

  int nArgs = Functions[fe-FirstFunc].nArgs;
  char chLimit = ')';
  char expChLimit = ')';
  char *strLimit = ",)";
  OperatorE pushOp = oOpenParen;

  if (fe == fP) {
    char p = PeekChar (pfx);
    if (p=='{') {
      ExpectChar (pfx, '{');
      pushOp = oOpenBrace;
      strLimit = ",}";
      chLimit = '}';
      expChLimit = '}';
    } else if (p=='[') {
      ExpectChar (pfx, '[');
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
      ExpectChar (pfx, '[');
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
  if (!PushOperatorStack (pfx, pushOp)) return MagickFalse;

  char * pExpStart = pfx->pex;
  int lenExp = 0;

  int FndArgs = 0;
  int ndx0 = NULL_ADDRESS, ndx1 = NULL_ADDRESS, ndx2 = NULL_ADDRESS, ndx3 = NULL_ADDRESS;
  ndx0 = pfx->usedElements;
  if (fe==fDo) {
    AddAddressingElement (pfx, rGoto, NULL_ADDRESS); /* address will be ndx1+1 */
  }
  while (nArgs > 0) {
    int FndOne = 0;
    if (TranslateStatementList (pfx, strLimit, &chLimit)) {
      FndOne = 1;
    } else {
      /* Maybe don't break because other expressions may be not empty. */
      if (!chLimit) break;
      if (fe == fP || fe == fS|| fe == fIf) {
        AddElement (pfx, 0, oNull);
        FndOne = 1;
      } 
    }

    if (strchr (strLimit, chLimit)==NULL) {
      ThrowMagickException (
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
        ndx1 = pfx->usedElements;
        if (fe==fWhile) {
          AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fDo) {
          AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
        } else if (fe==fIf) {
          AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2 + 1 */
          pfx->Elements[pfx->usedElements-1].DoPush = MagickTrue; /* we may need return from if() */
        }
        break;
      case 2:
        ndx2 = pfx->usedElements;
        if (fe==fWhile) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
          AddAddressingElement (pfx, rGoto, ndx0);
        } else if (fe==fDo) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
          AddAddressingElement (pfx, rGoto, ndx0 + 1);
        } else if (fe==fFor) {
          AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx3 */
          pfx->Elements[pfx->usedElements-1].DoPush = MagickTrue; /* we may need return from for() */
          AddAddressingElement (pfx, rZerStk, NULL_ADDRESS);
        } else if (fe==fIf) {
          AddAddressingElement (pfx, rGoto, NULL_ADDRESS); /* address will be ndx3 */
        }
        break;
      case 3:
        if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
          AddAddressingElement (pfx, rGoto, ndx1);
        }
        ndx3 = pfx->usedElements;
        break;
      default:
        break;
    }
    if (chLimit == expChLimit) {
      lenExp = pfx->pex - pExpStart - 1;
      break;
    }
  } /* end while args of a function */
  if (chLimit && chLimit != expChLimit && chLimit != ',' ) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected '%c', found '%c' at '%s'",
      funStr, expChLimit, chLimit ? chLimit : ' ', SetShortExp(pfx));
    return MagickFalse;
  }

  if (fe == fP || fe == fS || fe == fU) {
    while (FndArgs < Functions[fe-FirstFunc].nArgs) {
      AddElement (pfx, 0, oNull);
      FndArgs++;
    }
  }

  if (FndArgs > Functions[fe-FirstFunc].nArgs) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected %i arguments, found '%i' at '%s'",
      funStr, Functions[fe-FirstFunc].nArgs, FndArgs, SetShortExp(pfx));
    return MagickFalse;
  }
  if (FndArgs < Functions[fe-FirstFunc].nArgs) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "For function", "'%s' expected %i arguments, found too few (%i) at '%s'",
      funStr, Functions[fe-FirstFunc].nArgs, FndArgs, SetShortExp(pfx));
    return MagickFalse;
  }
  if (fe != fS && fe != fV && FndArgs == 0 && Functions[fe-FirstFunc].nArgs == 0) {
    /* This is for "rand()" and similar. */
    chLimit = expChLimit;
    if (!ExpectChar (pfx, ')')) return MagickFalse;
  }

  if (chLimit != expChLimit) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "For function", "'%s', arguments don't end with '%c' at '%s'",
      funStr, expChLimit, SetShortExp(pfx));
    return MagickFalse;
  }
  if (!PopOprOpenParen (pfx, pushOp)) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Bug: For function", "'%s' tos not '%s' at '%s'",
      funStr, Operators[pushOp].str, SetShortExp(pfx));
    return MagickFalse;
  }

  MagickBooleanType coordQual = MagickFalse;
  PixelChannel chQual = NO_CHAN_QUAL;
  ImgAttrE iaQual = aNull;

  if (IsQualifier (pfx)) {

    if (fe == fU || fe == fV || fe == fS) {

      coordQual = (GetCoordQualifier (pfx, fe) == 1);

      if (coordQual) {

        /* Remove last element, which should be fP */
        ElementT * pel = &pfx->Elements[pfx->usedElements-1];
        if (pel->oprNum != fP) {
          ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Bug: For function", "'%s' last element not 'p' at '%s'",
            funStr, SetShortExp(pfx));
          return MagickFalse;
        }
        chQual = pel->ChannelQual;
        expChLimit = (pel->IsRelative) ? ']' : '}';
        pfx->usedElements--;
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
      chQual = GetChannelQualifier (pfx, fe);
    }

    if (chQual == NO_CHAN_QUAL && (fe == fU || fe == fV || fe == fS)) {
      /* Note: we don't allow "p.mean" etc. */
      iaQual = GetImgAttrQualifier (pfx, fe);
    }
    if (IsQualifier (pfx) && chQual == NO_CHAN_QUAL && iaQual != aNull) {
      chQual = GetChannelQualifier (pfx, fe);
    }
    if (coordQual && iaQual != aNull) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s', can't have qualifiers 'p' and image attribute '%s' at '%s'",
        funStr, pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!coordQual && chQual == NO_CHAN_QUAL && iaQual == aNull) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s', bad qualifier '%s' at '%s'",
        funStr, pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!coordQual && chQual == CompositePixelChannel && iaQual == aNull) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "For function", "'%s', bad compsite qualifier '%s' at '%s'",
        funStr, pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }

    if (chQual == HUE_CHANNEL || chQual == SAT_CHANNEL || chQual == LIGHT_CHANNEL) {
      pfx->NeedHsl = MagickTrue;

      if (iaQual >= FirstImgAttr && iaQual < aNull) {
        ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Can't have image attribute with HLS qualifier at", "'%s'",
          SetShortExp(pfx));
        return NO_CHAN_QUAL;
      }
    }
  }

  if (fe==fWhile) {
    pfx->Elements[ndx1].EleNdx = ndx2+1;
  } else if (fe==fDo) {
    pfx->Elements[ndx0].EleNdx = ndx1+1;
    pfx->Elements[ndx1].EleNdx = ndx2+1;
  } else if (fe==fFor) {
    pfx->Elements[ndx2].EleNdx = ndx3;
  } else if (fe==fIf) {
    pfx->Elements[ndx1].EleNdx = ndx2 + 1;
    pfx->Elements[ndx2].EleNdx = ndx3;
  } else {
    if (fe == fU && iaQual == aNull) {
      ElementT * pel = &pfx->Elements[pfx->usedElements-1];
      if (pel->type == etConstant && pel->val == 0.0) {
        pfx->usedElements--;
        fe = fU0;
      }
    }
    AddElement (pfx, 0, fe);
    if (fe == fP || fe == fU  || fe == fU0 || fe == fUP ||
        fe == fV || fe == fVP || fe == fS || fe == fSP)
    {
      ElementT * pel = &pfx->Elements[pfx->usedElements-1];
      pel->IsRelative = (expChLimit == ']');
      if (chQual >= 0) pel->ChannelQual = chQual;
      if (iaQual != aNull && (fe == fU || fe == fV || fe == fS)) {
        /* Note: we don't allow "p[2,3].mean" or "p.mean" etc. */
        pel->ImgAttrQual = iaQual;
      }
    }
  }

  if (pExpStart && lenExp) {
    ElementT * pel = &pfx->Elements[pfx->usedElements-1];
    pel->pExpStart = pExpStart;
    pel->lenExp = lenExp;
  }

  if (fe == fDebug)
    pfx->ContainsDebug = MagickTrue;

  return MagickTrue;
}

static MagickBooleanType IsStealth (int op)
{
  return (op == fU0 || op == fUP || op == fSP || op == fVP ||
           (op >= FirstCont && op <= rNull)
         );
}

static MagickBooleanType GetOperand (
  fxT * pfx, MagickBooleanType * UserSymbol, MagickBooleanType * NewUserSymbol, int * UserSymNdx,
  MagickBooleanType * needPopAll)
{

  *NewUserSymbol = *UserSymbol = MagickFalse;
  *UserSymNdx = NULL_ADDRESS;

  SkipSpaces (pfx);
  if (!*pfx->pex) return MagickFalse;
  GetToken (pfx);

  if (pfx->lenToken==0) {

    /* Try '(' or unary prefix
    */
    OperatorE op = GetLeadingOp (pfx);
    if (op==oOpenParen) {
      if (!PushOperatorStack (pfx, op)) return MagickFalse;
      pfx->pex++;
      char chLimit = '\0';
      if (!TranslateExpression (pfx, ")", &chLimit, needPopAll)) {
        ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Empty expression in parentheses at", "'%s'",
          SetShortExp(pfx));
      }
      if (chLimit != ')') {
        ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "'(' but no ')' at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
      /* Top of opr stack should be '('. */
      if (!PopOprOpenParen (pfx, oOpenParen)) {
        ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Bug: tos not '(' at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }
      return MagickTrue;
    } else if (OprIsUnaryPrefix (pfx, op)) {
      if (!PushOperatorStack (pfx, op)) return MagickFalse;
      pfx->pex++;
      SkipSpaces (pfx);
      if (!*pfx->pex) return MagickFalse;

      if (!GetOperand (pfx, UserSymbol, NewUserSymbol, UserSymNdx, needPopAll)) {
        ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "After unary, bad operand at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }

      if (*NewUserSymbol) {
        ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "After unary, NewUserSymbol at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      }

      if (*UserSymbol) {
        AddAddressingElement (pfx, rCopyFrom, *UserSymNdx);
        *UserSymNdx = NULL_ADDRESS;

        *UserSymbol = MagickFalse;
        *NewUserSymbol = MagickFalse;
      }

      GetToken (pfx);
      return MagickTrue;
    } else if (*pfx->pex == '#') {
      fxFltType v0=0, v1=0, v2=0;
      int lenToken = GetHexColour (pfx, &v0, &v1, &v2);
      if (lenToken < 0) {
        ThrowMagickException (
          pfx->exception, GetMagickModule(), OptionError,
          "Bad hex number at", "'%s'",
          SetShortExp(pfx));
        return MagickFalse;
      } else if (lenToken > 0) {
        AddColourElement (pfx, etColourConstant, v0, v1, v2);
        pfx->pex+=lenToken;
      }
      return MagickTrue;
    }

    /* Try a constant number.
    */
    char * tailptr;
    fxFltType val = strtold (pfx->pex, &tailptr);
    if (pfx->pex != tailptr) {
      pfx->pex = tailptr;
      if (*tailptr) {
        /* Could have "prefix" K, Ki, M etc.
           See https://en.wikipedia.org/wiki/Metric_prefix
           and https://en.wikipedia.org/wiki/Binary_prefix
        */
        int Pow = 0.0;
        const char Prefices[] = "yzafpnum.kMGTPEZY";
        char * pSi = strchr (Prefices, *tailptr);
        if (pSi && *pSi != '.') Pow = (pSi - Prefices) * 3 - 24;
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
      AddElement (pfx, val, oNull);
      return MagickTrue;
    }

    val = 0;
    int lenOptArt = GetProperty (pfx, &val);
    if (lenOptArt < 0) return MagickFalse;
    if (lenOptArt > 0) {
      AddElement (pfx, val, oNull);
      pfx->pex += lenOptArt;
      return MagickTrue;
    }

  } /* end of lenToken==0 */

  if (pfx->lenToken > 0) {
    ConstantE ce;
    for (ce = (ConstantE)0; ce < cNull; ce++) {
      const char * ceStr = Constants[ce].str;
      if (LocaleCompare (ceStr, pfx->token)==0) {
        break;
      }
    }

    if (ce != cNull) {
      AddElement (pfx, Constants[ce].val, oNull);
      pfx->pex += pfx->lenToken;
      return MagickTrue;
    }

    FunctionE fe;
    for (fe = FirstFunc; fe < fNull; fe++) {
      char * feStr = Functions[fe-FirstFunc].str;
      if (LocaleCompare (feStr, pfx->token)==0) {
        break;
      }
    }

    if (fe == fV && pfx->ImgListLen < 2) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Symbol 'v' but fewer than two images at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }

    if (IsStealth (fe)) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Function", "'%s' not permitted at '%s'",
        pfx->token, SetShortExp(pfx));
    }

    if (fe == fDo || fe == fFor || fe == fIf || fe == fWhile) {
      *needPopAll = MagickTrue;
    }

    if (fe != fNull) return (GetFunction (pfx, fe));

    /* Try image attribute
    */
    ImgAttrE ia = GetImgAttrToken (pfx);
    if (ia != aNull) {
      fxFltType val = 0;
      AddElement (pfx, val, ia);

      if (ImgAttrs[ia-FirstImgAttr].NeedStats==1) {
        if (IsQualifier (pfx)) {
          PixelChannel chQual = GetChannelQualifier (pfx, ia);
          if (chQual == NO_CHAN_QUAL) {
            ThrowMagickException (
              pfx->exception, GetMagickModule(), OptionError,
              "Bad channel qualifier at", "'%s'",
              SetShortExp(pfx));
            return MagickFalse;
          }
          /* Adjust the element */
          ElementT * pel = &pfx->Elements[pfx->usedElements-1];
          pel->ChannelQual = chQual;
        }
      }
      return MagickTrue;
    }

    /* Try symbol
    */
    SymbolE se;
    for (se = FirstSym; se < sNull; se++) {
      const char * seStr = Symbols[se-FirstSym].str;
      if (LocaleCompare (seStr, pfx->token)==0) {
        break;
      }
    }
    if (se != sNull) {
      fxFltType val = 0;
      AddElement (pfx, val, se);
      pfx->pex += pfx->lenToken;

      if (se==sHue || se==sSaturation || se==sLightness) pfx->NeedHsl = MagickTrue;
      return MagickTrue;
    }

    /* Try constant colour.
    */
    fxFltType v0, v1, v2;
    int ColLen = GetConstantColour (pfx, &v0, &v1, &v2);
    if (ColLen > 0) {
      AddColourElement (pfx, etColourConstant, v0, v1, v2);
      pfx->pex+=ColLen;
      return MagickTrue;
    }

    /* Try user symbols. If it is, don't AddElement yet.
    */
    if (TokenMaybeUserSymbol (pfx)) {
      *UserSymbol = MagickTrue;
      *UserSymNdx = FindUserSymbol (pfx, pfx->token);
      if (*UserSymNdx == NULL_ADDRESS) {
        *UserSymNdx = AddUserSymbol (pfx, pfx->pex, pfx->lenToken); /* so future "CopyFrom" and "CopyTo" works. */
        *NewUserSymbol = MagickTrue;
      } else {
      }
      pfx->pex += pfx->lenToken;

      return MagickTrue;
    }
  }

  ThrowMagickException (
    pfx->exception, GetMagickModule(), OptionError,
    "Expected operand at", "'%s'",
    SetShortExp(pfx));

  return MagickFalse;
}

static MagickBooleanType inline IsRealOperator (OperatorE op)
{
  return (op < oOpenParen || op > oCloseBrace);
}

static MagickBooleanType inline ProcessTernaryOpr (fxT * pfx, TernaryT * ptern)
/* Ternary operator "... ? ... : ..."
   returns false iff we have exception
*/
{

  if (pfx->OperatorStack[pfx->usedOprStack-1] == oQuery) {
    if (ptern->addrQuery != NULL_ADDRESS) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have '?' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    ptern->addrQuery = pfx->usedElements;
    AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS);
    /* address will be one after the Colon address. */
  }
  if (pfx->OperatorStack[pfx->usedOprStack-1] == oColon) {
    if (ptern->addrQuery == NULL_ADDRESS) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Need '?' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    ptern->addrColon = pfx->usedElements;
    pfx->Elements[pfx->usedElements-1].DoPush = MagickTrue;
    AddAddressingElement (pfx, rGoto, NULL_ADDRESS);
    /* address will be after the subexpression */
  }
  return MagickTrue;
}

static MagickBooleanType GetOperator (
  fxT * pfx,
  MagickBooleanType * Assign, MagickBooleanType * Update, MagickBooleanType * IncrDecr,
  TernaryT * ptern)
{
  SkipSpaces (pfx);

  OperatorE op;
  int len = 0;
  for (op = (OperatorE)0; op != oNull; op++) {
    const char * opStr = Operators[op].str;
    len = strlen(opStr);
    if (LocaleNCompare (opStr, pfx->pex, len)==0) {
      break;
    }
  }

  if (!IsRealOperator (op)) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Not a real operator at", "'%s'",
      SetShortExp(pfx));
    return MagickFalse;
  }

  if (op==oNull) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Expected operator at", "'%s'",
      SetShortExp(pfx));
    return MagickFalse;
  }

  *Assign = (op==oAssign);
  *Update = OprInPlace (pfx, op);
  *IncrDecr = (op == oPlusPlus || op == oSubSub);

  /* while top of OperatorStack is not empty and is not open-parens or assign,
       and top of OperatorStack is higher precedence than new op,
     then move top of OperatorStack to Element list.
  */

  while (pfx->usedOprStack > 0) {
    OperatorE top = pfx->OperatorStack[pfx->usedOprStack-1];
    if (top == oOpenParen || top == oAssign || OprInPlace (pfx, top)) break;
    int precTop = Operators[top].precedence;
    int precNew = Operators[op].precedence;
    /* Assume left associativity.
       If right assoc, this would be "<=".
    */
    if (precTop < precNew) break;
    AddElement (pfx, 0, top);
    pfx->usedOprStack--;
  }

  MagickBooleanType DoneIt = MagickFalse;

  /* If new op is close paren, and stack top is open paren,
     remove stack top.
  */
  if (op==oCloseParen) {
    if (pfx->usedOprStack == 0) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Found ')' but nothing on stack at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }

    if (pfx->OperatorStack[pfx->usedOprStack-1] != oOpenParen) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Found ')' but no '(' on stack at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    DoneIt = MagickTrue;
  }

  if (!DoneIt) {
    if (!PushOperatorStack (pfx, op)) return MagickFalse;
  }

  pfx->pex += len;

  return MagickTrue;
}

static MagickBooleanType ResolveTernaryAddresses (fxT * pfx, TernaryT * ptern)
{
  if (ptern->addrQuery == NULL_ADDRESS && ptern->addrColon == NULL_ADDRESS)
    return MagickTrue;

  if (ptern->addrQuery != NULL_ADDRESS && ptern->addrColon != NULL_ADDRESS) {
    pfx->Elements[ptern->addrQuery].EleNdx = ptern->addrColon + 1;
    pfx->Elements[ptern->addrColon].EleNdx = pfx->usedElements;
    ptern->addrQuery = NULL_ADDRESS;
    ptern->addrColon = NULL_ADDRESS;
  } else if (ptern->addrQuery != NULL_ADDRESS) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "'?' with no corresponding ':'", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
  } else if (ptern->addrColon != NULL_ADDRESS) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "':' with no corresponding '?'", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
  }
  return MagickTrue;
}

static MagickBooleanType TranslateExpression (
  fxT * pfx, char * strLimit, char * chLimit, MagickBooleanType * needPopAll)
{
  pfx->teDepth++;

  *chLimit = '\0';

  TernaryT ternary;
  ternary.addrQuery = NULL_ADDRESS;
  ternary.addrColon = NULL_ADDRESS;

  int StartEleNdx = pfx->usedElements-1;
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

  /* There should be only one New per expression (oAssign), but can be many Old.
  */
  MagickBooleanType UserSymbol, NewUserSymbol;
  int UserSymNdx0, UserSymNdx1;

  if (!GetOperand (pfx, &UserSymbol, &NewUserSymbol, &UserSymNdx0, needPopAll)) return MagickFalse;
  SkipSpaces (pfx);

  MagickBooleanType
    Assign = MagickFalse,
    Update = MagickFalse,
    IncrDecr = MagickFalse;

  /* Loop through Operator, Operand, Operator, Operand, ...
  */
  while (*pfx->pex && (!*strLimit || (strchr(strLimit,*pfx->pex)==NULL))) {
    if (!GetOperator (pfx, &Assign, &Update, &IncrDecr, &ternary)) return MagickFalse;
    SkipSpaces (pfx);
    if (NewUserSymbol && !Assign) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Expected assignment after new UserSymbol", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!UserSymbol && Assign) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Attempted assignment to non-UserSymbol", "'%s' at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (!UserSymbol && Update) {
      ThrowMagickException (
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
      AddAddressingElement (pfx, rCopyFrom, UserSymNdx0);
      UserSymNdx0 = NULL_ADDRESS;
      ElementT * pel = &pfx->Elements[pfx->usedElements-1];
      pel->DoPush = MagickTrue;
    }

    if (UserSymbol) {
      while (TopOprIsUnaryPrefix (pfx)) {
        OperatorE op = pfx->OperatorStack[pfx->usedOprStack-1];
        AddElement (pfx, 0, op);
        pfx->usedOprStack--;
      }
    }

    if (!ProcessTernaryOpr (pfx, &ternary)) return MagickFalse;

    if (ternary.addrColon != NULL_ADDRESS) {
      if (!TranslateExpression (pfx, ",);", chLimit, needPopAll)) return MagickFalse;
      break;
    }

    UserSymbol = NewUserSymbol = MagickFalse;

    if (!*pfx->pex) break;
    if (*strLimit && (strchr(strLimit,*pfx->pex)!=NULL) ) break;

    if (IncrDecr) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "'++' and '--' must be the final operators in an expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }

    if (!GetOperand (pfx, &UserSymbol, &NewUserSymbol, &UserSymNdx1, needPopAll)) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Expected operand at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    SkipSpaces (pfx);
    if (NewUserSymbol && !Assign) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "NewUserSymbol", "'%s' after non-assignment operator at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    if (UserSymbol && !NewUserSymbol) {
      AddAddressingElement (pfx, rCopyFrom, UserSymNdx1);
      UserSymNdx1 = NULL_ADDRESS;
    }
    UserSymNdx0 = UserSymNdx1;
  }

  if (UserSymbol && !Assign && !Update && UserSymNdx0 != NULL_ADDRESS) {
    if (NewUserSymbol) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "NewUserSymbol", "'%s' needs assignment operator at '%s'",
        pfx->token, SetShortExp(pfx));
      return MagickFalse;
    }
    AddAddressingElement (pfx, rCopyFrom, UserSymNdx0);
    ElementT * pel = &pfx->Elements[pfx->usedElements-1];
    pel->DoPush = MagickTrue;
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
    if ( (op==oAssign && !Assign) || (OprInPlace(pfx, op) && !Update) ) {
      break;
    }
    pfx->usedOprStack--;
    AddElement (pfx, 0, op);
    if (op == oAssign) {
      /* Adjust last element, by deletion and add.
      */
      pfx->usedElements--;
      int addr = UserSymNdx0;
      AddAddressingElement (pfx, rCopyTo, addr);
      break;
    } else if (OprInPlace (pfx, op)) {
      int addr = UserSymNdx0;
      /* Modify latest element.
      */
      pfx->Elements[pfx->usedElements-1].EleNdx = addr;
      break;
    }
  }

  ResolveTernaryAddresses (pfx, &ternary);

  pfx->teDepth--;

  if (!pfx->teDepth && *needPopAll) {
    AddAddressingElement (pfx, rZerStk, NULL_ADDRESS);
    *needPopAll = MagickFalse;
  }

  return MagickTrue;
}


static MagickBooleanType TranslateStatement (fxT * pfx, char * strLimit, char * chLimit)
{
  SkipSpaces (pfx);

  if (!*pfx->pex) return MagickFalse;

  MagickBooleanType NeedPopAll = MagickFalse;

  if (!TranslateExpression (pfx, strLimit, chLimit, &NeedPopAll)) {
    return MagickFalse;
  }

  if (pfx->usedElements && *chLimit==';') {
    /* FIXME: not necessarily the last element,
       but the last _executed_ element, eg "goto" in a "for()"., 
       Pending a fix, we will use rZerStk.
    */
    ElementT * pel = &pfx->Elements[pfx->usedElements-1];
    if (pel->DoPush) pel->DoPush = MagickFalse;
  }

  return MagickTrue;
}

static MagickBooleanType TranslateStatementList (fxT * pfx, char * strLimit, char * chLimit)
{
  SkipSpaces (pfx);

  if (!*pfx->pex) return MagickFalse;

#define MAX_SLIMIT 10
  char sLimits[MAX_SLIMIT];
  CopyMagickString (sLimits, strLimit, MAX_SLIMIT-1);

  if (strchr(strLimit,';')==NULL)
    ConcatenateMagickString (sLimits, ";", MAX_SLIMIT);

  for (;;) {
    if (!TranslateStatement (pfx, sLimits, chLimit)) return MagickFalse;

    if (!*pfx->pex) break;

    if (*chLimit != ';') {
      break;
    }
  }

  return MagickTrue;
}

/*--------------------------------------------------------------------
   Run-time
*/

static MagickBooleanType CollectStatistics (fxT * pfx)
{

  pfx->statistics = AcquireMagickMemory (pfx->ImgListLen * sizeof (ChannelStatistics *));
  if (!pfx->statistics) {
    ThrowFatalException(ResourceLimitFatalError, "statistics oom");
    return MagickFalse;
  }

  Image * img = GetFirstImageInList (pfx->image);

  int imgNum=0;
  for (;;) {
    ChannelStatistics * cs = GetImageStatistics (img, pfx->exception);
    pfx->statistics[imgNum] = cs;
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

    if (++imgNum == pfx->ImgListLen) break;
    img = GetNextImageInList (img);
    assert (img);
  }
  return MagickTrue;
}

static MagickBooleanType inline PushVal (fxT * pfx, fxRtT * pfxrt, fxFltType val, int addr)
{
  if (pfxrt->usedValStack >=pfxrt->numValStack) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "ValStack overflow at addr=", "%i",
      addr);
    return MagickFalse;
  }

  pfxrt->ValStack[pfxrt->usedValStack++] = val;
  return MagickTrue;
}

static inline fxFltType PopVal (fxT * pfx, fxRtT * pfxrt, int addr)
{
  if (pfxrt->usedValStack <= 0) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "ValStack underflow at addr=", "%i",
      addr);
    return 0;
  }

  return pfxrt->ValStack[--pfxrt->usedValStack];
}

static fxFltType inline ImageStat (
  fxT * pfx, int ImgNum, PixelChannel channel, ImgAttrE ia)
{
  assert (channel >= 0 && channel <= MaxPixelChannels);

  ChannelStatistics * cs = NULL;

  if (pfx->NeedStats) cs = pfx->statistics[ImgNum];

  switch (ia) {
    case aDepth:
      return GetImageDepth (pfx->Images[ImgNum], pfx->exception);
      break;
    case aExtent:
      return GetBlobSize (pfx->image);
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
      return pfx->Images[ImgNum]->page.width;
    case aPageHt:
      return pfx->Images[ImgNum]->page.height;
    case aPageX:
      return pfx->Images[ImgNum]->page.x;
    case aPageY:
      return pfx->Images[ImgNum]->page.y;
    case aPrintsize:
      /* Do nothing */
      break;
    case aPrintsizeX:
      return PerceptibleReciprocal (pfx->Images[ImgNum]->resolution.x) * pfx->Images[ImgNum]->columns;
    case aPrintsizeY:
      return PerceptibleReciprocal (pfx->Images[ImgNum]->resolution.y) * pfx->Images[ImgNum]->rows;
    case aQuality:
      return pfx->Images[ImgNum]->quality;
    case aRes:
      /* Do nothing */
      break;
    case aResX:
      return pfx->Images[ImgNum]->resolution.x;
    case aResY:
      return pfx->Images[ImgNum]->resolution.y;
    case aSkewness:
      return cs[channel].skewness;
    case aStdDev:
      return cs[channel].standard_deviation;
    case aH:
      return pfx->Images[ImgNum]->rows;
    case aN:
      return pfx->ImgListLen;
    case aT: /* image index in list */
      return ImgNum;
    case aW:
      return pfx->Images[ImgNum]->columns;
    case aZ:
      return GetImageDepth (pfx->Images[ImgNum], pfx->exception);
      break;
    default:
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
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

static int inline ChkImgNum (fxT * pfx, fxFltType f)
{
  int i = floor (f + 0.5);
  if (i < 0) i += pfx->ImgListLen;
  if (i < 0 || i >= pfx->ImgListLen) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "ImgNum", "%i bad for ImgListLen %i",
      i, pfx->ImgListLen);
    
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

static fxFltType GetHslFlt (fxT * pfx, int ImgNum, const fxFltType fx, const fxFltType fy,
  int channel)
{
  Image * img = pfx->Images[ImgNum];

  double red, green, blue;

  MagickBooleanType okay = MagickTrue;
  if(!InterpolatePixelChannel (img, pfx->Views[ImgNum], RedPixelChannel, img->interpolate,
    fx, fy, &red, pfx->exception)) okay = MagickFalse;
  if(!InterpolatePixelChannel (img, pfx->Views[ImgNum], GreenPixelChannel, img->interpolate,
    fx, fy, &green, pfx->exception)) okay = MagickFalse;
  if(!InterpolatePixelChannel (img, pfx->Views[ImgNum], BluePixelChannel, img->interpolate,
    fx, fy, &blue, pfx->exception)) okay = MagickFalse;

  if (!okay)
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
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

static fxFltType GetHslInt (fxT * pfx, int ImgNum, const ssize_t imgx, const ssize_t imgy, int channel)
{
  Image * img = pfx->Images[ImgNum];

  const Quantum * p = GetCacheViewVirtualPixels (pfx->Views[ImgNum], imgx, imgy, 1, 1, pfx->exception);
  if (!p)
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "GetHslInt failure", "%i %li,%li %i", ImgNum, imgx, imgy, channel);

  double hue=0, saturation=0, lightness=0;

  ConvertRGBToHSL (
    GetPixelRed (img, p), GetPixelGreen (img, p), GetPixelBlue (img, p),
    &hue, &saturation, &lightness);

  if (channel == HUE_CHANNEL)   return hue;
  if (channel == SAT_CHANNEL)   return saturation;
  if (channel == LIGHT_CHANNEL) return lightness;

  return 0;
}

static fxFltType inline GetIntensity (fxT * pfx, int ImgNum, const ssize_t imgx, const ssize_t imgy)
{
  Quantum
    quantum_pixel[MaxPixelChannels];

  PixelInfo
    pixelinf;

  Image * img = pfx->Images[ImgNum];

  GetPixelInfo (img, &pixelinf);

  if (!InterpolatePixelInfo (img, pfx->Views[pfx->ImgNum], img->interpolate,
              imgx, imgy, &pixelinf, pfx->exception))
  {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "GetIntensity failure", "%i %li,%li", ImgNum, imgx, imgy);
  }

  SetPixelViaPixelInfo (img, &pixelinf, quantum_pixel);
  return QuantumScale * GetPixelIntensity (img, quantum_pixel);
}

static MagickBooleanType ExecuteRPN (fxT * pfx, fxRtT * pfxrt, FILE * fh, fxFltType *result,
  const PixelChannel channel, const ssize_t imgx, const ssize_t imgy)
{

  const Quantum * p = pfxrt->thisPixel;

  /* For -fx, this sets p to ImgNum 0.
     for %[fx:...], this sets p to the currrent image.
     Similarly img.
  */
  if (!p) p = GetCacheViewVirtualPixels (
    pfx->Views[pfx->ImgNum], imgx, imgy, 1, 1, pfx->exception);

  fxFltType regA=0, regB=0, regC=0, regD=0, regE=0;
  Image * img = pfx->image;
  ChannelStatistics * cs = NULL;
  if (pfx->NeedStats) {
    cs = pfx->statistics[pfx->ImgNum];
  }

  /*  Folllowing is only for expressions like "saturation", with no image specifier.
  */
  double hue=0, saturation=0, lightness=0;
  if (pfx->NeedHsl) {
    ConvertRGBToHSL (
      GetPixelRed (img, p), GetPixelGreen (img, p), GetPixelBlue (img, p),
      &hue, &saturation, &lightness);
  }

  int i;
  for (i=0; i < pfx->usedElements; i++) {
    ElementT *pel = &pfx->Elements[i];
      switch (pel->nArgs) {
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
          ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "Too many args:", "%i", pel->nArgs);
          break;
      }

      switch (pel->oprNum) {
        case oAddEq:
          regA = (pfxrt->UserSymVals[pel->EleNdx] += regA);
          break;
        case oSubtractEq:
          regA = (pfxrt->UserSymVals[pel->EleNdx] -= regA);
          break;
        case oMultiplyEq:
          regA = (pfxrt->UserSymVals[pel->EleNdx] *= regA);
          break;
        case oDivideEq:
          if (regA != 0) regA = (pfxrt->UserSymVals[pel->EleNdx] /= regA);
          break;
        case oPlusPlus:
          regA = pfxrt->UserSymVals[pel->EleNdx]++;
          break;
        case oSubSub:
          regA = pfxrt->UserSymVals[pel->EleNdx]--;
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

          fprintf (stderr, "%s[%li,%li].%i: %s=%.*Lg\n",
                   img->filename, imgx, imgy,
                   channel, SetPtrShortExp (pfx, pel->pExpStart, pel->lenExp+1),
                   pfx->precision, regA);
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
          regA = GetPseudoRandomValue (pfxrt->random_info);
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
          int ImgNum = ChkImgNum (pfx, regA);
          regA = 0;
          if (ImgNum == 0) {
            Image * pimg = pfx->Images[0];
            int pech = (int)pel->ChannelQual;
            if (pel->ImgAttrQual == aNull) {
              if (pech < 0) {
                if (pech == NO_CHAN_QUAL || pech == THIS_CHANNEL) {
                  if (pfx->ImgNum==0) {
                    regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                  } else {
                    const Quantum * pv = GetCacheViewVirtualPixels (
                                   pfx->Views[0], imgx, imgy, 1,1, pfx->exception);
                    if (!pv) {
                      ThrowMagickException (
                        pfx->exception, GetMagickModule(), OptionError,
                        "fU can't get cache", "%i", ImgNum);
                      break;
                    }
                    regA = QuantumScale * pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                  }
                } else if (pech == HUE_CHANNEL || pech == SAT_CHANNEL ||
                    pech == LIGHT_CHANNEL) {
                  regA = GetHslInt (pfx, ImgNum, imgx, imgy, pech);
                  break;
                } else if (pech == INTENSITY_CHANNEL) {
                  regA = GetIntensity (pfx, 0, imgx, imgy);
                }
              } else {
                if (pfx->ImgNum==0) {
                  regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                } else {
                  const Quantum * pv = GetCacheViewVirtualPixels (
                                 pfx->Views[0], imgx, imgy, 1,1, pfx->exception);
                  if (!pv) {
                    ThrowMagickException (
                      pfx->exception, GetMagickModule(), OptionError,
                      "fU can't get cache", "%i", ImgNum);
                    break;
                  }
                  regA = QuantumScale * pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
                }
              }
            } else {
              /* we have an image atttribute */
              regA = ImageStat (pfx, 0, WHICH_ATTR_CHAN, pel->ImgAttrQual);
            }
          } else {
            /* We have non-zero ImgNum. */
            if (pel->ImgAttrQual == aNull) {
              if ((int)pel->ChannelQual < 0) {
                if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL ||
                    pel->ChannelQual == LIGHT_CHANNEL)
                {
                  regA = GetHslInt (pfx, ImgNum, imgx, imgy, pel->ChannelQual);
                  break;
                } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
                  regA = GetIntensity (pfx, ImgNum, imgx, imgy);
                  break;
                }
              }

              const Quantum * pv = GetCacheViewVirtualPixels (
                                   pfx->Views[ImgNum], imgx, imgy, 1,1, pfx->exception);
              if (!pv) {
                ThrowMagickException (
                  pfx->exception, GetMagickModule(), OptionError,
                  "fU can't get cache", "%i", ImgNum);
                break;
              }
              regA = QuantumScale *
         pv[pfx->Images[ImgNum]->channel_map[WHICH_NON_ATTR_CHAN].offset];
            } else {
              regA = ImageStat (pfx, ImgNum, WHICH_ATTR_CHAN, pel->ImgAttrQual);
            }
          }
          break;
        }
        case fU0: {
          /* No args. No image attribute. We may have a ChannelQual.
             If called from %[fx:...], ChannelQual will be CompositePixelChannel.
          */
          Image * pimg = pfx->Images[0];
          int pech = (int)pel->ChannelQual;
          if (pech < 0) {
            if (pech == NO_CHAN_QUAL || pech == THIS_CHANNEL) {

              if (pfx->ImgNum==0) {
                regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
              } else {
                const Quantum * pv = GetCacheViewVirtualPixels (
                               pfx->Views[0], imgx, imgy, 1,1, pfx->exception);
                if (!pv) {
                  ThrowMagickException (
                    pfx->exception, GetMagickModule(), OptionError,
                    "fU0 can't get cache", "%i", 0);
                  break;
                }
                regA = QuantumScale * pv[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
              }

            } else if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL ||
                       pel->ChannelQual == LIGHT_CHANNEL) {
              regA = GetHslInt (pfx, 0, imgx, imgy, pel->ChannelQual);
              break;
            } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
              regA = GetIntensity (pfx, 0, imgx, imgy);
            }
          } else {
            if (pfx->ImgNum==0) {
              regA = QuantumScale * p[pimg->channel_map[WHICH_NON_ATTR_CHAN].offset];
            } else {
              const Quantum * pv = GetCacheViewVirtualPixels (
                                   pfx->Views[0], imgx, imgy, 1,1, pfx->exception);
              if (!pv) {
                ThrowMagickException (
                  pfx->exception, GetMagickModule(), OptionError,
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
          int ImgNum = ChkImgNum (pfx, regA);

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
              regA = GetHslFlt (pfx, ImgNum, fx, fy, pel->ChannelQual);
              break;
            } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
              regA = GetIntensity (pfx, ImgNum, fx, fy);
              break;
            }
          }

          double v;
          Image * imUP = pfx->Images[ImgNum];
          if (! InterpolatePixelChannel (imUP, pfx->Views[ImgNum], WHICH_NON_ATTR_CHAN,
                  imUP->interpolate, fx, fy, &v, pfx->exception))
          {
            ThrowMagickException (
              pfx->exception, GetMagickModule(), OptionError,
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
          if (pel->oprNum == fS) ImgNum = pfx->ImgNum;

          if (pel->ImgAttrQual == aNull) {
            const Quantum * pv = GetCacheViewVirtualPixels (
                                   pfx->Views[ImgNum], imgx, imgy, 1,1, pfx->exception);
            if (!pv) {
              ThrowMagickException (
                pfx->exception, GetMagickModule(), OptionError,
                "fV can't get cache", "%i", ImgNum);
              break;
            }

            if ((int)pel->ChannelQual < 0) {
              if (pel->ChannelQual == HUE_CHANNEL || pel->ChannelQual == SAT_CHANNEL ||
                  pel->ChannelQual == LIGHT_CHANNEL) {
                regA = GetHslInt (pfx, ImgNum, imgx, imgy, pel->ChannelQual);
                break;
              } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
                regA = GetIntensity (pfx, ImgNum, imgx, imgy);
                break;
              }
            }

            regA = QuantumScale *
         pv[pfx->Images[ImgNum]->channel_map[WHICH_NON_ATTR_CHAN].offset];
          } else {
            regA = ImageStat (pfx, ImgNum, WHICH_ATTR_CHAN, pel->ImgAttrQual);
          }

          break;
        }
        case fP:
        case fSP:
        case fVP: {
          /* 2 args are: x, y */
          int ImgNum = ImgNum = pfx->ImgNum;
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
              regA = GetHslFlt (pfx, ImgNum, fx, fy, pel->ChannelQual);
              break;
            } else if (pel->ChannelQual == INTENSITY_CHANNEL) {
              regA = GetIntensity (pfx, ImgNum, fx, fy);
            }
          }

          double v;

          if (! InterpolatePixelChannel (pfx->Images[ImgNum], pfx->Views[ImgNum],
                                         WHICH_NON_ATTR_CHAN, pfx->Images[ImgNum]->interpolate,
                                         fx, fy, &v, pfx->exception)
                                        )
          {
            ThrowMagickException (
              pfx->exception, GetMagickModule(), OptionError,
              "fSP or fVP can't get interp", "%i", ImgNum);
            break;
          }
          regA = v * (fxFltType)QuantumScale;

          break;
        }
        case fNull:
          break;
        case aDepth:
          regA = GetImageDepth (img, pfx->exception) / QuantumRange;
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
          regA = pfx->ImgListLen;
          break;
        case aT: /* image index in list */
          regA = pfx->ImgNum;
          break;
        case aW: /* image->columns */
          regA = img->columns;
          break;
        case aZ: /* image depth */
          regA = GetImageDepth (img, pfx->exception);
          break;
        case aNull:
          break;
        case sHue: /* of conversion to HSL */
          regA = hue;
          break;
        case sIntensity:
          regA = GetIntensity (pfx, pfx->ImgNum, imgx, imgy);
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
          regA = pfxrt->UserSymVals[pel->EleNdx];
          break;
        case rCopyTo:
          pfxrt->UserSymVals[pel->EleNdx] = regA;
          break;
        case rZerStk:
          pfxrt->usedValStack = 0;
          break;
        case rNull:
          break;

        default:
          ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "pel->oprNum", "%i '%s' not yet implemented",
            (int)pel->oprNum, OprStr(pfx, pel->oprNum));
          break;
    }
    if (i < 0) {
      ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Bad run-time address", "%i", i);
    }
    if (pel->DoPush) 
      if (!PushVal (pfx, pfxrt, regA, i)) break;
  }

  if (pfxrt->usedValStack > 0) regA = PopVal (pfx, pfxrt, 9999);

  *result = regA;

  if (pfxrt->usedValStack != 0) {
    fprintf (stderr, "ValStack not empty (%i)\n", pfxrt->usedValStack);
    return MagickFalse;
  }

  return MagickTrue;
}

/* Following is substitute for FxEvaluateChannelExpression().
*/
MagickBooleanType FxEvaluateChannelExpression (
  fxT *pfx,
  const PixelChannel channel, const ssize_t x, const ssize_t y,
  double *result, ExceptionInfo *exception)
{
  assert (pfx != NULL);
  assert (pfx->image != NULL);
  assert (pfx->Images != NULL);
  assert (pfx->Views != NULL);
  assert (pfx->fxrts != NULL);

  const int
    id = GetOpenMPThreadId();

  pfx->fxrts[id].thisPixel = NULL;

  fxFltType ret;

  if (!ExecuteRPN (pfx, &pfx->fxrts[id], stderr, &ret, channel, x, y)) {
    fprintf (stderr, "ExecuteRPN failed\n");
    return MagickFalse;
  }

  *result = ret;

  return MagickTrue;
}

fxT *AcquireFxInfo (const Image * images, const char * expression, ExceptionInfo *exception)
{
  fxT * pfx = AcquireMagickMemory (sizeof (*pfx));

  memset (pfx, 0, sizeof (*pfx));

  if (!InitFx (pfx, images, exception)) return NULL;

  if (!BuildRPN (pfx)) return NULL;

  pfx->pex = (char *)expression;

  if (*expression == '@')
    pfx->expression = FileToString (expression+1, ~0UL, exception);
  else
    pfx->expression = ConstantString (expression);

  pfx->pex = pfx->expression;

  pfx->teDepth = 0;
  char chLimit;
  if (!TranslateStatementList (pfx, ";", &chLimit)) return NULL;

  if (pfx->teDepth) {
    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "Translate expression depth", "(%i) not 0",
      pfx->teDepth);
    return NULL;
  }

  if (chLimit != '\0' && chLimit != ';') {
    fprintf (stderr, "AcquireFxInfo: TranslateExpression did not exhaust input '%s' chLimit='%c'\n",
             pfx->pex, chLimit ? chLimit : ' ');

    ThrowMagickException (
      pfx->exception, GetMagickModule(), OptionError,
      "AcquireFxInfo: TranslateExpression did not exhaust input at", "'%s'",
      pfx->pex);
    return NULL;
  }

  if (pfx->NeedStats && !pfx->statistics) {
    if (!CollectStatistics (pfx)) return MagickFalse;
  }

  if (pfx->DebugOpt) {
    DumpTables (pfx, stderr);
    DumpUserSymbols (pfx, stderr);
    DumpRPN (pfx, stderr);
  }

  size_t number_threads=(size_t) GetMagickResourceLimit(ThreadResource);

  pfx->fxrts = (fxRtT *)AcquireQuantumMemory (number_threads, sizeof(fxRtT));
  if (!pfx->fxrts) {
    ThrowFatalException(ResourceLimitFatalError, "fxrts oom");
    return NULL;
  }
  int t;
  for (t=0; t < number_threads; t++) {
    if (!AllocFxRt (pfx, &pfx->fxrts[t])) {
      ThrowFatalException(ResourceLimitFatalError, "AllocFxRt");
      return NULL;
    }
  }

  return pfx;
}

fxT *DestroyFxInfo (fxT * pfx)
{
  assert (pfx != NULL);
  assert (pfx->image != NULL);
  assert (pfx->Images != NULL);
  assert (pfx->Views != NULL);
  assert (pfx->fxrts != NULL);

  int t;
  for (t=0; t < GetMagickResourceLimit(ThreadResource); t++) {
    DestroyFxRt (&pfx->fxrts[t]);
  }
  pfx->fxrts = (fxRtT *) RelinquishMagickMemory (pfx->fxrts);

  DestroyRPN (pfx);

  pfx->expression = DestroyString (pfx->expression);
  pfx->pex = NULL;

  DeInitFx (pfx);

  pfx = RelinquishMagickMemory(pfx);

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

  fxT * pfx = AcquireFxInfo (image, expression, exception);

  if (!pfx) return NULL;
  assert (pfx->image != NULL);
  assert (pfx->Images != NULL);
  assert (pfx->Views != NULL);
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

        if (!ExecuteRPN (pfx, &pfx->fxrts[id], stderr, &result, channel, x, y)) {
          status=MagickFalse;
          continue;
        }

        q[i] = ClampToQuantum (QuantumRange*result);
      }
      p+=GetPixelChannels (image);
      q+=GetPixelChannels (fx_image);
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
    fprintf (stderr, "User symbols (%i):\n", pfx->usedUserSymbols);
    int t, i;
    char UserSym[MaxTokenLen];
    for (t=0; t < GetMagickResourceLimit(ThreadResource); t++) {
      for (i = 0; i < pfx->usedUserSymbols; i++) {
        fprintf (stderr, "th=%i us=%i '%s': %.*Lg\n",
                 t, i, NameOfUserSym (pfx, i, UserSym), pfx->precision, pfx->fxrts[t].UserSymVals[i]);
      }
    }
  }

  if (pfx->exception->severity != UndefinedException) {
    status = MagickFalse;
  }

  if (status == MagickFalse)
    fx_image = DestroyImage (fx_image);

  pfx = DestroyFxInfo (pfx);

  return(fx_image);
}

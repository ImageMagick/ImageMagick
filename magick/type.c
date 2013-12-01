/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        TTTTT  Y   Y  PPPP   EEEEE                           %
%                          T     Y Y   P   P  E                               %
%                          T      Y    PPPP   EEE                             %
%                          T      Y    P      E                               %
%                          T      Y    P      EEEEE                           %
%                                                                             %
%                                                                             %
%                       MagickCore Image Type Methods                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 May 2001                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/blob.h"
#include "magick/client.h"
#include "magick/configure.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/type.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"
#if defined(MAGICKCORE_FONTCONFIG_DELEGATE)
# include "fontconfig/fontconfig.h"
#if (FC_VERSION < 20209)
#undef FC_WEIGHT_LIGHT
#define FC_WIDTH                  "width"    /* Int */
#define FC_WIDTH_ULTRACONDENSED    50
#define FC_WIDTH_EXTRACONDENSED    63
#define FC_WIDTH_CONDENSED         75
#define FC_WIDTH_SEMICONDENSED     87
#define FC_WIDTH_NORMAL            100
#define FC_WIDTH_SEMIEXPANDED      113
#define FC_WIDTH_EXPANDED          125
#define FC_WIDTH_EXTRAEXPANDED     150
#define FC_WIDTH_ULTRAEXPANDED     200

#define FC_WEIGHT_THIN             0
#define FC_WEIGHT_EXTRALIGHT       40
#define FC_WEIGHT_ULTRALIGHT       FC_WEIGHT_EXTRALIGHT
#define FC_WEIGHT_LIGHT            50
#define FC_WEIGHT_BOOK             75
#define FC_WEIGHT_REGULAR          80
#define FC_WEIGHT_NORMAL           FC_WEIGHT_REGULAR
#define FC_WEIGHT_MEDIUM           100
#define FC_WEIGHT_DEMIBOLD         180
#define FC_WEIGHT_SEMIBOLD         FC_WEIGHT_DEMIBOLD
#define FC_WEIGHT_BOLD             200
#define FC_WEIGHT_EXTRABOLD        205
#define FC_WEIGHT_ULTRABOLD        FC_WEIGHT_EXTRABOLD
#define FC_WEIGHT_BLACK            210
#define FC_WEIGHT_HEAVY            FC_WEIGHT_BLACK
#endif
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
# include "magick/nt-feature.h"
#endif

/*
  Define declarations.
*/
#define MagickTypeFilename  "type.xml"

/*
  Declare type map.
*/
static const char
  *TypeMap = (const char *)
    "<?xml version=\"1.0\"?>"
    "<typemap>"
    "  <type stealth=\"True\" name=\"fixed\" family=\"helvetica\"/>"
    "  <type stealth=\"True\" name=\"helvetica\" family=\"helvetica\"/>"
    "</typemap>";

/*
  Static declarations.
*/
static SemaphoreInfo
  *type_semaphore = (SemaphoreInfo *) NULL;

static volatile MagickBooleanType
  instantiate_type = MagickFalse;

static SplayTreeInfo
  *type_list = (SplayTreeInfo *) NULL;

/*
  Forward declarations.
*/
static MagickBooleanType
  InitializeTypeList(ExceptionInfo *),
  LoadTypeLists(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t T y p e I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetTypeInfo searches the type list for the specified name and if found
%  returns attributes for that type.
%
%  The format of the GetTypeInfo method is:
%
%      const TypeInfo *GetTypeInfo(const char *name,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the type name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const TypeInfo *GetTypeInfo(const char *name,
  ExceptionInfo *exception)
{
  assert(exception != (ExceptionInfo *) NULL);
  if ((type_list == (SplayTreeInfo *) NULL) ||
      (instantiate_type == MagickFalse))
    if (InitializeTypeList(exception) == MagickFalse)
      return((const TypeInfo *) NULL);
  if ((type_list == (SplayTreeInfo *) NULL) ||
      (GetNumberOfNodesInSplayTree(type_list) == 0))
    return((const TypeInfo *) NULL);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    {
      ResetSplayTreeIterator(type_list);
      return((const TypeInfo *) GetNextValueInSplayTree(type_list));
    }
  return((const TypeInfo *) GetValueFromSplayTree(type_list,name));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t T y p e I n f o B y F a m i l y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetTypeInfoByFamily() searches the type list for the specified family and if
%  found returns attributes for that type.
%
%  Type substitution and scoring algorithm contributed by Bob Friesenhahn.
%
%  The format of the GetTypeInfoByFamily method is:
%
%      const TypeInfo *GetTypeInfoByFamily(const char *family,
%        const StyleType style,const StretchType stretch,
%        const size_t weight,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o family: the type family.
%
%    o style: the type style.
%
%    o stretch: the type stretch.
%
%    o weight: the type weight.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMax(const size_t x,
  const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline size_t MagickMin(const size_t x,
  const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport const TypeInfo *GetTypeInfoByFamily(const char *family,
  const StyleType style,const StretchType stretch,const size_t weight,
  ExceptionInfo *exception)
{
  typedef struct _Fontmap
  {
    const char
      *name,
      *substitute;
  } Fontmap;

  const TypeInfo
    *type_info;

  register const TypeInfo
    *p;

  register ssize_t
    i;

  ssize_t
    range;

  static const Fontmap
    fontmap[] =
    {
      { "fixed", "courier" },
      { "modern","courier" },
      { "monotype corsiva", "courier" },
      { "news gothic", "helvetica" },
      { "system", "courier" },
      { "terminal", "courier" },
      { "wingdings", "symbol" },
      { NULL, NULL }
    };

  size_t
    max_score,
    score;

  /*
    Check for an exact type match.
  */
  (void) GetTypeInfo("*",exception);
  if (type_list == (SplayTreeInfo *) NULL)
    return((TypeInfo *) NULL);
  LockSemaphoreInfo(type_semaphore);
  ResetSplayTreeIterator(type_list);
  type_info=(const TypeInfo *) NULL;
  p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
  while (p != (const TypeInfo *) NULL)
  {
    if (p->family == (char *) NULL)
      {
        p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
        continue;
      }
    if (family == (const char *) NULL)
      {
        if ((LocaleCompare(p->family,"arial") != 0) &&
            (LocaleCompare(p->family,"helvetica") != 0))
          {
            p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
            continue;
          }
      }
    else
      if (LocaleCompare(p->family,family) != 0)
        {
          p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
          continue;
        }
    if ((style != UndefinedStyle) && (style != AnyStyle) && (p->style != style))
      {
        p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
        continue;
      }
    if ((stretch != UndefinedStretch) && (stretch != AnyStretch) &&
        (p->stretch != stretch))
      {
        p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
        continue;
      }
    if ((weight != 0) && (p->weight != weight))
      {
        p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
        continue;
      }
    type_info=p;
    break;
  }
  UnlockSemaphoreInfo(type_semaphore);
  if (type_info != (const TypeInfo *) NULL)
    return(type_info);
  /*
    Check for types in the same family.
  */
  max_score=0;
  LockSemaphoreInfo(type_semaphore);
  ResetSplayTreeIterator(type_list);
  p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
  while (p != (const TypeInfo *) NULL)
  {
    if (p->family == (char *) NULL)
      {
        p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
        continue;
      }
    if (family == (const char *) NULL)
      {
        if ((LocaleCompare(p->family,"arial") != 0) &&
            (LocaleCompare(p->family,"helvetica") != 0))
          {
            p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
            continue;
          }
      }
    else
      if (LocaleCompare(p->family,family) != 0)
        {
          p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
          continue;
        }
    score=0;
    if ((style == UndefinedStyle) || (style == AnyStyle) || (p->style == style))
      score+=32;
    else
      if (((style == ItalicStyle) || (style == ObliqueStyle)) &&
          ((p->style == ItalicStyle) || (p->style == ObliqueStyle)))
        score+=25;
    if (weight == 0)
      score+=16;
    else
      score+=(16*(800-((ssize_t) MagickMax(MagickMin(weight,900),p->weight)-
        (ssize_t) MagickMin(MagickMin(weight,900),p->weight))))/800;
    if ((stretch == UndefinedStretch) || (stretch == AnyStretch))
      score+=8;
    else
      {
        range=(ssize_t) UltraExpandedStretch-(ssize_t) NormalStretch;
        score+=(8*(range-((ssize_t) MagickMax(stretch,p->stretch)-
          (ssize_t) MagickMin(stretch,p->stretch))))/range;
      }
    if (score > max_score)
      {
        max_score=score;
        type_info=p;
      }
    p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
  }
  UnlockSemaphoreInfo(type_semaphore);
  if (type_info != (const TypeInfo *) NULL)
    return(type_info);
  /*
    Check for table-based substitution match.
  */
  for (i=0; fontmap[i].name != (char *) NULL; i++)
  {
    if (family == (const char *) NULL)
      {
        if ((LocaleCompare(fontmap[i].name,"arial") != 0) &&
            (LocaleCompare(fontmap[i].name,"helvetica") != 0))
          continue;
      }
    else
      if (LocaleCompare(fontmap[i].name,family) != 0)
        continue;
    type_info=GetTypeInfoByFamily(fontmap[i].substitute,style,stretch,weight,
      exception);
    break;
  }
  if (type_info != (const TypeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),TypeError,
        "FontSubstitutionRequired","`%s'",type_info->family);
      return(type_info);
    }
  if (family != (const char *) NULL)
    type_info=GetTypeInfoByFamily((const char *) NULL,style,stretch,weight,
      exception);
  return(type_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t T y p e I n f o L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetTypeInfoList() returns any fonts that match the specified pattern.
%
%  The format of the GetTypeInfoList function is:
%
%      const TypeInfo **GetTypeInfoList(const char *pattern,
%        size_t *number_fonts,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_fonts:  This integer returns the number of types in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int TypeInfoCompare(const void *x,const void *y)
{
  const TypeInfo
    **p,
    **q;

  p=(const TypeInfo **) x,
  q=(const TypeInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->name,(*q)->name));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const TypeInfo **GetTypeInfoList(const char *pattern,
  size_t *number_fonts,ExceptionInfo *exception)
{
  const TypeInfo
    **fonts;

  register const TypeInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate type list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_fonts != (size_t *) NULL);
  *number_fonts=0;
  p=GetTypeInfo("*",exception);
  if (p == (const TypeInfo *) NULL)
    return((const TypeInfo **) NULL);
  fonts=(const TypeInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(type_list)+1UL,sizeof(*fonts));
  if (fonts == (const TypeInfo **) NULL)
    return((const TypeInfo **) NULL);
  /*
    Generate type list.
  */
  LockSemaphoreInfo(type_semaphore);
  ResetSplayTreeIterator(type_list);
  p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
  for (i=0; p != (const TypeInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      fonts[i++]=p;
    p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
  }
  UnlockSemaphoreInfo(type_semaphore);
  qsort((void *) fonts,(size_t) i,sizeof(*fonts),TypeInfoCompare);
  fonts[i]=(TypeInfo *) NULL;
  *number_fonts=(size_t) i;
  return(fonts);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t T y p e L i s t                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetTypeList() returns any fonts that match the specified pattern.
%
%  The format of the GetTypeList function is:
%
%      char **GetTypeList(const char *pattern,size_t *number_fonts,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_fonts:  This integer returns the number of fonts in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int TypeCompare(const void *x,const void *y)
{
  register const char
    **p,
    **q;

  p=(const char **) x;
  q=(const char **) y;
  return(LocaleCompare(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport char **GetTypeList(const char *pattern,size_t *number_fonts,
  ExceptionInfo *exception)
{
  char
    **fonts;

  register const TypeInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate type list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_fonts != (size_t *) NULL);
  *number_fonts=0;
  p=GetTypeInfo("*",exception);
  if (p == (const TypeInfo *) NULL)
    return((char **) NULL);
  fonts=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(type_list)+1UL,sizeof(*fonts));
  if (fonts == (char **) NULL)
    return((char **) NULL);
  /*
    Generate type list.
  */
  LockSemaphoreInfo(type_semaphore);
  ResetSplayTreeIterator(type_list);
  p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
  for (i=0; p != (const TypeInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      fonts[i++]=ConstantString(p->name);
    p=(const TypeInfo *) GetNextValueInSplayTree(type_list);
  }
  UnlockSemaphoreInfo(type_semaphore);
  qsort((void *) fonts,(size_t) i,sizeof(*fonts),TypeCompare);
  fonts[i]=(char *) NULL;
  *number_fonts=(size_t) i;
  return(fonts);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e T y p e L i s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeTypeList() initializes the type list.
%
%  The format of the InitializeTypeList method is:
%
%      MagickBooleanType InitializeTypeList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_FONTCONFIG_DELEGATE)
MagickExport MagickBooleanType LoadFontConfigFonts(SplayTreeInfo *type_list,
  ExceptionInfo *exception)
{
#if !defined(FC_FULLNAME)
#define FC_FULLNAME "fullname"
#endif

  char
    extension[MaxTextExtent],
    name[MaxTextExtent];

  FcChar8
    *family,
    *file,
    *fullname,
    *style;

  FcConfig
    *font_config;

  FcFontSet
    *font_set;

  FcObjectSet
    *object_set;

  FcPattern
    *pattern;

  FcResult
    status;

  int
    slant,
    width,
    weight;

  register ssize_t
    i;

  TypeInfo
    *type_info;

  /*
    Load system fonts.
  */
  (void) exception;
  font_config=FcInitLoadConfigAndFonts();
  if (font_config == (FcConfig *) NULL)
    return(MagickFalse);
  font_set=(FcFontSet *) NULL;
  object_set=FcObjectSetBuild(FC_FULLNAME,FC_FAMILY,FC_STYLE,FC_SLANT,
    FC_WIDTH,FC_WEIGHT,FC_FILE,(char *) NULL);
  if (object_set != (FcObjectSet *) NULL)
    {
      pattern=FcPatternCreate();
      if (pattern != (FcPattern *) NULL)
        {
          font_set=FcFontList(0,pattern,object_set);
          FcPatternDestroy(pattern);
        }
      FcObjectSetDestroy(object_set);
    }
  if (font_set == (FcFontSet *) NULL)
    {
      FcConfigDestroy(font_config);
      return(MagickFalse);
    }
  for (i=0; i < (ssize_t) font_set->nfont; i++)
  {
    status=FcPatternGetString(font_set->fonts[i],FC_FAMILY,0,&family);
    if (status != FcResultMatch)
      continue;
    status=FcPatternGetString(font_set->fonts[i],FC_FILE,0,&file);
    if (status != FcResultMatch)
      continue;
    *extension='\0';
    GetPathComponent((const char *) file,ExtensionPath,extension);
    if ((*extension != '\0') && (LocaleCompare(extension,"gz") == 0))
      continue;
    type_info=(TypeInfo *) AcquireMagickMemory(sizeof(*type_info));
    if (type_info == (TypeInfo *) NULL)
      continue;
    (void) ResetMagickMemory(type_info,0,sizeof(*type_info));
    type_info->path=ConstantString("System Fonts");
    type_info->signature=MagickSignature;
    (void) CopyMagickString(name,"Unknown",MaxTextExtent);
    status=FcPatternGetString(font_set->fonts[i],FC_FULLNAME,0,&fullname);
    if ((status == FcResultMatch) && (fullname != (FcChar8 *) NULL))
      (void) CopyMagickString(name,(const char *) fullname,MaxTextExtent);
    else
      {
        if (family != (FcChar8 *) NULL)
          (void) CopyMagickString(name,(const char *) family,MaxTextExtent);
        status=FcPatternGetString(font_set->fonts[i],FC_STYLE,0,&style);
        if ((status == FcResultMatch) && (style != (FcChar8 *) NULL) &&
            (LocaleCompare((const char *) style,"Regular") != 0))
          {
            (void) ConcatenateMagickString(name," ",MaxTextExtent);
            (void) ConcatenateMagickString(name,(const char *) style,
              MaxTextExtent);
          }
      }
    type_info->name=ConstantString(name);
    (void) SubstituteString(&type_info->name," ","-");
    type_info->family=ConstantString((const char *) family);
    status=FcPatternGetInteger(font_set->fonts[i],FC_SLANT,0,&slant);
    type_info->style=NormalStyle;
    if (slant == FC_SLANT_ITALIC)
      type_info->style=ItalicStyle;
    if (slant == FC_SLANT_OBLIQUE)
      type_info->style=ObliqueStyle;
    status=FcPatternGetInteger(font_set->fonts[i],FC_WIDTH,0,&width);
    type_info->stretch=NormalStretch;
    if (width >= FC_WIDTH_ULTRACONDENSED)
      type_info->stretch=UltraCondensedStretch;
    if (width >= FC_WIDTH_EXTRACONDENSED)
      type_info->stretch=ExtraCondensedStretch;
    if (width >= FC_WIDTH_CONDENSED)
      type_info->stretch=CondensedStretch;
    if (width >= FC_WIDTH_SEMICONDENSED)
      type_info->stretch=SemiCondensedStretch;
    if (width >= FC_WIDTH_NORMAL)
      type_info->stretch=NormalStretch;
    if (width >= FC_WIDTH_SEMIEXPANDED)
      type_info->stretch=SemiExpandedStretch;
    if (width >= FC_WIDTH_EXPANDED)
      type_info->stretch=ExpandedStretch;
    if (width >= FC_WIDTH_EXTRAEXPANDED)
      type_info->stretch=ExtraExpandedStretch;
    if (width >= FC_WIDTH_ULTRAEXPANDED)
      type_info->stretch=UltraExpandedStretch;
    type_info->weight=400;
    status=FcPatternGetInteger(font_set->fonts[i],FC_WEIGHT,0,&weight);
    if (weight >= FC_WEIGHT_THIN)
      type_info->weight=100;
    if (weight >= FC_WEIGHT_EXTRALIGHT)
      type_info->weight=200;
    if (weight >= FC_WEIGHT_LIGHT)
      type_info->weight=300;
    if (weight >= FC_WEIGHT_NORMAL)
      type_info->weight=400;
    if (weight >= FC_WEIGHT_MEDIUM)
      type_info->weight=500;
    if (weight >= FC_WEIGHT_DEMIBOLD)
      type_info->weight=600;
    if (weight >= FC_WEIGHT_BOLD)
      type_info->weight=700;
    if (weight >= FC_WEIGHT_EXTRABOLD)
      type_info->weight=800;
    if (weight >= FC_WEIGHT_BLACK)
      type_info->weight=900;
    type_info->glyphs=ConstantString((const char *) file);
    (void) AddValueToSplayTree(type_list,type_info->name,type_info);
  }
  FcFontSetDestroy(font_set);
  FcConfigDestroy(font_config);
  return(MagickTrue);
}
#endif

static MagickBooleanType InitializeTypeList(ExceptionInfo *exception)
{
  if ((type_list == (SplayTreeInfo *) NULL) &&
      (instantiate_type == MagickFalse))
    {
      if (type_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&type_semaphore);
      LockSemaphoreInfo(type_semaphore);
      if ((type_list == (SplayTreeInfo *) NULL) &&
          (instantiate_type == MagickFalse))
        {
          (void) LoadTypeLists(MagickTypeFilename,exception);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
          (void) NTLoadTypeLists(type_list,exception);
#endif
#if defined(MAGICKCORE_FONTCONFIG_DELEGATE)
          (void) LoadFontConfigFonts(type_list,exception);
#endif
          instantiate_type=MagickTrue;
        }
      UnlockSemaphoreInfo(type_semaphore);
    }
  return(type_list != (SplayTreeInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t T y p e I n f o                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListTypeInfo() lists the fonts to a file.
%
%  The format of the ListTypeInfo method is:
%
%      MagickBooleanType ListTypeInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListTypeInfo(FILE *file,ExceptionInfo *exception)
{
  char
    weight[MaxTextExtent];

  const char
    *family,
    *glyphs,
    *name,
    *path,
    *stretch,
    *style;

  const TypeInfo
    **type_info;

  register ssize_t
    i;

  size_t
    number_fonts;

  if (file == (FILE *) NULL)
    file=stdout;
  number_fonts=0;
  type_info=GetTypeInfoList("*",&number_fonts,exception);
  if (type_info == (const TypeInfo **) NULL)
    return(MagickFalse);
  *weight='\0';
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_fonts; i++)
  {
    if (type_info[i]->stealth != MagickFalse)
      continue;
    if (((path == (const char *) NULL) ||
         (LocaleCompare(path,type_info[i]->path) != 0)) &&
         (type_info[i]->path != (char *) NULL))
      (void) FormatLocaleFile(file,"\nPath: %s\n",type_info[i]->path);
    path=type_info[i]->path;
    name="unknown";
    if (type_info[i]->name != (char *) NULL)
      name=type_info[i]->name;
    family="unknown";
    if (type_info[i]->family != (char *) NULL)
      family=type_info[i]->family;
    style=CommandOptionToMnemonic(MagickStyleOptions,type_info[i]->style);
    stretch=CommandOptionToMnemonic(MagickStretchOptions,type_info[i]->stretch);
    glyphs="unknown";
    if (type_info[i]->glyphs != (char *) NULL)
      glyphs=type_info[i]->glyphs;
    (void) FormatLocaleString(weight,MaxTextExtent,"%.20g",(double)
      type_info[i]->weight);
    (void) FormatLocaleFile(file,"  Font: %s\n",name);
    (void) FormatLocaleFile(file,"    family: %s\n",family);
    (void) FormatLocaleFile(file,"    style: %s\n",style);
    (void) FormatLocaleFile(file,"    stretch: %s\n",stretch);
    (void) FormatLocaleFile(file,"    weight: %s\n",weight);
    (void) FormatLocaleFile(file,"    glyphs: %s\n",glyphs);
  }
  (void) fflush(file);
  type_info=(const TypeInfo **) RelinquishMagickMemory((void *) type_info);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d T y p e L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadTypeList() loads the type configuration file which provides a mapping
%  between type attributes and a type name.
%
%  The format of the LoadTypeList method is:
%
%      MagickBooleanType LoadTypeList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The type list in XML format.
%
%    o filename:  The type list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void *DestroyTypeNode(void *type_info)
{
  register TypeInfo
    *p;

  p=(TypeInfo *) type_info;
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->name != (char *) NULL)
    p->name=DestroyString(p->name);
  if (p->description != (char *) NULL)
    p->description=DestroyString(p->description);
  if (p->family != (char *) NULL)
    p->family=DestroyString(p->family);
  if (p->encoding != (char *) NULL)
    p->encoding=DestroyString(p->encoding);
  if (p->foundry != (char *) NULL)
    p->foundry=DestroyString(p->foundry);
  if (p->format != (char *) NULL)
    p->format=DestroyString(p->format);
  if (p->metrics != (char *) NULL)
    p->metrics=DestroyString(p->metrics);
  if (p->glyphs != (char *) NULL)
    p->glyphs=DestroyString(p->glyphs);
  return(RelinquishMagickMemory(p));
}

static MagickBooleanType LoadTypeList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    font_path[MaxTextExtent],
    keyword[MaxTextExtent],
    *token;

  const char
    *q;

  MagickBooleanType
    status;

  TypeInfo
    *type_info;

  /*
    Load the type map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading type configure file \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(MagickFalse);
  if (type_list == (SplayTreeInfo *) NULL)
    {
      type_list=NewSplayTree(CompareSplayTreeString,(void *(*)(void *)) NULL,
        DestroyTypeNode);
      if (type_list == (SplayTreeInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  type_info=(TypeInfo *) NULL;
  token=AcquireString(xml);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  /*
    Determine the Ghostscript font path.
  */
  *font_path='\0';
  if (NTGhostscriptFonts(font_path,MaxTextExtent-2))
    (void) ConcatenateMagickString(font_path,DirectorySeparator,MaxTextExtent);
#endif
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetMagickToken(q,&q,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MaxTextExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MaxTextExtent);
          GetMagickToken(q,&q,token);
          if (*token != '=')
            continue;
          GetMagickToken(q,&q,token);
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > 200)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ConfigureError,"IncludeNodeNestedTooDeeply","`%s'",token);
              else
                {
                  char
                    path[MaxTextExtent],
                    *xml;

                  ExceptionInfo
                    *sans_exception;

                  *path='\0';
                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateMagickString(path,DirectorySeparator,
                      MaxTextExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyMagickString(path,token,MaxTextExtent);
                  else
                    (void) ConcatenateMagickString(path,token,MaxTextExtent);
                  sans_exception=AcquireExceptionInfo();
                  xml=FileToString(path,~0UL,sans_exception);
                  sans_exception=DestroyExceptionInfo(sans_exception);
                  if (xml != (char *) NULL)
                    {
                      status=LoadTypeList(xml,path,depth+1,exception);
                      xml=(char *) RelinquishMagickMemory(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<type") == 0)
      {
        /*
          Type element.
        */
        type_info=(TypeInfo *) AcquireMagickMemory(sizeof(*type_info));
        if (type_info == (TypeInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(type_info,0,sizeof(*type_info));
        type_info->path=ConstantString(filename);
        type_info->signature=MagickSignature;
        continue;
      }
    if (type_info == (TypeInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        status=AddValueToSplayTree(type_list,type_info->name,type_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",type_info->name);
        type_info=(TypeInfo *) NULL;
      }
    GetMagickToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetMagickToken(q,&q,token);
    GetMagickToken(q,&q,token);
    switch (*keyword)
    {
      case 'E':
      case 'e':
      {
        if (LocaleCompare((char *) keyword,"encoding") == 0)
          {
            type_info->encoding=ConstantString(token);
            break;
          }
        break;
      }
      case 'F':
      case 'f':
      {
        if (LocaleCompare((char *) keyword,"face") == 0)
          {
            type_info->face=StringToUnsignedLong(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"family") == 0)
          {
            type_info->family=ConstantString(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"format") == 0)
          {
            type_info->format=ConstantString(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"foundry") == 0)
          {
            type_info->foundry=ConstantString(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"fullname") == 0)
          {
            type_info->description=ConstantString(token);
            break;
          }
        break;
      }
      case 'G':
      case 'g':
      {
        if (LocaleCompare((char *) keyword,"glyphs") == 0)
          {
            char
              *path;

            path=ConstantString(token);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
            if (strchr(path,'@') != (char *) NULL)
              SubstituteString(&path,"@ghostscript_font_path@",font_path);
#endif
            if (IsPathAccessible(path) == MagickFalse)
              {
                /*
                  Relative path.
                */
                path=DestroyString(path);
                GetPathComponent(filename,HeadPath,font_path);
                (void) ConcatenateMagickString(font_path,DirectorySeparator,
                  MaxTextExtent);
                (void) ConcatenateMagickString(font_path,token,MaxTextExtent);
                path=ConstantString(font_path);
                if (IsPathAccessible(path) == MagickFalse)
                  {
                    path=DestroyString(path);
                    path=ConstantString(token);
                  }
              }
            type_info->glyphs=path;
            break;
          }
        break;
      }
      case 'M':
      case 'm':
      {
        if (LocaleCompare((char *) keyword,"metrics") == 0)
          {
            char
              *path;

            path=ConstantString(token);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
            if (strchr(path,'@') != (char *) NULL)
              SubstituteString(&path,"@ghostscript_font_path@",font_path);
#endif
            if (IsPathAccessible(path) == MagickFalse)
              {
                /*
                  Relative path.
                */
                path=DestroyString(path);
                GetPathComponent(filename,HeadPath,font_path);
                (void) ConcatenateMagickString(font_path,DirectorySeparator,
                  MaxTextExtent);
                (void) ConcatenateMagickString(font_path,token,MaxTextExtent);
                path=ConstantString(font_path);
              }
            type_info->metrics=path;
            break;
          }
        break;
      }
      case 'N':
      case 'n':
      {
        if (LocaleCompare((char *) keyword,"name") == 0)
          {
            type_info->name=ConstantString(token);
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            type_info->stealth=IsMagickTrue(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"stretch") == 0)
          {
            type_info->stretch=(StretchType) ParseCommandOption(
              MagickStretchOptions,MagickFalse,token);
            break;
          }
        if (LocaleCompare((char *) keyword,"style") == 0)
          {
            type_info->style=(StyleType) ParseCommandOption(MagickStyleOptions,
              MagickFalse,token);
            break;
          }
        break;
      }
      case 'W':
      case 'w':
      {
        if (LocaleCompare((char *) keyword,"weight") == 0)
          {
            type_info->weight=StringToUnsignedLong(token);
            if (LocaleCompare(token,"bold") == 0)
              type_info->weight=700;
            if (LocaleCompare(token,"normal") == 0)
              type_info->weight=400;
            break;
          }
        break;
      }
      default:
        break;
    }
  }
  token=(char *) RelinquishMagickMemory(token);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L o a d T y p e L i s t s                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadTypeList() loads one or more type configuration files which provides a
%  mapping between type attributes and a type name.
%
%  The format of the LoadTypeLists method is:
%
%      MagickBooleanType LoadTypeLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadTypeLists(const char *filename,
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_ZERO_CONFIGURATION_SUPPORT)
  return(LoadTypeList(TypeMap,"built-in",0,exception));
#else
  char
    *font_path,
    path[MaxTextExtent];

  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  status=MagickFalse;
  *path='\0';
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    (void) CopyMagickString(path,GetStringInfoPath(option),MaxTextExtent);
    status&=LoadTypeList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  font_path=GetEnvironmentValue("MAGICK_FONT_PATH");
  if (font_path != (char *) NULL)
    {
      char
        *option;

      /*
        Search MAGICK_FONT_PATH.
      */
      (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",font_path,
        DirectorySeparator,filename);
      option=FileToString(path,~0UL,exception);
      if (option != (void *) NULL)
        {
          status&=LoadTypeList(option,path,0,exception);
          option=DestroyString(option);
        }
      font_path=DestroyString(font_path);
    }
  if ((type_list == (SplayTreeInfo *) NULL) ||
      (GetNumberOfNodesInSplayTree(type_list) == 0))
    status&=LoadTypeList(TypeMap,"built-in",0,exception);
  return(status != 0 ? MagickTrue : MagickFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   T y p e C o m p o n e n t G e n e s i s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TypeComponentGenesis() instantiates the type component.
%
%  The format of the TypeComponentGenesis method is:
%
%      MagickBooleanType TypeComponentGenesis(void)
%
*/
MagickExport MagickBooleanType TypeComponentGenesis(void)
{
  AcquireSemaphoreInfo(&type_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   T y p e C o m p o n e n t T e r m i n u s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TypeComponentTerminus() destroy type component.
%
%  The format of the TypeComponentTerminus method is:
%
%      void TypeComponentTerminus(void)
%
*/
MagickExport void TypeComponentTerminus(void)
{
  if (type_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&type_semaphore);
  LockSemaphoreInfo(type_semaphore);
  if (type_list != (SplayTreeInfo *) NULL)
    type_list=DestroySplayTree(type_list);
  instantiate_type=MagickFalse;
  UnlockSemaphoreInfo(type_semaphore);
  DestroySemaphoreInfo(&type_semaphore);
}

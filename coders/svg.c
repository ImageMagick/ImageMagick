/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS  V   V   GGGG                              %
%                            SS     V   V  G                                  %
%                             SSS   V   V  G GG                               %
%                               SS   V V   G   G                              %
%                            SSSSS    V     GGG                               %
%                                                                             %
%                                                                             %
%                  Read/Write Scalable Vector Graphics Format                 %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                             William Radcliffe                               %
%                                March 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/annotate.h"
#include "magick/artifact.h"
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/constitute.h"
#include "magick/composite-private.h"
#include "magick/delegate.h"
#include "magick/delegate-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_XML_DELEGATE)
#  if defined(MAGICKCORE_WINDOWS_SUPPORT)
#    if defined(__MINGW32__) || defined(__MINGW64__)
#      define _MSC_VER
#    else
#      include <win32config.h>
#    endif
#  endif
#  include <libxml/parser.h>
#  include <libxml/xmlmemory.h>
#  include <libxml/parserInternals.h>
#  include <libxml/xmlerror.h>
#endif

#if defined(MAGICKCORE_AUTOTRACE_DELEGATE)
#include "autotrace/autotrace.h"
#endif

#if defined(MAGICKCORE_RSVG_DELEGATE)
#include "librsvg/rsvg.h"
#if defined(MAGICKCORE_CAIRO_DELEGATE)
#include "librsvg/rsvg-cairo.h"
#endif
#include "librsvg/librsvg-features.h"
#endif

/*
  Typedef declarations.
*/
typedef struct _BoundingBox
{
  double
    x,
    y,
    width,
    height;
} BoundingBox;

typedef struct _ElementInfo
{
  double
    cx,
    cy,
    major,
    minor,
    angle;
} ElementInfo;

typedef struct _SVGInfo
{
  FILE
    *file;

  ExceptionInfo
    *exception;

  Image
    *image;

  const ImageInfo
    *image_info;

  AffineMatrix
    affine;

  size_t
    width,
    height;

  char
    *size,
    *title,
    *comment;

  int
    n;

  double
    *scale,
    pointsize;

  ElementInfo
    element;

  SegmentInfo
    segment;

  BoundingBox
    bounds,
    center,
    view_box;

  PointInfo
    radius;

  char
    *stop_color,
    *offset,
    *text,
    *vertices,
    *url;

#if defined(MAGICKCORE_XML_DELEGATE)
  xmlParserCtxtPtr
    parser;

  xmlDocPtr
    document;
#endif
} SVGInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSVGImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S V G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSVG()() returns MagickTrue if the image format type, identified by the
%  magick string, is SVG.
%
%  The format of the IsSVG method is:
%
%      MagickBooleanType IsSVG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsSVG(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"?xml",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_XML_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S V G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSVGImage() reads a Scalable Vector Gaphics file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadSVGImage method is:
%
%      Image *ReadSVGImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static SVGInfo *AcquireSVGInfo(void)
{
  SVGInfo
    *svg_info;

  svg_info=(SVGInfo *) AcquireMagickMemory(sizeof(*svg_info));
  if (svg_info == (SVGInfo *) NULL)
    return((SVGInfo *) NULL);
  (void) ResetMagickMemory(svg_info,0,sizeof(*svg_info));
  svg_info->text=AcquireString("");
  svg_info->scale=(double *) AcquireMagickMemory(sizeof(*svg_info->scale));
  if (svg_info->scale == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  GetAffineMatrix(&svg_info->affine);
  svg_info->scale[0]=ExpandAffine(&svg_info->affine);
  return(svg_info);
}

static SVGInfo *DestroySVGInfo(SVGInfo *svg_info)
{
  if (svg_info->text != (char *) NULL)
    svg_info->text=DestroyString(svg_info->text);
  if (svg_info->scale != (double *) NULL)
    svg_info->scale=(double *) (svg_info->scale);
  if (svg_info->title != (char *) NULL)
    svg_info->title=DestroyString(svg_info->title);
  if (svg_info->comment != (char *) NULL)
    svg_info->comment=DestroyString(svg_info->comment);
  return((SVGInfo *) RelinquishMagickMemory(svg_info));
}

static double GetUserSpaceCoordinateValue(const SVGInfo *svg_info,int type,
  const char *string)
{
  char
    token[MaxTextExtent];

  const char
    *p;

  double
    value;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",string);
  assert(string != (const char *) NULL);
  p=(const char *) string;
  GetMagickToken(p,&p,token);
  value=StringToDouble(token,(char **) NULL);
  if (strchr(token,'%') != (char *) NULL)
    {
      double
        alpha,
        beta;

      if (type > 0)
        {
          if (svg_info->view_box.width == 0.0)
            return(0.0);
          return(svg_info->view_box.width*value/100.0);
        }
      if (type < 0)
        {
          if (svg_info->view_box.height == 0.0)
            return(0.0);
          return(svg_info->view_box.height*value/100.0);
        }
      alpha=value-svg_info->view_box.width;
      beta=value-svg_info->view_box.height;
      return(hypot(alpha,beta)/sqrt(2.0)/100.0);
    }
  GetMagickToken(p,&p,token);
  if (LocaleNCompare(token,"cm",2) == 0)
    return(DefaultResolution*svg_info->scale[0]/2.54*value);
  if (LocaleNCompare(token,"em",2) == 0)
    return(svg_info->pointsize*value);
  if (LocaleNCompare(token,"ex",2) == 0)
    return(svg_info->pointsize*value/2.0);
  if (LocaleNCompare(token,"in",2) == 0)
    return(DefaultResolution*svg_info->scale[0]*value);
  if (LocaleNCompare(token,"mm",2) == 0)
    return(DefaultResolution*svg_info->scale[0]/25.4*value);
  if (LocaleNCompare(token,"pc",2) == 0)
    return(DefaultResolution*svg_info->scale[0]/6.0*value);
  if (LocaleNCompare(token,"pt",2) == 0)
    return(1.25*svg_info->scale[0]*value);
  if (LocaleNCompare(token,"px",2) == 0)
    return(value);
  return(value);
}

static void StripStyleTokens(char *message)
{
  register char
    *p,
    *q;

  size_t
    length;

  assert(message != (char *) NULL);
  if (*message == '\0')
    return;
  length=strlen(message);
  p=message;
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  q=message+length-1;
  while ((isspace((int) ((unsigned char) *q)) != 0) && (q > p))
    q--;
  (void) CopyMagickMemory(message,p,(size_t) (q-p+1));
  message[q-p+1]='\0';
  StripString(message);
}

static char **GetStyleTokens(void *context,const char *style,int *number_tokens)
{
  char
    *text,
    **tokens;

  register ssize_t
    i;

  SVGInfo
    *svg_info;

  svg_info=(SVGInfo *) context;
  (void) svg_info;
  *number_tokens=0;
  if (style == (const char *) NULL)
    return((char **) NULL);
  text=AcquireString(style);
  (void) SubstituteString(&text,":","\n");
  (void) SubstituteString(&text,";","\n");
  tokens=StringToList(text);
  text=DestroyString(text);
  for (i=0; tokens[i] != (char *) NULL; i++)
    StripStyleTokens(tokens[i]);
  *number_tokens=i;
  return(tokens);
}

static char **GetTransformTokens(void *context,const char *text,
  int *number_tokens)
{
  char
    **tokens;

  register const char
    *p,
    *q;

  register ssize_t
    i;

  SVGInfo
    *svg_info;

  svg_info=(SVGInfo *) context;
  *number_tokens=0;
  if (text == (const char *) NULL)
    return((char **) NULL);
  /*
    Determine the number of arguments.
  */
  for (p=text; *p != '\0'; p++)
  {
    if (*p == '(')
      (*number_tokens)+=2;
  }
  tokens=(char **) AcquireQuantumMemory(*number_tokens+2UL,sizeof(*tokens));
  if (tokens == (char **) NULL)
    {
      (void) ThrowMagickException(svg_info->exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",text);
      return((char **) NULL);
    }
  /*
    Convert string to an ASCII list.
  */
  i=0;
  p=text;
  for (q=p; *q != '\0'; q++)
  {
    if ((*q != '(') && (*q != ')') && (*q != '\0'))
      continue;
    tokens[i]=AcquireString(p);
    (void) CopyMagickString(tokens[i],p,(size_t) (q-p+1));
    StripString(tokens[i++]);
    p=q+1;
  }
  tokens[i]=AcquireString(p);
  (void) CopyMagickString(tokens[i],p,(size_t) (q-p+1));
  StripString(tokens[i++]);
  tokens[i]=(char *) NULL;
  return(tokens);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int SVGIsStandalone(void *context)
{
  SVGInfo
    *svg_info;

  /*
    Is this document tagged standalone?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.SVGIsStandalone()");
  svg_info=(SVGInfo *) context;
  return(svg_info->document->standalone == 1);
}

static int SVGHasInternalSubset(void *context)
{
  SVGInfo
    *svg_info;

  /*
    Does this document has an internal subset?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.SVGHasInternalSubset()");
  svg_info=(SVGInfo *) context;
  return(svg_info->document->intSubset != NULL);
}

static int SVGHasExternalSubset(void *context)
{
  SVGInfo
    *svg_info;

  /*
    Does this document has an external subset?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.SVGHasExternalSubset()");
  svg_info=(SVGInfo *) context;
  return(svg_info->document->extSubset != NULL);
}

static void SVGInternalSubset(void *context,const xmlChar *name,
  const xmlChar *external_id,const xmlChar *system_id)
{
  SVGInfo
    *svg_info;

  /*
    Does this document has an internal subset?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.internalSubset(%s, %s, %s)",(const char *) name,
    (external_id != (const xmlChar *) NULL ? (const char *) external_id : "none"),
    (system_id != (const xmlChar *) NULL ? (const char *) system_id : "none"));
  svg_info=(SVGInfo *) context;
  (void) xmlCreateIntSubset(svg_info->document,name,external_id,system_id);
}

static xmlParserInputPtr SVGResolveEntity(void *context,
  const xmlChar *public_id,const xmlChar *system_id)
{
  SVGInfo
    *svg_info;

  xmlParserInputPtr
    stream;

  /*
    Special entity resolver, better left to the parser, it has more
    context than the application layer.  The default behaviour is to
    not resolve the entities, in that case the ENTITY_REF nodes are
    built in the structure (and the parameter values).
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.resolveEntity(%s, %s)",
    (public_id != (const xmlChar *) NULL ? (const char *) public_id : "none"),
    (system_id != (const xmlChar *) NULL ? (const char *) system_id : "none"));
  svg_info=(SVGInfo *) context;
  stream=xmlLoadExternalEntity((const char *) system_id,(const char *)
    public_id,svg_info->parser);
  return(stream);
}

static xmlEntityPtr SVGGetEntity(void *context,const xmlChar *name)
{
  SVGInfo
    *svg_info;

  /*
    Get an entity by name.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.SVGGetEntity(%s)",
    name);
  svg_info=(SVGInfo *) context;
  return(xmlGetDocEntity(svg_info->document,name));
}

static xmlEntityPtr SVGGetParameterEntity(void *context,const xmlChar *name)
{
  SVGInfo
    *svg_info;

  /*
    Get a parameter entity by name.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.getParameterEntity(%s)",name);
  svg_info=(SVGInfo *) context;
  return(xmlGetParameterEntity(svg_info->document,name));
}

static void SVGEntityDeclaration(void *context,const xmlChar *name,int type,
  const xmlChar *public_id,const xmlChar *system_id,xmlChar *content)
{
  SVGInfo
    *svg_info;

  /*
    An entity definition has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.entityDecl(%s, %d, %s, %s, %s)",name,type,
    public_id != (xmlChar *) NULL ? (const char *) public_id : "none",
    system_id != (xmlChar *) NULL ? (const char *) system_id : "none",content);
  svg_info=(SVGInfo *) context;
  if (svg_info->parser->inSubset == 1)
    (void) xmlAddDocEntity(svg_info->document,name,type,public_id,system_id,
      content);
  else
    if (svg_info->parser->inSubset == 2)
      (void) xmlAddDtdEntity(svg_info->document,name,type,public_id,system_id,
        content);
}

static void SVGAttributeDeclaration(void *context,const xmlChar *element,
  const xmlChar *name,int type,int value,const xmlChar *default_value,
  xmlEnumerationPtr tree)
{
  SVGInfo
    *svg_info;

  xmlChar
    *fullname,
    *prefix;

  xmlParserCtxtPtr
    parser;

  /*
    An attribute definition has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.attributeDecl(%s, %s, %d, %d, %s, ...)",element,name,type,value,
    default_value);
  svg_info=(SVGInfo *) context;
  fullname=(xmlChar *) NULL;
  prefix=(xmlChar *) NULL;
  parser=svg_info->parser;
  fullname=(xmlChar *) xmlSplitQName(parser,name,&prefix);
  if (parser->inSubset == 1)
    (void) xmlAddAttributeDecl(&parser->vctxt,svg_info->document->intSubset,
      element,fullname,prefix,(xmlAttributeType) type,
      (xmlAttributeDefault) value,default_value,tree);
  else
    if (parser->inSubset == 2)
      (void) xmlAddAttributeDecl(&parser->vctxt,svg_info->document->extSubset,
        element,fullname,prefix,(xmlAttributeType) type,
        (xmlAttributeDefault) value,default_value,tree);
  if (prefix != (xmlChar *) NULL)
    xmlFree(prefix);
  if (fullname != (xmlChar *) NULL)
    xmlFree(fullname);
}

static void SVGElementDeclaration(void *context,const xmlChar *name,int type,
  xmlElementContentPtr content)
{
  SVGInfo
    *svg_info;

  xmlParserCtxtPtr
    parser;

  /*
    An element definition has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.elementDecl(%s, %d, ...)",name,type);
  svg_info=(SVGInfo *) context;
  parser=svg_info->parser;
  if (parser->inSubset == 1)
    (void) xmlAddElementDecl(&parser->vctxt,svg_info->document->intSubset,
      name,(xmlElementTypeVal) type,content);
  else
    if (parser->inSubset == 2)
      (void) xmlAddElementDecl(&parser->vctxt,svg_info->document->extSubset,
        name,(xmlElementTypeVal) type,content);
}

static void SVGNotationDeclaration(void *context,const xmlChar *name,
  const xmlChar *public_id,const xmlChar *system_id)
{
  SVGInfo
    *svg_info;

  xmlParserCtxtPtr
    parser;

  /*
    What to do when a notation declaration has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.notationDecl(%s, %s, %s)",name,
    public_id != (const xmlChar *) NULL ? (const char *) public_id : "none",
    system_id != (const xmlChar *) NULL ? (const char *) system_id : "none");
  svg_info=(SVGInfo *) context;
  parser=svg_info->parser;
  if (parser->inSubset == 1)
    (void) xmlAddNotationDecl(&parser->vctxt,svg_info->document->intSubset,
      name,public_id,system_id);
  else
    if (parser->inSubset == 2)
      (void) xmlAddNotationDecl(&parser->vctxt,svg_info->document->intSubset,
        name,public_id,system_id);
}

static void SVGUnparsedEntityDeclaration(void *context,const xmlChar *name,
  const xmlChar *public_id,const xmlChar *system_id,const xmlChar *notation)
{
  SVGInfo
    *svg_info;

  /*
    What to do when an unparsed entity declaration is parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.unparsedEntityDecl(%s, %s, %s, %s)",name,
    public_id != (xmlChar *) NULL ? (const char *) public_id : "none",
    system_id != (xmlChar *) NULL ? (const char *) system_id : "none",notation);
  svg_info=(SVGInfo *) context;
  (void) xmlAddDocEntity(svg_info->document,name,
    XML_EXTERNAL_GENERAL_UNPARSED_ENTITY,public_id,system_id,notation);

}

static void SVGSetDocumentLocator(void *context,xmlSAXLocatorPtr location)
{
  SVGInfo
    *svg_info;

  /*
    Receive the document locator at startup, actually xmlDefaultSAXLocator.
  */
  (void) location;
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.setDocumentLocator()");
  svg_info=(SVGInfo *) context;
  (void) svg_info;
}

static void SVGStartDocument(void *context)
{
  SVGInfo
    *svg_info;

  xmlParserCtxtPtr
    parser;

  /*
    Called when the document start being processed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.startDocument()");
  svg_info=(SVGInfo *) context;
  GetExceptionInfo(svg_info->exception);
  parser=svg_info->parser;
  svg_info->document=xmlNewDoc(parser->version);
  if (svg_info->document == (xmlDocPtr) NULL)
    return;
  if (parser->encoding == NULL)
    svg_info->document->encoding=(const xmlChar *) NULL;
  else
    svg_info->document->encoding=xmlStrdup(parser->encoding);
  svg_info->document->standalone=parser->standalone;
}

static void SVGEndDocument(void *context)
{
  SVGInfo
    *svg_info;

  /*
    Called when the document end has been detected.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.endDocument()");
  svg_info=(SVGInfo *) context;
  if (svg_info->offset != (char *) NULL)
    svg_info->offset=DestroyString(svg_info->offset);
  if (svg_info->stop_color != (char *) NULL)
    svg_info->stop_color=DestroyString(svg_info->stop_color);
  if (svg_info->scale != (double *) NULL)
    svg_info->scale=(double *) RelinquishMagickMemory(svg_info->scale);
  if (svg_info->text != (char *) NULL)
    svg_info->text=DestroyString(svg_info->text);
  if (svg_info->vertices != (char *) NULL)
    svg_info->vertices=DestroyString(svg_info->vertices);
  if (svg_info->url != (char *) NULL)
    svg_info->url=DestroyString(svg_info->url);
#if defined(MAGICKCORE_XML_DELEGATE)
  if (svg_info->document != (xmlDocPtr) NULL)
    {
      xmlFreeDoc(svg_info->document);
      svg_info->document=(xmlDocPtr) NULL;
    }
#endif
}

static void SVGStartElement(void *context,const xmlChar *name,
  const xmlChar **attributes)
{
  char
    *color,
    id[MaxTextExtent],
    token[MaxTextExtent],
    **tokens,
    *units;

  const char
    *keyword,
    *p,
    *value;

  int
    number_tokens;

  SVGInfo
    *svg_info;

  register ssize_t
    i,
    j;

  /*
    Called when an opening tag has been processed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.startElement(%s",
    name);
  svg_info=(SVGInfo *) context;
  svg_info->n++;
  svg_info->scale=(double *) ResizeQuantumMemory(svg_info->scale,
    svg_info->n+1UL,sizeof(*svg_info->scale));
  if (svg_info->scale == (double *) NULL)
    {
      (void) ThrowMagickException(svg_info->exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",name);
      return;
    }
  svg_info->scale[svg_info->n]=svg_info->scale[svg_info->n-1];
  color=AcquireString("none");
  units=AcquireString("userSpaceOnUse");
  value=(const char *) NULL;
  if (attributes != (const xmlChar **) NULL)
    for (i=0; (attributes[i] != (const xmlChar *) NULL); i+=2)
    {
      keyword=(const char *) attributes[i];
      value=(const char *) attributes[i+1];
      switch (*keyword)
      {
        case 'C':
        case 'c':
        {
          if (LocaleCompare(keyword,"cx") == 0)
            {
              svg_info->element.cx=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"cy") == 0)
            {
              svg_info->element.cy=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        case 'F':
        case 'f':
        {
          if (LocaleCompare(keyword,"fx") == 0)
            {
              svg_info->element.major=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"fy") == 0)
            {
              svg_info->element.minor=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        case 'H':
        case 'h':
        {
          if (LocaleCompare(keyword,"height") == 0)
            {
              svg_info->bounds.height=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        case 'I':
        case 'i':
        {
          if (LocaleCompare(keyword,"id") == 0)
            {
              (void) CopyMagickString(id,value,MaxTextExtent);
              break;
            }
          break;
        }
        case 'R':
        case 'r':
        {
          if (LocaleCompare(keyword,"r") == 0)
            {
              svg_info->element.angle=
                GetUserSpaceCoordinateValue(svg_info,0,value);
              break;
            }
          break;
        }
        case 'W':
        case 'w':
        {
          if (LocaleCompare(keyword,"width") == 0)
            {
              svg_info->bounds.width=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          break;
        }
        case 'X':
        case 'x':
        {
          if (LocaleCompare(keyword,"x") == 0)
            {
              svg_info->bounds.x=GetUserSpaceCoordinateValue(svg_info,1,value)-
                svg_info->center.x;
              break;
            }
          if (LocaleCompare(keyword,"x1") == 0)
            {
              svg_info->segment.x1=GetUserSpaceCoordinateValue(svg_info,1,
                value);
              break;
            }
          if (LocaleCompare(keyword,"x2") == 0)
            {
              svg_info->segment.x2=GetUserSpaceCoordinateValue(svg_info,1,
                value);
              break;
            }
          break;
        }
        case 'Y':
        case 'y':
        {
          if (LocaleCompare(keyword,"y") == 0)
            {
              svg_info->bounds.y=GetUserSpaceCoordinateValue(svg_info,-1,value)-
                svg_info->center.y;
              break;
            }
          if (LocaleCompare(keyword,"y1") == 0)
            {
              svg_info->segment.y1=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          if (LocaleCompare(keyword,"y2") == 0)
            {
              svg_info->segment.y2=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        default:
          break;
      }
    }
  if (strchr((char *) name,':') != (char *) NULL)
    {
      /*
        Skip over namespace.
      */
      for ( ; *name != ':'; name++) ;
      name++;
    }
  switch (*name)
  {
    case 'C':
    case 'c':
    {
      if (LocaleCompare((const char *) name,"circle") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"clipPath") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push clip-path '%s'\n",id);
          break;
        }
      break;
    }
    case 'D':
    case 'd':
    {
      if (LocaleCompare((const char *) name,"defs") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push defs\n");
          break;
        }
      break;
    }
    case 'E':
    case 'e':
    {
      if (LocaleCompare((const char *) name,"ellipse") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      break;
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare((const char *) name,"g") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      break;
    }
    case 'I':
    case 'i':
    {
      if (LocaleCompare((const char *) name,"image") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      break;
    }
    case 'L':
    case 'l':
    {
      if (LocaleCompare((const char *) name,"line") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"linearGradient") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,
            "push gradient '%s' linear %g,%g %g,%g\n",id,
            svg_info->segment.x1,svg_info->segment.y1,svg_info->segment.x2,
            svg_info->segment.y2);
          break;
        }
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare((const char *) name,"path") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"pattern") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,
            "push pattern '%s' %g,%g %g,%g\n",id,
            svg_info->bounds.x,svg_info->bounds.y,svg_info->bounds.width,
            svg_info->bounds.height);
          break;
        }
      if (LocaleCompare((const char *) name,"polygon") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"polyline") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      break;
    }
    case 'R':
    case 'r':
    {
      if (LocaleCompare((const char *) name,"radialGradient") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,
            "push gradient '%s' radial %g,%g %g,%g %g\n",
            id,svg_info->element.cx,svg_info->element.cy,
            svg_info->element.major,svg_info->element.minor,
            svg_info->element.angle);
          break;
        }
      if (LocaleCompare((const char *) name,"rect") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      break;
    }
    case 'S':
    case 's':
    {
      if (LocaleCompare((const char *) name,"svg") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      break;
    }
    case 'T':
    case 't':
    {
      if (LocaleCompare((const char *) name,"text") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          svg_info->bounds.x=0.0;
          svg_info->bounds.y=0.0;
          svg_info->bounds.width=0.0;
          svg_info->bounds.height=0.0;
          break;
        }
      if (LocaleCompare((const char *) name,"tspan") == 0)
        {
          if (*svg_info->text != '\0')
            {
              DrawInfo
                *draw_info;

              TypeMetric
                metrics;

              char
                *text;

              text=EscapeString(svg_info->text,'\'');
              (void) FormatLocaleFile(svg_info->file,"text %g,%g '%s'\n",
                svg_info->bounds.x-svg_info->center.x,svg_info->bounds.y-
                svg_info->center.y,text);
              text=DestroyString(text);
              draw_info=CloneDrawInfo(svg_info->image_info,(DrawInfo *) NULL);
              draw_info->pointsize=svg_info->pointsize;
              draw_info->text=AcquireString(svg_info->text);
              (void) ConcatenateString(&draw_info->text," ");
              GetTypeMetrics(svg_info->image,draw_info,&metrics);
              svg_info->bounds.x+=metrics.width;
              draw_info=DestroyDrawInfo(draw_info);
              *svg_info->text='\0';
            }
          (void) FormatLocaleFile(svg_info->file,"push graphic-context\n");
          break;
        }
      break;
    }
    default:
      break;
  }
  if (attributes != (const xmlChar **) NULL)
    for (i=0; (attributes[i] != (const xmlChar *) NULL); i+=2)
    {
      keyword=(const char *) attributes[i];
      value=(const char *) attributes[i+1];
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    %s = %s",keyword,value);
      switch (*keyword)
      {
        case 'A':
        case 'a':
        {
          if (LocaleCompare(keyword,"angle") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"angle %g\n",
                GetUserSpaceCoordinateValue(svg_info,0,value));
              break;
            }
          break;
        }
        case 'C':
        case 'c':
        {
          if (LocaleCompare(keyword,"clip-path") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"clip-path '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"clip-rule") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"clip-rule '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"clipPathUnits") == 0)
            {
              (void) CloneString(&units,value);
              (void) FormatLocaleFile(svg_info->file,"clip-units '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"color") == 0)
            {
              (void) CloneString(&color,value);
              break;
            }
          if (LocaleCompare(keyword,"cx") == 0)
            {
              svg_info->element.cx=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"cy") == 0)
            {
              svg_info->element.cy=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        case 'D':
        case 'd':
        {
          if (LocaleCompare(keyword,"d") == 0)
            {
              (void) CloneString(&svg_info->vertices,value);
              break;
            }
          if (LocaleCompare(keyword,"dx") == 0)
            {
              svg_info->bounds.x+=GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"dy") == 0)
            {
              svg_info->bounds.y+=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        case 'F':
        case 'f':
        {
          if (LocaleCompare(keyword,"fill") == 0)
            {
              if (LocaleCompare(value,"currentColor") == 0)
                {
                  (void) FormatLocaleFile(svg_info->file,"fill '%s'\n",color);
                  break;
                }
              (void) FormatLocaleFile(svg_info->file,"fill '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"fillcolor") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"fill '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"fill-rule") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"fill-rule '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"fill-opacity") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"fill-opacity '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"font-family") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-family '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"font-stretch") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-stretch '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"font-style") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-style '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"font-size") == 0)
            {
              svg_info->pointsize=GetUserSpaceCoordinateValue(svg_info,0,value);
              (void) FormatLocaleFile(svg_info->file,"font-size %g\n",
                svg_info->pointsize);
              break;
            }
          if (LocaleCompare(keyword,"font-weight") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-weight '%s'\n",
                value);
              break;
            }
          break;
        }
        case 'G':
        case 'g':
        {
          if (LocaleCompare(keyword,"gradientTransform") == 0)
            {
              AffineMatrix
                affine,
                current,
                transform;

              GetAffineMatrix(&transform);
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  ");
              tokens=GetTransformTokens(context,value,&number_tokens);
              for (j=0; j < (number_tokens-1); j+=2)
              {
                keyword=(char *) tokens[j];
                if (keyword == (char *) NULL)
                  continue;
                value=(char *) tokens[j+1];
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    %s: %s",keyword,value);
                current=transform;
                GetAffineMatrix(&affine);
                switch (*keyword)
                {
                  case 'M':
                  case 'm':
                  {
                    if (LocaleCompare(keyword,"matrix") == 0)
                      {
                        p=(const char *) value;
                        GetMagickToken(p,&p,token);
                        affine.sx=StringToDouble(value,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.rx=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.ry=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.sy=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.tx=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.ty=StringToDouble(token,(char **) NULL);
                        break;
                      }
                    break;
                  }
                  case 'R':
                  case 'r':
                  {
                    if (LocaleCompare(keyword,"rotate") == 0)
                      {
                        double
                          angle;

                        angle=GetUserSpaceCoordinateValue(svg_info,0,value);
                        affine.sx=cos(DegreesToRadians(fmod(angle,360.0)));
                        affine.rx=sin(DegreesToRadians(fmod(angle,360.0)));
                        affine.ry=(-sin(DegreesToRadians(fmod(angle,360.0))));
                        affine.sy=cos(DegreesToRadians(fmod(angle,360.0)));
                        break;
                      }
                    break;
                  }
                  case 'S':
                  case 's':
                  {
                    if (LocaleCompare(keyword,"scale") == 0)
                      {
                        for (p=(const char *) value; *p != '\0'; p++)
                          if ((isspace((int) ((unsigned char) *p)) != 0) ||
                              (*p == ','))
                            break;
                        affine.sx=GetUserSpaceCoordinateValue(svg_info,1,value);
                        affine.sy=affine.sx;
                        if (*p != '\0')
                          affine.sy=
                            GetUserSpaceCoordinateValue(svg_info,-1,p+1);
                        svg_info->scale[svg_info->n]=ExpandAffine(&affine);
                        break;
                      }
                    if (LocaleCompare(keyword,"skewX") == 0)
                      {
                        affine.sx=svg_info->affine.sx;
                        affine.ry=tan(DegreesToRadians(fmod(
                          GetUserSpaceCoordinateValue(svg_info,1,value),
                          360.0)));
                        affine.sy=svg_info->affine.sy;
                        break;
                      }
                    if (LocaleCompare(keyword,"skewY") == 0)
                      {
                        affine.sx=svg_info->affine.sx;
                        affine.rx=tan(DegreesToRadians(fmod(
                          GetUserSpaceCoordinateValue(svg_info,-1,value),
                          360.0)));
                        affine.sy=svg_info->affine.sy;
                        break;
                      }
                    break;
                  }
                  case 'T':
                  case 't':
                  {
                    if (LocaleCompare(keyword,"translate") == 0)
                      {
                        for (p=(const char *) value; *p != '\0'; p++)
                          if ((isspace((int) ((unsigned char) *p)) != 0) ||
                              (*p == ','))
                            break;
                        affine.tx=GetUserSpaceCoordinateValue(svg_info,1,value);
                        affine.ty=affine.tx;
                        if (*p != '\0')
                          affine.ty=
                            GetUserSpaceCoordinateValue(svg_info,-1,p+1);
                        break;
                      }
                    break;
                  }
                  default:
                    break;
                }
                transform.sx=affine.sx*current.sx+affine.ry*current.rx;
                transform.rx=affine.rx*current.sx+affine.sy*current.rx;
                transform.ry=affine.sx*current.ry+affine.ry*current.sy;
                transform.sy=affine.rx*current.ry+affine.sy*current.sy;
                transform.tx=affine.tx*current.sx+affine.ty*current.ry+
                  current.tx;
                transform.ty=affine.tx*current.rx+affine.ty*current.sy+
                  current.ty;
              }
              (void) FormatLocaleFile(svg_info->file,
                "affine %g %g %g %g %g %g\n",transform.sx,
                transform.rx,transform.ry,transform.sy,transform.tx,
                transform.ty);
              for (j=0; tokens[j] != (char *) NULL; j++)
                tokens[j]=DestroyString(tokens[j]);
              tokens=(char **) RelinquishMagickMemory(tokens);
              break;
            }
          if (LocaleCompare(keyword,"gradientUnits") == 0)
            {
              (void) CloneString(&units,value);
              (void) FormatLocaleFile(svg_info->file,"gradient-units '%s'\n",
                value);
              break;
            }
          break;
        }
        case 'H':
        case 'h':
        {
          if (LocaleCompare(keyword,"height") == 0)
            {
              svg_info->bounds.height=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          if (LocaleCompare(keyword,"href") == 0)
            {
              (void) CloneString(&svg_info->url,value);
              break;
            }
          break;
        }
        case 'M':
        case 'm':
        {
          if (LocaleCompare(keyword,"major") == 0)
            {
              svg_info->element.major=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"minor") == 0)
            {
              svg_info->element.minor=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        case 'O':
        case 'o':
        {
          if (LocaleCompare(keyword,"offset") == 0)
            {
              (void) CloneString(&svg_info->offset,value);
              break;
            }
          if (LocaleCompare(keyword,"opacity") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"opacity '%s'\n",value);
              break;
            }
          break;
        }
        case 'P':
        case 'p':
        {
          if (LocaleCompare(keyword,"path") == 0)
            {
              (void) CloneString(&svg_info->url,value);
              break;
            }
          if (LocaleCompare(keyword,"points") == 0)
            {
              (void) CloneString(&svg_info->vertices,value);
              break;
            }
          break;
        }
        case 'R':
        case 'r':
        {
          if (LocaleCompare(keyword,"r") == 0)
            {
              svg_info->element.major=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              svg_info->element.minor=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          if (LocaleCompare(keyword,"rotate") == 0)
            {
              double
                angle;

              angle=GetUserSpaceCoordinateValue(svg_info,0,value);
              (void) FormatLocaleFile(svg_info->file,"translate %g,%g\n",
                svg_info->bounds.x,svg_info->bounds.y);
              svg_info->bounds.x=0;
              svg_info->bounds.y=0;
              (void) FormatLocaleFile(svg_info->file,"rotate %g\n",angle);
              break;
            }
          if (LocaleCompare(keyword,"rx") == 0)
            {
              if (LocaleCompare((const char *) name,"ellipse") == 0)
                svg_info->element.major=
                  GetUserSpaceCoordinateValue(svg_info,1,value);
              else
                svg_info->radius.x=
                  GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"ry") == 0)
            {
              if (LocaleCompare((const char *) name,"ellipse") == 0)
                svg_info->element.minor=
                  GetUserSpaceCoordinateValue(svg_info,-1,value);
              else
                svg_info->radius.y=
                  GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        case 'S':
        case 's':
        {
          if (LocaleCompare(keyword,"stop-color") == 0)
            {
              (void) CloneString(&svg_info->stop_color,value);
              break;
            }
          if (LocaleCompare(keyword,"stroke") == 0)
            {
              if (LocaleCompare(value,"currentColor") == 0)
                {
                  (void) FormatLocaleFile(svg_info->file,"stroke '%s'\n",color);
                  break;
                }
              (void) FormatLocaleFile(svg_info->file,"stroke '%s'\n",value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-antialiasing") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-antialias %d\n",
                LocaleCompare(value,"true") == 0);
              break;
            }
          if (LocaleCompare(keyword,"stroke-dasharray") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-dasharray %s\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-dashoffset") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-dashoffset %s\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-linecap") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-linecap '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-linejoin") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-linejoin '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-miterlimit") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-miterlimit '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-opacity") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-opacity '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-width") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-width %g\n",
                GetUserSpaceCoordinateValue(svg_info,1,value));
              break;
            }
          if (LocaleCompare(keyword,"style") == 0)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  ");
              tokens=GetStyleTokens(context,value,&number_tokens);
              for (j=0; j < (number_tokens-1); j+=2)
              {
                keyword=(char *) tokens[j];
                value=(char *) tokens[j+1];
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    %s: %s",keyword,value);
                switch (*keyword)
                {
                  case 'C':
                  case 'c':
                  {
                     if (LocaleCompare(keyword,"clip-path") == 0)
                       {
                         (void) FormatLocaleFile(svg_info->file,
                           "clip-path '%s'\n",value);
                         break;
                       }
                    if (LocaleCompare(keyword,"clip-rule") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "clip-rule '%s'\n",value);
                        break;
                      }
                     if (LocaleCompare(keyword,"clipPathUnits") == 0)
                       {
                         (void) CloneString(&units,value);
                         (void) FormatLocaleFile(svg_info->file,
                          "clip-units '%s'\n",value);
                         break;
                       }
                    if (LocaleCompare(keyword,"color") == 0)
                      {
                        (void) CloneString(&color,value);
                        break;
                      }
                    break;
                  }
                  case 'F':
                  case 'f':
                  {
                    if (LocaleCompare(keyword,"fill") == 0)
                      {
                         if (LocaleCompare(value,"currentColor") == 0)
                           {
                             (void) FormatLocaleFile(svg_info->file,
                               "fill '%s'\n",color);
                             break;
                           }
                        if (LocaleCompare(value,"#00000000") == 0)
                          (void) FormatLocaleFile(svg_info->file,
                            "fill '#000000'\n");
                        else
                          (void) FormatLocaleFile(svg_info->file,"fill '%s'\n",
                            value);
                        break;
                      }
                    if (LocaleCompare(keyword,"fillcolor") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,"fill '%s'\n",
                          value);
                        break;
                      }
                    if (LocaleCompare(keyword,"fill-rule") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "fill-rule '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"fill-opacity") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "fill-opacity '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"font-family") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "font-family '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"font-stretch") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "font-stretch '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"font-style") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "font-style '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"font-size") == 0)
                      {
                        svg_info->pointsize=GetUserSpaceCoordinateValue(
                          svg_info,0,value);
                        (void) FormatLocaleFile(svg_info->file,"font-size %g\n",
                          svg_info->pointsize);
                        break;
                      }
                    if (LocaleCompare(keyword,"font-weight") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "font-weight '%s'\n",value);
                        break;
                      }
                    break;
                  }
                  case 'O':
                  case 'o':
                  {
                    if (LocaleCompare(keyword,"offset") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,"offset %g\n",
                          GetUserSpaceCoordinateValue(svg_info,1,value));
                        break;
                      }
                    if (LocaleCompare(keyword,"opacity") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "opacity '%s'\n",value);
                        break;
                      }
                    break;
                  }
                  case 'S':
                  case 's':
                  {
                    if (LocaleCompare(keyword,"stop-color") == 0)
                      {
                        (void) CloneString(&svg_info->stop_color,value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke") == 0)
                      {
                        if (LocaleCompare(value,"currentColor") == 0)
                          {
                            (void) FormatLocaleFile(svg_info->file,
                              "stroke '%s'\n",color);
                            break;
                          }
                        if (LocaleCompare(value,"#00000000") == 0)
                          (void) FormatLocaleFile(svg_info->file,
                            "fill '#000000'\n");
                        else
                          (void) FormatLocaleFile(svg_info->file,
                            "stroke '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-antialiasing") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-antialias %d\n",
                          LocaleCompare(value,"true") == 0);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-dasharray") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-dasharray %s\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-dashoffset") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-dashoffset %s\n",
                          value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-linecap") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-linecap '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-linejoin") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-linejoin '%s'\n",
                          value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-miterlimit") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-miterlimit '%s'\n",
                          value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-opacity") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-opacity '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"stroke-width") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "stroke-width %g\n",
                          GetUserSpaceCoordinateValue(svg_info,1,value));
                        break;
                      }
                    break;
                  }
                  case 't':
                  case 'T':
                  {
                    if (LocaleCompare(keyword,"text-align") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "text-align '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"text-anchor") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "text-anchor '%s'\n",value);
                        break;
                      }
                    if (LocaleCompare(keyword,"text-decoration") == 0)
                      {
                        if (LocaleCompare(value,"underline") == 0)
                          (void) FormatLocaleFile(svg_info->file,
                          "decorate underline\n");
                        if (LocaleCompare(value,"line-through") == 0)
                          (void) FormatLocaleFile(svg_info->file,
                          "decorate line-through\n");
                        if (LocaleCompare(value,"overline") == 0)
                          (void) FormatLocaleFile(svg_info->file,
                          "decorate overline\n");
                        break;
                      }
                    if (LocaleCompare(keyword,"text-antialiasing") == 0)
                      {
                        (void) FormatLocaleFile(svg_info->file,
                          "text-antialias %d\n",
                          LocaleCompare(value,"true") == 0);
                        break;
                      }
                    break;
                  }
                  default:
                    break;
                }
              }
              for (j=0; tokens[j] != (char *) NULL; j++)
                tokens[j]=DestroyString(tokens[j]);
              tokens=(char **) RelinquishMagickMemory(tokens);
              break;
            }
          break;
        }
        case 'T':
        case 't':
        {
          if (LocaleCompare(keyword,"text-align") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"text-align '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"text-anchor") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"text-anchor '%s'\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"text-decoration") == 0)
            {
              if (LocaleCompare(value,"underline") == 0)
                (void) FormatLocaleFile(svg_info->file,"decorate underline\n");
              if (LocaleCompare(value,"line-through") == 0)
                (void) FormatLocaleFile(svg_info->file,
                  "decorate line-through\n");
              if (LocaleCompare(value,"overline") == 0)
                (void) FormatLocaleFile(svg_info->file,"decorate overline\n");
              break;
            }
          if (LocaleCompare(keyword,"text-antialiasing") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"text-antialias %d\n",
                LocaleCompare(value,"true") == 0);
              break;
            }
          if (LocaleCompare(keyword,"transform") == 0)
            {
              AffineMatrix
                affine,
                current,
                transform;

              GetAffineMatrix(&transform);
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  ");
              tokens=GetTransformTokens(context,value,&number_tokens);
              for (j=0; j < (number_tokens-1); j+=2)
              {
                keyword=(char *) tokens[j];
                value=(char *) tokens[j+1];
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    %s: %s",keyword,value);
                current=transform;
                GetAffineMatrix(&affine);
                switch (*keyword)
                {
                  case 'M':
                  case 'm':
                  {
                    if (LocaleCompare(keyword,"matrix") == 0)
                      {
                        p=(const char *) value;
                        GetMagickToken(p,&p,token);
                        affine.sx=StringToDouble(value,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.rx=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.ry=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.sy=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.tx=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        affine.ty=StringToDouble(token,(char **) NULL);
                        break;
                      }
                    break;
                  }
                  case 'R':
                  case 'r':
                  {
                    if (LocaleCompare(keyword,"rotate") == 0)
                      {
                        double
                          angle,
                          x,
                          y;

                        p=(const char *) value;
                        GetMagickToken(p,&p,token);
                        angle=StringToDouble(value,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        x=StringToDouble(token,(char **) NULL);
                        GetMagickToken(p,&p,token);
                        if (*token == ',')
                          GetMagickToken(p,&p,token);
                        y=StringToDouble(token,(char **) NULL);
                        affine.sx=cos(DegreesToRadians(fmod(angle,360.0)));
                        affine.rx=sin(DegreesToRadians(fmod(angle,360.0)));
                        affine.ry=(-sin(DegreesToRadians(fmod(angle,360.0))));
                        affine.sy=cos(DegreesToRadians(fmod(angle,360.0)));
                        affine.tx=x;
                        affine.ty=y;
                        svg_info->center.x=x;
                        svg_info->center.y=y;
                        break;
                      }
                    break;
                  }
                  case 'S':
                  case 's':
                  {
                    if (LocaleCompare(keyword,"scale") == 0)
                      {
                        for (p=(const char *) value; *p != '\0'; p++)
                          if ((isspace((int) ((unsigned char) *p)) != 0) ||
                              (*p == ','))
                            break;
                        affine.sx=GetUserSpaceCoordinateValue(svg_info,1,value);
                        affine.sy=affine.sx;
                        if (*p != '\0')
                          affine.sy=GetUserSpaceCoordinateValue(svg_info,-1,
                            p+1);
                        svg_info->scale[svg_info->n]=ExpandAffine(&affine);
                        break;
                      }
                    if (LocaleCompare(keyword,"skewX") == 0)
                      {
                        affine.sx=svg_info->affine.sx;
                        affine.ry=tan(DegreesToRadians(fmod(
                          GetUserSpaceCoordinateValue(svg_info,1,value),
                          360.0)));
                        affine.sy=svg_info->affine.sy;
                        break;
                      }
                    if (LocaleCompare(keyword,"skewY") == 0)
                      {
                        affine.sx=svg_info->affine.sx;
                        affine.rx=tan(DegreesToRadians(fmod(
                          GetUserSpaceCoordinateValue(svg_info,-1,value),
                          360.0)));
                        affine.sy=svg_info->affine.sy;
                        break;
                      }
                    break;
                  }
                  case 'T':
                  case 't':
                  {
                    if (LocaleCompare(keyword,"translate") == 0)
                      {
                        for (p=(const char *) value; *p != '\0'; p++)
                          if ((isspace((int) ((unsigned char) *p)) != 0) ||
                              (*p == ','))
                            break;
                        affine.tx=GetUserSpaceCoordinateValue(svg_info,1,value);
                        affine.ty=affine.tx;
                        if (*p != '\0')
                          affine.ty=GetUserSpaceCoordinateValue(svg_info,-1,
                            p+1);
                        break;
                      }
                    break;
                  }
                  default:
                    break;
                }
                transform.sx=affine.sx*current.sx+affine.ry*current.rx;
                transform.rx=affine.rx*current.sx+affine.sy*current.rx;
                transform.ry=affine.sx*current.ry+affine.ry*current.sy;
                transform.sy=affine.rx*current.ry+affine.sy*current.sy;
                transform.tx=affine.tx*current.sx+affine.ty*current.ry+
                  current.tx;
                transform.ty=affine.tx*current.rx+affine.ty*current.sy+
                  current.ty;
              }
              (void) FormatLocaleFile(svg_info->file,
                "affine %g %g %g %g %g %g\n",transform.sx,transform.rx,
                transform.ry,transform.sy,transform.tx,transform.ty);
              for (j=0; tokens[j] != (char *) NULL; j++)
                tokens[j]=DestroyString(tokens[j]);
              tokens=(char **) RelinquishMagickMemory(tokens);
              break;
            }
          break;
        }
        case 'V':
        case 'v':
        {
          if (LocaleCompare(keyword,"verts") == 0)
            {
              (void) CloneString(&svg_info->vertices,value);
              break;
            }
          if (LocaleCompare(keyword,"viewBox") == 0)
            {
              p=(const char *) value;
              GetMagickToken(p,&p,token);
              svg_info->view_box.x=StringToDouble(token,(char **) NULL);
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
              svg_info->view_box.y=StringToDouble(token,(char **) NULL);
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
              svg_info->view_box.width=StringToDouble(token,
                (char **) NULL);
              if (svg_info->bounds.width == 0)
                svg_info->bounds.width=svg_info->view_box.width;
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
              svg_info->view_box.height=StringToDouble(token,
                (char **) NULL);
              if (svg_info->bounds.height == 0)
                svg_info->bounds.height=svg_info->view_box.height;
              break;
            }
          break;
        }
        case 'W':
        case 'w':
        {
          if (LocaleCompare(keyword,"width") == 0)
            {
              svg_info->bounds.width=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          break;
        }
        case 'X':
        case 'x':
        {
          if (LocaleCompare(keyword,"x") == 0)
            {
              svg_info->bounds.x=GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"xlink:href") == 0)
            {
              (void) CloneString(&svg_info->url,value);
              break;
            }
          if (LocaleCompare(keyword,"x1") == 0)
            {
              svg_info->segment.x1=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          if (LocaleCompare(keyword,"x2") == 0)
            {
              svg_info->segment.x2=
                GetUserSpaceCoordinateValue(svg_info,1,value);
              break;
            }
          break;
        }
        case 'Y':
        case 'y':
        {
          if (LocaleCompare(keyword,"y") == 0)
            {
              svg_info->bounds.y=GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          if (LocaleCompare(keyword,"y1") == 0)
            {
              svg_info->segment.y1=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          if (LocaleCompare(keyword,"y2") == 0)
            {
              svg_info->segment.y2=
                GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          break;
        }
        default:
          break;
      }
    }
  if (LocaleCompare((const char *) name,"svg") == 0)
    {
      if (svg_info->document->encoding != (const xmlChar *) NULL)
        (void) FormatLocaleFile(svg_info->file,"encoding \"%s\"\n",
          (const char *) svg_info->document->encoding);
      if (attributes != (const xmlChar **) NULL)
        {
          double
            sx,
            sy,
            tx,
            ty;

          if ((svg_info->view_box.width == 0.0) ||
              (svg_info->view_box.height == 0.0))
            svg_info->view_box=svg_info->bounds;
          svg_info->width=(size_t) floor(svg_info->bounds.width+0.5);
          svg_info->height=(size_t) floor(svg_info->bounds.height+0.5);
          (void) FormatLocaleFile(svg_info->file,"viewbox 0 0 %.20g %.20g\n",
            (double) svg_info->width,(double) svg_info->height);
          sx=(double) svg_info->width/svg_info->view_box.width;
          sy=(double) svg_info->height/svg_info->view_box.height;
          tx=svg_info->view_box.x != 0.0 ? (double) -sx*svg_info->view_box.x :
            0.0;
          ty=svg_info->view_box.y != 0.0 ? (double) -sy*svg_info->view_box.y :
            0.0;
          (void) FormatLocaleFile(svg_info->file,"affine %g 0 0 %g %g %g\n",
            sx,sy,tx,ty);
        }
    }
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  )");
  units=DestroyString(units);
  if (color != (char *) NULL)
    color=DestroyString(color);
}

static void SVGEndElement(void *context,const xmlChar *name)
{
  SVGInfo
    *svg_info;

  /*
    Called when the end of an element has been detected.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.endElement(%s)",name);
  svg_info=(SVGInfo *) context;
  if (strchr((char *) name,':') != (char *) NULL)
    {
      /*
        Skip over namespace.
      */
      for ( ; *name != ':'; name++) ;
      name++;
    }
  switch (*name)
  {
    case 'C':
    case 'c':
    {
      if (LocaleCompare((const char *) name,"circle") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"circle %g,%g %g,%g\n",
            svg_info->element.cx,svg_info->element.cy,svg_info->element.cx,
            svg_info->element.cy+svg_info->element.minor);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"clipPath") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop clip-path\n");
          break;
        }
      break;
    }
    case 'D':
    case 'd':
    {
      if (LocaleCompare((const char *) name,"defs") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop defs\n");
          break;
        }
      if (LocaleCompare((const char *) name,"desc") == 0)
        {
          register char
            *p;

          if (*svg_info->text == '\0')
            break;
          (void) fputc('#',svg_info->file);
          for (p=svg_info->text; *p != '\0'; p++)
          {
            (void) fputc(*p,svg_info->file);
            if (*p == '\n')
              (void) fputc('#',svg_info->file);
          }
          (void) fputc('\n',svg_info->file);
          *svg_info->text='\0';
          break;
        }
      break;
    }
    case 'E':
    case 'e':
    {
      if (LocaleCompare((const char *) name,"ellipse") == 0)
        {
          double
            angle;

          angle=svg_info->element.angle;
          (void) FormatLocaleFile(svg_info->file,"ellipse %g,%g %g,%g 0,360\n",
            svg_info->element.cx,svg_info->element.cy,
            angle == 0.0 ? svg_info->element.major : svg_info->element.minor,
            angle == 0.0 ? svg_info->element.minor : svg_info->element.major);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      break;
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare((const char *) name,"g") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      break;
    }
    case 'I':
    case 'i':
    {
      if (LocaleCompare((const char *) name,"image") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,
            "image Over %g,%g %g,%g '%s'\n",svg_info->bounds.x,
            svg_info->bounds.y,svg_info->bounds.width,svg_info->bounds.height,
            svg_info->url);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      break;
    }
    case 'L':
    case 'l':
    {
      if (LocaleCompare((const char *) name,"line") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"line %g,%g %g,%g\n",
            svg_info->segment.x1,svg_info->segment.y1,svg_info->segment.x2,
            svg_info->segment.y2);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"linearGradient") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop gradient\n");
          break;
        }
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare((const char *) name,"pattern") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop pattern\n");
          break;
        }
      if (LocaleCompare((const char *) name,"path") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"path '%s'\n",
            svg_info->vertices);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"polygon") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"polygon %s\n",
            svg_info->vertices);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"polyline") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"polyline %s\n",
            svg_info->vertices);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      break;
    }
    case 'R':
    case 'r':
    {
      if (LocaleCompare((const char *) name,"radialGradient") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop gradient\n");
          break;
        }
      if (LocaleCompare((const char *) name,"rect") == 0)
        {
          if ((svg_info->radius.x == 0.0) && (svg_info->radius.y == 0.0))
            {
              (void) FormatLocaleFile(svg_info->file,"rectangle %g,%g %g,%g\n",
                svg_info->bounds.x,svg_info->bounds.y,
                svg_info->bounds.x+svg_info->bounds.width,
                svg_info->bounds.y+svg_info->bounds.height);
              (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
              break;
            }
          if (svg_info->radius.x == 0.0)
            svg_info->radius.x=svg_info->radius.y;
          if (svg_info->radius.y == 0.0)
            svg_info->radius.y=svg_info->radius.x;
          (void) FormatLocaleFile(svg_info->file,
            "roundRectangle %g,%g %g,%g %g,%g\n",
            svg_info->bounds.x,svg_info->bounds.y,svg_info->bounds.x+
            svg_info->bounds.width,svg_info->bounds.y+svg_info->bounds.height,
            svg_info->radius.x,svg_info->radius.y);
          svg_info->radius.x=0.0;
          svg_info->radius.y=0.0;
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      break;
    }
    case 'S':
    case 's':
    {
      if (LocaleCompare((const char *) name,"stop") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"stop-color '%s' %s\n",
            svg_info->stop_color,svg_info->offset);
          break;
        }
      if (LocaleCompare((const char *) name,"svg") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      break;
    }
    case 'T':
    case 't':
    {
      if (LocaleCompare((const char *) name,"text") == 0)
        {
          if (*svg_info->text != '\0')
            {
              char
                *text;

              text=EscapeString(svg_info->text,'\'');
              (void) FormatLocaleFile(svg_info->file,"text %g,%g '%s'\n",
                svg_info->bounds.x,svg_info->bounds.y,text);
              text=DestroyString(text);
              *svg_info->text='\0';
            }
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"tspan") == 0)
        {
          if (*svg_info->text != '\0')
            {
              DrawInfo
                *draw_info;

              TypeMetric
                metrics;

              char
                *text;

              text=EscapeString(svg_info->text,'\'');
              (void) FormatLocaleFile(svg_info->file,"text %g,%g '%s'\n",
                svg_info->bounds.x,svg_info->bounds.y,text);
              text=DestroyString(text);
              draw_info=CloneDrawInfo(svg_info->image_info,(DrawInfo *) NULL);
              draw_info->pointsize=svg_info->pointsize;
              draw_info->text=AcquireString(svg_info->text);
              (void) ConcatenateString(&draw_info->text," ");
              GetTypeMetrics(svg_info->image,draw_info,&metrics);
              svg_info->bounds.x+=metrics.width;
              draw_info=DestroyDrawInfo(draw_info);
              *svg_info->text='\0';
            }
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"title") == 0)
        {
          if (*svg_info->text == '\0')
            break;
          (void) CloneString(&svg_info->title,svg_info->text);
          *svg_info->text='\0';
          break;
        }
      break;
    }
    default:
      break;
  }
  *svg_info->text='\0';
  (void) ResetMagickMemory(&svg_info->element,0,sizeof(svg_info->element));
  (void) ResetMagickMemory(&svg_info->segment,0,sizeof(svg_info->segment));
  svg_info->n--;
}

static void SVGCharacters(void *context,const xmlChar *c,int length)
{
  char
    *text;

  register char
    *p;

  register ssize_t
    i;

  SVGInfo
    *svg_info;

  /*
    Receiving some characters from the parser.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.characters(%s,%.20g)",c,(double) length);
  svg_info=(SVGInfo *) context;
  text=(char *) AcquireQuantumMemory(length+1,sizeof(*text));
  if (text == (char *) NULL)
    return;
  p=text;
  for (i=0; i < (ssize_t) length; i++)
    *p++=c[i];
  *p='\0';
  StripString(text);
  if (svg_info->text == (char *) NULL)
    svg_info->text=text;
  else
    {
      (void) ConcatenateString(&svg_info->text,text);
      text=DestroyString(text);
    }
}

static void SVGReference(void *context,const xmlChar *name)
{
  SVGInfo
    *svg_info;

  xmlParserCtxtPtr
    parser;

  /*
    Called when an entity reference is detected.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.reference(%s)",
    name);
  svg_info=(SVGInfo *) context;
  parser=svg_info->parser;
  if (parser == (xmlParserCtxtPtr) NULL)
    return;
  if (parser->node == (xmlNodePtr) NULL)
    return;
  if (*name == '#')
    (void) xmlAddChild(parser->node,xmlNewCharRef(svg_info->document,name));
  else
    (void) xmlAddChild(parser->node,xmlNewReference(svg_info->document,name));
}

static void SVGIgnorableWhitespace(void *context,const xmlChar *c,int length)
{
  SVGInfo
    *svg_info;

  /*
    Receiving some ignorable whitespaces from the parser.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.ignorableWhitespace(%.30s, %d)",c,length);
  svg_info=(SVGInfo *) context;
  (void) svg_info;
}

static void SVGProcessingInstructions(void *context,const xmlChar *target,
  const xmlChar *data)
{
  SVGInfo
    *svg_info;

  /*
    A processing instruction has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.processingInstruction(%s, %s)",target,data);
  svg_info=(SVGInfo *) context;
  (void) svg_info;
}

static void SVGComment(void *context,const xmlChar *value)
{
  SVGInfo
    *svg_info;

  /*
    A comment has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.comment(%s)",
    value);
  svg_info=(SVGInfo *) context;
  if (svg_info->comment != (char *) NULL)
    (void) ConcatenateString(&svg_info->comment,"\n");
  (void) ConcatenateString(&svg_info->comment,(const char *) value);
}

static void SVGWarning(void *context,const char *format,...)
{
  char
    *message,
    reason[MaxTextExtent];

  SVGInfo
    *svg_info;

  va_list
    operands;

  /**
    Display and format a warning messages, gives file, line, position and
    extra parameters.
  */
  va_start(operands,format);
  svg_info=(SVGInfo *) context;
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.warning: ");
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),format,operands);
#if !defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsprintf(reason,format,operands);
#else
  (void) vsnprintf(reason,MaxTextExtent,format,operands);
#endif
  message=GetExceptionMessage(errno);
  (void) ThrowMagickException(svg_info->exception,GetMagickModule(),
    DelegateWarning,reason,"`%s`",message);
  message=DestroyString(message);
  va_end(operands);
}

static void SVGError(void *context,const char *format,...)
{
  char
    *message,
    reason[MaxTextExtent];

  SVGInfo
    *svg_info;

  va_list
    operands;

  /*
    Display and format a error formats, gives file, line, position and
    extra parameters.
  */
  va_start(operands,format);
  svg_info=(SVGInfo *) context;
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.error: ");
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),format,operands);
#if !defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsprintf(reason,format,operands);
#else
  (void) vsnprintf(reason,MaxTextExtent,format,operands);
#endif
  message=GetExceptionMessage(errno);
  (void) ThrowMagickException(svg_info->exception,GetMagickModule(),CoderError,
    reason,"`%s`",message);
  message=DestroyString(message);
  va_end(operands);
}

static void SVGCDataBlock(void *context,const xmlChar *value,int length)
{
  SVGInfo
    *svg_info;

   xmlNodePtr
     child;

  xmlParserCtxtPtr
    parser;

  /*
    Called when a pcdata block has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.pcdata(%s, %d)",
    value,length);
  svg_info=(SVGInfo *) context;
  parser=svg_info->parser;
  child=xmlGetLastChild(parser->node);
  if ((child != (xmlNodePtr) NULL) && (child->type == XML_CDATA_SECTION_NODE))
    {
      xmlTextConcat(child,value,length);
      return;
    }
  (void) xmlAddChild(parser->node,xmlNewCDataBlock(parser->myDoc,value,length));
}

static void SVGExternalSubset(void *context,const xmlChar *name,
  const xmlChar *external_id,const xmlChar *system_id)
{
  SVGInfo
    *svg_info;

  xmlParserCtxt
    parser_context;

  xmlParserCtxtPtr
    parser;

  xmlParserInputPtr
    input;

  /*
    Does this document has an external subset?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.externalSubset(%s, %s, %s)",name,
    (external_id != (const xmlChar *) NULL ? (const char *) external_id : "none"),
    (system_id != (const xmlChar *) NULL ? (const char *) system_id : "none"));
  svg_info=(SVGInfo *) context;
  parser=svg_info->parser;
  if (((external_id == NULL) && (system_id == NULL)) ||
      ((parser->validate == 0) || (parser->wellFormed == 0) ||
      (svg_info->document == 0)))
    return;
  input=SVGResolveEntity(context,external_id,system_id);
  if (input == NULL)
    return;
  (void) xmlNewDtd(svg_info->document,name,external_id,system_id);
  parser_context=(*parser);
  parser->inputTab=(xmlParserInputPtr *) xmlMalloc(5*sizeof(*parser->inputTab));
  if (parser->inputTab == (xmlParserInputPtr *) NULL)
    {
      parser->errNo=XML_ERR_NO_MEMORY;
      parser->input=parser_context.input;
      parser->inputNr=parser_context.inputNr;
      parser->inputMax=parser_context.inputMax;
      parser->inputTab=parser_context.inputTab;
      return;
  }
  parser->inputNr=0;
  parser->inputMax=5;
  parser->input=NULL;
  xmlPushInput(parser,input);
  (void) xmlSwitchEncoding(parser,xmlDetectCharEncoding(parser->input->cur,4));
  if (input->filename == (char *) NULL)
    input->filename=(char *) xmlStrdup(system_id);
  input->line=1;
  input->col=1;
  input->base=parser->input->cur;
  input->cur=parser->input->cur;
  input->free=NULL;
  xmlParseExternalSubset(parser,external_id,system_id);
  while (parser->inputNr > 1)
    (void) xmlPopInput(parser);
  xmlFreeInputStream(parser->input);
  xmlFree(parser->inputTab);
  parser->input=parser_context.input;
  parser->inputNr=parser_context.inputNr;
  parser->inputMax=parser_context.inputMax;
  parser->inputTab=parser_context.inputTab;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*
  Static declarations.
*/
static char
  SVGDensityGeometry[] = "90.0x90.0";

static Image *ReadSVGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent];

  FILE
    *file;

  Image
    *image;

  int
    status,
    unique_file;

  ssize_t
    n;

  SVGInfo
    *svg_info;

  unsigned char
    message[MaxTextExtent];

  xmlSAXHandler
    sax_modules;

  xmlSAXHandlerPtr
    sax_handler;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if ((image->x_resolution < MagickEpsilon) ||
      (image->y_resolution < MagickEpsilon))
    {
      GeometryInfo
        geometry_info;

      int
        flags;

      flags=ParseGeometry(SVGDensityGeometry,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
    }
  if (LocaleCompare(image_info->magick,"MSVG") != 0)
    {
      const DelegateInfo
        *delegate_info;

      delegate_info=GetDelegateInfo("svg:decode",(char *) NULL,exception);
      if (delegate_info != (const DelegateInfo *) NULL)
        {
          char
            background[MaxTextExtent],
            command[MaxTextExtent],
            density[MaxTextExtent],
            filename[MaxTextExtent],
            opacity[MaxTextExtent],
            unique[MaxTextExtent];

          int
            status;

          /*
            Our best hope for compliance to the SVG standard.
          */
          (void) AcquireUniqueFilename(filename);
          (void) AcquireUniqueFilename(unique);
          (void) FormatLocaleString(density,MaxTextExtent,"%.20g,%.20g",
            image->x_resolution,image->y_resolution);
          (void) FormatLocaleString(background,MaxTextExtent,
            "rgb(%.20g%%,%.20g%%,%.20g%%)",
            100.0*QuantumScale*image->background_color.red,
            100.0*QuantumScale*image->background_color.green,
            100.0*QuantumScale*image->background_color.blue);
          (void) FormatLocaleString(opacity,MaxTextExtent,"%.20g",QuantumScale*
            (QuantumRange-image->background_color.opacity));
          (void) FormatLocaleString(command,MaxTextExtent,
            GetDelegateCommands(delegate_info),image->filename,filename,density,
              background,opacity,unique);
          status=SystemCommand(MagickFalse,image_info->verbose,command,
            exception);
          (void) RelinquishUniqueFileResource(unique);
          if (status == 0)
            {
              ImageInfo
                *read_info;

              read_info=CloneImageInfo(image_info);
              (void) CopyMagickString(read_info->filename,filename,
                MaxTextExtent);
              image=ReadImage(read_info,exception);
              read_info=DestroyImageInfo(read_info);
              (void) RelinquishUniqueFileResource(filename);
              if (image != (Image *) NULL)
                return(image);
            }
          (void) RelinquishUniqueFileResource(filename);
        }
      {
#if defined(MAGICKCORE_RSVG_DELEGATE)
#if defined(MAGICKCORE_CAIRO_DELEGATE)
        cairo_surface_t
          *cairo_surface;

        cairo_t
          *cairo_image;

        register unsigned char
          *p;

        RsvgDimensionData
          dimension_info;

        unsigned char
          *pixels;

#else
        GdkPixbuf
          *pixel_info;

        register const guchar
          *p;

#endif

        GError
          *error;

        ssize_t
          y;

        PixelPacket
          fill_color;

        register ssize_t
          x;

        register PixelPacket
          *q;

        RsvgHandle
          *svg_handle;

        svg_handle=rsvg_handle_new();
        if (svg_handle == (RsvgHandle *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        rsvg_handle_set_base_uri(svg_handle,image_info->filename);
        if ((image->x_resolution != 72.0) && (image->y_resolution != 72.0))
          rsvg_handle_set_dpi_x_y(svg_handle,image->x_resolution,
            image->y_resolution);
        while ((n=ReadBlob(image,MaxTextExtent,message)) != 0)
        {
          error=(GError *) NULL;
          (void) rsvg_handle_write(svg_handle,message,n,&error);
          if (error != (GError *) NULL)
            g_error_free(error);
        }
        error=(GError *) NULL;
        rsvg_handle_close(svg_handle,&error);
        if (error != (GError *) NULL)
          g_error_free(error);
#if defined(MAGICKCORE_CAIRO_DELEGATE)
        rsvg_handle_get_dimensions(svg_handle,&dimension_info);
        image->columns=image->x_resolution*dimension_info.width/72.0;
        image->rows=image->y_resolution*dimension_info.height/72.0;
        pixels=(unsigned char *) NULL;
#else
        pixel_info=rsvg_handle_get_pixbuf(svg_handle);
        rsvg_handle_free(svg_handle);
        image->columns=gdk_pixbuf_get_width(pixel_info);
        image->rows=gdk_pixbuf_get_height(pixel_info);
#endif
        image->matte=MagickTrue;
        SetImageProperty(image,"svg:base-uri",
          rsvg_handle_get_base_uri(svg_handle));
        if ((image->columns == 0) || (image->rows == 0))
          {
#if !defined(MAGICKCORE_CAIRO_DELEGATE)
            g_object_unref(G_OBJECT(pixel_info));
#endif
            g_object_unref(svg_handle);
            ThrowReaderException(MissingDelegateError,
              "NoDecodeDelegateForThisImageFormat");
          }
        if (image_info->ping == MagickFalse)
          {
            size_t
              stride;

#if defined(MAGICKCORE_CAIRO_DELEGATE)
            stride=4*image->columns;
#if defined(MAGICKCORE_PANGOCAIRO_DELEGATE)
            stride=(size_t) cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
              image->columns);
#endif
            pixels=(unsigned char *) AcquireQuantumMemory(stride,image->rows*
              sizeof(*pixels));
            if (pixels == (unsigned char *) NULL)
              {
                g_object_unref(svg_handle);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
#endif
            (void) SetImageBackgroundColor(image);
#if defined(MAGICKCORE_CAIRO_DELEGATE)
            cairo_surface=cairo_image_surface_create_for_data(pixels,
              CAIRO_FORMAT_ARGB32,image->columns,image->rows,4*image->columns);
            if (cairo_surface == (cairo_surface_t *) NULL)
              {
                pixels=(unsigned char *) RelinquishMagickMemory(pixels);
                g_object_unref(svg_handle);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            cairo_image=cairo_create(cairo_surface);
            cairo_set_operator(cairo_image,CAIRO_OPERATOR_CLEAR);
            cairo_paint(cairo_image);
            cairo_set_operator(cairo_image,CAIRO_OPERATOR_OVER);
            cairo_scale(cairo_image,image->x_resolution/72.0,
              image->y_resolution/72.0);
            rsvg_handle_render_cairo(svg_handle,cairo_image);
            cairo_destroy(cairo_image);
            cairo_surface_destroy(cairo_surface);
            g_object_unref(svg_handle);
            p=pixels;
#else
            p=gdk_pixbuf_get_pixels(pixel_info);
#endif
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
#if defined(MAGICKCORE_CAIRO_DELEGATE)
                fill_color.blue=ScaleCharToQuantum(*p++);
                fill_color.green=ScaleCharToQuantum(*p++);
                fill_color.red=ScaleCharToQuantum(*p++);
#else
                fill_color.red=ScaleCharToQuantum(*p++);
                fill_color.green=ScaleCharToQuantum(*p++);
                fill_color.blue=ScaleCharToQuantum(*p++);
#endif
                fill_color.opacity=QuantumRange-ScaleCharToQuantum(*p++);
#if defined(MAGICKCORE_CAIRO_DELEGATE)
                {
                  double
                    gamma;

                  gamma=1.0-QuantumScale*fill_color.opacity;
                  gamma=PerceptibleReciprocal(gamma);
                  fill_color.blue*=gamma;
                  fill_color.green*=gamma;
                  fill_color.red*=gamma;
                }
#endif
                MagickCompositeOver(&fill_color,fill_color.opacity,q,
                  (MagickRealType) q->opacity,q);
                q++;
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
          }
#if defined(MAGICKCORE_CAIRO_DELEGATE)
        if (pixels != (unsigned char *) NULL)
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
#else
        g_object_unref(G_OBJECT(pixel_info));
#endif
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
#endif
      }
    }
  /*
    Open draw file.
  */
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"w");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      (void) CopyMagickString(image->filename,filename,MaxTextExtent);
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Parse SVG file.
  */
  svg_info=AcquireSVGInfo();
  if (svg_info == (SVGInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  svg_info->file=file;
  svg_info->exception=exception;
  svg_info->image=image;
  svg_info->image_info=image_info;
  svg_info->bounds.width=image->columns;
  svg_info->bounds.height=image->rows;
  if (image_info->size != (char *) NULL)
    (void) CloneString(&svg_info->size,image_info->size);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"begin SAX");
  xmlInitParser();
  (void) xmlSubstituteEntitiesDefault(1);
  (void) ResetMagickMemory(&sax_modules,0,sizeof(sax_modules));
  sax_modules.internalSubset=SVGInternalSubset;
  sax_modules.isStandalone=SVGIsStandalone;
  sax_modules.hasInternalSubset=SVGHasInternalSubset;
  sax_modules.hasExternalSubset=SVGHasExternalSubset;
  sax_modules.resolveEntity=SVGResolveEntity;
  sax_modules.getEntity=SVGGetEntity;
  sax_modules.entityDecl=SVGEntityDeclaration;
  sax_modules.notationDecl=SVGNotationDeclaration;
  sax_modules.attributeDecl=SVGAttributeDeclaration;
  sax_modules.elementDecl=SVGElementDeclaration;
  sax_modules.unparsedEntityDecl=SVGUnparsedEntityDeclaration;
  sax_modules.setDocumentLocator=SVGSetDocumentLocator;
  sax_modules.startDocument=SVGStartDocument;
  sax_modules.endDocument=SVGEndDocument;
  sax_modules.startElement=SVGStartElement;
  sax_modules.endElement=SVGEndElement;
  sax_modules.reference=SVGReference;
  sax_modules.characters=SVGCharacters;
  sax_modules.ignorableWhitespace=SVGIgnorableWhitespace;
  sax_modules.processingInstruction=SVGProcessingInstructions;
  sax_modules.comment=SVGComment;
  sax_modules.warning=SVGWarning;
  sax_modules.error=SVGError;
  sax_modules.fatalError=SVGError;
  sax_modules.getParameterEntity=SVGGetParameterEntity;
  sax_modules.cdataBlock=SVGCDataBlock;
  sax_modules.externalSubset=SVGExternalSubset;
  sax_handler=(&sax_modules);
  n=ReadBlob(image,MaxTextExtent,message);
  if (n > 0)
    {
      svg_info->parser=xmlCreatePushParserCtxt(sax_handler,svg_info,(char *)
        message,n,image->filename);
      while ((n=ReadBlob(image,MaxTextExtent,message)) != 0)
      {
        status=xmlParseChunk(svg_info->parser,(char *) message,(int) n,0);
        if (status != 0)
          break;
      }
    }
  (void) xmlParseChunk(svg_info->parser,(char *) message,0,1);
  xmlFreeParserCtxt(svg_info->parser);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"end SAX");
  xmlCleanupParser();
  (void) fclose(file);
  (void) CloseBlob(image);
  image->columns=svg_info->width;
  image->rows=svg_info->height;
  if (exception->severity >= ErrorException)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  if (image_info->ping == MagickFalse)
    {
      ImageInfo
        *read_info;

      /*
        Draw image.
      */
      image=DestroyImage(image);
      image=(Image *) NULL;
      read_info=CloneImageInfo(image_info);
      SetImageInfoBlob(read_info,(void *) NULL,0);
      if (read_info->density != (char *) NULL)
        read_info->density=DestroyString(read_info->density);
      (void) FormatLocaleString(read_info->filename,MaxTextExtent,"mvg:%s",
        filename);
      image=ReadImage(read_info,exception);
      read_info=DestroyImageInfo(read_info);
      if (image != (Image *) NULL)
        (void) CopyMagickString(image->filename,image_info->filename,
          MaxTextExtent);
    }
  /*
    Relinquish resources.
  */
  if (image != (Image *) NULL)
    {
      if (svg_info->title != (char *) NULL)
        (void) SetImageProperty(image,"svg:title",svg_info->title);
      if (svg_info->comment != (char *) NULL)
        (void) SetImageProperty(image,"svg:comment",svg_info->comment);
    }
  svg_info=DestroySVGInfo(svg_info);
  (void) RelinquishUniqueFileResource(filename);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S V G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSVGImage() adds attributes for the SVG image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSVGImage method is:
%
%      size_t RegisterSVGImage(void)
%
*/
ModuleExport size_t RegisterSVGImage(void)
{
  char
    version[MaxTextExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(LIBXML_DOTTED_VERSION)
  (void) CopyMagickString(version,"XML " LIBXML_DOTTED_VERSION,MaxTextExtent);
#endif
#if defined(MAGICKCORE_RSVG_DELEGATE)
  g_type_init();
  (void) FormatLocaleString(version,MaxTextExtent,"RSVG %d.%d.%d",
    LIBRSVG_MAJOR_VERSION,LIBRSVG_MINOR_VERSION,LIBRSVG_MICRO_VERSION);
#endif
  entry=SetMagickInfo("SVG");
#if defined(MAGICKCORE_XML_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadSVGImage;
#endif
  entry->encoder=(EncodeImageHandler *) WriteSVGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickFalse;
  entry->description=ConstantString("Scalable Vector Graphics");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->magick=(IsImageFormatHandler *) IsSVG;
  entry->module=ConstantString("SVG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("SVGZ");
#if defined(MAGICKCORE_XML_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadSVGImage;
#endif
  entry->encoder=(EncodeImageHandler *) WriteSVGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickFalse;
  entry->description=ConstantString("Compressed Scalable Vector Graphics");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->magick=(IsImageFormatHandler *) IsSVG;
  entry->module=ConstantString("SVG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MSVG");
#if defined(MAGICKCORE_XML_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadSVGImage;
#endif
  entry->encoder=(EncodeImageHandler *) WriteSVGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickFalse;
  entry->description=ConstantString("ImageMagick's own SVG internal renderer");
  entry->magick=(IsImageFormatHandler *) IsSVG;
  entry->module=ConstantString("SVG");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S V G I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSVGImage() removes format registrations made by the
%  SVG module from the list of supported formats.
%
%  The format of the UnregisterSVGImage method is:
%
%      UnregisterSVGImage(void)
%
*/
ModuleExport void UnregisterSVGImage(void)
{
  (void) UnregisterMagickInfo("SVGZ");
  (void) UnregisterMagickInfo("SVG");
  (void) UnregisterMagickInfo("MSVG");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S V G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteSVGImage() writes a image in the SVG - XML based W3C standard
%  format.
%
%  The format of the WriteSVGImage method is:
%
%      MagickBooleanType WriteSVGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static void AffineToTransform(Image *image,AffineMatrix *affine)
{
  char
    transform[MaxTextExtent];

  if ((fabs(affine->tx) < MagickEpsilon) && (fabs(affine->ty) < MagickEpsilon))
    {
      if ((fabs(affine->rx) < MagickEpsilon) &&
          (fabs(affine->ry) < MagickEpsilon))
        {
          if ((fabs(affine->sx-1.0) < MagickEpsilon) &&
              (fabs(affine->sy-1.0) < MagickEpsilon))
            {
              (void) WriteBlobString(image,"\">\n");
              return;
            }
          (void) FormatLocaleString(transform,MaxTextExtent,
            "\" transform=\"scale(%g,%g)\">\n",affine->sx,affine->sy);
          (void) WriteBlobString(image,transform);
          return;
        }
      else
        {
          if ((fabs(affine->sx-affine->sy) < MagickEpsilon) &&
              (fabs(affine->rx+affine->ry) < MagickEpsilon) &&
              (fabs(affine->sx*affine->sx+affine->rx*affine->rx-1.0) <
               2*MagickEpsilon))
            {
              double
                theta;

              theta=(180.0/MagickPI)*atan2(affine->rx,affine->sx);
              (void) FormatLocaleString(transform,MaxTextExtent,
                "\" transform=\"rotate(%g)\">\n",theta);
              (void) WriteBlobString(image,transform);
              return;
            }
        }
    }
  else
    {
      if ((fabs(affine->sx-1.0) < MagickEpsilon) &&
          (fabs(affine->rx) < MagickEpsilon) &&
          (fabs(affine->ry) < MagickEpsilon) &&
          (fabs(affine->sy-1.0) < MagickEpsilon))
        {
          (void) FormatLocaleString(transform,MaxTextExtent,
            "\" transform=\"translate(%g,%g)\">\n",affine->tx,affine->ty);
          (void) WriteBlobString(image,transform);
          return;
        }
    }
  (void) FormatLocaleString(transform,MaxTextExtent,
    "\" transform=\"matrix(%g %g %g %g %g %g)\">\n",
    affine->sx,affine->rx,affine->ry,affine->sy,affine->tx,affine->ty);
  (void) WriteBlobString(image,transform);
}

static MagickBooleanType IsPoint(const char *point)
{
  char
    *p;

  ssize_t
    value;

  value=strtol(point,&p,10);
  (void) value;
  return(p != point ? MagickTrue : MagickFalse);
}

static MagickBooleanType TraceSVGImage(Image *image)
{
  ssize_t
    y;

  register const PixelPacket
    *p;

  register ssize_t
    x;

#if defined(MAGICKCORE_AUTOTRACE_DELEGATE)
  {
    at_bitmap_type
      *trace;

    at_fitting_opts_type
      *fitting_options;

    at_output_opts_type
      *output_options;

    at_splines_type
      *splines;

    ImageType
      type;

    register ssize_t
      i;

    size_t
      number_planes;

    /*
      Trace image and write as SVG.
    */
    fitting_options=at_fitting_opts_new();
    output_options=at_output_opts_new();
    type=GetImageType(image,&image->exception);
    number_planes=3;
    if ((type == BilevelType) || (type == GrayscaleType))
      number_planes=1;
    trace=at_bitmap_new(image->columns,image->rows,number_planes);
    i=0;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        trace->bitmap[i++]=GetPixelRed(p);
        if (number_planes == 3)
          {
            trace->bitmap[i++]=GetPixelGreen(p);
            trace->bitmap[i++]=GetPixelBlue(p);
          }
        p++;
      }
    }
    splines=at_splines_new_full(trace,fitting_options,NULL,NULL,NULL,NULL,NULL,
      NULL);
    at_splines_write(at_output_get_handler_by_suffix((char *) "svg"),
      GetBlobFileHandle(image),image->filename,output_options,splines,NULL,
      NULL);
    /*
      Free resources.
    */
    at_splines_free(splines);
    at_bitmap_free(trace);
    at_output_opts_free(output_options);
    at_fitting_opts_free(fitting_options);
  }
#else
  {
    char
      message[MaxTextExtent],
      tuple[MaxTextExtent];

    MagickPixelPacket
      pixel;

    register const IndexPacket
      *indexes;

    (void) WriteBlobString(image,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
    (void) WriteBlobString(image,
      "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"\n");
    (void) WriteBlobString(image,
      "  \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");
    (void) FormatLocaleString(message,MaxTextExtent,
      "<svg width=\"%.20g\" height=\"%.20g\">\n",(double) image->columns,
      (double) image->rows);
    (void) WriteBlobString(image,message);
    GetMagickPixelPacket(image,&pixel);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetMagickPixelPacket(image,p,indexes+x,&pixel);
        (void) QueryMagickColorname(image,&pixel,SVGCompliance,tuple,
          &image->exception);
        (void) FormatLocaleString(message,MaxTextExtent,
          "  <circle cx=\"%.20g\" cy=\"%.20g\" r=\"1\" fill=\"%s\"/>\n",
          (double) x,(double) y,tuple);
        (void) WriteBlobString(image,message);
        p++;
      }
    }
    (void) WriteBlobString(image,"</svg>\n");
  }
#endif
  return(MagickTrue);
}

static MagickBooleanType WriteSVGImage(const ImageInfo *image_info,Image *image)
{
#define BezierQuantum  200

  AffineMatrix
    affine;

  char
    keyword[MaxTextExtent],
    message[MaxTextExtent],
    name[MaxTextExtent],
    *token,
    type[MaxTextExtent];

  const char
    *p,
    *q,
    *value;

  int
    n;

  ssize_t
    j;

  MagickBooleanType
    active,
    status;

  PointInfo
    point;

  PrimitiveInfo
    *primitive_info;

  PrimitiveType
    primitive_type;

  register ssize_t
    x;

  register ssize_t
    i;

  size_t
    length;

  SVGInfo
    svg_info;

  size_t
    number_points;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  value=GetImageArtifact(image,"SVG");
  if (value != (char *) NULL)
    {
      (void) WriteBlobString(image,value);
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  value=GetImageArtifact(image,"MVG");
  if (value == (char *) NULL)
    return(TraceSVGImage(image));
  /*
    Write SVG header.
  */
  (void) WriteBlobString(image,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
  (void) WriteBlobString(image,
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"\n");
  (void) WriteBlobString(image,
    "  \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");
  (void) FormatLocaleString(message,MaxTextExtent,
    "<svg width=\"%.20g\" height=\"%.20g\">\n",(double) image->columns,(double)
    image->rows);
  (void) WriteBlobString(image,message);
  /*
    Allocate primitive info memory.
  */
  number_points=2047;
  primitive_info=(PrimitiveInfo *) AcquireQuantumMemory(number_points,
    sizeof(*primitive_info));
  if (primitive_info == (PrimitiveInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  GetAffineMatrix(&affine);
  token=AcquireString(value);
  active=MagickFalse;
  n=0;
  status=MagickTrue;
  for (q=(const char *) value; *q != '\0'; )
  {
    /*
      Interpret graphic primitive.
    */
    GetMagickToken(q,&q,keyword);
    if (*keyword == '\0')
      break;
    if (*keyword == '#')
      {
        /*
          Comment.
        */
        if (active != MagickFalse)
          {
            AffineToTransform(image,&affine);
            active=MagickFalse;
          }
        (void) WriteBlobString(image,"<desc>");
        (void) WriteBlobString(image,keyword+1);
        for ( ; (*q != '\n') && (*q != '\0'); q++)
          switch (*q)
          {
            case '<': (void) WriteBlobString(image,"&lt;"); break;
            case '>': (void) WriteBlobString(image,"&gt;"); break;
            case '&': (void) WriteBlobString(image,"&amp;"); break;
            default: (void) WriteBlobByte(image,*q); break;
          }
        (void) WriteBlobString(image,"</desc>\n");
        continue;
      }
    primitive_type=UndefinedPrimitive;
    switch (*keyword)
    {
      case ';':
        break;
      case 'a':
      case 'A':
      {
        if (LocaleCompare("affine",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            affine.sx=StringToDouble(token,(char **) NULL);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            affine.rx=StringToDouble(token,(char **) NULL);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            affine.ry=StringToDouble(token,(char **) NULL);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            affine.sy=StringToDouble(token,(char **) NULL);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            affine.tx=StringToDouble(token,(char **) NULL);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            affine.ty=StringToDouble(token,(char **) NULL);
            break;
          }
        if (LocaleCompare("angle",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            affine.rx=StringToDouble(token,(char **) NULL);
            affine.ry=StringToDouble(token,(char **) NULL);
            break;
          }
        if (LocaleCompare("arc",keyword) == 0)
          {
            primitive_type=ArcPrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'b':
      case 'B':
      {
        if (LocaleCompare("bezier",keyword) == 0)
          {
            primitive_type=BezierPrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'c':
      case 'C':
      {
        if (LocaleCompare("clip-path",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "clip-path:url(#%s);",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("clip-rule",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "clip-rule:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("clip-units",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "clipPathUnits=%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("circle",keyword) == 0)
          {
            primitive_type=CirclePrimitive;
            break;
          }
        if (LocaleCompare("color",keyword) == 0)
          {
            primitive_type=ColorPrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'd':
      case 'D':
      {
        if (LocaleCompare("decorate",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "text-decoration:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'e':
      case 'E':
      {
        if (LocaleCompare("ellipse",keyword) == 0)
          {
            primitive_type=EllipsePrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'f':
      case 'F':
      {
        if (LocaleCompare("fill",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,"fill:%s;",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("fill-rule",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "fill-rule:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("fill-opacity",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "fill-opacity:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-family",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "font-family:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-stretch",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "font-stretch:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-style",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "font-style:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-size",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "font-size:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-weight",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "font-weight:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'g':
      case 'G':
      {
        if (LocaleCompare("gradient-units",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            break;
          }
        if (LocaleCompare("text-align",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "text-align %s ",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("text-anchor",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "text-anchor %s ",token);
            (void) WriteBlobString(image,message);
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'i':
      case 'I':
      {
        if (LocaleCompare("image",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            primitive_type=ImagePrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'l':
      case 'L':
      {
        if (LocaleCompare("line",keyword) == 0)
          {
            primitive_type=LinePrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'm':
      case 'M':
      {
        if (LocaleCompare("matte",keyword) == 0)
          {
            primitive_type=MattePrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'o':
      case 'O':
      {
        if (LocaleCompare("opacity",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,"opacity %s ",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'p':
      case 'P':
      {
        if (LocaleCompare("path",keyword) == 0)
          {
            primitive_type=PathPrimitive;
            break;
          }
        if (LocaleCompare("point",keyword) == 0)
          {
            primitive_type=PointPrimitive;
            break;
          }
        if (LocaleCompare("polyline",keyword) == 0)
          {
            primitive_type=PolylinePrimitive;
            break;
          }
        if (LocaleCompare("polygon",keyword) == 0)
          {
            primitive_type=PolygonPrimitive;
            break;
          }
        if (LocaleCompare("pop",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            if (LocaleCompare("clip-path",token) == 0)
              {
                (void) WriteBlobString(image,"</clipPath>\n");
                break;
              }
            if (LocaleCompare("defs",token) == 0)
              {
                (void) WriteBlobString(image,"</defs>\n");
                break;
              }
            if (LocaleCompare("gradient",token) == 0)
              {
                (void) FormatLocaleString(message,MaxTextExtent,
                  "</%sGradient>\n",type);
                (void) WriteBlobString(image,message);
                break;
              }
            if (LocaleCompare("graphic-context",token) == 0)
              {
                n--;
                if (n < 0)
                  ThrowWriterException(DrawError,
                    "UnbalancedGraphicContextPushPop");
                (void) WriteBlobString(image,"</g>\n");
              }
            if (LocaleCompare("pattern",token) == 0)
              {
                (void) WriteBlobString(image,"</pattern>\n");
                break;
              }
            if (LocaleCompare("defs",token) == 0)
            (void) WriteBlobString(image,"</g>\n");
            break;
          }
        if (LocaleCompare("push",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            if (LocaleCompare("clip-path",token) == 0)
              {
                GetMagickToken(q,&q,token);
                (void) FormatLocaleString(message,MaxTextExtent,
                  "<clipPath id=\"%s\">\n",token);
                (void) WriteBlobString(image,message);
                break;
              }
            if (LocaleCompare("defs",token) == 0)
              {
                (void) WriteBlobString(image,"<defs>\n");
                break;
              }
            if (LocaleCompare("gradient",token) == 0)
              {
                GetMagickToken(q,&q,token);
                (void) CopyMagickString(name,token,MaxTextExtent);
                GetMagickToken(q,&q,token);
                (void) CopyMagickString(type,token,MaxTextExtent);
                GetMagickToken(q,&q,token);
                svg_info.segment.x1=StringToDouble(token,(char **) NULL);
                svg_info.element.cx=StringToDouble(token,(char **) NULL);
                GetMagickToken(q,&q,token);
                if (*token == ',')
                  GetMagickToken(q,&q,token);
                svg_info.segment.y1=StringToDouble(token,(char **) NULL);
                svg_info.element.cy=StringToDouble(token,(char **) NULL);
                GetMagickToken(q,&q,token);
                if (*token == ',')
                  GetMagickToken(q,&q,token);
                svg_info.segment.x2=StringToDouble(token,(char **) NULL);
                svg_info.element.major=StringToDouble(token,
                  (char **) NULL);
                GetMagickToken(q,&q,token);
                if (*token == ',')
                  GetMagickToken(q,&q,token);
                svg_info.segment.y2=StringToDouble(token,(char **) NULL);
                svg_info.element.minor=StringToDouble(token,
                  (char **) NULL);
                (void) FormatLocaleString(message,MaxTextExtent,
                  "<%sGradient id=\"%s\" x1=\"%g\" y1=\"%g\" x2=\"%g\" "
                  "y2=\"%g\">\n",type,name,svg_info.segment.x1,
                  svg_info.segment.y1,svg_info.segment.x2,svg_info.segment.y2);
                if (LocaleCompare(type,"radial") == 0)
                  {
                    GetMagickToken(q,&q,token);
                    if (*token == ',')
                      GetMagickToken(q,&q,token);
                    svg_info.element.angle=StringToDouble(token,
                      (char **) NULL);
                    (void) FormatLocaleString(message,MaxTextExtent,
                      "<%sGradient id=\"%s\" cx=\"%g\" cy=\"%g\" r=\"%g\" "
                      "fx=\"%g\" fy=\"%g\">\n",type,name,
                      svg_info.element.cx,svg_info.element.cy,
                      svg_info.element.angle,svg_info.element.major,
                      svg_info.element.minor);
                  }
                (void) WriteBlobString(image,message);
                break;
              }
            if (LocaleCompare("graphic-context",token) == 0)
              {
                n++;
                if (active)
                  {
                    AffineToTransform(image,&affine);
                    active=MagickFalse;
                  }
                (void) WriteBlobString(image,"<g style=\"");
                active=MagickTrue;
              }
            if (LocaleCompare("pattern",token) == 0)
              {
                GetMagickToken(q,&q,token);
                (void) CopyMagickString(name,token,MaxTextExtent);
                GetMagickToken(q,&q,token);
                svg_info.bounds.x=StringToDouble(token,(char **) NULL);
                GetMagickToken(q,&q,token);
                if (*token == ',')
                  GetMagickToken(q,&q,token);
                svg_info.bounds.y=StringToDouble(token,(char **) NULL);
                GetMagickToken(q,&q,token);
                if (*token == ',')
                  GetMagickToken(q,&q,token);
                svg_info.bounds.width=StringToDouble(token,
                  (char **) NULL);
                GetMagickToken(q,&q,token);
                if (*token == ',')
                  GetMagickToken(q,&q,token);
                svg_info.bounds.height=StringToDouble(token,
                  (char **) NULL);
                (void) FormatLocaleString(message,MaxTextExtent,
                  "<pattern id=\"%s\" x=\"%g\" y=\"%g\" width=\"%g\" "
                  "height=\"%g\">\n",name,svg_info.bounds.x,
                  svg_info.bounds.y,svg_info.bounds.width,
                  svg_info.bounds.height);
                (void) WriteBlobString(image,message);
                break;
              }
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'r':
      case 'R':
      {
        if (LocaleCompare("rectangle",keyword) == 0)
          {
            primitive_type=RectanglePrimitive;
            break;
          }
        if (LocaleCompare("roundRectangle",keyword) == 0)
          {
            primitive_type=RoundRectanglePrimitive;
            break;
          }
        if (LocaleCompare("rotate",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,"rotate(%s) ",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        status=MagickFalse;
        break;
      }
      case 's':
      case 'S':
      {
        if (LocaleCompare("scale",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            affine.sx=StringToDouble(token,(char **) NULL);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            affine.sy=StringToDouble(token,(char **) NULL);
            break;
          }
        if (LocaleCompare("skewX",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,"skewX(%s) ",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("skewY",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,"skewY(%s) ",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stop-color",keyword) == 0)
          {
            char
              color[MaxTextExtent];

            GetMagickToken(q,&q,token);
            (void) CopyMagickString(color,token,MaxTextExtent);
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "  <stop offset=\"%s\" stop-color=\"%s\" />\n",token,color);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,"stroke:%s;",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-antialias",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-antialias:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-dasharray",keyword) == 0)
          {
            if (IsPoint(q))
              {
                ssize_t
                  k;

                p=q;
                GetMagickToken(p,&p,token);
                for (k=0; IsPoint(token); k++)
                  GetMagickToken(p,&p,token);
                (void) WriteBlobString(image,"stroke-dasharray:");
                for (j=0; j < k; j++)
                {
                  GetMagickToken(q,&q,token);
                  (void) FormatLocaleString(message,MaxTextExtent,"%s ",
                    token);
                  (void) WriteBlobString(image,message);
                }
                (void) WriteBlobString(image,";");
                break;
              }
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-dasharray:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-dashoffset",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-dashoffset:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-linecap",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-linecap:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-linejoin",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-linejoin:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-miterlimit",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-miterlimit:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-opacity",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-opacity:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-width",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "stroke-width:%s;",token);
            (void) WriteBlobString(image,message);
            continue;
          }
        status=MagickFalse;
        break;
      }
      case 't':
      case 'T':
      {
        if (LocaleCompare("text",keyword) == 0)
          {
            primitive_type=TextPrimitive;
            break;
          }
        if (LocaleCompare("text-antialias",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            (void) FormatLocaleString(message,MaxTextExtent,
              "text-antialias:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("tspan",keyword) == 0)
          {
            primitive_type=TextPrimitive;
            break;
          }
        if (LocaleCompare("translate",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            affine.tx=StringToDouble(token,(char **) NULL);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            affine.ty=StringToDouble(token,(char **) NULL);
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'v':
      case 'V':
      {
        if (LocaleCompare("viewbox",keyword) == 0)
          {
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            GetMagickToken(q,&q,token);
            if (*token == ',')
              GetMagickToken(q,&q,token);
            GetMagickToken(q,&q,token);
            break;
          }
        status=MagickFalse;
        break;
      }
      default:
      {
        status=MagickFalse;
        break;
      }
    }
    if (status == MagickFalse)
      break;
    if (primitive_type == UndefinedPrimitive)
      continue;
    /*
      Parse the primitive attributes.
    */
    i=0;
    j=0;
    for (x=0; *q != '\0'; x++)
    {
      /*
        Define points.
      */
      if (IsPoint(q) == MagickFalse)
        break;
      GetMagickToken(q,&q,token);
      point.x=StringToDouble(token,(char **) NULL);
      GetMagickToken(q,&q,token);
      if (*token == ',')
        GetMagickToken(q,&q,token);
      point.y=StringToDouble(token,(char **) NULL);
      GetMagickToken(q,(const char **) NULL,token);
      if (*token == ',')
        GetMagickToken(q,&q,token);
      primitive_info[i].primitive=primitive_type;
      primitive_info[i].point=point;
      primitive_info[i].coordinates=0;
      primitive_info[i].method=FloodfillMethod;
      i++;
      if (i < (ssize_t) (number_points-6*BezierQuantum-360))
        continue;
      number_points+=6*BezierQuantum+360;
      primitive_info=(PrimitiveInfo *) ResizeQuantumMemory(primitive_info,
        number_points,sizeof(*primitive_info));
      if (primitive_info == (PrimitiveInfo *) NULL)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
          break;
        }
    }
    primitive_info[j].primitive=primitive_type;
    primitive_info[j].coordinates=x;
    primitive_info[j].method=FloodfillMethod;
    primitive_info[j].text=(char *) NULL;
    if (active)
      {
        AffineToTransform(image,&affine);
        active=MagickFalse;
      }
    active=MagickFalse;
    switch (primitive_type)
    {
      case PointPrimitive:
      default:
      {
        if (primitive_info[j].coordinates != 1)
          {
            status=MagickFalse;
            break;
          }
        break;
      }
      case LinePrimitive:
      {
        if (primitive_info[j].coordinates != 2)
          {
            status=MagickFalse;
            break;
          }
          (void) FormatLocaleString(message,MaxTextExtent,
          "  <line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\"/>\n",
          primitive_info[j].point.x,primitive_info[j].point.y,
          primitive_info[j+1].point.x,primitive_info[j+1].point.y);
        (void) WriteBlobString(image,message);
        break;
      }
      case RectanglePrimitive:
      {
        if (primitive_info[j].coordinates != 2)
          {
            status=MagickFalse;
            break;
          }
          (void) FormatLocaleString(message,MaxTextExtent,
          "  <rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\"/>\n",
          primitive_info[j].point.x,primitive_info[j].point.y,
          primitive_info[j+1].point.x-primitive_info[j].point.x,
          primitive_info[j+1].point.y-primitive_info[j].point.y);
        (void) WriteBlobString(image,message);
        break;
      }
      case RoundRectanglePrimitive:
      {
        if (primitive_info[j].coordinates != 3)
          {
            status=MagickFalse;
            break;
          }
        (void) FormatLocaleString(message,MaxTextExtent,
          "  <rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" rx=\"%g\" "
          "ry=\"%g\"/>\n",primitive_info[j].point.x,
          primitive_info[j].point.y,primitive_info[j+1].point.x-
          primitive_info[j].point.x,primitive_info[j+1].point.y-
          primitive_info[j].point.y,primitive_info[j+2].point.x,
          primitive_info[j+2].point.y);
        (void) WriteBlobString(image,message);
        break;
      }
      case ArcPrimitive:
      {
        if (primitive_info[j].coordinates != 3)
          {
            status=MagickFalse;
            break;
          }
        break;
      }
      case EllipsePrimitive:
      {
        if (primitive_info[j].coordinates != 3)
          {
            status=MagickFalse;
            break;
          }
          (void) FormatLocaleString(message,MaxTextExtent,
          "  <ellipse cx=\"%g\" cy=\"%g\" rx=\"%g\" ry=\"%g\"/>\n",
          primitive_info[j].point.x,primitive_info[j].point.y,
          primitive_info[j+1].point.x,primitive_info[j+1].point.y);
        (void) WriteBlobString(image,message);
        break;
      }
      case CirclePrimitive:
      {
        double
          alpha,
          beta;

        if (primitive_info[j].coordinates != 2)
          {
            status=MagickFalse;
            break;
          }
        alpha=primitive_info[j+1].point.x-primitive_info[j].point.x;
        beta=primitive_info[j+1].point.y-primitive_info[j].point.y;
        (void) FormatLocaleString(message,MaxTextExtent,
          "  <circle cx=\"%g\" cy=\"%g\" r=\"%g\"/>\n",
          primitive_info[j].point.x,primitive_info[j].point.y,
          hypot(alpha,beta));
        (void) WriteBlobString(image,message);
        break;
      }
      case PolylinePrimitive:
      {
        if (primitive_info[j].coordinates < 2)
          {
            status=MagickFalse;
            break;
          }
        (void) CopyMagickString(message,"  <polyline points=\"",MaxTextExtent);
        (void) WriteBlobString(image,message);
        length=strlen(message);
        for ( ; j < i; j++)
        {
          (void) FormatLocaleString(message,MaxTextExtent,"%g,%g ",
            primitive_info[j].point.x,primitive_info[j].point.y);
          length+=strlen(message);
          if (length >= 80)
            {
              (void) WriteBlobString(image,"\n    ");
              length=strlen(message)+5;
            }
          (void) WriteBlobString(image,message);
        }
        (void) WriteBlobString(image,"\"/>\n");
        break;
      }
      case PolygonPrimitive:
      {
        if (primitive_info[j].coordinates < 3)
          {
            status=MagickFalse;
            break;
          }
        primitive_info[i]=primitive_info[j];
        primitive_info[i].coordinates=0;
        primitive_info[j].coordinates++;
        i++;
        (void) CopyMagickString(message,"  <polygon points=\"",MaxTextExtent);
        (void) WriteBlobString(image,message);
        length=strlen(message);
        for ( ; j < i; j++)
        {
          (void) FormatLocaleString(message,MaxTextExtent,"%g,%g ",
            primitive_info[j].point.x,primitive_info[j].point.y);
          length+=strlen(message);
          if (length >= 80)
            {
              (void) WriteBlobString(image,"\n    ");
              length=strlen(message)+5;
            }
          (void) WriteBlobString(image,message);
        }
        (void) WriteBlobString(image,"\"/>\n");
        break;
      }
      case BezierPrimitive:
      {
        if (primitive_info[j].coordinates < 3)
          {
            status=MagickFalse;
            break;
          }
        break;
      }
      case PathPrimitive:
      {
        int
          number_attributes;

        GetMagickToken(q,&q,token);
        number_attributes=1;
        for (p=token; *p != '\0'; p++)
          if (isalpha((int) *p))
            number_attributes++;
        if (i > (ssize_t) (number_points-6*BezierQuantum*number_attributes-1))
          {
            number_points+=6*BezierQuantum*number_attributes;
            primitive_info=(PrimitiveInfo *) ResizeQuantumMemory(primitive_info,
              number_points,sizeof(*primitive_info));
            if (primitive_info == (PrimitiveInfo *) NULL)
              {
                (void) ThrowMagickException(&image->exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                break;
              }
          }
        (void) WriteBlobString(image,"  <path d=\"");
        (void) WriteBlobString(image,token);
        (void) WriteBlobString(image,"\"/>\n");
        break;
      }
      case ColorPrimitive:
      case MattePrimitive:
      {
        if (primitive_info[j].coordinates != 1)
          {
            status=MagickFalse;
            break;
          }
        GetMagickToken(q,&q,token);
        if (LocaleCompare("point",token) == 0)
          primitive_info[j].method=PointMethod;
        if (LocaleCompare("replace",token) == 0)
          primitive_info[j].method=ReplaceMethod;
        if (LocaleCompare("floodfill",token) == 0)
          primitive_info[j].method=FloodfillMethod;
        if (LocaleCompare("filltoborder",token) == 0)
          primitive_info[j].method=FillToBorderMethod;
        if (LocaleCompare("reset",token) == 0)
          primitive_info[j].method=ResetMethod;
        break;
      }
      case TextPrimitive:
      {
        register char
          *p;

        if (primitive_info[j].coordinates != 1)
          {
            status=MagickFalse;
            break;
          }
        GetMagickToken(q,&q,token);
        (void) FormatLocaleString(message,MaxTextExtent,
          "  <text x=\"%g\" y=\"%g\">",primitive_info[j].point.x,
          primitive_info[j].point.y);
        (void) WriteBlobString(image,message);
        for (p=token; *p != '\0'; p++)
          switch (*p)
          {
            case '<': (void) WriteBlobString(image,"&lt;"); break;
            case '>': (void) WriteBlobString(image,"&gt;"); break;
            case '&': (void) WriteBlobString(image,"&amp;"); break;
            default: (void) WriteBlobByte(image,*p); break;
          }
        (void) WriteBlobString(image,"</text>\n");
        break;
      }
      case ImagePrimitive:
      {
        if (primitive_info[j].coordinates != 2)
          {
            status=MagickFalse;
            break;
          }
        GetMagickToken(q,&q,token);
        (void) FormatLocaleString(message,MaxTextExtent,
          "  <image x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" "
          "xlink:href=\"%s\"/>\n",primitive_info[j].point.x,
          primitive_info[j].point.y,primitive_info[j+1].point.x,
          primitive_info[j+1].point.y,token);
        (void) WriteBlobString(image,message);
        break;
      }
    }
    if (primitive_info == (PrimitiveInfo *) NULL)
      break;
    primitive_info[i].primitive=UndefinedPrimitive;
    if (status == MagickFalse)
      break;
  }
  (void) WriteBlobString(image,"</svg>\n");
  /*
    Relinquish resources.
  */
  token=DestroyString(token);
  if (primitive_info != (PrimitiveInfo *) NULL)
    primitive_info=(PrimitiveInfo *) RelinquishMagickMemory(primitive_info);
  (void) CloseBlob(image);
  return(status);
}

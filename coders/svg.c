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
%                                   Cristy                                    %
%                             William Radcliffe                               %
%                                March 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/constitute.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/delegate.h"
#include "MagickCore/delegate-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"

#if defined(MAGICKCORE_XML_DELEGATE)
#  if defined(MAGICKCORE_WINDOWS_SUPPORT)
#    if !defined(__MINGW32__)
#      include <win32config.h>
#    endif
#  endif
#  include <libxml/xmlmemory.h>
#  include <libxml/parserInternals.h>
#  include <libxml/xmlerror.h>
#endif

#if defined(MAGICKCORE_AUTOTRACE_DELEGATE)
#include "autotrace/autotrace.h"
#endif

#if defined(MAGICKCORE_RSVG_DELEGATE)
#include "librsvg/rsvg.h"
#if !defined(LIBRSVG_CHECK_VERSION)
#include "librsvg/rsvg-cairo.h"
#include "librsvg/librsvg-features.h"
#elif !LIBRSVG_CHECK_VERSION(2,36,2)
#include "librsvg/rsvg-cairo.h"
#include "librsvg/librsvg-features.h"
#endif
#endif

/*
  Define declarations.
*/
#define DefaultSVGDensity  96.0

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
    text_offset,
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

  ssize_t
    svgDepth;
} SVGInfo;

/*
  Static declarations.
*/
static char
  SVGDensityGeometry[] = "96.0x96.0";

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSVGImage(const ImageInfo *,Image *,ExceptionInfo *);

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
  if (LocaleNCompare((const char *) magick+1,"svg",3) == 0)
    return(MagickTrue);
  if (length < 5)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick+1,"?xml",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

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

static Image *RenderSVGImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    background[MagickPathExtent],
    command[MagickPathExtent],
    *density,
    input_filename[MagickPathExtent],
    opacity[MagickPathExtent],
    output_filename[MagickPathExtent],
    unique[MagickPathExtent];

  const DelegateInfo
    *delegate_info;

  Image
    *next;

  int
    status;

  struct stat
    attributes;

  /*
    Our best hope for compliance with the SVG standard.
  */
  delegate_info=GetDelegateInfo("svg:decode",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    return((Image *) NULL);
  status=AcquireUniqueSymbolicLink(image->filename,input_filename);
  (void) AcquireUniqueFilename(unique);
  (void) FormatLocaleString(output_filename,MagickPathExtent,"%s.png",unique);
  (void) AcquireUniqueFilename(unique);
  density=AcquireString("");
  (void) FormatLocaleString(density,MagickPathExtent,"%.20g",
    ceil(sqrt(image->resolution.x*image->resolution.y)-0.5));
  (void) FormatLocaleString(background,MagickPathExtent,
    "rgb(%.20g%%,%.20g%%,%.20g%%)",
    100.0*QuantumScale*image->background_color.red,
    100.0*QuantumScale*image->background_color.green,
    100.0*QuantumScale*image->background_color.blue);
  (void) FormatLocaleString(opacity,MagickPathExtent,"%.20g",QuantumScale*
    image->background_color.alpha);
  (void) FormatLocaleString(command,MagickPathExtent,
    GetDelegateCommands(delegate_info),input_filename,output_filename,density,
    background,opacity,unique);
  density=DestroyString(density);
  status=ExternalDelegateCommand(MagickFalse,image_info->verbose,command,
    (char *) NULL,exception);
  (void) RelinquishUniqueFileResource(unique);
  (void) RelinquishUniqueFileResource(input_filename);
  if ((status == 0) && (stat(output_filename,&attributes) == 0) &&
      (attributes.st_size > 0))
    {
      Image
        *svg_image;

      ImageInfo
        *read_info;

      read_info=CloneImageInfo(image_info);
      (void) CopyMagickString(read_info->filename,output_filename,
        MagickPathExtent);
      svg_image=ReadImage(read_info,exception);
      read_info=DestroyImageInfo(read_info);
      if (svg_image != (Image *) NULL)
        {
          (void) RelinquishUniqueFileResource(output_filename);
          for (next=GetFirstImageInList(svg_image); next != (Image *) NULL; )
          {
            (void) CopyMagickString(next->filename,image->filename,
              MaxTextExtent);
            (void) CopyMagickString(next->magick,image->magick,MaxTextExtent);
            next=GetNextImageInList(next);
          }
          return(svg_image);
        }
    }
  (void) RelinquishUniqueFileResource(output_filename);
  return((Image *) NULL);
}

#if defined(MAGICKCORE_XML_DELEGATE)
static SVGInfo *AcquireSVGInfo(void)
{
  SVGInfo
    *svg_info;

  svg_info=(SVGInfo *) AcquireMagickMemory(sizeof(*svg_info));
  if (svg_info == (SVGInfo *) NULL)
    return((SVGInfo *) NULL);
  (void) memset(svg_info,0,sizeof(*svg_info));
  svg_info->text=AcquireString("");
  svg_info->scale=(double *) AcquireCriticalMemory(sizeof(*svg_info->scale));
  GetAffineMatrix(&svg_info->affine);
  svg_info->scale[0]=ExpandAffine(&svg_info->affine);
  return(svg_info);
}

static SVGInfo *DestroySVGInfo(SVGInfo *svg_info)
{
  if (svg_info->text != (char *) NULL)
    svg_info->text=DestroyString(svg_info->text);
  if (svg_info->scale != (double *) NULL)
    svg_info->scale=(double *) RelinquishMagickMemory(svg_info->scale);
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
    *next_token,
    token[MagickPathExtent];

  const char
    *p;

  double
    value;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",string);
  assert(string != (const char *) NULL);
  p=(const char *) string;
  (void) GetNextToken(p,&p,MagickPathExtent,token);
  value=StringToDouble(token,&next_token);
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
  (void) GetNextToken(p,&p,MagickPathExtent,token);
  if (LocaleNCompare(token,"cm",2) == 0)
    return(DefaultSVGDensity*svg_info->scale[0]/2.54*value);
  if (LocaleNCompare(token,"em",2) == 0)
    return(svg_info->pointsize*value);
  if (LocaleNCompare(token,"ex",2) == 0)
    return(svg_info->pointsize*value/2.0);
  if (LocaleNCompare(token,"in",2) == 0)
    return(DefaultSVGDensity*svg_info->scale[0]*value);
  if (LocaleNCompare(token,"mm",2) == 0)
    return(DefaultSVGDensity*svg_info->scale[0]/25.4*value);
  if (LocaleNCompare(token,"pc",2) == 0)
    return(DefaultSVGDensity*svg_info->scale[0]/6.0*value);
  if (LocaleNCompare(token,"pt",2) == 0)
    return(svg_info->scale[0]*value);
  if (LocaleNCompare(token,"px",2) == 0)
    return(value);
  return(value);
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

static void SVGStripString(const MagickBooleanType trim,char *message)
{
  register char
    *p,
    *q;

  size_t
    length;

  assert(message != (char *) NULL);
  if (*message == '\0')
    return;
  /*
    Remove comment.
  */
  q=message;
  for (p=message; *p != '\0'; p++)
  {
    if ((*p == '/') && (*(p+1) == '*'))
      {
        for ( ; *p != '\0'; p++)
          if ((*p == '*') && (*(p+1) == '/'))
            {
              p+=2;
              break;
            }
        if (*p == '\0')
          break;
      }
    *q++=(*p);
  }
  *q='\0';
  length=strlen(message);
  if ((trim != MagickFalse) && (length != 0))
    {
      /*
        Remove whitespace.
      */
      p=message;
      while (isspace((int) ((unsigned char) *p)) != 0)
        p++;
      if ((*p == '\'') || (*p == '"'))
        p++;
      q=message+length-1;
      while ((isspace((int) ((unsigned char) *q)) != 0) && (q > p))
        q--;
      if (q > p)
        if ((*q == '\'') || (*q == '"'))
          q--;
      (void) memmove(message,p,(size_t) (q-p+1));
      message[q-p+1]='\0';
    }
  /*
    Convert newlines to a space.
  */
  for (p=message; *p != '\0'; p++)
    if (*p == '\n')
      *p=' ';
}

static char **SVGKeyValuePairs(void *context,const int key_sentinel,
  const int value_sentinel,const char *text,size_t *number_tokens)
{
  char
    **tokens;

  register const char
    *p,
    *q;

  register ssize_t
    i;

  size_t
    extent;

  SVGInfo
    *svg_info;

  svg_info=(SVGInfo *) context;
  *number_tokens=0;
  if (text == (const char *) NULL)
    return((char **) NULL);
  extent=8;
  tokens=(char **) AcquireQuantumMemory(extent+2UL,sizeof(*tokens));
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
    if ((*q != key_sentinel) && (*q != value_sentinel) && (*q != '\0'))
      continue;
    if (i == (ssize_t) extent)
      {
        extent<<=1;
        tokens=(char **) ResizeQuantumMemory(tokens,extent+2,sizeof(*tokens));
        if (tokens == (char **) NULL)
          {
            (void) ThrowMagickException(svg_info->exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'",text);
            return((char **) NULL);
          }
      }
    tokens[i]=AcquireString(p);
    (void) CopyMagickString(tokens[i],p,(size_t) (q-p+1));
    SVGStripString(MagickTrue,tokens[i]);
    i++;
    p=q+1;
  }
  tokens[i]=AcquireString(p);
  (void) CopyMagickString(tokens[i],p,(size_t) (q-p+1));
  SVGStripString(MagickTrue,tokens[i++]);
  tokens[i]=(char *) NULL;
  *number_tokens=(size_t) i;
  return(tokens);
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

static void SVGProcessStyleElement(void *context,const xmlChar *name,
  const char *style)
{
  char
    background[MagickPathExtent],
    *color,
    *keyword,
    *units,
    *value;

  char
    **tokens;

  register ssize_t
    i;

  size_t
    number_tokens;

  SVGInfo
    *svg_info;

  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  ");
  svg_info=(SVGInfo *) context;
  tokens=SVGKeyValuePairs(context,':',';',style,&number_tokens);
  if (tokens == (char **) NULL)
    return;
  for (i=0; i < (ssize_t) (number_tokens-1); i+=2)
  {
    keyword=(char *) tokens[i];
    value=(char *) tokens[i+1];
    if (LocaleCompare(keyword,"font-size") != 0)
      continue;
    svg_info->pointsize=GetUserSpaceCoordinateValue(svg_info,0,value);
    (void) FormatLocaleFile(svg_info->file,"font-size %g\n",
      svg_info->pointsize);
  }
  color=AcquireString("none");
  units=AcquireString("userSpaceOnUse");
  for (i=0; i < (ssize_t) (number_tokens-1); i+=2)
  {
    keyword=(char *) tokens[i];
    value=(char *) tokens[i+1];
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"    %s: %s",keyword,
      value);
    switch (*keyword)
    {
      case 'B':
      case 'b':
      {
        if (LocaleCompare((const char *) name,"background") == 0)
          {
            if (LocaleCompare((const char *) name,"svg") == 0)
              (void) CopyMagickString(background,value,MagickPathExtent);
            break;
          }
        break;
      }
      case 'C':
      case 'c':
      {
         if (LocaleCompare(keyword,"clip-path") == 0)
           {
             (void) FormatLocaleFile(svg_info->file,"clip-path \"%s\"\n",value);
             break;
           }
        if (LocaleCompare(keyword,"clip-rule") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"clip-rule \"%s\"\n",value);
            break;
          }
         if (LocaleCompare(keyword,"clipPathUnits") == 0)
           {
             (void) CloneString(&units,value);
             (void) FormatLocaleFile(svg_info->file,"clip-units \"%s\"\n",
               value);
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
                 (void) FormatLocaleFile(svg_info->file,"fill \"%s\"\n",color);
                 break;
               }
            if (LocaleCompare(value,"#000000ff") == 0)
              (void) FormatLocaleFile(svg_info->file,"fill '#000000'\n");
            else
              (void) FormatLocaleFile(svg_info->file,"fill \"%s\"\n",value);
            break;
          }
        if (LocaleCompare(keyword,"fillcolor") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"fill \"%s\"\n",value);
            break;
          }
        if (LocaleCompare(keyword,"fill-rule") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"fill-rule \"%s\"\n",value);
            break;
          }
        if (LocaleCompare(keyword,"fill-opacity") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"fill-opacity \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"font") == 0)
          {
            char
              family[MagickPathExtent],
              size[MagickPathExtent],
              style[MagickPathExtent];

            if (sscanf(value,"%2048s %2048s %2048s",style,size,family) != 3)
              break;
            if (GetUserSpaceCoordinateValue(svg_info,0,style) == 0)
              (void) FormatLocaleFile(svg_info->file,"font-style \"%s\"\n",
                style);
            else
              if (sscanf(value,"%2048s %2048s",size,family) != 2)
                break;
            (void) FormatLocaleFile(svg_info->file,"font-size \"%s\"\n",size);
            (void) FormatLocaleFile(svg_info->file,"font-family \"%s\"\n",
              family);
            break;
          }
        if (LocaleCompare(keyword,"font-family") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"font-family \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"font-stretch") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"font-stretch \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"font-style") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"font-style \"%s\"\n",value);
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
            (void) FormatLocaleFile(svg_info->file,"font-weight \"%s\"\n",
              value);
            break;
          }
        break;
      }
      case 'K':
      case 'k':
      {
        if (LocaleCompare(keyword,"kerning") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"kerning \"%s\"\n",value);
            break;
          }
        break;
      }
      case 'L':
      case 'l':
      {
        if (LocaleCompare(keyword,"letter-spacing") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"letter-spacing \"%s\"\n",
              value);
            break;
          }
        break;
      }
      case 'M':
      case 'm':
      {
        if (LocaleCompare(keyword,"mask") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"mask \"%s\"\n",value);
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
            (void) FormatLocaleFile(svg_info->file,"opacity \"%s\"\n",value);
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
                (void) FormatLocaleFile(svg_info->file,"stroke \"%s\"\n",color);
                break;
              }
            if (LocaleCompare(value,"#000000ff") == 0)
              (void) FormatLocaleFile(svg_info->file,"fill '#000000'\n");
            else
              (void) FormatLocaleFile(svg_info->file,
                "stroke \"%s\"\n",value);
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
            (void) FormatLocaleFile(svg_info->file,"stroke-dashoffset %g\n",
              GetUserSpaceCoordinateValue(svg_info,1,value));
            break;
          }
        if (LocaleCompare(keyword,"stroke-linecap") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"stroke-linecap \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"stroke-linejoin") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"stroke-linejoin \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"stroke-miterlimit") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"stroke-miterlimit \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"stroke-opacity") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"stroke-opacity \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"stroke-width") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"stroke-width %g\n",
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
            (void) FormatLocaleFile(svg_info->file,"text-align \"%s\"\n",value);
            break;
          }
        if (LocaleCompare(keyword,"text-anchor") == 0)
          {
            (void) FormatLocaleFile(svg_info->file,"text-anchor \"%s\"\n",
              value);
            break;
          }
        if (LocaleCompare(keyword,"text-decoration") == 0)
          {
            if (LocaleCompare(value,"underline") == 0)
              (void) FormatLocaleFile(svg_info->file,"decorate underline\n");
            if (LocaleCompare(value,"line-through") == 0)
              (void) FormatLocaleFile(svg_info->file,"decorate line-through\n");
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
        break;
      }
      default:
        break;
    }
  }
  if (units != (char *) NULL)
    units=DestroyString(units);
  if (color != (char *) NULL)
    color=DestroyString(color);
  for (i=0; tokens[i] != (char *) NULL; i++)
    tokens[i]=DestroyString(tokens[i]);
  tokens=(char **) RelinquishMagickMemory(tokens);
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
#define PushGraphicContext(id) \
{ \
  if (*id == '\0') \
    (void) FormatLocaleFile(svg_info->file,"push graphic-context\n"); \
  else \
    (void) FormatLocaleFile(svg_info->file,"push graphic-context \"%s\"\n", \
      id); \
}

  char
    *color,
    background[MagickPathExtent],
    id[MagickPathExtent],
    *next_token,
    token[MagickPathExtent],
    **tokens,
    *units;

  const char
    *keyword,
    *p,
    *value;

  register ssize_t
    i,
    j;

  size_t
    number_tokens;

  SVGInfo
    *svg_info;

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
  *id='\0';
  *token='\0';
  *background='\0';
  value=(const char *) NULL;
  if ((LocaleCompare((char *) name,"image") == 0) ||
      (LocaleCompare((char *) name,"pattern") == 0) ||
      (LocaleCompare((char *) name,"rect") == 0) ||
      (LocaleCompare((char *) name,"text") == 0) ||
      (LocaleCompare((char *) name,"use") == 0))
    {
      svg_info->bounds.x=0.0;
      svg_info->bounds.y=0.0;
    }
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
              (void) CopyMagickString(id,value,MagickPathExtent);
              break;
            }
          break;
        }
        case 'R':
        case 'r':
        {
          if (LocaleCompare(keyword,"r") == 0)
            {
              svg_info->element.angle=GetUserSpaceCoordinateValue(svg_info,0,
                value);
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
              svg_info->bounds.y=GetUserSpaceCoordinateValue(svg_info,-1,value);
              break;
            }
          if (LocaleCompare(keyword,"y1") == 0)
            {
              svg_info->segment.y1=GetUserSpaceCoordinateValue(svg_info,-1,
                value);
              break;
            }
          if (LocaleCompare(keyword,"y2") == 0)
            {
              svg_info->segment.y2=GetUserSpaceCoordinateValue(svg_info,-1,
                value);
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
          PushGraphicContext(id);
          break;
        }
      if (LocaleCompare((const char *) name,"clipPath") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push clip-path \"%s\"\n",id);
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
          PushGraphicContext(id);
          break;
        }
      break;
    }
    case 'F':
    case 'f':
    {
      if (LocaleCompare((const char *) name,"foreignObject") == 0)
        {
          PushGraphicContext(id);
          break;
        }
      break;
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare((const char *) name,"g") == 0)
        {
          PushGraphicContext(id);
          break;
        }
      break;
    }
    case 'I':
    case 'i':
    {
      if (LocaleCompare((const char *) name,"image") == 0)
        {
          PushGraphicContext(id);
          break;
        }
      break;
    }
    case 'L':
    case 'l':
    {
      if (LocaleCompare((const char *) name,"line") == 0)
        {
          PushGraphicContext(id);
          break;
        }
      if (LocaleCompare((const char *) name,"linearGradient") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,
            "push gradient \"%s\" linear %g,%g %g,%g\n",id,
            svg_info->segment.x1,svg_info->segment.y1,svg_info->segment.x2,
            svg_info->segment.y2);
          break;
        }
      break;
    }
    case 'M':
    case 'm':
    {
      if (LocaleCompare((const char *) name,"mask") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push mask \"%s\"\n",id);
          break;
        }
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare((const char *) name,"path") == 0)
        {
          PushGraphicContext(id);
          break;
        }
      if (LocaleCompare((const char *) name,"pattern") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,
            "push pattern \"%s\" %g,%g %g,%g\n",id,
            svg_info->bounds.x,svg_info->bounds.y,svg_info->bounds.width,
            svg_info->bounds.height);
          break;
        }
      if (LocaleCompare((const char *) name,"polygon") == 0)
        {
          PushGraphicContext(id);
          break;
        }
      if (LocaleCompare((const char *) name,"polyline") == 0)
        {
          PushGraphicContext(id);
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
            "push gradient \"%s\" radial %g,%g %g,%g %g\n",
            id,svg_info->element.cx,svg_info->element.cy,
            svg_info->element.major,svg_info->element.minor,
            svg_info->element.angle);
          break;
        }
      if (LocaleCompare((const char *) name,"rect") == 0)
        {
          PushGraphicContext(id);
          break;
        }
      break;
    }
    case 'S':
    case 's':
    {
      if (LocaleCompare((char *) name,"style") == 0)
        break;
      if (LocaleCompare((const char *) name,"svg") == 0)
        {
          svg_info->svgDepth++;
          PushGraphicContext(id);
          (void) FormatLocaleFile(svg_info->file,"compliance \"SVG\"\n");
          (void) FormatLocaleFile(svg_info->file,"fill \"black\"\n");
          (void) FormatLocaleFile(svg_info->file,"fill-opacity 1\n");
          (void) FormatLocaleFile(svg_info->file,"stroke \"none\"\n");
          (void) FormatLocaleFile(svg_info->file,"stroke-width 1\n");
          (void) FormatLocaleFile(svg_info->file,"stroke-opacity 1\n");
          (void) FormatLocaleFile(svg_info->file,"fill-rule nonzero\n");
          break;
        }
      if (LocaleCompare((const char *) name,"symbol") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"push symbol\n");
          break;
        }
      break;
    }
    case 'T':
    case 't':
    {
      if (LocaleCompare((const char *) name,"text") == 0)
        {
          PushGraphicContext(id);
          (void) FormatLocaleFile(svg_info->file,"class \"text\"\n");
          svg_info->text_offset.x=svg_info->bounds.x;
          svg_info->text_offset.y=svg_info->bounds.y;
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
              char
                *text;

              text=EscapeString(svg_info->text,'\"');
              (void) FormatLocaleFile(svg_info->file,"text %g,%g \"%s\"\n",
                svg_info->text_offset.x,svg_info->text_offset.y,text);
              text=DestroyString(text);
              *svg_info->text='\0';
            }
          PushGraphicContext(id);
          break;
        }
      break;
    }
    case 'U':
    case 'u':
    {
      if (LocaleCompare((char *) name,"use") == 0)
        {
          PushGraphicContext(id);
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
          if (LocaleCompare(keyword,"class") == 0)
            {
              const char
                *p;

              p=value;
              (void) GetNextToken(p,&p,MagickPathExtent,token);
              if (*token == ',')
                (void) GetNextToken(p,&p,MagickPathExtent,token);
              if (*token != '\0')
                (void) FormatLocaleFile(svg_info->file,"class \"%s\"\n",value);
              else
                (void) FormatLocaleFile(svg_info->file,"class \"none\"\n");
              break;
            }
          if (LocaleCompare(keyword,"clip-path") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"clip-path \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"clip-rule") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"clip-rule \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"clipPathUnits") == 0)
            {
              (void) CloneString(&units,value);
              (void) FormatLocaleFile(svg_info->file,"clip-units \"%s\"\n",
                value);
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
              double
                dx;

              dx=GetUserSpaceCoordinateValue(svg_info,1,value);
              svg_info->bounds.x+=dx;
              svg_info->text_offset.x+=dx;
              if (LocaleCompare((char *) name,"text") == 0)
                (void) FormatLocaleFile(svg_info->file,"translate %g,0.0\n",dx);
              break;
            }
          if (LocaleCompare(keyword,"dy") == 0)
            {
              double
                dy;

              dy=GetUserSpaceCoordinateValue(svg_info,-1,value);
              svg_info->bounds.y+=dy;
              svg_info->text_offset.y+=dy;
              if (LocaleCompare((char *) name,"text") == 0)
                (void) FormatLocaleFile(svg_info->file,"translate 0.0,%g\n",dy);
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
                  (void) FormatLocaleFile(svg_info->file,"fill \"%s\"\n",color);
                  break;
                }
              (void) FormatLocaleFile(svg_info->file,"fill \"%s\"\n",value);
              break;
            }
          if (LocaleCompare(keyword,"fillcolor") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"fill \"%s\"\n",value);
              break;
            }
          if (LocaleCompare(keyword,"fill-rule") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"fill-rule \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"fill-opacity") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"fill-opacity \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"font-family") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-family \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"font-stretch") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-stretch \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"font-style") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-style \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"font-size") == 0)
            {
              if (LocaleCompare(value,"xx-small") == 0)
                svg_info->pointsize=6.144;
              else if (LocaleCompare(value,"x-small") == 0)
                svg_info->pointsize=7.68;
              else if (LocaleCompare(value,"small") == 0)
                svg_info->pointsize=9.6;
              else if (LocaleCompare(value,"medium") == 0)
                svg_info->pointsize=12.0;
              else if (LocaleCompare(value,"large") == 0)
                svg_info->pointsize=14.4;
              else if (LocaleCompare(value,"x-large") == 0)
                svg_info->pointsize=17.28;
              else if (LocaleCompare(value,"xx-large") == 0)
                svg_info->pointsize=20.736;
              else
                svg_info->pointsize=GetUserSpaceCoordinateValue(svg_info,0,
                  value);
              (void) FormatLocaleFile(svg_info->file,"font-size %g\n",
                svg_info->pointsize);
              break;
            }
          if (LocaleCompare(keyword,"font-weight") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"font-weight \"%s\"\n",
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
              tokens=SVGKeyValuePairs(context,'(',')',value,&number_tokens);
              if (tokens == (char **) NULL)
                break;
              for (j=0; j < (ssize_t) (number_tokens-1); j+=2)
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
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.sx=StringToDouble(value,(char **) NULL);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.rx=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.ry=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.sy=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.tx=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.ty=StringToDouble(token,&next_token);
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
              (void) FormatLocaleFile(svg_info->file,"gradient-units \"%s\"\n",
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
        case 'K':
        case 'k':
        {
          if (LocaleCompare(keyword,"kerning") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"kerning \"%s\"\n",
                value);
              break;
            }
          break;
        }
        case 'L':
        case 'l':
        {
          if (LocaleCompare(keyword,"letter-spacing") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"letter-spacing \"%s\"\n",
                value);
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
          if (LocaleCompare(keyword,"mask") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"mask \"%s\"\n",value);
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
              (void) FormatLocaleFile(svg_info->file,"opacity \"%s\"\n",value);
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
                  (void) FormatLocaleFile(svg_info->file,"stroke \"%s\"\n",
                    color);
                  break;
                }
              (void) FormatLocaleFile(svg_info->file,"stroke \"%s\"\n",value);
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
              (void) FormatLocaleFile(svg_info->file,"stroke-dashoffset %g\n",
                GetUserSpaceCoordinateValue(svg_info,1,value));
              break;
            }
          if (LocaleCompare(keyword,"stroke-linecap") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-linecap \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-linejoin") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-linejoin \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-miterlimit") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,
                "stroke-miterlimit \"%s\"\n",value);
              break;
            }
          if (LocaleCompare(keyword,"stroke-opacity") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"stroke-opacity \"%s\"\n",
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
              SVGProcessStyleElement(context,name,value);
              break;
            }
          break;
        }
        case 'T':
        case 't':
        {
          if (LocaleCompare(keyword,"text-align") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"text-align \"%s\"\n",
                value);
              break;
            }
          if (LocaleCompare(keyword,"text-anchor") == 0)
            {
              (void) FormatLocaleFile(svg_info->file,"text-anchor \"%s\"\n",
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
              tokens=SVGKeyValuePairs(context,'(',')',value,&number_tokens);
              if (tokens == (char **) NULL)
                break;
              for (j=0; j < (ssize_t) (number_tokens-1); j+=2)
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
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.sx=StringToDouble(value,(char **) NULL);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.rx=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.ry=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.sy=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.tx=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        affine.ty=StringToDouble(token,&next_token);
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
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        angle=StringToDouble(value,(char **) NULL);
                        affine.sx=cos(DegreesToRadians(fmod(angle,360.0)));
                        affine.rx=sin(DegreesToRadians(fmod(angle,360.0)));
                        affine.ry=(-sin(DegreesToRadians(fmod(angle,360.0))));
                        affine.sy=cos(DegreesToRadians(fmod(angle,360.0)));
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        x=StringToDouble(token,&next_token);
                        (void) GetNextToken(p,&p,MagickPathExtent,token);
                        if (*token == ',')
                          (void) GetNextToken(p,&p,MagickPathExtent,token);
                        y=StringToDouble(token,&next_token);
                        affine.tx=svg_info->bounds.x+x*
                          cos(DegreesToRadians(fmod(angle,360.0)))+y*
                          sin(DegreesToRadians(fmod(angle,360.0)));
                        affine.ty=svg_info->bounds.y-x*
                          sin(DegreesToRadians(fmod(angle,360.0)))+y*
                          cos(DegreesToRadians(fmod(angle,360.0)));
                        affine.tx-=x;
                        affine.ty-=y;
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
                        affine.ty=0;
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
              (void) GetNextToken(p,&p,MagickPathExtent,token);
              svg_info->view_box.x=StringToDouble(token,&next_token);
              (void) GetNextToken(p,&p,MagickPathExtent,token);
              if (*token == ',')
                (void) GetNextToken(p,&p,MagickPathExtent,token);
              svg_info->view_box.y=StringToDouble(token,&next_token);
              (void) GetNextToken(p,&p,MagickPathExtent,token);
              if (*token == ',')
                (void) GetNextToken(p,&p,MagickPathExtent,token);
              svg_info->view_box.width=StringToDouble(token,
                (char **) NULL);
              if (svg_info->bounds.width == 0)
                svg_info->bounds.width=svg_info->view_box.width;
              (void) GetNextToken(p,&p,MagickPathExtent,token);
              if (*token == ',')
                (void) GetNextToken(p,&p,MagickPathExtent,token);
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
          svg_info->width=0;
          if (svg_info->bounds.width > 0.0)
            svg_info->width=(size_t) floor(svg_info->bounds.width+0.5);
          svg_info->height=0;
          if (svg_info->bounds.height > 0.0)
            svg_info->height=(size_t) floor(svg_info->bounds.height+0.5);
          (void) FormatLocaleFile(svg_info->file,"viewbox 0 0 %.20g %.20g\n",
            (double) svg_info->width,(double) svg_info->height);
          sx=PerceptibleReciprocal(svg_info->view_box.width)*svg_info->width;
          sy=PerceptibleReciprocal(svg_info->view_box.height)*svg_info->height;
          tx=svg_info->view_box.x != 0.0 ? (double) -sx*svg_info->view_box.x :
            0.0;
          ty=svg_info->view_box.y != 0.0 ? (double) -sy*svg_info->view_box.y :
            0.0;
          (void) FormatLocaleFile(svg_info->file,"affine %g 0 0 %g %g %g\n",
            sx,sy,tx,ty);
          if ((svg_info->svgDepth == 1) && (*background != '\0'))
            {
              PushGraphicContext(id);
              (void) FormatLocaleFile(svg_info->file,"fill %s\n",background);
              (void) FormatLocaleFile(svg_info->file,
                "rectangle 0,0 %g,%g\n",svg_info->view_box.width,
                svg_info->view_box.height);
              (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
            }
        }
    }
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  )");
  if (units != (char *) NULL)
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
          (void) FormatLocaleFile(svg_info->file,"class \"circle\"\n");
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

          (void) FormatLocaleFile(svg_info->file,"class \"ellipse\"\n");
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
    case 'F':
    case 'f':
    {
      if (LocaleCompare((const char *) name,"foreignObject") == 0)
        {
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
            "image Over %g,%g %g,%g \"%s\"\n",svg_info->bounds.x,
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
          (void) FormatLocaleFile(svg_info->file,"class \"line\"\n");
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
    case 'M':
    case 'm':
    {
      if (LocaleCompare((const char *) name,"mask") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop mask\n");
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
          (void) FormatLocaleFile(svg_info->file,"class \"path\"\n");
          (void) FormatLocaleFile(svg_info->file,"path \"%s\"\n",
            svg_info->vertices);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"polygon") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"class \"polygon\"\n");
          (void) FormatLocaleFile(svg_info->file,"polygon %s\n",
            svg_info->vertices);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      if (LocaleCompare((const char *) name,"polyline") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"class \"polyline\"\n");
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
              (void) FormatLocaleFile(svg_info->file,"class \"rect\"\n");
              if ((fabs(svg_info->bounds.width-1.0) < MagickEpsilon) &&
                  (fabs(svg_info->bounds.height-1.0) < MagickEpsilon))
                (void) FormatLocaleFile(svg_info->file,"point %g,%g\n",
                  svg_info->bounds.x,svg_info->bounds.y);
              else
                (void) FormatLocaleFile(svg_info->file,
                  "rectangle %g,%g %g,%g\n",svg_info->bounds.x,
                  svg_info->bounds.y,svg_info->bounds.x+svg_info->bounds.width,
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
          (void) FormatLocaleFile(svg_info->file,"stop-color \"%s\" %s\n",
            svg_info->stop_color,svg_info->offset == (char *) NULL ? "100%" : 
            svg_info->offset);
          break;
        }
      if (LocaleCompare((char *) name,"style") == 0)
        {
          char
            *keyword,
            **tokens,
            *value;

          register ssize_t
            j;

          size_t
            number_tokens;

          /*
            Find style definitions in svg_info->text.
          */
          tokens=SVGKeyValuePairs(context,'{','}',svg_info->text,
            &number_tokens);
          if (tokens == (char **) NULL)
            break;
          for (j=0; j < (ssize_t) (number_tokens-1); j+=2)
          {
            keyword=(char *) tokens[j];
            value=(char *) tokens[j+1];
            (void) FormatLocaleFile(svg_info->file,"push class \"%s\"\n",
              *keyword == '.' ? keyword+1 : keyword);
            SVGProcessStyleElement(context,name,value);
            (void) FormatLocaleFile(svg_info->file,"pop class\n");
          }
          for (j=0; tokens[j] != (char *) NULL; j++)
            tokens[j]=DestroyString(tokens[j]);
          tokens=(char **) RelinquishMagickMemory(tokens);
          break;
        }
      if (LocaleCompare((const char *) name,"svg") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          svg_info->svgDepth--;
          break;
        }
      if (LocaleCompare((const char *) name,"symbol") == 0)
        {
          (void) FormatLocaleFile(svg_info->file,"pop symbol\n");
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

              SVGStripString(MagickTrue,svg_info->text);
              text=EscapeString(svg_info->text,'\"');
              (void) FormatLocaleFile(svg_info->file,"text %g,%g \"%s\"\n",
                svg_info->text_offset.x,svg_info->text_offset.y,text);
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
              char
                *text;

              (void) FormatLocaleFile(svg_info->file,"class \"tspan\"\n");
              text=EscapeString(svg_info->text,'\"');
              (void) FormatLocaleFile(svg_info->file,"text %g,%g \"%s\"\n",
                svg_info->bounds.x,svg_info->bounds.y,text);
              text=DestroyString(text);
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
    case 'U':
    case 'u':
    {
      if (LocaleCompare((char *) name,"use") == 0)
        {
          if ((svg_info->bounds.x != 0.0) || (svg_info->bounds.y != 0.0))
            (void) FormatLocaleFile(svg_info->file,"translate %g,%g\n",
              svg_info->bounds.x,svg_info->bounds.y);
          (void) FormatLocaleFile(svg_info->file,"use \"url(%s)\"\n",
            svg_info->url);
          (void) FormatLocaleFile(svg_info->file,"pop graphic-context\n");
          break;
        }
      break;
    }
    default:
      break;
  }
  *svg_info->text='\0';
  (void) memset(&svg_info->element,0,sizeof(svg_info->element));
  (void) memset(&svg_info->segment,0,sizeof(svg_info->segment));
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
  SVGStripString(MagickFalse,text);
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

static void SVGWarning(void *,const char *,...)
  magick_attribute((__format__ (__printf__,2,3)));

static void SVGWarning(void *context,const char *format,...)
{
  char
    *message,
    reason[MagickPathExtent];

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
  (void) vsnprintf(reason,MagickPathExtent,format,operands);
#endif
  message=GetExceptionMessage(errno);
  (void) ThrowMagickException(svg_info->exception,GetMagickModule(),
    DelegateWarning,reason,"`%s`",message);
  message=DestroyString(message);
  va_end(operands);
}

static void SVGError(void *,const char *,...)
  magick_attribute((__format__ (__printf__,2,3)));

static void SVGError(void *context,const char *format,...)
{
  char
    *message,
    reason[MagickPathExtent];

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
  (void) vsnprintf(reason,MagickPathExtent,format,operands);
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
  child=xmlNewCDataBlock(parser->myDoc,value,length);
  if (xmlAddChild(parser->node,child) == (xmlNodePtr) NULL)
    xmlFreeNode(child);
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

static Image *ReadSVGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  const char
    *option;

  FILE
    *file;

  Image
    *image,
    *next;

  int
    status,
    unique_file;

  ssize_t
    n;

  SVGInfo
    *svg_info;

  unsigned char
    message[MagickPathExtent];

  xmlSAXHandler
    sax_modules;

  xmlSAXHandlerPtr
    sax_handler;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if ((fabs(image->resolution.x) < MagickEpsilon) ||
      (fabs(image->resolution.y) < MagickEpsilon))
    {
      GeometryInfo
        geometry_info;

      int
        flags;

      flags=ParseGeometry(SVGDensityGeometry,&geometry_info);
      image->resolution.x=geometry_info.rho;
      image->resolution.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->resolution.y=image->resolution.x;
    }
  if (LocaleCompare(image_info->magick,"MSVG") != 0)
    {
      Image
        *svg_image;

      svg_image=RenderSVGImage(image_info,image,exception);
      if (svg_image != (Image *) NULL)
        {
          image=DestroyImageList(image);
          return(svg_image);
        }
      {
#if defined(MAGICKCORE_RSVG_DELEGATE)
#if defined(MAGICKCORE_CAIRO_DELEGATE)
        cairo_surface_t
          *cairo_surface;

        cairo_t
          *cairo_image;

        MagickBooleanType
          apply_density;

        MemoryInfo
          *pixel_info;

        register unsigned char
          *p;

        RsvgDimensionData
          dimension_info;

        unsigned char
          *pixels;

#else
        GdkPixbuf
          *pixel_buffer;

        register const guchar
          *p;
#endif

        GError
          *error;

        PixelInfo
          fill_color;

        register ssize_t
          x;

        register Quantum
          *q;

        RsvgHandle
          *svg_handle;

        ssize_t
          y;

        unsigned char
          *buffer;

        buffer=(unsigned char *) AcquireQuantumMemory(MagickMaxBufferExtent,
          sizeof(*buffer));
        if (buffer == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
#if LIBRSVG_CHECK_VERSION(2,40,3)
        option=GetImageOption(image_info,"svg:xml-parse-huge");
        if ((option != (char *) NULL) && (IsStringTrue(option) != MagickFalse))
          svg_handle=rsvg_handle_new_with_flags(RSVG_HANDLE_FLAG_UNLIMITED);
        else
#endif
          svg_handle=rsvg_handle_new();
        if (svg_handle == (RsvgHandle *) NULL)
          {
            buffer=(unsigned char *) RelinquishMagickMemory(buffer);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        rsvg_handle_set_base_uri(svg_handle,image_info->filename);
        if ((fabs(image->resolution.x) > MagickEpsilon) &&
            (fabs(image->resolution.y) > MagickEpsilon))
          rsvg_handle_set_dpi_x_y(svg_handle,image->resolution.x,
            image->resolution.y);
        while ((n=ReadBlob(image,MagickMaxBufferExtent-1,buffer)) != 0)
        {
          buffer[n]='\0';
          error=(GError *) NULL;
          (void) rsvg_handle_write(svg_handle,buffer,n,&error);
          if (error != (GError *) NULL)
            g_error_free(error);
        }
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
        error=(GError *) NULL;
        rsvg_handle_close(svg_handle,&error);
        if (error != (GError *) NULL)
          g_error_free(error);
#if defined(MAGICKCORE_CAIRO_DELEGATE)
        apply_density=MagickTrue;
        rsvg_handle_get_dimensions(svg_handle,&dimension_info);
        if ((image->resolution.x > 0.0) && (image->resolution.y > 0.0))
          {
            RsvgDimensionData
              dpi_dimension_info;

            /*
              We should not apply the density when the internal 'factor' is 'i'.
              This can be checked by using the trick below.
            */
            rsvg_handle_set_dpi_x_y(svg_handle,image->resolution.x*256,
              image->resolution.y*256);
            rsvg_handle_get_dimensions(svg_handle,&dpi_dimension_info);
            if ((dpi_dimension_info.width != dimension_info.width) ||
                (dpi_dimension_info.height != dimension_info.height))
              apply_density=MagickFalse;
            rsvg_handle_set_dpi_x_y(svg_handle,image->resolution.x,
              image->resolution.y);
          }
        if (image_info->size != (char *) NULL)
          {
            (void) GetGeometry(image_info->size,(ssize_t *) NULL,
              (ssize_t *) NULL,&image->columns,&image->rows);
            if ((image->columns != 0) || (image->rows != 0))
              {
                image->resolution.x=DefaultSVGDensity*image->columns/
                  dimension_info.width;
                image->resolution.y=DefaultSVGDensity*image->rows/
                  dimension_info.height;
                if (fabs(image->resolution.x) < MagickEpsilon)
                  image->resolution.x=image->resolution.y;
                else
                  if (fabs(image->resolution.y) < MagickEpsilon)
                    image->resolution.y=image->resolution.x;
                  else
                    image->resolution.x=image->resolution.y=MagickMin(
                      image->resolution.x,image->resolution.y);
                apply_density=MagickTrue;
              }
          }
        if (apply_density != MagickFalse)
          {
            image->columns=image->resolution.x*dimension_info.width/
              DefaultSVGDensity;
            image->rows=image->resolution.y*dimension_info.height/
              DefaultSVGDensity;
          }
        else
          {
            image->columns=dimension_info.width;
            image->rows=dimension_info.height;
          }
        pixel_info=(MemoryInfo *) NULL;
#else
        pixel_buffer=rsvg_handle_get_pixbuf(svg_handle);
        rsvg_handle_free(svg_handle);
        image->columns=gdk_pixbuf_get_width(pixel_buffer);
        image->rows=gdk_pixbuf_get_height(pixel_buffer);
#endif
        image->alpha_trait=BlendPixelTrait;
        if (image_info->ping == MagickFalse)
          {
#if defined(MAGICKCORE_CAIRO_DELEGATE)
            size_t
              stride;
#endif

            status=SetImageExtent(image,image->columns,image->rows,exception);
            if (status == MagickFalse)
              {
#if !defined(MAGICKCORE_CAIRO_DELEGATE)
                g_object_unref(G_OBJECT(pixel_buffer));
#endif
                g_object_unref(svg_handle);
                ThrowReaderException(MissingDelegateError,
                  "NoDecodeDelegateForThisImageFormat");
              }
#if defined(MAGICKCORE_CAIRO_DELEGATE)
            stride=4*image->columns;
#if defined(MAGICKCORE_PANGOCAIRO_DELEGATE)
            stride=(size_t) cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
              (int) image->columns);
#endif
            pixel_info=AcquireVirtualMemory(stride,image->rows*sizeof(*pixels));
            if (pixel_info == (MemoryInfo *) NULL)
              {
                g_object_unref(svg_handle);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
#endif
            (void) SetImageBackgroundColor(image,exception);
#if defined(MAGICKCORE_CAIRO_DELEGATE)
            cairo_surface=cairo_image_surface_create_for_data(pixels,
              CAIRO_FORMAT_ARGB32,(int) image->columns,(int) image->rows,(int)
              stride);
            if ((cairo_surface == (cairo_surface_t *) NULL) ||
                (cairo_surface_status(cairo_surface) != CAIRO_STATUS_SUCCESS))
              {
                if (cairo_surface != (cairo_surface_t *) NULL)
                  cairo_surface_destroy(cairo_surface);
                pixel_info=RelinquishVirtualMemory(pixel_info);
                g_object_unref(svg_handle);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            cairo_image=cairo_create(cairo_surface);
            cairo_set_operator(cairo_image,CAIRO_OPERATOR_CLEAR);
            cairo_paint(cairo_image);
            cairo_set_operator(cairo_image,CAIRO_OPERATOR_OVER);
            if (apply_density != MagickFalse)
              cairo_scale(cairo_image,image->resolution.x/DefaultSVGDensity,
                image->resolution.y/DefaultSVGDensity);
            rsvg_handle_render_cairo(svg_handle,cairo_image);
            cairo_destroy(cairo_image);
            cairo_surface_destroy(cairo_surface);
            g_object_unref(svg_handle);
            p=pixels;
#else
            p=gdk_pixbuf_get_pixels(pixel_buffer);
#endif
            GetPixelInfo(image,&fill_color);
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
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
                fill_color.alpha=ScaleCharToQuantum(*p++);
#if defined(MAGICKCORE_CAIRO_DELEGATE)
                {
                  double
                    gamma;

                  gamma=QuantumScale*fill_color.alpha;
                  gamma=PerceptibleReciprocal(gamma);
                  fill_color.blue*=gamma;
                  fill_color.green*=gamma;
                  fill_color.red*=gamma;
                }
#endif
                CompositePixelOver(image,&fill_color,fill_color.alpha,q,(double)
                  GetPixelAlpha(image,q),q);
                q+=GetPixelChannels(image);
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
        if (pixel_info != (MemoryInfo *) NULL)
          pixel_info=RelinquishVirtualMemory(pixel_info);
#else
        g_object_unref(G_OBJECT(pixel_buffer));
#endif
        (void) CloseBlob(image);
        for (next=GetFirstImageInList(image); next != (Image *) NULL; )
        {
          (void) CopyMagickString(next->filename,image->filename,MaxTextExtent);
          (void) CopyMagickString(next->magick,image->magick,MaxTextExtent);
          next=GetNextImageInList(next);
        }
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
      (void) CopyMagickString(image->filename,filename,MagickPathExtent);
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
    {
      (void) fclose(file);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  svg_info->file=file;
  svg_info->exception=exception;
  svg_info->image=image;
  svg_info->image_info=image_info;
  svg_info->bounds.width=image->columns;
  svg_info->bounds.height=image->rows;
  svg_info->svgDepth=0;
  if (image_info->size != (char *) NULL)
    (void) CloneString(&svg_info->size,image_info->size);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"begin SAX");
  xmlInitParser();
  (void) xmlSubstituteEntitiesDefault(1);
  (void) memset(&sax_modules,0,sizeof(sax_modules));
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
  n=ReadBlob(image,MagickPathExtent-1,message);
  message[n]='\0';
  if (n > 0)
    {
      svg_info->parser=xmlCreatePushParserCtxt(sax_handler,svg_info,(char *)
        message,n,image->filename);
      option=GetImageOption(image_info,"svg:xml-parse-huge");
      if ((option != (char *) NULL) && (IsStringTrue(option) != MagickFalse))
        (void) xmlCtxtUseOptions(svg_info->parser,XML_PARSE_HUGE);
      while ((n=ReadBlob(image,MagickPathExtent-1,message)) != 0)
      {
        message[n]='\0';
        status=xmlParseChunk(svg_info->parser,(char *) message,(int) n,0);
        if (status != 0)
          break;
      }
    }
  (void) xmlParseChunk(svg_info->parser,(char *) message,0,1);
  SVGEndDocument(svg_info);
  if (svg_info->parser->myDoc != (xmlDocPtr) NULL)
    xmlFreeDoc(svg_info->parser->myDoc);
  xmlFreeParserCtxt(svg_info->parser);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"end SAX");
  (void) fclose(file);
  (void) CloseBlob(image);
  image->columns=svg_info->width;
  image->rows=svg_info->height;
  if (exception->severity >= ErrorException)
    {
      svg_info=DestroySVGInfo(svg_info);
      (void) RelinquishUniqueFileResource(filename);
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
      (void) FormatLocaleString(read_info->filename,MagickPathExtent,"mvg:%s",
        filename);
      image=ReadImage(read_info,exception);
      read_info=DestroyImageInfo(read_info);
      if (image != (Image *) NULL)
        (void) CopyMagickString(image->filename,image_info->filename,
          MagickPathExtent);
    }
  /*
    Relinquish resources.
  */
  if (image != (Image *) NULL)
    {
      if (svg_info->title != (char *) NULL)
        (void) SetImageProperty(image,"svg:title",svg_info->title,exception);
      if (svg_info->comment != (char *) NULL)
        (void) SetImageProperty(image,"svg:comment",svg_info->comment,
          exception);
    }
  for (next=GetFirstImageInList(image); next != (Image *) NULL; )
  {
    (void) CopyMagickString(next->filename,image->filename,MaxTextExtent);
    (void) CopyMagickString(next->magick,image->magick,MaxTextExtent);
    next=GetNextImageInList(next);
  }
  svg_info=DestroySVGInfo(svg_info);
  (void) RelinquishUniqueFileResource(filename);
  return(GetFirstImageInList(image));
}
#else
static Image *ReadSVGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image,
    *svg_image;

  MagickBooleanType
    status;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if ((fabs(image->resolution.x) < MagickEpsilon) ||
      (fabs(image->resolution.y) < MagickEpsilon))
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      flags=ParseGeometry(SVGDensityGeometry,&geometry_info);
      image->resolution.x=geometry_info.rho;
      image->resolution.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->resolution.y=image->resolution.x;
    }
  svg_image=RenderSVGImage(image_info,image,exception);
  image=DestroyImage(image);
  return(svg_image);
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
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(LIBXML_DOTTED_VERSION)
  (void) CopyMagickString(version,"XML " LIBXML_DOTTED_VERSION,
    MagickPathExtent);
#endif
#if defined(MAGICKCORE_RSVG_DELEGATE)
#if !GLIB_CHECK_VERSION(2,35,0)
  g_type_init();
#endif
  (void) FormatLocaleString(version,MagickPathExtent,"RSVG %d.%d.%d",
    LIBRSVG_MAJOR_VERSION,LIBRSVG_MINOR_VERSION,LIBRSVG_MICRO_VERSION);
#endif
  entry=AcquireMagickInfo("SVG","SVG","Scalable Vector Graphics");
  entry->decoder=(DecodeImageHandler *) ReadSVGImage;
  entry->encoder=(EncodeImageHandler *) WriteSVGImage;
  entry->flags^=CoderBlobSupportFlag;
#if defined(MAGICKCORE_RSVG_DELEGATE)
  entry->flags^=CoderDecoderThreadSupportFlag;
#endif
  entry->mime_type=ConstantString("image/svg+xml");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->magick=(IsImageFormatHandler *) IsSVG;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("SVG","SVGZ","Compressed Scalable Vector Graphics");
#if defined(MAGICKCORE_XML_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadSVGImage;
#endif
  entry->encoder=(EncodeImageHandler *) WriteSVGImage;
  entry->flags^=CoderBlobSupportFlag;
#if defined(MAGICKCORE_RSVG_DELEGATE)
  entry->flags^=CoderDecoderThreadSupportFlag;
#endif
  entry->mime_type=ConstantString("image/svg+xml");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->magick=(IsImageFormatHandler *) IsSVG;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("SVG","MSVG",
    "ImageMagick's own SVG internal renderer");
#if defined(MAGICKCORE_XML_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadSVGImage;
#endif
  entry->encoder=(EncodeImageHandler *) WriteSVGImage;
  entry->flags^=CoderBlobSupportFlag;
#if defined(MAGICKCORE_RSVG_DELEGATE)
  entry->flags^=CoderDecoderThreadSupportFlag;
#endif
  entry->magick=(IsImageFormatHandler *) IsSVG;
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
%      MagickBooleanType WriteSVGImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void AffineToTransform(Image *image,AffineMatrix *affine)
{
  char
    transform[MagickPathExtent];

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
          (void) FormatLocaleString(transform,MagickPathExtent,
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
              (void) FormatLocaleString(transform,MagickPathExtent,
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
          (void) FormatLocaleString(transform,MagickPathExtent,
            "\" transform=\"translate(%g,%g)\">\n",affine->tx,affine->ty);
          (void) WriteBlobString(image,transform);
          return;
        }
    }
  (void) FormatLocaleString(transform,MagickPathExtent,
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

  value=(ssize_t) strtol(point,&p,10);
  (void) value;
  return(p != point ? MagickTrue : MagickFalse);
}

static MagickBooleanType TraceSVGImage(Image *image,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_AUTOTRACE_DELEGATE)
  {
    at_bitmap
      *trace;

    at_fitting_opts_type
      *fitting_options;

    at_output_opts_type
      *output_options;

    at_splines_type
      *splines;

    ImageType
      type;

    register const Quantum
      *p;

    register ssize_t
      i,
      x;

    size_t
      number_planes;

    ssize_t
      y;

    /*
      Trace image and write as SVG.
    */
    fitting_options=at_fitting_opts_new();
    output_options=at_output_opts_new();
    (void) SetImageGray(image,exception);
    type=GetImageType(image);
    number_planes=3;
    if ((type == BilevelType) || (type == GrayscaleType))
      number_planes=1;
    trace=at_bitmap_new(image->columns,image->rows,number_planes);
    i=0;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        trace->bitmap[i++]=GetPixelRed(image,p);
        if (number_planes == 3)
          {
            trace->bitmap[i++]=GetPixelGreen(image,p);
            trace->bitmap[i++]=GetPixelBlue(image,p);
          }
        p+=GetPixelChannels(image);
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
      *base64,
      filename[MagickPathExtent],
      message[MagickPathExtent];

    const DelegateInfo
      *delegate_info;

    Image
      *clone_image;

    ImageInfo
      *image_info;

    MagickBooleanType
      status;

    register char
      *p;

    size_t
      blob_length,
      encode_length;

    ssize_t
      i;

    unsigned char
      *blob;

    delegate_info=GetDelegateInfo((char *) NULL,"TRACE",exception);
    if (delegate_info != (DelegateInfo *) NULL)
      {
        /*
          Trace SVG with tracing delegate.
        */
        image_info=AcquireImageInfo();
        (void) CopyMagickString(image_info->magick,"TRACE",MagickPathExtent);
        (void) FormatLocaleString(filename,MagickPathExtent,"trace:%s",
          image_info->filename);
        (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
        status=WriteImage(image_info,image,exception);
        image_info=DestroyImageInfo(image_info);
        return(status);
      }
    (void) WriteBlobString(image,
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    (void) WriteBlobString(image,
      "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"");
    (void) WriteBlobString(image,
      " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    (void) FormatLocaleString(message,MagickPathExtent,
      "<svg version=\"1.1\" id=\"Layer_1\" "
      "xmlns=\"http://www.w3.org/2000/svg\" "
      "xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" "
      "width=\"%.20gpx\" height=\"%.20gpx\" viewBox=\"0 0 %.20g %.20g\" "
      "enable-background=\"new 0 0 %.20g %.20g\" xml:space=\"preserve\">",
      (double) image->columns,(double) image->rows,
      (double) image->columns,(double) image->rows,
      (double) image->columns,(double) image->rows);
    (void) WriteBlobString(image,message);
    clone_image=CloneImage(image,0,0,MagickTrue,exception);
    if (clone_image == (Image *) NULL)
      return(MagickFalse);
    image_info=AcquireImageInfo();
    (void) CopyMagickString(image_info->magick,"PNG",MagickPathExtent);
    blob_length=2048;
    blob=(unsigned char *) ImageToBlob(image_info,clone_image,&blob_length,
      exception);
    clone_image=DestroyImage(clone_image);
    image_info=DestroyImageInfo(image_info);
    if (blob == (unsigned char *) NULL)
      return(MagickFalse);
    encode_length=0;
    base64=Base64Encode(blob,blob_length,&encode_length);
    blob=(unsigned char *) RelinquishMagickMemory(blob);
    (void) FormatLocaleString(message,MagickPathExtent,
      "  <image id=\"image%.20g\" width=\"%.20g\" height=\"%.20g\" "
      "x=\"%.20g\" y=\"%.20g\"\n    href=\"data:image/png;base64,",
      (double) image->scene,(double) image->columns,(double) image->rows,
      (double) image->page.x,(double) image->page.y);
    (void) WriteBlobString(image,message);
    p=base64;
    for (i=(ssize_t) encode_length; i > 0; i-=76)
    {
      (void) FormatLocaleString(message,MagickPathExtent,"%.76s",p);
      (void) WriteBlobString(image,message);
      p+=76;
      if (i > 76)
        (void) WriteBlobString(image,"\n");
    }
    base64=DestroyString(base64);
    (void) WriteBlobString(image,"\" />\n");
    (void) WriteBlobString(image,"</svg>\n");
  }
#endif
  (void) CloseBlob(image);
  return(MagickTrue);
}

static MagickBooleanType WriteSVGImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
#define BezierQuantum  200

  AffineMatrix
    affine;

  char
    keyword[MagickPathExtent],
    message[MagickPathExtent],
    name[MagickPathExtent],
    *next_token,
    *token,
    type[MagickPathExtent];

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
    extent,
    length,
    number_points;

  SVGInfo
    svg_info;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  value=GetImageArtifact(image,"SVG");
  if (value != (char *) NULL)
    {
      (void) WriteBlobString(image,value);
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  value=GetImageArtifact(image,"mvg:vector-graphics");
  if (value == (char *) NULL)
    return(TraceSVGImage(image,exception));
  /*
    Write SVG header.
  */
  (void) WriteBlobString(image,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
  (void) WriteBlobString(image,
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"\n");
  (void) WriteBlobString(image,
    "  \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");
  (void) FormatLocaleString(message,MagickPathExtent,
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
  extent=strlen(token)+MagickPathExtent;
  active=MagickFalse;
  n=0;
  status=MagickTrue;
  for (q=(const char *) value; *q != '\0'; )
  {
    /*
      Interpret graphic primitive.
    */
    (void) GetNextToken(q,&q,MagickPathExtent,keyword);
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
            default: (void) WriteBlobByte(image,(unsigned char) *q); break;
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
            (void) GetNextToken(q,&q,extent,token);
            affine.sx=StringToDouble(token,&next_token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            affine.rx=StringToDouble(token,&next_token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            affine.ry=StringToDouble(token,&next_token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            affine.sy=StringToDouble(token,&next_token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            affine.tx=StringToDouble(token,&next_token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            affine.ty=StringToDouble(token,&next_token);
            break;
          }
        if (LocaleCompare("alpha",keyword) == 0)
          {
            primitive_type=AlphaPrimitive;
            break;
          }
        if (LocaleCompare("angle",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            affine.rx=StringToDouble(token,&next_token);
            affine.ry=StringToDouble(token,&next_token);
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
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "clip-path:url(#%s);",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("clip-rule",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"clip-rule:%s;",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("clip-units",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
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
        if (LocaleCompare("compliance",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
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
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
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
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"fill:%s;",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("fill-rule",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "fill-rule:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("fill-opacity",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "fill-opacity:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-family",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "font-family:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-stretch",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "font-stretch:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-style",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "font-style:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-size",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "font-size:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("font-weight",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
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
            (void) GetNextToken(q,&q,extent,token);
            break;
          }
        if (LocaleCompare("text-align",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "text-align %s ",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("text-anchor",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
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
            (void) GetNextToken(q,&q,extent,token);
            primitive_type=ImagePrimitive;
            break;
          }
        status=MagickFalse;
        break;
      }
      case 'k':
      case 'K':
      {
        if (LocaleCompare("kerning",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"kerning:%s;",
              token);
            (void) WriteBlobString(image,message);
          }
        break;
      }
      case 'l':
      case 'L':
      {
        if (LocaleCompare("letter-spacing",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "letter-spacing:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("line",keyword) == 0)
          {
            primitive_type=LinePrimitive;
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
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"opacity %s ",
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
            (void) GetNextToken(q,&q,extent,token);
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
                (void) FormatLocaleString(message,MagickPathExtent,
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
            if (LocaleCompare("symbol",token) == 0)
              {
                (void) WriteBlobString(image,"</symbol>\n");
                break;
              }
            if ((LocaleCompare("defs",token) == 0) ||
                (LocaleCompare("symbol",token) == 0))
              (void) WriteBlobString(image,"</g>\n");
            break;
          }
        if (LocaleCompare("push",keyword) == 0)
          {
            *name='\0';
            (void) GetNextToken(q,&q,extent,token);
            if (*q == '"')
              (void) GetNextToken(q,&q,extent,name);
            if (LocaleCompare("clip-path",token) == 0)
              {
                (void) GetNextToken(q,&q,extent,token);
                (void) FormatLocaleString(message,MagickPathExtent,
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
                (void) GetNextToken(q,&q,extent,token);
                (void) CopyMagickString(name,token,MagickPathExtent);
                (void) GetNextToken(q,&q,extent,token);
                (void) CopyMagickString(type,token,MagickPathExtent);
                (void) GetNextToken(q,&q,extent,token);
                svg_info.segment.x1=StringToDouble(token,&next_token);
                svg_info.element.cx=StringToDouble(token,&next_token);
                (void) GetNextToken(q,&q,extent,token);
                if (*token == ',')
                  (void) GetNextToken(q,&q,extent,token);
                svg_info.segment.y1=StringToDouble(token,&next_token);
                svg_info.element.cy=StringToDouble(token,&next_token);
                (void) GetNextToken(q,&q,extent,token);
                if (*token == ',')
                  (void) GetNextToken(q,&q,extent,token);
                svg_info.segment.x2=StringToDouble(token,&next_token);
                svg_info.element.major=StringToDouble(token,
                  (char **) NULL);
                (void) GetNextToken(q,&q,extent,token);
                if (*token == ',')
                  (void) GetNextToken(q,&q,extent,token);
                svg_info.segment.y2=StringToDouble(token,&next_token);
                svg_info.element.minor=StringToDouble(token,
                  (char **) NULL);
                (void) FormatLocaleString(message,MagickPathExtent,
                  "<%sGradient id=\"%s\" x1=\"%g\" y1=\"%g\" x2=\"%g\" "
                  "y2=\"%g\">\n",type,name,svg_info.segment.x1,
                  svg_info.segment.y1,svg_info.segment.x2,svg_info.segment.y2);
                if (LocaleCompare(type,"radial") == 0)
                  {
                    (void) GetNextToken(q,&q,extent,token);
                    if (*token == ',')
                      (void) GetNextToken(q,&q,extent,token);
                    svg_info.element.angle=StringToDouble(token,
                      (char **) NULL);
                    (void) FormatLocaleString(message,MagickPathExtent,
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
                (void) GetNextToken(q,&q,extent,token);
                (void) CopyMagickString(name,token,MagickPathExtent);
                (void) GetNextToken(q,&q,extent,token);
                svg_info.bounds.x=StringToDouble(token,&next_token);
                (void) GetNextToken(q,&q,extent,token);
                if (*token == ',')
                  (void) GetNextToken(q,&q,extent,token);
                svg_info.bounds.y=StringToDouble(token,&next_token);
                (void) GetNextToken(q,&q,extent,token);
                if (*token == ',')
                  (void) GetNextToken(q,&q,extent,token);
                svg_info.bounds.width=StringToDouble(token,
                  (char **) NULL);
                (void) GetNextToken(q,&q,extent,token);
                if (*token == ',')
                  (void) GetNextToken(q,&q,extent,token);
                svg_info.bounds.height=StringToDouble(token,(char **) NULL);
                (void) FormatLocaleString(message,MagickPathExtent,
                  "<pattern id=\"%s\" x=\"%g\" y=\"%g\" width=\"%g\" "
                  "height=\"%g\">\n",name,svg_info.bounds.x,svg_info.bounds.y,
                  svg_info.bounds.width,svg_info.bounds.height);
                (void) WriteBlobString(image,message);
                break;
              }
            if (LocaleCompare("symbol",token) == 0)
              {
                (void) WriteBlobString(image,"<symbol>\n");
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
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"rotate(%s) ",
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
            (void) GetNextToken(q,&q,extent,token);
            affine.sx=StringToDouble(token,&next_token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            affine.sy=StringToDouble(token,&next_token);
            break;
          }
        if (LocaleCompare("skewX",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"skewX(%s) ",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("skewY",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"skewY(%s) ",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stop-color",keyword) == 0)
          {
            char
              color[MagickPathExtent];

            (void) GetNextToken(q,&q,extent,token);
            (void) CopyMagickString(color,token,MagickPathExtent);
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "  <stop offset=\"%s\" stop-color=\"%s\" />\n",token,color);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,"stroke:%s;",
              token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-antialias",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
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
                (void) GetNextToken(p,&p,extent,token);
                for (k=0; IsPoint(token); k++)
                  (void) GetNextToken(p,&p,extent,token);
                (void) WriteBlobString(image,"stroke-dasharray:");
                for (j=0; j < k; j++)
                {
                  (void) GetNextToken(q,&q,extent,token);
                  (void) FormatLocaleString(message,MagickPathExtent,"%s ",
                    token);
                  (void) WriteBlobString(image,message);
                }
                (void) WriteBlobString(image,";");
                break;
              }
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "stroke-dasharray:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-dashoffset",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "stroke-dashoffset:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-linecap",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "stroke-linecap:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-linejoin",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "stroke-linejoin:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-miterlimit",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "stroke-miterlimit:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-opacity",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
              "stroke-opacity:%s;",token);
            (void) WriteBlobString(image,message);
            break;
          }
        if (LocaleCompare("stroke-width",keyword) == 0)
          {
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
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
            (void) GetNextToken(q,&q,extent,token);
            (void) FormatLocaleString(message,MagickPathExtent,
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
            (void) GetNextToken(q,&q,extent,token);
            affine.tx=StringToDouble(token,&next_token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            affine.ty=StringToDouble(token,&next_token);
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
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            (void) GetNextToken(q,&q,extent,token);
            if (*token == ',')
              (void) GetNextToken(q,&q,extent,token);
            (void) GetNextToken(q,&q,extent,token);
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
      (void) GetNextToken(q,&q,extent,token);
      point.x=StringToDouble(token,&next_token);
      (void) GetNextToken(q,&q,extent,token);
      if (*token == ',')
        (void) GetNextToken(q,&q,extent,token);
      point.y=StringToDouble(token,&next_token);
      (void) GetNextToken(q,(const char **) NULL,extent,token);
      if (*token == ',')
        (void) GetNextToken(q,&q,extent,token);
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
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
          break;
        }
    }
    primitive_info[j].primitive=primitive_type;
    primitive_info[j].coordinates=(size_t) x;
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
          (void) FormatLocaleString(message,MagickPathExtent,
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
          (void) FormatLocaleString(message,MagickPathExtent,
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
        (void) FormatLocaleString(message,MagickPathExtent,
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
          (void) FormatLocaleString(message,MagickPathExtent,
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
        (void) FormatLocaleString(message,MagickPathExtent,
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
        (void) CopyMagickString(message,"  <polyline points=\"",
           MagickPathExtent);
        (void) WriteBlobString(image,message);
        length=strlen(message);
        for ( ; j < i; j++)
        {
          (void) FormatLocaleString(message,MagickPathExtent,"%g,%g ",
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
        (void) CopyMagickString(message,"  <polygon points=\"",
          MagickPathExtent);
        (void) WriteBlobString(image,message);
        length=strlen(message);
        for ( ; j < i; j++)
        {
          (void) FormatLocaleString(message,MagickPathExtent,"%g,%g ",
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

        (void) GetNextToken(q,&q,extent,token);
        number_attributes=1;
        for (p=token; *p != '\0'; p++)
          if (isalpha((int) ((unsigned char) *p)) != 0)
            number_attributes++;
        if (i > (ssize_t) (number_points-6*BezierQuantum*number_attributes-1))
          {
            number_points+=6*BezierQuantum*number_attributes;
            primitive_info=(PrimitiveInfo *) ResizeQuantumMemory(primitive_info,
              number_points,sizeof(*primitive_info));
            if (primitive_info == (PrimitiveInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
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
      case AlphaPrimitive:
      case ColorPrimitive:
      {
        if (primitive_info[j].coordinates != 1)
          {
            status=MagickFalse;
            break;
          }
        (void) GetNextToken(q,&q,extent,token);
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
        (void) GetNextToken(q,&q,extent,token);
        (void) FormatLocaleString(message,MagickPathExtent,
          "  <text x=\"%g\" y=\"%g\">",primitive_info[j].point.x,
          primitive_info[j].point.y);
        (void) WriteBlobString(image,message);
        for (p=token; *p != '\0'; p++)
          switch (*p)
          {
            case '<': (void) WriteBlobString(image,"&lt;"); break;
            case '>': (void) WriteBlobString(image,"&gt;"); break;
            case '&': (void) WriteBlobString(image,"&amp;"); break;
            default: (void) WriteBlobByte(image,(unsigned char) *p); break;
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
        (void) GetNextToken(q,&q,extent,token);
        (void) FormatLocaleString(message,MagickPathExtent,
          "  <image x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" "
          "href=\"%s\"/>\n",primitive_info[j].point.x,
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             M   M  SSSSS  L                                 %
%                             MM MM  SS     L                                 %
%                             M M M   SSS   L                                 %
%                             M   M     SS  L                                 %
%                             M   M  SSSSS  LLLLL                             %
%                                                                             %
%                                                                             %
%                    Execute Magick Scripting Language Scripts.               %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                             Leonard Rosenthol                               %
%                             William Radcliffe                               %
%                               December 2001                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/composite.h"
#include "magick/constitute.h"
#include "magick/decorate.h"
#include "magick/display.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fx.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/quantum-private.h"
#include "magick/resize.h"
#include "magick/segment.h"
#include "magick/shear.h"
#include "magick/signature.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/threshold.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_XML_DELEGATE)
#  if defined(__WINDOWS__)
#    if defined(__MINGW32__)
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

/*
  Define Declatations.
*/
#define ThrowMSLException(severity,tag,reason) \
  (void) ThrowMagickException(msl_info->exception,GetMagickModule(),severity, \
    tag,"`%s'",reason);

/*
  Typedef declaractions.
*/
typedef struct _MSLGroupInfo
{
  unsigned long
    numImages;  /* how many images are in this group */
} MSLGroupInfo;

typedef struct _MSLInfo
{
  ExceptionInfo
    *exception;

  long
    n,
    number_groups;

  ImageInfo
    **image_info;

  DrawInfo
   **draw_info;

  Image
    **attributes,
    **image;

  char
    *content;

  MSLGroupInfo
    *group_info;

#if defined(MAGICKCORE_XML_DELEGATE)
  xmlParserCtxtPtr
    parser;

  xmlDocPtr
    document;
#endif
} MSLInfo;

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_XML_DELEGATE)
static MagickBooleanType
  WriteMSLImage(const ImageInfo *,Image *);
#endif

#if defined(MAGICKCORE_XML_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M S L I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadMSLImage() reads a Magick Scripting Language file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMSLImage method is:
%
%      Image *ReadMSLImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MSLIsStandalone(void *context)
{
  MSLInfo
    *msl_info;

  /*
    Is this document tagged standalone?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.MSLIsStandalone()");
  msl_info=(MSLInfo *) context;
  return(msl_info->document->standalone == 1);
}

static int MSLHasInternalSubset(void *context)
{
  MSLInfo
    *msl_info;

  /*
    Does this document has an internal subset?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.MSLHasInternalSubset()");
  msl_info=(MSLInfo *) context;
  return(msl_info->document->intSubset != NULL);
}

static int MSLHasExternalSubset(void *context)
{
  MSLInfo
    *msl_info;

  /*
    Does this document has an external subset?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.MSLHasExternalSubset()");
  msl_info=(MSLInfo *) context;
  return(msl_info->document->extSubset != NULL);
}

static void MSLInternalSubset(void *context,const xmlChar *name,
  const xmlChar *external_id,const xmlChar *system_id)
{
  MSLInfo
    *msl_info;

  /*
    Does this document has an internal subset?
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.internalSubset(%s %s %s)",name,
    (external_id != (const xmlChar *) NULL ? (char *) external_id : " "),
    (system_id != (const xmlChar *) NULL ? (char *) system_id : " "));
  msl_info=(MSLInfo *) context;
  (void) xmlCreateIntSubset(msl_info->document,name,external_id,system_id);
}

static xmlParserInputPtr MSLResolveEntity(void *context,
  const xmlChar *public_id,const xmlChar *system_id)
{
  MSLInfo
    *msl_info;

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
    (public_id != (const xmlChar *) NULL ? (char *) public_id : "none"),
    (system_id != (const xmlChar *) NULL ? (char *) system_id : "none"));
  msl_info=(MSLInfo *) context;
  stream=xmlLoadExternalEntity((const char *) system_id,(const char *)
    public_id,msl_info->parser);
  return(stream);
}

static xmlEntityPtr MSLGetEntity(void *context,const xmlChar *name)
{
  MSLInfo
    *msl_info;

  /*
    Get an entity by name.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.MSLGetEntity(%s)",(char *) name);
  msl_info=(MSLInfo *) context;
  return(xmlGetDocEntity(msl_info->document,name));
}

static xmlEntityPtr MSLGetParameterEntity(void *context,const xmlChar *name)
{
  MSLInfo
    *msl_info;

  /*
    Get a parameter entity by name.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.getParameterEntity(%s)",(char *) name);
  msl_info=(MSLInfo *) context;
  return(xmlGetParameterEntity(msl_info->document,name));
}

static void MSLEntityDeclaration(void *context,const xmlChar *name,int type,
  const xmlChar *public_id,const xmlChar *system_id,xmlChar *content)
{
  MSLInfo
    *msl_info;

  /*
    An entity definition has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.entityDecl(%s, %d, %s, %s, %s)",name,type,
    public_id != (const xmlChar *) NULL ? (char *) public_id : "none",
    system_id != (const xmlChar *) NULL ? (char *) system_id : "none",content);
  msl_info=(MSLInfo *) context;
  if (msl_info->parser->inSubset == 1)
    (void) xmlAddDocEntity(msl_info->document,name,type,public_id,system_id,
      content);
  else
    if (msl_info->parser->inSubset == 2)
      (void) xmlAddDtdEntity(msl_info->document,name,type,public_id,system_id,
        content);
}

static void MSLAttributeDeclaration(void *context,const xmlChar *element,
  const xmlChar *name,int type,int value,const xmlChar *default_value,
  xmlEnumerationPtr tree)
{
  MSLInfo
    *msl_info;

  xmlChar
    *fullname,
    *prefix;

  xmlParserCtxtPtr
    parser;

  /*
    An attribute definition has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.attributeDecl(%s, %s, %d, %d, %s, ...)\n",element,name,type,value,
    default_value);
  msl_info=(MSLInfo *) context;
  fullname=(xmlChar *) NULL;
  prefix=(xmlChar *) NULL;
  parser=msl_info->parser;
  fullname=(xmlChar *) xmlSplitQName(parser,name,&prefix);
  if (parser->inSubset == 1)
    (void) xmlAddAttributeDecl(&parser->vctxt,msl_info->document->intSubset,
      element,fullname,prefix,(xmlAttributeType) type,
      (xmlAttributeDefault) value,default_value,tree);
  else
    if (parser->inSubset == 2)
      (void) xmlAddAttributeDecl(&parser->vctxt,msl_info->document->extSubset,
        element,fullname,prefix,(xmlAttributeType) type,
        (xmlAttributeDefault) value,default_value,tree);
  if (prefix != (xmlChar *) NULL)
    xmlFree(prefix);
  if (fullname != (xmlChar *) NULL)
    xmlFree(fullname);
}

static void MSLElementDeclaration(void *context,const xmlChar *name,int type,
  xmlElementContentPtr content)
{
  MSLInfo
    *msl_info;

  xmlParserCtxtPtr
    parser;

  /*
    An element definition has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.elementDecl(%s, %d, ...)",name,type);
  msl_info=(MSLInfo *) context;
  parser=msl_info->parser;
  if (parser->inSubset == 1)
    (void) xmlAddElementDecl(&parser->vctxt,msl_info->document->intSubset,
      name,(xmlElementTypeVal) type,content);
  else
    if (parser->inSubset == 2)
      (void) xmlAddElementDecl(&parser->vctxt,msl_info->document->extSubset,
        name,(xmlElementTypeVal) type,content);
}

static void MSLNotationDeclaration(void *context,const xmlChar *name,
  const xmlChar *public_id,const xmlChar *system_id)
{
  MSLInfo
    *msl_info;

  xmlParserCtxtPtr
    parser;

  /*
    What to do when a notation declaration has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.notationDecl(%s, %s, %s)",name,
    public_id != (const xmlChar *) NULL ? (char *) public_id : "none",
    system_id != (const xmlChar *) NULL ? (char *) system_id : "none");
  msl_info=(MSLInfo *) context;
  parser=msl_info->parser;
  if (parser->inSubset == 1)
    (void) xmlAddNotationDecl(&parser->vctxt,msl_info->document->intSubset,
      name,public_id,system_id);
  else
    if (parser->inSubset == 2)
      (void) xmlAddNotationDecl(&parser->vctxt,msl_info->document->intSubset,
        name,public_id,system_id);
}

static void MSLUnparsedEntityDeclaration(void *context,const xmlChar *name,
  const xmlChar *public_id,const xmlChar *system_id,const xmlChar *notation)
{
  MSLInfo
    *msl_info;

  /*
    What to do when an unparsed entity declaration is parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.unparsedEntityDecl(%s, %s, %s, %s)",name,
    public_id != (const xmlChar *) NULL ? (char *) public_id : "none",
    system_id != (const xmlChar *) NULL ? (char *) system_id : "none",notation);
  msl_info=(MSLInfo *) context;
  (void) xmlAddDocEntity(msl_info->document,name,
    XML_EXTERNAL_GENERAL_UNPARSED_ENTITY,public_id,system_id,notation);

}

static void MSLSetDocumentLocator(void *context,xmlSAXLocatorPtr location)
{
  MSLInfo
    *msl_info;

  /*
    Receive the document locator at startup, actually xmlDefaultSAXLocator.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.setDocumentLocator()\n");
  (void) location;
  msl_info=(MSLInfo *) context;
}

static void MSLStartDocument(void *context)
{
  MSLInfo
    *msl_info;

  xmlParserCtxtPtr
    parser;

  /*
    Called when the document start being processed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.startDocument()");
  msl_info=(MSLInfo *) context;
  parser=msl_info->parser;
  msl_info->document=xmlNewDoc(parser->version);
  if (msl_info->document == (xmlDocPtr) NULL)
    return;
  if (parser->encoding == NULL)
    msl_info->document->encoding=NULL;
  else
    msl_info->document->encoding=xmlStrdup(parser->encoding);
  msl_info->document->standalone=parser->standalone;
}

static void MSLEndDocument(void *context)
{
  MSLInfo
    *msl_info;

  /*
    Called when the document end has been detected.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.endDocument()");
  msl_info=(MSLInfo *) context;
  if (msl_info->content != (char *) NULL)
    msl_info->content=DestroyString(msl_info->content);
}

static void MSLPushImage(MSLInfo *msl_info,Image *image)
{
  long
    n;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(msl_info != (MSLInfo *) NULL);
  msl_info->n++;
  n=msl_info->n;
  msl_info->image_info=(ImageInfo **) ResizeQuantumMemory(msl_info->image_info,
    (n+1),sizeof(*msl_info->image_info));
  msl_info->draw_info=(DrawInfo **) ResizeQuantumMemory(msl_info->draw_info,
    (n+1),sizeof(*msl_info->draw_info));
  msl_info->attributes=(Image **) ResizeQuantumMemory(msl_info->attributes,
    (n+1),sizeof(*msl_info->attributes));
  msl_info->image=(Image **) ResizeQuantumMemory(msl_info->image,(n+1),
    sizeof(*msl_info->image));
  if ((msl_info->image_info == (ImageInfo **) NULL) ||
    (msl_info->draw_info == (DrawInfo **) NULL) ||
    (msl_info->attributes == (Image **) NULL) ||
    (msl_info->image == (Image **) NULL))
      ThrowMSLException(ResourceLimitFatalError,"MemoryAllocationFailed","msl");
  msl_info->image_info[n]=CloneImageInfo(msl_info->image_info[n-1]);
  msl_info->draw_info[n]=CloneDrawInfo(msl_info->image_info[n-1],
    msl_info->draw_info[n-1]);
  if (image == (Image *) NULL)
    msl_info->attributes[n]=AcquireImage(msl_info->image_info[n]);
  else
    msl_info->attributes[n]=CloneImage(image,0,0,MagickTrue,&image->exception);
  msl_info->image[n]=(Image *) image;
  if ((msl_info->image_info[n] == (ImageInfo *) NULL) ||
    (msl_info->attributes[n] == (Image *) NULL))
    ThrowMSLException(ResourceLimitFatalError,"MemoryAllocationFailed","msl");
  if (msl_info->number_groups != 0)
    msl_info->group_info[msl_info->number_groups-1].numImages++;
}

static void MSLPopImage(MSLInfo *msl_info)
{
  if (msl_info->number_groups != 0)
    return;
  if (msl_info->image[msl_info->n] != (Image *) NULL)
    msl_info->image[msl_info->n]=DestroyImage(msl_info->image[msl_info->n]);
  msl_info->attributes[msl_info->n]=DestroyImage(
    msl_info->attributes[msl_info->n]);
  msl_info->image_info[msl_info->n]=DestroyImageInfo(
    msl_info->image_info[msl_info->n]);
  msl_info->n--;
}

static void MSLStartElement(void *context,const xmlChar *tag,
  const xmlChar **attributes)
{
  AffineMatrix
    affine,
    current;

  ChannelType
    channel;

  char
    key[MaxTextExtent],
    *value;

  const char
    *attribute,
    *keyword;

  double
    angle;

  DrawInfo
    *draw_info;

  ExceptionInfo
    exception;

  GeometryInfo
    geometry_info;

  Image
    *image;

  int
    flags;

  long
    option,
    j,
    n,
    x,
    y;

  MSLInfo
    *msl_info;

  RectangleInfo
    geometry;

  register long
    i;

  unsigned long
    height,
    width;

  /*
    Called when an opening tag has been processed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.startElement(%s",tag);
  GetExceptionInfo(&exception);
  msl_info=(MSLInfo *) context;
  n=msl_info->n;
  keyword=(const char *) NULL;
  value=(char *) NULL;
  SetGeometryInfo(&geometry_info);
  channel=DefaultChannels;
  switch (*tag)
  {
    case 'A':
    case 'a':
    {
      if (LocaleCompare((char *) tag,"add-noise") == 0)
        {
          Image
            *noise_image;

          NoiseType
            noise;

          /*
            Add noise image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          noise=UniformNoise;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"channel") == 0)
                    {
                      option=ParseChannelOption(value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedChannelType",
                          value);
                      channel=(ChannelType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'N':
                case 'n':
                {
                  if (LocaleCompare(keyword,"noise") == 0)
                    {
                      option=ParseMagickOption(MagickNoiseOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedNoiseType",
                          value);
                      noise=(NoiseType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          noise_image=AddNoiseImageChannel(msl_info->image[n],channel,noise,
            &msl_info->image[n]->exception);
          if (noise_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=noise_image;
          break;
        }
      if (LocaleCompare((char *) tag,"annotate") == 0)
        {
          char
            text[MaxTextExtent];

          /*
            Annotate image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          draw_info=CloneDrawInfo(msl_info->image_info[n],
            msl_info->draw_info[n]);
          angle=0.0;
          current=draw_info->affine;
          GetAffineMatrix(&affine);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'A':
                case 'a':
                {
                  if (LocaleCompare(keyword,"affine") == 0)
                    {
                      char
                        *p;

                      p=value;
                      draw_info->affine.sx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.rx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.ry=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.sy=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.tx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.ty=strtod(p,&p);
                      break;
                    }
                  if (LocaleCompare(keyword,"align") == 0)
                    {
                      option=ParseMagickOption(MagickAlignOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedAlignType",
                          value);
                      draw_info->align=(AlignType) option;
                      break;
                    }
                  if (LocaleCompare(keyword,"antialias") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      draw_info->stroke_antialias=(MagickBooleanType) option;
                      draw_info->text_antialias=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"density") == 0)
                    {
                      CloneString(&draw_info->density,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'E':
                case 'e':
                {
                  if (LocaleCompare(keyword,"encoding") == 0)
                    {
                      CloneString(&draw_info->encoding,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword, "fill") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->fill,
                        &exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"family") == 0)
                    {
                      CloneString(&draw_info->family,value);
                      break;
                    }
                  if (LocaleCompare(keyword,"font") == 0)
                    {
                      CloneString(&draw_info->font,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  if (LocaleCompare(keyword,"gravity") == 0)
                    {
                      option=ParseMagickOption(MagickGravityOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedGravityType",
                          value);
                      draw_info->gravity=(GravityType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'P':
                case 'p':
                {
                  if (LocaleCompare(keyword,"pointsize") == 0)
                    {
                      draw_info->pointsize=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"rotate") == 0)
                    {
                      angle=atof(value);
                      affine.sx=cos(DegreesToRadians(fmod(angle,360.0)));
                      affine.rx=sin(DegreesToRadians(fmod(angle,360.0)));
                      affine.ry=(-sin(DegreesToRadians(fmod(angle,360.0))));
                      affine.sy=cos(DegreesToRadians(fmod(angle,360.0)));
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"scale") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      affine.sx=geometry_info.rho;
                      affine.sy=geometry_info.sigma;
                      break;
                    }
                  if (LocaleCompare(keyword,"skewX") == 0)
                    {
                      angle=atof(value);
                      affine.ry=tan(DegreesToRadians(fmod((double) angle,
                        360.0)));
                      break;
                    }
                  if (LocaleCompare(keyword,"skewY") == 0)
                    {
                      angle=atof(value);
                      affine.rx=tan(DegreesToRadians(fmod((double) angle,
                        360.0)));
                      break;
                    }
                  if (LocaleCompare(keyword,"stretch") == 0)
                    {
                      option=ParseMagickOption(MagickStretchOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedStretchType",
                          value);
                      draw_info->stretch=(StretchType) option;
                      break;
                    }
                  if (LocaleCompare(keyword, "stroke") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->stroke,
                        &exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"strokewidth") == 0)
                    {
                      draw_info->stroke_width=atol(value);
                      break;
                    }
                  if (LocaleCompare(keyword,"style") == 0)
                    {
                      option=ParseMagickOption(MagickStyleOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedStyleType",
                          value);
                      draw_info->style=(StyleType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'T':
                case 't':
                {
                  if (LocaleCompare(keyword,"text") == 0)
                    {
                      CloneString(&draw_info->text,value);
                      break;
                    }
                  if (LocaleCompare(keyword,"translate") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      affine.tx=geometry_info.rho;
                      affine.ty=geometry_info.sigma;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'U':
                case 'u':
                {
                  if (LocaleCompare(keyword, "undercolor") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->undercolor,
                        &exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"weight") == 0)
                    {
                      draw_info->weight=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) FormatMagickString(text,MaxTextExtent,"%lux%lu%+ld%+ld",
            geometry.width,geometry.height,geometry.x,geometry.y);
          CloneString(&draw_info->geometry,text);
          draw_info->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
          draw_info->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
          draw_info->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
          draw_info->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
          draw_info->affine.tx=current.sx*affine.tx+current.ry*affine.ty+
            current.tx;
          draw_info->affine.ty=current.rx*affine.tx+current.sy*affine.ty+
            current.ty;
          (void) AnnotateImage(msl_info->image[n],draw_info);
          draw_info=DestroyDrawInfo(draw_info);
          break;
        }
      if (LocaleCompare((char *) tag,"append") == 0)
        {
          Image
            *append_image;

          MagickBooleanType
            stack;
   
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          stack=MagickFalse;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"stack") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      stack=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          append_image=AppendImages(msl_info->image[n],stack,
            &msl_info->image[n]->exception);
          if (append_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=append_image;
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
      break;
    }
    case 'B':
    case 'b':
    {
      if (LocaleCompare((char *) tag,"blur") == 0)
        {
          Image
            *blur_image;

          /*
            Blur image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"channel") == 0)
                    {
                      option=ParseChannelOption(value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedChannelType",
                          value);
                      channel=(ChannelType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"radius") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"sigma") == 0)
                    {
                      geometry_info.sigma=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          blur_image=BlurImageChannel(msl_info->image[n],channel,
            geometry_info.rho,geometry_info.sigma,
            &msl_info->image[n]->exception);
          if (blur_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=blur_image;
          break;
        }
      if (LocaleCompare((char *) tag,"border") == 0)
        {
          Image
            *border_image;

          /*
            Border image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          SetGeometry(msl_info->image[n],&geometry);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"compose") == 0)
                    {
                      option=ParseMagickOption(MagickComposeOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedComposeType",
                          value);
                      msl_info->image[n]->compose=(CompositeOperator) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword, "fill") == 0)
                    {
                      (void) QueryColorDatabase(value,
                        &msl_info->image[n]->border_color,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      geometry.height=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      geometry.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          border_image=BorderImage(msl_info->image[n],&geometry,
            &msl_info->image[n]->exception);
          if (border_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=border_image;
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'C':
    case 'c':
    {
      if (LocaleCompare((char *) tag,"colorize") == 0)
        {
          char
            opacity[MaxTextExtent];

          Image
            *colorize_image;

          PixelPacket
            target;

          /*
            Add noise image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          target=msl_info->image[n]->background_color;
          (void) CopyMagickString(opacity,"100",MaxTextExtent);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword,"fill") == 0)
                    {
                      (void) QueryColorDatabase(value,&target,
                        &msl_info->image[n]->exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'O':
                case 'o':
                {
                  if (LocaleCompare(keyword,"opacity") == 0)
                    {
                      (void) CopyMagickString(opacity,value,MaxTextExtent);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          colorize_image=ColorizeImage(msl_info->image[n],opacity,target,
            &msl_info->image[n]->exception);
          if (colorize_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=colorize_image;
          break;
        }
      if (LocaleCompare((char *) tag, "charcoal") == 0)
      {
        double  radius = 0.0,
            sigma = 1.0;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        /*
        NOTE: charcoal can have no attributes, since we use all the defaults!
        */
        if (attributes != (const xmlChar **) NULL)
        {
          for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
          {
          keyword=(const char *) attributes[i++];
          CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
            msl_info->attributes[n],(char *) attributes[i]));
          switch (*keyword)
          {
            case 'R':
            case 'r':
            {
              if (LocaleCompare(keyword, "radius") == 0)
              {
                radius = atof( value );
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'S':
            case 's':
            {
              if (LocaleCompare(keyword,"sigma") == 0)
              {
                sigma = atol( value );
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            default:
            {
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
          }
          }
        }

        /*
          charcoal image.
        */
        {
        Image
          *newImage;

        newImage=CharcoalImage(msl_info->image[n],radius,sigma,
          &msl_info->image[n]->exception);
        if (newImage == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=newImage;
        break;
        }
      }
      if (LocaleCompare((char *) tag,"chop") == 0)
        {
          Image
            *chop_image;

          /*
            Chop image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          SetGeometry(msl_info->image[n],&geometry);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      geometry.height=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      geometry.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          chop_image=ChopImage(msl_info->image[n],&geometry,
            &msl_info->image[n]->exception);
          if (chop_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=chop_image;
          break;
        }
      if (LocaleCompare((char *) tag,"color-floodfill") == 0)
        {
          PaintMethod
            paint_method;

          MagickPixelPacket
            target;

          /*
            Color floodfill image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          draw_info=CloneDrawInfo(msl_info->image_info[n],
            msl_info->draw_info[n]);
          SetGeometry(msl_info->image[n],&geometry);
          paint_method=FloodfillMethod;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'B':
                case 'b':
                {
                  if (LocaleCompare(keyword,"bordercolor") == 0)
                    {
                      (void) QueryMagickColor(value,&target,&exception);
                      paint_method=FillToBorderMethod;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword,"fill") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->fill,
                        &exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"fuzz") == 0)
                    {
                      msl_info->image[n]->fuzz=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      (void) GetOneVirtualMagickPixel(msl_info->image[n],
                        geometry.x,geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      (void) GetOneVirtualMagickPixel(msl_info->image[n],
                        geometry.x,geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      (void) GetOneVirtualMagickPixel(msl_info->image[n],
                        geometry.x,geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) FloodfillPaintImage(msl_info->image[n],DefaultChannels,
            draw_info,&target,geometry.x,geometry.y,
            paint_method == FloodfillMethod ? MagickFalse : MagickTrue);
          draw_info=DestroyDrawInfo(draw_info);
          break;
        }
      if (LocaleCompare((char *) tag,"comment") == 0)
        break;
      if (LocaleCompare((char *) tag,"composite") == 0)
        {
          char
            composite_geometry[MaxTextExtent];

          CompositeOperator
            compose;

          Image
            *composite_image,
            *rotate_image;

          PixelPacket
            target;

          /*
            Composite image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          composite_image=NewImageList();
          compose=OverCompositeOp;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"compose") == 0)
                    {
                      option=ParseMagickOption(MagickComposeOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedComposeType",
                          value);
                      compose=(CompositeOperator) option;
                      break;
                    }
                  break;
                }
                case 'I':
                case 'i':
                {
                  if (LocaleCompare(keyword,"image") == 0)
                    for (j=0; j < msl_info->n; j++)
                    {
                      const char
                        *attribute;
    
                      attribute=GetImageProperty(msl_info->attributes[j],"id");
                      if ((attribute != (const char *) NULL)  &&
                          (LocaleCompare(attribute,value) == 0))
                        {
                          composite_image=CloneImage(msl_info->image[j],0,0,
                            MagickFalse,&exception);
                          break;
                        }
                    }
                  break;
                }
                default:
                  break;
              }
            }
          if (composite_image == (Image *) NULL)
            break;
          rotate_image=NewImageList();
          SetGeometry(msl_info->image[n],&geometry);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'B':
                case 'b':
                {
                  if (LocaleCompare(keyword,"blend") == 0)
                    {
                      (void) SetImageArtifact(composite_image,
                                            "compose:args",value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"channel") == 0)
                    {
                      option=ParseChannelOption(value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedChannelType",
                          value);
                      channel=(ChannelType) option;
                      break;
                    }
                  if (LocaleCompare(keyword, "color") == 0)
                    {
                      (void) QueryColorDatabase(value,
                        &composite_image->background_color,&exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"compose") == 0)
                    break;
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      (void) GetOneVirtualPixel(msl_info->image[n],geometry.x,
                        geometry.y,&target,&exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"gravity") == 0)
                    {
                      option=ParseMagickOption(MagickGravityOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedGravityType",
                          value);
                      msl_info->image[n]->gravity=(GravityType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'I':
                case 'i':
                {
                  if (LocaleCompare(keyword,"image") == 0)
                    break;
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'M':
                case 'm':
                {
                  if (LocaleCompare(keyword,"mask") == 0)
                    for (j=0; j < msl_info->n; j++)
                    {
                      const char
                        *attribute;

                      attribute=GetImageProperty(msl_info->attributes[j],"id");
                      if ((attribute != (const char *) NULL)  &&
                          (LocaleCompare(value,value) == 0))
                        {
                          SetImageType(composite_image,TrueColorMatteType);
                          (void) CompositeImage(composite_image,
                            CopyOpacityCompositeOp,msl_info->image[j],0,0);
                          break;
                        }
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'O':
                case 'o':
                {
                  if (LocaleCompare(keyword,"opacity") == 0)
                    {
                      long
                        opacity,
                        y;
                    
                      register long
                        x;
                    
                      register PixelPacket
                        *q;
                    
                      CacheView
                        *composite_view;

                      opacity=QuantumRange-atol(value);
                      if (compose != DissolveCompositeOp)
                        {
                          (void) SetImageOpacity(composite_image,(Quantum) opacity);
                          break;
                        }
                      (void) SetImageArtifact(msl_info->image[n],
                                            "compose:args",value);
                      if (composite_image->matte != MagickTrue)
                        (void) SetImageOpacity(composite_image,OpaqueOpacity);
                      composite_view=AcquireCacheView(composite_image);
                      for (y=0; y < (long) composite_image->rows ; y++)
                      { 
                        q=GetCacheViewAuthenticPixels(composite_view,0,y,(long)
                          composite_image->columns,1,&exception);
                        for (x=0; x < (long) composite_image->columns; x++)
                        { 
                          if (q->opacity == OpaqueOpacity)
                            q->opacity=RoundToQuantum(opacity);
                          q++;
                        }
                        if (SyncCacheViewAuthenticPixels(composite_view,&exception) == MagickFalse)
                          break;
                      }
                      composite_view=DestroyCacheView(composite_view);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"rotate") == 0)
                    {
                      rotate_image=RotateImage(composite_image,atof(value),
                        &exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'T':
                case 't':
                {
                  if (LocaleCompare(keyword,"tile") == 0)
                    {
                      MagickBooleanType
                        tile;

                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      tile=(MagickBooleanType) option;
                      if (rotate_image != (Image *) NULL)
                        (void) SetImageArtifact(rotate_image,
                          "compose:outside-overlay","false");
                      else
                        (void) SetImageArtifact(composite_image,
                          "compose:outside-overlay","false");
                       image=msl_info->image[n];
                       height=composite_image->rows;
                       width=composite_image->columns;
                       for (y=0; y < (long) image->rows; y+=height)
                         for (x=0; x < (long) image->columns; x+=width)
                         {
                           if (rotate_image != (Image *) NULL)
                             (void) CompositeImage(image,compose,rotate_image,
                               x,y);
                           else
                             (void) CompositeImage(image,compose,
                               composite_image,x,y);
                         }
                      if (rotate_image != (Image *) NULL)
                        rotate_image=DestroyImage(rotate_image);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      (void) GetOneVirtualPixel(msl_info->image[n],geometry.x,
                        geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      (void) GetOneVirtualPixel(msl_info->image[n],geometry.x,
                        geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          image=msl_info->image[n];
          (void) FormatMagickString(composite_geometry,MaxTextExtent,
            "%lux%lu%+ld%+ld",composite_image->columns,composite_image->rows,
            geometry.x,geometry.y);
          flags=ParseGravityGeometry(image,composite_geometry,&geometry,
            &exception);
          if (rotate_image == (Image *) NULL)
            CompositeImageChannel(image,channel,compose,composite_image,
              geometry.x,geometry.y);
          else
            {
              /*
                Rotate image.
              */
              geometry.x-=(long) (rotate_image->columns-
                composite_image->columns)/2;
              geometry.y-=(long) (rotate_image->rows-composite_image->rows)/2;
              CompositeImageChannel(image,channel,compose,rotate_image,
                geometry.x,geometry.y);
              rotate_image=DestroyImage(rotate_image);
            }
          composite_image=DestroyImage(composite_image);
          break;
        }
      if (LocaleCompare((char *) tag,"contrast") == 0)
        {
          MagickBooleanType
            sharpen;

          /*
            Contrast image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          sharpen=MagickFalse;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"sharpen") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      sharpen=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) ContrastImage(msl_info->image[n],sharpen);
          break;
        }
      if (LocaleCompare((char *) tag,"crop") == 0)
        {
          Image
            *crop_image;

          /*
            Crop image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          SetGeometry(msl_info->image[n],&geometry);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      geometry.height=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      geometry.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          crop_image=CropImage(msl_info->image[n],&geometry,
            &msl_info->image[n]->exception);
          if (crop_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=crop_image;
          break;
        }
      if (LocaleCompare((char *) tag,"cycle-colormap") == 0)
        {
          long
            display;

          /*
            Cycle-colormap image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          display=0;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"display") == 0)
                    {
                      display=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) CycleColormapImage(msl_info->image[n],display);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'D':
    case 'd':
    {
      if (LocaleCompare((char *) tag,"despeckle") == 0)
        {
          Image
            *despeckle_image;

          /*
            Despeckle image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            }
          despeckle_image=DespeckleImage(msl_info->image[n],
            &msl_info->image[n]->exception);
          if (despeckle_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=despeckle_image;
          break;
        }
      if (LocaleCompare((char *) tag,"display") == 0)
        {
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) DisplayImages(msl_info->image_info[n],msl_info->image[n]);
          break;
        }
      if (LocaleCompare((char *) tag,"draw") == 0)
        {
          char
            text[MaxTextExtent];

          /*
            Annotate image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          draw_info=CloneDrawInfo(msl_info->image_info[n],
            msl_info->draw_info[n]);
          angle=0.0;
          current=draw_info->affine;
          GetAffineMatrix(&affine);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'A':
                case 'a':
                {
                  if (LocaleCompare(keyword,"affine") == 0)
                    {
                      char
                        *p;

                      p=value;
                      draw_info->affine.sx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.rx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.ry=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.sy=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.tx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.ty=strtod(p,&p);
                      break;
                    }
                  if (LocaleCompare(keyword,"align") == 0)
                    {
                      option=ParseMagickOption(MagickAlignOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedAlignType",
                          value);
                      draw_info->align=(AlignType) option;
                      break;
                    }
                  if (LocaleCompare(keyword,"antialias") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      draw_info->stroke_antialias=(MagickBooleanType) option;
                      draw_info->text_antialias=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"density") == 0)
                    {
                      CloneString(&draw_info->density,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'E':
                case 'e':
                {
                  if (LocaleCompare(keyword,"encoding") == 0)
                    {
                      CloneString(&draw_info->encoding,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword, "fill") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->fill,
                        &exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"family") == 0)
                    {
                      CloneString(&draw_info->family,value);
                      break;
                    }
                  if (LocaleCompare(keyword,"font") == 0)
                    {
                      CloneString(&draw_info->font,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  if (LocaleCompare(keyword,"gravity") == 0)
                    {
                      option=ParseMagickOption(MagickGravityOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedGravityType",
                          value);
                      draw_info->gravity=(GravityType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'P':
                case 'p':
                {
                  if (LocaleCompare(keyword,"primitive") == 0)
                    { 
                      CloneString(&draw_info->primitive,value);
                      break;
                    }
                  if (LocaleCompare(keyword,"pointsize") == 0)
                    {
                      draw_info->pointsize=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"rotate") == 0)
                    {
                      angle=atof(value);
                      affine.sx=cos(DegreesToRadians(fmod(angle,360.0)));
                      affine.rx=sin(DegreesToRadians(fmod(angle,360.0)));
                      affine.ry=(-sin(DegreesToRadians(fmod(angle,360.0))));
                      affine.sy=cos(DegreesToRadians(fmod(angle,360.0)));
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"scale") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      affine.sx=geometry_info.rho;
                      affine.sy=geometry_info.sigma;
                      break;
                    }
                  if (LocaleCompare(keyword,"skewX") == 0)
                    {
                      angle=atof(value);
                      affine.ry=cos(DegreesToRadians(fmod(angle,360.0)));
                      break;
                    }
                  if (LocaleCompare(keyword,"skewY") == 0)
                    {
                      angle=atof(value);
                      affine.rx=cos(DegreesToRadians(fmod(angle,360.0)));
                      break;
                    }
                  if (LocaleCompare(keyword,"stretch") == 0)
                    {
                      option=ParseMagickOption(MagickStretchOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedStretchType",
                          value);
                      draw_info->stretch=(StretchType) option;
                      break;
                    }
                  if (LocaleCompare(keyword, "stroke") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->stroke,
                        &exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"strokewidth") == 0)
                    {
                      draw_info->stroke_width=atol(value);
                      break;
                    }
                  if (LocaleCompare(keyword,"style") == 0)
                    {
                      option=ParseMagickOption(MagickStyleOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedStyleType",
                          value);
                      draw_info->style=(StyleType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'T':
                case 't':
                {
                  if (LocaleCompare(keyword,"text") == 0)
                    {
                      CloneString(&draw_info->text,value);
                      break;
                    }
                  if (LocaleCompare(keyword,"translate") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      affine.tx=geometry_info.rho;
                      affine.ty=geometry_info.sigma;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'U':
                case 'u':
                {
                  if (LocaleCompare(keyword, "undercolor") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->undercolor,
                        &exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"weight") == 0)
                    {
                      draw_info->weight=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) FormatMagickString(text,MaxTextExtent,"%lux%lu%+ld%+ld",
            geometry.width,geometry.height,geometry.x,geometry.y);
          CloneString(&draw_info->geometry,text);
          draw_info->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
          draw_info->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
          draw_info->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
          draw_info->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
          draw_info->affine.tx=current.sx*affine.tx+current.ry*affine.ty+
            current.tx;
          draw_info->affine.ty=current.rx*affine.tx+current.sy*affine.ty+
            current.ty;
          (void) DrawImage(msl_info->image[n],draw_info);
          draw_info=DestroyDrawInfo(draw_info);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'E':
    case 'e':
    {
      if (LocaleCompare((char *) tag,"edge") == 0)
        {
          Image
            *edge_image;

          /*
            Edge image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"radius") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          edge_image=EdgeImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (edge_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=edge_image;
          break;
        }
      if (LocaleCompare((char *) tag,"emboss") == 0)
        {
          Image
            *emboss_image;

          /*
            Emboss image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"radius") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"sigma") == 0)
                    {
                      geometry_info.sigma=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          emboss_image=EmbossImage(msl_info->image[n],geometry_info.rho,
            geometry_info.sigma,&msl_info->image[n]->exception);
          if (emboss_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=emboss_image;
          break;
        }
      if (LocaleCompare((char *) tag,"enhance") == 0)
        {
          Image
            *enhance_image;

          /*
            Enhance image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            }
          enhance_image=EnhanceImage(msl_info->image[n],
            &msl_info->image[n]->exception);
          if (enhance_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=enhance_image;
          break;
        }
      if (LocaleCompare((char *) tag,"equalize") == 0)
        {
          /*
            Equalize image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) EqualizeImage(msl_info->image[n]);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'F':
    case 'f':
    {
      if (LocaleCompare((char *) tag, "flatten") == 0)
      {
        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }

        /* no attributes here */

        /* process the image */
        {
          Image
            *newImage;

          newImage=MergeImageLayers(msl_info->image[n],FlattenLayer,
            &msl_info->image[n]->exception);
          if (newImage == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=newImage;
          break;
        }
      }
      if (LocaleCompare((char *) tag,"flip") == 0)
        {
          Image
            *flip_image;

          /*
            Flip image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            }
          flip_image=FlipImage(msl_info->image[n],
            &msl_info->image[n]->exception);
          if (flip_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=flip_image;
          break;
        }
      if (LocaleCompare((char *) tag,"flop") == 0)
        {
          Image
            *flop_image;

          /*
            Flop image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            }
          flop_image=FlopImage(msl_info->image[n],
            &msl_info->image[n]->exception);
          if (flop_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=flop_image;
          break;
        }
      if (LocaleCompare((char *) tag,"frame") == 0)
        {
          FrameInfo
            frame_info;

          Image
            *frame_image;

          /*
            Frame image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          SetGeometry(msl_info->image[n],&geometry);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"compose") == 0)
                    {
                      option=ParseMagickOption(MagickComposeOptions,
                        MagickFalse,value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedComposeType",
                          value);
                      msl_info->image[n]->compose=(CompositeOperator) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword, "fill") == 0)
                    {
                      (void) QueryColorDatabase(value,
                        &msl_info->image[n]->matte_color,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      frame_info.width=geometry.width;
                      frame_info.height=geometry.height;
                      frame_info.outer_bevel=geometry.x;
                      frame_info.inner_bevel=geometry.y;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      frame_info.height=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'I':
                case 'i':
                {
                  if (LocaleCompare(keyword,"inner") == 0)
                    {
                      frame_info.inner_bevel=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'O':
                case 'o':
                {
                  if (LocaleCompare(keyword,"outer") == 0)
                    {
                      frame_info.outer_bevel=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      frame_info.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          frame_info.x=(long) frame_info.width;
          frame_info.y=(long) frame_info.height;
          frame_info.width=msl_info->image[n]->columns+2*frame_info.x;
          frame_info.height=msl_info->image[n]->rows+2*frame_info.y;
          frame_image=FrameImage(msl_info->image[n],&frame_info,
            &msl_info->image[n]->exception);
          if (frame_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=frame_image;
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare((char *) tag,"gamma") == 0)
        {
          char
            gamma[MaxTextExtent];

          MagickPixelPacket
            pixel;

          /*
            Gamma image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          channel=UndefinedChannel;
          pixel.red=0.0;
          pixel.green=0.0;
          pixel.blue=0.0;
          *gamma='\0';
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'B':
                case 'b':
                {
                  if (LocaleCompare(keyword,"blue") == 0)
                    {
                      pixel.blue=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"channel") == 0)
                    {
                      option=ParseChannelOption(value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedChannelType",
                          value);
                      channel=(ChannelType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"gamma") == 0)
                    {
                      (void) CopyMagickString(gamma,value,MaxTextExtent);
                      break;
                    }
                  if (LocaleCompare(keyword,"green") == 0)
                    {
                      pixel.green=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"red") == 0)
                    {
                      pixel.red=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          if (*gamma == '\0')
            (void) FormatMagickString(gamma,MaxTextExtent,"%g,%g,%g",
              (double) pixel.red,(double) pixel.green,(double) pixel.blue);
          switch (channel)          
          {
            default:
            {
              (void) GammaImage(msl_info->image[n],gamma);
              break;
            }
            case RedChannel:
            {
              (void) GammaImageChannel(msl_info->image[n],RedChannel,pixel.red);
              break;
            }
            case GreenChannel:
            {
              (void) GammaImageChannel(msl_info->image[n],GreenChannel,
                pixel.green);
              break;
            }
            case BlueChannel:
            {
              (void) GammaImageChannel(msl_info->image[n],BlueChannel,
                pixel.blue);
              break;
            }
          }
          break;
        }
      else if (LocaleCompare((char *) tag,"get") == 0)
        {
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes == (const xmlChar **) NULL)
            break;
          for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
          {
            keyword=(const char *) attributes[i++];
            CloneString(&value,(char *) attributes[i]);
            (void) CopyMagickString(key,value,MaxTextExtent);
            switch (*keyword)
            {
              case 'H':
              case 'h':
              {
                if (LocaleCompare(keyword,"height") == 0)
                  {
                    (void) FormatMagickString(value,MaxTextExtent,"%ld",
                      msl_info->image[n]->rows);
                    (void) SetImageProperty(msl_info->attributes[n],key,value);
                    break;
                  }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              }
              case 'W':
              case 'w':
              {
                if (LocaleCompare(keyword,"width") == 0)
                  {
                    (void) FormatMagickString(value,MaxTextExtent,"%ld",
                      msl_info->image[n]->columns);
                    (void) SetImageProperty(msl_info->attributes[n],key,value);
                    break;
                  }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              }
              default:
              {
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
            }
          }
          break;
        }
    else if (LocaleCompare((char *) tag, "group") == 0)
    {
      msl_info->number_groups++;
      msl_info->group_info=(MSLGroupInfo *) ResizeQuantumMemory(
        msl_info->group_info,msl_info->number_groups+1UL,
        sizeof(*msl_info->group_info));
      break;
    }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'I':
    case 'i':
    {
      if (LocaleCompare((char *) tag,"image") == 0)
        {
          long
            n;

          MSLPushImage(msl_info,(Image *) NULL);
          n=msl_info->n;
          if (attributes == (const xmlChar **) NULL)
            break;
          for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
          {
            keyword=(const char *) attributes[i++];
            CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
              msl_info->attributes[n],(char *) attributes[i]));
            switch (*keyword)
            {
      case 'B':
      case 'b':
        {
          if (LocaleCompare(keyword,"background") == 0)
          {
            (void) QueryColorDatabase(value,
              &msl_info->image_info[n]->background_color,&exception);
            break;
          }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
        }

      case 'C':
      case 'c':
        {
          if (LocaleCompare(keyword,"color") == 0)
          {
            Image
              *next_image;

            (void) CopyMagickString(msl_info->image_info[n]->filename,"xc:",
              MaxTextExtent);
            (void) ConcatenateMagickString(msl_info->image_info[n]->filename,
              value,MaxTextExtent);
            next_image=ReadImage(msl_info->image_info[n],&exception);
            CatchException(&exception);
            if (next_image == (Image *) NULL)
              continue;
            if (msl_info->image[n] == (Image *) NULL)
              msl_info->image[n]=next_image;
            else
              {
              register Image
                *p;

              /*
                Link image into image list.
              */
              p=msl_info->image[n];
              for ( ; p->next != (Image *) NULL; p=GetNextImageInList(p)) ;
              next_image->previous=p;
              p->next=next_image;
              }
            break;
          }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
        }

      case 'I':
      case 'i':
        {
          if (LocaleCompare(keyword,"id") == 0)
          {
            (void) SetImageProperty(msl_info->attributes[n],keyword,NULL);  /* make sure to clear it! */
            (void) SetImageProperty(msl_info->attributes[n],keyword,value);
            break;
          }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
        }
              case 'S':
              case 's':
              {
                if (LocaleCompare(keyword,"size") == 0)
                {
          CloneString(&msl_info->image_info[n]->size,value);
          break;
        }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
              default:
              {
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
            }
          }
          break;
        }
      if (LocaleCompare((char *) tag,"implode") == 0)
        {
          Image
            *implode_image;

          /*
            Implode image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'A':
                case 'a':
                {
                  if (LocaleCompare(keyword,"amount") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          implode_image=ImplodeImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (implode_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=implode_image;
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'L':
    case 'l':
    {
      if (LocaleCompare((char *) tag,"label") == 0)
        break;
      if (LocaleCompare((char *) tag, "level") == 0)
      {
        double
          levelBlack = 0, levelGamma = 1, levelWhite = QuantumRange;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
          break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
          keyword=(const char *) attributes[i++];
          CloneString(&value,(char *) attributes[i]);
          (void) CopyMagickString(key,value,MaxTextExtent);
          switch (*keyword)
          {
            case 'B':
            case 'b':
            {
              if (LocaleCompare(keyword,"black") == 0)
              {
                levelBlack = atof( value );
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'G':
            case 'g':
            {
              if (LocaleCompare(keyword,"gamma") == 0)
              {
                levelGamma = atof( value );
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'W':
            case 'w':
            {
              if (LocaleCompare(keyword,"white") == 0)
              {
                levelWhite = atof( value );
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            default:
            {
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
          }
        }

        /* process image */
        {
          char level[MaxTextExtent + 1];
          (void) FormatMagickString(level,MaxTextExtent,"%3.6f/%3.6f/%3.6f/",
            levelBlack,levelGamma,levelWhite);
          LevelImage ( msl_info->image[n], level );
          break;
        }
      }
    }
    case 'M':
    case 'm':
    {
      if (LocaleCompare((char *) tag,"magnify") == 0)
        {
          Image
            *magnify_image;

          /*
            Magnify image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            }
          magnify_image=MagnifyImage(msl_info->image[n],
            &msl_info->image[n]->exception);
          if (magnify_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=magnify_image;
          break;
        }
      if (LocaleCompare((char *) tag,"map") == 0)
        {
          Image
            *affinity_image;

          MagickBooleanType
            dither;

          QuantizeInfo
            *quantize_info;

          /*
            Map image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          affinity_image=NewImageList();
          dither=MagickFalse;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"dither") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      dither=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'I':
                case 'i':
                {
                  if (LocaleCompare(keyword,"image") == 0)
                    for (j=0; j < msl_info->n; j++)
                    {
                      const char
                        *attribute;
    
                      attribute=GetImageProperty(msl_info->attributes[j],"id");
                      if ((attribute != (const char *) NULL)  &&
                          (LocaleCompare(attribute,value) == 0))
                        {
                          affinity_image=CloneImage(msl_info->image[j],0,0,
                            MagickFalse,&exception);
                          break;
                        }
                    }
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          quantize_info=AcquireQuantizeInfo(msl_info->image_info[n]);
          quantize_info->dither=dither;
          (void) RemapImages(quantize_info,msl_info->image[n],
            affinity_image);
          quantize_info=DestroyQuantizeInfo(quantize_info);
          affinity_image=DestroyImage(affinity_image);
          break;
        }
      if (LocaleCompare((char *) tag,"matte-floodfill") == 0)
        {
          double
            opacity;

          MagickPixelPacket
            target;

          PaintMethod
            paint_method;

          /*
            Matte floodfill image.
          */
          opacity=0.0;
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          SetGeometry(msl_info->image[n],&geometry);
          paint_method=FloodfillMethod;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'B':
                case 'b':
                {
                  if (LocaleCompare(keyword,"bordercolor") == 0)
                    {
                      (void) QueryMagickColor(value,&target,&exception);
                      paint_method=FillToBorderMethod;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword,"fuzz") == 0)
                    {
                      msl_info->image[n]->fuzz=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      (void) GetOneVirtualMagickPixel(msl_info->image[n],
                        geometry.x,geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'O':
                case 'o':
                {
                  if (LocaleCompare(keyword,"opacity") == 0)
                    {
                      opacity=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      (void) GetOneVirtualMagickPixel(msl_info->image[n],
                        geometry.x,geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      (void) GetOneVirtualMagickPixel(msl_info->image[n],
                        geometry.x,geometry.y,&target,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          draw_info=CloneDrawInfo(msl_info->image_info[n],
            msl_info->draw_info[n]);
          draw_info->fill.opacity=RoundToQuantum(opacity);
          (void) FloodfillPaintImage(msl_info->image[n],OpacityChannel,
            draw_info,&target,geometry.x,geometry.y,
            paint_method == FloodfillMethod ? MagickFalse : MagickTrue);
          draw_info=DestroyDrawInfo(draw_info);
          break;
        }
      if (LocaleCompare((char *) tag,"median-filter") == 0)
        {
          Image
            *median_image;

          /*
            Median-filter image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"radius") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          median_image=MedianFilterImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (median_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=median_image;
          break;
        }
      if (LocaleCompare((char *) tag,"minify") == 0)
        {
          Image
            *minify_image;

          /*
            Minify image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            }
          minify_image=MinifyImage(msl_info->image[n],
            &msl_info->image[n]->exception);
          if (minify_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=minify_image;
          break;
        }
      if (LocaleCompare((char *) tag,"msl") == 0 )
        break;
      if (LocaleCompare((char *) tag,"modulate") == 0)
        {
          char
            modulate[MaxTextExtent];

          /*
            Modulate image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          geometry_info.rho=100.0;
          geometry_info.sigma=100.0;
          geometry_info.xi=100.0;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'B':
                case 'b':
                {
                  if (LocaleCompare(keyword,"blackness") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  if (LocaleCompare(keyword,"brightness") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword,"factor") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"hue") == 0)
                    {
                      geometry_info.xi=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'L':
                case 'l':
                {
                  if (LocaleCompare(keyword,"lightness") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"saturation") == 0)
                    {
                      geometry_info.sigma=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"whiteness") == 0)
                    {
                      geometry_info.sigma=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) FormatMagickString(modulate,MaxTextExtent,"%g,%g,%g",
            geometry_info.rho,geometry_info.sigma,geometry_info.xi);
          (void) ModulateImage(msl_info->image[n],modulate);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'N':
    case 'n':
    {
      if (LocaleCompare((char *) tag,"negate") == 0)
        {
          MagickBooleanType
            gray;

          /*
            Negate image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          gray=MagickFalse;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"channel") == 0)
                    {
                      option=ParseChannelOption(value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedChannelType",
                          value);
                      channel=(ChannelType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"gray") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      gray=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) NegateImageChannel(msl_info->image[n],channel,gray);
          break;
        }
      if (LocaleCompare((char *) tag,"normalize") == 0)
        {
          /*
            Normalize image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"channel") == 0)
                    {
                      option=ParseChannelOption(value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedChannelType",
                          value);
                      channel=(ChannelType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) NormalizeImageChannel(msl_info->image[n],channel);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'O':
    case 'o':
    {
      if (LocaleCompare((char *) tag,"oil-paint") == 0)
        {
          Image
            *paint_image;

          /*
            Oil-paint image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"radius") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          paint_image=OilPaintImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (paint_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=paint_image;
          break;
        }
      if (LocaleCompare((char *) tag,"opaque") == 0)
        {
          MagickPixelPacket
            fill_color,
            target;

          /*
            Opaque image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          (void) QueryMagickColor("none",&target,&exception);
          (void) QueryMagickColor("none",&fill_color,&exception);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"channel") == 0)
                    {
                      option=ParseChannelOption(value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedChannelType",
                          value);
                      channel=(ChannelType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword,"fill") == 0)
                    {
                      (void) QueryMagickColor(value,&fill_color,&exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"fuzz") == 0)
                    {
                      msl_info->image[n]->fuzz=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) OpaquePaintImageChannel(msl_info->image[n],channel,
            &target,&fill_color,MagickFalse);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare((char *) tag,"print") == 0)
        {
          if (attributes == (const xmlChar **) NULL)
            break;
          for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
          {
            keyword=(const char *) attributes[i++];
            attribute=InterpretImageProperties(msl_info->image_info[n],
              msl_info->attributes[n],(char *) attributes[i]);
            CloneString(&value,attribute);
            switch (*keyword)
            {
              case 'O':
              case 'o':
              {
                if (LocaleCompare(keyword,"output") == 0)
                  {
                    (void) fprintf(stdout,"%s",value);
                    break;
                  }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
              default:
              {
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
            }
          }
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'Q':
    case 'q':
    {
      if (LocaleCompare((char *) tag,"quantize") == 0)
        {
          QuantizeInfo
            quantize_info;

          /*
            Quantize image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          GetQuantizeInfo(&quantize_info);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"colors") == 0)
                    {
                      quantize_info.number_colors=atol(value);
                      break;
                    }
                  if (LocaleCompare(keyword,"colorspace") == 0)
                    {
                      option=ParseMagickOption(MagickColorspaceOptions,
                        MagickFalse,value);
                      if (option < 0)
                        ThrowMSLException(OptionError,
                          "UnrecognizedColorspaceType",value);
                      quantize_info.colorspace=(ColorspaceType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"dither") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      quantize_info.dither=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'M':
                case 'm':
                {
                  if (LocaleCompare(keyword,"measure") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      quantize_info.measure_error=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'T':
                case 't':
                {
                  if (LocaleCompare(keyword,"treedepth") == 0)
                    {
                      quantize_info.tree_depth=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) QuantizeImage(&quantize_info,msl_info->image[n]);
          break;
        }
      if (LocaleCompare((char *) tag,"query-font-metrics") == 0)
        {
          char
            text[MaxTextExtent];

          MagickBooleanType
            status;

          TypeMetric
            metrics;

          /*
            Query font metrics.
          */
          draw_info=CloneDrawInfo(msl_info->image_info[n],
            msl_info->draw_info[n]);
          angle=0.0;
          current=draw_info->affine;
          GetAffineMatrix(&affine);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'A':
                case 'a':
                {
                  if (LocaleCompare(keyword,"affine") == 0)
                    {
                      char
                        *p;

                      p=value;
                      draw_info->affine.sx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.rx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.ry=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.sy=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.tx=strtod(p,&p);
                      if (*p ==',')
                        p++;
                      draw_info->affine.ty=strtod(p,&p);
                      break;
                    }
                  if (LocaleCompare(keyword,"align") == 0)
                    {
                      option=ParseMagickOption(MagickAlignOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedAlignType",
                          value);
                      draw_info->align=(AlignType) option;
                      break;
                    }
                  if (LocaleCompare(keyword,"antialias") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedBooleanType",
                          value);
                      draw_info->stroke_antialias=(MagickBooleanType) option;
                      draw_info->text_antialias=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"density") == 0)
                    {
                      CloneString(&draw_info->density,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'E':
                case 'e':
                {
                  if (LocaleCompare(keyword,"encoding") == 0)
                    {
                      CloneString(&draw_info->encoding,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword, "fill") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->fill,
                        &exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"family") == 0)
                    {
                      CloneString(&draw_info->family,value);
                      break;
                    }
                  if (LocaleCompare(keyword,"font") == 0)
                    {
                      CloneString(&draw_info->font,value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  if (LocaleCompare(keyword,"gravity") == 0)
                    {
                      option=ParseMagickOption(MagickGravityOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedGravityType",
                          value);
                      draw_info->gravity=(GravityType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'P':
                case 'p':
                {
                  if (LocaleCompare(keyword,"pointsize") == 0)
                    {
                      draw_info->pointsize=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"rotate") == 0)
                    {
                      angle=atof(value);
                      affine.sx=cos(DegreesToRadians(fmod(angle,360.0)));
                      affine.rx=sin(DegreesToRadians(fmod(angle,360.0)));
                      affine.ry=(-sin(DegreesToRadians(fmod(angle,360.0))));
                      affine.sy=cos(DegreesToRadians(fmod(angle,360.0)));
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"scale") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      affine.sx=geometry_info.rho;
                      affine.sy=geometry_info.sigma;
                      break;
                    }
                  if (LocaleCompare(keyword,"skewX") == 0)
                    {
                      angle=atof(value);
                      affine.ry=cos(DegreesToRadians(fmod(angle,360.0)));
                      break;
                    }
                  if (LocaleCompare(keyword,"skewY") == 0)
                    {
                      angle=atof(value);
                      affine.rx=cos(DegreesToRadians(fmod(angle,360.0)));
                      break;
                    }
                  if (LocaleCompare(keyword,"stretch") == 0)
                    {
                      option=ParseMagickOption(MagickStretchOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedStretchType",
                          value);
                      draw_info->stretch=(StretchType) option;
                      break;
                    }
                  if (LocaleCompare(keyword, "stroke") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->stroke,
                        &exception);
                      break;
                    }
                  if (LocaleCompare(keyword,"strokewidth") == 0)
                    {
                      draw_info->stroke_width=atol(value);
                      break;
                    }
                  if (LocaleCompare(keyword,"style") == 0)
                    {
                      option=ParseMagickOption(MagickStyleOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedStyleType",
                          value);
                      draw_info->style=(StyleType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'T':
                case 't':
                {
                  if (LocaleCompare(keyword,"text") == 0)
                    {
                      CloneString(&draw_info->text,value);
                      break;
                    }
                  if (LocaleCompare(keyword,"translate") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      affine.tx=geometry_info.rho;
                      affine.ty=geometry_info.sigma;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'U':
                case 'u':
                {
                  if (LocaleCompare(keyword, "undercolor") == 0)
                    {
                      (void) QueryColorDatabase(value,&draw_info->undercolor,
                        &exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"weight") == 0)
                    {
                      draw_info->weight=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) FormatMagickString(text,MaxTextExtent,"%lux%lu%+ld%+ld",
            geometry.width,geometry.height,geometry.x,geometry.y);
          CloneString(&draw_info->geometry,text);
          draw_info->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
          draw_info->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
          draw_info->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
          draw_info->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
          draw_info->affine.tx=current.sx*affine.tx+current.ry*affine.ty+
            current.tx;
          draw_info->affine.ty=current.rx*affine.tx+current.sy*affine.ty+
            current.ty;
          status=GetTypeMetrics(msl_info->attributes[n],draw_info,&metrics);
          if (status != MagickFalse)
            {
              Image
                *image;

              image=msl_info->attributes[n];
              FormatImageProperty(image,"msl:font-metrics.pixels_per_em.x","%g",
                metrics.pixels_per_em.x);
              FormatImageProperty(image,"msl:font-metrics.pixels_per_em.y","%g",
                metrics.pixels_per_em.y);
              FormatImageProperty(image,"msl:font-metrics.ascent","%g",
                metrics.ascent);
              FormatImageProperty(image,"msl:font-metrics.descent","%g",
                metrics.descent);
              FormatImageProperty(image,"msl:font-metrics.width","%g",
                metrics.width);
              FormatImageProperty(image,"msl:font-metrics.height","%g",
                metrics.height);
              FormatImageProperty(image,"msl:font-metrics.max_advance","%g",
                metrics.max_advance);
              FormatImageProperty(image,"msl:font-metrics.bounds.x1","%g",
                metrics.bounds.x1);
              FormatImageProperty(image,"msl:font-metrics.bounds.y1","%g",
                metrics.bounds.y1);
              FormatImageProperty(image,"msl:font-metrics.bounds.x2","%g",
                metrics.bounds.x2);
              FormatImageProperty(image,"msl:font-metrics.bounds.y2","%g",
                metrics.bounds.y2);
              FormatImageProperty(image,"msl:font-metrics.origin.x","%g",
                metrics.origin.x);
              FormatImageProperty(image,"msl:font-metrics.origin.y","%g",
                metrics.origin.y);
            }
          draw_info=DestroyDrawInfo(draw_info);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'R':
    case 'r':
    {
      if (LocaleCompare((char *) tag,"raise") == 0)
        {
          MagickBooleanType
            raise;

          /*
            Raise image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          raise=MagickFalse;
          SetGeometry(msl_info->image[n],&geometry);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      geometry.height=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"raise") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedNoiseType",
                          value);
                      raise=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      geometry.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) RaiseImage(msl_info->image[n],&geometry,raise);
          break;
        }
      if (LocaleCompare((char *) tag,"read") == 0)
        {
          if (attributes == (const xmlChar **) NULL)
            break;
          for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
          {
            keyword=(const char *) attributes[i++];
            CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
              msl_info->attributes[n],(char *) attributes[i]));
            switch (*keyword)
            {
              case 'F':
              case 'f':
              {
                if (LocaleCompare(keyword,"filename") == 0)
                  {
                    Image
                      *image;

                    (void) CopyMagickString(msl_info->image_info[n]->filename,
                      value,MaxTextExtent);
                    image=ReadImage(msl_info->image_info[n],&exception);
                    CatchException(&exception);
                    if (image == (Image *) NULL)
                      continue;
                    AppendImageToList(&msl_info->image[n],image);
                    break;
                  }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
              case 'S':
              case 's':
              {
                if (LocaleCompare(keyword,"size") == 0)
                  {
                    msl_info->image_info[n]->size=AcquireString(value);
                    break;
                  }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
              default:
              {
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
            }
          }
          break;
        }
      if (LocaleCompare((char *) tag,"reduce-noise") == 0)
        {
          Image
            *paint_image;

          /*
            Reduce-noise image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"radius") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          paint_image=ReduceNoiseImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (paint_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=paint_image;
          break;
        }
      else if (LocaleCompare((char *) tag,"repage") == 0)
      {
        /* init the values */
        width=msl_info->image[n]->page.width;
        height=msl_info->image[n]->page.height;
        x=msl_info->image[n]->page.x;
        y=msl_info->image[n]->page.y;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
        break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'G':
          case 'g':
          {
          if (LocaleCompare(keyword,"geometry") == 0)
            {
              int
                flags;

              RectangleInfo
                geometry;

            flags=ParseAbsoluteGeometry(value,&geometry);
            if ((flags & WidthValue) != 0)
              {
                if ((flags & HeightValue) == 0)
                  geometry.height=geometry.width;
                width=geometry.width;
                height=geometry.height;
              }
            if ((flags & AspectValue) != 0)
              {
                if ((flags & XValue) != 0)
                  x+=geometry.x;
                if ((flags & YValue) != 0)
                  y+=geometry.y;
              }
            else
              {
                if ((flags & XValue) != 0)
                  {
                    x=geometry.x;
                    if ((width == 0) && (geometry.x > 0))
                      width=msl_info->image[n]->columns+geometry.x;
                  }
                if ((flags & YValue) != 0)
                  {
                    y=geometry.y;
                    if ((height == 0) && (geometry.y > 0))
                      height=msl_info->image[n]->rows+geometry.y;
                  }
              }
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'H':
          case 'h':
          {
          if (LocaleCompare(keyword,"height") == 0)
            {
            height = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'W':
          case 'w':
          {
          if (LocaleCompare(keyword,"width") == 0)
            {
            width = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'X':
          case 'x':
          {
          if (LocaleCompare(keyword,"x") == 0)
            {
            x = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'Y':
          case 'y':
          {
          if (LocaleCompare(keyword,"y") == 0)
            {
            y = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          default:
          {
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
        }
        }

         msl_info->image[n]->page.width=width;        
         msl_info->image[n]->page.height=height;        
         msl_info->image[n]->page.x=x;        
         msl_info->image[n]->page.y=y;        
        break;
      }
    else if (LocaleCompare((char *) tag,"resample") == 0)
    {
      double
        x_resolution,
        y_resolution;

      if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
      if (attributes == (const xmlChar **) NULL)
        break;
      x_resolution=DefaultResolution;
      y_resolution=DefaultResolution;
      for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
      {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'b':
          {
            if (LocaleCompare(keyword,"blur") == 0)
              {
                msl_info->image[n]->blur=atof(value);
                break;
              }
            ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            break;
          }
          case 'G':
          case 'g':
          {
            if (LocaleCompare(keyword,"geometry") == 0)
              {
                long
                  flags;

                flags=ParseGeometry(value,&geometry_info);
                if ((flags & SigmaValue) == 0)
                  geometry_info.sigma*=geometry_info.rho;
                x_resolution=geometry_info.rho;
                y_resolution=geometry_info.sigma;
                break;
              }
            ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            break;
          }
          case 'X':
          case 'x':
          {
            if (LocaleCompare(keyword,"x-resolution") == 0)
              {
                x_resolution=atof(value);
                break;
              }
            ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            break;
          }
          case 'Y':
          case 'y':
          {
            if (LocaleCompare(keyword,"y-resolution") == 0)
              {
                y_resolution=atof(value);
                break;
              }
            ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            break;
          }
          default:
          {
            ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            break;
          }
        }
      }
      /*
        Resample image.
      */
      {
        double
          factor;

        Image
          *resample_image;

        factor=1.0;
        if (msl_info->image[n]->units == PixelsPerCentimeterResolution)
          factor=2.54;
        width=(unsigned long) (x_resolution*msl_info->image[n]->columns/
          (factor*(msl_info->image[n]->x_resolution == 0.0 ? DefaultResolution :
          msl_info->image[n]->x_resolution))+0.5);
        height=(unsigned long) (y_resolution*msl_info->image[n]->rows/
          (factor*(msl_info->image[n]->y_resolution == 0.0 ? DefaultResolution :
          msl_info->image[n]->y_resolution))+0.5);
        resample_image=ResizeImage(msl_info->image[n],width,height,
          msl_info->image[n]->filter,msl_info->image[n]->blur,
          &msl_info->image[n]->exception);
        if (resample_image == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=resample_image;
      }
      break;
    }
      if (LocaleCompare((char *) tag,"resize") == 0)
        {
          double
            blur;

          FilterTypes
            filter;

          Image
            *resize_image;

          /*
            Resize image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          filter=UndefinedFilter;
          blur=1.0;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword,"filter") == 0)
                    {
                      option=ParseMagickOption(MagickFilterOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedNoiseType",
                          value);
                      filter=(FilterTypes) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseRegionGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      geometry.height=(unsigned long) atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"support") == 0)
                    {
                      blur=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      geometry.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          resize_image=ResizeImage(msl_info->image[n],geometry.width,
            geometry.height,filter,blur,&msl_info->image[n]->exception);
          if (resize_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=resize_image;
          break;
        }
      if (LocaleCompare((char *) tag,"roll") == 0)
        {
          Image
            *roll_image;

          /*
            Roll image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          SetGeometry(msl_info->image[n],&geometry);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParsePageGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      if ((flags & HeightValue) == 0)
                        geometry.height=geometry.width;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry.x=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry.y=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          roll_image=RollImage(msl_info->image[n],geometry.x,geometry.y,
            &msl_info->image[n]->exception);
          if (roll_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=roll_image;
          break;
        }
      else if (LocaleCompare((char *) tag,"roll") == 0)
      {
        /* init the values */
        width=msl_info->image[n]->columns;
        height=msl_info->image[n]->rows;
        x = y = 0;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
        break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'G':
          case 'g':
          {
          if (LocaleCompare(keyword,"geometry") == 0)
            {
            (void) ParseMetaGeometry(value,&x,&y,&width,&height);
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'X':
          case 'x':
          {
          if (LocaleCompare(keyword,"x") == 0)
            {
            x = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'Y':
          case 'y':
          {
          if (LocaleCompare(keyword,"y") == 0)
            {
            y = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          default:
          {
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
        }
        }

        /*
          process image.
        */
        {
        Image
          *newImage;

        newImage=RollImage(msl_info->image[n], x, y, &msl_info->image[n]->exception);
        if (newImage == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=newImage;
        }

        break;
      }
      if (LocaleCompare((char *) tag,"rotate") == 0)
        {
          Image
            *rotate_image;

          /*
            Rotate image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"degrees") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          rotate_image=RotateImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (rotate_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=rotate_image;
          break;
        }
      else if (LocaleCompare((char *) tag,"rotate") == 0)
      {
        /* init the values */
        double  degrees = 0;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
        break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'D':
          case 'd':
          {
          if (LocaleCompare(keyword,"degrees") == 0)
            {
            degrees = atof( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          default:
          {
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
        }
        }

        /*
          process image.
        */
        {
        Image
          *newImage;

        newImage=RotateImage(msl_info->image[n], degrees, &msl_info->image[n]->exception);
        if (newImage == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=newImage;
        }

        break;
      }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'S':
    case 's':
    {
      if (LocaleCompare((char *) tag,"sample") == 0)
        {
          Image
            *sample_image;

          /*
            Sample image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseRegionGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      geometry.height=(unsigned long) atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      geometry.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          sample_image=SampleImage(msl_info->image[n],geometry.width,
            geometry.height,&msl_info->image[n]->exception);
          if (sample_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=sample_image;
          break;
        }
      if (LocaleCompare((char *) tag,"scale") == 0)
        {
          Image
            *scale_image;

          /*
            Scale image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseRegionGeometry(msl_info->image[n],value,
                        &geometry,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'H':
                case 'h':
                {
                  if (LocaleCompare(keyword,"height") == 0)
                    {
                      geometry.height=(unsigned long) atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'W':
                case 'w':
                {
                  if (LocaleCompare(keyword,"width") == 0)
                    {
                      geometry.width=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          scale_image=ScaleImage(msl_info->image[n],geometry.width,
            geometry.height,&msl_info->image[n]->exception);
          if (scale_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=scale_image;
          break;
        }
      if (LocaleCompare((char *) tag,"segment") == 0)
        {
          ColorspaceType
            colorspace;

          MagickBooleanType
            verbose;
            
          /*
            Segment image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          geometry_info.rho=1.0;
          geometry_info.sigma=1.5;
          colorspace=RGBColorspace;
          verbose=MagickFalse;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'C':
                case 'c':
                {
                  if (LocaleCompare(keyword,"cluster-threshold") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  if (LocaleCompare(keyword,"colorspace") == 0)
                    {
                      option=ParseMagickOption(MagickColorspaceOptions,
                        MagickFalse,value);
                      if (option < 0)
                        ThrowMSLException(OptionError,
                          "UnrecognizedColorspaceType",value);
                      colorspace=(ColorspaceType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.5;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"smoothing-threshold") == 0)
                    {
                      geometry_info.sigma=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) SegmentImage(msl_info->image[n],colorspace,verbose,
            geometry_info.rho,geometry_info.sigma);
          break;
        }
      else if (LocaleCompare((char *) tag, "set") == 0)
      {
        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }

        if (attributes == (const xmlChar **) NULL)
          break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
          keyword=(const char *) attributes[i++];
          CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
            msl_info->attributes[n],(char *) attributes[i]));
          switch (*keyword)
          {
            case 'B':
            case 'b':
            {
              if (LocaleCompare(keyword,"background") == 0)
              {
                (void) QueryColorDatabase(value,
                  &msl_info->image_info[n]->background_color,&exception);
                break;
              }
              else if (LocaleCompare(keyword,"bordercolor") == 0)
              {
                (void) QueryColorDatabase(value,
                  &msl_info->image_info[n]->border_color,&exception);
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'C':
            case 'c':
            {
              if (LocaleCompare(keyword,"clip-mask") == 0)
              {
                for (j=0; j<msl_info->n;j++)
                {
                  const char *
                    theAttr = GetImageProperty(msl_info->attributes[j], "id");
                  if (theAttr && LocaleCompare(theAttr, value) == 0)
                  {
                    SetImageMask( msl_info->image[n], msl_info->image[j] );
                    break;
                  }
                }
                break;
              }
              if (LocaleCompare(keyword,"clip-path") == 0)
              {
                for (j=0; j<msl_info->n;j++)
                {
                  const char *
                    theAttr = GetImageProperty(msl_info->attributes[j], "id");
                  if (theAttr && LocaleCompare(theAttr, value) == 0)
                  {
                    SetImageClipMask( msl_info->image[n], msl_info->image[j] );
                    break;
                  }
                }
                break;
              }
              else if (LocaleCompare(keyword, "colorspace") == 0)
              {
                if (LocaleCompare(value, "CMYK") == 0)
                  SetImageType(msl_info->image[n],ColorSeparationType );
                else if (LocaleCompare(value, "Gray") == 0)
                  SetImageType(msl_info->image[n],GrayscaleType );
                else if (LocaleCompare(value, "RGB") == 0)
                  SetImageType(msl_info->image[n],TrueColorType );
                else
                  ThrowMSLException(OptionError,"Unrecognized colorspace",
                    keyword);
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'D':
            case 'd':
            {
              if (LocaleCompare(keyword, "density") == 0)
              {
                GeometryInfo
                  geometry_info;

                MagickStatusType
                  flags;

                (void) CloneString(&msl_info->image_info[n]->density,
                  (char *) NULL);
                (void) CloneString(&msl_info->image_info[n]->density,value);
                (void) CloneString(&msl_info->draw_info[n]->density,
                  msl_info->image_info[n]->density);
                flags=ParseGeometry(msl_info->image_info[n]->density,
                  &geometry_info);
                msl_info->image[n]->x_resolution=geometry_info.rho;
                msl_info->image[n]->y_resolution=geometry_info.sigma;
                if ((flags & SigmaValue) == 0)
                  msl_info->image[n]->y_resolution=
                    msl_info->image[n]->x_resolution;
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'M':
            case 'm':
            {
              if (LocaleCompare(keyword, "magick") == 0)
              {
                (void) CopyMagickString(msl_info->image_info[n]->magick,
                  value,MaxTextExtent);
                break;
              }
              else if (LocaleCompare(keyword,"mattecolor") == 0)
              {
                (void) QueryColorDatabase(value,
                  &msl_info->image_info[n]->matte_color,&exception);
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'O':
            case 'o':
            {
              if (LocaleCompare(keyword, "opacity") == 0)
              {
                long  opac = OpaqueOpacity,
                  len = (long) strlen( value );

                if (value[len-1] == '%') {
                  char  tmp[100];
                  (void) CopyMagickString(tmp,value,len);
                  opac = atol( tmp );
                  opac = (int)(QuantumRange * ((float)opac/100));
                } else
                  opac = atol( value );
                (void) SetImageOpacity( msl_info->image[n], (Quantum) opac );
                break;
              }

              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'P':
            case 'p':
            {
              if (LocaleCompare(keyword, "page") == 0)
              {
                char
                  page[MaxTextExtent];

                const char
                  *image_option;

                MagickStatusType
                  flags;

                RectangleInfo
                  geometry;

                (void) ResetMagickMemory(&geometry,0,sizeof(geometry));
                image_option=GetImageOption(msl_info->image_info[n],"page");
                if (image_option != (const char *) NULL)
                  flags=ParseAbsoluteGeometry(image_option,&geometry);
                flags=ParseAbsoluteGeometry(value,&geometry);
                (void) FormatMagickString(page,MaxTextExtent,"%lux%lu",
                  geometry.width,geometry.height);
                if (((flags & XValue) != 0) || ((flags & YValue) != 0))
                  (void) FormatMagickString(page,MaxTextExtent,"%lux%lu%+ld%+ld",
                    geometry.width,geometry.height,geometry.x,geometry.y);
                (void) SetImageOption(msl_info->image_info[n],keyword,page);
                msl_info->image_info[n]->page=GetPageGeometry(page);
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            default:
            {
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
          }
        }
        break;
      }
      if (LocaleCompare((char *) tag,"shade") == 0)
        {
          Image
            *shade_image;

          MagickBooleanType
            gray;

          /*
            Shade image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          gray=MagickFalse;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'A':
                case 'a':
                {
                  if (LocaleCompare(keyword,"azimuth") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'E':
                case 'e':
                {
                  if (LocaleCompare(keyword,"elevation") == 0)
                    {
                      geometry_info.sigma=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  if (LocaleCompare(keyword,"gray") == 0)
                    {
                      option=ParseMagickOption(MagickBooleanOptions,MagickFalse,
                        value);
                      if (option < 0)
                        ThrowMSLException(OptionError,"UnrecognizedNoiseType",
                          value);
                      gray=(MagickBooleanType) option;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          shade_image=ShadeImage(msl_info->image[n],gray,geometry_info.rho,
            geometry_info.sigma,&msl_info->image[n]->exception);
          if (shade_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=shade_image;
          break;
        }
      if (LocaleCompare((char *) tag,"shadow") == 0)
        {
          Image
            *shadow_image;

          /*
            Shear image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'O':
                case 'o':
                {
                  if (LocaleCompare(keyword,"opacity") == 0)
                    {
                      geometry_info.rho=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'S':
                case 's':
                {
                  if (LocaleCompare(keyword,"sigma") == 0)
                    {
                      geometry_info.sigma=atol(value);
                      break;
                    }
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry_info.xi=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry_info.psi=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          shadow_image=ShadowImage(msl_info->image[n],geometry_info.rho,
            geometry_info.sigma,(long) (geometry_info.xi+0.5),(long)
            (geometry_info.psi+0.5),&msl_info->image[n]->exception);
          if (shadow_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=shadow_image;
          break;
        }
      if (LocaleCompare((char *) tag,"sharpen") == 0)
      {
        double  radius = 0.0,
            sigma = 1.0;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        /*
        NOTE: sharpen can have no attributes, since we use all the defaults!
        */
        if (attributes != (const xmlChar **) NULL)
        {
          for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
          {
          keyword=(const char *) attributes[i++];
          CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
            msl_info->attributes[n],(char *) attributes[i]));
          switch (*keyword)
          {
            case 'R':
            case 'r':
            {
              if (LocaleCompare(keyword, "radius") == 0)
              {
                radius = atof( value );
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            case 'S':
            case 's':
            {
              if (LocaleCompare(keyword,"sigma") == 0)
              {
                sigma = atol( value );
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            default:
            {
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
          }
          }
        }

        /*
          sharpen image.
        */
        {
        Image
          *newImage;

        newImage=SharpenImage(msl_info->image[n],radius,sigma,&msl_info->image[n]->exception);
        if (newImage == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=newImage;
        break;
        }
      }
      else if (LocaleCompare((char *) tag,"shave") == 0)
      {
        /* init the values */
        width = height = 0;
        x = y = 0;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
        break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'G':
          case 'g':
          {
          if (LocaleCompare(keyword,"geometry") == 0)
            {
            (void) ParseMetaGeometry(value,&x,&y,&width,&height);
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'H':
          case 'h':
          {
          if (LocaleCompare(keyword,"height") == 0)
            {
            height = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          case 'W':
          case 'w':
          {
          if (LocaleCompare(keyword,"width") == 0)
            {
            width = atol( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          default:
          {
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
        }
        }

        /*
          process image.
        */
        {
        Image
          *newImage;
        RectangleInfo
          rectInfo;

        rectInfo.height = height;
        rectInfo.width = width;
        rectInfo.x = x;
        rectInfo.y = y;


        newImage=ShaveImage(msl_info->image[n], &rectInfo,
          &msl_info->image[n]->exception);
        if (newImage == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=newImage;
        }

        break;
      }
      if (LocaleCompare((char *) tag,"shear") == 0)
        {
          Image
            *shear_image;

          /*
            Shear image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'F':
                case 'f':
                {
                  if (LocaleCompare(keyword, "fill") == 0)
                    {
                      (void) QueryColorDatabase(value,
                        &msl_info->image[n]->background_color,&exception);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'X':
                case 'x':
                {
                  if (LocaleCompare(keyword,"x") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'Y':
                case 'y':
                {
                  if (LocaleCompare(keyword,"y") == 0)
                    {
                      geometry_info.sigma=atol(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          shear_image=ShearImage(msl_info->image[n],geometry_info.rho,
            geometry_info.sigma,&msl_info->image[n]->exception);
          if (shear_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=shear_image;
          break;
        }
      if (LocaleCompare((char *) tag,"signature") == 0)
        {
          /*
            Signature image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) SignatureImage(msl_info->image[n]);
          break;
        }
      if (LocaleCompare((char *) tag,"solarize") == 0)
        {
          /*
            Solarize image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          geometry_info.rho=QuantumRange/2.0;
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'T':
                case 't':
                {
                  if (LocaleCompare(keyword,"threshold") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) SolarizeImage(msl_info->image[n],geometry_info.rho);
          break;
        }
      if (LocaleCompare((char *) tag,"spread") == 0)
        {
          Image
            *spread_image;

          /*
            Spread image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'R':
                case 'r':
                {
                  if (LocaleCompare(keyword,"radius") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          spread_image=SpreadImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (spread_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=spread_image;
          break;
        }
      else if (LocaleCompare((char *) tag,"stegano") == 0)
      {
        Image *
          watermark = (Image*)NULL;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
        break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'I':
          case 'i':
          {
          if (LocaleCompare(keyword,"image") == 0)
            {
            for (j=0; j<msl_info->n;j++)
            {
              const char *
                theAttr = GetImageProperty(msl_info->attributes[j], "id");
              if (theAttr && LocaleCompare(theAttr, value) == 0)
              {
                watermark = msl_info->image[j];
                break;
              }
            }
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          default:
          {
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
        }
        }

        /*
          process image.
        */
        if ( watermark != (Image*) NULL )
        {
        Image
          *newImage;

        newImage=SteganoImage(msl_info->image[n], watermark, &msl_info->image[n]->exception);
        if (newImage == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=newImage;
        break;
        } else
          ThrowMSLException(OptionError,"MissingWatermarkImage",keyword);
      }
      else if (LocaleCompare((char *) tag,"stereo") == 0)
      {
        Image *
          stereoImage = (Image*)NULL;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
        break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'I':
          case 'i':
          {
          if (LocaleCompare(keyword,"image") == 0)
            {
            for (j=0; j<msl_info->n;j++)
            {
              const char *
                theAttr = GetImageProperty(msl_info->attributes[j], "id");
              if (theAttr && LocaleCompare(theAttr, value) == 0)
              {
                stereoImage = msl_info->image[j];
                break;
              }
            }
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          default:
          {
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
        }
        }

        /*
          process image.
        */
        if ( stereoImage != (Image*) NULL )
        {
        Image
          *newImage;

        newImage=StereoImage(msl_info->image[n], stereoImage, &msl_info->image[n]->exception);
        if (newImage == (Image *) NULL)
          break;
        msl_info->image[n]=DestroyImage(msl_info->image[n]);
        msl_info->image[n]=newImage;
        break;
        } else
          ThrowMSLException(OptionError,"Missing stereo image",keyword);
      }
      if (LocaleCompare((char *) tag,"swap") == 0)
        {
          Image
            *p,
            *q,
            *swap;

          long
            index,
            swap_index;

          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          index=(-1);
          swap_index=(-2);
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"indexes") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      index=(long) geometry_info.rho;
                      if ((flags & SigmaValue) == 0)
                        swap_index=(long) geometry_info.sigma;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          /*
            Swap images.
          */
          p=GetImageFromList(msl_info->image[n],index);
          q=GetImageFromList(msl_info->image[n],swap_index);
          if ((p == (Image *) NULL) || (q == (Image *) NULL))
            {
              ThrowMSLException(OptionError,"NoSuchImage",(char *) tag);
              break;
            }
          swap=CloneImage(p,0,0,MagickTrue,&p->exception);
          ReplaceImageInList(&p,CloneImage(q,0,0,MagickTrue,&q->exception));
          ReplaceImageInList(&q,swap);
          msl_info->image[n]=GetFirstImageInList(q);
          break;
        }
      if (LocaleCompare((char *) tag,"swirl") == 0)
        {
          Image
            *swirl_image;

          /*
            Swirl image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'D':
                case 'd':
                {
                  if (LocaleCompare(keyword,"degrees") == 0)
                    {
                      geometry_info.rho=atof(value);
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                case 'G':
                case 'g':
                {
                  if (LocaleCompare(keyword,"geometry") == 0)
                    {
                      flags=ParseGeometry(value,&geometry_info);
                      if ((flags & SigmaValue) == 0)
                        geometry_info.sigma=1.0;
                      break;
                    }
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          swirl_image=SwirlImage(msl_info->image[n],geometry_info.rho,
            &msl_info->image[n]->exception);
          if (swirl_image == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=swirl_image;
          break;
        }
      if (LocaleCompare((char *) tag,"sync") == 0)
        {
          /*
            Sync image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) SyncImage(msl_info->image[n]);
          break;
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'T':
    case 't':
    {
      if (LocaleCompare((char *) tag,"map") == 0)
        {
          Image
            *texture_image;

          /*
            Texture image.
          */
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          texture_image=NewImageList();
          if (attributes != (const xmlChar **) NULL)
            for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
            {
              keyword=(const char *) attributes[i++];
              attribute=InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]);
              CloneString(&value,attribute);
              switch (*keyword)
              {
                case 'I':
                case 'i':
                {
                  if (LocaleCompare(keyword,"image") == 0)
                    for (j=0; j < msl_info->n; j++)
                    {
                      const char
                        *attribute;
    
                      attribute=GetImageProperty(msl_info->attributes[j],"id");
                      if ((attribute != (const char *) NULL)  &&
                          (LocaleCompare(attribute,value) == 0))
                        {
                          texture_image=CloneImage(msl_info->image[j],0,0,
                            MagickFalse,&exception);
                          break;
                        }
                    }
                  break;
                }
                default:
                {
                  ThrowMSLException(OptionError,"UnrecognizedAttribute",
                    keyword);
                  break;
                }
              }
            }
          (void) TextureImage(msl_info->image[n],texture_image);
          texture_image=DestroyImage(texture_image);
          break;
        }
      else if (LocaleCompare((char *) tag,"threshold") == 0)
      {
        /* init the values */
        double  threshold = 0;

        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
        break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
        keyword=(const char *) attributes[i++];
        CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
          msl_info->attributes[n],(char *) attributes[i]));
        switch (*keyword)
        {
          case 'T':
          case 't':
          {
          if (LocaleCompare(keyword,"threshold") == 0)
            {
            threshold = atof( value );
            break;
            }
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
          default:
          {
          ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
          break;
          }
        }
        }

        /*
          process image.
        */
        {
        BilevelImageChannel(msl_info->image[n],
          (ChannelType) ((long) (AllChannels &~ (long) OpacityChannel)),
          threshold);
        break;
        }
      }
      else if (LocaleCompare((char *) tag, "transparent") == 0)
      {
        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }
        if (attributes == (const xmlChar **) NULL)
          break;
        for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
        {
          keyword=(const char *) attributes[i++];
          CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
                msl_info->attributes[n],(char *) attributes[i]));
          switch (*keyword)
          {
            case 'C':
            case 'c':
            {
              if (LocaleCompare(keyword,"color") == 0)
              {
                MagickPixelPacket
                  target;

                (void) QueryMagickColor(value,&target,&exception);
                (void) TransparentPaintImage(msl_info->image[n],&target,
                  TransparentOpacity,MagickFalse);
                break;
              }
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              break;
            }
            default:
            {
              ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
            break;
            }
          }
        }
        break;
      }
      else if (LocaleCompare((char *) tag, "trim") == 0)
      {
        if (msl_info->image[n] == (Image *) NULL)
        {
          ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
          break;
        }

        /* no attributes here */

        /* process the image */
        {
          Image
            *newImage;
          RectangleInfo
            rectInfo;

          /* all zeros on a crop == trim edges! */
          rectInfo.height = rectInfo.width = 0;
          rectInfo.x =  rectInfo.y = 0;

          newImage=CropImage(msl_info->image[n],&rectInfo, &msl_info->image[n]->exception);
          if (newImage == (Image *) NULL)
            break;
          msl_info->image[n]=DestroyImage(msl_info->image[n]);
          msl_info->image[n]=newImage;
          break;
        }
      }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    case 'W':
    case 'w':
    {
      if (LocaleCompare((char *) tag,"write") == 0)
        {
          if (msl_info->image[n] == (Image *) NULL)
            {
              ThrowMSLException(OptionError,"NoImagesDefined",(char *) tag);
              break;
            }
          if (attributes == (const xmlChar **) NULL)
            break;
          for (i=0; (attributes[i] != (const xmlChar *) NULL); i++)
          {
            keyword=(const char *) attributes[i++];
            CloneString(&value,InterpretImageProperties(msl_info->image_info[n],
              msl_info->attributes[n],(char *) attributes[i]));
            switch (*keyword)
            {
              case 'F':
              case 'f':
              {
                if (LocaleCompare(keyword,"filename") == 0)
                  {
                    (void) CopyMagickString(msl_info->image[n]->filename,value,
                      MaxTextExtent);
                    break;
                  }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              }
              case 'Q':
              case 'q':
              {
                if (LocaleCompare(keyword,"quality") == 0)
                  {
                    msl_info->image_info[n]->quality=atol(value);
                    msl_info->image[n]->quality=
                      msl_info->image_info[n]->quality;
                    break;
                  }
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
              }
              default:
              {
                ThrowMSLException(OptionError,"UnrecognizedAttribute",keyword);
                break;
              }
            }
          }

          /* process */
          {
            (void) WriteImage(msl_info->image_info[n], msl_info->image[n]);
            break;
          }
        }
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
    }
    default:
    {
      ThrowMSLException(OptionError,"UnrecognizedElement",(const char *) tag);
      break;
    }
  }
  if ( value != NULL )
    value=DestroyString(value);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  )");
}

static void MSLEndElement(void *context,const xmlChar *tag)
{
  long
    n;

  MSLInfo
    *msl_info;

  /*
    Called when the end of an element has been detected.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.endElement(%s)",
    tag);
  msl_info=(MSLInfo *) context;
  n=msl_info->n;
  switch (*tag)
  {
    case 'C':
    case 'c':
    {
      if (LocaleCompare((char *) tag,"comment") == 0 )
        {
          (void) DeleteImageProperty(msl_info->image[n],"comment");
          if (msl_info->content == (char *) NULL)
            break;
          StripString(msl_info->content);
          (void) SetImageProperty(msl_info->image[n],"comment",
            msl_info->content);
          break;
        }
      break;
    }
    case 'G':
    case 'g':
    {
      if (LocaleCompare((char *) tag, "group") == 0 )
      {
        if (msl_info->group_info[msl_info->number_groups-1].numImages > 0 )
        {
          long  i = (long)
            (msl_info->group_info[msl_info->number_groups-1].numImages);
          while ( i-- )
          {
            if (msl_info->image[msl_info->n] != (Image *) NULL)
              msl_info->image[msl_info->n]=DestroyImage(msl_info->image[msl_info->n]);
            msl_info->attributes[msl_info->n]=DestroyImage(msl_info->attributes[msl_info->n]);
            msl_info->image_info[msl_info->n]=DestroyImageInfo(msl_info->image_info[msl_info->n]);
            msl_info->n--;
          }
        }
        msl_info->number_groups--;
      }
      break;
    }
    case 'I':
    case 'i':
    {
      if (LocaleCompare((char *) tag, "image") == 0)
        MSLPopImage(msl_info);
       break;
    }
    case 'L':
    case 'l':
    {
      if (LocaleCompare((char *) tag,"label") == 0 )
        {
          (void) DeleteImageProperty(msl_info->image[n],"label");
          if (msl_info->content == (char *) NULL)
            break;
          StripString(msl_info->content);
          (void) SetImageProperty(msl_info->image[n],"label",
            msl_info->content);
          break;
        }
      break;
    }
    case 'M':
    case 'm':
    {
      if (LocaleCompare((char *) tag, "msl") == 0 )
      {
        /*
          This our base element.
            at the moment we don't do anything special
            but someday we might!
        */
      }
      break;
    }
    default:
      break;
  }
  if (msl_info->content != (char *) NULL)
    msl_info->content=DestroyString(msl_info->content);
}

static void MSLCharacters(void *context,const xmlChar *c,int length)
{
  MSLInfo
    *msl_info;

  register char
    *p;

  register long
    i;

  /*
    Receiving some characters from the parser.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.characters(%s,%d)",c,length);
  msl_info=(MSLInfo *) context;
  if (msl_info->content != (char *) NULL)
    msl_info->content=(char *) ResizeQuantumMemory(msl_info->content,
      strlen(msl_info->content)+length+MaxTextExtent,
      sizeof(*msl_info->content));
  else
    {
      msl_info->content=(char *) NULL;
      if (~length >= MaxTextExtent)
        msl_info->content=(char *) AcquireQuantumMemory(length+MaxTextExtent,
          sizeof(*msl_info->content));
      if (msl_info->content != (char *) NULL)
        *msl_info->content='\0';
    }
  if (msl_info->content == (char *) NULL)
    return;
  p=msl_info->content+strlen(msl_info->content);
  for (i=0; i < length; i++)
    *p++=c[i];
  *p='\0';
}

static void MSLReference(void *context,const xmlChar *name)
{
  MSLInfo
    *msl_info;

  xmlParserCtxtPtr
    parser;

  /*
    Called when an entity reference is detected.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.reference(%s)",name);
  msl_info=(MSLInfo *) context;
  parser=msl_info->parser;
  if (*name == '#')
    (void) xmlAddChild(parser->node,xmlNewCharRef(msl_info->document,name));
  else
    (void) xmlAddChild(parser->node,xmlNewReference(msl_info->document,name));
}

static void MSLIgnorableWhitespace(void *context,const xmlChar *c,int length)
{
  MSLInfo
    *msl_info;

  /*
    Receiving some ignorable whitespaces from the parser.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.ignorableWhitespace(%.30s, %d)",c,length);
  msl_info=(MSLInfo *) context;
}

static void MSLProcessingInstructions(void *context,const xmlChar *target,
  const xmlChar *data)
{
  MSLInfo
    *msl_info;

  /*
    A processing instruction has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.processingInstruction(%s, %s)",
    target,data);
  msl_info=(MSLInfo *) context;
}

static void MSLComment(void *context,const xmlChar *value)
{
  MSLInfo
    *msl_info;

  /*
    A comment has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.comment(%s)",value);
  msl_info=(MSLInfo *) context;
}

static void MSLWarning(void *context,const char *format,...)
{
  char
    *message,
    reason[MaxTextExtent];

  MSLInfo
    *msl_info;

  va_list
    operands;

  /**
    Display and format a warning messages, gives file, line, position and
    extra parameters.
  */
  va_start(operands,format);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.warning: ");
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),format,operands);
  msl_info=(MSLInfo *) context;
#if !defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsprintf(reason,format,operands);
#else
  (void) vsnprintf(reason,MaxTextExtent,format,operands);
#endif
  message=GetExceptionMessage(errno);
  ThrowMSLException(CoderError,reason,message);
  message=DestroyString(message);
  va_end(operands);
}

static void MSLError(void *context,const char *format,...)
{
  char
    reason[MaxTextExtent];

  MSLInfo
    *msl_info;

  va_list
    operands;

  /*
    Display and format a error formats, gives file, line, position and
    extra parameters.
  */
  va_start(operands,format);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  SAX.error: ");
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),format,operands);
  msl_info=(MSLInfo *) context;
#if !defined(MAGICKCORE_HAVE_VSNPRINTF)
  (void) vsprintf(reason,format,operands);
#else
  (void) vsnprintf(reason,MaxTextExtent,format,operands);
#endif
  ThrowMSLException(DelegateFatalError,reason,"SAX error");
  va_end(operands);
}

static void MSLCDataBlock(void *context,const xmlChar *value,int length)
{
  MSLInfo
    *msl_info;

   xmlNodePtr
     child;

  xmlParserCtxtPtr
    parser;

  /*
    Called when a pcdata block has been parsed.
  */
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
    "  SAX.pcdata(%s, %d)",value,length);
  msl_info=(MSLInfo *) context;
  parser=msl_info->parser;
  child=xmlGetLastChild(parser->node);
  if ((child != (xmlNodePtr) NULL) && (child->type == XML_CDATA_SECTION_NODE))
    {
      xmlTextConcat(child,value,length);
      return;
    }
  (void) xmlAddChild(parser->node,xmlNewCDataBlock(parser->myDoc,value,length));
}

static void MSLExternalSubset(void *context,const xmlChar *name,
  const xmlChar *external_id,const xmlChar *system_id)
{
  MSLInfo
    *msl_info;

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
    "  SAX.externalSubset(%s %s %s)",name,
    (external_id != (const xmlChar *) NULL ? (char *) external_id : " "),
    (system_id != (const xmlChar *) NULL ? (char *) system_id : " "));
  msl_info=(MSLInfo *) context;
  parser=msl_info->parser;
  if (((external_id == NULL) && (system_id == NULL)) ||
      ((parser->validate == 0) || (parser->wellFormed == 0) ||
      (msl_info->document == 0)))
    return;
  input=MSLResolveEntity(context,external_id,system_id);
  if (input == NULL)
    return;
  (void) xmlNewDtd(msl_info->document,name,external_id,system_id);
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

static MagickBooleanType ProcessMSLScript(const ImageInfo *image_info,Image **image,
  ExceptionInfo *exception)
{
  xmlSAXHandler
    SAXModules =
    {
      MSLInternalSubset,
      MSLIsStandalone,
      MSLHasInternalSubset,
      MSLHasExternalSubset,
      MSLResolveEntity,
      MSLGetEntity,
      MSLEntityDeclaration,
      MSLNotationDeclaration,
      MSLAttributeDeclaration,
      MSLElementDeclaration,
      MSLUnparsedEntityDeclaration,
      MSLSetDocumentLocator,
      MSLStartDocument,
      MSLEndDocument,
      MSLStartElement,
      MSLEndElement,
      MSLReference,
      MSLCharacters,
      MSLIgnorableWhitespace,
      MSLProcessingInstructions,
      MSLComment,
      MSLWarning,
      MSLError,
      MSLError,
      MSLGetParameterEntity,
      MSLCDataBlock,
      MSLExternalSubset
    };

  char
    message[MaxTextExtent];

  Image
    *msl_image;

  int
    status;

  long
    n;

  MSLInfo
    msl_info;

  xmlSAXHandlerPtr
    SAXHandler;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image_info->filename);
  assert(image != (Image **) NULL);
  msl_image=AcquireImage(image_info);
  status=OpenBlob(image_info,msl_image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        msl_image->filename);
      msl_image=DestroyImageList(msl_image);
      return(MagickFalse);
    }
  msl_image->columns=1;
  msl_image->rows=1;
  /*
    Parse MSL file.
  */
  (void) ResetMagickMemory(&msl_info,0,sizeof(msl_info));
  msl_info.exception=exception;
  msl_info.image_info=(ImageInfo **) AcquireMagickMemory(
    sizeof(*msl_info.image_info));
  msl_info.draw_info=(DrawInfo **) AcquireMagickMemory(
    sizeof(*msl_info.draw_info));
  /* top of the stack is the MSL file itself */
  msl_info.image=(Image **) AcquireMagickMemory(sizeof(*msl_info.image));
  msl_info.attributes=(Image **) AcquireMagickMemory(
    sizeof(*msl_info.attributes));
  msl_info.group_info=(MSLGroupInfo *) AcquireMagickMemory(
    sizeof(*msl_info.group_info));
  if ((msl_info.image_info == (ImageInfo **) NULL) ||
      (msl_info.image == (Image **) NULL) ||
      (msl_info.attributes == (Image **) NULL) ||
      (msl_info.group_info == (MSLGroupInfo *) NULL))
    ThrowFatalException(ResourceLimitFatalError,
      "UnableToInterpretMSLImage");
  *msl_info.image_info=CloneImageInfo(image_info);
  *msl_info.draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  *msl_info.attributes=AcquireImage(image_info);
  msl_info.group_info[0].numImages=0;
  /* the first slot is used to point to the MSL file image */
  *msl_info.image=msl_image;
  if (*image != (Image *) NULL)
    MSLPushImage(&msl_info,*image);
  (void) xmlSubstituteEntitiesDefault(1);
  SAXHandler=(&SAXModules);
  msl_info.parser=xmlCreatePushParserCtxt(SAXHandler,&msl_info,(char *) NULL,0,
    msl_image->filename);
  while (ReadBlobString(msl_image,message) != (char *) NULL)
  {
    n=(long) strlen(message);
    if (n == 0)
      continue;
    status=xmlParseChunk(msl_info.parser,message,(int) n,MagickFalse);
    if (status != 0)
      break;
    (void) xmlParseChunk(msl_info.parser," ",1,MagickFalse);
    if (msl_info.exception->severity >= ErrorException)
      break;
  }
  if (msl_info.exception->severity == UndefinedException)
    (void) xmlParseChunk(msl_info.parser," ",1,MagickTrue);
  xmlFreeParserCtxt(msl_info.parser);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"end SAX");
  xmlCleanupParser();
  msl_info.group_info=(MSLGroupInfo *) RelinquishMagickMemory(
    msl_info.group_info);
  if (*image == (Image *) NULL)
    *image=(*msl_info.image);
  return((MagickBooleanType) ((*msl_info.image)->exception.severity == UndefinedException));
}

static Image *ReadMSLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=(Image *) NULL;
  (void) ProcessMSLScript(image_info,&image,exception);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M S L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMSLImage() adds attributes for the MSL image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMSLImage method is:
%
%      unsigned long RegisterMSLImage(void)
%
*/
ModuleExport unsigned long RegisterMSLImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("MSL");
#if defined(MAGICKCORE_XML_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadMSLImage;
  entry->encoder=(EncodeImageHandler *) WriteMSLImage;
#endif
  entry->description=ConstantString("Magick Scripting Language");
  entry->module=ConstantString("MSL");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M S L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMSLImage() removes format registrations made by the
%  MSL module from the list of supported formats.
%
%  The format of the UnregisterMSLImage method is:
%
%      UnregisterMSLImage(void)
%
*/
ModuleExport void UnregisterMSLImage(void)
{
  (void) UnregisterMagickInfo("MSL");
}

#if defined(MAGICKCORE_XML_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M S L I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMSLImage() writes an image to a file in MVG image format.
%
%  The format of the WriteMSLImage method is:
%
%      MagickBooleanType WriteMSLImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteMSLImage(const ImageInfo *image_info,Image *image)
{
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) ReferenceImage(image);
  (void) ProcessMSLScript(image_info,&image,&image->exception);
  return(MagickTrue);
}
#endif

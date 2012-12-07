/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             X   X  M   M  L                                 %
%                              X X   MM MM  L                                 %
%                               X    M M M  L                                 %
%                              X X   M   M  L                                 %
%                             X   X  M   M  LLLLL                             %
%                                                                             %
%                         TTTTT  RRRR   EEEEE  EEEEE                          %
%                           T    R   R  E      E                              %
%                           T    RRRR   EEE    EEE                            %
%                           T    R R    E      E                              %
%                           T    R  R   EEEEE  EEEEE                          %
%                                                                             %
%                                                                             %
%                              XML Tree Methods                               %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               December 2004                                 %
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
%  This module implements the standard handy xml-tree methods for storing and
%  retrieving nodes and attributes from an XML string.
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token-private.h"
#include "magick/xml-tree.h"
#include "magick/utility.h"

/*
  Define declarations.
*/
#define NumberPredefinedEntities  10
#define XMLWhitespace "\t\r\n "

/*
  Typedef declarations.
*/
struct _XMLTreeInfo
{
  char
    *tag,
    **attributes,
    *content;

  size_t
    offset;

  XMLTreeInfo
    *parent,
    *next,
    *sibling,
    *ordered,
    *child;

  MagickBooleanType
    debug;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

typedef struct _XMLTreeRoot
  XMLTreeRoot;

struct _XMLTreeRoot
{
  struct _XMLTreeInfo
    root;

  XMLTreeInfo
    *node;

  MagickBooleanType
    standalone;

  char
    ***processing_instructions,
    **entities,
    ***attributes;

  MagickBooleanType
    debug;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

/*
  Global declarations.
*/
static char
  *sentinel[] = { (char *) NULL };

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A d d C h i l d T o X M L T r e e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AddChildToXMLTree() adds a child tag at an offset relative to the start of
%  the parent tag's character content.  Return the child tag.
%
%  The format of the AddChildToXMLTree method is:
%
%      XMLTreeInfo *AddChildToXMLTree(XMLTreeInfo *xml_info,const char *tag,
%        const size_t offset)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o tag: the tag.
%
%    o offset: the tag offset.
%
*/
MagickExport XMLTreeInfo *AddChildToXMLTree(XMLTreeInfo *xml_info,
  const char *tag,const size_t offset)
{
  XMLTreeInfo
    *child;

  if (xml_info == (XMLTreeInfo *) NULL)
    return((XMLTreeInfo *) NULL);
  child=(XMLTreeInfo *) AcquireMagickMemory(sizeof(*child));
  if (child == (XMLTreeInfo *) NULL)
    return((XMLTreeInfo *) NULL);
  (void) ResetMagickMemory(child,0,sizeof(*child));
  child->tag=ConstantString(tag);
  child->attributes=sentinel;
  child->content=ConstantString("");
  child->debug=IsEventLogging();
  child->signature=MagickSignature;
  return(InsertTagIntoXMLTree(xml_info,child,offset));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A d d P a t h T o X M L T r e e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AddPathToXMLTree() adds a child tag at an offset relative to the start of
%  the parent tag's character content.  This method returns the child tag.
%
%  The format of the AddPathToXMLTree method is:
%
%      XMLTreeInfo *AddPathToXMLTree(XMLTreeInfo *xml_info,const char *path,
%        const size_t offset)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o path: the path.
%
%    o offset: the tag offset.
%
*/
MagickExport XMLTreeInfo *AddPathToXMLTree(XMLTreeInfo *xml_info,
  const char *path,const size_t offset)
{
  char
    **components,
    subnode[MaxTextExtent],
    tag[MaxTextExtent];

  register ssize_t
    i;

  size_t
    number_components;

  ssize_t
    j;

  XMLTreeInfo
    *child,
    *node;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  node=xml_info;
  components=GetPathComponents(path,&number_components);
  if (components == (char **) NULL)
    return((XMLTreeInfo *) NULL);
  for (i=0; i < (ssize_t) number_components; i++)
  {
    GetPathComponent(components[i],SubimagePath,subnode);
    GetPathComponent(components[i],CanonicalPath,tag);
    child=GetXMLTreeChild(node,tag);
    if (child == (XMLTreeInfo *) NULL)
      child=AddChildToXMLTree(node,tag,offset);
    node=child;
    if (node == (XMLTreeInfo *) NULL)
      break;
    for (j=(ssize_t) StringToLong(subnode)-1; j > 0; j--)
    {
      node=GetXMLTreeOrdered(node);
      if (node == (XMLTreeInfo *) NULL)
        break;
    }
    if (node == (XMLTreeInfo *) NULL)
      break;
    components[i]=DestroyString(components[i]);
  }
  for ( ; i < (ssize_t) number_components; i++)
    components[i]=DestroyString(components[i]);
  components=(char **) RelinquishMagickMemory(components);
  return(node);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C a n o n i c a l X M L C o n t e n t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CanonicalXMLContent() converts text to canonical XML content by converting
%  to UTF-8, substituting predefined entities, wrapping as CDATA, or encoding
%  as base-64 as required.
%
%  The format of the CanonicalXMLContent method is:
%
%
%      char *CanonicalXMLContent(const char *content,
%        const MagickBooleanType pedantic)
%
%  A description of each parameter follows:
%
%    o content: the content.
%
%    o pedantic: if true, replace newlines and tabs with their respective
%      entities.
%
*/
MagickExport char *CanonicalXMLContent(const char *content,
  const MagickBooleanType pedantic)
{
  char
    *base64,
    *canonical_content;

  register const unsigned char
    *p;

  register ssize_t
    i;

  size_t
    extent,
    length;

  unsigned char
    *utf8;

  utf8=ConvertLatin1ToUTF8((const unsigned char *) content);
  if (utf8 == (unsigned char *) NULL)
    return((char *) NULL);
  for (p=utf8; *p != '\0'; p++)
    if ((*p < 0x20) && (*p != 0x09) && (*p != 0x0a) && (*p != 0x0d))
      break;
  if (*p != '\0')
    {
      /*
        String is binary, base64-encode it.
      */
      base64=Base64Encode(utf8,strlen((char *) utf8),&length);
      utf8=(unsigned char *) RelinquishMagickMemory(utf8);
      if (base64 == (char *) NULL)
        return((char *) NULL);
      canonical_content=AcquireString("<base64>");
      (void) ConcatenateString(&canonical_content,base64);
      base64=DestroyString(base64);
      (void) ConcatenateString(&canonical_content,"</base64>");
      return(canonical_content);
    }
  /*
    Substitute predefined entities.
  */
  i=0;
  canonical_content=AcquireString((char *) NULL);
  extent=MaxTextExtent;
  for (p=utf8; *p != '\0'; p++)
  {
    if ((i+MaxTextExtent) > (ssize_t) extent)
      {
        extent+=MaxTextExtent;
        canonical_content=(char *) ResizeQuantumMemory(canonical_content,extent,
          sizeof(*canonical_content));
        if (canonical_content == (char *) NULL)
          return(canonical_content);
      }
    switch (*p)
    {
      case '&':
      {
        i+=FormatLocaleString(canonical_content+i,extent,"&amp;");
        break;
      }
      case '<':
      {
        i+=FormatLocaleString(canonical_content+i,extent,"&lt;");
        break;
      }
      case '>':
      {
        i+=FormatLocaleString(canonical_content+i,extent,"&gt;");
        break;
      }
      case '"':
      {
        i+=FormatLocaleString(canonical_content+i,extent,"&quot;");
        break;
      }
      case '\n':
      {
        if (pedantic == MagickFalse)
          {
            canonical_content[i++]=(char) (*p);
            break;
          }
        i+=FormatLocaleString(canonical_content+i,extent,"&#xA;");
        break;
      }
      case '\t':
      {
        if (pedantic == MagickFalse)
          {
            canonical_content[i++]=(char) (*p);
            break;
          }
        i+=FormatLocaleString(canonical_content+i,extent,"&#x9;");
        break;
      }
      case '\r':
      {
        i+=FormatLocaleString(canonical_content+i,extent,"&#xD;");
        break;
      }
      default:
      {
        canonical_content[i++]=(char) (*p);
        break;
      }
    }
  }
  canonical_content[i]='\0';
  utf8=(unsigned char *) RelinquishMagickMemory(utf8);
  return(canonical_content);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y X M L T r e e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyXMLTree() destroys the xml-tree.
%
%  The format of the DestroyXMLTree method is:
%
%      XMLTreeInfo *DestroyXMLTree(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/

static char **DestroyXMLTreeAttributes(char **attributes)
{
  register ssize_t
    i;

  /*
    Destroy a tag attribute list.
  */
  if ((attributes == (char **) NULL) || (attributes == sentinel))
    return((char **) NULL);
  for (i=0; attributes[i] != (char *) NULL; i+=2)
  {
    /*
      Destroy attribute tag and value.
    */
    if (attributes[i] != (char *) NULL)
      attributes[i]=DestroyString(attributes[i]);
    if (attributes[i+1] != (char *) NULL)
      attributes[i+1]=DestroyString(attributes[i+1]);
  }
  attributes=(char **) RelinquishMagickMemory(attributes);
  return((char **) NULL);
}

MagickExport XMLTreeInfo *DestroyXMLTree(XMLTreeInfo *xml_info)
{
  char
    **attributes;

  register ssize_t
    i;

  ssize_t
    j;

  XMLTreeRoot
    *root;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (xml_info->child != (XMLTreeInfo *) NULL)
    xml_info->child=DestroyXMLTree(xml_info->child);
  if (xml_info->ordered != (XMLTreeInfo *) NULL)
    xml_info->ordered=DestroyXMLTree(xml_info->ordered);
  if (xml_info->parent == (XMLTreeInfo *) NULL)
    {
      /*
        Free root tag allocations.
      */
      root=(XMLTreeRoot *) xml_info;
      for (i=NumberPredefinedEntities; root->entities[i]; i+=2)
        root->entities[i+1]=DestroyString(root->entities[i+1]);
      root->entities=(char **) RelinquishMagickMemory(root->entities);
      for (i=0; root->attributes[i] != (char **) NULL; i++)
      {
        attributes=root->attributes[i];
        if (attributes[0] != (char *) NULL)
          attributes[0]=DestroyString(attributes[0]);
        for (j=1; attributes[j] != (char *) NULL; j+=3)
        {
          if (attributes[j] != (char *) NULL)
            attributes[j]=DestroyString(attributes[j]);
          if (attributes[j+1] != (char *) NULL)
            attributes[j+1]=DestroyString(attributes[j+1]);
          if (attributes[j+2] != (char *) NULL)
            attributes[j+2]=DestroyString(attributes[j+2]);
        }
        attributes=(char **) RelinquishMagickMemory(attributes);
      }
      if (root->attributes[0] != (char **) NULL)
        root->attributes=(char ***) RelinquishMagickMemory(root->attributes);
      if (root->processing_instructions[0] != (char **) NULL)
        {
          for (i=0; root->processing_instructions[i] != (char **) NULL; i++)
          {
            for (j=0; root->processing_instructions[i][j] != (char *) NULL; j++)
              root->processing_instructions[i][j]=DestroyString(
                root->processing_instructions[i][j]);
            root->processing_instructions[i][j+1]=DestroyString(
              root->processing_instructions[i][j+1]);
            root->processing_instructions[i]=(char **) RelinquishMagickMemory(
              root->processing_instructions[i]);
          }
          root->processing_instructions=(char ***) RelinquishMagickMemory(
            root->processing_instructions);
        }
    }
  xml_info->attributes=DestroyXMLTreeAttributes(xml_info->attributes);
  xml_info->content=DestroyString(xml_info->content);
  xml_info->tag=DestroyString(xml_info->tag);
  xml_info=(XMLTreeInfo *) RelinquishMagickMemory(xml_info);
  return((XMLTreeInfo *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t X M L T r e e T a g                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextXMLTreeTag() returns the next tag or NULL if not found.
%
%  The format of the GetNextXMLTreeTag method is:
%
%      XMLTreeInfo *GetNextXMLTreeTag(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport XMLTreeInfo *GetNextXMLTreeTag(XMLTreeInfo *xml_info)
{
  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(xml_info->next);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e A t t r i b u t e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeAttribute() returns the value of the attribute tag with the
%  specified tag if found, otherwise NULL.
%
%  The format of the GetXMLTreeAttribute method is:
%
%      const char *GetXMLTreeAttribute(XMLTreeInfo *xml_info,const char *tag)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o tag: the attribute tag.
%
*/
MagickExport const char *GetXMLTreeAttribute(XMLTreeInfo *xml_info,
  const char *tag)
{
  register ssize_t
    i;

  ssize_t
    j;

  XMLTreeRoot
    *root;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (xml_info->attributes == (char **) NULL)
    return((const char *) NULL);
  i=0;
  while ((xml_info->attributes[i] != (char *) NULL) &&
         (strcmp(xml_info->attributes[i],tag) != 0))
    i+=2;
  if (xml_info->attributes[i] != (char *) NULL)
    return(xml_info->attributes[i+1]);
  root=(XMLTreeRoot*) xml_info;
  while (root->root.parent != (XMLTreeInfo *) NULL)
    root=(XMLTreeRoot *) root->root.parent;
  i=0;
  while ((root->attributes[i] != (char **) NULL) &&
         (strcmp(root->attributes[i][0],xml_info->tag) != 0))
    i++;
  if (root->attributes[i] == (char **) NULL)
    return((const char *) NULL);
  j=1;
  while ((root->attributes[i][j] != (char *) NULL) &&
         (strcmp(root->attributes[i][j],tag) != 0))
    j+=3;
  if (root->attributes[i][j] == (char *) NULL)
    return((const char *) NULL);
  return(root->attributes[i][j+1]);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e A t t r i b u t e s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeAttributes() injects all attributes associated with the current
%  tag in the specified splay-tree.
%
%  The format of the GetXMLTreeAttributes method is:
%
%      MagickBooleanType GetXMLTreeAttributes(const XMLTreeInfo *xml_info,
%        SplayTreeInfo *attributes)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o attributes: the attribute splay-tree.
%
*/
MagickExport MagickBooleanType GetXMLTreeAttributes(const XMLTreeInfo *xml_info,
  SplayTreeInfo *attributes)
{
  register ssize_t
    i;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((const XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(attributes != (SplayTreeInfo *) NULL);
  if (xml_info->attributes == (char **) NULL)
    return(MagickTrue);
  i=0;
  while (xml_info->attributes[i] != (char *) NULL)
  {
     (void) AddValueToSplayTree(attributes,
       ConstantString(xml_info->attributes[i]),
       ConstantString(xml_info->attributes[i+1]));
    i+=2;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e C h i l d                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeChild() returns the first child tag with the specified tag if
%  found, otherwise NULL.
%
%  The format of the GetXMLTreeChild method is:
%
%      XMLTreeInfo *GetXMLTreeChild(XMLTreeInfo *xml_info,const char *tag)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport XMLTreeInfo *GetXMLTreeChild(XMLTreeInfo *xml_info,const char *tag)
{
  XMLTreeInfo
    *child;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  child=xml_info->child;
  if (tag != (const char *) NULL)
    while ((child != (XMLTreeInfo *) NULL) && (strcmp(child->tag,tag) != 0))
      child=child->sibling;
  return(child);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e C o n t e n t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeContent() returns any content associated with specified
%  xml-tree node.
%
%  The format of the GetXMLTreeContent method is:
%
%      const char *GetXMLTreeContent(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport const char *GetXMLTreeContent(XMLTreeInfo *xml_info)
{
  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(xml_info->content);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e O r d e r e d                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeOrdered() returns the next ordered node if found, otherwise NULL.
%
%  The format of the GetXMLTreeOrdered method is:
%
%      XMLTreeInfo *GetXMLTreeOrdered(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport XMLTreeInfo *GetXMLTreeOrdered(XMLTreeInfo *xml_info)
{
  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(xml_info->ordered);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e P a t h                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreePath() traverses the XML-tree as defined by the specified path
%  and returns the node if found, otherwise NULL.
%
%  The format of the GetXMLTreePath method is:
%
%      XMLTreeInfo *GetXMLTreePath(XMLTreeInfo *xml_info,const char *path)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o path: the path (e.g. property/elapsed-time).
%
*/
MagickExport XMLTreeInfo *GetXMLTreePath(XMLTreeInfo *xml_info,const char *path)
{
  char
    **components,
    subnode[MaxTextExtent],
    tag[MaxTextExtent];

  register ssize_t
    i;

  size_t
    number_components;

  ssize_t
    j;

  XMLTreeInfo
    *node;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  node=xml_info;
  components=GetPathComponents(path,&number_components);
  if (components == (char **) NULL)
    return((XMLTreeInfo *) NULL);
  for (i=0; i < (ssize_t) number_components; i++)
  {
    GetPathComponent(components[i],SubimagePath,subnode);
    GetPathComponent(components[i],CanonicalPath,tag);
    node=GetXMLTreeChild(node,tag);
    if (node == (XMLTreeInfo *) NULL)
      break;
    for (j=(ssize_t) StringToLong(subnode)-1; j > 0; j--)
    {
      node=GetXMLTreeOrdered(node);
      if (node == (XMLTreeInfo *) NULL)
        break;
    }
    if (node == (XMLTreeInfo *) NULL)
      break;
    components[i]=DestroyString(components[i]);
  }
  for ( ; i < (ssize_t) number_components; i++)
    components[i]=DestroyString(components[i]);
  components=(char **) RelinquishMagickMemory(components);
  return(node);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e P r o c e s s i n g I n s t r u c t i o n s           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeProcessingInstructions() returns a null terminated array of
%  processing instructions for the given target.
%
%  The format of the GetXMLTreeProcessingInstructions method is:
%
%      const char **GetXMLTreeProcessingInstructions(XMLTreeInfo *xml_info,
%        const char *target)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport const char **GetXMLTreeProcessingInstructions(
  XMLTreeInfo *xml_info,const char *target)
{
  register ssize_t
    i;

  XMLTreeRoot
    *root;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  root=(XMLTreeRoot *) xml_info;
  while (root->root.parent != (XMLTreeInfo *) NULL)
    root=(XMLTreeRoot *) root->root.parent;
  i=0;
  while ((root->processing_instructions[i] != (char **) NULL) &&
         (strcmp(root->processing_instructions[i][0],target) != 0))
    i++;
  if (root->processing_instructions[i] == (char **) NULL)
    return((const char **) sentinel);
  return((const char **) (root->processing_instructions[i]+1));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e S i b l i n g                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeSibling() returns the node sibling if found, otherwise NULL.
%
%  The format of the GetXMLTreeSibling method is:
%
%      XMLTreeInfo *GetXMLTreeSibling(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport XMLTreeInfo *GetXMLTreeSibling(XMLTreeInfo *xml_info)
{
  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(xml_info->sibling);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t X M L T r e e T a g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetXMLTreeTag() returns the tag associated with specified xml-tree node.
%
%  The format of the GetXMLTreeTag method is:
%
%      const char *GetXMLTreeTag(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport const char *GetXMLTreeTag(XMLTreeInfo *xml_info)
{
  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(xml_info->tag);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n s e r t I n t o T a g X M L T r e e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InsertTagIntoXMLTree() inserts a tag at an offset relative to the start of
%  the parent tag's character content.  This method returns the child tag.
%
%  The format of the InsertTagIntoXMLTree method is:
%
%      XMLTreeInfo *InsertTagIntoXMLTree(XMLTreeInfo *xml_info,
%        XMLTreeInfo *child,const size_t offset)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o child: the child tag.
%
%    o offset: the tag offset.
%
*/
MagickExport XMLTreeInfo *InsertTagIntoXMLTree(XMLTreeInfo *xml_info,
  XMLTreeInfo *child,const size_t offset)
{
  XMLTreeInfo
    *head,
    *node,
    *previous;

  child->ordered=(XMLTreeInfo *) NULL;
  child->sibling=(XMLTreeInfo *) NULL;
  child->next=(XMLTreeInfo *) NULL;
  child->offset=offset;
  child->parent=xml_info;
  if (xml_info->child == (XMLTreeInfo *) NULL)
    {
      xml_info->child=child;
      return(child);
    }
  head=xml_info->child;
  if (head->offset > offset)
    {
      child->ordered=head;
      xml_info->child=child;
    }
  else
    {
      node=head;
      while ((node->ordered != (XMLTreeInfo *) NULL) &&
             (node->ordered->offset <= offset))
        node=node->ordered;
      child->ordered=node->ordered;
      node->ordered=child;
    }
  previous=(XMLTreeInfo *) NULL;
  node=head;
  while ((node != (XMLTreeInfo *) NULL) && (strcmp(node->tag,child->tag) != 0))
  {
    previous=node;
    node=node->sibling;
  }
  if ((node != (XMLTreeInfo *) NULL) && (node->offset <= offset))
    {
      while ((node->next != (XMLTreeInfo *) NULL) &&
             (node->next->offset <= offset))
        node=node->next;
      child->next=node->next;
      node->next=child;
    }
  else
    {
      if ((previous != (XMLTreeInfo *) NULL) && (node != (XMLTreeInfo *) NULL))
        previous->sibling=node->sibling;
      child->next=node;
      previous=(XMLTreeInfo *) NULL;
      node=head;
      while ((node != (XMLTreeInfo *) NULL) && (node->offset <= offset))
      {
        previous=node;
        node=node->sibling;
      }
      child->sibling=node;
      if (previous != (XMLTreeInfo *) NULL)
        previous->sibling=child;
    }
  return(child);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w X M L T r e e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewXMLTree() returns a XMLTreeInfo xml-tree as defined by the specified
%  XML string.
%
%  The format of the NewXMLTree method is:
%
%      XMLTreeInfo *NewXMLTree(const char *xml,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The XML string.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static char *ConvertUTF16ToUTF8(const char *content,size_t *length)
{
  char
    *utf8;

  int
    bits,
    byte,
    c,
    encoding;

  register ssize_t
    i;

  size_t
    extent;

  ssize_t
    j;

  utf8=(char *) AcquireQuantumMemory(*length,sizeof(*utf8));
  if (utf8 == (char *) NULL)
    return((char *) NULL);
  encoding=(*content == '\xFE') ? 1 : (*content == '\xFF') ? 0 : -1;
  if (encoding == -1)
    {
      /*
        Already UTF-8.
      */
      (void) CopyMagickMemory(utf8,content,*length*sizeof(*utf8));
      return(utf8);
    }
  j=0;
  extent=(*length);
  for (i=2; i < (ssize_t) (*length-1); i+=2)
  {
    c=(encoding != 0) ? ((content[i] & 0xff) << 8) | (content[i+1] & 0xff) :
      ((content[i+1] & 0xff) << 8) | (content[i] & 0xff);
    if ((c >= 0xd800) && (c <= 0xdfff) && ((i+=2) < (ssize_t) (*length-1)))
      {
        byte=(encoding != 0) ? ((content[i] & 0xff) << 8) |
          (content[i+1] & 0xff) : ((content[i+1] & 0xff) << 8) |
          (content[i] & 0xff);
        c=(((c & 0x3ff) << 10) | (byte & 0x3ff))+0x10000;
      }
    if ((size_t) (j+MaxTextExtent) > extent)
      {
        extent=(size_t) j+MaxTextExtent;
        utf8=(char *) ResizeQuantumMemory(utf8,extent,sizeof(*utf8));
        if (utf8 == (char *) NULL)
          return(utf8);
      }
    if (c < 0x80)
      {
        utf8[j]=c;
        j++;
        continue;
      }
    /*
      Multi-byte UTF-8 sequence.
    */
    byte=c;
    for (bits=0; byte != 0; byte/=2)
      bits++;
    bits=(bits-2)/5;
    utf8[j++]=(0xFF << (7-bits)) | (c >> (6*bits));
    while (bits != 0)
    {
      bits--;
      utf8[j]=0x80 | ((c >> (6*bits)) & 0x3f);
      j++;
    }
  }
  *length=(size_t) j;
  return((char *) ResizeQuantumMemory(utf8,*length,sizeof(*utf8)));
}

static char *ParseEntities(char *xml,char **entities,int state)
{
  char
    *entity;

  int
    byte,
    c;

  register char
    *p,
    *q;

  register ssize_t
    i;

  size_t
    extent,
    length;

  ssize_t
    offset;

  /*
    Normalize line endings.
  */
  p=xml;
  q=xml;
  for ( ; *xml != '\0'; xml++)
    while (*xml == '\r')
    {
      *(xml++)='\n';
      if (*xml == '\n')
        (void) CopyMagickMemory(xml,xml+1,strlen(xml));
    }
  for (xml=p; ; )
  {
    while ((*xml != '\0') && (*xml != '&') && ((*xml != '%') ||
           (state != '%')) && (isspace((int) ((unsigned char) *xml) == 0)))
      xml++;
    if (*xml == '\0')
      break;
    /*
      States include:
        '&' for general entity decoding
        '%' for parameter entity decoding
        'c' for CDATA sections
        ' ' for attributes normalization
        '*' for non-CDATA attributes normalization
    */
    if ((state != 'c') && (strncmp(xml,"&#",2) == 0))
      {
        /*
          Character reference.
        */
        if (xml[2] != 'x')
          c=strtol(xml+2,&entity,10);  /* base 10 */
        else
          c=strtol(xml+3,&entity,16);  /* base 16 */
        if ((c == 0) || (*entity != ';'))
          {
            /*
              Not a character reference.
            */
            xml++;
            continue;
          }
        if (c < 0x80)
          *(xml++)=c;
        else
          {
            /*
              Multi-byte UTF-8 sequence.
            */
            byte=c;
            for (i=0; byte != 0; byte/=2)
              i++;
            i=(i-2)/5;
            *xml=(char) ((0xFF << (7-i)) | (c >> (6*i)));
            xml++;
            while (i != 0)
            {
              i--;
              *xml=(char) (0x80 | ((c >> (6*i)) & 0x3F));
              xml++;
            }
          }
        (void) CopyMagickMemory(xml,strchr(xml,';')+1,strlen(strchr(xml,';')));
      }
    else
      if (((*xml == '&') && ((state == '&') || (state == ' ') ||
          (state == '*'))) || ((state == '%') && (*xml == '%')))
        {
          /*
            Find entity in the list.
          */
          i=0;
          while ((entities[i] != (char *) NULL) &&
                 (strncmp(xml+1,entities[i],strlen(entities[i])) != 0))
            i+=2;
          if (entities[i++] == (char *) NULL)
            xml++;
          else
            {
              /*
                Found a match.
              */
              length=strlen(entities[i]);
              entity=strchr(xml,';');
              if ((length-1L) >= (size_t) (entity-xml))
                {
                  offset=(ssize_t) (xml-p);
                  extent=(size_t) (offset+length+strlen(entity));
                  if (p != q)
                    p=(char *) ResizeQuantumMemory(p,extent,sizeof(*p));
                  else
                    {
                      char
                        *xml;

                      xml=(char *) AcquireQuantumMemory(extent,sizeof(*xml));
                      if (xml != (char *) NULL)
                        {
                          (void) CopyMagickString(xml,p,extent*sizeof(*xml));
                          p=xml;
                        }
                    }
                  if (p == (char *) NULL)
                    ThrowFatalException(ResourceLimitFatalError,
                      "MemoryAllocationFailed");
                  xml=p+offset;
                  entity=strchr(xml,';');
                }
              (void) CopyMagickMemory(xml+length,entity+1,strlen(entity));
              (void) strncpy(xml,entities[i],length);
            }
        }
      else
        if (((state == ' ') || (state == '*')) &&
            (isspace((int) ((unsigned char) *xml) != 0)))
          *(xml++)=' ';
        else
          xml++;
  }
  if (state == '*')
    {
      /*
        Normalize spaces for non-CDATA attributes.
      */
      for (xml=p; *xml != '\0'; xml++)
      {
        i=(ssize_t) strspn(xml," ");
        if (i != 0)
          (void) CopyMagickMemory(xml,xml+i,strlen(xml+i)+1);
        while ((*xml != '\0') && (*xml != ' '))
          xml++;
      }
      xml--;
      if ((xml >= p) && (*xml == ' '))
        *xml='\0';
    }
  return(p == q ? ConstantString(p) : p);
}

static void ParseCharacterContent(XMLTreeRoot *root,char *xml,
  const size_t length,const char state)
{
  XMLTreeInfo
    *xml_info;

  xml_info=root->node;
  if ((xml_info == (XMLTreeInfo *) NULL) || (xml_info->tag == (char *) NULL) ||
      (length == 0))
    return;
  xml[length]='\0';
  xml=ParseEntities(xml,root->entities,state);
  if (*xml_info->content != '\0')
    {
      (void) ConcatenateString(&xml_info->content,xml);
      xml=DestroyString(xml);
    }
  else
    {
      if (xml_info->content != (char *) NULL)
        xml_info->content=DestroyString(xml_info->content);
      xml_info->content=xml;
    }
}

static XMLTreeInfo *ParseCloseTag(XMLTreeRoot *root,char *tag,
  char *magick_unused(xml),ExceptionInfo *exception)
{
  if ((root->node == (XMLTreeInfo *) NULL) ||
      (root->node->tag == (char *) NULL) || (strcmp(tag,root->node->tag) != 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "ParseError","unexpected closing tag </%s>",tag);
      return(&root->root);
    }
  root->node=root->node->parent;
  return((XMLTreeInfo *) NULL);
}

static MagickBooleanType ValidateEntities(char *tag,char *xml,char **entities)
{
  register ssize_t
    i;

  /*
    Check for circular entity references.
  */
  for ( ; ; xml++)
  {
    while ((*xml != '\0') && (*xml != '&'))
      xml++;
    if (*xml == '\0')
      return(MagickTrue);
    if (strncmp(xml+1,tag,strlen(tag)) == 0)
      return(MagickFalse);
    i=0;
    while ((entities[i] != (char *) NULL) &&
           (strncmp(entities[i],xml+1,strlen(entities[i]) == 0)))
      i+=2;
    if ((entities[i] != (char *) NULL) &&
        (ValidateEntities(tag,entities[i+1],entities) == 0))
      return(MagickFalse);
  }
  return(MagickTrue);
}

static void ParseProcessingInstructions(XMLTreeRoot *root,char *xml,
  size_t length)
{
  char
    *target;

  register ssize_t
    i;

  ssize_t
    j;

  target=xml;
  xml[length]='\0';
  xml+=strcspn(xml,XMLWhitespace);
  if (*xml != '\0')
    {
      *xml='\0';
      xml+=strspn(xml+1,XMLWhitespace)+1;
    }
  if (strcmp(target,"xml") == 0)
    {
      xml=strstr(xml,"standalone");
      if ((xml != (char *) NULL) &&
          (strncmp(xml+strspn(xml+10,XMLWhitespace "='\"")+10,"yes",3) == 0))
        root->standalone=MagickTrue;
      return;
    }
  if (root->processing_instructions[0] == (char **) NULL)
    {
      root->processing_instructions=(char ***) AcquireMagickMemory(sizeof(
        *root->processing_instructions));
      if (root->processing_instructions ==(char ***) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      *root->processing_instructions=(char **) NULL;
    }
  i=0;
  while ((root->processing_instructions[i] != (char **) NULL) &&
         (strcmp(target,root->processing_instructions[i][0]) != 0))
    i++;
  if (root->processing_instructions[i] == (char **) NULL)
    {
      root->processing_instructions=(char ***) ResizeQuantumMemory(
        root->processing_instructions,(size_t) (i+2),
        sizeof(*root->processing_instructions));
      if (root->processing_instructions == (char ***) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      root->processing_instructions[i]=(char **) AcquireQuantumMemory(3,
        sizeof(**root->processing_instructions));
      if (root->processing_instructions[i] == (char **) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      root->processing_instructions[i+1]=(char **) NULL;
      root->processing_instructions[i][0]=ConstantString(target);
      root->processing_instructions[i][1]=(char *)
        root->processing_instructions[i+1];
      root->processing_instructions[i+1]=(char **) NULL;
      root->processing_instructions[i][2]=ConstantString("");
    }
  j=1;
  while (root->processing_instructions[i][j] != (char *) NULL)
    j++;
  root->processing_instructions[i]=(char **) ResizeQuantumMemory(
    root->processing_instructions[i],(size_t) (j+3),
    sizeof(**root->processing_instructions));
  if (root->processing_instructions[i] == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  root->processing_instructions[i][j+2]=(char *) ResizeQuantumMemory(
    root->processing_instructions[i][j+1],(size_t) (j+1),
    sizeof(**root->processing_instructions));
  if (root->processing_instructions[i][j+2] == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) CopyMagickString(root->processing_instructions[i][j+2]+j-1,
    root->root.tag != (char *) NULL ? ">" : "<",2);
  root->processing_instructions[i][j]=ConstantString(xml);
  root->processing_instructions[i][j+1]=(char *) NULL;
}

static MagickBooleanType ParseInternalDoctype(XMLTreeRoot *root,char *xml,
  size_t length,ExceptionInfo *exception)
{
  char
    *c,
    **entities,
    *n,
    **predefined_entitites,
    q,
    *t,
    *v;

  register ssize_t
    i;

  ssize_t
    j;

  n=(char *) NULL;
  predefined_entitites=(char **) AcquireMagickMemory(sizeof(sentinel));
  if (predefined_entitites == (char **) NULL)
    ThrowFatalException(ResourceLimitError,"MemoryAllocationFailed");
  (void) CopyMagickMemory(predefined_entitites,sentinel,sizeof(sentinel));
  for (xml[length]='\0'; xml != (char *) NULL; )
  {
    while ((*xml != '\0') && (*xml != '<') && (*xml != '%'))
      xml++;
    if (*xml == '\0')
      break;
    if (strncmp(xml,"<!ENTITY",8) == 0)
      {
        /*
          Parse entity definitions.
        */
        xml+=strspn(xml+8,XMLWhitespace)+8;
        c=xml;
        n=xml+strspn(xml,XMLWhitespace "%");
        xml=n+strcspn(n,XMLWhitespace);
        *xml=';';
        v=xml+strspn(xml+1,XMLWhitespace)+1;
        q=(*v);
        v++;
        if ((q != '"') && (q != '\''))
          {
            /*
              Skip externals.
            */
            xml=strchr(xml,'>');
            continue;
          }
        entities=(*c == '%') ? predefined_entitites : root->entities;
        for (i=0; entities[i] != (char *) NULL; i++) ;
        entities=(char **) ResizeQuantumMemory(entities,(size_t) (i+3),
          sizeof(*entities));
        if (entities == (char **) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        if (*c == '%')
          predefined_entitites=entities;
        else
          root->entities=entities;
        xml++;
        *xml='\0';
        xml=strchr(v,q);
        if (xml != (char *) NULL)
          {
            *xml='\0';
            xml++;
          }
        entities[i+1]=ParseEntities(v,predefined_entitites,'%');
        entities[i+2]=(char *) NULL;
        if (ValidateEntities(n,entities[i+1],entities) != MagickFalse)
          entities[i]=n;
        else
          {
            if (entities[i+1] != v)
              entities[i+1]=DestroyString(entities[i+1]);
            (void) ThrowMagickException(exception,GetMagickModule(),
              OptionWarning,"ParseError","circular entity declaration &%s",n);
            predefined_entitites=(char **) RelinquishMagickMemory(
              predefined_entitites);
            return(MagickFalse);
          }
        }
      else
       if (strncmp(xml,"<!ATTLIST",9) == 0)
         {
            /*
              Parse default attributes.
            */
            t=xml+strspn(xml+9,XMLWhitespace)+9;
            if (*t == '\0')
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionWarning,"ParseError","unclosed <!ATTLIST");
                predefined_entitites=(char **) RelinquishMagickMemory(
                  predefined_entitites);
                return(MagickFalse);
              }
            xml=t+strcspn(t,XMLWhitespace ">");
            if (*xml == '>')
              continue;
            *xml='\0';
            i=0;
            while ((root->attributes[i] != (char **) NULL) &&
                   (strcmp(n,root->attributes[i][0]) != 0))
              i++;
            while ((*(n=xml+strspn(xml+1,XMLWhitespace)+1) != '\0') &&
                   (*n != '>'))
            {
              xml=n+strcspn(n,XMLWhitespace);
              if (*xml != '\0')
                *xml='\0';
              else
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    OptionWarning,"ParseError","malformed <!ATTLIST");
                  predefined_entitites=(char **) RelinquishMagickMemory(
                    predefined_entitites);
                  return(MagickFalse);
                }
              xml+=strspn(xml+1,XMLWhitespace)+1;
              c=(char *) (strncmp(xml,"CDATA",5) != 0 ? "*" : " ");
              if (strncmp(xml,"NOTATION",8) == 0)
                xml+=strspn(xml+8,XMLWhitespace)+8;
              xml=(*xml == '(') ? strchr(xml,')') : xml+
                strcspn(xml,XMLWhitespace);
              if (xml == (char *) NULL)
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    OptionWarning,"ParseError","malformed <!ATTLIST");
                  predefined_entitites=(char **) RelinquishMagickMemory(
                    predefined_entitites);
                  return(MagickFalse);
                }
              xml+=strspn(xml,XMLWhitespace ")");
              if (strncmp(xml,"#FIXED",6) == 0)
                xml+=strspn(xml+6,XMLWhitespace)+6;
              if (*xml == '#')
                {
                  xml+=strcspn(xml,XMLWhitespace ">")-1;
                  if (*c == ' ')
                    continue;
                  v=(char *) NULL;
                }
              else
                if (((*xml == '"') || (*xml == '\''))  &&
                    ((xml=strchr(v=xml+1,*xml)) != (char *) NULL))
                  *xml='\0';
                else
                  {
                    (void) ThrowMagickException(exception,GetMagickModule(),
                      OptionWarning,"ParseError","malformed <!ATTLIST");
                    predefined_entitites=(char **) RelinquishMagickMemory(
                      predefined_entitites);
                    return(MagickFalse);
                  }
              if (root->attributes[i] == (char **) NULL)
                {
                  /*
                    New attribute tag.
                  */
                  if (i == 0)
                    root->attributes=(char ***) AcquireQuantumMemory(2,
                      sizeof(*root->attributes));
                  else
                    root->attributes=(char ***) ResizeQuantumMemory(
                      root->attributes,(size_t) (i+2),
                      sizeof(*root->attributes));
                  if (root->attributes == (char ***) NULL)
                    ThrowFatalException(ResourceLimitFatalError,
                      "MemoryAllocationFailed");
                  root->attributes[i]=(char **) AcquireQuantumMemory(2,
                    sizeof(*root->attributes));
                  if (root->attributes[i] == (char **) NULL)
                    ThrowFatalException(ResourceLimitFatalError,
                      "MemoryAllocationFailed");
                  root->attributes[i][0]=ConstantString(t);
                  root->attributes[i][1]=(char *) NULL;
                  root->attributes[i+1]=(char **) NULL;
                }
              for (j=1; root->attributes[i][j] != (char *) NULL; j+=3) ;
              root->attributes[i]=(char **) ResizeQuantumMemory(
                root->attributes[i],(size_t) (j+4),sizeof(*root->attributes));
              if (root->attributes[i] == (char **) NULL)
                ThrowFatalException(ResourceLimitFatalError,
                  "MemoryAllocationFailed");
              root->attributes[i][j+3]=(char *) NULL;
              root->attributes[i][j+2]=ConstantString(c);
              root->attributes[i][j+1]=(char *) NULL;
              if (v != (char *) NULL)
                root->attributes[i][j+1]=ParseEntities(v,root->entities,*c);
              root->attributes[i][j]=ConstantString(n);
            }
        }
      else
        if (strncmp(xml, "<!--", 4) == 0)
          xml=strstr(xml+4,"-->");
        else
          if (strncmp(xml,"<?", 2) == 0)
            {
              c=xml+2;
              xml=strstr(c,"?>");
              if (xml != (char *) NULL)
                {
                  ParseProcessingInstructions(root,c,(size_t) (xml-c));
                  xml++;
                }
            }
           else
             if (*xml == '<')
               xml=strchr(xml,'>');
             else
               if ((*(xml++) == '%') && (root->standalone == MagickFalse))
                 break;
    }
  predefined_entitites=(char **) RelinquishMagickMemory(predefined_entitites);
  return(MagickTrue);
}

static void ParseOpenTag(XMLTreeRoot *root,char *tag,char **attributes)
{
  XMLTreeInfo
    *xml_info;

  xml_info=root->node;
  if (xml_info->tag == (char *) NULL)
    xml_info->tag=ConstantString(tag);
  else
    xml_info=AddChildToXMLTree(xml_info,tag,strlen(xml_info->content));
  xml_info->attributes=attributes;
  root->node=xml_info;
}

MagickExport XMLTreeInfo *NewXMLTree(const char *xml,ExceptionInfo *exception)
{
  char
    **attribute,
    **attributes,
    *tag,
    *utf8;

  int
    c,
    terminal;

  MagickBooleanType
    status;

  register char
    *p;

  register ssize_t
    i;

  size_t
    length;

  ssize_t
    j,
    l;

  XMLTreeRoot
    *root;

  /*
    Convert xml-string to UTF8.
  */
  if ((xml == (const char *) NULL) || (strlen(xml) == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "ParseError","root tag missing");
      return((XMLTreeInfo *) NULL);
    }
  root=(XMLTreeRoot *) NewXMLTreeTag((char *) NULL);
  length=strlen(xml);
  utf8=ConvertUTF16ToUTF8(xml,&length);
  if (utf8 == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "ParseError","UTF16 to UTF8 failed");
      return((XMLTreeInfo *) NULL);
    }
  terminal=utf8[length-1];
  utf8[length-1]='\0';
  p=utf8;
  while ((*p != '\0') && (*p != '<'))
    p++;
  if (*p == '\0')
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "ParseError","root tag missing");
      utf8=DestroyString(utf8);
      return((XMLTreeInfo *) NULL);
    }
  attribute=(char **) NULL;
  for (p++; ; p++)
  {
    attributes=(char **) sentinel;
    tag=p;
    c=(*p);
    if ((isalpha((int) ((unsigned char) *p)) !=0) || (*p == '_') ||
        (*p == ':') || (c < '\0'))
      {
        /*
          Tag.
        */
        if (root->node == (XMLTreeInfo *) NULL)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              OptionWarning,"ParseError","root tag missing");
            utf8=DestroyString(utf8);
            return(&root->root);
          }
        p+=strcspn(p,XMLWhitespace "/>");
        while (isspace((int) ((unsigned char) *p)) != 0)
          *p++='\0';
        if ((*p != '\0') && (*p != '/') && (*p != '>'))
          {
            /*
              Find tag in default attributes list.
            */
            i=0;
            while ((root->attributes[i] != (char **) NULL) &&
                   (strcmp(root->attributes[i][0],tag) != 0))
              i++;
            attribute=root->attributes[i];
          }
        for (l=0; (*p != '\0') && (*p != '/') && (*p != '>'); l+=2)
        {
          /*
            Attribute.
          */
          if (l == 0)
            attributes=(char **) AcquireQuantumMemory(4,sizeof(*attributes));
          else
            attributes=(char **) ResizeQuantumMemory(attributes,(size_t) (l+4),
              sizeof(*attributes));
          if (attributes == (char **) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'","");
              utf8=DestroyString(utf8);
              return(&root->root);
            }
          attributes[l+2]=(char *) NULL;
          attributes[l+1]=(char *) NULL;
          attributes[l]=p;
          p+=strcspn(p,XMLWhitespace "=/>");
          if ((*p != '=') && (isspace((int) ((unsigned char) *p)) == 0))
            attributes[l]=ConstantString("");
          else
            {
              *p++='\0';
              p+=strspn(p,XMLWhitespace "=");
              c=(*p);
              if ((c == '"') || (c == '\''))
                {
                  /*
                    Attributes value.
                  */
                  p++;
                  attributes[l+1]=p;
                  while ((*p != '\0') && (*p != c))
                    p++;
                  if (*p != '\0')
                    *p++='\0';
                  else
                    {
                      attributes[l]=ConstantString("");
                      attributes[l+1]=ConstantString("");
                      (void) DestroyXMLTreeAttributes(attributes);
                      (void) ThrowMagickException(exception,GetMagickModule(),
                        OptionWarning,"ParseError","missing %c",c);
                      utf8=DestroyString(utf8);
                      return(&root->root);
                    }
                  j=1;
                  while ((attribute != (char **) NULL) &&
                         (attribute[j] != (char *) NULL) &&
                         (strcmp(attribute[j],attributes[l]) != 0))
                    j+=3;
                  attributes[l+1]=ParseEntities(attributes[l+1],root->entities,
                    (attribute != (char **) NULL) && (attribute[j] !=
                    (char *) NULL) ? *attribute[j+2] : ' ');
                }
              attributes[l]=ConstantString(attributes[l]);
            }
          while (isspace((int) ((unsigned char) *p)) != 0)
            p++;
        }
        if (*p == '/')
          {
            /*
              Self closing tag.
            */
            *p++='\0';
            if (((*p != '\0') && (*p != '>')) ||
                ((*p == '\0') && (terminal != '>')))
              {
                if (l != 0)
                  (void) DestroyXMLTreeAttributes(attributes);
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionWarning,"ParseError","missing >");
                utf8=DestroyString(utf8);
                return(&root->root);
              }
            ParseOpenTag(root,tag,attributes);
            (void) ParseCloseTag(root,tag,p,exception);
          }
        else
          {
            c=(*p);
            if ((*p == '>') || ((*p == '\0') && (terminal == '>')))
              {
                *p='\0';
                ParseOpenTag(root,tag,attributes);
                *p=c;
              }
            else
              {
                if (l != 0)
                  (void) DestroyXMLTreeAttributes(attributes);
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionWarning,"ParseError","missing >");
                utf8=DestroyString(utf8);
                return(&root->root);
              }
          }
      }
    else
      if (*p == '/')
        {
          /*
            Close tag.
          */
          tag=p+1;
          p+=strcspn(tag,XMLWhitespace ">")+1;
          c=(*p);
          if ((c == '\0') && (terminal != '>'))
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionWarning,"ParseError","missing >");
              utf8=DestroyString(utf8);
              return(&root->root);
            }
          *p='\0';
          if (ParseCloseTag(root,tag,p,exception) != (XMLTreeInfo *) NULL)
            {
              utf8=DestroyString(utf8);
              return(&root->root);
            }
          *p=c;
          if (isspace((int) ((unsigned char) *p)) != 0)
            p+=strspn(p,XMLWhitespace);
        }
      else
        if (strncmp(p,"!--",3) == 0)
          {
            /*
              Comment.
            */
            p=strstr(p+3,"--");
            if ((p == (char *) NULL) || ((*(p+=2) != '>') && (*p != '\0')) ||
                ((*p == '\0') && (terminal != '>')))
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionWarning,"ParseError","unclosed <!--");
                utf8=DestroyString(utf8);
                return(&root->root);
              }
          }
        else
          if (strncmp(p,"![CDATA[",8) == 0)
            {
              /*
                Cdata.
              */
              p=strstr(p,"]]>");
              if (p != (char *) NULL)
                {
                  p+=2;
                  ParseCharacterContent(root,tag+8,(size_t) (p-tag-10),'c');
                }
              else
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    OptionWarning,"ParseError","unclosed <![CDATA[");
                  utf8=DestroyString(utf8);
                  return(&root->root);
                }
            }
          else
            if (strncmp(p,"!DOCTYPE",8) == 0)
              {
                /*
                  DTD.
                */
                for (l=0; (*p != '\0') && (((l == 0) && (*p != '>')) ||
                     ((l != 0) && ((*p != ']') ||
                     (*(p+strspn(p+1,XMLWhitespace)+1) != '>'))));
                  l=(ssize_t) ((*p == '[') ? 1 : l))
                p+=strcspn(p+1,"[]>")+1;
                if ((*p == '\0') && (terminal != '>'))
                  {
                    (void) ThrowMagickException(exception,GetMagickModule(),
                      OptionWarning,"ParseError","unclosed <!DOCTYPE");
                    utf8=DestroyString(utf8);
                    return(&root->root);
                  }
                if (l != 0)
                  tag=strchr(tag,'[')+1;
                if (l != 0)
                  {
                    status=ParseInternalDoctype(root,tag,(size_t) (p-tag),
                      exception);
                    if (status == MagickFalse)
                      {
                        utf8=DestroyString(utf8);
                        return(&root->root);
                      }
                    p++;
                  }
              }
            else
              if (*p == '?')
                {
                  /*
                    Processing instructions.
                  */
                  do
                  {
                    p=strchr(p,'?');
                    if (p == (char *) NULL)
                      break;
                    p++;
                  } while ((*p != '\0') && (*p != '>'));
                  if ((p == (char *) NULL) || ((*p == '\0') &&
                      (terminal != '>')))
                    {
                      (void) ThrowMagickException(exception,GetMagickModule(),
                        OptionWarning,"ParseError","unclosed <?");
                      utf8=DestroyString(utf8);
                      return(&root->root);
                    }
                  ParseProcessingInstructions(root,tag+1,(size_t) (p-tag-2));
                }
              else
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    OptionWarning,"ParseError","unexpected <");
                  utf8=DestroyString(utf8);
                  return(&root->root);
                }
     if ((p == (char *) NULL) || (*p == '\0'))
       break;
     *p++='\0';
     tag=p;
     if ((*p != '\0') && (*p != '<'))
       {
        /*
          Tag character content.
        */
        while ((*p != '\0') && (*p != '<'))
          p++;
        if (*p == '\0')
          break;
        ParseCharacterContent(root,tag,(size_t) (p-tag),'&');
      }
    else
      if (*p == '\0')
        break;
  }
  utf8=DestroyString(utf8);
  if (root->node == (XMLTreeInfo *) NULL)
    return(&root->root);
  if (root->node->tag == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "ParseError","root tag missing");
      return(&root->root);
    }
  (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
    "ParseError","unclosed tag: `%s'",root->node->tag);
  return(&root->root);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w X M L T r e e T a g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewXMLTreeTag() returns a new empty xml structure for the xml-tree tag.
%
%  The format of the NewXMLTreeTag method is:
%
%      XMLTreeInfo *NewXMLTreeTag(const char *tag)
%
%  A description of each parameter follows:
%
%    o tag: the tag.
%
*/
MagickExport XMLTreeInfo *NewXMLTreeTag(const char *tag)
{
  static const char
    *predefined_entities[NumberPredefinedEntities+1] =
    {
      "lt;", "&#60;", "gt;", "&#62;", "quot;", "&#34;",
      "apos;", "&#39;", "amp;", "&#38;", (char *) NULL
    };

  XMLTreeRoot
    *root;

  root=(XMLTreeRoot *) AcquireMagickMemory(sizeof(*root));
  if (root == (XMLTreeRoot *) NULL)
    return((XMLTreeInfo *) NULL);
  (void) ResetMagickMemory(root,0,sizeof(*root));
  root->root.tag=(char *) NULL;
  if (tag != (char *) NULL)
    root->root.tag=ConstantString(tag);
  root->node=(&root->root);
  root->root.content=ConstantString("");
  root->entities=(char **) AcquireMagickMemory(sizeof(predefined_entities));
  if (root->entities == (char **) NULL)
    return((XMLTreeInfo *) NULL);
  (void) CopyMagickMemory(root->entities,predefined_entities,
    sizeof(predefined_entities));
  root->root.attributes=sentinel;
  root->attributes=(char ***) root->root.attributes;
  root->processing_instructions=(char ***) root->root.attributes;
  root->debug=IsEventLogging();
  root->signature=MagickSignature;
  return(&root->root);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r u n e T a g F r o m X M L T r e e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PruneTagFromXMLTree() prunes a tag from the xml-tree along with all its
%  subtags.
%
%  The format of the PruneTagFromXMLTree method is:
%
%      XMLTreeInfo *PruneTagFromXMLTree(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/
MagickExport XMLTreeInfo *PruneTagFromXMLTree(XMLTreeInfo *xml_info)
{
  XMLTreeInfo
    *node;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (xml_info->next != (XMLTreeInfo *) NULL)
    xml_info->next->sibling=xml_info->sibling;
  if (xml_info->parent != (XMLTreeInfo *) NULL)
    {
      node=xml_info->parent->child;
      if (node == xml_info)
        xml_info->parent->child=xml_info->ordered;
      else
        {
          while (node->ordered != xml_info)
            node=node->ordered;
          node->ordered=node->ordered->ordered;
          node=xml_info->parent->child;
          if (strcmp(node->tag,xml_info->tag) != 0)
            {
              while (strcmp(node->sibling->tag,xml_info->tag) != 0)
                node=node->sibling;
              if (node->sibling != xml_info)
                node=node->sibling;
              else
                node->sibling=(xml_info->next != (XMLTreeInfo *) NULL) ?
                  xml_info->next : node->sibling->sibling;
            }
          while ((node->next != (XMLTreeInfo *) NULL) &&
                 (node->next != xml_info))
            node=node->next;
          if (node->next != (XMLTreeInfo *) NULL)
            node->next=node->next->next;
        }
    }
  xml_info->ordered=(XMLTreeInfo *) NULL;
  xml_info->sibling=(XMLTreeInfo *) NULL;
  xml_info->next=(XMLTreeInfo *) NULL;
  return(xml_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t X M L T r e e A t t r i b u t e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetXMLTreeAttribute() sets the tag attributes or adds a new attribute if not
%  found.  A value of NULL removes the specified attribute.
%
%  The format of the SetXMLTreeAttribute method is:
%
%      XMLTreeInfo *SetXMLTreeAttribute(XMLTreeInfo *xml_info,const char *tag,
%        const char *value)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o tag:  The attribute tag.
%
%    o value:  The attribute value.
%
*/
MagickExport XMLTreeInfo *SetXMLTreeAttribute(XMLTreeInfo *xml_info,
  const char *tag,const char *value)
{
  register ssize_t
    i;

  ssize_t
    j;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  i=0;
  while ((xml_info->attributes[i] != (char *) NULL) &&
         (strcmp(xml_info->attributes[i],tag) != 0))
    i+=2;
  if (xml_info->attributes[i] == (char *) NULL)
    {
      /*
        Add new attribute tag.
      */
      if (value == (const char *) NULL)
        return(xml_info);
      if (xml_info->attributes != sentinel)
        xml_info->attributes=(char **) ResizeQuantumMemory(
          xml_info->attributes,(size_t) (i+4),sizeof(*xml_info->attributes));
      else
        {
          xml_info->attributes=(char **) AcquireQuantumMemory(4,
            sizeof(*xml_info->attributes));
          if (xml_info->attributes != (char **) NULL)
            xml_info->attributes[1]=ConstantString("");
        }
      if (xml_info->attributes == (char **) NULL)
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToAcquireString");
      xml_info->attributes[i]=ConstantString(tag);
      xml_info->attributes[i+2]=(char *) NULL;
      (void) strlen(xml_info->attributes[i+1]);
    }
  /*
    Add new value to an existing attribute.
  */
  for (j=i; xml_info->attributes[j] != (char *) NULL; j+=2) ;
  if (xml_info->attributes[i+1] != (char *) NULL)
    xml_info->attributes[i+1]=DestroyString(xml_info->attributes[i+1]);
  if (value != (const char *) NULL)
    {
      xml_info->attributes[i+1]=ConstantString(value);
      return(xml_info);
    }
  if (xml_info->attributes[i] != (char *) NULL)
    xml_info->attributes[i]=DestroyString(xml_info->attributes[i]);
  (void) CopyMagickMemory(xml_info->attributes+i,xml_info->attributes+i+2,
    (size_t) (j-i)*sizeof(*xml_info->attributes));
  j-=2;
  xml_info->attributes=(char **) ResizeQuantumMemory(xml_info->attributes,
    (size_t) (j+2),sizeof(*xml_info->attributes));
  if (xml_info->attributes == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  (void) CopyMagickMemory(xml_info->attributes[j+1]+(i/2),
    xml_info->attributes[j+1]+(i/2)+1,(size_t) ((j/2)-(i/2))*
    sizeof(*xml_info->attributes));
  return(xml_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t X M L T r e e C o n t e n t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetXMLTreeContent() sets the character content for the given tag and
%  returns the tag.
%
%  The format of the SetXMLTreeContent method is:
%
%      XMLTreeInfo *SetXMLTreeContent(XMLTreeInfo *xml_info,
%        const char *content)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
%    o content:  The content.
%
*/
MagickExport XMLTreeInfo *SetXMLTreeContent(XMLTreeInfo *xml_info,
  const char *content)
{
  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (xml_info->content != (char *) NULL)
    xml_info->content=DestroyString(xml_info->content);
  xml_info->content=(char *) ConstantString(content);
  return(xml_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M L T r e e I n f o T o X M L                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMLTreeInfoToXML() converts an xml-tree to an XML string.
%
%  The format of the XMLTreeInfoToXML method is:
%
%      char *XMLTreeInfoToXML(XMLTreeInfo *xml_info)
%
%  A description of each parameter follows:
%
%    o xml_info: the xml info.
%
*/

static char *EncodePredefinedEntities(const char *source,ssize_t offset,
  char **destination,size_t *length,size_t *extent,MagickBooleanType pedantic)
{
  char
    *canonical_content;

  if (offset < 0)
    canonical_content=CanonicalXMLContent(source,pedantic);
  else
    {
      char
        *content;

      content=AcquireString(source);
      content[offset]='\0';
      canonical_content=CanonicalXMLContent(content,pedantic);
      content=DestroyString(content);
    }
  if (canonical_content == (char *) NULL)
    return(*destination);
  if ((*length+strlen(canonical_content)+MaxTextExtent) > *extent)
    {
      *extent=(*length)+strlen(canonical_content)+MaxTextExtent;
      *destination=(char *) ResizeQuantumMemory(*destination,*extent,
        sizeof(**destination));
      if (*destination == (char *) NULL)
        return(*destination);
    }
  *length+=FormatLocaleString(*destination+(*length),*extent,"%s",
    canonical_content);
  canonical_content=DestroyString(canonical_content);
  return(*destination);
}

static char *XMLTreeTagToXML(XMLTreeInfo *xml_info,char **source,size_t *length,
  size_t *extent,size_t start,char ***attributes)
{
  char
    *content;

  const char
    *attribute;

  register ssize_t
    i;

  size_t
    offset;

  ssize_t
    j;

  content=(char *) "";
  if (xml_info->parent != (XMLTreeInfo *) NULL)
    content=xml_info->parent->content;
  offset=0;
  *source=EncodePredefinedEntities(content+start,(ssize_t) (xml_info->offset-
    start),source,length,extent,MagickFalse);
  if ((*length+strlen(xml_info->tag)+MaxTextExtent) > *extent)
    {
      *extent=(*length)+strlen(xml_info->tag)+MaxTextExtent;
      *source=(char *) ResizeQuantumMemory(*source,*extent,sizeof(*source));
      if (*source == (char *) NULL)
        return(*source);
    }
  *length+=FormatLocaleString(*source+(*length),*extent,"<%s",xml_info->tag);
  for (i=0; xml_info->attributes[i]; i+=2)
  {
    attribute=GetXMLTreeAttribute(xml_info,xml_info->attributes[i]);
    if (attribute != xml_info->attributes[i+1])
      continue;
    if ((*length+strlen(xml_info->attributes[i])+MaxTextExtent) > *extent)
      {
        *extent=(*length)+strlen(xml_info->attributes[i])+MaxTextExtent;
        *source=(char *) ResizeQuantumMemory(*source,*extent,sizeof(**source));
        if (*source == (char *) NULL)
          return((char *) NULL);
      }
    *length+=FormatLocaleString(*source+(*length),*extent," %s=\"",
      xml_info->attributes[i]);
    (void) EncodePredefinedEntities(xml_info->attributes[i+1],-1,source,length,
      extent,MagickTrue);
    *length+=FormatLocaleString(*source+(*length),*extent,"\"");
  }
  i=0;
  while ((attributes[i] != (char **) NULL) &&
         (strcmp(attributes[i][0],xml_info->tag) != 0))
    i++;
  j=1;
  while ((attributes[i] != (char **) NULL) &&
         (attributes[i][j] != (char *) NULL))
  {
    if ((attributes[i][j+1] == (char *) NULL) ||
        (GetXMLTreeAttribute(xml_info,attributes[i][j]) != attributes[i][j+1]))
      {
        j+=3;
        continue;
      }
    if ((*length+strlen(attributes[i][j])+MaxTextExtent) > *extent)
      {
        *extent=(*length)+strlen(attributes[i][j])+MaxTextExtent;
        *source=(char *) ResizeQuantumMemory(*source,*extent,sizeof(**source));
        if (*source == (char *) NULL)
          return((char *) NULL);
      }
    *length+=FormatLocaleString(*source+(*length),*extent," %s=\"",
      attributes[i][j]);
    (void) EncodePredefinedEntities(attributes[i][j+1],-1,source,length,extent,
      MagickTrue);
    *length+=FormatLocaleString(*source+(*length),*extent,"\"");
    j+=3;
  }
  *length+=FormatLocaleString(*source+(*length),*extent,*xml_info->content ?
    ">" : "/>");
  if (xml_info->child != (XMLTreeInfo *) NULL)
    *source=XMLTreeTagToXML(xml_info->child,source,length,extent,0,attributes);
  else
    *source=EncodePredefinedEntities(xml_info->content,-1,source,length,extent,
      MagickFalse);
  if ((*length+strlen(xml_info->tag)+MaxTextExtent) > *extent)
    {
      *extent=(*length)+strlen(xml_info->tag)+MaxTextExtent;
      *source=(char *) ResizeQuantumMemory(*source,*extent,sizeof(**source));
      if (*source == (char *) NULL)
        return((char *) NULL);
    }
  if (*xml_info->content != '\0')
    *length+=FormatLocaleString(*source+(*length),*extent,"</%s>",
      xml_info->tag);
  while ((content[offset] != '\0') && (offset < xml_info->offset))
    offset++;
  if (xml_info->ordered != (XMLTreeInfo *) NULL)
    content=XMLTreeTagToXML(xml_info->ordered,source,length,extent,offset,
      attributes);
  else
    content=EncodePredefinedEntities(content+offset,-1,source,length,extent,
      MagickFalse);
  return(content);
}

MagickExport char *XMLTreeInfoToXML(XMLTreeInfo *xml_info)
{
  char
    *xml;

  register char
    *p,
    *q;

  register ssize_t
    i;

  size_t
    extent,
    length;

  ssize_t
    j,
    k;

  XMLTreeInfo
    *ordered,
    *parent;

  XMLTreeRoot
    *root;

  assert(xml_info != (XMLTreeInfo *) NULL);
  assert((xml_info->signature == MagickSignature) ||
         (((XMLTreeRoot *) xml_info)->signature == MagickSignature));
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (xml_info->tag == (char *) NULL)
    return((char *) NULL);
  xml=AcquireString((char *) NULL);
  length=0;
  extent=MaxTextExtent;
  root=(XMLTreeRoot *) xml_info;
  while (root->root.parent != (XMLTreeInfo *) NULL)
    root=(XMLTreeRoot *) root->root.parent;
  parent=(XMLTreeInfo *) NULL;
  if (xml_info != (XMLTreeInfo *) NULL)
    parent=xml_info->parent;
  if (parent == (XMLTreeInfo *) NULL)
    for (i=0; root->processing_instructions[i] != (char **) NULL; i++)
    {
      /*
        Pre-root processing instructions.
      */
      for (k=2; root->processing_instructions[i][k-1]; k++) ;
      p=root->processing_instructions[i][1];
      for (j=1; p != (char *) NULL; j++)
      {
        if (root->processing_instructions[i][k][j-1] == '>')
          {
            p=root->processing_instructions[i][j];
            continue;
          }
        q=root->processing_instructions[i][0];
        if ((length+strlen(p)+strlen(q)+MaxTextExtent) > extent)
          {
            extent=length+strlen(p)+strlen(q)+MaxTextExtent;
            xml=(char *) ResizeQuantumMemory(xml,extent,sizeof(*xml));
            if (xml == (char *) NULL)
              return(xml);
          }
        length+=FormatLocaleString(xml+length,extent,"<?%s%s%s?>\n",q,
          *p != '\0' ? " " : "",p);
        p=root->processing_instructions[i][j];
      }
    }
  ordered=(XMLTreeInfo *) NULL;
  if (xml_info != (XMLTreeInfo *) NULL)
    ordered=xml_info->ordered;
  xml_info->parent=(XMLTreeInfo *) NULL;
  xml_info->ordered=(XMLTreeInfo *) NULL;
  xml=XMLTreeTagToXML(xml_info,&xml,&length,&extent,0,root->attributes);
  xml_info->parent=parent;
  xml_info->ordered=ordered;
  if (parent == (XMLTreeInfo *) NULL)
    for (i=0; root->processing_instructions[i] != (char **) NULL; i++)
    {
      /*
        Post-root processing instructions.
      */
      for (k=2; root->processing_instructions[i][k-1]; k++) ;
      p=root->processing_instructions[i][1];
      for (j=1; p != (char *) NULL; j++)
      {
        if (root->processing_instructions[i][k][j-1] == '<')
          {
            p=root->processing_instructions[i][j];
            continue;
          }
        q=root->processing_instructions[i][0];
        if ((length+strlen(p)+strlen(q)+MaxTextExtent) > extent)
          {
            extent=length+strlen(p)+strlen(q)+MaxTextExtent;
            xml=(char *) ResizeQuantumMemory(xml,extent,sizeof(*xml));
            if (xml == (char *) NULL)
              return(xml);
          }
        length+=FormatLocaleString(xml+length,extent,"\n<?%s%s%s?>",q,
          *p != '\0' ? " " : "",p);
        p=root->processing_instructions[i][j];
      }
    }
  return((char *) ResizeQuantumMemory(xml,length+1,sizeof(*xml)));
}

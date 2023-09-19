/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private xml-tree methods.
*/
#ifndef MAGICKCORE_XML_TREE_PRIVATE_H
#define MAGICKCORE_XML_TREE_PRIVATE_H

#include "MagickCore/locale_.h"
#include "MagickCore/memory_.h"
#include "MagickCore/string_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/xml-tree.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline char *SubstituteXMLEntities(const char *content,
  const MagickBooleanType pedantic)
{
  char
    *canonical_content;

  const char
    *p;

  size_t
    extent;

  ssize_t
    i;

  /*
    Substitute predefined entities.
  */
  i=0;
  canonical_content=AcquireString((char *) NULL);
  extent=MagickPathExtent;
  for (p=content; *p != '\0'; p++)
  {
    if ((i+MagickPathExtent) > (ssize_t) extent)
      {
        extent+=MagickPathExtent;
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
  return(canonical_content);
}

extern MagickPrivate char
  *CanonicalXMLContent(const char *,const MagickBooleanType),
  *FileToXML(const char *,const size_t);

extern MagickPrivate const char
  **GetXMLTreeProcessingInstructions(XMLTreeInfo *,const char *);

extern MagickPrivate MagickBooleanType
  GetXMLTreeAttributes(const XMLTreeInfo *,SplayTreeInfo *);

extern MagickPrivate XMLTreeInfo
  *AddPathToXMLTree(XMLTreeInfo *,const char *,const size_t),
  *GetXMLTreeOrdered(XMLTreeInfo *),
  *GetXMLTreePath(XMLTreeInfo *,const char *),
  *InsertTagIntoXMLTree(XMLTreeInfo *,XMLTreeInfo *,const size_t),
  *ParseTagFromXMLTree(XMLTreeInfo *),
  *PruneTagFromXMLTree(XMLTreeInfo *),
  *SetXMLTreeAttribute(XMLTreeInfo *,const char *,const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

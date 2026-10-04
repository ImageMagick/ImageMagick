/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/license/

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

extern MagickPrivate char
  *FileToXML(const char *,const size_t);

extern MagickPrivate MagickBooleanType
  SkipXMLDocType(const char **);

extern MagickPrivate void
  SkipXMLComment(const char **);

extern MagickExport char
  *SubstituteXMLEntities(const char *,const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

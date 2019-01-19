/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore splay-tree methods.
*/
#ifndef MAGICKCORE_SPLAY_H
#define MAGICKCORE_SPLAY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SplayTreeInfo
  SplayTreeInfo;

extern MagickExport MagickBooleanType
  AddValueToSplayTree(SplayTreeInfo *,const void *,const void *),
  DeleteNodeByValueFromSplayTree(SplayTreeInfo *,const void *),
  DeleteNodeFromSplayTree(SplayTreeInfo *,const void *);

extern MagickExport const void
  *GetNextKeyInSplayTree(SplayTreeInfo *),
  *GetNextValueInSplayTree(SplayTreeInfo *),
  *GetRootValueFromSplayTree(SplayTreeInfo *),
  *GetValueFromSplayTree(SplayTreeInfo *,const void *);

extern MagickExport int
  CompareSplayTreeString(const void *,const void *),
  CompareSplayTreeStringInfo(const void *,const void *);

extern MagickExport SplayTreeInfo
  *CloneSplayTree(SplayTreeInfo *,void *(*)(void *),void *(*)(void *)),
  *DestroySplayTree(SplayTreeInfo *),
  *NewSplayTree(int (*)(const void *,const void *),void *(*)(void *),
    void *(*)(void *));

extern MagickExport size_t
  GetNumberOfNodesInSplayTree(const SplayTreeInfo *);

extern MagickExport void
  *RemoveNodeByValueFromSplayTree(SplayTreeInfo *,const void *),
  *RemoveNodeFromSplayTree(SplayTreeInfo *,const void *),
  ResetSplayTree(SplayTreeInfo *),
  ResetSplayTreeIterator(SplayTreeInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

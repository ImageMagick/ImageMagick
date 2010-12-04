/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore hash methods.
*/
#ifndef _MAGICKCORE_HASHMAP_H
#define _MAGICKCORE_HASHMAP_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define SmallHashmapSize  17
#define MediumHashmapSize  509
#define LargeHashmapSize  8191
#define HugeHashmapSize  131071

typedef struct _HashmapInfo
  HashmapInfo;

typedef struct _LinkedListInfo
  LinkedListInfo;

extern MagickExport HashmapInfo
  *DestroyHashmap(HashmapInfo *),
  *NewHashmap(const size_t,size_t (*)(const void *),MagickBooleanType (*)
    (const void *,const void *),void *(*)(void *),void *(*)(void *));

extern MagickExport LinkedListInfo
  *DestroyLinkedList(LinkedListInfo *,void *(*)(void *)),
  *NewLinkedList(const size_t);

extern MagickExport MagickBooleanType
  AppendValueToLinkedList(LinkedListInfo *,const void *),
  CompareHashmapString(const void *,const void *),
  CompareHashmapStringInfo(const void *,const void *),
  InsertValueInLinkedList(LinkedListInfo *,const size_t,const void *),
  InsertValueInSortedLinkedList(LinkedListInfo *,
    int (*)(const void *,const void *),void **,const void *),
  IsHashmapEmpty(const HashmapInfo *),
  IsLinkedListEmpty(const LinkedListInfo *),
  LinkedListToArray(LinkedListInfo *,void **),
  PutEntryInHashmap(HashmapInfo *,const void *,const void *);

extern MagickExport size_t
  GetNumberOfElementsInLinkedList(const LinkedListInfo *),
  GetNumberOfEntriesInHashmap(const HashmapInfo *),
  HashPointerType(const void *),
  HashStringType(const void *),
  HashStringInfoType(const void *);

extern MagickExport void
  ClearLinkedList(LinkedListInfo *,void *(*)(void *)),
  *GetLastValueInLinkedList(LinkedListInfo *),
  *GetNextKeyInHashmap(HashmapInfo *),
  *GetNextValueInHashmap(HashmapInfo *),
  *GetNextValueInLinkedList(LinkedListInfo *),
  *GetValueFromHashmap(HashmapInfo *,const void *),
  *GetValueFromLinkedList(LinkedListInfo *,const size_t),
  *RemoveElementByValueFromLinkedList(LinkedListInfo *,const void *),
  *RemoveElementFromLinkedList(LinkedListInfo *,const size_t),
  *RemoveEntryFromHashmap(HashmapInfo *,const void *),
  *RemoveLastElementFromLinkedList(LinkedListInfo *),
  ResetHashmapIterator(HashmapInfo *),
  ResetLinkedListIterator(LinkedListInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

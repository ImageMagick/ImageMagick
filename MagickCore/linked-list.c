/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L      IIIII  N   N  K   K  EEEEE  DDDD      L      IIIII  SSSSS  TTTTT   %
%   L        I    NN  N  K  K   E      D   D     L        I    SS       T     %
%   L        I    N N N  KKK    EEE    D   D  =  L        I     SSS     T     %
%   L        I    N  NN  K  K   E      D   D     L        I       SS    T     %
%   LLLLL  IIIII  N   N  K   K  EEEEE  DDDD      LLLLL  IIIII  SSSSS    T     %
%                                                                             %
%                                                                             %
%                  MagickCore Linked-list Methods                             %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               December 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
%  This module implements the standard handy hash and linked-list methods for
%  storing and retrieving large numbers of data elements.  It is loosely based
%  on the Java implementation of these algorithms.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/linked-list-private.h"
#include "MagickCore/locale_.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"

/*
  Typedef declarations.
*/
struct _LinkedListInfo
{
  size_t
    capacity,
    elements;

  ElementInfo
    *head,
    *tail,
    *next;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A p p e n d V a l u e T o L i n k e d L i s t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AppendValueToLinkedList() appends a value to the end of the linked-list.
%
%  The format of the AppendValueToLinkedList method is:
%
%      MagickBooleanType AppendValueToLinkedList(LinkedListInfo *list_info,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
%    o value: the value.
%
*/
MagickExport MagickBooleanType AppendValueToLinkedList(
  LinkedListInfo *list_info,const void *value)
{
  ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if (list_info->elements == list_info->capacity)
    return(MagickFalse);
  next=(ElementInfo *) AcquireMagickMemory(sizeof(*next));
  if (next == (ElementInfo *) NULL)
    return(MagickFalse);
  next->value=(void *) value;
  next->next=(ElementInfo *) NULL;
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->next == (ElementInfo *) NULL)
    list_info->next=next;
  if (list_info->elements == 0)
    list_info->head=next;
  else
    list_info->tail->next=next;
  list_info->tail=next;
  list_info->elements++;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l e a r L i n k e d L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearLinkedList() clears all the elements from the linked-list.
%
%  The format of the ClearLinkedList method is:
%
%      void ClearLinkedList(LinkedListInfo *list_info,
%        void *(*relinquish_value)(void *))
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
%    o relinquish_value: the value deallocation method; typically
%      RelinquishMagickMemory().
%
*/
MagickExport void ClearLinkedList(LinkedListInfo *list_info,
  void *(*relinquish_value)(void *))
{
  ElementInfo
    *element;

  ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  LockSemaphoreInfo(list_info->semaphore);
  next=list_info->head;
  while (next != (ElementInfo *) NULL)
  {
    if (relinquish_value != (void *(*)(void *)) NULL)
      next->value=relinquish_value(next->value);
    element=next;
    next=next->next;
    element=(ElementInfo *) RelinquishMagickMemory(element);
  }
  list_info->head=(ElementInfo *) NULL;
  list_info->tail=(ElementInfo *) NULL;
  list_info->next=(ElementInfo *) NULL;
  list_info->elements=0;
  UnlockSemaphoreInfo(list_info->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y L i n k e d L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyLinkedList() frees the linked-list and all associated resources.
%
%  The format of the DestroyLinkedList method is:
%
%      LinkedListInfo *DestroyLinkedList(LinkedListInfo *list_info,
%        void *(*relinquish_value)(void *))
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
%    o relinquish_value: the value deallocation method;  typically
%      RelinquishMagickMemory().
%
*/
MagickExport LinkedListInfo *DestroyLinkedList(LinkedListInfo *list_info,
  void *(*relinquish_value)(void *))
{
  ElementInfo
    *entry;

  ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  LockSemaphoreInfo(list_info->semaphore);
  for (next=list_info->head; next != (ElementInfo *) NULL; )
  {
    if (relinquish_value != (void *(*)(void *)) NULL)
      next->value=relinquish_value(next->value);
    entry=next;
    next=next->next;
    entry=(ElementInfo *) RelinquishMagickMemory(entry);
  }
  list_info->signature=(~MagickCoreSignature);
  UnlockSemaphoreInfo(list_info->semaphore);
  RelinquishSemaphoreInfo(&list_info->semaphore);
  list_info=(LinkedListInfo *) RelinquishMagickMemory(list_info);
  return(list_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t H e a d E l e m e n t I n L i n k e d L i s t                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetHeadElementInLinkedList() gets the head element in the linked-list.
%
%  The format of the GetHeadElementInLinkedList method is:
%
%      ElementInfo *GetHeadElementInLinkedList(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: the linked_list info.
%
*/
MagickPrivate ElementInfo *GetHeadElementInLinkedList(
  LinkedListInfo *list_info)
{
  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  return(list_info->head);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L a s t V a l u e I n L i n k e d L i s t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLastValueInLinkedList() gets the last value in the linked-list.
%
%  The format of the GetLastValueInLinkedList method is:
%
%      void *GetLastValueInLinkedList(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: the linked_list info.
%
*/
MagickExport void *GetLastValueInLinkedList(LinkedListInfo *list_info)
{
  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if (list_info->elements == 0)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  value=list_info->tail->value;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t V a l u e I n L i n k e d L i s t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextValueInLinkedList() gets the next value in the linked-list.
%
%  The format of the GetNextValueInLinkedList method is:
%
%      void *GetNextValueInLinkedList(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
*/
MagickExport void *GetNextValueInLinkedList(LinkedListInfo *list_info)
{
  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->next == (ElementInfo *) NULL)
    {
      UnlockSemaphoreInfo(list_info->semaphore);
      return((void *) NULL);
    }
  value=list_info->next->value;
  list_info->next=list_info->next->next;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N u m b e r O f E l e m e n t s I n L i n k e d L i s t             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNumberOfElementsInLinkedList() returns the number of entries in the
%  linked-list.
%
%  The format of the GetNumberOfElementsInLinkedList method is:
%
%      size_t GetNumberOfElementsInLinkedList(
%        const LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
*/
MagickExport size_t GetNumberOfElementsInLinkedList(
  const LinkedListInfo *list_info)
{
  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  return(list_info->elements);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V a l u e F r o m L i n k e d L i s t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetValueFromLinkedList() gets a value from the linked-list at the specified
%  location.
%
%  The format of the GetValueFromLinkedList method is:
%
%      void *GetValueFromLinkedList(LinkedListInfo *list_info,
%        const size_t index)
%
%  A description of each parameter follows:
%
%    o list_info: the linked_list info.
%
%    o index: the list index.
%
*/
MagickExport void *GetValueFromLinkedList(LinkedListInfo *list_info,
  const size_t index)
{
  ElementInfo
    *next;

  ssize_t
    i;

  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if (index >= list_info->elements)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (index == 0)
    {
      value=list_info->head->value;
      UnlockSemaphoreInfo(list_info->semaphore);
      return(value);
    }
  if (index == (list_info->elements-1))
    {
      value=list_info->tail->value;
      UnlockSemaphoreInfo(list_info->semaphore);
      return(value);
    }
  next=list_info->head;
  for (i=0; i < (ssize_t) index; i++)
    next=next->next;
  value=next->value;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n s e r t V a l u e I n L i n k e d L i s t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InsertValueInLinkedList() inserts an element in the linked-list at the
%  specified location.
%
%  The format of the InsertValueInLinkedList method is:
%
%      MagickBooleanType InsertValueInLinkedList(ListInfo *list_info,
%        const size_t index,const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: the hashmap info.
%
%    o index: the index.
%
%    o value: the value.
%
*/
MagickExport MagickBooleanType InsertValueInLinkedList(
  LinkedListInfo *list_info,const size_t index,const void *value)
{
  ElementInfo
    *next;

  ssize_t
    i;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if (value == (const void *) NULL)
    return(MagickFalse);
  if ((index > list_info->elements) ||
      (list_info->elements == list_info->capacity))
    return(MagickFalse);
  next=(ElementInfo *) AcquireMagickMemory(sizeof(*next));
  if (next == (ElementInfo *) NULL)
    return(MagickFalse);
  next->value=(void *) value;
  next->next=(ElementInfo *) NULL;
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->elements == 0)
    {
      if (list_info->next == (ElementInfo *) NULL)
        list_info->next=next;
      list_info->head=next;
      list_info->tail=next;
    }
  else
    {
      if (index == 0)
        {
          if (list_info->next == list_info->head)
            list_info->next=next;
          next->next=list_info->head;
          list_info->head=next;
        }
      else
        if (index == list_info->elements)
          {
            if (list_info->next == (ElementInfo *) NULL)
              list_info->next=next;
            list_info->tail->next=next;
            list_info->tail=next;
          }
        else
          {
            ElementInfo
              *element;

            element=list_info->head;
            next->next=element->next;
            for (i=1; i < (ssize_t) index; i++)
            {
              element=element->next;
              next->next=element->next;
            }
            next=next->next;
            element->next=next;
            if (list_info->next == next->next)
              list_info->next=next;
          }
    }
  list_info->elements++;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n s e r t V a l u e I n S o r t e d L i n k e d L i s t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InsertValueInSortedLinkedList() inserts a value in the sorted linked-list.
%
%  The format of the InsertValueInSortedLinkedList method is:
%
%      MagickBooleanType InsertValueInSortedLinkedList(ListInfo *list_info,
%        int (*compare)(const void *,const void *),void **replace,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: the hashmap info.
%
%    o index: the index.
%
%    o compare: the compare method.
%
%    o replace: return previous element here.
%
%    o value: the value.
%
*/
MagickExport MagickBooleanType InsertValueInSortedLinkedList(
  LinkedListInfo *list_info,int (*compare)(const void *,const void *),
  void **replace,const void *value)
{
  ElementInfo
    *element;

  ElementInfo
    *next;

  ssize_t
    i;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if ((compare == (int (*)(const void *,const void *)) NULL) ||
      (value == (const void *) NULL))
    return(MagickFalse);
  if (list_info->elements == list_info->capacity)
    return(MagickFalse);
  next=(ElementInfo *) AcquireMagickMemory(sizeof(*next));
  if (next == (ElementInfo *) NULL)
    return(MagickFalse);
  next->value=(void *) value;
  element=(ElementInfo *) NULL;
  LockSemaphoreInfo(list_info->semaphore);
  next->next=list_info->head;
  while (next->next != (ElementInfo *) NULL)
  {
    i=(ssize_t) compare(value,next->next->value);
    if ((i < 0) || ((replace != (void **) NULL) && (i == 0)))
      {
        if (i == 0)
          {
            *replace=next->next->value;
            next->next=next->next->next;
            if (element != (ElementInfo *) NULL)
              element->next=(ElementInfo *) RelinquishMagickMemory(
                element->next);
            list_info->elements--;
          }
        if (element != (ElementInfo *) NULL)
          element->next=next;
        else
          list_info->head=next;
        break;
      }
    element=next->next;
    next->next=next->next->next;
  }
  if (next->next == (ElementInfo *) NULL)
    {
      if (element != (ElementInfo *) NULL)
        element->next=next;
      else
        list_info->head=next;
      list_info->tail=next;
    }
  list_info->elements++;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s L i n k e d L i s t E m p t y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsLinkedListEmpty() returns MagickTrue if the linked-list is empty.
%
%  The format of the IsLinkedListEmpty method is:
%
%      MagickBooleanType IsLinkedListEmpty(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
*/
MagickExport MagickBooleanType IsLinkedListEmpty(
  const LinkedListInfo *list_info)
{
  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  return(list_info->elements == 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i n k e d L i s t T o A r r a y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LinkedListToArray() converts the linked-list to an array.
%
%  The format of the LinkedListToArray method is:
%
%      MagickBooleanType LinkedListToArray(LinkedListInfo *list_info,
%        void **array)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
%    o array: the array.
%
*/
MagickExport MagickBooleanType LinkedListToArray(LinkedListInfo *list_info,
  void **array)
{
  ElementInfo
    *next;

  ssize_t
    i;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if (array == (void **) NULL)
    return(MagickFalse);
  LockSemaphoreInfo(list_info->semaphore);
  next=list_info->head;
  for (i=0; next != (ElementInfo *) NULL; i++)
  {
    array[i]=next->value;
    next=next->next;
  }
  UnlockSemaphoreInfo(list_info->semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w L i n k e d L i s t I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewLinkedList() returns a pointer to a LinkedListInfo structure
%  initialized to default values.
%
%  The format of the NewLinkedList method is:
%
%      LinkedListInfo *NewLinkedList(const size_t capacity)
%
%  A description of each parameter follows:
%
%    o capacity: the maximum number of elements in the list.
%
*/
MagickExport LinkedListInfo *NewLinkedList(const size_t capacity)
{
  LinkedListInfo
    *list_info;

  list_info=(LinkedListInfo *) AcquireCriticalMemory(sizeof(*list_info));
  (void) memset(list_info,0,sizeof(*list_info));
  list_info->capacity=capacity == 0 ? (size_t) (~0) : capacity;
  list_info->elements=0;
  list_info->head=(ElementInfo *) NULL;
  list_info->tail=(ElementInfo *) NULL;
  list_info->next=(ElementInfo *) NULL;
  list_info->semaphore=AcquireSemaphoreInfo();
  list_info->signature=MagickCoreSignature;
  return(list_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e E l e m e n t B y V a l u e F r o m L i n k e d L i s t       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveElementByValueFromLinkedList() removes an element from the linked-list
%  by value.
%
%  The format of the RemoveElementByValueFromLinkedList method is:
%
%      void *RemoveElementByValueFromLinkedList(LinkedListInfo *list_info,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: the list info.
%
%    o value: the value.
%
*/
MagickExport void *RemoveElementByValueFromLinkedList(LinkedListInfo *list_info,
  const void *value)
{
  ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if ((list_info->elements == 0) || (value == (const void *) NULL))
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (value == list_info->head->value)
    {
      if (list_info->next == list_info->head)
        list_info->next=list_info->head->next;
      next=list_info->head;
      list_info->head=list_info->head->next;
      next=(ElementInfo *) RelinquishMagickMemory(next);
    }
  else
    {
      ElementInfo
        *element;

      next=list_info->head;
      while ((next->next != (ElementInfo *) NULL) &&
             (next->next->value != value))
        next=next->next;
      if (next->next == (ElementInfo *) NULL)
        {
          UnlockSemaphoreInfo(list_info->semaphore);
          return((void *) NULL);
        }
      element=next->next;
      next->next=element->next;
      if (element == list_info->tail)
        list_info->tail=next;
      if (list_info->next == element)
        list_info->next=element->next;
      element=(ElementInfo *) RelinquishMagickMemory(element);
    }
  list_info->elements--;
  UnlockSemaphoreInfo(list_info->semaphore);
  return((void *) value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e E l e m e n t F r o m L i n k e d L i s t                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveElementFromLinkedList() removes an element from the linked-list at the
%  specified location.
%
%  The format of the RemoveElementFromLinkedList method is:
%
%      void *RemoveElementFromLinkedList(LinkedListInfo *list_info,
%        const size_t index)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
%    o index: the index.
%
*/
MagickExport void *RemoveElementFromLinkedList(LinkedListInfo *list_info,
  const size_t index)
{
  ElementInfo
    *next;

  ssize_t
    i;

  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if (index >= list_info->elements)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (index == 0)
    {
      if (list_info->next == list_info->head)
        list_info->next=list_info->head->next;
      value=list_info->head->value;
      next=list_info->head;
      list_info->head=list_info->head->next;
      next=(ElementInfo *) RelinquishMagickMemory(next);
    }
  else
    {
      ElementInfo
        *element;

      next=list_info->head;
      for (i=1; i < (ssize_t) index; i++)
        next=next->next;
      element=next->next;
      next->next=element->next;
      if (element == list_info->tail)
        list_info->tail=next;
      if (list_info->next == element)
        list_info->next=element->next;
      value=element->value;
      element=(ElementInfo *) RelinquishMagickMemory(element);
    }
  list_info->elements--;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e L a s t E l e m e n t F r o m L i n k e d L i s t             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveLastElementFromLinkedList() removes the last element from the
%  linked-list.
%
%  The format of the RemoveLastElementFromLinkedList method is:
%
%      void *RemoveLastElementFromLinkedList(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
*/
MagickExport void *RemoveLastElementFromLinkedList(LinkedListInfo *list_info)
{
  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  if (list_info->elements == 0)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->next == list_info->tail)
    list_info->next=(ElementInfo *) NULL;
  if (list_info->elements == 1UL)
    {
      value=list_info->head->value;
      list_info->head=(ElementInfo *) NULL;
      list_info->tail=(ElementInfo *) RelinquishMagickMemory(list_info->tail);
    }
  else
    {
      ElementInfo
        *next;

      value=list_info->tail->value;
      next=list_info->head;
      while (next->next != list_info->tail)
        next=next->next;
      list_info->tail=(ElementInfo *) RelinquishMagickMemory(list_info->tail);
      list_info->tail=next;
      next->next=(ElementInfo *) NULL;
    }
  list_info->elements--;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t L i n k e d L i s t I t e r a t o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetLinkedListIterator() resets the lined-list iterator.  Use it in
%  conjunction with GetNextValueInLinkedList() to iterate over all the values
%  in the linked-list.
%
%  The format of the ResetLinkedListIterator method is:
%
%      ResetLinkedListIterator(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
*/
MagickExport void ResetLinkedListIterator(LinkedListInfo *list_info)
{
  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  LockSemaphoreInfo(list_info->semaphore);
  list_info->next=list_info->head;
  UnlockSemaphoreInfo(list_info->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t H e a d E l e m e n t I n L i n k e d L i s t                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetHeadElementInLinkedList() sets the head element of the linked-list.
%
%  The format of the SetHeadElementInLinkedList method is:
%
%      SetHeadElementInLinkedList(LinkedListInfo *list_info,
%        ElementInfo *element)
%
%  A description of each parameter follows:
%
%    o list_info: the linked-list info.
%
%    o element: the element to set as the head.
%
*/
MagickPrivate void SetHeadElementInLinkedList(LinkedListInfo *list_info,
  ElementInfo *element)
{
  ElementInfo
    *prev;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == MagickCoreSignature);
  assert(element != (ElementInfo *) NULL);
  if (element == list_info->head)
    return;
  prev=list_info->head;
  while (prev != (ElementInfo *) NULL)
  {
    if (prev->next == element)
      {
        prev->next=element->next;
        element->next=list_info->head;
        if (list_info->head == list_info->next)
          list_info->next=element;
        list_info->head=element;
        break;
      }
    prev=prev->next;
  }
}

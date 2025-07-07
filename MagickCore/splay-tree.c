/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      SSSSS  PPPP   L       AAA   Y   Y                      %
%                      SS     P   P  L      A   A   Y Y                       %
%                       SSS   PPPP   L      AAAAA    Y                        %
%                         SS  P      L      A   A    Y                        %
%                      SSSSS  P      LLLLL  A   A    Y                        %
%                                                                             %
%                         TTTTT  RRRR   EEEEE  EEEEE                          %
%                           T    R   R  E      E                              %
%                           T    RRRR   EEE    EEE                            %
%                           T    R R    E      E                              %
%                           T    R  R   EEEEE  EEEEE                          %
%                                                                             %
%                                                                             %
%             MagickCore Self-adjusting Binary Search Tree Methods            %
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
%  This module implements the standard handy splay-tree methods for storing and
%  retrieving large numbers of data elements.  It is loosely based on the Java
%  implementation of these algorithms.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"

/*
  Define declarations.
*/
#define MaxSplayTreeDepth  1024

/*
  Typedef declarations.
*/
typedef struct _NodeInfo
{
  void
    *key;

  void
    *value;

  struct _NodeInfo
    *left,
    *right;
} NodeInfo;

struct _SplayTreeInfo
{
  NodeInfo
    *root;

  int
    (*compare)(const void *,const void *);

  void
    *(*relinquish_key)(void *),
    *(*relinquish_value)(void *);

  MagickBooleanType
    balance;

  void
    *key,
    *next;

  size_t
    nodes;

  MagickBooleanType
    debug;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

/*
  Forward declarations.
*/
static int
  IterateOverSplayTree(SplayTreeInfo *,int (*)(NodeInfo *,const void *),
    const void *);

static void
  SplaySplayTree(SplayTreeInfo *,const void *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A d d V a l u e T o S p l a y T r e e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AddValueToSplayTree() adds the given key and value to the splay-tree.  Both
%  key and value are used as is, without coping or cloning.  It returns
%  MagickTrue on success, otherwise MagickFalse.
%
%  The format of the AddValueToSplayTree method is:
%
%      MagickBooleanType AddValueToSplayTree(SplayTreeInfo *splay_tree,
%        const void *key,const void *value)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o key: the key.
%
%    o value: the value.
%
*/
MagickExport MagickBooleanType AddValueToSplayTree(SplayTreeInfo *splay_tree,
  const void *key,const void *value)
{
  int
    compare;

  NodeInfo
    *node;

  LockSemaphoreInfo(splay_tree->semaphore);
  SplaySplayTree(splay_tree,key);
  compare=0;
  if (splay_tree->root != (NodeInfo *) NULL)
    {
      if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
        compare=splay_tree->compare(splay_tree->root->key,key);
      else
        compare=(splay_tree->root->key > key) ? 1 :
          ((splay_tree->root->key < key) ? -1 : 0);
      if (compare == 0)
        {
          if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
              (splay_tree->root->value != (void *) NULL))
            splay_tree->root->value=splay_tree->relinquish_value(
              splay_tree->root->value);
          if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
              (splay_tree->root->key != (void *) NULL))
            splay_tree->root->key=splay_tree->relinquish_key(
              splay_tree->root->key);
          splay_tree->root->key=(void *) key;
          splay_tree->root->value=(void *) value;
          UnlockSemaphoreInfo(splay_tree->semaphore);
          return(MagickTrue);
        }
    }
  node=(NodeInfo *) AcquireMagickMemory(sizeof(*node));
  if (node == (NodeInfo *) NULL)
    {
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return(MagickFalse);
    }
  node->key=(void *) key;
  node->value=(void *) value;
  if (splay_tree->root == (NodeInfo *) NULL)
    {
      node->left=(NodeInfo *) NULL;
      node->right=(NodeInfo *) NULL;
    }
  else
    if (compare < 0)
      {
        node->left=splay_tree->root;
        node->right=node->left->right;
        node->left->right=(NodeInfo *) NULL;
      }
    else
      {
        node->right=splay_tree->root;
        node->left=node->right->left;
        node->right->left=(NodeInfo *) NULL;
      }
  splay_tree->root=node;
  splay_tree->key=(void *) NULL;
  splay_tree->nodes++;
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B a l a n c e S p l a y T r e e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BalanceSplayTree() balances the splay-tree.
%
%  The format of the BalanceSplayTree method is:
%
%      void *BalanceSplayTree(SplayTreeInfo *splay_tree,const void *key)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o key: the key.
%
*/

static NodeInfo *LinkSplayTreeNodes(NodeInfo **nodes,const size_t low,
  const size_t high)
{
  NodeInfo
    *node;

  size_t
    bisect;

  bisect=low+(high-low)/2;
  node=nodes[bisect];
  if ((low+1) > bisect)
    node->left=(NodeInfo *) NULL;
  else
    node->left=LinkSplayTreeNodes(nodes,low,bisect-1);
  if ((bisect+1) > high)
    node->right=(NodeInfo *) NULL;
  else
    node->right=LinkSplayTreeNodes(nodes,bisect+1,high);
  return(node);
}

static inline int SplayTreeToNodeArray(NodeInfo *node,const void *nodes)
{
  const NodeInfo
    ***p;

  p=(const NodeInfo ***) nodes;
  *(*p)=node;
  (*p)++;
  return(0);
}

static void BalanceSplayTree(SplayTreeInfo *splay_tree)
{
  NodeInfo
    **node,
    **nodes;

  if (splay_tree->nodes <= 2)
    {
      splay_tree->balance=MagickFalse;
      return;
    }
  nodes=(NodeInfo **) AcquireQuantumMemory((size_t) splay_tree->nodes,
    sizeof(*nodes));
  if (nodes == (NodeInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  node=nodes;
  (void) IterateOverSplayTree(splay_tree,SplayTreeToNodeArray,(const void *)
    &node);
  splay_tree->root=LinkSplayTreeNodes(nodes,0,splay_tree->nodes-1);
  splay_tree->balance=MagickFalse;
  nodes=(NodeInfo **) RelinquishMagickMemory(nodes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e S p l a y T r e e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneSplayTree() clones the splay tree.
%
%  The format of the CloneSplayTree method is:
%
%      SplayTreeInfo *CloneSplayTree(SplayTreeInfo *splay_tree,
%        void *(*clone_key)(void *),void *(*clone_value)(void *))
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
%    o clone_key: the key clone method, typically ConstantString(), called
%      whenever a key is added to the splay-tree.
%
%    o clone_value: the value clone method;  typically ConstantString(), called
%      whenever a value object is added to the splay-tree.
%
*/

static inline void *GetFirstSplayTreeNode(SplayTreeInfo *splay_tree)
{
  NodeInfo
    *node;

  node=splay_tree->root;
  if (splay_tree->root == (NodeInfo *) NULL)
    return((NodeInfo *) NULL);
  while (node->left != (NodeInfo *) NULL)
    node=node->left;
  return(node->key);
}

MagickExport SplayTreeInfo *CloneSplayTree(SplayTreeInfo *splay_tree,
  void *(*clone_key)(void *),void *(*clone_value)(void *))
{
  NodeInfo
    *next,
    *node;

  SplayTreeInfo
    *clone_tree;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  clone_tree=NewSplayTree(splay_tree->compare,splay_tree->relinquish_key,
    splay_tree->relinquish_value);
  LockSemaphoreInfo(splay_tree->semaphore);
  if (splay_tree->root == (NodeInfo *) NULL)
    {
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return(clone_tree);
    }
  next=(NodeInfo *) GetFirstSplayTreeNode(splay_tree);
  while (next != (NodeInfo *) NULL)
  {
    SplaySplayTree(splay_tree,next);
    (void) AddValueToSplayTree(clone_tree,clone_key(splay_tree->root->key),
      clone_value(splay_tree->root->value));
    next=(NodeInfo *) NULL;
    node=splay_tree->root->right;
    if (node != (NodeInfo *) NULL)
      {
        while (node->left != (NodeInfo *) NULL)
          node=node->left;
        next=(NodeInfo *) node->key;
      }
  }
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(clone_tree);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e S p l a y T r e e S t r i n g                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareSplayTreeString() method finds a node in a splay-tree based on the
%  contents of a string.
%
%  The format of the CompareSplayTreeString method is:
%
%      int CompareSplayTreeString(const void *target,const void *source)
%
%  A description of each parameter follows:
%
%    o target: the target string.
%
%    o source: the source string.
%
*/
MagickExport int CompareSplayTreeString(const void *target,const void *source)
{
  const char
    *p,
    *q;

  p=(const char *) target;
  q=(const char *) source;
  return(LocaleCompare(p,q));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e S p l a y T r e e S t r i n g I n f o                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareSplayTreeStringInfo() finds a node in a splay-tree based on the
%  contents of a string.
%
%  The format of the CompareSplayTreeStringInfo method is:
%
%      int CompareSplayTreeStringInfo(const void *target,const void *source)
%
%  A description of each parameter follows:
%
%    o target: the target string.
%
%    o source: the source string.
%
*/
MagickExport int CompareSplayTreeStringInfo(const void *target,
  const void *source)
{
  const StringInfo
    *p,
    *q;

  p=(const StringInfo *) target;
  q=(const StringInfo *) source;
  return(CompareStringInfo(p,q));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e N o d e B y V a l u e F r o m S p l a y T r e e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteNodeByValueFromSplayTree() deletes a node by value from the
%  splay-tree.
%
%  The format of the DeleteNodeByValueFromSplayTree method is:
%
%      MagickBooleanType DeleteNodeByValueFromSplayTree(
%        SplayTreeInfo *splay_tree,const void *value)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o value: the value.
%
*/
MagickExport MagickBooleanType DeleteNodeByValueFromSplayTree(
  SplayTreeInfo *splay_tree,const void *value)
{
  NodeInfo
    *next,
    *node;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  LockSemaphoreInfo(splay_tree->semaphore);
  if (splay_tree->root == (NodeInfo *) NULL)
    {
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return(MagickFalse);
    }
  next=(NodeInfo *) GetFirstSplayTreeNode(splay_tree);
  while (next != (NodeInfo *) NULL)
  {
    SplaySplayTree(splay_tree,next);
    next=(NodeInfo *) NULL;
    node=splay_tree->root->right;
    if (node != (NodeInfo *) NULL)
      {
        while (node->left != (NodeInfo *) NULL)
          node=node->left;
        next=(NodeInfo *) node->key;
      }
    if (splay_tree->root->value == value)
      {
        int
          compare;

        NodeInfo
          *left,
          *right;

        void
          *key;

        /*
          We found the node that matches the value; now delete it.
        */
        key=splay_tree->root->key;
        SplaySplayTree(splay_tree,key);
        splay_tree->key=(void *) NULL;
        if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
          compare=splay_tree->compare(splay_tree->root->key,key);
        else
          compare=(splay_tree->root->key > key) ? 1 :
            ((splay_tree->root->key < key) ? -1 : 0);
        if (compare != 0)
          {
            UnlockSemaphoreInfo(splay_tree->semaphore);
            return(MagickFalse);
          }
        left=splay_tree->root->left;
        right=splay_tree->root->right;
        if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
            (splay_tree->root->value != (void *) NULL))
          splay_tree->root->value=splay_tree->relinquish_value(
            splay_tree->root->value);
        if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
            (splay_tree->root->key != (void *) NULL))
          splay_tree->root->key=splay_tree->relinquish_key(
            splay_tree->root->key);
        splay_tree->root=(NodeInfo *) RelinquishMagickMemory(splay_tree->root);
        splay_tree->nodes--;
        if (left == (NodeInfo *) NULL)
          {
            splay_tree->root=right;
            UnlockSemaphoreInfo(splay_tree->semaphore);
            return(MagickTrue);
          }
        splay_tree->root=left;
        if (right != (NodeInfo *) NULL)
          {
            while (left->right != (NodeInfo *) NULL)
              left=left->right;
            left->right=right;
          }
        UnlockSemaphoreInfo(splay_tree->semaphore);
        return(MagickTrue);
      }
  }
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e N o d e F r o m S p l a y T r e e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteNodeFromSplayTree() deletes a node from the splay-tree.  It returns
%  MagickTrue if the option is found and successfully deleted from the
%  splay-tree.
%
%  The format of the DeleteNodeFromSplayTree method is:
%
%      MagickBooleanType DeleteNodeFromSplayTree(SplayTreeInfo *splay_tree,
%        const void *key)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o key: the key.
%
*/
MagickExport MagickBooleanType DeleteNodeFromSplayTree(
  SplayTreeInfo *splay_tree,const void *key)
{
  int
    compare;

  NodeInfo
    *left,
    *right;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (splay_tree->root == (NodeInfo *) NULL)
    return(MagickFalse);
  LockSemaphoreInfo(splay_tree->semaphore);
  SplaySplayTree(splay_tree,key);
  splay_tree->key=(void *) NULL;
  if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
    compare=splay_tree->compare(splay_tree->root->key,key);
  else
    compare=(splay_tree->root->key > key) ? 1 :
      ((splay_tree->root->key < key) ? -1 : 0);
  if (compare != 0)
    {
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return(MagickFalse);
    }
  left=splay_tree->root->left;
  right=splay_tree->root->right;
  if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
      (splay_tree->root->value != (void *) NULL))
    splay_tree->root->value=splay_tree->relinquish_value(
      splay_tree->root->value);
  if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
      (splay_tree->root->key != (void *) NULL))
    splay_tree->root->key=splay_tree->relinquish_key(splay_tree->root->key);
  splay_tree->root=(NodeInfo *) RelinquishMagickMemory(splay_tree->root);
  splay_tree->nodes--;
  if (left == (NodeInfo *) NULL)
    {
      splay_tree->root=right;
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return(MagickTrue);
    }
  splay_tree->root=left;
  if (right != (NodeInfo *) NULL)
    {
      while (left->right != (NodeInfo *) NULL)
        left=left->right;
      left->right=right;
    }
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S p l a y T r e e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySplayTree() destroys the splay-tree.
%
%  The format of the DestroySplayTree method is:
%
%      SplayTreeInfo *DestroySplayTree(SplayTreeInfo *splay_tree)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
*/
MagickExport SplayTreeInfo *DestroySplayTree(SplayTreeInfo *splay_tree)
{
  NodeInfo
    *active,
    *node,
    *pend;

  LockSemaphoreInfo(splay_tree->semaphore);
  if (splay_tree->root != (NodeInfo *) NULL)
    {
      if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
          (splay_tree->root->value != (void *) NULL))
        splay_tree->root->value=splay_tree->relinquish_value(
          splay_tree->root->value);
      if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
          (splay_tree->root->key != (void *) NULL))
        splay_tree->root->key=splay_tree->relinquish_key(splay_tree->root->key);
      splay_tree->root->key=(void *) NULL;
      for (pend=splay_tree->root; pend != (NodeInfo *) NULL; )
      {
        active=pend;
        for (pend=(NodeInfo *) NULL; active != (NodeInfo *) NULL; )
        {
          if (active->left != (NodeInfo *) NULL)
            {
              if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
                  (active->left->value != (void *) NULL))
                active->left->value=splay_tree->relinquish_value(
                  active->left->value);
              if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
                  (active->left->key != (void *) NULL))
                active->left->key=splay_tree->relinquish_key(active->left->key);
              active->left->key=(void *) pend;
              pend=active->left;
            }
          if (active->right != (NodeInfo *) NULL)
            {
              if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
                  (active->right->value != (void *) NULL))
                active->right->value=splay_tree->relinquish_value(
                  active->right->value);
              if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
                  (active->right->key != (void *) NULL))
                active->right->key=splay_tree->relinquish_key(
                  active->right->key);
              active->right->key=(void *) pend;
              pend=active->right;
            }
          node=active;
          active=(NodeInfo *) node->key;
          node=(NodeInfo *) RelinquishMagickMemory(node);
        }
      }
    }
  splay_tree->signature=(~MagickCoreSignature);
  UnlockSemaphoreInfo(splay_tree->semaphore);
  RelinquishSemaphoreInfo(&splay_tree->semaphore);
  splay_tree=(SplayTreeInfo *) RelinquishMagickMemory(splay_tree);
  return(splay_tree);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t K e y I n S p l a y T r e e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextKeyInSplayTree() gets the next key in the splay-tree.
%
%  The format of the GetNextKeyInSplayTree method is:
%
%      const void *GetNextKeyInSplayTree(SplayTreeInfo *splay_tree)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
%    o key: the key.
%
*/
MagickExport const void *GetNextKeyInSplayTree(SplayTreeInfo *splay_tree)
{
  NodeInfo
    *node;

  void
    *key;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if ((splay_tree->root == (NodeInfo *) NULL) ||
      (splay_tree->next == (void *) NULL))
    return((void *) NULL);
  LockSemaphoreInfo(splay_tree->semaphore);
  SplaySplayTree(splay_tree,splay_tree->next);
  splay_tree->next=(void *) NULL;
  node=splay_tree->root->right;
  if (node != (NodeInfo *) NULL)
    {
      while (node->left != (NodeInfo *) NULL)
        node=node->left;
      splay_tree->next=node->key;
    }
  key=splay_tree->root->key;
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t V a l u e I n S p l a y T r e e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextValueInSplayTree() gets the next value in the splay-tree.
%
%  The format of the GetNextValueInSplayTree method is:
%
%      const void *GetNextValueInSplayTree(SplayTreeInfo *splay_tree)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
%    o key: the key.
%
*/
MagickExport const void *GetNextValueInSplayTree(SplayTreeInfo *splay_tree)
{
  NodeInfo
    *node;

  void
    *value;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if ((splay_tree->root == (NodeInfo *) NULL) ||
      (splay_tree->next == (void *) NULL))
    return((void *) NULL);
  LockSemaphoreInfo(splay_tree->semaphore);
  SplaySplayTree(splay_tree,splay_tree->next);
  splay_tree->next=(void *) NULL;
  node=splay_tree->root->right;
  if (node != (NodeInfo *) NULL)
    {
      while (node->left != (NodeInfo *) NULL)
        node=node->left;
      splay_tree->next=node->key;
    }
  value=splay_tree->root->value;
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t R o o t V a l u e F r o m S p l a y T r e e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRootValueFromSplayTree() gets the root value from the splay-tree.
%
%  The format of the GetRootValueFromSplayTree method is:
%
%      const void *GetRootValueFromSplayTree(SplayTreeInfo *splay_tree)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
%    o key: the key.
%
*/
MagickExport const void *GetRootValueFromSplayTree(SplayTreeInfo *splay_tree)
{
  const void
    *value;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  value=(const void *) NULL;
  LockSemaphoreInfo(splay_tree->semaphore);
  if (splay_tree->root != (NodeInfo *) NULL)
    value=splay_tree->root->value;
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V a l u e F r o m S p l a y T r e e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetValueFromSplayTree() gets a value from the splay-tree by its key.
%
%  Note, the value is a constant.  Do not attempt to free it.
%
%  The format of the GetValueFromSplayTree method is:
%
%      const void *GetValueFromSplayTree(SplayTreeInfo *splay_tree,
%        const void *key)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
%    o key: the key.
%
*/
MagickExport const void *GetValueFromSplayTree(SplayTreeInfo *splay_tree,
  const void *key)
{
  int
    compare;

  void
    *value;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (splay_tree->root == (NodeInfo *) NULL)
    return((void *) NULL);
  LockSemaphoreInfo(splay_tree->semaphore);
  SplaySplayTree(splay_tree,key);
  if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
    compare=splay_tree->compare(splay_tree->root->key,key);
  else
    compare=(splay_tree->root->key > key) ? 1 :
      ((splay_tree->root->key < key) ? -1 : 0);
  if (compare != 0)
    {
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return((void *) NULL);
    }
  value=splay_tree->root->value;
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N u m b e r O f N o d e s I n S p l a y T r e e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNumberOfNodesInSplayTree() returns the number of nodes in the splay-tree.
%
%  The format of the GetNumberOfNodesInSplayTree method is:
%
%      size_t GetNumberOfNodesInSplayTree(
%        const SplayTreeInfo *splay_tree)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
*/
MagickExport size_t GetNumberOfNodesInSplayTree(
  const SplayTreeInfo *splay_tree)
{
  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(splay_tree->nodes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I t e r a t e O v e r S p l a y T r e e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IterateOverSplayTree() iterates over the splay-tree.
%
%  The format of the IterateOverSplayTree method is:
%
%      int IterateOverSplayTree(SplayTreeInfo *splay_tree,
%        int (*method)(NodeInfo *,void *),const void *value)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o method: the method.
%
%    o value: the value.
%
*/
static int IterateOverSplayTree(SplayTreeInfo *splay_tree,
  int (*method)(NodeInfo *,const void *),const void *value)
{
  typedef enum
  {
    LeftTransition,
    RightTransition,
    DownTransition,
    UpTransition
  } TransitionType;

  int
    status;

  MagickBooleanType
    final_transition;

  NodeInfo
    *node,
    **nodes;

  ssize_t
    i;

  TransitionType
    transition;

  unsigned char
    *transitions;

  if (splay_tree->root == (NodeInfo *) NULL)
    return(0);
  nodes=(NodeInfo **) AcquireQuantumMemory((size_t) splay_tree->nodes,
    sizeof(*nodes));
  transitions=(unsigned char *) AcquireQuantumMemory((size_t) splay_tree->nodes,
    sizeof(*transitions));
  if ((nodes == (NodeInfo **) NULL) || (transitions == (unsigned char *) NULL))
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  status=0;
  final_transition=MagickFalse;
  nodes[0]=splay_tree->root;
  transitions[0]=(unsigned char) LeftTransition;
  for (i=0; final_transition == MagickFalse; )
  {
    node=nodes[i];
    transition=(TransitionType) transitions[i];
    switch (transition)
    {
      case LeftTransition:
      {
        transitions[i]=(unsigned char) DownTransition;
        if (node->left == (NodeInfo *) NULL)
          break;
        i++;
        nodes[i]=node->left;
        transitions[i]=(unsigned char) LeftTransition;
        break;
      }
      case RightTransition:
      {
        transitions[i]=(unsigned char) UpTransition;
        if (node->right == (NodeInfo *) NULL)
          break;
        i++;
        nodes[i]=node->right;
        transitions[i]=(unsigned char) LeftTransition;
        break;
      }
      case DownTransition:
      default:
      {
        transitions[i]=(unsigned char) RightTransition;
        status=(*method)(node,value);
        if (status != 0)
          final_transition=MagickTrue;
        break;
      }
      case UpTransition:
      {
        if (i == 0)
          {
            final_transition=MagickTrue;
            break;
          }
        i--;
        break;
      }
    }
  }
  nodes=(NodeInfo **) RelinquishMagickMemory(nodes);
  transitions=(unsigned char *) RelinquishMagickMemory(transitions);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w S p l a y T r e e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewSplayTree() returns a pointer to a SplayTreeInfo structure initialized
%  to default values.
%
%  The format of the NewSplayTree method is:
%
%      SplayTreeInfo *NewSplayTree(int (*compare)(const void *,const void *),
%        void *(*relinquish_key)(void *),void *(*relinquish_value)(void *))
%
%  A description of each parameter follows:
%
%    o compare: the compare method.
%
%    o relinquish_key: the key deallocation method, typically
%      RelinquishMagickMemory(), called whenever a key is removed from the
%      splay-tree.
%
%    o relinquish_value: the value deallocation method;  typically
%      RelinquishMagickMemory(), called whenever a value object is removed from
%      the splay-tree.
%
*/
MagickExport SplayTreeInfo *NewSplayTree(
  int (*compare)(const void *,const void *),void *(*relinquish_key)(void *),
  void *(*relinquish_value)(void *))
{
  SplayTreeInfo
    *splay_tree;

  splay_tree=(SplayTreeInfo *) AcquireCriticalMemory(sizeof(*splay_tree));
  (void) memset(splay_tree,0,sizeof(*splay_tree));
  splay_tree->root=(NodeInfo *) NULL;
  splay_tree->compare=compare;
  splay_tree->relinquish_key=relinquish_key;
  splay_tree->relinquish_value=relinquish_value;
  splay_tree->balance=MagickFalse;
  splay_tree->key=(void *) NULL;
  splay_tree->next=(void *) NULL;
  splay_tree->nodes=0;
  splay_tree->debug=IsEventLogging();
  splay_tree->semaphore=AcquireSemaphoreInfo();
  splay_tree->signature=MagickCoreSignature;
  return(splay_tree);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e N o d e B y V a l u e F r o m S p l a y T r e e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveNodeByValueFromSplayTree() removes a node by value from the splay-tree
%  and returns its key.
%
%  The format of the RemoveNodeByValueFromSplayTree method is:
%
%      void *RemoveNodeByValueFromSplayTree(SplayTreeInfo *splay_tree,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o value: the value.
%
*/
MagickExport void *RemoveNodeByValueFromSplayTree(SplayTreeInfo *splay_tree,
  const void *value)
{
  NodeInfo
    *next,
    *node;

  void
    *key;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  key=(void *) NULL;
  if (splay_tree->root == (NodeInfo *) NULL)
    return(key);
  LockSemaphoreInfo(splay_tree->semaphore);
  next=(NodeInfo *) GetFirstSplayTreeNode(splay_tree);
  while (next != (NodeInfo *) NULL)
  {
    SplaySplayTree(splay_tree,next);
    next=(NodeInfo *) NULL;
    node=splay_tree->root->right;
    if (node != (NodeInfo *) NULL)
      {
        while (node->left != (NodeInfo *) NULL)
          node=node->left;
        next=(NodeInfo *) node->key;
      }
    if (splay_tree->root->value == value)
      {
        int
          compare;

        NodeInfo
          *left,
          *right;

        /*
          We found the node that matches the value; now remove it.
        */
        key=splay_tree->root->key;
        SplaySplayTree(splay_tree,key);
        splay_tree->key=(void *) NULL;
        if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
          compare=splay_tree->compare(splay_tree->root->key,key);
        else
          compare=(splay_tree->root->key > key) ? 1 :
            ((splay_tree->root->key < key) ? -1 : 0);
        if (compare != 0)
          {
            UnlockSemaphoreInfo(splay_tree->semaphore);
            return(key);
          }
        left=splay_tree->root->left;
        right=splay_tree->root->right;
        if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
            (splay_tree->root->value != (void *) NULL))
          splay_tree->root->value=splay_tree->relinquish_value(
            splay_tree->root->value);
        splay_tree->root=(NodeInfo *) RelinquishMagickMemory(splay_tree->root);
        splay_tree->nodes--;
        if (left == (NodeInfo *) NULL)
          {
            splay_tree->root=right;
            UnlockSemaphoreInfo(splay_tree->semaphore);
            return(key);
          }
        splay_tree->root=left;
        if (right != (NodeInfo *) NULL)
          {
            while (left->right != (NodeInfo *) NULL)
              left=left->right;
            left->right=right;
          }
        UnlockSemaphoreInfo(splay_tree->semaphore);
        return(key);
      }
  }
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e N o d e F r o m S p l a y T r e e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveNodeFromSplayTree() removes a node from the splay-tree and returns its
%  value.
%
%  The format of the RemoveNodeFromSplayTree method is:
%
%      void *RemoveNodeFromSplayTree(SplayTreeInfo *splay_tree,const void *key)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o key: the key.
%
*/
MagickExport void *RemoveNodeFromSplayTree(SplayTreeInfo *splay_tree,
  const void *key)
{
  int
    compare;

  NodeInfo
    *left,
    *right;

  void
    *value;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  value=(void *) NULL;
  if (splay_tree->root == (NodeInfo *) NULL)
    return(value);
  LockSemaphoreInfo(splay_tree->semaphore);
  SplaySplayTree(splay_tree,key);
  splay_tree->key=(void *) NULL;
  if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
    compare=splay_tree->compare(splay_tree->root->key,key);
  else
    compare=(splay_tree->root->key > key) ? 1 :
      ((splay_tree->root->key < key) ? -1 : 0);
  if (compare != 0)
    {
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return(value);
    }
  left=splay_tree->root->left;
  right=splay_tree->root->right;
  value=splay_tree->root->value;
  if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
      (splay_tree->root->key != (void *) NULL))
    splay_tree->root->key=splay_tree->relinquish_key(splay_tree->root->key);
  splay_tree->root=(NodeInfo *) RelinquishMagickMemory(splay_tree->root);
  splay_tree->nodes--;
  if (left == (NodeInfo *) NULL)
    {
      splay_tree->root=right;
      UnlockSemaphoreInfo(splay_tree->semaphore);
      return(value);
    }
  splay_tree->root=left;
  if (right != (NodeInfo *) NULL)
    {
      while (left->right != (NodeInfo *) NULL)
        left=left->right;
      left->right=right;
    }
  UnlockSemaphoreInfo(splay_tree->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t S p l a y T r e e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetSplayTree() resets the splay-tree.  That is, it deletes all the nodes
%  from the tree.
%
%  The format of the ResetSplayTree method is:
%
%      ResetSplayTree(SplayTreeInfo *splay_tree)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
*/
MagickExport void ResetSplayTree(SplayTreeInfo *splay_tree)
{
  NodeInfo
    *active,
    *node,
    *pend;

  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  LockSemaphoreInfo(splay_tree->semaphore);
  if (splay_tree->root != (NodeInfo *) NULL)
    {
      if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
          (splay_tree->root->value != (void *) NULL))
        splay_tree->root->value=splay_tree->relinquish_value(
          splay_tree->root->value);
      if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
          (splay_tree->root->key != (void *) NULL))
        splay_tree->root->key=splay_tree->relinquish_key(splay_tree->root->key);
      splay_tree->root->key=(void *) NULL;
      for (pend=splay_tree->root; pend != (NodeInfo *) NULL; )
      {
        active=pend;
        for (pend=(NodeInfo *) NULL; active != (NodeInfo *) NULL; )
        {
          if (active->left != (NodeInfo *) NULL)
            {
              if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
                  (active->left->value != (void *) NULL))
                active->left->value=splay_tree->relinquish_value(
                  active->left->value);
              if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
                  (active->left->key != (void *) NULL))
                active->left->key=splay_tree->relinquish_key(active->left->key);
              active->left->key=(void *) pend;
              pend=active->left;
            }
          if (active->right != (NodeInfo *) NULL)
            {
              if ((splay_tree->relinquish_value != (void *(*)(void *)) NULL) &&
                  (active->right->value != (void *) NULL))
                active->right->value=splay_tree->relinquish_value(
                  active->right->value);
              if ((splay_tree->relinquish_key != (void *(*)(void *)) NULL) &&
                  (active->right->key != (void *) NULL))
                active->right->key=splay_tree->relinquish_key(
                  active->right->key);
              active->right->key=(void *) pend;
              pend=active->right;
            }
          node=active;
          active=(NodeInfo *) node->key;
          node=(NodeInfo *) RelinquishMagickMemory(node);
        }
      }
    }
  splay_tree->root=(NodeInfo *) NULL;
  splay_tree->key=(void *) NULL;
  splay_tree->next=(void *) NULL;
  splay_tree->nodes=0;
  splay_tree->balance=MagickFalse;
  UnlockSemaphoreInfo(splay_tree->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t S p l a y T r e e I t e r a t o r                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetSplayTreeIterator() resets the splay-tree iterator.  Use it in
%  conjunction with GetNextValueInSplayTree() to iterate over all the nodes in
%  the splay-tree.
%
%  The format of the ResetSplayTreeIterator method is:
%
%      ResetSplayTreeIterator(SplayTreeInfo *splay_tree)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay tree.
%
*/
MagickExport void ResetSplayTreeIterator(SplayTreeInfo *splay_tree)
{
  assert(splay_tree != (SplayTreeInfo *) NULL);
  assert(splay_tree->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  LockSemaphoreInfo(splay_tree->semaphore);
  splay_tree->next=GetFirstSplayTreeNode(splay_tree);
  UnlockSemaphoreInfo(splay_tree->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p l a y S p l a y T r e e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SplaySplayTree() splays the splay-tree.
%
%  The format of the SplaySplayTree method is:
%
%      void SplaySplayTree(SplayTreeInfo *splay_tree,const void *key,
%        NodeInfo **node,NodeInfo **parent,NodeInfo **grandparent)
%
%  A description of each parameter follows:
%
%    o splay_tree: the splay-tree info.
%
%    o key: the key.
%
%    o node: the node.
%
%    o parent: the parent node.
%
%    o grandparent: the grandparent node.
%
*/

static NodeInfo *Splay(SplayTreeInfo *splay_tree,const size_t depth,
  const void *key,NodeInfo **node,NodeInfo **parent,NodeInfo **grandparent)
{
  int
    compare;

  NodeInfo
    *n,
    **next,
    *p;

  n=(*node);
  if (n == (NodeInfo *) NULL)
    {
      if (parent != (NodeInfo **) NULL)
        return(*parent);
      else
        return((NodeInfo *) NULL);
    }
  if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
    compare=splay_tree->compare(n->key,key);
  else
    compare=(n->key > key) ? 1 : ((n->key < key) ? -1 : 0);
  next=(NodeInfo **) NULL;
  if (compare > 0)
    next=(&n->left);
  else
    if (compare < 0)
      next=(&n->right);
  if (next != (NodeInfo **) NULL)
    {
      if (depth >= MaxSplayTreeDepth)
        {
          splay_tree->balance=MagickTrue;
          return(n);
        }
      n=Splay(splay_tree,depth+1,key,next,node,parent);
      if ((n != *node) || (splay_tree->balance != MagickFalse))
        return(n);
    }
  if (parent == (NodeInfo **) NULL)
    return(n);
  if (grandparent == (NodeInfo **) NULL)
    {
      if (n == (*parent)->left)
        {
          *node=n->right;
          n->right=(*parent);
        }
      else
        {
          *node=n->left;
          n->left=(*parent);
        }
      *parent=n;
      return(n);
    }
  if ((n == (*parent)->left) && (*parent == (*grandparent)->left))
    {
      p=(*parent);
      (*grandparent)->left=p->right;
      p->right=(*grandparent);
      p->left=n->right;
      n->right=p;
      *grandparent=n;
      return(n);
    }
  if ((n == (*parent)->right) && (*parent == (*grandparent)->right))
    {
      p=(*parent);
      (*grandparent)->right=p->left;
      p->left=(*grandparent);
      p->right=n->left;
      n->left=p;
      *grandparent=n;
      return(n);
    }
  if (n == (*parent)->left)
    {
      (*parent)->left=n->right;
      n->right=(*parent);
      (*grandparent)->right=n->left;
      n->left=(*grandparent);
      *grandparent=n;
      return(n);
    }
  (*parent)->right=n->left;
  n->left=(*parent);
  (*grandparent)->left=n->right;
  n->right=(*grandparent);
  *grandparent=n;
  return(n);
}

static void SplaySplayTree(SplayTreeInfo *splay_tree,const void *key)
{
  if (splay_tree->root == (NodeInfo *) NULL)
    return;
  if (splay_tree->key != (void *) NULL)
    {
      int
        compare;

      if (splay_tree->compare != (int (*)(const void *,const void *)) NULL)
        compare=splay_tree->compare(splay_tree->root->key,key);
      else
        compare=(splay_tree->key > key) ? 1 :
          ((splay_tree->key < key) ? -1 : 0);
      if (compare == 0)
        return;
    }
  (void) Splay(splay_tree,0UL,key,&splay_tree->root,(NodeInfo **) NULL,
    (NodeInfo **) NULL);
  if (splay_tree->balance != MagickFalse)
    {
      BalanceSplayTree(splay_tree);
      (void) Splay(splay_tree,0UL,key,&splay_tree->root,(NodeInfo **) NULL,
        (NodeInfo **) NULL);
      if (splay_tree->balance != MagickFalse)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
    }
  splay_tree->key=(void *) key;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            AAA   RRRR   TTTTT  IIIII  FFFFF   AAA    CCCC  TTTTT            %
%           A   A  R   R    T      I    F      A   A  C        T              %
%           AAAAA  RRRRR    T      I    FFF    AAAAA  C        T              %
%           A   A  R R      T      I    F      A   A  C        T              %
%           A   A  R  R     T    IIIII  F      A   A  CCCCC    T              %
%                                                                             %
%                                                                             %
%                         MagickCore Artifact Methods                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2000                                  %
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
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/compare.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx.h"
#include "MagickCore/fx-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/montage.h"
#include "MagickCore/option.h"
#include "MagickCore/profile.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resource_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/xml-tree.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e A r t i f a c t s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageArtifacts() clones all image artifacts to another image.
%
%  This will not delete any existing artifacts that may be present!
%
%  The format of the CloneImageArtifacts method is:
%
%      MagickBooleanType CloneImageArtifacts(Image *image,
%        const Image *clone_image)
%
%  A description of each parameter follows:
%
%    o image: the image, to receive the cloned artifacts.
%
%    o clone_image: the source image for artifacts to clone.
%
*/

typedef char
  *(*CloneKeyFunc)(const char *),
  *(*CloneValueFunc)(const char *);

static inline void *CloneArtifactKey(void *key)
{
  return((void *) ((CloneKeyFunc) ConstantString)((const char *) key));
}

static inline void *CloneArtifactValue(void *value)
{
  return((void *) ((CloneValueFunc) ConstantString)((const char *) value));
}

MagickExport MagickBooleanType CloneImageArtifacts(Image *image,
  const Image *clone_image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(clone_image != (const Image *) NULL);
  assert(clone_image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    {
      (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
      (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
        clone_image->filename);
    }
  if (clone_image->artifacts != (void *) NULL)
    {
      if (image->artifacts != (void *) NULL)
        DestroyImageArtifacts(image);
      image->artifacts=CloneSplayTree((SplayTreeInfo *) clone_image->artifacts,
        CloneArtifactKey,CloneArtifactValue);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e f i n e I m a g e A r t i f a c t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefineImageArtifact() associates an assignment string of the form
%  "key=value" with per-image artifact. It is equivalent to
%  SetImageArtifact().
%
%  The format of the DefineImageArtifact method is:
%
%      MagickBooleanType DefineImageArtifact(Image *image,
%        const char *artifact)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o artifact: the image artifact.
%
*/
MagickExport MagickBooleanType DefineImageArtifact(Image *image,
  const char *artifact)
{
  char
    key[MagickPathExtent],
    value[MagickPathExtent];

  char
    *p;

  assert(image != (Image *) NULL);
  assert(artifact != (const char *) NULL);
  (void) CopyMagickString(key,artifact,MagickPathExtent-1);
  for (p=key; *p != '\0'; p++)
    if (*p == '=')
      break;
  *value='\0';
  if (*p == '=')
    (void) CopyMagickString(value,p+1,MagickPathExtent);
  *p='\0';
  return(SetImageArtifact(image,key,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e A r t i f a c t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageArtifact() deletes an image artifact.
%
%  The format of the DeleteImageArtifact method is:
%
%      MagickBooleanType DeleteImageArtifact(Image *image,const char *artifact)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o artifact: the image artifact.
%
*/
MagickExport MagickBooleanType DeleteImageArtifact(Image *image,
  const char *artifact)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->artifacts == (void *) NULL)
    return(MagickFalse);
  return(DeleteNodeFromSplayTree((SplayTreeInfo *) image->artifacts,artifact));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e A r t i f a c t s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageArtifacts() destroys all artifacts and associated memory
%  attached to the given image.
%
%  The format of the DestroyImageArtifacts method is:
%
%      void DestroyImageArtifacts(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DestroyImageArtifacts(Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->artifacts != (void *) NULL)
    image->artifacts=(void *) DestroySplayTree((SplayTreeInfo *)
      image->artifacts);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e A r t i f a c t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageArtifact() gets a value associated with an image artifact.
%  If the requested artifact is NULL return the first artifact, to
%  prepare to iterate over all artifacts.
%
%  The returned string is a constant string in the tree and should NOT be
%  freed by the caller.
%
%  The format of the GetImageArtifact method is:
%
%      const char *GetImageArtifact(const Image *image,const char *key)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o key: the key.
%
*/
MagickExport const char *GetImageArtifact(const Image *image,
  const char *artifact)
{
  const char
    *p;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  p=(const char *) NULL;
  if (image->artifacts != (void *) NULL)
    {
      if (artifact == (const char *) NULL)
        return((const char *) GetRootValueFromSplayTree((SplayTreeInfo *)
          image->artifacts));
      p=(const char *) GetValueFromSplayTree((SplayTreeInfo *) image->artifacts,
        artifact);
      if (p != (const char *) NULL)
        return(p);
    }
  if ((image->image_info != (ImageInfo *) NULL) &&
      (image->image_info->options != (void *) NULL))
    p=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
      image->image_info->options,artifact);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e A r t i f a c t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImageArtifact() gets the next image artifact value.
%
%  The format of the GetNextImageArtifact method is:
%
%      char *GetNextImageArtifact(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const char *GetNextImageArtifact(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->artifacts == (void *) NULL)
    return((const char *) NULL);
  return((const char *) GetNextKeyInSplayTree(
   (SplayTreeInfo *) image->artifacts));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e I m a g e A r t i f a c t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveImageArtifact() removes an artifact from the image and returns its
%  value.
%
%  In this case the ConstantString() value returned should be freed by the
%  caller when finished.
%
%  The format of the RemoveImageArtifact method is:
%
%      char *RemoveImageArtifact(Image *image,const char *artifact)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o artifact: the image artifact.
%
*/
MagickExport char *RemoveImageArtifact(Image *image,const char *artifact)
{
  char
    *value;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->artifacts == (void *) NULL)
    return((char *) NULL);
  value=(char *) RemoveNodeFromSplayTree((SplayTreeInfo *) image->artifacts,
    artifact);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e A r t i f a c t I t e r a t o r                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImageArtifactIterator() resets the image artifact iterator.  Use it
%  in conjunction with GetNextImageArtifact() to iterate over all the values
%  associated with an image artifact.
%
%  Alternatively you can use GetImageArtifact() with a NULL artifact field to
%  reset the iterator and return the first artifact.
%
%  The format of the ResetImageArtifactIterator method is:
%
%      ResetImageArtifactIterator(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void ResetImageArtifactIterator(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->artifacts == (void *) NULL)
    return;
  ResetSplayTreeIterator((SplayTreeInfo *) image->artifacts);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e A r t i f a c t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageArtifact() sets a key-value pair in the image artifact namespace.
%  Artifacts differ from properties.  Properties are public and are generally
%  exported to an external image format if the format supports it.  Artifacts
%  are private and are utilized by the internal ImageMagick API to modify the
%  behavior of certain algorithms.
%
%  The format of the SetImageArtifact method is:
%
%      MagickBooleanType SetImageArtifact(Image *image,const char *artifact,
%        const char *value)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o artifact: the image artifact key.
%
%    o value: the image artifact value.
%
*/
MagickExport MagickBooleanType SetImageArtifact(Image *image,
  const char *artifact,const char *value)
{
  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Create tree if needed - specify how key,values are to be freed.
  */
  if (image->artifacts == (void *) NULL)
    image->artifacts=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
      RelinquishMagickMemory);
  /*
    Delete artifact if NULL --  empty string values are valid!,
  */
  if (value == (const char *) NULL)
    return(DeleteImageArtifact(image,artifact));
  /*
    Add artifact to splay-tree.
  */
  status=AddValueToSplayTree((SplayTreeInfo *) image->artifacts,
    ConstantString(artifact),ConstantString(value));
  return(status);
}

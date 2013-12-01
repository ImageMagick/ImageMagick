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
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/compare.h"
#include "magick/constitute.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fx.h"
#include "magick/fx-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/montage.h"
#include "magick/option.h"
#include "magick/profile.h"
#include "magick/quantum.h"
#include "magick/resource_.h"
#include "magick/splay-tree.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

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
%  CloneImageArtifacts() clones one or more image artifacts.
%
%  The format of the CloneImageArtifacts method is:
%
%      MagickBooleanType CloneImageArtifacts(Image *image,
%        const Image *clone_image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o clone_image: the clone image.
%
*/
MagickExport MagickBooleanType CloneImageArtifacts(Image *image,
  const Image *clone_image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(clone_image != (const Image *) NULL);
  assert(clone_image->signature == MagickSignature);
  if (clone_image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      clone_image->filename);
  if (clone_image->artifacts != (void *) NULL)
    {
      if (image->artifacts != (void *) NULL)
        DestroyImageArtifacts(image);
      image->artifacts=CloneSplayTree((SplayTreeInfo *) clone_image->artifacts,
        (void *(*)(void *)) ConstantString,(void *(*)(void *)) ConstantString);
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
%  DefineImageArtifact() associates a key/value pair with an image artifact.
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
    key[MaxTextExtent],
    value[MaxTextExtent];

  register char
    *p;

  assert(image != (Image *) NULL);
  assert(artifact != (const char *) NULL);
  (void) CopyMagickString(key,artifact,MaxTextExtent-1);
  for (p=key; *p != '\0'; p++)
    if (*p == '=')
      break;
  *value='\0';
  if (*p == '=')
    (void) CopyMagickString(value,p+1,MaxTextExtent);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
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
%  DestroyImageArtifacts() releases memory associated with image artifact
%  values.
%
%  The format of the DestroyDefines method is:
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
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
%
%  Note, the artifact is a constant.  Do not attempt to free it.
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
  register const char
    *p;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  p=(const char *) NULL;
  if (artifact == (const char *) NULL)
    {
      ResetSplayTreeIterator((SplayTreeInfo *) image->artifacts);
      p=(const char *) GetNextValueInSplayTree((SplayTreeInfo *)
        image->artifacts);
      return(p);
    }
  if (image->artifacts != (void *) NULL)
    {
      p=(const char *) GetValueFromSplayTree((SplayTreeInfo *) image->artifacts,
        artifact);
      if (p != (const char *) NULL)
        return(p);
    }
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
MagickExport char *GetNextImageArtifact(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->artifacts == (void *) NULL)
    return((char *) NULL);
  return((char *) GetNextKeyInSplayTree((SplayTreeInfo *) image->artifacts));
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
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
%  SetImageArtifact() associates makes a copy of the given string arguments
%  and inserts it into the artifact tree of the given image.
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Create tree if needed.
  */
  if (image->artifacts == (void *) NULL)
    image->artifacts=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
      RelinquishMagickMemory);
  /*
    Delete artifact if NULL --  empty string values are valid!
  */
  if (value == (const char *) NULL)
    return(DeleteImageArtifact(image,artifact));
  /*
    Add artifact to tree.
  */
  status=AddValueToSplayTree((SplayTreeInfo *) image->artifacts,
    ConstantString(artifact),ConstantString(value));
  return(status);
}

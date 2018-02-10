/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         L      IIIII  SSSSS  TTTTT                          %
%                         L        I    SS       T                            %
%                         L        I     SSS     T                            %
%                         L        I       SS    T                            %
%                         LLLLL  IIIII  SSSSS    T                            %
%                                                                             %
%                                                                             %
%                        MagickCore Image List Methods                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               December 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/string_.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A p p e n d I m a g e T o L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AppendImageToList() appends the second image list to the end of the first
%  list.  The given image list pointer is left unchanged, unless it was empty.
%
%  The format of the AppendImageToList method is:
%
%      AppendImageToList(Image *images,const Image *image)
%
%  A description of each parameter follows:
%
%    o images: the image list to be appended to.
%
%    o image: the appended image or image list.
%
*/
MagickExport void AppendImageToList(Image **images,const Image *append)
{
  register Image
    *p,
    *q;

  assert(images != (Image **) NULL);
  if (append == (Image *) NULL)
    return;
  assert(append->signature == MagickCoreSignature);
  if (append->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",append->filename);
  if ((*images) == (Image *) NULL)
    {
      *images=(Image *) append;
      return;
    }
  assert((*images)->signature == MagickCoreSignature);
  p=GetLastImageInList(*images);
  q=GetFirstImageInList(append);
  p->next=q;
  q->previous=p;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e L i s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageList() returns a duplicate of the image list.
%
%  The format of the CloneImageList method is:
%
%      Image *CloneImageList(const Image *images,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CloneImageList(const Image *images,ExceptionInfo *exception)
{
  Image
    *clone,
    *image;

  register Image
    *p;

  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  while (images->previous != (Image *) NULL)
  {
    assert(images != images->previous);
    images=images->previous;
  }
  image=(Image *) NULL;
  for (p=(Image *) NULL; images != (Image *) NULL; images=images->next)
  {
    assert(images != images->next);
    clone=CloneImage(images,0,0,MagickTrue,exception);
    if (clone == (Image *) NULL)
      {
        if (image != (Image *) NULL)
          image=DestroyImageList(image);
        return((Image *) NULL);
      }
    if (image == (Image *) NULL)
      {
        image=clone;
        p=image;
        continue;
      }
    p->next=clone;
    clone->previous=p;
    p=p->next;
  }
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e s                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImages() clones one or more images from an image sequence, using a
%  comma separated list of image numbers or ranges.
%
%  The numbers start at 0 for the first image in the list, while negative
%  numbers refer to images starting counting from the end of the range. Images
%  may be refered to multiple times to clone them multiple times. Images
%  refered beyond the available number of images in list are ignored.
%
%  Images referenced may be reversed, and results in a clone of those images
%  also being made with a reversed order.
%
%  The format of the CloneImages method is:
%
%      Image *CloneImages(const Image *images,const char *scenes,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o scenes: This character string specifies which scenes to clone
%      (e.g. 1,3-5,7-3,2).
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CloneImages(const Image *images,const char *scenes,
  ExceptionInfo *exception)
{
  char
    *p;

  const Image
    *next;

  Image
    *clone_images,
    *image;

  long
    first,
    last,
    step;

  register ssize_t
    i;

  size_t
    length;

  assert(images != (const Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  assert(scenes != (char *) NULL);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  clone_images=NewImageList();
  images=GetFirstImageInList(images);
  length=GetImageListLength(images);
  for (p=(char *) scenes; *p != '\0';)
  {
    while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
      p++;
    first=strtol(p,&p,10);
    if (first < 0)
      first+=(long) length;
    last=first;
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;
    if (*p == '-')
      {
        last=strtol(p+1,&p,10);
        if (last < 0)
          last+=(long) length;
      }
    for (step=first > last ? -1 : 1; first != (last+step); first+=step)
    {
      i=0;
      for (next=images; next != (Image *) NULL; next=GetNextImageInList(next))
      {
        if (i == (ssize_t) first)
          {
            image=CloneImage(next,0,0,MagickTrue,exception);
            if (image == (Image *) NULL)
              break;
            AppendImageToList(&clone_images,image);
          }
        i++;
      }
    }
  }
  return(GetFirstImageInList(clone_images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e F r o m L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageFromList() deletes an image from the list. List pointer
%  is moved to the next image, if one is present. See RemoveImageFromList().
%
%  The format of the DeleteImageFromList method is:
%
%      DeleteImageFromList(Image **images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport void DeleteImageFromList(Image **images)
{
  Image
    *image;

  image=RemoveImageFromList(images);
  if (image != (Image *) NULL)
    (void) DestroyImage(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImages() deletes one or more images from an image sequence, using a
%  comma separated list of image numbers or ranges.
%
%  The numbers start at 0 for the first image, while negative numbers refer to
%  images starting counting from the end of the range. Images may be refered to
%  multiple times without problems. Image refered beyond the available number
%  of images in list are ignored.
%
%  If the referenced images are in the reverse order, that range will be
%  completely ignored, unlike CloneImages().
%
%  The format of the DeleteImages method is:
%
%      DeleteImages(Image **images,const char *scenes,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o scenes: This character string specifies which scenes to delete
%      (e.g. 1,3-5,-2-6,2).
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void DeleteImages(Image **images,const char *scenes,
  ExceptionInfo *exception)
{
  char
    *p;

  Image
    *image;

  long
    first,
    last;

  MagickBooleanType
    *delete_list;

  register ssize_t
    i;

  size_t
    length;

  assert(images != (Image **) NULL);
  assert((*images)->signature == MagickCoreSignature);
  assert(scenes != (char *) NULL);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  *images=GetFirstImageInList(*images);
  length=GetImageListLength(*images);
  delete_list=(MagickBooleanType *) AcquireQuantumMemory(length,
    sizeof(*delete_list));
  if (delete_list == (MagickBooleanType *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",(*images)->filename);
      return;
    }
  image=(*images);
  for (i=0; i < (ssize_t) length; i++)
    delete_list[i]=MagickFalse;
  /*
    Note which images will be deleted, avoid duplicates.
  */
  for (p=(char *) scenes; *p != '\0';)
  {
    while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
      p++;
    first=strtol(p,&p,10);
    if (first < 0)
      first+=(long) length;
    last=first;
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;
    if (*p == '-')
      {
        last=strtol(p+1,&p,10);
        if (last < 0)
          last+=(long) length;
      }
    if (first > last)
      continue;
    for (i=(ssize_t) first; i <= (ssize_t) last; i++)
      if ((i >= 0) && (i < (ssize_t) length))
        delete_list[i]=MagickTrue;
  }
  /*
    Delete images marked for deletion, once only.
  */
  image=(*images);
  for (i=0; i < (ssize_t) length; i++)
  {
    *images=image;
    image=GetNextImageInList(image);
    if (delete_list[i] != MagickFalse)
      DeleteImageFromList(images);
  }
  (void) RelinquishMagickMemory(delete_list);
  *images=GetFirstImageInList(*images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageList() destroys an image list.
%
%  The format of the DestroyImageList method is:
%
%      Image *DestroyImageList(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
*/
MagickExport Image *DestroyImageList(Image *images)
{
  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  while (images != (Image *) NULL)
    DeleteImageFromList(&images);
  return((Image *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D u p l i c a t e I m a g e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DuplicateImages() duplicates one or more images from an image sequence,
%  using a count and a comma separated list of image numbers or ranges.
%
%  The numbers start at 0 for the first image, while negative numbers refer to
%  images starting counting from the end of the range. Images may be refered to
%  multiple times without problems. Image refered beyond the available number
%  of images in list are ignored.
%
%  The format of the DuplicateImages method is:
%
%      Image *DuplicateImages(Image *images,const size_t number_duplicates,
%        const char *scenes,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o number_duplicates: duplicate the image sequence this number of times.
%
%    o scenes: This character string specifies which scenes to duplicate (e.g.
%      1,3-5,-2-6,2).
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *DuplicateImages(Image *images,
  const size_t number_duplicates,const char *scenes,ExceptionInfo *exception)
{
  Image
    *clone_images,
    *duplicate_images;

  register ssize_t
    i;

  /*
    Duplicate images.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  assert(scenes != (char *) NULL);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  duplicate_images=NewImageList();
  for (i=0; i < (ssize_t) number_duplicates; i++)
  {
    clone_images=CloneImages(images,scenes,exception);
    AppendImageToList(&duplicate_images,clone_images);
  }
  return(duplicate_images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t F i r s t I m a g e I n L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetFirstImageInList() returns a pointer to the first image in the list.
%
%  The format of the GetFirstImageInList method is:
%
%      Image *GetFirstImageInList(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *GetFirstImageInList(const Image *images)
{
  register const Image
    *p;

  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  for (p=images; p->previous != (Image *) NULL; p=p->previous) ;
  return((Image *) p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e F r o m L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageFromList() returns an image at the specified index from the image
%  list. Starting with 0 as the first image in the list.
%
%  A negative offset will return the image from the end of the list, such that
%  an index of -1 is the last image.
%
%  If no such image exists at the specified offset a NULL image pointer is
%  returned.  This will only happen if index is less that the negative of
%  the list length, or larger than list length -1.  EG: ( -N to N-1 )
%
%  The format of the GetImageFromList method is:
%
%      Image *GetImageFromList(const Image *images,const ssize_t index)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o index: the position within the list.
%
*/
MagickExport Image *GetImageFromList(const Image *images,const ssize_t index)
{
  register const Image
    *p;

  register ssize_t
    i;

  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);

  /*
    Designed to efficiently find first image (index == 0), or last image
    (index == -1) as appropriate, without to go through the whole image list.
    That is it tries to avoid 'counting the whole list' to  handle the
    most common image indexes.
  */
  if ( index < 0 )
    {
      p=GetLastImageInList(images);
      for (i=-1; p != (Image *) NULL; p=p->previous)
        if (i-- == index)
          break;
    }
  else
    {
      p=GetFirstImageInList(images);
      for (i=0; p != (Image *) NULL; p=p->next)
        if (i++ == index)
          break;
    }
  return((Image *) p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e I n d e x I n L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageIndexInList() returns the offset in the list of the specified image.
%
%  The format of the GetImageIndexInList method is:
%
%      ssize_t GetImageIndexInList(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport ssize_t GetImageIndexInList(const Image *images)
{
  register ssize_t
    i;

  if (images == (const Image *) NULL)
    return(-1);
  assert(images->signature == MagickCoreSignature);
  for (i=0; images->previous != (Image *) NULL; i++)
  {
    assert(images != images->previous);
    images=images->previous;
  }
  return(i);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e L i s t L e n g t h                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageListLength() returns the length of the list (the number of images in
%  the list).
%
%  The format of the GetImageListLength method is:
%
%      size_t GetImageListLength(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport size_t GetImageListLength(const Image *images)
{
  register ssize_t
    i;

  if (images == (Image *) NULL)
    return(0);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  images=GetLastImageInList(images);
  for (i=0; images != (Image *) NULL; images=images->previous)
  {
    assert(images != images->previous);
    i++;
  }
  return((size_t) i);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L a s t I m a g e I n L i s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLastImageInList() returns a pointer to the last image in the list.
%
%  The format of the GetLastImageInList method is:
%
%      Image *GetLastImageInList(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *GetLastImageInList(const Image *images)
{
  register const Image
    *p;

  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  for (p=images; p->next != (Image *) NULL; p=p->next) ;
  return((Image *) p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e I n L i s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImageInList() returns the next image in the list.
%
%  The format of the GetNextImageInList method is:
%
%      Image *GetNextImageInList(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *GetNextImageInList(const Image *images)
{
  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  return(images->next);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P r e v i o u s I m a g e I n L i s t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPreviousImageInList() returns the previous image in the list.
%
%  The format of the GetPreviousImageInList method is:
%
%      Image *GetPreviousImageInList(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *GetPreviousImageInList(const Image *images)
{
  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  return(images->previous);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     I m a g e L i s t T o A r r a y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImageListToArray() is a convenience method that converts an image list to
%  a sequential array, with a NULL image pointer at the end of the array.
%
%  The images remain part of the original image list, with the array providing
%  an alternative means of indexing the image array.
%
%    group = ImageListToArray(images, exception);
%    while (i = 0; group[i] != (Image *) NULL; i++)
%      printf("%s\n", group[i]->filename);
%    printf("%d images\n", i);
%    group = RelinquishMagickMemory(group);
%
%  The format of the ImageListToArray method is:
%
%      Image **ImageListToArray(const Image *images,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image **ImageListToArray(const Image *images,
  ExceptionInfo *exception)
{
  Image
    **group;

  register ssize_t
    i;

  if (images == (Image *) NULL)
    return((Image **) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  group=(Image **) AcquireQuantumMemory((size_t) GetImageListLength(images)+1UL,
    sizeof(*group));
  if (group == (Image **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return((Image **) NULL);
    }
  images=GetFirstImageInList(images);
  for (i=0; images != (Image *) NULL; images=images->next)
  {
    assert(images != images->next);
    group[i++]=(Image *) images;
  }
  group[i]=(Image *) NULL;
  return(group);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n s e r t I m a g e I n L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InsertImageInList() insert the given image or image list, into the first
%  image list, immediately AFTER the image pointed to.  The given image list
%  pointer is left unchanged unless previously empty.
%
%  The format of the InsertImageInList method is:
%
%      InsertImageInList(Image **images,Image *insert)
%
%  A description of each parameter follows:
%
%    o images: the image list to insert into.
%
%    o insert: the image list to insert.
%
*/
MagickExport void InsertImageInList(Image **images,Image *insert)
{
  Image
    *split;

  assert(images != (Image **) NULL);
  assert(insert != (Image *) NULL);
  assert(insert->signature == MagickCoreSignature);
  if (insert->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",insert->filename);
  if ((*images) == (Image *) NULL)
    return;
  assert((*images)->signature == MagickCoreSignature);
  split=SplitImageList(*images);
  AppendImageToList(images,insert);
  AppendImageToList(images,split);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w I m a g e L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewImageList() creates an empty image list.
%
%  The format of the NewImageList method is:
%
%      Image *NewImageList(void)
%
*/
MagickExport Image *NewImageList(void)
{
  return((Image *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r e p e n d I m a g e T o L i s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PrependImageToList() prepends the image to the beginning of the list.
%
%  The format of the PrependImageToList method is:
%
%      PrependImageToList(Image *images,Image *image)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o image: the image.
%
*/
MagickExport void PrependImageToList(Image **images,Image *prepend)
{
  if (*images == (Image *) NULL)
    {
      *images=prepend;
      return;
    }
  AppendImageToList(&prepend,*images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e I m a g e F r o m L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveImageFromList() removes and returns the image pointed to.
%
%  The given image list pointer is set to point to the next image in list
%  if it exists, otherwise it is set to the previous image, or NULL if list
%  was emptied.
%
%  The format of the RemoveImageFromList method is:
%
%      Image *RemoveImageFromList(Image **images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *RemoveImageFromList(Image **images)
{
  register Image
    *p;

  assert(images != (Image **) NULL);
  if ((*images) == (Image *) NULL)
    return((Image *) NULL);
  assert((*images)->signature == MagickCoreSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  p=(*images);
  if ((p->previous == (Image *) NULL) && (p->next == (Image *) NULL))
    *images=(Image *) NULL;
  else
    {
      if (p->previous != (Image *) NULL)
        {
          p->previous->next=p->next;
          *images=p->previous;
        }
      if (p->next != (Image *) NULL)
        {
          p->next->previous=p->previous;
          *images=p->next;
        }
      p->previous=(Image *) NULL;
      p->next=(Image *) NULL;
    }
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e F i r s t I m a g e F r o m L i s t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveFirstImageFromList() removes and returns the first image in the list.
%
%  If the given image list pointer pointed to the removed first image, it is
%  set to the new first image of list, or NULL if list was emptied, otherwise
%  it is left as is.
%
%  The format of the RemoveFirstImageFromList method is:
%
%      Image *RemoveFirstImageFromList(Image **images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *RemoveFirstImageFromList(Image **images)
{
  Image
    *image;

  assert(images != (Image **) NULL);
  if ((*images) == (Image *) NULL)
    return((Image *) NULL);
  assert((*images)->signature == MagickCoreSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  image=(*images);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  if (image == *images)
    *images=(*images)->next;
  if (image->next != (Image *) NULL)
    {
      image->next->previous=(Image *) NULL;
      image->next=(Image *) NULL;
    }
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e L a s t I m a g e F r o m L i s t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveLastImageFromList() removes and returns the last image from the list.
%
%  If the given image list pointer pointed to the removed last image, it is
%  set to the new last image of list, or NULL if list was emptied, otherwise
%  it is left as is.
%
%  The format of the RemoveLastImageFromList method is:
%
%      Image *RemoveLastImageFromList(Image **images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *RemoveLastImageFromList(Image **images)
{
  Image
    *image;

  assert(images != (Image **) NULL);
  if ((*images) == (Image *) NULL)
    return((Image *) NULL);
  assert((*images)->signature == MagickCoreSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  image=(*images);
  while (image->next != (Image *) NULL)
    image=image->next;
  if (image == *images)
    *images=(*images)->previous;
  if (image->previous != (Image *) NULL)
    {
      image->previous->next=(Image *) NULL;
      image->previous=(Image *) NULL;
    }
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e p l a c e I m a g e I n L i s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReplaceImageInList() replaces an image in the list with the given image, or
%  list of images.  Old image is destroyed.
%
%  The images list pointer is set to point to the first image of the inserted
%  list of images.
%
%  The format of the ReplaceImageInList method is:
%
%      ReplaceImageInList(Image **images,Image *replace)
%
%  A description of each parameter follows:
%
%    o images: the list and pointer to image to replace
%
%    o replace: the image or image list replacing the original
%
*/
MagickExport void ReplaceImageInList(Image **images,Image *replace)
{
  assert(images != (Image **) NULL);
  assert(replace != (Image *) NULL);
  assert(replace->signature == MagickCoreSignature);
  if (replace->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",replace->filename);
  if ((*images) == (Image *) NULL)
    return;
  assert((*images)->signature == MagickCoreSignature);

  /* link next pointer */
  replace=GetLastImageInList(replace);
  replace->next=(*images)->next;
  if (replace->next != (Image *) NULL)
    replace->next->previous=replace;

  /* link previous pointer - set images position to first replacement image */
  replace=GetFirstImageInList(replace);
  replace->previous=(*images)->previous;
  if (replace->previous != (Image *) NULL)
    replace->previous->next=replace;

  /* destroy the replaced image that was in images */
  (void) DestroyImage(*images);
  (*images)=replace;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e p l a c e I m a g e I n L i s t R e t u r n L a s t                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReplaceImageInListReturnLast() is exactly as ReplaceImageInList() except
%  the images pointer is set to the last image in the list of replacement
%  images.
%
%  This allows you to simply use GetNextImageInList() to go to the image
%  that follows the just replaced image, even if a list of replacement images
%  was inserted.
%
%  The format of the ReplaceImageInList method is:
%
%      ReplaceImageInListReturnLast(Image **images,Image *replace)
%
%  A description of each parameter follows:
%
%    o images: the list and pointer to image to replace
%
%    o replace: the image or image list replacing the original
%
*/
MagickExport void ReplaceImageInListReturnLast(Image **images,Image *replace)
{
  assert(images != (Image **) NULL);
  assert(replace != (Image *) NULL);
  assert(replace->signature == MagickCoreSignature);
  if (replace->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",replace->filename);
  if ((*images) == (Image *) NULL)
    return;
  assert((*images)->signature == MagickCoreSignature);

  /* link previous pointer */
  replace=GetFirstImageInList(replace);
  replace->previous=(*images)->previous;
  if (replace->previous != (Image *) NULL)
    replace->previous->next=replace;

  /* link next pointer - set images position to last replacement image */
  replace=GetLastImageInList(replace);
  replace->next=(*images)->next;
  if (replace->next != (Image *) NULL)
    replace->next->previous=replace;

  /* destroy the replaced image that was in images */
  (void) DestroyImage(*images);
  (*images)=replace;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e v e r s e I m a g e L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReverseImageList() reverses the order of an image list.
%  The list pointer is reset to that start of the re-ordered list.
%
%  The format of the ReverseImageList method is:
%
%      void ReverseImageList(Image **images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport void ReverseImageList(Image **images)
{
  Image
    *next;

  register Image
    *p;

  assert(images != (Image **) NULL);
  if ((*images) == (Image *) NULL)
    return;
  assert((*images)->signature == MagickCoreSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  for (p=(*images); p->next != (Image *) NULL; p=p->next) ;
  *images=p;
  for ( ; p != (Image *) NULL; p=p->next)
  {
    next=p->next;
    p->next=p->previous;
    p->previous=next;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p l i c e I m a g e I n t o L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SpliceImageIntoList() removes 'length' images from the list and replaces
%  them with the specified splice. Removed images are returned.
%
%  The format of the SpliceImageIntoList method is:
%
%      SpliceImageIntoList(Image **images,const size_t,
%        const Image *splice)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o length: the length of the image list to remove.
%
%    o splice: Replace the removed image list with this list.
%
*/
MagickExport Image *SpliceImageIntoList(Image **images,
  const size_t length,const Image *splice)
{
  Image
    *image,
    *split;

  register size_t
    i;

  assert(images != (Image **) NULL);
  assert(splice != (Image *) NULL);
  assert(splice->signature == MagickCoreSignature);
  if ((*images) == (Image *) NULL)
    return((Image *) NULL);
  assert((*images)->signature == MagickCoreSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  split=SplitImageList(*images);
  AppendImageToList(images,splice);
  image=(Image *) NULL;
  for (i=0; (i < length) && (split != (Image *) NULL); i++)
    AppendImageToList(&image,RemoveImageFromList(&split));
  AppendImageToList(images,split);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p l i t I m a g e L i s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SplitImageList() splits an image into two lists, after given image
%  The list that was split off is returned, which may be empty.
%
%  The format of the SplitImageList method is:
%
%      Image *SplitImageList(Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *SplitImageList(Image *images)
{
  if ((images == (Image *) NULL) || (images->next == (Image *) NULL))
    return((Image *) NULL);
  images=images->next;
  images->previous->next=(Image *) NULL;
  images->previous=(Image *) NULL;
  return(images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c I m a g e L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncImageList() synchronizes the scene numbers in an image list.
%
%  The format of the SyncImageList method is:
%
%      void SyncImageList(Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport void SyncImageList(Image *images)
{
  register Image
    *p,
    *q;

  if (images == (Image *) NULL)
    return;
  assert(images->signature == MagickCoreSignature);
  for (p=images; p != (Image *) NULL; p=p->next)
  {
    for (q=p->next; q != (Image *) NULL; q=q->next)
      if (p->scene == q->scene)
        break;
    if (q != (Image *) NULL)
      break;
  }
  if (p == (Image *) NULL)
    return;
  for (p=images->next; p != (Image *) NULL; p=p->next)
    p->scene=p->previous->scene+1;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c N e x t I m a g e I n L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncNextImageInList() returns the next image in the list after the blob
%  referenced is synchronized with the current image.
%
%  The format of the SyncNextImageInList method is:
%
%      Image *SyncNextImageInList(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *SyncNextImageInList(const Image *images)
{
  if (images == (Image *) NULL)
    return((Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->next == (Image *) NULL)
    return((Image *) NULL);
  if (images->blob != images->next->blob)
    {
      DestroyBlob(images->next);
      images->next->blob=ReferenceBlob(images->blob);
    }
  images->next->compression=images->compression;
  images->next->endian=images->endian;
  return(images->next);
}

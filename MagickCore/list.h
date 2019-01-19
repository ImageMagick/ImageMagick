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

  MagickCore image list methods.
*/
#ifndef MAGICKCORE_LIST_H
#define MAGICKCORE_LIST_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport Image
  *CloneImageList(const Image *,ExceptionInfo *),
  *CloneImages(const Image *,const char *,ExceptionInfo *),
  *DestroyImageList(Image *),
  *DuplicateImages(Image *,const size_t,const char *,ExceptionInfo *),
  *GetFirstImageInList(const Image *) magick_attribute((__pure__)),
  *GetImageFromList(const Image *,const ssize_t) magick_attribute((__pure__)),
  *GetLastImageInList(const Image *) magick_attribute((__pure__)),
  *GetNextImageInList(const Image *) magick_attribute((__pure__)),
  *GetPreviousImageInList(const Image *) magick_attribute((__pure__)),
  **ImageListToArray(const Image *,ExceptionInfo *),
  *NewImageList(void) magick_attribute((__const__)),
  *RemoveImageFromList(Image **),
  *RemoveLastImageFromList(Image **),
  *RemoveFirstImageFromList(Image **),
  *SpliceImageIntoList(Image **,const size_t,const Image *),
  *SplitImageList(Image *),
  *SyncNextImageInList(const Image *);

extern MagickExport size_t
  GetImageListLength(const Image *) magick_attribute((__pure__));

extern MagickExport ssize_t
  GetImageIndexInList(const Image *) magick_attribute((__pure__));

extern MagickExport void
  AppendImageToList(Image **,const Image *),
  DeleteImageFromList(Image **),
  DeleteImages(Image **,const char *,ExceptionInfo *),
  InsertImageInList(Image **,Image *),
  PrependImageToList(Image **,Image *),
  ReplaceImageInList(Image **,Image *),
  ReplaceImageInListReturnLast(Image **,Image *),
  ReverseImageList(Image **),
  SyncImageList(Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

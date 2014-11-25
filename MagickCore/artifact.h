/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore artifact methods.
*/
#ifndef _MAGICKCORE_ARTIFACT_H
#define _MAGICKCORE_ARTIFACT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport char
  *GetNextImageArtifact(const Image *),
  *RemoveImageArtifact(Image *,const char *);

extern MagickExport const char
  *GetImageArtifact(const Image *,const char *);

extern MagickExport MagickBooleanType
  CloneImageArtifacts(Image *,const Image *),
  DefineImageArtifact(Image *,const char *),
  DeleteImageArtifact(Image *,const char *),
  SetImageArtifact(Image *,const char *,const char *);

extern MagickExport void
  DestroyImageArtifacts(Image *),
  ResetImageArtifactIterator(const Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

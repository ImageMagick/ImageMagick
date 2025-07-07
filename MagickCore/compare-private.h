/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore compare private methods.
*/
#ifndef MAGICKCORE_COMPARE_PRIVATE_H
#define MAGICKCORE_COMPARE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/image-private.h"

#define DefaultSimilarityThreshold  (-1.0)
#define MagickSafePSNRRecipicol(x)  ((x)*log10(1.0/MagickEpsilon))

static inline void SetImageCompareBounds(const Image *image,
  const Image *reconstruct_image,size_t *columns,size_t *rows)
{
  const char
    *artifact;

  *columns=MagickMax(image->columns,reconstruct_image->columns);
  *rows=MagickMax(image->rows,reconstruct_image->rows);
  artifact=GetImageArtifact(image,"compare:virtual-pixels");
  if ((artifact != (const char *) NULL) &&
      (IsStringTrue(artifact) == MagickFalse))
    {
      *columns=MagickMin(image->columns,reconstruct_image->columns);
      *rows=MagickMin(image->rows,reconstruct_image->rows);
    }
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

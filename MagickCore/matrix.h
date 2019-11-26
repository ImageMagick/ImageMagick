/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore matrix methods.
*/
#ifndef MAGICKCORE_MATRIX_H
#define MAGICKCORE_MATRIX_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _MatrixInfo
  MatrixInfo;

extern MagickExport double
  **AcquireMagickMatrix(const size_t,const size_t),
  **RelinquishMagickMatrix(double **,const size_t);

extern MagickExport Image
  *MatrixToImage(const MatrixInfo *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  GetMatrixElement(const MatrixInfo *,const ssize_t,const ssize_t,void *),
  NullMatrix(MatrixInfo *),
  SetMatrixElement(const MatrixInfo *,const ssize_t,const ssize_t,const void *);

MagickExport MatrixInfo
  *AcquireMatrixInfo(const size_t,const size_t,const size_t,ExceptionInfo *),
  *DestroyMatrixInfo(MatrixInfo *);

MagickExport size_t
  GetMatrixColumns(const MatrixInfo *),
  GetMatrixRows(const MatrixInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

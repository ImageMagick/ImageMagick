/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  M   M   AAA   TTTTT  RRRR   IIIII  X   X                   %
%                  MM MM  A   A    T    R   R    I     X X                    %
%                  M M M  AAAAA    T    RRRR     I      X                     %
%                  M   M  A   A    T    R R      I     X X                    %
%                  M   M  A   A    T    R  R   IIIII  X   X                   %
%                                                                             %
%                                                                             %
%                         MagickCore Matrix Methods                           %
%                                                                             %
%                            Software Design                                  %
%                                 Cristy                                      %
%                              August 2007                                    %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/matrix.h"
#include "MagickCore/matrix-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/utility.h"

/*
  Typedef declaration.
*/
struct _MatrixInfo
{
  CacheType
    type;

  size_t
    columns,
    rows,
    stride;

  MagickSizeType
    length;

  MagickBooleanType
    mapped,
    synchronize;

  char
    path[MagickPathExtent];

  int
    file;

  void
    *elements;

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
%   A c q u i r e M a t r i x I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMatrixInfo() allocates the ImageInfo structure.
%
%  The format of the AcquireMatrixInfo method is:
%
%      MatrixInfo *AcquireMatrixInfo(const size_t columns,const size_t rows,
%        const size_t stride,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o columns: the matrix columns.
%
%    o rows: the matrix rows.
%
%    o stride: the matrix stride.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(SIGBUS)
static void MatrixSignalHandler(int status)
{
  ThrowFatalException(CacheFatalError,"UnableToExtendMatrixCache");
}
#endif

static inline MagickOffsetType WriteMatrixElements(
  const MatrixInfo *magick_restrict matrix_info,const MagickOffsetType offset,
  const MagickSizeType length,const unsigned char *magick_restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PWRITE)
  LockSemaphoreInfo(matrix_info->semaphore);
  if (lseek(matrix_info->file,offset,SEEK_SET) < 0)
    {
      UnlockSemaphoreInfo(matrix_info->semaphore);
      return((MagickOffsetType) -1);
    }
#endif
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
#if !defined(MAGICKCORE_HAVE_PWRITE)
    count=write(matrix_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX));
#else
    count=pwrite(matrix_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX),(off_t) (offset+i));
#endif
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
#if !defined(MAGICKCORE_HAVE_PWRITE)
  UnlockSemaphoreInfo(matrix_info->semaphore);
#endif
  return(i);
}

static MagickBooleanType SetMatrixExtent(
  MatrixInfo *magick_restrict matrix_info,MagickSizeType length)
{
  MagickOffsetType
    count,
    extent,
    offset;

  if (length != (MagickSizeType) ((MagickOffsetType) length))
    return(MagickFalse);
  offset=(MagickOffsetType) lseek(matrix_info->file,0,SEEK_END);
  if (offset < 0)
    return(MagickFalse);
  if ((MagickSizeType) offset >= length)
    return(MagickTrue);
  extent=(MagickOffsetType) length-1;
  count=WriteMatrixElements(matrix_info,extent,1,(const unsigned char *) "");
#if defined(MAGICKCORE_HAVE_POSIX_FALLOCATE)
  if (matrix_info->synchronize != MagickFalse)
    (void) posix_fallocate(matrix_info->file,offset+1,extent-offset);
#endif
#if defined(SIGBUS)
  (void) signal(SIGBUS,MatrixSignalHandler);
#endif
  return(count != (MagickOffsetType) 1 ? MagickFalse : MagickTrue);
}

MagickExport MatrixInfo *AcquireMatrixInfo(const size_t columns,
  const size_t rows,const size_t stride,ExceptionInfo *exception)
{
  char
    *synchronize;

  MagickBooleanType
    status;

  MatrixInfo
    *matrix_info;

  matrix_info=(MatrixInfo *) AcquireMagickMemory(sizeof(*matrix_info));
  if (matrix_info == (MatrixInfo *) NULL)
    return((MatrixInfo *) NULL);
  (void) ResetMagickMemory(matrix_info,0,sizeof(*matrix_info));
  matrix_info->signature=MagickCoreSignature;
  matrix_info->columns=columns;
  matrix_info->rows=rows;
  matrix_info->stride=stride;
  matrix_info->semaphore=AcquireSemaphoreInfo();
  synchronize=GetEnvironmentValue("MAGICK_SYNCHRONIZE");
  if (synchronize != (const char *) NULL)
    {
      matrix_info->synchronize=IsStringTrue(synchronize);
      synchronize=DestroyString(synchronize);
    }
  matrix_info->length=(MagickSizeType) columns*rows*stride;
  if (matrix_info->columns != (size_t) (matrix_info->length/rows/stride))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "CacheResourcesExhausted","`%s'","matrix cache");
      return(DestroyMatrixInfo(matrix_info));
    }
  matrix_info->type=MemoryCache;
  status=AcquireMagickResource(AreaResource,matrix_info->length);
  if ((status != MagickFalse) &&
      (matrix_info->length == (MagickSizeType) ((size_t) matrix_info->length)))
    {
      status=AcquireMagickResource(MemoryResource,matrix_info->length);
      if (status != MagickFalse)
        {
          matrix_info->mapped=MagickFalse;
          matrix_info->elements=AcquireMagickMemory((size_t)
            matrix_info->length);
          if (matrix_info->elements == NULL)
            {
              matrix_info->mapped=MagickTrue;
              matrix_info->elements=MapBlob(-1,IOMode,0,(size_t)
                matrix_info->length);
            }
          if (matrix_info->elements == (unsigned short *) NULL)
            RelinquishMagickResource(MemoryResource,matrix_info->length);
        }
    }
  matrix_info->file=(-1);
  if (matrix_info->elements == (unsigned short *) NULL)
    {
      status=AcquireMagickResource(DiskResource,matrix_info->length);
      if (status == MagickFalse)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
            "CacheResourcesExhausted","`%s'","matrix cache");
          return(DestroyMatrixInfo(matrix_info));
        }
      matrix_info->type=DiskCache;
      matrix_info->file=AcquireUniqueFileResource(matrix_info->path);
      if (matrix_info->file == -1)
        return(DestroyMatrixInfo(matrix_info));
      status=AcquireMagickResource(MapResource,matrix_info->length);
      if (status != MagickFalse)
        {
          status=SetMatrixExtent(matrix_info,matrix_info->length);
          if (status != MagickFalse)
            matrix_info->elements=(void *) MapBlob(matrix_info->file,IOMode,0,
              (size_t) matrix_info->length);
          if (matrix_info->elements != NULL)
            matrix_info->type=MapCache;
          else
            RelinquishMagickResource(MapResource,matrix_info->length);
        }
    }
  return(matrix_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M a g i c k M a t r i x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagickMatrix() allocates and returns a matrix in the form of an
%  array of pointers to an array of doubles, with all values pre-set to zero.
%
%  This used to generate the two dimensional matrix, and vectors required
%  for the GaussJordanElimination() method below, solving some system of
%  simultanious equations.
%
%  The format of the AcquireMagickMatrix method is:
%
%      double **AcquireMagickMatrix(const size_t number_rows,
%        const size_t size)
%
%  A description of each parameter follows:
%
%    o number_rows: the number pointers for the array of pointers
%      (first dimension).
%
%    o size: the size of the array of doubles each pointer points to
%      (second dimension).
%
*/
MagickExport double **AcquireMagickMatrix(const size_t number_rows,
  const size_t size)
{
  double
    **matrix;

  register ssize_t
    i,
    j;

  matrix=(double **) AcquireQuantumMemory(number_rows,sizeof(*matrix));
  if (matrix == (double **) NULL)
    return((double **) NULL);
  for (i=0; i < (ssize_t) number_rows; i++)
  {
    matrix[i]=(double *) AcquireQuantumMemory(size,sizeof(*matrix[i]));
    if (matrix[i] == (double *) NULL)
      {
        for (j=0; j < i; j++)
          matrix[j]=(double *) RelinquishMagickMemory(matrix[j]);
        matrix=(double **) RelinquishMagickMemory(matrix);
        return((double **) NULL);
      }
    for (j=0; j < (ssize_t) size; j++)
      matrix[i][j]=0.0;
  }
  return(matrix);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y M a t r i x I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMatrixInfo() dereferences a matrix, deallocating memory associated
%  with the matrix.
%
%  The format of the DestroyImage method is:
%
%      MatrixInfo *DestroyMatrixInfo(MatrixInfo *matrix_info)
%
%  A description of each parameter follows:
%
%    o matrix_info: the matrix.
%
*/
MagickExport MatrixInfo *DestroyMatrixInfo(MatrixInfo *matrix_info)
{
  assert(matrix_info != (MatrixInfo *) NULL);
  assert(matrix_info->signature == MagickCoreSignature);
  LockSemaphoreInfo(matrix_info->semaphore);
  switch (matrix_info->type)
  {
    case MemoryCache:
    {
      if (matrix_info->mapped == MagickFalse)
        matrix_info->elements=RelinquishMagickMemory(matrix_info->elements);
      else
        {
          (void) UnmapBlob(matrix_info->elements,(size_t) matrix_info->length);
          matrix_info->elements=(unsigned short *) NULL;
        }
      RelinquishMagickResource(MemoryResource,matrix_info->length);
      break;
    }
    case MapCache:
    {
      (void) UnmapBlob(matrix_info->elements,(size_t) matrix_info->length);
      matrix_info->elements=NULL;
      RelinquishMagickResource(MapResource,matrix_info->length);
    }
    case DiskCache:
    {
      if (matrix_info->file != -1)
        (void) close(matrix_info->file);
      (void) RelinquishUniqueFileResource(matrix_info->path);
      RelinquishMagickResource(DiskResource,matrix_info->length);
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(matrix_info->semaphore);
  RelinquishSemaphoreInfo(&matrix_info->semaphore);
  return((MatrixInfo *) RelinquishMagickMemory(matrix_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G a u s s J o r d a n E l i m i n a t i o n                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GaussJordanElimination() returns a matrix in reduced row echelon form,
%  while simultaneously reducing and thus solving the augumented results
%  matrix.
%
%  See also  http://en.wikipedia.org/wiki/Gauss-Jordan_elimination
%
%  The format of the GaussJordanElimination method is:
%
%      MagickBooleanType GaussJordanElimination(double **matrix,
%        double **vectors,const size_t rank,const size_t number_vectors)
%
%  A description of each parameter follows:
%
%    o matrix: the matrix to be reduced, as an 'array of row pointers'.
%
%    o vectors: the additional matrix argumenting the matrix for row reduction.
%             Producing an 'array of column vectors'.
%
%    o rank:  The size of the matrix (both rows and columns).
%             Also represents the number terms that need to be solved.
%
%    o number_vectors: Number of vectors columns, argumenting the above matrix.
%             Usally 1, but can be more for more complex equation solving.
%
%  Note that the 'matrix' is given as a 'array of row pointers' of rank size.
%  That is values can be assigned as   matrix[row][column]   where 'row' is
%  typically the equation, and 'column' is the term of the equation.
%  That is the matrix is in the form of a 'row first array'.
%
%  However 'vectors' is a 'array of column pointers' which can have any number
%  of columns, with each column array the same 'rank' size as 'matrix'.
%
%  This allows for simpler handling of the results, especially is only one
%  column 'vector' is all that is required to produce the desired solution.
%
%  For example, the 'vectors' can consist of a pointer to a simple array of
%  doubles.  when only one set of simultanious equations is to be solved from
%  the given set of coefficient weighted terms.
%
%     double **matrix = AcquireMagickMatrix(8UL,8UL);
%     double coefficents[8];
%     ...
%     GaussJordanElimination(matrix, &coefficents, 8UL, 1UL);
%
%  However by specifing more 'columns' (as an 'array of vector columns',
%  you can use this function to solve a set of 'separable' equations.
%
%  For example a distortion function where    u = U(x,y)   v = V(x,y)
%  And the functions U() and V() have separate coefficents, but are being
%  generated from a common x,y->u,v  data set.
%
%  Another example is generation of a color gradient from a set of colors at
%  specific coordients, such as a list x,y -> r,g,b,a.
%
%  You can also use the 'vectors' to generate an inverse of the given 'matrix'
%  though as a 'column first array' rather than a 'row first array'. For
%  details see http://en.wikipedia.org/wiki/Gauss-Jordan_elimination
%
*/
MagickPrivate MagickBooleanType GaussJordanElimination(double **matrix,
  double **vectors,const size_t rank,const size_t number_vectors)
{
#define GaussJordanSwap(x,y) \
{ \
  if ((x) != (y)) \
    { \
      (x)+=(y); \
      (y)=(x)-(y); \
      (x)=(x)-(y); \
    } \
}

  double
    max,
    scale;

  register ssize_t
    i,
    j,
    k;

  ssize_t
    column,
    *columns,
    *pivots,
    row,
    *rows;

  columns=(ssize_t *) AcquireQuantumMemory(rank,sizeof(*columns));
  rows=(ssize_t *) AcquireQuantumMemory(rank,sizeof(*rows));
  pivots=(ssize_t *) AcquireQuantumMemory(rank,sizeof(*pivots));
  if ((rows == (ssize_t *) NULL) || (columns == (ssize_t *) NULL) ||
      (pivots == (ssize_t *) NULL))
    {
      if (pivots != (ssize_t *) NULL)
        pivots=(ssize_t *) RelinquishMagickMemory(pivots);
      if (columns != (ssize_t *) NULL)
        columns=(ssize_t *) RelinquishMagickMemory(columns);
      if (rows != (ssize_t *) NULL)
        rows=(ssize_t *) RelinquishMagickMemory(rows);
      return(MagickFalse);
    }
  (void) ResetMagickMemory(columns,0,rank*sizeof(*columns));
  (void) ResetMagickMemory(rows,0,rank*sizeof(*rows));
  (void) ResetMagickMemory(pivots,0,rank*sizeof(*pivots));
  column=0;
  row=0;
  for (i=0; i < (ssize_t) rank; i++)
  {
    max=0.0;
    for (j=0; j < (ssize_t) rank; j++)
      if (pivots[j] != 1)
        {
          for (k=0; k < (ssize_t) rank; k++)
            if (pivots[k] != 0)
              {
                if (pivots[k] > 1)
                  return(MagickFalse);
              }
            else
              if (fabs(matrix[j][k]) >= max)
                {
                  max=fabs(matrix[j][k]);
                  row=j;
                  column=k;
                }
        }
    pivots[column]++;
    if (row != column)
      {
        for (k=0; k < (ssize_t) rank; k++)
          GaussJordanSwap(matrix[row][k],matrix[column][k]);
        for (k=0; k < (ssize_t) number_vectors; k++)
          GaussJordanSwap(vectors[k][row],vectors[k][column]);
      }
    rows[i]=row;
    columns[i]=column;
    if (matrix[column][column] == 0.0)
      return(MagickFalse);  /* sigularity */
    scale=PerceptibleReciprocal(matrix[column][column]);
    matrix[column][column]=1.0;
    for (j=0; j < (ssize_t) rank; j++)
      matrix[column][j]*=scale;
    for (j=0; j < (ssize_t) number_vectors; j++)
      vectors[j][column]*=scale;
    for (j=0; j < (ssize_t) rank; j++)
      if (j != column)
        {
          scale=matrix[j][column];
          matrix[j][column]=0.0;
          for (k=0; k < (ssize_t) rank; k++)
            matrix[j][k]-=scale*matrix[column][k];
          for (k=0; k < (ssize_t) number_vectors; k++)
            vectors[k][j]-=scale*vectors[k][column];
        }
  }
  for (j=(ssize_t) rank-1; j >= 0; j--)
    if (columns[j] != rows[j])
      for (i=0; i < (ssize_t) rank; i++)
        GaussJordanSwap(matrix[i][rows[j]],matrix[i][columns[j]]);
  pivots=(ssize_t *) RelinquishMagickMemory(pivots);
  rows=(ssize_t *) RelinquishMagickMemory(rows);
  columns=(ssize_t *) RelinquishMagickMemory(columns);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a t r i x C o l u m n s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMatrixColumns() returns the number of columns in the matrix.
%
%  The format of the GetMatrixColumns method is:
%
%      size_t GetMatrixColumns(const MatrixInfo *matrix_info)
%
%  A description of each parameter follows:
%
%    o matrix_info: the matrix.
%
*/
MagickExport size_t GetMatrixColumns(const MatrixInfo *matrix_info)
{
  assert(matrix_info != (MatrixInfo *) NULL);
  assert(matrix_info->signature == MagickCoreSignature);
  return(matrix_info->columns);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a t r i x E l e m e n t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMatrixElement() returns the specifed element in the matrix.
%
%  The format of the GetMatrixElement method is:
%
%      MagickBooleanType GetMatrixElement(const MatrixInfo *matrix_info,
%        const ssize_t x,const ssize_t y,void *value)
%
%  A description of each parameter follows:
%
%    o matrix_info: the matrix columns.
%
%    o x: the matrix x-offset.
%
%    o y: the matrix y-offset.
%
%    o value: return the matrix element in this buffer.
%
*/

static inline ssize_t EdgeX(const ssize_t x,const size_t columns)
{
  if (x < 0L)
    return(0L);
  if (x >= (ssize_t) columns)
    return((ssize_t) (columns-1));
  return(x);
}

static inline ssize_t EdgeY(const ssize_t y,const size_t rows)
{
  if (y < 0L)
    return(0L);
  if (y >= (ssize_t) rows)
    return((ssize_t) (rows-1));
  return(y);
}

static inline MagickOffsetType ReadMatrixElements(
  const MatrixInfo *magick_restrict matrix_info,const MagickOffsetType offset,
  const MagickSizeType length,unsigned char *magick_restrict buffer)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PREAD)
  LockSemaphoreInfo(matrix_info->semaphore);
  if (lseek(matrix_info->file,offset,SEEK_SET) < 0)
    {
      UnlockSemaphoreInfo(matrix_info->semaphore);
      return((MagickOffsetType) -1);
    }
#endif
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
#if !defined(MAGICKCORE_HAVE_PREAD)
    count=read(matrix_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX));
#else
    count=pread(matrix_info->file,buffer+i,(size_t) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX),(off_t) (offset+i));
#endif
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
#if !defined(MAGICKCORE_HAVE_PREAD)
  UnlockSemaphoreInfo(matrix_info->semaphore);
#endif
  return(i);
}

MagickExport MagickBooleanType GetMatrixElement(const MatrixInfo *matrix_info,
  const ssize_t x,const ssize_t y,void *value)
{
  MagickOffsetType
    count,
    i;

  assert(matrix_info != (const MatrixInfo *) NULL);
  assert(matrix_info->signature == MagickCoreSignature);
  i=(MagickOffsetType) EdgeY(y,matrix_info->rows)*matrix_info->columns+
    EdgeX(x,matrix_info->columns);
  if (matrix_info->type != DiskCache)
    {
      (void) memcpy(value,(unsigned char *) matrix_info->elements+i*
        matrix_info->stride,matrix_info->stride);
      return(MagickTrue);
    }
  count=ReadMatrixElements(matrix_info,i*matrix_info->stride,
    matrix_info->stride,(unsigned char *) value);
  if (count != (MagickOffsetType) matrix_info->stride)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a t r i x R o w s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMatrixRows() returns the number of rows in the matrix.
%
%  The format of the GetMatrixRows method is:
%
%      size_t GetMatrixRows(const MatrixInfo *matrix_info)
%
%  A description of each parameter follows:
%
%    o matrix_info: the matrix.
%
*/
MagickExport size_t GetMatrixRows(const MatrixInfo *matrix_info)
{
  assert(matrix_info != (const MatrixInfo *) NULL);
  assert(matrix_info->signature == MagickCoreSignature);
  return(matrix_info->rows);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L e a s t S q u a r e s A d d T e r m s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LeastSquaresAddTerms() adds one set of terms and associate results to the
%  given matrix and vectors for solving using least-squares function fitting.
%
%  The format of the AcquireMagickMatrix method is:
%
%      void LeastSquaresAddTerms(double **matrix,double **vectors,
%        const double *terms,const double *results,const size_t rank,
%        const size_t number_vectors);
%
%  A description of each parameter follows:
%
%    o matrix: the square matrix to add given terms/results to.
%
%    o vectors: the result vectors to add terms/results to.
%
%    o terms: the pre-calculated terms (without the unknown coefficent
%             weights) that forms the equation being added.
%
%    o results: the result(s) that should be generated from the given terms
%               weighted by the yet-to-be-solved coefficents.
%
%    o rank: the rank or size of the dimensions of the square matrix.
%            Also the length of vectors, and number of terms being added.
%
%    o number_vectors: Number of result vectors, and number or results being
%      added.  Also represents the number of separable systems of equations
%      that is being solved.
%
%  Example of use...
%
%     2 dimensional Affine Equations (which are separable)
%         c0*x + c2*y + c4*1 => u
%         c1*x + c3*y + c5*1 => v
%
%     double **matrix = AcquireMagickMatrix(3UL,3UL);
%     double **vectors = AcquireMagickMatrix(2UL,3UL);
%     double terms[3], results[2];
%     ...
%     for each given x,y -> u,v
%        terms[0] = x;
%        terms[1] = y;
%        terms[2] = 1;
%        results[0] = u;
%        results[1] = v;
%        LeastSquaresAddTerms(matrix,vectors,terms,results,3UL,2UL);
%     ...
%     if ( GaussJordanElimination(matrix,vectors,3UL,2UL) ) {
%       c0 = vectors[0][0];
%       c2 = vectors[0][1];
%       c4 = vectors[0][2];
%       c1 = vectors[1][0];
%       c3 = vectors[1][1];
%       c5 = vectors[1][2];
%     }
%     else
%       printf("Matrix unsolvable\n);
%     RelinquishMagickMatrix(matrix,3UL);
%     RelinquishMagickMatrix(vectors,2UL);
%
*/
MagickPrivate void LeastSquaresAddTerms(double **matrix,double **vectors,
  const double *terms,const double *results,const size_t rank,
  const size_t number_vectors)
{
  register ssize_t
    i,
    j;

  for (j=0; j < (ssize_t) rank; j++)
  {
    for (i=0; i < (ssize_t) rank; i++)
      matrix[i][j]+=terms[i]*terms[j];
    for (i=0; i < (ssize_t) number_vectors; i++)
      vectors[i][j]+=results[i]*terms[j];
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a t r i x T o I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MatrixToImage() returns a matrix as an image.  The matrix elements must be
%  of type double otherwise nonsense is returned.
%
%  The format of the MatrixToImage method is:
%
%      Image *MatrixToImage(const MatrixInfo *matrix_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o matrix_info: the matrix.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MatrixToImage(const MatrixInfo *matrix_info,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  double
    max_value,
    min_value,
    scale_factor,
    value;

  Image
    *image;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(matrix_info != (const MatrixInfo *) NULL);
  assert(matrix_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (matrix_info->stride < sizeof(double))
    return((Image *) NULL);
  /*
    Determine range of matrix.
  */
  (void) GetMatrixElement(matrix_info,0,0,&value);
  min_value=value;
  max_value=value;
  for (y=0; y < (ssize_t) matrix_info->rows; y++)
  {
    register ssize_t
      x;

    for (x=0; x < (ssize_t) matrix_info->columns; x++)
    {
      if (GetMatrixElement(matrix_info,x,y,&value) == MagickFalse)
        continue;
      if (value < min_value)
        min_value=value;
      else
        if (value > max_value)
          max_value=value;
    }
  }
  if ((min_value == 0.0) && (max_value == 0.0))
    scale_factor=0;
  else
    if (min_value == max_value)
      {
        scale_factor=(double) QuantumRange/min_value;
        min_value=0;
      }
    else
      scale_factor=(double) QuantumRange/(max_value-min_value);
  /*
    Convert matrix to image.
  */
  image=AcquireImage((ImageInfo *) NULL,exception);
  image->columns=matrix_info->columns;
  image->rows=matrix_info->rows;
  image->colorspace=GRAYColorspace;
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      value;

    register Quantum
      *q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetMatrixElement(matrix_info,x,y,&value) == MagickFalse)
        continue;
      value=scale_factor*(value-min_value);
      *q=ClampToQuantum(value);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N u l l M a t r i x                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NullMatrix() sets all elements of the matrix to zero.
%
%  The format of the ResetMagickMemory method is:
%
%      MagickBooleanType *NullMatrix(MatrixInfo *matrix_info)
%
%  A description of each parameter follows:
%
%    o matrix_info: the matrix.
%
*/
MagickExport MagickBooleanType NullMatrix(MatrixInfo *matrix_info)
{
  register ssize_t
    x;

  ssize_t
    count,
    y;

  unsigned char
    value;

  assert(matrix_info != (const MatrixInfo *) NULL);
  assert(matrix_info->signature == MagickCoreSignature);
  if (matrix_info->type != DiskCache)
    {
      (void) ResetMagickMemory(matrix_info->elements,0,(size_t)
        matrix_info->length);
      return(MagickTrue);
    }
  value=0;
  (void) lseek(matrix_info->file,0,SEEK_SET);
  for (y=0; y < (ssize_t) matrix_info->rows; y++)
  {
    for (x=0; x < (ssize_t) matrix_info->length; x++)
    {
      count=write(matrix_info->file,&value,sizeof(value));
      if (count != (ssize_t) sizeof(value))
        break;
    }
    if (x < (ssize_t) matrix_info->length)
      break;
  }
  return(y < (ssize_t) matrix_info->rows ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h M a g i c k M a t r i x                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishMagickMatrix() frees the previously acquired matrix (array of
%  pointers to arrays of doubles).
%
%  The format of the RelinquishMagickMatrix method is:
%
%      double **RelinquishMagickMatrix(double **matrix,
%        const size_t number_rows)
%
%  A description of each parameter follows:
%
%    o matrix: the matrix to relinquish
%
%    o number_rows: the first dimension of the acquired matrix (number of
%      pointers)
%
*/
MagickExport double **RelinquishMagickMatrix(double **matrix,
  const size_t number_rows)
{
  register ssize_t
    i;

  if (matrix == (double **) NULL )
    return(matrix);
  for (i=0; i < (ssize_t) number_rows; i++)
    matrix[i]=(double *) RelinquishMagickMemory(matrix[i]);
  matrix=(double **) RelinquishMagickMemory(matrix);
  return(matrix);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a t r i x E l e m e n t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMatrixElement() sets the specifed element in the matrix.
%
%  The format of the SetMatrixElement method is:
%
%      MagickBooleanType SetMatrixElement(const MatrixInfo *matrix_info,
%        const ssize_t x,const ssize_t y,void *value)
%
%  A description of each parameter follows:
%
%    o matrix_info: the matrix columns.
%
%    o x: the matrix x-offset.
%
%    o y: the matrix y-offset.
%
%    o value: set the matrix element to this value.
%
*/

MagickExport MagickBooleanType SetMatrixElement(const MatrixInfo *matrix_info,
  const ssize_t x,const ssize_t y,const void *value)
{
  MagickOffsetType
    count,
    i;

  assert(matrix_info != (const MatrixInfo *) NULL);
  assert(matrix_info->signature == MagickCoreSignature);
  i=(MagickOffsetType) y*matrix_info->columns+x;
  if ((i < 0) ||
      ((MagickSizeType) (i*matrix_info->stride) >= matrix_info->length))
    return(MagickFalse);
  if (matrix_info->type != DiskCache)
    {
      (void) memcpy((unsigned char *) matrix_info->elements+i*
        matrix_info->stride,value,matrix_info->stride);
      return(MagickTrue);
    }
  count=WriteMatrixElements(matrix_info,i*matrix_info->stride,
    matrix_info->stride,(unsigned char *) value);
  if (count != (MagickOffsetType) matrix_info->stride)
    return(MagickFalse);
  return(MagickTrue);
}

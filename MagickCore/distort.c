/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               DDDD   IIIII  SSSSS  TTTTT   OOO   RRRR   TTTTT               %
%               D   D    I    SS       T    O   O  R   R    T                 %
%               D   D    I     SSS     T    O   O  RRRR     T                 %
%               D   D    I       SS    T    O   O  R R      T                 %
%               DDDD   IIIII  SSSSS    T     OOO   R  R     T                 %
%                                                                             %
%                                                                             %
%                     MagickCore Image Distortion Methods                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                              Anthony Thyssen                                %
%                                 June 2007                                   %
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
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/distort.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/image.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/list.h"
#include "MagickCore/matrix.h"
#include "MagickCore/matrix-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resample-private.h"
#include "MagickCore/registry.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/shear.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"

/*
  Numerous internal routines for image distortions.
*/
static inline void AffineArgsToCoefficients(double *affine)
{
  /* map  external sx,ry,rx,sy,tx,ty  to  internal c0,c2,c4,c1,c3,c5 */
  double tmp[4];  /* note indexes  0 and 5 remain unchanged */
  tmp[0]=affine[1]; tmp[1]=affine[2]; tmp[2]=affine[3]; tmp[3]=affine[4];
  affine[3]=tmp[0]; affine[1]=tmp[1]; affine[4]=tmp[2]; affine[2]=tmp[3];
}

static inline void CoefficientsToAffineArgs(double *coeff)
{
  /* map  internal c0,c1,c2,c3,c4,c5  to  external sx,ry,rx,sy,tx,ty */
  double tmp[4];  /* note indexes 0 and 5 remain unchanged */
  tmp[0]=coeff[3]; tmp[1]=coeff[1]; tmp[2]=coeff[4]; tmp[3]=coeff[2];
  coeff[1]=tmp[0]; coeff[2]=tmp[1]; coeff[3]=tmp[2]; coeff[4]=tmp[3];
}
static void InvertAffineCoefficients(const double *coeff,double *inverse)
{
  /* From "Digital Image Warping" by George Wolberg, page 50 */
  double determinant;

  determinant=PerceptibleReciprocal(coeff[0]*coeff[4]-coeff[1]*coeff[3]);
  inverse[0]=determinant*coeff[4];
  inverse[1]=determinant*(-coeff[1]);
  inverse[2]=determinant*(coeff[1]*coeff[5]-coeff[2]*coeff[4]);
  inverse[3]=determinant*(-coeff[3]);
  inverse[4]=determinant*coeff[0];
  inverse[5]=determinant*(coeff[2]*coeff[3]-coeff[0]*coeff[5]);
}

static void InvertPerspectiveCoefficients(const double *coeff,
  double *inverse)
{
  /* From "Digital Image Warping" by George Wolberg, page 53 */
  double determinant;

  determinant=PerceptibleReciprocal(coeff[0]*coeff[4]-coeff[3]*coeff[1]);
  inverse[0]=determinant*(coeff[4]-coeff[7]*coeff[5]);
  inverse[1]=determinant*(coeff[7]*coeff[2]-coeff[1]);
  inverse[2]=determinant*(coeff[1]*coeff[5]-coeff[4]*coeff[2]);
  inverse[3]=determinant*(coeff[6]*coeff[5]-coeff[3]);
  inverse[4]=determinant*(coeff[0]-coeff[6]*coeff[2]);
  inverse[5]=determinant*(coeff[3]*coeff[2]-coeff[0]*coeff[5]);
  inverse[6]=determinant*(coeff[3]*coeff[7]-coeff[6]*coeff[4]);
  inverse[7]=determinant*(coeff[6]*coeff[1]-coeff[0]*coeff[7]);
}

/*
 * Polynomial Term Defining Functions
 *
 * Order must either be an integer, or 1.5 to produce
 * the 2 number_valuesal polynomial function...
 *    affine     1   (3)      u = c0 + c1*x + c2*y
 *    bilinear   1.5 (4)      u = '' + c3*x*y
 *    quadratic  2   (6)      u = '' + c4*x*x + c5*y*y
 *    cubic      3   (10)     u = '' + c6*x^3 + c7*x*x*y + c8*x*y*y + c9*y^3
 *    quartic    4   (15)     u = '' + c10*x^4 + ... + c14*y^4
 *    quintic    5   (21)     u = '' + c15*x^5 + ... + c20*y^5
 * number in parenthesis minimum number of points needed.
 * Anything beyond quintic, has not been implemented until
 * a more automated way of determining terms is found.

 * Note the slight re-ordering of the terms for a quadratic polynomial
 * which is to allow the use of a bi-linear (order=1.5) polynomial.
 * All the later polynomials are ordered simply from x^N to y^N
 */
static size_t poly_number_terms(double order)
{
 /* Return the number of terms for a 2d polynomial */
  if ( order < 1 || order > 5 ||
       ( order != floor(order) && (order-1.5) > MagickEpsilon) )
    return 0; /* invalid polynomial order */
  return((size_t) floor((order+1)*(order+2)/2));
}

static double poly_basis_fn(ssize_t n, double x, double y)
{
  /* Return the result for this polynomial term */
  switch(n) {
    case  0:  return( 1.0 ); /* constant */
    case  1:  return(  x  );
    case  2:  return(  y  ); /* affine          order = 1   terms = 3 */
    case  3:  return( x*y ); /* bilinear        order = 1.5 terms = 4 */
    case  4:  return( x*x );
    case  5:  return( y*y ); /* quadratic       order = 2   terms = 6 */
    case  6:  return( x*x*x );
    case  7:  return( x*x*y );
    case  8:  return( x*y*y );
    case  9:  return( y*y*y ); /* cubic         order = 3   terms = 10 */
    case 10:  return( x*x*x*x );
    case 11:  return( x*x*x*y );
    case 12:  return( x*x*y*y );
    case 13:  return( x*y*y*y );
    case 14:  return( y*y*y*y ); /* quartic     order = 4   terms = 15 */
    case 15:  return( x*x*x*x*x );
    case 16:  return( x*x*x*x*y );
    case 17:  return( x*x*x*y*y );
    case 18:  return( x*x*y*y*y );
    case 19:  return( x*y*y*y*y );
    case 20:  return( y*y*y*y*y ); /* quintic   order = 5   terms = 21 */
  }
  return( 0 ); /* should never happen */
}
static const char *poly_basis_str(ssize_t n)
{
  /* return the result for this polynomial term */
  switch(n) {
    case  0:  return(""); /* constant */
    case  1:  return("*ii");
    case  2:  return("*jj"); /* affine                order = 1   terms = 3 */
    case  3:  return("*ii*jj"); /* bilinear           order = 1.5 terms = 4 */
    case  4:  return("*ii*ii");
    case  5:  return("*jj*jj"); /* quadratic          order = 2   terms = 6 */
    case  6:  return("*ii*ii*ii");
    case  7:  return("*ii*ii*jj");
    case  8:  return("*ii*jj*jj");
    case  9:  return("*jj*jj*jj"); /* cubic           order = 3   terms = 10 */
    case 10:  return("*ii*ii*ii*ii");
    case 11:  return("*ii*ii*ii*jj");
    case 12:  return("*ii*ii*jj*jj");
    case 13:  return("*ii*jj*jj*jj");
    case 14:  return("*jj*jj*jj*jj"); /* quartic      order = 4   terms = 15 */
    case 15:  return("*ii*ii*ii*ii*ii");
    case 16:  return("*ii*ii*ii*ii*jj");
    case 17:  return("*ii*ii*ii*jj*jj");
    case 18:  return("*ii*ii*jj*jj*jj");
    case 19:  return("*ii*jj*jj*jj*jj");
    case 20:  return("*jj*jj*jj*jj*jj"); /* quintic   order = 5   terms = 21 */
  }
  return( "UNKNOWN" ); /* should never happen */
}
static double poly_basis_dx(ssize_t n, double x, double y)
{
  /* polynomial term for x derivative */
  switch(n) {
    case  0:  return( 0.0 ); /* constant */
    case  1:  return( 1.0 );
    case  2:  return( 0.0 ); /* affine      order = 1   terms = 3 */
    case  3:  return(  y  ); /* bilinear    order = 1.5 terms = 4 */
    case  4:  return(  x  );
    case  5:  return( 0.0 ); /* quadratic   order = 2   terms = 6 */
    case  6:  return( x*x );
    case  7:  return( x*y );
    case  8:  return( y*y );
    case  9:  return( 0.0 ); /* cubic       order = 3   terms = 10 */
    case 10:  return( x*x*x );
    case 11:  return( x*x*y );
    case 12:  return( x*y*y );
    case 13:  return( y*y*y );
    case 14:  return( 0.0 ); /* quartic     order = 4   terms = 15 */
    case 15:  return( x*x*x*x );
    case 16:  return( x*x*x*y );
    case 17:  return( x*x*y*y );
    case 18:  return( x*y*y*y );
    case 19:  return( y*y*y*y );
    case 20:  return( 0.0 ); /* quintic     order = 5   terms = 21 */
  }
  return( 0.0 ); /* should never happen */
}
static double poly_basis_dy(ssize_t n, double x, double y)
{
  /* polynomial term for y derivative */
  switch(n) {
    case  0:  return( 0.0 ); /* constant */
    case  1:  return( 0.0 );
    case  2:  return( 1.0 ); /* affine      order = 1   terms = 3 */
    case  3:  return(  x  ); /* bilinear    order = 1.5 terms = 4 */
    case  4:  return( 0.0 );
    case  5:  return(  y  ); /* quadratic   order = 2   terms = 6 */
    default:  return( poly_basis_dx(n-1,x,y) ); /* weird but true */
  }
  /* NOTE: the only reason that last is not true for 'quadratic'
     is due to the re-arrangement of terms to allow for 'bilinear'
  */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A f f i n e T r a n s f o r m I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AffineTransformImage() transforms an image as dictated by the affine matrix.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the AffineTransformImage method is:
%
%      Image *AffineTransformImage(const Image *image,
%        AffineMatrix *affine_matrix,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o affine_matrix: the affine matrix.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AffineTransformImage(const Image *image,
  const AffineMatrix *affine_matrix,ExceptionInfo *exception)
{
  double
    distort[6];

  Image
    *deskew_image;

  /*
    Affine transform image.
  */
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(affine_matrix != (AffineMatrix *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  distort[0]=affine_matrix->sx;
  distort[1]=affine_matrix->rx;
  distort[2]=affine_matrix->ry;
  distort[3]=affine_matrix->sy;
  distort[4]=affine_matrix->tx;
  distort[5]=affine_matrix->ty;
  deskew_image=DistortImage(image,AffineProjectionDistortion,6,distort,
    MagickTrue,exception);
  return(deskew_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e n e r a t e C o e f f i c i e n t s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateCoefficients() takes user provided input arguments and generates
%  the coefficients, needed to apply the specific distortion for either
%  distorting images (generally using control points) or generating a color
%  gradient from sparsely separated color points.
%
%  The format of the GenerateCoefficients() method is:
%
%    Image *GenerateCoefficients(const Image *image,DistortMethod method,
%        const size_t number_arguments,const double *arguments,
%        size_t number_values, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to be distorted.
%
%    o method: the method of image distortion/ sparse gradient
%
%    o number_arguments: the number of arguments given.
%
%    o arguments: the arguments for this distortion method.
%
%    o number_values: the style and format of given control points, (caller type)
%         0: 2 dimensional mapping of control points (Distort)
%            Format:  u,v,x,y  where u,v is the 'source' of the
%            the color to be plotted, for DistortImage()
%         N: Interpolation of control points with N values (usally r,g,b)
%            Format: x,y,r,g,b    mapping x,y to color values r,g,b
%            IN future, variable number of values may be given (1 to N)
%
%    o exception: return any errors or warnings in this structure
%
%  Note that the returned array of double values must be freed by the
%  calling method using RelinquishMagickMemory().  This however may change in
%  the future to require a more 'method' specific method.
%
%  Because of this this method should not be classed as stable or used
%  outside other MagickCore library methods.
*/

static inline double MagickRound(double x)
{
  /*
    Round the fraction to nearest integer.
  */
  if ((x-floor(x)) < (ceil(x)-x))
    return(floor(x));
  return(ceil(x));
}

static double *GenerateCoefficients(const Image *image,
  DistortMethod *method,const size_t number_arguments,const double *arguments,
  size_t number_values,ExceptionInfo *exception)
{
  double
    *coeff;

  register size_t
    i;

  size_t
    number_coeff, /* number of coefficients to return (array size) */
    cp_size,      /* number floating point numbers per control point */
    cp_x,cp_y,    /* the x,y indexes for control point */
    cp_values;    /* index of values for this control point */
    /* number_values   Number of values given per control point */

  if ( number_values == 0 ) {
    /* Image distortion using control points (or other distortion)
       That is generate a mapping so that   x,y->u,v   given  u,v,x,y
    */
    number_values = 2;   /* special case: two values of u,v */
    cp_values = 0;       /* the values i,j are BEFORE the destination CP x,y */
    cp_x = 2;            /* location of x,y in input control values */
    cp_y = 3;
    /* NOTE: cp_values, also used for later 'reverse map distort' tests */
  }
  else {
    cp_x = 0;            /* location of x,y in input control values */
    cp_y = 1;
    cp_values = 2;       /* and the other values are after x,y */
    /* Typically in this case the values are R,G,B color values */
  }
  cp_size = number_values+2; /* each CP defintion involves this many numbers */

  /* If not enough control point pairs are found for specific distortions
     fall back to Affine distortion (allowing 0 to 3 point pairs)
  */
  if ( number_arguments < 4*cp_size &&
       (  *method == BilinearForwardDistortion
       || *method == BilinearReverseDistortion
       || *method == PerspectiveDistortion
       ) )
    *method = AffineDistortion;

  number_coeff=0;
  switch (*method) {
    case AffineDistortion:
    /* also BarycentricColorInterpolate: */
      number_coeff=3*number_values;
      break;
    case PolynomialDistortion:
      /* number of coefficents depend on the given polynomal 'order' */
      i = poly_number_terms(arguments[0]);
      number_coeff = 2 + i*number_values;
      if ( i == 0 ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                   "InvalidArgument","%s : '%s'","Polynomial",
                   "Invalid order, should be interger 1 to 5, or 1.5");
        return((double *) NULL);
      }
      if ( number_arguments < 1+i*cp_size ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
               "InvalidArgument", "%s : 'require at least %.20g CPs'",
               "Polynomial", (double) i);
        return((double *) NULL);
      }
      break;
    case BilinearReverseDistortion:
      number_coeff=4*number_values;
      break;
    /*
      The rest are constants as they are only used for image distorts
    */
    case BilinearForwardDistortion:
      number_coeff=10; /* 2*4 coeff plus 2 constants */
      cp_x = 0;        /* Reverse src/dest coords for forward mapping */
      cp_y = 1;
      cp_values = 2;
      break;
#if 0
    case QuadraterialDistortion:
      number_coeff=19; /* BilinearForward + BilinearReverse */
#endif
      break;
    case ShepardsDistortion:
      number_coeff=1;  /* The power factor to use */
      break;
    case ArcDistortion:
      number_coeff=5;
      break;
    case ScaleRotateTranslateDistortion:
    case AffineProjectionDistortion:
    case Plane2CylinderDistortion:
    case Cylinder2PlaneDistortion:
      number_coeff=6;
      break;
    case PolarDistortion:
    case DePolarDistortion:
      number_coeff=8;
      break;
    case PerspectiveDistortion:
    case PerspectiveProjectionDistortion:
      number_coeff=9;
      break;
    case BarrelDistortion:
    case BarrelInverseDistortion:
      number_coeff=10;
      break;
    default:
      perror("unknown method given"); /* just fail assertion */
  }

  /* allocate the array of coefficients needed */
  coeff = (double *) AcquireQuantumMemory(number_coeff,sizeof(*coeff));
  if (coeff == (double *) NULL) {
    (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "GenerateCoefficients");
    return((double *) NULL);
  }

  /* zero out coefficients array */
  for (i=0; i < number_coeff; i++)
    coeff[i] = 0.0;

  switch (*method)
  {
    case AffineDistortion:
    {
      /* Affine Distortion
           v =  c0*x + c1*y + c2
         for each 'value' given

         Input Arguments are sets of control points...
         For Distort Images    u,v, x,y  ...
         For Sparse Gradients  x,y, r,g,b  ...
      */
      if ( number_arguments%cp_size != 0 ||
           number_arguments < cp_size ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
               "InvalidArgument", "%s : 'require at least %.20g CPs'",
               "Affine", 1.0);
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      /* handle special cases of not enough arguments */
      if ( number_arguments == cp_size ) {
        /* Only 1 CP Set Given */
        if ( cp_values == 0 ) {
          /* image distortion - translate the image */
          coeff[0] = 1.0;
          coeff[2] = arguments[0] - arguments[2];
          coeff[4] = 1.0;
          coeff[5] = arguments[1] - arguments[3];
        }
        else {
          /* sparse gradient - use the values directly */
          for (i=0; i<number_values; i++)
            coeff[i*3+2] = arguments[cp_values+i];
        }
      }
      else {
        /* 2 or more points (usally 3) given.
           Solve a least squares simultaneous equation for coefficients.
        */
        double
          **matrix,
          **vectors,
          terms[3];

        MagickBooleanType
          status;

        /* create matrix, and a fake vectors matrix */
        matrix = AcquireMagickMatrix(3UL,3UL);
        vectors = (double **) AcquireQuantumMemory(number_values,sizeof(*vectors));
        if (matrix == (double **) NULL || vectors == (double **) NULL)
        {
          matrix  = RelinquishMagickMatrix(matrix, 3UL);
          vectors = (double **) RelinquishMagickMemory(vectors);
          coeff   = (double *) RelinquishMagickMemory(coeff);
          (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
          return((double *) NULL);
        }
        /* fake a number_values x3 vectors matrix from coefficients array */
        for (i=0; i < number_values; i++)
          vectors[i] = &(coeff[i*3]);
        /* Add given control point pairs for least squares solving */
        for (i=0; i < number_arguments; i+=cp_size) {
          terms[0] = arguments[i+cp_x];  /* x */
          terms[1] = arguments[i+cp_y];  /* y */
          terms[2] = 1;                  /* 1 */
          LeastSquaresAddTerms(matrix,vectors,terms,
                   &(arguments[i+cp_values]),3UL,number_values);
        }
        if ( number_arguments == 2*cp_size ) {
          /* Only two pairs were given, but we need 3 to solve the affine.
             Fake extra coordinates by rotating p1 around p0 by 90 degrees.
               x2 = x0 - (y1-y0)   y2 = y0 + (x1-x0)
           */
          terms[0] = arguments[cp_x]
                   - ( arguments[cp_size+cp_y] - arguments[cp_y] ); /* x2 */
          terms[1] = arguments[cp_y] +
                   + ( arguments[cp_size+cp_x] - arguments[cp_x] ); /* y2 */
          terms[2] = 1;                                             /* 1 */
          if ( cp_values == 0 ) {
            /* Image Distortion - rotate the u,v coordients too */
            double
              uv2[2];
            uv2[0] = arguments[0] - arguments[5] + arguments[1];   /* u2 */
            uv2[1] = arguments[1] + arguments[4] - arguments[0];   /* v2 */
            LeastSquaresAddTerms(matrix,vectors,terms,uv2,3UL,2UL);
          }
          else {
            /* Sparse Gradient - use values of p0 for linear gradient */
            LeastSquaresAddTerms(matrix,vectors,terms,
                  &(arguments[cp_values]),3UL,number_values);
          }
        }
        /* Solve for LeastSquares Coefficients */
        status=GaussJordanElimination(matrix,vectors,3UL,number_values);
        matrix = RelinquishMagickMatrix(matrix, 3UL);
        vectors = (double **) RelinquishMagickMemory(vectors);
        if ( status == MagickFalse ) {
          coeff = (double *) RelinquishMagickMemory(coeff);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument","%s : 'Unsolvable Matrix'",
              CommandOptionToMnemonic(MagickDistortOptions, *method) );
          return((double *) NULL);
        }
      }
      return(coeff);
    }
    case AffineProjectionDistortion:
    {
      /*
        Arguments: Affine Matrix (forward mapping)
        Arguments  sx, rx, ry, sy, tx, ty
        Where      u = sx*x + ry*y + tx
                   v = rx*x + sy*y + ty

        Returns coefficients (in there inverse form) ordered as...
             sx ry tx  rx sy ty

        AffineProjection Distortion Notes...
           + Will only work with a 2 number_values for Image Distortion
           + Can not be used for generating a sparse gradient (interpolation)
      */
      double inverse[8];
      if (number_arguments != 6) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument","%s : 'Needs 6 coeff values'",
              CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      }
      /* FUTURE: trap test for sx*sy-rx*ry == 0 (determinant = 0, no inverse) */
      for(i=0; i<6UL; i++ )
        inverse[i] = arguments[i];
      AffineArgsToCoefficients(inverse); /* map into coefficents */
      InvertAffineCoefficients(inverse, coeff); /* invert */
      *method = AffineDistortion;

      return(coeff);
    }
    case ScaleRotateTranslateDistortion:
    {
      /* Scale, Rotate and Translate Distortion
         An alternative Affine Distortion
         Argument options, by number of arguments given:
           7: x,y, sx,sy, a, nx,ny
           6: x,y,   s,   a, nx,ny
           5: x,y, sx,sy, a
           4: x,y,   s,   a
           3: x,y,        a
           2:        s,   a
           1:             a
         Where actions are (in order of application)
            x,y     'center' of transforms     (default = image center)
            sx,sy   scale image by this amount (default = 1)
            a       angle of rotation          (argument required)
            nx,ny   move 'center' here         (default = x,y or no movement)
         And convert to affine mapping coefficients

         ScaleRotateTranslate Distortion Notes...
           + Does not use a set of CPs in any normal way
           + Will only work with a 2 number_valuesal Image Distortion
           + Cannot be used for generating a sparse gradient (interpolation)
      */
      double
        cosine, sine,
        x,y,sx,sy,a,nx,ny;

      /* set default center, and default scale */
      x = nx = (double)(image->columns)/2.0 + (double)image->page.x;
      y = ny = (double)(image->rows)/2.0    + (double)image->page.y;
      sx = sy = 1.0;
      switch ( number_arguments ) {
      case 0:
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument","%s : 'Needs at least 1 argument'",
              CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      case 1:
        a = arguments[0];
        break;
      case 2:
        sx = sy = arguments[0];
        a = arguments[1];
        break;
      default:
        x = nx = arguments[0];
        y = ny = arguments[1];
        switch ( number_arguments ) {
        case 3:
          a = arguments[2];
          break;
        case 4:
          sx = sy = arguments[2];
          a = arguments[3];
          break;
        case 5:
          sx = arguments[2];
          sy = arguments[3];
          a = arguments[4];
          break;
        case 6:
          sx = sy = arguments[2];
          a = arguments[3];
          nx = arguments[4];
          ny = arguments[5];
          break;
        case 7:
          sx = arguments[2];
          sy = arguments[3];
          a = arguments[4];
          nx = arguments[5];
          ny = arguments[6];
          break;
        default:
          coeff = (double *) RelinquishMagickMemory(coeff);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument","%s : 'Too Many Arguments (7 or less)'",
              CommandOptionToMnemonic(MagickDistortOptions, *method) );
          return((double *) NULL);
        }
        break;
      }
      /* Trap if sx or sy == 0 -- image is scaled out of existance! */
      if ( fabs(sx) < MagickEpsilon || fabs(sy) < MagickEpsilon ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument","%s : 'Zero Scale Given'",
              CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      }
      /* Save the given arguments as an affine distortion */
      a=DegreesToRadians(a); cosine=cos(a); sine=sin(a);

      *method = AffineDistortion;
      coeff[0]=cosine/sx;
      coeff[1]=sine/sx;
      coeff[2]=x-nx*coeff[0]-ny*coeff[1];
      coeff[3]=(-sine)/sy;
      coeff[4]=cosine/sy;
      coeff[5]=y-nx*coeff[3]-ny*coeff[4];
      return(coeff);
    }
    case PerspectiveDistortion:
    { /*
         Perspective Distortion (a ratio of affine distortions)

                p(x,y)    c0*x + c1*y + c2
            u = ------ = ------------------
                r(x,y)    c6*x + c7*y + 1

                q(x,y)    c3*x + c4*y + c5
            v = ------ = ------------------
                r(x,y)    c6*x + c7*y + 1

           c8 = Sign of 'r', or the denominator affine, for the actual image.
                This determines what part of the distorted image is 'ground'
                side of the horizon, the other part is 'sky' or invalid.
                Valid values are  +1.0  or  -1.0  only.

         Input Arguments are sets of control points...
         For Distort Images    u,v, x,y  ...
         For Sparse Gradients  x,y, r,g,b  ...

         Perspective Distortion Notes...
           + Can be thought of as ratio of  3 affine transformations
           + Not separatable: r() or c6 and c7 are used by both equations
           + All 8 coefficients must be determined simultaniously
           + Will only work with a 2 number_valuesal Image Distortion
           + Can not be used for generating a sparse gradient (interpolation)
           + It is not linear, but is simple to generate an inverse
           + All lines within an image remain lines.
           + but distances between points may vary.
      */
      double
        **matrix,
        *vectors[1],
        terms[8];

      size_t
        cp_u = cp_values,
        cp_v = cp_values+1;

      MagickBooleanType
        status;

      if ( number_arguments%cp_size != 0 ||
           number_arguments < cp_size*4 ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument", "%s : 'require at least %.20g CPs'",
              CommandOptionToMnemonic(MagickDistortOptions, *method), 4.0);
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      /* fake 1x8 vectors matrix directly using the coefficients array */
      vectors[0] = &(coeff[0]);
      /* 8x8 least-squares matrix (zeroed) */
      matrix = AcquireMagickMatrix(8UL,8UL);
      if (matrix == (double **) NULL) {
        coeff=(double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
        return((double *) NULL);
      }
      /* Add control points for least squares solving */
      for (i=0; i < number_arguments; i+=4) {
        terms[0]=arguments[i+cp_x];            /*   c0*x   */
        terms[1]=arguments[i+cp_y];            /*   c1*y   */
        terms[2]=1.0;                          /*   c2*1   */
        terms[3]=0.0;
        terms[4]=0.0;
        terms[5]=0.0;
        terms[6]=-terms[0]*arguments[i+cp_u];  /* 1/(c6*x) */
        terms[7]=-terms[1]*arguments[i+cp_u];  /* 1/(c7*y) */
        LeastSquaresAddTerms(matrix,vectors,terms,&(arguments[i+cp_u]),
            8UL,1UL);

        terms[0]=0.0;
        terms[1]=0.0;
        terms[2]=0.0;
        terms[3]=arguments[i+cp_x];           /*   c3*x   */
        terms[4]=arguments[i+cp_y];           /*   c4*y   */
        terms[5]=1.0;                         /*   c5*1   */
        terms[6]=-terms[3]*arguments[i+cp_v]; /* 1/(c6*x) */
        terms[7]=-terms[4]*arguments[i+cp_v]; /* 1/(c7*y) */
        LeastSquaresAddTerms(matrix,vectors,terms,&(arguments[i+cp_v]),
            8UL,1UL);
      }
      /* Solve for LeastSquares Coefficients */
      status=GaussJordanElimination(matrix,vectors,8UL,1UL);
      matrix = RelinquishMagickMatrix(matrix, 8UL);
      if ( status == MagickFalse ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidArgument","%s : 'Unsolvable Matrix'",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      }
      /*
        Calculate 9'th coefficient! The ground-sky determination.
        What is sign of the 'ground' in r() denominator affine function?
        Just use any valid image coordinate (first control point) in
        destination for determination of what part of view is 'ground'.
      */
      coeff[8] = coeff[6]*arguments[cp_x]
                      + coeff[7]*arguments[cp_y] + 1.0;
      coeff[8] = (coeff[8] < 0.0) ? -1.0 : +1.0;

      return(coeff);
    }
    case PerspectiveProjectionDistortion:
    {
      /*
        Arguments: Perspective Coefficents (forward mapping)
      */
      if (number_arguments != 8) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument", "%s : 'Needs 8 coefficient values'",
              CommandOptionToMnemonic(MagickDistortOptions, *method));
        return((double *) NULL);
      }
      /* FUTURE: trap test  c0*c4-c3*c1 == 0  (determinate = 0, no inverse) */
      InvertPerspectiveCoefficients(arguments, coeff);
      /*
        Calculate 9'th coefficient! The ground-sky determination.
        What is sign of the 'ground' in r() denominator affine function?
        Just use any valid image cocodinate in destination for determination.
        For a forward mapped perspective the images 0,0 coord will map to
        c2,c5 in the distorted image, so set the sign of denominator of that.
      */
      coeff[8] = coeff[6]*arguments[2]
                           + coeff[7]*arguments[5] + 1.0;
      coeff[8] = (coeff[8] < 0.0) ? -1.0 : +1.0;
      *method = PerspectiveDistortion;

      return(coeff);
    }
    case BilinearForwardDistortion:
    case BilinearReverseDistortion:
    {
      /* Bilinear Distortion (Forward mapping)
            v = c0*x + c1*y + c2*x*y + c3;
         for each 'value' given

         This is actually a simple polynomial Distortion!  The difference
         however is when we need to reverse the above equation to generate a
         BilinearForwardDistortion (see below).

         Input Arguments are sets of control points...
         For Distort Images    u,v, x,y  ...
         For Sparse Gradients  x,y, r,g,b  ...

      */
      double
        **matrix,
        **vectors,
        terms[4];

      MagickBooleanType
        status;

      /* check the number of arguments */
      if ( number_arguments%cp_size != 0 ||
           number_arguments < cp_size*4 ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument", "%s : 'require at least %.20g CPs'",
              CommandOptionToMnemonic(MagickDistortOptions, *method), 4.0);
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      /* create matrix, and a fake vectors matrix */
      matrix = AcquireMagickMatrix(4UL,4UL);
      vectors = (double **) AcquireQuantumMemory(number_values,sizeof(*vectors));
      if (matrix == (double **) NULL || vectors == (double **) NULL)
      {
        matrix  = RelinquishMagickMatrix(matrix, 4UL);
        vectors = (double **) RelinquishMagickMemory(vectors);
        coeff   = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed",
                "%s", "DistortCoefficients");
        return((double *) NULL);
      }
      /* fake a number_values x4 vectors matrix from coefficients array */
      for (i=0; i < number_values; i++)
        vectors[i] = &(coeff[i*4]);
      /* Add given control point pairs for least squares solving */
      for (i=0; i < number_arguments; i+=cp_size) {
        terms[0] = arguments[i+cp_x];   /*  x  */
        terms[1] = arguments[i+cp_y];   /*  y  */
        terms[2] = terms[0]*terms[1];   /* x*y */
        terms[3] = 1;                   /*  1  */
        LeastSquaresAddTerms(matrix,vectors,terms,
             &(arguments[i+cp_values]),4UL,number_values);
      }
      /* Solve for LeastSquares Coefficients */
      status=GaussJordanElimination(matrix,vectors,4UL,number_values);
      matrix  = RelinquishMagickMatrix(matrix, 4UL);
      vectors = (double **) RelinquishMagickMemory(vectors);
      if ( status == MagickFalse ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidArgument","%s : 'Unsolvable Matrix'",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      }
      if ( *method == BilinearForwardDistortion ) {
         /* Bilinear Forward Mapped Distortion

         The above least-squares solved for coefficents but in the forward
         direction, due to changes to indexing constants.

            i = c0*x + c1*y + c2*x*y + c3;
            j = c4*x + c5*y + c6*x*y + c7;

         where i,j are in the destination image, NOT the source.

         Reverse Pixel mapping however needs to use reverse of these
         functions.  It required a full page of algbra to work out the
         reversed mapping formula, but resolves down to the following...

            c8 = c0*c5-c1*c4;
            c9 = 2*(c2*c5-c1*c6);   // '2*a' in the quadratic formula

            i = i - c3;   j = j - c7;
            b = c6*i - c2*j + c8;   // So that   a*y^2 + b*y + c == 0
            c = c4*i -  c0*j;       // y = ( -b +- sqrt(bb - 4ac) ) / (2*a)

            r = b*b - c9*(c+c);
            if ( c9 != 0 )
              y = ( -b + sqrt(r) ) / c9;
            else
              y = -c/b;

            x = ( i - c1*y) / ( c1 - c2*y );

         NB: if 'r' is negative there is no solution!
         NB: the sign of the sqrt() should be negative if image becomes
             flipped or flopped, or crosses over itself.
         NB: techniqually coefficient c5 is not needed, anymore,
             but kept for completness.

         See Anthony Thyssen <A.Thyssen@griffith.edu.au>
         or  Fred Weinhaus <fmw@alink.net>  for more details.

         */
         coeff[8] = coeff[0]*coeff[5] - coeff[1]*coeff[4];
         coeff[9] = 2*(coeff[2]*coeff[5] - coeff[1]*coeff[6]);
      }
      return(coeff);
    }
#if 0
    case QuadrilateralDistortion:
    {
      /* Map a Quadrilateral to a unit square using BilinearReverse
         Then map that unit square back to the final Quadrilateral
         using BilinearForward.

         Input Arguments are sets of control points...
         For Distort Images    u,v, x,y  ...
         For Sparse Gradients  x,y, r,g,b  ...

      */
      /* UNDER CONSTRUCTION */
      return(coeff);
    }
#endif

    case PolynomialDistortion:
    {
      /* Polynomial Distortion

         First two coefficents are used to hole global polynomal information
           c0 = Order of the polynimial being created
           c1 = number_of_terms in one polynomial equation

         Rest of the coefficients map to the equations....
            v = c0 + c1*x + c2*y + c3*x*y + c4*x^2 + c5*y^2 + c6*x^3 + ...
         for each 'value' (number_values of them) given.
         As such total coefficients =  2 + number_terms * number_values

         Input Arguments are sets of control points...
         For Distort Images    order  [u,v, x,y] ...
         For Sparse Gradients  order  [x,y, r,g,b] ...

         Polynomial Distortion Notes...
           + UNDER DEVELOPMENT -- Do not expect this to remain as is.
           + Currently polynomial is a reversed mapped distortion.
           + Order 1.5 is fudged to map into a bilinear distortion.
             though it is not the same order as that distortion.
      */
      double
        **matrix,
        **vectors,
        *terms;

      size_t
        nterms;   /* number of polynomial terms per number_values */

      register ssize_t
        j;

      MagickBooleanType
        status;

      /* first two coefficients hold polynomial order information */
      coeff[0] = arguments[0];
      coeff[1] = (double) poly_number_terms(arguments[0]);
      nterms = (size_t) coeff[1];

      /* create matrix, a fake vectors matrix, and least sqs terms */
      matrix = AcquireMagickMatrix(nterms,nterms);
      vectors = (double **) AcquireQuantumMemory(number_values,sizeof(*vectors));
      terms = (double *) AcquireQuantumMemory(nterms, sizeof(*terms));
      if (matrix  == (double **) NULL ||
          vectors == (double **) NULL ||
          terms   == (double *) NULL )
      {
        matrix  = RelinquishMagickMatrix(matrix, nterms);
        vectors = (double **) RelinquishMagickMemory(vectors);
        terms   = (double *) RelinquishMagickMemory(terms);
        coeff   = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed",
                "%s", "DistortCoefficients");
        return((double *) NULL);
      }
      /* fake a number_values x3 vectors matrix from coefficients array */
      for (i=0; i < number_values; i++)
        vectors[i] = &(coeff[2+i*nterms]);
      /* Add given control point pairs for least squares solving */
      for (i=1; i < number_arguments; i+=cp_size) { /* NB: start = 1 not 0 */
        for (j=0; j < (ssize_t) nterms; j++)
          terms[j] = poly_basis_fn(j,arguments[i+cp_x],arguments[i+cp_y]);
        LeastSquaresAddTerms(matrix,vectors,terms,
             &(arguments[i+cp_values]),nterms,number_values);
      }
      terms = (double *) RelinquishMagickMemory(terms);
      /* Solve for LeastSquares Coefficients */
      status=GaussJordanElimination(matrix,vectors,nterms,number_values);
      matrix  = RelinquishMagickMatrix(matrix, nterms);
      vectors = (double **) RelinquishMagickMemory(vectors);
      if ( status == MagickFalse ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidArgument","%s : 'Unsolvable Matrix'",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      }
      return(coeff);
    }
    case ArcDistortion:
    {
      /* Arc Distortion
         Args: arc_width  rotate  top_edge_radius  bottom_edge_radius
         All but first argument are optional
            arc_width      The angle over which to arc the image side-to-side
            rotate         Angle to rotate image from vertical center
            top_radius     Set top edge of source image at this radius
            bottom_radius  Set bootom edge to this radius (radial scaling)

         By default, if the radii arguments are nor provided the image radius
         is calculated so the horizontal center-line is fits the given arc
         without scaling.

         The output image size is ALWAYS adjusted to contain the whole image,
         and an offset is given to position image relative to the 0,0 point of
         the origin, allowing users to use relative positioning onto larger
         background (via -flatten).

         The arguments are converted to these coefficients
            c0: angle for center of source image
            c1: angle scale for mapping to source image
            c2: radius for top of source image
            c3: radius scale for mapping source image
            c4: centerline of arc within source image

         Note the coefficients use a center angle, so asymptotic join is
         furthest from both sides of the source image. This also means that
         for arc angles greater than 360 the sides of the image will be
         trimmed equally.

         Arc Distortion Notes...
           + Does not use a set of CPs
           + Will only work with Image Distortion
           + Can not be used for generating a sparse gradient (interpolation)
      */
      if ( number_arguments >= 1 && arguments[0] < MagickEpsilon ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidArgument","%s : 'Arc Angle Too Small'",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      }
      if ( number_arguments >= 3 && arguments[2] < MagickEpsilon ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidArgument","%s : 'Outer Radius Too Small'",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        return((double *) NULL);
      }
      coeff[0] = -MagickPI2;   /* -90, place at top! */
      if ( number_arguments >= 1 )
        coeff[1] = DegreesToRadians(arguments[0]);
      else
        coeff[1] = MagickPI2;   /* zero arguments - center is at top */
      if ( number_arguments >= 2 )
        coeff[0] += DegreesToRadians(arguments[1]);
      coeff[0] /= Magick2PI;  /* normalize radians */
      coeff[0] -= MagickRound(coeff[0]);
      coeff[0] *= Magick2PI;  /* de-normalize back to radians */
      coeff[3] = (double)image->rows-1;
      coeff[2] = (double)image->columns/coeff[1] + coeff[3]/2.0;
      if ( number_arguments >= 3 ) {
        if ( number_arguments >= 4 )
          coeff[3] = arguments[2] - arguments[3];
        else
          coeff[3] *= arguments[2]/coeff[2];
        coeff[2] = arguments[2];
      }
      coeff[4] = ((double)image->columns-1.0)/2.0;

      return(coeff);
    }
    case PolarDistortion:
    case DePolarDistortion:
    {
      /* (De)Polar Distortion   (same set of arguments)
         Args:  Rmax, Rmin,  Xcenter,Ycenter,  Afrom,Ato
         DePolar can also have the extra arguments of Width, Height

         Coefficients 0 to 5 is the sanatized version first 6 input args
         Coefficient 6  is the angle to coord ratio  and visa-versa
         Coefficient 7  is the radius to coord ratio and visa-versa

         WARNING: It is possible for  Radius max<min  and/or  Angle from>to
      */
      if ( number_arguments == 3
          || ( number_arguments > 6 && *method == PolarDistortion )
          || number_arguments > 8 ) {
          (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError,"InvalidArgument", "%s : number of arguments",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      /* Rmax -  if 0 calculate appropriate value */
      if ( number_arguments >= 1 )
        coeff[0] = arguments[0];
      else
        coeff[0] = 0.0;
      /* Rmin  - usally 0 */
      coeff[1] = number_arguments >= 2 ? arguments[1] : 0.0;
      /* Center X,Y */
      if ( number_arguments >= 4 ) {
        coeff[2] = arguments[2];
        coeff[3] = arguments[3];
      }
      else { /* center of actual image */
        coeff[2] = (double)(image->columns)/2.0+image->page.x;
        coeff[3] = (double)(image->rows)/2.0+image->page.y;
      }
      /* Angle from,to - about polar center 0 is downward */
      coeff[4] = -MagickPI;
      if ( number_arguments >= 5 )
        coeff[4] = DegreesToRadians(arguments[4]);
      coeff[5] = coeff[4];
      if ( number_arguments >= 6 )
        coeff[5] = DegreesToRadians(arguments[5]);
      if ( fabs(coeff[4]-coeff[5]) < MagickEpsilon )
        coeff[5] += Magick2PI; /* same angle is a full circle */
      /* if radius 0 or negative,  its a special value... */
      if ( coeff[0] < MagickEpsilon ) {
        /* Use closest edge  if radius == 0 */
        if ( fabs(coeff[0]) < MagickEpsilon ) {
          coeff[0]=MagickMin(fabs(coeff[2]-image->page.x),
                             fabs(coeff[3]-image->page.y));
          coeff[0]=MagickMin(coeff[0],
                       fabs(coeff[2]-image->page.x-image->columns));
          coeff[0]=MagickMin(coeff[0],
                       fabs(coeff[3]-image->page.y-image->rows));
        }
        /* furthest diagonal if radius == -1 */
        if ( fabs(-1.0-coeff[0]) < MagickEpsilon ) {
          double rx,ry;
          rx = coeff[2]-image->page.x;
          ry = coeff[3]-image->page.y;
          coeff[0] = rx*rx+ry*ry;
          ry = coeff[3]-image->page.y-image->rows;
          coeff[0] = MagickMax(coeff[0],rx*rx+ry*ry);
          rx = coeff[2]-image->page.x-image->columns;
          coeff[0] = MagickMax(coeff[0],rx*rx+ry*ry);
          ry = coeff[3]-image->page.y;
          coeff[0] = MagickMax(coeff[0],rx*rx+ry*ry);
          coeff[0] = sqrt(coeff[0]);
        }
      }
      /* IF Rmax <= 0 or Rmin < 0 OR Rmax < Rmin, THEN error */
      if ( coeff[0] < MagickEpsilon || coeff[1] < -MagickEpsilon
           || (coeff[0]-coeff[1]) < MagickEpsilon ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidArgument", "%s : Invalid Radius",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      /* converstion ratios */
      if ( *method == PolarDistortion ) {
        coeff[6]=(double) image->columns/(coeff[5]-coeff[4]);
        coeff[7]=(double) image->rows/(coeff[0]-coeff[1]);
      }
      else { /* *method == DePolarDistortion */
        coeff[6]=(coeff[5]-coeff[4])/image->columns;
        coeff[7]=(coeff[0]-coeff[1])/image->rows;
      }
      return(coeff);
    }
    case Cylinder2PlaneDistortion:
    case Plane2CylinderDistortion:
    {
      /* 3D Cylinder to/from a Tangential Plane

         Projection between a clinder and flat plain from a point on the
         center line of the cylinder.

         The two surfaces coincide in 3D space at the given centers of
         distortion (perpendicular to projection point) on both images.

         Args:  FOV_arc_width
         Coefficents: FOV(radians), Radius, center_x,y, dest_center_x,y

         FOV (Field Of View) the angular field of view of the distortion,
         across the width of the image, in degrees.  The centers are the
         points of least distortion in the input and resulting images.

         These centers are however determined later.

         Coeff 0 is the FOV angle of view of image width in radians
         Coeff 1 is calculated radius of cylinder.
         Coeff 2,3  center of distortion of input image
         Coefficents 4,5 Center of Distortion of dest (determined later)
      */
      if ( arguments[0] < MagickEpsilon || arguments[0] > 160.0 ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidArgument", "%s : Invalid FOV Angle",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      coeff[0] = DegreesToRadians(arguments[0]);
      if ( *method == Cylinder2PlaneDistortion )
        /* image is curved around cylinder, so FOV angle (in radians)
         * scales directly to image X coordinate, according to its radius.
         */
        coeff[1] = (double) image->columns/coeff[0];
      else
        /* radius is distance away from an image with this angular FOV */
        coeff[1] = (double) image->columns / ( 2 * tan(coeff[0]/2) );

      coeff[2] = (double)(image->columns)/2.0+image->page.x;
      coeff[3] = (double)(image->rows)/2.0+image->page.y;
      coeff[4] = coeff[2];
      coeff[5] = coeff[3]; /* assuming image size is the same */
      return(coeff);
    }
    case BarrelDistortion:
    case BarrelInverseDistortion:
    {
      /* Barrel Distortion
           Rs=(A*Rd^3 + B*Rd^2 + C*Rd + D)*Rd
         BarrelInv Distortion
           Rs=Rd/(A*Rd^3 + B*Rd^2 + C*Rd + D)

        Where Rd is the normalized radius from corner to middle of image
        Input Arguments are one of the following forms (number of arguments)...
            3:  A,B,C
            4:  A,B,C,D
            5:  A,B,C    X,Y
            6:  A,B,C,D  X,Y
            8:  Ax,Bx,Cx,Dx  Ay,By,Cy,Dy
           10:  Ax,Bx,Cx,Dx  Ay,By,Cy,Dy   X,Y

        Returns 10 coefficent values, which are de-normalized (pixel scale)
          Ax, Bx, Cx, Dx,   Ay, By, Cy, Dy,    Xc, Yc
      */
      /* Radius de-normalization scaling factor */
      double
        rscale = 2.0/MagickMin((double) image->columns,(double) image->rows);

      /* sanity check  number of args must = 3,4,5,6,8,10 or error */
      if ( (number_arguments  < 3) || (number_arguments == 7) ||
           (number_arguments == 9) || (number_arguments > 10) )
        {
          coeff=(double *) RelinquishMagickMemory(coeff);
          (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError,"InvalidArgument", "%s : number of arguments",
            CommandOptionToMnemonic(MagickDistortOptions, *method) );
          return((double *) NULL);
        }
      /* A,B,C,D coefficients */
      coeff[0] = arguments[0];
      coeff[1] = arguments[1];
      coeff[2] = arguments[2];
      if ((number_arguments == 3) || (number_arguments == 5) )
        coeff[3] = 1.0 - coeff[0] - coeff[1] - coeff[2];
      else
        coeff[3] = arguments[3];
      /* de-normalize the coefficients */
      coeff[0] *= pow(rscale,3.0);
      coeff[1] *= rscale*rscale;
      coeff[2] *= rscale;
      /* Y coefficients: as given OR same as X coefficients */
      if ( number_arguments >= 8 ) {
        coeff[4] = arguments[4] * pow(rscale,3.0);
        coeff[5] = arguments[5] * rscale*rscale;
        coeff[6] = arguments[6] * rscale;
        coeff[7] = arguments[7];
      }
      else {
        coeff[4] = coeff[0];
        coeff[5] = coeff[1];
        coeff[6] = coeff[2];
        coeff[7] = coeff[3];
      }
      /* X,Y Center of Distortion (image coodinates) */
      if ( number_arguments == 5 )  {
        coeff[8] = arguments[3];
        coeff[9] = arguments[4];
      }
      else if ( number_arguments == 6 ) {
        coeff[8] = arguments[4];
        coeff[9] = arguments[5];
      }
      else if ( number_arguments == 10 ) {
        coeff[8] = arguments[8];
        coeff[9] = arguments[9];
      }
      else {
        /* center of the image provided (image coodinates) */
        coeff[8] = (double)image->columns/2.0 + image->page.x;
        coeff[9] = (double)image->rows/2.0    + image->page.y;
      }
      return(coeff);
    }
    case ShepardsDistortion:
    {
      /* Shepards Distortion  input arguments are the coefficents!
         Just check the number of arguments is valid!
         Args:  u1,v1, x1,y1, ...
          OR :  u1,v1, r1,g1,c1, ...
      */
      if ( number_arguments%cp_size != 0 ||
           number_arguments < cp_size ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "InvalidArgument", "%s : 'requires CP's (4 numbers each)'",
              CommandOptionToMnemonic(MagickDistortOptions, *method));
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      /* User defined weighting power for Shepard's Method */
      { const char *artifact=GetImageArtifact(image,"shepards:power");
        if ( artifact != (const char *) NULL ) {
          coeff[0]=StringToDouble(artifact,(char **) NULL) / 2.0;
          if ( coeff[0] < MagickEpsilon ) {
            (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"InvalidArgument","%s", "-define shepards:power" );
            coeff=(double *) RelinquishMagickMemory(coeff);
            return((double *) NULL);
          }
        }
        else
          coeff[0]=1.0;  /* Default power of 2 (Inverse Squared) */
      }
      return(coeff);
    }
    default:
      break;
  }
  /* you should never reach this point */
  perror("no method handler"); /* just fail assertion */
  return((double *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s t o r t R e s i z e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DistortResizeImage() resize image using the equivalent but slower image
%  distortion operator.  The filter is applied using a EWA cylindrical
%  resampling. But like resize the final image size is limited to whole pixels
%  with no effects by virtual-pixels on the result.
%
%  Note that images containing a transparency channel will be twice as slow to
%  resize as images one without transparency.
%
%  The format of the DistortResizeImage method is:
%
%      Image *DistortResizeImage(const Image *image,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the resized image.
%
%    o rows: the number of rows in the resized image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *DistortResizeImage(const Image *image,
  const size_t columns,const size_t rows,ExceptionInfo *exception)
{
#define DistortResizeImageTag  "Distort/Image"

  Image
    *resize_image,
    *tmp_image;

  RectangleInfo
    crop_area;

  double
    distort_args[12];

  VirtualPixelMethod
    vp_save;

  /*
    Distort resize image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if ((columns == 0) || (rows == 0))
    return((Image *) NULL);
  /* Do not short-circuit this resize if final image size is unchanged */

  (void) memset(distort_args,0,12*sizeof(double));
  distort_args[4]=(double) image->columns;
  distort_args[6]=(double) columns;
  distort_args[9]=(double) image->rows;
  distort_args[11]=(double) rows;

  vp_save=GetImageVirtualPixelMethod(image);

  tmp_image=CloneImage(image,0,0,MagickTrue,exception);
  if ( tmp_image == (Image *) NULL )
    return((Image *) NULL);
  (void) SetImageVirtualPixelMethod(tmp_image,TransparentVirtualPixelMethod,
    exception);

  if (image->alpha_trait == UndefinedPixelTrait)
    {
      /*
        Image has not transparency channel, so we free to use it
      */
      (void) SetImageAlphaChannel(tmp_image,SetAlphaChannel,exception);
      resize_image=DistortImage(tmp_image,AffineDistortion,12,distort_args,
        MagickTrue,exception),

      tmp_image=DestroyImage(tmp_image);
      if ( resize_image == (Image *) NULL )
        return((Image *) NULL);

      (void) SetImageAlphaChannel(resize_image,DeactivateAlphaChannel,
        exception);
    }
  else
    {
      /*
        Image has transparency so handle colors and alpha separatly.
        Basically we need to separate Virtual-Pixel alpha in the resized
        image, so only the actual original images alpha channel is used.

        distort alpha channel separately
      */
      Image
        *resize_alpha;

      (void) SetImageAlphaChannel(tmp_image,ExtractAlphaChannel,exception);
      (void) SetImageAlphaChannel(tmp_image,OpaqueAlphaChannel,exception);
      resize_alpha=DistortImage(tmp_image,AffineDistortion,12,distort_args,
        MagickTrue,exception),
      tmp_image=DestroyImage(tmp_image);
      if (resize_alpha == (Image *) NULL)
        return((Image *) NULL);

      /* distort the actual image containing alpha + VP alpha */
      tmp_image=CloneImage(image,0,0,MagickTrue,exception);
      if ( tmp_image == (Image *) NULL )
        return((Image *) NULL);
      (void) SetImageVirtualPixelMethod(tmp_image,TransparentVirtualPixelMethod,        exception);
      resize_image=DistortImage(tmp_image,AffineDistortion,12,distort_args,
        MagickTrue,exception),
      tmp_image=DestroyImage(tmp_image);
      if ( resize_image == (Image *) NULL)
        {
          resize_alpha=DestroyImage(resize_alpha);
          return((Image *) NULL);
        }
      /* replace resize images alpha with the separally distorted alpha */
      (void) SetImageAlphaChannel(resize_image,OffAlphaChannel,exception);
      (void) SetImageAlphaChannel(resize_alpha,OffAlphaChannel,exception);
      (void) CompositeImage(resize_image,resize_alpha,CopyAlphaCompositeOp,
        MagickTrue,0,0,exception);
      resize_alpha=DestroyImage(resize_alpha);
    }
  (void) SetImageVirtualPixelMethod(resize_image,vp_save,exception);

  /*
    Clean up the results of the Distortion
  */
  crop_area.width=columns;
  crop_area.height=rows;
  crop_area.x=0;
  crop_area.y=0;

  tmp_image=resize_image;
  resize_image=CropImage(tmp_image,&crop_area,exception);
  tmp_image=DestroyImage(tmp_image);
  if (resize_image != (Image *) NULL)
    {
      resize_image->alpha_trait=image->alpha_trait;
      resize_image->compose=image->compose;
      resize_image->page.width=0;
      resize_image->page.height=0;
    }
  return(resize_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D i s t o r t I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DistortImage() distorts an image using various distortion methods, by
%  mapping color lookups of the source image to a new destination image
%  usally of the same size as the source image, unless 'bestfit' is set to
%  true.
%
%  If 'bestfit' is enabled, and distortion allows it, the destination image is
%  adjusted to ensure the whole source 'image' will just fit within the final
%  destination image, which will be sized and offset accordingly.  Also in
%  many cases the virtual offset of the source image will be taken into
%  account in the mapping.
%
%  If the '-verbose' control option has been set print to standard error the
%  equicelent '-fx' formula with coefficients for the function, if practical.
%
%  The format of the DistortImage() method is:
%
%      Image *DistortImage(const Image *image,const DistortMethod method,
%        const size_t number_arguments,const double *arguments,
%        MagickBooleanType bestfit, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to be distorted.
%
%    o method: the method of image distortion.
%
%        ArcDistortion always ignores source image offset, and always
%        'bestfit' the destination image with the top left corner offset
%        relative to the polar mapping center.
%
%        Affine, Perspective, and Bilinear, do least squares fitting of the
%        distrotion when more than the minimum number of control point pairs
%        are provided.
%
%        Perspective, and Bilinear, fall back to a Affine distortion when less
%        than 4 control point pairs are provided.  While Affine distortions
%        let you use any number of control point pairs, that is Zero pairs is
%        a No-Op (viewport only) distortion, one pair is a translation and
%        two pairs of control points do a scale-rotate-translate, without any
%        shearing.
%
%    o number_arguments: the number of arguments given.
%
%    o arguments: an array of floating point arguments for this method.
%
%    o bestfit: Attempt to 'bestfit' the size of the resulting image.
%        This also forces the resulting image to be a 'layered' virtual
%        canvas image.  Can be overridden using 'distort:viewport' setting.
%
%    o exception: return any errors or warnings in this structure
%
%  Extra Controls from Image meta-data (artifacts)...
%
%    o "verbose"
%        Output to stderr alternatives, internal coefficents, and FX
%        equivalents for the distortion operation (if feasible).
%        This forms an extra check of the distortion method, and allows users
%        access to the internal constants IM calculates for the distortion.
%
%    o "distort:viewport"
%        Directly set the output image canvas area and offest to use for the
%        resulting image, rather than use the original images canvas, or a
%        calculated 'bestfit' canvas.
%
%    o "distort:scale"
%        Scale the size of the output canvas by this amount to provide a
%        method of Zooming, and for super-sampling the results.
%
%  Other settings that can effect results include
%
%    o 'interpolate' For source image lookups (scale enlargements)
%
%    o 'filter'      Set filter to use for area-resampling (scale shrinking).
%                    Set to 'point' to turn off and use 'interpolate' lookup
%                    instead
%
*/
MagickExport Image *DistortImage(const Image *image, DistortMethod method,
  const size_t number_arguments,const double *arguments,
  MagickBooleanType bestfit,ExceptionInfo *exception)
{
#define DistortImageTag  "Distort/Image"

  double
    *coeff,
    output_scaling;

  Image
    *distort_image;

  RectangleInfo
    geometry;  /* geometry of the distorted space viewport */

  MagickBooleanType
    viewport_given;

  PixelInfo
    invalid;  /* the color to assign when distort result is invalid */

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);

  /*
    Handle Special Compound Distortions
  */
  if ( method == ResizeDistortion )
    {
      if ( number_arguments != 2 )
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                    "InvalidArgument","%s : '%s'","Resize",
                    "Invalid number of args: 2 only");
          return((Image *) NULL);
        }
      distort_image=DistortResizeImage(image,(size_t)arguments[0],
         (size_t)arguments[1], exception);
      return(distort_image);
    }

  /*
    Convert input arguments (usually as control points for reverse mapping)
    into mapping coefficients to apply the distortion.

    Note that some distortions are mapped to other distortions,
    and as such do not require specific code after this point.
  */
  coeff = GenerateCoefficients(image, &method, number_arguments,
      arguments, 0, exception);
  if ( coeff == (double *) NULL )
    return((Image *) NULL);

  /*
    Determine the size and offset for a 'bestfit' destination.
    Usally the four corners of the source image is enough.
  */

  /* default output image bounds, when no 'bestfit' is requested */
  geometry.width=image->columns;
  geometry.height=image->rows;
  geometry.x=0;
  geometry.y=0;

  if ( method == ArcDistortion ) {
    bestfit = MagickTrue;  /* always calculate a 'best fit' viewport */
  }

  /* Work out the 'best fit', (required for ArcDistortion) */
  if ( bestfit ) {
    PointInfo
      s,d,min,max;  /* source, dest coords --mapping--> min, max coords */

    MagickBooleanType
      fix_bounds = MagickTrue;   /* enlarge bounds for VP handling */

    s.x=s.y=min.x=max.x=min.y=max.y=0.0;   /* keep compiler happy */

/* defines to figure out the bounds of the distorted image */
#define InitalBounds(p) \
{ \
  /* printf("%lg,%lg -> %lg,%lg\n", s.x,s.y, d.x,d.y); */ \
  min.x = max.x = p.x; \
  min.y = max.y = p.y; \
}
#define ExpandBounds(p) \
{ \
  /* printf("%lg,%lg -> %lg,%lg\n", s.x,s.y, d.x,d.y); */ \
  min.x = MagickMin(min.x,p.x); \
  max.x = MagickMax(max.x,p.x); \
  min.y = MagickMin(min.y,p.y); \
  max.y = MagickMax(max.y,p.y); \
}

    switch (method)
    {
      case AffineDistortion:
      { double inverse[6];
        InvertAffineCoefficients(coeff, inverse);
        s.x = (double) image->page.x;
        s.y = (double) image->page.y;
        d.x = inverse[0]*s.x+inverse[1]*s.y+inverse[2];
        d.y = inverse[3]*s.x+inverse[4]*s.y+inverse[5];
        InitalBounds(d);
        s.x = (double) image->page.x+image->columns;
        s.y = (double) image->page.y;
        d.x = inverse[0]*s.x+inverse[1]*s.y+inverse[2];
        d.y = inverse[3]*s.x+inverse[4]*s.y+inverse[5];
        ExpandBounds(d);
        s.x = (double) image->page.x;
        s.y = (double) image->page.y+image->rows;
        d.x = inverse[0]*s.x+inverse[1]*s.y+inverse[2];
        d.y = inverse[3]*s.x+inverse[4]*s.y+inverse[5];
        ExpandBounds(d);
        s.x = (double) image->page.x+image->columns;
        s.y = (double) image->page.y+image->rows;
        d.x = inverse[0]*s.x+inverse[1]*s.y+inverse[2];
        d.y = inverse[3]*s.x+inverse[4]*s.y+inverse[5];
        ExpandBounds(d);
        break;
      }
      case PerspectiveDistortion:
      { double inverse[8], scale;
        InvertPerspectiveCoefficients(coeff, inverse);
        s.x = (double) image->page.x;
        s.y = (double) image->page.y;
        scale=inverse[6]*s.x+inverse[7]*s.y+1.0;
        scale=PerceptibleReciprocal(scale);
        d.x = scale*(inverse[0]*s.x+inverse[1]*s.y+inverse[2]);
        d.y = scale*(inverse[3]*s.x+inverse[4]*s.y+inverse[5]);
        InitalBounds(d);
        s.x = (double) image->page.x+image->columns;
        s.y = (double) image->page.y;
        scale=inverse[6]*s.x+inverse[7]*s.y+1.0;
        scale=PerceptibleReciprocal(scale);
        d.x = scale*(inverse[0]*s.x+inverse[1]*s.y+inverse[2]);
        d.y = scale*(inverse[3]*s.x+inverse[4]*s.y+inverse[5]);
        ExpandBounds(d);
        s.x = (double) image->page.x;
        s.y = (double) image->page.y+image->rows;
        scale=inverse[6]*s.x+inverse[7]*s.y+1.0;
        scale=PerceptibleReciprocal(scale);
        d.x = scale*(inverse[0]*s.x+inverse[1]*s.y+inverse[2]);
        d.y = scale*(inverse[3]*s.x+inverse[4]*s.y+inverse[5]);
        ExpandBounds(d);
        s.x = (double) image->page.x+image->columns;
        s.y = (double) image->page.y+image->rows;
        scale=inverse[6]*s.x+inverse[7]*s.y+1.0;
        scale=PerceptibleReciprocal(scale);
        d.x = scale*(inverse[0]*s.x+inverse[1]*s.y+inverse[2]);
        d.y = scale*(inverse[3]*s.x+inverse[4]*s.y+inverse[5]);
        ExpandBounds(d);
        break;
      }
      case ArcDistortion:
      { double a, ca, sa;
        /* Forward Map Corners */
        a = coeff[0]-coeff[1]/2; ca = cos(a); sa = sin(a);
        d.x = coeff[2]*ca;
        d.y = coeff[2]*sa;
        InitalBounds(d);
        d.x = (coeff[2]-coeff[3])*ca;
        d.y = (coeff[2]-coeff[3])*sa;
        ExpandBounds(d);
        a = coeff[0]+coeff[1]/2; ca = cos(a); sa = sin(a);
        d.x = coeff[2]*ca;
        d.y = coeff[2]*sa;
        ExpandBounds(d);
        d.x = (coeff[2]-coeff[3])*ca;
        d.y = (coeff[2]-coeff[3])*sa;
        ExpandBounds(d);
        /* Orthogonal points along top of arc */
        for( a=(double) (ceil((double) ((coeff[0]-coeff[1]/2.0)/MagickPI2))*MagickPI2);
               a<(coeff[0]+coeff[1]/2.0); a+=MagickPI2 ) {
          ca = cos(a); sa = sin(a);
          d.x = coeff[2]*ca;
          d.y = coeff[2]*sa;
          ExpandBounds(d);
        }
        /*
          Convert the angle_to_width and radius_to_height
          to appropriate scaling factors, to allow faster processing
          in the mapping function.
        */
        coeff[1] = (double) (Magick2PI*image->columns/coeff[1]);
        coeff[3] = (double)image->rows/coeff[3];
        break;
      }
      case PolarDistortion:
      {
        if (number_arguments < 2)
          coeff[2] = coeff[3] = 0.0;
        min.x = coeff[2]-coeff[0];
        max.x = coeff[2]+coeff[0];
        min.y = coeff[3]-coeff[0];
        max.y = coeff[3]+coeff[0];
        /* should be about 1.0 if Rmin = 0 */
        coeff[7]=(double) geometry.height/(coeff[0]-coeff[1]);
        break;
      }
      case DePolarDistortion:
      {
        /* direct calculation as it needs to tile correctly
         * for reversibility in a DePolar-Polar cycle */
        fix_bounds = MagickFalse;
        geometry.x = geometry.y = 0;
        geometry.height = (size_t) ceil(coeff[0]-coeff[1]);
        geometry.width = (size_t)
                  ceil((coeff[0]-coeff[1])*(coeff[5]-coeff[4])*0.5);
        /* correct scaling factors relative to new size */
        coeff[6]=(coeff[5]-coeff[4])/geometry.width; /* changed width */
        coeff[7]=(coeff[0]-coeff[1])/geometry.height; /* should be about 1.0 */
        break;
      }
      case Cylinder2PlaneDistortion:
      {
        /* direct calculation so center of distortion is either a pixel
         * center, or pixel edge. This allows for reversibility of the
         * distortion */
        geometry.x = geometry.y = 0;
        geometry.width = (size_t) ceil( 2.0*coeff[1]*tan(coeff[0]/2.0) );
        geometry.height = (size_t) ceil( 2.0*coeff[3]/cos(coeff[0]/2.0) );
        /* correct center of distortion relative to new size */
        coeff[4] = (double) geometry.width/2.0;
        coeff[5] = (double) geometry.height/2.0;
        fix_bounds = MagickFalse;
        break;
      }
      case Plane2CylinderDistortion:
      {
        /* direct calculation center is either pixel center, or pixel edge
         * so as to allow reversibility of the image distortion */
        geometry.x = geometry.y = 0;
        geometry.width = (size_t) ceil(coeff[0]*coeff[1]);  /* FOV * radius */
        geometry.height = (size_t) (2*coeff[3]);              /* input image height */
        /* correct center of distortion relative to new size */
        coeff[4] = (double) geometry.width/2.0;
        coeff[5] = (double) geometry.height/2.0;
        fix_bounds = MagickFalse;
        break;
      }
      case ShepardsDistortion:
      case BilinearForwardDistortion:
      case BilinearReverseDistortion:
#if 0
      case QuadrilateralDistortion:
#endif
      case PolynomialDistortion:
      case BarrelDistortion:
      case BarrelInverseDistortion:
      default:
        /* no calculated bestfit available for these distortions */
        bestfit = MagickFalse;
        fix_bounds = MagickFalse;
        break;
    }

    /* Set the output image geometry to calculated 'bestfit'.
       Yes this tends to 'over do' the file image size, ON PURPOSE!
       Do not do this for DePolar which needs to be exact for virtual tiling.
    */
    if ( fix_bounds ) {
      geometry.x = (ssize_t) floor(min.x-0.5);
      geometry.y = (ssize_t) floor(min.y-0.5);
      geometry.width=(size_t) ceil(max.x-geometry.x+0.5);
      geometry.height=(size_t) ceil(max.y-geometry.y+0.5);
    }

  }  /* end bestfit destination image calculations */

  /* The user provided a 'viewport' expert option which may
     overrides some parts of the current output image geometry.
     This also overrides its default 'bestfit' setting.
  */
  { const char *artifact=GetImageArtifact(image,"distort:viewport");
    viewport_given = MagickFalse;
    if ( artifact != (const char *) NULL ) {
      MagickStatusType flags=ParseAbsoluteGeometry(artifact,&geometry);
      if (flags==NoValue)
        (void) ThrowMagickException(exception,GetMagickModule(),
             OptionWarning,"InvalidSetting","'%s' '%s'",
             "distort:viewport",artifact);
      else
        viewport_given = MagickTrue;
    }
  }

  /* Verbose output */
  if (IsStringTrue(GetImageArtifact(image,"verbose")) != MagickFalse) {
    register ssize_t
       i;
    char image_gen[MagickPathExtent];
    const char *lookup;

    /* Set destination image size and virtual offset */
    if ( bestfit || viewport_given ) {
      (void) FormatLocaleString(image_gen, MagickPathExtent,"  -size %.20gx%.20g "
        "-page %+.20g%+.20g xc: +insert \\\n",(double) geometry.width,
        (double) geometry.height,(double) geometry.x,(double) geometry.y);
      lookup="v.p{ xx-v.page.x-.5, yy-v.page.y-.5 }";
    }
    else {
      image_gen[0] = '\0';             /* no destination to generate */
      lookup = "p{ xx-page.x-.5, yy-page.y-.5 }"; /* simplify lookup */
    }

    switch (method) {
      case AffineDistortion:
      {
        double *inverse;

        inverse = (double *) AcquireQuantumMemory(6,sizeof(*inverse));
        if (inverse == (double *) NULL) {
          coeff = (double *) RelinquishMagickMemory(coeff);
          (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortImages");
          return((Image *) NULL);
        }
        InvertAffineCoefficients(coeff, inverse);
        CoefficientsToAffineArgs(inverse);
        (void) FormatLocaleFile(stderr, "Affine Projection:\n");
        (void) FormatLocaleFile(stderr, "  -distort AffineProjection \\\n      '");
        for (i=0; i < 5; i++)
          (void) FormatLocaleFile(stderr, "%lf,", inverse[i]);
        (void) FormatLocaleFile(stderr, "%lf'\n", inverse[5]);
        inverse = (double *) RelinquishMagickMemory(inverse);

        (void) FormatLocaleFile(stderr, "Affine Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        (void) FormatLocaleFile(stderr, "       xx=%+lf*ii %+lf*jj %+lf;\n",
            coeff[0], coeff[1], coeff[2]);
        (void) FormatLocaleFile(stderr, "       yy=%+lf*ii %+lf*jj %+lf;\n",
            coeff[3], coeff[4], coeff[5]);
        (void) FormatLocaleFile(stderr, "       %s' \\\n", lookup);

        break;
      }

      case PerspectiveDistortion:
      {
        double *inverse;

        inverse = (double *) AcquireQuantumMemory(8,sizeof(*inverse));
        if (inverse == (double *) NULL) {
          coeff = (double *) RelinquishMagickMemory(coeff);
          (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
          return((Image *) NULL);
        }
        InvertPerspectiveCoefficients(coeff, inverse);
        (void) FormatLocaleFile(stderr, "Perspective Projection:\n");
        (void) FormatLocaleFile(stderr, "  -distort PerspectiveProjection \\\n      '");
        for (i=0; i<4; i++)
          (void) FormatLocaleFile(stderr, "%lf, ", inverse[i]);
        (void) FormatLocaleFile(stderr, "\n       ");
        for (; i<7; i++)
          (void) FormatLocaleFile(stderr, "%lf, ", inverse[i]);
        (void) FormatLocaleFile(stderr, "%lf'\n", inverse[7]);
        inverse = (double *) RelinquishMagickMemory(inverse);

        (void) FormatLocaleFile(stderr, "Perspective Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        (void) FormatLocaleFile(stderr, "       rr=%+lf*ii %+lf*jj + 1;\n",
            coeff[6], coeff[7]);
        (void) FormatLocaleFile(stderr, "       xx=(%+lf*ii %+lf*jj %+lf)/rr;\n",
            coeff[0], coeff[1], coeff[2]);
        (void) FormatLocaleFile(stderr, "       yy=(%+lf*ii %+lf*jj %+lf)/rr;\n",
            coeff[3], coeff[4], coeff[5]);
        (void) FormatLocaleFile(stderr, "       rr%s0 ? %s : blue' \\\n",
            coeff[8] < 0 ? "<" : ">", lookup);
        break;
      }

      case BilinearForwardDistortion:
        (void) FormatLocaleFile(stderr, "BilinearForward Mapping Equations:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "    i = %+lf*x %+lf*y %+lf*x*y %+lf;\n",
            coeff[0], coeff[1], coeff[2], coeff[3]);
        (void) FormatLocaleFile(stderr, "    j = %+lf*x %+lf*y %+lf*x*y %+lf;\n",
            coeff[4], coeff[5], coeff[6], coeff[7]);
#if 0
        /* for debugging */
        (void) FormatLocaleFile(stderr, "   c8 = %+lf  c9 = 2*a = %+lf;\n",
            coeff[8], coeff[9]);
#endif
        (void) FormatLocaleFile(stderr, "BilinearForward Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x%+lf; jj=j+page.y%+lf;\n",
            0.5-coeff[3], 0.5-coeff[7]);
        (void) FormatLocaleFile(stderr, "       bb=%lf*ii %+lf*jj %+lf;\n",
            coeff[6], -coeff[2], coeff[8]);
        /* Handle Special degenerate (non-quadratic) or trapezoidal case */
        if ( coeff[9] != 0 ) {
          (void) FormatLocaleFile(stderr, "       rt=bb*bb %+lf*(%lf*ii%+lf*jj);\n",
              -2*coeff[9],  coeff[4], -coeff[0]);
          (void) FormatLocaleFile(stderr, "       yy=( -bb + sqrt(rt) ) / %lf;\n",
               coeff[9]);
        } else
          (void) FormatLocaleFile(stderr, "       yy=(%lf*ii%+lf*jj)/bb;\n",
                -coeff[4], coeff[0]);
        (void) FormatLocaleFile(stderr, "       xx=(ii %+lf*yy)/(%lf %+lf*yy);\n",
             -coeff[1], coeff[0], coeff[2]);
        if ( coeff[9] != 0 )
          (void) FormatLocaleFile(stderr, "       (rt < 0 ) ? red : %s'\n", lookup);
        else
          (void) FormatLocaleFile(stderr, "       %s' \\\n", lookup);
        break;

      case BilinearReverseDistortion:
#if 0
        (void) FormatLocaleFile(stderr, "Polynomial Projection Distort:\n");
        (void) FormatLocaleFile(stderr, "  -distort PolynomialProjection \\\n");
        (void) FormatLocaleFile(stderr, "      '1.5, %lf, %lf, %lf, %lf,\n",
            coeff[3], coeff[0], coeff[1], coeff[2]);
        (void) FormatLocaleFile(stderr, "            %lf, %lf, %lf, %lf'\n",
            coeff[7], coeff[4], coeff[5], coeff[6]);
#endif
        (void) FormatLocaleFile(stderr, "BilinearReverse Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        (void) FormatLocaleFile(stderr, "       xx=%+lf*ii %+lf*jj %+lf*ii*jj %+lf;\n",
            coeff[0], coeff[1], coeff[2], coeff[3]);
        (void) FormatLocaleFile(stderr, "       yy=%+lf*ii %+lf*jj %+lf*ii*jj %+lf;\n",
            coeff[4], coeff[5], coeff[6], coeff[7]);
        (void) FormatLocaleFile(stderr, "       %s' \\\n", lookup);
        break;

      case PolynomialDistortion:
      {
        size_t nterms = (size_t) coeff[1];
        (void) FormatLocaleFile(stderr, "Polynomial (order %lg, terms %lu), FX Equivelent\n",
          coeff[0],(unsigned long) nterms);
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        (void) FormatLocaleFile(stderr, "       xx =");
        for (i=0; i<(ssize_t) nterms; i++) {
          if ( i != 0 && i%4 == 0 ) (void) FormatLocaleFile(stderr, "\n         ");
          (void) FormatLocaleFile(stderr, " %+lf%s", coeff[2+i],
               poly_basis_str(i));
        }
        (void) FormatLocaleFile(stderr, ";\n       yy =");
        for (i=0; i<(ssize_t) nterms; i++) {
          if ( i != 0 && i%4 == 0 ) (void) FormatLocaleFile(stderr, "\n         ");
          (void) FormatLocaleFile(stderr, " %+lf%s", coeff[2+i+nterms],
               poly_basis_str(i));
        }
        (void) FormatLocaleFile(stderr, ";\n       %s' \\\n", lookup);
        break;
      }
      case ArcDistortion:
      {
        (void) FormatLocaleFile(stderr, "Arc Distort, Internal Coefficients:\n");
        for ( i=0; i<5; i++ )
          (void) FormatLocaleFile(stderr, "  c%.20g = %+lf\n", (double) i, coeff[i]);
        (void) FormatLocaleFile(stderr, "Arc Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x; jj=j+page.y;\n");
        (void) FormatLocaleFile(stderr, "       xx=(atan2(jj,ii)%+lf)/(2*pi);\n",
                                  -coeff[0]);
        (void) FormatLocaleFile(stderr, "       xx=xx-round(xx);\n");
        (void) FormatLocaleFile(stderr, "       xx=xx*%lf %+lf;\n",
                            coeff[1], coeff[4]);
        (void) FormatLocaleFile(stderr, "       yy=(%lf - hypot(ii,jj)) * %lf;\n",
                            coeff[2], coeff[3]);
        (void) FormatLocaleFile(stderr, "       v.p{xx-.5,yy-.5}' \\\n");
        break;
      }
      case PolarDistortion:
      {
        (void) FormatLocaleFile(stderr, "Polar Distort, Internal Coefficents\n");
        for ( i=0; i<8; i++ )
          (void) FormatLocaleFile(stderr, "  c%.20g = %+lf\n", (double) i, coeff[i]);
        (void) FormatLocaleFile(stderr, "Polar Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x%+lf; jj=j+page.y%+lf;\n",
                         -coeff[2], -coeff[3]);
        (void) FormatLocaleFile(stderr, "       xx=(atan2(ii,jj)%+lf)/(2*pi);\n",
                         -(coeff[4]+coeff[5])/2 );
        (void) FormatLocaleFile(stderr, "       xx=xx-round(xx);\n");
        (void) FormatLocaleFile(stderr, "       xx=xx*2*pi*%lf + v.w/2;\n",
                         coeff[6] );
        (void) FormatLocaleFile(stderr, "       yy=(hypot(ii,jj)%+lf)*%lf;\n",
                         -coeff[1], coeff[7] );
        (void) FormatLocaleFile(stderr, "       v.p{xx-.5,yy-.5}' \\\n");
        break;
      }
      case DePolarDistortion:
      {
        (void) FormatLocaleFile(stderr, "DePolar Distort, Internal Coefficents\n");
        for ( i=0; i<8; i++ )
          (void) FormatLocaleFile(stderr, "  c%.20g = %+lf\n", (double) i, coeff[i]);
        (void) FormatLocaleFile(stderr, "DePolar Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'aa=(i+.5)*%lf %+lf;\n", coeff[6], +coeff[4] );
        (void) FormatLocaleFile(stderr, "       rr=(j+.5)*%lf %+lf;\n", coeff[7], +coeff[1] );
        (void) FormatLocaleFile(stderr, "       xx=rr*sin(aa) %+lf;\n", coeff[2] );
        (void) FormatLocaleFile(stderr, "       yy=rr*cos(aa) %+lf;\n", coeff[3] );
        (void) FormatLocaleFile(stderr, "       v.p{xx-.5,yy-.5}' \\\n");
        break;
      }
      case Cylinder2PlaneDistortion:
      {
        (void) FormatLocaleFile(stderr, "Cylinder to Plane Distort, Internal Coefficents\n");
        (void) FormatLocaleFile(stderr, "  cylinder_radius = %+lf\n", coeff[1]);
        (void) FormatLocaleFile(stderr, "Cylinder to Plane Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x%+lf+0.5; jj=j+page.y%+lf+0.5;\n",
                         -coeff[4], -coeff[5]);
        (void) FormatLocaleFile(stderr, "       aa=atan(ii/%+lf);\n", coeff[1] );
        (void) FormatLocaleFile(stderr, "       xx=%lf*aa%+lf;\n",
                         coeff[1], coeff[2] );
        (void) FormatLocaleFile(stderr, "       yy=jj*cos(aa)%+lf;\n", coeff[3] );
        (void) FormatLocaleFile(stderr, "       %s' \\\n", lookup);
        break;
      }
      case Plane2CylinderDistortion:
      {
        (void) FormatLocaleFile(stderr, "Plane to Cylinder Distort, Internal Coefficents\n");
        (void) FormatLocaleFile(stderr, "  cylinder_radius = %+lf\n", coeff[1]);
        (void) FormatLocaleFile(stderr, "Plane to Cylinder Distort, FX Equivelent:\n");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        (void) FormatLocaleFile(stderr, "  -fx 'ii=i+page.x%+lf+0.5; jj=j+page.y%+lf+0.5;\n",
                         -coeff[4], -coeff[5]);
        (void) FormatLocaleFile(stderr, "       ii=ii/%+lf;\n", coeff[1] );
        (void) FormatLocaleFile(stderr, "       xx=%lf*tan(ii)%+lf;\n",
                         coeff[1], coeff[2] );
        (void) FormatLocaleFile(stderr, "       yy=jj/cos(ii)%+lf;\n",
                         coeff[3] );
        (void) FormatLocaleFile(stderr, "       %s' \\\n", lookup);
        break;
      }
      case BarrelDistortion:
      case BarrelInverseDistortion:
      { double xc,yc;
        /* NOTE: This does the barrel roll in pixel coords not image coords
        ** The internal distortion must do it in image coordinates,
        ** so that is what the center coeff (8,9) is given in.
        */
        xc = ((double)image->columns-1.0)/2.0 + image->page.x;
        yc = ((double)image->rows-1.0)/2.0    + image->page.y;
        (void) FormatLocaleFile(stderr, "Barrel%s Distort, FX Equivelent:\n",
             method == BarrelDistortion ? "" : "Inv");
        (void) FormatLocaleFile(stderr, "%s", image_gen);
        if ( fabs(coeff[8]-xc-0.5) < 0.1 && fabs(coeff[9]-yc-0.5) < 0.1 )
          (void) FormatLocaleFile(stderr, "  -fx 'xc=(w-1)/2;  yc=(h-1)/2;\n");
        else
          (void) FormatLocaleFile(stderr, "  -fx 'xc=%lf;  yc=%lf;\n",
               coeff[8]-0.5, coeff[9]-0.5);
        (void) FormatLocaleFile(stderr,
             "       ii=i-xc;  jj=j-yc;  rr=hypot(ii,jj);\n");
        (void) FormatLocaleFile(stderr, "       ii=ii%s(%lf*rr*rr*rr %+lf*rr*rr %+lf*rr %+lf);\n",
             method == BarrelDistortion ? "*" : "/",
             coeff[0],coeff[1],coeff[2],coeff[3]);
        (void) FormatLocaleFile(stderr, "       jj=jj%s(%lf*rr*rr*rr %+lf*rr*rr %+lf*rr %+lf);\n",
             method == BarrelDistortion ? "*" : "/",
             coeff[4],coeff[5],coeff[6],coeff[7]);
        (void) FormatLocaleFile(stderr, "       v.p{fx*ii+xc,fy*jj+yc}' \\\n");
      }
      default:
        break;
    }
  }

  /* The user provided a 'scale' expert option will scale the
     output image size, by the factor given allowing for super-sampling
     of the distorted image space.  Any scaling factors must naturally
     be halved as a result.
  */
  { const char *artifact;
    artifact=GetImageArtifact(image,"distort:scale");
    output_scaling = 1.0;
    if (artifact != (const char *) NULL) {
      output_scaling = fabs(StringToDouble(artifact,(char **) NULL));
      geometry.width=(size_t) (output_scaling*geometry.width+0.5);
      geometry.height=(size_t) (output_scaling*geometry.height+0.5);
      geometry.x=(ssize_t) (output_scaling*geometry.x+0.5);
      geometry.y=(ssize_t) (output_scaling*geometry.y+0.5);
      if ( output_scaling < 0.1 ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                "InvalidArgument","%s", "-set option:distort:scale" );
        return((Image *) NULL);
      }
      output_scaling = 1/output_scaling;
    }
  }
#define ScaleFilter(F,A,B,C,D) \
    ScaleResampleFilter( (F), \
      output_scaling*(A), output_scaling*(B), \
      output_scaling*(C), output_scaling*(D) )

  /*
    Initialize the distort image attributes.
  */
  distort_image=CloneImage(image,geometry.width,geometry.height,MagickTrue,
    exception);
  if (distort_image == (Image *) NULL)
    {
      coeff=(double *) RelinquishMagickMemory(coeff);
      return((Image *) NULL);
    }
  /* if image is ColorMapped - change it to DirectClass */
  if (SetImageStorageClass(distort_image,DirectClass,exception) == MagickFalse)
    {
      coeff=(double *) RelinquishMagickMemory(coeff);
      distort_image=DestroyImage(distort_image);
      return((Image *) NULL);
    }
  if ((IsPixelInfoGray(&distort_image->background_color) == MagickFalse) &&
      (IsGrayColorspace(distort_image->colorspace) != MagickFalse))
    (void) SetImageColorspace(distort_image,sRGBColorspace,exception);
  if (distort_image->background_color.alpha_trait != UndefinedPixelTrait)
    distort_image->alpha_trait=BlendPixelTrait;
  distort_image->page.x=geometry.x;
  distort_image->page.y=geometry.y;
  ConformPixelInfo(distort_image,&distort_image->matte_color,&invalid,
    exception);

  { /* ----- MAIN CODE -----
       Sample the source image to each pixel in the distort image.
     */
    CacheView
      *distort_view;

    MagickBooleanType
      status;

    MagickOffsetType
      progress;

    PixelInfo
      zero;

    ResampleFilter
      **magick_restrict resample_filter;

    ssize_t
      j;

    status=MagickTrue;
    progress=0;
    GetPixelInfo(distort_image,&zero);
    resample_filter=AcquireResampleFilterThreadSet(image,
      UndefinedVirtualPixelMethod,MagickFalse,exception);
    distort_view=AcquireAuthenticCacheView(distort_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp parallel for schedule(static) shared(progress,status) \
      magick_number_threads(image,distort_image,distort_image->rows,1)
#endif
    for (j=0; j < (ssize_t) distort_image->rows; j++)
    {
      const int
        id = GetOpenMPThreadId();

      double
        validity;  /* how mathematically valid is this the mapping */

      MagickBooleanType
        sync;

      PixelInfo
        pixel;    /* pixel color to assign to distorted image */

      PointInfo
        d,
        s;  /* transform destination image x,y  to source image x,y */

      register ssize_t
        i;

      register Quantum
        *magick_restrict q;

      q=QueueCacheViewAuthenticPixels(distort_view,0,j,distort_image->columns,1,
        exception);
      if (q == (Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      pixel=zero;

      /* Define constant scaling vectors for Affine Distortions
        Other methods are either variable, or use interpolated lookup
      */
      switch (method)
      {
        case AffineDistortion:
          ScaleFilter( resample_filter[id],
            coeff[0], coeff[1],
            coeff[3], coeff[4] );
          break;
        default:
          break;
      }

      /* Initialize default pixel validity
      *    negative:         pixel is invalid  output 'matte_color'
      *    0.0 to 1.0:       antialiased, mix with resample output
      *    1.0 or greater:   use resampled output.
      */
      validity = 1.0;

      for (i=0; i < (ssize_t) distort_image->columns; i++)
      {
        /* map pixel coordinate to distortion space coordinate */
        d.x = (double) (geometry.x+i+0.5)*output_scaling;
        d.y = (double) (geometry.y+j+0.5)*output_scaling;
        s = d;  /* default is a no-op mapping */
        switch (method)
        {
          case AffineDistortion:
          {
            s.x=coeff[0]*d.x+coeff[1]*d.y+coeff[2];
            s.y=coeff[3]*d.x+coeff[4]*d.y+coeff[5];
            /* Affine partial derivitives are constant -- set above */
            break;
          }
          case PerspectiveDistortion:
          {
            double
              p,q,r,abs_r,abs_c6,abs_c7,scale;
            /* perspective is a ratio of affines */
            p=coeff[0]*d.x+coeff[1]*d.y+coeff[2];
            q=coeff[3]*d.x+coeff[4]*d.y+coeff[5];
            r=coeff[6]*d.x+coeff[7]*d.y+1.0;
            /* Pixel Validity -- is it a 'sky' or 'ground' pixel */
            validity = (r*coeff[8] < 0.0) ? 0.0 : 1.0;
            /* Determine horizon anti-alias blending */
            abs_r = fabs(r)*2;
            abs_c6 = fabs(coeff[6]);
            abs_c7 = fabs(coeff[7]);
            if ( abs_c6 > abs_c7 ) {
              if ( abs_r < abs_c6*output_scaling )
                validity = 0.5 - coeff[8]*r/(coeff[6]*output_scaling);
            }
            else if ( abs_r < abs_c7*output_scaling )
              validity = 0.5 - coeff[8]*r/(coeff[7]*output_scaling);
            /* Perspective Sampling Point (if valid) */
            if ( validity > 0.0 ) {
              /* divide by r affine, for perspective scaling */
              scale = 1.0/r;
              s.x = p*scale;
              s.y = q*scale;
              /* Perspective Partial Derivatives or Scaling Vectors */
              scale *= scale;
              ScaleFilter( resample_filter[id],
                (r*coeff[0] - p*coeff[6])*scale,
                (r*coeff[1] - p*coeff[7])*scale,
                (r*coeff[3] - q*coeff[6])*scale,
                (r*coeff[4] - q*coeff[7])*scale );
            }
            break;
          }
          case BilinearReverseDistortion:
          {
            /* Reversed Mapped is just a simple polynomial */
            s.x=coeff[0]*d.x+coeff[1]*d.y+coeff[2]*d.x*d.y+coeff[3];
            s.y=coeff[4]*d.x+coeff[5]*d.y
                    +coeff[6]*d.x*d.y+coeff[7];
            /* Bilinear partial derivitives of scaling vectors */
            ScaleFilter( resample_filter[id],
                coeff[0] + coeff[2]*d.y,
                coeff[1] + coeff[2]*d.x,
                coeff[4] + coeff[6]*d.y,
                coeff[5] + coeff[6]*d.x );
            break;
          }
          case BilinearForwardDistortion:
          {
            /* Forward mapped needs reversed polynomial equations
             * which unfortunatally requires a square root!  */
            double b,c;
            d.x -= coeff[3];  d.y -= coeff[7];
            b = coeff[6]*d.x - coeff[2]*d.y + coeff[8];
            c = coeff[4]*d.x - coeff[0]*d.y;

            validity = 1.0;
            /* Handle Special degenerate (non-quadratic) case
             * Currently without horizon anti-alising */
            if ( fabs(coeff[9]) < MagickEpsilon )
              s.y =  -c/b;
            else {
              c = b*b - 2*coeff[9]*c;
              if ( c < 0.0 )
                validity = 0.0;
              else
                s.y = ( -b + sqrt(c) )/coeff[9];
            }
            if ( validity > 0.0 )
              s.x = ( d.x - coeff[1]*s.y) / ( coeff[0] + coeff[2]*s.y );

            /* NOTE: the sign of the square root should be -ve for parts
                     where the source image becomes 'flipped' or 'mirrored'.
               FUTURE: Horizon handling
               FUTURE: Scaling factors or Deritives (how?)
            */
            break;
          }
#if 0
          case BilinearDistortion:
            /* Bilinear mapping of any Quadrilateral to any Quadrilateral */
            /* UNDER DEVELOPMENT */
            break;
#endif
          case PolynomialDistortion:
          {
            /* multi-ordered polynomial */
            register ssize_t
              k;

            ssize_t
              nterms=(ssize_t)coeff[1];

            PointInfo
              du,dv; /* the du,dv vectors from unit dx,dy -- derivatives */

            s.x=s.y=du.x=du.y=dv.x=dv.y=0.0;
            for(k=0; k < nterms; k++) {
              s.x  += poly_basis_fn(k,d.x,d.y)*coeff[2+k];
              du.x += poly_basis_dx(k,d.x,d.y)*coeff[2+k];
              du.y += poly_basis_dy(k,d.x,d.y)*coeff[2+k];
              s.y  += poly_basis_fn(k,d.x,d.y)*coeff[2+k+nterms];
              dv.x += poly_basis_dx(k,d.x,d.y)*coeff[2+k+nterms];
              dv.y += poly_basis_dy(k,d.x,d.y)*coeff[2+k+nterms];
            }
            ScaleFilter( resample_filter[id], du.x,du.y,dv.x,dv.y );
            break;
          }
          case ArcDistortion:
          {
            /* what is the angle and radius in the destination image */
            s.x  = (double) ((atan2(d.y,d.x) - coeff[0])/Magick2PI);
            s.x -= MagickRound(s.x);     /* angle */
            s.y  = hypot(d.x,d.y);       /* radius */

            /* Arc Distortion Partial Scaling Vectors
              Are derived by mapping the perpendicular unit vectors
              dR  and  dA*R*2PI  rather than trying to map dx and dy
              The results is a very simple orthogonal aligned ellipse.
            */
            if ( s.y > MagickEpsilon )
              ScaleFilter( resample_filter[id],
                  (double) (coeff[1]/(Magick2PI*s.y)), 0, 0, coeff[3] );
            else
              ScaleFilter( resample_filter[id],
                  distort_image->columns*2, 0, 0, coeff[3] );

            /* now scale the angle and radius for source image lookup point */
            s.x = s.x*coeff[1] + coeff[4] + image->page.x +0.5;
            s.y = (coeff[2] - s.y) * coeff[3] + image->page.y;
            break;
          }
          case PolarDistortion:
          { /* 2D Cartesain to Polar View */
            d.x -= coeff[2];
            d.y -= coeff[3];
            s.x  = atan2(d.x,d.y) - (coeff[4]+coeff[5])/2;
            s.x /= Magick2PI;
            s.x -= MagickRound(s.x);
            s.x *= Magick2PI;       /* angle - relative to centerline */
            s.y  = hypot(d.x,d.y);  /* radius */

            /* Polar Scaling vectors are based on mapping dR and dA vectors
               This results in very simple orthogonal scaling vectors
            */
            if ( s.y > MagickEpsilon )
              ScaleFilter( resample_filter[id],
                (double) (coeff[6]/(Magick2PI*s.y)), 0, 0, coeff[7] );
            else
              ScaleFilter( resample_filter[id],
                  distort_image->columns*2, 0, 0, coeff[7] );

            /* now finish mapping radius/angle to source x,y coords */
            s.x = s.x*coeff[6] + (double)image->columns/2.0 + image->page.x;
            s.y = (s.y-coeff[1])*coeff[7] + image->page.y;
            break;
          }
          case DePolarDistortion:
          { /* @D Polar to Carteasain  */
            /* ignore all destination virtual offsets */
            d.x = ((double)i+0.5)*output_scaling*coeff[6]+coeff[4];
            d.y = ((double)j+0.5)*output_scaling*coeff[7]+coeff[1];
            s.x = d.y*sin(d.x) + coeff[2];
            s.y = d.y*cos(d.x) + coeff[3];
            /* derivatives are usless - better to use SuperSampling */
            break;
          }
          case Cylinder2PlaneDistortion:
          { /* 3D Cylinder to Tangential Plane */
            double ax, cx;
            /* relative to center of distortion */
            d.x -= coeff[4]; d.y -= coeff[5];
            d.x /= coeff[1];        /* x' = x/r */
            ax=atan(d.x);           /* aa = atan(x/r) = u/r  */
            cx=cos(ax);             /* cx = cos(atan(x/r)) = 1/sqrt(x^2+u^2) */
            s.x = coeff[1]*ax;      /* u  = r*atan(x/r) */
            s.y = d.y*cx;           /* v  = y*cos(u/r) */
            /* derivatives... (see personnal notes) */
            ScaleFilter( resample_filter[id],
                  1.0/(1.0+d.x*d.x), 0.0, -d.x*s.y*cx*cx/coeff[1], s.y/d.y );
#if 0
if ( i == 0 && j == 0 ) {
  fprintf(stderr, "x=%lf  y=%lf  u=%lf  v=%lf\n", d.x*coeff[1], d.y, s.x, s.y);
  fprintf(stderr, "phi = %lf\n", (double)(ax * 180.0/MagickPI) );
  fprintf(stderr, "du/dx=%lf  du/dx=%lf  dv/dx=%lf  dv/dy=%lf\n",
                1.0/(1.0+d.x*d.x), 0.0, -d.x*s.y*cx*cx/coeff[1], s.y/d.y );
  fflush(stderr); }
#endif
            /* add center of distortion in source */
            s.x += coeff[2]; s.y += coeff[3];
            break;
          }
          case Plane2CylinderDistortion:
          { /* 3D Cylinder to Tangential Plane */
            /* relative to center of distortion */
            d.x -= coeff[4]; d.y -= coeff[5];

            /* is pixel valid - horizon of a infinite Virtual-Pixel Plane
             * (see Anthony Thyssen's personal note) */
            validity = (double) (coeff[1]*MagickPI2 - fabs(d.x))/output_scaling + 0.5;

            if ( validity > 0.0 ) {
              double cx,tx;
              d.x /= coeff[1];           /* x'= x/r */
              cx = 1/cos(d.x);           /* cx = 1/cos(x/r) */
              tx = tan(d.x);             /* tx = tan(x/r) */
              s.x = coeff[1]*tx;         /* u = r * tan(x/r) */
              s.y = d.y*cx;              /* v = y / cos(x/r) */
              /* derivatives...  (see Anthony Thyssen's personal notes) */
              ScaleFilter( resample_filter[id],
                    cx*cx, 0.0, s.y*cx/coeff[1], cx );
#if 0
/*if ( i == 0 && j == 0 )*/
if ( d.x == 0.5 && d.y == 0.5 ) {
  fprintf(stderr, "x=%lf  y=%lf  u=%lf  v=%lf\n", d.x*coeff[1], d.y, s.x, s.y);
  fprintf(stderr, "radius = %lf  phi = %lf  validity = %lf\n",
      coeff[1],  (double)(d.x * 180.0/MagickPI), validity );
  fprintf(stderr, "du/dx=%lf  du/dx=%lf  dv/dx=%lf  dv/dy=%lf\n",
      cx*cx, 0.0, s.y*cx/coeff[1], cx);
  fflush(stderr); }
#endif
            }
            /* add center of distortion in source */
            s.x += coeff[2]; s.y += coeff[3];
            break;
          }
          case BarrelDistortion:
          case BarrelInverseDistortion:
          { /* Lens Barrel Distionion Correction */
            double r,fx,fy,gx,gy;
            /* Radial Polynomial Distortion (de-normalized) */
            d.x -= coeff[8];
            d.y -= coeff[9];
            r = sqrt(d.x*d.x+d.y*d.y);
            if ( r > MagickEpsilon ) {
              fx = ((coeff[0]*r + coeff[1])*r + coeff[2])*r + coeff[3];
              fy = ((coeff[4]*r + coeff[5])*r + coeff[6])*r + coeff[7];
              gx = ((3*coeff[0]*r + 2*coeff[1])*r + coeff[2])/r;
              gy = ((3*coeff[4]*r + 2*coeff[5])*r + coeff[6])/r;
              /* adjust functions and scaling for 'inverse' form */
              if ( method == BarrelInverseDistortion ) {
                fx = 1/fx;  fy = 1/fy;
                gx *= -fx*fx;  gy *= -fy*fy;
              }
              /* Set the source pixel to lookup and EWA derivative vectors */
              s.x = d.x*fx + coeff[8];
              s.y = d.y*fy + coeff[9];
              ScaleFilter( resample_filter[id],
                  gx*d.x*d.x + fx, gx*d.x*d.y,
                  gy*d.x*d.y,      gy*d.y*d.y + fy );
            }
            else {
              /* Special handling to avoid divide by zero when r==0
              **
              ** The source and destination pixels match in this case
              ** which was set at the top of the loop using  s = d;
              ** otherwise...   s.x=coeff[8]; s.y=coeff[9];
              */
              if ( method == BarrelDistortion )
                ScaleFilter( resample_filter[id],
                     coeff[3], 0, 0, coeff[7] );
              else /* method == BarrelInverseDistortion */
                /* FUTURE, trap for D==0 causing division by zero */
                ScaleFilter( resample_filter[id],
                     1.0/coeff[3], 0, 0, 1.0/coeff[7] );
            }
            break;
          }
          case ShepardsDistortion:
          { /* Shepards Method, or Inverse Weighted Distance for
               displacement around the destination image control points
               The input arguments are the coefficents to the function.
               This is more of a 'displacement' function rather than an
               absolute distortion function.

               Note: We can not determine derivatives using shepards method
               so only a point sample interpolatation can be used.
            */
            size_t
              i;
            double
              denominator;

            denominator = s.x = s.y = 0;
            for(i=0; i<number_arguments; i+=4) {
              double weight =
                  ((double)d.x-arguments[i+2])*((double)d.x-arguments[i+2])
                + ((double)d.y-arguments[i+3])*((double)d.y-arguments[i+3]);
              weight = pow(weight,coeff[0]); /* shepards power factor */
              weight = ( weight < 1.0 ) ? 1.0 : 1.0/weight;

              s.x += (arguments[ i ]-arguments[i+2])*weight;
              s.y += (arguments[i+1]-arguments[i+3])*weight;
              denominator += weight;
            }
            s.x /= denominator;
            s.y /= denominator;
            s.x += d.x;   /* make it as relative displacement */
            s.y += d.y;
            break;
          }
          default:
            break; /* use the default no-op given above */
        }
        /* map virtual canvas location back to real image coordinate */
        if ( bestfit && method != ArcDistortion ) {
          s.x -= image->page.x;
          s.y -= image->page.y;
        }
        s.x -= 0.5;
        s.y -= 0.5;

        if ( validity <= 0.0 ) {
          /* result of distortion is an invalid pixel - don't resample */
          SetPixelViaPixelInfo(distort_image,&invalid,q);
        }
        else {
          /* resample the source image to find its correct color */
          (void) ResamplePixelColor(resample_filter[id],s.x,s.y,&pixel,
            exception);
          /* if validity between 0.0 and 1.0 mix result with invalid pixel */
          if ( validity < 1.0 ) {
            /* Do a blend of sample color and invalid pixel */
            /* should this be a 'Blend', or an 'Over' compose */
            CompositePixelInfoBlend(&pixel,validity,&invalid,(1.0-validity),
              &pixel);
          }
          SetPixelViaPixelInfo(distort_image,&pixel,q);
        }
        q+=GetPixelChannels(distort_image);
      }
      sync=SyncCacheViewAuthenticPixels(distort_view,exception);
      if (sync == MagickFalse)
        status=MagickFalse;
      if (image->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp critical (MagickCore_DistortImage)
#endif
          proceed=SetImageProgress(image,DistortImageTag,progress++,
            image->rows);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
    }
    distort_view=DestroyCacheView(distort_view);
    resample_filter=DestroyResampleFilterThreadSet(resample_filter);

    if (status == MagickFalse)
      distort_image=DestroyImage(distort_image);
  }

  /* Arc does not return an offset unless 'bestfit' is in effect
     And the user has not provided an overriding 'viewport'.
   */
  if ( method == ArcDistortion && !bestfit && !viewport_given ) {
    distort_image->page.x = 0;
    distort_image->page.y = 0;
  }
  coeff=(double *) RelinquishMagickMemory(coeff);
  return(distort_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R o t a t e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RotateImage() creates a new image that is a rotated copy of an existing
%  one.  Positive angles rotate counter-clockwise (right-hand rule), while
%  negative angles rotate clockwise.  Rotated images are usually larger than
%  the originals and have 'empty' triangular corners.  X axis.  Empty
%  triangles left over from shearing the image are filled with the background
%  color defined by member 'background_color' of the image.  RotateImage
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the RotateImage method is:
%
%      Image *RotateImage(const Image *image,const double degrees,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: Specifies the number of degrees to rotate the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *RotateImage(const Image *image,const double degrees,
  ExceptionInfo *exception)
{
  Image
    *distort_image,
    *rotate_image;

  double
    angle;

  PointInfo
    shear;

  size_t
    rotations;

  /*
    Adjust rotation angle.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  angle=fmod(degrees,360.0);
  while (angle < -45.0)
    angle+=360.0;
  for (rotations=0; angle > 45.0; rotations++)
    angle-=90.0;
  rotations%=4;
  shear.x=(-tan((double) DegreesToRadians(angle)/2.0));
  shear.y=sin((double) DegreesToRadians(angle));
  if ((fabs(shear.x) < MagickEpsilon) && (fabs(shear.y) < MagickEpsilon))
    return(IntegralRotateImage(image,rotations,exception));
  distort_image=CloneImage(image,0,0,MagickTrue,exception);
  if (distort_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageVirtualPixelMethod(distort_image,BackgroundVirtualPixelMethod,
    exception);
  rotate_image=DistortImage(distort_image,ScaleRotateTranslateDistortion,1,
    &degrees,MagickTrue,exception);
  distort_image=DestroyImage(distort_image);
  return(rotate_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p a r s e C o l o r I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SparseColorImage(), given a set of coordinates, interpolates the colors
%  found at those coordinates, across the whole image, using various methods.
%
%  The format of the SparseColorImage() method is:
%
%      Image *SparseColorImage(const Image *image,
%        const SparseColorMethod method,const size_t number_arguments,
%        const double *arguments,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to be filled in.
%
%    o method: the method to fill in the gradient between the control points.
%
%        The methods used for SparseColor() are often simular to methods
%        used for DistortImage(), and even share the same code for determination
%        of the function coefficents, though with more dimensions (or resulting
%        values).
%
%    o number_arguments: the number of arguments given.
%
%    o arguments: array of floating point arguments for this method--
%        x,y,color_values-- with color_values given as normalized values.
%
%    o exception: return any errors or warnings in this structure
%
*/
MagickExport Image *SparseColorImage(const Image *image,
  const SparseColorMethod method,const size_t number_arguments,
  const double *arguments,ExceptionInfo *exception)
{
#define SparseColorTag  "Distort/SparseColor"

  SparseColorMethod
    sparse_method;

  double
    *coeff;

  Image
    *sparse_image;

  size_t
    number_colors;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);

  /* Determine number of color values needed per control point */
  number_colors=0;
  if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
    number_colors++;
  if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
    number_colors++;
  if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
    number_colors++;
  if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
      (image->colorspace == CMYKColorspace))
    number_colors++;
  if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
      (image->alpha_trait != UndefinedPixelTrait))
    number_colors++;

  /*
    Convert input arguments into mapping coefficients, this this case
    we are mapping (distorting) colors, rather than coordinates.
  */
  { DistortMethod
      distort_method;

    distort_method=(DistortMethod) method;
    if ( distort_method >= SentinelDistortion )
      distort_method = ShepardsDistortion; /* Pretend to be Shepards */
    coeff = GenerateCoefficients(image, &distort_method, number_arguments,
                arguments, number_colors, exception);
    if ( coeff == (double *) NULL )
      return((Image *) NULL);
    /*
      Note some Distort Methods may fall back to other simpler methods,
      Currently the only fallback of concern is Bilinear to Affine
      (Barycentric), which is alaso sparse_colr method.  This also ensures
      correct two and one color Barycentric handling.
    */
    sparse_method = (SparseColorMethod) distort_method;
    if ( distort_method == ShepardsDistortion )
      sparse_method = method;   /* return non-distort methods to normal */
    if ( sparse_method == InverseColorInterpolate )
      coeff[0]=0.5;            /* sqrt() the squared distance for inverse */
  }

  /* Verbose output */
  if (IsStringTrue(GetImageArtifact(image,"verbose")) != MagickFalse) {

    switch (sparse_method) {
      case BarycentricColorInterpolate:
      {
        register ssize_t x=0;
        (void) FormatLocaleFile(stderr, "Barycentric Sparse Color:\n");
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          (void) FormatLocaleFile(stderr, "  -channel R -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          (void) FormatLocaleFile(stderr, "  -channel G -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          (void) FormatLocaleFile(stderr, "  -channel B -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
            (image->colorspace == CMYKColorspace))
          (void) FormatLocaleFile(stderr, "  -channel K -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
            (image->alpha_trait != UndefinedPixelTrait))
          (void) FormatLocaleFile(stderr, "  -channel A -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        break;
      }
      case BilinearColorInterpolate:
      {
        register ssize_t x=0;
        (void) FormatLocaleFile(stderr, "Bilinear Sparse Color\n");
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          (void) FormatLocaleFile(stderr, "   -channel R -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          (void) FormatLocaleFile(stderr, "   -channel G -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          (void) FormatLocaleFile(stderr, "   -channel B -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
            (image->colorspace == CMYKColorspace))
          (void) FormatLocaleFile(stderr, "   -channel K -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
            (image->alpha_trait != UndefinedPixelTrait))
          (void) FormatLocaleFile(stderr, "   -channel A -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        break;
      }
      default:
        /* sparse color method is too complex for FX emulation */
        break;
    }
  }

  /* Generate new image for generated interpolated gradient.
   * ASIDE: Actually we could have just replaced the colors of the original
   * image, but IM Core policy, is if storage class could change then clone
   * the image.
   */

  sparse_image=CloneImage(image,0,0,MagickTrue,exception);
  if (sparse_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(sparse_image,DirectClass,exception) == MagickFalse)
    { /* if image is ColorMapped - change it to DirectClass */
      sparse_image=DestroyImage(sparse_image);
      return((Image *) NULL);
    }
  { /* ----- MAIN CODE ----- */
    CacheView
      *sparse_view;

    MagickBooleanType
      status;

    MagickOffsetType
      progress;

    ssize_t
      j;

    status=MagickTrue;
    progress=0;
    sparse_view=AcquireAuthenticCacheView(sparse_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp parallel for schedule(static) shared(progress,status) \
      magick_number_threads(image,sparse_image,sparse_image->rows,1)
#endif
    for (j=0; j < (ssize_t) sparse_image->rows; j++)
    {
      MagickBooleanType
        sync;

      PixelInfo
        pixel;    /* pixel to assign to distorted image */

      register ssize_t
        i;

      register Quantum
        *magick_restrict q;

      q=GetCacheViewAuthenticPixels(sparse_view,0,j,sparse_image->columns,
        1,exception);
      if (q == (Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      GetPixelInfo(sparse_image,&pixel);
      for (i=0; i < (ssize_t) image->columns; i++)
      {
        GetPixelInfoPixel(image,q,&pixel);
        switch (sparse_method)
        {
          case BarycentricColorInterpolate:
          {
            register ssize_t x=0;
            if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
              pixel.red     = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
              pixel.green   = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
              pixel.blue    = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
                (image->colorspace == CMYKColorspace))
              pixel.black   = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
                (image->alpha_trait != UndefinedPixelTrait))
              pixel.alpha = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            break;
          }
          case BilinearColorInterpolate:
          {
            register ssize_t x=0;
            if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
              pixel.red     = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
              pixel.green   = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
              pixel.blue    = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
                (image->colorspace == CMYKColorspace))
              pixel.black   = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
                (image->alpha_trait != UndefinedPixelTrait))
              pixel.alpha = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            break;
          }
          case InverseColorInterpolate:
          case ShepardsColorInterpolate:
          { /* Inverse (Squared) Distance weights average (IDW) */
            size_t
              k;
            double
              denominator;

            if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
              pixel.red=0.0;
            if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
              pixel.green=0.0;
            if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
              pixel.blue=0.0;
            if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
                (image->colorspace == CMYKColorspace))
              pixel.black=0.0;
            if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
                (image->alpha_trait != UndefinedPixelTrait))
              pixel.alpha=0.0;
            denominator = 0.0;
            for(k=0; k<number_arguments; k+=2+number_colors) {
              register ssize_t x=(ssize_t) k+2;
              double weight =
                  ((double)i-arguments[ k ])*((double)i-arguments[ k ])
                + ((double)j-arguments[k+1])*((double)j-arguments[k+1]);
              weight = pow(weight,coeff[0]); /* inverse of power factor */
              weight = ( weight < 1.0 ) ? 1.0 : 1.0/weight;
              if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
                pixel.red     += arguments[x++]*weight;
              if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
                pixel.green   += arguments[x++]*weight;
              if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
                pixel.blue    += arguments[x++]*weight;
              if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
                  (image->colorspace == CMYKColorspace))
                pixel.black   += arguments[x++]*weight;
              if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
                  (image->alpha_trait != UndefinedPixelTrait))
                pixel.alpha += arguments[x++]*weight;
              denominator += weight;
            }
            if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
              pixel.red/=denominator;
            if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
              pixel.green/=denominator;
            if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
              pixel.blue/=denominator;
            if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
                (image->colorspace == CMYKColorspace))
              pixel.black/=denominator;
            if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
                (image->alpha_trait != UndefinedPixelTrait))
              pixel.alpha/=denominator;
            break;
          }
          case ManhattanColorInterpolate:
          {
            size_t
              k;

            double
              minimum = MagickMaximumValue;

            /*
              Just use the closest control point you can find!
            */
            for(k=0; k<number_arguments; k+=2+number_colors) {
              double distance =
                  fabs((double)i-arguments[ k ])
                + fabs((double)j-arguments[k+1]);
              if ( distance < minimum ) {
                register ssize_t x=(ssize_t) k+2;
                if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
                  pixel.red=arguments[x++];
                if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
                  pixel.green=arguments[x++];
                if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
                  pixel.blue=arguments[x++];
                if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
                    (image->colorspace == CMYKColorspace))
                  pixel.black=arguments[x++];
                if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
                    (image->alpha_trait != UndefinedPixelTrait))
                  pixel.alpha=arguments[x++];
                minimum = distance;
              }
            }
            break;
          }
          case VoronoiColorInterpolate:
          default:
          {
            size_t
              k;

            double
              minimum = MagickMaximumValue;

            /*
              Just use the closest control point you can find!
            */
            for (k=0; k<number_arguments; k+=2+number_colors) {
              double distance =
                  ((double)i-arguments[ k ])*((double)i-arguments[ k ])
                + ((double)j-arguments[k+1])*((double)j-arguments[k+1]);
              if ( distance < minimum ) {
                register ssize_t x=(ssize_t) k+2;
                if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
                  pixel.red=arguments[x++];
                if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
                  pixel.green=arguments[x++];
                if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
                  pixel.blue=arguments[x++];
                if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
                    (image->colorspace == CMYKColorspace))
                  pixel.black=arguments[x++];
                if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
                    (image->alpha_trait != UndefinedPixelTrait))
                  pixel.alpha=arguments[x++];
                minimum = distance;
              }
            }
            break;
          }
        }
        /* set the color directly back into the source image */
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          pixel.red=(MagickRealType) ClampPixel(QuantumRange*pixel.red);
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          pixel.green=(MagickRealType) ClampPixel(QuantumRange*pixel.green);
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          pixel.blue=(MagickRealType) ClampPixel(QuantumRange*pixel.blue);
        if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
            (image->colorspace == CMYKColorspace))
          pixel.black=(MagickRealType) ClampPixel(QuantumRange*pixel.black);
        if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
            (image->alpha_trait != UndefinedPixelTrait))
          pixel.alpha=(MagickRealType) ClampPixel(QuantumRange*pixel.alpha);
        SetPixelViaPixelInfo(sparse_image,&pixel,q);
        q+=GetPixelChannels(sparse_image);
      }
      sync=SyncCacheViewAuthenticPixels(sparse_view,exception);
      if (sync == MagickFalse)
        status=MagickFalse;
      if (image->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp critical (MagickCore_SparseColorImage)
#endif
          proceed=SetImageProgress(image,SparseColorTag,progress++,image->rows);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
    }
    sparse_view=DestroyCacheView(sparse_view);
    if (status == MagickFalse)
      sparse_image=DestroyImage(sparse_image);
  }
  coeff = (double *) RelinquishMagickMemory(coeff);
  return(sparse_image);
}

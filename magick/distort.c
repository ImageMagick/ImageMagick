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
%                                John Cristy                                  %
%                              Anthony Thyssen                                %
%                                 June 2007                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/cache.h"
#include "magick/cache-view.h"
#include "magick/colorspace-private.h"
#include "magick/composite-private.h"
#include "magick/distort.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/hashmap.h"
#include "magick/image.h"
#include "magick/list.h"
#include "magick/matrix.h"
#include "magick/memory_.h"
#include "magick/monitor-private.h"
#include "magick/pixel.h"
#include "magick/pixel-private.h"
#include "magick/resample.h"
#include "magick/resample-private.h"
#include "magick/registry.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/token.h"

/*
  Numerous internal routines for image distortions.
*/
static inline double MagickMin(const double x,const double y)
{
  return( x < y ? x : y);
}
static inline double MagickMax(const double x,const double y)
{
  return( x > y ? x : y);
}

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

  determinant=1.0/(coeff[0]*coeff[4]-coeff[1]*coeff[3]);
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

  determinant=1.0/(coeff[0]*coeff[4]-coeff[3]*coeff[1]);
  inverse[0]=determinant*(coeff[4]-coeff[7]*coeff[5]);
  inverse[1]=determinant*(coeff[7]*coeff[2]-coeff[1]);
  inverse[2]=determinant*(coeff[1]*coeff[5]-coeff[4]*coeff[2]);
  inverse[3]=determinant*(coeff[6]*coeff[5]-coeff[3]);
  inverse[4]=determinant*(coeff[0]-coeff[6]*coeff[2]);
  inverse[5]=determinant*(coeff[3]*coeff[2]-coeff[0]*coeff[5]);
  inverse[6]=determinant*(coeff[3]*coeff[7]-coeff[6]*coeff[4]);
  inverse[7]=determinant*(coeff[6]*coeff[1]-coeff[0]*coeff[7]);
}

static inline double MagickRound(double x)
{
  /* round the fraction to nearest integer */
  if (x >= 0.0)
    return((double) ((long) (x+0.5)));
  return((double) ((long) (x-0.5)));
}

/*
 * Polynomial Term Defining Functions
 *
 * Order must either be an integer, or 1.5 to produce
 * the 2 number_valuesal polyminal function...
 *    affine     1 (3)      u = c0 + c1*x + c2*y
 *    bilinear  1.5 (4)     u = '' + c3*x*y
 *    quadratic  2 (6)      u = '' + c4*x*x + c5*y*y
 *    cubic      3 (10)     u = '' + c6*x^3 + c7*x*x*y + c8*x*y*y + c9*y^3
 *    quartic    4 (15)     u = '' + c10*x^4 + ... + c14*y^4
 *    quintic    5 (21)     u = '' + c15*x^5 + ... + c20*y^5
 * number in parenthesis minimum number of points needed.
 * Anything beyond quintic, has not been implemented until
 * a more automated way of determined terms is found.

 * Note the slight re-ordering of the terms for a quadratic polynomial
 * which is to allow the use of a bi-linear (order=1.5) polynomial.
 * All the later polynomials are ordered simply from x^N to y^N
 */
static unsigned long poly_number_terms(double order)
{
 /* Return the number of terms for a 2d polynomial */
  if ( order < 1 || order > 5 ||
       ( order != floor(order) && (order-1.5) > MagickEpsilon) )
    return 0; /* invalid polynomial order */
  return((unsigned long) floor((order+1)*(order+2)/2));
}

static double poly_basis_fn(long n, double x, double y)
{
  /* Return the result for this polynomial term */
  switch(n) {
    case  0:  return( 1.0 ); /* constant */
    case  1:  return(  x  );
    case  2:  return(  y  ); /* affine      order = 1   terms = 3 */
    case  3:  return( x*y ); /* bilinear    order = 1.5 terms = 4 */
    case  4:  return( x*x );
    case  5:  return( y*y ); /* quadratic   order = 2   terms = 6 */
    case  6:  return( x*x*x );
    case  7:  return( x*x*y );
    case  8:  return( x*y*y );
    case  9:  return( y*y*y ); /* cubic       order = 3   terms = 10 */
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
    case 20:  return( y*y*y*y*y ); /* quintic     order = 5   terms = 21 */
  }
  return( 0 ); /* should never happen */
}
static const char *poly_basis_str(long n)
{
  /* return the result for this polynomial term */
  switch(n) {
    case  0:  return(""); /* constant */
    case  1:  return("*ii");
    case  2:  return("*jj"); /* affiine      order = 1   terms = 3 */
    case  3:  return("*ii*jj"); /* biiliinear    order = 1.5 terms = 4 */
    case  4:  return("*ii*ii");
    case  5:  return("*jj*jj"); /* quadratiic   order = 2   terms = 6 */
    case  6:  return("*ii*ii*ii");
    case  7:  return("*ii*ii*jj");
    case  8:  return("*ii*jj*jj");
    case  9:  return("*jj*jj*jj"); /* cubiic       order = 3   terms = 10 */
    case 10:  return("*ii*ii*ii*ii");
    case 11:  return("*ii*ii*ii*jj");
    case 12:  return("*ii*ii*jj*jj");
    case 13:  return("*ii*jj*jj*jj");
    case 14:  return("*jj*jj*jj*jj"); /* quartiic     order = 4   terms = 15 */
    case 15:  return("*ii*ii*ii*ii*ii");
    case 16:  return("*ii*ii*ii*ii*jj");
    case 17:  return("*ii*ii*ii*jj*jj");
    case 18:  return("*ii*ii*jj*jj*jj");
    case 19:  return("*ii*jj*jj*jj*jj");
    case 20:  return("*jj*jj*jj*jj*jj"); /* quiintiic     order = 5   terms = 21 */
  }
  return( "UNKNOWN" ); /* should never happen */
}
static double poly_basis_dx(long n, double x, double y)
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
static double poly_basis_dy(long n, double x, double y)
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
  /* NOTE: the only reason that last is not true for 'quadtratic'
     is due to the re-arrangement of terms to allow for 'bilinear'
  */
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
%    Image *GenerateCoefficients(const Image *image,DistortImageMethod method,
%        const unsigned long number_arguments,const double *arguments,
%        unsigned long number_values, ExceptionInfo *exception)
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
%            IN future, varible number of values may be given (1 to N)
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

static double *GenerateCoefficients(const Image *image,
  DistortImageMethod *method,const unsigned long number_arguments,
  const double *arguments,unsigned long number_values,ExceptionInfo *exception)
{
  double
    *coeff;

  register unsigned long
    i;

  unsigned long
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
      if ( number_arguments <= 1 && (number_arguments-1)%cp_size != 0)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                   "InvalidArgument","%s : '%s'","Polynomial",
                   "Invalid number of args: order [CPs]...");
        return((double *) NULL);
      }
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
               "InvalidArgument", "%s : 'require at least %ld CPs'",
               "Polynomial", i);
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
    case VoronoiColorInterpolate:
      number_coeff=1;  /* not used, but provide some type of return */
      break;
    case ArcDistortion:
      number_coeff=5;
      break;
    case ScaleRotateTranslateDistortion:
    case AffineProjectionDistortion:
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
    case UndefinedDistortion:
    default:
      assert(! "Unknown Method Given"); /* just fail assertion */
  }

  /* allocate the array of coefficients needed */
  coeff = (double *) AcquireQuantumMemory(number_coeff,sizeof(*coeff));
  if (coeff == (double *) NULL) {
    (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "GenerateCoefficients");
    return((double *) NULL);
  }

  /* zero out coeffiecents array */
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
               "InvalidArgument", "%s : 'require at least %ld CPs'",
               "Affine", 1L);
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
           Solve a least squares simultanious equation for coefficients.
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
                      "InvalidArgument","%s : '%s'","Affine",
                      "Unsolvable Matrix");
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
                     "InvalidArgument","%s : '%s'","AffineProjection",
                     "Needs 6 coeff values");
        return((double *) NULL);
      }
      /* FUTURE: trap test for sx*sy-rx*ry == 0 (determinate = 0, no inverse) */
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
         An alturnative Affine Distortion
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
           + Can not be used for generating a sparse gradient (interpolation)
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
                     "InvalidArgument","%s : '%s'", "ScaleTranslateRotate",
                     "Needs at least 1 argument");
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
                     "InvalidArgument","%s : '%s'", "ScaleTranslateRotate",
                     "Too Many Arguments (7 or less)");
          return((double *) NULL);
        }
        break;
      }
      /* Trap if sx or sy == 0 -- image is scaled out of existance! */
      if ( fabs(sx) < MagickEpsilon || fabs(sy) < MagickEpsilon ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'", "ScaleTranslateRotate",
                     "Zero Scale Given");
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

      unsigned long
        cp_u = cp_values,
        cp_v = cp_values+1;

      MagickBooleanType
        status;

      if ( number_arguments%cp_size != 0 ||
           number_arguments < cp_size*4 ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
               "InvalidArgument", "%s : 'require at least %ld CPs'",
               "Perspective", 4L);
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      /* fake 1x8 vectors matrix directly using the coefficients array */
      vectors[0] = &(coeff[0]);
      /* 8x8 least-squares matrix (zeroed) */
      matrix = AcquireMagickMatrix(8UL,8UL);
      if (matrix == (double **) NULL) {
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
                    "InvalidArgument","%s : '%s'","Perspective",
                    "Unsolvable Matrix");
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
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","PerspectiveProjection",
                     "Needs 8 coefficient values");
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
               "InvalidArgument", "%s : 'require at least %ld CPs'",
               *method == BilinearForwardDistortion ? "BilinearForward" :
               "BilinearReverse", 4L);
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
                     "InvalidArgument","%s : '%s'",
                     *method == BilinearForwardDistortion ?
                                "BilinearForward" : "BilinearReverse",
                     "Unsolvable Matrix");
        return((double *) NULL);
      }
      if ( *method == BilinearForwardDistortion ) {
         /* Bilinear Forward Mapped Distortion

         The above least-squares solved for coefficents but in the forward
         direction, due to changes to indexing constants.

            i = c0*x + c1*y + c2*x*y + c3;
            j = c4*x + c5*y + c6*x*y + c7;

         where u,v are in the destination image, NOT the source.

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

      unsigned long
        nterms;   /* number of polynomial terms per number_values */

      register long
        j;

      MagickBooleanType
        status;

      /* first two coefficients hold polynomial order information */
      coeff[0] = arguments[0];
      coeff[1] = (double) poly_number_terms(arguments[0]);
      nterms = (unsigned long) coeff[1];

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
        for (j=0; j < (long) nterms; j++)
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
                     "InvalidArgument","%s : '%s'","Polynomial",
                     "Unsolvable Matrix");
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
                     "InvalidArgument","%s : '%s'", "Arc",
                     "Arc Angle Too Small");
        return((double *) NULL);
      }
      if ( number_arguments >= 3 && arguments[2] < MagickEpsilon ) {
        coeff = (double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'", "Arc",
                     "Outer Radius Too Small");
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

         WARNING: It is posible for  Radius max<min  and/or  Angle from>to
      */
      if ( number_arguments == 3
          || ( number_arguments > 6 && *method == PolarDistortion )
          || number_arguments > 8 ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
               "InvalidArgument", "%s : number of arguments",
               *method == PolarDistortion ? "Polar" : "DePolar");
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
               *method == PolarDistortion ? "Polar" : "DePolar");
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
    case BarrelDistortion:
    case BarrelInverseDistortion:
    {
      /* Barrel Distortion
           Rs=(A*Rd^3 + B*Rd^2 + C*Rd + D)*Rd
         BarrelInv Distortion
           Rs=Rd/(A*Rd^3 + B*Rd^2 + C*Rd + D)

        Where Rd is the normalized radius from corner to middle of image
        Input Arguments are one of the following forms...
             A,B,C
             A,B,C,D
             A,B,C    X,Y
             A,B,C,D  X,Y
             Ax,Bx,Cx,Dx  Ay,By,Cy,Dy
             Ax,Bx,Cx,Dx  Ay,By,Cy,Dy   X,Y

        Returns 10 coefficent values, which are de-normalized (pixel scale)
          Ax, Bx, Cx, Dx,   Ay, By, Cy, Dy,    Xc, Yc
      */
      /* Radius de-normalization scaling factor */
      double
        rscale = 2.0/MagickMin((double) image->columns,(double) image->rows);

      if ( number_arguments != 4 && number_arguments != 6 &&
           number_arguments != 8 && number_arguments != 10 ) {
        coeff=(double *) RelinquishMagickMemory(coeff);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
               "InvalidArgument", "%s : '%s'", "Barrel(Inv)",
               "number of arguments" );
        return((double *) NULL);
      }
      /* A,B,C,D coefficients */
      coeff[0] = arguments[0];
      coeff[1] = arguments[1];
      coeff[2] = arguments[2];
      if ( number_arguments == 3 || number_arguments == 5 )
        coeff[3] = 1 - arguments[0] - arguments[1] - arguments[2];
      else
         coeff[3] = arguments[3];
      /* de-normalize the X coefficients */
      coeff[0] *= pow(rscale,3.0);
      coeff[1] *= rscale*rscale;
      coeff[2] *= rscale;
      /* Y coefficients: as given OR as X coefficients */
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
      /* X,Y Center of Distortion */
      coeff[8] = ((double)image->columns-1)/2.0 + image->page.x;
      coeff[9] = ((double)image->rows-1)/2.0    + image->page.y;
      if ( number_arguments == 5 )  {
        coeff[8] = arguments[3];
        coeff[9] = arguments[4];
      }
      if ( number_arguments == 6 ) {
        coeff[8] = arguments[4];
        coeff[9] = arguments[5];
      }
      if ( number_arguments == 10 ) {
        coeff[8] = arguments[8];
        coeff[9] = arguments[9];
      }
      return(coeff);
    }
    case ShepardsDistortion:
    case VoronoiColorInterpolate:
    {
      /* Shepards Distortion  input arguments are the coefficents!
         Just check the number of arguments is valid!
         Args:  u1,v1, x1,y1, ...
          OR :  u1,v1, r1,g1,c1, ...
      */
      if ( number_arguments%cp_size != 0 ||
           number_arguments < cp_size ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
               "InvalidArgument", "%s : 'require at least %ld CPs'",
               "Shepards", 1UL);
        coeff=(double *) RelinquishMagickMemory(coeff);
        return((double *) NULL);
      }
      return(coeff);
    }
    default:
      break;
  }
  /* you should never reach this point */
  assert(! "No Method Handler"); /* just fail assertion */
  return((double *) NULL);
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
%      Image *DistortImage(const Image *image,const DistortImageMethod method,
%        const unsigned long number_arguments,const double *arguments,
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
%        equivelents for the distortion operation (if feasible).
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
MagickExport Image *DistortImage(const Image *image,DistortImageMethod method,
  const unsigned long number_arguments,const double *arguments,
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

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  /*
    Convert input arguments (usally as control points for reverse mapping)
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
    /* always use the 'best fit' viewport */
    bestfit = MagickTrue;
  }

  /* Work out the 'best fit', (required for ArcDistortion) */
  if ( bestfit ) {
    PointInfo
      s,d,min,max;

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
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
        d.x = scale*(inverse[0]*s.x+inverse[1]*s.y+inverse[2]);
        d.y = scale*(inverse[3]*s.x+inverse[4]*s.y+inverse[5]);
        InitalBounds(d);
        s.x = (double) image->page.x+image->columns;
        s.y = (double) image->page.y;
        scale=inverse[6]*s.x+inverse[7]*s.y+1.0;
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
        d.x = scale*(inverse[0]*s.x+inverse[1]*s.y+inverse[2]);
        d.y = scale*(inverse[3]*s.x+inverse[4]*s.y+inverse[5]);
        ExpandBounds(d);
        s.x = (double) image->page.x;
        s.y = (double) image->page.y+image->rows;
        scale=inverse[6]*s.x+inverse[7]*s.y+1.0;
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
        d.x = scale*(inverse[0]*s.x+inverse[1]*s.y+inverse[2]);
        d.y = scale*(inverse[3]*s.x+inverse[4]*s.y+inverse[5]);
        ExpandBounds(d);
        s.x = (double) image->page.x+image->columns;
        s.y = (double) image->page.y+image->rows;
        scale=inverse[6]*s.x+inverse[7]*s.y+1.0;
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
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
        for( a=ceil((coeff[0]-coeff[1]/2.0)/MagickPI2)*MagickPI2;
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
        coeff[1] = Magick2PI*image->columns/coeff[1];
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
        geometry.x = geometry.y = 0;
        geometry.height = (unsigned long) ceil(coeff[0]-coeff[1]);
        geometry.width = (unsigned long)
                  ceil((coeff[0]-coeff[1])*(coeff[5]-coeff[4])*0.5);
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
        /* no bestfit available for this distortion */
        bestfit = MagickFalse;
        break;
    }
    /* Set the output image geometry to calculated 'bestfit'
       Do not do this for DePolar which needs to be exact for tiling
    */
    if ( bestfit && method != DePolarDistortion ) {
      geometry.x = (long) floor(min.x-0.5);
      geometry.y = (long) floor(min.y-0.5);
      geometry.width=(unsigned long) ceil(max.x-geometry.x+0.5);
      geometry.height=(unsigned long) ceil(max.y-geometry.y+0.5);
    }
    /* now that we have a new size lets fit distortion to it exactly */
    if ( method == DePolarDistortion ) {
      coeff[6]=(coeff[5]-coeff[4])/geometry.width; /* changed width */
      coeff[7]=(coeff[0]-coeff[1])/geometry.height; /* should be about 1.0 */
    }
  }

  /* The user provided a 'viewport' expert option which may
     overrides some parts of the current output image geometry.
     For ArcDistortion, this also overrides its default 'bestfit' setting.
  */
  { const char *artifact=GetImageArtifact(image,"distort:viewport");
    viewport_given = MagickFalse;
    if ( artifact != (const char *) NULL ) {
      (void) ParseAbsoluteGeometry(artifact,&geometry);
      viewport_given = MagickTrue;
    }
  }

  /* Verbose output */
  if ( GetImageArtifact(image,"verbose") != (const char *) NULL ) {
    register long
       i;
    char image_gen[MaxTextExtent];
    const char *lookup;

    /* Set destination image size and virtual offset */
    if ( bestfit || viewport_given ) {
      (void) FormatMagickString(image_gen, MaxTextExtent,"  -size %lux%lu "
        "-page %+ld%+ld xc: +insert \\\n",geometry.width,geometry.height,
        geometry.x,geometry.y);
      lookup="v.p{ xx-v.page.x-.5, yy-v.page.x-.5 }";
    }
    else {
      image_gen[0] = '\0';             /* no destination to generate */
      lookup = "p{ xx-page.x-.5, yy-page.x-.5 }"; /* simplify lookup */
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
        fprintf(stderr, "Affine Projection:\n");
        fprintf(stderr, "  -distort AffineProjection \\\n      '");
        for (i=0; i<5; i++)
          fprintf(stderr, "%lf,", inverse[i]);
        fprintf(stderr, "%lf'\n", inverse[5]);
        inverse = (double *) RelinquishMagickMemory(inverse);

        fprintf(stderr, "Affine Distort, FX Equivelent:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        fprintf(stderr, "       xx=%+lf*ii %+lf*jj %+lf;\n",
            coeff[0], coeff[1], coeff[2]);
        fprintf(stderr, "       yy=%+lf*ii %+lf*jj %+lf;\n",
            coeff[3], coeff[4], coeff[5]);
        fprintf(stderr, "       %s'\n", lookup);

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
        fprintf(stderr, "Perspective Projection:\n");
        fprintf(stderr, "  -distort PerspectiveProjection \\\n      '");
        for (i=0; i<4; i++)
          fprintf(stderr, "%lf, ", inverse[i]);
        fprintf(stderr, "\n       ");
        for (; i<7; i++)
          fprintf(stderr, "%lf, ", inverse[i]);
        fprintf(stderr, "%lf'\n", inverse[7]);
        inverse = (double *) RelinquishMagickMemory(inverse);

        fprintf(stderr, "Perspective Distort, FX Equivelent:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        fprintf(stderr, "       rr=%+lf*ii %+lf*jj + 1;\n",
            coeff[6], coeff[7]);
        fprintf(stderr, "       xx=(%+lf*ii %+lf*jj %+lf)/rr;\n",
            coeff[0], coeff[1], coeff[2]);
        fprintf(stderr, "       yy=(%+lf*ii %+lf*jj %+lf)/rr;\n",
            coeff[3], coeff[4], coeff[5]);
        fprintf(stderr, "       rr%s0 ? %s : blue'\n",
            coeff[8] < 0 ? "<" : ">", lookup);
        break;
      }

      case BilinearForwardDistortion:
        fprintf(stderr, "BilinearForward Mapping Equations:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "    i = %+lf*x %+lf*y %+lf*x*y %+lf;\n",
            coeff[0], coeff[1], coeff[2], coeff[3]);
        fprintf(stderr, "    j = %+lf*x %+lf*y %+lf*x*y %+lf;\n",
            coeff[4], coeff[5], coeff[6], coeff[7]);
#if 0
        /* for debugging */
        fprintf(stderr, "   c8 = %+lf  c9 = 2*a = %+lf;\n",
            coeff[8], coeff[9]);
#endif
        fprintf(stderr, "BilinearForward Distort, FX Equivelent:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'ii=i+page.x%+lf; jj=j+page.y%+lf;\n",
            0.5-coeff[3], 0.5-coeff[7]);
        fprintf(stderr, "       bb=%lf*ii %+lf*jj %+lf;\n",
            coeff[6], -coeff[2], coeff[8]);
        /* Handle Special degenerate (non-quadratic) or trapezoidal case */
        if ( coeff[9] != 0 ) {
          fprintf(stderr, "       rt=bb*bb %+lf*(%lf*ii%+lf*jj);\n",
              -2*coeff[9],  coeff[4], -coeff[0]);
          fprintf(stderr, "       yy=( -bb + sqrt(rt) ) / %lf;\n",
               coeff[9]);
        } else
          fprintf(stderr, "       yy=(%lf*ii%+lf*jj)/bb;\n",
                -coeff[4], coeff[0]);
        fprintf(stderr, "       xx=(ii %+lf*yy)/(%lf %+lf*yy);\n",
             -coeff[1], coeff[0], coeff[2]);
        if ( coeff[9] != 0 )
          fprintf(stderr, "       (rt < 0 ) ? red : %s'\n", lookup);
        else
          fprintf(stderr, "       %s'\n", lookup);
        break;

      case BilinearReverseDistortion:
#if 0
        fprintf(stderr, "Polynomial Projection Distort:\n");
        fprintf(stderr, "  -distort PolynomialProjection \\\n");
        fprintf(stderr, "      '1.5, %lf, %lf, %lf, %lf,\n",
            coeff[3], coeff[0], coeff[1], coeff[2]);
        fprintf(stderr, "            %lf, %lf, %lf, %lf'\n",
            coeff[7], coeff[4], coeff[5], coeff[6]);
#endif
        fprintf(stderr, "BilinearReverse Distort, FX Equivelent:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        fprintf(stderr, "       xx=%+lf*ii %+lf*jj %+lf*ii*jj %+lf;\n",
            coeff[0], coeff[1], coeff[2], coeff[3]);
        fprintf(stderr, "       yy=%+lf*ii %+lf*jj %+lf*ii*jj %+lf;\n",
            coeff[4], coeff[5], coeff[6], coeff[7]);
        fprintf(stderr, "       %s'\n", lookup);
        break;

      case PolynomialDistortion:
      {
        unsigned long nterms = (unsigned long) coeff[1];
        fprintf(stderr, "Polynomial (order %lg, terms %lu), FX Equivelent\n",
                       coeff[0], nterms);
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'ii=i+page.x+0.5; jj=j+page.y+0.5;\n");
        fprintf(stderr, "       xx =");
        for (i=0; i<(long) nterms; i++) {
          if ( i != 0 && i%4 == 0 ) fprintf(stderr, "\n         ");
          fprintf(stderr, " %+lf%s", coeff[2+i],
               poly_basis_str(i));
        }
        fprintf(stderr, ";\n       yy =");
        for (i=0; i<(long) nterms; i++) {
          if ( i != 0 && i%4 == 0 ) fprintf(stderr, "\n         ");
          fprintf(stderr, " %+lf%s", coeff[2+i+nterms],
               poly_basis_str(i));
        }
        fprintf(stderr, ";\n       %s'\n", lookup);
        break;
      }
      case ArcDistortion:
      {
        fprintf(stderr, "Arc Distort, Internal Coefficients:\n");
        for ( i=0; i<5; i++ )
          fprintf(stderr, "  c%ld = %+lf\n", i, coeff[i]);
        fprintf(stderr, "Arc Distort, FX Equivelent:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'ii=i+page.x; jj=j+page.y;\n");
        fprintf(stderr, "       xx=(atan2(jj,ii)%+lf)/(2*pi);\n",
                                  -coeff[0]);
        fprintf(stderr, "       xx=xx-round(xx);\n");
        fprintf(stderr, "       xx=xx*%lf %+lf;\n",
                            coeff[1], coeff[4]);
        fprintf(stderr, "       yy=(%lf - hypot(ii,jj)) * %lf;\n",
                            coeff[2], coeff[3]);
        fprintf(stderr, "       v.p{xx-.5,yy-.5}'\n");
        break;
      }
      case PolarDistortion:
      {
        fprintf(stderr, "Polar Distort, Internal Coefficents\n");
        for ( i=0; i<8; i++ )
          fprintf(stderr, "  c%ld = %+lf\n", i, coeff[i]);
        fprintf(stderr, "Polar Distort, FX Equivelent:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'ii=i+page.x%+lf; jj=j+page.y%+lf;\n",
                         -coeff[2], -coeff[3]);
        fprintf(stderr, "       xx=(atan2(ii,jj)%+lf)/(2*pi);\n",
                         -(coeff[4]+coeff[5])/2 );
        fprintf(stderr, "       xx=xx-round(xx);\n");
        fprintf(stderr, "       xx=xx*2*pi*%lf + v.w/2;\n",
                         coeff[6] );
        fprintf(stderr, "       yy=(hypot(ii,jj)%+lf)*%lf;\n",
                         -coeff[1], coeff[7] );
        fprintf(stderr, "       v.p{xx-.5,yy-.5}'\n");
        break;
      }
      case DePolarDistortion:
      {
        fprintf(stderr, "DePolar Distort, Internal Coefficents\n");
        for ( i=0; i<8; i++ )
          fprintf(stderr, "  c%ld = %+lf\n", i, coeff[i]);
        fprintf(stderr, "DePolar Distort, FX Equivelent:\n");
        fprintf(stderr, "%s", image_gen);
        fprintf(stderr, "  -fx 'aa=(i+.5)*%lf %+lf;\n", coeff[6], -coeff[4] );
        fprintf(stderr, "       rr=(j+.5)*%lf %+lf;\n", coeff[7], +coeff[1] );
        fprintf(stderr, "       xx=rr*sin(aa) %+lf;\n", coeff[2] );
        fprintf(stderr, "       yy=rr*cos(aa) %+lf;\n", coeff[3] );
        fprintf(stderr, "       v.p{xx-.5,yy-.5}'\n");
        break;
      }
      case BarrelDistortion:
      case BarrelInverseDistortion:
      { double xc,yc;
        xc = ((double)image->columns-1.0)/2.0 + image->page.x;
        yc = ((double)image->rows-1.0)/2.0    + image->page.y;
        fprintf(stderr, "Barrel%s Distort, FX Equivelent:\n",
             method == BarrelDistortion ? "" : "Inv");
        fprintf(stderr, "%s", image_gen);
        if ( fabs(coeff[8]-xc) < 0.1 && fabs(coeff[9]-yc) < 0.1 )
          fprintf(stderr, "  -fx 'xc=(w-1)/2;  yc=(h-1)/2;\n");
        else
          fprintf(stderr, "  -fx 'xc=%lf;  yc=%lf;\n",
               coeff[8], coeff[9]);
        fprintf(stderr,
             "       ii=i-xc;  jj=j-yc;  rr=hypot(ii,jj);\n");
        fprintf(stderr, "       ii=ii%s(%lf*rr*rr*rr %+lf*rr*rr %+lf*rr %+lf);\n",
             method == BarrelDistortion ? "*" : "/",
             coeff[0],coeff[1],coeff[2],coeff[3]);
        fprintf(stderr, "       jj=jj%s(%lf*rr*rr*rr %+lf*rr*rr %+lf*rr %+lf);\n",
             method == BarrelDistortion ? "*" : "/",
             coeff[4],coeff[5],coeff[6],coeff[7]);
        fprintf(stderr, "       v.p{fx*ii+xc,fy*jj+yc}'\n");
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
      output_scaling = fabs(StringToDouble(artifact));
      geometry.width  *= output_scaling;
      geometry.height *= output_scaling;
      geometry.x      *= output_scaling;
      geometry.y      *= output_scaling;
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
    return((Image *) NULL);
  if (SetImageStorageClass(distort_image,DirectClass) == MagickFalse)
    { /* if image is ColorMapped - change it to DirectClass */
      InheritException(exception,&distort_image->exception);
      distort_image=DestroyImage(distort_image);
      return((Image *) NULL);
    }
  distort_image->page.x=geometry.x;
  distort_image->page.y=geometry.y;
  if (distort_image->background_color.opacity != OpaqueOpacity)
    distort_image->matte=MagickTrue;

  { /* ----- MAIN CODE -----
       Sample the source image to each pixel in the distort image.
     */
    long
      j,
      progress,
      status;

    MagickPixelPacket
      zero;

    ResampleFilter
      **restrict resample_filter;

    CacheView
      *distort_view;

    status=MagickTrue;
    progress=0;
    GetMagickPixelPacket(distort_image,&zero);
    resample_filter=AcquireResampleFilterThreadSet(image,
      UndefinedVirtualPixelMethod,MagickFalse,exception);
    distort_view=AcquireCacheView(distort_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
    for (j=0; j < (long) distort_image->rows; j++)
    {
      double
        validity;  /* how mathematically valid is this the mapping */

      MagickBooleanType
        sync;

      MagickPixelPacket
        pixel,    /* pixel color to assign to distorted image */
        invalid;  /* the color to assign when distort result is invalid */

      PointInfo
        d,
        s;  /* transform destination image x,y  to source image x,y */

      register IndexPacket
        *restrict indexes;

      register long
        i,
        id;

      register PixelPacket
        *restrict q;

      q=QueueCacheViewAuthenticPixels(distort_view,0,j,distort_image->columns,1,
        exception);
      if (q == (PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      indexes=GetCacheViewAuthenticIndexQueue(distort_view);
      pixel=zero;

      /* Define constant scaling vectors for Affine Distortions
        Other methods are either variable, or use interpolated lookup
      */
      id=GetOpenMPThreadId();
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

      GetMagickPixelPacket(distort_image,&invalid);
      SetMagickPixelPacket(distort_image,&distort_image->matte_color,
        (IndexPacket *) NULL, &invalid);
      if (distort_image->colorspace == CMYKColorspace)
        ConvertRGBToCMYK(&invalid);   /* what about other color spaces? */

      for (i=0; i < (long) distort_image->columns; i++)
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
              if ( abs_r < abs_c6 )
                validity = 0.5 - coeff[8]*r/coeff[6];
            }
            else if ( abs_r < abs_c7 )
              validity = 0.5 - coeff[8]*r/coeff[7];
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
            /* Handle Special degenerate (non-quadratic) case */
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
          case QuadrilateralDistortion:
            /* Bilinear mapping of any Quadrilateral to any Quadrilateral */
            /* UNDER DEVELOPMENT */
            break;
#endif
          case PolynomialDistortion:
          {
            /* multi-ordered polynomial */
            register long
              k;
            long
              nterms=(long)coeff[1];

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
            s.x  = (atan2(d.y,d.x) - coeff[0])/Magick2PI;
            s.x -= MagickRound(s.x);     /* angle */
            s.y  = hypot(d.x,d.y);       /* radius */

            /* Arc Distortion Partial Scaling Vectors
              Are derived by mapping the perpendicular unit vectors
              dR  and  dA*R*2PI  rather than trying to map dx and dy
              The results is a very simple orthogonal aligned ellipse.
            */
            if ( s.y > MagickEpsilon )
              ScaleFilter( resample_filter[id],
                  coeff[1]/(Magick2PI*s.y), 0, 0, coeff[3] );
            else
              ScaleFilter( resample_filter[id],
                  distort_image->columns*2, 0, 0, coeff[3] );

            /* now scale the angle and radius for source image lookup point */
            s.x = s.x*coeff[1] + coeff[4] + image->page.x +0.5;
            s.y = (coeff[2] - s.y) * coeff[3] + image->page.y;
            break;
          }
          case PolarDistortion:
          { /* Rect/Cartesain/Cylinder to Polar View */
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
                  coeff[6]/(Magick2PI*s.y), 0, 0, coeff[7] );
            else
              ScaleFilter( resample_filter[id],
                  distort_image->columns*2, 0, 0, coeff[7] );

            /* now finish mapping radius/angle to source x,y coords */
            s.x = s.x*coeff[6] + (double)image->columns/2.0 + image->page.x;
            s.y = (s.y-coeff[1])*coeff[7] + image->page.y;
            break;
          }
          case DePolarDistortion:
          { /* Polar to Cylindical  */
            /* ignore all destination virtual offsets */
            d.x = ((double)i+0.5)*output_scaling*coeff[6]-coeff[4];
            d.y = ((double)j+0.5)*output_scaling*coeff[7]+coeff[1];
            s.x = d.y*sin(d.x) + coeff[2];
            s.y = d.y*cos(d.x) + coeff[3];
            /* derivatives are usless - better to use SuperSampling */
            break;
          }
          case BarrelDistortion:
          case BarrelInverseDistortion:
          {
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
              /* Set source pixel and EWA derivative vectors */
              s.x = d.x*fx + coeff[8];
              s.y = d.y*fy + coeff[9];
              ScaleFilter( resample_filter[id],
                  gx*d.x*d.x + fx, gx*d.x*d.y,
                  gy*d.x*d.y,      gy*d.y*d.y + fy );
            }
            else { /* Special handling to avoid divide by zero when r=0 */
              s.x=s.y=0.0;
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
            */
            unsigned long
              i;
            double
              denominator;

            denominator = s.x = s.y = 0;
            for(i=0; i<number_arguments; i+=4) {
              double weight =
                  ((double)d.x-arguments[i+2])*((double)d.x-arguments[i+2])
                + ((double)d.y-arguments[i+3])*((double)d.y-arguments[i+3]);
              if ( weight != 0 )
                weight = 1/weight;
              else
                weight = 1;

              s.x += (arguments[ i ]-arguments[i+2])*weight;
              s.y += (arguments[i+1]-arguments[i+3])*weight;
              denominator += weight;
            }
            s.x /= denominator;
            s.y /= denominator;
            s.x += d.x;
            s.y += d.y;

            /* We can not determine derivatives using shepards method
               only color interpolatation, not area-resampling */
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
          SetPixelPacket(distort_image,&invalid,q,indexes);
        }
        else {
          /* resample the source image to find its correct color */
          (void) ResamplePixelColor(resample_filter[id],s.x,s.y,&pixel);
          /* if validity between 0.0 and 1.0 mix result with invalid pixel */
          if ( validity < 1.0 ) {
            /* Do a blend of sample color and invalid pixel */
            /* should this be a 'Blend', or an 'Over' compose */
            MagickPixelCompositeBlend(&pixel,validity,&invalid,(1.0-validity),
              &pixel);
          }
          SetPixelPacket(distort_image,&pixel,q,indexes);
        }
        q++;
        indexes++;
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
#if 0
fprintf(stderr, "\n");
#endif
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
  coeff = (double *) RelinquishMagickMemory(coeff);
  return(distort_image);
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
%      Image *SparseColorImage(const Image *image,const ChannelType channel,
%        const SparseColorMethod method,const unsigned long number_arguments,
%        const double *arguments,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to be filled in.
%
%    o channel: Specify which color values (in RGBKA sequence) are being set.
%        This also determines the number of color_values in above.
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
  const ChannelType channel,const SparseColorMethod method,
  const unsigned long number_arguments,const double *arguments,
  ExceptionInfo *exception)
{
#define SparseColorTag  "Distort/SparseColor"

  DistortImageMethod
    distort_method;

  double
    *coeff;

  Image
    *sparse_image;

  MagickPixelPacket
    zero;

  unsigned long
    number_colors;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  /* Determine number of color values needed per control point */
  number_colors=0;
  if ( channel & RedChannel     ) number_colors++;
  if ( channel & GreenChannel   ) number_colors++;
  if ( channel & BlueChannel    ) number_colors++;
  if ( channel & IndexChannel   ) number_colors++;
  if ( channel & OpacityChannel ) number_colors++;

  /*
    Convert input arguments into mapping coefficients to apply the distortion.
    Note some Methods may fall back to other simpler methods.
  */
  distort_method=(DistortImageMethod) method;
  coeff = GenerateCoefficients(image, &distort_method, number_arguments,
    arguments, number_colors, exception);
  if ( coeff == (double *) NULL )
    return((Image *) NULL);

  /* Verbose output */
  if ( GetImageArtifact(image,"verbose") != (const char *) NULL ) {

    switch (method) {
      case BarycentricColorInterpolate:
      {
        register long x=0;
        fprintf(stderr, "Barycentric Sparse Color:\n");
        if ( channel & RedChannel )
          fprintf(stderr, "  -channel R -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if ( channel & GreenChannel )
          fprintf(stderr, "  -channel G -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if ( channel & BlueChannel )
          fprintf(stderr, "  -channel B -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if ( channel & IndexChannel )
          fprintf(stderr, "  -channel K -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        if ( channel & OpacityChannel )
          fprintf(stderr, "  -channel A -fx '%+lf*i %+lf*j %+lf' \\\n",
              coeff[x], coeff[x+1], coeff[x+2]),x+=3;
        break;
      }
      case BilinearColorInterpolate:
      {
        register long x=0;
        fprintf(stderr, "Bilinear Sparse Color\n");
        if ( channel & RedChannel )
          fprintf(stderr, "   -channel R -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if ( channel & GreenChannel )
          fprintf(stderr, "   -channel G -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if ( channel & BlueChannel )
          fprintf(stderr, "   -channel B -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if ( channel & IndexChannel )
          fprintf(stderr, "   -channel K -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        if ( channel & OpacityChannel )
          fprintf(stderr, "   -channel A -fx '%+lf*i %+lf*j %+lf*i*j %+lf;\n",
              coeff[ x ], coeff[x+1],
              coeff[x+2], coeff[x+3]),x+=4;
        break;
      }
      default:
        /* unknown, or which are too complex for FX alturnatives */
        break;
    }
  }

  /* Generate new image for generated interpolated gradient.
   * ASIDE: Actually we could have just replaced the colors of the original
   * image, but IM core policy, is if storage class could change then clone
   * the image.
   */

  sparse_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (sparse_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(sparse_image,DirectClass) == MagickFalse)
    { /* if image is ColorMapped - change it to DirectClass */
      InheritException(exception,&image->exception);
      sparse_image=DestroyImage(sparse_image);
      return((Image *) NULL);
    }
  { /* ----- MAIN CODE ----- */
    long
      j,
      progress;

    MagickBooleanType
      status;

    CacheView
      *sparse_view;

    status=MagickTrue;
    progress=0;
    GetMagickPixelPacket(sparse_image,&zero);
    sparse_view=AcquireCacheView(sparse_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
    for (j=0; j < (long) sparse_image->rows; j++)
    {
      MagickBooleanType
        sync;

      MagickPixelPacket
        pixel;    /* pixel to assign to distorted image */

      register IndexPacket
        *restrict indexes;

      register long
        i;

      register PixelPacket
        *restrict q;

      q=QueueCacheViewAuthenticPixels(sparse_view,0,j,sparse_image->columns,
        1,exception);
      if (q == (PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
/* FUTURE: get pixel from source image - so channel can replace parts */
      indexes=GetCacheViewAuthenticIndexQueue(sparse_view);
      pixel=zero;
      for (i=0; i < (long) sparse_image->columns; i++)
      {
        switch (method)
        {
          case BarycentricColorInterpolate:
          {
            register long x=0;
            if ( channel & RedChannel )
              pixel.red     = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if ( channel & GreenChannel )
              pixel.green   = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if ( channel & BlueChannel )
              pixel.blue    = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if ( channel & IndexChannel )
              pixel.index   = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            if ( channel & OpacityChannel )
              pixel.opacity = coeff[x]*i +coeff[x+1]*j
                              +coeff[x+2], x+=3;
            break;
          }
          case BilinearColorInterpolate:
          {
            register long x=0;
            if ( channel & RedChannel )
              pixel.red     = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if ( channel & GreenChannel )
              pixel.green   = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if ( channel & BlueChannel )
              pixel.blue    = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if ( channel & IndexChannel )
              pixel.index   = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            if ( channel & OpacityChannel )
              pixel.opacity = coeff[x]*i     + coeff[x+1]*j +
                              coeff[x+2]*i*j + coeff[x+3], x+=4;
            break;
          }
          case ShepardsColorInterpolate:
          { /* Shepards Method,uses its own input arguments as coefficients.
            */
            unsigned long
              k;
            double
              denominator;

            if ( channel & RedChannel     ) pixel.red     = 0.0;
            if ( channel & GreenChannel   ) pixel.green   = 0.0;
            if ( channel & BlueChannel    ) pixel.blue    = 0.0;
            if ( channel & IndexChannel   ) pixel.index   = 0.0;
            if ( channel & OpacityChannel ) pixel.opacity = 0.0;
            denominator = 0.0;
            for(k=0; k<number_arguments; k+=2+number_colors) {
              register long x=(long) k+2;
              double weight =
                  ((double)i-arguments[ k ])*((double)i-arguments[ k ])
                + ((double)j-arguments[k+1])*((double)j-arguments[k+1]);
              if ( weight != 0 )
                weight = 1/weight;
              else
                weight = 1;
              if ( channel & RedChannel )
                pixel.red     += arguments[x++]*weight;
              if ( channel & GreenChannel )
                pixel.green   += arguments[x++]*weight;
              if ( channel & BlueChannel )
                pixel.blue    += arguments[x++]*weight;
              if ( channel & IndexChannel )
                pixel.index   += arguments[x++]*weight;
              if ( channel & OpacityChannel )
                pixel.opacity += arguments[x++]*weight;
              denominator += weight;
            }
            if ( channel & RedChannel     ) pixel.red     /= denominator;
            if ( channel & GreenChannel   ) pixel.green   /= denominator;
            if ( channel & BlueChannel    ) pixel.blue    /= denominator;
            if ( channel & IndexChannel   ) pixel.index   /= denominator;
            if ( channel & OpacityChannel ) pixel.opacity /= denominator;
            break;
          }
          case VoronoiColorInterpolate:
          default:
          { /* Just use the closest control point you can find! */
            unsigned long
              k;
            double
              minimum = MagickHuge;

            for(k=0; k<number_arguments; k+=2+number_colors) {
              double distance =
                  ((double)i-arguments[ k ])*((double)i-arguments[ k ])
                + ((double)j-arguments[k+1])*((double)j-arguments[k+1]);
              if ( distance < minimum ) {
                register long x=(long) k+2;
                if ( channel & RedChannel     ) pixel.red     = arguments[x++];
                if ( channel & GreenChannel   ) pixel.green   = arguments[x++];
                if ( channel & BlueChannel    ) pixel.blue    = arguments[x++];
                if ( channel & IndexChannel   ) pixel.index   = arguments[x++];
                if ( channel & OpacityChannel ) pixel.opacity = arguments[x++];
                minimum = distance;
              }
            }
            break;
          }
        }
        /* set the color directly back into the source image */
        if ( channel & RedChannel     ) pixel.red     *= QuantumRange;
        if ( channel & GreenChannel   ) pixel.green   *= QuantumRange;
        if ( channel & BlueChannel    ) pixel.blue    *= QuantumRange;
        if ( channel & IndexChannel   ) pixel.index   *= QuantumRange;
        if ( channel & OpacityChannel ) pixel.opacity *= QuantumRange;
        SetPixelPacket(sparse_image,&pixel,q,indexes);
        q++;
        indexes++;
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               SSSSS  EEEEE   GGGG  M   M  EEEEE  N   N  TTTTT               %
%               SS     E      G      MM MM  E      NN  N    T                 %
%                SSS   EEE    G GGG  M M M  EEE    N N N    T                 %
%                  SS  E      G   G  M   M  E      N  NN    T                 %
%               SSSSS  EEEEE   GGGG  M   M  EEEEE  N   N    T                 %
%                                                                             %
%                                                                             %
%    MagickCore Methods to Segment an Image with Thresholding Fuzzy c-Means   %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                April 1993                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
%  Segment segments an image by analyzing the histograms of the color
%  components and identifying units that are homogeneous with the fuzzy
%  c-means technique.  The scale-space filter analyzes the histograms of
%  the three color components of the image and identifies a set of
%  classes.  The extents of each class is used to coarsely segment the
%  image with thresholding.  The color associated with each class is
%  determined by the mean color of all pixels within the extents of a
%  particular class.  Finally, any unclassified pixels are assigned to
%  the closest class with the fuzzy c-means technique.
%
%  The fuzzy c-Means algorithm can be summarized as follows:
%
%    o Build a histogram, one for each color component of the image.
%
%    o For each histogram, successively apply the scale-space filter and
%      build an interval tree of zero crossings in the second derivative
%      at each scale.  Analyze this scale-space ''fingerprint'' to
%      determine which peaks and valleys in the histogram are most
%      predominant.
%
%    o The fingerprint defines intervals on the axis of the histogram.
%      Each interval contains either a minima or a maxima in the original
%      signal.  If each color component lies within the maxima interval,
%      that pixel is considered ''classified'' and is assigned an unique
%      class number.
%
%    o Any pixel that fails to be classified in the above thresholding
%      pass is classified using the fuzzy c-Means technique.  It is
%      assigned to one of the classes discovered in the histogram analysis
%      phase.
%
%  The fuzzy c-Means technique attempts to cluster a pixel by finding
%  the local minima of the generalized within group sum of squared error
%  objective function.  A pixel is assigned to the closest class of
%  which the fuzzy membership has a maximum value.
%
%  Segment is strongly based on software written by Andy Gallo,
%  University of Delaware.
%
%  The following reference was used in creating this program:
%
%    Young Won Lim, Sang Uk Lee, "On The Color Image Segmentation
%    Algorithm Based on the Thresholding and the Fuzzy c-Means
%    Techniques", Pattern Recognition, Volume 23, Number 9, pages
%    935-952, 1990.
%
%
*/

#include "MagickCore/studio.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"

/*
  Define declarations.
*/
#define MaxDimension  3
#define DeltaTau  0.5f
#if defined(FastClassify)
#define WeightingExponent  2.0
#define SegmentPower(ratio) (ratio)
#else
#define WeightingExponent  2.5
#define SegmentPower(ratio) pow(ratio,(double) (1.0/(weighting_exponent-1.0)));
#endif
#define Tau  5.2f

/*
  Typedef declarations.
*/
typedef struct _ExtentPacket
{
  double
    center;

  ssize_t
    index,
    left,
    right;
} ExtentPacket;

typedef struct _Cluster
{
  struct _Cluster
    *next;

  ExtentPacket
    red,
    green,
    blue;

  ssize_t
    count,
    id;
} Cluster;

typedef struct _IntervalTree
{
  double
    tau;

  ssize_t
    left,
    right;

  double
    mean_stability,
    stability;

  struct _IntervalTree
    *sibling,
    *child;
} IntervalTree;

typedef struct _ZeroCrossing
{
  double
    tau,
    histogram[256];

  short
    crossings[256];
} ZeroCrossing;

/*
  Constant declarations.
*/
static const int
  Blue = 2,
  Green = 1,
  Red = 0,
  SafeMargin = 3,
  TreeLength = 600;

/*
  Method prototypes.
*/
static double
  OptimalTau(const ssize_t *,const double,const double,const double,
    const double,short *);

static ssize_t
  DefineRegion(const short *,ExtentPacket *);

static void
  FreeNodes(IntervalTree *),
  InitializeHistogram(const Image *,ssize_t **,ExceptionInfo *),
  ScaleSpace(const ssize_t *,const double,double *),
  ZeroCrossHistogram(double *,const double,short *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l a s s i f y                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Classify() defines one or more classes.  Each pixel is thresholded to
%  determine which class it belongs to.  If the class is not identified it is
%  assigned to the closest class based on the fuzzy c-Means technique.
%
%  The format of the Classify method is:
%
%      MagickBooleanType Classify(Image *image,short **extrema,
%        const double cluster_threshold,
%        const double weighting_exponent,
%        const MagickBooleanType verbose,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o extrema:  Specifies a pointer to an array of integers.  They
%      represent the peaks and valleys of the histogram for each color
%      component.
%
%    o cluster_threshold:  This double represents the minimum number of
%      pixels contained in a hexahedra before it can be considered valid
%      (expressed as a percentage).
%
%    o weighting_exponent: Specifies the membership weighting exponent.
%
%    o verbose:  A value greater than zero prints detailed information about
%      the identified classes.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType Classify(Image *image,short **extrema,
  const double cluster_threshold,
  const double weighting_exponent,const MagickBooleanType verbose,
  ExceptionInfo *exception)
{
#define SegmentImageTag  "Segment/Image"
#define ThrowClassifyException(severity,tag,label) \
{\
  for (cluster=head; cluster != (Cluster *) NULL; cluster=next_cluster) \
  { \
    next_cluster=cluster->next; \
    cluster=(Cluster *) RelinquishMagickMemory(cluster); \
  } \
  if (squares != (double *) NULL) \
    { \
      squares-=255; \
      free_squares=squares; \
      free_squares=(double *) RelinquishMagickMemory(free_squares); \
    } \
  ThrowBinaryException(severity,tag,label); \
}

  CacheView
    *image_view;

  Cluster
    *cluster,
    *head,
    *last_cluster,
    *next_cluster;

  ExtentPacket
    blue,
    green,
    red;

  MagickOffsetType
    progress;

  double
    *free_squares;

  MagickStatusType
    status;

  register ssize_t
    i;

  register double
    *squares;

  size_t
    number_clusters;

  ssize_t
    count,
    y;

  /*
    Form clusters.
  */
  cluster=(Cluster *) NULL;
  head=(Cluster *) NULL;
  squares=(double *) NULL;
  (void) memset(&red,0,sizeof(red));
  (void) memset(&green,0,sizeof(green));
  (void) memset(&blue,0,sizeof(blue));
  while (DefineRegion(extrema[Red],&red) != 0)
  {
    green.index=0;
    while (DefineRegion(extrema[Green],&green) != 0)
    {
      blue.index=0;
      while (DefineRegion(extrema[Blue],&blue) != 0)
      {
        /*
          Allocate a new class.
        */
        if (head != (Cluster *) NULL)
          {
            cluster->next=(Cluster *) AcquireMagickMemory(
              sizeof(*cluster->next));
            cluster=cluster->next;
          }
        else
          {
            cluster=(Cluster *) AcquireMagickMemory(sizeof(*cluster));
            head=cluster;
          }
        if (cluster == (Cluster *) NULL)
          ThrowClassifyException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);
        /*
          Initialize a new class.
        */
        cluster->count=0;
        cluster->red=red;
        cluster->green=green;
        cluster->blue=blue;
        cluster->next=(Cluster *) NULL;
      }
    }
  }
  if (head == (Cluster *) NULL)
    {
      /*
        No classes were identified-- create one.
      */
      cluster=(Cluster *) AcquireMagickMemory(sizeof(*cluster));
      if (cluster == (Cluster *) NULL)
        ThrowClassifyException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      /*
        Initialize a new class.
      */
      cluster->count=0;
      cluster->red=red;
      cluster->green=green;
      cluster->blue=blue;
      cluster->next=(Cluster *) NULL;
      head=cluster;
    }
  /*
    Count the pixels for each cluster.
  */
  status=MagickTrue;
  count=0;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *p;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
        if (((ssize_t) ScaleQuantumToChar(GetPixelRed(image,p)) >=
             (cluster->red.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelRed(image,p)) <=
             (cluster->red.right+SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelGreen(image,p)) >=
             (cluster->green.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelGreen(image,p)) <=
             (cluster->green.right+SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelBlue(image,p)) >=
             (cluster->blue.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelBlue(image,p)) <=
             (cluster->blue.right+SafeMargin)))
          {
            /*
              Count this pixel.
            */
            count++;
            cluster->red.center+=(double) ScaleQuantumToChar(
              GetPixelRed(image,p));
            cluster->green.center+=(double) ScaleQuantumToChar(
              GetPixelGreen(image,p));
            cluster->blue.center+=(double) ScaleQuantumToChar(
              GetPixelBlue(image,p));
            cluster->count++;
            break;
          }
      p+=GetPixelChannels(image);
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SegmentImageTag,progress,2*image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  /*
    Remove clusters that do not meet minimum cluster threshold.
  */
  count=0;
  last_cluster=head;
  next_cluster=head;
  for (cluster=head; cluster != (Cluster *) NULL; cluster=next_cluster)
  {
    next_cluster=cluster->next;
    if ((cluster->count > 0) &&
        (cluster->count >= (count*cluster_threshold/100.0)))
      {
        /*
          Initialize cluster.
        */
        cluster->id=count;
        cluster->red.center/=cluster->count;
        cluster->green.center/=cluster->count;
        cluster->blue.center/=cluster->count;
        count++;
        last_cluster=cluster;
        continue;
      }
    /*
      Delete cluster.
    */
    if (cluster == head)
      head=next_cluster;
    else
      last_cluster->next=next_cluster;
    cluster=(Cluster *) RelinquishMagickMemory(cluster);
  }
  number_clusters=(size_t) count;
  if (verbose != MagickFalse)
    {
      /*
        Print cluster statistics.
      */
      (void) FormatLocaleFile(stdout,"Fuzzy C-means Statistics\n");
      (void) FormatLocaleFile(stdout,"===================\n\n");
      (void) FormatLocaleFile(stdout,"\tCluster Threshold = %g\n",(double)
        cluster_threshold);
      (void) FormatLocaleFile(stdout,"\tWeighting Exponent = %g\n",(double)
        weighting_exponent);
      (void) FormatLocaleFile(stdout,"\tTotal Number of Clusters = %.20g\n\n",
        (double) number_clusters);
      /*
        Print the total number of points per cluster.
      */
      (void) FormatLocaleFile(stdout,"\n\nNumber of Vectors Per Cluster\n");
      (void) FormatLocaleFile(stdout,"=============================\n\n");
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
        (void) FormatLocaleFile(stdout,"Cluster #%.20g = %.20g\n",(double)
          cluster->id,(double) cluster->count);
      /*
        Print the cluster extents.
      */
      (void) FormatLocaleFile(stdout,
        "\n\n\nCluster Extents:        (Vector Size: %d)\n",MaxDimension);
      (void) FormatLocaleFile(stdout,"================");
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
      {
        (void) FormatLocaleFile(stdout,"\n\nCluster #%.20g\n\n",(double)
          cluster->id);
        (void) FormatLocaleFile(stdout,
          "%.20g-%.20g  %.20g-%.20g  %.20g-%.20g\n",(double)
          cluster->red.left,(double) cluster->red.right,(double)
          cluster->green.left,(double) cluster->green.right,(double)
          cluster->blue.left,(double) cluster->blue.right);
      }
      /*
        Print the cluster center values.
      */
      (void) FormatLocaleFile(stdout,
        "\n\n\nCluster Center Values:        (Vector Size: %d)\n",MaxDimension);
      (void) FormatLocaleFile(stdout,"=====================");
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
      {
        (void) FormatLocaleFile(stdout,"\n\nCluster #%.20g\n\n",(double)
          cluster->id);
        (void) FormatLocaleFile(stdout,"%g  %g  %g\n",(double)
          cluster->red.center,(double) cluster->green.center,(double)
          cluster->blue.center);
      }
      (void) FormatLocaleFile(stdout,"\n");
    }
  if (number_clusters > 256)
    ThrowClassifyException(ImageError,"TooManyClusters",image->filename);
  /*
    Speed up distance calculations.
  */
  squares=(double *) AcquireQuantumMemory(513UL,sizeof(*squares));
  if (squares == (double *) NULL)
    ThrowClassifyException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  squares+=255;
  for (i=(-255); i <= 255; i++)
    squares[i]=(double) i*(double) i;
  /*
    Allocate image colormap.
  */
  if (AcquireImageColormap(image,number_clusters,exception) == MagickFalse)
    ThrowClassifyException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  i=0;
  for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
  {
    image->colormap[i].red=(double) ScaleCharToQuantum((unsigned char)
      (cluster->red.center+0.5));
    image->colormap[i].green=(double) ScaleCharToQuantum((unsigned char)
      (cluster->green.center+0.5));
    image->colormap[i].blue=(double) ScaleCharToQuantum((unsigned char)
      (cluster->blue.center+0.5));
    i++;
  }
  /*
    Do course grain classes.
  */
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Cluster
      *clust;

    register const PixelInfo
      *magick_restrict p;

    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelIndex(image,(Quantum) 0,q);
      for (clust=head; clust != (Cluster *) NULL; clust=clust->next)
      {
        if (((ssize_t) ScaleQuantumToChar(GetPixelRed(image,q)) >=
             (clust->red.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelRed(image,q)) <=
             (clust->red.right+SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelGreen(image,q)) >=
             (clust->green.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelGreen(image,q)) <=
             (clust->green.right+SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelBlue(image,q)) >=
             (clust->blue.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelBlue(image,q)) <=
             (clust->blue.right+SafeMargin)))
          {
            /*
              Classify this pixel.
            */
            SetPixelIndex(image,(Quantum) clust->id,q);
            break;
          }
      }
      if (clust == (Cluster *) NULL)
        {
          double
            distance_squared,
            local_minima,
            numerator,
            ratio,
            sum;

          register ssize_t
            j,
            k;

          /*
            Compute fuzzy membership.
          */
          local_minima=0.0;
          for (j=0; j < (ssize_t) image->colors; j++)
          {
            sum=0.0;
            p=image->colormap+j;
            distance_squared=squares[(ssize_t) ScaleQuantumToChar(
              GetPixelRed(image,q))-(ssize_t)
              ScaleQuantumToChar(ClampToQuantum(p->red))]+squares[(ssize_t)
              ScaleQuantumToChar(GetPixelGreen(image,q))-(ssize_t)
              ScaleQuantumToChar(ClampToQuantum(p->green))]+squares[(ssize_t)
              ScaleQuantumToChar(GetPixelBlue(image,q))-(ssize_t)
              ScaleQuantumToChar(ClampToQuantum(p->blue))];
            numerator=distance_squared;
            for (k=0; k < (ssize_t) image->colors; k++)
            {
              p=image->colormap+k;
                distance_squared=squares[(ssize_t) ScaleQuantumToChar(
                  GetPixelRed(image,q))-(ssize_t)
                  ScaleQuantumToChar(ClampToQuantum(p->red))]+squares[
                  (ssize_t) ScaleQuantumToChar(GetPixelGreen(image,q))-(ssize_t)
                  ScaleQuantumToChar(ClampToQuantum(p->green))]+squares[
                  (ssize_t) ScaleQuantumToChar(GetPixelBlue(image,q))-(ssize_t)
                  ScaleQuantumToChar(ClampToQuantum(p->blue))];
              ratio=numerator/distance_squared;
              sum+=SegmentPower(ratio);
            }
            if ((sum != 0.0) && ((1.0/sum) > local_minima))
              {
                /*
                  Classify this pixel.
                */
                local_minima=1.0/sum;
                SetPixelIndex(image,(Quantum) j,q);
              }
          }
        }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SegmentImageTag,progress,2*image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  status&=SyncImage(image,exception);
  /*
    Relinquish resources.
  */
  for (cluster=head; cluster != (Cluster *) NULL; cluster=next_cluster)
  {
    next_cluster=cluster->next;
    cluster=(Cluster *) RelinquishMagickMemory(cluster);
  }
  squares-=255;
  free_squares=squares;
  free_squares=(double *) RelinquishMagickMemory(free_squares);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n s o l i d a t e C r o s s i n g s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConsolidateCrossings() guarantees that an even number of zero crossings
%  always lie between two crossings.
%
%  The format of the ConsolidateCrossings method is:
%
%      ConsolidateCrossings(ZeroCrossing *zero_crossing,
%        const size_t number_crossings)
%
%  A description of each parameter follows.
%
%    o zero_crossing: Specifies an array of structures of type ZeroCrossing.
%
%    o number_crossings: This size_t specifies the number of elements
%      in the zero_crossing array.
%
*/
static void ConsolidateCrossings(ZeroCrossing *zero_crossing,
  const size_t number_crossings)
{
  register ssize_t
    i,
    j,
    k,
    l;

  ssize_t
    center,
    correct,
    count,
    left,
    right;

  /*
    Consolidate zero crossings.
  */
  for (i=(ssize_t) number_crossings-1; i >= 0; i--)
    for (j=0; j <= 255; j++)
    {
      if (zero_crossing[i].crossings[j] == 0)
        continue;
      /*
        Find the entry that is closest to j and still preserves the
        property that there are an even number of crossings between
        intervals.
      */
      for (k=j-1; k > 0; k--)
        if (zero_crossing[i+1].crossings[k] != 0)
          break;
      left=MagickMax(k,0);
      center=j;
      for (k=j+1; k < 255; k++)
        if (zero_crossing[i+1].crossings[k] != 0)
          break;
      right=MagickMin(k,255);
      /*
        K is the zero crossing just left of j.
      */
      for (k=j-1; k > 0; k--)
        if (zero_crossing[i].crossings[k] != 0)
          break;
      if (k < 0)
        k=0;
      /*
        Check center for an even number of crossings between k and j.
      */
      correct=(-1);
      if (zero_crossing[i+1].crossings[j] != 0)
        {
          count=0;
          for (l=k+1; l < center; l++)
            if (zero_crossing[i+1].crossings[l] != 0)
              count++;
          if (((count % 2) == 0) && (center != k))
            correct=center;
        }
      /*
        Check left for an even number of crossings between k and j.
      */
      if (correct == -1)
        {
          count=0;
          for (l=k+1; l < left; l++)
            if (zero_crossing[i+1].crossings[l] != 0)
              count++;
          if (((count % 2) == 0) && (left != k))
            correct=left;
        }
      /*
        Check right for an even number of crossings between k and j.
      */
      if (correct == -1)
        {
          count=0;
          for (l=k+1; l < right; l++)
            if (zero_crossing[i+1].crossings[l] != 0)
              count++;
          if (((count % 2) == 0) && (right != k))
            correct=right;
        }
      l=(ssize_t) zero_crossing[i].crossings[j];
      zero_crossing[i].crossings[j]=0;
      if (correct != -1)
        zero_crossing[i].crossings[correct]=(short) l;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f i n e R e g i o n                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefineRegion() defines the left and right boundaries of a peak region.
%
%  The format of the DefineRegion method is:
%
%      ssize_t DefineRegion(const short *extrema,ExtentPacket *extents)
%
%  A description of each parameter follows.
%
%    o extrema:  Specifies a pointer to an array of integers.  They
%      represent the peaks and valleys of the histogram for each color
%      component.
%
%    o extents:  This pointer to an ExtentPacket represent the extends
%      of a particular peak or valley of a color component.
%
*/
static ssize_t DefineRegion(const short *extrema,ExtentPacket *extents)
{
  /*
    Initialize to default values.
  */
  extents->left=0;
  extents->center=0.0;
  extents->right=255;
  /*
    Find the left side (maxima).
  */
  for ( ; extents->index <= 255; extents->index++)
    if (extrema[extents->index] > 0)
      break;
  if (extents->index > 255)
    return(MagickFalse);  /* no left side - no region exists */
  extents->left=extents->index;
  /*
    Find the right side (minima).
  */
  for ( ; extents->index <= 255; extents->index++)
    if (extrema[extents->index] < 0)
      break;
  extents->right=extents->index-1;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e r i v a t i v e H i s t o g r a m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DerivativeHistogram() determines the derivative of the histogram using
%  central differencing.
%
%  The format of the DerivativeHistogram method is:
%
%      DerivativeHistogram(const double *histogram,
%        double *derivative)
%
%  A description of each parameter follows.
%
%    o histogram: Specifies an array of doubles representing the number
%      of pixels for each intensity of a particular color component.
%
%    o derivative: This array of doubles is initialized by
%      DerivativeHistogram to the derivative of the histogram using central
%      differencing.
%
*/
static void DerivativeHistogram(const double *histogram,
  double *derivative)
{
  register ssize_t
    i,
    n;

  /*
    Compute endpoints using second order polynomial interpolation.
  */
  n=255;
  derivative[0]=(-1.5*histogram[0]+2.0*histogram[1]-0.5*histogram[2]);
  derivative[n]=(0.5*histogram[n-2]-2.0*histogram[n-1]+1.5*histogram[n]);
  /*
    Compute derivative using central differencing.
  */
  for (i=1; i < n; i++)
    derivative[i]=(histogram[i+1]-histogram[i-1])/2.0;
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t I m a g e D y n a m i c T h r e s h o l d                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageDynamicThreshold() returns the dynamic threshold for an image.
%
%  The format of the GetImageDynamicThreshold method is:
%
%      MagickBooleanType GetImageDynamicThreshold(const Image *image,
%        const double cluster_threshold,const double smooth_threshold,
%        PixelInfo *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o cluster_threshold:  This double represents the minimum number of
%      pixels contained in a hexahedra before it can be considered valid
%      (expressed as a percentage).
%
%    o smooth_threshold: the smoothing threshold eliminates noise in the second
%      derivative of the histogram.  As the value is increased, you can expect a
%      smoother second derivative.
%
%    o pixel: return the dynamic threshold here.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetImageDynamicThreshold(const Image *image,
  const double cluster_threshold,const double smooth_threshold,
  PixelInfo *pixel,ExceptionInfo *exception)
{
  Cluster
    *background,
    *cluster,
    *object,
    *head,
    *last_cluster,
    *next_cluster;

  ExtentPacket
    blue,
    green,
    red;

  MagickBooleanType
    proceed;

  double
    threshold;

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  short
    *extrema[MaxDimension];

  ssize_t
    count,
    *histogram[MaxDimension],
    y;

  /*
    Allocate histogram and extrema.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  GetPixelInfo(image,pixel);
  for (i=0; i < MaxDimension; i++)
  {
    histogram[i]=(ssize_t *) AcquireQuantumMemory(256UL,sizeof(**histogram));
    extrema[i]=(short *) AcquireQuantumMemory(256UL,sizeof(**histogram));
    if ((histogram[i] == (ssize_t *) NULL) || (extrema[i] == (short *) NULL))
      {
        for (i-- ; i >= 0; i--)
        {
          extrema[i]=(short *) RelinquishMagickMemory(extrema[i]);
          histogram[i]=(ssize_t *) RelinquishMagickMemory(histogram[i]);
        }
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
        return(MagickFalse);
      }
  }
  /*
    Initialize histogram.
  */
  InitializeHistogram(image,histogram,exception);
  (void) OptimalTau(histogram[Red],Tau,0.2f,DeltaTau,
    (smooth_threshold == 0.0f ? 1.0f : smooth_threshold),extrema[Red]);
  (void) OptimalTau(histogram[Green],Tau,0.2f,DeltaTau,
    (smooth_threshold == 0.0f ? 1.0f : smooth_threshold),extrema[Green]);
  (void) OptimalTau(histogram[Blue],Tau,0.2f,DeltaTau,
    (smooth_threshold == 0.0f ? 1.0f : smooth_threshold),extrema[Blue]);
  /*
    Form clusters.
  */
  cluster=(Cluster *) NULL;
  head=(Cluster *) NULL;
  (void) memset(&red,0,sizeof(red));
  (void) memset(&green,0,sizeof(green));
  (void) memset(&blue,0,sizeof(blue));
  while (DefineRegion(extrema[Red],&red) != 0)
  {
    green.index=0;
    while (DefineRegion(extrema[Green],&green) != 0)
    {
      blue.index=0;
      while (DefineRegion(extrema[Blue],&blue) != 0)
      {
        /*
          Allocate a new class.
        */
        if (head != (Cluster *) NULL)
          {
            cluster->next=(Cluster *) AcquireMagickMemory(
              sizeof(*cluster->next));
            cluster=cluster->next;
          }
        else
          {
            cluster=(Cluster *) AcquireMagickMemory(sizeof(*cluster));
            head=cluster;
          }
        if (cluster == (Cluster *) NULL)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'",
              image->filename);
            return(MagickFalse);
          }
        /*
          Initialize a new class.
        */
        cluster->count=0;
        cluster->red=red;
        cluster->green=green;
        cluster->blue=blue;
        cluster->next=(Cluster *) NULL;
      }
    }
  }
  if (head == (Cluster *) NULL)
    {
      /*
        No classes were identified-- create one.
      */
      cluster=(Cluster *) AcquireMagickMemory(sizeof(*cluster));
      if (cluster == (Cluster *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
          return(MagickFalse);
        }
      /*
        Initialize a new class.
      */
      cluster->count=0;
      cluster->red=red;
      cluster->green=green;
      cluster->blue=blue;
      cluster->next=(Cluster *) NULL;
      head=cluster;
    }
  /*
    Count the pixels for each cluster.
  */
  count=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
        if (((ssize_t) ScaleQuantumToChar(GetPixelRed(image,p)) >=
             (cluster->red.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelRed(image,p)) <=
             (cluster->red.right+SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelGreen(image,p)) >=
             (cluster->green.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelGreen(image,p)) <=
             (cluster->green.right+SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelBlue(image,p)) >=
             (cluster->blue.left-SafeMargin)) &&
            ((ssize_t) ScaleQuantumToChar(GetPixelBlue(image,p)) <=
             (cluster->blue.right+SafeMargin)))
          {
            /*
              Count this pixel.
            */
            count++;
            cluster->red.center+=(double) ScaleQuantumToChar(
              GetPixelRed(image,p));
            cluster->green.center+=(double) ScaleQuantumToChar(
              GetPixelGreen(image,p));
            cluster->blue.center+=(double) ScaleQuantumToChar(
              GetPixelBlue(image,p));
            cluster->count++;
            break;
          }
      p+=GetPixelChannels(image);
    }
    proceed=SetImageProgress(image,SegmentImageTag,(MagickOffsetType) y,
      2*image->rows);
    if (proceed == MagickFalse)
      break;
  }
  /*
    Remove clusters that do not meet minimum cluster threshold.
  */
  count=0;
  last_cluster=head;
  next_cluster=head;
  for (cluster=head; cluster != (Cluster *) NULL; cluster=next_cluster)
  {
    next_cluster=cluster->next;
    if ((cluster->count > 0) &&
        (cluster->count >= (count*cluster_threshold/100.0)))
      {
        /*
          Initialize cluster.
        */
        cluster->id=count;
        cluster->red.center/=cluster->count;
        cluster->green.center/=cluster->count;
        cluster->blue.center/=cluster->count;
        count++;
        last_cluster=cluster;
        continue;
      }
    /*
      Delete cluster.
    */
    if (cluster == head)
      head=next_cluster;
    else
      last_cluster->next=next_cluster;
    cluster=(Cluster *) RelinquishMagickMemory(cluster);
  }
  object=head;
  background=head;
  if (count > 1)
    {
      object=head->next;
      for (cluster=object; cluster->next != (Cluster *) NULL; )
      {
        if (cluster->count < object->count)
          object=cluster;
        cluster=cluster->next;
      }
      background=head->next;
      for (cluster=background; cluster->next != (Cluster *) NULL; )
      {
        if (cluster->count > background->count)
          background=cluster;
        cluster=cluster->next;
      }
    }
  if (background != (Cluster *) NULL)
    {
      threshold=(background->red.center+object->red.center)/2.0;
      pixel->red=(double) ScaleCharToQuantum((unsigned char)
        (threshold+0.5));
      threshold=(background->green.center+object->green.center)/2.0;
      pixel->green=(double) ScaleCharToQuantum((unsigned char)
        (threshold+0.5));
      threshold=(background->blue.center+object->blue.center)/2.0;
      pixel->blue=(double) ScaleCharToQuantum((unsigned char)
        (threshold+0.5));
    }
  /*
    Relinquish resources.
  */
  for (cluster=head; cluster != (Cluster *) NULL; cluster=next_cluster)
  {
    next_cluster=cluster->next;
    cluster=(Cluster *) RelinquishMagickMemory(cluster);
  }
  for (i=0; i < MaxDimension; i++)
  {
    extrema[i]=(short *) RelinquishMagickMemory(extrema[i]);
    histogram[i]=(ssize_t *) RelinquishMagickMemory(histogram[i]);
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  I n i t i a l i z e H i s t o g r a m                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeHistogram() computes the histogram for an image.
%
%  The format of the InitializeHistogram method is:
%
%      InitializeHistogram(const Image *image,ssize_t **histogram)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%    o histogram: Specifies an array of integers representing the number
%      of pixels for each intensity of a particular color component.
%
*/
static void InitializeHistogram(const Image *image,ssize_t **histogram,
  ExceptionInfo *exception)
{
  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  /*
    Initialize histogram.
  */
  for (i=0; i <= 255; i++)
  {
    histogram[Red][i]=0;
    histogram[Green][i]=0;
    histogram[Blue][i]=0;
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      histogram[Red][(ssize_t) ScaleQuantumToChar(GetPixelRed(image,p))]++;
      histogram[Green][(ssize_t) ScaleQuantumToChar(GetPixelGreen(image,p))]++;
      histogram[Blue][(ssize_t) ScaleQuantumToChar(GetPixelBlue(image,p))]++;
      p+=GetPixelChannels(image);
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e I n t e r v a l T r e e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeIntervalTree() initializes an interval tree from the lists of
%  zero crossings.
%
%  The format of the InitializeIntervalTree method is:
%
%      InitializeIntervalTree(IntervalTree **list,ssize_t *number_nodes,
%        IntervalTree *node)
%
%  A description of each parameter follows.
%
%    o zero_crossing: Specifies an array of structures of type ZeroCrossing.
%
%    o number_crossings: This size_t specifies the number of elements
%      in the zero_crossing array.
%
*/

static void InitializeList(IntervalTree **list,ssize_t *number_nodes,
  IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  if (node->child == (IntervalTree *) NULL)
    list[(*number_nodes)++]=node;
  InitializeList(list,number_nodes,node->sibling);
  InitializeList(list,number_nodes,node->child);
}

static void MeanStability(IntervalTree *node)
{
  register IntervalTree
    *child;

  if (node == (IntervalTree *) NULL)
    return;
  node->mean_stability=0.0;
  child=node->child;
  if (child != (IntervalTree *) NULL)
    {
      register ssize_t
        count;

      register double
        sum;

      sum=0.0;
      count=0;
      for ( ; child != (IntervalTree *) NULL; child=child->sibling)
      {
        sum+=child->stability;
        count++;
      }
      node->mean_stability=sum/(double) count;
    }
  MeanStability(node->sibling);
  MeanStability(node->child);
}

static void Stability(IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  if (node->child == (IntervalTree *) NULL)
    node->stability=0.0;
  else
    node->stability=node->tau-(node->child)->tau;
  Stability(node->sibling);
  Stability(node->child);
}

static IntervalTree *InitializeIntervalTree(const ZeroCrossing *zero_crossing,
  const size_t number_crossings)
{
  IntervalTree
    *head,
    **list,
    *node,
    *root;

  register ssize_t
    i;

  ssize_t
    j,
    k,
    left,
    number_nodes;

  /*
    Allocate interval tree.
  */
  list=(IntervalTree **) AcquireQuantumMemory((size_t) TreeLength,
    sizeof(*list));
  if (list == (IntervalTree **) NULL)
    return((IntervalTree *) NULL);
  /*
    The root is the entire histogram.
  */
  root=(IntervalTree *) AcquireCriticalMemory(sizeof(*root));
  root->child=(IntervalTree *) NULL;
  root->sibling=(IntervalTree *) NULL;
  root->tau=0.0;
  root->left=0;
  root->right=255;
  root->mean_stability=0.0;
  root->stability=0.0;
  (void) memset(list,0,TreeLength*sizeof(*list));
  for (i=(-1); i < (ssize_t) number_crossings; i++)
  {
    /*
      Initialize list with all nodes with no children.
    */
    number_nodes=0;
    InitializeList(list,&number_nodes,root);
    /*
      Split list.
    */
    for (j=0; j < number_nodes; j++)
    {
      head=list[j];
      left=head->left;
      node=head;
      for (k=head->left+1; k < head->right; k++)
      {
        if (zero_crossing[i+1].crossings[k] != 0)
          {
            if (node == head)
              {
                node->child=(IntervalTree *) AcquireMagickMemory(
                  sizeof(*node->child));
                node=node->child;
              }
            else
              {
                node->sibling=(IntervalTree *) AcquireMagickMemory(
                  sizeof(*node->sibling));
                node=node->sibling;
              }
            if (node == (IntervalTree *) NULL)
              {
                list=(IntervalTree **) RelinquishMagickMemory(list);
                FreeNodes(root);
                return((IntervalTree *) NULL);
              }
            node->tau=zero_crossing[i+1].tau;
            node->child=(IntervalTree *) NULL;
            node->sibling=(IntervalTree *) NULL;
            node->left=left;
            node->right=k;
            left=k;
          }
        }
      if (left != head->left)
        {
          node->sibling=(IntervalTree *) AcquireMagickMemory(
            sizeof(*node->sibling));
          node=node->sibling;
          if (node == (IntervalTree *) NULL)
            {
              list=(IntervalTree **) RelinquishMagickMemory(list);
              FreeNodes(root);
              return((IntervalTree *) NULL);
            }
          node->tau=zero_crossing[i+1].tau;
          node->child=(IntervalTree *) NULL;
          node->sibling=(IntervalTree *) NULL;
          node->left=left;
          node->right=head->right;
        }
    }
  }
  /*
    Determine the stability: difference between a nodes tau and its child.
  */
  Stability(root->child);
  MeanStability(root->child);
  list=(IntervalTree **) RelinquishMagickMemory(list);
  return(root);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p t i m a l T a u                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OptimalTau() finds the optimal tau for each band of the histogram.
%
%  The format of the OptimalTau method is:
%
%    double OptimalTau(const ssize_t *histogram,const double max_tau,
%      const double min_tau,const double delta_tau,
%      const double smooth_threshold,short *extrema)
%
%  A description of each parameter follows.
%
%    o histogram: Specifies an array of integers representing the number
%      of pixels for each intensity of a particular color component.
%
%    o extrema:  Specifies a pointer to an array of integers.  They
%      represent the peaks and valleys of the histogram for each color
%      component.
%
*/

static void ActiveNodes(IntervalTree **list,ssize_t *number_nodes,
  IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  if (node->stability >= node->mean_stability)
    {
      list[(*number_nodes)++]=node;
      ActiveNodes(list,number_nodes,node->sibling);
    }
  else
    {
      ActiveNodes(list,number_nodes,node->sibling);
      ActiveNodes(list,number_nodes,node->child);
    }
}

static void FreeNodes(IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  FreeNodes(node->sibling);
  FreeNodes(node->child);
  node=(IntervalTree *) RelinquishMagickMemory(node);
}

static double OptimalTau(const ssize_t *histogram,const double max_tau,
  const double min_tau,const double delta_tau,const double smooth_threshold,
  short *extrema)
{
  IntervalTree
    **list,
    *node,
    *root;

  MagickBooleanType
    peak;

  double
    average_tau,
    *derivative,
    *second_derivative,
    tau,
    value;

  register ssize_t
    i,
    x;

  size_t
    count,
    number_crossings;

  ssize_t
    index,
    j,
    k,
    number_nodes;

  ZeroCrossing
    *zero_crossing;

  /*
    Allocate interval tree.
  */
  list=(IntervalTree **) AcquireQuantumMemory((size_t) TreeLength,
    sizeof(*list));
  if (list == (IntervalTree **) NULL)
    return(0.0);
  /*
    Allocate zero crossing list.
  */
  count=(size_t) ((max_tau-min_tau)/delta_tau)+2;
  zero_crossing=(ZeroCrossing *) AcquireQuantumMemory((size_t) count,
    sizeof(*zero_crossing));
  if (zero_crossing == (ZeroCrossing *) NULL)
    {
      list=(IntervalTree **) RelinquishMagickMemory(list);
      return(0.0);
    }
  for (i=0; i < (ssize_t) count; i++)
    zero_crossing[i].tau=(-1.0);
  /*
    Initialize zero crossing list.
  */
  derivative=(double *) AcquireCriticalMemory(256*sizeof(*derivative));
  second_derivative=(double *) AcquireCriticalMemory(256*
    sizeof(*second_derivative));
  i=0;
  for (tau=max_tau; tau >= min_tau; tau-=delta_tau)
  {
    zero_crossing[i].tau=tau;
    ScaleSpace(histogram,tau,zero_crossing[i].histogram);
    DerivativeHistogram(zero_crossing[i].histogram,derivative);
    DerivativeHistogram(derivative,second_derivative);
    ZeroCrossHistogram(second_derivative,smooth_threshold,
      zero_crossing[i].crossings);
    i++;
  }
  /*
    Add an entry for the original histogram.
  */
  zero_crossing[i].tau=0.0;
  for (j=0; j <= 255; j++)
    zero_crossing[i].histogram[j]=(double) histogram[j];
  DerivativeHistogram(zero_crossing[i].histogram,derivative);
  DerivativeHistogram(derivative,second_derivative);
  ZeroCrossHistogram(second_derivative,smooth_threshold,
    zero_crossing[i].crossings);
  number_crossings=(size_t) i;
  derivative=(double *) RelinquishMagickMemory(derivative);
  second_derivative=(double *) RelinquishMagickMemory(second_derivative);
  /*
    Ensure the scale-space fingerprints form lines in scale-space, not loops.
  */
  ConsolidateCrossings(zero_crossing,number_crossings);
  /*
    Force endpoints to be included in the interval.
  */
  for (i=0; i <= (ssize_t) number_crossings; i++)
  {
    for (j=0; j < 255; j++)
      if (zero_crossing[i].crossings[j] != 0)
        break;
    zero_crossing[i].crossings[0]=(-zero_crossing[i].crossings[j]);
    for (j=255; j > 0; j--)
      if (zero_crossing[i].crossings[j] != 0)
        break;
    zero_crossing[i].crossings[255]=(-zero_crossing[i].crossings[j]);
  }
  /*
    Initialize interval tree.
  */
  root=InitializeIntervalTree(zero_crossing,number_crossings);
  if (root == (IntervalTree *) NULL)
    {
      zero_crossing=(ZeroCrossing *) RelinquishMagickMemory(zero_crossing);
      list=(IntervalTree **) RelinquishMagickMemory(list);
      return(0.0);
    }
  /*
    Find active nodes:  stability is greater (or equal) to the mean stability of
    its children.
  */
  number_nodes=0;
  ActiveNodes(list,&number_nodes,root->child);
  /*
    Initialize extrema.
  */
  for (i=0; i <= 255; i++)
    extrema[i]=0;
  for (i=0; i < number_nodes; i++)
  {
    /*
      Find this tau in zero crossings list.
    */
    k=0;
    node=list[i];
    for (j=0; j <= (ssize_t) number_crossings; j++)
      if (zero_crossing[j].tau == node->tau)
        k=j;
    /*
      Find the value of the peak.
    */
    peak=zero_crossing[k].crossings[node->right] == -1 ? MagickTrue :
      MagickFalse;
    index=node->left;
    value=zero_crossing[k].histogram[index];
    for (x=node->left; x <= node->right; x++)
    {
      if (peak != MagickFalse)
        {
          if (zero_crossing[k].histogram[x] > value)
            {
              value=zero_crossing[k].histogram[x];
              index=x;
            }
        }
      else
        if (zero_crossing[k].histogram[x] < value)
          {
            value=zero_crossing[k].histogram[x];
            index=x;
          }
    }
    for (x=node->left; x <= node->right; x++)
    {
      if (index == 0)
        index=256;
      if (peak != MagickFalse)
        extrema[x]=(short) index;
      else
        extrema[x]=(short) (-index);
    }
  }
  /*
    Determine the average tau.
  */
  average_tau=0.0;
  for (i=0; i < number_nodes; i++)
    average_tau+=list[i]->tau;
  average_tau*=PerceptibleReciprocal((double) number_nodes);
  /*
    Relinquish resources.
  */
  FreeNodes(root);
  zero_crossing=(ZeroCrossing *) RelinquishMagickMemory(zero_crossing);
  list=(IntervalTree **) RelinquishMagickMemory(list);
  return(average_tau);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S c a l e S p a c e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ScaleSpace() performs a scale-space filter on the 1D histogram.
%
%  The format of the ScaleSpace method is:
%
%      ScaleSpace(const ssize_t *histogram,const double tau,
%        double *scale_histogram)
%
%  A description of each parameter follows.
%
%    o histogram: Specifies an array of doubles representing the number
%      of pixels for each intensity of a particular color component.
%
*/

static void ScaleSpace(const ssize_t *histogram,const double tau,
  double *scale_histogram)
{
  double
    alpha,
    beta,
    *gamma,
    sum;

  register ssize_t
    u,
    x;

  gamma=(double *) AcquireQuantumMemory(256,sizeof(*gamma));
  if (gamma == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,
      "UnableToAllocateGammaMap");
  alpha=PerceptibleReciprocal(tau*sqrt(2.0*MagickPI));
  beta=(-1.0*PerceptibleReciprocal(2.0*tau*tau));
  for (x=0; x <= 255; x++)
    gamma[x]=0.0;
  for (x=0; x <= 255; x++)
  {
    gamma[x]=exp((double) beta*x*x);
    if (gamma[x] < MagickEpsilon)
      break;
  }
  for (x=0; x <= 255; x++)
  {
    sum=0.0;
    for (u=0; u <= 255; u++)
      sum+=(double) histogram[u]*gamma[MagickAbsoluteValue(x-u)];
    scale_histogram[x]=alpha*sum;
  }
  gamma=(double *) RelinquishMagickMemory(gamma);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S e g m e n t I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SegmentImage() segment an image by analyzing the histograms of the color
%  components and identifying units that are homogeneous with the fuzzy
%  C-means technique.
%
%  The format of the SegmentImage method is:
%
%      MagickBooleanType SegmentImage(Image *image,
%        const ColorspaceType colorspace,const MagickBooleanType verbose,
%        const double cluster_threshold,const double smooth_threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o colorspace: Indicate the colorspace.
%
%    o verbose:  Set to MagickTrue to print detailed information about the
%      identified classes.
%
%    o cluster_threshold:  This represents the minimum number of pixels
%      contained in a hexahedra before it can be considered valid (expressed
%      as a percentage).
%
%    o smooth_threshold: the smoothing threshold eliminates noise in the second
%      derivative of the histogram.  As the value is increased, you can expect a
%      smoother second derivative.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SegmentImage(Image *image,
  const ColorspaceType colorspace,const MagickBooleanType verbose,
  const double cluster_threshold,const double smooth_threshold,
  ExceptionInfo *exception)
{
  ColorspaceType
    previous_colorspace;

  MagickBooleanType
    status;

  register ssize_t
    i;

  short
    *extrema[MaxDimension];

  ssize_t
    *histogram[MaxDimension];

  /*
    Allocate histogram and extrema.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  for (i=0; i < MaxDimension; i++)
  {
    histogram[i]=(ssize_t *) AcquireQuantumMemory(256,sizeof(**histogram));
    extrema[i]=(short *) AcquireQuantumMemory(256,sizeof(**extrema));
    if ((histogram[i] == (ssize_t *) NULL) || (extrema[i] == (short *) NULL))
      {
        for (i-- ; i >= 0; i--)
        {
          extrema[i]=(short *) RelinquishMagickMemory(extrema[i]);
          histogram[i]=(ssize_t *) RelinquishMagickMemory(histogram[i]);
        }
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename)
      }
  }
  /*
    Initialize histogram.
  */
  previous_colorspace=image->colorspace;
  (void) TransformImageColorspace(image,colorspace,exception);
  InitializeHistogram(image,histogram,exception);
  (void) OptimalTau(histogram[Red],Tau,0.2,DeltaTau,
    smooth_threshold == 0.0 ? 1.0 : smooth_threshold,extrema[Red]);
  (void) OptimalTau(histogram[Green],Tau,0.2,DeltaTau,
    smooth_threshold == 0.0 ? 1.0 : smooth_threshold,extrema[Green]);
  (void) OptimalTau(histogram[Blue],Tau,0.2,DeltaTau,
    smooth_threshold == 0.0 ? 1.0 : smooth_threshold,extrema[Blue]);
  /*
    Classify using the fuzzy c-Means technique.
  */
  status=Classify(image,extrema,cluster_threshold,WeightingExponent,verbose,
    exception);
  (void) TransformImageColorspace(image,previous_colorspace,exception);
  /*
    Relinquish resources.
  */
  for (i=0; i < MaxDimension; i++)
  {
    extrema[i]=(short *) RelinquishMagickMemory(extrema[i]);
    histogram[i]=(ssize_t *) RelinquishMagickMemory(histogram[i]);
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   Z e r o C r o s s H i s t o g r a m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ZeroCrossHistogram() find the zero crossings in a histogram and marks
%  directions as:  1 is negative to positive; 0 is zero crossing; and -1
%  is positive to negative.
%
%  The format of the ZeroCrossHistogram method is:
%
%      ZeroCrossHistogram(double *second_derivative,
%        const double smooth_threshold,short *crossings)
%
%  A description of each parameter follows.
%
%    o second_derivative: Specifies an array of doubles representing the
%      second derivative of the histogram of a particular color component.
%
%    o crossings:  This array of integers is initialized with
%      -1, 0, or 1 representing the slope of the first derivative of the
%      of a particular color component.
%
*/
static void ZeroCrossHistogram(double *second_derivative,
  const double smooth_threshold,short *crossings)
{
  register ssize_t
    i;

  ssize_t
    parity;

  /*
    Merge low numbers to zero to help prevent noise.
  */
  for (i=0; i <= 255; i++)
    if ((second_derivative[i] < smooth_threshold) &&
        (second_derivative[i] >= -smooth_threshold))
      second_derivative[i]=0.0;
  /*
    Mark zero crossings.
  */
  parity=0;
  for (i=0; i <= 255; i++)
  {
    crossings[i]=0;
    if (second_derivative[i] < 0.0)
      {
        if (parity > 0)
          crossings[i]=(-1);
        parity=1;
      }
    else
      if (second_derivative[i] > 0.0)
        {
          if (parity < 0)
            crossings[i]=1;
          parity=(-1);
        }
  }
}

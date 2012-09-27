// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Implementation of Image
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <cstdlib>
#include <string>
#include <string.h>
#include <errno.h>
#include <math.h>

using namespace std;

#include "Magick++/Image.h"
#include "Magick++/Functions.h"
#include "Magick++/Pixels.h"
#include "Magick++/Options.h"
#include "Magick++/ImageRef.h"

#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))
#define MagickPI  3.14159265358979323846264338327950288419716939937510
#define DegreesToRadians(x)  (MagickPI*(x)/180.0)

MagickPPExport const char *Magick::borderGeometryDefault = "6x6+0+0";
MagickPPExport const char *Magick::frameGeometryDefault  = "25x25+6+6";
MagickPPExport const char *Magick::raiseGeometryDefault  = "6x6+0+0";

static bool magick_initialized=false;

//
// Explicit template instantiations
//

//
// Friend functions to compare Image objects
//

MagickPPExport int Magick::operator == ( const Magick::Image& left_,
                                        const Magick::Image& right_ )
{
  // If image pixels and signature are the same, then the image is identical
  return ( ( left_.rows() == right_.rows() ) &&
	   ( left_.columns() == right_.columns() ) &&
	   ( left_.signature() == right_.signature() )
	   );
}
MagickPPExport int Magick::operator != ( const Magick::Image& left_,
                                        const Magick::Image& right_ )
{
  return ( ! (left_ == right_) );
}
MagickPPExport int Magick::operator >  ( const Magick::Image& left_,
                                        const Magick::Image& right_ )
{
  return ( !( left_ < right_ ) && ( left_ != right_ ) );
}
MagickPPExport int Magick::operator <  ( const Magick::Image& left_,
                                        const Magick::Image& right_ )
{
  // If image pixels are less, then image is smaller
  return ( ( left_.rows() * left_.columns() ) <
	   ( right_.rows() * right_.columns() )
	   );
}
MagickPPExport int Magick::operator >= ( const Magick::Image& left_,
                                        const Magick::Image& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
MagickPPExport int Magick::operator <= ( const Magick::Image& left_,
                                        const Magick::Image& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}

//
// Image object implementation
//

// Construct from image file or image specification
Magick::Image::Image( const std::string &imageSpec_ )
  : _imgRef(new ImageRef)
{
  try
    {
      // Initialize, Allocate and Read images
      read( imageSpec_ );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Construct a blank image canvas of specified size and color
Magick::Image::Image( const Geometry &size_,
		      const Color &color_ )
  : _imgRef(new ImageRef)
{
  // xc: prefix specifies an X11 color string
  std::string imageSpec("xc:");
  imageSpec += color_;

  try
    {
      // Set image size
      size( size_ );

      // Initialize, Allocate and Read images
      read( imageSpec );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Construct Image from in-memory BLOB
Magick::Image::Image ( const Blob &blob_ )
  : _imgRef(new ImageRef)
{
  try
    {
      // Initialize, Allocate and Read images
      read( blob_ );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Construct Image of specified size from in-memory BLOB
Magick::Image::Image ( const Blob &blob_,
		       const Geometry &size_ )
  : _imgRef(new ImageRef)
{
  try
    {
      // Read from Blob
      read( blob_, size_ );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Construct Image of specified size and depth from in-memory BLOB
Magick::Image::Image ( const Blob &blob_,
		       const Geometry &size_,
		       const size_t depth_ )
  : _imgRef(new ImageRef)
{
  try
    {
      // Read from Blob
      read( blob_, size_, depth_ );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Construct Image of specified size, depth, and format from in-memory BLOB
Magick::Image::Image ( const Blob &blob_,
		       const Geometry &size_,
		       const size_t depth_,
		       const std::string &magick_ )
  : _imgRef(new ImageRef)
{
  try
    {
      // Read from Blob
      read( blob_, size_, depth_, magick_ );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Construct Image of specified size, and format from in-memory BLOB
Magick::Image::Image ( const Blob &blob_,
		       const Geometry &size_,
		       const std::string &magick_ )
  : _imgRef(new ImageRef)
{
  try
    {
      // Read from Blob
      read( blob_, size_, magick_ );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Construct an image based on an array of raw pixels, of specified
// type and mapping, in memory
Magick::Image::Image ( const size_t width_,
                       const size_t height_,
                       const std::string &map_,
                       const StorageType type_,
                       const void *pixels_ )
  : _imgRef(new ImageRef)
{
  try
    {
      read( width_, height_, map_.c_str(), type_, pixels_ );
    }
  catch ( const Warning & /*warning_*/ )
    {
      // FIXME: need a way to report warnings in constructor
    }
  catch ( const Error & /*error_*/ )
    {
      // Release resources
      delete _imgRef;
      throw;
    }
}

// Default constructor
Magick::Image::Image( void )
  : _imgRef(new ImageRef)
{
}

// Destructor
/* virtual */
Magick::Image::~Image()
{
  bool doDelete = false;
  {
    Lock( &_imgRef->_mutexLock );
    if ( --_imgRef->_refCount == 0 )
      doDelete = true;
  }

  if ( doDelete )
    {
      delete _imgRef;
    }
  _imgRef = 0;
}

// Adaptive-blur image
void Magick::Image::adaptiveBlur( const double radius_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    AdaptiveBlurImage( image(), radius_, sigma_, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Local adaptive threshold image
// http://www.dai.ed.ac.uk/HIPR2/adpthrsh.htm
// Width x height define the size of the pixel neighborhood
// offset = constant to subtract from pixel neighborhood mean
void Magick::Image::adaptiveThreshold ( const size_t width_,
                                        const size_t height_,
                                        const ssize_t offset_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    AdaptiveThresholdImage( constImage(), width_, height_, offset_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Add noise to image
void Magick::Image::addNoise( const NoiseType noiseType_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    AddNoiseImage ( image(),
		    noiseType_,
		    &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

void Magick::Image::addNoiseChannel( const ChannelType channel_,
                                     const NoiseType noiseType_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    AddNoiseImageChannel ( image(),
                           channel_,
                           noiseType_,
                           &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Affine Transform image
void Magick::Image::affineTransform ( const DrawableAffine &affine_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  
  AffineMatrix _affine;
  _affine.sx = affine_.sx();
  _affine.sy = affine_.sy();
  _affine.rx = affine_.rx();
  _affine.ry = affine_.ry();
  _affine.tx = affine_.tx();  
  _affine.ty = affine_.ty();
  
  MagickCore::Image* newImage =
    AffineTransformImage( image(), &_affine, &exceptionInfo);     
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Annotate using specified text, and placement location
void Magick::Image::annotate ( const std::string &text_,
			       const Geometry &location_ )
{
  annotate ( text_, location_,  NorthWestGravity, 0.0 );
}
// Annotate using specified text, bounding area, and placement gravity
void Magick::Image::annotate ( const std::string &text_,
			       const Geometry &boundingArea_,
			       const GravityType gravity_ )
{
  annotate ( text_, boundingArea_, gravity_, 0.0 );
}
// Annotate with text using specified text, bounding area, placement
// gravity, and rotation.
void Magick::Image::annotate ( const std::string &text_,
			       const Geometry &boundingArea_,
			       const GravityType gravity_,
			       const double degrees_ )
{
  modifyImage();

  DrawInfo *drawInfo
    = options()->drawInfo();
  
  drawInfo->text = const_cast<char *>(text_.c_str());

  char boundingArea[MaxTextExtent];

  drawInfo->geometry = 0;
  if ( boundingArea_.isValid() ){
    if ( boundingArea_.width() == 0 || boundingArea_.height() == 0 )
      {
        FormatLocaleString( boundingArea, MaxTextExtent, "%+.20g%+.20g",
          (double) boundingArea_.xOff(), (double) boundingArea_.yOff() );
      }
    else
      {
        (void) CopyMagickString( boundingArea, string(boundingArea_).c_str(),
          MaxTextExtent);
      }
    drawInfo->geometry = boundingArea;
  }

  drawInfo->gravity = gravity_;

  AffineMatrix oaffine = drawInfo->affine;
  if ( degrees_ != 0.0)
    {
        AffineMatrix affine;
        affine.sx=1.0;
        affine.rx=0.0;
        affine.ry=0.0;
        affine.sy=1.0;
        affine.tx=0.0;
        affine.ty=0.0;

        AffineMatrix current = drawInfo->affine;
        affine.sx=cos(DegreesToRadians(fmod(degrees_,360.0)));
        affine.rx=sin(DegreesToRadians(fmod(degrees_,360.0)));
        affine.ry=(-sin(DegreesToRadians(fmod(degrees_,360.0))));
        affine.sy=cos(DegreesToRadians(fmod(degrees_,360.0)));

        drawInfo->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
        drawInfo->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
        drawInfo->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
        drawInfo->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
        drawInfo->affine.tx=current.sx*affine.tx+current.ry*affine.ty
          +current.tx;
    }

  AnnotateImage( image(), drawInfo );

  // Restore original values
  drawInfo->affine = oaffine;
  drawInfo->text = 0;
  drawInfo->geometry = 0;

  throwImageException();
}
// Annotate with text (bounding area is entire image) and placement gravity.
void Magick::Image::annotate ( const std::string &text_,
			       const GravityType gravity_ )
{
  modifyImage();

  DrawInfo *drawInfo
    = options()->drawInfo();

  drawInfo->text = const_cast<char *>(text_.c_str());

  drawInfo->gravity = gravity_;

  AnnotateImage( image(), drawInfo );

  drawInfo->gravity = NorthWestGravity;
  drawInfo->text = 0;

  throwImageException();
}

// Blur image
void Magick::Image::blur( const double radius_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    BlurImage( image(), radius_, sigma_, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

void Magick::Image::blurChannel( const ChannelType channel_,
                                 const double radius_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    BlurImageChannel( image(), channel_,radius_, sigma_, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Add border to image
// Only uses width & height
void Magick::Image::border( const Geometry &geometry_ )
{
  RectangleInfo borderInfo = geometry_;
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    BorderImage( image(), &borderInfo, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Extract channel from image
void Magick::Image::channel ( const ChannelType channel_ )
{
  modifyImage();
  SeparateImageChannel ( image(), channel_ );
  throwImageException();
}

// Set or obtain modulus channel depth
void Magick::Image::channelDepth ( const ChannelType channel_,
                                   const size_t depth_)
{
  modifyImage();
  SetImageChannelDepth( image(), channel_, depth_);
  throwImageException();
}
size_t Magick::Image::channelDepth ( const ChannelType channel_ )
{
  size_t channel_depth;

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  channel_depth=GetImageChannelDepth( constImage(), channel_,
                                      &exceptionInfo );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  return channel_depth;
}


// Charcoal-effect image
void Magick::Image::charcoal( const double radius_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    CharcoalImage( image(), radius_, sigma_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Chop image
void Magick::Image::chop( const Geometry &geometry_ )
{
  RectangleInfo chopInfo = geometry_;
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ChopImage( image(), &chopInfo, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// contains one or more color corrections and applies the correction to the
// image.
void Magick::Image::cdl ( const std::string &cdl_ )
{
  modifyImage();
  (void) ColorDecisionListImage( image(), cdl_.c_str() );
  throwImageException();
}

// Colorize
void Magick::Image::colorize ( const unsigned int opacityRed_,
                               const unsigned int opacityGreen_,
                               const unsigned int opacityBlue_,
			       const Color &penColor_ )
{
  if ( !penColor_.isValid() )
  {
    throwExceptionExplicit( OptionError,
			    "Pen color argument is invalid");
  }

  char opacity[MaxTextExtent];
  FormatLocaleString(opacity,MaxTextExtent,"%u/%u/%u",opacityRed_,opacityGreen_,opacityBlue_);

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
  ColorizeImage ( image(), opacity,
		  penColor_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::colorize ( const unsigned int opacity_,
			       const Color &penColor_ )
{
  colorize( opacity_, opacity_, opacity_, penColor_ );
}

// Apply a color matrix to the image channels.  The user supplied
// matrix may be of order 1 to 6 (1x1 through 6x6).
void Magick::Image::colorMatrix (const size_t order_,
         const double *color_matrix_)
{
  KernelInfo
    *kernel_info;

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
    kernel_info=AcquireKernelInfo("1");
  kernel_info->width=order_;
  kernel_info->height=order_;
  kernel_info->values=(double *) color_matrix_;
  MagickCore::Image* newImage =
    ColorMatrixImage( image(), kernel_info, &exceptionInfo );
  kernel_info->values=(double *) NULL;
  kernel_info=DestroyKernelInfo(kernel_info);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Compare current image with another image
// Sets meanErrorPerPixel, normalizedMaxError, and normalizedMeanError
// in the current image. False is returned if the images are identical.
bool Magick::Image::compare ( const Image &reference_ )
{
  modifyImage();
  Image ref = reference_;
  ref.modifyImage();
  return static_cast<bool>(IsImagesEqual(image(), ref.image()));
}

// Composite two images
void Magick::Image::composite ( const Image &compositeImage_,
				const ssize_t xOffset_,
				const ssize_t yOffset_,
				const CompositeOperator compose_ )
{
  // Image supplied as compositeImage is composited with current image and
  // results in updating current image.
  modifyImage();

  CompositeImage( image(),
		  compose_,
		  compositeImage_.constImage(),
		  xOffset_,
                  yOffset_ );
  throwImageException();
}
void Magick::Image::composite ( const Image &compositeImage_,
				const Geometry &offset_,
				const CompositeOperator compose_ )
{
  modifyImage();

  ssize_t x = offset_.xOff();
  ssize_t y = offset_.yOff();
  size_t width = columns();
  size_t height = rows();

  ParseMetaGeometry (static_cast<std::string>(offset_).c_str(),
		      &x, &y,
		      &width, &height );

  CompositeImage( image(),
		  compose_,
		  compositeImage_.constImage(),
		  x, y );
  throwImageException();
}
void Magick::Image::composite ( const Image &compositeImage_,
				const GravityType gravity_,
				const CompositeOperator compose_ )
{
  modifyImage();

  RectangleInfo geometry;

  SetGeometry(compositeImage_.constImage(), &geometry);
  GravityAdjustGeometry(columns(), rows(), gravity_, &geometry);

  CompositeImage( image(),
		  compose_,
		  compositeImage_.constImage(),
		  geometry.x, geometry.y );
  throwImageException();
}

// Contrast image
void Magick::Image::contrast ( const size_t sharpen_ )
{
  modifyImage();
  ContrastImage ( image(), (MagickBooleanType) sharpen_ );
  throwImageException();
}

// Convolve image.  Applies a general image convolution kernel to the image.
//  order_ represents the number of columns and rows in the filter kernel.
//  kernel_ is an array of doubles representing the convolution kernel.
void Magick::Image::convolve ( const size_t order_,
                               const double *kernel_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
  ConvolveImage ( image(), order_,
		  kernel_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Crop image
void Magick::Image::crop ( const Geometry &geometry_ )
{
  RectangleInfo cropInfo = geometry_;
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    CropImage( image(),
	       &cropInfo,
	       &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Cycle Color Map
void Magick::Image::cycleColormap ( const ssize_t amount_ )
{
  modifyImage();
  CycleColormapImage( image(), amount_ );
  throwImageException();
}

// Despeckle
void Magick::Image::despeckle ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    DespeckleImage( image(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Display image
void Magick::Image::display( void )
{
  DisplayImages( imageInfo(), image() );
}

// Distort image.  distorts an image using various distortion methods, by
// mapping color lookups of the source image to a new destination image
// usally of the same size as the source image, unless 'bestfit' is set to
// true.
void Magick::Image::distort ( const DistortImageMethod method_,
                              const size_t number_arguments_,
                              const double *arguments_,
                              const bool bestfit_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage = DistortImage ( image(), method_,
    number_arguments_, arguments_, bestfit_ == true ? MagickTrue : MagickFalse,
    &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Draw on image using single drawable
void Magick::Image::draw ( const Magick::Drawable &drawable_ )
{
  modifyImage();

  DrawingWand *wand = DrawAllocateWand( options()->drawInfo(), image());

  if(wand)
    {
      drawable_.operator()(wand);

      if( constImage()->exception.severity == UndefinedException)
        DrawRender(wand);

      wand=DestroyDrawingWand(wand);
    }

  throwImageException();
}

// Draw on image using a drawable list
void Magick::Image::draw ( const std::list<Magick::Drawable> &drawable_ )
{
  modifyImage();

  DrawingWand *wand = DrawAllocateWand( options()->drawInfo(), image());

  if(wand)
    {
      for( std::list<Magick::Drawable>::const_iterator p = drawable_.begin();
           p != drawable_.end(); p++ )
        {
          p->operator()(wand);
          if( constImage()->exception.severity != UndefinedException)
            break;
        }

      if( constImage()->exception.severity == UndefinedException)
        DrawRender(wand);

      wand=DestroyDrawingWand(wand);
    }

  throwImageException();
}

// Hilight edges in image
void Magick::Image::edge ( const double radius_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    EdgeImage( image(), radius_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Emboss image (hilight edges)
void Magick::Image::emboss ( const double radius_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    EmbossImage( image(), radius_, sigma_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Enhance image (minimize noise)
void Magick::Image::enhance ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    EnhanceImage( image(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Equalize image (histogram equalization)
void Magick::Image::equalize ( void )
{
  modifyImage();
  EqualizeImage( image() );
  throwImageException();
}

// Erase image to current "background color"
void Magick::Image::erase ( void )
{
  modifyImage();
  SetImageBackgroundColor( image() );
  throwImageException();
}

// Extends image as defined by the geometry.
//
void Magick::Image::extent ( const Geometry &geometry_ )
{
  RectangleInfo extentInfo = geometry_;
  modifyImage();
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ExtentImage ( image(), &extentInfo, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::extent ( const Geometry &geometry_, const Color &backgroundColor_ )
{
  backgroundColor ( backgroundColor_ );
  extent ( geometry_ );
}
void Magick::Image::extent ( const Geometry &geometry_, const GravityType gravity_ )
{
  RectangleInfo geometry;

  SetGeometry(image(), &geometry);
  GravityAdjustGeometry(image()->columns, image()->rows, gravity_, &geometry);
  extent ( geometry_ );
}
void Magick::Image::extent ( const Geometry &geometry_, const Color &backgroundColor_, const GravityType gravity_ )
{
  image()->gravity  = gravity_;
  backgroundColor ( backgroundColor_ );
  extent ( geometry_ );
}

// Flip image (reflect each scanline in the vertical direction)
void Magick::Image::flip ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    FlipImage( image(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Flood-fill color across pixels that match the color of the
// target pixel and are neighbors of the target pixel.
// Uses current fuzz setting when determining color match.
void Magick::Image::floodFillColor( const ssize_t x_,
                                    const ssize_t y_,
				    const Magick::Color &fillColor_ )
{
  floodFillTexture( x_, y_, Image( Geometry( 1, 1), fillColor_ ) );
}
void Magick::Image::floodFillColor( const Geometry &point_,
				    const Magick::Color &fillColor_ )
{
  floodFillTexture( point_, Image( Geometry( 1, 1), fillColor_) );
}

// Flood-fill color across pixels starting at target-pixel and
// stopping at pixels matching specified border color.
// Uses current fuzz setting when determining color match.
void Magick::Image::floodFillColor( const ssize_t x_,
                                    const ssize_t y_,
				    const Magick::Color &fillColor_,
				    const Magick::Color &borderColor_ )
{
  floodFillTexture( x_, y_, Image( Geometry( 1, 1), fillColor_),
                    borderColor_ );
}
void Magick::Image::floodFillColor( const Geometry &point_,
				    const Magick::Color &fillColor_,
				    const Magick::Color &borderColor_ )
{
  floodFillTexture( point_, Image( Geometry( 1, 1), fillColor_),
                    borderColor_ );
}

// Floodfill pixels matching color (within fuzz factor) of target
// pixel(x,y) with replacement opacity value using method.
void Magick::Image::floodFillOpacity( const ssize_t x_,
                                      const ssize_t y_,
                                      const unsigned int opacity_,
                                      const PaintMethod method_ )
{
  modifyImage();
  MagickPixelPacket target;
  GetMagickPixelPacket(image(),&target);
  PixelPacket pixel=static_cast<PixelPacket>(pixelColor(x_,y_));
  target.red=pixel.red;
  target.green=pixel.green;
  target.blue=pixel.blue;
  target.opacity=opacity_;
  FloodfillPaintImage ( image(),
                        DefaultChannels,
                        options()->drawInfo(), // const DrawInfo *draw_info
                        &target,
                  			static_cast<ssize_t>(x_), static_cast<ssize_t>(y_),
                        method_  == FloodfillMethod ? MagickFalse : MagickTrue);
  throwImageException();
}

// Flood-fill texture across pixels that match the color of the
// target pixel and are neighbors of the target pixel.
// Uses current fuzz setting when determining color match.
void Magick::Image::floodFillTexture( const ssize_t x_,
                                      const ssize_t y_,
				      const Magick::Image &texture_ )
{
  modifyImage();

  // Set drawing pattern
  options()->fillPattern(texture_.constImage());

  // Get pixel view
  Pixels pixels(*this);
  // Fill image
  PixelPacket *p = pixels.get(x_, y_, 1, 1 );
  MagickPixelPacket target;
  GetMagickPixelPacket(constImage(),&target);
  target.red=p->red;
  target.green=p->green;
  target.blue=p->blue;
  if (p)
    FloodfillPaintImage ( image(), // Image *image
                          DefaultChannels,
                          options()->drawInfo(), // const DrawInfo *draw_info
                          &target, // const MagickPacket target
                          static_cast<ssize_t>(x_), // const ssize_t x_offset
                          static_cast<ssize_t>(y_), // const ssize_t y_offset
                          MagickFalse // const PaintMethod method
      );

  throwImageException();
}
void Magick::Image::floodFillTexture( const Magick::Geometry &point_,
				      const Magick::Image &texture_ )
{
  floodFillTexture( point_.xOff(), point_.yOff(), texture_ );
}

// Flood-fill texture across pixels starting at target-pixel and
// stopping at pixels matching specified border color.
// Uses current fuzz setting when determining color match.
void Magick::Image::floodFillTexture( const ssize_t x_,
                                      const ssize_t y_,
				      const Magick::Image &texture_,
				      const Magick::Color &borderColor_ )
{
  modifyImage();

  // Set drawing fill pattern
  options()->fillPattern(texture_.constImage());

  MagickPixelPacket target;
  GetMagickPixelPacket(constImage(),&target);
  target.red=static_cast<PixelPacket>(borderColor_).red;
  target.green=static_cast<PixelPacket>(borderColor_).green;
  target.blue=static_cast<PixelPacket>(borderColor_).blue;
  FloodfillPaintImage ( image(),
                        DefaultChannels,
                        options()->drawInfo(),
                        &target,
                        static_cast<ssize_t>(x_),
                        static_cast<ssize_t>(y_),
                        MagickTrue);

  throwImageException();
}
void  Magick::Image::floodFillTexture( const Magick::Geometry &point_,
				       const Magick::Image &texture_,
				       const Magick::Color &borderColor_ )
{
  floodFillTexture( point_.xOff(), point_.yOff(), texture_, borderColor_ );
}

// Flop image (reflect each scanline in the horizontal direction)
void Magick::Image::flop ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    FlopImage( image(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Frame image
void Magick::Image::frame ( const Geometry &geometry_ )
{
  FrameInfo info;

  info.x           = static_cast<ssize_t>(geometry_.width());
  info.y           = static_cast<ssize_t>(geometry_.height());
  info.width       = columns() + ( static_cast<size_t>(info.x) << 1 );
  info.height      = rows() + ( static_cast<size_t>(info.y) << 1 );
  info.outer_bevel = geometry_.xOff();
  info.inner_bevel = geometry_.yOff();

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    FrameImage( image(), &info, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::frame ( const size_t width_,
                            const size_t height_,
			    const ssize_t outerBevel_, const ssize_t innerBevel_ )
{
  FrameInfo info;
  info.x           = static_cast<ssize_t>(width_);
  info.y           = static_cast<ssize_t>(height_);
  info.width       = columns() + ( static_cast<size_t>(info.x) << 1 );
  info.height      = rows() + ( static_cast<size_t>(info.y) << 1 );
  info.outer_bevel = static_cast<ssize_t>(outerBevel_);
  info.inner_bevel = static_cast<ssize_t>(innerBevel_);

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    FrameImage( image(), &info, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Fx image.  Applies a mathematical expression to the image.
void Magick::Image::fx ( const std::string expression )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    FxImageChannel ( image(), DefaultChannels, expression.c_str(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::fx ( const std::string expression,
                         const Magick::ChannelType channel )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    FxImageChannel ( image(), channel, expression.c_str(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Gamma correct image
void Magick::Image::gamma ( const double gamma_ )
{
  char gamma[MaxTextExtent + 1];
  FormatLocaleString( gamma, MaxTextExtent, "%3.6f", gamma_);

  modifyImage();
  GammaImage ( image(), gamma );
}

void Magick::Image::gamma ( const double gammaRed_,
                            const double gammaGreen_,
			    const double gammaBlue_ )
{
  char gamma[MaxTextExtent + 1];
  FormatLocaleString( gamma, MaxTextExtent, "%3.6f/%3.6f/%3.6f/",
		gammaRed_, gammaGreen_, gammaBlue_);

  modifyImage();
  GammaImage ( image(), gamma );
  throwImageException();
}

// Gaussian blur image
// The number of neighbor pixels to be included in the convolution
// mask is specified by 'width_'. The standard deviation of the
// gaussian bell curve is specified by 'sigma_'.
void Magick::Image::gaussianBlur ( const double width_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    GaussianBlurImage( image(), width_, sigma_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

void Magick::Image::gaussianBlurChannel ( const ChannelType channel_,
                                          const double width_,
                                          const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    GaussianBlurImageChannel( image(), channel_, width_, sigma_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Apply a color lookup table (Hald CLUT) to the image.
void  Magick::Image::haldClut ( const Image &clutImage_ )
{
  modifyImage();
  (void) HaldClutImage( image(), clutImage_.constImage() );
  throwImageException();
}

// Implode image
void Magick::Image::implode ( const double factor_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ImplodeImage( image(), factor_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// implements the inverse discrete Fourier transform (IFT) of the image either
// as a magnitude / phase or real / imaginary image pair.
void Magick::Image::inverseFourierTransform ( const Image &phase_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage = InverseFourierTransformImage( image(),
		 phase_.constImage(), MagickTrue, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::inverseFourierTransform ( const Image &phase_,
   const bool magnitude_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage = InverseFourierTransformImage( image(),
		 phase_.constImage(), magnitude_ == true ? MagickTrue : MagickFalse,
     &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Level image. Adjust the levels of the image by scaling the colors
// falling between specified white and black points to the full
// available quantum range. The parameters provided represent the
// black, mid (gamma), and white points.  The black point specifies
// the darkest color in the image. Colors darker than the black point
// are set to zero. Mid point (gamma) specifies a gamma correction to
// apply to the image. White point specifies the lightest color in the
// image.  Colors brighter than the white point are set to the maximum
// quantum value. The black and white point have the valid range 0 to
// QuantumRange while gamma has a useful range of 0 to ten.
void Magick::Image::level ( const double black_point,
                            const double white_point,
                            const double gamma )
{
  modifyImage();
  char levels[MaxTextExtent];
  FormatLocaleString( levels, MaxTextExtent, "%g,%g,%g",black_point,white_point,gamma);
  (void) LevelImage( image(), levels );
  throwImageException();
}

// Level image channel. Adjust the levels of the image channel by
// scaling the values falling between specified white and black points
// to the full available quantum range. The parameters provided
// represent the black, mid (gamma), and white points.  The black
// point specifies the darkest color in the image. Colors darker than
// the black point are set to zero. Mid point (gamma) specifies a
// gamma correction to apply to the image. White point specifies the
// lightest color in the image.  Colors brighter than the white point
// are set to the maximum quantum value. The black and white point
// have the valid range 0 to QuantumRange while gamma has a useful range of
// 0 to ten.
void  Magick::Image::levelChannel ( const Magick::ChannelType channel,
                                    const double black_point,
                                    const double white_point,
                                    const double gamma )
{
  modifyImage();
  (void) LevelImageChannel( image(), channel, black_point, white_point,
                            gamma );
  throwImageException();
}

// Magnify image by integral size
void Magick::Image::magnify ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    MagnifyImage( image(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Remap image colors with closest color from reference image
void Magick::Image::map ( const Image &mapImage_ , const bool dither_ )
{
  modifyImage();
  options()->quantizeDither( dither_ );
  RemapImage ( options()->quantizeInfo(), image(),
             mapImage_.constImage());
  throwImageException();
}
// Floodfill designated area with replacement opacity value
void Magick::Image::matteFloodfill ( const Color &target_ ,
				     const unsigned int opacity_,
				     const ssize_t x_, const ssize_t y_,
				     const Magick::PaintMethod method_ )
{
  modifyImage();
  MagickPixelPacket target;
  GetMagickPixelPacket(constImage(),&target);
  target.red=static_cast<PixelPacket>(target_).red;
  target.green=static_cast<PixelPacket>(target_).green;
  target.blue=static_cast<PixelPacket>(target_).blue;
  target.opacity=opacity_;
  FloodfillPaintImage ( image(), OpacityChannel, options()->drawInfo(), &target,
    x_, y_, method_ == FloodfillMethod ? MagickFalse : MagickTrue);
  throwImageException();
}

// Filter image by replacing each pixel component with the median
// color in a circular neighborhood
void Magick::Image::medianFilter ( const double radius_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    StatisticImage ( image(), MedianStatistic, (size_t) radius_, (size_t)
    radius_,&exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Merge layers
void Magick::Image::mergeLayers( const ImageLayerMethod layerMethod_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    MergeImageLayers ( image(),
        layerMethod_,
        &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Reduce image by integral size
void Magick::Image::minify ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    MinifyImage( image(), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Modulate percent hue, saturation, and brightness of an image
void Magick::Image::modulate ( const double brightness_,
			       const double saturation_,
			       const double hue_ )
{
  char modulate[MaxTextExtent + 1];
  FormatLocaleString( modulate, MaxTextExtent, "%3.6f,%3.6f,%3.6f",
		brightness_, saturation_, hue_);

  modifyImage();
  ModulateImage( image(), modulate );
  throwImageException();
}

// Motion blur image with specified blur factor
// The radius_ parameter specifies the radius of the Gaussian, in
// pixels, not counting the center pixel.  The sigma_ parameter
// specifies the standard deviation of the Laplacian, in pixels.
// The angle_ parameter specifies the angle the object appears
// to be comming from (zero degrees is from the right).
void            Magick::Image::motionBlur ( const double radius_,
                                            const double sigma_,
                                            const double angle_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    MotionBlurImage( image(), radius_, sigma_, angle_, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}
    
// Negate image.  Set grayscale_ to true to effect grayscale values
// only
void Magick::Image::negate ( const bool grayscale_ )
{
  modifyImage();
  NegateImage ( image(), grayscale_ == true ? MagickTrue : MagickFalse );
  throwImageException();
}

// Normalize image
void Magick::Image::normalize ( void )
{
  modifyImage();
  NormalizeImage ( image() );
  throwImageException();
}

// Oilpaint image
void Magick::Image::oilPaint ( const double radius_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    OilPaintImage( image(), radius_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Set or attenuate the opacity channel. If the image pixels are
// opaque then they are set to the specified opacity value, otherwise
// they are blended with the supplied opacity value.  The value of
// opacity_ ranges from 0 (completely opaque) to QuantumRange. The defines
// OpaqueOpacity and TransparentOpacity are available to specify
// completely opaque or completely transparent, respectively.
void Magick::Image::opacity ( const unsigned int opacity_ )
{
  modifyImage();
  SetImageOpacity( image(), opacity_ );
}

// Change the color of an opaque pixel to the pen color.
void Magick::Image::opaque ( const Color &opaqueColor_,
			     const Color &penColor_ )
{
  if ( !opaqueColor_.isValid() )
  {
    throwExceptionExplicit( OptionError,
			    "Opaque color argument is invalid" );
  }
  if ( !penColor_.isValid() )
  {
    throwExceptionExplicit( OptionError,
			    "Pen color argument is invalid" );
  }

  modifyImage();
  std::string opaqueColor = opaqueColor_;
  std::string penColor = penColor_;

  MagickPixelPacket opaque;
  MagickPixelPacket pen;
  (void) QueryMagickColor(std::string(opaqueColor_).c_str(),&opaque,&image()->exception);
  (void) QueryMagickColor(std::string(penColor_).c_str(),&pen,&image()->exception);
  OpaquePaintImage ( image(), &opaque, &pen, MagickFalse );
  throwImageException();
}

// Ping is similar to read except only enough of the image is read to
// determine the image columns, rows, and filesize.  Access the
// columns(), rows(), and fileSize() attributes after invoking ping.
// The image data is not valid after calling ping.
void Magick::Image::ping ( const std::string &imageSpec_ )
{
  options()->fileName( imageSpec_ );
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* image =
    PingImage( imageInfo(), &exceptionInfo );
  replaceImage( image );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Ping is similar to read except only enough of the image is read
// to determine the image columns, rows, and filesize.  Access the
// columns(), rows(), and fileSize() attributes after invoking
// ping.  The image data is not valid after calling ping.
void Magick::Image::ping ( const Blob& blob_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* image =
    PingBlob( imageInfo(), blob_.data(), blob_.length(), &exceptionInfo );
  replaceImage( image );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Execute a named process module using an argc/argv syntax similar to
// that accepted by a C 'main' routine. An exception is thrown if the
// requested process module doesn't exist, fails to load, or fails during
// execution.
void Magick::Image::process( std::string name_, const ssize_t argc, const char **argv )
{
  modifyImage();

  size_t status = 
    InvokeDynamicImageFilter( name_.c_str(), &image(), argc, argv,
      &image()->exception );

  if (status == false)
    throwException( image()->exception );
}

// Quantize colors in image using current quantization settings
// Set measureError_ to true in order to measure quantization error
void Magick::Image::quantize ( const bool measureError_  )
{
  modifyImage();
 
  if (measureError_)
    options()->quantizeInfo()->measure_error=MagickTrue;
  else
    options()->quantizeInfo()->measure_error=MagickFalse;

  QuantizeImage( options()->quantizeInfo(), image() );

  throwImageException();
}

// Apply an arithmetic or bitwise operator to the image pixel quantums.
void Magick::Image::quantumOperator ( const ChannelType channel_,
                                      const MagickEvaluateOperator operator_,
                                      double rvalue_)
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  EvaluateImageChannel( image(), channel_, operator_, rvalue_, &exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

void Magick::Image::quantumOperator ( const ssize_t x_,const ssize_t y_,
                                      const size_t columns_,
                                      const size_t rows_,
                                      const ChannelType channel_,
                                      const MagickEvaluateOperator operator_,
                                      const double rvalue_)
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  RectangleInfo geometry;
  geometry.width = columns_;
  geometry.height = rows_;
  geometry.x = x_;
  geometry.y = y_;
  MagickCore::Image *crop_image = CropImage( image(), &geometry,
    &exceptionInfo );
  EvaluateImageChannel( crop_image, channel_, operator_, rvalue_,
    &exceptionInfo );
  (void) CompositeImage( image(), image()->matte != MagickFalse ?
    OverCompositeOp : CopyCompositeOp, crop_image, geometry.x, geometry.y );
  crop_image = DestroyImageList(crop_image);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Raise image (lighten or darken the edges of an image to give a 3-D
// raised or lowered effect)
void Magick::Image::raise ( const Geometry &geometry_ ,
			    const bool raisedFlag_ )
{
  RectangleInfo raiseInfo = geometry_;
  modifyImage();
  RaiseImage ( image(), &raiseInfo, raisedFlag_ == true ? MagickTrue : MagickFalse );
  throwImageException();
}


// Random threshold image.
//
// Changes the value of individual pixels based on the intensity
// of each pixel compared to a random threshold.  The result is a
// low-contrast, two color image.  The thresholds_ argument is a
// geometry containing LOWxHIGH thresholds.  If the string
// contains 2x2, 3x3, or 4x4, then an ordered dither of order 2,
// 3, or 4 will be performed instead.  If a channel_ argument is
// specified then only the specified channel is altered.  This is
// a very fast alternative to 'quantize' based dithering.
void Magick::Image::randomThreshold( const Geometry &thresholds_ )
{
  randomThresholdChannel(thresholds_,DefaultChannels);
}
void Magick::Image::randomThresholdChannel( const Geometry &thresholds_,
                                            const ChannelType channel_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  modifyImage();
  (void) RandomThresholdImageChannel( image(),
                                      channel_,
                                      static_cast<std::string>(thresholds_).c_str(),
                                      &exceptionInfo );
  throwImageException();
  (void) DestroyExceptionInfo( &exceptionInfo );
}
    
// Read image into current object
void Magick::Image::read ( const std::string &imageSpec_ )
{
  options()->fileName( imageSpec_ );

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* image =
    ReadImage( imageInfo(), &exceptionInfo );

  // Ensure that multiple image frames were not read.
  if ( image && image->next )
    {
      // Destroy any extra image frames
      MagickCore::Image* next = image->next;
      image->next = 0;
      next->previous = 0;
      DestroyImageList( next );
 
    }
  if ( image )
    {
      (void) DestroyExceptionInfo( &exceptionInfo );
      throwException( image->exception );
    }
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  replaceImage( image );
}

// Read image of specified size into current object
void Magick::Image::read ( const Geometry &size_,
			   const std::string &imageSpec_ )
{
  size( size_ );
  read( imageSpec_ );
}

// Read image from in-memory BLOB
void Magick::Image::read ( const Blob &blob_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* image =
    BlobToImage( imageInfo(),
		 static_cast<const void *>(blob_.data()),
		 blob_.length(), &exceptionInfo );
  replaceImage( image );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  if ( image )
    throwException( image->exception );
}

// Read image of specified size from in-memory BLOB
void  Magick::Image::read ( const Blob &blob_,
			    const Geometry &size_ )
{
  // Set image size
  size( size_ );
  // Read from Blob
  read( blob_ );
}

// Read image of specified size and depth from in-memory BLOB
void Magick::Image::read ( const Blob &blob_,
			   const Geometry &size_,
			   const size_t depth_ )
{
  // Set image size
  size( size_ );
  // Set image depth
  depth( depth_ );
  // Read from Blob
  read( blob_ );
}

// Read image of specified size, depth, and format from in-memory BLOB
void Magick::Image::read ( const Blob &blob_,
			   const Geometry &size_,
			   const size_t depth_,
			   const std::string &magick_ )
{
  // Set image size
  size( size_ );
  // Set image depth
  depth( depth_ );
  // Set image magick
  magick( magick_ );
  // Set explicit image format
  fileName( magick_ + ':');
  // Read from Blob
  read( blob_ );
}

// Read image of specified size, and format from in-memory BLOB
void Magick::Image::read ( const Blob &blob_,
			   const Geometry &size_,
			   const std::string &magick_ )
{
  // Set image size
  size( size_ );
  // Set image magick
  magick( magick_ );
  // Set explicit image format
  fileName( magick_ + ':');
  // Read from Blob
  read( blob_ );
}

// Read image based on raw pixels in memory (ConstituteImage)
void Magick::Image::read ( const size_t width_,
                           const size_t height_,
                           const std::string &map_,
                           const StorageType type_,
                           const void *pixels_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* image =
    ConstituteImage( width_, height_, map_.c_str(), type_, pixels_,
                     &exceptionInfo );
  replaceImage( image );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  if ( image )
    throwException( image->exception );
}

// Reduce noise in image
void Magick::Image::reduceNoise ( const double order_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    StatisticImage( image(), NonpeakStatistic, (size_t) order_, (size_t) order_,
    &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Resize image
void Magick::Image::resize( const Geometry &geometry_ )
{
  // Calculate new size.  This code should be supported using binary arguments
  // in the ImageMagick library.
  ssize_t x = 0;
  ssize_t y = 0;
  size_t width = columns();
  size_t height = rows();

  ParseMetaGeometry (static_cast<std::string>(geometry_).c_str(),
                     &x, &y,
                     &width, &height );

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ResizeImage( image(),
               width,
               height,
               image()->filter,
               1.0,
               &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Roll image
void Magick::Image::roll ( const Geometry &roll_ )
{
  ssize_t xOff = roll_.xOff();
  if ( roll_.xNegative() )
    xOff = 0 - xOff;
  ssize_t yOff = roll_.yOff();
  if ( roll_.yNegative() )
    yOff = 0 - yOff;

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    RollImage( image(), xOff, yOff, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::roll ( const size_t columns_,
                           const size_t rows_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    RollImage( image(),
               static_cast<ssize_t>(columns_),
               static_cast<ssize_t>(rows_), &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Rotate image
void Magick::Image::rotate ( const double degrees_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    RotateImage( image(), degrees_, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Sample image
void Magick::Image::sample ( const Geometry &geometry_ )
{
  ssize_t x = 0;
  ssize_t y = 0;
  size_t width = columns();
  size_t height = rows();

  ParseMetaGeometry (static_cast<std::string>(geometry_).c_str(),
		      &x, &y,
		      &width, &height );

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    SampleImage( image(), width, height, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Scale image
void Magick::Image::scale ( const Geometry &geometry_ )
{
  ssize_t x = 0;
  ssize_t y = 0;
  size_t width = columns();
  size_t height = rows();

  ParseMetaGeometry (static_cast<std::string>(geometry_).c_str(),
		      &x, &y,
		      &width, &height );

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ScaleImage( image(), width, height, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Segment (coalesce similar image components) by analyzing the
// histograms of the color components and identifying units that are
// homogeneous with the fuzzy c-means technique.
void Magick::Image::segment ( const double clusterThreshold_, 
			      const double smoothingThreshold_ )
{
  modifyImage();
  SegmentImage ( image(),
		 options()->quantizeColorSpace(),
		 (MagickBooleanType) options()->verbose(),
		 clusterThreshold_,
		 smoothingThreshold_ );
  throwImageException();
  SyncImage( image() );
  throwImageException();
}

// Shade image using distant light source
void Magick::Image::shade ( const double azimuth_,
			    const double elevation_,
			    const bool   colorShading_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ShadeImage( image(),
		colorShading_ == true ? MagickTrue : MagickFalse,
		azimuth_,
		elevation_,
		&exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Simulate an image shadow
void Magick::Image::shadow( const double percent_opacity_, const double sigma_,
  const ssize_t x_, const ssize_t y_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage = ShadowImage( image(), percent_opacity_, sigma_,
    x_, y_, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Sharpen pixels in image
void Magick::Image::sharpen ( const double radius_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    SharpenImage( image(),
                  radius_,
                  sigma_,
                  &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

void Magick::Image::sharpenChannel ( const ChannelType channel_,
                                     const double radius_, const double sigma_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    SharpenImageChannel( image(),
                         channel_,
                         radius_,
                         sigma_,
                         &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Shave pixels from image edges.
void Magick::Image::shave ( const Geometry &geometry_ )
{
  RectangleInfo shaveInfo = geometry_;
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ShaveImage( image(),
	       &shaveInfo,
	       &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Shear image
void Magick::Image::shear ( const double xShearAngle_,
			    const double yShearAngle_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ShearImage( image(),
		xShearAngle_,
		yShearAngle_,
		&exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Contrast image
void Magick::Image::sigmoidalContrast ( const size_t sharpen_, const double contrast, const double midpoint )
{
  modifyImage();
  (void) SigmoidalContrastImageChannel( image(), DefaultChannels, (MagickBooleanType) sharpen_, contrast, midpoint );
  throwImageException();
}

// Solarize image (similar to effect seen when exposing a photographic
// film to light during the development process)
void Magick::Image::solarize ( const double factor_ )
{
  modifyImage();
  SolarizeImage ( image(), factor_ );
  throwImageException();
}

// Sparse color image, given a set of coordinates, interpolates the colors
// found at those coordinates, across the whole image, using various methods.
//
void Magick::Image::sparseColor ( const ChannelType channel,
                                  const SparseColorMethod method,
                                  const size_t number_arguments,
                                  const double *arguments )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage = SparseColorImage ( image(), channel, method,
    number_arguments, arguments, &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Spread pixels randomly within image by specified ammount
void Magick::Image::spread ( const size_t amount_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    SpreadImage( image(),
		 amount_,
		 &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Add a digital watermark to the image (based on second image)
void Magick::Image::stegano ( const Image &watermark_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    SteganoImage( image(),
		  watermark_.constImage(),
		  &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Stereo image (left image is current image)
void Magick::Image::stereo ( const Image &rightImage_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    StereoImage( image(),
		 rightImage_.constImage(),
		 &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Swirl image
void Magick::Image::swirl ( const double degrees_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    SwirlImage( image(), degrees_,
		&exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Texture image
void Magick::Image::texture ( const Image &texture_ )
{
  modifyImage();
  TextureImage( image(), texture_.constImage() );
  throwImageException();
}

// Threshold image
void Magick::Image::threshold ( const double threshold_ )
{
  modifyImage();
  BilevelImage( image(), threshold_ );
  throwImageException();
}

// Transform image based on image geometry only
void Magick::Image::transform ( const Geometry &imageGeometry_ )
{
  modifyImage();
  TransformImage ( &(image()), 0,
		   std::string(imageGeometry_).c_str() );
  throwImageException();
}
// Transform image based on image and crop geometries
void Magick::Image::transform ( const Geometry &imageGeometry_,
				const Geometry &cropGeometry_ )
{
  modifyImage();
  TransformImage ( &(image()), std::string(cropGeometry_).c_str(),
		   std::string(imageGeometry_).c_str() );
  throwImageException();
}

// Add matte image to image, setting pixels matching color to transparent
void Magick::Image::transparent ( const Color &color_ )
{
  if ( !color_.isValid() )
  {
    throwExceptionExplicit( OptionError,
			    "Color argument is invalid" );
  }

  std::string color = color_;

  MagickPixelPacket target;
  (void) QueryMagickColor(std::string(color_).c_str(),&target,&image()->exception);
  modifyImage();
  TransparentPaintImage ( image(), &target, TransparentOpacity, MagickFalse );
  throwImageException();
}

// Add matte image to image, setting pixels matching color to transparent
void Magick::Image::transparentChroma(const Color &colorLow_,
  const Color &colorHigh_)
{
  if ( !colorLow_.isValid() || !colorHigh_.isValid() )
  {
    throwExceptionExplicit( OptionError,
			    "Color argument is invalid" );
  }

  std::string colorLow = colorLow_;
  std::string colorHigh = colorHigh_;

  MagickPixelPacket targetLow;
  MagickPixelPacket targetHigh;
  (void) QueryMagickColor(std::string(colorLow_).c_str(),&targetLow,
    &image()->exception);
  (void) QueryMagickColor(std::string(colorHigh_).c_str(),&targetHigh,
    &image()->exception);
  modifyImage();
  TransparentPaintImageChroma ( image(), &targetLow, &targetHigh,
    TransparentOpacity, MagickFalse );
  throwImageException();
}


// Trim edges that are the background color from the image
void Magick::Image::trim ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    TrimImage( image(), &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Replace image with a sharpened version of the original image
// using the unsharp mask algorithm.
//  radius_
//    the radius of the Gaussian, in pixels, not counting the
//    center pixel.
//  sigma_
//    the standard deviation of the Gaussian, in pixels.
//  amount_
//    the percentage of the difference between the original and
//    the blur image that is added back into the original.
// threshold_
//   the threshold in pixels needed to apply the diffence amount.
void Magick::Image::unsharpmask ( const double radius_,
                                  const double sigma_,
                                  const double amount_,
                                  const double threshold_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    UnsharpMaskImage( image(),
                      radius_,
                      sigma_,
                      amount_,
                      threshold_,
                      &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

void Magick::Image::unsharpmaskChannel ( const ChannelType channel_,
                                         const double radius_,
                                         const double sigma_,
                                         const double amount_,
                                         const double threshold_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    UnsharpMaskImageChannel( image(),
                             channel_,
                             radius_,
                             sigma_,
                             amount_,
                             threshold_,
                             &exceptionInfo );
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Map image pixels to a sine wave
void Magick::Image::wave ( const double amplitude_, const double wavelength_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    WaveImage( image(),
	       amplitude_,
	       wavelength_,
	       &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Write image to file
void Magick::Image::write( const std::string &imageSpec_ )
{
  modifyImage();
  fileName( imageSpec_ );
  WriteImage( imageInfo(), image() );
  throwImageException();
}

// Write image to in-memory BLOB
void Magick::Image::write ( Blob *blob_ )
{
  modifyImage();
  size_t length = 2048; // Efficient size for small images
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  void* data = ImageToBlob( imageInfo(),
			    image(),
			    &length,
			    &exceptionInfo);
  throwException( exceptionInfo );
  blob_->updateNoCopy( data, length, Blob::MallocAllocator );
  throwImageException();
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::write ( Blob *blob_,
			    const std::string &magick_ )
{
  modifyImage();
  magick(magick_);
  size_t length = 2048; // Efficient size for small images
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  void* data = ImageToBlob( imageInfo(),
			    image(),
			    &length,
			    &exceptionInfo);
  throwException( exceptionInfo );
  blob_->updateNoCopy( data, length, Blob::MallocAllocator );
  throwImageException();
  (void) DestroyExceptionInfo( &exceptionInfo );
}
void Magick::Image::write ( Blob *blob_,
			    const std::string &magick_,
			    const size_t depth_ )
{
  modifyImage();
  magick(magick_);
  depth(depth_);
  size_t length = 2048; // Efficient size for small images
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  void* data = ImageToBlob( imageInfo(),
			    image(),
			    &length,
			    &exceptionInfo);
  throwException( exceptionInfo );
  blob_->updateNoCopy( data, length, Blob::MallocAllocator );
  throwImageException();
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Write image to an array of pixels with storage type specified
// by user (ExportImagePixels), e.g.
// image.write( 0, 0, 640, 1, "RGB", 0, pixels );
void Magick::Image::write ( const ssize_t x_,
                            const ssize_t y_,
                            const size_t columns_,
                            const size_t rows_,
                            const std::string &map_,
                            const StorageType type_,
                            void *pixels_ )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  ExportImagePixels( image(), x_, y_, columns_, rows_, map_.c_str(), type_,
                 pixels_,
    &exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Zoom image
void Magick::Image::zoom( const Geometry &geometry_ )
{
  // Calculate new size.  This code should be supported using binary arguments
  // in the ImageMagick library.
  ssize_t x = 0;
  ssize_t y = 0;
  size_t width = columns();
  size_t height = rows();

  ParseMetaGeometry (static_cast<std::string>(geometry_).c_str(),
                     &x, &y,
                     &width, &height );

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    ResizeImage( image(),
               width,
               height,
               image()->filter,
               image()->blur,
               &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

/*
 * Methods for setting image attributes
 *
 */

// Join images into a single multi-image file
void Magick::Image::adjoin ( const bool flag_ )
{
  modifyImage();
  options()->adjoin( flag_ );
}
bool Magick::Image::adjoin ( void ) const
{
  return constOptions()->adjoin();
}

// Remove pixel aliasing
void Magick::Image::antiAlias( const bool flag_ )
{
  modifyImage();
  options()->antiAlias( static_cast<size_t>(flag_) );
}
bool Magick::Image::antiAlias( void )
{
  return static_cast<bool>( options()->antiAlias( ) );
}

// Animation inter-frame delay
void Magick::Image::animationDelay ( const size_t delay_ )
{
  modifyImage();
  image()->delay = delay_;
}
size_t Magick::Image::animationDelay ( void ) const
{
  return constImage()->delay;
}

// Number of iterations to play animation
void Magick::Image::animationIterations ( const size_t iterations_ )
{
  modifyImage();
  image()->iterations = iterations_;
}
size_t Magick::Image::animationIterations ( void ) const
{
  return constImage()->iterations;
}

// Access/Update a named image attribute
void Magick::Image::attribute ( const std::string name_,
                                const std::string value_ )
{
  modifyImage();
  SetImageProperty( image(), name_.c_str(), value_.c_str() );
}
std::string Magick::Image::attribute ( const std::string name_ )
{
  const char *value = GetImageProperty( constImage(), name_.c_str() );

  if ( value )
    return std::string( value );

  return std::string(); // Intentionally no exception
}

// Background color
void Magick::Image::backgroundColor ( const Color &backgroundColor_ )
{
  modifyImage();

  if ( backgroundColor_.isValid() )
    {
      image()->background_color = backgroundColor_;
    }
  else
    {
      image()->background_color = Color();
    }

  options()->backgroundColor( backgroundColor_ );
}
Magick::Color Magick::Image::backgroundColor ( void ) const
{
  return constOptions()->backgroundColor( );
}

// Background fill texture
void Magick::Image::backgroundTexture ( const std::string &backgroundTexture_ )
{
  modifyImage();
  options()->backgroundTexture( backgroundTexture_ );
}
std::string Magick::Image::backgroundTexture ( void ) const
{
  return constOptions()->backgroundTexture( );
}

// Original image columns
size_t Magick::Image::baseColumns ( void ) const
{
  return constImage()->magick_columns;
}

// Original image name
std::string Magick::Image::baseFilename ( void ) const
{
  return std::string(constImage()->magick_filename);
}

// Original image rows
size_t Magick::Image::baseRows ( void ) const
{
  return constImage()->magick_rows;
}

// Border color
void Magick::Image::borderColor ( const Color &borderColor_ )
{
  modifyImage();

  if ( borderColor_.isValid() )
    {
      image()->border_color = borderColor_;
    }
  else
    {
      image()->border_color = Color();
    }

  options()->borderColor( borderColor_ );
}
Magick::Color Magick::Image::borderColor ( void ) const
{
  return constOptions()->borderColor( );
}

// Return smallest bounding box enclosing non-border pixels. The
// current fuzz value is used when discriminating between pixels.
// This is the crop bounding box used by crop(Geometry(0,0));
Magick::Geometry Magick::Image::boundingBox ( void ) const
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  RectangleInfo bbox = GetImageBoundingBox( constImage(), &exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  return Geometry( bbox );
}

// Text bounding-box base color
void Magick::Image::boxColor ( const Color &boxColor_ )
{
  modifyImage();
  options()->boxColor( boxColor_ );
}
Magick::Color Magick::Image::boxColor ( void ) const
{
  return constOptions()->boxColor( );
}

// Pixel cache threshold.  Once this threshold is exceeded, all
// subsequent pixels cache operations are to/from disk.
// This setting is shared by all Image objects.
/* static */
void Magick::Image::cacheThreshold ( const size_t threshold_ )
{
  SetMagickResourceLimit( MemoryResource, threshold_ );
}

void Magick::Image::chromaBluePrimary ( const double x_, const double y_ )
{
  modifyImage();
  image()->chromaticity.blue_primary.x = x_;
  image()->chromaticity.blue_primary.y = y_;
}
void Magick::Image::chromaBluePrimary ( double *x_, double *y_ ) const
{
  *x_ = constImage()->chromaticity.blue_primary.x;
  *y_ = constImage()->chromaticity.blue_primary.y;
}

void Magick::Image::chromaGreenPrimary ( const double x_, const double y_ )
{
  modifyImage();
  image()->chromaticity.green_primary.x = x_;
  image()->chromaticity.green_primary.y = y_;
}
void Magick::Image::chromaGreenPrimary ( double *x_, double *y_ ) const
{
  *x_ = constImage()->chromaticity.green_primary.x;
  *y_ = constImage()->chromaticity.green_primary.y;
}

void Magick::Image::chromaRedPrimary ( const double x_, const double y_ )
{
  modifyImage();
  image()->chromaticity.red_primary.x = x_;
  image()->chromaticity.red_primary.y = y_;
}
void Magick::Image::chromaRedPrimary ( double *x_, double *y_ ) const
{
  *x_ = constImage()->chromaticity.red_primary.x;
  *y_ = constImage()->chromaticity.red_primary.y;
}

void Magick::Image::chromaWhitePoint ( const double x_, const double y_ )
{
  modifyImage();
  image()->chromaticity.white_point.x = x_;
  image()->chromaticity.white_point.y = y_;
}
void Magick::Image::chromaWhitePoint ( double *x_, double *y_ ) const
{
  *x_ = constImage()->chromaticity.white_point.x;
  *y_ = constImage()->chromaticity.white_point.y;
}

// Set image storage class
void Magick::Image::classType ( const ClassType class_ )
{
  if ( classType() == PseudoClass && class_ == DirectClass )
    {
      // Use SyncImage to synchronize the DirectClass pixels with the
      // color map and then set to DirectClass type.
      modifyImage();
      SyncImage( image() );
      image()->colormap = (PixelPacket *)
        RelinquishMagickMemory( image()->colormap );
      image()->storage_class = static_cast<MagickCore::ClassType>(DirectClass);
      return;
    }

  if ( classType() == DirectClass && class_ == PseudoClass )
    {
      // Quantize to create PseudoClass color map
      modifyImage();
      quantizeColors(MaxColormapSize);
      quantize();
      image()->storage_class = static_cast<MagickCore::ClassType>(PseudoClass);
    }
}

// Associate a clip mask with the image. The clip mask must be the
// same dimensions as the image. Pass an invalid image to unset an
// existing clip mask.
void Magick::Image::clipMask ( const Magick::Image & clipMask_ )
{
  modifyImage();

  if( clipMask_.isValid() )
    {
      // Set clip mask
      SetImageClipMask( image(), clipMask_.constImage() );
    }
  else
    {
      // Unset existing clip mask
      SetImageClipMask( image(), 0 );
    }
}
Magick::Image Magick::Image::clipMask ( void  ) const
{
      ExceptionInfo exceptionInfo;
      GetExceptionInfo( &exceptionInfo );
      MagickCore::Image* image =
        GetImageClipMask( constImage(), &exceptionInfo );
      throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
      return Magick::Image( image );
}

void Magick::Image::colorFuzz ( const double fuzz_ )
{
  modifyImage();
  image()->fuzz = fuzz_;
  options()->colorFuzz( fuzz_ );
}
double Magick::Image::colorFuzz ( void ) const
{
  return constOptions()->colorFuzz( );
}

// Set color in colormap at index
void Magick::Image::colorMap ( const size_t index_,
			       const Color &color_ )
{
  MagickCore::Image* imageptr = image();

  if (index_ > (MaxColormapSize-1) )
    throwExceptionExplicit( OptionError,
                            "Colormap index must be less than MaxColormapSize" );
  
  if ( !color_.isValid() )
    throwExceptionExplicit( OptionError,
			    "Color argument is invalid");
  modifyImage();

  // Ensure that colormap size is large enough
  if ( colorMapSize() < (index_+1) )
    colorMapSize( index_ + 1 );

  // Set color at index in colormap
  (imageptr->colormap)[index_] = color_;
}
// Return color in colormap at index
Magick::Color Magick::Image::colorMap ( const size_t index_ ) const
{
  const MagickCore::Image* imageptr = constImage();

  if ( !imageptr->colormap )
    throwExceptionExplicit( OptionError,
			    "Image does not contain a colormap");

  if ( index_ > imageptr->colors-1 )
    throwExceptionExplicit( OptionError,
			    "Index out of range");

  return Magick::Color( (imageptr->colormap)[index_] );
}

// Colormap size (number of colormap entries)
void Magick::Image::colorMapSize ( const size_t entries_ )
{
  if (entries_ >MaxColormapSize )
    throwExceptionExplicit( OptionError,
                            "Colormap entries must not exceed MaxColormapSize" );

  modifyImage();

  MagickCore::Image* imageptr = image();

  if( !imageptr->colormap )
    {
      // Allocate colormap
      imageptr->colormap =
        static_cast<PixelPacket*>(AcquireMagickMemory(entries_*sizeof(PixelPacket)));
      imageptr->colors = 0;
    }
  else if ( entries_ > imageptr->colors )
    {
      // Re-allocate colormap
      imageptr->colormap=(PixelPacket *)
        ResizeMagickMemory(imageptr->colormap,(entries_)*sizeof(PixelPacket));
    }

  // Initialize any new colormap entries as all black
  Color black(0,0,0);
  for( size_t i=imageptr->colors; i<(entries_-1); i++ )
    (imageptr->colormap)[i] = black;

  imageptr->colors = entries_;
}
size_t Magick::Image::colorMapSize ( void )
{
  const MagickCore::Image* imageptr = constImage();

  if ( !imageptr->colormap )
    throwExceptionExplicit( OptionError,
			    "Image does not contain a colormap");

  return imageptr->colors;
}

// Image colorspace
void Magick::Image::colorSpace( const ColorspaceType colorSpace_ )
{
  // Nothing to do?
  if ( image()->colorspace == colorSpace_ )
    return;

  modifyImage();
  TransformImageColorspace( image(), colorSpace_ );
  throwImageException();
}
Magick::ColorspaceType Magick::Image::colorSpace ( void ) const
{
  return constImage()->colorspace;
}

// Set image colorspace type.
void Magick::Image::colorspaceType( const ColorspaceType colorSpace_ )
{
  modifyImage();
  SetImageColorspace( image(), colorSpace_ );
  throwImageException();
  options()->colorspaceType( colorSpace_ );
}
Magick::ColorspaceType Magick::Image::colorspaceType ( void ) const
{
  return constOptions()->colorspaceType();
}


// Comment string
void Magick::Image::comment ( const std::string &comment_ )
{
  modifyImage();
  SetImageProperty( image(), "Comment", NULL );
  if ( comment_.length() > 0 )
    SetImageProperty( image(), "Comment", comment_.c_str() );
  throwImageException();
}
std::string Magick::Image::comment ( void ) const
{
  const char *value = GetImageProperty( constImage(), "Comment" );

  if ( value )
    return std::string( value );

  return std::string(); // Intentionally no exception
}

// Composition operator to be used when composition is implicitly used
// (such as for image flattening).
void Magick::Image::compose (const CompositeOperator compose_)
{
  image()->compose=compose_;
}

Magick::CompositeOperator Magick::Image::compose ( void ) const
{
  return constImage()->compose;
}

// Compression algorithm
void Magick::Image::compressType ( const CompressionType compressType_ )
{
  modifyImage();
  image()->compression = compressType_;
  options()->compressType( compressType_ );
}
Magick::CompressionType Magick::Image::compressType ( void ) const
{
  return constImage()->compression;
}

// Enable printing of debug messages from ImageMagick
void Magick::Image::debug ( const bool flag_ )
{
  modifyImage();
  options()->debug( flag_ );
}
bool Magick::Image::debug ( void ) const
{
  return constOptions()->debug();
}

// Tagged image format define (set/access coder-specific option) The
// magick_ option specifies the coder the define applies to.  The key_
// option provides the key specific to that coder.  The value_ option
// provides the value to set (if any). See the defineSet() method if the
// key must be removed entirely.
void Magick::Image::defineValue ( const std::string &magick_,
                                  const std::string &key_,
                                  const std::string &value_ )
{
  modifyImage();
  std::string format = magick_ + ":" + key_;
  std::string option = value_;
  (void) SetImageOption ( imageInfo(), format.c_str(), option.c_str() );
}
std::string Magick::Image::defineValue ( const std::string &magick_,
                                         const std::string &key_ ) const
{
  std::string definition = magick_ + ":" + key_;
  const char *option =
    GetImageOption ( constImageInfo(), definition.c_str() );
  if (option)
    return std::string( option );
  return std::string( );
}

// Tagged image format define. Similar to the defineValue() method
// except that passing the flag_ value 'true' creates a value-less
// define with that format and key. Passing the flag_ value 'false'
// removes any existing matching definition. The method returns 'true'
// if a matching key exists, and 'false' if no matching key exists.
void Magick::Image::defineSet ( const std::string &magick_,
                                const std::string &key_,
                                bool flag_ )
{
  modifyImage();
  std::string definition = magick_ + ":" + key_;
  if (flag_)
    {
      (void) SetImageOption ( imageInfo(), definition.c_str(),  "" );
    }
  else
    {
      DeleteImageOption( imageInfo(), definition.c_str() );
    }
}
bool Magick::Image::defineSet ( const std::string &magick_,
                                const std::string &key_ ) const
{
  std::string key = magick_ + ":" + key_;
  const char *option =
    GetImageOption ( constImageInfo(), key.c_str() );
  if (option)
    return true;
  return false;
}

// Pixel resolution
void Magick::Image::density ( const Geometry &density_ )
{
  modifyImage();
  options()->density( density_ );
  if ( density_.isValid() )
    {
      image()->x_resolution = density_.width();
      if ( density_.height() != 0 )
        {
          image()->y_resolution = density_.height();
        }
      else
        {
          image()->y_resolution = density_.width();
        }
    }
  else
    {
      // Reset to default
      image()->x_resolution = 0;
      image()->y_resolution = 0;
    }
}
Magick::Geometry Magick::Image::density ( void ) const
{
  if (isValid())
    {
      ssize_t x_resolution=72;
      ssize_t y_resolution=72;

      if (constImage()->x_resolution > 0.0)
        x_resolution=static_cast<ssize_t>(constImage()->x_resolution + 0.5);

      if (constImage()->y_resolution > 0.0)
        y_resolution=static_cast<ssize_t>(constImage()->y_resolution + 0.5);

      return Geometry(x_resolution,y_resolution);
    }

  return constOptions()->density( );
}

// Image depth (bits allocated to red/green/blue components)
void Magick::Image::depth ( const size_t depth_ )
{
  size_t depth = depth_;

  if (depth > MAGICKCORE_QUANTUM_DEPTH)
    depth=MAGICKCORE_QUANTUM_DEPTH;

  modifyImage();
  image()->depth=depth;
  options()->depth( depth );
}
size_t Magick::Image::depth ( void ) const
{
  return constImage()->depth;
}

std::string Magick::Image::directory ( void ) const
{
  if ( constImage()->directory )
    return std::string( constImage()->directory );

  throwExceptionExplicit( CorruptImageWarning,
			  "Image does not contain a directory");

  return std::string();
}

// Endianness (little like Intel or big like SPARC) for image
// formats which support endian-specific options.
void Magick::Image::endian ( const Magick::EndianType endian_ )
{
  modifyImage();
  options()->endian( endian_ );
  image()->endian = endian_;
}
Magick::EndianType Magick::Image::endian ( void ) const
{
  return constImage()->endian;
}

// EXIF profile (BLOB)
void Magick::Image::exifProfile( const Magick::Blob &exifProfile_ )
{
  modifyImage();
  if ( exifProfile_.data() != 0 )
    {
      StringInfo * exif_profile = AcquireStringInfo( exifProfile_.length() );
      SetStringInfoDatum(exif_profile ,(unsigned char *) exifProfile_.data());
      (void) SetImageProfile( image(), "exif", exif_profile);
      exif_profile =DestroyStringInfo( exif_profile );
    }
}
Magick::Blob Magick::Image::exifProfile( void ) const
{
  const StringInfo * exif_profile = GetImageProfile( constImage(), "exif" );
  if ( exif_profile == (StringInfo *) NULL)
    return Blob( 0, 0 );
  return Blob(GetStringInfoDatum(exif_profile),GetStringInfoLength(exif_profile));
} 

// Image file name
void Magick::Image::fileName ( const std::string &fileName_ )
{
  modifyImage();

  fileName_.copy( image()->filename,
		  sizeof(image()->filename) - 1 );
  image()->filename[ fileName_.length() ] = 0; // Null terminate
  
  options()->fileName( fileName_ );
  
}
std::string Magick::Image::fileName ( void ) const
{
  return constOptions()->fileName( );
}

// Image file size
off_t Magick::Image::fileSize ( void ) const
{
  return (off_t) GetBlobSize( constImage() );
}

// Color to use when drawing inside an object
void Magick::Image::fillColor ( const Magick::Color &fillColor_ )
{
  modifyImage();
  options()->fillColor(fillColor_);
}
Magick::Color Magick::Image::fillColor ( void ) const
{
  return constOptions()->fillColor();
}

// Rule to use when filling drawn objects
void Magick::Image::fillRule ( const Magick::FillRule &fillRule_ )
{
  modifyImage();
  options()->fillRule(fillRule_);
}
Magick::FillRule Magick::Image::fillRule ( void ) const
{
  return constOptions()->fillRule();
}

// Pattern to use while filling drawn objects.
void Magick::Image::fillPattern ( const Image &fillPattern_ )
{
  modifyImage();
  if(fillPattern_.isValid())
    options()->fillPattern( fillPattern_.constImage() );
  else
    options()->fillPattern( static_cast<MagickCore::Image*>(NULL) );
}
Magick::Image  Magick::Image::fillPattern ( void  ) const
{
  // FIXME: This is inordinately innefficient
  Image texture;
  
  const MagickCore::Image* tmpTexture = constOptions()->fillPattern( );

  if ( tmpTexture )
    {
      ExceptionInfo exceptionInfo;
      GetExceptionInfo( &exceptionInfo );
      MagickCore::Image* image =
	CloneImage( tmpTexture,
                    0, // columns
                    0, // rows
                    MagickTrue, // orphan
                    &exceptionInfo);
      texture.replaceImage( image );
      throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
    }
  return texture;
}

// Filter used by zoom
void Magick::Image::filterType ( const Magick::FilterTypes filterType_ )
{
  modifyImage();
  image()->filter = filterType_;
}
Magick::FilterTypes Magick::Image::filterType ( void ) const
{
  return constImage()->filter;
}

// Font name
void Magick::Image::font ( const std::string &font_ )
{
  modifyImage();
  options()->font( font_ );
}
std::string Magick::Image::font ( void ) const
{
  return constOptions()->font( );
}

// Font point size
void Magick::Image::fontPointsize ( const double pointSize_ )
{
  modifyImage();
  options()->fontPointsize( pointSize_ );
}
double Magick::Image::fontPointsize ( void ) const
{
  return constOptions()->fontPointsize( );
}

// Font type metrics
void Magick::Image::fontTypeMetrics( const std::string &text_,
                                     TypeMetric *metrics )
{
  DrawInfo *drawInfo = options()->drawInfo();
  drawInfo->text = const_cast<char *>(text_.c_str());
  GetTypeMetrics( image(), drawInfo, &(metrics->_typeMetric) );
  drawInfo->text = 0;
}

// Image format string
std::string Magick::Image::format ( void ) const
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  const MagickInfo * magick_info
    = GetMagickInfo( constImage()->magick, &exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );

  if (( magick_info != 0 ) && 
      ( *magick_info->description != '\0' ))
    return std::string(magick_info->description);

  throwExceptionExplicit( CorruptImageWarning,
			  "Unrecognized image magick type" );
  return std::string();
}

// Gamma adjustment
double Magick::Image::gamma ( void ) const
{
  return constImage()->gamma;
}

Magick::Geometry Magick::Image::geometry ( void ) const
{
  if ( constImage()->geometry )
  {
    return Geometry(constImage()->geometry);
  }

  throwExceptionExplicit( OptionWarning,
			  "Image does not contain a geometry");

  return Geometry();
}

void Magick::Image::gifDisposeMethod ( const size_t disposeMethod_ )
{
  modifyImage();
  image()->dispose = (DisposeType) disposeMethod_;
}
size_t Magick::Image::gifDisposeMethod ( void ) const
{
  // FIXME: It would be better to return an enumeration
  return constImage()->dispose;
}

// ICC ICM color profile (BLOB)
void Magick::Image::iccColorProfile( const Magick::Blob &colorProfile_ )
{
  profile("icm",colorProfile_);
}
Magick::Blob Magick::Image::iccColorProfile( void ) const
{
  const StringInfo * color_profile = GetImageProfile( constImage(), "icc" );
  if ( color_profile == (StringInfo *) NULL)
    return Blob( 0, 0 );
  return Blob( GetStringInfoDatum(color_profile), GetStringInfoLength(color_profile) );
}

void Magick::Image::interlaceType ( const Magick::InterlaceType interlace_ )
{
  modifyImage();
  image()->interlace = interlace_;
  options()->interlaceType ( interlace_ );
}
Magick::InterlaceType Magick::Image::interlaceType ( void ) const
{
  return constImage()->interlace;
}

// IPTC profile (BLOB)
void Magick::Image::iptcProfile( const Magick::Blob &iptcProfile_ )
{
  modifyImage();
  if (  iptcProfile_.data() != 0 )
    {
      StringInfo * iptc_profile = AcquireStringInfo( iptcProfile_.length() );
      SetStringInfoDatum(iptc_profile ,(unsigned char *) iptcProfile_.data());
      (void) SetImageProfile( image(), "iptc", iptc_profile);
       iptc_profile =DestroyStringInfo( iptc_profile );
    }
}
Magick::Blob Magick::Image::iptcProfile( void ) const
{
  const StringInfo * iptc_profile = GetImageProfile( constImage(), "iptc" );
  if ( iptc_profile == (StringInfo *) NULL)
    return Blob( 0, 0 );
  return Blob( GetStringInfoDatum(iptc_profile), GetStringInfoLength(iptc_profile));
}

// Does object contain valid image?
void Magick::Image::isValid ( const bool isValid_ )
{
  if ( !isValid_ )
    {
      delete _imgRef;
      _imgRef = new ImageRef;
    }
  else if ( !isValid() )
    {
      // Construct with single-pixel black image to make
      // image valid.  This is an obvious hack.
      size( Geometry(1,1) );
      read( "xc:#000000" );
    }
}

bool Magick::Image::isValid ( void ) const
{
  if ( rows() && columns() )
    return true;

  return false;
}

// Label image
void Magick::Image::label ( const std::string &label_ )
{
  modifyImage();
  SetImageProperty ( image(), "Label", NULL );
  if ( label_.length() > 0 )
    SetImageProperty ( image(), "Label", label_.c_str() );
  throwImageException();
}
std::string Magick::Image::label ( void ) const
{
  const char *value = GetImageProperty( constImage(), "Label" );

  if ( value )
    return std::string( value );

  return std::string();
}

void Magick::Image::magick ( const std::string &magick_ )
{
  modifyImage();

  magick_.copy( image()->magick,
		sizeof(image()->magick) - 1 );
  image()->magick[ magick_.length() ] = 0;
  
  options()->magick( magick_ );
}
std::string Magick::Image::magick ( void ) const
{
  if ( *(constImage()->magick) != '\0' )
    return std::string(constImage()->magick);

  return constOptions()->magick( );
}

void Magick::Image::matte ( const bool matteFlag_ )
{
  modifyImage();

  // If matte channel is requested, but image doesn't already have a
  // matte channel, then create an opaque matte channel.  Likewise, if
  // the image already has a matte channel but a matte channel is not
  // desired, then set the matte channel to opaque.
  if ((matteFlag_ && !constImage()->matte) ||
      (constImage()->matte && !matteFlag_))
    SetImageOpacity(image(),OpaqueOpacity);

  image()->matte = (MagickBooleanType) matteFlag_;
}
bool Magick::Image::matte ( void ) const
{
  if ( constImage()->matte )
    return true;
  else
    return false;
}

void Magick::Image::matteColor ( const Color &matteColor_ )
{
  modifyImage();
  
  if ( matteColor_.isValid() )
    {
      image()->matte_color = matteColor_;
      options()->matteColor( matteColor_ );
    }
  else
    {
      // Set to default matte color
      Color tmpColor( "#BDBDBD" );
      image()->matte_color = tmpColor;
      options()->matteColor( tmpColor );
    }
}
Magick::Color Magick::Image::matteColor ( void ) const
{
  return Color( constImage()->matte_color.red,
		constImage()->matte_color.green,
		constImage()->matte_color.blue );
}

double Magick::Image::meanErrorPerPixel ( void ) const
{
  return(constImage()->error.mean_error_per_pixel);
}

// Image modulus depth (minimum number of bits required to support
// red/green/blue components without loss of accuracy)
void Magick::Image::modulusDepth ( const size_t depth_ )
{
  modifyImage();
  SetImageDepth( image(), depth_ );
  options()->depth( depth_ );
}
size_t Magick::Image::modulusDepth ( void ) const
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  size_t depth=GetImageDepth( constImage(), &exceptionInfo );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  return depth;
}

void Magick::Image::monochrome ( const bool monochromeFlag_ )
{
  modifyImage();
  options()->monochrome( monochromeFlag_ );
}
bool Magick::Image::monochrome ( void ) const
{
  return constOptions()->monochrome( );
}

Magick::Geometry Magick::Image::montageGeometry ( void ) const
{
  if ( constImage()->montage )
    return Magick::Geometry(constImage()->montage);

  throwExceptionExplicit( CorruptImageWarning,
			  "Image does not contain a montage" );

  return Magick::Geometry();
}

double Magick::Image::normalizedMaxError ( void ) const
{
  return(constImage()->error.normalized_maximum_error);
}

double Magick::Image::normalizedMeanError ( void ) const
{
  return constImage()->error.normalized_mean_error;
}

// Image orientation
void Magick::Image::orientation ( const Magick::OrientationType orientation_ )
{
  modifyImage();
  image()->orientation = orientation_;
}
Magick::OrientationType Magick::Image::orientation ( void ) const
{
  return constImage()->orientation;
}

void Magick::Image::penColor ( const Color &penColor_ )
{
  modifyImage();
  options()->fillColor(penColor_);
  options()->strokeColor(penColor_);
}
Magick::Color Magick::Image::penColor ( void  ) const
{
  return constOptions()->fillColor();
}

void Magick::Image::penTexture ( const Image &penTexture_ )
{
  modifyImage();
  if(penTexture_.isValid())
    options()->fillPattern( penTexture_.constImage() );
  else
    options()->fillPattern( static_cast<MagickCore::Image*>(NULL) );
}

Magick::Image  Magick::Image::penTexture ( void  ) const
{
  // FIXME: This is inordinately innefficient
  Image texture;
  
  const MagickCore::Image* tmpTexture = constOptions()->fillPattern( );

  if ( tmpTexture )
    {
      ExceptionInfo exceptionInfo;
      GetExceptionInfo( &exceptionInfo );
      MagickCore::Image* image =
	CloneImage( tmpTexture,
                    0, // columns
                    0, // rows
                    MagickTrue, // orphan
                    &exceptionInfo);
      texture.replaceImage( image );
      throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
    }
  return texture;
}

// Set the color of a pixel.
void Magick::Image::pixelColor ( const ssize_t x_, const ssize_t y_,
				 const Color &color_ )
{
  // Test arguments to ensure they are within the image.
  if ( y_ > (ssize_t) rows() || x_ > (ssize_t) columns() )
    throwExceptionExplicit( OptionError,
	    "Access outside of image boundary" );
      
  modifyImage();

  // Set image to DirectClass
  classType( DirectClass );

  // Get pixel view
  Pixels pixels(*this);
    // Set pixel value
  *(pixels.get(x_, y_, 1, 1 )) = color_;
  // Tell ImageMagick that pixels have been updated
  pixels.sync();

  return;
}

// Get the color of a pixel
Magick::Color Magick::Image::pixelColor ( const ssize_t x_,
					  const ssize_t y_ ) const
{
  ClassType storage_class;
  storage_class = classType();
  // DirectClass
  const PixelPacket* pixel = getConstPixels( x_, y_, 1, 1 );
  if ( storage_class == DirectClass )
    {
      if ( pixel )
        return Color( *pixel );
    }

  // PseudoClass
  if ( storage_class == PseudoClass )
    {
      const IndexPacket* indexes = getConstIndexes();
      if ( indexes )
        return colorMap( (size_t) *indexes );
    }

  return Color(); // invalid
}

// Preferred size and location of an image canvas.
void Magick::Image::page ( const Magick::Geometry &pageSize_ )
{
  modifyImage();
  options()->page( pageSize_ );
  image()->page = pageSize_;
}
Magick::Geometry Magick::Image::page ( void ) const
{
  return Geometry( constImage()->page.width,
		   constImage()->page.height,
		   AbsoluteValue(constImage()->page.x),
		   AbsoluteValue(constImage()->page.y),
                   constImage()->page.x < 0 ? true : false,
                   constImage()->page.y < 0 ? true : false);
}

// Add a named profile to an image or remove a named profile by
// passing an empty Blob (use default Blob constructor).
// Valid names are:
// "*", "8BIM", "ICM", "IPTC", or a generic profile name.
void Magick::Image::profile( const std::string name_,
                             const Magick::Blob &profile_ )
{
  modifyImage();
  ssize_t result = ProfileImage( image(), name_.c_str(),
                             (unsigned char *)profile_.data(),
                             profile_.length(), MagickTrue);

  if( !result )
    throwImageException();
}

// Retrieve a named profile from the image.
// Valid names are:
// "8BIM", "8BIMTEXT", "APP1", "APP1JPEG", "ICC", "ICM", & "IPTC" or
// an existing generic profile name.
Magick::Blob Magick::Image::profile( const std::string name_ ) const
{
  const MagickCore::Image* image = constImage();
                                                                                
  const StringInfo * profile = GetImageProfile( image, name_.c_str() );
                                                                                
  if ( profile != (StringInfo *) NULL)
      return Blob( (void*) GetStringInfoDatum(profile), GetStringInfoLength(profile));
                                                                                
  Blob blob;
  Image temp_image = *this;
  temp_image.write( &blob, name_ );
  return blob;
}

void Magick::Image::quality ( const size_t quality_ )
{
  modifyImage();
  image()->quality = quality_;
  options()->quality( quality_ );
}
size_t Magick::Image::quality ( void ) const
{
  return constImage()->quality;
}

void Magick::Image::quantizeColors ( const size_t colors_ )
{
  modifyImage();
  options()->quantizeColors( colors_ );
}
size_t Magick::Image::quantizeColors ( void ) const
{
  return constOptions()->quantizeColors( );
}

void Magick::Image::quantizeColorSpace
  ( const Magick::ColorspaceType colorSpace_ )
{
  modifyImage();
  options()->quantizeColorSpace( colorSpace_ );
}
Magick::ColorspaceType Magick::Image::quantizeColorSpace ( void ) const
{
  return constOptions()->quantizeColorSpace( );
}

void Magick::Image::quantizeDither ( const bool ditherFlag_ )
{
  modifyImage();
  options()->quantizeDither( ditherFlag_ );
}
bool Magick::Image::quantizeDither ( void ) const
{
  return constOptions()->quantizeDither( );
}

void Magick::Image::quantizeTreeDepth ( const size_t treeDepth_ )
{
  modifyImage();
  options()->quantizeTreeDepth( treeDepth_ );
}
size_t Magick::Image::quantizeTreeDepth ( void ) const
{
  return constOptions()->quantizeTreeDepth( );
}

void Magick::Image::renderingIntent
  ( const Magick::RenderingIntent renderingIntent_ )
{
  modifyImage();
  image()->rendering_intent = renderingIntent_;
}
Magick::RenderingIntent Magick::Image::renderingIntent ( void ) const
{
  return static_cast<Magick::RenderingIntent>(constImage()->rendering_intent);
}

void Magick::Image::resolutionUnits
  ( const Magick::ResolutionType resolutionUnits_ )
{
  modifyImage();
  image()->units = resolutionUnits_;
  options()->resolutionUnits( resolutionUnits_ );
}
Magick::ResolutionType Magick::Image::resolutionUnits ( void ) const
{
  return constOptions()->resolutionUnits( );
}

void Magick::Image::scene ( const size_t scene_ )
{
  modifyImage();
  image()->scene = scene_;
}
size_t Magick::Image::scene ( void ) const
{
  return constImage()->scene;
}

std::string Magick::Image::signature ( const bool force_ ) const
{
  Lock( &_imgRef->_mutexLock );

  // Re-calculate image signature if necessary
  if ( force_ ||
       !GetImageProperty(constImage(), "Signature") ||
       constImage()->taint )
    {
      SignatureImage( const_cast<MagickCore::Image *>(constImage()) );
    }

  const char *property = GetImageProperty(constImage(), "Signature");

  return std::string( property );
}

void Magick::Image::size ( const Geometry &geometry_ )
{
  modifyImage();
  options()->size( geometry_ );
  image()->rows = geometry_.height();
  image()->columns = geometry_.width();
}
Magick::Geometry Magick::Image::size ( void ) const
{
  return Magick::Geometry( constImage()->columns, constImage()->rows );
}

// Splice image
void Magick::Image::splice( const Geometry &geometry_ )
{
  RectangleInfo spliceInfo = geometry_;
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  MagickCore::Image* newImage =
    SpliceImage( image(), &spliceInfo, &exceptionInfo);
  replaceImage( newImage );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Obtain image statistics. Statistics are normalized to the range of
// 0.0 to 1.0 and are output to the specified ImageStatistics
// structure.
void Magick::Image::statistics ( ImageStatistics *statistics ) const
{
  double
    maximum,
    minimum;

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  (void) GetImageChannelRange(constImage(),RedChannel,&minimum,&maximum,
    &exceptionInfo);
  statistics->red.minimum=minimum;
	statistics->red.maximum=maximum;
  (void) GetImageChannelMean(constImage(),RedChannel,
    &statistics->red.mean,&statistics->red.standard_deviation,&exceptionInfo);
  (void) GetImageChannelKurtosis(constImage(),RedChannel,
    &statistics->red.kurtosis,&statistics->red.skewness,&exceptionInfo);
  (void) GetImageChannelRange(constImage(),GreenChannel,&minimum,&maximum,
    &exceptionInfo);
  statistics->green.minimum=minimum;
	statistics->green.maximum=maximum;
  (void) GetImageChannelMean(constImage(),GreenChannel,
    &statistics->green.mean,&statistics->green.standard_deviation,
    &exceptionInfo);
  (void) GetImageChannelKurtosis(constImage(),GreenChannel,
    &statistics->green.kurtosis,&statistics->green.skewness,&exceptionInfo);
  (void) GetImageChannelRange(constImage(),BlueChannel,&minimum,&maximum,
    &exceptionInfo);
  statistics->blue.minimum=minimum;
	statistics->blue.maximum=maximum;
  (void) GetImageChannelMean(constImage(),BlueChannel,
    &statistics->blue.mean,&statistics->blue.standard_deviation,&exceptionInfo);
  (void) GetImageChannelKurtosis(constImage(),BlueChannel,
    &statistics->blue.kurtosis,&statistics->blue.skewness,&exceptionInfo);
  (void) GetImageChannelRange(constImage(),OpacityChannel,&minimum,&maximum,
    &exceptionInfo);
  statistics->opacity.minimum=minimum;
	statistics->opacity.maximum=maximum;
  (void) GetImageChannelMean(constImage(),OpacityChannel,
    &statistics->opacity.mean,&statistics->opacity.standard_deviation,
    &exceptionInfo);
  (void) GetImageChannelKurtosis(constImage(),OpacityChannel,
    &statistics->opacity.kurtosis,&statistics->opacity.skewness,&exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Strip strips an image of all profiles and comments.
void Magick::Image::strip ( void )
{
  modifyImage();
  StripImage( image() );
  throwImageException();
}

// enabled/disable stroke anti-aliasing
void Magick::Image::strokeAntiAlias ( const bool flag_ )
{
  modifyImage();
  options()->strokeAntiAlias(flag_);
}
bool Magick::Image::strokeAntiAlias ( void ) const
{
  return constOptions()->strokeAntiAlias();
}

// Color to use when drawing object outlines
void Magick::Image::strokeColor ( const Magick::Color &strokeColor_ )
{
  modifyImage();
  options()->strokeColor(strokeColor_);
}
Magick::Color Magick::Image::strokeColor ( void ) const
{
  return constOptions()->strokeColor();
}

// dash pattern for drawing vector objects (default one)
void Magick::Image::strokeDashArray ( const double* strokeDashArray_ )
{
  modifyImage();
  options()->strokeDashArray( strokeDashArray_ );
}

const double* Magick::Image::strokeDashArray ( void ) const
{
  return constOptions()->strokeDashArray( );
}

// dash offset for drawing vector objects (default one)
void Magick::Image::strokeDashOffset ( const double strokeDashOffset_ )
{
  modifyImage();
  options()->strokeDashOffset( strokeDashOffset_ );
}

double Magick::Image::strokeDashOffset ( void ) const
{
  return constOptions()->strokeDashOffset( );
}

// Specify the shape to be used at the end of open subpaths when they
// are stroked. Values of LineCap are UndefinedCap, ButtCap, RoundCap,
// and SquareCap.
void Magick::Image::strokeLineCap ( const Magick::LineCap lineCap_ )
{
  modifyImage();
  options()->strokeLineCap( lineCap_ );
}
Magick::LineCap Magick::Image::strokeLineCap ( void ) const
{
  return constOptions()->strokeLineCap( );
}

// Specify the shape to be used at the corners of paths (or other
// vector shapes) when they are stroked. Values of LineJoin are
// UndefinedJoin, MiterJoin, RoundJoin, and BevelJoin.
void Magick::Image::strokeLineJoin ( const Magick::LineJoin lineJoin_ )
{
  modifyImage();
  options()->strokeLineJoin( lineJoin_ );
}
Magick::LineJoin Magick::Image::strokeLineJoin ( void ) const
{
  return constOptions()->strokeLineJoin( );
}

// Specify miter limit. When two line segments meet at a sharp angle
// and miter joins have been specified for 'lineJoin', it is possible
// for the miter to extend far beyond the thickness of the line
// stroking the path. The miterLimit' imposes a limit on the ratio of
// the miter length to the 'lineWidth'. The default value of this
// parameter is 4.
void Magick::Image::strokeMiterLimit ( const size_t strokeMiterLimit_ )
{
  modifyImage();
  options()->strokeMiterLimit( strokeMiterLimit_ );
}
size_t Magick::Image::strokeMiterLimit ( void ) const
{
  return constOptions()->strokeMiterLimit( );
}

// Pattern to use while stroking drawn objects.
void Magick::Image::strokePattern ( const Image &strokePattern_ )
{
  modifyImage();
  if(strokePattern_.isValid())
    options()->strokePattern( strokePattern_.constImage() );
  else
    options()->strokePattern( static_cast<MagickCore::Image*>(NULL) );
}
Magick::Image  Magick::Image::strokePattern ( void  ) const
{
  // FIXME: This is inordinately innefficient
  Image texture;
  
  const MagickCore::Image* tmpTexture = constOptions()->strokePattern( );

  if ( tmpTexture )
    {
      ExceptionInfo exceptionInfo;
      GetExceptionInfo( &exceptionInfo );
      MagickCore::Image* image =
	CloneImage( tmpTexture,
                    0, // columns
                    0, // rows
                    MagickTrue, // orphan
                    &exceptionInfo);
      throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
      texture.replaceImage( image );
    }
  return texture;
}

// Stroke width for drawing lines, circles, ellipses, etc.
void Magick::Image::strokeWidth ( const double strokeWidth_ )
{
  modifyImage();
  options()->strokeWidth( strokeWidth_ );
}
double Magick::Image::strokeWidth ( void ) const
{
  return constOptions()->strokeWidth( );
}

void Magick::Image::subImage ( const size_t subImage_ )
{
  modifyImage();
  options()->subImage( subImage_ );
}
size_t Magick::Image::subImage ( void ) const
{
  return constOptions()->subImage( );
}

void Magick::Image::subRange ( const size_t subRange_ )
{
  modifyImage();
  options()->subRange( subRange_ );
}
size_t Magick::Image::subRange ( void ) const
{
  return constOptions()->subRange( );
}

// Annotation text encoding (e.g. "UTF-16")
void Magick::Image::textEncoding ( const std::string &encoding_ )
{
  modifyImage();
  options()->textEncoding( encoding_ );
}
std::string Magick::Image::textEncoding ( void ) const
{
  return constOptions()->textEncoding( );
}

void Magick::Image::tileName ( const std::string &tileName_ )
{
  modifyImage();
  options()->tileName( tileName_ );
}
std::string Magick::Image::tileName ( void ) const
{
  return constOptions()->tileName( );
}

size_t Magick::Image::totalColors ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  size_t colors = GetNumberColors( image(), 0, &exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  return colors;
}

// Origin of coordinate system to use when annotating with text or drawing
void Magick::Image::transformOrigin ( const double x_, const double y_ )
{
  modifyImage();
  options()->transformOrigin( x_, y_ );
}

// Rotation to use when annotating with text or drawing
void Magick::Image::transformRotation ( const double angle_ )
{
  modifyImage();
  options()->transformRotation( angle_ );
}

// Reset transformation parameters to default
void Magick::Image::transformReset ( void )
{
  modifyImage();
  options()->transformReset();
}

// Scale to use when annotating with text or drawing
void Magick::Image::transformScale ( const double sx_, const double sy_ )
{
  modifyImage();
  options()->transformScale( sx_, sy_ );
}

// Skew to use in X axis when annotating with text or drawing
void Magick::Image::transformSkewX ( const double skewx_ )
{
  modifyImage();
  options()->transformSkewX( skewx_ );
}

// Skew to use in Y axis when annotating with text or drawing
void Magick::Image::transformSkewY ( const double skewy_ )
{
  modifyImage();
  options()->transformSkewY( skewy_ );
}

// Image representation type
Magick::ImageType Magick::Image::type ( void ) const
{

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  ImageType image_type = constOptions()->type();
  if ( image_type == UndefinedType )
    image_type= GetImageType( constImage(), &exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  return image_type;
}
void Magick::Image::type ( const Magick::ImageType type_)
{
  modifyImage();
  options()->type( type_ );
  SetImageType( image(), type_ );
}

void Magick::Image::verbose ( const bool verboseFlag_ )
{
  modifyImage();
  options()->verbose( verboseFlag_ );
}
bool Magick::Image::verbose ( void ) const
{
  return constOptions()->verbose( );
}

void Magick::Image::view ( const std::string &view_ )
{
  modifyImage();
  options()->view( view_ );
}
std::string Magick::Image::view ( void ) const
{
  return constOptions()->view( );
}

// Virtual pixel method
void Magick::Image::virtualPixelMethod ( const VirtualPixelMethod virtual_pixel_method_ )
{
  modifyImage();
  SetImageVirtualPixelMethod( image(), virtual_pixel_method_ );
  options()->virtualPixelMethod( virtual_pixel_method_ );
}
Magick::VirtualPixelMethod Magick::Image::virtualPixelMethod ( void ) const
{
  return GetImageVirtualPixelMethod( constImage() );
}

void Magick::Image::x11Display ( const std::string &display_ )
{
  modifyImage();
  options()->x11Display( display_ );
}
std::string Magick::Image::x11Display ( void ) const
{
  return constOptions()->x11Display( );
}

double Magick::Image::xResolution ( void ) const
{
  return constImage()->x_resolution;
}
double Magick::Image::yResolution ( void ) const
{
  return constImage()->y_resolution;
}

// Copy Constructor
Magick::Image::Image( const Image & image_ )
  : _imgRef(image_._imgRef)
{
  Lock( &_imgRef->_mutexLock );

  // Increase reference count
  ++_imgRef->_refCount;
}

// Assignment operator
Magick::Image& Magick::Image::operator=( const Magick::Image &image_ )
{
  if( this != &image_ )
    {
      {
        Lock( &image_._imgRef->_mutexLock );
        ++image_._imgRef->_refCount;
      }

      bool doDelete = false;
      {
        Lock( &_imgRef->_mutexLock );
        if ( --_imgRef->_refCount == 0 )
          doDelete = true;
      }

      if ( doDelete )
        {
          // Delete old image reference with associated image and options.
          delete _imgRef;
          _imgRef = 0;
        }
      // Use new image reference
      _imgRef = image_._imgRef;
    }

  return *this;
}

//////////////////////////////////////////////////////////////////////    
//
// Low-level Pixel Access Routines
//
// Also see the Pixels class, which provides support for multiple
// cache views. The low-level pixel access routines in the Image
// class are provided in order to support backward compatability.
//
//////////////////////////////////////////////////////////////////////

// Transfers read-only pixels from the image to the pixel cache as
// defined by the specified region
const Magick::PixelPacket* Magick::Image::getConstPixels
  ( const ssize_t x_, const ssize_t y_,
    const size_t columns_,
    const size_t rows_ ) const
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  const PixelPacket* p = (*GetVirtualPixels)( constImage(),
                                                x_, y_,
                                                columns_, rows_,
                                                &exceptionInfo );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  return p;
}

// Obtain read-only pixel indexes (valid for PseudoClass images)
const Magick::IndexPacket* Magick::Image::getConstIndexes ( void ) const
{
  const Magick::IndexPacket* result = GetVirtualIndexQueue( constImage() );

  if( !result )
    throwImageException();

  return result;
}

// Obtain image pixel indexes (valid for PseudoClass images)
Magick::IndexPacket* Magick::Image::getIndexes ( void )
{
  Magick::IndexPacket* result = GetAuthenticIndexQueue( image() );

  if( !result )
    throwImageException();

  return ( result );
}

// Transfers pixels from the image to the pixel cache as defined
// by the specified region. Modified pixels may be subsequently
// transferred back to the image via syncPixels.
Magick::PixelPacket* Magick::Image::getPixels ( const ssize_t x_, const ssize_t y_,
						const size_t columns_,
						const size_t rows_ )
{
  modifyImage();
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  PixelPacket* result = (*GetAuthenticPixels)( image(),
                                           x_, y_,
                                           columns_, rows_, &exceptionInfo );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );

  return result;
}

// Allocates a pixel cache region to store image pixels as defined
// by the region rectangle.  This area is subsequently transferred
// from the pixel cache to the image via syncPixels.
Magick::PixelPacket* Magick::Image::setPixels ( const ssize_t x_, const ssize_t y_,
						const size_t columns_,
						const size_t rows_ )
{
  modifyImage();
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  PixelPacket* result = (*QueueAuthenticPixels)( image(),
                                           x_, y_,
                                           columns_, rows_, &exceptionInfo );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );

  return result;
}

// Transfers the image cache pixels to the image.
void Magick::Image::syncPixels ( void )
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  (*SyncAuthenticPixels)( image(), &exceptionInfo );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

// Transfers one or more pixel components from a buffer or file
// into the image pixel cache of an image.
// Used to support image decoders.
void Magick::Image::readPixels ( const Magick::QuantumType quantum_,
                                 const unsigned char *source_ )
{
  QuantumInfo
    *quantum_info;

  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  ImportQuantumPixels(image(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,source_, &exceptionInfo);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  quantum_info=DestroyQuantumInfo(quantum_info);
}

// Transfers one or more pixel components from the image pixel
// cache to a buffer or file.
// Used to support image encoders.
void Magick::Image::writePixels ( const Magick::QuantumType quantum_,
                                  unsigned char *destination_ )
{
  QuantumInfo
    *quantum_info;

  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  ExportQuantumPixels(image(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,destination_, &exceptionInfo);
  quantum_info=DestroyQuantumInfo(quantum_info);
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
}

/////////////////////////////////////////////////////////////////////
//
// No end-user methods beyond this point
//
/////////////////////////////////////////////////////////////////////


//
// Construct using existing image and default options
//
Magick::Image::Image ( MagickCore::Image* image_ )
  : _imgRef(new ImageRef( image_))
{
}

// Get Magick::Options*
Magick::Options* Magick::Image::options( void )
{
  return _imgRef->options();
}
const Magick::Options* Magick::Image::constOptions( void ) const
{
  return _imgRef->options();
}

// Get MagickCore::Image*
MagickCore::Image*& Magick::Image::image( void )
{
  return _imgRef->image();
}
const MagickCore::Image* Magick::Image::constImage( void ) const
{
  return _imgRef->image();
}

// Get ImageInfo *
MagickCore::ImageInfo* Magick::Image::imageInfo( void )
{
  return _imgRef->options()->imageInfo();
}
const MagickCore::ImageInfo * Magick::Image::constImageInfo( void ) const
{
  return _imgRef->options()->imageInfo();
}

// Get QuantizeInfo *
MagickCore::QuantizeInfo* Magick::Image::quantizeInfo( void )
{
  return _imgRef->options()->quantizeInfo();
}
const MagickCore::QuantizeInfo * Magick::Image::constQuantizeInfo( void ) const
{
  return _imgRef->options()->quantizeInfo();
}

//
// Replace current image
//
MagickCore::Image * Magick::Image::replaceImage
  ( MagickCore::Image* replacement_ )
{
  MagickCore::Image* image;
  
  if( replacement_ )
    image = replacement_;
  else
    image = AcquireImage(constImageInfo());

  {
    Lock( &_imgRef->_mutexLock );

    if ( _imgRef->_refCount == 1 )
      {
        // We own the image, just replace it, and de-register
        _imgRef->id( -1 );
        _imgRef->image(image);
      }
    else
      {
        // We don't own the image, dereference and replace with copy
        --_imgRef->_refCount;
        _imgRef = new ImageRef( image, constOptions() );
      }
  }

  return _imgRef->_image;
}

//
// Prepare to modify image or image options
// Replace current image and options with copy if reference count > 1
//
void Magick::Image::modifyImage( void )
{
  {
    Lock( &_imgRef->_mutexLock );
    if ( _imgRef->_refCount == 1 )
      {
        // De-register image and return
        _imgRef->id( -1 );
        return;
      }
  }

  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  replaceImage( CloneImage( image(),
                            0, // columns
                            0, // rows
                            MagickTrue, // orphan
                            &exceptionInfo) );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  return;
}

//
// Test for an ImageMagick reported error and throw exception if one
// has been reported.  Secretly resets image->exception back to default
// state even though this method is const.
//
void Magick::Image::throwImageException( void ) const
{
  // Throw C++ exception while resetting Image exception to default state
  throwException( const_cast<MagickCore::Image*>(constImage())->exception );
}

// Register image with image registry or obtain registration id
ssize_t Magick::Image::registerId( void )
{
  Lock( &_imgRef->_mutexLock );
  if( _imgRef->id() < 0 )
    {
      char id[MaxTextExtent];
      ExceptionInfo exceptionInfo;
      GetExceptionInfo( &exceptionInfo );
      _imgRef->id(_imgRef->id()+1);
      sprintf(id,"%.20g\n",(double) _imgRef->id());
      SetImageRegistry(ImageRegistryType, id, image(), &exceptionInfo);
      throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
    }
  return _imgRef->id();
}

// Unregister image from image registry
void Magick::Image::unregisterId( void )
{
  modifyImage();
  _imgRef->id( -1 );
}

//
// Create a local wrapper around MagickCoreTerminus
//
namespace Magick
{
  extern "C" {
    void MagickPlusPlusDestroyMagick(void);
  }
}

void Magick::MagickPlusPlusDestroyMagick(void)
{
  if (magick_initialized)
    {
      magick_initialized=false;
      MagickCore::MagickCoreTerminus();
    }
}

// C library initialization routine
void MagickPPExport Magick::InitializeMagick(const char *path_)
{
  MagickCore::MagickCoreGenesis(path_,MagickFalse);
  if (!magick_initialized)
    magick_initialized=true;
}

//
// Cleanup class to ensure that ImageMagick singletons are destroyed
// so as to avoid any resemblence to a memory leak (which seems to
// confuse users)
//
namespace Magick
{

  class MagickCleanUp
  {
  public:
    MagickCleanUp( void );
    ~MagickCleanUp( void );
  };

  // The destructor for this object is invoked when the destructors for
  // static objects in this translation unit are invoked.
  static MagickCleanUp magickCleanUpGuard;
}

Magick::MagickCleanUp::MagickCleanUp ( void )
{
  // Don't even think about invoking InitializeMagick here!
}

Magick::MagickCleanUp::~MagickCleanUp ( void )
{
  MagickPlusPlusDestroyMagick();
}

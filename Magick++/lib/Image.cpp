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

#define GetPPException \
  ExceptionInfo \
    exceptionInfo; \
  GetExceptionInfo(&exceptionInfo)
#define ThrowPPException \
  throwException(exceptionInfo); \
  (void) DestroyExceptionInfo(&exceptionInfo)

MagickPPExport const char *Magick::borderGeometryDefault="6x6+0+0";
MagickPPExport const char *Magick::frameGeometryDefault="25x25+6+6";
MagickPPExport const char *Magick::raiseGeometryDefault="6x6+0+0";

static bool magick_initialized=false;

MagickPPExport int Magick::operator == (const Magick::Image &left_,
  const Magick::Image &right_)
{
  // If image pixels and signature are the same, then the image is identical
  return((left_.rows() == right_.rows()) &&
    (left_.columns() == right_.columns()) &&
    (left_.signature() == right_.signature()));
}

MagickPPExport int Magick::operator != (const Magick::Image &left_,
  const Magick::Image &right_)
{
  return(!(left_ == right_));
}

MagickPPExport int Magick::operator > (const Magick::Image &left_,
  const Magick::Image &right_)
{
  return(!(left_ < right_) && (left_ != right_));
}

MagickPPExport int Magick::operator < (const Magick::Image &left_,
  const Magick::Image &right_)
{
  // If image pixels are less, then image is smaller
  return((left_.rows() * left_.columns()) < (right_.rows() * 
    right_.columns()));
}

MagickPPExport int Magick::operator >= (const Magick::Image &left_,
  const Magick::Image &right_)
{
  return((left_ > right_) || (left_ == right_));
}

MagickPPExport int Magick::operator <= (const Magick::Image &left_,
  const Magick::Image &right_)
{
  return((left_ < right_) || (left_ == right_));
}

MagickPPExport void Magick::InitializeMagick(const char *path_)
{
  MagickCore::MagickCoreGenesis(path_,MagickFalse);
  if (!magick_initialized)
    magick_initialized=true;
}

Magick::Image::Image(void)
  : _imgRef(new ImageRef)
{
}

Magick::Image::Image(const Blob &blob_)
  : _imgRef(new ImageRef)
{
  try
  {
    // Initialize, Allocate and Read images
    read(blob_);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const Blob &blob_,const Geometry &size_)
  : _imgRef(new ImageRef)
{
  try
  {
    // Read from Blob
    read(blob_,size_);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const Blob &blob_,const Geometry &size_,
  const size_t depth_)
  : _imgRef(new ImageRef)
{
  try
  {
    // Read from Blob
    read(blob_,size_,depth_);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const Blob &blob_,const Geometry &size_,
  const size_t depth_,const std::string &magick_)
  : _imgRef(new ImageRef)
{
  try
  {
    // Read from Blob
    read(blob_,size_,depth_,magick_);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const Blob &blob_,const Geometry &size_,
  const std::string &magick_)
  : _imgRef(new ImageRef)
{
  try
  {
    // Read from Blob
    read(blob_,size_,magick_);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const Geometry &size_,const Color &color_)
  : _imgRef(new ImageRef)
{
  // xc: prefix specifies an X11 color string
  std::string imageSpec("xc:");
  imageSpec+=color_;

  try
  {
    // Set image size
    size(size_);

    // Initialize, Allocate and Read images
    read(imageSpec);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const Image &image_)
  : _imgRef(image_._imgRef)
{
  Lock(&_imgRef->_mutexLock);

  // Increase reference count
  ++_imgRef->_refCount;
}

Magick::Image::Image(const size_t width_,const size_t height_,
  const std::string &map_,const StorageType type_,const void *pixels_)
  : _imgRef(new ImageRef)
{
  try
  {
    read(width_,height_,map_,type_,pixels_);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const std::string &imageSpec_)
  : _imgRef(new ImageRef)
{
  try
  {
    // Initialize, Allocate and Read images
    read(imageSpec_);
  }
  catch(const Warning & /*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error & /*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::~Image ()
{
  bool
    doDelete=false;

  {
    Lock(&_imgRef->_mutexLock);
    if (--_imgRef->_refCount == 0)
      doDelete=true;
  }

  if (doDelete)
    delete _imgRef;

  _imgRef=0;
}

Magick::Image& Magick::Image::operator=(const Magick::Image &image_)
{
  if (this != &image_)
    {
      bool
        doDelete=false;

      {
        Lock(&image_._imgRef->_mutexLock);
        ++image_._imgRef->_refCount;
      }

      {
        Lock(&_imgRef->_mutexLock);
        if (--_imgRef->_refCount == 0)
          doDelete=true;
      }

      if (doDelete)
        {
          // Delete old image reference with associated image and options.
          delete _imgRef;
          _imgRef=0;
        }

      // Use new image reference
      _imgRef=image_._imgRef;
    }

  return(*this);
}

void Magick::Image::adjoin(const bool flag_)
{
  modifyImage();
  options()->adjoin(flag_);
}

bool Magick::Image::adjoin(void) const
{
  return(constOptions()->adjoin());
}

void Magick::Image::antiAlias(const bool flag_)
{
  modifyImage();
  options()->antiAlias(static_cast<size_t>(flag_));
}

bool Magick::Image::antiAlias(void)
{
  return(static_cast<bool>(options()->antiAlias()));
}

void Magick::Image::animationDelay(const size_t delay_)
{
  modifyImage();
  image()->delay=delay_;
}

size_t Magick::Image::animationDelay(void) const
{
  return(constImage()->delay);
}

void Magick::Image::animationIterations(const size_t iterations_)
{
  modifyImage();
  image()->iterations=iterations_;
}

size_t Magick::Image::animationIterations(void) const
{
  return(constImage()->iterations);
}

void Magick::Image::backgroundColor(const Color &backgroundColor_)
{
  modifyImage();

  if (backgroundColor_.isValid())
    image()->background_color=backgroundColor_;
  else
    image()->background_color=Color();

  options()->backgroundColor(backgroundColor_);
}

Magick::Color Magick::Image::backgroundColor(void) const
{
  return(constOptions()->backgroundColor());
}

void Magick::Image::backgroundTexture(const std::string &backgroundTexture_)
{
  modifyImage();
  options()->backgroundTexture(backgroundTexture_);
}

std::string Magick::Image::backgroundTexture(void) const
{
  return(constOptions()->backgroundTexture());
}

size_t Magick::Image::baseColumns(void) const
{
  return(constImage()->magick_columns);
}

std::string Magick::Image::baseFilename(void) const
{
  return(std::string(constImage()->magick_filename));
}

size_t Magick::Image::baseRows(void) const
{
  return(constImage()->magick_rows);
}

void Magick::Image::borderColor(const Color &borderColor_)
{
  modifyImage();

  if (borderColor_.isValid())
    image()->border_color=borderColor_;
  else
    image()->border_color=Color();

  options()->borderColor(borderColor_);
}

Magick::Color Magick::Image::borderColor(void) const
{
  return(constOptions()->borderColor());
}

Magick::Geometry Magick::Image::boundingBox(void) const
{
  RectangleInfo
    bbox; 
  
  GetPPException;
  bbox=GetImageBoundingBox(constImage(),&exceptionInfo);
  ThrowPPException;
  return(Geometry(bbox));
}

void Magick::Image::boxColor(const Color &boxColor_)
{
  modifyImage();
  options()->boxColor(boxColor_);
}

Magick::Color Magick::Image::boxColor(void) const
{
  return(constOptions()->boxColor());
}

void Magick::Image::cacheThreshold(const size_t threshold_)
{
  SetMagickResourceLimit(MemoryResource,threshold_);
}

void Magick::Image::classType(const ClassType class_)
{
  if (classType() == PseudoClass && class_ == DirectClass)
    {
      // Use SyncImage to synchronize the DirectClass pixels with the
      // color map and then set to DirectClass type.
      modifyImage();
      SyncImage(image());
      image()->colormap=(PixelPacket *)RelinquishMagickMemory(
        image()->colormap);
      image()->storage_class=static_cast<MagickCore::ClassType>(DirectClass);
    }
  else if (classType() == DirectClass && class_ == PseudoClass)
    {
      // Quantize to create PseudoClass color map
      modifyImage();
      quantizeColors(MaxColormapSize);
      quantize();
      image()->storage_class=static_cast<MagickCore::ClassType>(PseudoClass);
    }
}

void Magick::Image::clipMask(const Magick::Image &clipMask_)
{
  modifyImage();

  if (clipMask_.isValid())
    SetImageClipMask(image(),clipMask_.constImage());
  else
    SetImageClipMask(image(),0);
}

Magick::Image Magick::Image::clipMask(void) const
{
  MagickCore::Image
    *image;

  GetPPException;
  image=GetImageClipMask(constImage(),&exceptionInfo);
  ThrowPPException;

  if (image == (MagickCore::Image *) NULL)
    return(Magick::Image());
  else
    return(Magick::Image(image));
}

void Magick::Image::colorFuzz(const double fuzz_)
{
  modifyImage();
  image()->fuzz=fuzz_;
  options()->colorFuzz(fuzz_);
}

double Magick::Image::colorFuzz(void) const
{
  return(constOptions()->colorFuzz());
}

void Magick::Image::colorMapSize(const size_t entries_)
{
  if (entries_ > MaxColormapSize)
    throwExceptionExplicit(OptionError,
      "Colormap entries must not exceed MaxColormapSize");

  modifyImage();
  AcquireImageColormap(image(),entries_);
}

size_t Magick::Image::colorMapSize(void)
{
  if (!constImage()->colormap)
    throwExceptionExplicit(OptionError,"Image does not contain a colormap");

  return(constImage()->colors);
}

void Magick::Image::colorSpace(const ColorspaceType colorSpace_)
{
  if (image()->colorspace == colorSpace_)
    return;

  modifyImage();
  TransformImageColorspace(image(),colorSpace_);
  throwImageException();
}

Magick::ColorspaceType Magick::Image::colorSpace(void) const
{
  return(constImage()->colorspace);
}

void Magick::Image::colorspaceType(const ColorspaceType colorSpace_)
{
  modifyImage();
  SetImageColorspace(image(),colorSpace_);
  throwImageException();
  options()->colorspaceType(colorSpace_);
}

Magick::ColorspaceType Magick::Image::colorspaceType(void) const
{
  return(constOptions()->colorspaceType());
}

void Magick::Image::comment(const std::string &comment_)
{
  modifyImage();
  SetImageProperty(image(),"Comment",NULL);
  if (comment_.length() > 0)
    SetImageProperty(image(),"Comment",comment_.c_str());
  throwImageException();
}

std::string Magick::Image::comment(void) const
{
  const char
    *value;

  value=GetImageProperty(constImage(),"Comment");

  if (value)
    return(std::string(value));

  return(std::string()); // Intentionally no exception
}

void Magick::Image::compose(const CompositeOperator compose_)
{
  image()->compose=compose_;
}

Magick::CompositeOperator Magick::Image::compose(void) const
{
  return(constImage()->compose);
}

void Magick::Image::compressType(const CompressionType compressType_)
{
  modifyImage();
  image()->compression=compressType_;
  options()->compressType(compressType_);
}

Magick::CompressionType Magick::Image::compressType(void) const
{
  return(constImage()->compression);
}

void Magick::Image::debug(const bool flag_)
{
  modifyImage();
  options()->debug(flag_);
}

bool Magick::Image::debug(void) const
{
  return(constOptions()->debug());
}

void Magick::Image::density(const Geometry &density_)
{
  modifyImage();
  options()->density(density_);
  if (density_.isValid())
    {
      image()->x_resolution=density_.width();
      if (density_.height() != 0)
        image()->y_resolution=density_.height();
      else
        image()->y_resolution=density_.width();
    }
  else
    {
      // Reset to default
      image()->x_resolution=0;
      image()->y_resolution=0;
    }
}

Magick::Geometry Magick::Image::density(void) const
{
  if (isValid())
    {
      ssize_t
        x_resolution=72,
        y_resolution=72;

      if (constImage()->x_resolution > 0.0)
        x_resolution=static_cast<ssize_t>(constImage()->x_resolution + 0.5);

      if (constImage()->y_resolution > 0.0)
        y_resolution=static_cast<ssize_t>(constImage()->y_resolution + 0.5);

      return(Geometry(x_resolution,y_resolution));
    }

  return(constOptions()->density());
}

void Magick::Image::depth(const size_t depth_)
{
  size_t
    depth=depth_;

  if (depth > MAGICKCORE_QUANTUM_DEPTH)
    depth=MAGICKCORE_QUANTUM_DEPTH;

  modifyImage();
  image()->depth=depth;
  options()->depth(depth);
}

size_t Magick::Image::depth(void) const
{
  return(constImage()->depth);
}

std::string Magick::Image::directory(void) const
{
  if (constImage()->directory)
    return(std::string(constImage()->directory));

  throwExceptionExplicit(CorruptImageWarning,
    "Image does not contain a directory");

  return(std::string());
}

void Magick::Image::endian(const Magick::EndianType endian_)
{
  modifyImage();
  options()->endian(endian_);
  image()->endian=endian_;
}

Magick::EndianType Magick::Image::endian(void) const
{
  return(constImage()->endian);
}

void Magick::Image::exifProfile(const Magick::Blob &exifProfile_)
{
  if (exifProfile_.data() != 0)
    {
      StringInfo 
        *exif_profile;

      modifyImage();
      exif_profile=AcquireStringInfo(exifProfile_.length());
      SetStringInfoDatum(exif_profile,(unsigned char *) exifProfile_.data());
      (void) SetImageProfile(image(),"exif",exif_profile);
      exif_profile=DestroyStringInfo(exif_profile);
    }
}

Magick::Blob Magick::Image::exifProfile(void) const
{
  const StringInfo
    *exif_profile;

  exif_profile=GetImageProfile(constImage(),"exif");
  if (exif_profile == (StringInfo *) NULL)
    return(Blob());

  return(Blob(GetStringInfoDatum(exif_profile),GetStringInfoLength(
    exif_profile)));
} 

void Magick::Image::fileName(const std::string &fileName_)
{
  modifyImage();

  fileName_.copy(image()->filename,sizeof(image()->filename)-1);
  image()->filename[fileName_.length()]=0; // Null terminate
  
  options()->fileName(fileName_);
}

std::string Magick::Image::fileName(void) const
{
  return(constOptions()->fileName());
}

off_t Magick::Image::fileSize(void) const
{
  return((off_t) GetBlobSize(constImage()));
}

void Magick::Image::fillColor(const Magick::Color &fillColor_)
{
  modifyImage();
  options()->fillColor(fillColor_);
}

Magick::Color Magick::Image::fillColor(void) const
{
  return(constOptions()->fillColor());
}

void Magick::Image::fillRule(const Magick::FillRule &fillRule_)
{
  modifyImage();
  options()->fillRule(fillRule_);
}

Magick::FillRule Magick::Image::fillRule(void) const
{
  return(constOptions()->fillRule());
}

void Magick::Image::fillPattern(const Image &fillPattern_)
{
  modifyImage();
  if(fillPattern_.isValid())
    options()->fillPattern(fillPattern_.constImage());
  else
    options()->fillPattern(static_cast<MagickCore::Image*>(NULL));
}

Magick::Image Magick::Image::fillPattern(void) const
{
  // FIXME: This is inordinately innefficient
  const MagickCore::Image
    *tmpTexture;

  Image
    texture;

  tmpTexture=constOptions()->fillPattern();

  if(tmpTexture)
    {
      MagickCore::Image
        *image;

      GetPPException;
      image=CloneImage(tmpTexture,0,0,MagickTrue,&exceptionInfo);
      texture.replaceImage(image);
      ThrowPPException;
    }

  return(texture);
}

void Magick::Image::filterType(const Magick::FilterTypes filterType_)
{
  modifyImage();
  image()->filter=filterType_;
}

Magick::FilterTypes Magick::Image::filterType(void) const
{
  return(constImage()->filter);
}

void Magick::Image::font(const std::string &font_)
{
  modifyImage();
  options()->font(font_);
}

std::string Magick::Image::font(void) const
{
  return(constOptions()->font());
}

void Magick::Image::fontPointsize(const double pointSize_)
{
  modifyImage();
  options()->fontPointsize(pointSize_);
}

double Magick::Image::fontPointsize(void) const
{
  return(constOptions()->fontPointsize());
}

std::string Magick::Image::format(void) const
{
  const MagickInfo
    *magick_info;

  GetPPException;
  magick_info=GetMagickInfo(constImage()->magick,&exceptionInfo);
  ThrowPPException;

  if ((magick_info != 0) && (*magick_info->description != '\0'))
    return(std::string(magick_info->description));

  throwExceptionExplicit(CorruptImageWarning,"Unrecognized image magick type");
  return(std::string());
}

std::string Magick::Image::formatExpression(const std::string expression)
{
  char
    *text;

  std::string
    result;

  text=InterpretImageProperties(imageInfo(),image(),expression.c_str());
  if (text != (char *) NULL)
    {
      result=std::string(text);
      text=DestroyString(text);
    }
  throwImageException();
  return(result);
}

double Magick::Image::gamma(void) const
{
  return(constImage()->gamma);
}

Magick::Geometry Magick::Image::geometry(void) const
{
  if (constImage()->geometry)
    return(Geometry(constImage()->geometry));

  throwExceptionExplicit(OptionWarning,"Image does not contain a geometry");

  return(Geometry());
}

void Magick::Image::gifDisposeMethod(const size_t disposeMethod_)
{
  modifyImage();
  image()->dispose=(DisposeType) disposeMethod_;
}

size_t Magick::Image::gifDisposeMethod(void) const
{
  // FIXME: It would be better to return an enumeration
  return ((size_t) constImage()->dispose);
}

// ICC ICM color profile (BLOB)
void Magick::Image::iccColorProfile(const Magick::Blob &colorProfile_)
{
  profile("icm",colorProfile_);
}

Magick::Blob Magick::Image::iccColorProfile(void) const
{
  const StringInfo
    *color_profile;

  color_profile=GetImageProfile(constImage(),"icc");
  if (color_profile == (StringInfo *) NULL)
    return Blob();

  return(Blob(GetStringInfoDatum(color_profile),GetStringInfoLength(
    color_profile)));
}

void Magick::Image::interlaceType(const InterlaceType interlace_)
{
  modifyImage();
  image()->interlace=interlace_;
  options()->interlaceType(interlace_);
}

Magick::InterlaceType Magick::Image::interlaceType(void) const
{
  return constImage()->interlace;
}

void Magick::Image::interpolate(const InterpolatePixelMethod interpolate_)
{
  modifyImage();
  image()->interpolate=interpolate_;
}

Magick::InterpolatePixelMethod Magick::Image::interpolate(void) const
{
  return constImage()->interpolate;
}

void Magick::Image::iptcProfile(const Magick::Blob &iptcProfile_)
{
  modifyImage();
  if (iptcProfile_.data() != 0)
    {
      StringInfo
        *iptc_profile;

      iptc_profile=AcquireStringInfo(iptcProfile_.length());
      SetStringInfoDatum(iptc_profile,(unsigned char *) iptcProfile_.data());
      (void) SetImageProfile(image(),"iptc",iptc_profile);
      iptc_profile=DestroyStringInfo(iptc_profile );
    }
}

Magick::Blob Magick::Image::iptcProfile(void) const
{
  const StringInfo
    *iptc_profile;

  iptc_profile=GetImageProfile(constImage(),"iptc");
  if (iptc_profile == (StringInfo *) NULL)
    return(Blob());
  return(Blob(GetStringInfoDatum(iptc_profile),GetStringInfoLength(
    iptc_profile)));
}

void Magick::Image::isValid(const bool isValid_)
{
  if (!isValid_)
    {
      delete _imgRef;
      _imgRef = new ImageRef;
    }
  else if (!isValid())
    {
      // Construct with single-pixel black image to make
      // image valid.  This is an obvious hack.
      size(Geometry(1,1));
      read("xc:black");
    }
}

bool Magick::Image::isValid(void) const
{
  return(rows() && columns());
}

void Magick::Image::label(const std::string &label_)
{
  modifyImage();
  SetImageProperty(image(),"Label",NULL);
  if (label_.length() > 0)
    SetImageProperty(image(),"Label",label_.c_str());
  throwImageException();
}

std::string Magick::Image::label(void) const
{
  const char
    *value;

  value=GetImageProperty(constImage(),"Label");

  if (value)
    return(std::string(value));

  return(std::string());
}

void Magick::Image::magick(const std::string &magick_)
{
  modifyImage();

  magick_.copy(image()->magick,sizeof(image()->magick)-1);
  image()->magick[magick_.length()]=0;

  options()->magick(magick_);
}

std::string Magick::Image::magick(void) const
{
  if (*(constImage()->magick) != '\0')
    return(std::string(constImage()->magick));

  return(constOptions()->magick());
}

void Magick::Image::matte(const bool matteFlag_)
{
  modifyImage();

  // If matte channel is requested, but image doesn't already have a
  // matte channel, then create an opaque matte channel.  Likewise, if
  // the image already has a matte channel but a matte channel is not
  // desired, then set the matte channel to opaque.
  if ((matteFlag_ && !constImage()->matte) || (constImage()->matte &&
    !matteFlag_))
    SetImageOpacity(image(),OpaqueOpacity);

  image()->matte=(MagickBooleanType) matteFlag_;
}

bool Magick::Image::matte(void) const
{
  if (constImage()->matte)
    return true;
  else
    return false;
}

void Magick::Image::matteColor(const Color &matteColor_)
{
  modifyImage();

  if (matteColor_.isValid())
    {
      image()->matte_color=matteColor_;
      options()->matteColor(matteColor_);
    }
  else
    {
      // Set to default matte color
      Color
        tmpColor("#BDBDBD");

      image()->matte_color=tmpColor;
      options()->matteColor(tmpColor);
    }
}

Magick::Color Magick::Image::matteColor(void) const
{
  return(Color(constImage()->matte_color.red,constImage()->matte_color.green,
    constImage()->matte_color.blue));
}

double Magick::Image::meanErrorPerPixel(void) const
{
  return(constImage()->error.mean_error_per_pixel);
}

void Magick::Image::modulusDepth(const size_t depth_)
{
  modifyImage();
  SetImageDepth(image(),depth_);
  options()->depth(depth_);
}

size_t Magick::Image::modulusDepth(void) const
{
  size_t
    depth;

  GetPPException;
  depth=GetImageDepth(constImage(),&exceptionInfo);
  ThrowPPException;
  return(depth);
}

void Magick::Image::monochrome(const bool monochromeFlag_)
{
  modifyImage();
  options()->monochrome(monochromeFlag_);
}

bool Magick::Image::monochrome(void) const
{
  return(constOptions()->monochrome());
}

Magick::Geometry Magick::Image::montageGeometry(void) const
{
  if (constImage()->montage)
    return(Magick::Geometry(constImage()->montage));

  throwExceptionExplicit(CorruptImageWarning,
    "Image does not contain a montage");

  return(Magick::Geometry());
}

double Magick::Image::normalizedMaxError(void) const
{
  return(constImage()->error.normalized_maximum_error);
}

double Magick::Image::normalizedMeanError(void) const
{
  return (constImage()->error.normalized_mean_error);
}

void Magick::Image::orientation(const Magick::OrientationType orientation_)
{
  modifyImage();
  image()->orientation=orientation_;
}

Magick::OrientationType Magick::Image::orientation(void) const
{
  return(constImage()->orientation);
}

void Magick::Image::page(const Magick::Geometry &pageSize_)
{
  modifyImage();
  options()->page(pageSize_);
  image()->page=pageSize_;
}

Magick::Geometry Magick::Image::page(void) const
{
  return(Geometry(constImage()->page.width,constImage()->page.height,
    AbsoluteValue(constImage()->page.x),AbsoluteValue(constImage()->page.y),
    constImage()->page.x < 0 ? true : false,
    constImage()->page.y < 0 ? true : false));
}

void Magick::Image::penColor(const Color &penColor_)
{
  modifyImage();
  options()->fillColor(penColor_);
  options()->strokeColor(penColor_);
}

Magick::Color Magick::Image::penColor(void) const
{
  return(constOptions()->fillColor());
}

void Magick::Image::penTexture(const Image &penTexture_)
{
  modifyImage();
  if(penTexture_.isValid())
    options()->fillPattern(penTexture_.constImage());
  else
    options()->fillPattern(static_cast<MagickCore::Image*>(NULL));
}

Magick::Image Magick::Image::penTexture(void) const
{
  // FIXME: This is inordinately innefficient
  const MagickCore::Image
    *tmpTexture;

  Image
    texture;

  tmpTexture=constOptions()->fillPattern();

  if (tmpTexture)
    {
      MagickCore::Image
        *image;

      GetPPException;
      image=CloneImage(tmpTexture,0,0,MagickTrue,&exceptionInfo);
      texture.replaceImage(image);
      ThrowPPException;
    }
  return(texture);
}

void Magick::Image::quality(const size_t quality_)
{
  modifyImage();
  image()->quality=quality_;
  options()->quality(quality_);
}

size_t Magick::Image::quality(void) const
{
  return(constImage()->quality);
}

void Magick::Image::quantizeColors(const size_t colors_)
{
  modifyImage();
  options()->quantizeColors(colors_);
}

size_t Magick::Image::quantizeColors(void) const
{
  return(constOptions()->quantizeColors());
}

void Magick::Image::quantizeColorSpace(
  const Magick::ColorspaceType colorSpace_)
{
  modifyImage();
  options()->quantizeColorSpace(colorSpace_);
}

Magick::ColorspaceType Magick::Image::quantizeColorSpace(void) const
{
  return(constOptions()->quantizeColorSpace());
}

void Magick::Image::quantizeDither(const bool ditherFlag_)
{
  modifyImage();
  options()->quantizeDither(ditherFlag_);
}

bool Magick::Image::quantizeDither(void) const
{
  return(constOptions()->quantizeDither());
}

void Magick::Image::quantizeTreeDepth(const size_t treeDepth_)
{
  modifyImage();
  options()->quantizeTreeDepth(treeDepth_);
}

size_t Magick::Image::quantizeTreeDepth(void) const
{
  return(constOptions()->quantizeTreeDepth());
}

void Magick::Image::renderingIntent(
  const Magick::RenderingIntent renderingIntent_)
{
  modifyImage();
  image()->rendering_intent=renderingIntent_;
}

Magick::RenderingIntent Magick::Image::renderingIntent(void) const
{
  return(static_cast<Magick::RenderingIntent>(
    constImage()->rendering_intent));
}

void Magick::Image::resolutionUnits(
  const Magick::ResolutionType resolutionUnits_)
{
  modifyImage();
  image()->units=resolutionUnits_;
  options()->resolutionUnits(resolutionUnits_);
}

Magick::ResolutionType Magick::Image::resolutionUnits(void) const
{
  return(constOptions()->resolutionUnits());
}

void Magick::Image::scene(const size_t scene_)
{
  modifyImage();
  image()->scene=scene_;
}

size_t Magick::Image::scene(void) const
{
  return(constImage()->scene);
}

void Magick::Image::size(const Geometry &geometry_)
{
  modifyImage();
  options()->size(geometry_);
  image()->rows=geometry_.height();
  image()->columns=geometry_.width();
}

Magick::Geometry Magick::Image::size(void) const
{
  return(Magick::Geometry(constImage()->columns,constImage()->rows));
}

void Magick::Image::strokeAntiAlias(const bool flag_)
{
  modifyImage();
  options()->strokeAntiAlias(flag_);
}

bool Magick::Image::strokeAntiAlias(void) const
{
  return(constOptions()->strokeAntiAlias());
}

void Magick::Image::strokeColor(const Magick::Color &strokeColor_)
{
  modifyImage();
  options()->strokeColor(strokeColor_);
}

Magick::Color Magick::Image::strokeColor(void) const
{
  return(constOptions()->strokeColor());
}

void Magick::Image::strokeDashArray(const double *strokeDashArray_)
{
  modifyImage();
  options()->strokeDashArray(strokeDashArray_);
}

const double *Magick::Image::strokeDashArray(void) const
{
  return(constOptions()->strokeDashArray());
}

void Magick::Image::strokeDashOffset(const double strokeDashOffset_)
{
  modifyImage();
  options()->strokeDashOffset(strokeDashOffset_);
}

double Magick::Image::strokeDashOffset(void) const
{
  return(constOptions()->strokeDashOffset());
}

void Magick::Image::strokeLineCap(const Magick::LineCap lineCap_)
{
  modifyImage();
  options()->strokeLineCap(lineCap_);
}

Magick::LineCap Magick::Image::strokeLineCap(void) const
{
  return(constOptions()->strokeLineCap());
}

void Magick::Image::strokeLineJoin(const Magick::LineJoin lineJoin_)
{
  modifyImage();
  options()->strokeLineJoin(lineJoin_);
}

Magick::LineJoin Magick::Image::strokeLineJoin(void) const
{
  return(constOptions()->strokeLineJoin());
}

void Magick::Image::strokeMiterLimit(const size_t strokeMiterLimit_)
{
  modifyImage();
  options()->strokeMiterLimit(strokeMiterLimit_);
}

size_t Magick::Image::strokeMiterLimit(void) const
{
  return constOptions()->strokeMiterLimit();
}

void Magick::Image::strokePattern(const Image &strokePattern_)
{
  modifyImage();
  if(strokePattern_.isValid())
    options()->strokePattern(strokePattern_.constImage());
  else
    options()->strokePattern(static_cast<MagickCore::Image*>(NULL));
}

Magick::Image Magick::Image::strokePattern(void) const
{
  const MagickCore::Image
    *tmpTexture;

  Image
    texture;

  tmpTexture=constOptions()->strokePattern();

  if (tmpTexture)
    {
      MagickCore::Image
        *image;

      GetPPException;
      image=CloneImage(tmpTexture,0,0,MagickTrue,&exceptionInfo);
      texture.replaceImage(image);
      ThrowPPException;
    }
  return(texture);
}

void Magick::Image::strokeWidth(const double strokeWidth_)
{
  modifyImage();
  options()->strokeWidth(strokeWidth_);
}

double Magick::Image::strokeWidth(void) const
{
  return(constOptions()->strokeWidth());
}

void Magick::Image::subImage(const size_t subImage_)
{
  modifyImage();
  options()->subImage(subImage_);
}

size_t Magick::Image::subImage(void) const
{
  return(constOptions()->subImage());
}

void Magick::Image::subRange(const size_t subRange_)
{
  modifyImage();
  options()->subRange(subRange_);
}

size_t Magick::Image::subRange(void) const
{
  return(constOptions()->subRange());
}

void Magick::Image::textEncoding(const std::string &encoding_)
{
  modifyImage();
  options()->textEncoding(encoding_);
}

std::string Magick::Image::textEncoding(void) const
{
  return(constOptions()->textEncoding());
}

void Magick::Image::textInterlineSpacing(double spacing_)
{
  modifyImage();
  options()->textInterlineSpacing(spacing_);
}

double Magick::Image::textInterlineSpacing(void) const
{
  return(constOptions()->textInterlineSpacing());
}

void Magick::Image::textInterwordSpacing(double spacing_)
{
  modifyImage();
  options()->textInterwordSpacing(spacing_);
}

double Magick::Image::textInterwordSpacing(void) const
{
  return(constOptions()->textInterwordSpacing());
}

void Magick::Image::textKerning(double kerning_)
{
  modifyImage();
  options()->textKerning(kerning_);
}

double Magick::Image::textKerning(void) const
{
  return(constOptions()->textKerning());
}

void Magick::Image::tileName(const std::string &tileName_)
{
  modifyImage();
  options()->tileName(tileName_);
}

std::string Magick::Image::tileName(void) const
{
  return(constOptions()->tileName());
}

size_t Magick::Image::totalColors(void)
{
  size_t
    colors;

  GetPPException;
  colors=GetNumberColors(image(),0,&exceptionInfo);
  ThrowPPException;
  return(colors);
}

void Magick::Image::transformRotation(const double angle_)
{
  modifyImage();
  options()->transformRotation(angle_);
}

void Magick::Image::transformSkewX(const double skewx_)
{
  modifyImage();
  options()->transformSkewX(skewx_);
}

void Magick::Image::transformSkewY(const double skewy_)
{
  modifyImage();
  options()->transformSkewY(skewy_);
}

void Magick::Image::type(const Magick::ImageType type_)
{
  modifyImage();
  options()->type(type_);
  SetImageType(image(),type_);
}

Magick::ImageType Magick::Image::type(void) const
{
  ImageType
    image_type;

  image_type=constOptions()->type();
  if (image_type == UndefinedType)
    {
      GetPPException;
      image_type=GetImageType(constImage(),&exceptionInfo);
      ThrowPPException;
    }
  return(image_type);
}

void Magick::Image::verbose(const bool verboseFlag_)
{
  modifyImage();
  options()->verbose(verboseFlag_);
}

bool Magick::Image::verbose(void) const
{
  return(constOptions()->verbose());
}

void Magick::Image::view(const std::string &view_)
{
  modifyImage();
  options()->view(view_);
}

std::string Magick::Image::view(void) const
{
  return(constOptions()->view());
}

void Magick::Image::virtualPixelMethod(
  const VirtualPixelMethod virtual_pixel_method_)
{
  modifyImage();
  SetImageVirtualPixelMethod(image(),virtual_pixel_method_);
  options()->virtualPixelMethod(virtual_pixel_method_);
}

Magick::VirtualPixelMethod Magick::Image::virtualPixelMethod(void) const
{
  return(GetImageVirtualPixelMethod(constImage()));
}

void Magick::Image::x11Display(const std::string &display_)
{
  modifyImage();
  options()->x11Display(display_);
}

std::string Magick::Image::x11Display(void) const
{
  return(constOptions()->x11Display());
}

double Magick::Image::xResolution(void) const
{
  return(constImage()->x_resolution);
}

double Magick::Image::yResolution(void) const
{
  return(constImage()->y_resolution);
}

void Magick::Image::adaptiveBlur(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AdaptiveBlurImage(constImage(),radius_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::adaptiveResize(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    width=columns(),
    height=rows();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=AdaptiveResizeImage(constImage(),width,height,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::adaptiveSharpen(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AdaptiveSharpenImage(constImage(),radius_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::adaptiveSharpenChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AdaptiveSharpenImageChannel(constImage(),channel_,radius_,sigma_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::adaptiveThreshold(const size_t width_,const size_t height_,
  const ssize_t offset_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AdaptiveThresholdImage(constImage(),width_,height_,offset_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::addNoise(const NoiseType noiseType_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AddNoiseImage(constImage(),noiseType_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::addNoiseChannel(const ChannelType channel_,
  const NoiseType noiseType_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AddNoiseImageChannel(constImage(),channel_,noiseType_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::affineTransform(const DrawableAffine &affine_ )
{
  AffineMatrix
    _affine;

  MagickCore::Image
    *newImage;

  _affine.sx = affine_.sx();
  _affine.sy = affine_.sy();
  _affine.rx = affine_.rx();
  _affine.ry = affine_.ry();
  _affine.tx = affine_.tx();
  _affine.ty = affine_.ty();

  GetPPException;
  newImage=AffineTransformImage(constImage(),&_affine,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::alphaChannel(AlphaChannelType alphaType_)
{
  modifyImage();
  SetImageAlphaChannel(image(), alphaType_);
  throwImageException();
}

void Magick::Image::annotate(const std::string &text_,
  const Geometry &location_)
{
  annotate(text_,location_,NorthWestGravity,0.0);
}

void Magick::Image::annotate(const std::string &text_,
  const Geometry &boundingArea_,const GravityType gravity_)
{
  annotate(text_,boundingArea_,gravity_,0.0);
}

void Magick::Image::annotate(const std::string &text_,
  const Geometry &boundingArea_,const GravityType gravity_,
  const double degrees_)
{
  AffineMatrix
    oaffine;

  char
    boundingArea[MaxTextExtent];

  DrawInfo
    *drawInfo;

  modifyImage();

  drawInfo=options()->drawInfo();
  drawInfo->text=const_cast<char *>(text_.c_str());
  drawInfo->geometry=0;

  if (boundingArea_.isValid())
    {
      if (boundingArea_.width() == 0 || boundingArea_.height() == 0)
        {
          FormatLocaleString(boundingArea,MaxTextExtent,"%+.20g%+.20g",
            (double) boundingArea_.xOff(),(double) boundingArea_.yOff());
        }
      else
        {
          (void) CopyMagickString(boundingArea,string(boundingArea_).c_str(),
            MaxTextExtent);
        }
      drawInfo->geometry=boundingArea;
    }

  drawInfo->gravity=gravity_;

  oaffine=drawInfo->affine;
  if (degrees_ != 0.0)
    {
       AffineMatrix
         affine,
         current;

       affine.sx=1.0;
       affine.rx=0.0;
       affine.ry=0.0;
       affine.sy=1.0;
       affine.tx=0.0;
       affine.ty=0.0;

       current=drawInfo->affine;
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

  AnnotateImage(image(),drawInfo);

  // Restore original values
  drawInfo->affine=oaffine;
  drawInfo->text=0;
  drawInfo->geometry=0;

  throwImageException();
}

void Magick::Image::annotate(const std::string &text_,
  const GravityType gravity_)
{
  DrawInfo
    *drawInfo;

  modifyImage();

  drawInfo=options()->drawInfo();
  drawInfo->text=const_cast<char *>(text_.c_str());
  drawInfo->gravity=gravity_;

  AnnotateImage(image(),drawInfo);

  drawInfo->gravity=NorthWestGravity;
  drawInfo->text=0;

  throwImageException();
}

void Magick::Image::artifact(const std::string &name_,
  const std::string &value_)
{
  modifyImage();
  (void) SetImageArtifact(image(),name_.c_str(),value_.c_str());
}

std::string Magick::Image::artifact(const std::string &name_)
{
  const char
    *value;

  value=GetImageArtifact(constImage(),name_.c_str());
  if (value)
    return(std::string(value));
  return(std::string());
}

void Magick::Image::attribute(const std::string name_,const std::string value_)
{
  modifyImage();
  SetImageProperty(image(),name_.c_str(),value_.c_str());
}

std::string Magick::Image::attribute(const std::string name_)
{
  const char
    *value;

  value=GetImageProperty(constImage(),name_.c_str());

  if (value)
    return(std::string(value));

  return(std::string()); // Intentionally no exception
}

void Magick::Image::autoGamma(void)
{
  modifyImage();
  (void) SyncImageSettings(imageInfo(),image());
  (void) AutoGammaImage(image());
  throwImageException();
}

void Magick::Image::autoGammaChannel(const ChannelType channel_)
{
  modifyImage();
  (void) SyncImageSettings(imageInfo(),image());
  (void) AutoGammaImageChannel(image(),channel_);
  throwImageException();
}

void Magick::Image::autoLevel(void)
{
  modifyImage();
  (void) SyncImageSettings(imageInfo(),image());
  (void) AutoLevelImage(image());
  throwImageException();
}

void Magick::Image::autoLevelChannel(const ChannelType channel_)
{
  modifyImage();
  (void) SyncImageSettings(imageInfo(),image());
  (void) AutoLevelImageChannel(image(),channel_);
  throwImageException();
}

void Magick::Image::autoOrient(void)
{
  MagickCore::Image
    *newImage;

  if (image()->orientation == UndefinedOrientation ||
      image()->orientation == TopLeftOrientation)
    return;

  GetPPException;
  (void) SyncImageSettings(imageInfo(),image());
  newImage=AutoOrientImage(constImage(),image()->orientation,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::blackThreshold(const std::string &threshold_)
{
  modifyImage();
  BlackThresholdImage(image(),threshold_.c_str());
  throwImageException();
}

void Magick::Image::blackThresholdChannel(const ChannelType channel_,
  const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  BlackThresholdImageChannel(image(),channel_,threshold_.c_str(),
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::blueShift(const double factor_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=BlueShiftImage(constImage(),factor_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

// Blur image
void Magick::Image::blur(const double radius_, const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=BlurImage(constImage(),radius_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::blurChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=BlurImageChannel(constImage(),channel_,radius_,sigma_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::border(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    borderInfo=geometry_;

  GetPPException;
  newImage=BorderImage(constImage(),&borderInfo,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::brightnessContrast(const double brightness_,
  const double contrast_)
{
  modifyImage();
  BrightnessContrastImage(image(),brightness_,contrast_);
  throwImageException();
}

void Magick::Image::brightnessContrastChannel(const ChannelType channel_,
  const double brightness_,const double contrast_)
{
  modifyImage();
  BrightnessContrastImageChannel(image(),channel_,brightness_,contrast_);
  throwImageException();
}

void Magick::Image::channel(const ChannelType channel_)
{
  modifyImage();
  SeparateImageChannel(image(),channel_);
  throwImageException();
}

void Magick::Image::channelDepth(const ChannelType channel_,
  const size_t depth_)
{
  modifyImage();
  SetImageChannelDepth(image(),channel_,depth_);
  throwImageException();
}

size_t Magick::Image::channelDepth(const ChannelType channel_)
{
  size_t
    channel_depth;

  GetPPException;
  channel_depth=GetImageChannelDepth(constImage(), channel_,&exceptionInfo);
  ThrowPPException;
  return channel_depth;
}

void Magick::Image::charcoal(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=CharcoalImage(constImage(),radius_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::chop(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;
  
  RectangleInfo
    chopInfo=geometry_;

  GetPPException;
  newImage=ChopImage(constImage(),&chopInfo,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::chromaBluePrimary(const double x_,const double y_)
{
  modifyImage();
  image()->chromaticity.blue_primary.x=x_;
  image()->chromaticity.blue_primary.y=y_;
}

void Magick::Image::chromaBluePrimary(double *x_,double *y_) const
{
  *x_=constImage()->chromaticity.blue_primary.x;
  *y_=constImage()->chromaticity.blue_primary.y;
}

void Magick::Image::chromaGreenPrimary(const double x_,const double y_)
{
  modifyImage();
  image()->chromaticity.green_primary.x=x_;
  image()->chromaticity.green_primary.y=y_;
}

void Magick::Image::chromaGreenPrimary(double *x_,double *y_) const
{
  *x_=constImage()->chromaticity.green_primary.x;
  *y_=constImage()->chromaticity.green_primary.y;
}

void Magick::Image::chromaRedPrimary(const double x_,const double y_)
{
  modifyImage();
  image()->chromaticity.red_primary.x=x_;
  image()->chromaticity.red_primary.y=y_;
}

void Magick::Image::chromaRedPrimary(double *x_,double *y_) const
{
  *x_=constImage()->chromaticity.red_primary.x;
  *y_=constImage()->chromaticity.red_primary.y;
}

void Magick::Image::chromaWhitePoint(const double x_,const double y_)
{
  modifyImage();
  image()->chromaticity.white_point.x=x_;
  image()->chromaticity.white_point.y=y_;
}

void Magick::Image::chromaWhitePoint(double *x_,double *y_) const
{
  *x_=constImage()->chromaticity.white_point.x;
  *y_=constImage()->chromaticity.white_point.y;
}

void Magick::Image::cdl(const std::string &cdl_)
{
  modifyImage();
  (void) ColorDecisionListImage(image(),cdl_.c_str());
  throwImageException();
}

void Magick::Image::clamp(void)
{
  modifyImage();
  ClampImage(image());
  throwImageException();
}

void Magick::Image::clampChannel(const ChannelType channel_)
{
  modifyImage();
  ClampImageChannel(image(),channel_);
  throwImageException();
}

void Magick::Image::clip(void )
{
  modifyImage();
  ClipImage(image());
  throwImageException();
}

void Magick::Image::clipPath(const std::string pathname_,const bool inside_)
{
  modifyImage();
  ClipImagePath(image(),pathname_.c_str(),(MagickBooleanType) inside_);
  throwImageException();
}

void Magick::Image::clut(const Image &clutImage_)
{
  modifyImage();
  ClutImage(image(),clutImage_.constImage());
  throwImageException();
}

void Magick::Image::clutChannel(const ChannelType channel_,
  const Image &clutImage_)
{
  modifyImage();
  ClutImageChannel(image(),channel_,clutImage_.constImage());
  throwImageException();
}

void Magick::Image::colorize(const unsigned int opacityRed_,
  const unsigned int opacityGreen_,const unsigned int opacityBlue_,
  const Color &penColor_)
{
  char
    opacity[MaxTextExtent];

  MagickCore::Image
    *newImage;

  if (!penColor_.isValid())
    throwExceptionExplicit( OptionError, "Pen color argument is invalid" );

  FormatLocaleString(opacity,MaxTextExtent,"%u/%u/%u",opacityRed_,
    opacityGreen_,opacityBlue_);

  GetPPException;
  newImage=ColorizeImage(image(),opacity,penColor_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::colorize(const unsigned int opacity_,
  const Color &penColor_)
{
  colorize(opacity_,opacity_,opacity_,penColor_);
}

void Magick::Image::colorMap(const size_t index_,const Color &color_)
{
  if (index_ > (MaxColormapSize-1) )
    throwExceptionExplicit(OptionError,
      "Colormap index must be less than MaxColormapSize");
  
  if (!color_.isValid())
    throwExceptionExplicit(OptionError,"Color argument is invalid");

  modifyImage();

  // Ensure that colormap size is large enough
  if (colorMapSize() < (index_+1))
    colorMapSize(index_+1);

  // Set color at index in colormap
  (image()->colormap)[index_]=color_;
}

Magick::Color Magick::Image::colorMap(const size_t index_) const
{
  if (!constImage()->colormap)
    throwExceptionExplicit(OptionError,"Image does not contain a colormap");

  if (index_ > constImage()->colors-1)
    throwExceptionExplicit(OptionError,"Index out of range");

  return(Color((constImage()->colormap)[index_]));
}

void Magick::Image::colorMatrix(const size_t order_,
  const double *color_matrix_)
{
  KernelInfo
    *kernel_info;

  MagickCore::Image
    *newImage;

  GetPPException;
  
  kernel_info=AcquireKernelInfo("1");
  kernel_info->width=order_;
  kernel_info->height=order_;
  kernel_info->values=(double *) color_matrix_;
  newImage=ColorMatrixImage(constImage(),kernel_info,&exceptionInfo);
  kernel_info->values=(double *) NULL;
  kernel_info=DestroyKernelInfo(kernel_info);
  replaceImage(newImage);
  ThrowPPException;
}

bool Magick::Image::compare(const Image &reference_)
{
  bool
    status;

  Image
    ref=reference_;

  modifyImage();
  ref.modifyImage();
  status=static_cast<bool>(IsImagesEqual(image(),ref.constImage()));
  throwImageException();
  return(status);
}

double Magick::Image::compare(const Image &reference_,const MetricType metric_)
{
  double
    distortion=0.0;

  GetPPException;
  GetImageDistortion(image(),reference_.constImage(),metric_,&distortion,
    &exceptionInfo);
  ThrowPPException;
  return(distortion);
}

double Magick::Image::compareChannel(const ChannelType channel_,
  const Image &reference_,const MetricType metric_)
{
  double
    distortion=0.0;

  GetPPException;
  GetImageChannelDistortion(image(),reference_.constImage(),channel_,metric_,
    &distortion,&exceptionInfo);
  ThrowPPException;
  return(distortion);
}

Magick::Image Magick::Image::compare(const Image &reference_,
  const MetricType metric_,double *distortion)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=CompareImages(image(),reference_.constImage(),metric_,distortion,
    &exceptionInfo);
  ThrowPPException;
  if (newImage == (MagickCore::Image *) NULL)
    return(Magick::Image());
  else
    return(Magick::Image(newImage));
}

Magick::Image Magick::Image::compareChannel(const ChannelType channel_,
  const Image &reference_,const MetricType metric_,double *distortion)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=CompareImageChannels(image(),reference_.constImage(),channel_,
    metric_,distortion,&exceptionInfo);
  ThrowPPException;
  if (newImage == (MagickCore::Image *) NULL)
    return(Magick::Image());
  else
    return(Magick::Image(newImage));
}

void Magick::Image::composite(const Image &compositeImage_,
  const Geometry &offset_,const CompositeOperator compose_)
{
  size_t
    height=rows(),
    width=columns();

  ssize_t
    x=offset_.xOff(),
    y=offset_.yOff();

  modifyImage();
  ParseMetaGeometry(static_cast<std::string>(offset_).c_str(),&x,&y,&width,
    &height);

  CompositeImage(image(),compose_,compositeImage_.constImage(),x,y);
  throwImageException();
}

void Magick::Image::composite(const Image &compositeImage_,
  const GravityType gravity_,const CompositeOperator compose_)
{
  RectangleInfo
    geometry;

  modifyImage();

  SetGeometry(compositeImage_.constImage(),&geometry);
  GravityAdjustGeometry(columns(),rows(),gravity_,&geometry);

  CompositeImage(image(),compose_,compositeImage_.constImage(),geometry.x,
    geometry.y);
  throwImageException();
}

void Magick::Image::composite(const Image &compositeImage_,
  const ssize_t xOffset_,const ssize_t yOffset_,
  const CompositeOperator compose_)
{
  // Image supplied as compositeImage is composited with current image and
  // results in updating current image.
  modifyImage();

  CompositeImage(image(),compose_,compositeImage_.constImage(),xOffset_,
    yOffset_);
  throwImageException();
}

void Magick::Image::contrast(const size_t sharpen_)
{
  modifyImage();
  ContrastImage(image(),(MagickBooleanType) sharpen_);
  throwImageException();
}

void Magick::Image::contrastStretch(const double black_point_,
  const double white_point_)
{
  modifyImage();
  ContrastStretchImageChannel(image(),DefaultChannels,black_point_,
    white_point_);
  throwImageException();
}

void Magick::Image::contrastStretchChannel(const ChannelType channel_,
  const double black_point_,const double white_point_)
{
  modifyImage();
  ContrastStretchImageChannel(image(),channel_,black_point_,white_point_);
  throwImageException();
}

void Magick::Image::convolve(const size_t order_,const double *kernel_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ConvolveImage(constImage(),order_,kernel_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::crop(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    cropInfo=geometry_;

  GetPPException;
  newImage=CropImage(constImage(),&cropInfo,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::cycleColormap(const ssize_t amount_)
{
  modifyImage();
  CycleColormapImage(image(),amount_);
  throwImageException();
}

void Magick::Image::decipher(const std::string &passphrase_)
{
  modifyImage();
  GetPPException;
  DecipherImage(image(),passphrase_.c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::defineSet(const std::string &magick_,
  const std::string &key_,bool flag_)
{
  std::string
    definition;

  modifyImage();
  definition=magick_ + ":" + key_;
  if (flag_)
    (void) SetImageOption(imageInfo(),definition.c_str(),"");
  else
    DeleteImageOption(imageInfo(),definition.c_str());
}

bool Magick::Image::defineSet(const std::string &magick_,
  const std::string &key_) const
{
  const char
    *option;

  std::string
    key;

  key=magick_ + ":" + key_;
  option=GetImageOption(constImageInfo(),key.c_str());
  if (option)
    return(true);
  return(false);
}

void Magick::Image::defineValue(const std::string &magick_,
  const std::string &key_,const std::string &value_)
{
  std::string
    format;

  modifyImage();
  format=magick_ + ":" + key_;
  (void) SetImageOption(imageInfo(),format.c_str(),value_.c_str());
}

std::string Magick::Image::defineValue(const std::string &magick_,
  const std::string &key_) const
{
  const char
    *option;

  std::string
    definition;

  definition=magick_ + ":" + key_;
  option=GetImageOption(constImageInfo(),definition.c_str());
  if (option)
    return(std::string(option));
  return(std::string());
}

void Magick::Image::deskew(const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=DeskewImage(constImage(),threshold_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::despeckle(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=DespeckleImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::display(void)
{
  DisplayImages(imageInfo(),image());
}

void Magick::Image::distort(const DistortImageMethod method_,
  const size_t number_arguments_,const double *arguments_,const bool bestfit_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=DistortImage(constImage(),method_,number_arguments_,arguments_,
    bestfit_ == true ? MagickTrue : MagickFalse,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::draw(const Magick::Drawable &drawable_)
{
  DrawingWand
    *wand;

  modifyImage();

  wand=DrawAllocateWand(options()->drawInfo(),image());

  if(wand)
    {
      drawable_.operator()(wand);

      if (constImage()->exception.severity == UndefinedException)
        DrawRender(wand);

      wand=DestroyDrawingWand(wand);
    }

  throwImageException();
}

void Magick::Image::draw(const std::list<Magick::Drawable> &drawable_)
{
  DrawingWand
    *wand;

  modifyImage();

  wand=DrawAllocateWand(options()->drawInfo(),image());

  if(wand)
    {
      for (std::list<Magick::Drawable>::const_iterator p = drawable_.begin();
           p != drawable_.end(); p++)
        {
          p->operator()(wand);
          if (constImage()->exception.severity != UndefinedException)
            break;
        }

      if (constImage()->exception.severity == UndefinedException)
        DrawRender(wand);

      wand=DestroyDrawingWand(wand);
    }

  throwImageException();
}

void Magick::Image::edge(const double radius_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=EdgeImage(constImage(),radius_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::emboss(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=EmbossImage(constImage(),radius_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::encipher(const std::string &passphrase_)
{
  modifyImage();
  GetPPException;
  EncipherImage(image(),passphrase_.c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::enhance(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=EnhanceImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::equalize(void)
{
  modifyImage();
  EqualizeImage(image());
  throwImageException();
}

void Magick::Image::erase(void)
{
  modifyImage();
  SetImageBackgroundColor(image());
  throwImageException();
}

void Magick::Image::extent(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    extentInfo;

  modifyImage();

  GetPPException;
  extentInfo=geometry_;
  extentInfo.x=geometry_.xOff();
  extentInfo.y=geometry_.yOff();
  newImage=ExtentImage(constImage(),&extentInfo,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::extent(const Geometry &geometry_,
  const Color &backgroundColor_)
{
  backgroundColor(backgroundColor_);
  extent(geometry_);
}

void Magick::Image::extent(const Geometry &geometry_,
  const Color &backgroundColor_,const GravityType gravity_)
{
  image()->gravity=gravity_;
  backgroundColor(backgroundColor_);
  extent(geometry_,gravity_);
}

void Magick::Image::extent(const Geometry &geometry_,
  const GravityType gravity_)
{
  RectangleInfo
    geometry;

  SetGeometry(image(),&geometry);
  geometry.width=geometry_.width();
  geometry.height=geometry_.height();
  GravityAdjustGeometry(image()->columns,image()->rows,gravity_,&geometry);
  extent(geometry);
}

void Magick::Image::flip(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=FlipImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::floodFillColor(const Geometry &point_,
  const Magick::Color &fillColor_)
{
  floodFillTexture(point_,Image(Geometry(1,1),fillColor_));
}

void Magick::Image::floodFillColor(const ssize_t x_,const ssize_t y_,
  const Magick::Color &fillColor_)
{
  floodFillTexture(x_,y_,Image(Geometry(1,1),fillColor_));
}

void Magick::Image::floodFillColor(const Geometry &point_,
  const Magick::Color &fillColor_,const Magick::Color &borderColor_)
{
  floodFillTexture(point_,Image(Geometry(1,1),fillColor_),borderColor_);
}

void Magick::Image::floodFillColor(const ssize_t x_,const ssize_t y_,
  const Magick::Color &fillColor_,const Magick::Color &borderColor_)
{
  floodFillTexture(x_,y_,Image(Geometry(1,1),fillColor_),borderColor_);
}

void Magick::Image::floodFillOpacity(const ssize_t x_,const ssize_t y_,
  const unsigned int opacity_,const PaintMethod method_)
{
  MagickPixelPacket
    target;

  PixelPacket
    pixel;

  modifyImage();
  GetMagickPixelPacket(image(),&target);
  pixel=static_cast<PixelPacket>(pixelColor(x_,y_));
  target.red=pixel.red;
  target.green=pixel.green;
  target.blue=pixel.blue;
  target.opacity=opacity_;
  FloodfillPaintImage(image(),DefaultChannels,options()->drawInfo(),&target,
    static_cast<ssize_t>(x_), static_cast<ssize_t>(y_),
    method_  == FloodfillMethod ? MagickFalse : MagickTrue);
  throwImageException();
}

void Magick::Image::floodFillTexture(const Magick::Geometry &point_,
  const Magick::Image &texture_)
{
  floodFillTexture(point_.xOff(),point_.yOff(),texture_);
}

void Magick::Image::floodFillTexture(const ssize_t x_,const ssize_t y_,
  const Magick::Image &texture_)
{
  MagickCore::Image
    *fillPattern;

  PixelPacket
    *p;

  modifyImage();

  // Set drawing fill pattern
  fillPattern=(MagickCore::Image *)NULL;
  if (options()->fillPattern() != (MagickCore::Image *)NULL)
    {
      GetPPException;
      fillPattern=CloneImage(options()->fillPattern(),0,0,MagickTrue,
        &exceptionInfo);
      ThrowPPException;
    }
  options()->fillPattern(texture_.constImage());

  // Fill image
  Pixels pixels(*this);
  p=pixels.get(x_,y_,1,1);
  if (p)
    {
      MagickPixelPacket
        target;

      GetMagickPixelPacket(constImage(),&target);
      target.red=p->red;
      target.green=p->green;
      target.blue=p->blue;

      FloodfillPaintImage(image(),DefaultChannels,options()->drawInfo(),
        &target,static_cast<ssize_t>(x_),static_cast<ssize_t>(y_),MagickFalse);
    }
  options()->fillPattern(fillPattern);
  throwImageException();
}

void Magick::Image::floodFillTexture(const Magick::Geometry &point_,
  const Magick::Image &texture_,const Magick::Color &borderColor_)
{
  floodFillTexture(point_.xOff(),point_.yOff(),texture_,borderColor_);
}

void Magick::Image::floodFillTexture(const ssize_t x_,const ssize_t y_,
  const Magick::Image &texture_,const Magick::Color &borderColor_)
{
  MagickCore::Image
    *fillPattern;

  MagickPixelPacket
    target;

  modifyImage();

  // Set drawing fill pattern
  fillPattern=(MagickCore::Image *)NULL;
  if (options()->fillPattern() != (MagickCore::Image *)NULL)
    {
      GetPPException;
      fillPattern=CloneImage(options()->fillPattern(),0,0,MagickTrue,
        &exceptionInfo);
      ThrowPPException;
    }
  options()->fillPattern(texture_.constImage());

  GetMagickPixelPacket(constImage(),&target);
  target.red=static_cast<PixelPacket>(borderColor_).red;
  target.green=static_cast<PixelPacket>(borderColor_).green;
  target.blue=static_cast<PixelPacket>(borderColor_).blue;
  FloodfillPaintImage(image(),DefaultChannels,options()->drawInfo(),&target,
    static_cast<ssize_t>(x_),static_cast<ssize_t>(y_),MagickTrue);
  options()->fillPattern(fillPattern);
  throwImageException();
}

void Magick::Image::flop(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=FlopImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::fontTypeMetrics(const std::string &text_,
  TypeMetric *metrics)
{
  DrawInfo
    *drawInfo;

  drawInfo=options()->drawInfo();
  drawInfo->text=const_cast<char *>(text_.c_str());
  GetTypeMetrics(image(),drawInfo,&(metrics->_typeMetric));
  drawInfo->text=0;
}

void Magick::Image::frame(const Geometry &geometry_)
{
  FrameInfo
    info;

  MagickCore::Image
    *newImage;

  info.x=static_cast<ssize_t>(geometry_.width());
  info.y=static_cast<ssize_t>(geometry_.height());
  info.width=columns() + ( static_cast<size_t>(info.x) << 1 );
  info.height=rows() + ( static_cast<size_t>(info.y) << 1 );
  info.outer_bevel=geometry_.xOff();
  info.inner_bevel=geometry_.yOff();

  GetPPException;
  newImage=FrameImage(constImage(),&info,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::frame(const size_t width_,const size_t height_,
  const ssize_t outerBevel_,const ssize_t innerBevel_)
{
  FrameInfo
    info;

  MagickCore::Image
    *newImage;

  info.x=static_cast<ssize_t>(width_);
  info.y=static_cast<ssize_t>(height_);
  info.width=columns() + ( static_cast<size_t>(info.x) << 1 );
  info.height=rows() + ( static_cast<size_t>(info.y) << 1 );
  info.outer_bevel=static_cast<ssize_t>(outerBevel_);
  info.inner_bevel=static_cast<ssize_t>(innerBevel_);

  GetPPException;
  newImage=FrameImage(constImage(),&info,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::fx(const std::string expression)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=FxImageChannel(constImage(),DefaultChannels,expression.c_str(),
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::fx(const std::string expression,
  const Magick::ChannelType channel)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=FxImageChannel(constImage(),channel,expression.c_str(),
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::gamma(const double gamma_)
{
  char
    gamma[MaxTextExtent + 1];

  FormatLocaleString(gamma,MaxTextExtent,"%3.6f",gamma_);

  modifyImage();
  GammaImage(image(),gamma);
}

void Magick::Image::gamma(const double gammaRed_,const double gammaGreen_,
  const double gammaBlue_)
{
  char
    gamma[MaxTextExtent + 1];

  FormatLocaleString(gamma,MaxTextExtent,"%3.6f/%3.6f/%3.6f/",gammaRed_,
    gammaGreen_,gammaBlue_);

  modifyImage();
  GammaImage(image(),gamma);
  throwImageException();
}

void Magick::Image::gaussianBlur(const double width_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=GaussianBlurImage(constImage(),width_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::gaussianBlurChannel(const ChannelType channel_,
  const double width_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=GaussianBlurImageChannel(constImage(),channel_,width_,sigma_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

const Magick::IndexPacket* Magick::Image::getConstIndexes(void) const
{
  const Magick::IndexPacket
    *result;

  result=GetVirtualIndexQueue(constImage());
  if (!result)
    throwImageException();

  return(result);
}

const Magick::PixelPacket* Magick::Image::getConstPixels(const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_) const
{
  const PixelPacket
    *result;

  GetPPException;
  result=(*GetVirtualPixels)(constImage(),x_,y_,columns_,rows_,&exceptionInfo);
  ThrowPPException;
  return(result);
}

Magick::IndexPacket *Magick::Image::getIndexes(void)
{
  Magick::IndexPacket
    *result;

  result=GetAuthenticIndexQueue(image());

  if(!result)
    throwImageException();

  return(result);
}

Magick::PixelPacket *Magick::Image::getPixels(const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_)
{
  PixelPacket
    *result;

  modifyImage();
  GetPPException;
  result=(*GetAuthenticPixels)(image(),x_,y_,columns_,rows_,&exceptionInfo);
  ThrowPPException;
  return(result);
}

void Magick::Image::haldClut(const Image &clutImage_)
{
  modifyImage();
  (void) HaldClutImage(image(),clutImage_.constImage());
  throwImageException();
}

void Magick::Image::implode(const double factor_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ImplodeImage(constImage(),factor_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::inverseFourierTransform(const Image &phase_)
{
  inverseFourierTransform(phase_,true);
}

void Magick::Image::inverseFourierTransform(const Image &phase_,
  const bool magnitude_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=InverseFourierTransformImage(constImage(),phase_.constImage(),
    magnitude_ == true ? MagickTrue : MagickFalse,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::level(const double black_point,const double white_point,
  const double gamma)
{
  char
    levels[MaxTextExtent];

  modifyImage();
  FormatLocaleString(levels,MaxTextExtent,"%g,%g,%g",black_point,white_point,
    gamma);
  (void) LevelImage(image(),levels);
  throwImageException();
}

void Magick::Image::levelChannel(const Magick::ChannelType channel,
  const double black_point,const double white_point,const double gamma)
{
  modifyImage();
  (void) LevelImageChannel(image(),channel,black_point,white_point,gamma);
  throwImageException();
}

void Magick::Image::levelColors(const Color &blackColor_,
  const Color &whiteColor_,const bool invert_)
{
  MagickPixelPacket
    black,
    white;

  PixelPacket
    pixel;

  modifyImage();

  GetMagickPixelPacket(image(),&black);
  pixel=static_cast<PixelPacket>(blackColor_);
  black.red=pixel.red;
  black.green=pixel.green;
  black.blue=pixel.blue;
  black.opacity=pixel.opacity;

  GetMagickPixelPacket(image(),&white);
  pixel=static_cast<PixelPacket>(whiteColor_);
  white.red=pixel.red;
  white.green=pixel.green;
  white.blue=pixel.blue;
  white.opacity=pixel.opacity;

  (void) LevelColorsImage(image(),&black,&white,
    invert_ == true ? MagickTrue : MagickFalse);
  throwImageException();
}

void Magick::Image::levelColorsChannel(const ChannelType channel_,
  const Color &blackColor_,const Color &whiteColor_,const bool invert_)
{
  MagickPixelPacket
    black,
    white;

  PixelPacket
    pixel;

  modifyImage();

  GetMagickPixelPacket(image(),&black);
  pixel=static_cast<PixelPacket>(blackColor_);
  black.red=pixel.red;
  black.green=pixel.green;
  black.blue=pixel.blue;
  black.opacity=pixel.opacity;

  GetMagickPixelPacket(image(),&white);
  pixel=static_cast<PixelPacket>(whiteColor_);
  white.red=pixel.red;
  white.green=pixel.green;
  white.blue=pixel.blue;
  white.opacity=pixel.opacity;

  (void) LevelColorsImageChannel(image(),channel_,&black,&white,
    invert_ == true ? MagickTrue : MagickFalse);
  throwImageException();
}

void Magick::Image::linearStretch(const double blackPoint_,
  const double whitePoint_)
{
  modifyImage();
  LinearStretchImage(image(),blackPoint_,whitePoint_);
  throwImageException();
}

void Magick::Image::liquidRescale(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    height=rows(),
    width=columns();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=LiquidRescaleImage(constImage(),width,height,x,y,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::magnify(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=MagnifyImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::map(const Image &mapImage_,const bool dither_)
{
  modifyImage();
  options()->quantizeDither(dither_);
  RemapImage(options()->quantizeInfo(),image(),mapImage_.constImage());
  throwImageException();
}

void Magick::Image::matteFloodfill(const Color &target_,
  const unsigned int opacity_,const ssize_t x_,const ssize_t y_,
  const Magick::PaintMethod method_)
{
  MagickPixelPacket
    target;

  modifyImage();
  GetMagickPixelPacket(constImage(),&target);
  target.red=static_cast<PixelPacket>(target_).red;
  target.green=static_cast<PixelPacket>(target_).green;
  target.blue=static_cast<PixelPacket>(target_).blue;
  target.opacity=opacity_;
  FloodfillPaintImage(image(),OpacityChannel,options()->drawInfo(),&target,x_,
    y_,method_ == FloodfillMethod ? MagickFalse : MagickTrue);
  throwImageException();
}

void Magick::Image::medianFilter(const double radius_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=StatisticImage(constImage(),MedianStatistic,(size_t) radius_,
    (size_t) radius_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::mergeLayers(const ImageLayerMethod layerMethod_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=MergeImageLayers(image(),layerMethod_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::minify(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=MinifyImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::modulate(const double brightness_,const double saturation_,
  const double hue_)
{
  char
    modulate[MaxTextExtent + 1];

  FormatLocaleString(modulate,MaxTextExtent,"%3.6f,%3.6f,%3.6f",brightness_,
    saturation_,hue_);

  modifyImage();
  ModulateImage(image(),modulate);
  throwImageException();
}

void Magick::Image::motionBlur(const double radius_,const double sigma_,
  const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=MotionBlurImage(constImage(),radius_,sigma_,angle_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::negate(const bool grayscale_)
{
  modifyImage();
  NegateImage(image(),(MagickBooleanType) grayscale_);
  throwImageException();
}

void Magick::Image::negateChannel(const ChannelType channel_,
  const bool grayscale_)
{
  modifyImage();
  NegateImageChannel(image(),channel_,(MagickBooleanType) grayscale_);
  throwImageException();
}

void Magick::Image::normalize(void)
{
  modifyImage();
  NormalizeImage(image());
  throwImageException();
}

void Magick::Image::oilPaint(const double radius_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=OilPaintImage(constImage(),radius_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::opacity(const unsigned int opacity_)
{
  modifyImage();
  SetImageOpacity(image(),opacity_);
}

void Magick::Image::opaque(const Color &opaqueColor_,const Color &penColor_)
{
  MagickPixelPacket
    opaque,
    pen;

  std::string
    opaqueColor,
    penColor;

  if (!opaqueColor_.isValid())
    throwExceptionExplicit(OptionError,"Opaque color argument is invalid");
  
  if (!penColor_.isValid())
    throwExceptionExplicit(OptionError,"Pen color argument is invalid");

  opaqueColor=opaqueColor_;
  penColor=penColor_;

  (void) QueryMagickColor(opaqueColor.c_str(),&opaque,&image()->exception);
  (void) QueryMagickColor(penColor.c_str(),&pen,&image()->exception);
  modifyImage();
  OpaquePaintImage(image(),&opaque,&pen,MagickFalse);
  throwImageException();
}

void Magick::Image::perceptible(const double epsilon_)
{
  modifyImage();
  PerceptibleImage(image(),epsilon_);
  throwImageException();
}

void Magick::Image::perceptibleChannel(const ChannelType channel_,
  const double epsilon_)
{
  modifyImage();
  PerceptibleImageChannel(image(),channel_,epsilon_);
  throwImageException();
}

void Magick::Image::ping(const Blob& blob_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=PingBlob(imageInfo(),blob_.data(),blob_.length(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::ping(const std::string &imageSpec_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  options()->fileName(imageSpec_);
  newImage=PingImage(imageInfo(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::pixelColor(const ssize_t x_,const ssize_t y_,
  const Color &color_)
{
  // Test arguments to ensure they are within the image.
  if (y_ > (ssize_t) rows() || x_ > (ssize_t) columns())
    throwExceptionExplicit(OptionError,"Access outside of image boundary");

  modifyImage();

  // Set image to DirectClass
  classType(DirectClass);

  // Get pixel view
  Pixels pixels(*this);
  // Set pixel value
  *(pixels.get(x_,y_,1,1))=color_;
  // Tell ImageMagick that pixels have been updated
  pixels.sync();
}

Magick::Color Magick::Image::pixelColor(const ssize_t x_,
  const ssize_t y_) const
{
  ClassType
    storage_class;

  storage_class=classType();
  if (storage_class == DirectClass)
    {
      const PixelPacket
        *pixel;

      pixel=getConstPixels(x_,y_,1,1);
      if (pixel)
        return(Color(*pixel));
    }
  else if (storage_class == PseudoClass)
    {
      const IndexPacket
        *indexes;

      indexes=getConstIndexes();
      if(indexes)
        return(colorMap((size_t) *indexes));
    }

  return(Color()); // invalid
}

void Magick::Image::polaroid(const std::string &caption_,const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  (void) SetImageProperty(image(),"Caption",caption_.c_str());
  newImage=PolaroidImage(constImage(),options()->drawInfo(),angle_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::posterize(const size_t levels_,const bool dither_)
{
  modifyImage();
  PosterizeImage(image(),levels_,(MagickBooleanType) dither_);
  throwImageException();
}

void Magick::Image::posterizeChannel(const ChannelType channel_,
  const size_t levels_,const bool dither_)
{
  modifyImage();
  PosterizeImageChannel(image(),channel_,levels_,
    (MagickBooleanType) dither_);
  throwImageException();
}

void Magick::Image::process(std::string name_,const ssize_t argc,
  const char **argv)
{
  size_t
    status;

  modifyImage();

  status=InvokeDynamicImageFilter(name_.c_str(),&image(),argc, argv,
    &image()->exception);

  if (status == false)
    throwException(image()->exception);
}

void Magick::Image::profile(const std::string name_,
  const Magick::Blob &profile_)
{
  ssize_t
    result;

  modifyImage();
  result=ProfileImage(image(),name_.c_str(),(unsigned char *)profile_.data(),
    profile_.length(),MagickTrue);

  if (!result)
    throwImageException();
}

Magick::Blob Magick::Image::profile(const std::string name_) const
{
  const StringInfo
    *profile;

  profile=GetImageProfile(constImage(),name_.c_str());

  if (profile == (StringInfo *) NULL)
    return(Blob());
  return(Blob((void*) GetStringInfoDatum(profile),GetStringInfoLength(
    profile)));
}

void Magick::Image::quantize(const bool measureError_)
{
  modifyImage();
 
  if (measureError_)
    options()->quantizeInfo()->measure_error=MagickTrue;
  else
    options()->quantizeInfo()->measure_error=MagickFalse;

  QuantizeImage(options()->quantizeInfo(),image());

  throwImageException();
}

void Magick::Image::quantumOperator(const ChannelType channel_,
  const MagickEvaluateOperator operator_,double rvalue_)
{
  GetPPException;
  EvaluateImageChannel(image(),channel_,operator_,rvalue_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::quantumOperator (const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_,const ChannelType channel_,
  const MagickEvaluateOperator operator_,const double rvalue_)
{
  MagickCore::Image
    *cropImage;

  RectangleInfo
    geometry;

  GetPPException;
  geometry.width=columns_;
  geometry.height=rows_;
  geometry.x=x_;
  geometry.y=y_;
  cropImage=CropImage(image(),&geometry,&exceptionInfo);
  EvaluateImageChannel(cropImage,channel_,operator_,rvalue_,&exceptionInfo);
  (void) CompositeImage(image(),image()->matte != MagickFalse ?
    OverCompositeOp : CopyCompositeOp,cropImage,geometry.x, geometry.y);
  cropImage=DestroyImageList(cropImage);
  ThrowPPException;
}

void Magick::Image::raise(const Geometry &geometry_,const bool raisedFlag_)
{
  RectangleInfo
    raiseInfo;

  raiseInfo=geometry_;
  modifyImage();
  RaiseImage(image(),&raiseInfo,raisedFlag_ == true ?
    MagickTrue : MagickFalse);
  throwImageException();
}

void Magick::Image::randomThreshold( const Geometry &thresholds_ )
{
  GetPPException;
  modifyImage();
  (void) RandomThresholdImage(image(),static_cast<std::string>(
    thresholds_).c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::randomThresholdChannel(const Geometry &thresholds_,
  const ChannelType channel_)
{
  GetPPException;
  modifyImage();
  (void) RandomThresholdImageChannel(image(),channel_,static_cast<std::string>(
    thresholds_).c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::read(const Blob &blob_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=BlobToImage(imageInfo(),static_cast<const void *>(blob_.data()),
    blob_.length(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
  if (newImage)
    throwException(newImage->exception);
}

void Magick::Image::read(const Blob &blob_,const Geometry &size_)
{
  size(size_);
  read(blob_);
}

void Magick::Image::read(const Blob &blob_,const Geometry &size_,
  const size_t depth_)
{
  size(size_);
  depth(depth_);
  read(blob_);
}

void Magick::Image::read(const Blob &blob_,const Geometry &size_,
  const size_t depth_,const std::string &magick_)
{
  size(size_);
  depth(depth_);
  magick(magick_);
  fileName(magick_ + ':');
  read(blob_);
}

void Magick::Image::read(const Blob &blob_,const Geometry &size_,
  const std::string &magick_)
{
  size(size_);
  magick(magick_);
  fileName(magick_ + ':');
  read(blob_);
}

void Magick::Image::read(const Geometry &size_,const std::string &imageSpec_)
{
  size(size_);
  read(imageSpec_);
}

void Magick::Image::read(const size_t width_,const size_t height_,
  const std::string &map_,const StorageType type_,const void *pixels_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ConstituteImage(width_,height_,map_.c_str(),type_,pixels_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
  if (newImage)
    throwException(newImage->exception);
}

void Magick::Image::read(const std::string &imageSpec_)
{
  MagickCore::Image
    *newImage;

  options()->fileName(imageSpec_);

  GetPPException;
  newImage=ReadImage(imageInfo(),&exceptionInfo);

  // Ensure that multiple image frames were not read.
  if (newImage && newImage->next)
    {
      MagickCore::Image
        *next;
      // Destroy any extra image frames
      next=newImage->next;
      newImage->next=0;
      next->previous=0;
      DestroyImageList(next);
    }
  replaceImage(newImage);
  ThrowPPException;
  if (newImage)
    throwException(newImage->exception);
}

void Magick::Image::readPixels(const Magick::QuantumType quantum_,
  const unsigned char *source_)
{
  QuantumInfo
    *quantum_info;

  GetPPException;
  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  ImportQuantumPixels(image(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,source_,&exceptionInfo);
  quantum_info=DestroyQuantumInfo(quantum_info);
  ThrowPPException;
}

void Magick::Image::reduceNoise(const double order_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=StatisticImage(constImage(),NonpeakStatistic,(size_t) order_,
    (size_t) order_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::resample(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    width=columns(),
    height=rows();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x, &y,&width,
    &height);

  GetPPException;
  newImage=ResampleImage(constImage(),width,height,image()->filter,1.0,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::resize(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    width=columns(),
    height=rows();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x, &y,&width,
    &height);

  GetPPException;
  newImage=ResizeImage(constImage(),width,height,image()->filter,1.0,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::roll(const Geometry &roll_)
{
  MagickCore::Image
    *newImage;

  ssize_t
    xOff=roll_.xOff(),
    yOff=roll_.yOff();

  if (roll_.xNegative())
    xOff=0-xOff;
  if (roll_.yNegative())
    yOff=0-yOff;

  GetPPException;
  newImage=RollImage(constImage(),xOff,yOff,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::roll(const size_t columns_,const size_t rows_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=RollImage(constImage(),static_cast<ssize_t>(columns_),
    static_cast<ssize_t>(rows_),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::rotate(const double degrees_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=RotateImage(constImage(),degrees_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::sample(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    height=rows(),
    width=columns();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=SampleImage(constImage(),width,height,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::scale(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    height=rows(),
    width=columns();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=ScaleImage(constImage(),width,height,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::segment(const double clusterThreshold_,
  const double smoothingThreshold_)
{
  modifyImage();
  SegmentImage(image(),options()->quantizeColorSpace(),
    (MagickBooleanType) options()->verbose(),clusterThreshold_,
    smoothingThreshold_);
  throwImageException();
  SyncImage(image());
  throwImageException();
}

Magick::PixelPacket *Magick::Image::setPixels(const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_)
{
  PixelPacket
    *result;

  modifyImage();
  GetPPException;
  result=(*QueueAuthenticPixels)(image(),x_, y_,columns_,rows_,&exceptionInfo);
  ThrowPPException;
  return(result);
}

void Magick::Image::shade(const double azimuth_,const double elevation_,
  const bool colorShading_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ShadeImage(constImage(),colorShading_ == true ?
    MagickTrue : MagickFalse,azimuth_,elevation_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::shadow(const double percent_opacity_,const double sigma_,
  const ssize_t x_,const ssize_t y_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ShadowImage(constImage(),percent_opacity_,sigma_,x_,y_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::sharpen(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SharpenImage(constImage(),radius_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::sharpenChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SharpenImageChannel(constImage(),channel_,radius_,sigma_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::shave(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;
  
  RectangleInfo
    shaveInfo=geometry_;

  GetPPException;
  newImage=ShaveImage(constImage(),&shaveInfo,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::shear(const double xShearAngle_,const double yShearAngle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ShearImage(constImage(),xShearAngle_,yShearAngle_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::sigmoidalContrast(const size_t sharpen_,
  const double contrast,const double midpoint)
{
  modifyImage();
  (void) SigmoidalContrastImageChannel(image(),DefaultChannels,
    (MagickBooleanType) sharpen_,contrast,midpoint);
  throwImageException();
}

std::string Magick::Image::signature(const bool force_) const
{
  const char
    *property;

  Lock(&_imgRef->_mutexLock);

  // Re-calculate image signature if necessary
  if (force_ ||  !GetImageProperty(constImage(), "Signature") ||
    constImage()->taint)
    SignatureImage(const_cast<MagickCore::Image *>(constImage()));

  property=GetImageProperty(constImage(),"Signature");

  return(std::string(property));
}

void Magick::Image::solarize(const double factor_)
{
  modifyImage();
  SolarizeImage(image(),factor_);
  throwImageException();
}

void Magick::Image::sparseColor(const ChannelType channel,
  const SparseColorMethod method,const size_t number_arguments,
  const double *arguments)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SparseColorImage(constImage(),channel,method,number_arguments,
    arguments,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::splice(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    spliceInfo=geometry_;

  GetPPException;
  newImage=SpliceImage(constImage(),&spliceInfo,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::spread(const size_t amount_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SpreadImage(constImage(),amount_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::statistics(ImageStatistics *statistics) const
{
  double
    maximum,
    minimum;

  GetPPException;
  (void) GetImageChannelRange(constImage(),RedChannel,&minimum,&maximum,
    &exceptionInfo);
  statistics->red.minimum=minimum;
  statistics->red.maximum=maximum;
  (void) GetImageChannelMean(constImage(),RedChannel,&statistics->red.mean,
    &statistics->red.standard_deviation,&exceptionInfo);
  (void) GetImageChannelKurtosis(constImage(),RedChannel,
    &statistics->red.kurtosis,&statistics->red.skewness,&exceptionInfo);
  (void) GetImageChannelRange(constImage(),GreenChannel,&minimum,&maximum,
    &exceptionInfo);
  statistics->green.minimum=minimum;
  statistics->green.maximum=maximum;
  (void) GetImageChannelMean(constImage(),GreenChannel,&statistics->green.mean,
    &statistics->green.standard_deviation,&exceptionInfo);
  (void) GetImageChannelKurtosis(constImage(),GreenChannel,
    &statistics->green.kurtosis,&statistics->green.skewness,&exceptionInfo);
  (void) GetImageChannelRange(constImage(),BlueChannel,&minimum,&maximum,
    &exceptionInfo);
  statistics->blue.minimum=minimum;
  statistics->blue.maximum=maximum;
  (void) GetImageChannelMean(constImage(),BlueChannel,&statistics->blue.mean,
    &statistics->blue.standard_deviation,&exceptionInfo);
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
    &statistics->opacity.kurtosis,&statistics->opacity.skewness,
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::stegano(const Image &watermark_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SteganoImage(constImage(),watermark_.constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::stereo(const Image &rightImage_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=StereoImage(constImage(),rightImage_.constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::strip(void)
{
  modifyImage();
  StripImage(image());
  throwImageException();
}

void Magick::Image::swirl(const double degrees_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SwirlImage(constImage(),degrees_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::syncPixels(void)
{
  GetPPException;
  (*SyncAuthenticPixels)(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::texture(const Image &texture_)
{
  modifyImage();
  TextureImage(image(),texture_.constImage());
  throwImageException();
}

void Magick::Image::threshold(const double threshold_)
{
  modifyImage();
  BilevelImage(image(),threshold_);
  throwImageException();
}

void Magick::Image::thumbnail(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    height=rows(),
    width=columns();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=ThumbnailImage(constImage(),width,height,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::transform(const Geometry &imageGeometry_)
{
  modifyImage();
  TransformImage(&(image()),0,std::string(imageGeometry_).c_str());
  throwImageException();
}

void Magick::Image::transform(const Geometry &imageGeometry_,
  const Geometry &cropGeometry_)
{
  modifyImage();
  TransformImage(&(image()),std::string(cropGeometry_).c_str(),
    std::string(imageGeometry_).c_str());
  throwImageException();
}

void Magick::Image::transformOrigin(const double x_,const double y_)
{
  modifyImage();
  options()->transformOrigin(x_,y_);
}

void Magick::Image::transformReset(void)
{
  modifyImage();
  options()->transformReset();
}

void Magick::Image::transformScale(const double sx_,const double sy_)
{
  modifyImage();
  options()->transformScale(sx_,sy_);
}

void Magick::Image::transparent(const Color &color_)
{
  MagickPixelPacket
    target;

  std::string
    color;

  if (!color_.isValid())
    throwExceptionExplicit(OptionError,"Color argument is invalid");

  color=color_;

  (void) QueryMagickColor(std::string(color_).c_str(),&target,
    &image()->exception);
  modifyImage();
  TransparentPaintImage(image(),&target,TransparentOpacity,MagickFalse);
  throwImageException();
}

void Magick::Image::transparentChroma(const Color &colorLow_,
  const Color &colorHigh_)
{
  MagickPixelPacket
    targetHigh,
    targetLow;

  std::string
    colorHigh,
    colorLow;

  if (!colorLow_.isValid() || !colorHigh_.isValid())
    throwExceptionExplicit(OptionError,"Color argument is invalid");

  colorLow=colorLow_;
  colorHigh=colorHigh_;

  (void) QueryMagickColor(colorLow.c_str(),&targetLow,&image()->exception);
  (void) QueryMagickColor(colorHigh.c_str(),&targetHigh,&image()->exception);
  modifyImage();
  TransparentPaintImageChroma(image(),&targetLow,&targetHigh,
    TransparentOpacity,MagickFalse);
  throwImageException();
}

void Magick::Image::trim(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=TrimImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::unsharpmask(const double radius_,const double sigma_,
  const double amount_,const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=UnsharpMaskImage(constImage(),radius_,sigma_,amount_,threshold_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::unsharpmaskChannel(const ChannelType channel_,
  const double radius_,const double sigma_,const double amount_,
  const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=UnsharpMaskImageChannel(constImage(),channel_,radius_,sigma_,
    amount_,threshold_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::wave(const double amplitude_,const double wavelength_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=WaveImage(constImage(),amplitude_,wavelength_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::whiteThreshold(const std::string &threshold_)
{
  modifyImage();
  WhiteThresholdImage(image(),threshold_.c_str());
  throwImageException();
}

void Magick::Image::whiteThresholdChannel(const ChannelType channel_,
  const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  WhiteThresholdImageChannel(image(),channel_,threshold_.c_str(),
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::write(Blob *blob_)
{
  size_t
    length=2048; // Efficient size for small images

  void
    *data;

  modifyImage();
  GetPPException;
  data=ImagesToBlob(constImageInfo(),image(),&length,&exceptionInfo);
  ThrowPPException;
  blob_->updateNoCopy(data,length,Blob::MallocAllocator);
  throwImageException();
}

void Magick::Image::write(Blob *blob_,const std::string &magick_)
{
  size_t
    length=2048; // Efficient size for small images

  void
    *data;

  modifyImage();
  magick(magick_);
  GetPPException;
  data=ImagesToBlob(constImageInfo(),image(),&length,&exceptionInfo);
  ThrowPPException;
  blob_->updateNoCopy(data,length,Blob::MallocAllocator);
  throwImageException();
}

void Magick::Image::write(Blob *blob_,const std::string &magick_,
  const size_t depth_)
{
  size_t
    length=2048; // Efficient size for small images

  void
    *data;

  modifyImage();
  magick(magick_);
  depth(depth_);
  GetPPException;
  data=ImagesToBlob(constImageInfo(),image(),&length,&exceptionInfo);
  ThrowPPException;
  blob_->updateNoCopy(data,length,Blob::MallocAllocator);
  throwImageException();
}

void Magick::Image::write(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_,const std::string &map_,
  const StorageType type_,void *pixels_)
{
  GetPPException;
  ExportImagePixels(constImage(),x_,y_,columns_,rows_,map_.c_str(),type_,
    pixels_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::write(const std::string &imageSpec_)
{
  modifyImage();
  fileName(imageSpec_);
  WriteImage(constImageInfo(),image());
  throwImageException();
}

void Magick::Image::writePixels(const Magick::QuantumType quantum_,
  unsigned char *destination_)
{
  QuantumInfo
    *quantum_info;

  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  GetPPException;
  ExportQuantumPixels(constImage(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,destination_,&exceptionInfo);
  quantum_info=DestroyQuantumInfo(quantum_info);
  ThrowPPException;
}

void Magick::Image::zoom(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    height=rows(),
    width=columns();

  ssize_t
    x=0,
    y=0;

  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=ResizeImage(constImage(),width,height,image()->filter,image()->blur,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

Magick::Image::Image(MagickCore::Image *image_)
  : _imgRef(new ImageRef(image_))
{
}

MagickCore::Image *&Magick::Image::image(void)
{
  return(_imgRef->image());
}

const MagickCore::Image *Magick::Image::constImage(void) const
{
  return(_imgRef->image());
}

MagickCore::ImageInfo *Magick::Image::imageInfo(void)
{
  return(_imgRef->options()->imageInfo());
}

const MagickCore::ImageInfo *Magick::Image::constImageInfo(void) const
{
  return(_imgRef->options()->imageInfo());
}

Magick::Options *Magick::Image::options(void)
{
  return(_imgRef->options());
}

const Magick::Options *Magick::Image::constOptions(void) const
{
  return(_imgRef->options());
}

MagickCore::QuantizeInfo *Magick::Image::quantizeInfo(void)
{
  return(_imgRef->options()->quantizeInfo());
}

const MagickCore::QuantizeInfo *Magick::Image::constQuantizeInfo(void) const
{
  return(_imgRef->options()->quantizeInfo());
}

void Magick::Image::modifyImage(void)
{
  {
    Lock(&_imgRef->_mutexLock);
    if (_imgRef->_refCount == 1)
      {
        // De-register image and return
        _imgRef->id(-1);
        return;
      }
  }

  GetPPException;
  replaceImage(CloneImage(constImage(),0,0,MagickTrue,&exceptionInfo));
  ThrowPPException;
  return;
}

ssize_t Magick::Image::registerId(void)
{
  Lock(&_imgRef->_mutexLock);
  if (_imgRef->id() < 0)
    {
      char
        id[MaxTextExtent];

      GetPPException;
      _imgRef->id(_imgRef->id()+1);
      sprintf(id,"%.20g\n",(double) _imgRef->id());
      SetImageRegistry(ImageRegistryType,id,image(),&exceptionInfo);
      ThrowPPException;
    }
  return(_imgRef->id());
}

MagickCore::Image *Magick::Image::replaceImage(MagickCore::Image *replacement_)
{
  MagickCore::Image
    *image;
  
  if (replacement_)
    image=replacement_;
  else
    image=AcquireImage(constImageInfo());

  {
    Lock(&_imgRef->_mutexLock);

    if (_imgRef->_refCount == 1)
      {
        // We own the image, just replace it, and de-register
        _imgRef->id(-1);
        _imgRef->image(image);
      }
    else
      {
        // We don't own the image, dereference and replace with copy
        --_imgRef->_refCount;
        _imgRef=new ImageRef(image,constOptions());
      }
  }

  return(_imgRef->_image);
}

void Magick::Image::throwImageException(void) const
{
  // Throw C++ exception while resetting Image exception to default state
  throwException(const_cast<MagickCore::Image*>(constImage())->exception);
}

void Magick::Image::unregisterId(void)
{
  modifyImage();
  _imgRef->id(-1);
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

Magick::MagickCleanUp::MagickCleanUp(void)
{
  // Don't even think about invoking InitializeMagick here!
}

Magick::MagickCleanUp::~MagickCleanUp(void)
{
  MagickPlusPlusDestroyMagick();
}

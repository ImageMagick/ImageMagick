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

MagickPPExport const char *Magick::borderGeometryDefault="6x6+0+0";
MagickPPExport const char *Magick::frameGeometryDefault="25x25+6+6";
MagickPPExport const char *Magick::raiseGeometryDefault="6x6+0+0";

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
  return((left_.rows() * left_.columns()) <
    (right_.rows() * right_.columns()));
}

MagickPPExport int Magick::operator >= (const Magick::Image &left_,
  const Magick::Image &right_)
{
  return((left_ > right_) || (left_ == right_));
}

MagickPPExport int Magick::operator <= (const Magick::Image &left_,
  const Magick::Image &right_)
{
  return((left_ < right_) || ( left_ == right_));
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
  catch(const Warning &/*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch (const Error &/*error_*/)
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
    read(blob_, size_);
  }
  catch(const Warning &/*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error &/*error_*/)
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
  catch(const Warning &/*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error &/*error_*/)
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
  catch(const Warning &/*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error &/*error_*/)
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
  catch(const Warning &/*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error &/*error_*/)
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
  catch(const Warning &/*warning_*/)
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
    read(width_,height_,map_.c_str(),type_,pixels_);
  }
  catch(const Warning &/*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error &/*error_*/)
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
  catch(const Warning &/*warning_*/)
  {
    // FIXME: need a way to report warnings in constructor
  }
  catch(const Error &/*error_*/)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::~Image()
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
  if(this != &image_)
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

void Magick::Image::alpha(const bool matteFlag_)
{
  modifyImage();

  // If matte channel is requested, but image doesn't already have a
  // matte channel, then create an opaque matte channel.  Likewise, if
  // the image already has a matte channel but a matte channel is not
  // desired, then set the matte channel to opaque.
  GetPPException;
  if ((matteFlag_ && !constImage()->alpha_trait) ||
      (constImage()->alpha_trait && !matteFlag_))
    SetImageAlpha(image(),OpaqueAlpha,&exceptionInfo);
  ThrowPPException;

  image()->alpha_trait=matteFlag_ ? BlendPixelTrait : UndefinedPixelTrait;
}

bool Magick::Image::alpha(void) const
{
  if (constImage()->alpha_trait == BlendPixelTrait)
    return(true);
  else
    return(false);
}

void Magick::Image::alphaColor(const Color &alphaColor_)
{
  modifyImage();

  if (alphaColor_.isValid())
    {
      image()->matte_color=alphaColor_;
      options()->matteColor(alphaColor_);
    }
  else
    {
      // Set to default matte color
      Color tmpColor("#BDBDBD");
      image()->matte_color=tmpColor;
      options()->matteColor(tmpColor);
    }
}

Magick::Color Magick::Image::alphaColor(void) const
{
  return(Color(ClampToQuantum(constImage()->matte_color.red),
    ClampToQuantum(constImage()->matte_color.green),
    ClampToQuantum(constImage()->matte_color.blue)));
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

void Magick::Image::blackPointCompensation(const bool flag_)
{
  image()->black_point_compensation=(MagickBooleanType) flag_;
}

bool Magick::Image::blackPointCompensation(void) const
{
  return(static_cast<bool>(constImage()->black_point_compensation));
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

void Magick::Image::channelDepth(const size_t depth_)
{
  modifyImage();
  GetPPException;
  SetImageDepth(image(),depth_,&exceptionInfo);
  ThrowPPException;
}

size_t Magick::Image::channelDepth()
{
  size_t
    channel_depth;

  GetPPException;
  channel_depth=GetImageDepth(constImage(),&exceptionInfo);
  ThrowPPException;
  return(channel_depth);
}

size_t Magick::Image::channels() const
{
  return(constImage()->number_channels);
}

void Magick::Image::classType(const ClassType class_)
{
  if (classType() == PseudoClass && class_ == DirectClass)
    {
      // Use SyncImage to synchronize the DirectClass pixels with the
      // color map and then set to DirectClass type.
      modifyImage();
      GetPPException;
      SyncImage(image(),&exceptionInfo);
      ThrowPPException;
      image()->colormap=(PixelInfo *)RelinquishMagickMemory(image()->colormap);
      image()->storage_class=static_cast<MagickCore::ClassType>(DirectClass);
      return;
    }

  if (classType() == DirectClass && class_ == PseudoClass)
    {
      // Quantize to create PseudoClass color map
      modifyImage();
      quantizeColors(MaxColormapSize);
      quantize();
      image()->storage_class=static_cast<MagickCore::ClassType>(PseudoClass);
    }
}

Magick::ClassType Magick::Image::classType(void) const
{
  return static_cast<Magick::ClassType>(constImage()->storage_class);
}

void Magick::Image::clipMask(const Magick::Image &clipMask_)
{
  modifyImage();

  GetPPException;
  if (clipMask_.isValid())
    SetImageMask(image(),clipMask_.constImage(),&exceptionInfo);
  else
    SetImageMask(image(),0,&exceptionInfo);
  ThrowPPException;
}

Magick::Image Magick::Image::clipMask(void) const
{
  MagickCore::Image
    *image;

  GetPPException;
  image=GetImageMask(constImage(),&exceptionInfo);
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
  if (entries_ >MaxColormapSize)
    throwExceptionExplicit(OptionError,
      "Colormap entries must not exceed MaxColormapSize");

  modifyImage();
  GetPPException;
  AcquireImageColormap(image(),entries_,&exceptionInfo);
  ThrowPPException;
}

size_t Magick::Image::colorMapSize(void) const
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
  GetPPException;
  TransformImageColorspace(image(),colorSpace_,&exceptionInfo);
  ThrowPPException;
}

Magick::ColorspaceType Magick::Image::colorSpace(void) const
{
  return (constImage()->colorspace);
}

void Magick::Image::colorSpaceType(const ColorspaceType colorSpace_)
{
  modifyImage();
  GetPPException;
  SetImageColorspace(image(),colorSpace_,&exceptionInfo);
  ThrowPPException;
  options()->colorspaceType(colorSpace_);
}

Magick::ColorspaceType Magick::Image::colorSpaceType(void) const
{
  return(constOptions()->colorspaceType());
}

size_t Magick::Image::columns(void) const
{
  return(constImage()->columns);
}

void Magick::Image::comment(const std::string &comment_)
{
  modifyImage();
  GetPPException;
  SetImageProperty(image(),"Comment",NULL,&exceptionInfo);
  if (comment_.length() > 0)
    SetImageProperty(image(),"Comment",comment_.c_str(),&exceptionInfo);
  ThrowPPException;
}

std::string Magick::Image::comment(void) const
{
  const char
    *value;

  GetPPException;
  value=GetImageProperty(constImage(),"Comment",&exceptionInfo);
  ThrowPPException;

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
      image()->resolution.x=density_.width();
      if (density_.height() != 0)
        image()->resolution.y=density_.height();
      else
        image()->resolution.y=density_.width();
    }
  else
    {
      // Reset to default
      image()->resolution.x=0;
      image()->resolution.y=0;
    }
}

Magick::Geometry Magick::Image::density(void) const
{
  if (isValid())
    {
      ssize_t
        x_resolution=72,
        y_resolution=72;

      if (constImage()->resolution.x > 0.0)
        x_resolution=static_cast<ssize_t>(constImage()->resolution.x + 0.5);

      if (constImage()->resolution.y > 0.0)
        y_resolution=static_cast<ssize_t>(constImage()->resolution.y + 0.5);

      return(Geometry(x_resolution,y_resolution));
    }

  return(constOptions()->density());
}

void Magick::Image::depth(const size_t depth_)
{
  size_t
    depth = depth_;

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
  modifyImage();

  if (exifProfile_.data() != 0)
    {
      StringInfo
        *exif_profile;

      exif_profile=AcquireStringInfo(exifProfile_.length());
      SetStringInfoDatum(exif_profile,(unsigned char *) exifProfile_.data());
      GetPPException;
      (void) SetImageProfile(image(),"exif",exif_profile,&exceptionInfo);
      exif_profile=DestroyStringInfo(exif_profile);
      ThrowPPException;
    }
}

Magick::Blob Magick::Image::exifProfile(void) const
{
  const StringInfo 
    *exif_profile;

  exif_profile=GetImageProfile(constImage(),"exif");
  if (exif_profile == (StringInfo *) NULL)
    return(Blob());
  return(Blob(GetStringInfoDatum(exif_profile),
    GetStringInfoLength(exif_profile)));
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

MagickCore::MagickSizeType Magick::Image::fileSize(void) const
{
  return(GetBlobSize(constImage()));
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
  return constOptions()->fillRule();
}

void Magick::Image::fillPattern(const Image &fillPattern_)
{
  modifyImage();
  if (fillPattern_.isValid())
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

  GetPPException;
  text=InterpretImageProperties(imageInfo(),image(),expression.c_str(),
    &exceptionInfo);
  if (text != (char *) NULL)
    {
      result=std::string(text);
      text=DestroyString(text);
    }
  ThrowPPException;
  return(result);
}

double Magick::Image::gamma(void) const
{
  return(constImage()->gamma);
}

Magick::Geometry Magick::Image::geometry(void) const
{
  if (constImage()->geometry)
    return Geometry(constImage()->geometry);

  throwExceptionExplicit(OptionWarning,"Image does not contain a geometry");

  return(Geometry());
}

void Magick::Image::gifDisposeMethod(
  const MagickCore::DisposeType disposeMethod_)
{
  modifyImage();
  image()->dispose=disposeMethod_;
}

MagickCore::DisposeType Magick::Image::gifDisposeMethod(void) const
{
  return(constImage()->dispose);
}

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
    return(Blob());
  return(Blob(GetStringInfoDatum(color_profile),GetStringInfoLength(
    color_profile)));
}

void Magick::Image::interlaceType(const Magick::InterlaceType interlace_)
{
  modifyImage();
  image()->interlace=interlace_;
  options()->interlaceType(interlace_);
}

Magick::InterlaceType Magick::Image::interlaceType(void) const
{
  return(constImage()->interlace);
}

void Magick::Image::interpolate(const PixelInterpolateMethod interpolate_)
{
  modifyImage();
  image()->interpolate=interpolate_;
}

Magick::PixelInterpolateMethod Magick::Image::interpolate(void) const
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
      GetPPException;
      (void) SetImageProfile(image(),"iptc",iptc_profile,&exceptionInfo);
      iptc_profile=DestroyStringInfo(iptc_profile);
      ThrowPPException;
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
      _imgRef=new ImageRef;
    }
  else if (!isValid())
    {
      // Construct with single-pixel black image to make
      // image valid. This is an obvious hack.
      size(Geometry(1,1));
      read("xc:black");
    }
}

bool Magick::Image::isValid(void) const
{
  return rows() && columns();
}

void Magick::Image::label(const std::string &label_)
{
  modifyImage();
  GetPPException;
  SetImageProperty(image(),"Label",NULL,&exceptionInfo);
  if (label_.length() > 0)
    SetImageProperty(image(),"Label",label_.c_str(),&exceptionInfo);
  ThrowPPException;
}

std::string Magick::Image::label(void) const
{
  const char
    *value;

  GetPPException;
  value=GetImageProperty(constImage(),"Label",&exceptionInfo);
  ThrowPPException;

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

double Magick::Image::meanErrorPerPixel(void) const
{
  return(constImage()->error.mean_error_per_pixel);
}

void Magick::Image::modulusDepth(const size_t depth_)
{
  modifyImage();
  GetPPException;
  SetImageDepth(image(),depth_,&exceptionInfo);
  ThrowPPException;
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
    return Magick::Geometry(constImage()->montage);

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
  return(constImage()->error.normalized_mean_error);
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

void Magick::Image::quantizeDitherMethod(const DitherMethod ditherMethod_)
{
  modifyImage();
  options()->quantizeDitherMethod(ditherMethod_);
}

MagickCore::DitherMethod Magick::Image::quantizeDitherMethod(void) const
{
  return(constOptions()->quantizeDitherMethod());
}

void Magick::Image::quantizeTreeDepth(const size_t treeDepth_)
{
  modifyImage();
  options()->quantizeTreeDepth(treeDepth_);
}

size_t Magick::Image::quantizeTreeDepth() const
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
  return(static_cast<Magick::RenderingIntent>(constImage()->rendering_intent));
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

size_t Magick::Image::rows(void) const
{
  return(constImage()->rows);
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

const double* Magick::Image::strokeDashArray(void) const
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
  return(constOptions()->strokeMiterLimit());
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
  // FIXME: This is inordinately innefficient
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

void Magick::Image::textDirection(DirectionType direction_)
{
  modifyImage();
  options()->textDirection(direction_);
}

Magick::DirectionType Magick::Image::textDirection(void) const
{
  return(constOptions()->textDirection());
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

void Magick::Image::textGravity(GravityType gravity_)
{
  modifyImage();
  options()->textGravity(gravity_);
}

Magick::GravityType Magick::Image::textGravity(void) const
{
  return(constOptions()->textGravity());
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

size_t Magick::Image::totalColors(void) const
{
  size_t
    colors;

  GetPPException;
  colors=GetNumberColors(constImage(),0,&exceptionInfo);
  ThrowPPException;
  return colors;
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

Magick::ImageType Magick::Image::type(void) const
{
  ImageType
    image_type;

  GetPPException;
  image_type=constOptions()->type();
  if (image_type == UndefinedType)
    image_type=GetImageType(constImage(),&exceptionInfo);
  ThrowPPException;
  return image_type;
}

void Magick::Image::type(const Magick::ImageType type_)
{
  modifyImage();
  options()->type(type_);
  GetPPException;
  SetImageType(image(),type_,&exceptionInfo);
  ThrowPPException;
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
  const VirtualPixelMethod virtualPixelMethod_)
{
  modifyImage();
  GetPPException;
  SetImageVirtualPixelMethod(image(),virtualPixelMethod_,&exceptionInfo);
  ThrowPPException;
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
  return(constImage()->resolution.x);
}

double Magick::Image::yResolution(void) const
{
  return(constImage()->resolution.y);
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
    height=rows(),
    width=columns();

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
  const double radius_,const double sigma_ )
{
  MagickCore::Image
    *newImage;

  GetPPException;
  SetPPChannelMask(channel_);
  newImage=AdaptiveSharpenImage(constImage(),radius_,sigma_,&exceptionInfo);
  RestorePPChannelMask;
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
  newImage=AddNoiseImage(constImage(),noiseType_,1.0,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::addNoiseChannel(const ChannelType channel_,
  const NoiseType noiseType_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  SetPPChannelMask(channel_);
  newImage=AddNoiseImage(constImage(),noiseType_,1.0,&exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::affineTransform(const DrawableAffine &affine_)
{
  AffineMatrix
    _affine;

  MagickCore::Image
    *newImage;

  _affine.sx=affine_.sx();
  _affine.sy=affine_.sy();
  _affine.rx=affine_.rx();
  _affine.ry=affine_.ry();
  _affine.tx=affine_.tx();
  _affine.ty=affine_.ty();

  GetPPException;
  newImage=AffineTransformImage(constImage(),&_affine,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::alpha(const unsigned int alpha_)
{
  modifyImage();
  GetPPException;
  SetImageAlpha(image(),alpha_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::alphaChannel(AlphaChannelOption alphaOption_)
{
  modifyImage();
  GetPPException;
  SetImageAlphaChannel(image(),alphaOption_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::alphaFloodfill(const Color &target_,
  const unsigned int alpha_,const ssize_t x_,const ssize_t y_,
  const Magick::PaintMethod method_)
{
  PixelInfo
    target;

  modifyImage();
  GetPixelInfo(constImage(),&target);
  target.red=static_cast<PixelInfo>(target_).red;
  target.green=static_cast<PixelInfo>(target_).green;
  target.blue=static_cast<PixelInfo>(target_).blue;
  target.alpha=alpha_;
  GetPPException;
  SetPPChannelMask(AlphaChannel);
  FloodfillPaintImage(image(),options()->drawInfo(),&target,x_,y_,
    method_ == FloodfillMethod ? MagickFalse : MagickTrue,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
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

  GetPPException;
  AnnotateImage(image(),drawInfo,&exceptionInfo);

  // Restore original values
  drawInfo->affine=oaffine;
  drawInfo->text=0;
  drawInfo->geometry=0;

  ThrowPPException;
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

  GetPPException;
  AnnotateImage(image(),drawInfo,&exceptionInfo);

  drawInfo->gravity=NorthWestGravity;
  drawInfo->text=0;

  ThrowPPException;
}

void Magick::Image::artifact(const std::string &name_,const std::string &value_)
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
  GetPPException;
  SetImageProperty(image(),name_.c_str(),value_.c_str(),&exceptionInfo);
  ThrowPPException;
}

std::string Magick::Image::attribute(const std::string name_)
{
  const char
    *value;

  GetPPException;
  value=GetImageProperty(constImage(),name_.c_str(),&exceptionInfo);
  ThrowPPException;

  if (value)
    return(std::string(value));

  return(std::string()); // Intentionally no exception
}

void Magick::Image::autoGamma(void)
{
  modifyImage();
  GetPPException;
  (void) SyncImageSettings(imageInfo(),image(),&exceptionInfo);
  (void) AutoGammaImage(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::autoGammaChannel(const ChannelType channel_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  (void) SyncImageSettings(imageInfo(),image(),&exceptionInfo);
  (void) AutoGammaImage(image(),&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::autoLevel(void)
{
  modifyImage();
  GetPPException;
  (void) SyncImageSettings(imageInfo(),image(),&exceptionInfo);
  (void) AutoLevelImage(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::autoLevelChannel(const ChannelType channel_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  (void) SyncImageSettings(imageInfo(),image(),&exceptionInfo);
  (void) AutoLevelImage(image(),&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::autoOrient(void)
{
  MagickCore::Image
    *newImage;

  if (image()->orientation == UndefinedOrientation ||
      image()->orientation == TopLeftOrientation)
    return;

  GetPPException;
  (void) SyncImageSettings(imageInfo(),image(),&exceptionInfo);
  newImage=AutoOrientImage(constImage(),image()->orientation,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::blackThreshold(const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  BlackThresholdImage(image(),threshold_.c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::blackThresholdChannel(const ChannelType channel_,
  const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  BlackThresholdImage(image(),threshold_.c_str(),&exceptionInfo);
  RestorePPChannelMask;
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

void Magick::Image::blur(const double radius_,const double sigma_)
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
  SetPPChannelMask(channel_);
  newImage=BlurImage(constImage(),radius_,sigma_,&exceptionInfo);
  RestorePPChannelMask;
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
  newImage=BorderImage(constImage(),&borderInfo,image()->compose,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::brightnessContrast(const double brightness_,
  const double contrast_)
{
  modifyImage();
  GetPPException;
  BrightnessContrastImage(image(),brightness_,contrast_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::brightnessContrastChannel(const ChannelType channel_,
  const double brightness_,const double contrast_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  BrightnessContrastImage(image(),brightness_,contrast_,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::channel(const ChannelType channel_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SeparateImage(image(),channel_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::charcoal(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=CharcoalImage(image(),radius_,sigma_,&exceptionInfo);
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
  newImage=ChopImage(image(),&chopInfo,&exceptionInfo);
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
  GetPPException;
  (void) ColorDecisionListImage(image(),cdl_.c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::clamp(void)
{
  modifyImage();
  GetPPException;
  ClampImage(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::clampChannel(const ChannelType channel_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  ClampImage(image(),&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::clip(void)
{
  modifyImage();
  GetPPException;
  ClipImage(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::clipPath(const std::string pathname_,const bool inside_)
{
  modifyImage();
  GetPPException;
  ClipImagePath(image(),pathname_.c_str(),(MagickBooleanType) inside_,
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::clut(const Image &clutImage_,
  const PixelInterpolateMethod method)
{
  modifyImage();
  GetPPException;
  ClutImage(image(),clutImage_.constImage(),method,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::clutChannel(const ChannelType channel_,
  const Image &clutImage_,const PixelInterpolateMethod method)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  ClutImage(image(),clutImage_.constImage(),method,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::colorize(const unsigned int alpha_,const Color &penColor_)
{
  colorize(alpha_,alpha_,alpha_,penColor_);
}

void Magick::Image::colorize(const unsigned int alphaRed_,
  const unsigned int alphaGreen_,const unsigned int alphaBlue_,
  const Color &penColor_)
{
  char
    blend[MaxTextExtent];

  MagickCore::Image
    *newImage;

  PixelInfo
    pixel,
    target;

  if (!penColor_.isValid())
    throwExceptionExplicit(OptionError,"Pen color argument is invalid");

  FormatLocaleString(blend,MaxTextExtent,"%u/%u/%u",alphaRed_,alphaGreen_,
    alphaBlue_);

  GetPixelInfo(image(),&target);
  pixel=static_cast<PixelInfo>(penColor_);
  target.red=pixel.red;
  target.green=pixel.green;
  target.blue=pixel.blue;
  target.alpha=pixel.alpha;
  GetPPException;
  newImage=ColorizeImage(image(),blend,&target,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::colorMap(const size_t index_,const Color &color_)
{
  MagickCore::Image
    *imageptr;

  imageptr=image();

  if (index_ > (MaxColormapSize-1))
    throwExceptionExplicit(OptionError,
      "Colormap index must be less than MaxColormapSize");

  if (!color_.isValid())
    throwExceptionExplicit(OptionError,"Color argument is invalid");

  modifyImage();

  // Ensure that colormap size is large enough
  if (colorMapSize() < (index_+1))
    colorMapSize(index_+1);

  // Set color at index in colormap
  (imageptr->colormap)[index_]=color_;
}

Magick::Color Magick::Image::colorMap(const size_t index_) const
{
  if (!constImage()->colormap)
    {
      throwExceptionExplicit(OptionError,"Image does not contain a colormap");
      return(Color());
    }

  if (index_ > constImage()->colors-1)
    throwExceptionExplicit(OptionError,"Index out of range");

  return(Magick::Color((constImage()->colormap)[index_]));
}

void Magick::Image::colorMatrix(const size_t order_,
  const double *color_matrix_)
{
  KernelInfo
    *kernel_info;

  GetPPException;
  kernel_info=AcquireKernelInfo((const char *) NULL);
  kernel_info->width=order_;
  kernel_info->height=order_;
  kernel_info->values=(MagickRealType *) AcquireAlignedMemory(order_,
    order_*sizeof(*kernel_info->values));
  if (kernel_info->values != (MagickRealType *) NULL)
    {
      MagickCore::Image
        *newImage;

      for (ssize_t i=0; i < (ssize_t) (order_*order_); i++)
        kernel_info->values[i]=color_matrix_[i];
      newImage=ColorMatrixImage(image(),kernel_info,&exceptionInfo);
      replaceImage(newImage);
    }
  kernel_info=DestroyKernelInfo(kernel_info);
  ThrowPPException;
}

bool Magick::Image::compare(const Image &reference_)
{
  bool
    status;

  Image
    ref=reference_;

  GetPPException;
  modifyImage();
  ref.modifyImage();
  status=static_cast<bool>(IsImagesEqual(image(),ref.image(),&exceptionInfo));
  ThrowPPException;
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
  SetPPChannelMask(channel_);
  GetImageDistortion(image(),reference_.constImage(),metric_,&distortion,
    &exceptionInfo);
  RestorePPChannelMask;
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
  SetPPChannelMask(channel_);
  newImage=CompareImages(image(),reference_.constImage(),metric_,distortion,
    &exceptionInfo);
  RestorePPChannelMask;
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

  ParseMetaGeometry(static_cast<std::string>(offset_).c_str(),&x,&y,&width,
    &height);

  modifyImage();
  GetPPException;
  CompositeImage(image(),compositeImage_.constImage(),compose_,MagickFalse,
    x,y,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::composite(const Image &compositeImage_,
  const GravityType gravity_,const CompositeOperator compose_)
{
  RectangleInfo
    geometry;

  modifyImage();
  SetGeometry(compositeImage_.constImage(),&geometry);
  GravityAdjustGeometry(columns(),rows(),gravity_,&geometry);

  GetPPException;
  CompositeImage(image(),compositeImage_.constImage(),compose_,MagickFalse,
    geometry.x,geometry.y,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::composite(const Image &compositeImage_,
  const ssize_t xOffset_,const ssize_t yOffset_,
  const CompositeOperator compose_)
{
  // Image supplied as compositeImage is composited with current image and
  // results in updating current image.
  modifyImage();
  GetPPException;
  CompositeImage(image(),compositeImage_.constImage(),compose_,MagickFalse,
    xOffset_,yOffset_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::contrast(const size_t sharpen_)
{
  modifyImage();
  GetPPException;
  ContrastImage(image(),(MagickBooleanType) sharpen_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::contrastStretch(const double blackPoint_,
  const double whitePoint_)
{
  modifyImage();
  GetPPException;
  ContrastStretchImage(image(),blackPoint_,whitePoint_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::contrastStretchChannel(const ChannelType channel_,
  const double blackPoint_,const double whitePoint_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  ContrastStretchImage(image(),blackPoint_,whitePoint_,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::convolve(const size_t order_,const double *kernel_)
{
  KernelInfo
    *kernel_info;

  GetPPException;
  kernel_info=AcquireKernelInfo((const char *) NULL);
  kernel_info->width=order_;
  kernel_info->height=order_;
  kernel_info->values=(MagickRealType *) AcquireAlignedMemory(order_,
    order_*sizeof(*kernel_info->values));
  if (kernel_info->values != (MagickRealType *) NULL)
    {
      MagickCore::Image
        *newImage;

      for (ssize_t i=0; i < (ssize_t) (order_*order_); i++)
        kernel_info->values[i]=kernel_[i];
      newImage=ConvolveImage(image(),kernel_info,&exceptionInfo);
      replaceImage(newImage);
    }
  kernel_info=DestroyKernelInfo(kernel_info);
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
  GetPPException;
  CycleColormapImage(image(),amount_,&exceptionInfo);
  ThrowPPException;
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
  const std::string &key_ ) const
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
    format,
    option;

  modifyImage();
  format=magick_ + ":" + key_;
  option=value_;
  (void) SetImageOption(imageInfo(),format.c_str(),option.c_str());
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
  GetPPException;
  DisplayImages(imageInfo(),image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::distort(const DistortImageMethod method_,
  const size_t numberArguments_,const double *arguments_,const bool bestfit_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=DistortImage(constImage(), method_,numberArguments_,arguments_,
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

      DrawRender(wand);

      ClonePPDrawException(wand);
      wand=DestroyDrawingWand(wand);
      ThrowPPDrawException;
    }
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
           p != drawable_.end(); p++ )
        {
          p->operator()(wand);
          if (DrawGetExceptionType(wand) != UndefinedException)
            break;
        }

      if (DrawGetExceptionType(wand) == UndefinedException)
        DrawRender(wand);

      ClonePPDrawException(wand);
      wand=DestroyDrawingWand(wand);
      ThrowPPDrawException;
    }
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
  GetPPException;
  EqualizeImage(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::erase(void)
{
  modifyImage();
  GetPPException;
  SetImageBackgroundColor(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::extent(const Geometry &geometry_ )
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    extentInfo=geometry_;

  modifyImage();
  extentInfo.x=geometry_.xOff();
  extentInfo.y=geometry_.yOff();
  GetPPException;
  newImage=ExtentImage(image(),&extentInfo,&exceptionInfo);
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

void Magick::Image::floodFillAlpha(const ssize_t x_,const ssize_t y_,
  const unsigned int alpha_,const PaintMethod method_)
{
  PixelInfo
    pixel,
    target;

  modifyImage();
  GetPixelInfo(image(),&target);
  pixel=static_cast<PixelInfo>(pixelColor(x_,y_));
  target.red=pixel.red;
  target.green=pixel.green;
  target.blue=pixel.blue;
  target.alpha=alpha_;

  GetPPException;
  FloodfillPaintImage(image(),options()->drawInfo(),&target,
    static_cast<ssize_t>(x_), static_cast<ssize_t>(y_),
    method_  == FloodfillMethod ? MagickFalse : MagickTrue,&exceptionInfo);
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

  Quantum
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

  // Get pixel view
  Pixels pixels(*this);
  // Fill image
  p=pixels.get(x_,y_,1,1);

  if (p)
    {
      PixelInfo
        target;

      GetPixelInfo(constImage(),&target);
      target.red=GetPixelRed(constImage(),p);
      target.green=GetPixelGreen(constImage(),p);
      target.blue=GetPixelBlue(constImage(),p);
      GetPPException;
      FloodfillPaintImage(image(),options()->drawInfo(),&target,
        static_cast<ssize_t>(x_),static_cast<ssize_t>(y_),MagickFalse,
        &exceptionInfo);
      options()->fillPattern(fillPattern);
      ThrowPPException;
    }
  else
    options()->fillPattern(fillPattern);
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

  PixelInfo
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

  GetPixelInfo(constImage(),&target);
  target.red=static_cast<PixelInfo>(borderColor_).red;
  target.green=static_cast<PixelInfo>(borderColor_).green;
  target.blue=static_cast<PixelInfo>(borderColor_).blue;
  GetPPException;
  FloodfillPaintImage(image(),options()->drawInfo(),&target,
    static_cast<ssize_t>(x_),static_cast<ssize_t>(y_),MagickTrue,
    &exceptionInfo);
  options()->fillPattern(fillPattern);
  ThrowPPException;
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
  GetPPException;
  GetTypeMetrics(image(),drawInfo,&(metrics->_typeMetric),&exceptionInfo);
  drawInfo->text=0;
  ThrowPPException;
}

void Magick::Image::fontTypeMetricsMultiline(const std::string &text_,
  TypeMetric *metrics)
{
  DrawInfo
    *drawInfo;

  drawInfo=options()->drawInfo();
  drawInfo->text=const_cast<char *>(text_.c_str());
  GetPPException;
  GetMultilineTypeMetrics(image(),drawInfo,&(metrics->_typeMetric),&exceptionInfo);
  drawInfo->text=0;
  ThrowPPException;
}

void Magick::Image::frame(const Geometry &geometry_)
{
  FrameInfo
    info;
  
  MagickCore::Image
    *newImage;

  info.x=static_cast<ssize_t>(geometry_.width());
  info.y=static_cast<ssize_t>(geometry_.height());
  info.width=columns() + (static_cast<size_t>(info.x) << 1);
  info.height=rows() + (static_cast<size_t>(info.y) << 1);
  info.outer_bevel=geometry_.xOff();
  info.inner_bevel=geometry_.yOff();

  GetPPException;
  newImage=FrameImage(constImage(),&info,image()->compose,&exceptionInfo);
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
  info.width=columns() + (static_cast<size_t>(info.x) << 1);
  info.height=rows() + (static_cast<size_t>(info.y) << 1);
  info.outer_bevel=static_cast<ssize_t>(outerBevel_);
  info.inner_bevel=static_cast<ssize_t>(innerBevel_);

  GetPPException;
  newImage=FrameImage(constImage(),&info,image()->compose,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::fx(const std::string expression_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=FxImage(constImage(),expression_.c_str(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::fx(const std::string expression_,
  const Magick::ChannelType channel_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  SetPPChannelMask(channel_);
  newImage=FxImage(constImage(),expression_.c_str(),&exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::gamma(const double gamma_)
{
  modifyImage();
  GetPPException;
  GammaImage(image(),gamma_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::gamma(const double gammaRed_,const double gammaGreen_,
  const double gammaBlue_)
{
  char
    gamma[MaxTextExtent + 1];

  FormatLocaleString(gamma,MaxTextExtent,"%3.6f/%3.6f/%3.6f/",gammaRed_,
    gammaGreen_,gammaBlue_);

  modifyImage();
  GetPPException;
  GammaImage(image(),atof(gamma),&exceptionInfo);
  ThrowPPException;
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
  SetPPChannelMask(channel_);
  newImage=GaussianBlurImage(constImage(),width_,sigma_,&exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowPPException;
}

const Magick::Quantum *Magick::Image::getConstPixels(const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_) const
{
  const Quantum
    *p;

  GetPPException;
  p=(*GetVirtualPixels)(constImage(),x_, y_,columns_, rows_,&exceptionInfo);
  ThrowPPException;
  return(p);
}

const void *Magick::Image::getConstMetacontent(void) const
{
  const void
    *result;

  result=GetVirtualMetacontent(constImage());

  if(!result)
    throwExceptionExplicit(OptionError,"Unable to retrieve meta content.");

  return(result);
}

void *Magick::Image::getMetacontent(void )
{
  void
    *result;

  result=GetAuthenticMetacontent(image());

  if(!result)
    throwExceptionExplicit(OptionError,"Unable to retrieve meta content.");

  return(result);
}

Magick::Quantum *Magick::Image::getPixels(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  Quantum
    *result;

  modifyImage();
  GetPPException;
  result=(*GetAuthenticPixels)(image(),x_, y_,columns_,rows_,&exceptionInfo);
  ThrowPPException;

  return(result);
}

void  Magick::Image::haldClut(const Image &clutImage_)
{
  modifyImage();
  GetPPException;
  (void) HaldClutImage(image(),clutImage_.constImage(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::implode(const double factor_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ImplodeImage(constImage(),factor_,image()->interpolate,
    &exceptionInfo);
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

void Magick::Image::level(const double blackPoint_,const double whitePoint_,
  const double gamma_)
{
  modifyImage();
  GetPPException;
  (void) LevelImage(image(),blackPoint_,whitePoint_,gamma_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::levelChannel(const ChannelType channel_,
  const double blackPoint_,const double whitePoint_,const double gamma_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  (void) LevelImage(image(),blackPoint_,whitePoint_,gamma_,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::levelColors(const Color &blackColor_,
  const Color &whiteColor_,const bool invert_)
{
  PixelInfo
    black,
    pixel,
    white;

  modifyImage();

  GetPixelInfo(image(),&black);
  pixel=static_cast<PixelInfo>(blackColor_);
  black.red=pixel.red;
  black.green=pixel.green;
  black.blue=pixel.blue;
  black.alpha=pixel.alpha;

  GetPixelInfo(image(),&white);
  pixel=static_cast<PixelInfo>(whiteColor_);
  white.red=pixel.red;
  white.green=pixel.green;
  white.blue=pixel.blue;
  white.alpha=pixel.alpha;

  GetPPException;
  (void) LevelImageColors(image(),&black,&white,invert_ == true ?
    MagickTrue : MagickFalse,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::levelColorsChannel(const ChannelType channel_,
  const Color &blackColor_,const Color &whiteColor_,const bool invert_)
{
  PixelInfo
    black,
    pixel,
    white;

  modifyImage();

  GetPixelInfo(image(),&black);
  pixel=static_cast<PixelInfo>(blackColor_);
  black.red=pixel.red;
  black.green=pixel.green;
  black.blue=pixel.blue;
  black.alpha=pixel.alpha;

  GetPixelInfo(image(),&white);
  pixel=static_cast<PixelInfo>(whiteColor_);
  white.red=pixel.red;
  white.green=pixel.green;
  white.blue=pixel.blue;
  white.alpha=pixel.alpha;

  GetPPException;
  SetPPChannelMask(channel_);
  (void) LevelImageColors(image(),&black,&white,invert_ == true ?
    MagickTrue : MagickFalse,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::linearStretch(const double blackPoint_,
  const double whitePoint_)
{
  modifyImage();
  GetPPException;
  LinearStretchImage(image(),blackPoint_,whitePoint_,&exceptionInfo);
  ThrowPPException;
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
  newImage=LiquidRescaleImage(image(),width,height,x,y,&exceptionInfo);
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
  GetPPException;
  options()->quantizeDither(dither_);
  RemapImage(options()->quantizeInfo(),image(),mapImage_.constImage(),
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::medianFilter(const double radius_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=StatisticImage(image(),MedianStatistic,(size_t) radius_,
    (size_t) radius_,&exceptionInfo);
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
  GetPPException;
  ModulateImage(image(),modulate,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::morphology(const MorphologyMethod method_,
  const std::string kernel_,const ssize_t iterations_)
{
  KernelInfo
    *kernel;

  MagickCore::Image
    *newImage;

  kernel=AcquireKernelInfo(kernel_.c_str());
  if (kernel == (KernelInfo *)NULL)
    throwExceptionExplicit(OptionError,"Unable to parse kernel.");

  GetPPException;
  newImage=MorphologyImage(constImage(),method_,iterations_,kernel,
    &exceptionInfo);
  replaceImage(newImage);
  kernel=DestroyKernelInfo(kernel);
  ThrowPPException;
}

void Magick::Image::morphology(const MorphologyMethod method_,
  const KernelInfoType kernel_,const std::string arguments_,
  const ssize_t iterations_)
{
  const char
    *option;

  std::string
    kernel;

  option=CommandOptionToMnemonic(MagickKernelOptions,kernel_);
  if (option == (const char *)NULL)
    throwExceptionExplicit(OptionError,"Unable to determine kernel type.");

  kernel=std::string(option);
  if (!arguments_.empty())
    kernel+=":"+arguments_;

  morphology(method_,kernel,iterations_);
}

void Magick::Image::morphologyChannel(const ChannelType channel_,
  const MorphologyMethod method_,const std::string kernel_,
  const ssize_t iterations_)
{
  KernelInfo
    *kernel;

  MagickCore::Image
    *newImage;

  kernel=AcquireKernelInfo(kernel_.c_str());
  if (kernel == (KernelInfo *)NULL)
    throwExceptionExplicit(OptionError,"Unable to parse kernel.");

  GetPPException;
  SetPPChannelMask(channel_);
  newImage=MorphologyImage(constImage(),method_,iterations_,kernel,
    &exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  kernel=DestroyKernelInfo(kernel);
  ThrowPPException;
}

void Magick::Image::morphologyChannel(const ChannelType channel_,
  const MorphologyMethod method_,const KernelInfoType kernel_,
  const std::string arguments_,const ssize_t iterations_)
{
  const char
    *option;

  std::string
    kernel;

  option=CommandOptionToMnemonic(MagickKernelOptions,kernel_);
  if (option == (const char *)NULL)
    throwExceptionExplicit(OptionError,"Unable to determine kernel type.");

  kernel=std::string(option);
  if (!arguments_.empty())
    kernel+=":"+arguments_;

  morphologyChannel(channel_,method_,kernel,iterations_);
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
  GetPPException;
  NegateImage(image(),(MagickBooleanType) grayscale_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::negateChannel(const ChannelType channel_,
  const bool grayscale_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  NegateImage(image(),(MagickBooleanType) grayscale_,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::normalize(void)
{
  modifyImage();
  GetPPException;
  NormalizeImage(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::oilPaint(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=OilPaintImage(constImage(),radius_,sigma_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::opaque(const Color &opaqueColor_,const Color &penColor_)
{
  std::string
    opaqueColor,
    penColor;

  PixelInfo
    opaque,
    pen;

  if (!opaqueColor_.isValid())
    throwExceptionExplicit(OptionError,"Opaque color argument is invalid");

  if (!penColor_.isValid())
    throwExceptionExplicit(OptionError,"Pen color argument is invalid");

  modifyImage();
  opaqueColor=opaqueColor_;
  penColor=penColor_;

  GetPPException;
  (void) QueryColorCompliance(opaqueColor.c_str(),AllCompliance,&opaque,
    &exceptionInfo);
  (void) QueryColorCompliance(penColor.c_str(),AllCompliance,&pen,
    &exceptionInfo);
  OpaquePaintImage(image(),&opaque,&pen,MagickFalse,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::orderedDither(std::string thresholdMap_)
{
  modifyImage();
  GetPPException;
  (void) OrderedPosterizeImage(image(),thresholdMap_.c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::orderedDitherChannel(const ChannelType channel_,
  std::string thresholdMap_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  (void) OrderedPosterizeImage(image(),thresholdMap_.c_str(),&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::perceptible(const double epsilon_)
{
  modifyImage();
  GetPPException;
  PerceptibleImage(image(),epsilon_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::perceptibleChannel(const ChannelType channel_,
  const double epsilon_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  PerceptibleImage(image(),epsilon_,&exceptionInfo);
  RestorePPChannelMask;
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

void Magick::Image::ping(const Blob& blob_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=PingBlob(imageInfo(),blob_.data(),blob_.length(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::pixelColor(const ssize_t x_,const ssize_t y_,
  const Color &color_)
{
  PixelInfo
    packet;

  Quantum
    *pixel;

  // Test arguments to ensure they are within the image.
  if (y_ > (ssize_t) rows() || x_ > (ssize_t) columns())
    throwExceptionExplicit(OptionError,"Access outside of image boundary");

  modifyImage();

  // Set image to DirectClass
  classType(DirectClass );

  // Get pixel view
  Pixels pixels(*this);
    // Set pixel value
  pixel=pixels.get(x_, y_, 1, 1 );
  packet=color_;
  MagickCore::SetPixelInfoPixel(constImage(),&packet,pixel);
  // Tell ImageMagick that pixels have been updated
  pixels.sync();
}

Magick::Color Magick::Image::pixelColor(const ssize_t x_,
  const ssize_t y_) const
{
  const Quantum
    *pixel;

  pixel=getConstPixels(x_,y_,1,1);
  if (pixel)
    {
      PixelInfo
        packet;

      MagickCore::GetPixelInfoPixel(constImage(),pixel,&packet);
      return(Color(packet));
    }

  return(Color()); // invalid
}

void Magick::Image::polaroid(const std::string &caption_,const double angle_,
  const PixelInterpolateMethod method_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=PolaroidImage(constImage(),options()->drawInfo(),caption_.c_str(),
    angle_,method_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::posterize(const size_t levels_,const DitherMethod method_)
{
  modifyImage();
  GetPPException;
  PosterizeImage(image(),levels_,method_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::posterizeChannel(const ChannelType channel_,
  const size_t levels_,const DitherMethod method_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  PosterizeImage(image(),levels_,method_,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::process(std::string name_,const ssize_t argc,
  const char **argv)
{
  modifyImage();

  GetPPException;
  (void) InvokeDynamicImageFilter(name_.c_str(),&image(),argc,argv,
      &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::profile(const std::string name_,
  const Magick::Blob &profile_)
{
  modifyImage();
  GetPPException;
  (void) ProfileImage(image(),name_.c_str(),(unsigned char *)profile_.data(),
    profile_.length(),&exceptionInfo);
  ThrowPPException;
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

  GetPPException;
  QuantizeImage(options()->quantizeInfo(),image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::quantumOperator(const ChannelType channel_,
  const MagickEvaluateOperator operator_,double rvalue_)
{
  GetPPException;
  SetPPChannelMask(channel_);
  EvaluateImage(image(),operator_,rvalue_,&exceptionInfo);
  RestorePPChannelMask;
  ThrowPPException;
}

void Magick::Image::quantumOperator(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_,const ChannelType channel_,
  const MagickEvaluateOperator operator_,const double rvalue_)
{
  RectangleInfo
    geometry;

  MagickCore::Image
    *cropImage;

  geometry.width = columns_;
  geometry.height = rows_;
  geometry.x = x_;
  geometry.y = y_;

  GetPPException;
  cropImage=CropImage(image(),&geometry,&exceptionInfo);
  SetPPChannelMask(channel_);
  EvaluateImage(cropImage,operator_,rvalue_,&exceptionInfo);
  RestorePPChannelMask;
  (void) CompositeImage(image(),cropImage,image()->alpha_trait == 
    BlendPixelTrait ? OverCompositeOp : CopyCompositeOp,MagickFalse,
    geometry.x,geometry.y,&exceptionInfo );
  cropImage=DestroyImageList(cropImage);
  ThrowPPException;
}

void Magick::Image::raise(const Geometry &geometry_,const bool raisedFlag_)
{
  RectangleInfo
    raiseInfo=geometry_;

  GetPPException;
  modifyImage();
  RaiseImage(image(),&raiseInfo,raisedFlag_ == true ? MagickTrue : MagickFalse,
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::randomThreshold(const Geometry &thresholds_)
{
  GetPPException;
  (void) RandomThresholdImage(image(),static_cast<std::string>(
    thresholds_).c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::randomThresholdChannel(const ChannelType channel_,
  const Geometry &thresholds_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  (void) RandomThresholdImage(image(),static_cast<std::string>(
    thresholds_).c_str(),&exceptionInfo);
  RestorePPChannelMask;
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
  // Set explicit image format
  fileName(magick_ + ':');
  read(blob_);
}

void Magick::Image::read(const Blob &blob_,const Geometry &size_,
  const std::string &magick_)
{
  size(size_);
  magick(magick_);
  // Set explicit image format
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
  newImage=ConstituteImage(width_,height_,map_.c_str(),type_, pixels_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::read(const std::string &imageSpec_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  options()->fileName(imageSpec_);
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
}

void Magick::Image::readPixels(const Magick::QuantumType quantum_,
  const unsigned char *source_)
{
  QuantumInfo
    *quantum_info;

  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  GetPPException;
  ImportQuantumPixels(image(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,source_,&exceptionInfo);
  quantum_info=DestroyQuantumInfo(quantum_info);
  ThrowPPException;
}

void Magick::Image::reduceNoise(void)
{
  reduceNoise(3.0);
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
    height=rows(),
    width=columns();

  ssize_t
    x=0,
    y=0;

  // Calculate new size.  This code should be supported using binary arguments
  // in the ImageMagick library.
  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=ResampleImage(constImage(),width,height,image()->filter,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::resize(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  size_t
    height=rows(),
    width=columns();

  ssize_t
    x=0,
    y=0;

  // Calculate new size.  This code should be supported using binary arguments
  // in the ImageMagick library.
  ParseMetaGeometry(static_cast<std::string>(geometry_).c_str(),&x,&y,&width,
    &height);

  GetPPException;
  newImage=ResizeImage(constImage(),width,height,image()->filter,
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
    xOff = 0 - xOff;
  if (roll_.yNegative())
    yOff = 0 - yOff;

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

void Magick::Image::rotationalBlur(const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=RotationalBlurImage(constImage(),angle_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::rotationalBlurChannel(const ChannelType channel_,
  const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  SetPPChannelMask(channel_);
  newImage=RotationalBlurImage(constImage(),angle_,&exceptionInfo);
  RestorePPChannelMask;
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
  GetPPException;
  SegmentImage(image(),options()->quantizeColorSpace(),
    (MagickBooleanType) options()->verbose(),clusterThreshold_,
    smoothingThreshold_,&exceptionInfo);
  SyncImage(image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::selectiveBlur(const double radius_,const double sigma_,
  const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SelectiveBlurImage(constImage(),radius_,sigma_,threshold_,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::selectiveBlurChannel(const ChannelType channel_,
  const double radius_,const double sigma_,const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  SetPPChannelMask(channel_);
  newImage=SelectiveBlurImage(constImage(),radius_,sigma_,threshold_,
    &exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowPPException;
}

Magick::Image Magick::Image::separate(const ChannelType channel_)
{
  MagickCore::Image
    *image;

  GetPPException;
  image=SeparateImage(constImage(),channel_,&exceptionInfo);
  ThrowPPException;
  if (image == (MagickCore::Image *) NULL)
    return(Magick::Image());
  else
    return(Magick::Image(image));
}

void Magick::Image::sepiaTone(const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SepiaToneImage(constImage(),threshold_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

Magick::Quantum *Magick::Image::setPixels(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  Quantum
    *result;

  modifyImage();
  GetPPException;
  result=(*QueueAuthenticPixels)(image(),x_,y_,columns_,rows_,&exceptionInfo);
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
  newImage=ShadowImage(constImage(),percent_opacity_, sigma_,x_, y_,
    &exceptionInfo);
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
  SetPPChannelMask(channel_);
  newImage=SharpenImage(constImage(),radius_,sigma_,&exceptionInfo);
  RestorePPChannelMask;
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
  GetPPException;
  (void) SigmoidalContrastImage(image(),(MagickBooleanType) sharpen_,contrast,
    midpoint,&exceptionInfo);
  ThrowPPException;
}

std::string Magick::Image::signature(const bool force_) const
{
  const char
    *property;

  Lock(&_imgRef->_mutexLock);

  // Re-calculate image signature if necessary
  GetPPException;
  if (force_ || !GetImageProperty(constImage(),"Signature",&exceptionInfo) ||
    constImage()->taint)
    SignatureImage(const_cast<MagickCore::Image *>(constImage()),
      &exceptionInfo);

  property=GetImageProperty(constImage(),"Signature",&exceptionInfo);
  ThrowPPException;

  return(std::string(property));
}

void Magick::Image::sketch(const double radius_,const double sigma_,
  const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SketchImage(constImage(),radius_,sigma_,angle_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::solarize(const double factor_)
{
  modifyImage();
  GetPPException;
  SolarizeImage(image(),factor_,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::sparseColor(const ChannelType channel_,
  const SparseColorMethod method_,const size_t numberArguments_,
  const double *arguments_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  SetPPChannelMask(channel_);
  newImage=SparseColorImage(constImage(),method_,numberArguments_,arguments_,
    &exceptionInfo);
  RestorePPChannelMask;
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
  newImage=SpreadImage(constImage(),amount_,image()->interpolate,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::statistics(ImageStatistics *statistics)
{
  double
    maximum,
    minimum;

  GetPPException;

  SetPPChannelMask(RedChannel);
  (void) GetImageRange(constImage(),&minimum,&maximum,&exceptionInfo);
  statistics->red.minimum=minimum;
  statistics->red.maximum=maximum;
  (void) GetImageMean(constImage(),&statistics->red.mean,
    &statistics->red.standard_deviation,&exceptionInfo);
  (void) GetImageKurtosis(constImage(),&statistics->red.kurtosis,
    &statistics->red.skewness,&exceptionInfo);

  (void) SetImageChannelMask(image(),GreenChannel);
  (void) GetImageRange(constImage(),&minimum,&maximum,&exceptionInfo);
  statistics->green.minimum=minimum;
  statistics->green.maximum=maximum;
  (void) GetImageMean(constImage(),&statistics->green.mean,
    &statistics->green.standard_deviation,&exceptionInfo);
  (void) GetImageKurtosis(constImage(),&statistics->green.kurtosis,
    &statistics->green.skewness,&exceptionInfo);

  (void) SetImageChannelMask(image(),GreenChannel);
  (void) GetImageRange(constImage(),&minimum,&maximum,&exceptionInfo);
  statistics->blue.minimum=minimum;
  statistics->blue.maximum=maximum;
  (void) GetImageMean(constImage(),&statistics->blue.mean,
    &statistics->blue.standard_deviation,&exceptionInfo);
  (void) GetImageKurtosis(constImage(),&statistics->blue.kurtosis,
    &statistics->blue.skewness,&exceptionInfo);

  (void) SetImageChannelMask(image(),AlphaChannel);
  (void) GetImageRange(constImage(),&minimum,&maximum,&exceptionInfo);
  statistics->alpha.minimum=minimum;
  statistics->alpha.maximum=maximum;
  (void) GetImageMean(constImage(),&statistics->alpha.mean,
    &statistics->alpha.standard_deviation,&exceptionInfo);
  (void) GetImageKurtosis(constImage(),&statistics->alpha.kurtosis,
    &statistics->alpha.skewness,&exceptionInfo);
  RestorePPChannelMask;
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
  GetPPException;
  StripImage(image(),&exceptionInfo);
  ThrowPPException;
}

Magick::Image Magick::Image::subImageSearch(const Image &reference_,
  const MetricType metric_,Geometry *offset_,double *similarityMetric_,
  const double similarityThreshold)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    offset;

  GetPPException;
  newImage=SimilarityImage(image(),reference_.constImage(),metric_,
    similarityThreshold,&offset,similarityMetric_,&exceptionInfo);
  ThrowPPException;
  if (offset_ != (Geometry *) NULL)
    *offset_=offset;
  if (newImage == (MagickCore::Image *) NULL)
    return(Magick::Image());
  else
    return(Magick::Image(newImage));
}

void Magick::Image::swirl(const double degrees_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SwirlImage(constImage(),degrees_,image()->interpolate,
    &exceptionInfo);
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
  GetPPException;
  TextureImage(image(),texture_.constImage(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::threshold(const double threshold_)
{
  modifyImage();
  GetPPException;
  BilevelImage(image(),threshold_,&exceptionInfo);
  ThrowPPException;
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

void Magick::Image::tint(const std::string opacity_)
{
  MagickCore::Image
    *newImage;

  PixelInfo
    color;

  GetPPException;
  color=static_cast<PixelInfo>(constOptions()->fillColor());
  newImage=TintImage(constImage(),opacity_.c_str(),&color,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::transform(const Geometry &imageGeometry_)
{
  modifyImage();
  GetPPException;
  TransformImage(&(image()),0,std::string(imageGeometry_).c_str(),
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::transform(const Geometry &imageGeometry_,
  const Geometry &cropGeometry_)
{
  modifyImage();
  GetPPException;
  TransformImage(&(image()),std::string(cropGeometry_).c_str(),std::string(
    imageGeometry_).c_str(), &exceptionInfo);
  ThrowPPException;
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
  PixelInfo
    target;

  std::string
    color;

  if (!color_.isValid())
    throwExceptionExplicit(OptionError,"Color argument is invalid");

  color=color_;
  GetPPException;
  (void) QueryColorCompliance(color.c_str(),AllCompliance,&target,
    &exceptionInfo);
  modifyImage();
  TransparentPaintImage(image(),&target,TransparentAlpha,MagickFalse,
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::transparentChroma(const Color &colorLow_,
  const Color &colorHigh_)
{
  std::string
    colorHigh,
    colorLow;

  PixelInfo
    targetHigh,
    targetLow;

  if (!colorLow_.isValid() || !colorHigh_.isValid())
    throwExceptionExplicit(OptionError,"Color argument is invalid");

  colorLow=colorLow_;
  colorHigh=colorHigh_;

  GetPPException;
  (void) QueryColorCompliance(colorLow.c_str(),AllCompliance,&targetLow,
    &exceptionInfo);
  (void) QueryColorCompliance(colorHigh.c_str(),AllCompliance,&targetHigh,
    &exceptionInfo);
  modifyImage();
  TransparentPaintImageChroma(image(),&targetLow,&targetHigh,TransparentAlpha,
    MagickFalse,&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::transpose(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=TransposeImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::transverse(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=TransverseImage(constImage(),&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
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

Magick::Image Magick::Image::uniqueColors(void)
{
  MagickCore::Image
    *image;

  GetPPException;
  image=UniqueImageColors(constImage(),&exceptionInfo);
  ThrowPPException;
  if (image == (MagickCore::Image *) NULL)
    return(Magick::Image());
  else
    return(Magick::Image(image));
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
  SetPPChannelMask(channel_);
  newImage=UnsharpMaskImage(constImage(),radius_,sigma_,amount_,threshold_,
    &exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::vignette(const double radius_,const double sigma_,
  const ssize_t x_,const ssize_t y_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=VignetteImage(constImage(),radius_,sigma_,x_,y_,&exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::wave(const double amplitude_,const double wavelength_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=WaveImage(constImage(),amplitude_,wavelength_,image()->interpolate,
    &exceptionInfo);
  replaceImage(newImage);
  ThrowPPException;
}

void Magick::Image::whiteThreshold(const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  WhiteThresholdImage(image(),threshold_.c_str(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::whiteThresholdChannel(const ChannelType channel_,
  const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  SetPPChannelMask(channel_);
  WhiteThresholdImage(image(),threshold_.c_str(),&exceptionInfo);
  RestorePPChannelMask;
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
}

void Magick::Image::write(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_,const std::string &map_,
  const StorageType type_,void *pixels_)
{
  GetPPException;
  ExportImagePixels(image(),x_,y_,columns_,rows_,map_.c_str(),type_,pixels_,
    &exceptionInfo);
  ThrowPPException;
}

void Magick::Image::write(const std::string &imageSpec_)
{
  modifyImage();
  fileName(imageSpec_);
  GetPPException;
  WriteImage(constImageInfo(),image(),&exceptionInfo);
  ThrowPPException;
}

void Magick::Image::writePixels(const Magick::QuantumType quantum_,
  unsigned char *destination_)
{
  QuantumInfo
    *quantum_info;

  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  GetPPException;
  ExportQuantumPixels(image(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,destination_, &exceptionInfo);
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
  newImage=ResizeImage(constImage(),width,height,image()->filter,&exceptionInfo);
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
  replaceImage(CloneImage(image(),0,0,MagickTrue,&exceptionInfo));
  ThrowPPException;
}

ssize_t Magick::Image::registerId(void)
{
  Lock(&_imgRef->_mutexLock);
  if ( _imgRef->id() < 0)
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
    image = replacement_;
  else
    {
      GetPPException;
      image=AcquireImage(constImageInfo(),&exceptionInfo);
      ThrowPPException;
    }

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

void Magick::Image::unregisterId(void)
{
  modifyImage();
  _imgRef->id(-1);
}

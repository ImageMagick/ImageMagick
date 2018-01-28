// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2013-2017
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
#define ThrowImageException ThrowPPException(quiet())

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
    quiet(true);
    read(blob_);
    quiet(false);
  }
  catch (const Error&)
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
    quiet(true);
    read(blob_, size_);
    quiet(false);
  }
  catch(const Error&)
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
    quiet(true);
    read(blob_,size_,depth_);
    quiet(false);
  }
  catch(const Error&)
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
    quiet(true);
    read(blob_,size_,depth_,magick_);
    quiet(false);
  }
  catch(const Error&)
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
    quiet(true);
    read(blob_,size_,magick_);
    quiet(false);
  }
  catch(const Error&)
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
    quiet(true);
    // Set image size
    size(size_);

    // Initialize, Allocate and Read images
    read(imageSpec);
    quiet(false);
  }
  catch(const Error&)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::Image(const Image &image_)
  : _imgRef(image_._imgRef)
{
  _imgRef->increase();
}

Magick::Image::Image(const Image &image_,const Geometry &geometry_)
  : _imgRef(new ImageRef)
{
  const RectangleInfo
    geometry=geometry_;

  OffsetInfo
    offset;

  MagickCore::Image
    *image;

  GetPPException;
  image=CloneImage(image_.constImage(),geometry_.width(),geometry_.height(),
    MagickTrue,exceptionInfo);
  replaceImage(image);
  _imgRef->options(new Options(*image_.constOptions()));
  offset.x=0;
  offset.y=0;
  (void) CopyImagePixels(image,image_.constImage(),&geometry,&offset,
    exceptionInfo);
  ThrowImageException;
}

Magick::Image::Image(const size_t width_,const size_t height_,
  const std::string &map_,const StorageType type_,const void *pixels_)
  : _imgRef(new ImageRef)
{
  try
  {
    quiet(true);
    read(width_,height_,map_.c_str(),type_,pixels_);
    quiet(false);
  }
  catch(const Error&)
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
    quiet(true);
    read(imageSpec_);
    quiet(false);
  }
  catch(const Error&)
  {
    // Release resources
    delete _imgRef;
    throw;
  }
}

Magick::Image::~Image()
{
  try
  {
    if (_imgRef->decrease() == 0)
      delete _imgRef;
  }
  catch(Magick::Exception)
  {
  }

  _imgRef=(Magick::ImageRef *) NULL;
}

Magick::Image& Magick::Image::operator=(const Magick::Image &image_)
{
  if (this != &image_)
    {
      image_._imgRef->increase();
      if (_imgRef->decrease() == 0)
        delete _imgRef;

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

void Magick::Image::alpha(const bool alphaFlag_)
{
  modifyImage();

  // If matte channel is requested, but image doesn't already have a
  // matte channel, then create an opaque matte channel.  Likewise, if
  // the image already has a matte channel but a matte channel is not
  // desired, then set the matte channel to opaque.
  GetPPException;
  if ((alphaFlag_ && !constImage()->alpha_trait) ||
      (constImage()->alpha_trait && !alphaFlag_))
    SetImageAlpha(image(),OpaqueAlpha,exceptionInfo);
  ThrowImageException;

  image()->alpha_trait=alphaFlag_ ? BlendPixelTrait : UndefinedPixelTrait;
}

bool Magick::Image::alpha(void) const
{
  if (constImage()->alpha_trait == BlendPixelTrait)
    return(true);
  else
    return(false);
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
      Color tmpColor("#BDBDBD");
      image()->matte_color=tmpColor;
      options()->matteColor(tmpColor);
    }
}

Magick::Color Magick::Image::matteColor(void) const
{
  return(Color(constImage()->matte_color));
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
  bbox=GetImageBoundingBox(constImage(),exceptionInfo);
  ThrowImageException;
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

void Magick::Image::channelDepth(const ChannelType channel_,
  const size_t depth_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  SetImageDepth(image(),depth_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

size_t Magick::Image::channelDepth(const ChannelType channel_)
{
  size_t
    channel_depth;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  channel_depth=GetImageDepth(constImage(),exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
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
      SyncImage(image(),exceptionInfo);
      ThrowImageException;
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
    throwExceptionExplicit(MagickCore::OptionError,
      "Colormap entries must not exceed MaxColormapSize");

  modifyImage();
  GetPPException;
  (void) AcquireImageColormap(image(),entries_,exceptionInfo);
  ThrowImageException;
}

size_t Magick::Image::colorMapSize(void) const
{
  if (!constImage()->colormap)
    throwExceptionExplicit(MagickCore::OptionError,
      "Image does not contain a colormap");

  return(constImage()->colors);
}

void Magick::Image::colorSpace(const ColorspaceType colorSpace_)
{
  if (image()->colorspace == colorSpace_)
    return;

  modifyImage();
  GetPPException;
  TransformImageColorspace(image(),colorSpace_,exceptionInfo);
  ThrowImageException;
}

Magick::ColorspaceType Magick::Image::colorSpace(void) const
{
  return (constImage()->colorspace);
}

void Magick::Image::colorSpaceType(const ColorspaceType colorSpace_)
{
  modifyImage();
  GetPPException;
  SetImageColorspace(image(),colorSpace_,exceptionInfo);
  ThrowImageException;
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
  SetImageProperty(image(),"Comment",NULL,exceptionInfo);
  if (comment_.length() > 0)
    SetImageProperty(image(),"Comment",comment_.c_str(),exceptionInfo);
  ThrowImageException;
}

std::string Magick::Image::comment(void) const
{
  const char
    *value;

  GetPPException;
  value=GetImageProperty(constImage(),"Comment",exceptionInfo);
  ThrowImageException;

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

void Magick::Image::density(const Point &density_)
{
  modifyImage();
  options()->density(density_);
  if (density_.isValid())
    {
      image()->resolution.x=density_.x();
      if (density_.y() != 0.0)
        image()->resolution.y=density_.y();
      else
        image()->resolution.y=density_.x();
    }
  else
    {
      // Reset to default
      image()->resolution.x=0.0;
      image()->resolution.y=0.0;
    }
}

Magick::Point Magick::Image::density(void) const
{
  if (isValid())
    {
      ssize_t
        x_resolution=72,
        y_resolution=72;

      if (constImage()->resolution.x > 0.0)
        x_resolution=constImage()->resolution.x;

      if (constImage()->resolution.y > 0.0)
        y_resolution=constImage()->resolution.y;

      return(Point(x_resolution,y_resolution));
    }

  return(constOptions()->density());
}

void Magick::Image::depth(const size_t depth_)
{
  modifyImage();
  image()->depth=depth_;
  options()->depth(depth_);
}

size_t Magick::Image::depth(void) const
{
  return(constImage()->depth);
}

std::string Magick::Image::directory(void) const
{
  if (constImage()->directory)
    return(std::string(constImage()->directory));

  if (!quiet())
    throwExceptionExplicit(MagickCore::CorruptImageWarning,
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
      (void) SetImageProfile(image(),"exif",exif_profile,exceptionInfo);
      exif_profile=DestroyStringInfo(exif_profile);
      ThrowImageException;
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
  ssize_t
    max_length;

  modifyImage();

  max_length=sizeof(image()->filename)-1;
  fileName_.copy(image()->filename,max_length);
  if ((ssize_t) fileName_.length() > max_length)
    image()->filename[max_length]=0;
  else
    image()->filename[fileName_.length()]=0;

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
      image=CloneImage(tmpTexture,0,0,MagickTrue,exceptionInfo);
      texture.replaceImage(image);
      ThrowImageException;
    }
  return(texture);
}

void Magick::Image::filterType(const Magick::FilterType filterType_)
{
  modifyImage();
  image()->filter=filterType_;
}

Magick::FilterType Magick::Image::filterType(void) const
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

void Magick::Image::fontFamily(const std::string &family_)
{
  modifyImage();
  options()->fontFamily(family_);
}

std::string Magick::Image::fontFamily(void) const
{
  return(constOptions()->fontFamily());
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

void Magick::Image::fontStyle(const StyleType pointSize_)
{
  modifyImage();
  options()->fontStyle(pointSize_);
}

Magick::StyleType Magick::Image::fontStyle(void) const
{
  return(constOptions()->fontStyle());
}

void Magick::Image::fontWeight(const size_t weight_)
{
  modifyImage();
  options()->fontWeight(weight_);
}

size_t Magick::Image::fontWeight(void) const
{
  return(constOptions()->fontWeight());
}

std::string Magick::Image::format(void) const
{
  const MagickInfo 
   *magick_info;

  GetPPException;
  magick_info=GetMagickInfo(constImage()->magick,exceptionInfo);
  ThrowImageException;

  if ((magick_info != 0) && (*magick_info->description != '\0'))
    return(std::string(magick_info->description));

  if (!quiet())
    throwExceptionExplicit(MagickCore::CorruptImageWarning,
      "Unrecognized image magick type");

  return(std::string());
}

std::string Magick::Image::formatExpression(const std::string expression)
{
  char
    *text;

  std::string
    text_string;

  GetPPException;
  modifyImage();
  text=InterpretImageProperties(imageInfo(),image(),expression.c_str(),
    exceptionInfo);
  if (text != (char *) NULL)
    {
      text_string=std::string(text);
      text=DestroyString(text);
    }
  ThrowImageException;
  return(text_string);
}

double Magick::Image::gamma(void) const
{
  return(constImage()->gamma);
}

Magick::Geometry Magick::Image::geometry(void) const
{
  if (constImage()->geometry)
    return Geometry(constImage()->geometry);

  if (!quiet())
    throwExceptionExplicit(MagickCore::OptionWarning,
      "Image does not contain a geometry");

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

bool Magick::Image::hasChannel(const PixelChannel channel) const
{
  if (GetPixelChannelTraits(constImage(),channel) == UndefinedPixelTrait)
    return(false);

  if (channel == GreenPixelChannel || channel == BluePixelChannel)
    return (GetPixelChannelOffset(constImage(),channel) == (ssize_t)channel);

  return(true);
}

void Magick::Image::highlightColor(const Color color_)
{
  std::string
    value;

  value=color_;
  artifact("compare:highlight-color",value);
}

void Magick::Image::iccColorProfile(const Magick::Blob &colorProfile_)
{
  profile("icc",colorProfile_);
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
      (void) SetImageProfile(image(),"iptc",iptc_profile,exceptionInfo);
      iptc_profile=DestroyStringInfo(iptc_profile);
      ThrowImageException;
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

bool Magick::Image::isOpaque(void) const
{
  MagickBooleanType
    result;

  GetPPException;
  result=IsImageOpaque(constImage(),exceptionInfo);
  ThrowImageException;
  return(result != MagickFalse ? true : false);
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
  (void) SetImageProperty(image(),"Label",NULL,exceptionInfo);
  if (label_.length() > 0)
    (void) SetImageProperty(image(),"Label",label_.c_str(),exceptionInfo);
  ThrowImageException;
}

std::string Magick::Image::label(void) const
{
  const char
    *value;

  GetPPException;
  value=GetImageProperty(constImage(),"Label",exceptionInfo);
  ThrowImageException;

  if (value)
    return(std::string(value));

  return(std::string());
}

void Magick::Image::lowlightColor(const Color color_)
{
  std::string
    value;

  value=color_;
  artifact("compare:lowlight-color",value);
}

void Magick::Image::magick(const std::string &magick_)
{
  size_t
    length;

  modifyImage();

  length=sizeof(image()->magick)-1;
  if (magick_.length() < length)
    length=magick_.length();

  if (!magick_.empty())
    magick_.copy(image()->magick,length);
  image()->magick[length]=0;

  options()->magick(magick_);
}

std::string Magick::Image::magick(void) const
{
  if (*(constImage()->magick) != '\0')
    return(std::string(constImage()->magick));

  return(constOptions()->magick());
}

void Magick::Image::masklightColor(const Color color_)
{
  std::string
    value;

  value=color_;
  artifact("compare:masklight-color",value);
}

double Magick::Image::meanErrorPerPixel(void) const
{
  return(constImage()->error.mean_error_per_pixel);
}

void Magick::Image::modulusDepth(const size_t depth_)
{
  modifyImage();
  GetPPException;
  SetImageDepth(image(),depth_,exceptionInfo);
  ThrowImageException;
  options()->depth(depth_);
}

size_t Magick::Image::modulusDepth(void) const
{
  size_t 
    depth;

  GetPPException;
  depth=GetImageDepth(constImage(),exceptionInfo);
  ThrowImageException;
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

  if (!quiet())
    throwExceptionExplicit(MagickCore::CorruptImageWarning,
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
    constImage()->page.x,constImage()->page.y));
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

void Magick::Image::quiet(const bool quiet_)
{
  modifyImage();
  options()->quiet(quiet_);
}

bool Magick::Image::quiet(void) const
{
  return(constOptions()->quiet());
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
  return(static_cast<Magick::ResolutionType>(constImage()->units));
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
  std::string
    value;

  modifyImage();
  options()->strokeColor(strokeColor_);
  value=strokeColor_;
  artifact("stroke",value);
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
      image=CloneImage(tmpTexture,0,0,MagickTrue,exceptionInfo);
      texture.replaceImage(image);
      ThrowImageException;
    }
  return(texture);
}

void Magick::Image::strokeWidth(const double strokeWidth_)
{
  char
    value[MagickPathExtent];

  modifyImage();
  options()->strokeWidth(strokeWidth_);
  FormatLocaleString(value,MagickPathExtent,"%.20g",strokeWidth_);
  (void) SetImageArtifact(image(),"strokewidth",value);
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

void Magick::Image::textAntiAlias(const bool flag_)
{
  modifyImage();
  options()->textAntiAlias(flag_);
}

bool Magick::Image::textAntiAlias(void) const
{
  return(constOptions()->textAntiAlias());
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

void Magick::Image::textUnderColor(const Color &underColor_)
{
  modifyImage();
  options()->textUnderColor(underColor_);
}

Magick::Color Magick::Image::textUnderColor(void) const
{
  return(constOptions()->textUnderColor());
}

size_t Magick::Image::totalColors(void) const
{
  size_t
    colors;

  GetPPException;
  colors=GetNumberColors(constImage(),(FILE *) NULL,exceptionInfo);
  ThrowImageException;
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
  if (constOptions()->type() != UndefinedType)
    return(constOptions()->type());
  return(GetImageType(constImage()));
}

void Magick::Image::type(const Magick::ImageType type_)
{
  modifyImage();
  options()->type(type_);
  GetPPException;
  SetImageType(image(),type_,exceptionInfo);
  ThrowImageException;
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

void Magick::Image::virtualPixelMethod(
  const VirtualPixelMethod virtualPixelMethod_)
{
  modifyImage();
  GetPPException;
  SetImageVirtualPixelMethod(image(),virtualPixelMethod_,exceptionInfo);
  ThrowImageException;
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
  newImage=AdaptiveBlurImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
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
  newImage=AdaptiveResizeImage(constImage(),width,height,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::adaptiveSharpen(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AdaptiveSharpenImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::adaptiveSharpenChannel(const ChannelType channel_,
  const double radius_,const double sigma_ )
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=AdaptiveSharpenImage(constImage(),radius_,sigma_,exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::adaptiveThreshold(const size_t width_,const size_t height_,
   const double bias_)
{

  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AdaptiveThresholdImage(constImage(),width_,height_,bias_,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::addNoise(const NoiseType noiseType_,const double attenuate_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=AddNoiseImage(constImage(),noiseType_,attenuate_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::addNoiseChannel(const ChannelType channel_,
  const NoiseType noiseType_,const double attenuate_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=AddNoiseImage(constImage(),noiseType_,attenuate_,exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
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
  newImage=AffineTransformImage(constImage(),&_affine,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::alpha(const unsigned int alpha_)
{
  modifyImage();
  GetPPException;
  SetImageAlpha(image(),alpha_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::alphaChannel(AlphaChannelOption alphaOption_)
{
  modifyImage();
  GetPPException;
  SetImageAlphaChannel(image(),alphaOption_,exceptionInfo);
  ThrowImageException;
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
    boundingArea[MagickPathExtent];

  DrawInfo
    *drawInfo;

  modifyImage();

  drawInfo=options()->drawInfo();
  drawInfo->text=DestroyString(drawInfo->text);
  drawInfo->text=const_cast<char *>(text_.c_str());
  drawInfo->geometry=DestroyString(drawInfo->geometry);

  if (boundingArea_.isValid())
    {
      if (boundingArea_.width() == 0 || boundingArea_.height() == 0)
        {
          FormatLocaleString(boundingArea,MagickPathExtent,"%+.20g%+.20g",
            (double) boundingArea_.xOff(),(double) boundingArea_.yOff());
        }
      else
        {
          (void) CopyMagickString(boundingArea,
            std::string(boundingArea_).c_str(), MagickPathExtent);
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
  AnnotateImage(image(),drawInfo,exceptionInfo);

  // Restore original values
  drawInfo->affine=oaffine;
  drawInfo->text=(char *) NULL;
  drawInfo->geometry=(char *) NULL;

  ThrowImageException;
}

void Magick::Image::annotate(const std::string &text_,
  const GravityType gravity_)
{
  DrawInfo
    *drawInfo;

  modifyImage();

  drawInfo=options()->drawInfo();
  drawInfo->text=DestroyString(drawInfo->text);
  drawInfo->text=const_cast<char *>(text_.c_str());
  drawInfo->gravity=gravity_;

  GetPPException;
  AnnotateImage(image(),drawInfo,exceptionInfo);

  drawInfo->gravity=NorthWestGravity;
  drawInfo->text=(char *) NULL;

  ThrowImageException;
}

void Magick::Image::artifact(const std::string &name_,const std::string &value_)
{
  modifyImage();
  (void) SetImageArtifact(image(),name_.c_str(),value_.c_str());
}

std::string Magick::Image::artifact(const std::string &name_) const
{
  const char
    *value;

  value=GetImageArtifact(constImage(),name_.c_str());
  if (value)
    return(std::string(value));
  return(std::string());
}

void Magick::Image::attribute(const std::string name_,const char *value_)
{
  modifyImage();
  GetPPException;
  SetImageProperty(image(),name_.c_str(),value_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::attribute(const std::string name_,const std::string value_)
{
  modifyImage();
  GetPPException;
  SetImageProperty(image(),name_.c_str(),value_.c_str(),exceptionInfo);
  ThrowImageException;
}

std::string Magick::Image::attribute(const std::string name_) const
{
  const char
    *value;

  GetPPException;
  value=GetImageProperty(constImage(),name_.c_str(),exceptionInfo);
  ThrowImageException;

  if (value)
    return(std::string(value));

  return(std::string()); // Intentionally no exception
}

void Magick::Image::autoGamma(void)
{
  modifyImage();
  GetPPException;
  (void) SyncImageSettings(imageInfo(),image(),exceptionInfo);
  (void) AutoGammaImage(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::autoGammaChannel(const ChannelType channel_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  (void) SyncImageSettings(imageInfo(),image(),exceptionInfo);
  (void) AutoGammaImage(image(),exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::autoLevel(void)
{
  modifyImage();
  GetPPException;
  (void) AutoLevelImage(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::autoLevelChannel(const ChannelType channel_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  (void) AutoLevelImage(image(),exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::autoOrient(void)
{
  MagickCore::Image
    *newImage;

  if (image()->orientation == UndefinedOrientation ||
      image()->orientation == TopLeftOrientation)
    return;

  GetPPException;
  newImage=AutoOrientImage(constImage(),image()->orientation,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::autoThreshold(const AutoThresholdMethod method_)
{
  modifyImage();
  GetPPException;
  AutoThresholdImage(image(),method_, exceptionInfo);
  ThrowImageException;
}

void Magick::Image::blackThreshold(const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  BlackThresholdImage(image(),threshold_.c_str(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::blackThresholdChannel(const ChannelType channel_,
  const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  BlackThresholdImage(image(),threshold_.c_str(),exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::blueShift(const double factor_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=BlueShiftImage(constImage(),factor_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::blur(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=BlurImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::blurChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=BlurImage(constImage(),radius_,sigma_,exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::border(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    borderInfo=geometry_;

  GetPPException;
  newImage=BorderImage(constImage(),&borderInfo,image()->compose,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::brightnessContrast(const double brightness_,
  const double contrast_)
{
  modifyImage();
  GetPPException;
  BrightnessContrastImage(image(),brightness_,contrast_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::brightnessContrastChannel(const ChannelType channel_,
  const double brightness_,const double contrast_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  BrightnessContrastImage(image(),brightness_,contrast_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::cannyEdge(const double radius_,const double sigma_,
  const double lowerPercent_,const double upperPercent_)
{
  MagickCore::Image
    *newImage;

  modifyImage();
  GetPPException;
  newImage=CannyEdgeImage(constImage(),radius_,sigma_,lowerPercent_,
    upperPercent_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::cdl(const std::string &cdl_)
{
  modifyImage();
  GetPPException;
  (void) ColorDecisionListImage(image(),cdl_.c_str(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::channel(const ChannelType channel_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SeparateImage(image(),channel_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::charcoal(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=CharcoalImage(image(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::charcoalChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=CharcoalImage(image(),radius_,sigma_,exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::chop(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    chopInfo=geometry_;

  GetPPException;
  newImage=ChopImage(image(),&chopInfo,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::chromaBluePrimary(const double x_,const double y_,
  const double z_)
{
  modifyImage();
  image()->chromaticity.blue_primary.x=x_;
  image()->chromaticity.blue_primary.y=y_;
  image()->chromaticity.blue_primary.z=z_;
}

void Magick::Image::chromaBluePrimary(double *x_,double *y_,double *z_) const
{
  *x_=constImage()->chromaticity.blue_primary.x;
  *y_=constImage()->chromaticity.blue_primary.y;
  *z_=constImage()->chromaticity.blue_primary.z;
}

void Magick::Image::chromaGreenPrimary(const double x_,const double y_,
  const double z_)
{
  modifyImage();
  image()->chromaticity.green_primary.x=x_;
  image()->chromaticity.green_primary.y=y_;
  image()->chromaticity.green_primary.z=z_;
}

void Magick::Image::chromaGreenPrimary(double *x_,double *y_,double *z_) const
{
  *x_=constImage()->chromaticity.green_primary.x;
  *y_=constImage()->chromaticity.green_primary.y;
  *z_=constImage()->chromaticity.green_primary.z;
}

void Magick::Image::chromaRedPrimary(const double x_,const double y_,
  const double z_)
{
  modifyImage();
  image()->chromaticity.red_primary.x=x_;
  image()->chromaticity.red_primary.y=y_;
  image()->chromaticity.red_primary.z=z_;
}

void Magick::Image::chromaRedPrimary(double *x_,double *y_,double *z_) const
{
  *x_=constImage()->chromaticity.red_primary.x;
  *y_=constImage()->chromaticity.red_primary.y;
  *z_=constImage()->chromaticity.red_primary.z;
}

void Magick::Image::chromaWhitePoint(const double x_,const double y_,
  const double z_)
{
  modifyImage();
  image()->chromaticity.white_point.x=x_;
  image()->chromaticity.white_point.y=y_;
  image()->chromaticity.white_point.z=z_;
}

void Magick::Image::chromaWhitePoint(double *x_,double *y_,double *z_) const
{
  *x_=constImage()->chromaticity.white_point.x;
  *y_=constImage()->chromaticity.white_point.y;
  *z_=constImage()->chromaticity.white_point.z;
}

void Magick::Image::clamp(void)
{
  modifyImage();
  GetPPException;
  ClampImage(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::clampChannel(const ChannelType channel_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  ClampImage(image(),exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::clip(void)
{
  modifyImage();
  GetPPException;
  ClipImage(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::clipPath(const std::string pathname_,const bool inside_)
{
  modifyImage();
  GetPPException;
  ClipImagePath(image(),pathname_.c_str(),(MagickBooleanType) inside_,
    exceptionInfo);
  ThrowImageException;
}

void Magick::Image::clut(const Image &clutImage_,
  const PixelInterpolateMethod method)
{
  modifyImage();
  GetPPException;
  ClutImage(image(),clutImage_.constImage(),method,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::clutChannel(const ChannelType channel_,
  const Image &clutImage_,const PixelInterpolateMethod method)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  ClutImage(image(),clutImage_.constImage(),method,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
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
    blend[MagickPathExtent];

  MagickCore::Image
    *newImage;

  PixelInfo
    target;

  if (!penColor_.isValid())
    throwExceptionExplicit(MagickCore::OptionError,
      "Pen color argument is invalid");

  FormatLocaleString(blend,MagickPathExtent,"%u/%u/%u",alphaRed_,alphaGreen_,
    alphaBlue_);

  target=static_cast<PixelInfo>(penColor_);
  GetPPException;
  newImage=ColorizeImage(image(),blend,&target,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::colorMap(const size_t index_,const Color &color_)
{
  MagickCore::Image
    *imageptr;

  imageptr=image();

  if (index_ > (MaxColormapSize-1))
    throwExceptionExplicit(MagickCore::OptionError,
      "Colormap index must be less than MaxColormapSize");

  if (!color_.isValid())
    throwExceptionExplicit(MagickCore::OptionError,
      "Color argument is invalid");

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
      throwExceptionExplicit(MagickCore::OptionError,
        "Image does not contain a colormap");
      return(Color());
    }

  if (index_ > constImage()->colors-1)
    throwExceptionExplicit(MagickCore::OptionError,"Index out of range");

  return(Magick::Color((constImage()->colormap)[index_]));
}

void Magick::Image::colorMatrix(const size_t order_,
  const double *color_matrix_)
{
  KernelInfo
    *kernel_info;

  GetPPException;
  kernel_info=AcquireKernelInfo((const char *) NULL,exceptionInfo);
  if (kernel_info != (KernelInfo *) NULL)
    {
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
          newImage=ColorMatrixImage(image(),kernel_info,exceptionInfo);
          replaceImage(newImage);
        }
      kernel_info=DestroyKernelInfo(kernel_info);
    }
  ThrowImageException;
}

bool Magick::Image::compare(const Image &reference_) const
{
  bool
    status;

  Image
    ref=reference_;

  GetPPException;
  status=static_cast<bool>(IsImagesEqual(constImage(),ref.constImage(),
    exceptionInfo));
  ThrowImageException;
  return(status);
}

double Magick::Image::compare(const Image &reference_,const MetricType metric_)
{
  double
    distortion=0.0;

  GetPPException;
  GetImageDistortion(image(),reference_.constImage(),metric_,&distortion,
    exceptionInfo);
  ThrowImageException;
  return(distortion);
}

double Magick::Image::compareChannel(const ChannelType channel_,
  const Image &reference_,const MetricType metric_)
{
  double
    distortion=0.0;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  GetImageDistortion(image(),reference_.constImage(),metric_,&distortion,
    exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
  return(distortion);
}

Magick::Image Magick::Image::compare(const Image &reference_,
  const MetricType metric_,double *distortion)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=CompareImages(image(),reference_.constImage(),metric_,distortion,
    exceptionInfo);
  ThrowImageException;
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
  GetAndSetPPChannelMask(channel_);
  newImage=CompareImages(image(),reference_.constImage(),metric_,distortion,
    exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
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
  CompositeImage(image(),compositeImage_.constImage(),compose_,MagickTrue,
    x,y,exceptionInfo);
  ThrowImageException;
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
  CompositeImage(image(),compositeImage_.constImage(),compose_,MagickTrue,
    geometry.x,geometry.y,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::composite(const Image &compositeImage_,
  const ssize_t xOffset_,const ssize_t yOffset_,
  const CompositeOperator compose_)
{
  // Image supplied as compositeImage is composited with current image and
  // results in updating current image.
  modifyImage();
  GetPPException;
  CompositeImage(image(),compositeImage_.constImage(),compose_,MagickTrue,
    xOffset_,yOffset_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::connectedComponents(const size_t connectivity_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ConnectedComponentsImage(constImage(),connectivity_,
    (CCObjectInfo **) NULL,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::contrast(const bool sharpen_)
{
  modifyImage();
  GetPPException;
  ContrastImage(image(),(MagickBooleanType) sharpen_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::contrastStretch(const double blackPoint_,
  const double whitePoint_)
{
  modifyImage();
  GetPPException;
  ContrastStretchImage(image(),blackPoint_,whitePoint_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::contrastStretchChannel(const ChannelType channel_,
  const double blackPoint_,const double whitePoint_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  ContrastStretchImage(image(),blackPoint_,whitePoint_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::convolve(const size_t order_,const double *kernel_)
{
  KernelInfo
    *kernel_info;

  GetPPException;
  kernel_info=AcquireKernelInfo((const char *) NULL,exceptionInfo);
  kernel_info->width=order_;
  kernel_info->height=order_;
  kernel_info->x=(ssize_t) (order_-1)/2;
  kernel_info->y=(ssize_t) (order_-1)/2;
  kernel_info->values=(MagickRealType *) AcquireAlignedMemory(order_,
    order_*sizeof(*kernel_info->values));
  if (kernel_info->values != (MagickRealType *) NULL)
    {
      MagickCore::Image
        *newImage;

      for (ssize_t i=0; i < (ssize_t) (order_*order_); i++)
        kernel_info->values[i]=kernel_[i];
      newImage=ConvolveImage(image(),kernel_info,exceptionInfo);
      replaceImage(newImage);
    }
  kernel_info=DestroyKernelInfo(kernel_info);
  ThrowImageException;
}

void Magick::Image::copyPixels(const Image &source_,const Geometry &geometry_,
  const Offset &offset_)
{
  const OffsetInfo
    offset=offset_;

  const RectangleInfo
    geometry=geometry_;

  GetPPException;
  (void) CopyImagePixels(image(),source_.constImage(),&geometry,&offset,
    exceptionInfo);
  ThrowImageException;
}

void Magick::Image::crop(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    cropInfo=geometry_;

  GetPPException;
  newImage=CropImage(constImage(),&cropInfo,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::cycleColormap(const ssize_t amount_)
{
  modifyImage();
  GetPPException;
  CycleColormapImage(image(),amount_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::decipher(const std::string &passphrase_)
{
  modifyImage();
  GetPPException;
  DecipherImage(image(),passphrase_.c_str(),exceptionInfo);
  ThrowImageException;
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
  newImage=DeskewImage(constImage(),threshold_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::despeckle(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=DespeckleImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::display(void)
{
  GetPPException;
  DisplayImages(imageInfo(),image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::distort(const DistortMethod method_,
  const size_t numberArguments_,const double *arguments_,const bool bestfit_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=DistortImage(constImage(), method_,numberArguments_,arguments_,
    bestfit_ == true ? MagickTrue : MagickFalse,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::draw(const Magick::Drawable &drawable_)
{
  DrawingWand
    *wand;

  modifyImage();

  wand=AcquireDrawingWand(options()->drawInfo(),image());

  if(wand)
    {
      drawable_.operator()(wand);

      DrawRender(wand);

      ClonePPDrawException(wand);
      wand=DestroyDrawingWand(wand);
      ThrowPPDrawException(quiet());
    }
}

void Magick::Image::draw(const std::vector<Magick::Drawable> &drawable_)
{
  DrawingWand
    *wand;

  modifyImage();

  wand= AcquireDrawingWand(options()->drawInfo(),image());

  if(wand)
    {
      for (std::vector<Magick::Drawable>::const_iterator p = drawable_.begin();
           p != drawable_.end(); p++ )
        {
          p->operator()(wand);
          if (DrawGetExceptionType(wand) != MagickCore::UndefinedException)
            break;
        }

      if (DrawGetExceptionType(wand) == MagickCore::UndefinedException)
        DrawRender(wand);

      ClonePPDrawException(wand);
      wand=DestroyDrawingWand(wand);
      ThrowPPDrawException(quiet());
    }
}

void Magick::Image::edge(const double radius_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=EdgeImage(constImage(),radius_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::emboss(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=EmbossImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::encipher(const std::string &passphrase_)
{
  modifyImage();
  GetPPException;
  EncipherImage(image(),passphrase_.c_str(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::enhance(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=EnhanceImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::equalize(void)
{
  modifyImage();
  GetPPException;
  EqualizeImage(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::erase(void)
{
  modifyImage();
  GetPPException;
  (void) SetImageBackgroundColor(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::evaluate(const ChannelType channel_,
  const MagickEvaluateOperator operator_,double rvalue_)
{
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  EvaluateImage(image(),operator_,rvalue_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::evaluate(const ChannelType channel_,
  const MagickFunction function_,const size_t number_parameters_,
  const double *parameters_)
{
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  FunctionImage(image(),function_,number_parameters_,parameters_,
    exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::evaluate(const ChannelType channel_,const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_,
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
  cropImage=CropImage(image(),&geometry,exceptionInfo);
  GetAndSetPPChannelMask(channel_);
  EvaluateImage(cropImage,operator_,rvalue_,exceptionInfo);
  RestorePPChannelMask;
  (void) CompositeImage(image(),cropImage,image()->alpha_trait == 
    BlendPixelTrait ? OverCompositeOp : CopyCompositeOp,MagickFalse,
    geometry.x,geometry.y,exceptionInfo );
  cropImage=DestroyImageList(cropImage);
  ThrowImageException;
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
  newImage=ExtentImage(image(),&extentInfo,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
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
  newImage=FlipImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::floodFillAlpha(const ssize_t x_,const ssize_t y_,
  const unsigned int alpha_,const bool invert_)
{
  PixelInfo
    target;

  modifyImage();

  target=static_cast<PixelInfo>(pixelColor(x_,y_));
  target.alpha=alpha_;
  GetPPException;
  GetAndSetPPChannelMask(AlphaChannel);
  FloodfillPaintImage(image(),options()->drawInfo(),&target,x_,y_,
    (MagickBooleanType)invert_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::floodFillAlpha(const ssize_t x_,const ssize_t y_,
  const unsigned int alpha_,const Color &target_,const bool invert_)
{
  PixelInfo
    target;

  modifyImage();

  target=static_cast<PixelInfo>(target_);
  target.alpha=alpha_;
  GetPPException;
  GetAndSetPPChannelMask(AlphaChannel);
  FloodfillPaintImage(image(),options()->drawInfo(),&target,x_,y_,
    (MagickBooleanType)invert_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::floodFillColor(const Geometry &point_,
  const Magick::Color &fillColor_,const bool invert_)
{
  floodFillColor(point_.xOff(),point_.yOff(),fillColor_,invert_);
}

void Magick::Image::floodFillColor(const ssize_t x_,const ssize_t y_,
  const Magick::Color &fillColor_,const bool invert_)
{
  PixelInfo
    pixel;

  modifyImage();

  pixel=static_cast<PixelInfo>(pixelColor(x_,y_));
  floodFill(x_,y_,(Magick::Image *)NULL,fillColor_,&pixel,invert_);
}

void Magick::Image::floodFillColor(const Geometry &point_,
  const Magick::Color &fillColor_,const Magick::Color &borderColor_,
  const bool invert_)
{
  floodFillColor(point_.xOff(),point_.yOff(),fillColor_,borderColor_,invert_);
}

void Magick::Image::floodFillColor(const ssize_t x_,const ssize_t y_,
  const Magick::Color &fillColor_,const Magick::Color &borderColor_,
  const bool invert_)
{
  PixelInfo
    pixel;

  modifyImage();

  pixel=static_cast<PixelInfo>(borderColor_);
  floodFill(x_,y_,(Magick::Image *)NULL,fillColor_,&pixel,invert_);
}

void Magick::Image::floodFillTexture(const Magick::Geometry &point_,
  const Magick::Image &texture_,const bool invert_)
{
  floodFillTexture(point_.xOff(),point_.yOff(),texture_,invert_);
}

void Magick::Image::floodFillTexture(const ssize_t x_,const ssize_t y_,
  const Magick::Image &texture_,const bool invert_)
{
  PixelInfo
    pixel;

  modifyImage();

  pixel=static_cast<PixelInfo>(pixelColor(x_,y_));
  floodFill(x_,y_,&texture_,Magick::Color(),&pixel,invert_);
}

void Magick::Image::floodFillTexture(const Magick::Geometry &point_,
  const Magick::Image &texture_,const Magick::Color &borderColor_,
  const bool invert_)
{
  floodFillTexture(point_.xOff(),point_.yOff(),texture_,borderColor_,invert_);
}

void Magick::Image::floodFillTexture(const ssize_t x_,const ssize_t y_,
  const Magick::Image &texture_,const Magick::Color &borderColor_,
  const bool invert_)
{
  PixelInfo
    pixel;

  modifyImage();

  pixel=static_cast<PixelInfo>(borderColor_);
  floodFill(x_,y_,&texture_,Magick::Color(),&pixel,invert_);
}

void Magick::Image::flop(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=FlopImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::fontTypeMetrics(const std::string &text_,
  TypeMetric *metrics)
{
  DrawInfo
    *drawInfo;

  drawInfo=options()->drawInfo();
  drawInfo->text=const_cast<char *>(text_.c_str());
  GetPPException;
  GetTypeMetrics(image(),drawInfo,&(metrics->_typeMetric),exceptionInfo);
  drawInfo->text=0;
  ThrowImageException;
}

void Magick::Image::fontTypeMetricsMultiline(const std::string &text_,
  TypeMetric *metrics)
{
  DrawInfo
    *drawInfo;

  drawInfo=options()->drawInfo();
  drawInfo->text=const_cast<char *>(text_.c_str());
  GetPPException;
  GetMultilineTypeMetrics(image(),drawInfo,&(metrics->_typeMetric),exceptionInfo);
  drawInfo->text=0;
  ThrowImageException;
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
  newImage=FrameImage(constImage(),&info,image()->compose,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::frame(const size_t width_,const size_t height_,
  const ssize_t innerBevel_,const ssize_t outerBevel_)
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
  newImage=FrameImage(constImage(),&info,image()->compose,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::fx(const std::string expression_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=FxImage(constImage(),expression_.c_str(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::fx(const std::string expression_,
  const Magick::ChannelType channel_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=FxImage(constImage(),expression_.c_str(),exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::gamma(const double gamma_)
{
  modifyImage();
  GetPPException;
  GammaImage(image(),gamma_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::gamma(const double gammaRed_,const double gammaGreen_,
  const double gammaBlue_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(RedChannel);
  (void) GammaImage(image(),gammaRed_,exceptionInfo);
  SetPPChannelMask(GreenChannel);
  (void) GammaImage(image(),gammaGreen_,exceptionInfo);
  SetPPChannelMask(BlueChannel);
  (void) GammaImage(image(),gammaBlue_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::gaussianBlur(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=GaussianBlurImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::gaussianBlurChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=GaussianBlurImage(constImage(),radius_,sigma_,exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

const Magick::Quantum *Magick::Image::getConstPixels(const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_) const
{
  const Quantum
    *p;

  GetPPException;
  p=GetVirtualPixels(constImage(),x_, y_,columns_, rows_,exceptionInfo);
  ThrowImageException;
  return(p);
}

const void *Magick::Image::getConstMetacontent(void) const
{
  const void
    *result;

  result=GetVirtualMetacontent(constImage());

  if(!result)
    throwExceptionExplicit(MagickCore::OptionError,
      "Unable to retrieve meta content.");

  return(result);
}

void *Magick::Image::getMetacontent(void )
{
  void
    *result;

  result=GetAuthenticMetacontent(image());

  if(!result)
    throwExceptionExplicit(MagickCore::OptionError,
      "Unable to retrieve meta content.");

  return(result);
}

Magick::Quantum *Magick::Image::getPixels(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  Quantum
    *result;

  modifyImage();
  GetPPException;
  result=GetAuthenticPixels(image(),x_, y_,columns_,rows_,exceptionInfo);
  ThrowImageException;

  return(result);
}

void Magick::Image::grayscale(const PixelIntensityMethod method_)
{
  modifyImage();
  GetPPException;
  (void) GrayscaleImage(image(),method_,exceptionInfo);
  ThrowImageException;
}

void  Magick::Image::haldClut(const Image &clutImage_)
{
  modifyImage();
  GetPPException;
  (void) HaldClutImage(image(),clutImage_.constImage(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::houghLine(const size_t width_,const size_t height_,
  const size_t threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=HoughLineImage(constImage(),width_,height_,threshold_,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

Magick::ImageType Magick::Image::identifyType(void) const
{
  ImageType
    image_type;

  GetPPException;
  image_type=IdentifyImageType(constImage(),exceptionInfo);
  ThrowImageException;
  return(image_type);
}

void Magick::Image::implode(const double factor_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ImplodeImage(constImage(),factor_,image()->interpolate,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
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
    magnitude_ == true ? MagickTrue : MagickFalse,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::kuwahara(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=KuwaharaImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::kuwaharaChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=KuwaharaImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::level(const double blackPoint_,const double whitePoint_,
  const double gamma_)
{
  modifyImage();
  GetPPException;
  (void) LevelImage(image(),blackPoint_,whitePoint_,gamma_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::levelChannel(const ChannelType channel_,
  const double blackPoint_,const double whitePoint_,const double gamma_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  (void) LevelImage(image(),blackPoint_,whitePoint_,gamma_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::levelColors(const Color &blackColor_,
  const Color &whiteColor_,const bool invert_)
{
  PixelInfo
    black,
    white;

  modifyImage();

  black=static_cast<PixelInfo>(blackColor_);
  white=static_cast<PixelInfo>(whiteColor_);
  GetPPException;
  (void) LevelImageColors(image(),&black,&white,invert_ == true ?
    MagickTrue : MagickFalse,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::levelColorsChannel(const ChannelType channel_,
  const Color &blackColor_,const Color &whiteColor_,const bool invert_)
{
  PixelInfo
    black,
    white;

  modifyImage();

  black=static_cast<PixelInfo>(blackColor_);
  white=static_cast<PixelInfo>(whiteColor_);
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  (void) LevelImageColors(image(),&black,&white,invert_ == true ?
    MagickTrue : MagickFalse,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::levelize(const double blackPoint_,const double whitePoint_,
  const double gamma_)
{
  modifyImage();
  GetPPException;
  (void) LevelizeImage(image(),blackPoint_,whitePoint_,gamma_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::levelizeChannel(const ChannelType channel_,
  const double blackPoint_,const double whitePoint_,const double gamma_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  (void) LevelizeImage(image(),blackPoint_,whitePoint_,gamma_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::linearStretch(const double blackPoint_,
  const double whitePoint_)
{
  modifyImage();
  GetPPException;
  LinearStretchImage(image(),blackPoint_,whitePoint_,exceptionInfo);
  ThrowImageException;
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
  newImage=LiquidRescaleImage(image(),width,height,x,y,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::localContrast(const double radius_,const double strength_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=LocalContrastImage(constImage(),radius_,strength_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::magnify(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=MagnifyImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::map(const Image &mapImage_,const bool dither_)
{
  modifyImage();
  GetPPException;
  options()->quantizeDither(dither_);
  RemapImage(options()->quantizeInfo(),image(),mapImage_.constImage(),
    exceptionInfo);
  ThrowImageException;
}

void Magick::Image::meanShift(const size_t width_,const size_t height_,
  const double color_distance_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=MeanShiftImage(constImage(),width_,height_,color_distance_,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::medianFilter(const double radius_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=StatisticImage(image(),MedianStatistic,(size_t) radius_,
    (size_t) radius_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::minify(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=MinifyImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::modulate(const double brightness_,const double saturation_,
  const double hue_)
{
  char
    modulate[MagickPathExtent + 1];

  FormatLocaleString(modulate,MagickPathExtent,"%3.6f,%3.6f,%3.6f",brightness_,
    saturation_,hue_);

  modifyImage();
  GetPPException;
  ModulateImage(image(),modulate,exceptionInfo);
  ThrowImageException;
}

Magick::ImageMoments Magick::Image::moments(void) const
{
  return(ImageMoments(*this));
}

void Magick::Image::morphology(const MorphologyMethod method_,
  const std::string kernel_,const ssize_t iterations_)
{
  KernelInfo
    *kernel;

  MagickCore::Image
    *newImage;

  GetPPException;
  kernel=AcquireKernelInfo(kernel_.c_str(),exceptionInfo);
  if (kernel == (KernelInfo *) NULL)
    throwExceptionExplicit(MagickCore::OptionError,"Unable to parse kernel.");
  newImage=MorphologyImage(constImage(),method_,iterations_,kernel,
    exceptionInfo);
  replaceImage(newImage);
  kernel=DestroyKernelInfo(kernel);
  ThrowImageException;
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
    {
      throwExceptionExplicit(MagickCore::OptionError,
        "Unable to determine kernel type.");
      return;
    }
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


  GetPPException;
  kernel=AcquireKernelInfo(kernel_.c_str(),exceptionInfo);
  if (kernel == (KernelInfo *)NULL)
    {
      throwExceptionExplicit(MagickCore::OptionError,
        "Unable to parse kernel.");
      return;
    }
  GetAndSetPPChannelMask(channel_);
  newImage=MorphologyImage(constImage(),method_,iterations_,kernel,
    exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  kernel=DestroyKernelInfo(kernel);
  ThrowImageException;
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
    {
      throwExceptionExplicit(MagickCore::OptionError,
        "Unable to determine kernel type.");
      return;
    }

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
  newImage=MotionBlurImage(constImage(),radius_,sigma_,angle_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::negate(const bool grayscale_)
{
  modifyImage();
  GetPPException;
  NegateImage(image(),(MagickBooleanType) grayscale_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::negateChannel(const ChannelType channel_,
  const bool grayscale_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  NegateImage(image(),(MagickBooleanType) grayscale_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::normalize(void)
{
  modifyImage();
  GetPPException;
  NormalizeImage(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::oilPaint(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=OilPaintImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::opaque(const Color &opaqueColor_,const Color &penColor_,
  const bool invert_)
{
  std::string
    opaqueColor,
    penColor;

  PixelInfo
    opaque,
    pen;

  if (!opaqueColor_.isValid())
    throwExceptionExplicit(MagickCore::OptionError,
      "Opaque color argument is invalid");

  if (!penColor_.isValid())
    throwExceptionExplicit(MagickCore::OptionError,
      "Pen color argument is invalid");

  modifyImage();
  opaqueColor=opaqueColor_;
  penColor=penColor_;

  GetPPException;
  (void) QueryColorCompliance(opaqueColor.c_str(),AllCompliance,&opaque,
    exceptionInfo);
  (void) QueryColorCompliance(penColor.c_str(),AllCompliance,&pen,
    exceptionInfo);
  OpaquePaintImage(image(),&opaque,&pen,invert_ ? MagickTrue : MagickFalse,
    exceptionInfo);
  ThrowImageException;
}

void Magick::Image::orderedDither(std::string thresholdMap_)
{
  modifyImage();
  GetPPException;
  (void) OrderedDitherImage(image(),thresholdMap_.c_str(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::orderedDitherChannel(const ChannelType channel_,
  std::string thresholdMap_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  (void)OrderedDitherImage(image(),thresholdMap_.c_str(),exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::perceptible(const double epsilon_)
{
  modifyImage();
  GetPPException;
  PerceptibleImage(image(),epsilon_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::perceptibleChannel(const ChannelType channel_,
  const double epsilon_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  PerceptibleImage(image(),epsilon_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

 Magick::ImagePerceptualHash Magick::Image::perceptualHash() const
{
  return(ImagePerceptualHash(*this));
}

void Magick::Image::ping(const std::string &imageSpec_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  options()->fileName(imageSpec_);
  newImage=PingImage(imageInfo(),exceptionInfo);
  read(newImage,exceptionInfo);
}

void Magick::Image::ping(const Blob& blob_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=PingBlob(imageInfo(),blob_.data(),blob_.length(),exceptionInfo);
  read(newImage,exceptionInfo);
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
    throwExceptionExplicit(MagickCore::OptionError,
      "Access outside of image boundary");

  modifyImage();

  // Set image to DirectClass
  classType(DirectClass );

  // Get pixel view
  Pixels pixels(*this);
    // Set pixel value
  pixel=pixels.get(x_, y_, 1, 1 );
  packet=color_;
  MagickCore::SetPixelViaPixelInfo(constImage(),&packet,pixel);
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
    angle_,method_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::posterize(const size_t levels_,const DitherMethod method_)
{
  modifyImage();
  GetPPException;
  PosterizeImage(image(),levels_,method_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::posterizeChannel(const ChannelType channel_,
  const size_t levels_,const DitherMethod method_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  PosterizeImage(image(),levels_,method_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::process(std::string name_,const ssize_t argc,
  const char **argv)
{
  modifyImage();

  GetPPException;
  (void) InvokeDynamicImageFilter(name_.c_str(),&image(),argc,argv,
      exceptionInfo);
  ThrowImageException;
}

void Magick::Image::profile(const std::string name_,
  const Magick::Blob &profile_)
{
  modifyImage();
  GetPPException;
  (void) ProfileImage(image(),name_.c_str(),(unsigned char *)profile_.data(),
    profile_.length(),exceptionInfo);
  ThrowImageException;
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
  QuantizeImage(options()->quantizeInfo(),image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::raise(const Geometry &geometry_,const bool raisedFlag_)
{
  RectangleInfo
    raiseInfo=geometry_;

  GetPPException;
  modifyImage();
  RaiseImage(image(),&raiseInfo,raisedFlag_ == true ? MagickTrue : MagickFalse,
    exceptionInfo);
  ThrowImageException;
}

void Magick::Image::randomThreshold(const double low_,const double high_)
{
  GetPPException;
  (void) RandomThresholdImage(image(),low_,high_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::randomThresholdChannel(const ChannelType channel_,
  const double low_,const double high_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  (void) RandomThresholdImage(image(),low_,high_,exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::read(const Blob &blob_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=BlobToImage(imageInfo(),static_cast<const void *>(blob_.data()),
    blob_.length(),exceptionInfo);
  read(newImage,exceptionInfo);
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
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::read(const std::string &imageSpec_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  options()->fileName(imageSpec_);
  newImage=ReadImage(imageInfo(),exceptionInfo);
  read(newImage,exceptionInfo);
}

void Magick::Image::readMask(const Magick::Image &mask_)
{
  mask(mask_,ReadPixelMask);
}

Magick::Image Magick::Image::readMask(void) const
{
  return(mask(ReadPixelMask));
}

void Magick::Image::readPixels(const Magick::QuantumType quantum_,
  const unsigned char *source_)
{
  QuantumInfo
    *quantum_info;

  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  GetPPException;
  ImportQuantumPixels(image(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,source_,exceptionInfo);
  quantum_info=DestroyQuantumInfo(quantum_info);
  ThrowImageException;
}

void Magick::Image::reduceNoise(void)
{
  reduceNoise(3);
}

void Magick::Image::reduceNoise(const size_t order_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=StatisticImage(constImage(),NonpeakStatistic,order_,
    order_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::repage()
{
  modifyImage();
  options()->page(Geometry());
  image()->page.width = 0;
  image()->page.height = 0;
  image()->page.x = 0;
  image()->page.y = 0;
}

void Magick::Image::resample(const Point &density_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ResampleImage(constImage(),density_.x(),density_.y(),
    image()->filter,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
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
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::roll(const Geometry &roll_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=RollImage(constImage(),roll_.xOff(),roll_.yOff(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::roll(const size_t columns_,const size_t rows_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=RollImage(constImage(),static_cast<ssize_t>(columns_),
    static_cast<ssize_t>(rows_),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::rotate(const double degrees_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=RotateImage(constImage(),degrees_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::rotationalBlur(const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=RotationalBlurImage(constImage(),angle_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::rotationalBlurChannel(const ChannelType channel_,
  const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=RotationalBlurImage(constImage(),angle_,exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
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
  newImage=SampleImage(constImage(),width,height,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
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
  newImage=ScaleImage(constImage(),width,height,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::segment(const double clusterThreshold_,
  const double smoothingThreshold_)
{
  modifyImage();
  GetPPException;
  SegmentImage(image(),options()->quantizeColorSpace(),
    (MagickBooleanType) options()->verbose(),clusterThreshold_,
    smoothingThreshold_,exceptionInfo);
  SyncImage(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::selectiveBlur(const double radius_,const double sigma_,
  const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SelectiveBlurImage(constImage(),radius_,sigma_,threshold_,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::selectiveBlurChannel(const ChannelType channel_,
  const double radius_,const double sigma_,const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=SelectiveBlurImage(constImage(),radius_,sigma_,threshold_,
    exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

Magick::Image Magick::Image::separate(const ChannelType channel_) const
{
  MagickCore::Image
    *image;

  GetPPException;
  image=SeparateImage(constImage(),channel_,exceptionInfo);
  ThrowImageException;
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
  newImage=SepiaToneImage(constImage(),threshold_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

bool Magick::Image::setColorMetric(const Image &reference_)
{
  bool
    status;

  Image
    ref=reference_;

  GetPPException;
  modifyImage();
  status=static_cast<bool>(SetImageColorMetric(image(),ref.constImage(),
    exceptionInfo));
  ThrowImageException;
  return(status);
}

Magick::Quantum *Magick::Image::setPixels(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  Quantum
    *result;

  modifyImage();
  GetPPException;
  result=QueueAuthenticPixels(image(),x_,y_,columns_,rows_,exceptionInfo);
  ThrowImageException;
  return(result);
}

void Magick::Image::shade(const double azimuth_,const double elevation_,
  const bool colorShading_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ShadeImage(constImage(),colorShading_ == true ?
    MagickTrue : MagickFalse,azimuth_,elevation_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::shadow(const double percent_opacity_,const double sigma_,
  const ssize_t x_,const ssize_t y_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ShadowImage(constImage(),percent_opacity_, sigma_,x_, y_,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::sharpen(const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SharpenImage(constImage(),radius_,sigma_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::sharpenChannel(const ChannelType channel_,
  const double radius_,const double sigma_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=SharpenImage(constImage(),radius_,sigma_,exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::shave(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    shaveInfo=geometry_;

  GetPPException;
  newImage=ShaveImage(constImage(),&shaveInfo,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::shear(const double xShearAngle_,const double yShearAngle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=ShearImage(constImage(),xShearAngle_,yShearAngle_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::sigmoidalContrast(const bool sharpen_,
  const double contrast,const double midpoint)
{
  modifyImage();
  GetPPException;
  (void) SigmoidalContrastImage(image(),(MagickBooleanType) sharpen_,contrast,
    midpoint,exceptionInfo);
  ThrowImageException;
}

std::string Magick::Image::signature(const bool force_) const
{
  return(_imgRef->signature(force_));
}

void Magick::Image::sketch(const double radius_,const double sigma_,
  const double angle_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SketchImage(constImage(),radius_,sigma_,angle_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::solarize(const double factor_)
{
  modifyImage();
  GetPPException;
  SolarizeImage(image(),factor_,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::sparseColor(const ChannelType channel_,
  const SparseColorMethod method_,const size_t numberArguments_,
  const double *arguments_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=SparseColorImage(constImage(),method_,numberArguments_,arguments_,
    exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::splice(const Geometry &geometry_)
{
  MagickCore::Image
    *newImage;

  RectangleInfo
    spliceInfo=geometry_;

  GetPPException;
  newImage=SpliceImage(constImage(),&spliceInfo,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::splice(const Geometry &geometry_,
  const Color &backgroundColor_)
{
  backgroundColor(backgroundColor_);
  splice(geometry_);
}

void Magick::Image::splice(const Geometry &geometry_,
  const Color &backgroundColor_,const GravityType gravity_)
{
  backgroundColor(backgroundColor_);
  image()->gravity=gravity_;
  splice(geometry_);
}

void Magick::Image::spread(const double amount_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SpreadImage(constImage(),image()->interpolate,amount_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

Magick::ImageStatistics Magick::Image::statistics() const
{
  return(ImageStatistics(*this));
}

void Magick::Image::stegano(const Image &watermark_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=SteganoImage(constImage(),watermark_.constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::stereo(const Image &rightImage_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=StereoImage(constImage(),rightImage_.constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::strip(void)
{
  modifyImage();
  GetPPException;
  StripImage(image(),exceptionInfo);
  ThrowImageException;
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
    similarityThreshold,&offset,similarityMetric_,exceptionInfo);
  ThrowImageException;
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
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::syncPixels(void)
{
  GetPPException;
  (void) SyncAuthenticPixels(image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::texture(const Image &texture_)
{
  modifyImage();
  GetPPException;
  TextureImage(image(),texture_.constImage(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::threshold(const double threshold_)
{
  modifyImage();
  GetPPException;
  BilevelImage(image(),threshold_,exceptionInfo);
  ThrowImageException;
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
  newImage=ThumbnailImage(constImage(),width,height,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::tint(const std::string opacity_)
{
  MagickCore::Image
    *newImage;

  PixelInfo
    color;

  GetPPException;
  color=static_cast<PixelInfo>(constOptions()->fillColor());
  newImage=TintImage(constImage(),opacity_.c_str(),&color,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
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

void Magick::Image::transparent(const Color &color_,const bool inverse_)
{
  PixelInfo
    target;

  std::string
    color;

  if (!color_.isValid())
    throwExceptionExplicit(MagickCore::OptionError,
      "Color argument is invalid");

  color=color_;
  GetPPException;
  (void) QueryColorCompliance(color.c_str(),AllCompliance,&target,
    exceptionInfo);
  modifyImage();
  TransparentPaintImage(image(),&target,TransparentAlpha,
    inverse_ == true ? MagickTrue : MagickFalse,exceptionInfo);
  ThrowImageException;
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
    throwExceptionExplicit(MagickCore::OptionError,
      "Color argument is invalid");

  colorLow=colorLow_;
  colorHigh=colorHigh_;

  GetPPException;
  (void) QueryColorCompliance(colorLow.c_str(),AllCompliance,&targetLow,
    exceptionInfo);
  (void) QueryColorCompliance(colorHigh.c_str(),AllCompliance,&targetHigh,
    exceptionInfo);
  modifyImage();
  TransparentPaintImageChroma(image(),&targetLow,&targetHigh,TransparentAlpha,
    MagickFalse,exceptionInfo);
  ThrowImageException;
}

void Magick::Image::transpose(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=TransposeImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::transverse(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=TransverseImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::trim(void)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=TrimImage(constImage(),exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

Magick::Image Magick::Image::uniqueColors(void) const
{
  MagickCore::Image
    *image;

  GetPPException;
  image=UniqueImageColors(constImage(),exceptionInfo);
  ThrowImageException;
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
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::unsharpmaskChannel(const ChannelType channel_,
  const double radius_,const double sigma_,const double amount_,
  const double threshold_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  GetAndSetPPChannelMask(channel_);
  newImage=UnsharpMaskImage(constImage(),radius_,sigma_,amount_,threshold_,
    exceptionInfo);
  RestorePPChannelMask;
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::vignette(const double radius_,const double sigma_,
  const ssize_t x_,const ssize_t y_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=VignetteImage(constImage(),radius_,sigma_,x_,y_,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::wave(const double amplitude_,const double wavelength_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=WaveImage(constImage(),amplitude_,wavelength_,image()->interpolate,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::waveletDenoise(const double threshold_,
  const double softness_)
{
  MagickCore::Image
    *newImage;

  GetPPException;
  newImage=WaveletDenoiseImage(constImage(),threshold_,softness_,
    exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
}

void Magick::Image::whiteThreshold(const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  WhiteThresholdImage(image(),threshold_.c_str(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::whiteThresholdChannel(const ChannelType channel_,
  const std::string &threshold_)
{
  modifyImage();
  GetPPException;
  GetAndSetPPChannelMask(channel_);
  WhiteThresholdImage(image(),threshold_.c_str(),exceptionInfo);
  RestorePPChannelMask;
  ThrowImageException;
}

void Magick::Image::write(Blob *blob_)
{
  size_t
    length=0;

  void
    *data;

  modifyImage();
  GetPPException;
  data=ImagesToBlob(constImageInfo(),image(),&length,exceptionInfo);
  if (length > 0)
    blob_->updateNoCopy(data,length,Blob::MallocAllocator);
  ThrowImageException;
}

void Magick::Image::write(Blob *blob_,const std::string &magick_)
{
  size_t
    length=0;

  void
    *data;

  modifyImage();
  magick(magick_);
  GetPPException;
  data=ImagesToBlob(constImageInfo(),image(),&length,exceptionInfo);
  if (length > 0)
    blob_->updateNoCopy(data,length,Blob::MallocAllocator);
  ThrowImageException;
}

void Magick::Image::write(Blob *blob_,const std::string &magick_,
  const size_t depth_)
{
  size_t
    length=0;

  void
    *data;

  modifyImage();
  magick(magick_);
  depth(depth_);
  GetPPException;
  data=ImagesToBlob(constImageInfo(),image(),&length,exceptionInfo);
  if (length > 0)
    blob_->updateNoCopy(data,length,Blob::MallocAllocator);
  ThrowImageException;
}

void Magick::Image::write(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_,const std::string &map_,
  const StorageType type_,void *pixels_)
{
  GetPPException;
  ExportImagePixels(image(),x_,y_,columns_,rows_,map_.c_str(),type_,pixels_,
    exceptionInfo);
  ThrowImageException;
}

void Magick::Image::write(const std::string &imageSpec_)
{
  modifyImage();
  fileName(imageSpec_);
  GetPPException;
  WriteImage(constImageInfo(),image(),exceptionInfo);
  ThrowImageException;
}

void Magick::Image::writeMask(const Magick::Image &mask_)
{
  mask(mask_,WritePixelMask);
}

Magick::Image Magick::Image::writeMask(void) const
{
  return(mask(WritePixelMask));
}

void Magick::Image::writePixels(const Magick::QuantumType quantum_,
  unsigned char *destination_)
{
  QuantumInfo
    *quantum_info;

  quantum_info=AcquireQuantumInfo(imageInfo(),image());
  GetPPException;
  ExportQuantumPixels(image(),(MagickCore::CacheView *) NULL,quantum_info,
    quantum_,destination_, exceptionInfo);
  quantum_info=DestroyQuantumInfo(quantum_info);
  ThrowImageException;
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
  newImage=ResizeImage(constImage(),width,height,image()->filter,exceptionInfo);
  replaceImage(newImage);
  ThrowImageException;
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
  if (!_imgRef->isShared())
    return;

  GetPPException;
  replaceImage(CloneImage(image(),0,0,MagickTrue,exceptionInfo));
  ThrowImageException;
}

MagickCore::Image *Magick::Image::replaceImage(MagickCore::Image *replacement_)
{
  MagickCore::Image
    *image;

  if (replacement_)
    image=replacement_;
  else
    {
      GetPPException;
      image=AcquireImage(constImageInfo(),exceptionInfo);
      ThrowImageException;
    }

  _imgRef=ImageRef::replaceImage(_imgRef,image);
  return(image);
}

void Magick::Image::read(MagickCore::Image *image,
  MagickCore::ExceptionInfo *exceptionInfo)
{
  // Ensure that multiple image frames were not read.
  if (image != (MagickCore::Image *) NULL &&
      image->next != (MagickCore::Image *) NULL)
    {
      MagickCore::Image
        *next;

      // Destroy any extra image frames
      next=image->next;
      image->next=(MagickCore::Image *) NULL;
      next->previous=(MagickCore::Image *) NULL;
      DestroyImageList(next);
    }
  replaceImage(image);
  if (exceptionInfo->severity == MagickCore::UndefinedException &&
      image == (MagickCore::Image *) NULL)
    {
      (void) MagickCore::DestroyExceptionInfo(exceptionInfo);
      if (!quiet())
        throwExceptionExplicit(MagickCore::ImageWarning,
          "No image was loaded.");
      return;
    }
  ThrowImageException;
}

void Magick::Image::floodFill(const ssize_t x_,const ssize_t y_,
  const Magick::Image *fillPattern_,const Magick::Color &fill_,
  const MagickCore::PixelInfo *target_,const bool invert_)
{
  Magick::Color
    fillColor;

  MagickCore::Image
    *fillPattern;

  // Set drawing fill pattern or fill color
  fillColor=options()->fillColor();
  fillPattern=(MagickCore::Image *)NULL;
  if (options()->fillPattern() != (MagickCore::Image *)NULL)
    {
      GetPPException;
      fillPattern=CloneImage(options()->fillPattern(),0,0,MagickTrue,
        exceptionInfo);
      ThrowImageException;
    }

  if (fillPattern_ == (Magick::Image *)NULL)
    {
      options()->fillPattern((MagickCore::Image *)NULL);
      options()->fillColor(fill_);
    }
  else
    options()->fillPattern(fillPattern_->constImage());

  GetPPException;
  (void) FloodfillPaintImage(image(),options()->drawInfo(),
    target_,static_cast<ssize_t>(x_),static_cast<ssize_t>(y_),
    (MagickBooleanType) invert_,exceptionInfo);

  options()->fillColor(fillColor);
  options()->fillPattern(fillPattern);
  ThrowImageException;
}

void Magick::Image::mask(const Magick::Image &mask_,const PixelMask type)
{
  modifyImage();

  GetPPException;
  if (mask_.isValid())
    SetImageMask(image(),type,mask_.constImage(),exceptionInfo);
  else
    SetImageMask(image(),type,(MagickCore::Image *) NULL,
      exceptionInfo);
  ThrowImageException;
}

Magick::Image Magick::Image::mask(const PixelMask type) const
{
  MagickCore::Image
    *image;

  GetPPException;
  image = GetImageMask(constImage(),type,exceptionInfo);
  ThrowImageException;

  if (image == (MagickCore::Image *) NULL)
    return(Magick::Image());
  else
    return(Magick::Image(image));
}

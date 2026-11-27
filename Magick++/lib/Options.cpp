// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Implementation of Options
//
// A wrapper around DrawInfo, ImageInfo, and QuantizeInfo
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include "Magick++/Options.h"
#include "Magick++/Functions.h"
#include "Magick++/Exception.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#if !defined(MagickPI)
#define MagickPI  3.14159265358979323846264338327950288419716939937510
#endif
#if !defined(DegreesToRadians)
#define DegreesToRadians(x)  (MagickPI*(x)/180.0)
#endif

namespace
{
  // Post-multiply the current affine matrix by "affine" (in place).
  void ConcatenateAffine(MagickCore::AffineMatrix *target,
    const MagickCore::AffineMatrix &affine)
  {
    const MagickCore::AffineMatrix
      current=*target;

    target->sx=current.sx*affine.sx+current.ry*affine.rx;
    target->rx=current.rx*affine.sx+current.sy*affine.rx;
    target->ry=current.sx*affine.ry+current.ry*affine.sy;
    target->sy=current.rx*affine.ry+current.sy*affine.sy;
    target->tx=current.sx*affine.tx+current.ry*affine.ty+current.tx;
    target->ty=current.rx*affine.tx+current.sy*affine.ty+current.ty;
  }

  MagickCore::AffineMatrix IdentityAffine(void)
  {
    MagickCore::AffineMatrix
      affine;

    affine.sx=1.0;
    affine.rx=0.0;
    affine.ry=0.0;
    affine.sy=1.0;
    affine.tx=0.0;
    affine.ty=0.0;
    return(affine);
  }
}

Magick::Options::Options(void)
  : _imageInfo(static_cast<ImageInfo*>(AcquireMagickMemory(
      sizeof(ImageInfo)))),
    _quantizeInfo(static_cast<QuantizeInfo*>(AcquireMagickMemory(
      sizeof(QuantizeInfo)))),
    _drawInfo(static_cast<DrawInfo*>(AcquireMagickMemory(sizeof(DrawInfo)))),
    _quiet(false)
{
  if ((_imageInfo == (MagickCore::ImageInfo *) NULL) ||
      (_quantizeInfo == (MagickCore::QuantizeInfo *) NULL) ||
      (_drawInfo == (MagickCore::DrawInfo *) NULL))
    {
      // A throwing constructor does not run the destructor, so clean up here.
      if (_imageInfo != (MagickCore::ImageInfo *) NULL)
        (void) RelinquishMagickMemory(_imageInfo);
      if (_quantizeInfo != (MagickCore::QuantizeInfo *) NULL)
        (void) RelinquishMagickMemory(_quantizeInfo);
      if (_drawInfo != (MagickCore::DrawInfo *) NULL)
        (void) RelinquishMagickMemory(_drawInfo);
      throwExceptionExplicit(MagickCore::ResourceLimitError,
        "Unable to allocate options memory");
    }

  // Initialize image info with defaults
  GetImageInfo(_imageInfo);

  // Initialize quantization info
  GetQuantizeInfo(_quantizeInfo);

  // Initialize drawing info
  GetDrawInfo(_imageInfo,_drawInfo);
}

Magick::Options::Options(const Options& options_)
  : _imageInfo(CloneImageInfo(options_._imageInfo)),
    _quantizeInfo(CloneQuantizeInfo(options_._quantizeInfo)),
    _drawInfo(CloneDrawInfo(_imageInfo,options_._drawInfo)),
    _quiet(options_._quiet)
{
}

Magick::Options::~Options()
{
  // Destroy image info
   _imageInfo=DestroyImageInfo(_imageInfo);

  // Destroy quantization info
   _quantizeInfo=DestroyQuantizeInfo(_quantizeInfo);

  // Destroy drawing info
   _drawInfo=DestroyDrawInfo(_drawInfo);
}

void Magick::Options::adjoin(const bool flag_)
{
  _imageInfo->adjoin=static_cast<MagickBooleanType>(
    flag_ ? MagickTrue : MagickFalse);
}

bool Magick::Options::adjoin(void) const
{
  return(static_cast<bool>(_imageInfo->adjoin));
}

void Magick::Options::matteColor(const Color &matteColor_)
{
  _imageInfo->matte_color=matteColor_;
}

Magick::Color Magick::Options::matteColor(void) const
{
  return(Magick::Color(_imageInfo->matte_color));
}

void Magick::Options::backgroundColor(const Color &color_)
{
  _imageInfo->background_color=color_;
}

Magick::Color Magick::Options::backgroundColor(void) const
{
  return(Color(_imageInfo->background_color));
}

void Magick::Options::backgroundTexture(const std::string &backgroundTexture_)
{
  if (backgroundTexture_.length() == 0)
    _imageInfo->texture=(char *) RelinquishMagickMemory(_imageInfo->texture);
  else
    Magick::CloneString(&_imageInfo->texture,backgroundTexture_);
}

std::string Magick::Options::backgroundTexture(void) const
{
  if (_imageInfo->texture)
    return(std::string(_imageInfo->texture));
  else
    return(std::string());
}

void Magick::Options::borderColor(const Color &color_)
{
  _imageInfo->border_color=color_;
  _drawInfo->border_color=color_;
}

Magick::Color Magick::Options::borderColor(void) const
{
  return(Color(_imageInfo->border_color));
}

void Magick::Options::boxColor(const Color &boxColor_)
{
  _drawInfo->undercolor=boxColor_;
}

Magick::Color Magick::Options::boxColor(void) const
{
  return(Color(_drawInfo->undercolor));
}

void Magick::Options::colorspaceType(const ColorspaceType colorspace_)
{
  _imageInfo->colorspace=colorspace_;
}

Magick::ColorspaceType Magick::Options::colorspaceType(void) const
{
  return(static_cast<Magick::ColorspaceType>(_imageInfo->colorspace));
}

void Magick::Options::compressType(const CompressionType compressType_)
{
  _imageInfo->compression=compressType_;
}

Magick::CompressionType Magick::Options::compressType(void) const
{
  return(static_cast<Magick::CompressionType>(_imageInfo->compression));
}

void Magick::Options::colorFuzz(const double fuzz_)
{
  _imageInfo->fuzz=fuzz_;
}

double Magick::Options::colorFuzz(void) const
{
  return(_imageInfo->fuzz);
}

void Magick::Options::debug(const bool flag_)
{
  (void) SetLogEventMask(flag_ ? "All" : "None");
}

bool Magick::Options::debug(void) const
{
  return(IsEventLogging() != MagickFalse);
}

void Magick::Options::density(const Point &density_)
{
  if (!density_.isValid())
    _imageInfo->density=(char *) RelinquishMagickMemory(_imageInfo->density);
  else
    CloneString(&_imageInfo->density,density_);
}

Magick::Point Magick::Options::density(void) const
{
  if (_imageInfo->density)
    return(Point(_imageInfo->density));

  return(Point());
}

void Magick::Options::depth(const size_t depth_)
{
  _imageInfo->depth=depth_;
}

size_t Magick::Options::depth(void) const
{
  return(_imageInfo->depth);
}

void Magick::Options::endian(const Magick::EndianType endian_)
{
  _imageInfo->endian=endian_;
}

Magick::EndianType Magick::Options::endian(void) const
{
  return(_imageInfo->endian);
}

void Magick::Options::file(FILE *file_)
{
  SetImageInfoFile(_imageInfo,file_);
}

FILE *Magick::Options::file(void) const
{
  return(GetImageInfoFile(_imageInfo));
}

void Magick::Options::fileName(const std::string &fileName_)
{
  // Truncate to the size of the fixed-length buffer and always terminate.
  const size_t
    length=std::min(fileName_.length(),sizeof(_imageInfo->filename)-1);

  (void) fileName_.copy(_imageInfo->filename,length);
  _imageInfo->filename[length]=0;
}

std::string Magick::Options::fileName(void) const
{
  return(std::string(_imageInfo->filename));
}

void Magick::Options::fillColor(const Color &fillColor_)
{
  _drawInfo->fill=fillColor_;
  if (fillColor_ == Color())
    fillPattern((const MagickCore::Image*) NULL);
  setOption("fill",fillColor_);
}

Magick::Color Magick::Options::fillColor(void) const
{
  return(_drawInfo->fill);
}

void Magick::Options::fillPattern(const MagickCore::Image *fillPattern_)
{
  if (_drawInfo->fill_pattern)
    _drawInfo->fill_pattern=DestroyImageList(_drawInfo->fill_pattern);

  if (fillPattern_)
    {
      GetPPException;
      _drawInfo->fill_pattern=CloneImage(const_cast<MagickCore::Image*>(
        fillPattern_),0,0,static_cast<MagickBooleanType>(MagickTrue),
        exceptionInfo);
      ThrowPPException(_quiet);
    }
}

const MagickCore::Image *Magick::Options::fillPattern(void) const
{
  return(_drawInfo->fill_pattern);
}

void Magick::Options::fillRule(const FillRule &fillRule_)
{
  _drawInfo->fill_rule=fillRule_;
}

Magick::FillRule Magick::Options::fillRule(void) const
{
  return(_drawInfo->fill_rule);
}

void Magick::Options::font(const std::string &font_)
{
  if (font_.length() == 0)
    {
      _imageInfo->font=(char *) RelinquishMagickMemory(_imageInfo->font);
      _drawInfo->font=(char *) RelinquishMagickMemory(_drawInfo->font);
    }
  else
    {
      Magick::CloneString(&_imageInfo->font,font_);
      Magick::CloneString(&_drawInfo->font,font_);
    }
}

std::string Magick::Options::font(void) const
{
  if (_imageInfo->font)
    return(std::string(_imageInfo->font));

  return(std::string());
}

void Magick::Options::fontFamily(const std::string &family_)
{
  if (family_.length() == 0)
    {
      _drawInfo->family=(char *) RelinquishMagickMemory(_drawInfo->family);
      DestroyString(RemoveImageOption(imageInfo(),"family"));
    }
  else
    {
      Magick::CloneString(&_drawInfo->family,family_);
      (void) SetImageOption(imageInfo(),"family",family_.c_str());
    }
}

std::string Magick::Options::fontFamily(void) const
{
  if (_drawInfo->family)
    return(std::string(_drawInfo->family));

  return(std::string());
}

void Magick::Options::fontPointsize(const double pointSize_)
{
  _imageInfo->pointsize=pointSize_;
  _drawInfo->pointsize=pointSize_;
}

double Magick::Options::fontPointsize(void) const
{
  return(_imageInfo->pointsize);
}

void Magick::Options::fontStyle(const StyleType style_)
{
  _drawInfo->style=style_;
  (void) SetImageOption(_imageInfo,"style",CommandOptionToMnemonic(
    MagickStyleOptions,(ssize_t) style_));
}

Magick::StyleType Magick::Options::fontStyle(void) const
{
  return(_drawInfo->style);
}

void Magick::Options::fontWeight(const size_t weight_)
{
  _drawInfo->weight=weight_;
  setOption("weight",(double) weight_);
}

size_t Magick::Options::fontWeight(void) const
{
  return(_drawInfo->weight);
}

std::string Magick::Options::format(void) const
{
  const MagickInfo
    *magick_info=0;

  GetPPException;
  if (*_imageInfo->magick != '\0' )
    magick_info = GetMagickInfo(_imageInfo->magick,exceptionInfo);
  ThrowPPException(_quiet);

  if ((magick_info != 0) && (*magick_info->description != '\0'))
    return(std::string( magick_info->description));

  return(std::string());
}

void Magick::Options::interlaceType(const InterlaceType interlace_)
{
  _imageInfo->interlace=interlace_;
}

Magick::InterlaceType Magick::Options::interlaceType(void) const
{
  return(static_cast<Magick::InterlaceType>(_imageInfo->interlace));
}

void Magick::Options::magick(const std::string &magick_)
{
  if (magick_.empty())
  {
    _imageInfo->magick[0] = '\0';
    return;
  }

  FormatLocaleString(_imageInfo->filename,MagickPathExtent,"%.1024s:",
    magick_.c_str());
  GetPPException;
  SetImageInfo(_imageInfo,1,exceptionInfo);
  ThrowPPException(_quiet);
  if ( _imageInfo->magick[0] == '\0' )
    throwExceptionExplicit(MagickCore::OptionError,"Unrecognized image format",
      magick_.c_str());
}

std::string Magick::Options::magick(void) const
{
  if ( _imageInfo->magick[0] != '\0' )
    return(std::string(_imageInfo->magick));

  return(std::string());
}

void Magick::Options::monochrome(const bool monochromeFlag_)
{
  _imageInfo->monochrome=(monochromeFlag_ ? MagickTrue : MagickFalse);
}

bool Magick::Options::monochrome(void) const
{
  return(static_cast<bool>(_imageInfo->monochrome));
}

void Magick::Options::page(const Geometry &pageSize_)
{
  if (!pageSize_.isValid())
    _imageInfo->page=(char *) RelinquishMagickMemory(_imageInfo->page);
  else
    Magick::CloneString(&_imageInfo->page,pageSize_);
}

Magick::Geometry Magick::Options::page(void) const
{
  if (_imageInfo->page)
    return(Geometry(_imageInfo->page));

  return(Geometry());
}

void Magick::Options::quality(const size_t quality_)
{
  _imageInfo->quality=quality_;
}

size_t Magick::Options::quality(void) const
{
  return(_imageInfo->quality);
}

void Magick::Options::quantizeColors(const size_t colors_)
{
  _quantizeInfo->number_colors=colors_;
}

size_t Magick::Options::quantizeColors(void) const
{
  return(_quantizeInfo->number_colors);
}

void Magick::Options::quantizeColorSpace(const ColorspaceType colorSpace_)
{
  _quantizeInfo->colorspace=colorSpace_;
}

Magick::ColorspaceType Magick::Options::quantizeColorSpace(void) const
{
  return(static_cast<Magick::ColorspaceType>(_quantizeInfo->colorspace));
}

void Magick::Options::quantizeDither(const bool ditherFlag_)
{
  quantizeDither(ditherFlag_ ? RiemersmaDitherMethod : NoDitherMethod);
}

bool Magick::Options::quantizeDither(void) const
{
  return(static_cast<bool>(_imageInfo->dither));
}

void Magick::Options::quantizeDither(const DitherMethod ditherMethod_)
{
  quantizeDitherMethod(ditherMethod_);
}

void Magick::Options::quantizeDitherMethod(const DitherMethod ditherMethod_)
{
  // Keep the boolean dither flag and the method consistent with each other.
  _imageInfo->dither=(ditherMethod_ != NoDitherMethod ? MagickTrue :
    MagickFalse);
  _quantizeInfo->dither_method=ditherMethod_;
}

MagickCore::DitherMethod Magick::Options::quantizeDitherMethod(void) const
{
  return(_quantizeInfo->dither_method);
}

void Magick::Options::quantizeTreeDepth(const size_t treeDepth_)
{
  _quantizeInfo->tree_depth=treeDepth_;
}

size_t Magick::Options::quantizeTreeDepth(void) const
{
  return(_quantizeInfo->tree_depth);
}

void Magick::Options::quiet(const bool quiet_)
{
  _quiet=quiet_;
}

bool Magick::Options::quiet(void) const
{
  return(_quiet);
}

void Magick::Options::resolutionUnits(const ResolutionType resolutionUnits_)
{
  _imageInfo->units=resolutionUnits_;
}

Magick::ResolutionType Magick::Options::resolutionUnits(void) const
{
  return(_imageInfo->units);
}

void Magick::Options::samplingFactor(const std::string &samplingFactor_)
{
  if (samplingFactor_.length() == 0)
    _imageInfo->sampling_factor=(char *) RelinquishMagickMemory(
      _imageInfo->sampling_factor);
  else
    Magick::CloneString(&_imageInfo->sampling_factor,samplingFactor_);
}

std::string Magick::Options::samplingFactor(void) const
{
  if (_imageInfo->sampling_factor)
    return(std::string(_imageInfo->sampling_factor));

  return(std::string());
}

void Magick::Options::size(const Geometry &geometry_)
{
  _imageInfo->size=(char *) RelinquishMagickMemory(_imageInfo->size);

  if (geometry_.isValid())
    Magick::CloneString(&_imageInfo->size,geometry_);
}

Magick::Geometry Magick::Options::size(void) const
{
  if (_imageInfo->size)
    return(Geometry(_imageInfo->size));

  return(Geometry());
}

void Magick::Options::strokeAntiAlias(const bool flag_)
{
  _drawInfo->stroke_antialias=(flag_ ? MagickTrue : MagickFalse);
}

bool Magick::Options::strokeAntiAlias(void) const
{
  return(_drawInfo->stroke_antialias != 0 ? true : false);
}

void Magick::Options::strokeColor(const Color &strokeColor_)
{
  _drawInfo->stroke=strokeColor_;
  if (strokeColor_ == Color())
    strokePattern((const MagickCore::Image*) NULL);
  setOption("stroke",strokeColor_);
}

Magick::Color Magick::Options::strokeColor(void) const
{
  return(_drawInfo->stroke);
}

void Magick::Options::strokeDashArray(const double *strokeDashArray_)
{
  _drawInfo->dash_pattern=(double *) RelinquishMagickMemory(
    _drawInfo->dash_pattern);

  if(strokeDashArray_)
    {
      size_t
        x;

      // The array is terminated by a zero entry.
      for (x=0; std::fabs(strokeDashArray_[x]) >= MagickEpsilon; x++) ;
      _drawInfo->dash_pattern=static_cast<double*>(AcquireMagickMemory((x+1)*
        sizeof(double)));
      if (!_drawInfo->dash_pattern)
        throwExceptionExplicit(MagickCore::ResourceLimitError,
          "Unable to allocate dash-pattern memory");
      else
        {
          (void) memcpy(_drawInfo->dash_pattern,strokeDashArray_,
            x*sizeof(double));
          _drawInfo->dash_pattern[x]=0.0;
        }
    }
}

const double *Magick::Options::strokeDashArray(void) const
{
  return(_drawInfo->dash_pattern);
}

void Magick::Options::strokeDashOffset(const double strokeDashOffset_)
{
  _drawInfo->dash_offset=strokeDashOffset_;
}

double Magick::Options::strokeDashOffset(void) const
{
  return(_drawInfo->dash_offset);
}

void Magick::Options::strokeLineCap(const LineCap lineCap_)
{
  _drawInfo->linecap=lineCap_;
}

Magick::LineCap Magick::Options::strokeLineCap(void) const
{
  return(_drawInfo->linecap);
}

void Magick::Options::strokeLineJoin(const LineJoin lineJoin_)
{
  _drawInfo->linejoin=lineJoin_;
}

Magick::LineJoin Magick::Options::strokeLineJoin(void) const
{
  return(_drawInfo->linejoin);
}

void Magick::Options::strokeMiterLimit(const size_t miterLimit_)
{
  _drawInfo->miterlimit=miterLimit_;
}

size_t Magick::Options::strokeMiterLimit(void) const
{
  return(_drawInfo->miterlimit);
}

void Magick::Options::strokePattern(const MagickCore::Image *strokePattern_)
{
  if (_drawInfo->stroke_pattern)
    _drawInfo->stroke_pattern=DestroyImageList(_drawInfo->stroke_pattern);

  if (strokePattern_)
    {
      GetPPException;
      _drawInfo->stroke_pattern=CloneImage(const_cast<MagickCore::Image*>(
        strokePattern_),0,0,MagickTrue,exceptionInfo);
      ThrowPPException(_quiet);
    }
}

const MagickCore::Image *Magick::Options::strokePattern(void) const
{
  return(_drawInfo->stroke_pattern);
}

void Magick::Options::strokeWidth(const double strokeWidth_)
{
  _drawInfo->stroke_width=strokeWidth_;
  setOption("strokewidth",strokeWidth_);
}

double Magick::Options::strokeWidth(void) const
{
  return(_drawInfo->stroke_width);
}

void Magick::Options::subImage(const size_t subImage_)
{
  _imageInfo->scene=subImage_;
}

size_t Magick::Options::subImage(void) const
{
  return(_imageInfo->scene);
}

void Magick::Options::subRange(const size_t subRange_)
{
  _imageInfo->number_scenes=subRange_;
}

size_t Magick::Options::subRange(void) const
{
  return(_imageInfo->number_scenes);
}

void Magick::Options::textAntiAlias(const bool flag_)
{
  _drawInfo->text_antialias=static_cast<MagickBooleanType>(
    flag_ ? MagickTrue : MagickFalse);
}

bool Magick::Options::textAntiAlias(void) const
{
  return(static_cast<bool>(_drawInfo->text_antialias));
}

void Magick::Options::textDirection(const DirectionType direction_)
{
  _drawInfo->direction=direction_;
  (void) SetImageOption(_imageInfo,"direction",CommandOptionToMnemonic(
    MagickDirectionOptions,(ssize_t) direction_));
}

Magick::DirectionType Magick::Options::textDirection() const
{
  return(_drawInfo->direction);
}

void Magick::Options::textEncoding(const std::string &encoding_)
{
  if (encoding_.length() == 0)
    {
      _drawInfo->encoding=(char *) RelinquishMagickMemory(_drawInfo->encoding);
      DestroyString(RemoveImageOption(_imageInfo,"encoding"));
    }
  else
    {
      Magick::CloneString(&_drawInfo->encoding,encoding_);
      (void) SetImageOption(_imageInfo,"encoding",encoding_.c_str());
    }
}

std::string Magick::Options::textEncoding(void) const
{
  if (_drawInfo->encoding && *_drawInfo->encoding)
    return(std::string(_drawInfo->encoding));

  return(std::string());
}

void Magick::Options::textGravity(const GravityType gravity_)
{
  _drawInfo->gravity=gravity_;
  (void) SetImageOption(_imageInfo,"gravity",CommandOptionToMnemonic(
    MagickGravityOptions,(ssize_t) gravity_));
}

Magick::GravityType Magick::Options::textGravity() const
{
  return(_drawInfo->gravity);
}

void Magick::Options::textInterlineSpacing(const double spacing_)
{
  _drawInfo->interline_spacing=spacing_;
  setOption("interline-spacing",spacing_);
}

double Magick::Options::textInterlineSpacing(void) const
{
  return(_drawInfo->interline_spacing);
}

void Magick::Options::textInterwordSpacing(const double spacing_)
{
  _drawInfo->interword_spacing=spacing_;
  setOption("interword-spacing",spacing_);
}

double Magick::Options::textInterwordSpacing(void) const
{
  return(_drawInfo->interword_spacing);
}

void Magick::Options::textKerning(const double kerning_)
{
  _drawInfo->kerning=kerning_;
  setOption("kerning",kerning_);
}

double Magick::Options::textKerning(void) const
{
  return(_drawInfo->kerning);
}

void Magick::Options::textUnderColor(const Color &undercolor_)
{
  _drawInfo->undercolor=undercolor_;
  setOption("undercolor",undercolor_);
}

Magick::Color Magick::Options::textUnderColor(void) const
{
  return(_drawInfo->undercolor);
}

void Magick::Options::transformOrigin(const double tx_,const double ty_)
{
  MagickCore::AffineMatrix
    affine=IdentityAffine();

  affine.tx=tx_;
  affine.ty=ty_;
  ConcatenateAffine(&_drawInfo->affine,affine);
}

void Magick::Options::transformReset(void)
{
  _drawInfo->affine=IdentityAffine();
}

void Magick::Options::transformRotation(const double angle_)
{
  const double
    radians=DegreesToRadians(std::fmod(angle_,360.0));

  MagickCore::AffineMatrix
    affine=IdentityAffine();

  affine.sx=std::cos(radians);
  affine.rx=(-std::sin(radians));
  affine.ry=std::sin(radians);
  affine.sy=std::cos(radians);
  ConcatenateAffine(&_drawInfo->affine,affine);
}

void Magick::Options::transformScale(const double sx_,const double sy_)
{
  MagickCore::AffineMatrix
    affine=IdentityAffine();

  affine.sx=sx_;
  affine.sy=sy_;
  ConcatenateAffine(&_drawInfo->affine,affine);
}

void Magick::Options::transformSkewX(const double skewx_)
{
  MagickCore::AffineMatrix
    affine=IdentityAffine();

  affine.ry=std::tan(DegreesToRadians(std::fmod(skewx_,360.0)));
  ConcatenateAffine(&_drawInfo->affine,affine);
}

void Magick::Options::transformSkewY(const double skewy_)
{
  MagickCore::AffineMatrix
    affine=IdentityAffine();

  affine.rx=std::tan(DegreesToRadians(std::fmod(skewy_,360.0)));
  ConcatenateAffine(&_drawInfo->affine,affine);
}

void Magick::Options::type(const ImageType type_)
{
  _imageInfo->type=type_;
}

Magick::ImageType Magick::Options::type(void) const
{
  return(_imageInfo->type);
}

void Magick::Options::verbose(const bool verboseFlag_)
{
  _imageInfo->verbose=(verboseFlag_ ? MagickTrue : MagickFalse);
}

bool Magick::Options::verbose(void) const
{
  return(static_cast<bool>(_imageInfo->verbose));
}

void Magick::Options::x11Display(const std::string &display_)
{
  if (display_.length() == 0)
    _imageInfo->server_name=(char *) RelinquishMagickMemory(
      _imageInfo->server_name);
  else
    Magick::CloneString(&_imageInfo->server_name,display_);
}

std::string Magick::Options::x11Display(void) const
{
  if (_imageInfo->server_name)
    return(std::string( _imageInfo->server_name));

  return(std::string());
}

MagickCore::DrawInfo *Magick::Options::drawInfo(void)
{
  return(_drawInfo);
}

MagickCore::ImageInfo *Magick::Options::imageInfo(void)
{
  return(_imageInfo);
}

MagickCore::QuantizeInfo *Magick::Options::quantizeInfo(void)
{
  return(_quantizeInfo);
}

Magick::Options::Options(const MagickCore::ImageInfo* imageInfo_,
  const MagickCore::QuantizeInfo* quantizeInfo_,
  const MagickCore::DrawInfo* drawInfo_)
: _imageInfo((MagickCore::ImageInfo* ) NULL),
  _quantizeInfo((MagickCore::QuantizeInfo* ) NULL),
  _drawInfo((MagickCore::DrawInfo* ) NULL),
  _quiet(false)
{
  _imageInfo=CloneImageInfo(imageInfo_);
  _quantizeInfo=CloneQuantizeInfo(quantizeInfo_);
  _drawInfo=CloneDrawInfo(imageInfo_,drawInfo_);
}

void Magick::Options::setOption(const char *name,const Color &value_)
{
  std::string
    option;

  option=value_;
  (void) SetImageOption(imageInfo(),name,option.c_str());
}

void Magick::Options::setOption(const char *name,const double value_)
{
  char
    option[MagickPathExtent];

  (void) FormatLocaleString(option,MagickPathExtent,"%.17g",value_);
  (void) SetImageOption(_imageInfo,name,option);
}


// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Inclusion of ImageMagick headers (with namespace magic)

#ifndef Magick_Include_header
#define Magick_Include_header

#if !defined(_MAGICK_CONFIG_H)
# define _MAGICK_CONFIG_H
# if !defined(vms) && !defined(macintosh)
#  include "magick/magick-config.h"
# else
#  include "magick-config.h"
# endif
# undef inline // Remove possible definition from config.h
# undef class
#endif

// Needed for stdio FILE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(MAGICKCORE_HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif

#if defined(__BORLANDC__)
# include <vcl.h> /* Borland C++ Builder 4.0 requirement */
#endif // defined(__BORLANDC__)

//
// Include ImageMagick headers into namespace "MagickCore". If
// MAGICKCORE_IMPLEMENTATION is defined, include ImageMagick development
// headers.  This scheme minimizes the possibility of conflict with
// user code.
//
namespace MagickCore
{
#include <magick/MagickCore.h>
#include <wand/MagickWand.h>
#undef inline // Remove possible definition from config.h

#undef class
}

//
// Provide appropriate DLL imports/exports for Visual C++,
// Borland C++Builder and MinGW builds.
//
#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
# define MagickCplusPlusDLLSupported
#endif
#if defined(MagickCplusPlusDLLSupported)
#  if defined(_MT) && defined(_DLL) && !defined(_LIB) && !defined(STATIC_MAGICK)
//
// In a native Windows build, the following defines are used:
//
//   _MT         = Multithreaded
//   _DLL        = Using code is part of a DLL
//   _LIB        = Using code is being built as a library.
//   _MAGICKMOD_ = Build uses loadable modules (Magick++ does not care about this)
//
// In the case where ImageMagick is built as a static library but the
// using code is dynamic, STATIC_MAGICK may be defined in the project to
// override triggering dynamic library behavior.
//
#    if defined(_VISUALC_)
#      define MagickDLLExplicitTemplate
#      pragma warning( disable: 4273 )  /* Disable the stupid dll linkage warnings */
#      pragma warning( disable: 4251 )
#    endif
#    if !defined(MAGICKCORE_IMPLEMENTATION)
#      if defined(__GNUC__)
#       define MagickPPExport __attribute__ ((dllimport))
#      else
#       define MagickPPExport __declspec(dllimport)
#      endif
#      define MagickPPPrivate extern __declspec(dllimport)
#      if defined(_VISUALC_)
#        pragma message( "Magick++ lib DLL import" )
#      endif
#    else
#      if defined(__BORLANDC__) || defined(__MINGW32__)
#        define MagickPPExport __declspec(dllexport)
#        define MagickPPPrivate __declspec(dllexport)
#        if defined(__BORLANDC__)
#          pragma message( "BCBMagick++ lib DLL export" )
#        endif
#      else
#        if defined(__GNUC__)
#         define MagickPPExport __attribute__ ((dllexport))
#        else
#         define MagickPPExport __declspec(dllexport)
#        endif
#        define MagickPPPrivate extern __declspec(dllexport)
#      endif
#      if defined(_VISUALC_)
#        pragma message( "Magick++ lib DLL export" )
#      endif
#    endif
#  else
#    define MagickPPExport
#    define MagickPPPrivate
#    if defined(_VISUALC_)
#      pragma message( "Magick++ lib static interface" )
#    endif
#  endif
#else
# if __GNUC__ >= 4
#  define MagickPPExport __attribute__ ((visibility ("default")))
#  define MagickPPPrivate  __attribute__ ((visibility ("hidden")))
# else
#   define MagickPPExport
#   define MagickPPPrivate
# endif
#endif

#if defined(WIN32) && defined(_VISUALC_)
#  pragma warning(disable : 4996) /* function deprecation warnings */
#endif

//
// Import ImageMagick symbols and types which are used as part of the
// Magick++ API definition into namespace "Magick".
//
namespace Magick
{
  // The datatype for an RGB component
  using MagickCore::Quantum;
  using MagickCore::MagickRealType;
  using MagickCore::MagickSizeType;

  // Boolean types
  using MagickCore::MagickBooleanType;
  using MagickCore::MagickFalse;
  using MagickCore::MagickTrue;

  // Image class types
  using MagickCore::ClassType;
  using MagickCore::UndefinedClass;
  using MagickCore::DirectClass;
  using MagickCore::PseudoClass;
  
  // Channel types
  using MagickCore::ChannelType;
  using MagickCore::UndefinedChannel;
  using MagickCore::RedChannel;
  using MagickCore::CyanChannel;
  using MagickCore::GreenChannel;
  using MagickCore::MagentaChannel;
  using MagickCore::BlueChannel;
  using MagickCore::YellowChannel;
  using MagickCore::OpacityChannel;
  using MagickCore::BlackChannel;
  using MagickCore::MatteChannel;
  using MagickCore::DefaultChannels;
  using MagickCore::AllChannels;
  
  // Color-space types
  using MagickCore::CMYKColorspace;
  using MagickCore::ColorspaceType;
  using MagickCore::GRAYColorspace;
  using MagickCore::HSLColorspace;
  using MagickCore::HWBColorspace;
  using MagickCore::LogColorspace;
  using MagickCore::OHTAColorspace;
  using MagickCore::Rec601LumaColorspace;
  using MagickCore::Rec709LumaColorspace;
  using MagickCore::RGBColorspace;
  using MagickCore::sRGBColorspace;
  using MagickCore::TransparentColorspace;
  using MagickCore::UndefinedColorspace;
  using MagickCore::XYZColorspace;
  using MagickCore::YCbCrColorspace;
  using MagickCore::YCCColorspace;
  using MagickCore::YIQColorspace;
  using MagickCore::YPbPrColorspace;
  using MagickCore::YUVColorspace;
  
  // Composition operations
  using MagickCore::AddCompositeOp;
  using MagickCore::AtopCompositeOp;
  using MagickCore::BlendCompositeOp;
  using MagickCore::BumpmapCompositeOp;
  using MagickCore::ClearCompositeOp;
  using MagickCore::ColorizeCompositeOp;
  using MagickCore::CompositeOperator;
  using MagickCore::CopyBlueCompositeOp;
  using MagickCore::CopyCompositeOp;
  using MagickCore::CopyCyanCompositeOp;
  using MagickCore::CopyGreenCompositeOp;
  using MagickCore::CopyMagentaCompositeOp;
  using MagickCore::CopyOpacityCompositeOp;
  using MagickCore::CopyRedCompositeOp;
  using MagickCore::CopyYellowCompositeOp;
  using MagickCore::DarkenCompositeOp;
  using MagickCore::DifferenceCompositeOp;
  using MagickCore::DisplaceCompositeOp;
  using MagickCore::DissolveCompositeOp;
  using MagickCore::DstOverCompositeOp;
  using MagickCore::ExclusionCompositeOp;
  using MagickCore::HardLightCompositeOp;
  using MagickCore::HueCompositeOp;
  using MagickCore::InCompositeOp;
  using MagickCore::LightenCompositeOp;
  using MagickCore::LuminizeCompositeOp;
  using MagickCore::MinusCompositeOp;
  using MagickCore::ModulateCompositeOp;
  using MagickCore::MultiplyCompositeOp;
  using MagickCore::NoCompositeOp;
  using MagickCore::OutCompositeOp;
  using MagickCore::OverCompositeOp;
  using MagickCore::OverlayCompositeOp;
  using MagickCore::PlusCompositeOp;
  using MagickCore::SaturateCompositeOp;
  using MagickCore::ScreenCompositeOp;
  using MagickCore::SoftLightCompositeOp;
  using MagickCore::SubtractCompositeOp;
  using MagickCore::ThresholdCompositeOp;
  using MagickCore::UndefinedCompositeOp;
  using MagickCore::XorCompositeOp;
  using MagickCore::CopyBlackCompositeOp;
  
  // Compression algorithms
  using MagickCore::CompressionType;
  using MagickCore::UndefinedCompression;
  using MagickCore::NoCompression;
  using MagickCore::BZipCompression;
  using MagickCore::FaxCompression;
  using MagickCore::Group4Compression;
  using MagickCore::JPEGCompression;
  using MagickCore::LZMACompression;
  using MagickCore::LZWCompression;
  using MagickCore::RLECompression;
  using MagickCore::ZipCompression;

  // Dispose methods
  using MagickCore::DisposeType;
  using MagickCore::UndefinedDispose;
  using MagickCore::NoneDispose;
  using MagickCore::BackgroundDispose;
  using MagickCore::PreviousDispose;

  // Distort methods
  using MagickCore::DistortImageMethod;
  using MagickCore::UndefinedDistortion;
  using MagickCore::AffineDistortion;
  using MagickCore::AffineProjectionDistortion;
  using MagickCore::ScaleRotateTranslateDistortion;
  using MagickCore::PerspectiveDistortion;
  using MagickCore::PerspectiveProjectionDistortion;
  using MagickCore::BilinearDistortion;
  using MagickCore::PolynomialDistortion;
  using MagickCore::ArcDistortion;
  using MagickCore::PolarDistortion;
  using MagickCore::DePolarDistortion;
  using MagickCore::BarrelDistortion;
  using MagickCore::BarrelInverseDistortion;
  using MagickCore::ShepardsDistortion;
  using MagickCore::SentinelDistortion;

  // Endian options
  using MagickCore::EndianType;
  using MagickCore::UndefinedEndian;
  using MagickCore::LSBEndian;
  using MagickCore::MSBEndian;

  // Evaluate options
  using MagickCore::UndefinedEvaluateOperator;
  using MagickCore::AddEvaluateOperator;
  using MagickCore::AndEvaluateOperator;
  using MagickCore::DivideEvaluateOperator;
  using MagickCore::LeftShiftEvaluateOperator;
  using MagickCore::MaxEvaluateOperator;
  using MagickCore::MinEvaluateOperator;
  using MagickCore::MultiplyEvaluateOperator;
  using MagickCore::OrEvaluateOperator;
  using MagickCore::RightShiftEvaluateOperator;
  using MagickCore::SetEvaluateOperator;
  using MagickCore::SubtractEvaluateOperator;
  using MagickCore::XorEvaluateOperator;
  using MagickCore::MagickEvaluateOperator;

  // Fill rules
  using MagickCore::FillRule;
  using MagickCore::UndefinedRule;
  using MagickCore::EvenOddRule;
  using MagickCore::NonZeroRule;
  
  // Filter types
  using MagickCore::FilterTypes;
  using MagickCore::UndefinedFilter;
  using MagickCore::PointFilter;
  using MagickCore::BoxFilter;
  using MagickCore::TriangleFilter;
  using MagickCore::HermiteFilter;
  using MagickCore::HanningFilter;
  using MagickCore::HammingFilter;
  using MagickCore::BlackmanFilter;
  using MagickCore::GaussianFilter;
  using MagickCore::QuadraticFilter;
  using MagickCore::CubicFilter;
  using MagickCore::CatromFilter;
  using MagickCore::MitchellFilter;
  using MagickCore::JincFilter;
  using MagickCore::SincFilter;
  using MagickCore::SincFastFilter;
  using MagickCore::KaiserFilter;
  using MagickCore::WelshFilter;
  using MagickCore::ParzenFilter;
  using MagickCore::BohmanFilter;
  using MagickCore::BartlettFilter;
  using MagickCore::LagrangeFilter;
  using MagickCore::LanczosFilter;
  using MagickCore::LanczosSharpFilter;
  using MagickCore::Lanczos2Filter;
  using MagickCore::Lanczos2SharpFilter;
  using MagickCore::RobidouxFilter;
  using MagickCore::SentinelFilter;

  // Bit gravity
  using MagickCore::GravityType;
  using MagickCore::ForgetGravity;
  using MagickCore::NorthWestGravity;
  using MagickCore::NorthGravity;
  using MagickCore::NorthEastGravity;
  using MagickCore::WestGravity;
  using MagickCore::CenterGravity;
  using MagickCore::EastGravity;
  using MagickCore::SouthWestGravity;
  using MagickCore::SouthGravity;
  using MagickCore::SouthEastGravity;
  using MagickCore::StaticGravity;

  // Image types
  using MagickCore::ImageType;
  using MagickCore::UndefinedType;
  using MagickCore::BilevelType;
  using MagickCore::GrayscaleType;
  using MagickCore::GrayscaleMatteType;
  using MagickCore::PaletteType;
  using MagickCore::PaletteMatteType;
  using MagickCore::TrueColorType;
  using MagickCore::TrueColorMatteType;
  using MagickCore::ColorSeparationType;
  using MagickCore::ColorSeparationMatteType;
  using MagickCore::OptimizeType;
  
  // Interlace types
  using MagickCore::InterlaceType;
  using MagickCore::UndefinedInterlace;
  using MagickCore::NoInterlace;
  using MagickCore::LineInterlace;
  using MagickCore::PlaneInterlace;
  using MagickCore::PartitionInterlace;

  // Layer method
  using MagickCore::ImageLayerMethod;
  using MagickCore::UndefinedLayer;
  using MagickCore::CoalesceLayer;
  using MagickCore::CompareAnyLayer;
  using MagickCore::CompareClearLayer;
  using MagickCore::CompareOverlayLayer;
  using MagickCore::DisposeLayer;
  using MagickCore::OptimizeLayer;
  using MagickCore::OptimizeImageLayer;
  using MagickCore::OptimizePlusLayer;
  using MagickCore::OptimizeTransLayer;
  using MagickCore::RemoveDupsLayer;
  using MagickCore::RemoveZeroLayer;
  using MagickCore::CompositeLayer;
  using MagickCore::MergeLayer;
  using MagickCore::FlattenLayer;
  using MagickCore::MosaicLayer;
  using MagickCore::TrimBoundsLayer;

  // Line cap types
  using MagickCore::LineCap;
  using MagickCore::UndefinedCap;
  using MagickCore::ButtCap;
  using MagickCore::RoundCap;
  using MagickCore::SquareCap;

  // Line join types
  using MagickCore::LineJoin;
  using MagickCore::UndefinedJoin;
  using MagickCore::MiterJoin;
  using MagickCore::RoundJoin;
  using MagickCore::BevelJoin;

  // Noise types
  using MagickCore::NoiseType;
  using MagickCore::UniformNoise;
  using MagickCore::GaussianNoise;
  using MagickCore::MultiplicativeGaussianNoise;
  using MagickCore::ImpulseNoise;
  using MagickCore::LaplacianNoise;
  using MagickCore::PoissonNoise;

  // Orientation types
  using MagickCore::OrientationType;
  using MagickCore::UndefinedOrientation;
  using MagickCore::TopLeftOrientation;
  using MagickCore::TopRightOrientation;
  using MagickCore::BottomRightOrientation;
  using MagickCore::BottomLeftOrientation;
  using MagickCore::LeftTopOrientation;
  using MagickCore::RightTopOrientation;
  using MagickCore::RightBottomOrientation;
  using MagickCore::LeftBottomOrientation;
  
  // Paint methods
  using MagickCore::PaintMethod;
  using MagickCore::PointMethod;
  using MagickCore::ReplaceMethod;
  using MagickCore::FloodfillMethod;
  using MagickCore::FillToBorderMethod;
  using MagickCore::ResetMethod;

  // Preview types.  Not currently used by Magick++
  using MagickCore::PreviewType;
  using MagickCore::UndefinedPreview;
  using MagickCore::RotatePreview;
  using MagickCore::ShearPreview;
  using MagickCore::RollPreview;
  using MagickCore::HuePreview;
  using MagickCore::SaturationPreview;
  using MagickCore::BrightnessPreview;
  using MagickCore::GammaPreview;
  using MagickCore::SpiffPreview;
  using MagickCore::DullPreview;
  using MagickCore::GrayscalePreview;
  using MagickCore::QuantizePreview;
  using MagickCore::DespecklePreview;
  using MagickCore::ReduceNoisePreview;
  using MagickCore::AddNoisePreview;
  using MagickCore::SharpenPreview;
  using MagickCore::BlurPreview;
  using MagickCore::ThresholdPreview;
  using MagickCore::EdgeDetectPreview;
  using MagickCore::SpreadPreview;
  using MagickCore::SolarizePreview;
  using MagickCore::ShadePreview;
  using MagickCore::RaisePreview;
  using MagickCore::SegmentPreview;
  using MagickCore::SwirlPreview;
  using MagickCore::ImplodePreview;
  using MagickCore::WavePreview;
  using MagickCore::OilPaintPreview;
  using MagickCore::CharcoalDrawingPreview;
  using MagickCore::JPEGPreview;

  // Quantum types
  using MagickCore::QuantumType;
  using MagickCore::IndexQuantum;
  using MagickCore::GrayQuantum;
  using MagickCore::IndexAlphaQuantum;
  using MagickCore::GrayAlphaQuantum;
  using MagickCore::RedQuantum;
  using MagickCore::CyanQuantum;
  using MagickCore::GreenQuantum;
  using MagickCore::YellowQuantum;
  using MagickCore::BlueQuantum;
  using MagickCore::MagentaQuantum;
  using MagickCore::AlphaQuantum;
  using MagickCore::BlackQuantum;
  using MagickCore::RGBQuantum;
  using MagickCore::RGBAQuantum;
  using MagickCore::CMYKQuantum;

  // Rendering intents
  using MagickCore::RenderingIntent;
  using MagickCore::UndefinedIntent;
  using MagickCore::SaturationIntent;
  using MagickCore::PerceptualIntent;
  using MagickCore::AbsoluteIntent;
  using MagickCore::RelativeIntent;
  
  // Resource types
  using MagickCore::MemoryResource;

  // Resolution units
  using MagickCore::ResolutionType;
  using MagickCore::UndefinedResolution;
  using MagickCore::PixelsPerInchResolution;
  using MagickCore::PixelsPerCentimeterResolution;

  // PixelPacket structure
  using MagickCore::PixelPacket;

  // IndexPacket type
  using MagickCore::IndexPacket;

  // Sparse Color methods
  using MagickCore::SparseColorMethod;
  using MagickCore::UndefinedColorInterpolate;
  using MagickCore::BarycentricColorInterpolate;
  using MagickCore::BilinearColorInterpolate;
  using MagickCore::PolynomialColorInterpolate;
  using MagickCore::ShepardsColorInterpolate;
  using MagickCore::VoronoiColorInterpolate;

  // Statistic type
  using MagickCore::MedianStatistic;
  using MagickCore::NonpeakStatistic;

  // StorageType type
  using MagickCore::StorageType;
  using MagickCore::CharPixel;
  using MagickCore::ShortPixel;
  using MagickCore::IntegerPixel;
  using MagickCore::FloatPixel;
  using MagickCore::DoublePixel;

  // StretchType type
  using MagickCore::StretchType;
  using MagickCore::NormalStretch;
  using MagickCore::UltraCondensedStretch;
  using MagickCore::ExtraCondensedStretch;
  using MagickCore::CondensedStretch;
  using MagickCore::SemiCondensedStretch;
  using MagickCore::SemiExpandedStretch;
  using MagickCore::ExpandedStretch;
  using MagickCore::ExtraExpandedStretch;
  using MagickCore::UltraExpandedStretch;
  using MagickCore::AnyStretch;

  // StyleType type
  using MagickCore::StyleType;
  using MagickCore::NormalStyle;
  using MagickCore::ItalicStyle;
  using MagickCore::ObliqueStyle;
  using MagickCore::AnyStyle;

  // Decoration types
  using MagickCore::DecorationType;
  using MagickCore::NoDecoration;
  using MagickCore::UnderlineDecoration;
  using MagickCore::OverlineDecoration;
  using MagickCore::LineThroughDecoration;

  // Virtual pixel methods
  using MagickCore::VirtualPixelMethod;
  using MagickCore::UndefinedVirtualPixelMethod;
  using MagickCore::BackgroundVirtualPixelMethod;
  using MagickCore::DitherVirtualPixelMethod;
  using MagickCore::EdgeVirtualPixelMethod;
  using MagickCore::MirrorVirtualPixelMethod;
  using MagickCore::RandomVirtualPixelMethod;
  using MagickCore::TileVirtualPixelMethod;
  using MagickCore::TransparentVirtualPixelMethod;
  using MagickCore::MaskVirtualPixelMethod;
  using MagickCore::BlackVirtualPixelMethod;
  using MagickCore::GrayVirtualPixelMethod;
  using MagickCore::WhiteVirtualPixelMethod;
  using MagickCore::HorizontalTileVirtualPixelMethod;
  using MagickCore::VerticalTileVirtualPixelMethod;
  using MagickCore::HorizontalTileEdgeVirtualPixelMethod;
  using MagickCore::VerticalTileEdgeVirtualPixelMethod;
  using MagickCore::CheckerTileVirtualPixelMethod;

#if defined(MAGICKCORE_IMPLEMENTATION)
  //
  // ImageMagick symbols used in implementation code
  //
  using MagickCore::AcquireCacheView;
  using MagickCore::AcquireExceptionInfo;
  using MagickCore::GetCacheViewVirtualPixels;
  using MagickCore::AcquireImage;
  using MagickCore::GetVirtualPixels;
  using MagickCore::AcquireKernelInfo;
  using MagickCore::AcquireMagickMemory;
  using MagickCore::AcquireQuantumInfo;
  using MagickCore::AcquireString;
  using MagickCore::AcquireStringInfo;
  using MagickCore::AdaptiveBlurImage;
  using MagickCore::AdaptiveThresholdImage;
  using MagickCore::AddNoiseImage;
  using MagickCore::AddNoiseImageChannel;
  using MagickCore::AffineMatrix;
  using MagickCore::AffineTransformImage;
  using MagickCore::AnnotateImage;
  using MagickCore::AspectValue;
  using MagickCore::Base64Decode;
  using MagickCore::Base64Encode;
  using MagickCore::BilevelImage;
  using MagickCore::BlobError;
  using MagickCore::BlobFatalError;
  using MagickCore::BlobToImage;
  using MagickCore::BlobWarning;
  using MagickCore::BlurImage;
  using MagickCore::BlurImageChannel;
  using MagickCore::BorderImage;
  using MagickCore::CacheError;
  using MagickCore::CacheFatalError;
  using MagickCore::CacheWarning;
  using MagickCore::CharcoalImage;
  using MagickCore::ChopImage;
  using MagickCore::ClearMagickException;
  using MagickCore::CloneDrawInfo;
  using MagickCore::CloneImage;
  using MagickCore::CloneImageInfo;
  using MagickCore::CloneQuantizeInfo;
  using MagickCore::CoderError;
  using MagickCore::CoderFatalError;
  using MagickCore::CoderWarning;
  using MagickCore::ColorDecisionListImage;
  using MagickCore::ColorizeImage;
  using MagickCore::ColorMatrixImage;
  using MagickCore::ColorPacket;
  using MagickCore::CompositeImage;
  using MagickCore::ConfigureError;
  using MagickCore::ConfigureFatalError;
  using MagickCore::ConfigureWarning;
  using MagickCore::ConstituteImage;
  using MagickCore::ContrastImage;
  using MagickCore::ConvertHSLToRGB;
  using MagickCore::ConvertRGBToHSL;
  using MagickCore::ConvolveImage;
  using MagickCore::CopyMagickString;
  using MagickCore::CorruptImageError;
  using MagickCore::CorruptImageFatalError;
  using MagickCore::CorruptImageWarning;
  using MagickCore::CropImage;
  using MagickCore::CycleColormapImage;
  using MagickCore::DeconstructImages;
  using MagickCore::DelegateError;
  using MagickCore::DelegateFatalError;
  using MagickCore::DelegateWarning;
  using MagickCore::DeleteImageOption;
  using MagickCore::DeleteImageRegistry;
  using MagickCore::DespeckleImage;
  using MagickCore::DestroyCacheView;
  using MagickCore::DestroyDrawInfo;
  using MagickCore::DestroyDrawingWand;
  using MagickCore::DestroyExceptionInfo;
  using MagickCore::DestroyImageInfo;
  using MagickCore::DestroyImageList;
  using MagickCore::DestroyKernelInfo;
  using MagickCore::DestroyMagickWand;
  using MagickCore::DestroyPixelWand;
  using MagickCore::DestroyQuantizeInfo;
  using MagickCore::DestroyQuantumInfo;
  using MagickCore::DestroyStringInfo;
  using MagickCore::DisplayImages;
  using MagickCore::DistortImage;
  using MagickCore::DrawAffine;
  using MagickCore::DrawAllocateWand;
  using MagickCore::DrawAnnotation;
  using MagickCore::DrawArc;
  using MagickCore::DrawBezier;
  using MagickCore::DrawCircle;
  using MagickCore::DrawColor;
  using MagickCore::DrawComment;
  using MagickCore::DrawComposite;
  using MagickCore::DrawEllipse;
  using MagickCore::DrawError;
  using MagickCore::DrawFatalError;
  using MagickCore::DrawImage;
  using MagickCore::DrawInfo;
  using MagickCore::DrawingWand;
  using MagickCore::DrawLine;
  using MagickCore::DrawMatte;
  using MagickCore::DrawPathClose;
  using MagickCore::DrawPathCurveToAbsolute;
  using MagickCore::DrawPathCurveToQuadraticBezierAbsolute;
  using MagickCore::DrawPathCurveToQuadraticBezierRelative;
  using MagickCore::DrawPathCurveToQuadraticBezierSmoothAbsolute;
  using MagickCore::DrawPathCurveToQuadraticBezierSmoothRelative;
  using MagickCore::DrawPathCurveToRelative;
  using MagickCore::DrawPathCurveToSmoothAbsolute;
  using MagickCore::DrawPathCurveToSmoothRelative;
  using MagickCore::DrawPathEllipticArcAbsolute;
  using MagickCore::DrawPathEllipticArcRelative;
  using MagickCore::DrawPathFinish;
  using MagickCore::DrawPathLineToAbsolute;
  using MagickCore::DrawPathLineToHorizontalAbsolute;
  using MagickCore::DrawPathLineToHorizontalRelative;
  using MagickCore::DrawPathLineToRelative;
  using MagickCore::DrawPathLineToVerticalAbsolute;
  using MagickCore::DrawPathLineToVerticalRelative;
  using MagickCore::DrawPathMoveToAbsolute;
  using MagickCore::DrawPathMoveToRelative;
  using MagickCore::DrawPathStart;
  using MagickCore::DrawPoint;
  using MagickCore::DrawPolygon;
  using MagickCore::DrawPolyline;
  using MagickCore::DrawPopClipPath;
  using MagickCore::DrawPopDefs;
  using MagickCore::DrawPopPattern;
  using MagickCore::DrawPushClipPath;
  using MagickCore::DrawPushDefs;
  using MagickCore::DrawPushPattern;
  using MagickCore::DrawRectangle;
  using MagickCore::DrawRender;
  using MagickCore::DrawRotate;
  using MagickCore::DrawRoundRectangle;
  using MagickCore::DrawScale;
  using MagickCore::DrawSetClipPath;
  using MagickCore::DrawSetClipRule;
  using MagickCore::DrawSetClipUnits;
  using MagickCore::DrawSetFillColor;
  using MagickCore::DrawSetFillOpacity;
  using MagickCore::DrawSetFillPatternURL;
  using MagickCore::DrawSetFillRule;
  using MagickCore::DrawSetFont;
  using MagickCore::DrawSetFontFamily;
  using MagickCore::DrawSetFontSize;
  using MagickCore::DrawSetFontStretch;
  using MagickCore::DrawSetFontStyle;
  using MagickCore::DrawSetFontWeight;
  using MagickCore::DrawSetGravity;
  using MagickCore::DrawSetStrokeAntialias;
  using MagickCore::DrawSetStrokeColor;
  using MagickCore::DrawSetStrokeDashArray;
  using MagickCore::DrawSetStrokeDashOffset;
  using MagickCore::DrawSetStrokeLineCap;
  using MagickCore::DrawSetStrokeLineJoin;
  using MagickCore::DrawSetStrokeMiterLimit;
  using MagickCore::DrawSetStrokeOpacity;
  using MagickCore::DrawSetStrokePatternURL;
  using MagickCore::DrawSetStrokeWidth;
  using MagickCore::DrawSetTextAntialias;
  using MagickCore::DrawSetTextDecoration;
  using MagickCore::DrawSetTextEncoding;
  using MagickCore::DrawSetTextUnderColor;
  using MagickCore::DrawSetViewbox;
  using MagickCore::DrawSkewX;
  using MagickCore::DrawSkewY;
  using MagickCore::DrawTranslate;
  using MagickCore::DrawWarning;
  using MagickCore::EdgeImage;
  using MagickCore::EmbossImage;
  using MagickCore::EnhanceImage;
  using MagickCore::EqualizeImage;
  using MagickCore::EvaluateImage;
  using MagickCore::EvaluateImageChannel;
  using MagickCore::ExceptionInfo;
  using MagickCore::ExceptionType;
  using MagickCore::ExportImagePixels;
  using MagickCore::ExportQuantumPixels;
  using MagickCore::ExtentImage;
  using MagickCore::FileOpenError;
  using MagickCore::FileOpenFatalError;
  using MagickCore::FileOpenWarning;
  using MagickCore::FlipImage;
  using MagickCore::FloodfillPaintImage;
  using MagickCore::FlopImage;
  using MagickCore::FormatLocaleString;
  using MagickCore::ForwardFourierTransformImage;
  using MagickCore::FrameImage;
  using MagickCore::FrameInfo;
  using MagickCore::FxImageChannel;
  using MagickCore::GammaImage;
  using MagickCore::GammaImage;
  using MagickCore::GaussianBlurImage;
  using MagickCore::GaussianBlurImageChannel;
  using MagickCore::GetAffineMatrix;
  using MagickCore::GetAuthenticIndexQueue;
  using MagickCore::GetBlobSize;
  using MagickCore::GetCacheViewException;
  using MagickCore::GetCacheViewAuthenticIndexQueue;
  using MagickCore::GetCacheViewAuthenticPixels;
  using MagickCore::GetColorTuple;
  using MagickCore::GetDrawInfo;
  using MagickCore::GetExceptionInfo;
  using MagickCore::GetGeometry;
  using MagickCore::GetImageBoundingBox;
  using MagickCore::GetImageChannelDepth;
  using MagickCore::GetImageChannelMean;
  using MagickCore::GetImageChannelKurtosis;
  using MagickCore::GetImageChannelRange;
  using MagickCore::GetImageClipMask;
  using MagickCore::GetImageDepth;
  using MagickCore::GetImageInfo;
  using MagickCore::GetImageInfoFile;
  using MagickCore::GetImageOption;
  using MagickCore::GetAuthenticPixels;
  using MagickCore::GetImageProfile;
  using MagickCore::GetImageProperty;
  using MagickCore::GetImageQuantizeError;
  using MagickCore::GetImageType;
  using MagickCore::GetMagickInfo;
  using MagickCore::GetMagickPixelPacket;
  using MagickCore::GetNumberColors;
  using MagickCore::GetPageGeometry;
  using MagickCore::GetQuantizeInfo;
  using MagickCore::GetStringInfoDatum;
  using MagickCore::GetStringInfoLength;
  using MagickCore::GetTypeMetrics;
  using MagickCore::GetVirtualIndexQueue;
  using MagickCore::GetImageVirtualPixelMethod;
  using MagickCore::GlobExpression;
  using MagickCore::GravityAdjustGeometry;
  using MagickCore::GreaterValue;
  using MagickCore::HaldClutImage;
  using MagickCore::HeightValue;
  using MagickCore::ImageError;
  using MagickCore::ImageFatalError;
  using MagickCore::ImageInfo;
  using MagickCore::ImageRegistryType;
  using MagickCore::ImageToBlob;
  using MagickCore::ImageWarning;
  using MagickCore::ImplodeImage;
  using MagickCore::ImportQuantumPixels;
  using MagickCore::InverseFourierTransformImage;
  using MagickCore::InvokeDynamicImageFilter;
  using MagickCore::IsEventLogging;
  using MagickCore::IsGeometry;
  using MagickCore::IsImagesEqual;
  using MagickCore::KernelInfo;
  using MagickCore::LessValue;
  using MagickCore::LevelImage;
  using MagickCore::LevelImageChannel;
  using MagickCore::LocaleCompare;
  using MagickCore::LogMagickEvent;
  using MagickCore::MagickCoreTerminus;
  using MagickCore::MagickInfo;
  using MagickCore::MagickPixelPacket;
  using MagickCore::MagickToMime;
  using MagickCore::MagickWand;
  using MagickCore::MagnifyImage;
  using MagickCore::MergeImageLayers;
  using MagickCore::MinifyImage;
  using MagickCore::MissingDelegateError;
  using MagickCore::MissingDelegateFatalError;
  using MagickCore::MissingDelegateWarning;
  using MagickCore::ModulateImage;
  using MagickCore::ModuleError;
  using MagickCore::ModuleFatalError;
  using MagickCore::ModuleWarning;
  using MagickCore::MonitorError;
  using MagickCore::MonitorFatalError;
  using MagickCore::MonitorWarning;
  using MagickCore::MontageInfo;
  using MagickCore::MotionBlurImage;
  using MagickCore::NegateImage;
  using MagickCore::NewMagickWandFromImage;
  using MagickCore::NewPixelWand;
  using MagickCore::NoiseType;
  using MagickCore::NormalizeImage;
  using MagickCore::NoValue;
  using MagickCore::OilPaintImage;
  using MagickCore::OpaquePaintImage;
  using MagickCore::OptionError;
  using MagickCore::OptionFatalError;
  using MagickCore::OptionWarning;
  using MagickCore::ParseMetaGeometry;
  using MagickCore::PercentValue;
  using MagickCore::PingBlob;
  using MagickCore::PingImage;
  using MagickCore::PixelSetQuantumColor;
	using MagickCore::PixelWand;
  using MagickCore::PointInfo;
  using MagickCore::PopDrawingWand;
  using MagickCore::ProfileImage;
  using MagickCore::ProfileInfo;
  using MagickCore::PushDrawingWand;
  using MagickCore::QuantizeImage;
  using MagickCore::QuantizeInfo;
  using MagickCore::QuantumInfo;
  using MagickCore::QueryColorDatabase;
  using MagickCore::QueryMagickColor;
  using MagickCore::QueueAuthenticPixels;
  using MagickCore::QueueCacheViewAuthenticPixels;
  using MagickCore::RaiseImage;
  using MagickCore::RandomThresholdImageChannel;
  using MagickCore::ReadImage;
  using MagickCore::RectangleInfo;
  using MagickCore::RegisterMagickInfo;
  using MagickCore::RegistryError;
  using MagickCore::RegistryFatalError;
  using MagickCore::RegistryType;
  using MagickCore::RegistryWarning;
  using MagickCore::RelinquishMagickMemory;
  using MagickCore::RemapImage;
  using MagickCore::ResizeImage;
  using MagickCore::ResizeMagickMemory;
  using MagickCore::ResourceLimitError;
  using MagickCore::ResourceLimitFatalError;
  using MagickCore::ResourceLimitWarning;
  using MagickCore::RollImage;
  using MagickCore::RotateImage;
  using MagickCore::SampleImage;
  using MagickCore::ScaleImage;
  using MagickCore::SegmentImage;
  using MagickCore::SeparateImageChannel;
  using MagickCore::SetClientName;
  using MagickCore::SetGeometry;
  using MagickCore::SetImageBackgroundColor;
  using MagickCore::SetImageChannelDepth;
  using MagickCore::SetImageClipMask;
  using MagickCore::SetImageDepth;
  using MagickCore::SetImageExtent;
  using MagickCore::SetImageInfo;
  using MagickCore::SetImageInfoFile;
  using MagickCore::SetImageOpacity;
  using MagickCore::SetImageOption;
  using MagickCore::SetImageProfile;
  using MagickCore::SetImageProperty;
  using MagickCore::SetImageRegistry;
  using MagickCore::SetImageType;
  using MagickCore::SetLogEventMask;
  using MagickCore::SetMagickInfo;
  using MagickCore::SetMagickResourceLimit;
  using MagickCore::SetStringInfoDatum;
  using MagickCore::SetImageVirtualPixelMethod;
  using MagickCore::ShadeImage;
  using MagickCore::SharpenImage;
  using MagickCore::SharpenImageChannel;
  using MagickCore::ShaveImage;
  using MagickCore::ShearImage;
  using MagickCore::SigmoidalContrastImageChannel;
  using MagickCore::SignatureImage;
  using MagickCore::SolarizeImage;
  using MagickCore::SparseColorImage;
  using MagickCore::SpliceImage;
  using MagickCore::SpreadImage;
  using MagickCore::StatisticImage;
  using MagickCore::SteganoImage;
  using MagickCore::StereoImage;
  using MagickCore::StreamError;
  using MagickCore::StreamFatalError;
  using MagickCore::StreamWarning;
  using MagickCore::StringInfo;
  using MagickCore::StripImage;
  using MagickCore::SwirlImage;
  using MagickCore::SyncCacheViewAuthenticPixels;
  using MagickCore::SyncImage;
  using MagickCore::SyncAuthenticPixels;
  using MagickCore::TextureImage;
  using MagickCore::ThrowException;
  using MagickCore::TransformImage;
  using MagickCore::TransformImageColorspace;
  using MagickCore::TransparentPaintImage;
  using MagickCore::TransparentPaintImageChroma;
  using MagickCore::TrimImage;
  using MagickCore::TypeError;
  using MagickCore::TypeFatalError;
  using MagickCore::TypeWarning;
  using MagickCore::UndefinedException;
  using MagickCore::UndefinedRegistryType;
  using MagickCore::UnregisterMagickInfo;
  using MagickCore::UnsharpMaskImage;
  using MagickCore::UnsharpMaskImageChannel;
  using MagickCore::CacheView;
  using MagickCore::WaveImage;
  using MagickCore::WidthValue;
  using MagickCore::WriteImage;
  using MagickCore::XNegative;
  using MagickCore::XServerError;
  using MagickCore::XServerFatalError;
  using MagickCore::XServerWarning;
  using MagickCore::XValue;
  using MagickCore::YNegative;
  using MagickCore::YValue;

#endif // MAGICKCORE_IMPLEMENTATION

}

#endif // Magick_Include_header

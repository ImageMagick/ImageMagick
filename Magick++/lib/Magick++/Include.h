// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Inclusion of ImageMagick headers (with namespace magic)

#ifndef Magick_Include_header
#define Magick_Include_header

#if !defined(_MAGICK_CONFIG_H)
#  define _MAGICK_CONFIG_H
#  if !defined(vms)
#    include "MagickCore/magick-config.h"
#  else
#    include "magick-config.h"
#  endif
#  undef inline // Remove possible definition from config.h
#  undef class
#endif

// Needed for stdio FILE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>

//
// Include ImageMagick headers into namespace "MagickCore". If
// MAGICKCORE_IMPLEMENTATION is defined, include ImageMagick development
// headers.  This scheme minimizes the possibility of conflict with
// user code.
//
namespace MagickCore
{
#include <MagickCore/MagickCore.h>
#include <MagickWand/MagickWand.h>
#undef inline // Remove possible definition from config.h

#undef class
}

//
// Provide appropriate DLL imports/exports for Visual C++,
// Borland C++Builder and MinGW builds.
//
#if (defined(WIN32) || defined(_WIN32_WINNT)) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#  define MagickCplusPlusDLLSupported
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
#      pragma warning(disable: 4275) /* non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2' */
#      pragma warning(disable: 4251) /* 'type': 'type1' needs to have dll-interface to be used by clients of 'type2' */
#    endif
#    if !defined(MAGICKCORE_IMPLEMENTATION)
#      if defined(__GNUC__)
#        define MagickPPExport __attribute__ ((dllimport))
#      else
#        define MagickPPExport __declspec(dllimport)
#      endif
#      define MagickPPPrivate extern __declspec(dllimport)
#    else
#      if defined(__MINGW32__)
#        define MagickPPExport __declspec(dllexport)
#        define MagickPPPrivate __declspec(dllexport)
#      else
#        if defined(__GNUC__)
#          define MagickPPExport __attribute__ ((dllexport))
#        else
#          define MagickPPExport __declspec(dllexport)
#        endif
#        define MagickPPPrivate extern __declspec(dllexport)
#      endif
#    endif
#  else
#    define MagickPPExport
#    define MagickPPPrivate
#  endif
#else
#  if __GNUC__ >= 4
#    define MagickPPExport __attribute__ ((visibility ("default")))
#    define MagickPPPrivate  __attribute__ ((visibility ("hidden")))
#  else
#    define MagickPPExport
#    define MagickPPPrivate
#  endif
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
  using MagickCore::MagickStatusType;

  // Structures
  using MagickCore::AffineMatrix;
  using MagickCore::CacheView;
  using MagickCore::CCObjectInfo;
  using MagickCore::DrawInfo;
  using MagickCore::DrawingWand;
  using MagickCore::ExceptionInfo;
  using MagickCore::FrameInfo;
  using MagickCore::ImageInfo;
  using MagickCore::KernelInfo;
  using MagickCore::LinkedListInfo;
  using MagickCore::MagickInfo;
  using MagickCore::MagickWand;
  using MagickCore::MontageInfo;
  using MagickCore::OffsetInfo;
  using MagickCore::PixelInfo;
  using MagickCore::PixelWand;
  using MagickCore::PointInfo;
  using MagickCore::ProfileInfo;
  using MagickCore::QuantizeInfo;
  using MagickCore::QuantumInfo;
  using MagickCore::RectangleInfo;
  using MagickCore::StringInfo;

  // Alignment types.
  using MagickCore::AlignType;
  using MagickCore::UndefinedAlign;
  using MagickCore::LeftAlign;
  using MagickCore::CenterAlign;
  using MagickCore::RightAlign;

  // Alpha channel options
  using MagickCore::AlphaChannelOption;
  using MagickCore::UndefinedAlphaChannel;
  using MagickCore::ActivateAlphaChannel;
  using MagickCore::AssociateAlphaChannel;
  using MagickCore::BackgroundAlphaChannel;
  using MagickCore::CopyAlphaChannel;
  using MagickCore::DeactivateAlphaChannel;
  using MagickCore::DiscreteAlphaChannel;
  using MagickCore::DisassociateAlphaChannel;
  using MagickCore::ExtractAlphaChannel;
  using MagickCore::OffAlphaChannel;
  using MagickCore::OnAlphaChannel;
  using MagickCore::OpaqueAlphaChannel;
  using MagickCore::RemoveAlphaChannel;
  using MagickCore::SetAlphaChannel;
  using MagickCore::ShapeAlphaChannel;
  using MagickCore::TransparentAlphaChannel;

  // Auto threshold methods
  using MagickCore::AutoThresholdMethod;
  using MagickCore::UndefinedThresholdMethod;
  using MagickCore::KapurThresholdMethod;
  using MagickCore::OTSUThresholdMethod;
  using MagickCore::TriangleThresholdMethod;

  // Channel types
  using MagickCore::ChannelType;
  using MagickCore::UndefinedChannel;
  using MagickCore::RedChannel;
  using MagickCore::GrayChannel;
  using MagickCore::CyanChannel;
  using MagickCore::GreenChannel;
  using MagickCore::MagentaChannel;
  using MagickCore::BlueChannel;
  using MagickCore::YellowChannel;
  using MagickCore::BlackChannel;
  using MagickCore::AlphaChannel;
  using MagickCore::OpacityChannel;
  using MagickCore::IndexChannel;
  using MagickCore::ReadMaskChannel;
  using MagickCore::WriteMaskChannel;
  using MagickCore::MetaChannel;
  using MagickCore::CompositeChannels;
  using MagickCore::AllChannels;
  using MagickCore::TrueAlphaChannel;
  using MagickCore::RGBChannels;
  using MagickCore::GrayChannels;
  using MagickCore::SyncChannels;
  using MagickCore::DefaultChannels;

  // Image class types
  using MagickCore::ClassType;
  using MagickCore::UndefinedClass;
  using MagickCore::DirectClass;
  using MagickCore::PseudoClass;

  // Clip path units
  using MagickCore::ClipPathUnits;
  using MagickCore::UndefinedPathUnits;
  using MagickCore::UserSpace;
  using MagickCore::UserSpaceOnUse;
  using MagickCore::ObjectBoundingBox;

  // Color-space types
  using MagickCore::ColorspaceType;
  using MagickCore::UndefinedColorspace;
  using MagickCore::CMYColorspace;
  using MagickCore::CMYKColorspace;
  using MagickCore::GRAYColorspace;
  using MagickCore::HCLColorspace;
  using MagickCore::HCLpColorspace;
  using MagickCore::HSBColorspace;
  using MagickCore::HSIColorspace;
  using MagickCore::HSLColorspace;
  using MagickCore::HSVColorspace;
  using MagickCore::HWBColorspace;
  using MagickCore::LabColorspace;
  using MagickCore::LCHColorspace;
  using MagickCore::LCHabColorspace;
  using MagickCore::LCHuvColorspace;
  using MagickCore::LogColorspace;
  using MagickCore::LMSColorspace;
  using MagickCore::LuvColorspace;
  using MagickCore::OHTAColorspace;
  using MagickCore::Rec601YCbCrColorspace;
  using MagickCore::Rec709YCbCrColorspace;
  using MagickCore::RGBColorspace;
  using MagickCore::scRGBColorspace;
  using MagickCore::sRGBColorspace;
  using MagickCore::TransparentColorspace;
  using MagickCore::xyYColorspace;
  using MagickCore::XYZColorspace;
  using MagickCore::YCbCrColorspace;
  using MagickCore::YCCColorspace;
  using MagickCore::YDbDrColorspace;
  using MagickCore::YIQColorspace;
  using MagickCore::YPbPrColorspace;
  using MagickCore::YUVColorspace;
  using MagickCore::LinearGRAYColorspace;

  // Command options
  using MagickCore::CommandOption;
  using MagickCore::MagickDirectionOptions;
  using MagickCore::MagickGravityOptions;
  using MagickCore::MagickKernelOptions;
  using MagickCore::MagickStyleOptions;

  // Compliance types
  using MagickCore::ComplianceType;
  using MagickCore::AllCompliance;

  // Composition operations
  using MagickCore::CompositeOperator;
  using MagickCore::AlphaCompositeOp;
  using MagickCore::AtopCompositeOp;
  using MagickCore::BlendCompositeOp;
  using MagickCore::BlurCompositeOp;
  using MagickCore::BumpmapCompositeOp;
  using MagickCore::ChangeMaskCompositeOp;
  using MagickCore::ClearCompositeOp;
  using MagickCore::ColorBurnCompositeOp;
  using MagickCore::ColorDodgeCompositeOp;
  using MagickCore::ColorizeCompositeOp;
  using MagickCore::CopyBlackCompositeOp;
  using MagickCore::CopyBlueCompositeOp;
  using MagickCore::CopyCompositeOp;
  using MagickCore::CopyCyanCompositeOp;
  using MagickCore::CopyGreenCompositeOp;
  using MagickCore::CopyMagentaCompositeOp;
  using MagickCore::CopyAlphaCompositeOp;
  using MagickCore::CopyRedCompositeOp;
  using MagickCore::CopyYellowCompositeOp;
  using MagickCore::DarkenCompositeOp;
  using MagickCore::DarkenIntensityCompositeOp;
  using MagickCore::DifferenceCompositeOp;
  using MagickCore::DisplaceCompositeOp;
  using MagickCore::DissolveCompositeOp;
  using MagickCore::DistortCompositeOp;
  using MagickCore::DivideDstCompositeOp;
  using MagickCore::DivideSrcCompositeOp;
  using MagickCore::DstAtopCompositeOp;
  using MagickCore::DstCompositeOp;
  using MagickCore::DstInCompositeOp;
  using MagickCore::DstOutCompositeOp;
  using MagickCore::DstOverCompositeOp;
  using MagickCore::ExclusionCompositeOp;
  using MagickCore::HardLightCompositeOp;
  using MagickCore::HardMixCompositeOp;
  using MagickCore::HueCompositeOp;
  using MagickCore::InCompositeOp;
  using MagickCore::IntensityCompositeOp;
  using MagickCore::LightenCompositeOp;
  using MagickCore::LightenIntensityCompositeOp;
  using MagickCore::LinearBurnCompositeOp;
  using MagickCore::LinearDodgeCompositeOp;
  using MagickCore::LinearLightCompositeOp;
  using MagickCore::LuminizeCompositeOp;
  using MagickCore::MathematicsCompositeOp;
  using MagickCore::MinusDstCompositeOp;
  using MagickCore::MinusSrcCompositeOp;
  using MagickCore::ModulateCompositeOp;
  using MagickCore::ModulusAddCompositeOp;
  using MagickCore::ModulusSubtractCompositeOp;
  using MagickCore::MultiplyCompositeOp;
  using MagickCore::NoCompositeOp;
  using MagickCore::OutCompositeOp;
  using MagickCore::OverCompositeOp;
  using MagickCore::OverlayCompositeOp;
  using MagickCore::PegtopLightCompositeOp;
  using MagickCore::PinLightCompositeOp;
  using MagickCore::PlusCompositeOp;
  using MagickCore::ReplaceCompositeOp;
  using MagickCore::SaturateCompositeOp;
  using MagickCore::ScreenCompositeOp;
  using MagickCore::SoftLightCompositeOp;
  using MagickCore::SrcAtopCompositeOp;
  using MagickCore::SrcCompositeOp;
  using MagickCore::SrcInCompositeOp;
  using MagickCore::SrcOutCompositeOp;
  using MagickCore::SrcOverCompositeOp;
  using MagickCore::ThresholdCompositeOp;
  using MagickCore::UndefinedCompositeOp;
  using MagickCore::VividLightCompositeOp;
  using MagickCore::XorCompositeOp;

  // Compression algorithms
  using MagickCore::CompressionType;
  using MagickCore::UndefinedCompression;
  using MagickCore::NoCompression;
  using MagickCore::B44ACompression;
  using MagickCore::B44Compression;
  using MagickCore::BZipCompression;
  using MagickCore::DWAACompression;
  using MagickCore::DWABCompression;
  using MagickCore::DXT1Compression;
  using MagickCore::DXT3Compression;
  using MagickCore::DXT5Compression;
  using MagickCore::BC7Compression;
  using MagickCore::FaxCompression;
  using MagickCore::Group4Compression;
  using MagickCore::JBIG1Compression;
  using MagickCore::JBIG2Compression;
  using MagickCore::JPEG2000Compression;
  using MagickCore::JPEGCompression;
  using MagickCore::LosslessJPEGCompression;
  using MagickCore::LZMACompression;
  using MagickCore::LZWCompression;
  using MagickCore::PizCompression;
  using MagickCore::Pxr24Compression;
  using MagickCore::RLECompression;
  using MagickCore::WebPCompression;
  using MagickCore::ZipCompression;
  using MagickCore::ZipSCompression;
  using MagickCore::ZstdCompression;

  // Decoration types
  using MagickCore::DecorationType;
  using MagickCore::UndefinedDecoration;
  using MagickCore::NoDecoration;
  using MagickCore::UnderlineDecoration;
  using MagickCore::OverlineDecoration;
  using MagickCore::LineThroughDecoration;

  // Direction types
  using MagickCore::DirectionType;
  using MagickCore::UndefinedDirection;
  using MagickCore::RightToLeftDirection;
  using MagickCore::LeftToRightDirection;

  // Dispose methods
  using MagickCore::DisposeType;
  using MagickCore::UndefinedDispose;
  using MagickCore::NoneDispose;
  using MagickCore::BackgroundDispose;
  using MagickCore::PreviousDispose;

  // Distort methods
  using MagickCore::DistortMethod;
  using MagickCore::UndefinedDistortion;
  using MagickCore::AffineDistortion;
  using MagickCore::AffineProjectionDistortion;
  using MagickCore::ScaleRotateTranslateDistortion;
  using MagickCore::PerspectiveDistortion;
  using MagickCore::PerspectiveProjectionDistortion;
  using MagickCore::BilinearForwardDistortion;
  using MagickCore::BilinearDistortion;
  using MagickCore::BilinearReverseDistortion;
  using MagickCore::PolynomialDistortion;
  using MagickCore::ArcDistortion;
  using MagickCore::PolarDistortion;
  using MagickCore::DePolarDistortion;
  using MagickCore::Cylinder2PlaneDistortion;
  using MagickCore::Plane2CylinderDistortion;
  using MagickCore::BarrelDistortion;
  using MagickCore::BarrelInverseDistortion;
  using MagickCore::ShepardsDistortion;
  using MagickCore::ResizeDistortion;
  using MagickCore::SentinelDistortion;

  // Dither methods
  using MagickCore::DitherMethod;
  using MagickCore::UndefinedDitherMethod;
  using MagickCore::NoDitherMethod;
  using MagickCore::RiemersmaDitherMethod;
  using MagickCore::FloydSteinbergDitherMethod;

  // Endian options
  using MagickCore::EndianType;
  using MagickCore::UndefinedEndian;
  using MagickCore::LSBEndian;
  using MagickCore::MSBEndian;

  // Boolean types
  using MagickCore::MagickBooleanType;
  using MagickCore::MagickFalse;
  using MagickCore::MagickTrue;

  // Evaluate options
  using MagickCore::MagickEvaluateOperator;
  using MagickCore::UndefinedEvaluateOperator;
  using MagickCore::AbsEvaluateOperator;
  using MagickCore::AddEvaluateOperator;
  using MagickCore::AddModulusEvaluateOperator;
  using MagickCore::AndEvaluateOperator;
  using MagickCore::CosineEvaluateOperator;
  using MagickCore::DivideEvaluateOperator;
  using MagickCore::ExponentialEvaluateOperator;
  using MagickCore::GaussianNoiseEvaluateOperator;
  using MagickCore::ImpulseNoiseEvaluateOperator;
  using MagickCore::LaplacianNoiseEvaluateOperator;
  using MagickCore::LeftShiftEvaluateOperator;
  using MagickCore::LogEvaluateOperator;
  using MagickCore::MaxEvaluateOperator;
  using MagickCore::MeanEvaluateOperator;
  using MagickCore::MedianEvaluateOperator;
  using MagickCore::MinEvaluateOperator;
  using MagickCore::MultiplicativeNoiseEvaluateOperator;
  using MagickCore::MultiplyEvaluateOperator;
  using MagickCore::OrEvaluateOperator;
  using MagickCore::PoissonNoiseEvaluateOperator;
  using MagickCore::PowEvaluateOperator;
  using MagickCore::RootMeanSquareEvaluateOperator;
  using MagickCore::RightShiftEvaluateOperator;
  using MagickCore::SetEvaluateOperator;
  using MagickCore::SineEvaluateOperator;
  using MagickCore::SubtractEvaluateOperator;
  using MagickCore::SumEvaluateOperator;
  using MagickCore::ThresholdBlackEvaluateOperator;
  using MagickCore::ThresholdEvaluateOperator;
  using MagickCore::ThresholdWhiteEvaluateOperator;
  using MagickCore::UniformNoiseEvaluateOperator;
  using MagickCore::XorEvaluateOperator;

  // Fill rules
  using MagickCore::FillRule;
  using MagickCore::UndefinedRule;
  using MagickCore::EvenOddRule;
  using MagickCore::NonZeroRule;

  // Filter types
  using MagickCore::FilterType;
  using MagickCore::UndefinedFilter;
  using MagickCore::PointFilter;
  using MagickCore::BoxFilter;
  using MagickCore::TriangleFilter;
  using MagickCore::HermiteFilter;
  using MagickCore::HannFilter;
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
  using MagickCore::WelchFilter;
  using MagickCore::ParzenFilter;
  using MagickCore::BohmanFilter;
  using MagickCore::BartlettFilter;
  using MagickCore::LagrangeFilter;
  using MagickCore::LanczosFilter;
  using MagickCore::LanczosSharpFilter;
  using MagickCore::Lanczos2Filter;
  using MagickCore::Lanczos2SharpFilter;
  using MagickCore::RobidouxFilter;
  using MagickCore::RobidouxSharpFilter;
  using MagickCore::CosineFilter;
  using MagickCore::SplineFilter;
  using MagickCore::LanczosRadiusFilter;
  using MagickCore::SentinelFilter;

  // Geometry flags;
  using MagickCore::GeometryFlags;
  using MagickCore::AreaValue;
  using MagickCore::AspectValue;
  using MagickCore::GreaterValue;
  using MagickCore::HeightValue;
  using MagickCore::LessValue;
  using MagickCore::MinimumValue;
  using MagickCore::NoValue;
  using MagickCore::PercentValue;
  using MagickCore::WidthValue;
  using MagickCore::XNegative;
  using MagickCore::XValue;
  using MagickCore::YNegative;
  using MagickCore::YValue;

  // Bit gravity
  using MagickCore::GravityType;
  using MagickCore::UndefinedGravity;
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

  // Image types
  using MagickCore::ImageType;
  using MagickCore::UndefinedType;
  using MagickCore::BilevelType;
  using MagickCore::GrayscaleType;
  using MagickCore::GrayscaleAlphaType;
  using MagickCore::PaletteType;
  using MagickCore::PaletteAlphaType;
  using MagickCore::TrueColorType;
  using MagickCore::TrueColorAlphaType;
  using MagickCore::ColorSeparationType;
  using MagickCore::ColorSeparationAlphaType;
  using MagickCore::OptimizeType;
  using MagickCore::PaletteBilevelAlphaType;

  // Interlace types
  using MagickCore::InterlaceType;
  using MagickCore::UndefinedInterlace;
  using MagickCore::NoInterlace;
  using MagickCore::LineInterlace;
  using MagickCore::PlaneInterlace;
  using MagickCore::PartitionInterlace;
  using MagickCore::GIFInterlace;
  using MagickCore::JPEGInterlace;
  using MagickCore::PNGInterlace;

  // Built-in kernels
  using MagickCore::KernelInfoType;
  using MagickCore::UndefinedKernel;
  using MagickCore::UnityKernel;
  using MagickCore::GaussianKernel;
  using MagickCore::DoGKernel;
  using MagickCore::LoGKernel;
  using MagickCore::BlurKernel;
  using MagickCore::CometKernel;
  using MagickCore::BinomialKernel;
  using MagickCore::LaplacianKernel;
  using MagickCore::SobelKernel;
  using MagickCore::FreiChenKernel;
  using MagickCore::RobertsKernel;
  using MagickCore::PrewittKernel;
  using MagickCore::CompassKernel;
  using MagickCore::KirschKernel;
  using MagickCore::DiamondKernel;
  using MagickCore::SquareKernel;
  using MagickCore::RectangleKernel;
  using MagickCore::OctagonKernel;
  using MagickCore::DiskKernel;
  using MagickCore::PlusKernel;
  using MagickCore::CrossKernel;
  using MagickCore::RingKernel;
  using MagickCore::PeaksKernel;
  using MagickCore::EdgesKernel;
  using MagickCore::CornersKernel;
  using MagickCore::DiagonalsKernel;
  using MagickCore::LineEndsKernel;
  using MagickCore::LineJunctionsKernel;
  using MagickCore::RidgesKernel;
  using MagickCore::ConvexHullKernel;
  using MagickCore::ThinSEKernel;
  using MagickCore::SkeletonKernel;
  using MagickCore::ChebyshevKernel;
  using MagickCore::ManhattanKernel;
  using MagickCore::OctagonalKernel;
  using MagickCore::EuclideanKernel;
  using MagickCore::UserDefinedKernel;

  // Layer method
  using MagickCore::LayerMethod;
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

  // Log event types
  using MagickCore::LogEventType;
  using MagickCore::UndefinedEvents;
  using MagickCore::NoEvents;
  using MagickCore::AccelerateEvent;
  using MagickCore::AnnotateEvent;
  using MagickCore::BlobEvent;
  using MagickCore::CacheEvent;
  using MagickCore::CoderEvent;
  using MagickCore::ConfigureEvent;
  using MagickCore::DeprecateEvent;
  using MagickCore::DrawEvent;
  using MagickCore::ExceptionEvent;
  using MagickCore::ImageEvent;
  using MagickCore::LocaleEvent;
  using MagickCore::ModuleEvent;
  using MagickCore::PixelEvent;
  using MagickCore::PolicyEvent;
  using MagickCore::ResourceEvent;
  using MagickCore::TraceEvent;
  using MagickCore::TransformEvent;
  using MagickCore::UserEvent;
  using MagickCore::WandEvent;
  using MagickCore::X11Event;
  using MagickCore::CommandEvent;
  using MagickCore::AllEvents;

  // Magick functions
  using MagickCore::MagickFunction;
  using MagickCore::UndefinedFunction;
  using MagickCore::ArcsinFunction;
  using MagickCore::ArctanFunction;
  using MagickCore::PolynomialFunction;
  using MagickCore::SinusoidFunction;

  // Metric types
  using MagickCore::MetricType;
  using MagickCore::UndefinedErrorMetric;
  using MagickCore::AbsoluteErrorMetric;
  using MagickCore::FuzzErrorMetric;
  using MagickCore::MeanAbsoluteErrorMetric;
  using MagickCore::MeanErrorPerPixelErrorMetric;
  using MagickCore::MeanSquaredErrorMetric;
  using MagickCore::NormalizedCrossCorrelationErrorMetric;
  using MagickCore::PeakAbsoluteErrorMetric;
  using MagickCore::PeakSignalToNoiseRatioErrorMetric;
  using MagickCore::PerceptualHashErrorMetric;
  using MagickCore::RootMeanSquaredErrorMetric;

  // Morphology methods
  using MagickCore::MorphologyMethod;
  using MagickCore::UndefinedMorphology;
  using MagickCore::ConvolveMorphology;
  using MagickCore::CorrelateMorphology;
  using MagickCore::ErodeMorphology;
  using MagickCore::DilateMorphology;
  using MagickCore::ErodeIntensityMorphology;
  using MagickCore::DilateIntensityMorphology;
  using MagickCore::IterativeDistanceMorphology;
  using MagickCore::OpenMorphology;
  using MagickCore::CloseMorphology;
  using MagickCore::OpenIntensityMorphology;
  using MagickCore::CloseIntensityMorphology;
  using MagickCore::SmoothMorphology;
  using MagickCore::EdgeInMorphology;
  using MagickCore::EdgeOutMorphology;
  using MagickCore::EdgeMorphology;
  using MagickCore::TopHatMorphology;
  using MagickCore::BottomHatMorphology;
  using MagickCore::HitAndMissMorphology;
  using MagickCore::ThinningMorphology;
  using MagickCore::ThickenMorphology;
  using MagickCore::DistanceMorphology;
  using MagickCore::VoronoiMorphology;

  // Noise types
  using MagickCore::NoiseType;
  using MagickCore::UndefinedNoise;
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
  using MagickCore::UndefinedMethod;
  using MagickCore::PointMethod;
  using MagickCore::ReplaceMethod;
  using MagickCore::FloodfillMethod;
  using MagickCore::FillToBorderMethod;
  using MagickCore::ResetMethod;

  using MagickCore::PixelChannel;
  using MagickCore::UndefinedPixelChannel;
  using MagickCore::RedPixelChannel;
  using MagickCore::CyanPixelChannel;
  using MagickCore::GrayPixelChannel;
  using MagickCore::LPixelChannel;
  using MagickCore::YPixelChannel;
  using MagickCore::aPixelChannel;
  using MagickCore::GreenPixelChannel;
  using MagickCore::MagentaPixelChannel;
  using MagickCore::CbPixelChannel;
  using MagickCore::bPixelChannel;
  using MagickCore::BluePixelChannel;
  using MagickCore::YellowPixelChannel;
  using MagickCore::CrPixelChannel;
  using MagickCore::BlackPixelChannel;
  using MagickCore::AlphaPixelChannel;
  using MagickCore::IndexPixelChannel;
  using MagickCore::ReadMaskPixelChannel;
  using MagickCore::WriteMaskPixelChannel;
  using MagickCore::MetaPixelChannel;
  using MagickCore::IntensityPixelChannel;
  using MagickCore::CompositePixelChannel;
  using MagickCore::SyncPixelChannel;

  // Pixel intensity method
  using MagickCore::PixelIntensityMethod;
  using MagickCore::UndefinedPixelIntensityMethod;
  using MagickCore::AveragePixelIntensityMethod;
  using MagickCore::BrightnessPixelIntensityMethod;
  using MagickCore::LightnessPixelIntensityMethod;
  using MagickCore::MSPixelIntensityMethod;
  using MagickCore::Rec601LumaPixelIntensityMethod;
  using MagickCore::Rec601LuminancePixelIntensityMethod;
  using MagickCore::Rec709LumaPixelIntensityMethod;
  using MagickCore::Rec709LuminancePixelIntensityMethod;
  using MagickCore::RMSPixelIntensityMethod;

  // PixelInterpolate methods
  using MagickCore::PixelInterpolateMethod;
  using MagickCore::UndefinedInterpolatePixel;
  using MagickCore::AverageInterpolatePixel;
  using MagickCore::Average9InterpolatePixel;
  using MagickCore::Average16InterpolatePixel;
  using MagickCore::BackgroundInterpolatePixel;
  using MagickCore::BilinearInterpolatePixel;
  using MagickCore::BlendInterpolatePixel;
  using MagickCore::CatromInterpolatePixel;
  using MagickCore::IntegerInterpolatePixel;
  using MagickCore::MeshInterpolatePixel;
  using MagickCore::NearestInterpolatePixel;
  using MagickCore::SplineInterpolatePixel;

  // Pixel traits
  using MagickCore::PixelTrait;
  using MagickCore::UndefinedPixelTrait;
  using MagickCore::CopyPixelTrait;
  using MagickCore::UpdatePixelTrait;
  using MagickCore::BlendPixelTrait;

  // Policy domains
  using MagickCore::PolicyDomain;
  using MagickCore::UndefinedPolicyDomain;
  using MagickCore::CoderPolicyDomain;
  using MagickCore::DelegatePolicyDomain;
  using MagickCore::FilterPolicyDomain;
  using MagickCore::PathPolicyDomain;
  using MagickCore::ResourcePolicyDomain;
  using MagickCore::SystemPolicyDomain;
  using MagickCore::CachePolicyDomain;

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

  // Pixel mask types
  using MagickCore::PixelMask;
  using MagickCore::UndefinedPixelMask;
  using MagickCore::ReadPixelMask;
  using MagickCore::WritePixelMask;

  // Rendering intents
  using MagickCore::RenderingIntent;
  using MagickCore::UndefinedIntent;
  using MagickCore::SaturationIntent;
  using MagickCore::PerceptualIntent;
  using MagickCore::AbsoluteIntent;
  using MagickCore::RelativeIntent;

  // Resource types
  using MagickCore::ResourceType;
  using MagickCore::UndefinedResource;
  using MagickCore::AreaResource;
  using MagickCore::DiskResource;
  using MagickCore::FileResource;
  using MagickCore::HeightResource;
  using MagickCore::MapResource;
  using MagickCore::MemoryResource;
  using MagickCore::ThreadResource;
  using MagickCore::ThrottleResource;
  using MagickCore::TimeResource;
  using MagickCore::WidthResource;
  using MagickCore::ListLengthResource;

  // Resolution units
  using MagickCore::ResolutionType;
  using MagickCore::UndefinedResolution;
  using MagickCore::PixelsPerInchResolution;
  using MagickCore::PixelsPerCentimeterResolution;

  // Sparse Color methods
  using MagickCore::SparseColorMethod;
  using MagickCore::UndefinedColorInterpolate;
  using MagickCore::BarycentricColorInterpolate;
  using MagickCore::BilinearColorInterpolate;
  using MagickCore::PolynomialColorInterpolate;
  using MagickCore::ShepardsColorInterpolate;
  using MagickCore::VoronoiColorInterpolate;
  using MagickCore::InverseColorInterpolate;
  using MagickCore::ManhattanColorInterpolate;

  // Statistic type
  using MagickCore::StatisticType;
  using MagickCore::UndefinedStatistic;
  using MagickCore::GradientStatistic;
  using MagickCore::MaximumStatistic;
  using MagickCore::MeanStatistic;
  using MagickCore::MedianStatistic;
  using MagickCore::MinimumStatistic;
  using MagickCore::ModeStatistic;
  using MagickCore::NonpeakStatistic;
  using MagickCore::RootMeanSquareStatistic;
  using MagickCore::StandardDeviationStatistic;

  // StorageType type
  using MagickCore::StorageType;
  using MagickCore::UndefinedPixel;
  using MagickCore::CharPixel;
  using MagickCore::DoublePixel;
  using MagickCore::FloatPixel;
  using MagickCore::LongPixel;
  using MagickCore::LongLongPixel;
  using MagickCore::QuantumPixel;
  using MagickCore::ShortPixel;

  // StretchType type
  using MagickCore::StretchType;
  using MagickCore::UndefinedStretch;
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
  using MagickCore::UndefinedStyle;
  using MagickCore::NormalStyle;
  using MagickCore::ItalicStyle;
  using MagickCore::ObliqueStyle;
  using MagickCore::AnyStyle;

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
  using MagickCore::AcquireAlignedMemory;
  using MagickCore::AcquireAuthenticCacheView;
  using MagickCore::AcquireDrawingWand;
  using MagickCore::AcquireExceptionInfo;
  using MagickCore::AcquireVirtualCacheView;
  using MagickCore::AcquireImage;
  using MagickCore::AcquireKernelInfo;
  using MagickCore::AcquireMagickInfo;
  using MagickCore::AcquireMagickMemory;
  using MagickCore::AcquireQuantumInfo;
  using MagickCore::AcquireString;
  using MagickCore::AcquireStringInfo;
  using MagickCore::AdaptiveBlurImage;
  using MagickCore::AdaptiveResizeImage;
  using MagickCore::AdaptiveSharpenImage;
  using MagickCore::AdaptiveThresholdImage;
  using MagickCore::AddNoiseImage;
  using MagickCore::AffineTransformImage;
  using MagickCore::AnnotateImage;
  using MagickCore::AutoGammaImage;
  using MagickCore::AutoLevelImage;
  using MagickCore::AutoOrientImage;
  using MagickCore::AutoThresholdImage;
  using MagickCore::Base64Decode;
  using MagickCore::Base64Encode;
  using MagickCore::BilevelImage;
  using MagickCore::BlackThresholdImage;
  using MagickCore::BlobToImage;
  using MagickCore::BlueShiftImage;
  using MagickCore::BlurImage;
  using MagickCore::BrightnessContrastImage;
  using MagickCore::BorderImage;
  using MagickCore::CharcoalImage;
  using MagickCore::CannyEdgeImage;
  using MagickCore::ChopImage;
  using MagickCore::ClampImage;
  using MagickCore::ClampToQuantum;
  using MagickCore::ClearMagickException;
  using MagickCore::CloneDrawInfo;
  using MagickCore::CloneImage;
  using MagickCore::CloneImageInfo;
  using MagickCore::CloneQuantizeInfo;
  using MagickCore::ClutImage;
  using MagickCore::ColorDecisionListImage;
  using MagickCore::ColorizeImage;
  using MagickCore::ColorMatrixImage;
  using MagickCore::CommandOptionToMnemonic;
  using MagickCore::CompareImages;
  using MagickCore::CompareImagesLayers;
  using MagickCore::CompositeImage;
  using MagickCore::ConnectedComponentsImage;
  using MagickCore::ConstituteImage;
  using MagickCore::ContrastImage;
  using MagickCore::ContrastStretchImage;
  using MagickCore::ConvertHSLToRGB;
  using MagickCore::ConvertRGBToHSL;
  using MagickCore::ConvolveImage;
  using MagickCore::CopyImagePixels;
  using MagickCore::CopyMagickString;
  using MagickCore::CropImage;
  using MagickCore::CropImageToTiles;
  using MagickCore::CycleColormapImage;
  using MagickCore::DecipherImage;
  using MagickCore::DeleteImageOption;
  using MagickCore::DeleteImageRegistry;
  using MagickCore::DeskewImage;
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
  using MagickCore::DestroyString;
  using MagickCore::DestroyStringInfo;
  using MagickCore::DisplayImages;
  using MagickCore::DistortImage;
  using MagickCore::DrawAffine;
  using MagickCore::DrawAlpha;
  using MagickCore::DrawAnnotation;
  using MagickCore::DrawArc;
  using MagickCore::DrawBezier;
  using MagickCore::DrawCircle;
  using MagickCore::DrawColor;
  using MagickCore::DrawComment;
  using MagickCore::DrawComposite;
  using MagickCore::DrawEllipse;
  using MagickCore::DrawImage;
  using MagickCore::DrawLine;
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
  using MagickCore::DrawSetTextInterlineSpacing;
  using MagickCore::DrawSetTextInterwordSpacing;
  using MagickCore::DrawSetTextKerning;
  using MagickCore::DrawSetTextUnderColor;
  using MagickCore::DrawSetViewbox;
  using MagickCore::DrawSkewX;
  using MagickCore::DrawSkewY;
  using MagickCore::DrawTranslate;
  using MagickCore::EdgeImage;
  using MagickCore::EmbossImage;
  using MagickCore::EncipherImage;
  using MagickCore::EnhanceImage;
  using MagickCore::EqualizeImage;
  using MagickCore::EvaluateImage;
  using MagickCore::ExportImagePixels;
  using MagickCore::ExportQuantumPixels;
  using MagickCore::ExtentImage;
  using MagickCore::FlipImage;
  using MagickCore::FloodfillPaintImage;
  using MagickCore::FlopImage;
  using MagickCore::FormatLocaleString;
  using MagickCore::ForwardFourierTransformImage;
  using MagickCore::FrameImage;
  using MagickCore::FxImage;
  using MagickCore::GammaImage;
  using MagickCore::GaussianBlurImage;
  using MagickCore::GaussianBlurImage;
  using MagickCore::GetAffineMatrix;
  using MagickCore::GetAuthenticMetacontent;
  using MagickCore::GetBlobSize;
  using MagickCore::GetCacheViewAuthenticMetacontent;
  using MagickCore::GetCacheViewAuthenticPixels;
  using MagickCore::GetCacheViewVirtualPixels;
  using MagickCore::GetClientName;
  using MagickCore::GetColorTuple;
  using MagickCore::GetDrawInfo;
  using MagickCore::GetGeometry;
  using MagickCore::GetImageArtifact;
  using MagickCore::GetImageMoments;
  using MagickCore::GetImageBoundingBox;
  using MagickCore::GetImageDistortion;
  using MagickCore::GetImageDepth;
  using MagickCore::GetImageMask;
  using MagickCore::GetImageMean;
  using MagickCore::GetImageKurtosis;
  using MagickCore::GetImageRange;
  using MagickCore::GetImageDepth;
  using MagickCore::GetImageInfo;
  using MagickCore::GetImageInfoFile;
  using MagickCore::GetImageOption;
  using MagickCore::GetImagePerceptualHash;
  using MagickCore::GetAuthenticPixels;
  using MagickCore::GetImageProfile;
  using MagickCore::GetImageProperty;
  using MagickCore::GetImageQuantizeError;
  using MagickCore::GetImageType;
  using MagickCore::GetMagickDecoderThreadSupport;
  using MagickCore::GetMagickEncoderThreadSupport;
  using MagickCore::GetMagickInfo;
  using MagickCore::GetMultilineTypeMetrics;
  using MagickCore::GetNextValueInLinkedList;
  using MagickCore::GetNumberOfElementsInLinkedList;
  using MagickCore::GetPixelBlue;
  using MagickCore::GetPixelChannelOffset;
  using MagickCore::GetPixelChannelTraits;
  using MagickCore::GetPixelGreen;
  using MagickCore::GetPixelInfo;
  using MagickCore::GetPixelRed;
  using MagickCore::GetNumberColors;
  using MagickCore::GetPageGeometry;
  using MagickCore::GetQuantizeInfo;
  using MagickCore::GetStringInfoDatum;
  using MagickCore::GetStringInfoLength;
  using MagickCore::GetTypeMetrics;
  using MagickCore::GetValueFromLinkedList;
  using MagickCore::GetVirtualMetacontent;
  using MagickCore::GetVirtualPixels;
  using MagickCore::GetImageVirtualPixelMethod;
  using MagickCore::GlobExpression;
  using MagickCore::GravityAdjustGeometry;
  using MagickCore::GrayscaleImage;
  using MagickCore::HaldClutImage;
  using MagickCore::HoughLineImage;
  using MagickCore::ImageToBlob;
  using MagickCore::ImagesToBlob;
  using MagickCore::ImplodeImage;
  using MagickCore::ImportQuantumPixels;
  using MagickCore::InterpretImageProperties;
  using MagickCore::InverseFourierTransformImage;
  using MagickCore::InvokeDynamicImageFilter;
  using MagickCore::IsEventLogging;
  using MagickCore::IsGeometry;
  using MagickCore::IsImageOpaque;
  using MagickCore::IsImagesEqual;
  using MagickCore::KuwaharaImage;
  using MagickCore::LevelImage;
  using MagickCore::LevelImageColors;
  using MagickCore::LevelizeImage;
  using MagickCore::LinearStretchImage;
  using MagickCore::LiquidRescaleImage;
  using MagickCore::LocalContrastImage;
  using MagickCore::LocaleCompare;
  using MagickCore::LockSemaphoreInfo;
  using MagickCore::LogMagickEvent;
  using MagickCore::MagickCoreTerminus;
  using MagickCore::MagickToMime;
  using MagickCore::MagnifyImage;
  using MagickCore::MergeImageLayers;
  using MagickCore::MinifyImage;
  using MagickCore::ModulateImage;
  using MagickCore::MorphologyImage;
  using MagickCore::MotionBlurImage;
  using MagickCore::NegateImage;
  using MagickCore::NewMagickWandFromImage;
  using MagickCore::NewPixelWand;
  using MagickCore::NormalizeImage;
  using MagickCore::OilPaintImage;
  using MagickCore::OpaquePaintImage;
  using MagickCore::OrderedDitherImage;
  using MagickCore::OptimizeImageLayers;
  using MagickCore::OptimizeImageTransparency;
  using MagickCore::OptimizePlusImageLayers;
  using MagickCore::ParseMetaGeometry;
  using MagickCore::PerceptibleImage;
  using MagickCore::PingBlob;
  using MagickCore::PingImage;
  using MagickCore::PixelSetPixelColor;
  using MagickCore::PolaroidImage;
  using MagickCore::PopDrawingWand;
  using MagickCore::PosterizeImage;
  using MagickCore::ProfileImage;
  using MagickCore::PushDrawingWand;
  using MagickCore::QuantizeImage;
  using MagickCore::QueueAuthenticPixels;
  using MagickCore::QueueCacheViewAuthenticPixels;
  using MagickCore::RaiseImage;
  using MagickCore::RandomThresholdImage;
  using MagickCore::ReadImage;
  using MagickCore::RegisterMagickInfo;
  using MagickCore::RelinquishMagickMemory;
  using MagickCore::RemapImage;
  using MagickCore::ResampleImage;
  using MagickCore::ResetLinkedListIterator;
  using MagickCore::ResizeImage;
  using MagickCore::ResizeMagickMemory;
  using MagickCore::RollImage;
  using MagickCore::RotateImage;
  using MagickCore::RotationalBlurImage;
  using MagickCore::SampleImage;
  using MagickCore::ScaleImage;
  using MagickCore::SegmentImage;
  using MagickCore::SelectiveBlurImage;
  using MagickCore::SeparateImage;
  using MagickCore::SepiaToneImage;
  using MagickCore::SetGeometry;
  using MagickCore::SetImageAlpha;
  using MagickCore::SetImageArtifact;
  using MagickCore::SetImageBackgroundColor;
  using MagickCore::SetImageColorspace;
  using MagickCore::SetImageDepth;
  using MagickCore::SetImageExtent;
  using MagickCore::SetImageInfo;
  using MagickCore::SetImageInfoFile;
  using MagickCore::SetImageMask;
  using MagickCore::SetImageOption;
  using MagickCore::SetImageProfile;
  using MagickCore::SetImageProperty;
  using MagickCore::SetImageRegistry;
  using MagickCore::SetImageType;
  using MagickCore::SetLogEventMask;
  using MagickCore::SetMagickResourceLimit;
  using MagickCore::SetImageVirtualPixelMethod;
  using MagickCore::SetPixelChannel;
  using MagickCore::SetImageChannelMask;
  using MagickCore::SetStringInfoDatum;
  using MagickCore::ShadeImage;
  using MagickCore::ShadowImage;
  using MagickCore::SharpenImage;
  using MagickCore::SharpenImage;
  using MagickCore::ShaveImage;
  using MagickCore::ShearImage;
  using MagickCore::SigmoidalContrastImage;
  using MagickCore::SignatureImage;
  using MagickCore::SimilarityImage;
  using MagickCore::SketchImage;
  using MagickCore::SmushImages;
  using MagickCore::SolarizeImage;
  using MagickCore::SparseColorImage;
  using MagickCore::SpliceImage;
  using MagickCore::SpreadImage;
  using MagickCore::StatisticImage;
  using MagickCore::SteganoImage;
  using MagickCore::StereoImage;
  using MagickCore::StripImage;
  using MagickCore::SwirlImage;
  using MagickCore::SyncCacheViewAuthenticPixels;
  using MagickCore::SyncImage;
  using MagickCore::SyncAuthenticPixels;
  using MagickCore::TextureImage;
  using MagickCore::ThrowException;
  using MagickCore::TintImage;
  using MagickCore::TransformImageColorspace;
  using MagickCore::TransparentPaintImage;
  using MagickCore::TransparentPaintImageChroma;
  using MagickCore::TransposeImage;
  using MagickCore::TransverseImage;
  using MagickCore::TrimImage;
  using MagickCore::UniqueImageColors;
  using MagickCore::UnlockSemaphoreInfo;
  using MagickCore::UnregisterMagickInfo;
  using MagickCore::UnsharpMaskImage;
  using MagickCore::VignetteImage;
  using MagickCore::WaveImage;
  using MagickCore::WaveletDenoiseImage;
  using MagickCore::WhiteThresholdImage;
  using MagickCore::WriteImage;

#endif // MAGICKCORE_IMPLEMENTATION

}

//////////////////////////////////////////////////////////////////////
//
// No user-serviceable parts beyond this point
//
//////////////////////////////////////////////////////////////////////
#define GetPPException \
  MagickCore::ExceptionInfo \
    *exceptionInfo; \
  exceptionInfo=MagickCore::AcquireExceptionInfo();
#define GetAndSetPPChannelMask(channel) \
  MagickCore::ChannelType \
    channel_mask; \
  channel_mask=MagickCore::SetImageChannelMask(image(),channel)
#define ClonePPDrawException(wand) \
  MagickCore::ExceptionInfo \
    *exceptionInfo; \
  exceptionInfo=MagickCore::DrawCloneExceptionInfo(wand)
#define RestorePPChannelMask \
  MagickCore::SetPixelChannelMask(image(),channel_mask)
#define SetPPChannelMask(channel) \
  (void) MagickCore::SetImageChannelMask(image(),channel)
#define ThrowPPDrawException(quiet) \
  throwException(exceptionInfo,quiet); \
  (void) MagickCore::DestroyExceptionInfo(exceptionInfo)
#define ThrowPPException(quiet) \
  throwException(exceptionInfo,quiet); \
  (void) MagickCore::DestroyExceptionInfo(exceptionInfo)

#endif // Magick_Include_header

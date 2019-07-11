/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand image Methods.
*/

#ifndef MAGICKWAND_MAGICK_IMAGE_H
#define MAGICKWAND_MAGICK_IMAGE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern WandExport ChannelFeatures
  *MagickGetImageFeatures(MagickWand *,const size_t);

extern WandExport ChannelType
   MagickSetImageChannelMask(MagickWand *,const ChannelType);

extern WandExport ChannelStatistics
  *MagickGetImageStatistics(MagickWand *);

extern WandExport char
  *MagickGetImageFilename(MagickWand *),
  *MagickGetImageFormat(MagickWand *),
  *MagickGetImageSignature(MagickWand *),
  *MagickIdentifyImage(MagickWand *);

extern WandExport ColorspaceType
  MagickGetImageColorspace(MagickWand *);

extern WandExport CompositeOperator
  MagickGetImageCompose(MagickWand *);

extern WandExport CompressionType
  MagickGetImageCompression(MagickWand *);

extern WandExport DisposeType
  MagickGetImageDispose(MagickWand *);

extern WandExport double
  *MagickGetImageDistortions(MagickWand *,const MagickWand *,
    const MetricType),
  MagickGetImageFuzz(MagickWand *),
  MagickGetImageGamma(MagickWand *),
  MagickGetImageTotalInkDensity(MagickWand *);

extern WandExport EndianType
  MagickGetImageEndian(MagickWand *);

extern WandExport GravityType
  MagickGetImageGravity(MagickWand *);

extern WandExport Image
  *MagickDestroyImage(Image *),
  *GetImageFromMagickWand(const MagickWand *);

extern WandExport ImageType
  MagickGetImageType(MagickWand *),
  MagickIdentifyImageType(MagickWand *);

extern WandExport InterlaceType
  MagickGetImageInterlaceScheme(MagickWand *);

extern WandExport PixelInterpolateMethod
  MagickGetImageInterpolateMethod(MagickWand *);

extern WandExport MagickBooleanType
  MagickAdaptiveBlurImage(MagickWand *,const double,const double),
  MagickAdaptiveResizeImage(MagickWand *,const size_t,const size_t),
  MagickAdaptiveSharpenImage(MagickWand *,const double,const double),
  MagickAdaptiveThresholdImage(MagickWand *,const size_t,const size_t,
    const double),
  MagickAddImage(MagickWand *,const MagickWand *),
  MagickAddNoiseImage(MagickWand *,const NoiseType,const double),
  MagickAffineTransformImage(MagickWand *,const DrawingWand *),
  MagickAnnotateImage(MagickWand *,const DrawingWand *,const double,
    const double,const double,const char *),
  MagickAnimateImages(MagickWand *,const char *),
  MagickAutoGammaImage(MagickWand *),
  MagickAutoLevelImage(MagickWand *),
  MagickAutoOrientImage(MagickWand *),
  MagickAutoThresholdImage(MagickWand *,const AutoThresholdMethod),
  MagickBlackThresholdImage(MagickWand *,const PixelWand *),
  MagickBlueShiftImage(MagickWand *,const double),
  MagickBlurImage(MagickWand *,const double,const double),
  MagickBorderImage(MagickWand *,const PixelWand *,const size_t,const size_t,
    const CompositeOperator compose),
  MagickBrightnessContrastImage(MagickWand *,const double,const double),
  MagickCannyEdgeImage(MagickWand *,const double,const double,const double,
    const double),
  MagickCharcoalImage(MagickWand *,const double,const double),
  MagickChopImage(MagickWand *,const size_t,const size_t,const ssize_t,
    const ssize_t),
  MagickCLAHEImage(MagickWand *,const size_t,const size_t,const double,
    const double),
  MagickClampImage(MagickWand *),
  MagickClipImage(MagickWand *),
  MagickClipImagePath(MagickWand *,const char *,const MagickBooleanType),
  MagickClutImage(MagickWand *,const MagickWand *,const PixelInterpolateMethod),
  MagickColorDecisionListImage(MagickWand *,const char *),
  MagickColorizeImage(MagickWand *,const PixelWand *,const PixelWand *),
  MagickColorMatrixImage(MagickWand *,const KernelInfo *),
  MagickCommentImage(MagickWand *,const char *),
  MagickCompositeImage(MagickWand *,const MagickWand *,const CompositeOperator,
    const MagickBooleanType,const ssize_t,const ssize_t),
  MagickCompositeImageGravity(MagickWand *,const MagickWand *,
    const CompositeOperator,const GravityType),
  MagickCompositeLayers(MagickWand *,const MagickWand *,const CompositeOperator,
    const ssize_t,const ssize_t),
  MagickConnectedComponentsImage(MagickWand *,const size_t,CCObjectInfo **),
  MagickConstituteImage(MagickWand *,const size_t,const size_t,const char *,
    const StorageType,const void *),
  MagickContrastImage(MagickWand *,const MagickBooleanType),
  MagickContrastStretchImage(MagickWand *,const double,const double),
  MagickConvolveImage(MagickWand *,const KernelInfo *),
  MagickCropImage(MagickWand *,const size_t,const size_t,const ssize_t,
    const ssize_t),
  MagickCycleColormapImage(MagickWand *,const ssize_t),
  MagickDecipherImage(MagickWand *,const char *),
  MagickDeskewImage(MagickWand *,const double),
  MagickDespeckleImage(MagickWand *),
  MagickDisplayImage(MagickWand *,const char *),
  MagickDisplayImages(MagickWand *,const char *),
  MagickDistortImage(MagickWand *,const DistortMethod,const size_t,
    const double *,const MagickBooleanType),
  MagickDrawImage(MagickWand *,const DrawingWand *),
  MagickEdgeImage(MagickWand *,const double),
  MagickEmbossImage(MagickWand *,const double,const double),
  MagickEncipherImage(MagickWand *,const char *),
  MagickEnhanceImage(MagickWand *),
  MagickEqualizeImage(MagickWand *),
  MagickEvaluateImage(MagickWand *,const MagickEvaluateOperator,const double),
  MagickExportImagePixels(MagickWand *,const ssize_t,const ssize_t,
    const size_t,const size_t,const char *,const StorageType,void *),
  MagickExtentImage(MagickWand *,const size_t,const size_t,const ssize_t,
    const ssize_t),
  MagickFlipImage(MagickWand *),
  MagickFloodfillPaintImage(MagickWand *,const PixelWand *,const double,
    const PixelWand *,const ssize_t,const ssize_t,const MagickBooleanType),
  MagickFlopImage(MagickWand *),
  MagickForwardFourierTransformImage(MagickWand *,const MagickBooleanType),
  MagickFrameImage(MagickWand *,const PixelWand *,const size_t,const size_t,
    const ssize_t,const ssize_t,const CompositeOperator),
  MagickFunctionImage(MagickWand *,const MagickFunction,const size_t,
    const double *),
  MagickGammaImage(MagickWand *,const double),
  MagickGaussianBlurImage(MagickWand *,const double,const double),
  MagickGetImageAlphaChannel(MagickWand *),
  MagickGetImageBackgroundColor(MagickWand *,PixelWand *),
  MagickGetImageBluePrimary(MagickWand *,double *,double *,double *),
  MagickGetImageBorderColor(MagickWand *,PixelWand *),
  MagickGetImageKurtosis(MagickWand *,double *,double *),
  MagickGetImageMean(MagickWand *,double *,double *),
  MagickGetImageRange(MagickWand *,double *,double *),
  MagickGetImageColormapColor(MagickWand *,const size_t,PixelWand *),
  MagickGetImageDistortion(MagickWand *,const MagickWand *,const MetricType,
    double *),
  MagickGetImageGreenPrimary(MagickWand *,double *,double *,double *),
  MagickGetImageLength(MagickWand *,MagickSizeType *),
  MagickGetImageMatteColor(MagickWand *,PixelWand *),
  MagickGetImagePage(MagickWand *,size_t *,size_t *,ssize_t *,
    ssize_t *),
  MagickGetImagePixelColor(MagickWand *,const ssize_t,const ssize_t,
    PixelWand *),
  MagickGetImageRange(MagickWand *,double *,double *),
  MagickGetImageRedPrimary(MagickWand *,double *,double *,double *),
  MagickGetImageResolution(MagickWand *,double *,double *),
  MagickGetImageWhitePoint(MagickWand *,double *,double *,double *),
  MagickHaldClutImage(MagickWand *,const MagickWand *),
  MagickHasNextImage(MagickWand *),
  MagickHasPreviousImage(MagickWand *),
  MagickHoughLineImage(MagickWand *,const size_t,const size_t,const size_t),
  MagickImplodeImage(MagickWand *,const double,const PixelInterpolateMethod),
  MagickImportImagePixels(MagickWand *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *),
  MagickInterpolativeResizeImage(MagickWand *,const size_t,const size_t,
    const PixelInterpolateMethod),
  MagickInverseFourierTransformImage(MagickWand *,MagickWand *,
    const MagickBooleanType),
  MagickKuwaharaImage(MagickWand *,const double,const double),
  MagickLabelImage(MagickWand *,const char *),
  MagickLevelImage(MagickWand *,const double,const double,const double),
  MagickLevelImageColors(MagickWand *,const PixelWand *,const PixelWand *,
    const MagickBooleanType),
  MagickLevelizeImage(MagickWand *,const double,const double,const double),
  MagickLinearStretchImage(MagickWand *,const double,const double),
  MagickLiquidRescaleImage(MagickWand *,const size_t,const size_t,const double,
    const double),
  MagickLocalContrastImage(MagickWand *,const double,const double),
  MagickMagnifyImage(MagickWand *),
  MagickMeanShiftImage(MagickWand *,const size_t,const size_t,const double),
  MagickMedianConvolveImage(MagickWand *,const double),
  MagickMinifyImage(MagickWand *),
  MagickModeImage(MagickWand *,const double),
  MagickModulateImage(MagickWand *,const double,const double,const double),
  MagickMorphologyImage(MagickWand *,MorphologyMethod,const ssize_t,
    KernelInfo *),
  MagickMotionBlurImage(MagickWand *,const double,const double,const double),
  MagickNegateImage(MagickWand *,const MagickBooleanType),
  MagickNewImage(MagickWand *,const size_t,const size_t,const PixelWand *),
  MagickNextImage(MagickWand *),
  MagickNormalizeImage(MagickWand *),
  MagickOilPaintImage(MagickWand *,const double,const double),
  MagickOpaquePaintImage(MagickWand *,const PixelWand *,const PixelWand *,
    const double,const MagickBooleanType),
  MagickOptimizeImageTransparency(MagickWand *),
  MagickOrderedDitherImage(MagickWand *,const char *),
  MagickPolynomialImage(MagickWand *,const size_t,const double *),
  MagickTransparentPaintImage(MagickWand *,const PixelWand *,
    const double,const double,const MagickBooleanType invert),
  MagickPingImage(MagickWand *,const char *),
  MagickPingImageBlob(MagickWand *,const void *,const size_t),
  MagickPingImageFile(MagickWand *,FILE *),
  MagickPolaroidImage(MagickWand *,const DrawingWand *,const char *,
    const double,const PixelInterpolateMethod),
  MagickPosterizeImage(MagickWand *,const size_t,const DitherMethod),
  MagickPreviousImage(MagickWand *),
  MagickQuantizeImage(MagickWand *,const size_t,const ColorspaceType,
    const size_t,const DitherMethod,const MagickBooleanType),
  MagickQuantizeImages(MagickWand *,const size_t,const ColorspaceType,
    const size_t,const DitherMethod,const MagickBooleanType),
  MagickRangeThresholdImage(MagickWand *,const double,const double,
    const double,const double),
  MagickRotationalBlurImage(MagickWand *,const double),
  MagickRaiseImage(MagickWand *,const size_t,const size_t,const ssize_t,
    const ssize_t,const MagickBooleanType),
  MagickRandomThresholdImage(MagickWand *,const double,const double),
  MagickReadImage(MagickWand *,const char *),
  MagickReadImageBlob(MagickWand *,const void *,const size_t),
  MagickReadImageFile(MagickWand *,FILE *),
  MagickReduceNoiseImage(MagickWand *,const double),
  MagickRemapImage(MagickWand *,const MagickWand *,const DitherMethod),
  MagickRemoveImage(MagickWand *),
  MagickResampleImage(MagickWand *,const double,const double,const FilterType),
  MagickResetImagePage(MagickWand *,const char *),
  MagickResizeImage(MagickWand *,const size_t,const size_t,const FilterType),
  MagickRollImage(MagickWand *,const ssize_t,const ssize_t),
  MagickRotateImage(MagickWand *,const PixelWand *,const double),
  MagickSampleImage(MagickWand *,const size_t,const size_t),
  MagickScaleImage(MagickWand *,const size_t,const size_t),
  MagickSegmentImage(MagickWand *,const ColorspaceType,const MagickBooleanType,
    const double,const double),
  MagickSelectiveBlurImage(MagickWand *,const double,const double,
    const double),
  MagickSeparateImage(MagickWand *,const ChannelType),
  MagickSepiaToneImage(MagickWand *,const double),
  MagickSetImage(MagickWand *,const MagickWand *),
  MagickSetImageAlpha(MagickWand *,const double),
  MagickSetImageAlphaChannel(MagickWand *,const AlphaChannelOption),
  MagickSetImageBackgroundColor(MagickWand *,const PixelWand *),
  MagickSetImageBluePrimary(MagickWand *,const double,const double,
    const double),
  MagickSetImageBorderColor(MagickWand *,const PixelWand *),
  MagickSetImageColor(MagickWand *,const PixelWand *),
  MagickSetImageColormapColor(MagickWand *,const size_t,
    const PixelWand *),
  MagickSetImageColorspace(MagickWand *,const ColorspaceType),
  MagickSetImageCompose(MagickWand *,const CompositeOperator),
  MagickSetImageCompression(MagickWand *,const CompressionType),
  MagickSetImageDelay(MagickWand *,const size_t),
  MagickSetImageDepth(MagickWand *,const size_t),
  MagickSetImageDispose(MagickWand *,const DisposeType),
  MagickSetImageCompressionQuality(MagickWand *,const size_t),
  MagickSetImageEndian(MagickWand *,const EndianType),
  MagickSetImageExtent(MagickWand *,const size_t,const size_t),
  MagickSetImageFilename(MagickWand *,const char *),
  MagickSetImageFormat(MagickWand *,const char *),
  MagickSetImageFuzz(MagickWand *,const double),
  MagickSetImageGamma(MagickWand *,const double),
  MagickSetImageGravity(MagickWand *,const GravityType),
  MagickSetImageGreenPrimary(MagickWand *,const double,const double,
    const double),
  MagickSetImageInterlaceScheme(MagickWand *,const InterlaceType),
  MagickSetImageInterpolateMethod(MagickWand *,const PixelInterpolateMethod),
  MagickSetImageIterations(MagickWand *,const size_t),
  MagickSetImageMatte(MagickWand *,const MagickBooleanType),
  MagickSetImageMatteColor(MagickWand *,const PixelWand *),
  MagickSetImageOrientation(MagickWand *,const OrientationType),
  MagickSetImagePage(MagickWand *,const size_t,const size_t,const ssize_t,
    const ssize_t),
  MagickSetImagePixelColor(MagickWand *,const ssize_t,const ssize_t,
    const PixelWand *),
  MagickSetImageRedPrimary(MagickWand *,const double,const double,
    const double),
  MagickSetImageRenderingIntent(MagickWand *,const RenderingIntent),
  MagickSetImageResolution(MagickWand *,const double,const double),
  MagickSetImageScene(MagickWand *,const size_t),
  MagickSetImageTicksPerSecond(MagickWand *,const ssize_t),
  MagickSetImageType(MagickWand *,const ImageType),
  MagickSetImageUnits(MagickWand *,const ResolutionType),
  MagickSetImageWhitePoint(MagickWand *,const double,const double,
    const double),
  MagickShadeImage(MagickWand *,const MagickBooleanType,const double,
    const double),
  MagickShadowImage(MagickWand *,const double,const double,const ssize_t,
    const ssize_t),
  MagickSharpenImage(MagickWand *,const double,const double),
  MagickShaveImage(MagickWand *,const size_t,const size_t),
  MagickShearImage(MagickWand *,const PixelWand *,const double,const double),
  MagickSigmoidalContrastImage(MagickWand *,const MagickBooleanType,
    const double,const double),
  MagickSketchImage(MagickWand *,const double,const double,const double),
  MagickSolarizeImage(MagickWand *,const double),
  MagickSparseColorImage(MagickWand *,const SparseColorMethod,const size_t,
    const double *),
  MagickSpliceImage(MagickWand *,const size_t,const size_t,const ssize_t,
    const ssize_t),
  MagickSpreadImage(MagickWand *,const PixelInterpolateMethod,const double),
  MagickStatisticImage(MagickWand *,const StatisticType,const size_t,
    const size_t),
  MagickStripImage(MagickWand *),
  MagickSwirlImage(MagickWand *,const double,const PixelInterpolateMethod),
  MagickTintImage(MagickWand *,const PixelWand *,const PixelWand *),
  MagickTransformImageColorspace(MagickWand *,const ColorspaceType),
  MagickTransposeImage(MagickWand *),
  MagickTransverseImage(MagickWand *),
  MagickThresholdImage(MagickWand *,const double),
  MagickThresholdImageChannel(MagickWand *,const ChannelType,const double),
  MagickThumbnailImage(MagickWand *,const size_t,const size_t),
  MagickTrimImage(MagickWand *,const double),
  MagickUniqueImageColors(MagickWand *),
  MagickUnsharpMaskImage(MagickWand *,const double,const double,const double,
    const double),
  MagickVignetteImage(MagickWand *,const double,const double,const ssize_t,
    const ssize_t),
  MagickWaveImage(MagickWand *,const double,const double,
    const PixelInterpolateMethod),
  MagickWaveletDenoiseImage(MagickWand *,const double,const double),
  MagickWhiteThresholdImage(MagickWand *,const PixelWand *),
  MagickWriteImage(MagickWand *,const char *),
  MagickWriteImageFile(MagickWand *,FILE *),
  MagickWriteImages(MagickWand *,const char *,const MagickBooleanType),
  MagickWriteImagesFile(MagickWand *,FILE *);

extern WandExport MagickProgressMonitor
  MagickSetImageProgressMonitor(MagickWand *,const MagickProgressMonitor,
    void *);

extern WandExport MagickWand
  *MagickAppendImages(MagickWand *,const MagickBooleanType),
  *MagickChannelFxImage(MagickWand *,const char *),
  *MagickCoalesceImages(MagickWand *),
  *MagickCombineImages(MagickWand *,const ColorspaceType),
  *MagickCompareImages(MagickWand *,const MagickWand *,const MetricType,
    double *),
  *MagickCompareImagesLayers(MagickWand *,const LayerMethod),
  *MagickComplexImages(MagickWand *,const ComplexOperator),
  *MagickDeconstructImages(MagickWand *),
  *MagickEvaluateImages(MagickWand *,const MagickEvaluateOperator),
  *MagickFxImage(MagickWand *,const char *),
  *MagickGetImage(MagickWand *),
  *MagickGetImageMask(MagickWand *,const PixelMask),
  *MagickGetImageRegion(MagickWand *,const size_t,const size_t,const ssize_t,
    const ssize_t),
  *MagickMergeImageLayers(MagickWand *,const LayerMethod),
  *MagickMorphImages(MagickWand *,const size_t),
  *MagickMontageImage(MagickWand *,const DrawingWand *,const char *,
    const char *,const MontageMode,const char *),
  *MagickOptimizeImageLayers(MagickWand *),
  *MagickPreviewImages(MagickWand *wand,const PreviewType),
  *MagickSimilarityImage(MagickWand *,const MagickWand *,const MetricType,
    const double,RectangleInfo *,double *),
  *MagickSmushImages(MagickWand *,const MagickBooleanType,const ssize_t),
  *MagickSteganoImage(MagickWand *,const MagickWand *,const ssize_t),
  *MagickStereoImage(MagickWand *,const MagickWand *),
  *MagickTextureImage(MagickWand *,const MagickWand *);

extern WandExport OrientationType
  MagickGetImageOrientation(MagickWand *);

extern WandExport PixelWand
  **MagickGetImageHistogram(MagickWand *,size_t *);

extern WandExport RenderingIntent
  MagickGetImageRenderingIntent(MagickWand *);

extern WandExport ResolutionType
  MagickGetImageUnits(MagickWand *);

extern WandExport size_t
  MagickGetImageColors(MagickWand *),
  MagickGetImageCompressionQuality(MagickWand *),
  MagickGetImageDelay(MagickWand *),
  MagickGetImageDepth(MagickWand *),
  MagickGetImageHeight(MagickWand *),
  MagickGetImageIterations(MagickWand *),
  MagickGetImageScene(MagickWand *),
  MagickGetImageTicksPerSecond(MagickWand *),
  MagickGetImageWidth(MagickWand *),
  MagickGetNumberImages(MagickWand *);

extern WandExport unsigned char
  *MagickGetImageBlob(MagickWand *,size_t *),
  *MagickGetImagesBlob(MagickWand *,size_t *);

extern WandExport VirtualPixelMethod
  MagickGetImageVirtualPixelMethod(MagickWand *),
  MagickSetImageVirtualPixelMethod(MagickWand *,const VirtualPixelMethod);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif





<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickWand, C API for ImageMagick: Image Methods</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickwc, api, for, imagemagick:, image, methods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
  <meta name="rating" content="GENERAL"/>
  <meta name="robots" content="INDEX, FOLLOW"/>
  <meta name="generator" content="ImageMagick Studio LLC"/>
  <meta name="author" content="ImageMagick Studio LLC"/>
  <meta name="revisit-after" content="2 DAYS"/>
  <meta name="resource-type" content="document"/>
  <meta name="copyright" content="Copyright (c) 1999-2016 ImageMagick Studio LLC"/>
  <meta name="distribution" content="Global"/>
  <meta name="magick-serial" content="P131-S030410-R485315270133-P82224-A6668-G1245-1"/>
  <link rel="icon" href="../image/wand.png"/>
  <link rel="shortcut icon" href="../image/wand.ico"/>
  <link rel="stylesheet" href="../css/magick.php"/>
</head>
<body>
<div class="main">
<div class="magick-masthead">
  <div class="container">
    <script async="async" src="http://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>    <ins class="adsbygoogle"
         style="display:block"
         data-ad-client="ca-pub-3129977114552745"
         data-ad-slot="6345125851"
         data-ad-format="auto"></ins>
    <script>
      (adsbygoogle = window.adsbygoogle || []).push({});
    </script>
    <nav class="magick-nav">
      <a class="magick-nav-item " href="../index.php">Home</a>
      <a class="magick-nav-item " href="../script/binary-releases.php">Download</a>
      <a class="magick-nav-item " href="../script/command-line-tools.php">Tools</a>
      <a class="magick-nav-item " href="../script/command-line-options.php">Options</a>
      <a class="magick-nav-item " href="../script/resources.php">Resources</a>
      <a class="magick-nav-item " href="../script/api.php">Develop</a>
      <a class="magick-nav-item " href="../script/search.php">Search</a>
      <a class="magick-nav-item pull-right" href="http://www.imagemagick.org/discourse-server/">Community</a>
    </nav>
  </div>
</div>
<div class="container">
<div class="magick-header">
<p class="text-center"><a href="magick-image.php#GetImageFromMagickWand">GetImageFromMagickWand</a> &bull; <a href="magick-image.php#MagickAdaptiveBlurImage">MagickAdaptiveBlurImage</a> &bull; <a href="magick-image.php#MagickAdaptiveResizeImage">MagickAdaptiveResizeImage</a> &bull; <a href="magick-image.php#MagickAdaptiveSharpenImage">MagickAdaptiveSharpenImage</a> &bull; <a href="magick-image.php#MagickAdaptiveThresholdImage">MagickAdaptiveThresholdImage</a> &bull; <a href="magick-image.php#MagickAddImage">MagickAddImage</a> &bull; <a href="magick-image.php#MagickAddNoiseImage">MagickAddNoiseImage</a> &bull; <a href="magick-image.php#MagickAffineTransformImage">MagickAffineTransformImage</a> &bull; <a href="magick-image.php#MagickAnnotateImage">MagickAnnotateImage</a> &bull; <a href="magick-image.php#MagickAnimateImages">MagickAnimateImages</a> &bull; <a href="magick-image.php#MagickAppendImages">MagickAppendImages</a> &bull; <a href="magick-image.php#MagickAutoGammaImage">MagickAutoGammaImage</a> &bull; <a href="magick-image.php#MagickAutoLevelImage">MagickAutoLevelImage</a> &bull; <a href="magick-image.php#MagickAutoOrientImage">MagickAutoOrientImage</a> &bull; <a href="magick-image.php#MagickBlackThresholdImage">MagickBlackThresholdImage</a> &bull; <a href="magick-image.php#MagickBlueShiftImage">MagickBlueShiftImage</a> &bull; <a href="magick-image.php#MagickBlurImage">MagickBlurImage</a> &bull; <a href="magick-image.php#MagickBorderImage">MagickBorderImage</a> &bull; <a href="magick-image.php#Use MagickBrightnessContrastImage">Use MagickBrightnessContrastImage</a> &bull; <a href="magick-image.php#MagickChannelFxImage">MagickChannelFxImage</a> &bull; <a href="magick-image.php#MagickCharcoalImage">MagickCharcoalImage</a> &bull; <a href="magick-image.php#MagickChopImage">MagickChopImage</a> &bull; <a href="magick-image.php#MagickClampImage">MagickClampImage</a> &bull; <a href="magick-image.php#MagickClipImage">MagickClipImage</a> &bull; <a href="magick-image.php#MagickClipImagePath">MagickClipImagePath</a> &bull; <a href="magick-image.php#MagickClutImage">MagickClutImage</a> &bull; <a href="magick-image.php#MagickCoalesceImages">MagickCoalesceImages</a> &bull; <a href="magick-image.php#MagickColorDecisionListImage">MagickColorDecisionListImage</a> &bull; <a href="magick-image.php#MagickColorizeImage">MagickColorizeImage</a> &bull; <a href="magick-image.php#MagickColorMatrixImage">MagickColorMatrixImage</a> &bull; <a href="magick-image.php#MagickCombineImages">MagickCombineImages</a> &bull; <a href="magick-image.php#MagickCommentImage">MagickCommentImage</a> &bull; <a href="magick-image.php#MagickCompareImagesLayers">MagickCompareImagesLayers</a> &bull; <a href="magick-image.php#MagickCompareImages">MagickCompareImages</a> &bull; <a href="magick-image.php#MagickCompositeImage">MagickCompositeImage</a> &bull; <a href="magick-image.php#MagickCompositeImageGravity">MagickCompositeImageGravity</a> &bull; <a href="magick-image.php#MagickCompositeLayers">MagickCompositeLayers</a> &bull; <a href="magick-image.php#MagickContrastImage">MagickContrastImage</a> &bull; <a href="magick-image.php#MagickContrastStretchImage">MagickContrastStretchImage</a> &bull; <a href="magick-image.php#MagickConvolveImage">MagickConvolveImage</a> &bull; <a href="magick-image.php#MagickCropImage">MagickCropImage</a> &bull; <a href="magick-image.php#MagickCycleColormapImage">MagickCycleColormapImage</a> &bull; <a href="magick-image.php#MagickConstituteImage">MagickConstituteImage</a> &bull; <a href="magick-image.php#MagickDecipherImage">MagickDecipherImage</a> &bull; <a href="magick-image.php#MagickDeconstructImages">MagickDeconstructImages</a> &bull; <a href="magick-image.php#MagickDeskewImage">MagickDeskewImage</a> &bull; <a href="magick-image.php#MagickDespeckleImage">MagickDespeckleImage</a> &bull; <a href="magick-image.php#MagickDestroyImage">MagickDestroyImage</a> &bull; <a href="magick-image.php#MagickDisplayImage">MagickDisplayImage</a> &bull; <a href="magick-image.php#MagickDisplayImages">MagickDisplayImages</a> &bull; <a href="magick-image.php#MagickDistortImage">MagickDistortImage</a> &bull; <a href="magick-image.php#MagickDrawImage">MagickDrawImage</a> &bull; <a href="magick-image.php#MagickEdgeImage">MagickEdgeImage</a> &bull; <a href="magick-image.php#MagickEmbossImage">MagickEmbossImage</a> &bull; <a href="magick-image.php#MagickEncipherImage">MagickEncipherImage</a> &bull; <a href="magick-image.php#MagickEnhanceImage">MagickEnhanceImage</a> &bull; <a href="magick-image.php#MagickEqualizeImage">MagickEqualizeImage</a> &bull; <a href="magick-image.php#MagickEvaluateImage">MagickEvaluateImage</a> &bull; <a href="magick-image.php#MagickExportImagePixels">MagickExportImagePixels</a> &bull; <a href="magick-image.php#MagickExtentImage">MagickExtentImage</a> &bull; <a href="magick-image.php#MagickFlipImage">MagickFlipImage</a> &bull; <a href="magick-image.php#MagickFloodfillPaintImage">MagickFloodfillPaintImage</a> &bull; <a href="magick-image.php#MagickFlopImage">MagickFlopImage</a> &bull; <a href="magick-image.php#MagickForwardFourierTransformImage">MagickForwardFourierTransformImage</a> &bull; <a href="magick-image.php#MagickFrameImage">MagickFrameImage</a> &bull; <a href="magick-image.php#MagickFunctionImage">MagickFunctionImage</a> &bull; <a href="magick-image.php#MagickFxImage">MagickFxImage</a> &bull; <a href="magick-image.php#MagickGammaImage">MagickGammaImage</a> &bull; <a href="magick-image.php#MagickGaussianBlurImage">MagickGaussianBlurImage</a> &bull; <a href="magick-image.php#MagickGetImage">MagickGetImage</a> &bull; <a href="magick-image.php#MagickGetImageAlphaChannel">MagickGetImageAlphaChannel</a> &bull; <a href="magick-image.php#MagickGetImageMask">MagickGetImageMask</a> &bull; <a href="magick-image.php#MagickGetImageBackgroundColor">MagickGetImageBackgroundColor</a> &bull; <a href="magick-image.php#MagickGetImageBlob">MagickGetImageBlob</a> &bull; <a href="magick-image.php#MagickGetImageBlob">MagickGetImageBlob</a> &bull; <a href="magick-image.php#MagickGetImageBluePrimary">MagickGetImageBluePrimary</a> &bull; <a href="magick-image.php#MagickGetImageBorderColor">MagickGetImageBorderColor</a> &bull; <a href="magick-image.php#MagickGetImageFeatures">MagickGetImageFeatures</a> &bull; <a href="magick-image.php#MagickGetImageKurtosis">MagickGetImageKurtosis</a> &bull; <a href="magick-image.php#MagickGetImageMean">MagickGetImageMean</a> &bull; <a href="magick-image.php#MagickGetImageRange">MagickGetImageRange</a> &bull; <a href="magick-image.php#MagickGetImageStatistics">MagickGetImageStatistics</a> &bull; <a href="magick-image.php#MagickGetImageColormapColor">MagickGetImageColormapColor</a> &bull; <a href="magick-image.php#MagickGetImageColors">MagickGetImageColors</a> &bull; <a href="magick-image.php#MagickGetImageColorspace">MagickGetImageColorspace</a> &bull; <a href="magick-image.php#MagickGetImageCompose">MagickGetImageCompose</a> &bull; <a href="magick-image.php#MagickGetImageCompression">MagickGetImageCompression</a> &bull; <a href="magick-image.php#MagickGetImageCompressionQuality">MagickGetImageCompressionQuality</a> &bull; <a href="magick-image.php#MagickGetImageDelay">MagickGetImageDelay</a> &bull; <a href="magick-image.php#MagickGetImageDepth">MagickGetImageDepth</a> &bull; <a href="magick-image.php#MagickGetImageDispose">MagickGetImageDispose</a> &bull; <a href="magick-image.php#MagickGetImageDistortion">MagickGetImageDistortion</a> &bull; <a href="magick-image.php#MagickGetImageDistortions">MagickGetImageDistortions</a> &bull; <a href="magick-image.php#MagickGetImageEndian">MagickGetImageEndian</a> &bull; <a href="magick-image.php#MagickGetImageFilename">MagickGetImageFilename</a> &bull; <a href="magick-image.php#MagickGetImageFormat">MagickGetImageFormat</a> &bull; <a href="magick-image.php#MagickGetImageFuzz">MagickGetImageFuzz</a> &bull; <a href="magick-image.php#MagickGetImageGamma">MagickGetImageGamma</a> &bull; <a href="magick-image.php#MagickGetImageGravity">MagickGetImageGravity</a> &bull; <a href="magick-image.php#MagickGetImageGreenPrimary">MagickGetImageGreenPrimary</a> &bull; <a href="magick-image.php#MagickGetImageHeight">MagickGetImageHeight</a> &bull; <a href="magick-image.php#MagickGetImageHistogram">MagickGetImageHistogram</a> &bull; <a href="magick-image.php#MagickGetImageInterlaceScheme">MagickGetImageInterlaceScheme</a> &bull; <a href="magick-image.php#MagickGetImageInterpolateMethod">MagickGetImageInterpolateMethod</a> &bull; <a href="magick-image.php#MagickGetImageIterations">MagickGetImageIterations</a> &bull; <a href="magick-image.php#MagickGetImageLength">MagickGetImageLength</a> &bull; <a href="magick-image.php#MagickGetImageMatteColor">MagickGetImageMatteColor</a> &bull; <a href="magick-image.php#MagickGetImageOrientation">MagickGetImageOrientation</a> &bull; <a href="magick-image.php#MagickGetImagePage">MagickGetImagePage</a> &bull; <a href="magick-image.php#MagickGetImagePixelColor">MagickGetImagePixelColor</a> &bull; <a href="magick-image.php#MagickGetImageRedPrimary">MagickGetImageRedPrimary</a> &bull; <a href="magick-image.php#MagickGetImageRegion">MagickGetImageRegion</a> &bull; <a href="magick-image.php#MagickGetImageRenderingIntent">MagickGetImageRenderingIntent</a> &bull; <a href="magick-image.php#MagickGetImageResolution">MagickGetImageResolution</a> &bull; <a href="magick-image.php#MagickGetImageScene">MagickGetImageScene</a> &bull; <a href="magick-image.php#MagickGetImageSignature">MagickGetImageSignature</a> &bull; <a href="magick-image.php#MagickGetImageTicksPerSecond">MagickGetImageTicksPerSecond</a> &bull; <a href="magick-image.php#MagickGetImageType">MagickGetImageType</a> &bull; <a href="magick-image.php#MagickGetImageUnits">MagickGetImageUnits</a> &bull; <a href="magick-image.php#MagickGetImageVirtualPixelMethod">MagickGetImageVirtualPixelMethod</a> &bull; <a href="magick-image.php#MagickGetImageWhitePoint">MagickGetImageWhitePoint</a> &bull; <a href="magick-image.php#MagickGetImageWidth">MagickGetImageWidth</a> &bull; <a href="magick-image.php#MagickGetNumberImages">MagickGetNumberImages</a> &bull; <a href="magick-image.php#MagickGetImageTotalInkDensity">MagickGetImageTotalInkDensity</a> &bull; <a href="magick-image.php#MagickHaldClutImage">MagickHaldClutImage</a> &bull; <a href="magick-image.php#MagickHasNextImage">MagickHasNextImage</a> &bull; <a href="magick-image.php#MagickHasPreviousImage">MagickHasPreviousImage</a> &bull; <a href="magick-image.php#MagickIdentifyImage">MagickIdentifyImage</a> &bull; <a href="magick-image.php#MagickIdentifyImageType">MagickIdentifyImageType</a> &bull; <a href="magick-image.php#MagickImplodeImage">MagickImplodeImage</a> &bull; <a href="magick-image.php#MagickImportImagePixels">MagickImportImagePixels</a> &bull; <a href="magick-image.php#MagickInterpolativeResizeImage">MagickInterpolativeResizeImage</a> &bull; <a href="magick-image.php#MagickInverseFourierTransformImage">MagickInverseFourierTransformImage</a> &bull; <a href="magick-image.php#MagickLabelImage">MagickLabelImage</a> &bull; <a href="magick-image.php#MagickLevelImage">MagickLevelImage</a> &bull; <a href="magick-image.php#MagickLinearStretchImage">MagickLinearStretchImage</a> &bull; <a href="magick-image.php#MagickLiquidRescaleImage">MagickLiquidRescaleImage</a> &bull; <a href="magick-image.php#MagickMagnifyImage">MagickMagnifyImage</a> &bull; <a href="magick-image.php#MagickMergeImageLayers">MagickMergeImageLayers</a> &bull; <a href="magick-image.php#MagickMinifyImage">MagickMinifyImage</a> &bull; <a href="magick-image.php#MagickModulateImage">MagickModulateImage</a> &bull; <a href="magick-image.php#MagickMontageImage">MagickMontageImage</a> &bull; <a href="magick-image.php#MagickMorphImages">MagickMorphImages</a> &bull; <a href="magick-image.php#MagickMorphologyImage">MagickMorphologyImage</a> &bull; <a href="magick-image.php#MagickMotionBlurImage">MagickMotionBlurImage</a> &bull; <a href="magick-image.php#MagickNegateImage">MagickNegateImage</a> &bull; <a href="magick-image.php#MagickNewImage">MagickNewImage</a> &bull; <a href="magick-image.php#MagickNextImage">MagickNextImage</a> &bull; <a href="magick-image.php#MagickNormalizeImage">MagickNormalizeImage</a> &bull; <a href="magick-image.php#MagickOilPaintImage">MagickOilPaintImage</a> &bull; <a href="magick-image.php#MagickOpaquePaintImage">MagickOpaquePaintImage</a> &bull; <a href="magick-image.php#MagickOptimizeImageLayers">MagickOptimizeImageLayers</a> &bull; <a href="magick-image.php#MagickOptimizeImageTransparency">MagickOptimizeImageTransparency</a> &bull; <a href="magick-image.php#MagickOrderedPosterizeImage">MagickOrderedPosterizeImage</a> &bull; <a href="magick-image.php#MagickPingImage">MagickPingImage</a> &bull; <a href="magick-image.php#MagickPingImageBlob">MagickPingImageBlob</a> &bull; <a href="magick-image.php#MagickPingImageFile">MagickPingImageFile</a> &bull; <a href="magick-image.php#MagickPolaroidImage">MagickPolaroidImage</a> &bull; <a href="magick-image.php#MagickPosterizeImage">MagickPosterizeImage</a> &bull; <a href="magick-image.php#MagickPreviewImages">MagickPreviewImages</a> &bull; <a href="magick-image.php#MagickPreviousImage">MagickPreviousImage</a> &bull; <a href="magick-image.php#MagickQuantizeImage">MagickQuantizeImage</a> &bull; <a href="magick-image.php#MagickQuantizeImages">MagickQuantizeImages</a> &bull; <a href="magick-image.php#MagickRotationalBlurImage">MagickRotationalBlurImage</a> &bull; <a href="magick-image.php#MagickRaiseImage">MagickRaiseImage</a> &bull; <a href="magick-image.php#MagickRandomThresholdImage">MagickRandomThresholdImage</a> &bull; <a href="magick-image.php#MagickReadImage">MagickReadImage</a> &bull; <a href="magick-image.php#MagickReadImageBlob">MagickReadImageBlob</a> &bull; <a href="magick-image.php#MagickReadImageFile">MagickReadImageFile</a> &bull; <a href="magick-image.php#MagickRemapImage">MagickRemapImage</a> &bull; <a href="magick-image.php#MagickRemoveImage">MagickRemoveImage</a> &bull; <a href="magick-image.php#MagickResampleImage">MagickResampleImage</a> &bull; <a href="magick-image.php#MagickResetImagePage">MagickResetImagePage</a> &bull; <a href="magick-image.php#MagickResizeImage">MagickResizeImage</a> &bull; <a href="magick-image.php#MagickRollImage">MagickRollImage</a> &bull; <a href="magick-image.php#MagickRotateImage">MagickRotateImage</a> &bull; <a href="magick-image.php#MagickSampleImage">MagickSampleImage</a> &bull; <a href="magick-image.php#MagickScaleImage">MagickScaleImage</a> &bull; <a href="magick-image.php#MagickSegmentImage">MagickSegmentImage</a> &bull; <a href="magick-image.php#MagickSelectiveBlurImage">MagickSelectiveBlurImage</a> &bull; <a href="magick-image.php#MagickSeparateImage">MagickSeparateImage</a> &bull; <a href="magick-image.php#MagickSepiaToneImage">MagickSepiaToneImage</a> &bull; <a href="magick-image.php#MagickSetImage">MagickSetImage</a> &bull; <a href="magick-image.php#MagickSetImageAlphaChannel">MagickSetImageAlphaChannel</a> &bull; <a href="magick-image.php#MagickSetImageBackgroundColor">MagickSetImageBackgroundColor</a> &bull; <a href="magick-image.php#MagickSetImageBluePrimary">MagickSetImageBluePrimary</a> &bull; <a href="magick-image.php#MagickSetImageBorderColor">MagickSetImageBorderColor</a> &bull; <a href="magick-image.php#MagickSetImageChannelMask">MagickSetImageChannelMask</a> &bull; <a href="magick-image.php#MagickSetImageMask">MagickSetImageMask</a> &bull; <a href="magick-image.php#MagickSetImageColor">MagickSetImageColor</a> &bull; <a href="magick-image.php#MagickSetImageColormapColor">MagickSetImageColormapColor</a> &bull; <a href="magick-image.php#MagickSetImageColorspace">MagickSetImageColorspace</a> &bull; <a href="magick-image.php#MagickSetImageCompose">MagickSetImageCompose</a> &bull; <a href="magick-image.php#MagickSetImageCompression">MagickSetImageCompression</a> &bull; <a href="magick-image.php#MagickSetImageCompressionQuality">MagickSetImageCompressionQuality</a> &bull; <a href="magick-image.php#MagickSetImageDelay">MagickSetImageDelay</a> &bull; <a href="magick-image.php#MagickSetImageDepth">MagickSetImageDepth</a> &bull; <a href="magick-image.php#MagickSetImageDispose">MagickSetImageDispose</a> &bull; <a href="magick-image.php#MagickSetImageEndian">MagickSetImageEndian</a> &bull; <a href="magick-image.php#MagickSetImageExtent">MagickSetImageExtent</a> &bull; <a href="magick-image.php#MagickSetImageFilename">MagickSetImageFilename</a> &bull; <a href="magick-image.php#MagickSetImageFormat">MagickSetImageFormat</a> &bull; <a href="magick-image.php#MagickSetImageFuzz">MagickSetImageFuzz</a> &bull; <a href="magick-image.php#MagickSetImageGamma">MagickSetImageGamma</a> &bull; <a href="magick-image.php#MagickSetImageGravity">MagickSetImageGravity</a> &bull; <a href="magick-image.php#MagickSetImageGreenPrimary">MagickSetImageGreenPrimary</a> &bull; <a href="magick-image.php#MagickSetImageInterlaceScheme">MagickSetImageInterlaceScheme</a> &bull; <a href="magick-image.php#MagickSetImagePixelInterpolateMethod">MagickSetImagePixelInterpolateMethod</a> &bull; <a href="magick-image.php#MagickSetImageIterations">MagickSetImageIterations</a> &bull; <a href="magick-image.php#MagickSetImageMatte">MagickSetImageMatte</a> &bull; <a href="magick-image.php#MagickSetImageMatteColor">MagickSetImageMatteColor</a> &bull; <a href="magick-image.php#MagickSetImageAlpha">MagickSetImageAlpha</a> &bull; <a href="magick-image.php#MagickSetImageOrientation">MagickSetImageOrientation</a> &bull; <a href="magick-image.php#MagickSetImagePage">MagickSetImagePage</a> &bull; <a href="magick-image.php#MagickSetImageProgressMonitor">MagickSetImageProgressMonitor</a> &bull; <a href="magick-image.php#MagickSetImageRedPrimary">MagickSetImageRedPrimary</a> &bull; <a href="magick-image.php#MagickSetImageRenderingIntent">MagickSetImageRenderingIntent</a> &bull; <a href="magick-image.php#MagickSetImageResolution">MagickSetImageResolution</a> &bull; <a href="magick-image.php#MagickSetImageScene">MagickSetImageScene</a> &bull; <a href="magick-image.php#MagickSetImageTicksPerSecond">MagickSetImageTicksPerSecond</a> &bull; <a href="magick-image.php#MagickSetImageType">MagickSetImageType</a> &bull; <a href="magick-image.php#MagickSetImageUnits">MagickSetImageUnits</a> &bull; <a href="magick-image.php#MagickSetImageVirtualPixelMethod">MagickSetImageVirtualPixelMethod</a> &bull; <a href="magick-image.php#MagickSetImageWhitePoint">MagickSetImageWhitePoint</a> &bull; <a href="magick-image.php#MagickShadeImage">MagickShadeImage</a> &bull; <a href="magick-image.php#MagickShadowImage">MagickShadowImage</a> &bull; <a href="magick-image.php#MagickSharpenImage">MagickSharpenImage</a> &bull; <a href="magick-image.php#MagickShaveImage">MagickShaveImage</a> &bull; <a href="magick-image.php#MagickShearImage">MagickShearImage</a> &bull; <a href="magick-image.php#MagickSigmoidalContrastImage">MagickSigmoidalContrastImage</a> &bull; <a href="magick-image.php#MagickSimilarityImage">MagickSimilarityImage</a> &bull; <a href="magick-image.php#MagickSketchImage">MagickSketchImage</a> &bull; <a href="magick-image.php#MagickSmushImages">MagickSmushImages</a> &bull; <a href="magick-image.php#MagickSolarizeImage">MagickSolarizeImage</a> &bull; <a href="magick-image.php#MagickSparseColorImage">MagickSparseColorImage</a> &bull; <a href="magick-image.php#MagickSpliceImage">MagickSpliceImage</a> &bull; <a href="magick-image.php#MagickSpreadImage">MagickSpreadImage</a> &bull; <a href="magick-image.php#MagickStatisticImage">MagickStatisticImage</a> &bull; <a href="magick-image.php#MagickSteganoImage">MagickSteganoImage</a> &bull; <a href="magick-image.php#MagickStereoImage">MagickStereoImage</a> &bull; <a href="magick-image.php#MagickStripImage">MagickStripImage</a> &bull; <a href="magick-image.php#MagickSwirlImage">MagickSwirlImage</a> &bull; <a href="magick-image.php#MagickTextureImage">MagickTextureImage</a> &bull; <a href="magick-image.php#MagickThresholdImage">MagickThresholdImage</a> &bull; <a href="magick-image.php#MagickThumbnailImage">MagickThumbnailImage</a> &bull; <a href="magick-image.php#MagickTintImage">MagickTintImage</a> &bull; <a href="magick-image.php#MagickTransformImage">MagickTransformImage</a> &bull; <a href="magick-image.php#MagickTransformImageColorspace">MagickTransformImageColorspace</a> &bull; <a href="magick-image.php#MagickTransparentPaintImage">MagickTransparentPaintImage</a> &bull; <a href="magick-image.php#MagickTransposeImage">MagickTransposeImage</a> &bull; <a href="magick-image.php#MagickTransverseImage">MagickTransverseImage</a> &bull; <a href="magick-image.php#MagickTrimImage">MagickTrimImage</a> &bull; <a href="magick-image.php#MagickUniqueImageColors">MagickUniqueImageColors</a> &bull; <a href="magick-image.php#MagickUnsharpMaskImage">MagickUnsharpMaskImage</a> &bull; <a href="magick-image.php#MagickVignetteImage">MagickVignetteImage</a> &bull; <a href="magick-image.php#MagickWaveImage">MagickWaveImage</a> &bull; <a href="magick-image.php#MagickWhiteThresholdImage">MagickWhiteThresholdImage</a> &bull; <a href="magick-image.php#MagickWriteImage">MagickWriteImage</a> &bull; <a href="magick-image.php#MagickWriteImageFile">MagickWriteImageFile</a> &bull; <a href="magick-image.php#MagickWriteImages">MagickWriteImages</a> &bull; <a href="magick-image.php#MagickWriteImagesFile">MagickWriteImagesFile</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="GetImageFromMagickWand">GetImageFromMagickWand</a></h2>

<p>GetImageFromMagickWand() returns the current image from the magick wand.</p>

<p>The format of the GetImageFromMagickWand method is:</p>

<pre class="text">
Image *GetImageFromMagickWand(const MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAdaptiveBlurImage">MagickAdaptiveBlurImage</a></h2>

<p>MagickAdaptiveBlurImage() adaptively blurs the image by blurring less intensely near image edges and more intensely far from edges. We blur the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and MagickAdaptiveBlurImage() selects a suitable radius for you.</p>

<p>The format of the MagickAdaptiveBlurImage method is:</p>

<pre class="text">
MagickBooleanType MagickAdaptiveBlurImage(MagickWand *wand,
  const double radius,const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAdaptiveResizeImage">MagickAdaptiveResizeImage</a></h2>

<p>MagickAdaptiveResizeImage() adaptively resize image with data dependent triangulation.</p>

<p>MagickBooleanType MagickAdaptiveResizeImage(MagickWand *wand, const size_t columns,const size_t rows)</p>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAdaptiveSharpenImage">MagickAdaptiveSharpenImage</a></h2>

<p>MagickAdaptiveSharpenImage() adaptively sharpens the image by sharpening more intensely near image edges and less intensely far from edges. We sharpen the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and MagickAdaptiveSharpenImage() selects a suitable radius for you.</p>

<p>The format of the MagickAdaptiveSharpenImage method is:</p>

<pre class="text">
MagickBooleanType MagickAdaptiveSharpenImage(MagickWand *wand,
  const double radius,const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAdaptiveThresholdImage">MagickAdaptiveThresholdImage</a></h2>

<p>MagickAdaptiveThresholdImage() selects an individual threshold for each pixel based on the range of intensity values in its local neighborhood.  This allows for thresholding of an image whose global intensity histogram doesn't contain distinctive peaks.</p>

<p>The format of the AdaptiveThresholdImage method is:</p>

<pre class="text">
MagickBooleanType MagickAdaptiveThresholdImage(MagickWand *wand,
  const size_t width,const size_t height,const double bias)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the width of the local neighborhood. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the height of the local neighborhood. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>the mean bias. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAddImage">MagickAddImage</a></h2>

<p>MagickAddImage() adds a clone of the images from the second wand and inserts them into the first wand.</p>

<p>Use MagickSetLastIterator(), to append new images into an existing wand, current image will be set to last image so later adds with also be appened to end of wand.</p>

<p>Use MagickSetFirstIterator() to prepend new images into wand, any more images added will also be prepended before other images in the wand. However the order of a list of new images will not change.</p>

<p>Otherwise the new images will be inserted just after the current image, and any later image will also be added after this current image but before the previously added images.  Caution is advised when multiple image adds are inserted into the middle of the wand image list.</p>

<p>The format of the MagickAddImage method is:</p>

<pre class="text">
MagickBooleanType MagickAddImage(MagickWand *wand,
  const MagickWand *add_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>add_wand</dt>
<dd>A wand that contains the image list to be added </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAddNoiseImage">MagickAddNoiseImage</a></h2>

<p>MagickAddNoiseImage() adds random noise to the image.</p>

<p>The format of the MagickAddNoiseImage method is:</p>

<pre class="text">
MagickBooleanType MagickAddNoiseImage(MagickWand *wand,
  const NoiseType noise_type,const double attenuate)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>noise_type</dt>
<dd> The type of noise: Uniform, Gaussian, Multiplicative, Impulse, Laplacian, or Poisson. </dd>

<dd> </dd>
<dt>attenuate</dt>
<dd> attenuate the random distribution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAffineTransformImage">MagickAffineTransformImage</a></h2>

<p>MagickAffineTransformImage() transforms an image as dictated by the affine matrix of the drawing wand.</p>

<p>The format of the MagickAffineTransformImage method is:</p>

<pre class="text">
MagickBooleanType MagickAffineTransformImage(MagickWand *wand,
  const DrawingWand *drawing_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>drawing_wand</dt>
<dd>the draw wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAnnotateImage">MagickAnnotateImage</a></h2>

<p>MagickAnnotateImage() annotates an image with text.</p>

<p>The format of the MagickAnnotateImage method is:</p>

<pre class="text">
MagickBooleanType MagickAnnotateImage(MagickWand *wand,
  const DrawingWand *drawing_wand,const double x,const double y,
  const double angle,const char *text)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>drawing_wand</dt>
<dd>the draw wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate to left of text </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate to text baseline </dd>

<dd> </dd>
<dt>angle</dt>
<dd>rotate text relative to this angle. </dd>

<dd> </dd>
<dt>text</dt>
<dd>text to draw </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAnimateImages">MagickAnimateImages</a></h2>

<p>MagickAnimateImages() animates an image or image sequence.</p>

<p>The format of the MagickAnimateImages method is:</p>

<pre class="text">
MagickBooleanType MagickAnimateImages(MagickWand *wand,
  const char *server_name)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>server_name</dt>
<dd>the X server name. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAppendImages">MagickAppendImages</a></h2>

<p>MagickAppendImages() append the images in a wand from the current image onwards, creating a new wand with the single image result.  This is affected by the gravity and background settings of the first image.</p>

<p>Typically you would call either MagickResetIterator() or MagickSetFirstImage() before calling this function to ensure that all the images in the wand's image list will be appended together.</p>

<p>The format of the MagickAppendImages method is:</p>

<pre class="text">
MagickWand *MagickAppendImages(MagickWand *wand,
  const MagickBooleanType stack)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>stack</dt>
<dd>By default, images are stacked left-to-right. Set stack to MagickTrue to stack them top-to-bottom. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAutoGammaImage">MagickAutoGammaImage</a></h2>

<p>MagickAutoGammaImage() extracts the 'mean' from the image and adjust the image to try make set its gamma appropriatally.</p>

<p>The format of the MagickAutoGammaImage method is:</p>

<pre class="text">
MagickBooleanType MagickAutoGammaImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAutoLevelImage">MagickAutoLevelImage</a></h2>

<p>MagickAutoLevelImage() adjusts the levels of a particular image channel by scaling the minimum and maximum values to the full quantum range.</p>

<p>The format of the MagickAutoLevelImage method is:</p>

<pre class="text">
MagickBooleanType MagickAutoLevelImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickAutoOrientImage">MagickAutoOrientImage</a></h2>

<p>MagickAutoOrientImage() adjusts an image so that its orientation is suitable $  for viewing (i.e. top-left orientation).</p>

<p>The format of the MagickAutoOrientImage method is:</p>

<pre class="text">
MagickBooleanType MagickAutoOrientImage(MagickWand *image)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickBlackThresholdImage">MagickBlackThresholdImage</a></h2>

<p>MagickBlackThresholdImage() is like MagickThresholdImage() but  forces all pixels below the threshold into black while leaving all pixels above the threshold unchanged.</p>

<p>The format of the MagickBlackThresholdImage method is:</p>

<pre class="text">
MagickBooleanType MagickBlackThresholdImage(MagickWand *wand,
  const PixelWand *threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickBlueShiftImage">MagickBlueShiftImage</a></h2>

<p>MagickBlueShiftImage() mutes the colors of the image to simulate a scene at nighttime in the moonlight.</p>

<p>The format of the MagickBlueShiftImage method is:</p>

<pre class="text">
MagickBooleanType MagickBlueShiftImage(MagickWand *wand,
  const double factor)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>factor</dt>
<dd>the blue shift factor (default 1.5) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickBlurImage">MagickBlurImage</a></h2>

<p>MagickBlurImage() blurs an image.  We convolve the image with a gaussian operator of the given radius and standard deviation (sigma). For reasonable results, the radius should be larger than sigma.  Use a radius of 0 and BlurImage() selects a suitable radius for you.</p>

<p>The format of the MagickBlurImage method is:</p>

<pre class="text">
MagickBooleanType MagickBlurImage(MagickWand *wand,const double radius,
  const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the , in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the , in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickBorderImage">MagickBorderImage</a></h2>

<p>MagickBorderImage() surrounds the image with a border of the color defined by the bordercolor pixel wand.</p>

<p>The format of the MagickBorderImage method is:</p>

<pre class="text">
MagickBooleanType MagickBorderImage(MagickWand *wand,
  const PixelWand *bordercolor,const size_t width,
  const size_t height,const CompositeOperator compose)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>bordercolor</dt>
<dd>the border color pixel wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the border width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the border height. </dd>

<dd> </dd>
<dt>compose</dt>
<dd>the composite operator. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="Use_MagickBrightnessContrastImage">Use MagickBrightnessContrastImage</a></h2>

<p>Use MagickBrightnessContrastImage() to change the brightness and/or contrast of an image.  It converts the brightness and contrast parameters into slope and intercept and calls a polynomical function to apply to the image.</p>


<p>The format of the MagickBrightnessContrastImage method is:</p>

<pre class="text">
MagickBooleanType MagickBrightnessContrastImage(MagickWand *wand,
  const double brightness,const double contrast)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>brightness</dt>
<dd>the brightness percent (-100 .. 100). </dd>

<dd> </dd>
<dt>contrast</dt>
<dd>the contrast percent (-100 .. 100). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickChannelFxImage">MagickChannelFxImage</a></h2>

<p>MagickChannelFxImage() applies a channel expression to the specified image. The expression consists of one or more channels, either mnemonic or numeric (e.g. red, 1), separated by actions as follows:</p>

<dd>
</dd>

<dd> &lt;=&gt;     exchange two channels (e.g. red&lt;=&gt;blue) =&gt;      transfer a channel to another (e.g. red=&gt;green) ,       separate channel operations (e.g. red, green) |       read channels from next input image (e.g. red | green) ;       write channels to next output image (e.g. red; green; blue) </dd>

<dd> A channel without a operation symbol implies extract. For example, to create 3 grayscale images from the red, green, and blue channels of an image, use: </dd>

<pre class="text">
    -channel-fx "red; green; blue"
</pre>

<p>The format of the MagickChannelFxImage method is: </dd>

<pre class="text">
MagickWand *MagickChannelFxImage(MagickWand *wand,const char *expression)
</pre>

<p>A description of each parameter follows: </dd>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>expression</dt>
<dd>the expression. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCharcoalImage">MagickCharcoalImage</a></h2>

<p>MagickCharcoalImage() simulates a charcoal drawing.</p>

<p>The format of the MagickCharcoalImage method is:</p>

<pre class="text">
MagickBooleanType MagickCharcoalImage(MagickWand *wand,
  const double radius,const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickChopImage">MagickChopImage</a></h2>

<p>MagickChopImage() removes a region of an image and collapses the image to occupy the removed portion</p>

<p>The format of the MagickChopImage method is:</p>

<pre class="text">
MagickBooleanType MagickChopImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the region width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the region height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the region x offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the region y offset. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickClampImage">MagickClampImage</a></h2>

<p>MagickClampImage() restricts the color range from 0 to the quantum depth.</p>

<p>The format of the MagickClampImage method is:</p>

<pre class="text">
MagickBooleanType MagickClampImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>channel</dt>
<dd>the channel. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickClipImage">MagickClipImage</a></h2>

<p>MagickClipImage() clips along the first path from the 8BIM profile, if present.</p>

<p>The format of the MagickClipImage method is:</p>

<pre class="text">
MagickBooleanType MagickClipImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickClipImagePath">MagickClipImagePath</a></h2>

<p>MagickClipImagePath() clips along the named paths from the 8BIM profile, if present. Later operations take effect inside the path.  Id may be a number if preceded with #, to work on a numbered path, e.g., "#1" to use the first path.</p>

<p>The format of the MagickClipImagePath method is:</p>

<pre class="text">
MagickBooleanType MagickClipImagePath(MagickWand *wand,
  const char *pathname,const MagickBooleanType inside)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>pathname</dt>
<dd>name of clipping path resource. If name is preceded by #, use clipping path numbered by name. </dd>

<dd> </dd>
<dt>inside</dt>
<dd>if non-zero, later operations take effect inside clipping path. Otherwise later operations take effect outside clipping path. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickClutImage">MagickClutImage</a></h2>

<p>MagickClutImage() replaces colors in the image from a color lookup table.</p>

<p>The format of the MagickClutImage method is:</p>

<pre class="text">
MagickBooleanType MagickClutImage(MagickWand *wand,
  const MagickWand *clut_wand,const PixelInterpolateMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>clut_image</dt>
<dd>the clut image. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCoalesceImages">MagickCoalesceImages</a></h2>

<p>MagickCoalesceImages() composites a set of images while respecting any page offsets and disposal methods.  GIF, MIFF, and MNG animation sequences typically start with an image background and each subsequent image varies in size and offset.  MagickCoalesceImages() returns a new sequence where each image in the sequence is the same size as the first and composited with the next image in the sequence.</p>

<p>The format of the MagickCoalesceImages method is:</p>

<pre class="text">
MagickWand *MagickCoalesceImages(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickColorDecisionListImage">MagickColorDecisionListImage</a></h2>

<p>MagickColorDecisionListImage() accepts a lightweight Color Correction Collection (CCC) file which solely contains one or more color corrections and applies the color correction to the image.  Here is a sample CCC file:</p>

<pre class="text">
    &lt;ColorCorrectionCollection xmlns="urn:ASC:CDL:v1.2"&gt;
    &lt;ColorCorrection id="cc03345"&gt;
          &lt;SOPNode&gt;
               &lt;Slope&gt; 0.9 1.2 0.5 &lt;/Slope&gt;
               &lt;Offset&gt; 0.4 -0.5 0.6 &lt;/Offset&gt;
               &lt;Power&gt; 1.0 0.8 1.5 &lt;/Power&gt;
          &lt;/SOPNode&gt;
          &lt;SATNode&gt;
               &lt;Saturation&gt; 0.85 &lt;/Saturation&gt;
          &lt;/SATNode&gt;
    &lt;/ColorCorrection&gt;
    &lt;/ColorCorrectionCollection&gt;
</pre>

<p>which includes the offset, slope, and power for each of the RGB channels as well as the saturation.</p>

<p>The format of the MagickColorDecisionListImage method is:</p>

<pre class="text">
MagickBooleanType MagickColorDecisionListImage(MagickWand *wand,
  const char *color_correction_collection)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>color_correction_collection</dt>
<dd>the color correction collection in XML. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickColorizeImage">MagickColorizeImage</a></h2>

<p>MagickColorizeImage() blends the fill color with each pixel in the image.</p>

<p>The format of the MagickColorizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickColorizeImage(MagickWand *wand,
  const PixelWand *colorize,const PixelWand *blend)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>colorize</dt>
<dd>the colorize pixel wand. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>the alpha pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickColorMatrixImage">MagickColorMatrixImage</a></h2>

<p>MagickColorMatrixImage() apply color transformation to an image. The method permits saturation changes, hue rotation, luminance to alpha, and various other effects.  Although variable-sized transformation matrices can be used, typically one uses a 5x5 matrix for an RGBA image and a 6x6 for CMYKA (or RGBA with offsets).  The matrix is similar to those used by Adobe Flash except offsets are in column 6 rather than 5 (in support of CMYKA images) and offsets are normalized (divide Flash offset by 255).</p>

<p>The format of the MagickColorMatrixImage method is:</p>

<pre class="text">
MagickBooleanType MagickColorMatrixImage(MagickWand *wand,
  const KernelInfo *color_matrix)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>color_matrix</dt>
<dd> the color matrix. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCombineImages">MagickCombineImages</a></h2>

<p>MagickCombineImages() combines one or more images into a single image.  The grayscale value of the pixels of each image in the sequence is assigned in order to the specified  hannels of the combined image.   The typical ordering would be image 1 =&gt; Red, 2 =&gt; Green, 3 =&gt; Blue, etc.</p>

<p>The format of the MagickCombineImages method is:</p>

<pre class="text">
MagickWand *MagickCombineImages(MagickWand *wand,
  const ColorspaceType colorspace)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>colorspace</dt>
<dd>the colorspace. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCommentImage">MagickCommentImage</a></h2>

<p>MagickCommentImage() adds a comment to your image.</p>

<p>The format of the MagickCommentImage method is:</p>

<pre class="text">
MagickBooleanType MagickCommentImage(MagickWand *wand,
  const char *comment)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>comment</dt>
<dd>the image comment. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCompareImagesLayers">MagickCompareImagesLayers</a></h2>

<p>MagickCompareImagesLayers() compares each image with the next in a sequence and returns the maximum bounding region of any pixel differences it discovers.</p>

<p>The format of the MagickCompareImagesLayers method is:</p>

<pre class="text">
MagickWand *MagickCompareImagesLayers(MagickWand *wand,
  const LayerMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the compare method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCompareImages">MagickCompareImages</a></h2>

<p>MagickCompareImages() compares an image to a reconstructed image and returns the specified difference image.</p>

<p>The format of the MagickCompareImages method is:</p>

<pre class="text">
MagickWand *MagickCompareImages(MagickWand *wand,
  const MagickWand *reference,const MetricType metric,
  double *distortion)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>reference</dt>
<dd>the reference wand. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd> </dd>
<dt>distortion</dt>
<dd>the computed distortion between the images. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCompositeImage">MagickCompositeImage</a></h2>

<p>MagickCompositeImage() composite one image onto another at the specified offset.</p>

<p>The format of the MagickCompositeImage method is:</p>

<pre class="text">
MagickBooleanType MagickCompositeImage(MagickWand *wand,
  const MagickWand *source_wand,const CompositeOperator compose,
  const MagickBooleanType clip_to_self,const ssize_t x,const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand holding the destination images </dd>

<dd> </dd>
<dt>source_image</dt>
<dd>the magick wand holding source image. </dd>

<dd> </dd>
<dt>compose</dt>
<dd>This operator affects how the composite is applied to the image.  The default is Over.  These are some of the compose methods availble. </dd>

<dd> OverCompositeOp       InCompositeOp         OutCompositeOp AtopCompositeOp       XorCompositeOp        PlusCompositeOp MinusCompositeOp      AddCompositeOp        SubtractCompositeOp DifferenceCompositeOp BumpmapCompositeOp    CopyCompositeOp DisplaceCompositeOp </dd>

<dd> </dd>
<dt>clip_to_self</dt>
<dd>set to MagickTrue to limit composition to area composed. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the column offset of the composited image. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the row offset of the composited image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCompositeImageGravity">MagickCompositeImageGravity</a></h2>

<p>MagickCompositeImageGravity() composite one image onto another using the specified gravity.</p>

<p>The format of the MagickCompositeImageGravity method is:</p>

<pre class="text">
MagickBooleanType MagickCompositeImageGravity(MagickWand *wand,
  const MagickWand *source_wand,const CompositeOperator compose,
  const GravityType gravity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand holding the destination images </dd>

<dd> </dd>
<dt>source_image</dt>
<dd>the magick wand holding source image. </dd>

<dd> </dd>
<dt>compose</dt>
<dd>This operator affects how the composite is applied to the image.  The default is Over.  These are some of the compose methods availble. </dd>

<dd> OverCompositeOp       InCompositeOp         OutCompositeOp AtopCompositeOp       XorCompositeOp        PlusCompositeOp MinusCompositeOp      AddCompositeOp        SubtractCompositeOp DifferenceCompositeOp BumpmapCompositeOp    CopyCompositeOp DisplaceCompositeOp </dd>

<dd> </dd>
<dt>gravity</dt>
<dd>positioning gravity (NorthWestGravity, NorthGravity, NorthEastGravity, WestGravity, CenterGravity, EastGravity, SouthWestGravity, SouthGravity, SouthEastGravity) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCompositeLayers">MagickCompositeLayers</a></h2>

<p>MagickCompositeLayers() composite the images in the source wand over the images in the destination wand in sequence, starting with the current image in both lists.</p>

<p>Each layer from the two image lists are composted together until the end of one of the image lists is reached.  The offset of each composition is also adjusted to match the virtual canvas offsets of each layer. As such the given offset is relative to the virtual canvas, and not the actual image.</p>

<p>Composition uses given x and y offsets, as the 'origin' location of the source images virtual canvas (not the real image) allowing you to compose a list of 'layer images' into the destiantioni images.  This makes it well sutiable for directly composing 'Clears Frame Animations' or 'Coaleased Animations' onto a static or other 'Coaleased Animation' destination image list.  GIF disposal handling is not looked at.</p>

<p>Special case:- If one of the image sequences is the last image (just a single image remaining), that image is repeatally composed with all the images in the other image list.  Either the source or destination lists may be the single image, for this situation.</p>

<p>In the case of a single destination image (or last image given), that image will ve cloned to match the number of images remaining in the source image list.</p>

<p>This is equivelent to the "-layer Composite" Shell API operator.</p>

<p>The format of the MagickCompositeLayers method is:</p>

<pre class="text">
MagickBooleanType MagickCompositeLayers(MagickWand *wand,
  const MagickWand *source_wand, const CompositeOperator compose,
  const ssize_t x,const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand holding destaintion images </dd>

<dd> </dd>
<dt>source_wand</dt>
<dd>the wand holding the source images </dd>

<dd> </dd>
<dt>compose, x, y</dt>
<dd> composition arguments </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickContrastImage">MagickContrastImage</a></h2>

<p>MagickContrastImage() enhances the intensity differences between the lighter and darker elements of the image.  Set sharpen to a value other than 0 to increase the image contrast otherwise the contrast is reduced.</p>

<p>The format of the MagickContrastImage method is:</p>

<pre class="text">
MagickBooleanType MagickContrastImage(MagickWand *wand,
  const MagickBooleanType sharpen)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>sharpen</dt>
<dd>Increase or decrease image contrast. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickContrastStretchImage">MagickContrastStretchImage</a></h2>

<p>MagickContrastStretchImage() enhances the contrast of a color image by adjusting the pixels color to span the entire range of colors available. You can also reduce the influence of a particular channel with a gamma value of 0.</p>

<p>The format of the MagickContrastStretchImage method is:</p>

<pre class="text">
MagickBooleanType MagickContrastStretchImage(MagickWand *wand,
  const double black_point,const double white_point)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>black_point</dt>
<dd>the black point. </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>the white point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickConvolveImage">MagickConvolveImage</a></h2>

<p>MagickConvolveImage() applies a custom convolution kernel to the image.</p>

<p>The format of the MagickConvolveImage method is:</p>

<pre class="text">
MagickBooleanType MagickConvolveImage(MagickWand *wand,
  const KernelInfo *kernel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>kernel</dt>
<dd>An array of doubles representing the convolution kernel. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCropImage">MagickCropImage</a></h2>

<p>MagickCropImage() extracts a region of the image.</p>

<p>The format of the MagickCropImage method is:</p>

<pre class="text">
MagickBooleanType MagickCropImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the region width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the region height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the region x-offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the region y-offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickCycleColormapImage">MagickCycleColormapImage</a></h2>

<p>MagickCycleColormapImage() displaces an image's colormap by a given number of positions.  If you cycle the colormap a number of times you can produce a psychodelic effect.</p>

<p>The format of the MagickCycleColormapImage method is:</p>

<pre class="text">
MagickBooleanType MagickCycleColormapImage(MagickWand *wand,
  const ssize_t displace)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>pixel_wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickConstituteImage">MagickConstituteImage</a></h2>

<p>MagickConstituteImage() adds an image to the wand comprised of the pixel data you supply.  The pixel data must be in scanline order top-to-bottom. The data can be char, short int, int, float, or double.  Float and double require the pixels to be normalized [0..1], otherwise [0..Max],  where Max is the maximum value the type can accomodate (e.g. 255 for char).  For example, to create a 640x480 image from unsigned red-green-blue character data, use</p>

<p>MagickConstituteImage(wand,640,480,"RGB",CharPixel,pixels);</p>

<p>The format of the MagickConstituteImage method is:</p>

<pre class="text">
MagickBooleanType MagickConstituteImage(MagickWand *wand,
  const size_t columns,const size_t rows,const char *map,
  const StorageType storage,void *pixels)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>width in pixels of the image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>height in pixels of the image. </dd>

<dd> </dd>
<dt>map</dt>
<dd> This string reflects the expected ordering of the pixel array. It can be any combination or order of R = red, G = green, B = blue, A = alpha (0 is transparent), O = alpha (0 is opaque), C = cyan, Y = yellow, M = magenta, K = black, I = intensity (for grayscale), P = pad. </dd>

<dd> </dd>
<dt>storage</dt>
<dd>Define the data type of the pixels.  Float and double types are expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from these types: CharPixel, DoublePixel, FloatPixel, IntegerPixel, LongPixel, QuantumPixel, or ShortPixel. </dd>

<dd> </dd>
<dt>pixels</dt>
<dd>This array of values contain the pixel components as defined by map and type.  You must preallocate this array where the expected length varies depending on the values of width, height, map, and type. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDecipherImage">MagickDecipherImage</a></h2>

<p>MagickDecipherImage() converts cipher pixels to plain pixels.</p>

<p>The format of the MagickDecipherImage method is:</p>

<pre class="text">
MagickBooleanType MagickDecipherImage(MagickWand *wand,
  const char *passphrase)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>passphrase</dt>
<dd>the passphrase. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDeconstructImages">MagickDeconstructImages</a></h2>

<p>MagickDeconstructImages() compares each image with the next in a sequence and returns the maximum bounding region of any pixel differences it discovers.</p>

<p>The format of the MagickDeconstructImages method is:</p>

<pre class="text">
MagickWand *MagickDeconstructImages(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDeskewImage">MagickDeskewImage</a></h2>

<p>MagickDeskewImage() removes skew from the image.  Skew is an artifact that occurs in scanned images because of the camera being misaligned, imperfections in the scanning or surface, or simply because the paper was not placed completely flat when scanned.</p>

<p>The format of the MagickDeskewImage method is:</p>

<pre class="text">
MagickBooleanType MagickDeskewImage(MagickWand *wand,
  const double threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>separate background from foreground. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDespeckleImage">MagickDespeckleImage</a></h2>

<p>MagickDespeckleImage() reduces the speckle noise in an image while perserving the edges of the original image.</p>

<p>The format of the MagickDespeckleImage method is:</p>

<pre class="text">
MagickBooleanType MagickDespeckleImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDestroyImage">MagickDestroyImage</a></h2>

<p>MagickDestroyImage() dereferences an image, deallocating memory associated with the image if the reference count becomes zero.</p>

<p>The format of the MagickDestroyImage method is:</p>

<pre class="text">
Image *MagickDestroyImage(Image *image)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDisplayImage">MagickDisplayImage</a></h2>

<p>MagickDisplayImage() displays an image.</p>

<p>The format of the MagickDisplayImage method is:</p>

<pre class="text">
MagickBooleanType MagickDisplayImage(MagickWand *wand,
  const char *server_name)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>server_name</dt>
<dd>the X server name. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDisplayImages">MagickDisplayImages</a></h2>

<p>MagickDisplayImages() displays an image or image sequence.</p>

<p>The format of the MagickDisplayImages method is:</p>

<pre class="text">
MagickBooleanType MagickDisplayImages(MagickWand *wand,
  const char *server_name)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>server_name</dt>
<dd>the X server name. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDistortImage">MagickDistortImage</a></h2>

<p>MagickDistortImage() distorts an image using various distortion methods, by mapping color lookups of the source image to a new destination image usally of the same size as the source image, unless 'bestfit' is set to true.</p>

<p>If 'bestfit' is enabled, and distortion allows it, the destination image is adjusted to ensure the whole source 'image' will just fit within the final destination image, which will be sized and offset accordingly.  Also in many cases the virtual offset of the source image will be taken into account in the mapping.</p>

<p>The format of the MagickDistortImage method is:</p>

<pre class="text">
MagickBooleanType MagickDistortImage(MagickWand *wand,
  const DistortImageMethod method,const size_t number_arguments,
  const double *arguments,const MagickBooleanType bestfit)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image to be distorted. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the method of image distortion. </dd>

<dd> ArcDistortion always ignores the source image offset, and always 'bestfit' the destination image with the top left corner offset relative to the polar mapping center. </dd>

<dd> Bilinear has no simple inverse mapping so it does not allow 'bestfit' style of image distortion. </dd>

<dd> Affine, Perspective, and Bilinear, do least squares fitting of the distortion when more than the minimum number of control point pairs are provided. </dd>

<dd> Perspective, and Bilinear, falls back to a Affine distortion when less that 4 control point pairs are provided. While Affine distortions let you use any number of control point pairs, that is Zero pairs is a no-Op (viewport only) distrotion, one pair is a translation and two pairs of control points do a scale-rotate-translate, without any shearing. </dd>

<dd> </dd>
<dt>number_arguments</dt>
<dd>the number of arguments given for this distortion method. </dd>

<dd> </dd>
<dt>arguments</dt>
<dd>the arguments for this distortion method. </dd>

<dd> </dd>
<dt>bestfit</dt>
<dd>Attempt to resize destination to fit distorted source. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickDrawImage">MagickDrawImage</a></h2>

<p>MagickDrawImage() renders the drawing wand on the current image.</p>

<p>The format of the MagickDrawImage method is:</p>

<pre class="text">
MagickBooleanType MagickDrawImage(MagickWand *wand,
  const DrawingWand *drawing_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>drawing_wand</dt>
<dd>the draw wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickEdgeImage">MagickEdgeImage</a></h2>

<p>MagickEdgeImage() enhance edges within the image with a convolution filter of the given radius.  Use a radius of 0 and Edge() selects a suitable radius for you.</p>

<p>The format of the MagickEdgeImage method is:</p>

<pre class="text">
MagickBooleanType MagickEdgeImage(MagickWand *wand,const double radius)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the pixel neighborhood. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickEmbossImage">MagickEmbossImage</a></h2>

<p>MagickEmbossImage() returns a grayscale image with a three-dimensional effect.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and Emboss() selects a suitable radius for you.</p>

<p>The format of the MagickEmbossImage method is:</p>

<pre class="text">
MagickBooleanType MagickEmbossImage(MagickWand *wand,const double radius,
  const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickEncipherImage">MagickEncipherImage</a></h2>

<p>MagickEncipherImage() converts plaint pixels to cipher pixels.</p>

<p>The format of the MagickEncipherImage method is:</p>

<pre class="text">
MagickBooleanType MagickEncipherImage(MagickWand *wand,
  const char *passphrase)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>passphrase</dt>
<dd>the passphrase. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickEnhanceImage">MagickEnhanceImage</a></h2>

<p>MagickEnhanceImage() applies a digital filter that improves the quality of a noisy image.</p>

<p>The format of the MagickEnhanceImage method is:</p>

<pre class="text">
MagickBooleanType MagickEnhanceImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickEqualizeImage">MagickEqualizeImage</a></h2>

<p>MagickEqualizeImage() equalizes the image histogram.</p>

<p>The format of the MagickEqualizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickEqualizeImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>channel</dt>
<dd>the image channel(s). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickEvaluateImage">MagickEvaluateImage</a></h2>

<p>MagickEvaluateImage() applys an arithmetic, relational, or logical expression to an image.  Use these operators to lighten or darken an image, to increase or decrease contrast in an image, or to produce the "negative" of an image.</p>

<p>The format of the MagickEvaluateImage method is:</p>

<pre class="text">
MagickBooleanType MagickEvaluateImage(MagickWand *wand,
  const MagickEvaluateOperator operator,const double value)
MagickBooleanType MagickEvaluateImages(MagickWand *wand,
  const MagickEvaluateOperator operator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>op</dt>
<dd>A channel operator. </dd>

<dd> </dd>
<dt>value</dt>
<dd>A value value. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickExportImagePixels">MagickExportImagePixels</a></h2>

<p>MagickExportImagePixels() extracts pixel data from an image and returns it to you.  The method returns MagickTrue on success otherwise MagickFalse if an error is encountered.  The data is returned as char, short int, int, ssize_t, float, or double in the order specified by map.</p>

<p>Suppose you want to extract the first scanline of a 640x480 image as character data in red-green-blue order:</p>

<pre class="text">
MagickExportImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
</pre>

<p>The format of the MagickExportImagePixels method is:</p>

<pre class="text">
MagickBooleanType MagickExportImagePixels(MagickWand *wand,
  const ssize_t x,const ssize_t y,const size_t columns,
  const size_t rows,const char *map,const StorageType storage,
  void *pixels)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x, y, columns, rows</dt>
<dd> These values define the perimeter of a region of pixels you want to extract. </dd>

<dd> </dd>
<dt>map</dt>
<dd> This string reflects the expected ordering of the pixel array. It can be any combination or order of R = red, G = green, B = blue, A = alpha (0 is transparent), O = alpha (0 is opaque), C = cyan, Y = yellow, M = magenta, K = black, I = intensity (for grayscale), P = pad. </dd>

<dd> </dd>
<dt>storage</dt>
<dd>Define the data type of the pixels.  Float and double types are expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from these types: CharPixel, DoublePixel, FloatPixel, IntegerPixel, LongPixel, QuantumPixel, or ShortPixel. </dd>

<dd> </dd>
<dt>pixels</dt>
<dd>This array of values contain the pixel components as defined by map and type.  You must preallocate this array where the expected length varies depending on the values of width, height, map, and type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickExtentImage">MagickExtentImage</a></h2>

<p>MagickExtentImage() extends the image as defined by the geometry, gravity, and wand background color.  Set the (x,y) offset of the geometry to move the original wand relative to the extended wand.</p>

<p>The format of the MagickExtentImage method is:</p>

<pre class="text">
MagickBooleanType MagickExtentImage(MagickWand *wand,const size_t width,
  const size_t height,const ssize_t x,const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the region width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the region height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the region x offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the region y offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickFlipImage">MagickFlipImage</a></h2>

<p>MagickFlipImage() creates a vertical mirror image by reflecting the pixels around the central x-axis.</p>

<p>The format of the MagickFlipImage method is:</p>

<pre class="text">
MagickBooleanType MagickFlipImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickFloodfillPaintImage">MagickFloodfillPaintImage</a></h2>

<p>MagickFloodfillPaintImage() changes the color value of any pixel that matches target and is an immediate neighbor.  If the method FillToBorderMethod is specified, the color value is changed for any neighbor pixel that does not match the bordercolor member of image.</p>

<p>The format of the MagickFloodfillPaintImage method is:</p>

<pre class="text">
MagickBooleanType MagickFloodfillPaintImage(MagickWand *wand,
  const PixelWand *fill,const double fuzz,const PixelWand *bordercolor,
  const ssize_t x,const ssize_t y,const MagickBooleanType invert)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>fill</dt>
<dd>the floodfill color pixel wand. </dd>

<dd> </dd>
<dt>fuzz</dt>
<dd>By default target must match a particular pixel color exactly.  However, in many cases two colors may differ by a small amount. The fuzz member of image defines how much tolerance is acceptable to consider two colors as the same.  For example, set fuzz to 10 and the color red at intensities of 100 and 102 respectively are now interpreted as the same color for the purposes of the floodfill. </dd>

<dd> </dd>
<dt>bordercolor</dt>
<dd>the border color pixel wand. </dd>

<dd> </dd>
<dt>x,y</dt>
<dd>the starting location of the operation. </dd>

<dd> </dd>
<dt>invert</dt>
<dd>paint any pixel that does not match the target color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickFlopImage">MagickFlopImage</a></h2>

<p>MagickFlopImage() creates a horizontal mirror image by reflecting the pixels around the central y-axis.</p>

<p>The format of the MagickFlopImage method is:</p>

<pre class="text">
MagickBooleanType MagickFlopImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickForwardFourierTransformImage">MagickForwardFourierTransformImage</a></h2>

<p>MagickForwardFourierTransformImage() implements the discrete Fourier transform (DFT) of the image either as a magnitude / phase or real / imaginary image pair.</p>

<p>The format of the MagickForwardFourierTransformImage method is:</p>

<pre class="text">
MagickBooleanType MagickForwardFourierTransformImage(MagickWand *wand,
  const MagickBooleanType magnitude)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>magnitude</dt>
<dd>if true, return as magnitude / phase pair otherwise a real / imaginary image pair. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickFrameImage">MagickFrameImage</a></h2>

<p>MagickFrameImage() adds a simulated three-dimensional border around the image.  The width and height specify the border width of the vertical and horizontal sides of the frame.  The inner and outer bevels indicate the width of the inner and outer shadows of the frame.</p>

<p>The format of the MagickFrameImage method is:</p>

<pre class="text">
MagickBooleanType MagickFrameImage(MagickWand *wand,
  const PixelWand *matte_color,const size_t width,
  const size_t height,const ssize_t inner_bevel,
  const ssize_t outer_bevel,const CompositeOperator compose)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>matte_color</dt>
<dd>the frame color pixel wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the border width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the border height. </dd>

<dd> </dd>
<dt>inner_bevel</dt>
<dd>the inner bevel width. </dd>

<dd> </dd>
<dt>outer_bevel</dt>
<dd>the outer bevel width. </dd>

<dd> </dd>
<dt>compose</dt>
<dd>the composite operator. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickFunctionImage">MagickFunctionImage</a></h2>

<p>MagickFunctionImage() applys an arithmetic, relational, or logical expression to an image.  Use these operators to lighten or darken an image, to increase or decrease contrast in an image, or to produce the "negative" of an image.</p>

<p>The format of the MagickFunctionImage method is:</p>

<pre class="text">
MagickBooleanType MagickFunctionImage(MagickWand *wand,
  const MagickFunction function,const size_t number_arguments,
  const double *arguments)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>function</dt>
<dd>the image function. </dd>

<dd> </dd>
<dt>number_arguments</dt>
<dd>the number of function arguments. </dd>

<dd> </dd>
<dt>arguments</dt>
<dd>the function arguments. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickFxImage">MagickFxImage</a></h2>

<p>MagickFxImage() evaluate expression for each pixel in the image.</p>

<p>The format of the MagickFxImage method is:</p>

<pre class="text">
MagickWand *MagickFxImage(MagickWand *wand,const char *expression)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>expression</dt>
<dd>the expression. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGammaImage">MagickGammaImage</a></h2>

<p>MagickGammaImage() gamma-corrects an image.  The same image viewed on different devices will have perceptual differences in the way the image's intensities are represented on the screen.  Specify individual gamma levels for the red, green, and blue channels, or adjust all three with the gamma parameter.  Values typically range from 0.8 to 2.3.</p>

<p>You can also reduce the influence of a particular channel with a gamma value of 0.</p>

<p>The format of the MagickGammaImage method is:</p>

<pre class="text">
MagickBooleanType MagickGammaImage(MagickWand *wand,const double gamma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>level</dt>
<dd>Define the level of gamma correction. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGaussianBlurImage">MagickGaussianBlurImage</a></h2>

<p>MagickGaussianBlurImage() blurs an image.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma). For reasonable results, the radius should be larger than sigma.  Use a radius of 0 and MagickGaussianBlurImage() selects a suitable radius for you.</p>

<p>The format of the MagickGaussianBlurImage method is:</p>

<pre class="text">
MagickBooleanType MagickGaussianBlurImage(MagickWand *wand,
  const double radius,const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImage">MagickGetImage</a></h2>

<p>MagickGetImage() gets the image at the current image index.</p>

<p>The format of the MagickGetImage method is:</p>

<pre class="text">
MagickWand *MagickGetImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageAlphaChannel">MagickGetImageAlphaChannel</a></h2>

<p>MagickGetImageAlphaChannel() returns MagickFalse if the image alpha channel is not activated.  That is, the image is RGB rather than RGBA or CMYK rather than CMYKA.</p>

<p>The format of the MagickGetImageAlphaChannel method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageAlphaChannel(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageMask">MagickGetImageMask</a></h2>

<p>MagickGetImageMask() gets the image clip mask at the current image index.</p>

<p>The format of the MagickGetImageMask method is:</p>

<pre class="text">
MagickWand *MagickGetImageMask(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageBackgroundColor">MagickGetImageBackgroundColor</a></h2>

<p>MagickGetImageBackgroundColor() returns the image background color.</p>

<p>The format of the MagickGetImageBackgroundColor method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageBackgroundColor(MagickWand *wand,
  PixelWand *background_color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>background_color</dt>
<dd>Return the background color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageBlob">MagickGetImageBlob</a></h2>

<p>MagickGetImageBlob() implements direct to memory image formats.  It returns the image as a blob (a formatted "file" in memory) and its length, starting from the current position in the image sequence.  Use MagickSetImageFormat() to set the format to write to the blob (GIF, JPEG,  PNG, etc.).</p>

<p>Utilize MagickResetIterator() to ensure the write is from the beginning of the image sequence.</p>

<p>Use MagickRelinquishMemory() to free the blob when you are done with it.</p>

<p>The format of the MagickGetImageBlob method is:</p>

<pre class="text">
unsigned char *MagickGetImageBlob(MagickWand *wand,size_t *length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the length of the blob. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageBlob">MagickGetImageBlob</a></h2>

<p>MagickGetImageBlob() implements direct to memory image formats.  It returns the image sequence as a blob and its length.  The format of the image determines the format of the returned blob (GIF, JPEG,  PNG, etc.).  To return a different image format, use MagickSetImageFormat().</p>

<p>Note, some image formats do not permit multiple images to the same image stream (e.g. JPEG).  in this instance, just the first image of the sequence is returned as a blob.</p>

<p>The format of the MagickGetImagesBlob method is:</p>

<pre class="text">
unsigned char *MagickGetImagesBlob(MagickWand *wand,size_t *length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the length of the blob. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageBluePrimary">MagickGetImageBluePrimary</a></h2>

<p>MagickGetImageBluePrimary() returns the chromaticy blue primary point for the image.</p>

<p>The format of the MagickGetImageBluePrimary method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageBluePrimary(MagickWand *wand,double *x,
  double *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the chromaticity blue primary x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the chromaticity blue primary y-point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageBorderColor">MagickGetImageBorderColor</a></h2>

<p>MagickGetImageBorderColor() returns the image border color.</p>

<p>The format of the MagickGetImageBorderColor method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageBorderColor(MagickWand *wand,
  PixelWand *border_color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>border_color</dt>
<dd>Return the border color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageFeatures">MagickGetImageFeatures</a></h2>

<p>MagickGetImageFeatures() returns features for each channel in the image in each of four directions (horizontal, vertical, left and right diagonals) for the specified distance.  The features include the angular second moment, contrast, correlation, sum of squares: variance, inverse difference moment, sum average, sum varience, sum entropy, entropy, difference variance, difference entropy, information measures of correlation 1, information measures of correlation 2, and maximum correlation coefficient.  You can access the red channel contrast, for example, like this:</p>

<pre class="text">
channel_features=MagickGetImageFeatures(wand,1);
contrast=channel_features[RedPixelChannel].contrast[0];
</pre>

<p>Use MagickRelinquishMemory() to free the statistics buffer.</p>

<p>The format of the MagickGetImageFeatures method is:</p>

<pre class="text">
ChannelFeatures *MagickGetImageFeatures(MagickWand *wand,
  const size_t distance)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>distance</dt>
<dd>the distance. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageKurtosis">MagickGetImageKurtosis</a></h2>

<p>MagickGetImageKurtosis() gets the kurtosis and skewness of one or more image channels.</p>

<p>The format of the MagickGetImageKurtosis method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageKurtosis(MagickWand *wand,
  double *kurtosis,double *skewness)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>kurtosis</dt>
<dd> The kurtosis for the specified channel(s). </dd>

<dd> </dd>
<dt>skewness</dt>
<dd> The skewness for the specified channel(s). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageMean">MagickGetImageMean</a></h2>

<p>MagickGetImageMean() gets the mean and standard deviation of one or more image channels.</p>

<p>The format of the MagickGetImageMean method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageMean(MagickWand *wand,double *mean,
  double *standard_deviation)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>channel</dt>
<dd>the image channel(s). </dd>

<dd> </dd>
<dt>mean</dt>
<dd> The mean pixel value for the specified channel(s). </dd>

<dd> </dd>
<dt>standard_deviation</dt>
<dd> The standard deviation for the specified channel(s). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageRange">MagickGetImageRange</a></h2>

<p>MagickGetImageRange() gets the range for one or more image channels.</p>

<p>The format of the MagickGetImageRange method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageRange(MagickWand *wand,double *minima,
  double *maxima)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>minima</dt>
<dd> The minimum pixel value for the specified channel(s). </dd>

<dd> </dd>
<dt>maxima</dt>
<dd> The maximum pixel value for the specified channel(s). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageStatistics">MagickGetImageStatistics</a></h2>

<p>MagickGetImageStatistics() returns statistics for each channel in the image.  The statistics include the channel depth, its minima and maxima, the mean, the standard deviation, the kurtosis and the skewness. You can access the red channel mean, for example, like this:</p>

<pre class="text">
channel_statistics=MagickGetImageStatistics(wand);
red_mean=channel_statistics[RedPixelChannel].mean;
</pre>

<p>Use MagickRelinquishMemory() to free the statistics buffer.</p>

<p>The format of the MagickGetImageStatistics method is:</p>

<pre class="text">
ChannelStatistics *MagickGetImageStatistics(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageColormapColor">MagickGetImageColormapColor</a></h2>

<p>MagickGetImageColormapColor() returns the color of the specified colormap index.</p>

<p>The format of the MagickGetImageColormapColor method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageColormapColor(MagickWand *wand,
  const size_t index,PixelWand *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>index</dt>
<dd>the offset into the image colormap. </dd>

<dd> </dd>
<dt>color</dt>
<dd>Return the colormap color in this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageColors">MagickGetImageColors</a></h2>

<p>MagickGetImageColors() gets the number of unique colors in the image.</p>

<p>The format of the MagickGetImageColors method is:</p>

<pre class="text">
size_t MagickGetImageColors(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageColorspace">MagickGetImageColorspace</a></h2>

<p>MagickGetImageColorspace() gets the image colorspace.</p>

<p>The format of the MagickGetImageColorspace method is:</p>

<pre class="text">
ColorspaceType MagickGetImageColorspace(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageCompose">MagickGetImageCompose</a></h2>

<p>MagickGetImageCompose() returns the composite operator associated with the image.</p>

<p>The format of the MagickGetImageCompose method is:</p>

<pre class="text">
CompositeOperator MagickGetImageCompose(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageCompression">MagickGetImageCompression</a></h2>

<p>MagickGetImageCompression() gets the image compression.</p>

<p>The format of the MagickGetImageCompression method is:</p>

<pre class="text">
CompressionType MagickGetImageCompression(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageCompressionQuality">MagickGetImageCompressionQuality</a></h2>

<p>MagickGetImageCompressionQuality() gets the image compression quality.</p>

<p>The format of the MagickGetImageCompressionQuality method is:</p>

<pre class="text">
size_t MagickGetImageCompressionQuality(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageDelay">MagickGetImageDelay</a></h2>

<p>MagickGetImageDelay() gets the image delay.</p>

<p>The format of the MagickGetImageDelay method is:</p>

<pre class="text">
size_t MagickGetImageDelay(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageDepth">MagickGetImageDepth</a></h2>

<p>MagickGetImageDepth() gets the image depth.</p>

<p>The format of the MagickGetImageDepth method is:</p>

<pre class="text">
size_t MagickGetImageDepth(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageDispose">MagickGetImageDispose</a></h2>

<p>MagickGetImageDispose() gets the image disposal method.</p>

<p>The format of the MagickGetImageDispose method is:</p>

<pre class="text">
DisposeType MagickGetImageDispose(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageDistortion">MagickGetImageDistortion</a></h2>

<p>MagickGetImageDistortion() compares an image to a reconstructed image and returns the specified distortion metric.</p>

<p>The format of the MagickGetImageDistortion method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageDistortion(MagickWand *wand,
  const MagickWand *reference,const MetricType metric,
  double *distortion)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>reference</dt>
<dd>the reference wand. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd> </dd>
<dt>distortion</dt>
<dd>the computed distortion between the images. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageDistortions">MagickGetImageDistortions</a></h2>

<p>MagickGetImageDistortions() compares one or more pixel channels of an image to a reconstructed image and returns the specified distortion metrics.</p>

<p>Use MagickRelinquishMemory() to free the metrics when you are done with them.</p>

<p>The format of the MagickGetImageDistortion method is:</p>

<pre class="text">
double *MagickGetImageDistortion(MagickWand *wand,
  const MagickWand *reference,const MetricType metric)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>reference</dt>
<dd>the reference wand. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageEndian">MagickGetImageEndian</a></h2>

<p>MagickGetImageEndian() gets the image endian.</p>

<p>The format of the MagickGetImageEndian method is:</p>

<pre class="text">
EndianType MagickGetImageEndian(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageFilename">MagickGetImageFilename</a></h2>

<p>MagickGetImageFilename() returns the filename of a particular image in a sequence.</p>

<p>The format of the MagickGetImageFilename method is:</p>

<pre class="text">
char *MagickGetImageFilename(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageFormat">MagickGetImageFormat</a></h2>

<p>MagickGetImageFormat() returns the format of a particular image in a sequence.</p>

<p>The format of the MagickGetImageFormat method is:</p>

<pre class="text">
char *MagickGetImageFormat(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageFuzz">MagickGetImageFuzz</a></h2>

<p>MagickGetImageFuzz() gets the image fuzz.</p>

<p>The format of the MagickGetImageFuzz method is:</p>

<pre class="text">
double MagickGetImageFuzz(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageGamma">MagickGetImageGamma</a></h2>

<p>MagickGetImageGamma() gets the image gamma.</p>

<p>The format of the MagickGetImageGamma method is:</p>

<pre class="text">
double MagickGetImageGamma(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageGravity">MagickGetImageGravity</a></h2>

<p>MagickGetImageGravity() gets the image gravity.</p>

<p>The format of the MagickGetImageGravity method is:</p>

<pre class="text">
GravityType MagickGetImageGravity(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageGreenPrimary">MagickGetImageGreenPrimary</a></h2>

<p>MagickGetImageGreenPrimary() returns the chromaticy green primary point.</p>

<p>The format of the MagickGetImageGreenPrimary method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageGreenPrimary(MagickWand *wand,double *x,
  double *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the chromaticity green primary x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the chromaticity green primary y-point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageHeight">MagickGetImageHeight</a></h2>

<p>MagickGetImageHeight() returns the image height.</p>

<p>The format of the MagickGetImageHeight method is:</p>

<pre class="text">
size_t MagickGetImageHeight(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageHistogram">MagickGetImageHistogram</a></h2>

<p>MagickGetImageHistogram() returns the image histogram as an array of PixelWand wands.</p>

<p>The format of the MagickGetImageHistogram method is:</p>

<pre class="text">
PixelWand **MagickGetImageHistogram(MagickWand *wand,
  size_t *number_colors)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>number_colors</dt>
<dd>the number of unique colors in the image and the number of pixel wands returned. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageInterlaceScheme">MagickGetImageInterlaceScheme</a></h2>

<p>MagickGetImageInterlaceScheme() gets the image interlace scheme.</p>

<p>The format of the MagickGetImageInterlaceScheme method is:</p>

<pre class="text">
InterlaceType MagickGetImageInterlaceScheme(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageInterpolateMethod">MagickGetImageInterpolateMethod</a></h2>

<p>MagickGetImageInterpolateMethod() returns the interpolation method for the sepcified image.</p>

<p>The format of the MagickGetImageInterpolateMethod method is:</p>

<pre class="text">
PixelInterpolateMethod MagickGetImagePixelInterpolateMethod(
  MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageIterations">MagickGetImageIterations</a></h2>

<p>MagickGetImageIterations() gets the image iterations.</p>

<p>The format of the MagickGetImageIterations method is:</p>

<pre class="text">
size_t MagickGetImageIterations(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageLength">MagickGetImageLength</a></h2>

<p>MagickGetImageLength() returns the image length in bytes.</p>

<p>The format of the MagickGetImageLength method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageLength(MagickWand *wand,
  MagickSizeType *length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the image length in bytes. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageMatteColor">MagickGetImageMatteColor</a></h2>

<p>MagickGetImageMatteColor() returns the image matte color.</p>

<p>The format of the MagickGetImageMatteColor method is:</p>

<pre class="text">
MagickBooleanType MagickGetImagematteColor(MagickWand *wand,
  PixelWand *matte_color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>matte_color</dt>
<dd>Return the matte color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageOrientation">MagickGetImageOrientation</a></h2>

<p>MagickGetImageOrientation() returns the image orientation.</p>

<p>The format of the MagickGetImageOrientation method is:</p>

<pre class="text">
OrientationType MagickGetImageOrientation(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImagePage">MagickGetImagePage</a></h2>

<p>MagickGetImagePage() returns the page geometry associated with the image.</p>

<p>The format of the MagickGetImagePage method is:</p>

<pre class="text">
MagickBooleanType MagickGetImagePage(MagickWand *wand,
  size_t *width,size_t *height,ssize_t *x,ssize_t *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the page width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the page height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the page x-offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the page y-offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImagePixelColor">MagickGetImagePixelColor</a></h2>

<p>MagickGetImagePixelColor() returns the color of the specified pixel.</p>

<p>The format of the MagickGetImagePixelColor method is:</p>

<pre class="text">
MagickBooleanType MagickGetImagePixelColor(MagickWand *wand,
  const ssize_t x,const ssize_t y,PixelWand *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x,y</dt>
<dd>the pixel offset into the image. </dd>

<dd> </dd>
<dt>color</dt>
<dd>Return the colormap color in this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageRedPrimary">MagickGetImageRedPrimary</a></h2>

<p>MagickGetImageRedPrimary() returns the chromaticy red primary point.</p>

<p>The format of the MagickGetImageRedPrimary method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageRedPrimary(MagickWand *wand,double *x,
  double *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the chromaticity red primary x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the chromaticity red primary y-point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageRegion">MagickGetImageRegion</a></h2>

<p>MagickGetImageRegion() extracts a region of the image and returns it as a a new wand.</p>

<p>The format of the MagickGetImageRegion method is:</p>

<pre class="text">
MagickWand *MagickGetImageRegion(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the region width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the region height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the region x offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the region y offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageRenderingIntent">MagickGetImageRenderingIntent</a></h2>

<p>MagickGetImageRenderingIntent() gets the image rendering intent.</p>

<p>The format of the MagickGetImageRenderingIntent method is:</p>

<pre class="text">
RenderingIntent MagickGetImageRenderingIntent(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageResolution">MagickGetImageResolution</a></h2>

<p>MagickGetImageResolution() gets the image X and Y resolution.</p>

<p>The format of the MagickGetImageResolution method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageResolution(MagickWand *wand,double *x,
  double *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the image x-resolution. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the image y-resolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageScene">MagickGetImageScene</a></h2>

<p>MagickGetImageScene() gets the image scene.</p>

<p>The format of the MagickGetImageScene method is:</p>

<pre class="text">
size_t MagickGetImageScene(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageSignature">MagickGetImageSignature</a></h2>

<p>MagickGetImageSignature() generates an SHA-256 message digest for the image pixel stream.</p>

<p>The format of the MagickGetImageSignature method is:</p>

<pre class="text">
char *MagickGetImageSignature(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageTicksPerSecond">MagickGetImageTicksPerSecond</a></h2>

<p>MagickGetImageTicksPerSecond() gets the image ticks-per-second.</p>

<p>The format of the MagickGetImageTicksPerSecond method is:</p>

<pre class="text">
size_t MagickGetImageTicksPerSecond(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageType">MagickGetImageType</a></h2>

<p>MagickGetImageType() gets the potential image type:</p>

<p>Bilevel        Grayscale       GrayscaleMatte Palette        PaletteMatte    TrueColor TrueColorMatte ColorSeparation ColorSeparationMatte</p>

<p>The format of the MagickGetImageType method is:</p>

<pre class="text">
ImageType MagickGetImageType(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageUnits">MagickGetImageUnits</a></h2>

<p>MagickGetImageUnits() gets the image units of resolution.</p>

<p>The format of the MagickGetImageUnits method is:</p>

<pre class="text">
ResolutionType MagickGetImageUnits(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageVirtualPixelMethod">MagickGetImageVirtualPixelMethod</a></h2>

<p>MagickGetImageVirtualPixelMethod() returns the virtual pixel method for the sepcified image.</p>

<p>The format of the MagickGetImageVirtualPixelMethod method is:</p>

<pre class="text">
VirtualPixelMethod MagickGetImageVirtualPixelMethod(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageWhitePoint">MagickGetImageWhitePoint</a></h2>

<p>MagickGetImageWhitePoint() returns the chromaticy white point.</p>

<p>The format of the MagickGetImageWhitePoint method is:</p>

<pre class="text">
MagickBooleanType MagickGetImageWhitePoint(MagickWand *wand,double *x,
  double *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the chromaticity white x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the chromaticity white y-point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageWidth">MagickGetImageWidth</a></h2>

<p>MagickGetImageWidth() returns the image width.</p>

<p>The format of the MagickGetImageWidth method is:</p>

<pre class="text">
size_t MagickGetImageWidth(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetNumberImages">MagickGetNumberImages</a></h2>

<p>MagickGetNumberImages() returns the number of images associated with a magick wand.</p>

<p>The format of the MagickGetNumberImages method is:</p>

<pre class="text">
size_t MagickGetNumberImages(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickGetImageTotalInkDensity">MagickGetImageTotalInkDensity</a></h2>

<p>MagickGetImageTotalInkDensity() gets the image total ink density.</p>

<p>The format of the MagickGetImageTotalInkDensity method is:</p>

<pre class="text">
double MagickGetImageTotalInkDensity(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickHaldClutImage">MagickHaldClutImage</a></h2>

<p>MagickHaldClutImage() replaces colors in the image from a Hald color lookup table.   A Hald color lookup table is a 3-dimensional color cube mapped to 2 dimensions.  Create it with the HALD coder.  You can apply any color transformation to the Hald image and then use this method to apply the transform to the image.</p>

<p>The format of the MagickHaldClutImage method is:</p>

<pre class="text">
MagickBooleanType MagickHaldClutImage(MagickWand *wand,
  const MagickWand *hald_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>hald_image</dt>
<dd>the hald CLUT image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickHasNextImage">MagickHasNextImage</a></h2>

<p>MagickHasNextImage() returns MagickTrue if the wand has more images when traversing the list in the forward direction</p>

<p>The format of the MagickHasNextImage method is:</p>

<pre class="text">
MagickBooleanType MagickHasNextImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickHasPreviousImage">MagickHasPreviousImage</a></h2>

<p>MagickHasPreviousImage() returns MagickTrue if the wand has more images when traversing the list in the reverse direction</p>

<p>The format of the MagickHasPreviousImage method is:</p>

<pre class="text">
MagickBooleanType MagickHasPreviousImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickIdentifyImage">MagickIdentifyImage</a></h2>

<p>MagickIdentifyImage() identifies an image by printing its attributes to the file.  Attributes include the image width, height, size, and others.</p>

<p>The format of the MagickIdentifyImage method is:</p>

<pre class="text">
const char *MagickIdentifyImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickIdentifyImageType">MagickIdentifyImageType</a></h2>

<p>MagickIdentifyImageType() gets the potential image type:</p>

<p>Bilevel        Grayscale       GrayscaleMatte Palette        PaletteMatte    TrueColor TrueColorMatte ColorSeparation ColorSeparationMatte</p>

<p>To ensure the image type matches its potential, use MagickSetImageType():</p>

<pre class="text">
    (void) MagickSetImageType(wand,MagickIdentifyImageType(wand));
</pre>

<p>The format of the MagickIdentifyImageType method is:</p>

<pre class="text">
ImageType MagickIdentifyImageType(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickImplodeImage">MagickImplodeImage</a></h2>

<p>MagickImplodeImage() creates a new image that is a copy of an existing one with the image pixels "implode" by the specified percentage.  It allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the MagickImplodeImage method is:</p>

<pre class="text">
MagickBooleanType MagickImplodeImage(MagickWand *wand,
  const double radius,const PixelInterpolateMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>amount</dt>
<dd>Define the extent of the implosion. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickImportImagePixels">MagickImportImagePixels</a></h2>

<p>MagickImportImagePixels() accepts pixel datand stores it in the image at the location you specify.  The method returns MagickFalse on success otherwise MagickTrue if an error is encountered.  The pixel data can be either char, short int, int, ssize_t, float, or double in the order specified by map.</p>

<p>Suppose your want to upload the first scanline of a 640x480 image from character data in red-green-blue order:</p>

<pre class="text">
MagickImportImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
</pre>

<p>The format of the MagickImportImagePixels method is:</p>

<pre class="text">
MagickBooleanType MagickImportImagePixels(MagickWand *wand,
  const ssize_t x,const ssize_t y,const size_t columns,
  const size_t rows,const char *map,const StorageType storage,
  const void *pixels)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x, y, columns, rows</dt>
<dd> These values define the perimeter of a region of pixels you want to define. </dd>

<dd> </dd>
<dt>map</dt>
<dd> This string reflects the expected ordering of the pixel array. It can be any combination or order of R = red, G = green, B = blue, A = alpha (0 is transparent), O = alpha (0 is opaque), C = cyan, Y = yellow, M = magenta, K = black, I = intensity (for grayscale), P = pad. </dd>

<dd> </dd>
<dt>storage</dt>
<dd>Define the data type of the pixels.  Float and double types are expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from these types: CharPixel, ShortPixel, IntegerPixel, LongPixel, FloatPixel, or DoublePixel. </dd>

<dd> </dd>
<dt>pixels</dt>
<dd>This array of values contain the pixel components as defined by map and type.  You must preallocate this array where the expected length varies depending on the values of width, height, map, and type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickInterpolativeResizeImage">MagickInterpolativeResizeImage</a></h2>

<p>MagickInterpolativeResizeImage() resize image using a interpolative method.</p>

<p>MagickBooleanType MagickInterpolativeResizeImage(MagickWand *wand, const size_t columns,const size_t rows, const PixelInterpolateMethod method)</p>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>
<dt>interpolate</dt>
<dd>the pixel interpolation method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickInverseFourierTransformImage">MagickInverseFourierTransformImage</a></h2>

<p>MagickInverseFourierTransformImage() implements the inverse discrete Fourier transform (DFT) of the image either as a magnitude / phase or real / imaginary image pair.</p>

<p>The format of the MagickInverseFourierTransformImage method is:</p>

<pre class="text">
MagickBooleanType MagickInverseFourierTransformImage(
  MagickWand *magnitude_wand,MagickWand *phase_wand,
  const MagickBooleanType magnitude)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>magnitude_wand</dt>
<dd>the magnitude or real wand. </dd>

<dd> </dd>
<dt>phase_wand</dt>
<dd>the phase or imaginary wand. </dd>

<dd> </dd>
<dt>magnitude</dt>
<dd>if true, return as magnitude / phase pair otherwise a real / imaginary image pair. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickLabelImage">MagickLabelImage</a></h2>

<p>MagickLabelImage() adds a label to your image.</p>

<p>The format of the MagickLabelImage method is:</p>

<pre class="text">
MagickBooleanType MagickLabelImage(MagickWand *wand,const char *label)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>label</dt>
<dd>the image label. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickLevelImage">MagickLevelImage</a></h2>

<p>MagickLevelImage() adjusts the levels of an image by scaling the colors falling between specified white and black points to the full available quantum range. The parameters provided represent the black, mid, and white points. The black point specifies the darkest color in the image. Colors darker than the black point are set to zero. Mid point specifies a gamma correction to apply to the image.  White point specifies the lightest color in the image. Colors brighter than the white point are set to the maximum quantum value.</p>

<p>The format of the MagickLevelImage method is:</p>

<pre class="text">
MagickBooleanType MagickLevelImage(MagickWand *wand,
  const double black_point,const double gamma,const double white_point)
MagickBooleanType MagickLevelImage(MagickWand *wand,
  const ChannelType channel,const double black_point,const double gamma,
  const double white_point)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>channel</dt>
<dd>Identify which channel to level: RedPixelChannel, GreenPixelChannel, etc. </dd>

<dd> </dd>
<dt>black_point</dt>
<dd>the black point. </dd>

<dd> </dd>
<dt>gamma</dt>
<dd>the gamma. </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>the white point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickLinearStretchImage">MagickLinearStretchImage</a></h2>

<p>MagickLinearStretchImage() stretches with saturation the image intensity.</p>

<p>You can also reduce the influence of a particular channel with a gamma value of 0.</p>

<p>The format of the MagickLinearStretchImage method is:</p>

<pre class="text">
MagickBooleanType MagickLinearStretchImage(MagickWand *wand,
  const double black_point,const double white_point)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>black_point</dt>
<dd>the black point. </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>the white point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickLiquidRescaleImage">MagickLiquidRescaleImage</a></h2>

<p>MagickLiquidRescaleImage() rescales image with seam carving.</p>

<p>MagickBooleanType MagickLiquidRescaleImage(MagickWand *wand, const size_t columns,const size_t rows, const double delta_x,const double rigidity)</p>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>
<dt>delta_x</dt>
<dd>maximum seam transversal step (0 means straight seams). </dd>

<dd> </dd>
<dt>rigidity</dt>
<dd>introduce a bias for non-straight seams (typically 0). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickMagnifyImage">MagickMagnifyImage</a></h2>

<p>MagickMagnifyImage() is a convenience method that scales an image proportionally to twice its original size.</p>

<p>The format of the MagickMagnifyImage method is:</p>

<pre class="text">
MagickBooleanType MagickMagnifyImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickMergeImageLayers">MagickMergeImageLayers</a></h2>

<p>MagickMergeImageLayers() composes all the image layers from the current given image onward to produce a single image of the merged layers.</p>

<p>The inital canvas's size depends on the given LayerMethod, and is initialized using the first images background color.  The images are then compositied onto that image in sequence using the given composition that has been assigned to each individual image.</p>

<p>The format of the MagickMergeImageLayers method is:</p>

<pre class="text">
MagickWand *MagickMergeImageLayers(MagickWand *wand,
  const LayerMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the method of selecting the size of the initial canvas. </dd>

<dd> MergeLayer: Merge all layers onto a canvas just large enough to hold all the actual images. The virtual canvas of the first image is preserved but otherwise ignored. </dd>

<dd> FlattenLayer: Use the virtual canvas size of first image. Images which fall outside this canvas is clipped. This can be used to 'fill out' a given virtual canvas. </dd>

<dd> MosaicLayer: Start with the virtual canvas of the first image, enlarging left and right edges to contain all images. Images with negative offsets will be clipped. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickMinifyImage">MagickMinifyImage</a></h2>

<p>MagickMinifyImage() is a convenience method that scales an image proportionally to one-half its original size</p>

<p>The format of the MagickMinifyImage method is:</p>

<pre class="text">
MagickBooleanType MagickMinifyImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickModulateImage">MagickModulateImage</a></h2>

<p>MagickModulateImage() lets you control the brightness, saturation, and hue of an image.  Hue is the percentage of absolute rotation from the current position.  For example 50 results in a counter-clockwise rotation of 90 degrees, 150 results in a clockwise rotation of 90 degrees, with 0 and 200 both resulting in a rotation of 180 degrees.</p>

<p>To increase the color brightness by 20 and decrease the color saturation by 10 and leave the hue unchanged, use: 120,90,100.</p>

<p>The format of the MagickModulateImage method is:</p>

<pre class="text">
MagickBooleanType MagickModulateImage(MagickWand *wand,
  const double brightness,const double saturation,const double hue)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>brightness</dt>
<dd>the percent change in brighness. </dd>

<dd> </dd>
<dt>saturation</dt>
<dd>the percent change in saturation. </dd>

<dd> </dd>
<dt>hue</dt>
<dd>the percent change in hue. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickMontageImage">MagickMontageImage</a></h2>

<p>MagickMontageImage() creates a composite image by combining several separate images. The images are tiled on the composite image with the name of the image optionally appearing just below the individual tile.</p>

<p>The format of the MagickMontageImage method is:</p>

<pre class="text">
MagickWand *MagickMontageImage(MagickWand *wand,
  const DrawingWand drawing_wand,const char *tile_geometry,
  const char *thumbnail_geometry,const MontageMode mode,
  const char *frame)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>drawing_wand</dt>
<dd>the drawing wand.  The font name, size, and color are obtained from this wand. </dd>

<dd> </dd>
<dt>tile_geometry</dt>
<dd>the number of tiles per row and page (e.g. 6x4+0+0). </dd>

<dd> </dd>
<dt>thumbnail_geometry</dt>
<dd>Preferred image size and border size of each thumbnail (e.g. 120x120+4+3&gt;). </dd>

<dd> </dd>
<dt>mode</dt>
<dd>Thumbnail framing mode: Frame, Unframe, or Concatenate. </dd>

<dd> </dd>
<dt>frame</dt>
<dd>Surround the image with an ornamental border (e.g. 15x15+3+3). The frame color is that of the thumbnail's matte color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickMorphImages">MagickMorphImages</a></h2>

<p>MagickMorphImages() method morphs a set of images.  Both the image pixels and size are linearly interpolated to give the appearance of a meta-morphosis from one image to the next.</p>

<p>The format of the MagickMorphImages method is:</p>

<pre class="text">
MagickWand *MagickMorphImages(MagickWand *wand,
  const size_t number_frames)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>number_frames</dt>
<dd>the number of in-between images to generate. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickMorphologyImage">MagickMorphologyImage</a></h2>

<p>MagickMorphologyImage() applies a user supplied kernel to the image according to the given mophology method.</p>

<p>The format of the MagickMorphologyImage method is:</p>

<pre class="text">
MagickBooleanType MagickMorphologyImage(MagickWand *wand,
  MorphologyMethod method,const ssize_t iterations,KernelInfo *kernel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the morphology method to be applied. </dd>

<dd> </dd>
<dt>iterations</dt>
<dd>apply the operation this many times (or no change). A value of -1 means loop until no change found.  How this is applied may depend on the morphology method.  Typically this is a value of 1. </dd>

<dd> </dd>
<dt>kernel</dt>
<dd>An array of doubles representing the morphology kernel. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickMotionBlurImage">MagickMotionBlurImage</a></h2>

<p>MagickMotionBlurImage() simulates motion blur.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma). For reasonable results, radius should be larger than sigma.  Use a radius of 0 and MotionBlurImage() selects a suitable radius for you. Angle gives the angle of the blurring motion.</p>

<p>The format of the MagickMotionBlurImage method is:</p>

<pre class="text">
MagickBooleanType MagickMotionBlurImage(MagickWand *wand,
  const double radius,const double sigma,const double angle)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>angle</dt>
<dd>Apply the effect along this angle. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickNegateImage">MagickNegateImage</a></h2>

<p>MagickNegateImage() negates the colors in the reference image.  The Grayscale option means that only grayscale values within the image are negated.</p>

<p>You can also reduce the influence of a particular channel with a gamma value of 0.</p>

<p>The format of the MagickNegateImage method is:</p>

<pre class="text">
MagickBooleanType MagickNegateImage(MagickWand *wand,
  const MagickBooleanType gray)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>gray</dt>
<dd>If MagickTrue, only negate grayscale pixels within the image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickNewImage">MagickNewImage</a></h2>

<p>MagickNewImage() adds a blank image canvas of the specified size and background color to the wand.</p>

<p>The format of the MagickNewImage method is:</p>

<pre class="text">
MagickBooleanType MagickNewImage(MagickWand *wand,
  const size_t columns,const size_t rows,
  const PixelWand *background)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the image width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the image height. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the image color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickNextImage">MagickNextImage</a></h2>

<p>MagickNextImage() sets the next image in the wand as the current image.</p>

<p>It is typically used after MagickResetIterator(), after which its first use will set the first image as the current image (unless the wand is empty).</p>

<p>It will return MagickFalse when no more images are left to be returned which happens when the wand is empty, or the current image is the last image.</p>

<p>When the above condition (end of image list) is reached, the iterator is automaticall set so that you can start using MagickPreviousImage() to again iterate over the images in the reverse direction, starting with the last image (again).  You can jump to this condition immeditally using MagickSetLastIterator().</p>

<p>The format of the MagickNextImage method is:</p>

<pre class="text">
MagickBooleanType MagickNextImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickNormalizeImage">MagickNormalizeImage</a></h2>

<p>MagickNormalizeImage() enhances the contrast of a color image by adjusting the pixels color to span the entire range of colors available</p>

<p>You can also reduce the influence of a particular channel with a gamma value of 0.</p>

<p>The format of the MagickNormalizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickNormalizeImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickOilPaintImage">MagickOilPaintImage</a></h2>

<p>MagickOilPaintImage() applies a special effect filter that simulates an oil painting.  Each pixel is replaced by the most frequent color occurring in a circular region defined by radius.</p>

<p>The format of the MagickOilPaintImage method is:</p>

<pre class="text">
MagickBooleanType MagickOilPaintImage(MagickWand *wand,
  const double radius,const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the circular neighborhood. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickOpaquePaintImage">MagickOpaquePaintImage</a></h2>

<p>MagickOpaquePaintImage() changes any pixel that matches color with the color defined by fill.</p>

<p>The format of the MagickOpaquePaintImage method is:</p>

<pre class="text">
MagickBooleanType MagickOpaquePaintImage(MagickWand *wand,
  const PixelWand *target,const PixelWand *fill,const double fuzz,
  const MagickBooleanType invert)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>target</dt>
<dd>Change this target color to the fill color within the image. </dd>

<dd> </dd>
<dt>fill</dt>
<dd>the fill pixel wand. </dd>

<dd> </dd>
<dt>fuzz</dt>
<dd>By default target must match a particular pixel color exactly.  However, in many cases two colors may differ by a small amount. The fuzz member of image defines how much tolerance is acceptable to consider two colors as the same.  For example, set fuzz to 10 and the color red at intensities of 100 and 102 respectively are now interpreted as the same color for the purposes of the floodfill. </dd>

<dd> </dd>
<dt>invert</dt>
<dd>paint any pixel that does not match the target color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickOptimizeImageLayers">MagickOptimizeImageLayers</a></h2>

<p>MagickOptimizeImageLayers() compares each image the GIF disposed forms of the previous image in the sequence.  From this it attempts to select the smallest cropped image to replace each frame, while preserving the results of the animation.</p>

<p>The format of the MagickOptimizeImageLayers method is:</p>

<pre class="text">
MagickWand *MagickOptimizeImageLayers(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickOptimizeImageTransparency">MagickOptimizeImageTransparency</a></h2>

<p>MagickOptimizeImageTransparency() takes a frame optimized GIF animation, and compares the overlayed pixels against the disposal image resulting from all the previous frames in the animation.  Any pixel that does not change the disposal image (and thus does not effect the outcome of an overlay) is made transparent.</p>

<p>WARNING: This modifies the current images directly, rather than generate a new image sequence. The format of the MagickOptimizeImageTransparency method is:</p>

<pre class="text">
MagickBooleanType MagickOptimizeImageTransparency(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickOrderedPosterizeImage">MagickOrderedPosterizeImage</a></h2>

<p>MagickOrderedPosterizeImage() performs an ordered dither based on a number of pre-defined dithering threshold maps, but over multiple intensity levels, which can be different for different channels, according to the input arguments.</p>

<p>The format of the MagickOrderedPosterizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickOrderedPosterizeImage(MagickWand *wand,
  const char *threshold_map)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>threshold_map</dt>
<dd>A string containing the name of the threshold dither map to use, followed by zero or more numbers representing the number of color levels tho dither between. </dd>

<dd> Any level number less than 2 is equivalent to 2, and means only binary dithering will be applied to each color channel. </dd>

<dd> No numbers also means a 2 level (bitmap) dither will be applied to all channels, while a single number is the number of levels applied to each channel in sequence.  More numbers will be applied in turn to each of the color channels. </dd>

<dd> For example: "o3x3,6" generates a 6 level posterization of the image with a ordered 3x3 diffused pixel dither being applied between each level. While checker,8,8,4 will produce a 332 colormaped image with only a single checkerboard hash pattern (50 grey) between each color level, to basically double the number of color levels with a bare minimim of dithering. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickPingImage">MagickPingImage</a></h2>

<p>MagickPingImage() is the same as MagickReadImage() except the only valid information returned is the image width, height, size, and format.  It is designed to efficiently obtain this information from a file without reading the entire image sequence into memory.</p>

<p>The format of the MagickPingImage method is:</p>

<pre class="text">
MagickBooleanType MagickPingImage(MagickWand *wand,const char *filename)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickPingImageBlob">MagickPingImageBlob</a></h2>

<p>MagickPingImageBlob() pings an image or image sequence from a blob.</p>

<p>The format of the MagickPingImageBlob method is:</p>

<pre class="text">
MagickBooleanType MagickPingImageBlob(MagickWand *wand,
  const void *blob,const size_t length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>blob</dt>
<dd>the blob. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the blob length. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickPingImageFile">MagickPingImageFile</a></h2>

<p>MagickPingImageFile() pings an image or image sequence from an open file descriptor.</p>

<p>The format of the MagickPingImageFile method is:</p>

<pre class="text">
MagickBooleanType MagickPingImageFile(MagickWand *wand,FILE *file)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>file</dt>
<dd>the file descriptor. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickPolaroidImage">MagickPolaroidImage</a></h2>

<p>MagickPolaroidImage() simulates a Polaroid picture.</p>

<p>The format of the MagickPolaroidImage method is:</p>

<pre class="text">
MagickBooleanType MagickPolaroidImage(MagickWand *wand,
  const DrawingWand *drawing_wand,const char *caption,const double angle,
  const PixelInterpolateMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>drawing_wand</dt>
<dd>the draw wand. </dd>

<dd> </dd>
<dt>caption</dt>
<dd>the Polaroid caption. </dd>

<dd> </dd>
<dt>angle</dt>
<dd>Apply the effect along this angle. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickPosterizeImage">MagickPosterizeImage</a></h2>

<p>MagickPosterizeImage() reduces the image to a limited number of color level.</p>

<p>The format of the MagickPosterizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickPosterizeImage(MagickWand *wand,
  const size_t levels,const DitherMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>levels</dt>
<dd>Number of color levels allowed in each channel.  Very low values (2, 3, or 4) have the most visible effect. </dd>

<dd> </dd>
<dt>method</dt>
<dd>choose the dither method: UndefinedDitherMethod, NoDitherMethod, RiemersmaDitherMethod, or FloydSteinbergDitherMethod. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickPreviewImages">MagickPreviewImages</a></h2>

<p>MagickPreviewImages() tiles 9 thumbnails of the specified image with an image processing operation applied at varying strengths.  This helpful to quickly pin-point an appropriate parameter for an image processing operation.</p>

<p>The format of the MagickPreviewImages method is:</p>

<pre class="text">
MagickWand *MagickPreviewImages(MagickWand *wand,
  const PreviewType preview)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>preview</dt>
<dd>the preview type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickPreviousImage">MagickPreviousImage</a></h2>

<p>MagickPreviousImage() sets the previous image in the wand as the current image.</p>

<p>It is typically used after MagickSetLastIterator(), after which its first use will set the last image as the current image (unless the wand is empty).</p>

<p>It will return MagickFalse when no more images are left to be returned which happens when the wand is empty, or the current image is the first image.  At that point the iterator is than reset to again process images in the forward direction, again starting with the first image in list. Images added at this point are prepended.</p>

<p>Also at that point any images added to the wand using MagickAddImages() or MagickReadImages() will be prepended before the first image. In this sense the condition is not quite exactly the same as MagickResetIterator().</p>

<p>The format of the MagickPreviousImage method is:</p>

<pre class="text">
MagickBooleanType MagickPreviousImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickQuantizeImage">MagickQuantizeImage</a></h2>

<p>MagickQuantizeImage() analyzes the colors within a reference image and chooses a fixed number of colors to represent the image.  The goal of the algorithm is to minimize the color difference between the input and output image while minimizing the processing time.</p>

<p>The format of the MagickQuantizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickQuantizeImage(MagickWand *wand,
  const size_t number_colors,const ColorspaceType colorspace,
  const size_t treedepth,const DitherMethod dither_method,
  const MagickBooleanType measure_error)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>number_colors</dt>
<dd>the number of colors. </dd>

<dd> </dd>
<dt>colorspace</dt>
<dd>Perform color reduction in this colorspace, typically RGBColorspace. </dd>

<dd> </dd>
<dt>treedepth</dt>
<dd>Normally, this integer value is zero or one.  A zero or one tells Quantize to choose a optimal tree depth of Log4(number_colors).      A tree of this depth generally allows the best representation of the reference image with the least amount of memory and the fastest computational speed.  In some cases, such as an image with low color dispersion (a few number of colors), a value other than Log4(number_colors) is required.  To expand the color tree completely, use a value of 8. </dd>

<dd> </dd>
<dt>dither_method</dt>
<dd>choose from UndefinedDitherMethod, NoDitherMethod, RiemersmaDitherMethod, FloydSteinbergDitherMethod. </dd>

<dd> </dd>
<dt>measure_error</dt>
<dd>A value other than zero measures the difference between the original and quantized images.  This difference is the total quantization error.  The error is computed by summing over all pixels in an image the distance squared in RGB space between each reference pixel value and its quantized value. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickQuantizeImages">MagickQuantizeImages</a></h2>

<p>MagickQuantizeImages() analyzes the colors within a sequence of images and chooses a fixed number of colors to represent the image.  The goal of the algorithm is to minimize the color difference between the input and output image while minimizing the processing time.</p>

<p>The format of the MagickQuantizeImages method is:</p>

<pre class="text">
MagickBooleanType MagickQuantizeImages(MagickWand *wand,
  const size_t number_colors,const ColorspaceType colorspace,
  const size_t treedepth,const DitherMethod dither_method,
  const MagickBooleanType measure_error)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>number_colors</dt>
<dd>the number of colors. </dd>

<dd> </dd>
<dt>colorspace</dt>
<dd>Perform color reduction in this colorspace, typically RGBColorspace. </dd>

<dd> </dd>
<dt>treedepth</dt>
<dd>Normally, this integer value is zero or one.  A zero or one tells Quantize to choose a optimal tree depth of Log4(number_colors).      A tree of this depth generally allows the best representation of the reference image with the least amount of memory and the fastest computational speed.  In some cases, such as an image with low color dispersion (a few number of colors), a value other than Log4(number_colors) is required.  To expand the color tree completely, use a value of 8. </dd>

<dd> </dd>
<dt>dither_method</dt>
<dd>choose from these dither methods: NoDitherMethod, RiemersmaDitherMethod, or FloydSteinbergDitherMethod. </dd>

<dd> </dd>
<dt>measure_error</dt>
<dd>A value other than zero measures the difference between the original and quantized images.  This difference is the total quantization error.  The error is computed by summing over all pixels in an image the distance squared in RGB space between each reference pixel value and its quantized value. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickRotationalBlurImage">MagickRotationalBlurImage</a></h2>

<p>MagickRotationalBlurImage() rotational blurs an image.</p>

<p>The format of the MagickRotationalBlurImage method is:</p>

<pre class="text">
MagickBooleanType MagickRotationalBlurImage(MagickWand *wand,
  const double angle)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>angle</dt>
<dd>the angle of the blur in degrees. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickRaiseImage">MagickRaiseImage</a></h2>

<p>MagickRaiseImage() creates a simulated three-dimensional button-like effect by lightening and darkening the edges of the image.  Members width and height of raise_info define the width of the vertical and horizontal edge of the effect.</p>

<p>The format of the MagickRaiseImage method is:</p>

<pre class="text">
MagickBooleanType MagickRaiseImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y,const MagickBooleanType raise)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width,height,x,y</dt>
<dd> Define the dimensions of the area to raise. </dd>

<dd> </dd>
<dt>raise</dt>
<dd>A value other than zero creates a 3-D raise effect, otherwise it has a lowered effect. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickRandomThresholdImage">MagickRandomThresholdImage</a></h2>

<p>MagickRandomThresholdImage() changes the value of individual pixels based on the intensity of each pixel compared to threshold.  The result is a high-contrast, two color image.</p>

<p>The format of the MagickRandomThresholdImage method is:</p>

<pre class="text">
MagickBooleanType MagickRandomThresholdImage(MagickWand *wand,
  const double low,const double high)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>low,high</dt>
<dd>Specify the high and low thresholds.  These values range from 0 to QuantumRange. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickReadImage">MagickReadImage</a></h2>

<p>MagickReadImage() reads an image or image sequence.  The images are inserted jjust before the current image pointer position.</p>

<p>Use MagickSetFirstIterator(), to insert new images before all the current images in the wand, MagickSetLastIterator() to append add to the end, MagickSetIteratorIndex() to place images just after the given index.</p>

<p>The format of the MagickReadImage method is:</p>

<pre class="text">
MagickBooleanType MagickReadImage(MagickWand *wand,const char *filename)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickReadImageBlob">MagickReadImageBlob</a></h2>

<p>MagickReadImageBlob() reads an image or image sequence from a blob. In all other respects it is like MagickReadImage().</p>

<p>The format of the MagickReadImageBlob method is:</p>

<pre class="text">
MagickBooleanType MagickReadImageBlob(MagickWand *wand,
  const void *blob,const size_t length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>blob</dt>
<dd>the blob. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the blob length. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickReadImageFile">MagickReadImageFile</a></h2>

<p>MagickReadImageFile() reads an image or image sequence from an already opened file descriptor.  Otherwise it is like MagickReadImage().</p>

<p>The format of the MagickReadImageFile method is:</p>

<pre class="text">
MagickBooleanType MagickReadImageFile(MagickWand *wand,FILE *file)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>file</dt>
<dd>the file descriptor. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickRemapImage">MagickRemapImage</a></h2>

<p>MagickRemapImage() replaces the colors of an image with the closest color from a reference image.</p>

<p>The format of the MagickRemapImage method is:</p>

<pre class="text">
MagickBooleanType MagickRemapImage(MagickWand *wand,
  const MagickWand *remap_wand,const DitherMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>affinity</dt>
<dd>the affinity wand. </dd>

<dd> </dd>
<dt>method</dt>
<dd>choose from these dither methods: NoDitherMethod, RiemersmaDitherMethod, or FloydSteinbergDitherMethod. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickRemoveImage">MagickRemoveImage</a></h2>

<p>MagickRemoveImage() removes an image from the image list.</p>

<p>The format of the MagickRemoveImage method is:</p>

<pre class="text">
MagickBooleanType MagickRemoveImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>insert</dt>
<dd>the splice wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickResampleImage">MagickResampleImage</a></h2>

<p>MagickResampleImage() resample image to desired resolution.</p>

<p>Bessel   Blackman   Box Catrom   Cubic      Gaussian Hanning  Hermite    Lanczos Mitchell Point      Quandratic Sinc     Triangle</p>

<p>Most of the filters are FIR (finite impulse response), however, Bessel, Gaussian, and Sinc are IIR (infinite impulse response).  Bessel and Sinc are windowed (brought down to zero) with the Blackman filter.</p>

<p>The format of the MagickResampleImage method is:</p>

<pre class="text">
MagickBooleanType MagickResampleImage(MagickWand *wand,
  const double x_resolution,const double y_resolution,
  const FilterTypes filter)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x_resolution</dt>
<dd>the new image x resolution. </dd>

<dd> </dd>
<dt>y_resolution</dt>
<dd>the new image y resolution. </dd>

<dd> </dd>
<dt>filter</dt>
<dd>Image filter to use. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickResetImagePage">MagickResetImagePage</a></h2>

<p>MagickResetImagePage() resets the Wand page canvas and position.</p>

<p>The format of the MagickResetImagePage method is:</p>

<pre class="text">
MagickBooleanType MagickResetImagePage(MagickWand *wand,
  const char *page)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>page</dt>
<dd>the relative page specification. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickResizeImage">MagickResizeImage</a></h2>

<p>MagickResizeImage() scales an image to the desired dimensions with one of these filters:</p>

<pre class="text">
    Bessel   Blackman   Box
    Catrom   CubicGaussian
    Hanning  Hermite    Lanczos
    Mitchell PointQuandratic
    Sinc     Triangle
</pre>

<p>Most of the filters are FIR (finite impulse response), however, Bessel, Gaussian, and Sinc are IIR (infinite impulse response).  Bessel and Sinc are windowed (brought down to zero) with the Blackman filter.</p>

<p>The format of the MagickResizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickResizeImage(MagickWand *wand,
  const size_t columns,const size_t rows,const FilterTypes filter)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>
<dt>filter</dt>
<dd>Image filter to use. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickRollImage">MagickRollImage</a></h2>

<p>MagickRollImage() offsets an image as defined by x and y.</p>

<p>The format of the MagickRollImage method is:</p>

<pre class="text">
MagickBooleanType MagickRollImage(MagickWand *wand,const ssize_t x,
  const size_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the x offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the y offset. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickRotateImage">MagickRotateImage</a></h2>

<p>MagickRotateImage() rotates an image the specified number of degrees. Empty triangles left over from rotating the image are filled with the background color.</p>

<p>The format of the MagickRotateImage method is:</p>

<pre class="text">
MagickBooleanType MagickRotateImage(MagickWand *wand,
  const PixelWand *background,const double degrees)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the background pixel wand. </dd>

<dd> </dd>
<dt>degrees</dt>
<dd>the number of degrees to rotate the image. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSampleImage">MagickSampleImage</a></h2>

<p>MagickSampleImage() scales an image to the desired dimensions with pixel sampling.  Unlike other scaling methods, this method does not introduce any additional color into the scaled image.</p>

<p>The format of the MagickSampleImage method is:</p>

<pre class="text">
MagickBooleanType MagickSampleImage(MagickWand *wand,
  const size_t columns,const size_t rows)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickScaleImage">MagickScaleImage</a></h2>

<p>MagickScaleImage() scales the size of an image to the given dimensions.</p>

<p>The format of the MagickScaleImage method is:</p>

<pre class="text">
MagickBooleanType MagickScaleImage(MagickWand *wand,
  const size_t columns,const size_t rows)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSegmentImage">MagickSegmentImage</a></h2>

<p>MagickSegmentImage() segments an image by analyzing the histograms of the color components and identifying units that are homogeneous with the fuzzy C-means technique.</p>

<p>The format of the SegmentImage method is:</p>

<pre class="text">
MagickBooleanType MagickSegmentImage(MagickWand *wand,
  const ColorspaceType colorspace,const MagickBooleanType verbose,
  const double cluster_threshold,const double smooth_threshold)
</pre>

<p>A description of each parameter follows.</p>

<dt>wand</dt>
<p>the wand.</p>

<dt>colorspace</dt>
<p>the image colorspace.</p>

<dt>verbose</dt>
<p>Set to MagickTrue to print detailed information about the identified classes.</p>

<dt>cluster_threshold</dt>
<p>This represents the minimum number of pixels contained in a hexahedra before it can be considered valid (expressed as a percentage).</p>

<dt>smooth_threshold</dt>
<p>the smoothing threshold eliminates noise in the second derivative of the histogram.  As the value is increased, you can expect a smoother second derivative.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSelectiveBlurImage">MagickSelectiveBlurImage</a></h2>

<p>MagickSelectiveBlurImage() selectively blur an image within a contrast threshold. It is similar to the unsharpen mask that sharpens everything with contrast above a certain threshold.</p>

<p>The format of the MagickSelectiveBlurImage method is:</p>

<pre class="text">
MagickBooleanType MagickSelectiveBlurImage(MagickWand *wand,
  const double radius,const double sigma,const double threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the gaussian, in pixels. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>only pixels within this contrast threshold are included in the blur operation. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSeparateImage">MagickSeparateImage</a></h2>

<p>MagickSeparateImage() separates a channel from the image and returns a grayscale image.  A channel is a particular color component of each pixel in the image.</p>

<p>The format of the MagickSeparateImage method is:</p>

<pre class="text">
MagickBooleanType MagickSeparateImage(MagickWand *wand,
  const ChannelType channel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>channel</dt>
<dd>the channel. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSepiaToneImage">MagickSepiaToneImage</a></h2>

<p>MagickSepiaToneImage() applies a special effect to the image, similar to the effect achieved in a photo darkroom by sepia toning.  Threshold ranges from 0 to QuantumRange and is a measure of the extent of the sepia toning.  A threshold of 80 is a good starting point for a reasonable tone.</p>

<p>The format of the MagickSepiaToneImage method is:</p>

<pre class="text">
MagickBooleanType MagickSepiaToneImage(MagickWand *wand,
  const double threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd> Define the extent of the sepia toning. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImage">MagickSetImage</a></h2>

<p>MagickSetImage() replaces the last image returned by MagickSetIteratorIndex(), MagickNextImage(), MagickPreviousImage() with the images from the specified wand.</p>

<p>The format of the MagickSetImage method is:</p>

<pre class="text">
MagickBooleanType MagickSetImage(MagickWand *wand,
  const MagickWand *set_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>set_wand</dt>
<dd>the set_wand wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageAlphaChannel">MagickSetImageAlphaChannel</a></h2>

<p>MagickSetImageAlphaChannel() activates, deactivates, resets, or sets the alpha channel.</p>

<p>The format of the MagickSetImageAlphaChannel method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageAlphaChannel(MagickWand *wand,
  const AlphaChannelOption alpha_type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>alpha_type</dt>
<dd>the alpha channel type: ActivateAlphaChannel, DeactivateAlphaChannel, OpaqueAlphaChannel, or SetAlphaChannel. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageBackgroundColor">MagickSetImageBackgroundColor</a></h2>

<p>MagickSetImageBackgroundColor() sets the image background color.</p>

<p>The format of the MagickSetImageBackgroundColor method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageBackgroundColor(MagickWand *wand,
  const PixelWand *background)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the background pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageBluePrimary">MagickSetImageBluePrimary</a></h2>

<p>MagickSetImageBluePrimary() sets the image chromaticity blue primary point.</p>

<p>The format of the MagickSetImageBluePrimary method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageBluePrimary(MagickWand *wand,
  const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the blue primary x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the blue primary y-point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageBorderColor">MagickSetImageBorderColor</a></h2>

<p>MagickSetImageBorderColor() sets the image border color.</p>

<p>The format of the MagickSetImageBorderColor method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageBorderColor(MagickWand *wand,
  const PixelWand *border)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>border</dt>
<dd>the border pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageChannelMask">MagickSetImageChannelMask</a></h2>

<p>MagickSetImageChannelMask() sets image channel mask.</p>

<p>The format of the MagickSetImageChannelMask method is:</p>

<pre class="text">
ChannelType MagickSetImageChannelMask(MagickWand *wand,
  const ChannelType channel_mask)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>channel_mask</dt>
<dd>the channel_mask wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageMask">MagickSetImageMask</a></h2>

<p>MagickSetImageMask() sets image clip mask.</p>

<p>The format of the MagickSetImageMask method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageMask(MagickWand *wand,
  const PixelMask type,const MagickWand *clip_mask)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>type</dt>
<dd>type of mask, ReadPixelMask or WritePixelMask. </dd>

<dd> </dd>
<dt>clip_mask</dt>
<dd>the clip_mask wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageColor">MagickSetImageColor</a></h2>

<p>MagickSetImageColor() set the entire wand canvas to the specified color.</p>

<p>The format of the MagickSetImageColor method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageColor(MagickWand *wand,
  const PixelWand *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the image color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageColormapColor">MagickSetImageColormapColor</a></h2>

<p>MagickSetImageColormapColor() sets the color of the specified colormap index.</p>

<p>The format of the MagickSetImageColormapColor method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageColormapColor(MagickWand *wand,
  const size_t index,const PixelWand *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>index</dt>
<dd>the offset into the image colormap. </dd>

<dd> </dd>
<dt>color</dt>
<dd>Return the colormap color in this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageColorspace">MagickSetImageColorspace</a></h2>

<p>MagickSetImageColorspace() sets the image colorspace. But does not modify the image data.</p>

<p>The format of the MagickSetImageColorspace method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageColorspace(MagickWand *wand,
  const ColorspaceType colorspace)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>colorspace</dt>
<dd>the image colorspace:   UndefinedColorspace, RGBColorspace, GRAYColorspace, TransparentColorspace, OHTAColorspace, XYZColorspace, YCbCrColorspace, YCCColorspace, YIQColorspace, YPbPrColorspace, YPbPrColorspace, YUVColorspace, CMYKColorspace, sRGBColorspace, HSLColorspace, or HWBColorspace. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageCompose">MagickSetImageCompose</a></h2>

<p>MagickSetImageCompose() sets the image composite operator, useful for specifying how to composite the image thumbnail when using the MagickMontageImage() method.</p>

<p>The format of the MagickSetImageCompose method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageCompose(MagickWand *wand,
  const CompositeOperator compose)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>compose</dt>
<dd>the image composite operator. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageCompression">MagickSetImageCompression</a></h2>

<p>MagickSetImageCompression() sets the image compression.</p>

<p>The format of the MagickSetImageCompression method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageCompression(MagickWand *wand,
  const CompressionType compression)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>compression</dt>
<dd>the image compression type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageCompressionQuality">MagickSetImageCompressionQuality</a></h2>

<p>MagickSetImageCompressionQuality() sets the image compression quality.</p>

<p>The format of the MagickSetImageCompressionQuality method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageCompressionQuality(MagickWand *wand,
  const size_t quality)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>quality</dt>
<dd>the image compression tlityype. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageDelay">MagickSetImageDelay</a></h2>

<p>MagickSetImageDelay() sets the image delay.</p>

<p>The format of the MagickSetImageDelay method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageDelay(MagickWand *wand,
  const size_t delay)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>delay</dt>
<dd>the image delay in ticks-per-second units. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageDepth">MagickSetImageDepth</a></h2>

<p>MagickSetImageDepth() sets the image depth.</p>

<p>The format of the MagickSetImageDepth method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageDepth(MagickWand *wand,
  const size_t depth)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>depth</dt>
<dd>the image depth in bits: 8, 16, or 32. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageDispose">MagickSetImageDispose</a></h2>

<p>MagickSetImageDispose() sets the image disposal method.</p>

<p>The format of the MagickSetImageDispose method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageDispose(MagickWand *wand,
  const DisposeType dispose)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>dispose</dt>
<dd>the image disposeal type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageEndian">MagickSetImageEndian</a></h2>

<p>MagickSetImageEndian() sets the image endian method.</p>

<p>The format of the MagickSetImageEndian method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageEndian(MagickWand *wand,
  const EndianType endian)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>endian</dt>
<dd>the image endian type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageExtent">MagickSetImageExtent</a></h2>

<p>MagickSetImageExtent() sets the image size (i.e. columns &amp; rows).</p>

<p>The format of the MagickSetImageExtent method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageExtent(MagickWand *wand,
  const size_t columns,const unsigned rows)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd> The image width in pixels. </dd>

<dd> </dd>
<dt>rows</dt>
<dd> The image height in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageFilename">MagickSetImageFilename</a></h2>

<p>MagickSetImageFilename() sets the filename of a particular image in a sequence.</p>

<p>The format of the MagickSetImageFilename method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageFilename(MagickWand *wand,
  const char *filename)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageFormat">MagickSetImageFormat</a></h2>

<p>MagickSetImageFormat() sets the format of a particular image in a sequence.</p>

<p>The format of the MagickSetImageFormat method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageFormat(MagickWand *wand,
  const char *format)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>format</dt>
<dd>the image format. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageFuzz">MagickSetImageFuzz</a></h2>

<p>MagickSetImageFuzz() sets the image fuzz.</p>

<p>The format of the MagickSetImageFuzz method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageFuzz(MagickWand *wand,
  const double fuzz)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>fuzz</dt>
<dd>the image fuzz. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageGamma">MagickSetImageGamma</a></h2>

<p>MagickSetImageGamma() sets the image gamma.</p>

<p>The format of the MagickSetImageGamma method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageGamma(MagickWand *wand,
  const double gamma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>gamma</dt>
<dd>the image gamma. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageGravity">MagickSetImageGravity</a></h2>

<p>MagickSetImageGravity() sets the image gravity type.</p>

<p>The format of the MagickSetImageGravity method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageGravity(MagickWand *wand,
  const GravityType gravity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>gravity</dt>
<dd>positioning gravity (NorthWestGravity, NorthGravity, NorthEastGravity, WestGravity, CenterGravity, EastGravity, SouthWestGravity, SouthGravity, SouthEastGravity) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageGreenPrimary">MagickSetImageGreenPrimary</a></h2>

<p>MagickSetImageGreenPrimary() sets the image chromaticity green primary point.</p>

<p>The format of the MagickSetImageGreenPrimary method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageGreenPrimary(MagickWand *wand,
  const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the green primary x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the green primary y-point. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageInterlaceScheme">MagickSetImageInterlaceScheme</a></h2>

<p>MagickSetImageInterlaceScheme() sets the image interlace scheme.</p>

<p>The format of the MagickSetImageInterlaceScheme method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageInterlaceScheme(MagickWand *wand,
  const InterlaceType interlace)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>interlace</dt>
<dd>the image interlace scheme: NoInterlace, LineInterlace, PlaneInterlace, PartitionInterlace. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImagePixelInterpolateMethod">MagickSetImagePixelInterpolateMethod</a></h2>

<p>MagickSetImagePixelInterpolateMethod() sets the image interpolate pixel method.</p>

<p>The format of the MagickSetImagePixelInterpolateMethod method is:</p>

<pre class="text">
MagickBooleanType MagickSetImagePixelInterpolateMethod(MagickWand *wand,
  const PixelInterpolateMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the image interpole pixel methods: choose from Undefined, Average, Bicubic, Bilinear, Filter, Integer, Mesh, NearestNeighbor. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageIterations">MagickSetImageIterations</a></h2>

<p>MagickSetImageIterations() sets the image iterations.</p>

<p>The format of the MagickSetImageIterations method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageIterations(MagickWand *wand,
  const size_t iterations)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>delay</dt>
<dd>the image delay in 1/100th of a second. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageMatte">MagickSetImageMatte</a></h2>

<p>MagickSetImageMatte() sets the image matte channel.</p>

<p>The format of the MagickSetImageMatteColor method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageMatteColor(MagickWand *wand,
  const MagickBooleanType *matte)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>matte</dt>
<dd>Set to MagickTrue to enable the image matte channel otherwise MagickFalse. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageMatteColor">MagickSetImageMatteColor</a></h2>

<p>MagickSetImageMatteColor() sets the image matte color.</p>

<p>The format of the MagickSetImageMatteColor method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageMatteColor(MagickWand *wand,
  const PixelWand *matte)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>matte</dt>
<dd>the matte pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageAlpha">MagickSetImageAlpha</a></h2>

<p>MagickSetImageAlpha() sets the image to the specified alpha level.</p>

<p>The format of the MagickSetImageAlpha method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageAlpha(MagickWand *wand,
  const double alpha)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>the level of transparency: 1.0 is fully opaque and 0.0 is fully transparent. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageOrientation">MagickSetImageOrientation</a></h2>

<p>MagickSetImageOrientation() sets the image orientation.</p>

<p>The format of the MagickSetImageOrientation method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageOrientation(MagickWand *wand,
  const OrientationType orientation)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>orientation</dt>
<dd>the image orientation type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImagePage">MagickSetImagePage</a></h2>

<p>MagickSetImagePage() sets the page geometry of the image.</p>

<p>The format of the MagickSetImagePage method is:</p>

<pre class="text">
MagickBooleanType MagickSetImagePage(MagickWand *wand,const size_t width,        const size_t height,const ssize_t x,const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the page width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the page height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the page x-offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the page y-offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageProgressMonitor">MagickSetImageProgressMonitor</a></h2>

<p>MagickSetImageProgressMonitor() sets the wand image progress monitor to the specified method and returns the previous progress monitor if any.  The progress monitor method looks like this:</p>

<pre class="text">
    MagickBooleanType MagickProgressMonitor(const char *text,
const MagickOffsetType offset,const MagickSizeType span,
void *client_data)
</pre>

<p>If the progress monitor returns MagickFalse, the current operation is interrupted.</p>

<p>The format of the MagickSetImageProgressMonitor method is:</p>

<pre class="text">
MagickProgressMonitor MagickSetImageProgressMonitor(MagickWand *wand
  const MagickProgressMonitor progress_monitor,void *client_data)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>progress_monitor</dt>
<dd>Specifies a pointer to a method to monitor progress of an image operation. </dd>

<dd> </dd>
<dt>client_data</dt>
<dd>Specifies a pointer to any client data. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageRedPrimary">MagickSetImageRedPrimary</a></h2>

<p>MagickSetImageRedPrimary() sets the image chromaticity red primary point.</p>

<p>The format of the MagickSetImageRedPrimary method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageRedPrimary(MagickWand *wand,
  const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the red primary x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the red primary y-point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageRenderingIntent">MagickSetImageRenderingIntent</a></h2>

<p>MagickSetImageRenderingIntent() sets the image rendering intent.</p>

<p>The format of the MagickSetImageRenderingIntent method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageRenderingIntent(MagickWand *wand,
  const RenderingIntent rendering_intent)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>rendering_intent</dt>
<dd>the image rendering intent: UndefinedIntent, SaturationIntent, PerceptualIntent, AbsoluteIntent, or RelativeIntent. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageResolution">MagickSetImageResolution</a></h2>

<p>MagickSetImageResolution() sets the image resolution.</p>

<p>The format of the MagickSetImageResolution method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageResolution(MagickWand *wand,
  const double x_resolution,const double y_resolution)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x_resolution</dt>
<dd>the image x resolution. </dd>

<dd> </dd>
<dt>y_resolution</dt>
<dd>the image y resolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageScene">MagickSetImageScene</a></h2>

<p>MagickSetImageScene() sets the image scene.</p>

<p>The format of the MagickSetImageScene method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageScene(MagickWand *wand,
  const size_t scene)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>delay</dt>
<dd>the image scene number. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageTicksPerSecond">MagickSetImageTicksPerSecond</a></h2>

<p>MagickSetImageTicksPerSecond() sets the image ticks-per-second.</p>

<p>The format of the MagickSetImageTicksPerSecond method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageTicksPerSecond(MagickWand *wand,
  const ssize_t ticks_per-second)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>ticks_per_second</dt>
<dd>the units to use for the image delay. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageType">MagickSetImageType</a></h2>

<p>MagickSetImageType() sets the image type.</p>

<p>The format of the MagickSetImageType method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageType(MagickWand *wand,
  const ImageType image_type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>image_type</dt>
<dd>the image type:   UndefinedType, BilevelType, GrayscaleType, GrayscaleAlphaType, PaletteType, PaletteAlphaType, TrueColorType, TrueColorAlphaType, ColorSeparationType, ColorSeparationAlphaType, or OptimizeType. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageUnits">MagickSetImageUnits</a></h2>

<p>MagickSetImageUnits() sets the image units of resolution.</p>

<p>The format of the MagickSetImageUnits method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageUnits(MagickWand *wand,
  const ResolutionType units)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>units</dt>
<dd>the image units of resolution : UndefinedResolution, PixelsPerInchResolution, or PixelsPerCentimeterResolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageVirtualPixelMethod">MagickSetImageVirtualPixelMethod</a></h2>

<p>MagickSetImageVirtualPixelMethod() sets the image virtual pixel method.</p>

<p>The format of the MagickSetImageVirtualPixelMethod method is:</p>

<pre class="text">
VirtualPixelMethod MagickSetImageVirtualPixelMethod(MagickWand *wand,
  const VirtualPixelMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the image virtual pixel method : UndefinedVirtualPixelMethod, ConstantVirtualPixelMethod,  EdgeVirtualPixelMethod, MirrorVirtualPixelMethod, or TileVirtualPixelMethod. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSetImageWhitePoint">MagickSetImageWhitePoint</a></h2>

<p>MagickSetImageWhitePoint() sets the image chromaticity white point.</p>

<p>The format of the MagickSetImageWhitePoint method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageWhitePoint(MagickWand *wand,
  const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the white x-point. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the white y-point. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickShadeImage">MagickShadeImage</a></h2>

<p>MagickShadeImage() shines a distant light on an image to create a three-dimensional effect. You control the positioning of the light with azimuth and elevation; azimuth is measured in degrees off the x axis and elevation is measured in pixels above the Z axis.</p>

<p>The format of the MagickShadeImage method is:</p>

<pre class="text">
MagickBooleanType MagickShadeImage(MagickWand *wand,
  const MagickBooleanType gray,const double azimuth,
  const double elevation)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>gray</dt>
<dd>A value other than zero shades the intensity of each pixel. </dd>

<dd> </dd>
<dt>azimuth, elevation</dt>
<dd> Define the light source direction. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickShadowImage">MagickShadowImage</a></h2>

<p>MagickShadowImage() simulates an image shadow.</p>

<p>The format of the MagickShadowImage method is:</p>

<pre class="text">
MagickBooleanType MagickShadowImage(MagickWand *wand,const double alpha,
  const double sigma,const ssize_t x,const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>percentage transparency. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the shadow x-offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the shadow y-offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSharpenImage">MagickSharpenImage</a></h2>

<p>MagickSharpenImage() sharpens an image.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma). For reasonable results, the radius should be larger than sigma.  Use a radius of 0 and MagickSharpenImage() selects a suitable radius for you.</p>

<p>The format of the MagickSharpenImage method is:</p>

<pre class="text">
MagickBooleanType MagickSharpenImage(MagickWand *wand,
  const double radius,const double sigma)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickShaveImage">MagickShaveImage</a></h2>

<p>MagickShaveImage() shaves pixels from the image edges.  It allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the MagickShaveImage method is:</p>

<pre class="text">
MagickBooleanType MagickShaveImage(MagickWand *wand,
  const size_t columns,const size_t rows)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickShearImage">MagickShearImage</a></h2>

<p>MagickShearImage() slides one edge of an image along the X or Y axis, creating a parallelogram.  An X direction shear slides an edge along the X axis, while a Y direction shear slides an edge along the Y axis.  The amount of the shear is controlled by a shear angle.  For X direction shears, x_shear is measured relative to the Y axis, and similarly, for Y direction shears y_shear is measured relative to the X axis.  Empty triangles left over from shearing the image are filled with the background color.</p>

<p>The format of the MagickShearImage method is:</p>

<pre class="text">
MagickBooleanType MagickShearImage(MagickWand *wand,
  const PixelWand *background,const double x_shear,const double y_shear)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the background pixel wand. </dd>

<dd> </dd>
<dt>x_shear</dt>
<dd>the number of degrees to shear the image. </dd>

<dd> </dd>
<dt>y_shear</dt>
<dd>the number of degrees to shear the image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSigmoidalContrastImage">MagickSigmoidalContrastImage</a></h2>

<p>MagickSigmoidalContrastImage() adjusts the contrast of an image with a non-linear sigmoidal contrast algorithm.  Increase the contrast of the image using a sigmoidal transfer function without saturating highlights or shadows.  Contrast indicates how much to increase the contrast (0 is none; 3 is typical; 20 is pushing it); mid-point indicates where midtones fall in the resultant image (0 is white; 50 is middle-gray; 100 is black).  Set sharpen to MagickTrue to increase the image contrast otherwise the contrast is reduced.</p>

<p>The format of the MagickSigmoidalContrastImage method is:</p>

<pre class="text">
MagickBooleanType MagickSigmoidalContrastImage(MagickWand *wand,
  const MagickBooleanType sharpen,const double alpha,const double beta)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>sharpen</dt>
<dd>Increase or decrease image contrast. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>strength of the contrast, the larger the number the more 'threshold-like' it becomes. </dd>

<dd> </dd>
<dt>beta</dt>
<dd>midpoint of the function as a color value 0 to QuantumRange. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSimilarityImage">MagickSimilarityImage</a></h2>

<p>MagickSimilarityImage() compares the reference image of the image and returns the best match offset.  In addition, it returns a similarity image such that an exact match location is completely white and if none of the pixels match, black, otherwise some gray level in-between.</p>

<p>The format of the MagickSimilarityImage method is:</p>

<pre class="text">
MagickWand *MagickSimilarityImage(MagickWand *wand,
  const MagickWand *reference,const MetricType metric,
  const double similarity_threshold,RectangeInfo *offset,
  double *similarity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>reference</dt>
<dd>the reference wand. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd> </dd>
<dt>similarity_threshold</dt>
<dd>minimum distortion for (sub)image match. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>the best match offset of the reference image within the image. </dd>

<dd> </dd>
<dt>similarity</dt>
<dd>the computed similarity between the images. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSketchImage">MagickSketchImage</a></h2>

<p>MagickSketchImage() simulates a pencil sketch.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma). For reasonable results, radius should be larger than sigma.  Use a radius of 0 and SketchImage() selects a suitable radius for you. Angle gives the angle of the blurring motion.</p>

<p>The format of the MagickSketchImage method is:</p>

<pre class="text">
MagickBooleanType MagickSketchImage(MagickWand *wand,
  const double radius,const double sigma,const double angle)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>angle</dt>
<dd>apply the effect along this angle. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSmushImages">MagickSmushImages</a></h2>

<p>MagickSmushImages() takes all images from the current image pointer to the end of the image list and smushs them to each other top-to-bottom if the stack parameter is true, otherwise left-to-right.</p>

<p>The format of the MagickSmushImages method is:</p>

<pre class="text">
MagickWand *MagickSmushImages(MagickWand *wand,
  const MagickBooleanType stack,const ssize_t offset)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>stack</dt>
<dd>By default, images are stacked left-to-right. Set stack to MagickTrue to stack them top-to-bottom. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>minimum distance in pixels between images. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSolarizeImage">MagickSolarizeImage</a></h2>

<p>MagickSolarizeImage() applies a special effect to the image, similar to the effect achieved in a photo darkroom by selectively exposing areas of photo sensitive paper to light.  Threshold ranges from 0 to QuantumRange and is a measure of the extent of the solarization.</p>

<p>The format of the MagickSolarizeImage method is:</p>

<pre class="text">
MagickBooleanType MagickSolarizeImage(MagickWand *wand,
  const double threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd> Define the extent of the solarization. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSparseColorImage">MagickSparseColorImage</a></h2>

<p>MagickSparseColorImage(), given a set of coordinates, interpolates the colors found at those coordinates, across the whole image, using various methods.</p>

<p>The format of the MagickSparseColorImage method is:</p>

<pre class="text">
MagickBooleanType MagickSparseColorImage(MagickWand *wand,
  const SparseColorMethod method,const size_t number_arguments,
  const double *arguments)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image to be sparseed. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the method of image sparseion. </dd>

<dd> ArcSparseColorion will always ignore source image offset, and always 'bestfit' the destination image with the top left corner offset relative to the polar mapping center. </dd>

<dd> Bilinear has no simple inverse mapping so will not allow 'bestfit' style of image sparseion. </dd>

<dd> Affine, Perspective, and Bilinear, will do least squares fitting of the distrotion when more than the minimum number of control point pairs are provided. </dd>

<dd> Perspective, and Bilinear, will fall back to a Affine sparseion when less than 4 control point pairs are provided. While Affine sparseions will let you use any number of control point pairs, that is Zero pairs is a No-Op (viewport only) distrotion, one pair is a translation and two pairs of control points will do a scale-rotate-translate, without any shearing. </dd>

<dd> </dd>
<dt>number_arguments</dt>
<dd>the number of arguments given for this sparseion method. </dd>

<dd> </dd>
<dt>arguments</dt>
<dd>the arguments for this sparseion method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSpliceImage">MagickSpliceImage</a></h2>

<p>MagickSpliceImage() splices a solid color into the image.</p>

<p>The format of the MagickSpliceImage method is:</p>

<pre class="text">
MagickBooleanType MagickSpliceImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the region width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the region height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the region x offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the region y offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSpreadImage">MagickSpreadImage</a></h2>

<p>MagickSpreadImage() is a special effects method that randomly displaces each pixel in a block defined by the radius parameter.</p>

<p>The format of the MagickSpreadImage method is:</p>

<pre class="text">
MagickBooleanType MagickSpreadImage(MagickWand *wand,
  const PixelInterpolateMethod method,const double radius)
  
  A description of each parameter follows:
</pre>

<dt>wand</dt>
<p>the magick wand.</p>

<dt>method</dt>
<p>intepolation method.</p>

<dt>radius</dt>
<p>Choose a random pixel in a neighborhood of this extent.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickStatisticImage">MagickStatisticImage</a></h2>

<p>MagickStatisticImage() replace each pixel with corresponding statistic from the neighborhood of the specified width and height.</p>

<p>The format of the MagickStatisticImage method is:</p>

<pre class="text">
MagickBooleanType MagickStatisticImage(MagickWand *wand,
  const StatisticType type,const double width,const size_t height)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>type</dt>
<dd>the statistic type (e.g. median, mode, etc.). </dd>

<dd> </dd>
<dt>width</dt>
<dd>the width of the pixel neighborhood. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the height of the pixel neighborhood. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSteganoImage">MagickSteganoImage</a></h2>

<p>MagickSteganoImage() hides a digital watermark within the image. Recover the hidden watermark later to prove that the authenticity of an image.  Offset defines the start position within the image to hide the watermark.</p>

<p>The format of the MagickSteganoImage method is:</p>

<pre class="text">
MagickWand *MagickSteganoImage(MagickWand *wand,
  const MagickWand *watermark_wand,const ssize_t offset)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>watermark_wand</dt>
<dd>the watermark wand. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>Start hiding at this offset into the image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickStereoImage">MagickStereoImage</a></h2>

<p>MagickStereoImage() composites two images and produces a single image that is the composite of a left and right image of a stereo pair</p>

<p>The format of the MagickStereoImage method is:</p>

<pre class="text">
MagickWand *MagickStereoImage(MagickWand *wand,
  const MagickWand *offset_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>offset_wand</dt>
<dd>Another image wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickStripImage">MagickStripImage</a></h2>

<p>MagickStripImage() strips an image of all profiles and comments.</p>

<p>The format of the MagickStripImage method is:</p>

<pre class="text">
MagickBooleanType MagickStripImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickSwirlImage">MagickSwirlImage</a></h2>

<p>MagickSwirlImage() swirls the pixels about the center of the image, where degrees indicates the sweep of the arc through which each pixel is moved. You get a more dramatic effect as the degrees move from 1 to 360.</p>

<p>The format of the MagickSwirlImage method is:</p>

<pre class="text">
MagickBooleanType MagickSwirlImage(MagickWand *wand,const double degrees,
  const PixelInterpolateMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>degrees</dt>
<dd>Define the tightness of the swirling effect. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTextureImage">MagickTextureImage</a></h2>

<p>MagickTextureImage() repeatedly tiles the texture image across and down the image canvas.</p>

<p>The format of the MagickTextureImage method is:</p>

<pre class="text">
MagickWand *MagickTextureImage(MagickWand *wand,
  const MagickWand *texture_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>texture_wand</dt>
<dd>the texture wand </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickThresholdImage">MagickThresholdImage</a></h2>

<p>MagickThresholdImage() changes the value of individual pixels based on the intensity of each pixel compared to threshold.  The result is a high-contrast, two color image.</p>

<p>The format of the MagickThresholdImage method is:</p>

<pre class="text">
MagickBooleanType MagickThresholdImage(MagickWand *wand,
  const double threshold)
MagickBooleanType MagickThresholdImageChannel(MagickWand *wand,
  const ChannelType channel,const double threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>channel</dt>
<dd>the image channel(s). </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>Define the threshold value. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickThumbnailImage">MagickThumbnailImage</a></h2>

<p>MagickThumbnailImage()  changes the size of an image to the given dimensions and removes any associated profiles.  The goal is to produce small low cost thumbnail images suited for display on the Web.</p>

<p>The format of the MagickThumbnailImage method is:</p>

<pre class="text">
MagickBooleanType MagickThumbnailImage(MagickWand *wand,
  const size_t columns,const size_t rows)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTintImage">MagickTintImage</a></h2>

<p>MagickTintImage() applies a color vector to each pixel in the image.  The length of the vector is 0 for black and white and at its maximum for the midtones.  The vector weighting function is f(x)=(1-(4.0*((x-0.5)*(x-0.5)))).</p>

<p>The format of the MagickTintImage method is:</p>

<pre class="text">
MagickBooleanType MagickTintImage(MagickWand *wand,
  const PixelWand *tint,const PixelWand *blend)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>tint</dt>
<dd>the tint pixel wand. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>the alpha pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTransformImage">MagickTransformImage</a></h2>

<p>MagickTransformImage() is a convenience method that behaves like MagickResizeImage() or MagickCropImage() but accepts scaling and/or cropping information as a region geometry specification.  If the operation fails, a NULL image handle is returned.</p>

<p>The format of the MagickTransformImage method is:</p>

<pre class="text">
MagickWand *MagickTransformImage(MagickWand *wand,const char *crop,
  const char *geometry)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>crop</dt>
<dd>A crop geometry string.  This geometry defines a subregion of the image to crop. </dd>

<dd> </dd>
<dt>geometry</dt>
<dd>An image geometry string.  This geometry defines the final size of the image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTransformImageColorspace">MagickTransformImageColorspace</a></h2>

<p>MagickTransformImageColorspace() transform the image colorspace, setting the images colorspace while transforming the images data to that colorspace.</p>

<p>The format of the MagickTransformImageColorspace method is:</p>

<pre class="text">
MagickBooleanType MagickTransformImageColorspace(MagickWand *wand,
  const ColorspaceType colorspace)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>colorspace</dt>
<dd>the image colorspace:   UndefinedColorspace, sRGBColorspace, RGBColorspace, GRAYColorspace, OHTAColorspace, XYZColorspace, YCbCrColorspace, YCCColorspace, YIQColorspace, YPbPrColorspace, YPbPrColorspace, YUVColorspace, CMYKColorspace, HSLColorspace, HWBColorspace. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTransparentPaintImage">MagickTransparentPaintImage</a></h2>

<p>MagickTransparentPaintImage() changes any pixel that matches color with the color defined by fill.</p>

<p>The format of the MagickTransparentPaintImage method is:</p>

<pre class="text">
MagickBooleanType MagickTransparentPaintImage(MagickWand *wand,
  const PixelWand *target,const double alpha,const double fuzz,
  const MagickBooleanType invert)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>target</dt>
<dd>Change this target color to specified alpha value within the image. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>the level of transparency: 1.0 is fully opaque and 0.0 is fully transparent. </dd>

<dd> </dd>
<dt>fuzz</dt>
<dd>By default target must match a particular pixel color exactly.  However, in many cases two colors may differ by a small amount. The fuzz member of image defines how much tolerance is acceptable to consider two colors as the same.  For example, set fuzz to 10 and the color red at intensities of 100 and 102 respectively are now interpreted as the same color for the purposes of the floodfill. </dd>

<dd> </dd>
<dt>invert</dt>
<dd>paint any pixel that does not match the target color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTransposeImage">MagickTransposeImage</a></h2>

<p>MagickTransposeImage() creates a vertical mirror image by reflecting the pixels around the central x-axis while rotating them 90-degrees.</p>

<p>The format of the MagickTransposeImage method is:</p>

<pre class="text">
MagickBooleanType MagickTransposeImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTransverseImage">MagickTransverseImage</a></h2>

<p>MagickTransverseImage() creates a horizontal mirror image by reflecting the pixels around the central y-axis while rotating them 270-degrees.</p>

<p>The format of the MagickTransverseImage method is:</p>

<pre class="text">
MagickBooleanType MagickTransverseImage(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickTrimImage">MagickTrimImage</a></h2>

<p>MagickTrimImage() remove edges that are the background color from the image.</p>

<p>The format of the MagickTrimImage method is:</p>

<pre class="text">
MagickBooleanType MagickTrimImage(MagickWand *wand,const double fuzz)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>fuzz</dt>
<dd>By default target must match a particular pixel color exactly.  However, in many cases two colors may differ by a small amount. The fuzz member of image defines how much tolerance is acceptable to consider two colors as the same.  For example, set fuzz to 10 and the color red at intensities of 100 and 102 respectively are now interpreted as the same color for the purposes of the floodfill. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickUniqueImageColors">MagickUniqueImageColors</a></h2>

<p>MagickUniqueImageColors() discards all but one of any pixel color.</p>

<p>The format of the MagickUniqueImageColors method is:</p>

<pre class="text">
MagickBooleanType MagickUniqueImageColors(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickUnsharpMaskImage">MagickUnsharpMaskImage</a></h2>

<p>MagickUnsharpMaskImage() sharpens an image.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma). For reasonable results, radius should be larger than sigma.  Use a radius of 0 and UnsharpMaskImage() selects a suitable radius for you.</p>

<p>The format of the MagickUnsharpMaskImage method is:</p>

<pre class="text">
MagickBooleanType MagickUnsharpMaskImage(MagickWand *wand,
  const double radius,const double sigma,const double gain,
  const double threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>gain</dt>
<dd>the percentage of the difference between the original and the blur image that is added back into the original. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>the threshold in pixels needed to apply the diffence gain. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickVignetteImage">MagickVignetteImage</a></h2>

<p>MagickVignetteImage() softens the edges of the image in vignette style.</p>

<p>The format of the MagickVignetteImage method is:</p>

<pre class="text">
MagickBooleanType MagickVignetteImage(MagickWand *wand,
  const double radius,const double sigma,const ssize_t x,
  const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the sigma. </dd>

<dd> </dd>
<dt>x, y</dt>
<dd> Define the x and y ellipse offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickWaveImage">MagickWaveImage</a></h2>

<p>MagickWaveImage()  creates a "ripple" effect in the image by shifting the pixels vertically along a sine wave whose amplitude and wavelength is specified by the given parameters.</p>

<p>The format of the MagickWaveImage method is:</p>

<pre class="text">
MagickBooleanType MagickWaveImage(MagickWand *wand,
  const double amplitude,const double wave_length,
  const PixelInterpolateMethod method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>amplitude, wave_length</dt>
<dd> Define the amplitude and wave length of the sine wave. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickWhiteThresholdImage">MagickWhiteThresholdImage</a></h2>

<p>MagickWhiteThresholdImage() is like ThresholdImage() but  force all pixels above the threshold into white while leaving all pixels below the threshold unchanged.</p>

<p>The format of the MagickWhiteThresholdImage method is:</p>

<pre class="text">
MagickBooleanType MagickWhiteThresholdImage(MagickWand *wand,
  const PixelWand *threshold)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickWriteImage">MagickWriteImage</a></h2>

<p>MagickWriteImage() writes an image to the specified filename.  If the filename parameter is NULL, the image is written to the filename set by MagickReadImage() or MagickSetImageFilename().</p>

<p>The format of the MagickWriteImage method is:</p>

<pre class="text">
MagickBooleanType MagickWriteImage(MagickWand *wand,
  const char *filename)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickWriteImageFile">MagickWriteImageFile</a></h2>

<p>MagickWriteImageFile() writes an image to an open file descriptor.</p>

<p>The format of the MagickWriteImageFile method is:</p>

<pre class="text">
MagickBooleanType MagickWriteImageFile(MagickWand *wand,FILE *file)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>file</dt>
<dd>the file descriptor. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickWriteImages">MagickWriteImages</a></h2>

<p>MagickWriteImages() writes an image or image sequence.</p>

<p>The format of the MagickWriteImages method is:</p>

<pre class="text">
MagickBooleanType MagickWriteImages(MagickWand *wand,
  const char *filename,const MagickBooleanType adjoin)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd> </dd>
<dt>adjoin</dt>
<dd>join images into a single multi-image file. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-image_8c.html" id="MagickWriteImagesFile">MagickWriteImagesFile</a></h2>

<p>MagickWriteImagesFile() writes an image sequence to an open file descriptor.</p>

<p>The format of the MagickWriteImagesFile method is:</p>

<pre class="text">
MagickBooleanType MagickWriteImagesFile(MagickWand *wand,FILE *file)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>file</dt>
<dd>the file descriptor. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="magick-image.php#">Back to top</a> •
    <a href="http://pgp.mit.edu:11371/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="../script/contact.php">Contact Us</a></p>
        <p><small>©  1999-2016 ImageMagick Studio LLC</small></p>
  </footer>
</div><!-- /.container -->

  <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
  <script src="http://nextgen.imagemagick.org/js/magick.php"></script>
</div>
</body>
</html>

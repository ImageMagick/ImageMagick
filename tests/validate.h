/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  Wizard the License for the specific language governing permissions and
  limitations under the License.

  ImageMagick test vectors.
*/
#ifndef _IMAGEMAGICK_VALIDATE_H
#define _IMAGEMAGICK_VALIDATE_H

#define ReferenceFilename  "rose:"
#define ReferenceImageFormat  "MIFF"

static const char
  *compare_options[] =
  {
    "-metric RMSE -fuzz 5%",
    "-metric AE -fuzz 5%",
    (const char *) NULL
  };

static const char
  *composite_options[] =
  {
    "",
    "-compose Atop",
    "-compose Blend",
    "-compose Bumpmap",
    "-compose ChangeMask",
    "-compose Clear",
    "-compose ColorBurn",
    "-compose ColorDodge",
    "-compose Colorize",
    "-compose CopyBlack",
    "-compose CopyBlue",
    "-compose CopyCyan",
    "-compose CopyGreen",
    "-compose Copy",
    "-compose CopyMagenta",
    "-compose CopyOpacity",
    "-compose CopyRed",
    "-compose CopyYellow",
    "-compose Darken",
    "-compose Divide",
    "-compose Dst",
    "-compose Difference",
    "-compose Displace",
    "-compose Dissolve",
    "-compose DstAtop",
    "-compose DstIn",
    "-compose DstOut",
    "-compose DstOver",
    "-compose Dst",
    "-compose Exclusion",
    "-compose HardLight",
    "-compose Hue",
    "-compose In",
    "-compose Lighten",
    "-compose LinearLight",
    "-compose Luminize",
    "-compose Minus",
    "-compose Modulate",
    "-compose ModulusAdd",
    "-compose ModulusSubtract",
    "-compose Multiply",
    "-compose None",
    "-compose Out",
    "-compose Overlay",
    "-compose Over",
    "-compose Plus",
    "-compose Replace",
    "-compose Saturate",
    "-compose Screen",
    "-compose SoftLight",
    "-compose Src",
    "-compose SrcAtop",
    "-compose SrcIn",
    "-compose SrcOut",
    "-compose SrcOver",
    "-compose Src",
    "-compose Threshold",
    "-compose Xor",
    "-geometry +35+65 -label Magick",
    (const char *) NULL
  };

static const char
  *convert_options[] =
  {
    "-noop",
    "-affine 1,0,0.785,1,0,0 -transform",
    "-black-threshold 20%",
    "-blur 0x0.5",
    "-border 6x6",
    "-canny 0x1+10%+80%",
    "-charcoal 0x1",
    "-chop 8x6+20+30",
    "-colors 16",
    "-colorspace CMYK",
    "-colorspace GRAY",
    "-colorspace HSL",
    "-colorspace HWB",
    "-colorspace OHTA",
    "-colorspace YCbCr",
    "-colorspace YIQ",
    "-colorspace YUV",
    "-contrast",
    "+contrast",
    "-convolve 1,1,1,1,4,1,1,1,1",
    "-colorize 30%/20%/50%",
    "-crop 17x9+10+10",
    "-cycle 200",
    "-despeckle",
    "-draw \"rectangle 20,10 80,50\"",
    "-edge 0x1",
    "-emboss 0x1",
    "-enhance",
    "-equalize",
    "-flip",
    "-flop",
    "-frame 15x15+3+3",
    "-fx \"(1.0/(1.0+exp(10.0*(0.5-u)))-0.006693)*1.0092503\"",
    "-gamma 1.6",
    "-gaussian 0x0.5",
    "-implode 0.5",
    "-implode -1",
    "-label Magick",
    "-lat 10x10-5%",
    "-level 10%,1.2,90%",
    "-map netscape:",
    "-median 2",
    "-modulate 110/100/95",
    "-monochrome",
    "-motion-blur 0x3+30",
    "-negate",
    "+noise Uniform",
    "+noise Gaussian",
    "+noise Multiplicative",
    "+noise Impulse",
    "+noise Laplacian",
    "+noise Poisson",
    "-noise 2",
    "-normalize",
    "-fill blue -fuzz 35% -opaque red",
    "-ordered-dither 2x2",
    "-paint 0x1",
    "-raise 10x10",
    "-random-threshold 10%",
    "-color-matrix '0.9 0 0, 0 0.9 0, 0 0 1.2'",
    "-density 75x75 -resample 50x50",
    "-resize 10%",
    "-resize 50%",
    "-resize 50x150%",
    "-resize 100%",
    "-resize 150%",
    "-resize 150x75%",
    "-roll +20+10",
    "-rotate 0",
    "-rotate 45",
    "-rotate 90",
    "-rotate 180",
    "-rotate 270",
    "-sample 5%",
    "-sample 50%",
    "-sample 50x150%",
    "-sample 100%",
    "-sample 150%",
    "-sample 150x50%",
    "-scale 5%",
    "-scale 50%",
    "-scale 50x150%",
    "-scale 100%",
    "-scale 150%",
    "-scale 150x50%",
    "-segment 1x1.5",
    "-shade 30x30",
    "-sharpen 0x1.0",
    "-shave 10x10",
    "-shear 45x45",
    "-size 130x194",
    "-solarize 50%",
    "-spread 3",
    "-swirl 90",
    "-threshold 35%",
    "-fuzz 35% -transparent red",
    "-fuzz 5% -trim",
    "-unsharp 0x1.0+20+1",
    "-wave 25x150",
    "-white-threshold 80%",
    (const char *) NULL
  };

static const char
  *identify_options[] =
  {
    "",
    "-verbose",
    "-features 1 -verbose",
    "-unique -verbose",
    (const char *) NULL
  };

static const char
  *montage_options[] =
  {
    "",
    "-frame 5",
    "-geometry 13x19+10+5 -gravity Center",
    "-label %f",
    "-pointsize 10",
    "-shadow",
    "-tile 3x3",
    (const char *) NULL
  };

static const char
  *stream_options[] =
  {
    "",
    (const char *) NULL
  };

struct ReferenceFormats
{
  const char
    *magick;

  CompressionType
    compression;

  double
    fuzz;
};

static const struct ReferenceFormats
  reference_formats[] =
  {
    { "ART", UndefinedCompression, 0.0 },
    { "AVS", UndefinedCompression, 0.0 },
    { "BMP", UndefinedCompression, 0.0 },
    { "BMP2", UndefinedCompression, 0.0 },
    { "BMP3", UndefinedCompression, 0.0 },
    { "CIN", UndefinedCompression, 0.0 },
    { "CMYK", UndefinedCompression, 0.0 },
    { "CMYKA", UndefinedCompression, 0.0 },
    { "CUT", UndefinedCompression, 0.0 },
    { "DCM", UndefinedCompression, 0.0 },
    { "DCR", UndefinedCompression, 0.0 },
    { "DCX", UndefinedCompression, 0.0 },
    { "DDS", UndefinedCompression, 0.0 },
    { "DFONT", UndefinedCompression, 0.0 },
    { "DJVU", UndefinedCompression, 0.0 },
    { "DNG", UndefinedCompression, 0.0 },
    { "DOT", UndefinedCompression, 0.0 },
    { "DPS", UndefinedCompression, 0.0 },
    { "DPX", UndefinedCompression, 0.004 },
    { "ERF", UndefinedCompression, 0.0 },
    { "EXR", UndefinedCompression, 0.0 },
    { "FPX", UndefinedCompression, 0.0 },
    { "FRACTAL", UndefinedCompression, 0.0 },
    { "GIF", UndefinedCompression, 0.0 },
    { "GIF87", UndefinedCompression, 0.0 },
    { "GRAY", UndefinedCompression, 0.004 },
    { "HTM", UndefinedCompression, 0.0 },
    { "HTML", UndefinedCompression, 0.0 },
    { "ICB", UndefinedCompression, 0.0 },
    { "ICO", UndefinedCompression, 0.0 },
    { "ICON", UndefinedCompression, 0.0 },
    { "INFO", UndefinedCompression, 0.0 },
    { "JBG", UndefinedCompression, 0.0 },
    { "JNG", UndefinedCompression, 0.004 },
    { "JNG", JPEGCompression, 0.004 },
    { "JP2", UndefinedCompression, 0.004 },
    { "J2K", UndefinedCompression, 0.004 },
    { "JPEG", UndefinedCompression, 0.004 },
    { "JPG", UndefinedCompression, 0.004 },
    { "K25", UndefinedCompression, 0.0 },
    { "KDC", UndefinedCompression, 0.0 },
    { "MATTE", UndefinedCompression, 0.0 },
    { "MIFF", UndefinedCompression, 0.0 },
    { "MNG", UndefinedCompression, 0.0 },
    { "MONO", UndefinedCompression, 0.0 },
    { "MRW", UndefinedCompression, 0.0 },
    { "MTV", UndefinedCompression, 0.0 },
    { "NEF", UndefinedCompression, 0.0 },
    { "ORF", UndefinedCompression, 0.0 },
    { "OTB", UndefinedCompression, 0.0 },
    { "OTF", UndefinedCompression, 0.0 },
    { "PAL", UndefinedCompression, 0.0 },
    { "PAM", UndefinedCompression, 0.0 },
    { "PBM", UndefinedCompression, 0.0 },
    { "PCT", UndefinedCompression, 0.004 },
    { "PCX", UndefinedCompression, 0.0 },
    { "PEF", UndefinedCompression, 0.0 },
    { "PFA", UndefinedCompression, 0.0 },
    { "PFB", UndefinedCompression, 0.0 },
    { "PFM", UndefinedCompression, 0.004 },
    { "PGM", UndefinedCompression, 0.0 },
    { "PGX", UndefinedCompression, 0.0 },
    { "PICT", UndefinedCompression, 0.004 },
    { "PIX", UndefinedCompression, 0.0 },
    { "PJPEG", UndefinedCompression, 0.004 },
    { "PLASMA", UndefinedCompression, 0.0 },
    { "PNG", UndefinedCompression, 0.0 },
    { "PNG8", UndefinedCompression, 0.0 },
    { "PNG24", UndefinedCompression, 0.0 },
    { "PNG32", UndefinedCompression, 0.0 },
    { "PNG48", UndefinedCompression, 0.0 },
    { "PNG64", UndefinedCompression, 0.0 },
    { "PNG00", UndefinedCompression, 0.0 },
    { "PNM", UndefinedCompression, 0.0 },
    { "PPM", UndefinedCompression, 0.0 },
    { "PREVIEW", UndefinedCompression, 0.0 },
    { "PTIF", UndefinedCompression, 0.0 },
    { "PWP", UndefinedCompression, 0.0 },
    { "RADIAL-GR", UndefinedCompression, 0.0 },
    { "RAF", UndefinedCompression, 0.0 },
    { "RAS", UndefinedCompression, 0.0 },
    { "RGB", UndefinedCompression, 0.0 },
    { "RGBA", UndefinedCompression, 0.004 },
    { "RGBO", UndefinedCompression, 0.004 },
    { "RLA", UndefinedCompression, 0.0 },
    { "RLE", UndefinedCompression, 0.0 },
    { "SCR", UndefinedCompression, 0.0 },
    { "SCT", UndefinedCompression, 0.0 },
    { "SFW", UndefinedCompression, 0.0 },
    { "SGI", UndefinedCompression, 0.0 },
    { "SHTML", UndefinedCompression, 0.0 },
    { "SR2", UndefinedCompression, 0.0 },
    { "SRF", UndefinedCompression, 0.0 },
    { "STEGANO", UndefinedCompression, 0.0 },
    { "SUN", UndefinedCompression, 0.0 },
    { "TGA", UndefinedCompression, 0.0 },
    { "TIFF", UndefinedCompression, 0.0 },
    { "TIFF64", UndefinedCompression, 0.0 },
    { "TILE", UndefinedCompression, 0.0 },
    { "TIM", UndefinedCompression, 0.0 },
    { "TTC", UndefinedCompression, 0.0 },
    { "TTF", UndefinedCompression, 0.0 },
    { "TXT", UndefinedCompression, 0.0 },
    { "UIL", UndefinedCompression, 0.0 },
    { "UYVY", UndefinedCompression, 0.0 },
    { "VDA", UndefinedCompression, 0.0 },
    { "VICAR", UndefinedCompression, 0.0 },
    { "VIFF", UndefinedCompression, 0.004 },
    { "VST", UndefinedCompression, 0.0 },
    { "WBMP", UndefinedCompression, 0.0 },
    { "WPG", UndefinedCompression, 0.0 },
    { "X3F", UndefinedCompression, 0.0 },
    { "XBM", UndefinedCompression, 0.0 },
    { "XCF", UndefinedCompression, 0.0 },
    { "XPM", UndefinedCompression, 0.004 },
    { "XPS", UndefinedCompression, 0.0 },
    { "XV", UndefinedCompression, 0.004 },
#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
    { "XWD", UndefinedCompression, 0.0 },
#endif
    { "YUV", UndefinedCompression, 0.0 },
    { "YCbCr", UndefinedCompression, 0.0 },
    { "YCbCrA", UndefinedCompression, 0.0 },
#if defined(MAGICKCORE_GS_DELEGATE)
    { "AI", UndefinedCompression, 0.0 },
    { "EPDF", UndefinedCompression, 0.0 },
    { "EPI", UndefinedCompression, 0.0 },
    { "EPS", UndefinedCompression, 0.0 },
    { "EPS2", UndefinedCompression, 0.0 },
    { "EPS3", UndefinedCompression, 0.0 },
    { "EPSF", UndefinedCompression, 0.0 },
    { "EPSI", UndefinedCompression, 0.0 },
    { "EPT", UndefinedCompression, 0.0 },
    { "PDF", UndefinedCompression, 0.0 },
    { "PDF", ZipCompression, 0.0 },
    { "PDF", FaxCompression, 0.0 },
    { "PDF", JPEGCompression, 0.004 },
    { "PDF", RLECompression, 0.0 },
    { "PDF", LZWCompression, 0.0 },
    { "PDFA", UndefinedCompression, 0.0 },
    { "PS", UndefinedCompression, 0.0 },
    { "PS2", UndefinedCompression, 0.0 },
    { "PS3", UndefinedCompression, 0.0 },
    { "PS3", ZipCompression, 0.0 },
    { "PS3", FaxCompression, 0.0 },
    { "PS3", JPEGCompression, 0.004 },
    { "PS3", RLECompression, 0.0 },
    { "PS3", LZWCompression, 0.0 },
#endif
    { (const char *) NULL, UndefinedCompression, 0.0 }
  };

static const char
  *reference_map[] =
  {
    "bgro",
    "bgrp",
    "bgr",
    "cmyk",
    "cmy",
    "i",
    "prgb",
    "rgba",
    "rgbo",
    "rgb",
    (char *) NULL
  };

struct ReferenceStorage
{
  StorageType
    type;

  size_t
    quantum;
};

static const struct ReferenceStorage
  reference_storage[] =
  {
    { CharPixel, sizeof(unsigned char) },
    { DoublePixel, sizeof(double) },
    { FloatPixel, sizeof(float) },
    { LongPixel, sizeof(unsigned int) },
    { LongLongPixel, sizeof(MagickSizeType) },
    { ShortPixel, sizeof(unsigned short) },
    { UndefinedPixel, 0 }
  };

struct ReferenceTypes
{
  ImageType
    type;

  size_t
    depth;
};

static const struct ReferenceTypes
  reference_types[] =
  {
    { TrueColorType, 8 },
    { TrueColorAlphaType, 8 },
    { GrayscaleType, 8 },
    { GrayscaleAlphaType, 8 },
    { PaletteType, 8 },
    { PaletteAlphaType, 8 },
    { PaletteBilevelAlphaType, 8 },
    { BilevelType, 1 },
    { ColorSeparationType, 8 },
    { ColorSeparationAlphaType, 8 },
    { TrueColorType, 10 },
    { TrueColorType, 12 },
    { TrueColorType, 16 },
    { UndefinedType, 0 }
  };

#endif

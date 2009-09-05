/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

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
    "",
    "-compose Src",
    "-highlight-color SeaGreen",
    "-metric AE -fuzz 5%",
    (const char *) NULL
  };

static const char
  *composite_options[] =
  {
    "",
    "-compose Add",
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
    "-compose Subtract",
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
    "-recolor '0.9 0 0, 0 0.9 0, 0 0 1.2'",
    "-density 75x75 -resample 50x50",
    "-resize 10%",
    "-resize 50%",
    "-resize 150%",
    "-roll +20+10",
    "-rotate 0",
    "-rotate 45",
    "-rotate 90",
    "-rotate 180",
    "-rotate 270",
    "-sample 5%",
    "-sample 50%",
    "-sample 150%",
    "-scale 5%",
    "-scale 50%",
    "-scale 150%",
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

  double
    fuzz;
};

static const struct ReferenceFormats
  reference_formats[] =
  {
    { "AI", 0.0 },
    { "ART", 0.0 },
    { "AVS", 0.0 },
    { "BMP", 0.0 },
    { "CIN", 0.0 },
    { "CMYK", 0.0 },
    { "CMYKA", 0.0 },
    { "CUT", 0.0 },
    { "DCM", 0.0 },
    { "DCR", 0.0 },
    { "DCX", 0.0 },
    { "DDS", 0.0 },
    { "DFONT", 0.0 },
    { "DJVU", 0.0 },
    { "DNG", 0.0 },
    { "DOT", 0.0 },
    { "DPS", 0.0 },
    { "DPX", 0.0 },
    { "ERF", 0.0 },
    { "EXR", 0.0 },
    { "FITS", 0.003 },
    { "FPX", 0.0 },
    { "FRACTAL", 0.0 },
    { "FTS", 0.003 },
    { "GIF", 0.0 },
    { "GIF87", 0.0 },
    { "GRAY", 0.0 },
    { "HTM", 0.0 },
    { "HRZ", 0.0 },
    { "HTML", 0.0 },
    { "ICB", 0.0 },
    { "ICO", 0.0 },
    { "ICON", 0.0 },
    { "INFO", 0.0 },
    { "INLINE", 0.0 },
    { "JBG", 0.0 },
    { "JNG", 0.003 },
    { "JP2", 0.3 },
    { "JPC", 0.0 },
    { "JPEG", 0.003 },
    { "JPG", 0.003 },
    { "K25", 0.0 },
    { "KDC", 0.0 },
    { "MAT", 0.0 },
    { "MATTE", 0.0 },
    { "MIFF", 0.0 },
    { "MNG", 0.0 },
    { "MONO", 0.0 },
    { "MOV", 0.0 },
    { "MRW", 0.0 },
    { "MTV", 0.0 },
    { "NEF", 0.0 },
    { "ORF", 0.0 },
    { "OTB", 0.0 },
    { "OTF", 0.0 },
    { "PAL", 0.0 },
    { "PAM", 0.0 },
    { "PBM", 0.0 },
    { "PCT", 0.003 },
    { "PCX", 0.0 },
    { "PEF", 0.0 },
    { "PFA", 0.0 },
    { "PFB", 0.0 },
    { "PFM", 0.0 },
    { "PGM", 0.0 },
    { "PGX", 0.0 },
    { "PICT", 0.003 },
    { "PIX", 0.0 },
    { "PJPEG", 0.003 },
    { "PLASMA", 0.0 },
    { "PNG", 0.0 },
    { "PNG24", 0.0 },
    { "PNG32", 0.0 },
    { "PNM", 0.0 },
    { "PPM", 0.0 },
    { "PREVIEW", 0.0 },
    { "PTIF", 0.0 },
    { "PWP", 0.0 },
    { "RADIAL-GR", 0.0 },
    { "RAF", 0.0 },
    { "RAS", 0.0 },
    { "RGB", 0.0 },
    { "RGBA", 0.003 },
    { "RGBO", 0.0 },
    { "RLA", 0.0 },
    { "RLE", 0.0 },
    { "SCR", 0.0 },
    { "SCT", 0.0 },
    { "SFW", 0.0 },
    { "SGI", 0.0 },
    { "SHTML", 0.0 },
    { "SR2", 0.0 },
    { "SRF", 0.0 },
    { "STEGANO", 0.0 },
    { "SUN", 0.0 },
    { "TGA", 0.0 },
    { "TIFF", 0.0 },
    { "TIFF64", 0.0 },
    { "TILE", 0.0 },
    { "TIM", 0.0 },
    { "TTC", 0.0 },
    { "TTF", 0.0 },
    { "TXT", 0.0 },
    { "UIL", 0.0 },
    { "UYVY", 0.0 },
    { "VDA", 0.0 },
    { "VICAR", 0.0 },
    { "VIFF", 0.0 },
    { "VST", 0.0 },
    { "WBMP", 0.0 },
    { "WPG", 0.0 },
    { "X3F", 0.0 },
    { "XBM", 0.0 },
    { "XCF", 0.0 },
    { "XPM", 0.0 },
    { "XPS", 0.0 },
    { "XV", 0.0 },
    { "XWD", 0.0 },
    { "YUV", 0.0 },
    { "YCbCr", 0.0 },
    { "YCbCrA", 0.0 },
#if defined(MAGICKCORE_GS_DELEGATE)
    { "EPDF", 0.0 },
    { "EPI", 0.0 },
    { "EPS", 0.0 },
    { "EPS2", 0.0 },
    { "EPS3", 0.0 },
    { "EPSF", 0.0 },
    { "EPSI", 0.0 },
    { "EPT", 0.0 },
    { "PDF", 0.0 },
    { "PDFA", 0.0 },
    { "PS", 0.0 },
    { "PS2", 0.0 },
    { "PS3", 0.0 },
#endif
    { (const char *) NULL, 0.0 }
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
    { IntegerPixel, sizeof(unsigned int) },
    { LongPixel, sizeof(unsigned long) },
    { ShortPixel, sizeof(unsigned short) },
    { UndefinedPixel, 0 }
  };

struct ReferenceTypes
{
  ImageType
    type;

  unsigned long
    depth;
};

static const struct ReferenceTypes
  reference_types[] =
  {
    { TrueColorType, 8 },
    { TrueColorMatteType, 8 },
    { GrayscaleType, 8 },
    { GrayscaleMatteType, 8 },
    { PaletteType, 8 },
    { PaletteMatteType, 8 },
    { PaletteBilevelMatteType, 8 },
    { BilevelType, 1 },
    { ColorSeparationType, 8 },
    { ColorSeparationMatteType, 8 },
    { TrueColorType, 10 },
    { TrueColorType, 12 },
    { TrueColorType, 16 },
    { UndefinedType, 0 }
  };

#endif

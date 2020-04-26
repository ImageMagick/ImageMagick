/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   OOO   PPPP   TTTTT  IIIII   OOO   N   N                   %
%                  O   O  P   P    T      I    O   O  NN  N                   %
%                  O   O  PPPP     T      I    O   O  N N N                   %
%                  O   O  P        T      I    O   O  N  NN                   %
%                   OOO   P        T    IIIII   OOO   N   N                   %
%                                                                             %
%                                                                             %
%                         MagickCore Option Methods                           %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2000                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/compare.h"
#include "MagickCore/constitute.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fourier.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/layer.h"
#include "MagickCore/mime-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/montage.h"
#include "MagickCore/morphology.h"
#include "MagickCore/option.h"
#include "MagickCore/option-private.h"
#include "MagickCore/pixel.h"
#include "MagickCore/policy.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resample.h"
#include "MagickCore/resource_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/threshold.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/visual-effects.h"

/*
  ImageMagick options.
*/
static const OptionInfo
  AlignOptions[] =
  {
    { "Undefined", UndefinedAlign, UndefinedOptionFlag, MagickTrue },
    { "Center", CenterAlign, UndefinedOptionFlag, MagickFalse },
    { "End", RightAlign, UndefinedOptionFlag, MagickFalse },
    { "Left", LeftAlign, UndefinedOptionFlag, MagickFalse },
    { "Middle", CenterAlign, UndefinedOptionFlag, MagickFalse },
    { "Right", RightAlign, UndefinedOptionFlag, MagickFalse },
    { "Start", LeftAlign, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedAlign, UndefinedOptionFlag, MagickFalse }
  },
  AlphaChannelOptions[] =
  {
    { "Undefined", UndefinedAlphaChannel, UndefinedOptionFlag, MagickTrue },
    { "Activate", ActivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Associate", AssociateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Background", BackgroundAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Copy", CopyAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Deactivate", DeactivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Discrete", DiscreteAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Disassociate", DisassociateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Extract", ExtractAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Off", OffAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "On", OnAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Opaque", OpaqueAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Remove", RemoveAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Set", SetAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Shape", ShapeAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Reset", SetAlphaChannel, DeprecateOptionFlag, MagickTrue },
    { "Transparent", TransparentAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedAlphaChannel, UndefinedOptionFlag, MagickFalse }
  },
  AutoThresholdOptions[] =
  {
    { "Undefined", UndefinedThresholdMethod, UndefinedOptionFlag, MagickTrue },
    { "Kapur", KapurThresholdMethod, UndefinedOptionFlag, MagickFalse },
    { "OTSU", OTSUThresholdMethod, UndefinedOptionFlag, MagickFalse },
    { "Triangle", TriangleThresholdMethod, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedThresholdMethod, UndefinedOptionFlag, MagickFalse }
  },
  BooleanOptions[] =
  {
    { "False", MagickFalse, UndefinedOptionFlag, MagickFalse },
    { "True", MagickTrue, UndefinedOptionFlag, MagickFalse },
    { "0", MagickFalse, UndefinedOptionFlag, MagickFalse },
    { "1", MagickTrue, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, MagickFalse, UndefinedOptionFlag, MagickFalse }
  },
  CacheOptions[] =
  {
    { "Disk", DiskCache, UndefinedOptionFlag, MagickFalse },
    { "Distributed", DistributedCache, UndefinedOptionFlag, MagickFalse },
    { "Map", MapCache, UndefinedOptionFlag, MagickFalse },
    { "Memory", MemoryCache, UndefinedOptionFlag, MagickFalse },
    { "Ping", PingCache, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, MagickFalse, UndefinedOptionFlag, MagickFalse }
  },
  ChannelOptions[] =
  {
    { "Undefined", UndefinedChannel, UndefinedOptionFlag, MagickTrue },
    /* special */
    { "All", CompositeChannels, UndefinedOptionFlag, MagickFalse },
    { "Sync", SyncChannels, UndefinedOptionFlag, MagickFalse },
    { "Default", DefaultChannels, UndefinedOptionFlag, MagickFalse },
    /* individual channel */
    { "A", AlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Alpha", AlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Black", BlackChannel, UndefinedOptionFlag, MagickFalse },
    { "B", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Blue", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "C", CyanChannel, UndefinedOptionFlag, MagickFalse },
    { "Chroma", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Cyan", CyanChannel, UndefinedOptionFlag, MagickFalse },
    { "Gray", GrayChannel, UndefinedOptionFlag, MagickFalse },
    { "G", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Green", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "H", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "Hue", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "K", BlackChannel, UndefinedOptionFlag, MagickFalse },
    { "L", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Lightness", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Luminance", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Luminosity", BlueChannel, DeprecateOptionFlag, MagickTrue },
    { "M", MagentaChannel, UndefinedOptionFlag, MagickFalse },
    { "Magenta", MagentaChannel, UndefinedOptionFlag, MagickFalse },
    { "Matte", AlphaChannel, DeprecateOptionFlag, MagickTrue },/*depreciate*/
    { "Meta", MetaChannel, UndefinedOptionFlag, MagickFalse },
    { "Opacity", AlphaChannel, DeprecateOptionFlag, MagickTrue },/*depreciate*/
    { "R", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "Red", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "S", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Saturation", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Y", YellowChannel, UndefinedOptionFlag, MagickFalse },
    { "Yellow", YellowChannel, UndefinedOptionFlag, MagickFalse },
    { "0", (ssize_t) (1L << 0), UndefinedOptionFlag, MagickFalse },
    { "1", (ssize_t) (1L << 1), UndefinedOptionFlag, MagickFalse },
    { "2", (ssize_t) (1L << 2), UndefinedOptionFlag, MagickFalse },
    { "3", (ssize_t) (1L << 3), UndefinedOptionFlag, MagickFalse },
    { "4", (ssize_t) (1L << 4), UndefinedOptionFlag, MagickFalse },
    { "5", (ssize_t) (1L << 5), UndefinedOptionFlag, MagickFalse },
    { "6", (ssize_t) (1L << 6), UndefinedOptionFlag, MagickFalse },
    { "7", (ssize_t) (1L << 7), UndefinedOptionFlag, MagickFalse },
    { "8", (ssize_t) (1L << 8), UndefinedOptionFlag, MagickFalse },
    { "9", (ssize_t) (1L << 9), UndefinedOptionFlag, MagickFalse },
    { "10", (ssize_t) (1L << 10), UndefinedOptionFlag, MagickFalse },
    { "11", (ssize_t) (1L << 11), UndefinedOptionFlag, MagickFalse },
    { "12", (ssize_t) (1L << 12), UndefinedOptionFlag, MagickFalse },
    { "13", (ssize_t) (1L << 13), UndefinedOptionFlag, MagickFalse },
    { "14", (ssize_t) (1L << 14), UndefinedOptionFlag, MagickFalse },
    { "15", (ssize_t) (1L << 15), UndefinedOptionFlag, MagickFalse },
    { "16", (ssize_t) (1L << 16), UndefinedOptionFlag, MagickFalse },
    { "17", (ssize_t) (1L << 17), UndefinedOptionFlag, MagickFalse },
    { "18", (ssize_t) (1L << 18), UndefinedOptionFlag, MagickFalse },
    { "19", (ssize_t) (1L << 19), UndefinedOptionFlag, MagickFalse },
    { "20", (ssize_t) (1L << 20), UndefinedOptionFlag, MagickFalse },
    { "21", (ssize_t) (1L << 21), UndefinedOptionFlag, MagickFalse },
    { "22", (ssize_t) (1L << 22), UndefinedOptionFlag, MagickFalse },
    { "23", (ssize_t) (1L << 23), UndefinedOptionFlag, MagickFalse },
    { "24", (ssize_t) (1L << 24), UndefinedOptionFlag, MagickFalse },
    { "25", (ssize_t) (1L << 25), UndefinedOptionFlag, MagickFalse },
    { "26", (ssize_t) (1L << 26), UndefinedOptionFlag, MagickFalse },
    { "27", (ssize_t) (1L << 27), UndefinedOptionFlag, MagickFalse },
    { "28", (ssize_t) (1L << 28), UndefinedOptionFlag, MagickFalse },
    { "29", (ssize_t) (1L << 29), UndefinedOptionFlag, MagickFalse },
    { "30", (ssize_t) (1L << 30), UndefinedOptionFlag, MagickFalse },
    { "31", (ssize_t) (1L << 31), UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedChannel, UndefinedOptionFlag, MagickFalse }
  },
  ClassOptions[] =
  {
    { "Undefined", UndefinedClass, UndefinedOptionFlag, MagickTrue },
    { "DirectClass", DirectClass, UndefinedOptionFlag, MagickFalse },
    { "PseudoClass", PseudoClass, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedClass, UndefinedOptionFlag, MagickFalse }
  },
  CLIOptions[] =
  {
    { "Setting", 0, UndefinedOptionFlag, MagickFalse },
    { "  adjoin", 0, UndefinedOptionFlag, MagickFalse },
    { "  affine", 0, UndefinedOptionFlag, MagickFalse },
    { "  alpha", 0, UndefinedOptionFlag, MagickFalse },
    { "  antialias", 0, UndefinedOptionFlag, MagickFalse },
    { "  authenticate", 0, UndefinedOptionFlag, MagickFalse },
    { "  background", 0, UndefinedOptionFlag, MagickFalse },
    { "  bias", 0, UndefinedOptionFlag, MagickFalse },
    { "  black-point-compensation", 0, UndefinedOptionFlag, MagickFalse },
    { "  blue-primary", 0, UndefinedOptionFlag, MagickFalse },
    { "  bordercolor", 0, UndefinedOptionFlag, MagickFalse },
    { "  caption", 0, UndefinedOptionFlag, MagickFalse },
    { "  channel", 0, UndefinedOptionFlag, MagickFalse },
    { "  comment", 0, UndefinedOptionFlag, MagickFalse },
    { "  compress", 0, UndefinedOptionFlag, MagickFalse },
    { "  debug", 0, UndefinedOptionFlag, MagickFalse },
    { "  define", 0, UndefinedOptionFlag, MagickFalse },
    { "  delay", 0, UndefinedOptionFlag, MagickFalse },
    { "  density", 0, UndefinedOptionFlag, MagickFalse },
    { "  depth", 0, UndefinedOptionFlag, MagickFalse },
    { "  direction", 0, UndefinedOptionFlag, MagickFalse },
    { "  display", 0, UndefinedOptionFlag, MagickFalse },
    { "  dispose", 0, UndefinedOptionFlag, MagickFalse },
    { "  dither", 0, UndefinedOptionFlag, MagickFalse },
    { "  encoding", 0, UndefinedOptionFlag, MagickFalse },
    { "  endian", 0, UndefinedOptionFlag, MagickFalse },
    { "  extract", 0, UndefinedOptionFlag, MagickFalse },
    { "  family", 0, UndefinedOptionFlag, MagickFalse },
    { "  fill", 0, UndefinedOptionFlag, MagickFalse },
    { "  filter", 0, UndefinedOptionFlag, MagickFalse },
    { "  font", 0, UndefinedOptionFlag, MagickFalse },
    { "  format", 0, UndefinedOptionFlag, MagickFalse },
    { "  fuzz", 0, UndefinedOptionFlag, MagickFalse },
    { "  geometry", 0, UndefinedOptionFlag, MagickFalse },
    { "  gravity", 0, UndefinedOptionFlag, MagickFalse },
    { "  green-primary", 0, UndefinedOptionFlag, MagickFalse },
    { "  interlace", 0, UndefinedOptionFlag, MagickFalse },
    { "  intent", 0, UndefinedOptionFlag, MagickFalse },
    { "  interpolate", 0, UndefinedOptionFlag, MagickFalse },
    { "  label", 0, UndefinedOptionFlag, MagickFalse },
    { "  limit", 0, UndefinedOptionFlag, MagickFalse },
    { "  linewidth", 0, UndefinedOptionFlag, MagickFalse },
    { "  log", 0, UndefinedOptionFlag, MagickFalse },
    { "  loop", 0, UndefinedOptionFlag, MagickFalse },
    { "  mattecolor", 0, UndefinedOptionFlag, MagickFalse },
    { "  monitor", 0, UndefinedOptionFlag, MagickFalse },
    { "  orient", 0, UndefinedOptionFlag, MagickFalse },
    { "  page", 0, UndefinedOptionFlag, MagickFalse },
    { "  pointsize", 0, UndefinedOptionFlag, MagickFalse },
    { "  preview", 0, UndefinedOptionFlag, MagickFalse },
    { "  quality", 0, UndefinedOptionFlag, MagickFalse },
    { "  quiet", 0, UndefinedOptionFlag, MagickFalse },
    { "  read-mask", 0, UndefinedOptionFlag, MagickFalse },
    { "  red-primary", 0, UndefinedOptionFlag, MagickFalse },
    { "  region", 0, UndefinedOptionFlag, MagickFalse },
    { "  render", 0, UndefinedOptionFlag, MagickFalse },
    { "  repage", 0, UndefinedOptionFlag, MagickFalse },
    { "  sampling-factor", 0, UndefinedOptionFlag, MagickFalse },
    { "  scene", 0, UndefinedOptionFlag, MagickFalse },
    { "  seed", 0, UndefinedOptionFlag, MagickFalse },
    { "  size", 0, UndefinedOptionFlag, MagickFalse },
    { "  stretch", 0, UndefinedOptionFlag, MagickFalse },
    { "  stroke", 0, UndefinedOptionFlag, MagickFalse },
    { "  strokewidth", 0, UndefinedOptionFlag, MagickFalse },
    { "  style", 0, UndefinedOptionFlag, MagickFalse },
    { "  texture", 0, UndefinedOptionFlag, MagickFalse },
    { "  tile", 0, UndefinedOptionFlag, MagickFalse },
    { "  transparent-color", 0, UndefinedOptionFlag, MagickFalse },
    { "  treedepth", 0, UndefinedOptionFlag, MagickFalse },
    { "  type", 0, UndefinedOptionFlag, MagickFalse },
    { "  undercolor", 0, UndefinedOptionFlag, MagickFalse },
    { "  units", 0, UndefinedOptionFlag, MagickFalse },
    { "  verbose", 0, UndefinedOptionFlag, MagickFalse },
    { "  virtual-pixel", 0, UndefinedOptionFlag, MagickFalse },
    { "  weight", 0, UndefinedOptionFlag, MagickFalse },
    { "  write-mask", 0, UndefinedOptionFlag, MagickFalse },
    { "Operator", 0, UndefinedOptionFlag, MagickFalse },
    { "  annotate", 0, UndefinedOptionFlag, MagickFalse },
    { "  black-threshold", 0, UndefinedOptionFlag, MagickFalse },
    { "  blur", 0, UndefinedOptionFlag, MagickFalse },
    { "  border", 0, UndefinedOptionFlag, MagickFalse },
    { "  charcoal", 0, UndefinedOptionFlag, MagickFalse },
    { "  chop", 0, UndefinedOptionFlag, MagickFalse },
    { "  clip", 0, UndefinedOptionFlag, MagickFalse },
    { "  clip-path", 0, UndefinedOptionFlag, MagickFalse },
    { "  clip-mask", 0, UndefinedOptionFlag, MagickFalse },
    { "  colors", 0, UndefinedOptionFlag, MagickFalse },
    { "  colorize", 0, UndefinedOptionFlag, MagickFalse },
    { "  colorspace", 0, UndefinedOptionFlag, MagickFalse },
    { "  color-threshold", 0, UndefinedOptionFlag, MagickFalse },
    { "  compose", 0, UndefinedOptionFlag, MagickFalse },
    { "  contrast", 0, UndefinedOptionFlag, MagickFalse },
    { "  convolve", 0, UndefinedOptionFlag, MagickFalse },
    { "  crop", 0, UndefinedOptionFlag, MagickFalse },
    { "  cycle", 0, UndefinedOptionFlag, MagickFalse },
    { "  despeckle", 0, UndefinedOptionFlag, MagickFalse },
    { "  draw", 0, UndefinedOptionFlag, MagickFalse },
    { "  edge", 0, UndefinedOptionFlag, MagickFalse },
    { "  emboss", 0, UndefinedOptionFlag, MagickFalse },
    { "  enhance", 0, UndefinedOptionFlag, MagickFalse },
    { "  equalize", 0, UndefinedOptionFlag, MagickFalse },
    { "  evaluate", 0, UndefinedOptionFlag, MagickFalse },
    { "  extent", 0, UndefinedOptionFlag, MagickFalse },
    { "  flip", 0, UndefinedOptionFlag, MagickFalse },
    { "  flop", 0, UndefinedOptionFlag, MagickFalse },
    { "  floodfill", 0, UndefinedOptionFlag, MagickFalse },
    { "  frame", 0, UndefinedOptionFlag, MagickFalse },
    { "  gamma", 0, UndefinedOptionFlag, MagickFalse },
    { "  gaussian-blur", 0, UndefinedOptionFlag, MagickFalse },
    { "  grayscale", 0, UndefinedOptionFlag, MagickFalse },
    { "  implode", 0, UndefinedOptionFlag, MagickFalse },
    { "  kmeans", 0, UndefinedOptionFlag, MagickFalse },
    { "  lat", 0, UndefinedOptionFlag, MagickFalse },
    { "  level", 0, UndefinedOptionFlag, MagickFalse },
    { "  map", 0, UndefinedOptionFlag, MagickFalse },
    { "  median", 0, UndefinedOptionFlag, MagickFalse },
    { "  modulate", 0, UndefinedOptionFlag, MagickFalse },
    { "  monochrome", 0, UndefinedOptionFlag, MagickFalse },
    { "  negate", 0, UndefinedOptionFlag, MagickFalse },
    { "  noise", 0, UndefinedOptionFlag, MagickFalse },
    { "  normalize", 0, UndefinedOptionFlag, MagickFalse },
    { "  opaque", 0, UndefinedOptionFlag, MagickFalse },
    { "  ordered-dither", 0, UndefinedOptionFlag, MagickFalse },
    { "  paint", 0, UndefinedOptionFlag, MagickFalse },
    { "  posterize", 0, UndefinedOptionFlag, MagickFalse },
    { "  raise", 0, UndefinedOptionFlag, MagickFalse },
    { "  profile", 0, UndefinedOptionFlag, MagickFalse },
    { "  radial-blur", 0, UndefinedOptionFlag, MagickFalse },
    { "  raise", 0, UndefinedOptionFlag, MagickFalse },
    { "  random-threshold", 0, UndefinedOptionFlag, MagickFalse },
    { "  range-threshold", 0, UndefinedOptionFlag, MagickFalse },
    { "  resample", 0, UndefinedOptionFlag, MagickFalse },
    { "  resize", 0, UndefinedOptionFlag, MagickFalse },
    { "  roll", 0, UndefinedOptionFlag, MagickFalse },
    { "  rotate", 0, UndefinedOptionFlag, MagickFalse },
    { "  sample", 0, UndefinedOptionFlag, MagickFalse },
    { "  scale", 0, UndefinedOptionFlag, MagickFalse },
    { "  sepia-tone", 0, UndefinedOptionFlag, MagickFalse },
    { "  segment", 0, UndefinedOptionFlag, MagickFalse },
    { "  shade", 0, UndefinedOptionFlag, MagickFalse },
    { "  shadow", 0, UndefinedOptionFlag, MagickFalse },
    { "  sharpen", 0, UndefinedOptionFlag, MagickFalse },
    { "  shave", 0, UndefinedOptionFlag, MagickFalse },
    { "  shear", 0, UndefinedOptionFlag, MagickFalse },
    { "  sigmoidal-contrast", 0, UndefinedOptionFlag, MagickFalse },
    { "  solarize", 0, UndefinedOptionFlag, MagickFalse },
    { "  splice", 0, UndefinedOptionFlag, MagickFalse },
    { "  spread", 0, UndefinedOptionFlag, MagickFalse },
    { "  strip", 0, UndefinedOptionFlag, MagickFalse },
    { "  swirl", 0, UndefinedOptionFlag, MagickFalse },
    { "  threshold", 0, UndefinedOptionFlag, MagickFalse },
    { "  transparent", 0, UndefinedOptionFlag, MagickFalse },
    { "  thumbnail", 0, UndefinedOptionFlag, MagickFalse },
    { "  tint", 0, UndefinedOptionFlag, MagickFalse },
    { "  transform", 0, UndefinedOptionFlag, MagickFalse },
    { "  trim", 0, UndefinedOptionFlag, MagickFalse },
    { "  unsharp", 0, UndefinedOptionFlag, MagickFalse },
    { "  version", 0, UndefinedOptionFlag, MagickFalse },
    { "  wave", 0, UndefinedOptionFlag, MagickFalse },
    { "  white-point", 0, UndefinedOptionFlag, MagickFalse },
    { "  white-threshold", 0, UndefinedOptionFlag, MagickFalse },
    { "Channel Operator", 0, UndefinedOptionFlag, MagickFalse },
    { "  channel-fx", 0, UndefinedOptionFlag, MagickFalse },
    { "  separate", 0, UndefinedOptionFlag, MagickFalse },
    { "Sequence Operator", 0, UndefinedOptionFlag, MagickFalse },
    { "  append", 0, UndefinedOptionFlag, MagickFalse },
    { "  affinity", 0, UndefinedOptionFlag, MagickFalse },
    { "  average", 0, UndefinedOptionFlag, MagickFalse },
    { "  clut", 0, UndefinedOptionFlag, MagickFalse },
    { "  coalesce", 0, UndefinedOptionFlag, MagickFalse },
    { "  combine", 0, UndefinedOptionFlag, MagickFalse },
    { "  compare", 0, UndefinedOptionFlag, MagickFalse },
    { "  complex", 0, UndefinedOptionFlag, MagickFalse },
    { "  composite", 0, UndefinedOptionFlag, MagickFalse },
    { "  copy", 0, UndefinedOptionFlag, MagickFalse },
    { "  crop", 0, UndefinedOptionFlag, MagickFalse },
    { "  debug", 0, UndefinedOptionFlag, MagickFalse },
    { "  deconstruct", 0, UndefinedOptionFlag, MagickFalse },
    { "  delete", 0, UndefinedOptionFlag, MagickFalse },
    { "  evaluate-sequence", 0, UndefinedOptionFlag, MagickFalse },
    { "  fft", 0, UndefinedOptionFlag, MagickFalse },
    { "  flatten", 0, UndefinedOptionFlag, MagickFalse },
    { "  fx", 0, UndefinedOptionFlag, MagickFalse },
    { "  hald-clut", 0, UndefinedOptionFlag, MagickFalse },
    { "  ift", 0, UndefinedOptionFlag, MagickFalse },
    { "  identify", 0, UndefinedOptionFlag, MagickFalse },
    { "  insert", 0, UndefinedOptionFlag, MagickFalse },
    { "  layers", 0, UndefinedOptionFlag, MagickFalse },
    { "  limit", 0, UndefinedOptionFlag, MagickFalse },
    { "  map", 0, UndefinedOptionFlag, MagickFalse },
    { "  maximum", 0, UndefinedOptionFlag, MagickFalse },
    { "  minimum", 0, UndefinedOptionFlag, MagickFalse },
    { "  morph", 0, UndefinedOptionFlag, MagickFalse },
    { "  mosaic", 0, UndefinedOptionFlag, MagickFalse },
    { "  optimize", 0, UndefinedOptionFlag, MagickFalse },
    { "  print", 0, UndefinedOptionFlag, MagickFalse },
    { "  process", 0, UndefinedOptionFlag, MagickFalse },
    { "  quiet", 0, UndefinedOptionFlag, MagickFalse },
    { "  swap", 0, UndefinedOptionFlag, MagickFalse },
    { "  write", 0, UndefinedOptionFlag, MagickFalse },
    { "Geometry", 0, UndefinedOptionFlag, MagickFalse },
    { "  adaptive-resize", 0, UndefinedOptionFlag, MagickFalse },
    { "  border", 0, UndefinedOptionFlag, MagickFalse },
    { "  borderwidth", 0, UndefinedOptionFlag, MagickFalse },
    { "  chop", 0, UndefinedOptionFlag, MagickFalse },
    { "  crop", 0, UndefinedOptionFlag, MagickFalse },
    { "  density", 0, UndefinedOptionFlag, MagickFalse },
    { "  extent", 0, UndefinedOptionFlag, MagickFalse },
    { "  extract", 0, UndefinedOptionFlag, MagickFalse },
    { "  frame", 0, UndefinedOptionFlag, MagickFalse },
    { "  geometry", 0, UndefinedOptionFlag, MagickFalse },
    { "  iconGeometry", 0, UndefinedOptionFlag, MagickFalse },
    { "  liquid-rescale", 0, UndefinedOptionFlag, MagickFalse },
    { "  page", 0, UndefinedOptionFlag, MagickFalse },
    { "  region", 0, UndefinedOptionFlag, MagickFalse },
    { "  repage", 0, UndefinedOptionFlag, MagickFalse },
    { "  resize", 0, UndefinedOptionFlag, MagickFalse },
    { "  sample", 0, UndefinedOptionFlag, MagickFalse },
    { "  scale", 0, UndefinedOptionFlag, MagickFalse },
    { "  shave", 0, UndefinedOptionFlag, MagickFalse },
    { "  splice", 0, UndefinedOptionFlag, MagickFalse },
    { "  thumbnail", 0, UndefinedOptionFlag, MagickFalse },
    { "  window", 0, UndefinedOptionFlag, MagickFalse },
    { "Stack", 0, UndefinedOptionFlag, MagickFalse },
    { "  clone", 0, UndefinedOptionFlag, MagickFalse },
    { "  delete", 0, UndefinedOptionFlag, MagickFalse },
    { "  insert", 0, UndefinedOptionFlag, MagickFalse },
    { "  swap", 0, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, 0, UndefinedOptionFlag, MagickFalse }
  },
  ClipPathOptions[] =
  {
    { "Undefined", UndefinedPathUnits, UndefinedOptionFlag, MagickTrue },
    { "ObjectBoundingBox", ObjectBoundingBox, UndefinedOptionFlag, MagickFalse },
    { "UserSpace", UserSpace, UndefinedOptionFlag, MagickFalse },
    { "UserSpaceOnUse", UserSpaceOnUse, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPathUnits, UndefinedOptionFlag, MagickFalse }
  },
  ColorspaceOptions[] =
  {
    { "Undefined", UndefinedColorspace, UndefinedOptionFlag, MagickTrue },
    { "CIELab", LabColorspace, UndefinedOptionFlag, MagickFalse },
    { "CMY", CMYColorspace, UndefinedOptionFlag, MagickFalse },
    { "CMYK", CMYKColorspace, UndefinedOptionFlag, MagickFalse },
    { "Gray", GRAYColorspace, UndefinedOptionFlag, MagickFalse },
    { "HCL", HCLColorspace, UndefinedOptionFlag, MagickFalse },
    { "HCLp", HCLpColorspace, UndefinedOptionFlag, MagickFalse },
    { "HSB", HSBColorspace, UndefinedOptionFlag, MagickFalse },
    { "HSI", HSIColorspace, UndefinedOptionFlag, MagickFalse },
    { "HSL", HSLColorspace, UndefinedOptionFlag, MagickFalse },
    { "HSV", HSVColorspace, UndefinedOptionFlag, MagickFalse },
    { "HWB", HWBColorspace, UndefinedOptionFlag, MagickFalse },
    { "Jzazbz", JzazbzColorspace, UndefinedOptionFlag, MagickFalse },
    { "Lab", LabColorspace, UndefinedOptionFlag, MagickFalse },
    { "LCH", LCHColorspace, UndefinedOptionFlag, MagickFalse },
    { "LCHab", LCHabColorspace, UndefinedOptionFlag, MagickFalse },
    { "LCHuv", LCHuvColorspace, UndefinedOptionFlag, MagickFalse },
    { "LinearGray", LinearGRAYColorspace, UndefinedOptionFlag, MagickFalse },
    { "LMS", LMSColorspace, UndefinedOptionFlag, MagickFalse },
    { "Log", LogColorspace, UndefinedOptionFlag, MagickFalse },
    { "Luv", LuvColorspace, UndefinedOptionFlag, MagickFalse },
    { "OHTA", OHTAColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec601YCbCr", Rec601YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec709YCbCr", Rec709YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "RGB", RGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "scRGB", scRGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "sRGB", sRGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "Transparent", TransparentColorspace, UndefinedOptionFlag, MagickFalse },
    { "xyY", xyYColorspace, UndefinedOptionFlag, MagickFalse },
    { "XYZ", XYZColorspace, UndefinedOptionFlag, MagickFalse },
    { "YCbCr", YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "YDbDr", YDbDrColorspace, UndefinedOptionFlag, MagickFalse },
    { "YCC", YCCColorspace, UndefinedOptionFlag, MagickFalse },
    { "YIQ", YIQColorspace, UndefinedOptionFlag, MagickFalse },
    { "YPbPr", YPbPrColorspace, UndefinedOptionFlag, MagickFalse },
    { "YUV", YUVColorspace, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedColorspace, UndefinedOptionFlag, MagickFalse }
  },
  ComplexOptions[] =
  {
    { "Undefined", UndefinedComplexOperator, UndefinedOptionFlag, MagickTrue },
    { "Add", AddComplexOperator, UndefinedOptionFlag, MagickFalse },
    { "Conjugate", ConjugateComplexOperator, UndefinedOptionFlag, MagickFalse },
    { "Divide", DivideComplexOperator, UndefinedOptionFlag, MagickFalse },
    { "MagnitudePhase", MagnitudePhaseComplexOperator, UndefinedOptionFlag, MagickFalse },
    { "Multiply", MultiplyComplexOperator, UndefinedOptionFlag, MagickFalse },
    { "RealImaginary", RealImaginaryComplexOperator, UndefinedOptionFlag, MagickFalse },
    { "Subtract", SubtractComplexOperator, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedComplexOperator, UndefinedOptionFlag, MagickFalse }
  },
  CommandOptions[] =
  {
    /*
      Must be ordered lexigraphically.
    */
    { "+alpha", 1L, DeprecateOptionFlag, MagickTrue },
    { "-alpha", 1L, SimpleOperatorFlag, MagickFalse },
    { "+background", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-background", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+format", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-format", 1L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-quiet", 0L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "+quiet", 0L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "-regard-warnings", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+regard-warnings", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+repage", 0L, SimpleOperatorFlag, MagickFalse },
    { "-repage", 1L, SimpleOperatorFlag, MagickFalse },
    { "+size", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-size", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+virtual-pixel", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-virtual-pixel", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+blur", 0L, DeprecateOptionFlag, MagickTrue },
    { "-blur", 1L, SimpleOperatorFlag, MagickFalse },
    { "+resize", 1L, DeprecateOptionFlag, MagickTrue },
    { "-resize", 1L, SimpleOperatorFlag, MagickFalse },
    { "(", 0L, NoImageOperatorFlag, MagickTrue },
    { ")", 0L, NoImageOperatorFlag, MagickTrue },
    { "{", 0L, NoImageOperatorFlag, MagickTrue },
    { "}", 0L, NoImageOperatorFlag, MagickTrue },
    { "--", 1L, NoImageOperatorFlag, MagickTrue },
    { "+adaptive-blur", 1L, DeprecateOptionFlag, MagickTrue },
    { "-adaptive-blur", 1L, SimpleOperatorFlag, MagickFalse },
    { "+adaptive-resize", 1L, DeprecateOptionFlag, MagickTrue },
    { "-adaptive-resize", 1L, SimpleOperatorFlag, MagickFalse },
    { "+adaptive-sharpen", 1L, DeprecateOptionFlag, MagickTrue },
    { "-adaptive-sharpen", 1L, SimpleOperatorFlag, MagickFalse },
    { "-adjoin", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+adjoin", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+affine", 0L, ReplacedOptionFlag | DrawInfoOptionFlag, MagickTrue },
    { "-affine", 1L, ReplacedOptionFlag | DrawInfoOptionFlag, MagickTrue },
    { "+affinity", 0L, DeprecateOptionFlag, MagickTrue },
    { "-affinity", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "+mattecolor", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-mattecolor", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+annotate", 0L, DeprecateOptionFlag, MagickTrue },
    { "-annotate", 2L, SimpleOperatorFlag | AlwaysInterpretArgsFlag, MagickFalse },
    { "-antialias", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+antialias", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-append", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+append", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+attenuate", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-attenuate", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+authenticate", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-authenticate", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+auto-gamma", 0L, DeprecateOptionFlag, MagickTrue },
    { "-auto-gamma", 0L, SimpleOperatorFlag, MagickFalse },
    { "+auto-level", 0L, DeprecateOptionFlag, MagickTrue },
    { "-auto-level", 0L, SimpleOperatorFlag, MagickFalse },
    { "+auto-orient", 0L, DeprecateOptionFlag, MagickTrue },
    { "-auto-orient", 0L, SimpleOperatorFlag, MagickFalse },
    { "+auto-threshold", 1L, DeprecateOptionFlag, MagickTrue },
    { "-auto-threshold", 1L, SimpleOperatorFlag, MagickFalse },
    { "+average", 0L, DeprecateOptionFlag, MagickTrue },
    { "-average", 0L, ReplacedOptionFlag | ListOperatorFlag | FireOptionFlag, MagickTrue },
    { "+backdrop", 0L, NonMagickOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-backdrop", 1L, NonMagickOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+bench", 1L, DeprecateOptionFlag, MagickTrue },
    { "-bench", 1L, GenesisOptionFlag, MagickFalse },
    { "+bias", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-bias", 1L, ImageInfoOptionFlag, MagickFalse },
    { "-black-point-compensation", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+black-point-compensation", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+black-threshold", 0L, DeprecateOptionFlag, MagickTrue },
    { "-black-threshold", 1L, SimpleOperatorFlag, MagickFalse },
    { "+blend", 0L, NonMagickOptionFlag, MagickFalse },
    { "-blend", 1L, NonMagickOptionFlag, MagickFalse },
    { "+blue-primary", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-blue-primary", 1L, ImageInfoOptionFlag, MagickFalse },
    { "-blue-shift", 1L, SimpleOperatorFlag, MagickFalse },
    { "+blue-shift", 1L, SimpleOperatorFlag, MagickFalse },
    { "+border", 1L, DeprecateOptionFlag, MagickTrue },
    { "-border", 1L, SimpleOperatorFlag, MagickFalse },
    { "+bordercolor", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-bordercolor", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+borderwidth", 0L, NonMagickOptionFlag, MagickFalse },
    { "-borderwidth", 1L, NonMagickOptionFlag, MagickFalse },
    { "+box", 0L, ReplacedOptionFlag | ImageInfoOptionFlag | DrawInfoOptionFlag, MagickTrue },
    { "-box", 1L, ReplacedOptionFlag | ImageInfoOptionFlag | DrawInfoOptionFlag, MagickTrue },
    { "+brightness-contrast", 0L, DeprecateOptionFlag, MagickTrue },
    { "-brightness-contrast", 1L, SimpleOperatorFlag, MagickFalse },
    { "+cache", 0L, GlobalOptionFlag, MagickFalse },
    { "-cache", 1L, GlobalOptionFlag, MagickFalse },
    { "+canny", 1L, DeprecateOptionFlag, MagickTrue },
    { "-canny", 1L, SimpleOperatorFlag, MagickTrue },
    { "+caption", 0L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-caption", 1L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+cdl", 1L, DeprecateOptionFlag, MagickTrue },
    { "-cdl", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+channel", 0L, SimpleOperatorFlag, MagickFalse },
    { "-channel", 1L, SimpleOperatorFlag, MagickFalse },
    { "-channel-fx", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+charcoal", 1L, DeprecateOptionFlag, MagickTrue },
    { "-charcoal", 1L, SimpleOperatorFlag, MagickFalse },
    { "+chop", 1L, DeprecateOptionFlag, MagickTrue },
    { "-chop", 1L, SimpleOperatorFlag, MagickFalse },
    { "+clahe", 1L, DeprecateOptionFlag, MagickTrue },
    { "-clahe", 1L, SimpleOperatorFlag, MagickFalse },
    { "+clamp", 0L, DeprecateOptionFlag, MagickTrue },
    { "-clamp", 0L, SimpleOperatorFlag, MagickFalse },
    { "-clip", 0L, SimpleOperatorFlag, MagickFalse },
    { "+clip", 0L, SimpleOperatorFlag, MagickFalse },
    { "+clip-mask", 0L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-clip-mask", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-clip-path", 1L, SimpleOperatorFlag, MagickFalse },
    { "+clip-path", 1L, SimpleOperatorFlag, MagickFalse },
    { "+clone", 0L, NoImageOperatorFlag, MagickFalse },
    { "-clone", 1L, NoImageOperatorFlag, MagickFalse },
    { "+clut", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-clut", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+coalesce", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-coalesce", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+colorize", 1L, DeprecateOptionFlag, MagickTrue },
    { "-colorize", 1L, SimpleOperatorFlag, MagickFalse },
    { "+colormap", 0L, NonMagickOptionFlag, MagickFalse },
    { "-colormap", 1L, NonMagickOptionFlag, MagickFalse },
    { "+color-matrix", 1L, DeprecateOptionFlag, MagickTrue },
    { "-color-matrix", 1L, SimpleOperatorFlag, MagickFalse },
    { "+colors", 1L, DeprecateOptionFlag, MagickTrue },
    { "-colors", 1L, SimpleOperatorFlag, MagickFalse },
    { "+colorspace", 0L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "-colorspace", 1L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "+color-threshold", 0L, DeprecateOptionFlag, MagickTrue },
    { "-color-threshold", 1L, SimpleOperatorFlag, MagickFalse },
    { "-combine", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+combine", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+comment", 0L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-comment", 1L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+compare", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-compare", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+complex", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-complex", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+compose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-compose", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+composite", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-composite", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+compress", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-compress", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+concurrent", 0L, DeprecateOptionFlag, MagickTrue },
    { "-concurrent", 0L, GenesisOptionFlag, MagickFalse },
    { "+connected-components", 1L, DeprecateOptionFlag, MagickTrue },
    { "-connected-components", 1L, SimpleOperatorFlag, MagickFalse },
    { "-contrast", 0L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "+contrast", 0L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "+contrast-stretch", 1L, DeprecateOptionFlag, MagickTrue },
    { "-contrast-stretch", 1L, SimpleOperatorFlag, MagickFalse },
    { "+convolve", 1L, DeprecateOptionFlag, MagickTrue },
    { "-convolve", 1L, SimpleOperatorFlag, MagickFalse },
    { "+copy", 2L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-copy", 2L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+crop", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-crop", 1L, SimpleOperatorFlag | FireOptionFlag, MagickFalse },
    { "+cycle", 1L, DeprecateOptionFlag, MagickTrue },
    { "-cycle", 1L, SimpleOperatorFlag, MagickFalse },
    { "+debug", 0L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "-debug", 1L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "+decipher", 1L, DeprecateOptionFlag, MagickTrue },
    { "-decipher", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+deconstruct", 0L, DeprecateOptionFlag, MagickTrue },
    { "-deconstruct", 0L, ReplacedOptionFlag | ListOperatorFlag | FireOptionFlag, MagickTrue },
    { "-define", 1L, ImageInfoOptionFlag | NeverInterpretArgsFlag | FireOptionFlag, MagickFalse },
    { "+define", 1L, ImageInfoOptionFlag | NeverInterpretArgsFlag | FireOptionFlag, MagickFalse },
    { "+delay", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-delay", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+delete", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-delete", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+density", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-density", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+depth", 0L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "-depth", 1L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "+descend", 0L, NonMagickOptionFlag, MagickFalse },
    { "-descend", 1L, NonMagickOptionFlag, MagickFalse },
    { "+deskew", 0L, SimpleOperatorFlag, MagickFalse },
    { "-deskew", 1L, SimpleOperatorFlag, MagickFalse },
    { "+despeckle", 0L, DeprecateOptionFlag, MagickTrue },
    { "-despeckle", 0L, SimpleOperatorFlag, MagickFalse },
    { "+direction", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-direction", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+displace", 0L, NonMagickOptionFlag, MagickFalse },
    { "-displace", 1L, NonMagickOptionFlag, MagickFalse },
    { "-display", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+display", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+dispose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-dispose", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+dissimilarity-threshold", 0L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "-dissimilarity-threshold", 1L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+dissolve", 0L, NonMagickOptionFlag, MagickFalse },
    { "-dissolve", 1L, NonMagickOptionFlag, MagickFalse },
    { "-distort", 2L, SimpleOperatorFlag | AlwaysInterpretArgsFlag, MagickFalse },
    { "+distort", 2L, SimpleOperatorFlag | AlwaysInterpretArgsFlag, MagickFalse },
    { "+dither", 0L, ImageInfoOptionFlag | QuantizeInfoOptionFlag, MagickFalse },
    { "-dither", 1L, ImageInfoOptionFlag | QuantizeInfoOptionFlag, MagickFalse },
    { "+draw", 0L, DeprecateOptionFlag, MagickTrue },
    { "-draw", 1L, SimpleOperatorFlag, MagickFalse },
    { "+duplicate", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-duplicate", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-duration", 1L, GenesisOptionFlag, MagickFalse },
    { "+duration", 1L, GenesisOptionFlag, MagickFalse },
    { "+edge", 1L, DeprecateOptionFlag, MagickTrue },
    { "-edge", 1L, SimpleOperatorFlag, MagickFalse },
    { "+emboss", 1L, DeprecateOptionFlag, MagickTrue },
    { "-emboss", 1L, SimpleOperatorFlag, MagickFalse },
    { "+encipher", 1L, DeprecateOptionFlag, MagickTrue },
    { "-encipher", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+encoding", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-encoding", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+endian", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-endian", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+enhance", 0L, DeprecateOptionFlag, MagickTrue },
    { "-enhance", 0L, SimpleOperatorFlag, MagickFalse },
    { "+equalize", 0L, DeprecateOptionFlag, MagickTrue },
    { "-equalize", 0L, SimpleOperatorFlag, MagickFalse },
    { "+evaluate", 2L, DeprecateOptionFlag, MagickTrue },
    { "-evaluate", 2L, SimpleOperatorFlag, MagickFalse },
    { "+evaluate-sequence", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-evaluate-sequence", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-exit", 0L, SpecialOptionFlag, MagickFalse },
    { "+extent", 1L, DeprecateOptionFlag, MagickTrue },
    { "-extent", 1L, SimpleOperatorFlag, MagickFalse },
    { "+extract", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-extract", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+family", 0L, DeprecateOptionFlag, MagickTrue },
    { "-family", 1L, DrawInfoOptionFlag, MagickFalse },
    { "+features", 0L, SimpleOperatorFlag | FireOptionFlag, MagickFalse },
    { "-features", 1L, SimpleOperatorFlag | FireOptionFlag, MagickFalse },
    { "-fft", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+fft", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+fill", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-fill", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+filter", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-filter", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+flatten", 0L, DeprecateOptionFlag, MagickTrue },
    { "-flatten", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+flip", 0L, DeprecateOptionFlag, MagickTrue },
    { "-flip", 0L, SimpleOperatorFlag, MagickFalse },
    { "-floodfill", 2L, SimpleOperatorFlag, MagickFalse },
    { "+floodfill", 2L, SimpleOperatorFlag, MagickFalse },
    { "+flop", 0L, DeprecateOptionFlag, MagickTrue },
    { "-flop", 0L, SimpleOperatorFlag, MagickFalse },
    { "+font", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-font", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+foreground", 0L, NonMagickOptionFlag, MagickFalse },
    { "-foreground", 1L, NonMagickOptionFlag, MagickFalse },
    { "+frame", 1L, DeprecateOptionFlag, MagickTrue },
    { "-frame", 1L, SimpleOperatorFlag, MagickFalse },
    { "+function", 2L, DeprecateOptionFlag, MagickTrue },
    { "-function", 2L,SimpleOperatorFlag | AlwaysInterpretArgsFlag, MagickFalse },
    { "+fuzz", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-fuzz", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+fx", 1L, DeprecateOptionFlag | FireOptionFlag | NeverInterpretArgsFlag, MagickTrue },
    { "-fx", 1L, ListOperatorFlag | FireOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-gamma", 1L, SimpleOperatorFlag, MagickFalse },
    { "+gamma", 1L, SimpleOperatorFlag, MagickFalse },
    { "+gaussian", 1L, DeprecateOptionFlag, MagickTrue },
    { "-gaussian", 1L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "+gaussian-blur", 1L, DeprecateOptionFlag, MagickTrue },
    { "-gaussian-blur", 1L, SimpleOperatorFlag, MagickFalse },
    { "+geometry", 0L, SimpleOperatorFlag, MagickFalse },
    { "-geometry", 1L, SimpleOperatorFlag, MagickFalse },
    { "+gravity", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-gravity", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+grayscale", 1L, SimpleOperatorFlag, MagickTrue },
    { "-grayscale", 1L, SimpleOperatorFlag, MagickFalse },
    { "+green-primary", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-green-primary", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+hald-clut", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-hald-clut", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+highlight-color", 0L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "-highlight-color", 1L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+hough-lines", 1L, DeprecateOptionFlag, MagickTrue },
    { "-hough-lines", 1L, SimpleOperatorFlag, MagickTrue },
    { "+iconGeometry", 0L, NonMagickOptionFlag, MagickFalse },
    { "-iconGeometry", 1L, NonMagickOptionFlag, MagickFalse },
    { "-iconic", 0L, NonMagickOptionFlag, MagickFalse },
    { "+iconic", 0L, NonMagickOptionFlag, MagickFalse },
    { "+identify", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-identify", 0L, SimpleOperatorFlag | FireOptionFlag, MagickFalse },
    { "-ift", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+ift", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-immutable", 0L, NonMagickOptionFlag, MagickFalse },
    { "+immutable", 0L, NonMagickOptionFlag, MagickFalse },
    { "+implode", 0L, DeprecateOptionFlag, MagickTrue },
    { "-implode", 1L, SimpleOperatorFlag, MagickFalse },
    { "+insert", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-insert", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+intensity", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-intensity", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+intent", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-intent", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+interlace", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-interlace", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+interline-spacing", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-interline-spacing", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+interpolate", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-interpolate", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+interpolative-resize", 1L, DeprecateOptionFlag, MagickTrue },
    { "-interpolative-resize", 1L, SimpleOperatorFlag, MagickFalse },
    { "+interword-spacing", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-interword-spacing", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+kerning", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-kerning", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-kmeans", 1L, SimpleOperatorFlag, MagickFalse },
    { "+kuwahara", 0L, DeprecateOptionFlag, MagickTrue },
    { "-kuwahara", 1L, SimpleOperatorFlag, MagickFalse },
    { "+label", 0L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-label", 1L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+lat", 1L, DeprecateOptionFlag, MagickTrue },
    { "-lat", 1L, SimpleOperatorFlag, MagickFalse },
    { "+layers", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-layers", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-level", 1L, SimpleOperatorFlag, MagickFalse },
    { "+level", 1L, SimpleOperatorFlag, MagickFalse },
    { "-level-colors", 1L, SimpleOperatorFlag, MagickFalse },
    { "+level-colors", 1L, SimpleOperatorFlag, MagickFalse },
    { "+limit", 0L, DeprecateOptionFlag, MagickTrue },
    { "-limit", 2L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "+linear-stretch", 1L, DeprecateOptionFlag, MagickTrue },
    { "-linear-stretch", 1L, SimpleOperatorFlag, MagickFalse },
    { "+liquid-rescale", 1L, DeprecateOptionFlag, MagickTrue },
    { "-liquid-rescale", 1L, SimpleOperatorFlag, MagickFalse },
    { "+list", 0L, DeprecateOptionFlag, MagickTrue },
    { "-list", 1L, NoImageOperatorFlag, MagickFalse },
    { "+local-contrast", 0L, DeprecateOptionFlag, MagickTrue },
    { "-local-contrast", 1L, SimpleOperatorFlag, MagickFalse },
    { "+log", 0L, DeprecateOptionFlag, MagickFalse },
    { "-log", 1L, GlobalOptionFlag, MagickFalse },
    { "+loop", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-loop", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+lowlight-color", 0L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "-lowlight-color", 1L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+magnify", 0L, DeprecateOptionFlag, MagickTrue },
    { "-magnify", 0L, SimpleOperatorFlag, MagickFalse },
    { "+map", 0L, ReplacedOptionFlag | ListOperatorFlag | FireOptionFlag, MagickTrue },
    { "-map", 1L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "+mask", 0L, DeprecateOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-mask", 1L, DeprecateOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-matte", 0L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "+matte", 0L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "-maximum", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "+maximum", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "+mean-shift", 1L, DeprecateOptionFlag, MagickTrue },
    { "-mean-shift", 1L, SimpleOperatorFlag, MagickTrue },
    { "+median", 1L, DeprecateOptionFlag, MagickTrue },
    { "-median", 1L, ReplacedOptionFlag | SimpleOperatorFlag | FireOptionFlag, MagickTrue },
    { "+metric", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-metric", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-minimum", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "+minimum", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "+mode", 1L, NonMagickOptionFlag, MagickFalse },
    { "-mode", 1L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "+modulate", 1L, DeprecateOptionFlag, MagickTrue },
    { "-modulate", 1L, SimpleOperatorFlag, MagickFalse },
    { "-moments", 0L, SimpleOperatorFlag | FireOptionFlag, MagickFalse },
    { "+moments", 0L, SimpleOperatorFlag | FireOptionFlag, MagickFalse },
    { "-monitor", 0L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "+monitor", 0L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "+monochrome", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-monochrome", 0L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "+morph", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-morph", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+morphology", 2L, DeprecateOptionFlag, MagickTrue },
    { "-morphology", 2L, SimpleOperatorFlag, MagickFalse },
    { "+mosaic", 0L, DeprecateOptionFlag, MagickTrue },
    { "-mosaic", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+motion-blur", 1L, DeprecateOptionFlag, MagickTrue },
    { "-motion-blur", 1L, SimpleOperatorFlag, MagickFalse },
    { "+name", 0L, NonMagickOptionFlag, MagickFalse },
    { "-name", 1L, NonMagickOptionFlag, MagickFalse },
    { "+negate", 0L, SimpleOperatorFlag, MagickFalse },
    { "-negate", 0L, SimpleOperatorFlag, MagickFalse },
    { "-noise", 1L, ReplacedOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "+noise", 1L, SimpleOperatorFlag, MagickFalse },
    { "-noop", 0L, NoImageOperatorFlag, MagickFalse },
    { "+normalize", 0L, DeprecateOptionFlag, MagickTrue },
    { "-normalize", 0L, SimpleOperatorFlag, MagickFalse },
    { "-opaque", 1L, SimpleOperatorFlag, MagickFalse },
    { "+opaque", 1L, SimpleOperatorFlag, MagickFalse },
    { "+ordered-dither", 0L, DeprecateOptionFlag, MagickTrue },
    { "-ordered-dither", 1L, SimpleOperatorFlag, MagickFalse },
    { "+orient", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-orient", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+page", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-page", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+paint", 0L, DeprecateOptionFlag, MagickTrue },
    { "-paint", 1L, SimpleOperatorFlag, MagickFalse },
    { "+path", 0L, NonMagickOptionFlag, MagickFalse },
    { "-path", 1L, NonMagickOptionFlag, MagickFalse },
    { "+pause", 0L, NonMagickOptionFlag, MagickFalse },
    { "-pause", 1L, NonMagickOptionFlag, MagickFalse },
    { "-ping", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+ping", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+pointsize", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-pointsize", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+polaroid", 0L, SimpleOperatorFlag, MagickFalse },
    { "-polaroid", 1L, SimpleOperatorFlag, MagickFalse },
    { "+poly", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-poly", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+posterize", 1L, DeprecateOptionFlag, MagickTrue },
    { "-posterize", 1L, SimpleOperatorFlag, MagickFalse },
    { "+precision", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-precision", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+preview", 0L, DeprecateOptionFlag, MagickTrue },
    { "-preview", 1L, SimpleOperatorFlag, MagickFalse },
    { "+print", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-print", 1L, NoImageOperatorFlag | AlwaysInterpretArgsFlag | FireOptionFlag, MagickFalse },
    { "+process", 1L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-process", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+profile", 1L, SimpleOperatorFlag, MagickFalse },
    { "-profile", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+quality", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-quality", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+quantize", 0L, QuantizeInfoOptionFlag, MagickFalse },
    { "-quantize", 1L, QuantizeInfoOptionFlag, MagickFalse },
    { "-raise", 1L, SimpleOperatorFlag, MagickFalse },
    { "+raise", 1L, SimpleOperatorFlag, MagickFalse },
    { "+random-threshold", 1L, DeprecateOptionFlag, MagickTrue },
    { "-random-threshold", 1L, SimpleOperatorFlag, MagickFalse },
    { "+range-threshold", 1L, DeprecateOptionFlag, MagickTrue },
    { "-range-threshold", 1L, SimpleOperatorFlag, MagickFalse },
    { "-read", 1L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+read-mask", 0L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-read-mask", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+recolor", 1L, DeprecateOptionFlag, MagickTrue },
    { "-recolor", 1L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "+red-primary", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-red-primary", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+region", 0L, SimpleOperatorFlag, MagickFalse },
    { "-region", 1L, SimpleOperatorFlag, MagickFalse },
    { "+remap", 0L, ListOperatorFlag | NeverInterpretArgsFlag | FireOptionFlag, MagickFalse },
    { "-remap", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+remote", 0L, NonMagickOptionFlag, MagickFalse },
    { "-remote", 1L, NonMagickOptionFlag, MagickFalse },
    { "-render", 0L, DrawInfoOptionFlag, MagickFalse },
    { "+render", 0L, DrawInfoOptionFlag, MagickFalse },
    { "+resample", 1L, DeprecateOptionFlag, MagickTrue },
    { "-resample", 1L, SimpleOperatorFlag, MagickFalse },
    { "-respect-parenthesis", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+respect-parenthesis", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+reverse", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-reverse", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+roll", 1L, DeprecateOptionFlag, MagickTrue },
    { "-roll", 1L, SimpleOperatorFlag, MagickFalse },
    { "+rotate", 1L, DeprecateOptionFlag, MagickTrue },
    { "-rotate", 1L, SimpleOperatorFlag, MagickFalse },
    { "-rotational-blur", 1L, SimpleOperatorFlag, MagickFalse },
    { "+sample", 1L, DeprecateOptionFlag, MagickTrue },
    { "-sample", 1L, SimpleOperatorFlag, MagickFalse },
    { "+sampling-factor", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-sampling-factor", 1L, ImageInfoOptionFlag, MagickFalse },
    { "-sans0", 0L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue },
    { "+sans0", 0L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue }, /* equivelent to 'noop' */
    { "+sans1", 1L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue },
    { "-sans1", 1L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue }, /* equivelent to 'sans' */
    { "-sans", 1L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue },
    { "+sans", 1L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue },
    { "-sans2", 2L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue },
    { "+sans2", 2L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickTrue },
    { "+scale", 1L, DeprecateOptionFlag, MagickTrue },
    { "-scale", 1L, SimpleOperatorFlag, MagickFalse },
    { "+scene", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-scene", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+scenes", 0L, NonMagickOptionFlag, MagickFalse },
    { "-scenes", 1L, NonMagickOptionFlag, MagickFalse },
    { "+screen", 0L, NonMagickOptionFlag, MagickFalse },
    { "-screen", 1L, NonMagickOptionFlag, MagickFalse },
    { "-script", 1L, SpecialOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+seed", 0L, GlobalOptionFlag, MagickFalse },
    { "-seed", 1L, GlobalOptionFlag, MagickFalse },
    { "+segment", 1L, DeprecateOptionFlag, MagickTrue },
    { "-segment", 1L, SimpleOperatorFlag, MagickFalse },
    { "+selective-blur", 1L, DeprecateOptionFlag, MagickTrue },
    { "-selective-blur", 1L, SimpleOperatorFlag, MagickFalse },
    { "+separate", 0L, DeprecateOptionFlag | FireOptionFlag, MagickTrue },
    { "-separate", 0L, SimpleOperatorFlag | FireOptionFlag, MagickFalse },
    { "+sepia-tone", 1L, DeprecateOptionFlag, MagickTrue },
    { "-sepia-tone", 1L, SimpleOperatorFlag, MagickFalse },
    { "+set", 1L, NoImageOperatorFlag, MagickFalse },
    { "-set", 2L, NoImageOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+shade", 0L, DeprecateOptionFlag, MagickTrue },
    { "-shade", 1L, SimpleOperatorFlag, MagickFalse },
    { "+shadow", 1L, DeprecateOptionFlag, MagickTrue },
    { "-shadow", 1L, SimpleOperatorFlag, MagickFalse },
    { "+shared-memory", 0L, NonMagickOptionFlag, MagickFalse },
    { "-shared-memory", 1L, NonMagickOptionFlag, MagickFalse },
    { "+sharpen", 1L, DeprecateOptionFlag, MagickTrue },
    { "-sharpen", 1L, SimpleOperatorFlag, MagickFalse },
    { "+shave", 1L, DeprecateOptionFlag, MagickTrue },
    { "-shave", 1L, SimpleOperatorFlag, MagickFalse },
    { "+shear", 1L, DeprecateOptionFlag, MagickTrue },
    { "-shear", 1L, SimpleOperatorFlag, MagickFalse },
    { "-sigmoidal-contrast", 1L, SimpleOperatorFlag, MagickFalse },
    { "+sigmoidal-contrast", 1L, SimpleOperatorFlag, MagickFalse },
    { "+silent", 0L, NonMagickOptionFlag, MagickFalse },
    { "-silent", 1L, NonMagickOptionFlag, MagickFalse },
    { "+similarity-threshold", 0L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "-similarity-threshold", 1L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+sketch", 1L, DeprecateOptionFlag, MagickTrue },
    { "-sketch", 1L, SimpleOperatorFlag, MagickFalse },
    { "-smush", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+smush", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+snaps", 0L, NonMagickOptionFlag, MagickFalse },
    { "-snaps", 1L, NonMagickOptionFlag, MagickFalse },
    { "+solarize", 1L, DeprecateOptionFlag, MagickTrue },
    { "-solarize", 1L, SimpleOperatorFlag, MagickFalse },
    { "+sparse-color", 1L, DeprecateOptionFlag, MagickTrue },
    { "-sparse-color", 2L, SimpleOperatorFlag | AlwaysInterpretArgsFlag, MagickFalse },
    { "+splice", 1L, DeprecateOptionFlag, MagickTrue },
    { "-splice", 1L, SimpleOperatorFlag, MagickFalse },
    { "+spread", 1L, DeprecateOptionFlag, MagickTrue },
    { "-spread", 1L, SimpleOperatorFlag, MagickFalse },
    { "+statistic", 2L, DeprecateOptionFlag, MagickTrue },
    { "-statistic", 2L, SimpleOperatorFlag, MagickFalse },
    { "+stegano", 0L, NonMagickOptionFlag, MagickFalse },
    { "-stegano", 1L, NonMagickOptionFlag, MagickFalse },
    { "+stereo", 0L, DeprecateOptionFlag, MagickTrue },
    { "-stereo", 1L, NonMagickOptionFlag, MagickFalse },
    { "+stretch", 1L, DeprecateOptionFlag, MagickTrue },
    { "-stretch", 1L, SimpleOperatorFlag, MagickFalse },
    { "+strip", 0L, DeprecateOptionFlag, MagickTrue },
    { "-strip", 0L, SimpleOperatorFlag, MagickFalse },
    { "+stroke", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-stroke", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-strokewidth", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+strokewidth", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+style", 0L, DrawInfoOptionFlag, MagickFalse },
    { "-style", 1L, DrawInfoOptionFlag, MagickFalse },
    { "-subimage", 0L, ListOperatorFlag, MagickFalse },
    { "-subimage-search", 0L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+subimage-search", 0L, NonMagickOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+swap", 0L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "-swap", 1L, ListOperatorFlag | FireOptionFlag, MagickFalse },
    { "+swirl", 1L, DeprecateOptionFlag, MagickTrue },
    { "-swirl", 1L, SimpleOperatorFlag, MagickFalse },
    { "-synchronize", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+synchronize", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-taint", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+taint", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+text-font", 0L, NonMagickOptionFlag, MagickFalse },
    { "-text-font", 1L, NonMagickOptionFlag, MagickFalse },
    { "+texture", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-texture", 1L, ImageInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+threshold", 0L, SimpleOperatorFlag, MagickFalse },
    { "-threshold", 1L, SimpleOperatorFlag, MagickFalse },
    { "+thumbnail", 1L, DeprecateOptionFlag, MagickTrue },
    { "-thumbnail", 1L, SimpleOperatorFlag, MagickFalse },
    { "+tile", 0L, DrawInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-tile", 1L, DrawInfoOptionFlag | NeverInterpretArgsFlag, MagickFalse },
    { "+tile-offset", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-tile-offset", 1L, ImageInfoOptionFlag, MagickFalse },
    { "-tint", 1L, SimpleOperatorFlag, MagickFalse },
    { "+tint", 1L, SimpleOperatorFlag, MagickFalse },
    { "+title", 0L, NonMagickOptionFlag, MagickFalse },
    { "-title", 1L, NonMagickOptionFlag, MagickFalse },
    { "+transform", 0L, DeprecateOptionFlag, MagickTrue },
    { "-transform", 0L, ReplacedOptionFlag | SimpleOperatorFlag, MagickTrue },
    { "-transparent", 1L, SimpleOperatorFlag, MagickFalse },
    { "+transparent", 1L, SimpleOperatorFlag, MagickFalse },
    { "+transparent-color", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-transparent-color", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+transpose", 0L, DeprecateOptionFlag, MagickTrue },
    { "-transpose", 0L, SimpleOperatorFlag, MagickFalse },
    { "+transverse", 0L, DeprecateOptionFlag, MagickTrue },
    { "-transverse", 0L, SimpleOperatorFlag, MagickFalse },
    { "+treedepth", 1L, DeprecateOptionFlag, MagickTrue },
    { "-treedepth", 1L, QuantizeInfoOptionFlag, MagickFalse },
    { "+trim", 0L, DeprecateOptionFlag, MagickTrue },
    { "-trim", 0L, SimpleOperatorFlag, MagickFalse },
    { "+type", 0L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "-type", 1L, ImageInfoOptionFlag | SimpleOperatorFlag, MagickFalse },
    { "+undercolor", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-undercolor", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-unique", 0L, SimpleOperatorFlag, MagickFalse },
    { "+unique", 0L, SimpleOperatorFlag, MagickFalse },
    { "+unique-colors", 0L, DeprecateOptionFlag, MagickTrue },
    { "-unique-colors", 0L, SimpleOperatorFlag, MagickFalse },
    { "+units", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-units", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+unsharp", 1L, DeprecateOptionFlag, MagickTrue },
    { "-unsharp", 1L, SimpleOperatorFlag, MagickFalse },
    { "+update", 0L, NonMagickOptionFlag, MagickFalse },
    { "-update", 1L, NonMagickOptionFlag, MagickFalse },
    { "+use-pixmap", 0L, NonMagickOptionFlag, MagickFalse },
    { "-use-pixmap", 1L, NonMagickOptionFlag, MagickFalse },
    { "-verbose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+verbose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+version", 0L, DeprecateOptionFlag, MagickTrue },
    { "-version", 0L, NoImageOperatorFlag, MagickFalse },
    { "+view", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-view", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+vignette", 1L, DeprecateOptionFlag, MagickTrue },
    { "-vignette", 1L, SimpleOperatorFlag, MagickFalse },
    { "+visual", 0L, NonMagickOptionFlag, MagickFalse },
    { "-visual", 1L, NonMagickOptionFlag, MagickFalse },
    { "+watermark", 0L, NonMagickOptionFlag, MagickFalse },
    { "-watermark", 1L, NonMagickOptionFlag, MagickFalse },
    { "+wave", 1L, DeprecateOptionFlag, MagickTrue },
    { "-wave", 1L, SimpleOperatorFlag, MagickFalse },
    { "+wavelet-denoise", 0L, DeprecateOptionFlag, MagickTrue },
    { "-wavelet-denoise", 1L, SimpleOperatorFlag, MagickFalse },
    { "+weight", 1L, DeprecateOptionFlag, MagickTrue },
    { "-weight", 1L, DrawInfoOptionFlag, MagickFalse },
    { "+white-point", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-white-point", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+white-threshold", 1L, DeprecateOptionFlag, MagickTrue },
    { "-white-threshold", 1L, SimpleOperatorFlag, MagickFalse },
    { "+window", 0L, NonMagickOptionFlag, MagickFalse },
    { "-window", 1L, NonMagickOptionFlag, MagickFalse },
    { "+window-group", 0L, NonMagickOptionFlag, MagickFalse },
    { "-window-group", 1L, NonMagickOptionFlag, MagickFalse },
    { "-write", 1L, NoImageOperatorFlag | NeverInterpretArgsFlag | FireOptionFlag, MagickFalse },
    { "+write", 1L, NoImageOperatorFlag | NeverInterpretArgsFlag | FireOptionFlag, MagickFalse },
    { "+write-mask", 0L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { "-write-mask", 1L, SimpleOperatorFlag | NeverInterpretArgsFlag, MagickFalse },
    { (char *) NULL, 0L, UndefinedOptionFlag, MagickFalse }
  },
  ComplianceOptions[] =
  {
    { "Undefined", UndefinedCompliance, UndefinedOptionFlag, MagickTrue },
    { "CSS", CSSCompliance, UndefinedOptionFlag, MagickFalse },
    { "MVG", MVGCompliance, UndefinedOptionFlag, MagickFalse },
    { "No", NoCompliance, UndefinedOptionFlag, MagickFalse },
    { "SVG", SVGCompliance, UndefinedOptionFlag, MagickFalse },
    { "X11", X11Compliance, UndefinedOptionFlag, MagickFalse },
    { "XPM", XPMCompliance, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedClass, UndefinedOptionFlag, MagickFalse }
  },
  ComposeOptions[] =
  {
    { "Undefined", UndefinedCompositeOp, UndefinedOptionFlag, MagickTrue },
    { "Add", ModulusAddCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "Atop", AtopCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Blend", BlendCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Blur", BlurCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Bumpmap", BumpmapCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ChangeMask", ChangeMaskCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Clear", ClearCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ColorBurn", ColorBurnCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ColorDodge", ColorDodgeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Colorize", ColorizeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyAlpha", CopyAlphaCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyBlack", CopyBlackCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyBlue", CopyBlueCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Copy", CopyCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyCyan", CopyCyanCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyGreen", CopyGreenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyMagenta", CopyMagentaCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyOpacity", CopyAlphaCompositeOp, UndefinedOptionFlag, MagickTrue },
    { "CopyRed", CopyRedCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyYellow", CopyYellowCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Darken", DarkenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DarkenIntensity", DarkenIntensityCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Difference", DifferenceCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Displace", DisplaceCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Dissolve", DissolveCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Distort", DistortCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Divide", DivideDstCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "DivideDst", DivideDstCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DivideSrc", DivideSrcCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstAtop", DstAtopCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Dst", DstCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstIn", DstInCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstOut", DstOutCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstOver", DstOverCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Exclusion", ExclusionCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Freeze", FreezeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "HardLight", HardLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "HardMix", HardMixCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Hue", HueCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "In", InCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Intensity", IntensityCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Interpolate", InterpolateCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LightenIntensity", LightenIntensityCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Lighten", LightenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LinearBurn", LinearBurnCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LinearDodge", LinearDodgeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LinearLight", LinearLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Luminize", LuminizeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Mathematics", MathematicsCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "MinusDst", MinusDstCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Minus", MinusDstCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "MinusSrc", MinusSrcCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Modulate", ModulateCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ModulusAdd", ModulusAddCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ModulusSubtract", ModulusSubtractCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Multiply", MultiplyCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Negate", NegateCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "None", NoCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Out", OutCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Overlay", OverlayCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Over", OverCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "PegtopLight", PegtopLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "PinLight", PinLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Plus", PlusCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Reflect", ReflectCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Replace", ReplaceCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Saturate", SaturateCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Screen", ScreenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SoftBurn", SoftBurnCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SoftDodge", SoftDodgeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SoftLight", SoftLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcAtop", SrcAtopCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcIn", SrcInCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcOut", SrcOutCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcOver", SrcOverCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Src", SrcCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Stamp", StampCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Stereo", StereoCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Subtract", ModulusSubtractCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "Threshold", ThresholdCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "VividLight", VividLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Xor", XorCompositeOp, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedCompositeOp, UndefinedOptionFlag, MagickFalse }
  },
  CompressOptions[] =
  {
    { "Undefined", UndefinedCompression, UndefinedOptionFlag, MagickTrue },
    { "B44A", B44ACompression, UndefinedOptionFlag, MagickFalse },
    { "B44", B44Compression, UndefinedOptionFlag, MagickFalse },
    { "BZip", BZipCompression, UndefinedOptionFlag, MagickFalse },
    { "DWAA", DWAACompression, UndefinedOptionFlag, MagickFalse },
    { "DWAB", DWABCompression, UndefinedOptionFlag, MagickFalse },
    { "DXT1", DXT1Compression, UndefinedOptionFlag, MagickFalse },
    { "DXT3", DXT3Compression, UndefinedOptionFlag, MagickFalse },
    { "DXT5", DXT5Compression, UndefinedOptionFlag, MagickFalse },
    { "Fax", FaxCompression, UndefinedOptionFlag, MagickFalse },
    { "Group4", Group4Compression, UndefinedOptionFlag, MagickFalse },
    { "JBIG1", JBIG1Compression, UndefinedOptionFlag, MagickFalse },
    { "JBIG2", JBIG2Compression, UndefinedOptionFlag, MagickFalse },
    { "JPEG2000", JPEG2000Compression, UndefinedOptionFlag, MagickFalse },
    { "JPEG", JPEGCompression, UndefinedOptionFlag, MagickFalse },
    { "LosslessJPEG", LosslessJPEGCompression, UndefinedOptionFlag, MagickFalse },
    { "Lossless", LosslessJPEGCompression, UndefinedOptionFlag, MagickFalse },
    { "LZMA", LZMACompression, UndefinedOptionFlag, MagickFalse },
    { "LZW", LZWCompression, UndefinedOptionFlag, MagickFalse },
    { "None", NoCompression, UndefinedOptionFlag, MagickFalse },
    { "Piz", PizCompression, UndefinedOptionFlag, MagickFalse },
    { "Pxr24", Pxr24Compression, UndefinedOptionFlag, MagickFalse },
    { "RLE", RLECompression, UndefinedOptionFlag, MagickFalse },
    { "RunlengthEncoded", RLECompression, UndefinedOptionFlag, MagickFalse },
    { "WebP", WebPCompression, UndefinedOptionFlag, MagickFalse },
    { "ZipS", ZipSCompression, UndefinedOptionFlag, MagickFalse },
    { "Zip", ZipCompression, UndefinedOptionFlag, MagickFalse },
    { "Zstd", ZstdCompression, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedCompression, UndefinedOptionFlag, MagickFalse }
  },
  DataTypeOptions[] =
  {
    { "Undefined", UndefinedData, UndefinedOptionFlag, MagickTrue },
    { "Byte", ByteData, UndefinedOptionFlag, MagickFalse },
    { "Long", LongData, UndefinedOptionFlag, MagickFalse },
    { "Short", ShortData, UndefinedOptionFlag, MagickFalse },
    { "String", StringData, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedData, UndefinedOptionFlag, MagickFalse }
  },
  DecorateOptions[] =
  {
    { "Undefined", UndefinedDecoration, UndefinedOptionFlag, MagickTrue },
    { "LineThrough", LineThroughDecoration, UndefinedOptionFlag, MagickFalse },
    { "None", NoDecoration, UndefinedOptionFlag, MagickFalse },
    { "Overline", OverlineDecoration, UndefinedOptionFlag, MagickFalse },
    { "Underline", UnderlineDecoration, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedDecoration, UndefinedOptionFlag, MagickFalse }
  },
  DirectionOptions[] =
  {
    { "Undefined", UndefinedDirection, UndefinedOptionFlag, MagickTrue },
    { "right-to-left", RightToLeftDirection, UndefinedOptionFlag, MagickFalse },
    { "left-to-right", LeftToRightDirection, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedDirection, UndefinedOptionFlag, MagickFalse }
  },
  DisposeOptions[] =
  {
    { "Undefined", UndefinedDispose, UndefinedOptionFlag, MagickFalse },
    { "Background", BackgroundDispose, UndefinedOptionFlag, MagickFalse },
    { "None", NoneDispose, UndefinedOptionFlag, MagickFalse },
    { "Previous", PreviousDispose, UndefinedOptionFlag, MagickFalse },
    { "0", UndefinedDispose, UndefinedOptionFlag, MagickFalse },
    { "1", NoneDispose, UndefinedOptionFlag, MagickFalse },
    { "2", BackgroundDispose, UndefinedOptionFlag, MagickFalse },
    { "3", PreviousDispose, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedDispose, UndefinedOptionFlag, MagickFalse }
  },
  DistortOptions[] =
  {
    { "Affine", AffineDistortion, UndefinedOptionFlag, MagickFalse },
    { "RigidAffine", RigidAffineDistortion, UndefinedOptionFlag, MagickFalse },
    { "AffineProjection", AffineProjectionDistortion, UndefinedOptionFlag, MagickFalse },
    { "ScaleRotateTranslate", ScaleRotateTranslateDistortion, UndefinedOptionFlag, MagickFalse },
    { "SRT", ScaleRotateTranslateDistortion, UndefinedOptionFlag, MagickFalse },
    { "Perspective", PerspectiveDistortion, UndefinedOptionFlag, MagickFalse },
    { "PerspectiveProjection", PerspectiveProjectionDistortion, UndefinedOptionFlag, MagickFalse },
    { "Bilinear", BilinearForwardDistortion, UndefinedOptionFlag, MagickTrue },
    { "BilinearForward", BilinearForwardDistortion, UndefinedOptionFlag, MagickFalse },
    { "BilinearReverse", BilinearReverseDistortion, UndefinedOptionFlag, MagickFalse },
    { "Polynomial", PolynomialDistortion, UndefinedOptionFlag, MagickFalse },
    { "Arc", ArcDistortion, UndefinedOptionFlag, MagickFalse },
    { "Polar", PolarDistortion, UndefinedOptionFlag, MagickFalse },
    { "DePolar", DePolarDistortion, UndefinedOptionFlag, MagickFalse },
    { "Barrel", BarrelDistortion, UndefinedOptionFlag, MagickFalse },
    { "Cylinder2Plane", Cylinder2PlaneDistortion, UndefinedOptionFlag, MagickTrue },
    { "Plane2Cylinder", Plane2CylinderDistortion, UndefinedOptionFlag, MagickTrue },
    { "BarrelInverse", BarrelInverseDistortion, UndefinedOptionFlag, MagickFalse },
    { "Shepards", ShepardsDistortion, UndefinedOptionFlag, MagickFalse },
    { "Resize", ResizeDistortion, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedDistortion, UndefinedOptionFlag, MagickFalse }
  },
  DitherOptions[] =
  {
    { "Undefined", UndefinedDitherMethod, UndefinedOptionFlag, MagickTrue },
    { "None", NoDitherMethod, UndefinedOptionFlag, MagickFalse },
    { "FloydSteinberg", FloydSteinbergDitherMethod, UndefinedOptionFlag, MagickFalse },
    { "Riemersma", RiemersmaDitherMethod, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedEndian, UndefinedOptionFlag, MagickFalse }
  },
  EndianOptions[] =
  {
    { "Undefined", UndefinedEndian, UndefinedOptionFlag, MagickTrue },
    { "LSB", LSBEndian, UndefinedOptionFlag, MagickFalse },
    { "MSB", MSBEndian, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedEndian, UndefinedOptionFlag, MagickFalse }
  },
  EvaluateOptions[] =
  {
    { "Undefined", UndefinedEvaluateOperator, UndefinedOptionFlag, MagickTrue },
    { "Abs", AbsEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Add", AddEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "AddModulus", AddModulusEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "And", AndEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Cos", CosineEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Cosine", CosineEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Divide", DivideEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Exp", ExponentialEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Exponential", ExponentialEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "GaussianNoise", GaussianNoiseEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "ImpulseNoise", ImpulseNoiseEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "LaplacianNoise", LaplacianNoiseEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "LeftShift", LeftShiftEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Log", LogEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Max", MaxEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Mean", MeanEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Median", MedianEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Min", MinEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "MultiplicativeNoise", MultiplicativeNoiseEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Multiply", MultiplyEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Or", OrEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "PoissonNoise", PoissonNoiseEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Pow", PowEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "RightShift", RightShiftEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "RMS", RootMeanSquareEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "RootMeanSquare", RootMeanSquareEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Set", SetEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Sin", SineEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Sine", SineEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Subtract", SubtractEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Sum", SumEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Threshold", ThresholdEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "ThresholdBlack", ThresholdBlackEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "ThresholdWhite", ThresholdWhiteEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "UniformNoise", UniformNoiseEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { "Xor", XorEvaluateOperator, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedEvaluateOperator, UndefinedOptionFlag, MagickFalse }
  },
  FillRuleOptions[] =
  {
    { "Undefined", UndefinedRule, UndefinedOptionFlag, MagickTrue },
    { "Evenodd", EvenOddRule, UndefinedOptionFlag, MagickFalse },
    { "NonZero", NonZeroRule, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedRule, UndefinedOptionFlag, MagickFalse }
  },
  FilterOptions[] =
  {
    { "Undefined", UndefinedFilter, UndefinedOptionFlag, MagickTrue },
    { "Bartlett", BartlettFilter, UndefinedOptionFlag, MagickFalse },
    { "Blackman", BlackmanFilter, UndefinedOptionFlag, MagickFalse },
    { "Bohman", BohmanFilter, UndefinedOptionFlag, MagickFalse },
    { "Box", BoxFilter, UndefinedOptionFlag, MagickFalse },
    { "Catrom", CatromFilter, UndefinedOptionFlag, MagickFalse },
    { "Cosine", CosineFilter, UndefinedOptionFlag, MagickFalse },
    { "Cubic", CubicFilter, UndefinedOptionFlag, MagickFalse },
    { "Gaussian", GaussianFilter, UndefinedOptionFlag, MagickFalse },
    { "Hamming", HammingFilter, UndefinedOptionFlag, MagickFalse },
    { "Hann", HannFilter, UndefinedOptionFlag, MagickFalse },
    { "Hanning", HannFilter, UndefinedOptionFlag, MagickTrue }, /*misspell*/
    { "Hermite", HermiteFilter, UndefinedOptionFlag, MagickFalse },
    { "Jinc", JincFilter, UndefinedOptionFlag, MagickFalse },
    { "Kaiser", KaiserFilter, UndefinedOptionFlag, MagickFalse },
    { "Lagrange", LagrangeFilter, UndefinedOptionFlag, MagickFalse },
    { "Lanczos", LanczosFilter, UndefinedOptionFlag, MagickFalse },
    { "Lanczos2", Lanczos2Filter, UndefinedOptionFlag, MagickFalse },
    { "Lanczos2Sharp", Lanczos2SharpFilter, UndefinedOptionFlag, MagickFalse },
    { "LanczosRadius", LanczosRadiusFilter, UndefinedOptionFlag, MagickFalse },
    { "LanczosSharp", LanczosSharpFilter, UndefinedOptionFlag, MagickFalse },
    { "Mitchell", MitchellFilter, UndefinedOptionFlag, MagickFalse },
    { "Parzen", ParzenFilter, UndefinedOptionFlag, MagickFalse },
    { "Point", PointFilter, UndefinedOptionFlag, MagickFalse },
    { "Quadratic", QuadraticFilter, UndefinedOptionFlag, MagickFalse },
    { "Robidoux", RobidouxFilter, UndefinedOptionFlag, MagickFalse },
    { "RobidouxSharp", RobidouxSharpFilter, UndefinedOptionFlag, MagickFalse },
    { "Sinc", SincFilter, UndefinedOptionFlag, MagickFalse },
    { "SincFast", SincFastFilter, UndefinedOptionFlag, MagickFalse },
    { "Spline", SplineFilter, UndefinedOptionFlag, MagickFalse },
    { "CubicSpline", CubicSplineFilter, UndefinedOptionFlag, MagickFalse },
    { "Triangle", TriangleFilter, UndefinedOptionFlag, MagickFalse },
    { "Welch", WelchFilter, UndefinedOptionFlag, MagickFalse },
    { "Welsh", WelchFilter, UndefinedOptionFlag, MagickTrue }, /*misspell*/
    { (char *) NULL, UndefinedFilter, UndefinedOptionFlag, MagickFalse }
  },
  FunctionOptions[] =
  {
    { "Undefined", UndefinedFunction, UndefinedOptionFlag, MagickTrue },
    { "Polynomial", PolynomialFunction, UndefinedOptionFlag, MagickFalse },
    { "Sinusoid", SinusoidFunction, UndefinedOptionFlag, MagickFalse },
    { "ArcSin", ArcsinFunction, UndefinedOptionFlag, MagickFalse },
    { "ArcTan", ArctanFunction, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedFunction, UndefinedOptionFlag, MagickFalse }
  },
  GradientOptions[] =
  {
    { "Undefined", UndefinedGradient, UndefinedOptionFlag, MagickTrue },
    { "Linear", LinearGradient, UndefinedOptionFlag, MagickFalse },
    { "Radial", RadialGradient, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedRule, UndefinedOptionFlag, MagickFalse }
  },
  GravityOptions[] =
  {
    { "Undefined", UndefinedGravity, UndefinedOptionFlag, MagickTrue },
    { "None", UndefinedGravity, UndefinedOptionFlag, MagickFalse },
    { "Center", CenterGravity, UndefinedOptionFlag, MagickFalse },
    { "East", EastGravity, UndefinedOptionFlag, MagickFalse },
    { "Forget", ForgetGravity, UndefinedOptionFlag, MagickFalse },
    { "NorthEast", NorthEastGravity, UndefinedOptionFlag, MagickFalse },
    { "North", NorthGravity, UndefinedOptionFlag, MagickFalse },
    { "NorthWest", NorthWestGravity, UndefinedOptionFlag, MagickFalse },
    { "SouthEast", SouthEastGravity, UndefinedOptionFlag, MagickFalse },
    { "South", SouthGravity, UndefinedOptionFlag, MagickFalse },
    { "SouthWest", SouthWestGravity, UndefinedOptionFlag, MagickFalse },
    { "West", WestGravity, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedGravity, UndefinedOptionFlag, MagickFalse }
  },
  IntentOptions[] =
  {
    { "Undefined", UndefinedIntent, UndefinedOptionFlag, MagickTrue },
    { "Absolute", AbsoluteIntent, UndefinedOptionFlag, MagickFalse },
    { "Perceptual", PerceptualIntent, UndefinedOptionFlag, MagickFalse },
    { "Relative", RelativeIntent, UndefinedOptionFlag, MagickFalse },
    { "Saturation", SaturationIntent, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedIntent, UndefinedOptionFlag, MagickFalse }
  },
  InterlaceOptions[] =
  {
    { "Undefined", UndefinedInterlace, UndefinedOptionFlag, MagickTrue },
    { "Line", LineInterlace, UndefinedOptionFlag, MagickFalse },
    { "None", NoInterlace, UndefinedOptionFlag, MagickFalse },
    { "Plane", PlaneInterlace, UndefinedOptionFlag, MagickFalse },
    { "Partition", PartitionInterlace, UndefinedOptionFlag, MagickFalse },
    { "GIF", GIFInterlace, UndefinedOptionFlag, MagickFalse },
    { "JPEG", JPEGInterlace, UndefinedOptionFlag, MagickFalse },
    { "PNG", PNGInterlace, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedInterlace, UndefinedOptionFlag, MagickFalse }
  },
  InterpolateOptions[] =
  {
    { "Undefined", UndefinedInterpolatePixel, UndefinedOptionFlag, MagickTrue },
    { "Average", AverageInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Average4", AverageInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Average9", Average9InterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Average16", Average16InterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Background", BackgroundInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Bilinear", BilinearInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Blend", BlendInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Catrom", CatromInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Integer", IntegerInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Mesh", MeshInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Nearest", NearestInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "NearestNeighbor", NearestInterpolatePixel, UndefinedOptionFlag, MagickTrue },
    { "Spline", SplineInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedInterpolatePixel, UndefinedOptionFlag, MagickFalse }
  },
  KernelOptions[] =
  {
    { "Undefined", UndefinedKernel, UndefinedOptionFlag, MagickTrue },
    { "Unity", UnityKernel, UndefinedOptionFlag, MagickFalse },
    { "Gaussian", GaussianKernel, UndefinedOptionFlag, MagickFalse },
    { "DoG", DoGKernel, UndefinedOptionFlag, MagickFalse },
    { "LoG", LoGKernel, UndefinedOptionFlag, MagickFalse },
    { "Blur", BlurKernel, UndefinedOptionFlag, MagickFalse },
    { "Comet", CometKernel, UndefinedOptionFlag, MagickFalse },
    { "Binomial", BinomialKernel, UndefinedOptionFlag, MagickFalse },
    { "Laplacian", LaplacianKernel, UndefinedOptionFlag, MagickFalse },
    { "Sobel", SobelKernel, UndefinedOptionFlag, MagickFalse },
    { "FreiChen", FreiChenKernel, UndefinedOptionFlag, MagickFalse },
    { "Roberts", RobertsKernel, UndefinedOptionFlag, MagickFalse },
    { "Prewitt", PrewittKernel, UndefinedOptionFlag, MagickFalse },
    { "Compass", CompassKernel, UndefinedOptionFlag, MagickFalse },
    { "Kirsch", KirschKernel, UndefinedOptionFlag, MagickFalse },
    { "Diamond", DiamondKernel, UndefinedOptionFlag, MagickFalse },
    { "Square", SquareKernel, UndefinedOptionFlag, MagickFalse },
    { "Rectangle", RectangleKernel, UndefinedOptionFlag, MagickFalse },
    { "Disk", DiskKernel, UndefinedOptionFlag, MagickFalse },
    { "Octagon", OctagonKernel, UndefinedOptionFlag, MagickFalse },
    { "Plus", PlusKernel, UndefinedOptionFlag, MagickFalse },
    { "Cross", CrossKernel, UndefinedOptionFlag, MagickFalse },
    { "Ring", RingKernel, UndefinedOptionFlag, MagickFalse },
    { "Peaks", PeaksKernel, UndefinedOptionFlag, MagickFalse },
    { "Edges", EdgesKernel, UndefinedOptionFlag, MagickFalse },
    { "Corners", CornersKernel, UndefinedOptionFlag, MagickFalse },
    { "Diagonals", DiagonalsKernel, UndefinedOptionFlag, MagickFalse },
    { "LineEnds", LineEndsKernel, UndefinedOptionFlag, MagickFalse },
    { "LineJunctions", LineJunctionsKernel, UndefinedOptionFlag, MagickFalse },
    { "Ridges", RidgesKernel, UndefinedOptionFlag, MagickFalse },
    { "ConvexHull", ConvexHullKernel, UndefinedOptionFlag, MagickFalse },
    { "ThinSe", ThinSEKernel, UndefinedOptionFlag, MagickFalse },
    { "Skeleton", SkeletonKernel, UndefinedOptionFlag, MagickFalse },
    { "Chebyshev", ChebyshevKernel, UndefinedOptionFlag, MagickFalse },
    { "Manhattan", ManhattanKernel, UndefinedOptionFlag, MagickFalse },
    { "Octagonal", OctagonalKernel, UndefinedOptionFlag, MagickFalse },
    { "Euclidean", EuclideanKernel, UndefinedOptionFlag, MagickFalse },
    { "User Defined", UserDefinedKernel, UndefinedOptionFlag, MagickTrue },
    { (char *) NULL, UndefinedKernel, UndefinedOptionFlag, MagickFalse }
  },
  LayerOptions[] =
  {
    { "Undefined", UndefinedLayer, UndefinedOptionFlag, MagickTrue },
    { "Coalesce", CoalesceLayer, UndefinedOptionFlag, MagickFalse },
    { "CompareAny", CompareAnyLayer, UndefinedOptionFlag, MagickFalse },
    { "CompareClear", CompareClearLayer, UndefinedOptionFlag, MagickFalse },
    { "CompareOverlay", CompareOverlayLayer, UndefinedOptionFlag, MagickFalse },
    { "Dispose", DisposeLayer, UndefinedOptionFlag, MagickFalse },
    { "Optimize", OptimizeLayer, UndefinedOptionFlag, MagickFalse },
    { "OptimizeFrame", OptimizeImageLayer, UndefinedOptionFlag, MagickFalse },
    { "OptimizePlus", OptimizePlusLayer, UndefinedOptionFlag, MagickFalse },
    { "OptimizeTransparency", OptimizeTransLayer, UndefinedOptionFlag, MagickFalse },
    { "RemoveDups", RemoveDupsLayer, UndefinedOptionFlag, MagickFalse },
    { "RemoveZero", RemoveZeroLayer, UndefinedOptionFlag, MagickFalse },
    { "Composite", CompositeLayer, UndefinedOptionFlag, MagickFalse },
    { "Merge", MergeLayer, UndefinedOptionFlag, MagickFalse },
    { "Flatten", FlattenLayer, UndefinedOptionFlag, MagickFalse },
    { "Mosaic", MosaicLayer, UndefinedOptionFlag, MagickFalse },
    { "TrimBounds", TrimBoundsLayer, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedLayer, UndefinedOptionFlag, MagickFalse }
  },
  LineCapOptions[] =
  {
    { "Undefined", UndefinedCap, UndefinedOptionFlag, MagickTrue },
    { "Butt", ButtCap, UndefinedOptionFlag, MagickFalse },
    { "Round", RoundCap, UndefinedOptionFlag, MagickFalse },
    { "Square", SquareCap, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedCap, UndefinedOptionFlag, MagickFalse }
  },
  LineJoinOptions[] =
  {
    { "Undefined", UndefinedJoin, UndefinedOptionFlag, MagickTrue },
    { "Bevel", BevelJoin, UndefinedOptionFlag, MagickFalse },
    { "Miter", MiterJoin, UndefinedOptionFlag, MagickFalse },
    { "Round", RoundJoin, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedJoin, UndefinedOptionFlag, MagickFalse }
  },
  ListOptions[] =
  {
    { "Align", MagickAlignOptions, UndefinedOptionFlag, MagickFalse },
    { "Alpha", MagickAlphaChannelOptions, UndefinedOptionFlag, MagickFalse },
    { "AutoThreshold", MagickAutoThresholdOptions, UndefinedOptionFlag, MagickFalse },
    { "Boolean", MagickBooleanOptions, UndefinedOptionFlag, MagickFalse },
    { "Cache", MagickCacheOptions, UndefinedOptionFlag, MagickFalse },
    { "Channel", MagickChannelOptions, UndefinedOptionFlag, MagickFalse },
    { "Class", MagickClassOptions, UndefinedOptionFlag, MagickFalse },
    { "CLI", MagickCLIOptions, UndefinedOptionFlag, MagickFalse },
    { "ClipPath", MagickClipPathOptions, UndefinedOptionFlag, MagickFalse },
    { "Coder", MagickCoderOptions, UndefinedOptionFlag, MagickFalse },
    { "Color", MagickColorOptions, UndefinedOptionFlag, MagickFalse },
    { "Colorspace", MagickColorspaceOptions, UndefinedOptionFlag, MagickFalse },
    { "Command", MagickCommandOptions, UndefinedOptionFlag, MagickFalse },
    { "Compliance", MagickComplianceOptions, UndefinedOptionFlag, MagickFalse },
    { "Complex", MagickComplexOptions, UndefinedOptionFlag, MagickFalse },
    { "Compose", MagickComposeOptions, UndefinedOptionFlag, MagickFalse },
    { "Compress", MagickCompressOptions, UndefinedOptionFlag, MagickFalse },
    { "Configure", MagickConfigureOptions, UndefinedOptionFlag, MagickFalse },
    { "DataType", MagickDataTypeOptions, UndefinedOptionFlag, MagickFalse },
    { "Debug", MagickDebugOptions, UndefinedOptionFlag, MagickFalse },
    { "Decoration", MagickDecorateOptions, UndefinedOptionFlag, MagickFalse },
    { "Delegate", MagickDelegateOptions, UndefinedOptionFlag, MagickFalse },
    { "Direction", MagickDirectionOptions, UndefinedOptionFlag, MagickFalse },
    { "Dispose", MagickDisposeOptions, UndefinedOptionFlag, MagickFalse },
    { "Distort", MagickDistortOptions, UndefinedOptionFlag, MagickFalse },
    { "Dither", MagickDitherOptions, UndefinedOptionFlag, MagickFalse },
    { "Endian", MagickEndianOptions, UndefinedOptionFlag, MagickFalse },
    { "Evaluate", MagickEvaluateOptions, UndefinedOptionFlag, MagickFalse },
    { "FillRule", MagickFillRuleOptions, UndefinedOptionFlag, MagickFalse },
    { "Filter", MagickFilterOptions, UndefinedOptionFlag, MagickFalse },
    { "Font", MagickFontOptions, UndefinedOptionFlag, MagickFalse },
    { "Format", MagickFormatOptions, UndefinedOptionFlag, MagickFalse },
    { "Function", MagickFunctionOptions, UndefinedOptionFlag, MagickFalse },
    { "Gradient", MagickGradientOptions, UndefinedOptionFlag, MagickFalse },
    { "Gravity", MagickGravityOptions, UndefinedOptionFlag, MagickFalse },
    { "Intensity", MagickPixelIntensityOptions, UndefinedOptionFlag, MagickFalse },
    { "Intent", MagickIntentOptions, UndefinedOptionFlag, MagickFalse },
    { "Interlace", MagickInterlaceOptions, UndefinedOptionFlag, MagickFalse },
    { "Interpolate", MagickInterpolateOptions, UndefinedOptionFlag, MagickFalse },
    { "Kernel", MagickKernelOptions, UndefinedOptionFlag, MagickFalse },
    { "Layers", MagickLayerOptions, UndefinedOptionFlag, MagickFalse },
    { "LineCap", MagickLineCapOptions, UndefinedOptionFlag, MagickFalse },
    { "LineJoin", MagickLineJoinOptions, UndefinedOptionFlag, MagickFalse },
    { "List", MagickListOptions, UndefinedOptionFlag, MagickFalse },
    { "Locale", MagickLocaleOptions, UndefinedOptionFlag, MagickFalse },
    { "LogEvent", MagickLogEventOptions, UndefinedOptionFlag, MagickFalse },
    { "Log", MagickLogOptions, UndefinedOptionFlag, MagickFalse },
    { "Magic", MagickMagicOptions, UndefinedOptionFlag, MagickFalse },
    { "Method", MagickMethodOptions, UndefinedOptionFlag, MagickFalse },
    { "Metric", MagickMetricOptions, UndefinedOptionFlag, MagickFalse },
    { "Mime", MagickMimeOptions, UndefinedOptionFlag, MagickFalse },
    { "Mode", MagickModeOptions, UndefinedOptionFlag, MagickFalse },
    { "Morphology", MagickMorphologyOptions, UndefinedOptionFlag, MagickFalse },
    { "Module", MagickModuleOptions, UndefinedOptionFlag, MagickFalse },
    { "Noise", MagickNoiseOptions, UndefinedOptionFlag, MagickFalse },
    { "Orientation", MagickOrientationOptions, UndefinedOptionFlag, MagickFalse },
    { "PixelChannel", MagickPixelChannelOptions, UndefinedOptionFlag, MagickFalse },
    { "PixelIntensity", MagickPixelIntensityOptions, UndefinedOptionFlag, MagickFalse },
    { "PixelMask", MagickPixelMaskOptions, UndefinedOptionFlag, MagickFalse },
    { "PixelTrait", MagickPixelTraitOptions, UndefinedOptionFlag, MagickFalse },
    { "Policy", MagickPolicyOptions, UndefinedOptionFlag, MagickFalse },
    { "PolicyDomain", MagickPolicyDomainOptions, UndefinedOptionFlag, MagickFalse },
    { "PolicyRights", MagickPolicyRightsOptions, UndefinedOptionFlag, MagickFalse },
    { "Preview", MagickPreviewOptions, UndefinedOptionFlag, MagickFalse },
    { "Primitive", MagickPrimitiveOptions, UndefinedOptionFlag, MagickFalse },
    { "QuantumFormat", MagickQuantumFormatOptions, UndefinedOptionFlag, MagickFalse },
    { "Resource", MagickResourceOptions, UndefinedOptionFlag, MagickFalse },
    { "SparseColor", MagickSparseColorOptions, UndefinedOptionFlag, MagickFalse },
    { "Statistic", MagickStatisticOptions, UndefinedOptionFlag, MagickFalse },
    { "Storage", MagickStorageOptions, UndefinedOptionFlag, MagickFalse },
    { "Stretch", MagickStretchOptions, UndefinedOptionFlag, MagickFalse },
    { "Style", MagickStyleOptions, UndefinedOptionFlag, MagickFalse },
    { "Threshold", MagickThresholdOptions, UndefinedOptionFlag, MagickFalse },
    { "Tool", MagickToolOptions, UndefinedOptionFlag, MagickFalse },
    { "Type", MagickTypeOptions, UndefinedOptionFlag, MagickFalse },
    { "Units", MagickResolutionOptions, UndefinedOptionFlag, MagickFalse },
    { "Undefined", MagickUndefinedOptions, UndefinedOptionFlag, MagickTrue },
    { "Validate", MagickValidateOptions, UndefinedOptionFlag, MagickFalse },
    { "VirtualPixel", MagickVirtualPixelOptions, UndefinedOptionFlag, MagickFalse },
    { "Weight", MagickWeightOptions, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, MagickUndefinedOptions, UndefinedOptionFlag, MagickFalse }
  },
  LogEventOptions[] =
  {
    { "Undefined", UndefinedEvents, UndefinedOptionFlag, MagickTrue },
    { "All", (AllEvents &~ TraceEvent), UndefinedOptionFlag, MagickFalse },
    { "Accelerate", AccelerateEvent, UndefinedOptionFlag, MagickFalse },
    { "Annotate", AnnotateEvent, UndefinedOptionFlag, MagickFalse },
    { "Blob", BlobEvent, UndefinedOptionFlag, MagickFalse },
    { "Cache", CacheEvent, UndefinedOptionFlag, MagickFalse },
    { "Coder", CoderEvent, UndefinedOptionFlag, MagickFalse },
    { "Command", CommandEvent, UndefinedOptionFlag, MagickFalse },
    { "Configure", ConfigureEvent, UndefinedOptionFlag, MagickFalse },
    { "Deprecate", DeprecateEvent, UndefinedOptionFlag, MagickFalse },
    { "Draw", DrawEvent, UndefinedOptionFlag, MagickFalse },
    { "Exception", ExceptionEvent, UndefinedOptionFlag, MagickFalse },
    { "Locale", LocaleEvent, UndefinedOptionFlag, MagickFalse },
    { "Module", ModuleEvent, UndefinedOptionFlag, MagickFalse },
    { "None", NoEvents, UndefinedOptionFlag, MagickFalse },
    { "Pixel", PixelEvent, UndefinedOptionFlag, MagickFalse },
    { "Policy", PolicyEvent, UndefinedOptionFlag, MagickFalse },
    { "Resource", ResourceEvent, UndefinedOptionFlag, MagickFalse },
    { "Trace", TraceEvent, UndefinedOptionFlag, MagickFalse },
    { "Transform", TransformEvent, UndefinedOptionFlag, MagickFalse },
    { "User", UserEvent, UndefinedOptionFlag, MagickFalse },
    { "Wand", WandEvent, UndefinedOptionFlag, MagickFalse },
    { "X11", X11Event, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedEvents, UndefinedOptionFlag, MagickFalse }
  },
  MetricOptions[] =
  {
    { "Undefined", UndefinedErrorMetric, UndefinedOptionFlag, MagickTrue },
    { "AE", AbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "DSSIM", StructuralDissimilarityErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "Fuzz", FuzzErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "MAE", MeanAbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "MEPP", MeanErrorPerPixelErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "MSE", MeanSquaredErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "NCC", NormalizedCrossCorrelationErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PAE", PeakAbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PHASH", PerceptualHashErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PSNR", PeakSignalToNoiseRatioErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "RMSE", RootMeanSquaredErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "SSIM", StructuralSimilarityErrorMetric, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedErrorMetric, UndefinedOptionFlag, MagickFalse }
  },
  MethodOptions[] =
  {
    { "Undefined", UndefinedMethod, UndefinedOptionFlag, MagickTrue },
    { "FillToBorder", FillToBorderMethod, UndefinedOptionFlag, MagickFalse },
    { "Floodfill", FloodfillMethod, UndefinedOptionFlag, MagickFalse },
    { "Point", PointMethod, UndefinedOptionFlag, MagickFalse },
    { "Replace", ReplaceMethod, UndefinedOptionFlag, MagickFalse },
    { "Reset", ResetMethod, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedMethod, UndefinedOptionFlag, MagickFalse }
  },
  ModeOptions[] =
  {
    { "Undefined", UndefinedMode, UndefinedOptionFlag, MagickTrue },
    { "Concatenate", ConcatenateMode, UndefinedOptionFlag, MagickFalse },
    { "Frame", FrameMode, UndefinedOptionFlag, MagickFalse },
    { "Unframe", UnframeMode, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedMode, UndefinedOptionFlag, MagickFalse }
  },
  MorphologyOptions[] =
  {
    { "Undefined", UndefinedMorphology, UndefinedOptionFlag, MagickTrue },
    { "Correlate", CorrelateMorphology, UndefinedOptionFlag, MagickFalse },
    { "Convolve", ConvolveMorphology, UndefinedOptionFlag, MagickFalse },
    { "Dilate", DilateMorphology, UndefinedOptionFlag, MagickFalse },
    { "Erode", ErodeMorphology, UndefinedOptionFlag, MagickFalse },
    { "Close", CloseMorphology, UndefinedOptionFlag, MagickFalse },
    { "Open", OpenMorphology, UndefinedOptionFlag, MagickFalse },
    { "DilateIntensity", DilateIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "ErodeIntensity", ErodeIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "CloseIntensity", CloseIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "OpenIntensity", OpenIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "DilateI", DilateIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "ErodeI", ErodeIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "CloseI", CloseIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "OpenI", OpenIntensityMorphology, UndefinedOptionFlag, MagickFalse },
    { "Smooth", SmoothMorphology, UndefinedOptionFlag, MagickFalse },
    { "EdgeOut", EdgeOutMorphology, UndefinedOptionFlag, MagickFalse },
    { "EdgeIn", EdgeInMorphology, UndefinedOptionFlag, MagickFalse },
    { "Edge", EdgeMorphology, UndefinedOptionFlag, MagickFalse },
    { "TopHat", TopHatMorphology, UndefinedOptionFlag, MagickFalse },
    { "BottomHat", BottomHatMorphology, UndefinedOptionFlag, MagickFalse },
    { "Hmt", HitAndMissMorphology, UndefinedOptionFlag, MagickFalse },
    { "HitNMiss", HitAndMissMorphology, UndefinedOptionFlag, MagickFalse },
    { "HitAndMiss", HitAndMissMorphology, UndefinedOptionFlag, MagickFalse },
    { "Thinning", ThinningMorphology, UndefinedOptionFlag, MagickFalse },
    { "Thicken", ThickenMorphology, UndefinedOptionFlag, MagickFalse },
    { "Distance", DistanceMorphology, UndefinedOptionFlag, MagickFalse },
    { "IterativeDistance", IterativeDistanceMorphology, UndefinedOptionFlag, MagickFalse },
    { "Voronoi", VoronoiMorphology, UndefinedOptionFlag, MagickTrue },
    { (char *) NULL, UndefinedMorphology, UndefinedOptionFlag, MagickFalse }
  },
  NoiseOptions[] =
  {
    { "Undefined", UndefinedNoise, UndefinedOptionFlag, MagickTrue },
    { "Gaussian", GaussianNoise, UndefinedOptionFlag, MagickFalse },
    { "Impulse", ImpulseNoise, UndefinedOptionFlag, MagickFalse },
    { "Laplacian", LaplacianNoise, UndefinedOptionFlag, MagickFalse },
    { "Multiplicative", MultiplicativeGaussianNoise, UndefinedOptionFlag, MagickFalse },
    { "Poisson", PoissonNoise, UndefinedOptionFlag, MagickFalse },
    { "Random", RandomNoise, UndefinedOptionFlag, MagickFalse },
    { "Uniform", UniformNoise, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedNoise, UndefinedOptionFlag, MagickFalse }
  },
  OrientationOptions[] =
  {
    { "Undefined", UndefinedOrientation, UndefinedOptionFlag, MagickTrue },
    { "TopLeft", TopLeftOrientation, UndefinedOptionFlag, MagickFalse },
    { "TopRight", TopRightOrientation, UndefinedOptionFlag, MagickFalse },
    { "BottomRight", BottomRightOrientation, UndefinedOptionFlag, MagickFalse },
    { "BottomLeft", BottomLeftOrientation, UndefinedOptionFlag, MagickFalse },
    { "LeftTop", LeftTopOrientation, UndefinedOptionFlag, MagickFalse },
    { "RightTop", RightTopOrientation, UndefinedOptionFlag, MagickFalse },
    { "RightBottom", RightBottomOrientation, UndefinedOptionFlag, MagickFalse },
    { "LeftBottom", LeftBottomOrientation, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedOrientation, UndefinedOptionFlag, MagickFalse }
  },
  PixelChannelOptions[] =
  {
    { "Undefined", UndefinedPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "A", AlphaPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Alpha", AlphaPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "B", BluePixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Bk", BlackPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Black", BlackPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Blue", BluePixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Cb", CbPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Composite", CompositePixelChannel, UndefinedOptionFlag, MagickFalse },
    { "CompositeMask", CompositeMaskPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "C", CyanPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Cr", CrPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Cyan", CyanPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Gray", GrayPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "G", GreenPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Green", GreenPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Index", IndexPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Intensity", IntensityPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "K", BlackPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "M", MagentaPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Magenta", MagentaPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Meta", MetaPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "O", AlphaPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "R", RedPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "ReadMask", ReadMaskPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Red", RedPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Sync", SyncPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "WriteMask", WriteMaskPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Y", YellowPixelChannel, UndefinedOptionFlag, MagickFalse },
    { "Yellow", YellowPixelChannel, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPixelChannel, UndefinedOptionFlag, MagickFalse }
  },
  PixelIntensityOptions[] =
  {
    { "Undefined", UndefinedPixelIntensityMethod, UndefinedOptionFlag, MagickTrue },
    { "Average", AveragePixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Brightness", BrightnessPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Lightness", LightnessPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Mean", AveragePixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "MS", MSPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec601Luma", Rec601LumaPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec601Luminance", Rec601LuminancePixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec709Luma", Rec709LumaPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec709Luminance", Rec709LuminancePixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "RMS", RMSPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPixelIntensityMethod, UndefinedOptionFlag, MagickFalse }
  },
  PixelMaskOptions[] =
  {
    { "Undefined", UndefinedPixelMask, UndefinedOptionFlag, MagickTrue },
    { "R", ReadPixelMask, UndefinedOptionFlag, MagickFalse },
    { "Read", ReadPixelMask, UndefinedOptionFlag, MagickFalse },
    { "W", WritePixelMask, UndefinedOptionFlag, MagickFalse },
    { "Write", WritePixelMask, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPixelMask, UndefinedOptionFlag, MagickFalse }
  },
  PixelTraitOptions[] =
  {
    { "Undefined", UndefinedPixelTrait, UndefinedOptionFlag, MagickTrue },
    { "Blend", BlendPixelTrait, UndefinedOptionFlag, MagickFalse },
    { "Copy", CopyPixelTrait, UndefinedOptionFlag, MagickFalse },
    { "Update", UpdatePixelTrait, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPixelTrait, UndefinedOptionFlag, MagickFalse }
  },
  PolicyDomainOptions[] =
  {
    { "Undefined", UndefinedPolicyDomain, UndefinedOptionFlag, MagickTrue },
    { "Cache", CachePolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Coder", CoderPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Delegate", DelegatePolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Filter", FilterPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Module", ModulePolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Path", PathPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Resource", ResourcePolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "System", SystemPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPolicyDomain, UndefinedOptionFlag, MagickFalse }
  },
  PolicyRightsOptions[] =
  {
    { "Undefined", UndefinedPolicyRights, UndefinedOptionFlag, MagickTrue },
    { "All", AllPolicyRights, UndefinedOptionFlag, MagickFalse },
    { "Execute", ExecutePolicyRights, UndefinedOptionFlag, MagickFalse },
    { "None", NoPolicyRights, UndefinedOptionFlag, MagickFalse },
    { "Read", ReadPolicyRights, UndefinedOptionFlag, MagickFalse },
    { "Write", WritePolicyRights, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPolicyRights, UndefinedOptionFlag, MagickFalse }
  },
  PreviewOptions[] =
  {
    { "Undefined", UndefinedPreview, UndefinedOptionFlag, MagickTrue },
    { "AddNoise", AddNoisePreview, UndefinedOptionFlag, MagickFalse },
    { "Blur", BlurPreview, UndefinedOptionFlag, MagickFalse },
    { "Brightness", BrightnessPreview, UndefinedOptionFlag, MagickFalse },
    { "Charcoal", CharcoalDrawingPreview, UndefinedOptionFlag, MagickFalse },
    { "Despeckle", DespecklePreview, UndefinedOptionFlag, MagickFalse },
    { "Dull", DullPreview, UndefinedOptionFlag, MagickFalse },
    { "EdgeDetect", EdgeDetectPreview, UndefinedOptionFlag, MagickFalse },
    { "Gamma", GammaPreview, UndefinedOptionFlag, MagickFalse },
    { "Grayscale", GrayscalePreview, UndefinedOptionFlag, MagickFalse },
    { "Hue", HuePreview, UndefinedOptionFlag, MagickFalse },
    { "Implode", ImplodePreview, UndefinedOptionFlag, MagickFalse },
    { "JPEG", JPEGPreview, UndefinedOptionFlag, MagickFalse },
    { "OilPaint", OilPaintPreview, UndefinedOptionFlag, MagickFalse },
    { "Quantize", QuantizePreview, UndefinedOptionFlag, MagickFalse },
    { "Raise", RaisePreview, UndefinedOptionFlag, MagickFalse },
    { "ReduceNoise", ReduceNoisePreview, UndefinedOptionFlag, MagickFalse },
    { "Roll", RollPreview, UndefinedOptionFlag, MagickFalse },
    { "Rotate", RotatePreview, UndefinedOptionFlag, MagickFalse },
    { "Saturation", SaturationPreview, UndefinedOptionFlag, MagickFalse },
    { "Segment", SegmentPreview, UndefinedOptionFlag, MagickFalse },
    { "Shade", ShadePreview, UndefinedOptionFlag, MagickFalse },
    { "Sharpen", SharpenPreview, UndefinedOptionFlag, MagickFalse },
    { "Shear", ShearPreview, UndefinedOptionFlag, MagickFalse },
    { "Solarize", SolarizePreview, UndefinedOptionFlag, MagickFalse },
    { "Spiff", SpiffPreview, UndefinedOptionFlag, MagickFalse },
    { "Spread", SpreadPreview, UndefinedOptionFlag, MagickFalse },
    { "Swirl", SwirlPreview, UndefinedOptionFlag, MagickFalse },
    { "Threshold", ThresholdPreview, UndefinedOptionFlag, MagickFalse },
    { "Wave", WavePreview, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPreview, UndefinedOptionFlag, MagickFalse }
  },
  PrimitiveOptions[] =
  {
    { "Undefined", UndefinedPrimitive, UndefinedOptionFlag, MagickTrue },
    { "Alpha", AlphaPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Arc", ArcPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Bezier", BezierPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Circle", CirclePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Color", ColorPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Ellipse", EllipsePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Image", ImagePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Line", LinePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Matte", AlphaPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Path", PathPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Point", PointPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Polygon", PolygonPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Polyline", PolylinePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Rectangle", RectanglePrimitive, UndefinedOptionFlag, MagickFalse },
    { "RoundRectangle", RoundRectanglePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Text", TextPrimitive, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPrimitive, UndefinedOptionFlag, MagickFalse }
  },
  QuantumFormatOptions[] =
  {
    { "Undefined", UndefinedQuantumFormat, UndefinedOptionFlag, MagickTrue },
    { "FloatingPoint", FloatingPointQuantumFormat, UndefinedOptionFlag, MagickFalse },
    { "Signed", SignedQuantumFormat, UndefinedOptionFlag, MagickFalse },
    { "Unsigned", UnsignedQuantumFormat, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, FloatingPointQuantumFormat, UndefinedOptionFlag, MagickFalse }
  },
  ResolutionOptions[] =
  {
    { "Undefined", UndefinedResolution, UndefinedOptionFlag, MagickTrue },
    { "PixelsPerInch", PixelsPerInchResolution, UndefinedOptionFlag, MagickFalse },
    { "PixelsPerCentimeter", PixelsPerCentimeterResolution, UndefinedOptionFlag, MagickFalse },
    { "1", UndefinedResolution, UndefinedOptionFlag, MagickFalse },
    { "2", PixelsPerInchResolution, UndefinedOptionFlag, MagickFalse },
    { "3", PixelsPerCentimeterResolution, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedResolution, UndefinedOptionFlag, MagickFalse }
  },
  ResourceOptions[] =
  {
    { "Undefined", UndefinedResource, UndefinedOptionFlag, MagickTrue },
    { "Area", AreaResource, UndefinedOptionFlag, MagickFalse },
    { "Disk", DiskResource, UndefinedOptionFlag, MagickFalse },
    { "File", FileResource, UndefinedOptionFlag, MagickFalse },
    { "Height", HeightResource, UndefinedOptionFlag, MagickFalse },
    { "Map", MapResource, UndefinedOptionFlag, MagickFalse },
    { "Memory", MemoryResource, UndefinedOptionFlag, MagickFalse },
    { "Thread", ThreadResource, UndefinedOptionFlag, MagickFalse },
    { "Throttle", ThrottleResource, UndefinedOptionFlag, MagickFalse },
    { "Time", TimeResource, UndefinedOptionFlag, MagickFalse },
    { "Width", WidthResource, UndefinedOptionFlag, MagickFalse },
    { "ListLength", ListLengthResource, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedResource, UndefinedOptionFlag, MagickFalse }
  },
  SparseColorOptions[] =
  {
    { "Undefined", UndefinedDistortion, UndefinedOptionFlag, MagickTrue },
    { "Barycentric", BarycentricColorInterpolate, UndefinedOptionFlag, MagickFalse },
    { "Bilinear", BilinearColorInterpolate, UndefinedOptionFlag, MagickFalse },
    { "Inverse", InverseColorInterpolate, UndefinedOptionFlag, MagickFalse },
    { "Shepards", ShepardsColorInterpolate, UndefinedOptionFlag, MagickFalse },
    { "Voronoi", VoronoiColorInterpolate, UndefinedOptionFlag, MagickFalse },
    { "Manhattan", ManhattanColorInterpolate, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedResource, UndefinedOptionFlag, MagickFalse }
  },
  StatisticOptions[] =
  {
    { "Undefined", UndefinedStatistic, UndefinedOptionFlag, MagickTrue },
    { "Gradient", GradientStatistic, UndefinedOptionFlag, MagickFalse },
    { "Maximum", MaximumStatistic, UndefinedOptionFlag, MagickFalse },
    { "Mean", MeanStatistic, UndefinedOptionFlag, MagickFalse },
    { "Median", MedianStatistic, UndefinedOptionFlag, MagickFalse },
    { "Minimum", MinimumStatistic, UndefinedOptionFlag, MagickFalse },
    { "Mode", ModeStatistic, UndefinedOptionFlag, MagickFalse },
    { "NonPeak", NonpeakStatistic, UndefinedOptionFlag, MagickFalse },
    { "RootMeanSquare", RootMeanSquareStatistic, UndefinedOptionFlag, MagickFalse },
    { "RMS", RootMeanSquareStatistic, UndefinedOptionFlag, MagickFalse },
    { "StandardDeviation", StandardDeviationStatistic, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedMethod, UndefinedOptionFlag, MagickFalse }
  },
  StorageOptions[] =
  {
    { "Undefined", UndefinedPixel, UndefinedOptionFlag, MagickTrue },
    { "Char", CharPixel, UndefinedOptionFlag, MagickFalse },
    { "Double", DoublePixel, UndefinedOptionFlag, MagickFalse },
    { "Float", FloatPixel, UndefinedOptionFlag, MagickFalse },
    { "Long", LongPixel, UndefinedOptionFlag, MagickFalse },
    { "LongLong", LongLongPixel, UndefinedOptionFlag, MagickFalse },
    { "Quantum", QuantumPixel, UndefinedOptionFlag, MagickFalse },
    { "Short", ShortPixel, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedResource, UndefinedOptionFlag, MagickFalse }
  },
  StretchOptions[] =
  {
    { "Undefined", UndefinedStretch, UndefinedOptionFlag, MagickTrue },
    { "Any", AnyStretch, UndefinedOptionFlag, MagickFalse },
    { "Condensed", CondensedStretch, UndefinedOptionFlag, MagickFalse },
    { "Expanded", ExpandedStretch, UndefinedOptionFlag, MagickFalse },
    { "ExtraCondensed", ExtraCondensedStretch, UndefinedOptionFlag, MagickFalse },
    { "ExtraExpanded", ExtraExpandedStretch, UndefinedOptionFlag, MagickFalse },
    { "Normal", NormalStretch, UndefinedOptionFlag, MagickFalse },
    { "SemiCondensed", SemiCondensedStretch, UndefinedOptionFlag, MagickFalse },
    { "SemiExpanded", SemiExpandedStretch, UndefinedOptionFlag, MagickFalse },
    { "UltraCondensed", UltraCondensedStretch, UndefinedOptionFlag, MagickFalse },
    { "UltraExpanded", UltraExpandedStretch, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedStretch, UndefinedOptionFlag, MagickFalse }
  },
  StyleOptions[] =
  {
    { "Undefined", UndefinedStyle, UndefinedOptionFlag, MagickTrue },
    { "Any", AnyStyle, UndefinedOptionFlag, MagickFalse },
    { "Bold", BoldStyle, UndefinedOptionFlag, MagickFalse },
    { "Italic", ItalicStyle, UndefinedOptionFlag, MagickFalse },
    { "Normal", NormalStyle, UndefinedOptionFlag, MagickFalse },
    { "Oblique", ObliqueStyle, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedStyle, UndefinedOptionFlag, MagickFalse }
  },
  ToolOptions[] =
  {
    { "animate", 0, UndefinedOptionFlag, MagickFalse },
    { "compare", 0, UndefinedOptionFlag, MagickFalse },
    { "composite", 0, UndefinedOptionFlag, MagickFalse },
    { "conjure", 0, UndefinedOptionFlag, MagickFalse },
    { "convert", 0, UndefinedOptionFlag, MagickFalse },
    { "display", 0, UndefinedOptionFlag, MagickFalse },
    { "identify", 0, UndefinedOptionFlag, MagickFalse },
    { "import", 0, UndefinedOptionFlag, MagickFalse },
    { "mogrify", 0, UndefinedOptionFlag, MagickFalse },
    { "montage", 0, UndefinedOptionFlag, MagickFalse },
    { "stream", 0, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, 0, UndefinedOptionFlag, MagickFalse }
  },
  TypeOptions[] =
  {
    { "Undefined", UndefinedType, UndefinedOptionFlag, MagickTrue },
    { "Bilevel", BilevelType, UndefinedOptionFlag, MagickFalse },
    { "ColorSeparation", ColorSeparationType, UndefinedOptionFlag, MagickFalse },
    { "ColorSeparationAlpha", ColorSeparationAlphaType, UndefinedOptionFlag, MagickFalse },
    { "ColorSeparationMatte", ColorSeparationAlphaType, UndefinedOptionFlag, MagickFalse },
    { "Grayscale", GrayscaleType, UndefinedOptionFlag, MagickFalse },
    { "GrayscaleAlpha", GrayscaleAlphaType, UndefinedOptionFlag, MagickFalse },
    { "GrayscaleMatte", GrayscaleAlphaType, UndefinedOptionFlag, MagickFalse },
    { "Optimize", OptimizeType, UndefinedOptionFlag, MagickFalse },
    { "Palette", PaletteType, UndefinedOptionFlag, MagickFalse },
    { "PaletteBilevelAlpha", PaletteBilevelAlphaType, UndefinedOptionFlag, MagickFalse },
    { "PaletteBilevelMatte", PaletteBilevelAlphaType, UndefinedOptionFlag, MagickFalse },
    { "PaletteAlpha", PaletteAlphaType, UndefinedOptionFlag, MagickFalse },
    { "PaletteMatte", PaletteAlphaType, UndefinedOptionFlag, MagickFalse },
    { "TrueColorAlpha", TrueColorAlphaType, UndefinedOptionFlag, MagickFalse },
    { "TrueColorMatte", TrueColorAlphaType, UndefinedOptionFlag, MagickFalse },
    { "TrueColor", TrueColorType, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedType, UndefinedOptionFlag, MagickFalse }
  },
  ValidateOptions[] =
  {
    { "Undefined", UndefinedValidate, UndefinedOptionFlag, MagickTrue },
    { "All", AllValidate, UndefinedOptionFlag, MagickFalse },
    { "Colorspace", ColorspaceValidate, UndefinedOptionFlag, MagickFalse },
    { "Compare", CompareValidate, UndefinedOptionFlag, MagickFalse },
    { "Composite", CompositeValidate, UndefinedOptionFlag, MagickFalse },
    { "Convert", ConvertValidate, UndefinedOptionFlag, MagickFalse },
    { "FormatsDisk", FormatsDiskValidate, UndefinedOptionFlag, MagickFalse },
    { "FormatsMap", FormatsMapValidate, UndefinedOptionFlag, MagickFalse },
    { "FormatsMemory", FormatsMemoryValidate, UndefinedOptionFlag, MagickFalse },
    { "Identify", IdentifyValidate, UndefinedOptionFlag, MagickFalse },
    { "ImportExport", ImportExportValidate, UndefinedOptionFlag, MagickFalse },
    { "Montage", MontageValidate, UndefinedOptionFlag, MagickFalse },
    { "Stream", StreamValidate, UndefinedOptionFlag, MagickFalse },
    { "None", NoValidate, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedValidate, UndefinedOptionFlag, MagickFalse }
  },
  VirtualPixelOptions[] =
  {
    { "Undefined", UndefinedVirtualPixelMethod, UndefinedOptionFlag, MagickTrue },
    { "Background", BackgroundVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Black", BlackVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Constant", BackgroundVirtualPixelMethod, DeprecateOptionFlag, MagickTrue },
    { "CheckerTile", CheckerTileVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Dither", DitherVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Edge", EdgeVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Gray", GrayVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "HorizontalTile", HorizontalTileVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "HorizontalTileEdge", HorizontalTileEdgeVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Mirror", MirrorVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "None", TransparentVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Random", RandomVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Tile", TileVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "Transparent", TransparentVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "VerticalTile", VerticalTileVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "VerticalTileEdge", VerticalTileEdgeVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { "White", WhiteVirtualPixelMethod, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedVirtualPixelMethod, UndefinedOptionFlag, MagickFalse }
  },
  WeightOptions[] =
  {
    { "Undefined", 0L, UndefinedOptionFlag, MagickTrue },
    { "Thin", 100L, UndefinedOptionFlag, MagickFalse },
    { "ExtraLight", 200L, UndefinedOptionFlag, MagickFalse },
    { "UltraLight", 200L, UndefinedOptionFlag, MagickFalse },
    { "Light", 300L, DeprecateOptionFlag, MagickTrue },
    { "Normal", 400L, UndefinedOptionFlag, MagickFalse },
    { "Regular", 400L, UndefinedOptionFlag, MagickFalse },
    { "Medium", 500L, UndefinedOptionFlag, MagickFalse },
    { "DemiBold", 600L, UndefinedOptionFlag, MagickFalse },
    { "SemiBold", 600L, UndefinedOptionFlag, MagickFalse },
    { "Bold", 700L, UndefinedOptionFlag, MagickFalse },
    { "ExtraBold", 800L, UndefinedOptionFlag, MagickFalse },
    { "UltraBold", 800L, UndefinedOptionFlag, MagickFalse },
    { "Heavy", 900L, UndefinedOptionFlag, MagickFalse },
    { "Black", 900L, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, 0L, UndefinedOptionFlag, MagickFalse }
  };

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e O p t i o n s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageOptions() clones all global image options, to another image_info
%
%  The format of the CloneImageOptions method is:
%
%      MagickBooleanType CloneImageOptions(ImageInfo *image_info,
%        const ImageInfo *clone_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info to recieve the cloned options.
%
%    o clone_info: the source image info for options to clone.
%
*/
MagickExport MagickBooleanType CloneImageOptions(ImageInfo *image_info,
  const ImageInfo *clone_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(clone_info != (const ImageInfo *) NULL);
  assert(clone_info->signature == MagickCoreSignature);
  if (clone_info->options != (void *) NULL)
    {
      if (image_info->options != (void *) NULL)
        DestroyImageOptions(image_info);
      image_info->options=CloneSplayTree((SplayTreeInfo *) clone_info->options,
        (void *(*)(void *)) ConstantString,(void *(*)(void *)) ConstantString);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e f i n e I m a g e O p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefineImageOption() associates an assignment string of the form
%  "key=value" with a global image option. It is equivelent to
%  SetImageOption().
%
%  The format of the DefineImageOption method is:
%
%      MagickBooleanType DefineImageOption(ImageInfo *image_info,
%        const char *option)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o option: the image option assignment string.
%
*/
MagickExport MagickBooleanType DefineImageOption(ImageInfo *image_info,
  const char *option)
{
  char
    key[MagickPathExtent],
    value[MagickPathExtent];

  register char
    *p;

  assert(image_info != (ImageInfo *) NULL);
  assert(option != (const char *) NULL);
  (void) CopyMagickString(key,option,MagickPathExtent);
  for (p=key; *p != '\0'; p++)
    if (*p == '=')
      break;
  *value='\0';
  if (*p == '=')
    (void) CopyMagickString(value,p+1,MagickPathExtent);
  *p='\0';
  return(SetImageOption(image_info,key,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e O p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageOption() deletes an key from the global image options.
%
%  Returns MagickTrue is the option is found and deleted from the Options.
%
%  The format of the DeleteImageOption method is:
%
%      MagickBooleanType DeleteImageOption(ImageInfo *image_info,
%        const char *key)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o option: the image option.
%
*/
MagickExport MagickBooleanType DeleteImageOption(ImageInfo *image_info,
  const char *option)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options == (void *) NULL)
    return(MagickFalse);
  return(DeleteNodeFromSplayTree((SplayTreeInfo *) image_info->options,option));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e O p t i o n s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageOptions() destroys all global options and associated memory
%  attached to the given image_info image list.
%
%  The format of the DestroyDefines method is:
%
%      void DestroyImageOptions(ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport void DestroyImageOptions(ImageInfo *image_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options != (void *) NULL)
    image_info->options=DestroySplayTree((SplayTreeInfo *) image_info->options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e O p t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageOption() gets a value associated with the global image options.
%
%  The returned string is a constant string in the tree and should NOT be
%  freed by the caller.
%
%  The format of the GetImageOption method is:
%
%      const char *GetImageOption(const ImageInfo *image_info,
%        const char *option)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o option: the option.
%
*/
MagickExport const char *GetImageOption(const ImageInfo *image_info,
  const char *option)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options == (void *) NULL)
    return((const char *) NULL);
  return((const char *) GetValueFromSplayTree((SplayTreeInfo *)
    image_info->options,option));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o m m a n d O p t i o n F l a g s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCommandOptionFlags() parses a string and returns an enumerated option
%  flags(s).  Return a value of -1 if no such option is found.
%
%  The format of the GetCommandOptionFlags method is:
%
%      ssize_t GetCommandOptionFlags(const CommandOption option,
%        const MagickBooleanType list,const char *options)
%
%  A description of each parameter follows:
%
%    o option: Index to the option table to lookup
%
%    o list: A option other than zero permits more than one option separated by
%      a comma or pipe.
%
%    o options: One or more options separated by commas.
%
*/

static const OptionInfo *GetOptionInfo(const CommandOption option)
{
  switch (option)
  {
    case MagickAlignOptions: return(AlignOptions);
    case MagickAlphaChannelOptions: return(AlphaChannelOptions);
    case MagickAutoThresholdOptions: return(AutoThresholdOptions);
    case MagickBooleanOptions: return(BooleanOptions);
    case MagickCacheOptions: return(CacheOptions);
    case MagickChannelOptions: return(ChannelOptions);
    case MagickClassOptions: return(ClassOptions);
    case MagickCLIOptions: return(CLIOptions);
    case MagickClipPathOptions: return(ClipPathOptions);
    case MagickColorspaceOptions: return(ColorspaceOptions);
    case MagickCommandOptions: return(CommandOptions);
    case MagickComplianceOptions: return(ComplianceOptions);
    case MagickComplexOptions: return(ComplexOptions);
    case MagickComposeOptions: return(ComposeOptions);
    case MagickCompressOptions: return(CompressOptions);
    case MagickDataTypeOptions: return(DataTypeOptions);
    case MagickDebugOptions: return(LogEventOptions);
    case MagickDecorateOptions: return(DecorateOptions);
    case MagickDirectionOptions: return(DirectionOptions);
    case MagickDisposeOptions: return(DisposeOptions);
    case MagickDistortOptions: return(DistortOptions);
    case MagickDitherOptions: return(DitherOptions);
    case MagickEndianOptions: return(EndianOptions);
    case MagickEvaluateOptions: return(EvaluateOptions);
    case MagickFillRuleOptions: return(FillRuleOptions);
    case MagickFilterOptions: return(FilterOptions);
    case MagickFunctionOptions: return(FunctionOptions);
    case MagickGradientOptions: return(GradientOptions);
    case MagickGravityOptions: return(GravityOptions);
    case MagickIntentOptions: return(IntentOptions);
    case MagickInterlaceOptions: return(InterlaceOptions);
    case MagickInterpolateOptions: return(InterpolateOptions);
    case MagickKernelOptions: return(KernelOptions);
    case MagickLayerOptions: return(LayerOptions);
    case MagickLineCapOptions: return(LineCapOptions);
    case MagickLineJoinOptions: return(LineJoinOptions);
    case MagickListOptions: return(ListOptions);
    case MagickLogEventOptions: return(LogEventOptions);
    case MagickMetricOptions: return(MetricOptions);
    case MagickMethodOptions: return(MethodOptions);
    case MagickModeOptions: return(ModeOptions);
    case MagickMorphologyOptions: return(MorphologyOptions);
    case MagickNoiseOptions: return(NoiseOptions);
    case MagickOrientationOptions: return(OrientationOptions);
    case MagickPixelChannelOptions: return(PixelChannelOptions);
    case MagickPixelIntensityOptions: return(PixelIntensityOptions);
    case MagickPixelMaskOptions: return(PixelMaskOptions);
    case MagickPixelTraitOptions: return(PixelTraitOptions);
    case MagickPolicyDomainOptions: return(PolicyDomainOptions);
    case MagickPolicyRightsOptions: return(PolicyRightsOptions);
    case MagickPreviewOptions: return(PreviewOptions);
    case MagickPrimitiveOptions: return(PrimitiveOptions);
    case MagickQuantumFormatOptions: return(QuantumFormatOptions);
    case MagickResolutionOptions: return(ResolutionOptions);
    case MagickResourceOptions: return(ResourceOptions);
    case MagickSparseColorOptions: return(SparseColorOptions);
    case MagickStatisticOptions: return(StatisticOptions);
    case MagickStorageOptions: return(StorageOptions);
    case MagickStretchOptions: return(StretchOptions);
    case MagickToolOptions: return(ToolOptions);
    case MagickStyleOptions: return(StyleOptions);
    case MagickTypeOptions: return(TypeOptions);
    case MagickValidateOptions: return(ValidateOptions);
    case MagickVirtualPixelOptions: return(VirtualPixelOptions);
    case MagickWeightOptions: return(WeightOptions);
    default: break;
  }
  return((const OptionInfo *) NULL);
}

MagickExport ssize_t GetCommandOptionFlags(const CommandOption option,
  const MagickBooleanType list,const char *options)
{
  char
    token[MagickPathExtent];

  const OptionInfo
    *command_info,
    *option_info;

  int
    sentinel;

  MagickBooleanType
    negate;

  register char
    *q;

  register const char
    *p;

  register ssize_t
    i;

  ssize_t
    option_types;

  if ((options == (const char *) NULL) || (*options == '\0'))
    return(-1);
  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return(UndefinedOptionFlag);
  option_types=0;
  sentinel=',';
  if (strchr(options,'|') != (char *) NULL)
    sentinel='|';
  for (p=options; p != (char *) NULL; p=strchr(p,sentinel))
  {
    while (((isspace((int) ((unsigned char) *p)) != 0) || (*p == sentinel)) &&
           (*p != '\0'))
      p++;
    negate=(*p == '!') ? MagickTrue : MagickFalse;
    if (negate != MagickFalse)
      p++;
    q=token;
    while (((isspace((int) ((unsigned char) *p)) == 0) && (*p != sentinel)) &&
           (*p != '\0'))
    {
      if ((q-token) >= (MagickPathExtent-1))
        break;
      *q++=(*p++);
    }
    *q='\0';
    for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
      if (LocaleCompare(token,option_info[i].mnemonic) == 0)
        break;
    command_info=option_info+i;
    if ((command_info->mnemonic == (const char *) NULL) && (*token != '\0') &&
        ((strchr(token+1,'-') != (char *) NULL) ||
         (strchr(token+1,'_') != (char *) NULL)))
      {
        while ((q=strchr(token+1,'-')) != (char *) NULL)
          (void) CopyMagickString(q,q+1,MagickPathExtent-strlen(q));
        while ((q=strchr(token+1,'_')) != (char *) NULL)
          (void) CopyMagickString(q,q+1,MagickPathExtent-strlen(q));
        for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
          if (LocaleCompare(token,option_info[i].mnemonic) == 0)
            break;
        command_info=option_info+i;
      }
    if (command_info->mnemonic == (const char *) NULL)
      return(-1);
    if (negate != MagickFalse)
      option_types=option_types &~ command_info->flags;
    else
      option_types=option_types | command_info->flags;
    if (list == MagickFalse)
      break;
  }
  return(option_types);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o m m a n d O p t i o n I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCommandOptionInfo() returns a pointer to the matching OptionInfo entry
%  for the "CommandOptions" table.  It returns both the type (argument count)
%  and flags (argument type).
%
%  The format of the GetCommandOptionInfo method is:
%
%      const char **GetCommandOptionInfo(const char *option)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
*/
MagickExport const OptionInfo *GetCommandOptionInfo(const char *option)
{
  register ssize_t
    i;

  for (i=0; CommandOptions[i].mnemonic != (char *) NULL; i++)
    if (LocaleCompare(option,CommandOptions[i].mnemonic) == 0)
      break;
  return(CommandOptions+i);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o m m a n d O p t i o n s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCommandOptions() returns a list of command options.
%
%  The format of the GetCommandOptions method is:
%
%      const char **GetCommandOptions(const CommandOption option)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
*/
MagickExport char **GetCommandOptions(const CommandOption option)
{
  char
    **options;

  const OptionInfo
    *option_info;

  register ssize_t
    i;

  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return((char **) NULL);
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++) ;
  options=(char **) AcquireQuantumMemory((size_t) i+1UL,sizeof(*options));
  if (options == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++)
    options[i]=AcquireString(option_info[i].mnemonic);
  options[i]=(char *) NULL;
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e O p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImageOption() gets the next global option value.
%
%  The format of the GetNextImageOption method is:
%
%      char *GetNextImageOption(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport char *GetNextImageOption(const ImageInfo *image_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options == (void *) NULL)
    return((char *) NULL);
  return((char *) GetNextKeyInSplayTree((SplayTreeInfo *) image_info->options));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s C o m m a n d O p t i o n                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsCommandOption() returns MagickTrue if the option begins with a - or + and
%  the first character that follows is alphanumeric.
%
%  The format of the IsCommandOption method is:
%
%      MagickBooleanType IsCommandOption(const char *option)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
*/
MagickExport MagickBooleanType IsCommandOption(const char *option)
{
  assert(option != (const char *) NULL);
  if ((*option != '-') && (*option != '+'))
    return(MagickFalse);
  if (strlen(option) == 1)
    return(((*option == '{') || (*option == '}') || (*option == '[') ||
      (*option == ']')) ? MagickTrue : MagickFalse);
  option++;
  if (isalpha((int) ((unsigned char) *option)) == 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m m a n d O p t i o n T o M n e m o n i c                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CommandOptionToMnemonic() returns an enumerated value as a mnemonic.
%
%  The format of the CommandOptionToMnemonic method is:
%
%      const char *CommandOptionToMnemonic(const CommandOption option,
%        const ssize_t type)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
%    o type: one or more values separated by commas.
%
*/
MagickExport const char *CommandOptionToMnemonic(const CommandOption option,
  const ssize_t type)
{
  const OptionInfo
    *option_info;

  register ssize_t
    i;

  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return((const char *) NULL);
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++)
    if (type == option_info[i].type)
      break;
  if (option_info[i].mnemonic == (const char *) NULL)
    return("Unrecognized");
  return(option_info[i].mnemonic);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s O p t i o n M e m b e r                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsOptionMember() returns MagickTrue if the option is a member of the options
%  list (e.g. ICC is a member of xmp,icc,iptc).
%
%  The format of the IsOptionMember function is:
%
%      MagickBooleanType IsOptionMember(const char *option,
%        const char *options)
%
%  A description of each parameter follows:
%
%    o option: an option or option expression (e.g. ICC or *).
%
%    o options: one or more options separated by commas.
%
*/
MagickExport MagickBooleanType IsOptionMember(const char *option,
  const char *options)
{
  char
    **option_list,
    *string;

  int
    number_options;

  MagickBooleanType
    member;

  register ssize_t
    i;

  /*
    Is option a member of the options list?
  */
  if (options == (const char *) NULL)
    return(MagickFalse);
  string=ConstantString(options);
  (void) SubstituteString(&string,","," ");
  option_list=StringToArgv(string,&number_options);
  string=DestroyString(string);
  if (option_list == (char **) NULL)
    return(MagickFalse);
  member=MagickFalse;
  option_list[0]=DestroyString(option_list[0]);
  for (i=1; i < (ssize_t) number_options; i++)
  {
    if ((*option_list[i] == '!') &&
        (LocaleCompare(option,option_list[i]+1) == 0))
      break;
    if (GlobExpression(option,option_list[i],MagickTrue) != MagickFalse)
      {
        member=MagickTrue;
        break;
      }
    option_list[i]=DestroyString(option_list[i]);
  }
  for ( ; i < (ssize_t) number_options; i++)
    option_list[i]=DestroyString(option_list[i]);
  option_list=(char **) RelinquishMagickMemory(option_list);
  return(member);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i s t C o m m a n d O p t i o n s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListCommandOptions() lists the contents of enumerated option type(s).
%
%  The format of the ListCommandOptions method is:
%
%      MagickBooleanType ListCommandOptions(FILE *file,
%        const CommandOption option,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o file:  list options to this file handle.
%
%    o option:  list these options.
%
%    o exception:  return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListCommandOptions(FILE *file,
  const CommandOption option,ExceptionInfo *magick_unused(exception))
{
  const OptionInfo
    *option_info;

  register ssize_t
    i;

  magick_unreferenced(exception);
  if (file == (FILE *) NULL)
    file=stdout;
  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return(MagickFalse);
  for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
  {
    if (option_info[i].stealth != MagickFalse)
      continue;
    (void) FormatLocaleFile(file,"%s\n",option_info[i].mnemonic);
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e C h a n n e l O p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseChannelOption() parses a string and returns an enumerated channel
%  type(s).
%
%  The format of the ParseChannelOption method is:
%
%      ssize_t ParseChannelOption(const char *channels)
%
%  A description of each parameter follows:
%
%    o options: One or more values separated by commas.
%
*/
MagickExport ssize_t ParseChannelOption(const char *channels)
{
  register ssize_t
    i;

  size_t
    length;

  ssize_t
    channel;

  channel=ParseCommandOption(MagickChannelOptions,MagickTrue,channels);
  if (channel >= 0)
    return(channel);
  channel=0;
  length=strlen(channels);
  for (i=0; i < (ssize_t) length; i++)
  {
    switch (channels[i])
    {
      case 'A':
      case 'a':
      {
        channel|=AlphaChannel;
        break;
      }
      case 'B':
      case 'b':
      {
        channel|=BlueChannel;
        break;
      }
      case 'C':
      case 'c':
      {
        channel|=CyanChannel;
        break;
      }
      case 'g':
      case 'G':
      {
        channel|=GreenChannel;
        break;
      }
      case 'K':
      case 'k':
      {
        channel|=BlackChannel;
        break;
      }
      case 'M':
      case 'm':
      {
        channel|=MagentaChannel;
        break;
      }
      case 'o':
      case 'O':
      {
        channel|=AlphaChannel; /* depreciate */
        break;
      }
      case 'R':
      case 'r':
      {
        channel|=RedChannel;
        break;
      }
      case 'Y':
      case 'y':
      {
        channel|=YellowChannel;
        break;
      }
      case ',':
      {
        ssize_t
          type;

        /*
          Gather the additional channel flags and merge with shorthand.
        */
        type=ParseCommandOption(MagickChannelOptions,MagickTrue,channels+i+1);
        if (type < 0)
          return(type);
        channel|=type;
        return(channel);
      }
      default:
        return(-1);
    }
  }
  return(channel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e C o m m a n d O p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseCommandOption() parses a string and returns an enumerated option
%  type(s).  Return a value of -1 if no such option is found.
%
%  The format of the ParseCommandOption method is:
%
%      ssize_t ParseCommandOption(const CommandOption option,
%        const MagickBooleanType list,const char *options)
%
%  A description of each parameter follows:
%
%    o option: Index to the option table to lookup
%
%    o list: A option other than zero permits more than one option separated by
%      a comma or pipe.
%
%    o options: One or more options separated by commas.
%
*/
MagickExport ssize_t ParseCommandOption(const CommandOption option,
  const MagickBooleanType list,const char *options)
{
  char
    token[MagickPathExtent];

  const OptionInfo
    *command_info,
    *option_info;

  int
    sentinel;

  MagickBooleanType
    negate;

  register char
    *q;

  register const char
    *p;

  register ssize_t
    i;

  ssize_t
    option_types;

  if ((options == (const char *) NULL) || (*options == '\0'))
    return(-1);
  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return(-1);
  option_types=0;
  sentinel=',';
  if (strchr(options,'|') != (char *) NULL)
    sentinel='|';
  for (p=options; p != (char *) NULL; p=strchr(p,sentinel))
  {
    while (((isspace((int) ((unsigned char) *p)) != 0) || (*p == sentinel)) &&
           (*p != '\0'))
      p++;
    negate=(*p == '!') ? MagickTrue : MagickFalse;
    if (negate != MagickFalse)
      p++;
    q=token;
    while (((isspace((int) ((unsigned char) *p)) == 0) && (*p != sentinel)) &&
           (*p != '\0'))
    {
      if ((q-token) >= (MagickPathExtent-1))
        break;
      *q++=(*p++);
    }
    *q='\0';
    for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
      if (LocaleCompare(token,option_info[i].mnemonic) == 0)
        break;
    command_info=option_info+i;
    if ((command_info->mnemonic == (const char *) NULL) && (*token != '\0') &&
        ((strchr(token+1,'-') != (char *) NULL) ||
         (strchr(token+1,'_') != (char *) NULL)))
        {
          while ((q=strchr(token+1,'-')) != (char *) NULL)
            (void) CopyMagickString(q,q+1,MagickPathExtent-strlen(q));
          while ((q=strchr(token+1,'_')) != (char *) NULL)
            (void) CopyMagickString(q,q+1,MagickPathExtent-strlen(q));
          for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
            if (LocaleCompare(token,option_info[i].mnemonic) == 0)
              break;
          command_info=option_info+i;
        }
    if (command_info->mnemonic == (const char *) NULL)
      return(-1);
    if (negate != MagickFalse)
      option_types=option_types &~ command_info->type;
    else
      option_types=option_types | command_info->type;
    if (list == MagickFalse)
      break;
  }
  return(option_types);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e P i x e l C h a n n e l O p t i o n                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParsePixelChannelOption() parses a string and returns an enumerated pixel
%  channel type(s).
%
%  The format of the ParsePixelChannelOption method is:
%
%      ssize_t ParsePixelChannelOption(const char *channels)
%
%  A description of each parameter follows:
%
%    o channels: One or more channels separated by commas.
%
*/
MagickExport ssize_t ParsePixelChannelOption(const char *channels)
{
  char
    *q,
    token[MagickPathExtent];

  ssize_t
    channel;

  (void) GetNextToken(channels,(const char **) NULL,MagickPathExtent,token);
  if ((*token == ';') || (*token == '|'))
    return(RedPixelChannel);
  channel=ParseCommandOption(MagickPixelChannelOptions,MagickTrue,token);
  if (channel >= 0)
    return(channel);
  q=(char *) token;
  channel=(ssize_t) InterpretLocaleValue(token,&q);
  if ((q == token) || (channel < 0) || (channel >= MaxPixelChannels))
    return(-1);
  return(channel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e I m a g e O p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveImageOption() removes an option from the image and returns its value.
%
%  In this case the ConstantString() value returned should be freed by the
%  caller when finished.
%
%  The format of the RemoveImageOption method is:
%
%      char *RemoveImageOption(ImageInfo *image_info,const char *option)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o option: the image option.
%
*/
MagickExport char *RemoveImageOption(ImageInfo *image_info,const char *option)
{
  char
    *value;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options == (void *) NULL)
    return((char *) NULL);
  value=(char *) RemoveNodeFromSplayTree((SplayTreeInfo *)
    image_info->options,option);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e O p t i o n                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImageOptions() resets the image_info option.  That is, it deletes
%  all global options associated with the image_info structure.
%
%  The format of the ResetImageOptions method is:
%
%      ResetImageOptions(ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport void ResetImageOptions(const ImageInfo *image_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options == (void *) NULL)
    return;
  ResetSplayTree((SplayTreeInfo *) image_info->options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e O p t i o n I t e r a t o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImageOptionIterator() resets the image_info values iterator.  Use it
%  in conjunction with GetNextImageOption() to iterate over all the values
%  associated with an image option.
%
%  The format of the ResetImageOptionIterator method is:
%
%      ResetImageOptionIterator(ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport void ResetImageOptionIterator(const ImageInfo *image_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options == (void *) NULL)
    return;
  ResetSplayTreeIterator((SplayTreeInfo *) image_info->options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e O p t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageOption() associates an value with an image option.
%
%  The format of the SetImageOption method is:
%
%      MagickBooleanType SetImageOption(ImageInfo *image_info,
%        const char *option,const char *value)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o option: the image option.
%
%    o values: the image option values.
%
*/
MagickExport MagickBooleanType SetImageOption(ImageInfo *image_info,
  const char *option,const char *value)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  /*
    Specific global option settings.
  */
  if (LocaleCompare(option,"size") == 0) {
    (void) CloneString(&image_info->size,value);
    return(MagickTrue);
  }
  /*
    Create tree if needed - specify how key,values are to be freed.
  */
  if (image_info->options == (void *) NULL)
    image_info->options=NewSplayTree(CompareSplayTreeString,
      RelinquishMagickMemory,RelinquishMagickMemory);
  /*
    Delete Option if NULL --  empty string values are valid!
  */
  if (value == (const char *) NULL)
    return(DeleteImageOption(image_info,option));
  /*
    Add option to splay-tree.
  */
  return(AddValueToSplayTree((SplayTreeInfo *) image_info->options,
    ConstantString(option),ConstantString(value)));
}

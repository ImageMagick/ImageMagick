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
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
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
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/compare.h"
#include "magick/constitute.h"
#include "magick/distort.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fourier.h"
#include "magick/fx.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/layer.h"
#include "magick/mime-private.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/montage.h"
#include "magick/morphology.h"
#include "magick/option.h"
#include "magick/policy.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/resample.h"
#include "magick/resource_.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"

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
  AlphaOptions[] =
  {
    { "Undefined", UndefinedAlphaChannel, UndefinedOptionFlag, MagickTrue },
    { "Activate", ActivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Background", BackgroundAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Copy", CopyAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Deactivate", DeactivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Extract", ExtractAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Flatten", FlattenAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Off", DeactivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "On", ActivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Opaque", OpaqueAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Remove", RemoveAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Set", SetAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Shape", ShapeAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Reset", SetAlphaChannel, DeprecateOptionFlag, MagickTrue },
    { "Transparent", TransparentAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedAlphaChannel, UndefinedOptionFlag, MagickFalse }
  },
  BooleanOptions[] =
  {
    { "False", 0L, UndefinedOptionFlag, MagickFalse },
    { "True", 1L, UndefinedOptionFlag, MagickFalse },
    { "0", 0L, UndefinedOptionFlag, MagickFalse },
    { "1", 1L, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, 0L, UndefinedOptionFlag, MagickFalse }
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
    { "A", AlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "All", CompositeChannels, UndefinedOptionFlag, MagickFalse },
    { "Alpha", OpacityChannel, UndefinedOptionFlag, MagickFalse },
    { "B", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Black", BlackChannel, UndefinedOptionFlag, MagickFalse },
    { "Blue", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "C", CyanChannel, UndefinedOptionFlag, MagickFalse },
    { "Cyan", CyanChannel, UndefinedOptionFlag, MagickFalse },
    { "Default", DefaultChannels, UndefinedOptionFlag, MagickFalse },
    { "G", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Gray", GrayChannel, UndefinedOptionFlag, MagickFalse },
    { "Green", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "H", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "Hue", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "Index", IndexChannel, UndefinedOptionFlag, MagickFalse },
    { "K", BlackChannel, UndefinedOptionFlag, MagickFalse },
    { "L", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Lightness", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Luminance", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Luminosity", BlueChannel, DeprecateOptionFlag, MagickTrue },
    { "Magenta", MagentaChannel, UndefinedOptionFlag, MagickFalse },
    { "Matte", OpacityChannel, UndefinedOptionFlag, MagickFalse },
    { "M", MagentaChannel, UndefinedOptionFlag, MagickFalse },
    { "O", OpacityChannel, UndefinedOptionFlag, MagickFalse },
    { "Opacity", OpacityChannel, UndefinedOptionFlag, MagickFalse },
    { "Red", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "R", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "Saturation", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "S", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Sync", SyncChannels, UndefinedOptionFlag, MagickFalse },
    { "Y", YellowChannel, UndefinedOptionFlag, MagickFalse },
    { "Yellow", YellowChannel, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedChannel, UndefinedOptionFlag, MagickFalse }
  },
  ClassOptions[] =
  {
    { "Undefined", UndefinedClass, UndefinedOptionFlag, MagickTrue },
    { "DirectClass", DirectClass, UndefinedOptionFlag, MagickFalse },
    { "PseudoClass", PseudoClass, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedClass, UndefinedOptionFlag, MagickFalse }
  },
  ClipPathOptions[] =
  {
    { "Undefined", UndefinedPathUnits, UndefinedOptionFlag, MagickTrue },
    { "ObjectBoundingBox", ObjectBoundingBox, UndefinedOptionFlag, MagickFalse },
    { "UserSpace", UserSpace, UndefinedOptionFlag, MagickFalse },
    { "UserSpaceOnUse", UserSpaceOnUse, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPathUnits, UndefinedOptionFlag, MagickFalse }
  },
  CommandOptions[] =
  {
    { "+adjoin", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-adjoin", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+adaptive-blur", 1L, DeprecateOptionFlag, MagickFalse },
    { "-adaptive-blur", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+adaptive-resize", 1L, DeprecateOptionFlag, MagickFalse },
    { "-adaptive-resize", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+adaptive-sharpen", 1L, DeprecateOptionFlag, MagickFalse },
    { "-adaptive-sharpen", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+affine", 0L, DrawInfoOptionFlag, MagickFalse },
    { "-affine", 1L, DrawInfoOptionFlag, MagickFalse },
    { "+affinity", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-affinity", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "+alpha", 1L, DeprecateOptionFlag, MagickFalse },
    { "-alpha", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+annotate", 0L, DeprecateOptionFlag, MagickFalse },
    { "-annotate", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "+antialias", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-antialias", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+append", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-append", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+attenuate", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-attenuate", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+authenticate", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-authenticate", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+auto-gamma", 0L, DeprecateOptionFlag, MagickFalse },
    { "-auto-gamma", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+auto-level", 0L, DeprecateOptionFlag, MagickFalse },
    { "-auto-level", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+auto-orient", 0L, DeprecateOptionFlag, MagickFalse },
    { "-auto-orient", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+average", 0L, ListOperatorOptionFlag | FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-average", 0L, ListOperatorOptionFlag | FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+backdrop", 0L, NonConvertOptionFlag, MagickFalse },
    { "-backdrop", 1L, NonConvertOptionFlag, MagickFalse },
    { "+background", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-background", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+bench", 0L, GenesisOptionFlag, MagickFalse },
    { "-bench", 1L, GenesisOptionFlag, MagickFalse },
    { "+bias", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-bias", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+black-point-compensation", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-black-point-compensation", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+black-threshold", 0L, DeprecateOptionFlag, MagickFalse },
    { "-black-threshold", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+blend", 0L, NonConvertOptionFlag, MagickFalse },
    { "-blend", 1L, NonConvertOptionFlag, MagickFalse },
    { "+blue-primary", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-blue-primary", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+blue-shift", 1L, DeprecateOptionFlag, MagickFalse },
    { "-blue-shift", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+blur", 1L, DeprecateOptionFlag, MagickFalse },
    { "-blur", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+border", 1L, DeprecateOptionFlag, MagickFalse },
    { "-border", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+bordercolor", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-bordercolor", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+borderwidth", 0L, NonConvertOptionFlag, MagickFalse },
    { "-borderwidth", 1L, NonConvertOptionFlag, MagickFalse },
    { "+box", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-box", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+brightness-contrast", 0L, DeprecateOptionFlag, MagickFalse },
    { "-brightness-contrast", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+cache", 0L, GlobalOptionFlag, MagickFalse },
    { "-cache", 1L, GlobalOptionFlag, MagickFalse },
    { "+cdl", 1L, DeprecateOptionFlag, MagickFalse },
    { "-cdl", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+channel", 0L, ImageInfoOptionFlag | ListOperatorOptionFlag, MagickFalse },
    { "-channel", 1L, ImageInfoOptionFlag | ListOperatorOptionFlag, MagickFalse },
    { "+charcoal", 0L, DeprecateOptionFlag, MagickFalse },
    { "-charcoal", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+chop", 1L, DeprecateOptionFlag, MagickFalse },
    { "-chop", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+clamp", 0L, DeprecateOptionFlag, MagickFalse },
    { "-clamp", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+clip", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-clip", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+clip-mask", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-clip-mask", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+clip-path", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-clip-path", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+clone", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "-clone", 1L, SpecialOperatorOptionFlag, MagickFalse },
    { "+clut", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-clut", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+coalesce", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-coalesce", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+colorize", 1L, DeprecateOptionFlag, MagickFalse },
    { "-colorize", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+colormap", 0L, NonConvertOptionFlag, MagickFalse },
    { "-colormap", 1L, NonConvertOptionFlag, MagickFalse },
    { "+color-matrix", 1L, DeprecateOptionFlag, MagickFalse },
    { "-color-matrix", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+colors", 1L, DeprecateOptionFlag, MagickFalse },
    { "-colors", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+colorspace", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "-colorspace", 1L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "+combine", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-combine", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+comment", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-comment", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+compare", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-compare", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+complex", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-complex", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+compose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-compose", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+composite", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-composite", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+compress", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-compress", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+concurrent", 0L, GenesisOptionFlag, MagickTrue },
    { "-concurrent", 0L, GenesisOptionFlag, MagickTrue },
    { "+contrast", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-contrast", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+contrast-stretch", 1L, DeprecateOptionFlag, MagickFalse },
    { "-contrast-stretch", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+convolve", 1L, DeprecateOptionFlag, MagickFalse },
    { "-convolve", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+crop", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-crop", 1L, SimpleOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+cycle", 1L, DeprecateOptionFlag, MagickFalse },
    { "-cycle", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+debug", 0L, GlobalOptionFlag|GenesisOptionFlag | FireOptionFlag, MagickFalse },
    { "-debug", 1L, GlobalOptionFlag|GenesisOptionFlag | FireOptionFlag, MagickFalse },
    { "+decipher", 1L, DeprecateOptionFlag, MagickFalse },
    { "-decipher", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+deconstruct", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-deconstruct", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+define", 1L, ImageInfoOptionFlag | FireOptionFlag, MagickFalse },
    { "-define", 1L, ImageInfoOptionFlag | FireOptionFlag, MagickFalse },
    { "+delay", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-delay", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+delete", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-delete", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+density", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-density", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+depth", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-depth", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+descend", 0L, NonConvertOptionFlag, MagickFalse },
    { "-descend", 1L, NonConvertOptionFlag, MagickFalse },
    { "+deskew", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-deskew", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+despeckle", 0L, DeprecateOptionFlag, MagickFalse },
    { "-despeckle", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+direction", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-direction", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+displace", 0L, NonConvertOptionFlag, MagickFalse },
    { "-displace", 1L, NonConvertOptionFlag, MagickFalse },
    { "+display", 1L, ImageInfoOptionFlag, MagickFalse },
    { "-display", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+dispose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-dispose", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+dissimilarity-threshold", 0L, NonConvertOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "-dissimilarity-threshold", 1L, NonConvertOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+dissolve", 0L, NonConvertOptionFlag, MagickFalse },
    { "-dissolve", 1L, NonConvertOptionFlag, MagickFalse },
    { "+distort", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "-distort", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "+dither", 0L, ListOperatorOptionFlag | ImageInfoOptionFlag | QuantizeInfoOptionFlag, MagickFalse },
    { "-dither", 1L, ListOperatorOptionFlag | ImageInfoOptionFlag | QuantizeInfoOptionFlag, MagickFalse },
    { "+draw", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-draw", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+duplicate", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-duplicate", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+duration", 1L, GenesisOptionFlag, MagickFalse },
    { "-duration", 1L, GenesisOptionFlag, MagickFalse },
    { "+edge", 1L, DeprecateOptionFlag, MagickFalse },
    { "-edge", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+emboss", 1L, DeprecateOptionFlag, MagickFalse },
    { "-emboss", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+encipher", 1L, DeprecateOptionFlag, MagickFalse },
    { "-encipher", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+encoding", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-encoding", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+endian", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-endian", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+enhance", 0L, DeprecateOptionFlag, MagickFalse },
    { "-enhance", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+equalize", 0L, DeprecateOptionFlag, MagickFalse },
    { "-equalize", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+evaluate", 2L, DeprecateOptionFlag, MagickFalse },
    { "-evaluate", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "+evaluate-sequence", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-evaluate-sequence", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+extent", 1L, DeprecateOptionFlag, MagickFalse },
    { "-extent", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+extract", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-extract", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+family", 0L, DeprecateOptionFlag, MagickFalse },
    { "-family", 1L, DrawInfoOptionFlag, MagickFalse },
    { "+features", 0L, SimpleOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-features", 1L, SimpleOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+fft", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-fft", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+fill", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-fill", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+filter", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-filter", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+flatten", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-flatten", 0L, ListOperatorOptionFlag | FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+flip", 0L, DeprecateOptionFlag, MagickFalse },
    { "-flip", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+flop", 0L, DeprecateOptionFlag, MagickFalse },
    { "-flop", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+floodfill", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "-floodfill", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "+font", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-font", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+foreground", 0L, NonConvertOptionFlag, MagickFalse },
    { "-foreground", 1L, NonConvertOptionFlag, MagickFalse },
    { "+format", 0L, DeprecateOptionFlag, MagickFalse },
    { "-format", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+frame", 1L, DeprecateOptionFlag, MagickFalse },
    { "-frame", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+fuzz", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-fuzz", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+fx", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-fx", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+gamma", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-gamma", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+gaussian", 1L, DeprecateOptionFlag, MagickFalse },
    { "-gaussian", 1L, SimpleOperatorOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+gaussian-blur", 1L, DeprecateOptionFlag, MagickFalse },
    { "-gaussian-blur", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+geometry", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-geometry", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+gravity", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-gravity", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+grayscale", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-grayscale", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+green-primary", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-green-primary", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+hald-clut", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-hald-clut", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+help", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "-help", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "+highlight-color", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-highlight-color", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+iconGeometry", 0L, NonConvertOptionFlag, MagickFalse },
    { "-iconGeometry", 1L, NonConvertOptionFlag, MagickFalse },
    { "+iconic", 0L, NonConvertOptionFlag, MagickFalse },
    { "-iconic", 1L, NonConvertOptionFlag, MagickFalse },
    { "+identify", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-identify", 0L, SimpleOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+ift", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-ift", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+immutable", 0L, NonConvertOptionFlag, MagickFalse },
    { "-immutable", 0L, NonConvertOptionFlag, MagickFalse },
    { "+implode", 0L, DeprecateOptionFlag, MagickFalse },
    { "-implode", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+insert", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-insert", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
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
    { "+interpolative-resize", 1L, DeprecateOptionFlag, MagickFalse },
    { "-interpolative-resize", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+interword-spacing", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-interword-spacing", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+kerning", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-kerning", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+label", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-label", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+lat", 1L, DeprecateOptionFlag, MagickFalse },
    { "-lat", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+layers", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-layers", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+level", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-level", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+level-colors", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-level-colors", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+limit", 0L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "-limit", 2L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "+linear-stretch", 1L, DeprecateOptionFlag, MagickFalse },
    { "-linear-stretch", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+linewidth", 0L, DrawInfoOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-linewidth", 1L, DrawInfoOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+liquid-rescale", 1L, DeprecateOptionFlag, MagickFalse },
    { "-liquid-rescale", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+list", 0L, GlobalOptionFlag, MagickFalse },
    { "-list", 1L, GlobalOptionFlag, MagickFalse },
    { "+log", 0L, GlobalOptionFlag, MagickFalse },
    { "-log", 1L, GlobalOptionFlag, MagickFalse },
    { "+loop", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-loop", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+lowlight-color", 1L, DeprecateOptionFlag, MagickFalse },
    { "-lowlight-color", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+magnify", 0L, DeprecateOptionFlag, MagickFalse },
    { "-magnify", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+map", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-map", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+mask", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-mask", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+matte", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-matte", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+mattecolor", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-mattecolor", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+maximum", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-maximum", 0L, ListOperatorOptionFlag | FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+median", 1L, DeprecateOptionFlag, MagickFalse },
    { "-median", 1L, SimpleOperatorOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+metric", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-metric", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+minimum", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-minimum", 0L, ImageInfoOptionFlag | FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+mode", 1L, NonConvertOptionFlag, MagickFalse },
    { "-mode", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+modulate", 1L, DeprecateOptionFlag, MagickFalse },
    { "-modulate", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+moments", 0L, SimpleOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-moments", 0L, SimpleOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+monitor", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "-monitor", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "+monochrome", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-monochrome", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "+morph", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-morph", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+morphology", 2L, DeprecateOptionFlag, MagickFalse },
    { "-morphology", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "+mosaic", 0L, ListOperatorOptionFlag | FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-mosaic", 0L, ListOperatorOptionFlag | FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+motion-blur", 1L, DeprecateOptionFlag, MagickFalse },
    { "-motion-blur", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+name", 0L, NonConvertOptionFlag, MagickFalse },
    { "-name", 1L, NonConvertOptionFlag, MagickFalse },
    { "+negate", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-negate", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+noise", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-noise", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+noop", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "-noop", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "+normalize", 0L, DeprecateOptionFlag, MagickFalse },
    { "-normalize", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+opaque", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-opaque", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+ordered-dither", 0L, DeprecateOptionFlag, MagickFalse },
    { "-ordered-dither", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+orient", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-orient", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+origin", 0L, DeprecateOptionFlag, MagickFalse },
    { "-origin", 1L, DeprecateOptionFlag, MagickFalse },
    { "+page", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-page", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+paint", 0L, DeprecateOptionFlag, MagickFalse },
    { "-paint", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+path", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "-path", 1L, SpecialOperatorOptionFlag, MagickFalse },
    { "+pause", 0L, NonConvertOptionFlag, MagickFalse },
    { "-pause", 1L, NonConvertOptionFlag, MagickFalse },
    { "+passphrase", 0L, DeprecateOptionFlag, MagickFalse },
    { "-passphrase", 1L, DeprecateOptionFlag, MagickFalse },
    { "+pen", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-pen", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+ping", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-ping", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+poly", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-poly", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+pointsize", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-pointsize", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+polaroid", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-polaroid", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+posterize", 1L, DeprecateOptionFlag, MagickFalse },
    { "-posterize", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+preview", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-preview", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+print", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-print", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+process", 1L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-process", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+profile", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-profile", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+quality", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-quality", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+quantize", 0L, QuantizeInfoOptionFlag, MagickFalse },
    { "-quantize", 1L, QuantizeInfoOptionFlag, MagickFalse },
    { "+quiet", 0L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "-quiet", 0L, GlobalOptionFlag | FireOptionFlag, MagickFalse },
    { "+radial-blur", 1L, DeprecateOptionFlag, MagickFalse },
    { "-radial-blur", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+raise", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-raise", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+random-threshold", 1L, DeprecateOptionFlag, MagickFalse },
    { "-random-threshold", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+recolor", 1L, DeprecateOptionFlag, MagickFalse },
    { "-recolor", 1L, SimpleOperatorOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+red-primary", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-red-primary", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+regard-warnings", 0L, GenesisOptionFlag, MagickFalse },
    { "-regard-warnings", 0L, GenesisOptionFlag, MagickFalse },
    { "+region", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "-region", 1L, SpecialOperatorOptionFlag, MagickFalse },
    { "+remote", 0L, NonConvertOptionFlag, MagickFalse },
    { "-remote", 1L, NonConvertOptionFlag, MagickFalse },
    { "+render", 0L, DrawInfoOptionFlag, MagickFalse },
    { "-render", 0L, DrawInfoOptionFlag, MagickFalse },
    { "+remap", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-remap", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+repage", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-repage", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+resample", 1L, DeprecateOptionFlag, MagickFalse },
    { "-resample", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+resize", 1L, DeprecateOptionFlag, MagickFalse },
    { "-resize", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+respect-parenthesis", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "-respect-parenthesis", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "+reverse", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-reverse", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+roll", 1L, DeprecateOptionFlag, MagickFalse },
    { "-roll", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+rotate", 1L, DeprecateOptionFlag, MagickFalse },
    { "-rotate", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+sample", 1L, DeprecateOptionFlag, MagickFalse },
    { "-sample", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+sampling-factor", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-sampling-factor", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+sans", 1L, NonConvertOptionFlag, MagickTrue },
    { "-sans", 1L, NonConvertOptionFlag, MagickTrue },
    { "+sans0", 0L, NonConvertOptionFlag, MagickTrue },
    { "-sans0", 0L, NonConvertOptionFlag, MagickTrue },
    { "+sans2", 2L, NonConvertOptionFlag, MagickTrue },
    { "-sans2", 2L, NonConvertOptionFlag, MagickTrue },
    { "+scale", 1L, DeprecateOptionFlag, MagickFalse },
    { "-scale", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+scene", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-scene", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+scenes", 0L, NonConvertOptionFlag, MagickFalse },
    { "-scenes", 1L, NonConvertOptionFlag, MagickFalse },
    { "+screen", 0L, NonConvertOptionFlag, MagickFalse },
    { "-screen", 1L, NonConvertOptionFlag, MagickFalse },
    { "+seed", 0L, GlobalOptionFlag, MagickFalse },
    { "-seed", 1L, GlobalOptionFlag, MagickFalse },
    { "+segment", 1L, DeprecateOptionFlag, MagickFalse },
    { "-segment", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+selective-blur", 1L, DeprecateOptionFlag, MagickFalse },
    { "-selective-blur", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+separate", 0L, DeprecateOptionFlag | FireOptionFlag, MagickFalse },
    { "-separate", 0L, SimpleOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+sepia-tone", 1L, DeprecateOptionFlag, MagickFalse },
    { "-sepia-tone", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+set", 1L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "-set", 2L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "+shade", 0L, DeprecateOptionFlag, MagickFalse },
    { "-shade", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+shadow", 1L, DeprecateOptionFlag, MagickFalse },
    { "-shadow", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+shared-memory", 0L, NonConvertOptionFlag, MagickFalse },
    { "-shared-memory", 1L, NonConvertOptionFlag, MagickFalse },
    { "+sharpen", 1L, DeprecateOptionFlag, MagickFalse },
    { "-sharpen", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+shave", 1L, DeprecateOptionFlag, MagickFalse },
    { "-shave", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+shear", 1L, DeprecateOptionFlag, MagickFalse },
    { "-shear", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+sigmoidal-contrast", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-sigmoidal-contrast", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+silent", 0L, NonConvertOptionFlag, MagickFalse },
    { "-silent", 1L, NonConvertOptionFlag, MagickFalse },
    { "+similarity-threshold", 0L, NonConvertOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "-similarity-threshold", 1L, NonConvertOptionFlag | ImageInfoOptionFlag, MagickFalse },
    { "+size", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-size", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+sketch", 1L, DeprecateOptionFlag, MagickFalse },
    { "-sketch", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+smush", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-smush", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+snaps", 0L, NonConvertOptionFlag, MagickFalse },
    { "-snaps", 1L, NonConvertOptionFlag, MagickFalse },
    { "+solarize", 1L, DeprecateOptionFlag, MagickFalse },
    { "-solarize", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+sparse-color", 2L, DeprecateOptionFlag, MagickFalse },
    { "-sparse-color", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "+splice", 1L, DeprecateOptionFlag, MagickFalse },
    { "-splice", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+spread", 1L, DeprecateOptionFlag, MagickFalse },
    { "-spread", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+statistic", 2L, DeprecateOptionFlag, MagickFalse },
    { "-statistic", 2L, SimpleOperatorOptionFlag, MagickFalse },
    { "+stegano", 0L, NonConvertOptionFlag, MagickFalse },
    { "-stegano", 1L, NonConvertOptionFlag, MagickFalse },
    { "+stereo", 0L, DeprecateOptionFlag, MagickFalse },
    { "-stereo", 1L, NonConvertOptionFlag, MagickFalse },
    { "+stretch", 1L, DeprecateOptionFlag, MagickFalse },
    { "-stretch", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+strip", 0L, DeprecateOptionFlag, MagickFalse },
    { "-strip", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+stroke", 0L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "-stroke", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+strokewidth", 1L, ImageInfoOptionFlag, MagickFalse },
    { "-strokewidth", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+style", 0L, DrawInfoOptionFlag, MagickFalse },
    { "-style", 1L, DrawInfoOptionFlag, MagickFalse },
    { "+subimage-search", 0L, NonConvertOptionFlag, MagickFalse },
    { "-subimage-search", 0L, NonConvertOptionFlag, MagickFalse },
    { "+swap", 0L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-swap", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "+swirl", 1L, DeprecateOptionFlag, MagickFalse },
    { "-swirl", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+synchronize", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-synchronize", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+taint", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-taint", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+text-font", 0L, NonConvertOptionFlag, MagickFalse },
    { "-text-font", 1L, NonConvertOptionFlag, MagickFalse },
    { "+texture", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-texture", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+threshold", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "-threshold", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+thumbnail", 1L, DeprecateOptionFlag, MagickFalse },
    { "-thumbnail", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+tile", 0L, DrawInfoOptionFlag, MagickFalse },
    { "-tile", 1L, DrawInfoOptionFlag, MagickFalse },
    { "+tile-offset", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-tile-offset", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+tint", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-tint", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+title", 0L, NonConvertOptionFlag, MagickFalse },
    { "-title", 1L, NonConvertOptionFlag, MagickFalse },
    { "+transform", 0L, DeprecateOptionFlag, MagickFalse },
    { "-transform", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+transparent", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "-transparent", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+transparent-color", 1L, ImageInfoOptionFlag, MagickFalse },
    { "-transparent-color", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+transpose", 0L, DeprecateOptionFlag, MagickFalse },
    { "-transpose", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+transverse", 0L, DeprecateOptionFlag, MagickFalse },
    { "-transverse", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+treedepth", 1L, DeprecateOptionFlag, MagickFalse },
    { "-treedepth", 1L, QuantizeInfoOptionFlag, MagickFalse },
    { "+trim", 0L, DeprecateOptionFlag, MagickFalse },
    { "-trim", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+type", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "-type", 1L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "+undercolor", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-undercolor", 1L, ImageInfoOptionFlag | DrawInfoOptionFlag, MagickFalse },
    { "+unique", 0L, DeprecateOptionFlag, MagickFalse },
    { "-unique", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+unique-colors", 0L, DeprecateOptionFlag, MagickFalse },
    { "-unique-colors", 0L, SimpleOperatorOptionFlag, MagickFalse },
    { "+units", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-units", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+unsharp", 1L, DeprecateOptionFlag, MagickFalse },
    { "-unsharp", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+update", 0L, NonConvertOptionFlag, MagickFalse },
    { "-update", 1L, NonConvertOptionFlag, MagickFalse },
    { "+use-pixmap", 0L, NonConvertOptionFlag, MagickFalse },
    { "-use-pixmap", 1L, NonConvertOptionFlag, MagickFalse },
    { "+verbose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-verbose", 0L, ImageInfoOptionFlag, MagickFalse },
    { "+version", 0L, SpecialOperatorOptionFlag, MagickFalse },
    { "-version", 1L, SpecialOperatorOptionFlag, MagickFalse },
    { "+view", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-view", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+vignette", 1L, DeprecateOptionFlag, MagickFalse },
    { "-vignette", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+virtual-pixel", 0L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "-virtual-pixel", 1L, ImageInfoOptionFlag | SimpleOperatorOptionFlag, MagickFalse },
    { "+visual", 0L, NonConvertOptionFlag, MagickFalse },
    { "-visual", 1L, NonConvertOptionFlag, MagickFalse },
    { "+watermark", 0L, NonConvertOptionFlag, MagickFalse },
    { "-watermark", 1L, NonConvertOptionFlag, MagickFalse },
    { "+wave", 1L, DeprecateOptionFlag, MagickFalse },
    { "-wave", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+weight", 1L, DeprecateOptionFlag, MagickFalse },
    { "-weight", 1L, DrawInfoOptionFlag, MagickFalse },
    { "+white-point", 0L, ImageInfoOptionFlag, MagickFalse },
    { "-white-point", 1L, ImageInfoOptionFlag, MagickFalse },
    { "+white-threshold", 1L, DeprecateOptionFlag, MagickFalse },
    { "-white-threshold", 1L, SimpleOperatorOptionFlag, MagickFalse },
    { "+window", 0L, NonConvertOptionFlag, MagickFalse },
    { "-window", 1L, NonConvertOptionFlag, MagickFalse },
    { "+window-group", 0L, NonConvertOptionFlag, MagickFalse },
    { "-window-group", 1L, NonConvertOptionFlag, MagickFalse },
    { "+write", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { "-write", 1L, ListOperatorOptionFlag | FireOptionFlag, MagickFalse },
    { (char *) NULL, 0L, UndefinedOptionFlag, MagickFalse }
  },
  ComposeOptions[] =
  {
    { "Undefined", UndefinedCompositeOp, UndefinedOptionFlag, MagickTrue },
    { "Atop", AtopCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Blend", BlendCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Blur", BlurCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Bumpmap", BumpmapCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ChangeMask", ChangeMaskCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Clear", ClearCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ColorBurn", ColorBurnCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ColorDodge", ColorDodgeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Colorize", ColorizeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyBlack", CopyBlackCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyBlue", CopyBlueCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyCyan", CopyCyanCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyGreen", CopyGreenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Copy", CopyCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyMagenta", CopyMagentaCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyOpacity", CopyOpacityCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyRed", CopyRedCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "CopyYellow", CopyYellowCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Darken", DarkenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DarkenIntensity", DarkenIntensityCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DivideDst", DivideDstCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DivideSrc", DivideSrcCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Dst", DstCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Difference", DifferenceCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Displace", DisplaceCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Dissolve", DissolveCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Distort", DistortCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstAtop", DstAtopCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstIn", DstInCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstOut", DstOutCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "DstOver", DstOverCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Exclusion", ExclusionCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "HardLight", HardLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Hue", HueCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "In", InCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Lighten", LightenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LightenIntensity", LightenIntensityCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LinearBurn", LinearBurnCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LinearDodge", LinearDodgeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "LinearLight", LinearLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Luminize", LuminizeCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Mathematics", MathematicsCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "MinusDst", MinusDstCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "MinusSrc", MinusSrcCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Modulate", ModulateCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ModulusAdd", ModulusAddCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "ModulusSubtract", ModulusSubtractCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Multiply", MultiplyCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "None", NoCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Out", OutCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Overlay", OverlayCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Over", OverCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "PegtopLight", PegtopLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "PinLight", PinLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Plus", PlusCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Replace", ReplaceCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Saturate", SaturateCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Screen", ScreenCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SoftLight", SoftLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Src", SrcCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcAtop", SrcAtopCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcIn", SrcInCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcOut", SrcOutCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "SrcOver", SrcOverCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "VividLight", VividLightCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Xor", XorCompositeOp, UndefinedOptionFlag, MagickFalse },
    { "Add", AddCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "Divide", DivideDstCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "Minus", MinusDstCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "Subtract", SubtractCompositeOp, DeprecateOptionFlag, MagickTrue },
    { "Threshold", ThresholdCompositeOp, DeprecateOptionFlag, MagickTrue },
    { (char *) NULL, UndefinedCompositeOp, UndefinedOptionFlag, MagickFalse }
  },
  CompressOptions[] =
  {
    { "Undefined", UndefinedCompression, UndefinedOptionFlag, MagickTrue },
    { "B44", B44Compression, UndefinedOptionFlag, MagickFalse },
    { "B44A", B44ACompression, UndefinedOptionFlag, MagickFalse },
    { "BZip", BZipCompression, UndefinedOptionFlag, MagickFalse },
    { "DXT1", DXT1Compression, UndefinedOptionFlag, MagickFalse },
    { "DXT3", DXT3Compression, UndefinedOptionFlag, MagickFalse },
    { "DXT5", DXT5Compression, UndefinedOptionFlag, MagickFalse },
    { "Fax", FaxCompression, UndefinedOptionFlag, MagickFalse },
    { "Group4", Group4Compression, UndefinedOptionFlag, MagickFalse },
    { "JBIG1", JBIG1Compression, UndefinedOptionFlag, MagickFalse },
    { "JBIG2", JBIG2Compression, UndefinedOptionFlag, MagickFalse },
    { "JPEG", JPEGCompression, UndefinedOptionFlag, MagickFalse },
    { "JPEG2000", JPEG2000Compression, UndefinedOptionFlag, MagickFalse },
    { "Lossless", LosslessJPEGCompression, UndefinedOptionFlag, MagickFalse },
    { "LosslessJPEG", LosslessJPEGCompression, UndefinedOptionFlag, MagickFalse },
    { "LZMA", LZMACompression, UndefinedOptionFlag, MagickFalse },
    { "LZW", LZWCompression, UndefinedOptionFlag, MagickFalse },
    { "None", NoCompression, UndefinedOptionFlag, MagickFalse },
    { "Piz", PizCompression, UndefinedOptionFlag, MagickFalse },
    { "Pxr24", Pxr24Compression, UndefinedOptionFlag, MagickFalse },
    { "RLE", RLECompression, UndefinedOptionFlag, MagickFalse },
    { "Zip", ZipCompression, UndefinedOptionFlag, MagickFalse },
    { "RunlengthEncoded", RLECompression, UndefinedOptionFlag, MagickFalse },
    { "ZipS", ZipSCompression, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedCompression, UndefinedOptionFlag, MagickFalse }
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
    { "Lab", LabColorspace, UndefinedOptionFlag, MagickFalse },
    { "LCH", LCHColorspace, UndefinedOptionFlag, MagickFalse },
    { "LCHab", LCHabColorspace, UndefinedOptionFlag, MagickFalse },
    { "LCHuv", LCHuvColorspace, UndefinedOptionFlag, MagickFalse },
    { "LMS", LMSColorspace, UndefinedOptionFlag, MagickFalse },
    { "Log", LogColorspace, UndefinedOptionFlag, MagickFalse },
    { "Luv", LuvColorspace, UndefinedOptionFlag, MagickFalse },
    { "OHTA", OHTAColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec601Luma", Rec601LumaColorspace, DeprecateOptionFlag, MagickFalse },
    { "Rec601YCbCr", Rec601YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec709Luma", Rec709LumaColorspace, DeprecateOptionFlag, MagickFalse },
    { "Rec709YCbCr", Rec709YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "RGB", RGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "scRGB", scRGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "sRGB", sRGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "Transparent", TransparentColorspace, UndefinedOptionFlag, MagickFalse },
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
    { "Undefined", UndefinedDispose, UndefinedOptionFlag, MagickTrue },
    { "Background", BackgroundDispose, UndefinedOptionFlag, MagickFalse },
    { "None", NoneDispose, UndefinedOptionFlag, MagickFalse },
    { "Previous", PreviousDispose, UndefinedOptionFlag, MagickFalse },
    { "Undefined", UndefinedDispose, UndefinedOptionFlag, MagickFalse },
    { "0", UndefinedDispose, UndefinedOptionFlag, MagickFalse },
    { "1", NoneDispose, UndefinedOptionFlag, MagickFalse },
    { "2", BackgroundDispose, UndefinedOptionFlag, MagickFalse },
    { "3", PreviousDispose, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedDispose, UndefinedOptionFlag, MagickFalse }
  },
  DistortOptions[] =
  {
    { "Undefined", UndefinedDistortion, UndefinedOptionFlag, MagickTrue },
    { "Affine", AffineDistortion, UndefinedOptionFlag, MagickFalse },
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
    { "Hann", HanningFilter, UndefinedOptionFlag, MagickFalse },
    { "Hanning", HanningFilter, UndefinedOptionFlag, MagickTrue }, /*misspell*/
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
    { "Triangle", TriangleFilter, UndefinedOptionFlag, MagickFalse },
    { "Welch", WelshFilter, UndefinedOptionFlag, MagickFalse },
    { "Welsh", WelshFilter, UndefinedOptionFlag, MagickTrue }, /*misspell*/
    /* For backward compatibility - must be after "Jinc" */
    { "Bessel", JincFilter, UndefinedOptionFlag, MagickTrue },
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
    { "Static", StaticGravity, UndefinedOptionFlag, MagickTrue },
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
    { "Bicubic", CatromInterpolatePixel, UndefinedOptionFlag, MagickTrue },
    { "Bilinear", BilinearInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Blend", BlendInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Catrom", CatromInterpolatePixel, UndefinedOptionFlag, MagickTrue },
    { "Integer", IntegerInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Mesh", MeshInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Nearest", NearestNeighborInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "NearestNeighbor", NearestNeighborInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Spline", SplineInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    /* depreciation of slow and useless interpolation method */
    { "Filter", FilterInterpolatePixel, UndefinedOptionFlag, MagickTrue },
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
    { "ThinDiagonals", DiagonalsKernel, DeprecateOptionFlag, MagickTrue },
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
    { "Alpha", MagickAlphaOptions, UndefinedOptionFlag, MagickFalse },
    { "Boolean", MagickBooleanOptions, UndefinedOptionFlag, MagickFalse },
    { "Cache", MagickCacheOptions, UndefinedOptionFlag, MagickFalse },
    { "Channel", MagickChannelOptions, UndefinedOptionFlag, MagickFalse },
    { "Class", MagickClassOptions, UndefinedOptionFlag, MagickFalse },
    { "ClipPath", MagickClipPathOptions, UndefinedOptionFlag, MagickFalse },
    { "Coder", MagickCoderOptions, UndefinedOptionFlag, MagickFalse },
    { "Color", MagickColorOptions, UndefinedOptionFlag, MagickFalse },
    { "Colorspace", MagickColorspaceOptions, UndefinedOptionFlag, MagickFalse },
    { "Command", MagickCommandOptions, UndefinedOptionFlag, MagickFalse },
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
    { "PixelIntensity", MagickPixelIntensityOptions, UndefinedOptionFlag, MagickFalse },
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
    { "Type", MagickTypeOptions, UndefinedOptionFlag, MagickFalse },
    { "Units", MagickResolutionOptions, UndefinedOptionFlag, MagickFalse },
    { "Undefined", MagickUndefinedOptions, UndefinedOptionFlag, MagickTrue },
    { "Validate", MagickValidateOptions, UndefinedOptionFlag, MagickFalse },
    { "VirtualPixel", MagickVirtualPixelOptions, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, MagickUndefinedOptions, UndefinedOptionFlag, MagickFalse }
  },
  LogEventOptions[] =
  {
    { "Undefined", UndefinedEvents, UndefinedOptionFlag, MagickTrue },
    { "All", (AllEvents &~ TraceEvent), UndefinedOptionFlag, MagickFalse },
    { "Annotate", AnnotateEvent, UndefinedOptionFlag, MagickFalse },
    { "Accelerate", AccelerateEvent, UndefinedOptionFlag, MagickFalse },
    { "Blob", BlobEvent, UndefinedOptionFlag, MagickFalse },
    { "Cache", CacheEvent, UndefinedOptionFlag, MagickFalse },
    { "Coder", CoderEvent, UndefinedOptionFlag, MagickFalse },
    { "Configure", ConfigureEvent, UndefinedOptionFlag, MagickFalse },
    { "Deprecate", DeprecateEvent, UndefinedOptionFlag, MagickFalse },
    { "Draw", DrawEvent, UndefinedOptionFlag, MagickFalse },
    { "Exception", ExceptionEvent, UndefinedOptionFlag, MagickFalse },
    { "Locale", LocaleEvent, UndefinedOptionFlag, MagickFalse },
    { "Module", ModuleEvent, UndefinedOptionFlag, MagickFalse },
    { "None", NoEvents, UndefinedOptionFlag, MagickFalse },
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
    { "Fuzz", FuzzErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "MAE", MeanAbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "MEPP", MeanErrorPerPixelMetric, UndefinedOptionFlag, MagickFalse },
    { "MSE", MeanSquaredErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "NCC", NormalizedCrossCorrelationErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PAE", PeakAbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PHASH", PerceptualHashErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PSNR", PeakSignalToNoiseRatioMetric, UndefinedOptionFlag, MagickFalse },
    { "RMSE", RootMeanSquaredErrorMetric, UndefinedOptionFlag, MagickFalse },
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
  PolicyDomainOptions[] =
  {
    { "Undefined", UndefinedPolicyDomain, UndefinedOptionFlag, MagickTrue },
    { "Coder", CoderPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Delegate", DelegatePolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Filter", FilterPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Path", PathPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "Resource", ResourcePolicyDomain, UndefinedOptionFlag, MagickFalse },
    { "System", SystemPolicyDomain, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPolicyDomain, UndefinedOptionFlag, MagickFalse }
  },
  PolicyRightsOptions[] =
  {
    { "Undefined", UndefinedPolicyRights, UndefinedOptionFlag, MagickTrue },
    { "None", NoPolicyRights, UndefinedOptionFlag, MagickFalse },
    { "Read", ReadPolicyRights, UndefinedOptionFlag, MagickFalse },
    { "Write", WritePolicyRights, UndefinedOptionFlag, MagickFalse },
    { "Execute", ExecutePolicyRights, UndefinedOptionFlag, MagickFalse },
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
    { "Arc", ArcPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Bezier", BezierPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Circle", CirclePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Color", ColorPrimitive, UndefinedOptionFlag, MagickFalse },
    { "Ellipse", EllipsePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Image", ImagePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Line", LinePrimitive, UndefinedOptionFlag, MagickFalse },
    { "Matte", MattePrimitive, UndefinedOptionFlag, MagickFalse },
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
  PixelIntensityOptions[] =
  {
    { "Undefined", UndefinedPixelIntensityMethod, UndefinedOptionFlag, MagickTrue },
    { "Average", AveragePixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Brightness", BrightnessPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Lightness", LightnessPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "MS", MSPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec601Luma", Rec601LumaPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec601Luminance", Rec601LuminancePixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec709Luma", Rec709LumaPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "Rec709Luminance", Rec709LuminancePixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { "RMS", RMSPixelIntensityMethod, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedPixelIntensityMethod, UndefinedOptionFlag, MagickFalse }
  },
  ResolutionOptions[] =
  {
    { "Undefined", UndefinedResolution, UndefinedOptionFlag, MagickTrue },
    { "PixelsPerInch", PixelsPerInchResolution, UndefinedOptionFlag, MagickFalse },
    { "PixelsPerCentimeter", PixelsPerCentimeterResolution, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedResolution, UndefinedOptionFlag, MagickFalse }
  },
  ResourceOptions[] =
  {
    { "Undefined", UndefinedResource, UndefinedOptionFlag, MagickTrue },
    { "Area", AreaResource, UndefinedOptionFlag, MagickFalse },
    { "Disk", DiskResource, UndefinedOptionFlag, MagickFalse },
    { "File", FileResource, UndefinedOptionFlag, MagickFalse },
    { "Map", MapResource, UndefinedOptionFlag, MagickFalse },
    { "Memory", MemoryResource, UndefinedOptionFlag, MagickFalse },
    { "Thread", ThreadResource, UndefinedOptionFlag, MagickFalse },
    { "Time", TimeResource, UndefinedOptionFlag, MagickFalse },
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
    { "Nonpeak", NonpeakStatistic, UndefinedOptionFlag, MagickFalse },
    { "StandardDeviation", StandardDeviationStatistic, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedMethod, UndefinedOptionFlag, MagickFalse }
  },
  StorageOptions[] =
  {
    { "Undefined", UndefinedPixel, UndefinedOptionFlag, MagickTrue },
    { "Char", CharPixel, UndefinedOptionFlag, MagickFalse },
    { "Double", DoublePixel, UndefinedOptionFlag, MagickFalse },
    { "Float", FloatPixel, UndefinedOptionFlag, MagickFalse },
    { "Integer", IntegerPixel, UndefinedOptionFlag, MagickFalse },
    { "Long", LongPixel, UndefinedOptionFlag, MagickFalse },
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
    { "Italic", ItalicStyle, UndefinedOptionFlag, MagickFalse },
    { "Normal", NormalStyle, UndefinedOptionFlag, MagickFalse },
    { "Oblique", ObliqueStyle, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedStyle, UndefinedOptionFlag, MagickFalse }
  },
  TypeOptions[] =
  {
    { "Undefined", UndefinedType, UndefinedOptionFlag, MagickTrue },
    { "Bilevel", BilevelType, UndefinedOptionFlag, MagickFalse },
    { "ColorSeparation", ColorSeparationType, UndefinedOptionFlag, MagickFalse },
    { "ColorSeparationAlpha", ColorSeparationMatteType, UndefinedOptionFlag, MagickFalse },
    { "ColorSeparationMatte", ColorSeparationMatteType, UndefinedOptionFlag, MagickFalse },
    { "Grayscale", GrayscaleType, UndefinedOptionFlag, MagickFalse },
    { "GrayscaleAlpha", GrayscaleMatteType, UndefinedOptionFlag, MagickFalse },
    { "GrayscaleMatte", GrayscaleMatteType, UndefinedOptionFlag, MagickFalse },
    { "Optimize", OptimizeType, UndefinedOptionFlag, MagickFalse },
    { "Palette", PaletteType, UndefinedOptionFlag, MagickFalse },
    { "PaletteBilevelAlpha", PaletteBilevelMatteType, UndefinedOptionFlag, MagickFalse },
    { "PaletteBilevelMatte", PaletteBilevelMatteType, UndefinedOptionFlag, MagickFalse },
    { "PaletteAlpha", PaletteMatteType, UndefinedOptionFlag, MagickFalse },
    { "PaletteMatte", PaletteMatteType, UndefinedOptionFlag, MagickFalse },
    { "TrueColorAlpha", TrueColorMatteType, UndefinedOptionFlag, MagickFalse },
    { "TrueColorMatte", TrueColorMatteType, UndefinedOptionFlag, MagickFalse },
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
%  CloneImageOptions() clones one or more image options.
%
%  The format of the CloneImageOptions method is:
%
%      MagickBooleanType CloneImageOptions(ImageInfo *image_info,
%        const ImageInfo *clone_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o clone_info: the clone image info.
%
*/
MagickExport MagickBooleanType CloneImageOptions(ImageInfo *image_info,
  const ImageInfo *clone_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(clone_info != (const ImageInfo *) NULL);
  assert(clone_info->signature == MagickSignature);
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
%  DefineImageOption() associates a key/value pair with an image option.
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
%    o option: the image option.
%
*/
MagickExport MagickBooleanType DefineImageOption(ImageInfo *image_info,
  const char *option)
{
  char
    key[MaxTextExtent],
    value[MaxTextExtent];

  register char
    *p;

  assert(image_info != (ImageInfo *) NULL);
  assert(option != (const char *) NULL);
  (void) CopyMagickString(key,option,MaxTextExtent);
  for (p=key; *p != '\0'; p++)
    if (*p == '=')
      break;
  *value='\0';
  if (*p == '=')
    (void) CopyMagickString(value,p+1,MaxTextExtent);
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
%  DeleteImageOption() deletes an key from the image map.
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
  assert(image_info->signature == MagickSignature);
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
%  DestroyImageOptions() releases memory associated with image option values.
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
  assert(image_info->signature == MagickSignature);
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
%  GetImageOption() gets a value associated with an image option.
%
%  The format of the GetImageOption method is:
%
%      const char *GetImageOption(const ImageInfo *image_info,
%        const char *key)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o key: the key.
%
*/
MagickExport const char *GetImageOption(const ImageInfo *image_info,
  const char *key)
{
  const char
    *option;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->options == (void *) NULL)
    return((const char *) NULL);
  option=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
    image_info->options,key);
  return(option);
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
    case MagickAlphaOptions: return(AlphaOptions);
    case MagickBooleanOptions: return(BooleanOptions);
    case MagickCacheOptions: return(CacheOptions);
    case MagickChannelOptions: return(ChannelOptions);
    case MagickClassOptions: return(ClassOptions);
    case MagickClipPathOptions: return(ClipPathOptions);
    case MagickColorspaceOptions: return(ColorspaceOptions);
    case MagickCommandOptions: return(CommandOptions);
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
    case MagickGravityOptions: return(GravityOptions);
    case MagickIntensityOptions: return(PixelIntensityOptions);
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
    case MagickPixelIntensityOptions: return(PixelIntensityOptions);
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
    case MagickStyleOptions: return(StyleOptions);
    case MagickTypeOptions: return(TypeOptions);
    case MagickValidateOptions: return(ValidateOptions);
    case MagickVirtualPixelOptions: return(VirtualPixelOptions);
    default: break;
  }
  return((const OptionInfo *) NULL);
}

MagickExport ssize_t GetCommandOptionFlags(const CommandOption option,
  const MagickBooleanType list,const char *options)
{
  char
    token[MaxTextExtent];

  const OptionInfo
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
      if ((q-token) >= (MaxTextExtent-1))
        break;
      *q++=(*p++);
    }
    *q='\0';
    for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
      if (LocaleCompare(token,option_info[i].mnemonic) == 0)
        {
          if (*token == '!')
            option_types=option_types &~ option_info[i].flags;
          else
            option_types=option_types | option_info[i].flags;
          break;
        }
    if ((option_info[i].mnemonic == (char *) NULL) &&
        ((strchr(token+1,'-') != (char *) NULL) ||
         (strchr(token+1,'_') != (char *) NULL)))
      {
        while ((q=strchr(token+1,'-')) != (char *) NULL)
          (void) CopyMagickString(q,q+1,MaxTextExtent-strlen(q));
        while ((q=strchr(token+1,'_')) != (char *) NULL)
          (void) CopyMagickString(q,q+1,MaxTextExtent-strlen(q));
        for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
          if (LocaleCompare(token,option_info[i].mnemonic) == 0)
            {
              if (*token == '!')
                option_types=option_types &~ option_info[i].flags;
              else
                option_types=option_types | option_info[i].flags;
              break;
            }
      }
    if (option_info[i].mnemonic == (char *) NULL)
      return(-1);
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
%   G e t C o m m a n d O p t i o n s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCommandOptions() returns a list of values.
%
%  The format of the GetCommandOptions method is:
%
%      const char **GetCommandOptions(const CommandOption value)
%
%  A description of each parameter follows:
%
%    o value: the value.
%
*/
MagickExport char **GetCommandOptions(const CommandOption value)
{
  char
    **values;

  const OptionInfo
    *option_info;

  register ssize_t
    i;

  option_info=GetOptionInfo(value);
  if (option_info == (const OptionInfo *) NULL)
    return((char **) NULL);
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++) ;
  values=(char **) AcquireQuantumMemory((size_t) i+1UL,sizeof(*values));
  if (values == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++)
    values[i]=AcquireString(option_info[i].mnemonic);
  values[i]=(char *) NULL;
  return(values);
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
%  GetNextImageOption() gets the next image option value.
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
  assert(image_info->signature == MagickSignature);
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
    return(MagickFalse);
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
    return("undefined");
  return(option_info[i].mnemonic);
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

  ssize_t
    channel;

  channel=ParseCommandOption(MagickChannelOptions,MagickTrue,channels);
  if (channel >= 0)
    return(channel);
  channel=0;
  for (i=0; i < (ssize_t) strlen(channels); i++)
  {
    switch (channels[i])
    {
      case 'A':
      case 'a':
      {
        channel|=OpacityChannel;
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
      case 'I':
      case 'i':
      {
        channel|=IndexChannel;
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
        channel|=OpacityChannel;
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
    token[MaxTextExtent];

  const OptionInfo
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
      if ((q-token) >= (MaxTextExtent-1))
        break;
      *q++=(*p++);
    }
    *q='\0';
    for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
      if (LocaleCompare(token,option_info[i].mnemonic) == 0)
        {
          if (*token == '!')
            option_types=option_types &~ option_info[i].type;
          else
            option_types=option_types | option_info[i].type;
          break;
        }
    if ((option_info[i].mnemonic == (char *) NULL) &&
        ((strchr(token+1,'-') != (char *) NULL) ||
         (strchr(token+1,'_') != (char *) NULL)))
      {
        while ((q=strchr(token+1,'-')) != (char *) NULL)
          (void) CopyMagickString(q,q+1,MaxTextExtent-strlen(q));
        while ((q=strchr(token+1,'_')) != (char *) NULL)
          (void) CopyMagickString(q,q+1,MaxTextExtent-strlen(q));
        for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
          if (LocaleCompare(token,option_info[i].mnemonic) == 0)
            {
              if (*token == '!')
                option_types=option_types &~ option_info[i].type;
              else
                option_types=option_types | option_info[i].type;
              break;
            }
      }
    if (option_info[i].mnemonic == (char *) NULL)
      return(-1);
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
%   R e m o v e I m a g e O p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveImageOption() removes an option from the image and returns its value.
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
  assert(image_info->signature == MagickSignature);
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
%  all options associated with the image_info structure.
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
  assert(image_info->signature == MagickSignature);
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
  assert(image_info->signature == MagickSignature);
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
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  /*
    FUTURE: This should not be here!
  */
  if (LocaleCompare(option,"size") == 0)
    (void) CloneString(&image_info->size,value);
  /*
    Create tree if needed - specify how key,values are to be freed.
  */
  if (image_info->options == (void *) NULL)
    image_info->options=NewSplayTree(CompareSplayTreeString,
      RelinquishMagickMemory,RelinquishMagickMemory);
  /*
    Delete Option if NULL.
  */
  if (value == (const char *) NULL)
    return(DeleteImageOption(image_info,option));
  /*
    Add option to splay-tree.
  */
  return(AddValueToSplayTree((SplayTreeInfo *) image_info->options,
    ConstantString(option),ConstantString(value)));
}

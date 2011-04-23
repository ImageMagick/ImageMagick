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
%                                John Cristy                                  %
%                                 March 2000                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
    { "Off", DeactivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "On", ActivateAlphaChannel, UndefinedOptionFlag, MagickFalse },
    { "Opaque", OpaqueAlphaChannel, UndefinedOptionFlag, MagickFalse },
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
  ChannelOptions[] =
  {
    { "Undefined", UndefinedChannel, UndefinedOptionFlag, MagickTrue },
    { "All", AllChannels, UndefinedOptionFlag, MagickFalse },
    { "Alpha", OpacityChannel, UndefinedOptionFlag, MagickFalse },
    { "Black", BlackChannel, UndefinedOptionFlag, MagickFalse },
    { "Blue", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Cyan", CyanChannel, UndefinedOptionFlag, MagickFalse },
    { "Default", DefaultChannels, UndefinedOptionFlag, MagickFalse },
    { "Gray", GrayChannel, UndefinedOptionFlag, MagickFalse },
    { "Green", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Hue", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "Index", IndexChannel, UndefinedOptionFlag, MagickFalse },
    { "Lightness", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Luminance", BlueChannel, UndefinedOptionFlag, MagickFalse },
    { "Luminosity", BlueChannel, DeprecateOptionFlag, MagickTrue },
    { "Magenta", MagentaChannel, UndefinedOptionFlag, MagickFalse },
    { "Matte", OpacityChannel, UndefinedOptionFlag, MagickFalse },
    { "Opacity", OpacityChannel, UndefinedOptionFlag, MagickFalse },
    { "Red", RedChannel, UndefinedOptionFlag, MagickFalse },
    { "Saturation", GreenChannel, UndefinedOptionFlag, MagickFalse },
    { "Yellow", YellowChannel, UndefinedOptionFlag, MagickFalse },
    { "Sync", SyncChannels, UndefinedOptionFlag, MagickFalse },
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
    { "+adjoin", 0L, UndefinedOptionFlag, MagickFalse },
    { "-adjoin", 0L, UndefinedOptionFlag, MagickFalse },
    { "+adaptive-sharpen", 1L, UndefinedOptionFlag, MagickFalse },
    { "-adaptive-sharpen", 1L, UndefinedOptionFlag, MagickFalse },
    { "+adaptive-threshold", 1L, UndefinedOptionFlag, MagickFalse },
    { "-adaptive-threshold", 1L, UndefinedOptionFlag, MagickFalse },
    { "+affine", 0L, UndefinedOptionFlag, MagickFalse },
    { "-affine", 1L, UndefinedOptionFlag, MagickFalse },
    { "+affinity", 0L, FireOptionFlag, MagickFalse },
    { "-affinity", 1L, FireOptionFlag, MagickFalse },
    { "+alpha", 1L, UndefinedOptionFlag, MagickFalse },
    { "-alpha", 1L, UndefinedOptionFlag, MagickFalse },
    { "+annotate", 0L, UndefinedOptionFlag, MagickFalse },
    { "-annotate", 2L, UndefinedOptionFlag, MagickFalse },
    { "+antialias", 0L, UndefinedOptionFlag, MagickFalse },
    { "-antialias", 0L, UndefinedOptionFlag, MagickFalse },
    { "+append", 0L, FireOptionFlag, MagickFalse },
    { "-append", 0L, FireOptionFlag, MagickFalse },
    { "+authenticate", 0L, UndefinedOptionFlag, MagickFalse },
    { "-authenticate", 1L, UndefinedOptionFlag, MagickFalse },
    { "+auto-gamma", 0L, UndefinedOptionFlag, MagickTrue },
    { "-auto-gamma", 0L, UndefinedOptionFlag, MagickTrue },
    { "+auto-level", 0L, UndefinedOptionFlag, MagickTrue },
    { "-auto-level", 0L, UndefinedOptionFlag, MagickTrue },
    { "+auto-orient", 0L, UndefinedOptionFlag, MagickFalse },
    { "-auto-orient", 0L, UndefinedOptionFlag, MagickFalse },
    { "+average", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-average", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+backdrop", 0L, UndefinedOptionFlag, MagickFalse },
    { "-backdrop", 1L, UndefinedOptionFlag, MagickFalse },
    { "+background", 0L, UndefinedOptionFlag, MagickFalse },
    { "-background", 1L, UndefinedOptionFlag, MagickFalse },
    { "+bench", 0L, UndefinedOptionFlag, MagickTrue },
    { "-bench", 1L, UndefinedOptionFlag, MagickTrue },
    { "+bias", 0L, UndefinedOptionFlag, MagickFalse },
    { "-bias", 1L, UndefinedOptionFlag, MagickFalse },
    { "+black-threshold", 0L, UndefinedOptionFlag, MagickFalse },
    { "-black-threshold", 1L, UndefinedOptionFlag, MagickFalse },
    { "+blend", 0L, UndefinedOptionFlag, MagickFalse },
    { "-blend", 1L, UndefinedOptionFlag, MagickFalse },
    { "+blue-primary", 0L, UndefinedOptionFlag, MagickFalse },
    { "-blue-primary", 1L, UndefinedOptionFlag, MagickFalse },
    { "+blue-shift", 1L, UndefinedOptionFlag, MagickFalse },
    { "-blue-shift", 1L, UndefinedOptionFlag, MagickFalse },
    { "+blur", 0L, UndefinedOptionFlag, MagickFalse },
    { "-blur", 1L, UndefinedOptionFlag, MagickFalse },
    { "+border", 0L, UndefinedOptionFlag, MagickFalse },
    { "-border", 1L, UndefinedOptionFlag, MagickFalse },
    { "+bordercolor", 0L, UndefinedOptionFlag, MagickFalse },
    { "-bordercolor", 1L, UndefinedOptionFlag, MagickFalse },
    { "+borderwidth", 0L, UndefinedOptionFlag, MagickFalse },
    { "-borderwidth", 1L, UndefinedOptionFlag, MagickFalse },
    { "+box", 0L, UndefinedOptionFlag, MagickFalse },
    { "-box", 1L, UndefinedOptionFlag, MagickFalse },
    { "+brightness-contrast", 0L, UndefinedOptionFlag, MagickFalse },
    { "-brightness-contrast", 1L, UndefinedOptionFlag, MagickFalse },
    { "+cache", 0L, UndefinedOptionFlag, MagickFalse },
    { "-cache", 1L, UndefinedOptionFlag, MagickFalse },
    { "+cdl", 1L, UndefinedOptionFlag, MagickFalse },
    { "-cdl", 1L, UndefinedOptionFlag, MagickFalse },
    { "+channel", 0L, UndefinedOptionFlag, MagickFalse },
    { "-channel", 1L, UndefinedOptionFlag, MagickFalse },
    { "+charcoal", 0L, UndefinedOptionFlag, MagickFalse },
    { "-charcoal", 0L, UndefinedOptionFlag, MagickFalse },
    { "+chop", 0L, UndefinedOptionFlag, MagickFalse },
    { "-chop", 1L, UndefinedOptionFlag, MagickFalse },
    { "+clip", 0L, UndefinedOptionFlag, MagickFalse },
    { "-clip", 0L, UndefinedOptionFlag, MagickFalse },
    { "+clip-mask", 0L, UndefinedOptionFlag, MagickFalse },
    { "-clip-mask", 1L, UndefinedOptionFlag, MagickFalse },
    { "+clip-path", 0L, UndefinedOptionFlag, MagickFalse },
    { "-clip-path", 1L, UndefinedOptionFlag, MagickFalse },
    { "+clone", 0L, UndefinedOptionFlag, MagickFalse },
    { "-clone", 1L, UndefinedOptionFlag, MagickFalse },
    { "+clut", 0L, FireOptionFlag, MagickFalse },
    { "-clut", 0L, FireOptionFlag, MagickFalse },
    { "+coalesce", 0L, FireOptionFlag, MagickFalse },
    { "-coalesce", 0L, FireOptionFlag, MagickFalse },
    { "+colorize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-colorize", 1L, UndefinedOptionFlag, MagickFalse },
    { "+colormap", 0L, UndefinedOptionFlag, MagickFalse },
    { "-colormap", 1L, UndefinedOptionFlag, MagickFalse },
    { "+color-matrix", 0L, UndefinedOptionFlag, MagickFalse },
    { "-color-matrix", 1L, UndefinedOptionFlag, MagickFalse },
    { "+colors", 0L, UndefinedOptionFlag, MagickFalse },
    { "-colors", 1L, UndefinedOptionFlag, MagickFalse },
    { "+colorspace", 0L, UndefinedOptionFlag, MagickFalse },
    { "-colorspace", 1L, UndefinedOptionFlag, MagickFalse },
    { "+combine", 0L, FireOptionFlag, MagickFalse },
    { "-combine", 0L, FireOptionFlag, MagickFalse },
    { "+comment", 0L, UndefinedOptionFlag, MagickFalse },
    { "-comment", 1L, UndefinedOptionFlag, MagickFalse },
    { "+compose", 0L, UndefinedOptionFlag, MagickFalse },
    { "-compose", 1L, UndefinedOptionFlag, MagickFalse },
    { "+composite", 0L, FireOptionFlag, MagickFalse },
    { "-composite", 0L, FireOptionFlag, MagickFalse },
    { "+compress", 0L, UndefinedOptionFlag, MagickFalse },
    { "-compress", 1L, UndefinedOptionFlag, MagickFalse },
    { "+concurrent", 0L, UndefinedOptionFlag, MagickTrue },
    { "-concurrent", 0L, UndefinedOptionFlag, MagickTrue },
    { "+contrast", 0L, UndefinedOptionFlag, MagickFalse },
    { "-contrast", 0L, UndefinedOptionFlag, MagickFalse },
    { "+contrast-stretch", 0L, UndefinedOptionFlag, MagickFalse },
    { "-contrast-stretch", 1L, UndefinedOptionFlag, MagickFalse },
    { "+convolve", 0L, UndefinedOptionFlag, MagickFalse },
    { "-convolve", 1L, UndefinedOptionFlag, MagickFalse },
    { "+crop", 0L, FireOptionFlag, MagickFalse },
    { "-crop", 1L, FireOptionFlag, MagickFalse },
    { "+cycle", 0L, UndefinedOptionFlag, MagickFalse },
    { "-cycle", 1L, UndefinedOptionFlag, MagickFalse },
    { "+debug", 0L, FireOptionFlag, MagickFalse },
    { "-debug", 1L, FireOptionFlag, MagickFalse },
    { "+decipher", 1L, UndefinedOptionFlag, MagickFalse },
    { "-decipher", 1L, UndefinedOptionFlag, MagickFalse },
    { "+deconstruct", 0L, FireOptionFlag, MagickFalse },
    { "-deconstruct", 0L, FireOptionFlag, MagickFalse },
    { "+define", 1L, UndefinedOptionFlag, MagickFalse },
    { "-define", 1L, UndefinedOptionFlag, MagickFalse },
    { "+delay", 0L, UndefinedOptionFlag, MagickFalse },
    { "-delay", 1L, UndefinedOptionFlag, MagickFalse },
    { "+delete", 0L, FireOptionFlag, MagickFalse },
    { "-delete", 1L, FireOptionFlag, MagickFalse },
    { "+density", 0L, UndefinedOptionFlag, MagickFalse },
    { "-density", 1L, UndefinedOptionFlag, MagickFalse },
    { "+depth", 0L, UndefinedOptionFlag, MagickFalse },
    { "-depth", 1L, UndefinedOptionFlag, MagickFalse },
    { "+descend", 0L, UndefinedOptionFlag, MagickFalse },
    { "-descend", 1L, UndefinedOptionFlag, MagickFalse },
    { "+deskew", 0L, UndefinedOptionFlag, MagickFalse },
    { "-deskew", 1L, UndefinedOptionFlag, MagickFalse },
    { "+despeckle", 0L, UndefinedOptionFlag, MagickFalse },
    { "-despeckle", 0L, UndefinedOptionFlag, MagickFalse },
    { "+direction", 0L, UndefinedOptionFlag, MagickFalse },
    { "-direction", 1L, UndefinedOptionFlag, MagickFalse },
    { "+displace", 0L, UndefinedOptionFlag, MagickFalse },
    { "-displace", 1L, UndefinedOptionFlag, MagickFalse },
    { "+display", 0L, UndefinedOptionFlag, MagickFalse },
    { "-display", 1L, UndefinedOptionFlag, MagickFalse },
    { "+dispose", 0L, UndefinedOptionFlag, MagickFalse },
    { "-dispose", 1L, UndefinedOptionFlag, MagickFalse },
    { "+dissolve", 0L, UndefinedOptionFlag, MagickFalse },
    { "-dissolve", 1L, UndefinedOptionFlag, MagickFalse },
    { "+distort", 2L, UndefinedOptionFlag, MagickFalse },
    { "-distort", 2L, UndefinedOptionFlag, MagickFalse },
    { "+dither", 0L, UndefinedOptionFlag, MagickFalse },
    { "-dither", 1L, UndefinedOptionFlag, MagickFalse },
    { "+draw", 0L, UndefinedOptionFlag, MagickFalse },
    { "-draw", 1L, UndefinedOptionFlag, MagickFalse },
    { "+duplicate", 0L, FireOptionFlag, MagickFalse },
    { "-duplicate", 1L, FireOptionFlag, MagickFalse },
    { "+duration", 1L, UndefinedOptionFlag, MagickFalse },
    { "-duration", 1L, UndefinedOptionFlag, MagickFalse },
    { "+edge", 0L, UndefinedOptionFlag, MagickFalse },
    { "-edge", 1L, UndefinedOptionFlag, MagickFalse },
    { "+emboss", 0L, UndefinedOptionFlag, MagickFalse },
    { "-emboss", 1L, UndefinedOptionFlag, MagickFalse },
    { "+encipher", 1L, UndefinedOptionFlag, MagickFalse },
    { "-encipher", 1L, UndefinedOptionFlag, MagickFalse },
    { "+encoding", 0L, UndefinedOptionFlag, MagickFalse },
    { "-encoding", 1L, UndefinedOptionFlag, MagickFalse },
    { "+endian", 0L, UndefinedOptionFlag, MagickFalse },
    { "-endian", 1L, UndefinedOptionFlag, MagickFalse },
    { "+enhance", 0L, UndefinedOptionFlag, MagickFalse },
    { "-enhance", 0L, UndefinedOptionFlag, MagickFalse },
    { "+equalize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-equalize", 0L, UndefinedOptionFlag, MagickFalse },
    { "+evaluate", 0L, UndefinedOptionFlag, MagickFalse },
    { "-evaluate", 2L, UndefinedOptionFlag, MagickFalse },
    { "+evaluate-sequence", 0L, FireOptionFlag, MagickFalse },
    { "-evaluate-sequence", 1L, FireOptionFlag, MagickFalse },
    { "+extent", 0L, UndefinedOptionFlag, MagickFalse },
    { "-extent", 1L, UndefinedOptionFlag, MagickFalse },
    { "+extract", 0L, UndefinedOptionFlag, MagickFalse },
    { "-extract", 1L, UndefinedOptionFlag, MagickFalse },
    { "+family", 0L, UndefinedOptionFlag, MagickFalse },
    { "-family", 1L, UndefinedOptionFlag, MagickFalse },
    { "+fft", 0L, FireOptionFlag, MagickFalse },
    { "-fft", 0L, FireOptionFlag, MagickFalse },
    { "+fill", 0L, UndefinedOptionFlag, MagickFalse },
    { "-fill", 1L, UndefinedOptionFlag, MagickFalse },
    { "+filter", 0L, UndefinedOptionFlag, MagickFalse },
    { "-filter", 1L, UndefinedOptionFlag, MagickFalse },
    { "+flatten", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-flatten", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+flip", 0L, UndefinedOptionFlag, MagickFalse },
    { "-flip", 0L, UndefinedOptionFlag, MagickFalse },
    { "+floodfill", 0L, UndefinedOptionFlag, MagickFalse },
    { "-floodfill", 2L, UndefinedOptionFlag, MagickFalse },
    { "+flop", 0L, UndefinedOptionFlag, MagickFalse },
    { "-flop", 0L, UndefinedOptionFlag, MagickFalse },
    { "+font", 0L, UndefinedOptionFlag, MagickFalse },
    { "-font", 1L, UndefinedOptionFlag, MagickFalse },
    { "+foreground", 0L, UndefinedOptionFlag, MagickFalse },
    { "-foreground", 1L, UndefinedOptionFlag, MagickFalse },
    { "+format", 0L, UndefinedOptionFlag, MagickFalse },
    { "-format", 1L, UndefinedOptionFlag, MagickFalse },
    { "+frame", 0L, UndefinedOptionFlag, MagickFalse },
    { "-frame", 1L, UndefinedOptionFlag, MagickFalse },
    { "+fuzz", 0L, UndefinedOptionFlag, MagickFalse },
    { "-fuzz", 1L, UndefinedOptionFlag, MagickFalse },
    { "+fx", 0L, FireOptionFlag, MagickFalse },
    { "-fx", 1L, FireOptionFlag, MagickFalse },
    { "+gamma", 0L, UndefinedOptionFlag, MagickFalse },
    { "-gamma", 1L, UndefinedOptionFlag, MagickFalse },
    { "+gaussian", 0L, UndefinedOptionFlag, MagickFalse },
    { "-gaussian", 1L, UndefinedOptionFlag, MagickFalse },
    { "+gaussian-blur", 0L, UndefinedOptionFlag, MagickFalse },
    { "-gaussian-blur", 1L, UndefinedOptionFlag, MagickFalse },
    { "+geometry", 0L, UndefinedOptionFlag, MagickFalse },
    { "-geometry", 1L, UndefinedOptionFlag, MagickFalse },
    { "+gravity", 0L, UndefinedOptionFlag, MagickFalse },
    { "-gravity", 1L, UndefinedOptionFlag, MagickFalse },
    { "+green-primary", 0L, UndefinedOptionFlag, MagickFalse },
    { "-green-primary", 1L, UndefinedOptionFlag, MagickFalse },
    { "+hald-clut", 0L, FireOptionFlag, MagickFalse },
    { "-hald-clut", 0L, FireOptionFlag, MagickFalse },
    { "+help", 0L, UndefinedOptionFlag, MagickFalse },
    { "-help", 0L, UndefinedOptionFlag, MagickFalse },
    { "+highlight-color", 1L, UndefinedOptionFlag, MagickFalse },
    { "-highlight-color", 1L, UndefinedOptionFlag, MagickFalse },
    { "+iconGeometry", 0L, UndefinedOptionFlag, MagickFalse },
    { "-iconGeometry", 1L, UndefinedOptionFlag, MagickFalse },
    { "+iconic", 0L, UndefinedOptionFlag, MagickFalse },
    { "-iconic", 1L, UndefinedOptionFlag, MagickFalse },
    { "+identify", 0L, FireOptionFlag, MagickFalse },
    { "-identify", 0L, FireOptionFlag, MagickFalse },
    { "+ift", 0L, FireOptionFlag, MagickFalse },
    { "-ift", 0L, FireOptionFlag, MagickFalse },
    { "+immutable", 0L, UndefinedOptionFlag, MagickFalse },
    { "-immutable", 0L, UndefinedOptionFlag, MagickFalse },
    { "+implode", 0L, UndefinedOptionFlag, MagickFalse },
    { "-implode", 1L, UndefinedOptionFlag, MagickFalse },
    { "+insert", 0L, FireOptionFlag, MagickFalse },
    { "-insert", 1L, FireOptionFlag, MagickFalse },
    { "+intent", 0L, UndefinedOptionFlag, MagickFalse },
    { "-intent", 1L, UndefinedOptionFlag, MagickFalse },
    { "+interlace", 0L, UndefinedOptionFlag, MagickFalse },
    { "-interlace", 1L, UndefinedOptionFlag, MagickFalse },
    { "+interpolate", 0L, UndefinedOptionFlag, MagickFalse },
    { "-interpolate", 1L, UndefinedOptionFlag, MagickFalse },
    { "+interword-spacing", 0L, UndefinedOptionFlag, MagickFalse },
    { "-interword-spacing", 1L, UndefinedOptionFlag, MagickFalse },
    { "+kerning", 0L, UndefinedOptionFlag, MagickFalse },
    { "-kerning", 1L, UndefinedOptionFlag, MagickFalse },
    { "+label", 0L, UndefinedOptionFlag, MagickFalse },
    { "-label", 1L, UndefinedOptionFlag, MagickFalse },
    { "+lat", 0L, UndefinedOptionFlag, MagickFalse },
    { "-lat", 1L, UndefinedOptionFlag, MagickFalse },
    { "+layers", 0L, FireOptionFlag, MagickFalse },
    { "-layers", 1L, FireOptionFlag, MagickFalse },
    { "+level", 1L, UndefinedOptionFlag, MagickFalse },
    { "-level", 1L, UndefinedOptionFlag, MagickFalse },
    { "+level-colors", 1L, UndefinedOptionFlag, MagickFalse },
    { "-level-colors", 1L, UndefinedOptionFlag, MagickFalse },
    { "+limit", 0L, FireOptionFlag, MagickFalse },
    { "-limit", 2L, FireOptionFlag, MagickFalse },
    { "+linear-stretch", 0L, UndefinedOptionFlag, MagickFalse },
    { "-linear-stretch", 1L, UndefinedOptionFlag, MagickFalse },
    { "+linewidth", 0L, UndefinedOptionFlag, MagickFalse },
    { "-linewidth", 1L, UndefinedOptionFlag, MagickFalse },
    { "+liquid-rescale", 0L, UndefinedOptionFlag, MagickFalse },
    { "-liquid-rescale", 1L, UndefinedOptionFlag, MagickFalse },
    { "+list", 0L, UndefinedOptionFlag, MagickFalse },
    { "-list", 1L, UndefinedOptionFlag, MagickFalse },
    { "+log", 0L, UndefinedOptionFlag, MagickFalse },
    { "-log", 1L, UndefinedOptionFlag, MagickFalse },
    { "+loop", 0L, UndefinedOptionFlag, MagickFalse },
    { "-loop", 1L, UndefinedOptionFlag, MagickFalse },
    { "+lowlight-color", 1L, UndefinedOptionFlag, MagickFalse },
    { "-lowlight-color", 1L, UndefinedOptionFlag, MagickFalse },
    { "+magnify", 0L, UndefinedOptionFlag, MagickFalse },
    { "-magnify", 1L, UndefinedOptionFlag, MagickFalse },
    { "+map", 0L, FireOptionFlag, MagickFalse },
    { "-map", 1L, FireOptionFlag, MagickFalse },
    { "+mask", 0L, UndefinedOptionFlag, MagickFalse },
    { "-mask", 1L, UndefinedOptionFlag, MagickFalse },
    { "+matte", 0L, DeprecateOptionFlag, MagickFalse },
    { "-matte", 0L, DeprecateOptionFlag, MagickFalse },
    { "+mattecolor", 0L, UndefinedOptionFlag, MagickFalse },
    { "-mattecolor", 1L, UndefinedOptionFlag, MagickFalse },
    { "+maximum", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-maximum", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+median", 0L, UndefinedOptionFlag, MagickFalse },
    { "-median", 1L, UndefinedOptionFlag, MagickFalse },
    { "+metric", 0L, UndefinedOptionFlag, MagickFalse },
    { "-metric", 1L, UndefinedOptionFlag, MagickFalse },
    { "+minimum", 0L, UndefinedOptionFlag, MagickFalse },
    { "-minimum", 0L, UndefinedOptionFlag, MagickFalse },
    { "+mode", 0L, UndefinedOptionFlag, MagickFalse },
    { "-mode", 1L, UndefinedOptionFlag, MagickFalse },
    { "+modulate", 0L, UndefinedOptionFlag, MagickFalse },
    { "-modulate", 1L, UndefinedOptionFlag, MagickFalse },
    { "+monitor", 0L, UndefinedOptionFlag, MagickFalse },
    { "-monitor", 0L, UndefinedOptionFlag, MagickFalse },
    { "+monochrome", 0L, UndefinedOptionFlag, MagickFalse },
    { "-monochrome", 0L, UndefinedOptionFlag, MagickFalse },
    { "+morph", 0L, FireOptionFlag, MagickFalse },
    { "-morph", 1L, FireOptionFlag, MagickFalse },
    { "+morphology", 0L, UndefinedOptionFlag, MagickFalse },
    { "-morphology", 2L, UndefinedOptionFlag, MagickFalse },
    { "+mosaic", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "-mosaic", 0L, FireOptionFlag | DeprecateOptionFlag, MagickFalse },
    { "+motion-blur", 0L, UndefinedOptionFlag, MagickFalse },
    { "-motion-blur", 1L, UndefinedOptionFlag, MagickFalse },
    { "+name", 0L, UndefinedOptionFlag, MagickFalse },
    { "-name", 1L, UndefinedOptionFlag, MagickFalse },
    { "+negate", 0L, UndefinedOptionFlag, MagickFalse },
    { "-negate", 0L, UndefinedOptionFlag, MagickFalse },
    { "+noise", 1L, UndefinedOptionFlag, MagickFalse },
    { "-noise", 1L, UndefinedOptionFlag, MagickFalse },
    { "+noop", 0L, UndefinedOptionFlag, MagickFalse },
    { "-noop", 0L, UndefinedOptionFlag, MagickFalse },
    { "+normalize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-normalize", 0L, UndefinedOptionFlag, MagickFalse },
    { "+opaque", 1L, UndefinedOptionFlag, MagickFalse },
    { "-opaque", 1L, UndefinedOptionFlag, MagickFalse },
    { "+ordered-dither", 0L, UndefinedOptionFlag, MagickFalse },
    { "-ordered-dither", 1L, UndefinedOptionFlag, MagickFalse },
    { "+orient", 0L, UndefinedOptionFlag, MagickFalse },
    { "-orient", 1L, UndefinedOptionFlag, MagickFalse },
    { "+origin", 0L, UndefinedOptionFlag, MagickFalse },
    { "-origin", 1L, UndefinedOptionFlag, MagickFalse },
    { "+page", 0L, UndefinedOptionFlag, MagickFalse },
    { "-page", 1L, UndefinedOptionFlag, MagickFalse },
    { "+paint", 0L, UndefinedOptionFlag, MagickFalse },
    { "-paint", 1L, UndefinedOptionFlag, MagickFalse },
    { "+path", 0L, UndefinedOptionFlag, MagickFalse },
    { "-path", 1L, UndefinedOptionFlag, MagickFalse },
    { "+pause", 0L, UndefinedOptionFlag, MagickFalse },
    { "-pause", 1L, UndefinedOptionFlag, MagickFalse },
    { "+passphrase", 0L, UndefinedOptionFlag, MagickFalse },
    { "-passphrase", 1L, UndefinedOptionFlag, MagickFalse },
    { "+pen", 0L, DeprecateOptionFlag, MagickFalse },
    { "-pen", 1L, DeprecateOptionFlag, MagickFalse },
    { "+ping", 0L, UndefinedOptionFlag, MagickFalse },
    { "-ping", 0L, UndefinedOptionFlag, MagickFalse },
    { "+pointsize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-pointsize", 1L, UndefinedOptionFlag, MagickFalse },
    { "+polaroid", 0L, UndefinedOptionFlag, MagickFalse },
    { "-polaroid", 1L, UndefinedOptionFlag, MagickFalse },
    { "+posterize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-posterize", 1L, UndefinedOptionFlag, MagickFalse },
    { "+preview", 0L, UndefinedOptionFlag, MagickFalse },
    { "-preview", 1L, UndefinedOptionFlag, MagickFalse },
    { "+print", 1L, FireOptionFlag, MagickFalse },
    { "-print", 1L, FireOptionFlag, MagickFalse },
    { "+process", 0L, FireOptionFlag, MagickFalse },
    { "-process", 1L, FireOptionFlag, MagickFalse },
    { "+profile", 1L, UndefinedOptionFlag, MagickFalse },
    { "-profile", 1L, UndefinedOptionFlag, MagickFalse },
    { "+quality", 0L, UndefinedOptionFlag, MagickFalse },
    { "-quality", 1L, UndefinedOptionFlag, MagickFalse },
    { "+quiet", 0L, FireOptionFlag, MagickFalse },
    { "-quiet", 0L, FireOptionFlag, MagickFalse },
    { "+radial-blur", 0L, UndefinedOptionFlag, MagickFalse },
    { "-radial-blur", 1L, UndefinedOptionFlag, MagickFalse },
    { "+raise", 0L, UndefinedOptionFlag, MagickFalse },
    { "-raise", 1L, UndefinedOptionFlag, MagickFalse },
    { "+random-threshold", 0L, UndefinedOptionFlag, MagickFalse },
    { "-random-threshold", 1L, UndefinedOptionFlag, MagickFalse },
    { "+recolor", 0L, UndefinedOptionFlag, MagickFalse },
    { "-recolor", 1L, UndefinedOptionFlag, MagickFalse },
    { "+red-primary", 0L, UndefinedOptionFlag, MagickFalse },
    { "-red-primary", 1L, UndefinedOptionFlag, MagickFalse },
    { "+regard-warnings", 0L, UndefinedOptionFlag, MagickFalse },
    { "-regard-warnings", 0L, UndefinedOptionFlag, MagickFalse },
    { "+region", 0L, UndefinedOptionFlag, MagickFalse },
    { "-region", 1L, UndefinedOptionFlag, MagickFalse },
    { "+remote", 0L, UndefinedOptionFlag, MagickFalse },
    { "-remote", 1L, UndefinedOptionFlag, MagickFalse },
    { "+render", 0L, UndefinedOptionFlag, MagickFalse },
    { "-render", 0L, UndefinedOptionFlag, MagickFalse },
    { "+repage", 0L, UndefinedOptionFlag, MagickFalse },
    { "-repage", 1L, UndefinedOptionFlag, MagickFalse },
    { "+resample", 0L, UndefinedOptionFlag, MagickFalse },
    { "-resample", 1L, UndefinedOptionFlag, MagickFalse },
    { "+resize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-resize", 1L, UndefinedOptionFlag, MagickFalse },
    { "+respect-parenthesis", 0L, UndefinedOptionFlag, MagickFalse },
    { "-respect-parenthesis", 0L, UndefinedOptionFlag, MagickFalse },
    { "+reverse", 0L, UndefinedOptionFlag, MagickFalse },
    { "-reverse", 0L, UndefinedOptionFlag, MagickFalse },
    { "+roll", 0L, UndefinedOptionFlag, MagickFalse },
    { "-roll", 1L, UndefinedOptionFlag, MagickFalse },
    { "+rotate", 0L, UndefinedOptionFlag, MagickFalse },
    { "-rotate", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sample", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sample", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sampling-factor", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sampling-factor", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sans", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sans", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sans0", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sans0", 0L, UndefinedOptionFlag, MagickFalse },
    { "+sans2", 2L, UndefinedOptionFlag, MagickFalse },
    { "-sans2", 2L, UndefinedOptionFlag, MagickFalse },
    { "+scale", 0L, UndefinedOptionFlag, MagickFalse },
    { "-scale", 1L, UndefinedOptionFlag, MagickFalse },
    { "+scene", 0L, UndefinedOptionFlag, MagickFalse },
    { "-scene", 1L, UndefinedOptionFlag, MagickFalse },
    { "+scenes", 0L, UndefinedOptionFlag, MagickFalse },
    { "-scenes", 1L, UndefinedOptionFlag, MagickFalse },
    { "+screen", 0L, UndefinedOptionFlag, MagickFalse },
    { "-screen", 1L, UndefinedOptionFlag, MagickFalse },
    { "+seed", 0L, UndefinedOptionFlag, MagickFalse },
    { "-seed", 1L, UndefinedOptionFlag, MagickFalse },
    { "+segment", 0L, UndefinedOptionFlag, MagickFalse },
    { "-segment", 1L, UndefinedOptionFlag, MagickFalse },
    { "+separate", 0L, FireOptionFlag, MagickFalse },
    { "-separate", 0L, FireOptionFlag, MagickFalse },
    { "+sepia-tone", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sepia-tone", 1L, UndefinedOptionFlag, MagickFalse },
    { "+set", 1L, UndefinedOptionFlag, MagickFalse },
    { "-set", 2L, UndefinedOptionFlag, MagickFalse },
    { "+shade", 0L, UndefinedOptionFlag, MagickFalse },
    { "-shade", 1L, UndefinedOptionFlag, MagickFalse },
    { "+shadow", 0L, UndefinedOptionFlag, MagickFalse },
    { "-shadow", 1L, UndefinedOptionFlag, MagickFalse },
    { "+shared-memory", 0L, UndefinedOptionFlag, MagickFalse },
    { "-shared-memory", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sharpen", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sharpen", 1L, UndefinedOptionFlag, MagickFalse },
    { "+shave", 0L, UndefinedOptionFlag, MagickFalse },
    { "-shave", 1L, UndefinedOptionFlag, MagickFalse },
    { "+shear", 0L, UndefinedOptionFlag, MagickFalse },
    { "-shear", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sigmoidal-contrast", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sigmoidal-contrast", 1L, UndefinedOptionFlag, MagickFalse },
    { "+silent", 0L, UndefinedOptionFlag, MagickFalse },
    { "-silent", 1L, UndefinedOptionFlag, MagickFalse },
    { "+size", 0L, UndefinedOptionFlag, MagickFalse },
    { "-size", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sketch", 0L, UndefinedOptionFlag, MagickFalse },
    { "-sketch", 1L, UndefinedOptionFlag, MagickFalse },
    { "+smush", 1L, FireOptionFlag, MagickFalse },
    { "-smush", 1L, FireOptionFlag, MagickFalse },
    { "+snaps", 0L, UndefinedOptionFlag, MagickFalse },
    { "-snaps", 1L, UndefinedOptionFlag, MagickFalse },
    { "+solarize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-solarize", 1L, UndefinedOptionFlag, MagickFalse },
    { "+splice", 0L, UndefinedOptionFlag, MagickFalse },
    { "-splice", 1L, UndefinedOptionFlag, MagickFalse },
    { "+sparse-color", 2L, UndefinedOptionFlag, MagickFalse },
    { "-sparse-color", 2L, UndefinedOptionFlag, MagickFalse },
    { "+spread", 0L, UndefinedOptionFlag, MagickFalse },
    { "-spread", 1L, UndefinedOptionFlag, MagickFalse },
    { "+statistic", 2L, UndefinedOptionFlag, MagickFalse },
    { "-statistic", 2L, UndefinedOptionFlag, MagickFalse },
    { "+stegano", 0L, UndefinedOptionFlag, MagickFalse },
    { "-stegano", 1L, UndefinedOptionFlag, MagickFalse },
    { "+stereo", 0L, UndefinedOptionFlag, MagickFalse },
    { "-stereo", 1L, UndefinedOptionFlag, MagickFalse },
    { "+stretch", 0L, UndefinedOptionFlag, MagickFalse },
    { "-stretch", 1L, UndefinedOptionFlag, MagickFalse },
    { "+strip", 0L, UndefinedOptionFlag, MagickFalse },
    { "-strip", 0L, UndefinedOptionFlag, MagickFalse },
    { "+stroke", 0L, UndefinedOptionFlag, MagickFalse },
    { "-stroke", 1L, UndefinedOptionFlag, MagickFalse },
    { "+strokewidth", 0L, UndefinedOptionFlag, MagickFalse },
    { "-strokewidth", 1L, UndefinedOptionFlag, MagickFalse },
    { "+style", 0L, UndefinedOptionFlag, MagickFalse },
    { "-style", 1L, UndefinedOptionFlag, MagickFalse },
    { "+subimage-search", 0L, UndefinedOptionFlag, MagickFalse },
    { "-subimage-search", 0L, UndefinedOptionFlag, MagickFalse },
    { "+swap", 0L, FireOptionFlag, MagickFalse },
    { "-swap", 1L, FireOptionFlag, MagickFalse },
    { "+swirl", 0L, UndefinedOptionFlag, MagickFalse },
    { "-swirl", 1L, UndefinedOptionFlag, MagickFalse },
    { "+synchronize", 0L, UndefinedOptionFlag, MagickFalse },
    { "-synchronize", 0L, UndefinedOptionFlag, MagickFalse },
    { "+taint", 0L, UndefinedOptionFlag, MagickFalse },
    { "-taint", 0L, UndefinedOptionFlag, MagickFalse },
    { "+text-font", 0L, UndefinedOptionFlag, MagickFalse },
    { "-text-font", 1L, UndefinedOptionFlag, MagickFalse },
    { "+texture", 0L, UndefinedOptionFlag, MagickFalse },
    { "-texture", 1L, UndefinedOptionFlag, MagickFalse },
    { "+threshold", 0L, UndefinedOptionFlag, MagickFalse },
    { "-threshold", 1L, UndefinedOptionFlag, MagickFalse },
    { "+thumbnail", 0L, UndefinedOptionFlag, MagickFalse },
    { "-thumbnail", 1L, UndefinedOptionFlag, MagickFalse },
    { "+thumnail", 0L, UndefinedOptionFlag, MagickFalse },
    { "-thumnail", 1L, UndefinedOptionFlag, MagickFalse },
    { "+tile", 0L, UndefinedOptionFlag, MagickFalse },
    { "-tile", 1L, UndefinedOptionFlag, MagickFalse },
    { "+tile-offset", 0L, UndefinedOptionFlag, MagickFalse },
    { "-tile-offset", 1L, UndefinedOptionFlag, MagickFalse },
    { "+tint", 0L, UndefinedOptionFlag, MagickFalse },
    { "-tint", 1L, UndefinedOptionFlag, MagickFalse },
    { "+title", 0L, UndefinedOptionFlag, MagickFalse },
    { "-title", 1L, UndefinedOptionFlag, MagickFalse },
    { "+transform", 0L, UndefinedOptionFlag, MagickFalse },
    { "-transform", 0L, UndefinedOptionFlag, MagickFalse },
    { "+transparent", 1L, UndefinedOptionFlag, MagickFalse },
    { "-transparent", 1L, UndefinedOptionFlag, MagickFalse },
    { "+transparent-color", 1L, UndefinedOptionFlag, MagickFalse },
    { "-transparent-color", 1L, UndefinedOptionFlag, MagickFalse },
    { "+transpose", 0L, UndefinedOptionFlag, MagickFalse },
    { "-transpose", 0L, UndefinedOptionFlag, MagickFalse },
    { "+transverse", 0L, UndefinedOptionFlag, MagickFalse },
    { "-transverse", 0L, UndefinedOptionFlag, MagickFalse },
    { "+treedepth", 0L, UndefinedOptionFlag, MagickFalse },
    { "-treedepth", 1L, UndefinedOptionFlag, MagickFalse },
    { "+trim", 0L, UndefinedOptionFlag, MagickFalse },
    { "-trim", 0L, UndefinedOptionFlag, MagickFalse },
    { "+type", 0L, UndefinedOptionFlag, MagickFalse },
    { "-type", 1L, UndefinedOptionFlag, MagickFalse },
    { "+undercolor", 0L, UndefinedOptionFlag, MagickFalse },
    { "-undercolor", 1L, UndefinedOptionFlag, MagickFalse },
    { "+unique-colors", 0L, UndefinedOptionFlag, MagickFalse },
    { "-unique-colors", 0L, UndefinedOptionFlag, MagickFalse },
    { "+units", 0L, UndefinedOptionFlag, MagickFalse },
    { "-units", 1L, UndefinedOptionFlag, MagickFalse },
    { "+unsharp", 0L, UndefinedOptionFlag, MagickFalse },
    { "-unsharp", 1L, UndefinedOptionFlag, MagickFalse },
    { "+update", 0L, UndefinedOptionFlag, MagickFalse },
    { "-update", 1L, UndefinedOptionFlag, MagickFalse },
    { "+use-pixmap", 0L, UndefinedOptionFlag, MagickFalse },
    { "-use-pixmap", 1L, UndefinedOptionFlag, MagickFalse },
    { "+verbose", 0L, UndefinedOptionFlag, MagickFalse },
    { "-verbose", 0L, UndefinedOptionFlag, MagickFalse },
    { "+version", 0L, UndefinedOptionFlag, MagickFalse },
    { "-version", 1L, UndefinedOptionFlag, MagickFalse },
    { "+view", 0L, UndefinedOptionFlag, MagickFalse },
    { "-view", 1L, UndefinedOptionFlag, MagickFalse },
    { "+vignette", 0L, UndefinedOptionFlag, MagickFalse },
    { "-vignette", 1L, UndefinedOptionFlag, MagickFalse },
    { "+virtual-pixel", 0L, UndefinedOptionFlag, MagickFalse },
    { "-virtual-pixel", 1L, UndefinedOptionFlag, MagickFalse },
    { "+visual", 0L, UndefinedOptionFlag, MagickFalse },
    { "-visual", 1L, UndefinedOptionFlag, MagickFalse },
    { "+watermark", 0L, UndefinedOptionFlag, MagickFalse },
    { "-watermark", 1L, UndefinedOptionFlag, MagickFalse },
    { "+wave", 0L, UndefinedOptionFlag, MagickFalse },
    { "-wave", 1L, UndefinedOptionFlag, MagickFalse },
    { "+weight", 0L, UndefinedOptionFlag, MagickFalse },
    { "-weight", 1L, UndefinedOptionFlag, MagickFalse },
    { "+white-point", 0L, UndefinedOptionFlag, MagickFalse },
    { "-white-point", 1L, UndefinedOptionFlag, MagickFalse },
    { "+white-threshold", 0L, UndefinedOptionFlag, MagickFalse },
    { "-white-threshold", 1L, UndefinedOptionFlag, MagickFalse },
    { "+window", 0L, UndefinedOptionFlag, MagickFalse },
    { "-window", 1L, UndefinedOptionFlag, MagickFalse },
    { "+window-group", 0L, UndefinedOptionFlag, MagickFalse },
    { "-window-group", 1L, UndefinedOptionFlag, MagickFalse },
    { "+write", 1L, FireOptionFlag, MagickFalse },
    { "-write", 1L, FireOptionFlag, MagickFalse },
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
    { "Dst", DstCompositeOp, UndefinedOptionFlag, MagickFalse },
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
    { "Src", SrcCompositeOp, UndefinedOptionFlag, MagickFalse },
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
    { "CMY", CMYColorspace, UndefinedOptionFlag, MagickFalse },
    { "CMYK", CMYKColorspace, UndefinedOptionFlag, MagickFalse },
    { "Gray", GRAYColorspace, UndefinedOptionFlag, MagickFalse },
    { "HSB", HSBColorspace, UndefinedOptionFlag, MagickFalse },
    { "HSL", HSLColorspace, UndefinedOptionFlag, MagickFalse },
    { "HWB", HWBColorspace, UndefinedOptionFlag, MagickFalse },
    { "Lab", LabColorspace, UndefinedOptionFlag, MagickFalse },
    { "Log", LogColorspace, UndefinedOptionFlag, MagickFalse },
    { "OHTA", OHTAColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec601Luma", Rec601LumaColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec601YCbCr", Rec601YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec709Luma", Rec709LumaColorspace, UndefinedOptionFlag, MagickFalse },
    { "Rec709YCbCr", Rec709YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "RGB", RGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "sRGB", sRGBColorspace, UndefinedOptionFlag, MagickFalse },
    { "Transparent", TransparentColorspace, UndefinedOptionFlag, MagickFalse },
    { "XYZ", XYZColorspace, UndefinedOptionFlag, MagickFalse },
    { "YCbCr", YCbCrColorspace, UndefinedOptionFlag, MagickFalse },
    { "YCC", YCCColorspace, UndefinedOptionFlag, MagickFalse },
    { "YIQ", YIQColorspace, UndefinedOptionFlag, MagickFalse },
    { "YPbPr", YPbPrColorspace, UndefinedOptionFlag, MagickFalse },
    { "YUV", YUVColorspace, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedColorspace, UndefinedOptionFlag, MagickFalse }
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
    { "Cubic", CubicFilter, UndefinedOptionFlag, MagickFalse },
    { "Gaussian", GaussianFilter, UndefinedOptionFlag, MagickFalse },
    { "Hamming", HammingFilter, UndefinedOptionFlag, MagickFalse },
    { "Hanning", HanningFilter, UndefinedOptionFlag, MagickFalse },
    { "Hermite", HermiteFilter, UndefinedOptionFlag, MagickFalse },
    { "Jinc", JincFilter, UndefinedOptionFlag, MagickFalse },
    { "Kaiser", KaiserFilter, UndefinedOptionFlag, MagickFalse },
    { "Lagrange", LagrangeFilter, UndefinedOptionFlag, MagickFalse },
    { "Lanczos", LanczosFilter, UndefinedOptionFlag, MagickFalse },
    { "LanczosSharp", LanczosSharpFilter, UndefinedOptionFlag, MagickFalse },
    { "Lanczos2", Lanczos2Filter, UndefinedOptionFlag, MagickFalse },
    { "Lanczos2Sharp", Lanczos2SharpFilter, UndefinedOptionFlag, MagickFalse },
    { "Mitchell", MitchellFilter, UndefinedOptionFlag, MagickFalse },
    { "Parzen", ParzenFilter, UndefinedOptionFlag, MagickFalse },
    { "Point", PointFilter, UndefinedOptionFlag, MagickFalse },
    { "Quadratic", QuadraticFilter, UndefinedOptionFlag, MagickFalse },
    { "Robidoux", RobidouxFilter, UndefinedOptionFlag, MagickFalse },
    { "Sinc", SincFilter, UndefinedOptionFlag, MagickFalse },
    { "SincFast", SincFastFilter, UndefinedOptionFlag, MagickFalse },
    { "Triangle", TriangleFilter, UndefinedOptionFlag, MagickFalse },
    { "Welsh", WelshFilter, UndefinedOptionFlag, MagickFalse },
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
    { "Static", StaticGravity, UndefinedOptionFlag, MagickFalse },
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
    { "Bicubic", BicubicInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Bilinear", BilinearInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "filter", FilterInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Integer", IntegerInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "Mesh", MeshInterpolatePixel, UndefinedOptionFlag, MagickFalse },
    { "NearestNeighbor", NearestNeighborInterpolatePixel, UndefinedOptionFlag, MagickFalse },
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
    { "Channel", MagickChannelOptions, UndefinedOptionFlag, MagickFalse },
    { "Class", MagickClassOptions, UndefinedOptionFlag, MagickFalse },
    { "ClipPath", MagickClipPathOptions, UndefinedOptionFlag, MagickFalse },
    { "Coder", MagickCoderOptions, UndefinedOptionFlag, MagickFalse },
    { "Color", MagickColorOptions, UndefinedOptionFlag, MagickFalse },
    { "Colorspace", MagickColorspaceOptions, UndefinedOptionFlag, MagickFalse },
    { "Command", MagickCommandOptions, UndefinedOptionFlag, MagickFalse },
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
    { "Undefined", UndefinedMetric, UndefinedOptionFlag, MagickTrue },
    { "AE", AbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "Fuzz", FuzzErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "MAE", MeanAbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "MEPP", MeanErrorPerPixelMetric, UndefinedOptionFlag, MagickFalse },
    { "MSE", MeanSquaredErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "NCC", NormalizedCrossCorrelationErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PAE", PeakAbsoluteErrorMetric, UndefinedOptionFlag, MagickFalse },
    { "PSNR", PeakSignalToNoiseRatioMetric, UndefinedOptionFlag, MagickFalse },
    { "RMSE", RootMeanSquaredErrorMetric, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedMetric, UndefinedOptionFlag, MagickFalse }
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
    { "Distance", DistanceMorphology, UndefinedOptionFlag, MagickFalse },
    { "Hmt", HitAndMissMorphology, UndefinedOptionFlag, MagickFalse },
    { "HitNMiss", HitAndMissMorphology, UndefinedOptionFlag, MagickFalse },
    { "HitAndMiss", HitAndMissMorphology, UndefinedOptionFlag, MagickFalse },
    { "Thinning", ThinningMorphology, UndefinedOptionFlag, MagickFalse },
    { "Thicken", ThickenMorphology, UndefinedOptionFlag, MagickFalse },
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
    { "ColorSeparationMatte", ColorSeparationMatteType, UndefinedOptionFlag, MagickFalse },
    { "Grayscale", GrayscaleType, UndefinedOptionFlag, MagickFalse },
    { "GrayscaleMatte", GrayscaleMatteType, UndefinedOptionFlag, MagickFalse },
    { "Optimize", OptimizeType, UndefinedOptionFlag, MagickFalse },
    { "Palette", PaletteType, UndefinedOptionFlag, MagickFalse },
    { "PaletteBilevelMatte", PaletteBilevelMatteType, UndefinedOptionFlag, MagickFalse },
    { "PaletteMatte", PaletteMatteType, UndefinedOptionFlag, MagickFalse },
    { "TrueColorMatte", TrueColorMatteType, UndefinedOptionFlag, MagickFalse },
    { "TrueColor", TrueColorType, UndefinedOptionFlag, MagickFalse },
    { (char *) NULL, UndefinedType, UndefinedOptionFlag, MagickFalse }
  },
  ValidateOptions[] =
  {
    { "Undefined", UndefinedValidate, UndefinedOptionFlag, MagickTrue },
    { "All", AllValidate, UndefinedOptionFlag, MagickFalse },
    { "Compare", CompareValidate, UndefinedOptionFlag, MagickFalse },
    { "Composite", CompositeValidate, UndefinedOptionFlag, MagickFalse },
    { "Convert", ConvertValidate, UndefinedOptionFlag, MagickFalse },
    { "FormatsInMemory", FormatsInMemoryValidate, UndefinedOptionFlag, MagickFalse },
    { "FormatsOnDisk", FormatsOnDiskValidate, UndefinedOptionFlag, MagickFalse },
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
    image_info->options=CloneSplayTree((SplayTreeInfo *) clone_info->options,
      (void *(*)(void *)) ConstantString,(void *(*)(void *)) ConstantString);
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
    case MagickChannelOptions: return(ChannelOptions);
    case MagickClassOptions: return(ClassOptions);
    case MagickClipPathOptions: return(ClipPathOptions);
    case MagickColorspaceOptions: return(ColorspaceOptions);
    case MagickCommandOptions: return(CommandOptions);
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
/*  case MagickImageListOptions: return(ImageListOptions); */
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
      if ((q-token) >= MaxTextExtent)
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
%      MagickBooleanType ListCommandOptions(FILE *file,const CommandOption option,
%        ExceptionInfo *exception)
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

  if (file == (FILE *) NULL)
    file=stdout;
  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return(MagickFalse);
  for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
  {
    if (option_info[i].stealth != MagickFalse)
      continue;
    (void) fprintf(file,"%s\n",option_info[i].mnemonic);
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
      if ((q-token) >= MaxTextExtent)
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
  MagickBooleanType
    status;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (LocaleCompare(option,"size") == 0)
    (void) CloneString(&image_info->size,value);
  if (image_info->options == (void *) NULL)
    image_info->options=NewSplayTree(CompareSplayTreeString,
      RelinquishMagickMemory,RelinquishMagickMemory);
  status=AddValueToSplayTree((SplayTreeInfo *) image_info->options,
    ConstantString(option),ConstantString(value));
  return(status);
}

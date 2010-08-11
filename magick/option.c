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
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
    { "Undefined", (ssize_t) UndefinedAlign, MagickTrue },
    { "Center", (ssize_t) CenterAlign, MagickFalse },
    { "End", (ssize_t) RightAlign, MagickFalse },
    { "Left", (ssize_t) LeftAlign, MagickFalse },
    { "Middle", (ssize_t) CenterAlign, MagickFalse },
    { "Right", (ssize_t) RightAlign, MagickFalse },
    { "Start", (ssize_t) LeftAlign, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedAlign, MagickFalse }
  },
  AlphaOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedAlphaChannel, MagickTrue },
    { "Activate", (ssize_t) ActivateAlphaChannel, MagickFalse },
    { "Background", (ssize_t) BackgroundAlphaChannel, MagickFalse },
    { "Copy", (ssize_t) CopyAlphaChannel, MagickFalse },
    { "Deactivate", (ssize_t) DeactivateAlphaChannel, MagickFalse },
    { "Extract", (ssize_t) ExtractAlphaChannel, MagickFalse },
    { "Off", (ssize_t) DeactivateAlphaChannel, MagickFalse },
    { "On", (ssize_t) ActivateAlphaChannel, MagickFalse },
    { "Opaque", (ssize_t) OpaqueAlphaChannel, MagickFalse },
    { "Set", (ssize_t) SetAlphaChannel, MagickFalse },
    { "Shape", (ssize_t) ShapeAlphaChannel, MagickFalse },
    { "Reset", (ssize_t) SetAlphaChannel, MagickTrue },  /* deprecated */
    { "Transparent", (ssize_t) TransparentAlphaChannel, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedAlphaChannel, MagickFalse }
  },
  BooleanOptions[] =
  {
    { "False", 0L, MagickFalse },
    { "True", 1L, MagickFalse },
    { "0", 0L, MagickFalse },
    { "1", 1L, MagickFalse },
    { (char *) NULL, 0L, MagickFalse }
  },
  ChannelOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedChannel, MagickTrue },
    { "All", (ssize_t) AllChannels, MagickFalse },
    { "Alpha", (ssize_t) OpacityChannel, MagickFalse },
    { "Black", (ssize_t) BlackChannel, MagickFalse },
    { "Blue", (ssize_t) BlueChannel, MagickFalse },
    { "Cyan", (ssize_t) CyanChannel, MagickFalse },
    { "Default", (ssize_t) DefaultChannels, MagickFalse },
    { "Gray", (ssize_t) GrayChannel, MagickFalse },
    { "Green", (ssize_t) GreenChannel, MagickFalse },
    { "Hue", (ssize_t) RedChannel, MagickFalse },
    { "Index", (ssize_t) IndexChannel, MagickFalse },
    { "Lightness", (ssize_t) BlueChannel, MagickFalse },
    { "Luminance", (ssize_t) BlueChannel, MagickFalse },
    { "Luminosity", (ssize_t) BlueChannel, MagickFalse },  /* deprecated */
    { "Magenta", (ssize_t) MagentaChannel, MagickFalse },
    { "Matte", (ssize_t) OpacityChannel, MagickFalse },
    { "Opacity", (ssize_t) OpacityChannel, MagickFalse },
    { "Red", (ssize_t) RedChannel, MagickFalse },
    { "Saturation", (ssize_t) GreenChannel, MagickFalse },
    { "Yellow", (ssize_t) YellowChannel, MagickFalse },
    { "Sync", (ssize_t) SyncChannels, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedChannel, MagickFalse }
  },
  ClassOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedClass, MagickTrue },
    { "DirectClass", (ssize_t) DirectClass, MagickFalse },
    { "PseudoClass", (ssize_t) PseudoClass, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedClass, MagickFalse }
  },
  ClipPathOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedPathUnits, MagickTrue },
    { "ObjectBoundingBox", (ssize_t) ObjectBoundingBox, MagickFalse },
    { "UserSpace", (ssize_t) UserSpace, MagickFalse },
    { "UserSpaceOnUse", (ssize_t) UserSpaceOnUse, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedPathUnits, MagickFalse }
  },
  CommandOptions[] =
  {
    { "+adjoin", 0L, MagickFalse },
    { "-adjoin", 0L, MagickFalse },
    { "+adaptive-sharpen", 1L, MagickFalse },
    { "-adaptive-sharpen", 1L, MagickFalse },
    { "+adaptive-threshold", 1L, MagickFalse },
    { "-adaptive-threshold", 1L, MagickFalse },
    { "+affine", 0L, MagickFalse },
    { "-affine", 1L, MagickFalse },
    { "+affinity", 0L, MagickFalse },
    { "-affinity", 1L, MagickFalse },
    { "+alpha", 1L, MagickFalse },
    { "-alpha", 1L, MagickFalse },
    { "+annotate", 0L, MagickFalse },
    { "-annotate", 2L, MagickFalse },
    { "+antialias", 0L, MagickFalse },
    { "-antialias", 0L, MagickFalse },
    { "+append", 0L, MagickFalse },
    { "-append", 0L, MagickFalse },
    { "+authenticate", 0L, MagickFalse },
    { "-authenticate", 1L, MagickFalse },
    { "+auto-gamma", 0L, MagickTrue },
    { "-auto-gamma", 0L, MagickTrue },
    { "+auto-level", 0L, MagickTrue },
    { "-auto-level", 0L, MagickTrue },
    { "+auto-orient", 0L, MagickFalse },
    { "-auto-orient", 0L, MagickFalse },
    { "+average", 0L, MagickFalse },
    { "-average", 0L, MagickFalse },
    { "+backdrop", 0L, MagickFalse },
    { "-backdrop", 1L, MagickFalse },
    { "+background", 0L, MagickFalse },
    { "-background", 1L, MagickFalse },
    { "+bench", 0L, MagickTrue },
    { "-bench", 1L, MagickTrue },
    { "+bias", 0L, MagickFalse },
    { "-bias", 1L, MagickFalse },
    { "+black-threshold", 0L, MagickFalse },
    { "-black-threshold", 1L, MagickFalse },
    { "+blend", 0L, MagickFalse },
    { "-blend", 1L, MagickFalse },
    { "+blue-primary", 0L, MagickFalse },
    { "-blue-primary", 1L, MagickFalse },
    { "+blue-shift", 1L, MagickFalse },
    { "-blue-shift", 1L, MagickFalse },
    { "+blur", 0L, MagickFalse },
    { "-blur", 1L, MagickFalse },
    { "+border", 0L, MagickFalse },
    { "-border", 1L, MagickFalse },
    { "+bordercolor", 0L, MagickFalse },
    { "-bordercolor", 1L, MagickFalse },
    { "+borderwidth", 0L, MagickFalse },
    { "-borderwidth", 1L, MagickFalse },
    { "+box", 0L, MagickFalse },
    { "-box", 1L, MagickFalse },
    { "+brightness-contrast", 0L, MagickFalse },
    { "-brightness-contrast", 1L, MagickFalse },
    { "+cache", 0L, MagickFalse },
    { "-cache", 1L, MagickFalse },
    { "+cdl", 1L, MagickFalse },
    { "-cdl", 1L, MagickFalse },
    { "+channel", 0L, MagickFalse },
    { "-channel", 1L, MagickFalse },
    { "+charcoal", 0L, MagickFalse },
    { "-charcoal", 0L, MagickFalse },
    { "+chop", 0L, MagickFalse },
    { "-chop", 1L, MagickFalse },
    { "+clip", 0L, MagickFalse },
    { "-clip", 0L, MagickFalse },
    { "+clip-mask", 0L, MagickFalse },
    { "-clip-mask", 1L, MagickFalse },
    { "+clip-path", 0L, MagickFalse },
    { "-clip-path", 1L, MagickFalse },
    { "+clone", 0L, MagickFalse },
    { "-clone", 1L, MagickFalse },
    { "+clut", 0L, MagickFalse },
    { "-clut", 0L, MagickFalse },
    { "+coalesce", 0L, MagickFalse },
    { "-coalesce", 0L, MagickFalse },
    { "+colorize", 0L, MagickFalse },
    { "-colorize", 1L, MagickFalse },
    { "+colormap", 0L, MagickFalse },
    { "-colormap", 1L, MagickFalse },
    { "+color-matrix", 0L, MagickFalse },
    { "-color-matrix", 1L, MagickFalse },
    { "+colors", 0L, MagickFalse },
    { "-colors", 1L, MagickFalse },
    { "+colorspace", 0L, MagickFalse },
    { "-colorspace", 1L, MagickFalse },
    { "+combine", 0L, MagickFalse },
    { "-combine", 0L, MagickFalse },
    { "+comment", 0L, MagickFalse },
    { "-comment", 1L, MagickFalse },
    { "+compose", 0L, MagickFalse },
    { "-compose", 1L, MagickFalse },
    { "+composite", 0L, MagickFalse },
    { "-composite", 0L, MagickFalse },
    { "+compress", 0L, MagickFalse },
    { "-compress", 1L, MagickFalse },
    { "+concurrent", 0L, MagickTrue },
    { "-concurrent", 0L, MagickTrue },
    { "+contrast", 0L, MagickFalse },
    { "-contrast", 0L, MagickFalse },
    { "+contrast-stretch", 0L, MagickFalse },
    { "-contrast-stretch", 1L, MagickFalse },
    { "+convolve", 0L, MagickFalse },
    { "-convolve", 1L, MagickFalse },
    { "+crop", 0L, MagickFalse },
    { "-crop", 1L, MagickFalse },
    { "+cycle", 0L, MagickFalse },
    { "-cycle", 1L, MagickFalse },
    { "+debug", 0L, MagickFalse },
    { "-debug", 1L, MagickFalse },
    { "+decipher", 1L, MagickFalse },
    { "-decipher", 1L, MagickFalse },
    { "+deconstruct", 0L, MagickFalse },
    { "-deconstruct", 0L, MagickFalse },
    { "+define", 1L, MagickFalse },
    { "-define", 1L, MagickFalse },
    { "+delay", 0L, MagickFalse },
    { "-delay", 1L, MagickFalse },
    { "+delete", 0L, MagickFalse },
    { "-delete", 1L, MagickFalse },
    { "+density", 0L, MagickFalse },
    { "-density", 1L, MagickFalse },
    { "+depth", 0L, MagickFalse },
    { "-depth", 1L, MagickFalse },
    { "+descend", 0L, MagickFalse },
    { "-descend", 1L, MagickFalse },
    { "+deskew", 0L, MagickFalse },
    { "-deskew", 1L, MagickFalse },
    { "+despeckle", 0L, MagickFalse },
    { "-despeckle", 0L, MagickFalse },
    { "+direction", 0L, MagickFalse },
    { "-direction", 1L, MagickFalse },
    { "+displace", 0L, MagickFalse },
    { "-displace", 1L, MagickFalse },
    { "+display", 0L, MagickFalse },
    { "-display", 1L, MagickFalse },
    { "+dispose", 0L, MagickFalse },
    { "-dispose", 1L, MagickFalse },
    { "+dissolve", 0L, MagickFalse },
    { "-dissolve", 1L, MagickFalse },
    { "+distort", 2L, MagickFalse },
    { "-distort", 2L, MagickFalse },
    { "+dither", 0L, MagickFalse },
    { "-dither", 1L, MagickFalse },
    { "+draw", 0L, MagickFalse },
    { "-draw", 1L, MagickFalse },
    { "+duration", 1L, MagickTrue },
    { "-duration", 1L, MagickTrue },
    { "+edge", 0L, MagickFalse },
    { "-edge", 1L, MagickFalse },
    { "+emboss", 0L, MagickFalse },
    { "-emboss", 1L, MagickFalse },
    { "+encipher", 1L, MagickFalse },
    { "-encipher", 1L, MagickFalse },
    { "+encoding", 0L, MagickFalse },
    { "-encoding", 1L, MagickFalse },
    { "+endian", 0L, MagickFalse },
    { "-endian", 1L, MagickFalse },
    { "+enhance", 0L, MagickFalse },
    { "-enhance", 0L, MagickFalse },
    { "+equalize", 0L, MagickFalse },
    { "-equalize", 0L, MagickFalse },
    { "+evaluate", 0L, MagickFalse },
    { "-evaluate", 2L, MagickFalse },
    { "+evaluate-sequence", 0L, MagickFalse },
    { "-evaluate-sequence", 1L, MagickFalse },
    { "+extent", 0L, MagickFalse },
    { "-extent", 1L, MagickFalse },
    { "+extract", 0L, MagickFalse },
    { "-extract", 1L, MagickFalse },
    { "+family", 0L, MagickFalse },
    { "-family", 1L, MagickFalse },
    { "+fft", 0L, MagickFalse },
    { "-fft", 0L, MagickFalse },
    { "+fill", 0L, MagickFalse },
    { "-fill", 1L, MagickFalse },
    { "+filter", 0L, MagickFalse },
    { "-filter", 1L, MagickFalse },
    { "+flatten", 0L, MagickFalse },
    { "-flatten", 0L, MagickFalse },
    { "+flip", 0L, MagickFalse },
    { "-flip", 0L, MagickFalse },
    { "+floodfill", 0L, MagickFalse },
    { "-floodfill", 2L, MagickFalse },
    { "+flop", 0L, MagickFalse },
    { "-flop", 0L, MagickFalse },
    { "+font", 0L, MagickFalse },
    { "-font", 1L, MagickFalse },
    { "+foreground", 0L, MagickFalse },
    { "-foreground", 1L, MagickFalse },
    { "+format", 0L, MagickFalse },
    { "-format", 1L, MagickFalse },
    { "+frame", 0L, MagickFalse },
    { "-frame", 1L, MagickFalse },
    { "+fuzz", 0L, MagickFalse },
    { "-fuzz", 1L, MagickFalse },
    { "+fx", 0L, MagickFalse },
    { "-fx", 1L, MagickFalse },
    { "+gamma", 0L, MagickFalse },
    { "-gamma", 1L, MagickFalse },
    { "+gaussian", 0L, MagickFalse },
    { "-gaussian", 1L, MagickFalse },
    { "+gaussian-blur", 0L, MagickFalse },
    { "-gaussian-blur", 1L, MagickFalse },
    { "+geometry", 0L, MagickFalse },
    { "-geometry", 1L, MagickFalse },
    { "+gravity", 0L, MagickFalse },
    { "-gravity", 1L, MagickFalse },
    { "+green-primary", 0L, MagickFalse },
    { "-green-primary", 1L, MagickFalse },
    { "+hald-clut", 0L, MagickFalse },
    { "-hald-clut", 0L, MagickFalse },
    { "+help", 0L, MagickFalse },
    { "-help", 0L, MagickFalse },
    { "+highlight-color", 1L, MagickFalse },
    { "-highlight-color", 1L, MagickFalse },
    { "+iconGeometry", 0L, MagickFalse },
    { "-iconGeometry", 1L, MagickFalse },
    { "+iconic", 0L, MagickFalse },
    { "-iconic", 1L, MagickFalse },
    { "+identify", 0L, MagickFalse },
    { "-identify", 0L, MagickFalse },
    { "+immutable", 0L, MagickFalse },
    { "-immutable", 0L, MagickFalse },
    { "+implode", 0L, MagickFalse },
    { "-implode", 1L, MagickFalse },
    { "+insert", 0L, MagickFalse },
    { "-insert", 1L, MagickFalse },
    { "+intent", 0L, MagickFalse },
    { "-intent", 1L, MagickFalse },
    { "+interlace", 0L, MagickFalse },
    { "-interlace", 1L, MagickFalse },
    { "+interpolate", 0L, MagickFalse },
    { "-interpolate", 1L, MagickFalse },
    { "+interword-spacing", 0L, MagickFalse },
    { "-interword-spacing", 1L, MagickFalse },
    { "+kerning", 0L, MagickFalse },
    { "-kerning", 1L, MagickFalse },
    { "+label", 0L, MagickFalse },
    { "-label", 1L, MagickFalse },
    { "+lat", 0L, MagickFalse },
    { "-lat", 1L, MagickFalse },
    { "+layers", 0L, MagickFalse },
    { "-layers", 1L, MagickFalse },
    { "+level", 1L, MagickFalse },
    { "-level", 1L, MagickFalse },
    { "+level-colors", 1L, MagickFalse },
    { "-level-colors", 1L, MagickFalse },
    { "+limit", 0L, MagickFalse },
    { "-limit", 2L, MagickFalse },
    { "+linear-stretch", 0L, MagickFalse },
    { "-linear-stretch", 1L, MagickFalse },
    { "+linewidth", 0L, MagickFalse },
    { "-linewidth", 1L, MagickFalse },
    { "+liquid-rescale", 0L, MagickFalse },
    { "-liquid-rescale", 1L, MagickFalse },
    { "+list", 0L, MagickFalse },
    { "-list", 1L, MagickFalse },
    { "+log", 0L, MagickFalse },
    { "-log", 1L, MagickFalse },
    { "+loop", 0L, MagickFalse },
    { "-loop", 1L, MagickFalse },
    { "+lowlight-color", 1L, MagickFalse },
    { "-lowlight-color", 1L, MagickFalse },
    { "+magnify", 0L, MagickFalse },
    { "-magnify", 1L, MagickFalse },
    { "+map", 0L, MagickFalse },
    { "-map", 1L, MagickFalse },
    { "+mask", 0L, MagickFalse },
    { "-mask", 1L, MagickFalse },
    { "+matte", 0L, MagickFalse },
    { "-matte", 0L, MagickFalse },
    { "+mattecolor", 0L, MagickFalse },
    { "-mattecolor", 1L, MagickFalse },
    { "+maximum", 0L, MagickFalse },
    { "-maximum", 0L, MagickFalse },
    { "+median", 0L, MagickFalse },
    { "-median", 1L, MagickFalse },
    { "+metric", 0L, MagickFalse },
    { "-metric", 1L, MagickFalse },
    { "+minimum", 0L, MagickFalse },
    { "-minimum", 0L, MagickFalse },
    { "+mode", 0L, MagickFalse },
    { "-mode", 1L, MagickFalse },
    { "+modulate", 0L, MagickFalse },
    { "-modulate", 1L, MagickFalse },
    { "+monitor", 0L, MagickFalse },
    { "-monitor", 0L, MagickFalse },
    { "+monochrome", 0L, MagickFalse },
    { "-monochrome", 0L, MagickFalse },
    { "+morph", 0L, MagickFalse },
    { "-morph", 1L, MagickFalse },
    { "+morphology", 0L, MagickFalse },
    { "-morphology", 2L, MagickFalse },
    { "+mosaic", 0L, MagickFalse },
    { "-mosaic", 0L, MagickFalse },
    { "+motion-blur", 0L, MagickFalse },
    { "-motion-blur", 1L, MagickFalse },
    { "+name", 0L, MagickFalse },
    { "-name", 1L, MagickFalse },
    { "+negate", 0L, MagickFalse },
    { "-negate", 0L, MagickFalse },
    { "+noise", 1L, MagickFalse },
    { "-noise", 1L, MagickFalse },
    { "+noop", 0L, MagickFalse },
    { "-noop", 0L, MagickFalse },
    { "+normalize", 0L, MagickFalse },
    { "-normalize", 0L, MagickFalse },
    { "+opaque", 1L, MagickFalse },
    { "-opaque", 1L, MagickFalse },
    { "+ordered-dither", 0L, MagickFalse },
    { "-ordered-dither", 1L, MagickFalse },
    { "+orient", 0L, MagickFalse },
    { "-orient", 1L, MagickFalse },
    { "+origin", 0L, MagickFalse },
    { "-origin", 1L, MagickFalse },
    { "+page", 0L, MagickFalse },
    { "-page", 1L, MagickFalse },
    { "+paint", 0L, MagickFalse },
    { "-paint", 1L, MagickFalse },
    { "+path", 0L, MagickFalse },
    { "-path", 1L, MagickFalse },
    { "+pause", 0L, MagickFalse },
    { "-pause", 1L, MagickFalse },
    { "+passphrase", 0L, MagickFalse },
    { "-passphrase", 1L, MagickFalse },
    { "+pen", 0L, MagickFalse },
    { "-pen", 1L, MagickFalse },
    { "+ping", 0L, MagickFalse },
    { "-ping", 0L, MagickFalse },
    { "+pointsize", 0L, MagickFalse },
    { "-pointsize", 1L, MagickFalse },
    { "+polaroid", 0L, MagickFalse },
    { "-polaroid", 1L, MagickFalse },
    { "+posterize", 0L, MagickFalse },
    { "-posterize", 1L, MagickFalse },
    { "+preview", 0L, MagickFalse },
    { "-preview", 1L, MagickFalse },
    { "+process", 0L, MagickFalse },
    { "-process", 1L, MagickFalse },
    { "+profile", 1L, MagickFalse },
    { "-profile", 1L, MagickFalse },
    { "+quality", 0L, MagickFalse },
    { "-quality", 1L, MagickFalse },
    { "+quiet", 0L, MagickFalse },
    { "-quiet", 0L, MagickFalse },
    { "+radial-blur", 0L, MagickFalse },
    { "-radial-blur", 1L, MagickFalse },
    { "+raise", 0L, MagickFalse },
    { "-raise", 1L, MagickFalse },
    { "+random-threshold", 0L, MagickFalse },
    { "-random-threshold", 1L, MagickFalse },
    { "+recolor", 0L, MagickFalse },
    { "-recolor", 1L, MagickFalse },
    { "+red-primary", 0L, MagickFalse },
    { "-red-primary", 1L, MagickFalse },
    { "+regard-warnings", 0L, MagickFalse },
    { "-regard-warnings", 0L, MagickFalse },
    { "+region", 0L, MagickFalse },
    { "-region", 1L, MagickFalse },
    { "+remote", 0L, MagickFalse },
    { "-remote", 1L, MagickFalse },
    { "+render", 0L, MagickFalse },
    { "-render", 0L, MagickFalse },
    { "+repage", 0L, MagickFalse },
    { "-repage", 1L, MagickFalse },
    { "+resample", 0L, MagickFalse },
    { "-resample", 1L, MagickFalse },
    { "+resize", 0L, MagickFalse },
    { "-resize", 1L, MagickFalse },
    { "+respect-parenthesis", 0L, MagickFalse },
    { "-respect-parenthesis", 0L, MagickFalse },
    { "+reverse", 0L, MagickFalse },
    { "-reverse", 0L, MagickFalse },
    { "+roll", 0L, MagickFalse },
    { "-roll", 1L, MagickFalse },
    { "+rotate", 0L, MagickFalse },
    { "-rotate", 1L, MagickFalse },
    { "+sample", 0L, MagickFalse },
    { "-sample", 1L, MagickFalse },
    { "+sampling-factor", 0L, MagickFalse },
    { "-sampling-factor", 1L, MagickFalse },
    { "+sans", 0L, MagickFalse },
    { "-sans", 1L, MagickFalse },
    { "+sans0", 0L, MagickFalse },
    { "-sans0", 0L, MagickFalse },
    { "+sans2", 2L, MagickFalse },
    { "-sans2", 2L, MagickFalse },
    { "+scale", 0L, MagickFalse },
    { "-scale", 1L, MagickFalse },
    { "+scene", 0L, MagickFalse },
    { "-scene", 1L, MagickFalse },
    { "+scenes", 0L, MagickFalse },
    { "-scenes", 1L, MagickFalse },
    { "+screen", 0L, MagickFalse },
    { "-screen", 1L, MagickFalse },
    { "+seed", 0L, MagickFalse },
    { "-seed", 1L, MagickFalse },
    { "+segment", 0L, MagickFalse },
    { "-segment", 1L, MagickFalse },
    { "+separate", 0L, MagickFalse },
    { "-separate", 0L, MagickFalse },
    { "+sepia-tone", 0L, MagickFalse },
    { "-sepia-tone", 1L, MagickFalse },
    { "+set", 1L, MagickFalse },
    { "-set", 2L, MagickFalse },
    { "+shade", 0L, MagickFalse },
    { "-shade", 1L, MagickFalse },
    { "+shadow", 0L, MagickFalse },
    { "-shadow", 1L, MagickFalse },
    { "+shared-memory", 0L, MagickFalse },
    { "-shared-memory", 1L, MagickFalse },
    { "+sharpen", 0L, MagickFalse },
    { "-sharpen", 1L, MagickFalse },
    { "+shave", 0L, MagickFalse },
    { "-shave", 1L, MagickFalse },
    { "+shear", 0L, MagickFalse },
    { "-shear", 1L, MagickFalse },
    { "+sigmoidal-contrast", 0L, MagickFalse },
    { "-sigmoidal-contrast", 1L, MagickFalse },
    { "+silent", 0L, MagickFalse },
    { "-silent", 1L, MagickFalse },
    { "+size", 0L, MagickFalse },
    { "-size", 1L, MagickFalse },
    { "+sketch", 0L, MagickFalse },
    { "-sketch", 1L, MagickFalse },
    { "+snaps", 0L, MagickFalse },
    { "-snaps", 1L, MagickFalse },
    { "+solarize", 0L, MagickFalse },
    { "-solarize", 1L, MagickFalse },
    { "+splice", 0L, MagickFalse },
    { "-splice", 1L, MagickFalse },
    { "+sparse-color", 2L, MagickFalse },
    { "-sparse-color", 2L, MagickFalse },
    { "+spread", 0L, MagickFalse },
    { "-spread", 1L, MagickFalse },
    { "+stegano", 0L, MagickFalse },
    { "-stegano", 1L, MagickFalse },
    { "+stereo", 0L, MagickFalse },
    { "-stereo", 1L, MagickFalse },
    { "+stretch", 0L, MagickFalse },
    { "-stretch", 1L, MagickFalse },
    { "+strip", 0L, MagickFalse },
    { "-strip", 0L, MagickFalse },
    { "+stroke", 0L, MagickFalse },
    { "-stroke", 1L, MagickFalse },
    { "+strokewidth", 0L, MagickFalse },
    { "-strokewidth", 1L, MagickFalse },
    { "+style", 0L, MagickFalse },
    { "-style", 1L, MagickFalse },
    { "+subimage-search", 0L, MagickFalse },
    { "-subimage-search", 0L, MagickFalse },
    { "+swap", 0L, MagickFalse },
    { "-swap", 1L, MagickFalse },
    { "+swirl", 0L, MagickFalse },
    { "-swirl", 1L, MagickFalse },
    { "+text-font", 0L, MagickFalse },
    { "-text-font", 1L, MagickFalse },
    { "+texture", 0L, MagickFalse },
    { "-texture", 1L, MagickFalse },
    { "+threshold", 0L, MagickFalse },
    { "-threshold", 1L, MagickFalse },
    { "+thumbnail", 0L, MagickFalse },
    { "-thumbnail", 1L, MagickFalse },
    { "+thumnail", 0L, MagickFalse },
    { "-thumnail", 1L, MagickFalse },
    { "+tile", 0L, MagickFalse },
    { "-tile", 1L, MagickFalse },
    { "+tile-offset", 0L, MagickFalse },
    { "-tile-offset", 1L, MagickFalse },
    { "+tint", 0L, MagickFalse },
    { "-tint", 1L, MagickFalse },
    { "+title", 0L, MagickFalse },
    { "-title", 1L, MagickFalse },
    { "+transform", 0L, MagickFalse },
    { "-transform", 0L, MagickFalse },
    { "+transparent", 1L, MagickFalse },
    { "-transparent", 1L, MagickFalse },
    { "+transparent-color", 1L, MagickFalse },
    { "-transparent-color", 1L, MagickFalse },
    { "+transpose", 0L, MagickFalse },
    { "-transpose", 0L, MagickFalse },
    { "+transverse", 0L, MagickFalse },
    { "-transverse", 0L, MagickFalse },
    { "+treedepth", 0L, MagickFalse },
    { "-treedepth", 1L, MagickFalse },
    { "+trim", 0L, MagickFalse },
    { "-trim", 0L, MagickFalse },
    { "+type", 0L, MagickFalse },
    { "-type", 1L, MagickFalse },
    { "+undercolor", 0L, MagickFalse },
    { "-undercolor", 1L, MagickFalse },
    { "+unique-colors", 0L, MagickFalse },
    { "-unique-colors", 0L, MagickFalse },
    { "+units", 0L, MagickFalse },
    { "-units", 1L, MagickFalse },
    { "+unsharp", 0L, MagickFalse },
    { "-unsharp", 1L, MagickFalse },
    { "+update", 0L, MagickFalse },
    { "-update", 1L, MagickFalse },
    { "+use-pixmap", 0L, MagickFalse },
    { "-use-pixmap", 1L, MagickFalse },
    { "+verbose", 0L, MagickFalse },
    { "-verbose", 0L, MagickFalse },
    { "+version", 0L, MagickFalse },
    { "-version", 1L, MagickFalse },
    { "+view", 0L, MagickFalse },
    { "-view", 1L, MagickFalse },
    { "+vignette", 0L, MagickFalse },
    { "-vignette", 1L, MagickFalse },
    { "+virtual-pixel", 0L, MagickFalse },
    { "-virtual-pixel", 1L, MagickFalse },
    { "+visual", 0L, MagickFalse },
    { "-visual", 1L, MagickFalse },
    { "+watermark", 0L, MagickFalse },
    { "-watermark", 1L, MagickFalse },
    { "+wave", 0L, MagickFalse },
    { "-wave", 1L, MagickFalse },
    { "+weight", 0L, MagickFalse },
    { "-weight", 1L, MagickFalse },
    { "+white-point", 0L, MagickFalse },
    { "-white-point", 1L, MagickFalse },
    { "+white-threshold", 0L, MagickFalse },
    { "-white-threshold", 1L, MagickFalse },
    { "+window", 0L, MagickFalse },
    { "-window", 1L, MagickFalse },
    { "+window-group", 0L, MagickFalse },
    { "-window-group", 1L, MagickFalse },
    { "+write", 1L, MagickFalse },
    { "-write", 1L, MagickFalse },
    { (char *) NULL, (ssize_t) 0L, MagickFalse }
  },
  ComposeOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedCompositeOp, MagickTrue },
    { "Atop", (ssize_t) AtopCompositeOp, MagickFalse },
    { "Blend", (ssize_t) BlendCompositeOp, MagickFalse },
    { "Blur", (ssize_t) BlurCompositeOp, MagickFalse },
    { "Bumpmap", (ssize_t) BumpmapCompositeOp, MagickFalse },
    { "ChangeMask", (ssize_t) ChangeMaskCompositeOp, MagickFalse },
    { "Clear", (ssize_t) ClearCompositeOp, MagickFalse },
    { "ColorBurn", (ssize_t) ColorBurnCompositeOp, MagickFalse },
    { "ColorDodge", (ssize_t) ColorDodgeCompositeOp, MagickFalse },
    { "Colorize", (ssize_t) ColorizeCompositeOp, MagickFalse },
    { "CopyBlack", (ssize_t) CopyBlackCompositeOp, MagickFalse },
    { "CopyBlue", (ssize_t) CopyBlueCompositeOp, MagickFalse },
    { "CopyCyan", (ssize_t) CopyCyanCompositeOp, MagickFalse },
    { "CopyGreen", (ssize_t) CopyGreenCompositeOp, MagickFalse },
    { "Copy", (ssize_t) CopyCompositeOp, MagickFalse },
    { "CopyMagenta", (ssize_t) CopyMagentaCompositeOp, MagickFalse },
    { "CopyOpacity", (ssize_t) CopyOpacityCompositeOp, MagickFalse },
    { "CopyRed", (ssize_t) CopyRedCompositeOp, MagickFalse },
    { "CopyYellow", (ssize_t) CopyYellowCompositeOp, MagickFalse },
    { "Darken", (ssize_t) DarkenCompositeOp, MagickFalse },
    { "Divide", (ssize_t) DivideCompositeOp, MagickFalse },
    { "Dst", (ssize_t) DstCompositeOp, MagickFalse },
    { "Difference", (ssize_t) DifferenceCompositeOp, MagickFalse },
    { "Displace", (ssize_t) DisplaceCompositeOp, MagickFalse },
    { "Dissolve", (ssize_t) DissolveCompositeOp, MagickFalse },
    { "Distort", (ssize_t) DistortCompositeOp, MagickFalse },
    { "DstAtop", (ssize_t) DstAtopCompositeOp, MagickFalse },
    { "DstIn", (ssize_t) DstInCompositeOp, MagickFalse },
    { "DstOut", (ssize_t) DstOutCompositeOp, MagickFalse },
    { "DstOver", (ssize_t) DstOverCompositeOp, MagickFalse },
    { "Dst", (ssize_t) DstCompositeOp, MagickFalse },
    { "Exclusion", (ssize_t) ExclusionCompositeOp, MagickFalse },
    { "HardLight", (ssize_t) HardLightCompositeOp, MagickFalse },
    { "Hue", (ssize_t) HueCompositeOp, MagickFalse },
    { "In", (ssize_t) InCompositeOp, MagickFalse },
    { "Lighten", (ssize_t) LightenCompositeOp, MagickFalse },
    { "LinearBurn", (ssize_t) LinearBurnCompositeOp, MagickFalse },
    { "LinearDodge", (ssize_t) LinearDodgeCompositeOp, MagickFalse },
    { "LinearLight", (ssize_t) LinearLightCompositeOp, MagickFalse },
    { "Luminize", (ssize_t) LuminizeCompositeOp, MagickFalse },
    { "Mathematics", (ssize_t) MathematicsCompositeOp, MagickFalse },
    { "Minus", (ssize_t) MinusCompositeOp, MagickFalse },
    { "Modulate", (ssize_t) ModulateCompositeOp, MagickFalse },
    { "ModulusAdd", (ssize_t) ModulusAddCompositeOp, MagickFalse },
    { "ModulusSubtract", (ssize_t) ModulusSubtractCompositeOp, MagickFalse },
    { "Multiply", (ssize_t) MultiplyCompositeOp, MagickFalse },
    { "None", (ssize_t) NoCompositeOp, MagickFalse },
    { "Out", (ssize_t) OutCompositeOp, MagickFalse },
    { "Overlay", (ssize_t) OverlayCompositeOp, MagickFalse },
    { "Over", (ssize_t) OverCompositeOp, MagickFalse },
    { "PegtopLight", (ssize_t) PegtopLightCompositeOp, MagickFalse },
    { "PinLight", (ssize_t) PinLightCompositeOp, MagickFalse },
    { "Plus", (ssize_t) PlusCompositeOp, MagickFalse },
    { "Replace", (ssize_t) ReplaceCompositeOp, MagickFalse },
    { "Saturate", (ssize_t) SaturateCompositeOp, MagickFalse },
    { "Screen", (ssize_t) ScreenCompositeOp, MagickFalse },
    { "SoftLight", (ssize_t) SoftLightCompositeOp, MagickFalse },
    { "Src", (ssize_t) SrcCompositeOp, MagickFalse },
    { "SrcAtop", (ssize_t) SrcAtopCompositeOp, MagickFalse },
    { "SrcIn", (ssize_t) SrcInCompositeOp, MagickFalse },
    { "SrcOut", (ssize_t) SrcOutCompositeOp, MagickFalse },
    { "SrcOver", (ssize_t) SrcOverCompositeOp, MagickFalse },
    { "Src", (ssize_t) SrcCompositeOp, MagickFalse },
    { "VividLight", (ssize_t) VividLightCompositeOp, MagickFalse },
    { "Xor", (ssize_t) XorCompositeOp, MagickFalse },
    { "Add", (ssize_t) AddCompositeOp, MagickTrue }, /* deprecate */
    { "Subtract", (ssize_t) SubtractCompositeOp, MagickTrue }, /* deprecate */
    { "Threshold", (ssize_t) ThresholdCompositeOp, MagickTrue }, /* deprecate */
    { (char *) NULL, (ssize_t) UndefinedCompositeOp, MagickFalse }
  },
  CompressOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedCompression, MagickTrue },
    { "B44", (ssize_t) B44Compression, MagickFalse },
    { "B44A", (ssize_t) B44ACompression, MagickFalse },
    { "BZip", (ssize_t) BZipCompression, MagickFalse },
    { "DXT1", (ssize_t) DXT1Compression, MagickFalse },
    { "DXT3", (ssize_t) DXT3Compression, MagickFalse },
    { "DXT5", (ssize_t) DXT5Compression, MagickFalse },
    { "Fax", (ssize_t) FaxCompression, MagickFalse },
    { "Group4", (ssize_t) Group4Compression, MagickFalse },
    { "JPEG", (ssize_t) JPEGCompression, MagickFalse },
    { "JPEG2000", (ssize_t) JPEG2000Compression, MagickFalse },
    { "Lossless", (ssize_t) LosslessJPEGCompression, MagickFalse },
    { "LosslessJPEG", (ssize_t) LosslessJPEGCompression, MagickFalse },
    { "LZW", (ssize_t) LZWCompression, MagickFalse },
    { "None", (ssize_t) NoCompression, MagickFalse },
    { "Piz", (ssize_t) PizCompression, MagickFalse },
    { "Pxr24", (ssize_t) Pxr24Compression, MagickFalse },
    { "RLE", (ssize_t) RLECompression, MagickFalse },
    { "Zip", (ssize_t) ZipCompression, MagickFalse },
    { "RunlengthEncoded", (ssize_t) RLECompression, MagickFalse },
    { "ZipS", (ssize_t) ZipSCompression, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedCompression, MagickFalse }
  },
  ColorspaceOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedColorspace, MagickTrue },
    { "CMY", (ssize_t) CMYColorspace, MagickFalse },
    { "CMYK", (ssize_t) CMYKColorspace, MagickFalse },
    { "Gray", (ssize_t) GRAYColorspace, MagickFalse },
    { "HSB", (ssize_t) HSBColorspace, MagickFalse },
    { "HSL", (ssize_t) HSLColorspace, MagickFalse },
    { "HWB", (ssize_t) HWBColorspace, MagickFalse },
    { "Lab", (ssize_t) LabColorspace, MagickFalse },
    { "Log", (ssize_t) LogColorspace, MagickFalse },
    { "OHTA", (ssize_t) OHTAColorspace, MagickFalse },
    { "Rec601Luma", (ssize_t) Rec601LumaColorspace, MagickFalse },
    { "Rec601YCbCr", (ssize_t) Rec601YCbCrColorspace, MagickFalse },
    { "Rec709Luma", (ssize_t) Rec709LumaColorspace, MagickFalse },
    { "Rec709YCbCr", (ssize_t) Rec709YCbCrColorspace, MagickFalse },
    { "RGB", (ssize_t) RGBColorspace, MagickFalse },
    { "sRGB", (ssize_t) sRGBColorspace, MagickFalse },
    { "Transparent", (ssize_t) TransparentColorspace, MagickFalse },
    { "XYZ", (ssize_t) XYZColorspace, MagickFalse },
    { "YCbCr", (ssize_t) YCbCrColorspace, MagickFalse },
    { "YCC", (ssize_t) YCCColorspace, MagickFalse },
    { "YIQ", (ssize_t) YIQColorspace, MagickFalse },
    { "YPbPr", (ssize_t) YPbPrColorspace, MagickFalse },
    { "YUV", (ssize_t) YUVColorspace, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedColorspace, MagickFalse }
  },
  DataTypeOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedData, MagickTrue },
    { "Byte", (ssize_t) ByteData, MagickFalse },
    { "Long", (ssize_t) LongData, MagickFalse },
    { "Short", (ssize_t) ShortData, MagickFalse },
    { "String", (ssize_t) StringData, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedData, MagickFalse }
  },
  DecorateOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedDecoration, MagickTrue },
    { "LineThrough", (ssize_t) LineThroughDecoration, MagickFalse },
    { "None", (ssize_t) NoDecoration, MagickFalse },
    { "Overline", (ssize_t) OverlineDecoration, MagickFalse },
    { "Underline", (ssize_t) UnderlineDecoration, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedDecoration, MagickFalse }
  },
  DirectionOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedDirection, MagickTrue },
    { "right-to-left", (ssize_t) RightToLeftDirection, MagickFalse },
    { "left-to-right", (ssize_t) LeftToRightDirection, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedDirection, MagickFalse }
  },
  DisposeOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedDispose, MagickTrue },
    { "Background", (ssize_t) BackgroundDispose, MagickFalse },
    { "None", (ssize_t) NoneDispose, MagickFalse },
    { "Previous", (ssize_t) PreviousDispose, MagickFalse },
    { "Undefined", (ssize_t) UndefinedDispose, MagickFalse },
    { "0", (ssize_t) UndefinedDispose, MagickFalse },
    { "1", (ssize_t) NoneDispose, MagickFalse },
    { "2", (ssize_t) BackgroundDispose, MagickFalse },
    { "3", (ssize_t) PreviousDispose, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedDispose, MagickFalse }
  },
  DistortOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedDistortion, MagickTrue },
    { "Affine", (ssize_t) AffineDistortion, MagickFalse },
    { "AffineProjection", (ssize_t) AffineProjectionDistortion, MagickFalse },
    { "ScaleRotateTranslate", (ssize_t) ScaleRotateTranslateDistortion, MagickFalse },
    { "SRT", (ssize_t) ScaleRotateTranslateDistortion, MagickFalse },
    { "Perspective", (ssize_t) PerspectiveDistortion, MagickFalse },
    { "PerspectiveProjection", (ssize_t) PerspectiveProjectionDistortion, MagickFalse },
    { "Bilinear", (ssize_t) BilinearForwardDistortion, MagickTrue },
    { "BilinearForward", (ssize_t) BilinearForwardDistortion, MagickFalse },
    { "BilinearReverse", (ssize_t) BilinearReverseDistortion, MagickFalse },
    { "Polynomial", (ssize_t) PolynomialDistortion, MagickFalse },
    { "Arc", (ssize_t) ArcDistortion, MagickFalse },
    { "Polar", (ssize_t) PolarDistortion, MagickFalse },
    { "DePolar", (ssize_t) DePolarDistortion, MagickFalse },
    { "Barrel", (ssize_t) BarrelDistortion, MagickFalse },
    { "BarrelInverse", (ssize_t) BarrelInverseDistortion, MagickFalse },
    { "Shepards", (ssize_t) ShepardsDistortion, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedDistortion, MagickFalse }
  },
  DitherOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedDitherMethod, MagickTrue },
    { "None", (ssize_t) NoDitherMethod, MagickFalse },
    { "FloydSteinberg", (ssize_t) FloydSteinbergDitherMethod, MagickFalse },
    { "Riemersma", (ssize_t) RiemersmaDitherMethod, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedEndian, MagickFalse }
  },
  EndianOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedEndian, MagickTrue },
    { "LSB", (ssize_t) LSBEndian, MagickFalse },
    { "MSB", (ssize_t) MSBEndian, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedEndian, MagickFalse }
  },
  EvaluateOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedEvaluateOperator, MagickTrue },
    { "Abs", (ssize_t) AbsEvaluateOperator, MagickFalse },
    { "Add", (ssize_t) AddEvaluateOperator, MagickFalse },
    { "AddModulus", (ssize_t) AddModulusEvaluateOperator, MagickFalse },
    { "And", (ssize_t) AndEvaluateOperator, MagickFalse },
    { "Cos", (ssize_t) CosineEvaluateOperator, MagickFalse },
    { "Cosine", (ssize_t) CosineEvaluateOperator, MagickFalse },
    { "Divide", (ssize_t) DivideEvaluateOperator, MagickFalse },
    { "GaussianNoise", (ssize_t) GaussianNoiseEvaluateOperator, MagickFalse },
    { "ImpulseNoise", (ssize_t) ImpulseNoiseEvaluateOperator, MagickFalse },
    { "LaplacianNoise", (ssize_t) LaplacianNoiseEvaluateOperator, MagickFalse },
    { "LeftShift", (ssize_t) LeftShiftEvaluateOperator, MagickFalse },
    { "Log", (ssize_t) LogEvaluateOperator, MagickFalse },
    { "Max", (ssize_t) MaxEvaluateOperator, MagickFalse },
    { "Mean", (ssize_t) MeanEvaluateOperator, MagickFalse },
    { "Min", (ssize_t) MinEvaluateOperator, MagickFalse },
    { "MultiplicativeNoise", (ssize_t) MultiplicativeNoiseEvaluateOperator, MagickFalse },
    { "Multiply", (ssize_t) MultiplyEvaluateOperator, MagickFalse },
    { "Or", (ssize_t) OrEvaluateOperator, MagickFalse },
    { "PoissonNoise", (ssize_t) PoissonNoiseEvaluateOperator, MagickFalse },
    { "Pow", (ssize_t) PowEvaluateOperator, MagickFalse },
    { "RightShift", (ssize_t) RightShiftEvaluateOperator, MagickFalse },
    { "Set", (ssize_t) SetEvaluateOperator, MagickFalse },
    { "Sin", (ssize_t) SineEvaluateOperator, MagickFalse },
    { "Sine", (ssize_t) SineEvaluateOperator, MagickFalse },
    { "Subtract", (ssize_t) SubtractEvaluateOperator, MagickFalse },
    { "Threshold", (ssize_t) ThresholdEvaluateOperator, MagickFalse },
    { "ThresholdBlack", (ssize_t) ThresholdBlackEvaluateOperator, MagickFalse },
    { "ThresholdWhite", (ssize_t) ThresholdWhiteEvaluateOperator, MagickFalse },
    { "UniformNoise", (ssize_t) UniformNoiseEvaluateOperator, MagickFalse },
    { "Xor", (ssize_t) XorEvaluateOperator, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedEvaluateOperator, MagickFalse }
  },
  FillRuleOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedRule, MagickTrue },
    { "Evenodd", (ssize_t) EvenOddRule, MagickFalse },
    { "NonZero", (ssize_t) NonZeroRule, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedRule, MagickFalse }
  },
  FilterOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedFilter, MagickTrue },
    { "Bartlett", (ssize_t) BartlettFilter, MagickFalse },
    { "Bessel", (ssize_t) BesselFilter, MagickFalse },
    { "Blackman", (ssize_t) BlackmanFilter, MagickFalse },
    { "Bohman", (ssize_t) BohmanFilter, MagickFalse },
    { "Box", (ssize_t) BoxFilter, MagickFalse },
    { "Catrom", (ssize_t) CatromFilter, MagickFalse },
    { "Cubic", (ssize_t) CubicFilter, MagickFalse },
    { "Gaussian", (ssize_t) GaussianFilter, MagickFalse },
    { "Hamming", (ssize_t) HammingFilter, MagickFalse },
    { "Hanning", (ssize_t) HanningFilter, MagickFalse },
    { "Hermite", (ssize_t) HermiteFilter, MagickFalse },
    { "Kaiser", (ssize_t) KaiserFilter, MagickFalse },
    { "Lagrange", (ssize_t) LagrangeFilter, MagickFalse },
    { "Lanczos", (ssize_t) LanczosFilter, MagickFalse },
    { "Mitchell", (ssize_t) MitchellFilter, MagickFalse },
    { "Parzen", (ssize_t) ParzenFilter, MagickFalse },
    { "Point", (ssize_t) PointFilter, MagickFalse },
    { "Quadratic", (ssize_t) QuadraticFilter, MagickFalse },
    { "Sinc", (ssize_t) SincFilter, MagickFalse },
    { "Triangle", (ssize_t) TriangleFilter, MagickFalse },
    { "Welsh", (ssize_t) WelshFilter, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedFilter, MagickFalse }
  },
  FunctionOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedFunction, MagickTrue },
    { "Polynomial", (ssize_t) PolynomialFunction, MagickFalse },
    { "Sinusoid", (ssize_t) SinusoidFunction, MagickFalse },
    { "ArcSin", (ssize_t) ArcsinFunction, MagickFalse },
    { "ArcTan", (ssize_t) ArctanFunction, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedFunction, MagickFalse }
  },
  GravityOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedGravity, MagickTrue },
    { "None", (ssize_t) UndefinedGravity, MagickFalse },
    { "Center", (ssize_t) CenterGravity, MagickFalse },
    { "East", (ssize_t) EastGravity, MagickFalse },
    { "Forget", (ssize_t) ForgetGravity, MagickFalse },
    { "NorthEast", (ssize_t) NorthEastGravity, MagickFalse },
    { "North", (ssize_t) NorthGravity, MagickFalse },
    { "NorthWest", (ssize_t) NorthWestGravity, MagickFalse },
    { "SouthEast", (ssize_t) SouthEastGravity, MagickFalse },
    { "South", (ssize_t) SouthGravity, MagickFalse },
    { "SouthWest", (ssize_t) SouthWestGravity, MagickFalse },
    { "West", (ssize_t) WestGravity, MagickFalse },
    { "Static", (ssize_t) StaticGravity, MagickFalse },
    { (char *) NULL, UndefinedGravity, MagickFalse }
  },
  ImageListOptions[] =
  {
    { "append", MagickTrue, MagickFalse },
    { "affinity", MagickTrue, MagickFalse },
    { "average", MagickTrue, MagickFalse },
    { "clut", MagickTrue, MagickFalse },
    { "coalesce", MagickTrue, MagickFalse },
    { "combine", MagickTrue, MagickFalse },
    { "composite", MagickTrue, MagickFalse },
    { "crop", MagickTrue, MagickFalse },
    { "debug", MagickTrue, MagickFalse },
    { "deconstruct", MagickTrue, MagickFalse },
    { "delete", MagickTrue, MagickFalse },
    { "evaluate-seqence", MagickTrue, MagickFalse },
    { "fft", MagickTrue, MagickFalse },
    { "flatten", MagickTrue, MagickFalse },
    { "fx", MagickTrue, MagickFalse },
    { "hald-clut", MagickTrue, MagickFalse },
    { "ift", MagickTrue, MagickFalse },
    { "identify", MagickTrue, MagickFalse },
    { "insert", MagickTrue, MagickFalse },
    { "layers", MagickTrue, MagickFalse },
    { "limit", MagickTrue, MagickFalse },
    { "map", MagickTrue, MagickFalse },
    { "maximum", MagickTrue, MagickFalse },
    { "minimum", MagickTrue, MagickFalse },
    { "morph", MagickTrue, MagickFalse },
    { "mosaic", MagickTrue, MagickFalse },
    { "optimize", MagickTrue, MagickFalse },
    { "print", MagickTrue, MagickFalse },
    { "process", MagickTrue, MagickFalse },
    { "quiet", MagickTrue, MagickFalse },
    { "separate", MagickTrue, MagickFalse },
    { "swap", MagickTrue, MagickFalse },
    { "write", MagickTrue, MagickFalse },
    { (char *) NULL, MagickFalse, MagickFalse }
  },
  IntentOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedIntent, MagickTrue },
    { "Absolute", (ssize_t) AbsoluteIntent, MagickFalse },
    { "Perceptual", (ssize_t) PerceptualIntent, MagickFalse },
    { "Relative", (ssize_t) RelativeIntent, MagickFalse },
    { "Saturation", (ssize_t) SaturationIntent, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedIntent, MagickFalse }
  },
  InterlaceOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedInterlace, MagickTrue },
    { "Line", (ssize_t) LineInterlace, MagickFalse },
    { "None", (ssize_t) NoInterlace, MagickFalse },
    { "Plane", (ssize_t) PlaneInterlace, MagickFalse },
    { "Partition", (ssize_t) PartitionInterlace, MagickFalse },
    { "GIF", (ssize_t) GIFInterlace, MagickFalse },
    { "JPEG", (ssize_t) JPEGInterlace, MagickFalse },
    { "PNG", (ssize_t) PNGInterlace, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedInterlace, MagickFalse }
  },
  InterpolateOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedInterpolatePixel, MagickTrue },
    { "Average", (ssize_t) AverageInterpolatePixel, MagickFalse },
    { "Bicubic", (ssize_t) BicubicInterpolatePixel, MagickFalse },
    { "Bilinear", (ssize_t) BilinearInterpolatePixel, MagickFalse },
    { "filter", (ssize_t) FilterInterpolatePixel, MagickFalse },
    { "Integer", (ssize_t) IntegerInterpolatePixel, MagickFalse },
    { "Mesh", (ssize_t) MeshInterpolatePixel, MagickFalse },
    { "NearestNeighbor", (ssize_t) NearestNeighborInterpolatePixel, MagickFalse },
    { "Spline", (ssize_t) SplineInterpolatePixel, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedInterpolatePixel, MagickFalse }
  },
  KernelOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedKernel, MagickTrue },
    { "Unity", (ssize_t) UnityKernel, MagickFalse },
    { "Gaussian", (ssize_t) GaussianKernel, MagickFalse },
    { "DoG", (ssize_t) DoGKernel, MagickFalse },
    { "LoG", (ssize_t) LoGKernel, MagickFalse },
    { "Blur", (ssize_t) BlurKernel, MagickFalse },
    { "Comet", (ssize_t) CometKernel, MagickFalse },
    { "Laplacian", (ssize_t) LaplacianKernel, MagickFalse },
    { "Sobel", (ssize_t) SobelKernel, MagickFalse },
    { "FreiChen", (ssize_t) FreiChenKernel, MagickFalse },
    { "Roberts", (ssize_t) RobertsKernel, MagickFalse },
    { "Prewitt", (ssize_t) PrewittKernel, MagickFalse },
    { "Compass", (ssize_t) CompassKernel, MagickFalse },
    { "Kirsch", (ssize_t) KirschKernel, MagickFalse },
    { "Rectangle", (ssize_t) RectangleKernel, MagickFalse },
    { "Square", (ssize_t) SquareKernel, MagickFalse },
    { "Diamond", (ssize_t) DiamondKernel, MagickFalse },
    { "Disk", (ssize_t) DiskKernel, MagickFalse },
    { "Plus", (ssize_t) PlusKernel, MagickFalse },
    { "Cross", (ssize_t) CrossKernel, MagickFalse },
    { "Ring", (ssize_t) RingKernel, MagickFalse },
    { "Peaks", (ssize_t) PeaksKernel, MagickFalse },
    { "Edges", (ssize_t) EdgesKernel, MagickFalse },
    { "Corners", (ssize_t) CornersKernel, MagickFalse },
    { "ThinDiagonals", (ssize_t) ThinDiagonalsKernel, MagickFalse },
    { "LineEnds", (ssize_t) LineEndsKernel, MagickFalse },
    { "LineJunctions", (ssize_t) LineJunctionsKernel, MagickFalse },
    { "Ridges", (ssize_t) RidgesKernel, MagickFalse },
    { "ConvexHull", (ssize_t) ConvexHullKernel, MagickFalse },
    { "Skeleton", (ssize_t) SkeletonKernel, MagickFalse },
    { "Chebyshev", (ssize_t) ChebyshevKernel, MagickFalse },
    { "Manhattan", (ssize_t) ManhattanKernel, MagickFalse },
    { "Euclidean", (ssize_t) EuclideanKernel, MagickFalse },
    { "User Defined", (ssize_t) UserDefinedKernel, MagickTrue }, /* internal */
    { (char *) NULL, (ssize_t) UndefinedKernel, MagickFalse }
  },
  LayerOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedLayer, MagickTrue },
    { "Coalesce", (ssize_t) CoalesceLayer, MagickFalse },
    { "CompareAny", (ssize_t) CompareAnyLayer, MagickFalse },
    { "CompareClear", (ssize_t) CompareClearLayer, MagickFalse },
    { "CompareOverlay", (ssize_t) CompareOverlayLayer, MagickFalse },
    { "Dispose", (ssize_t) DisposeLayer, MagickFalse },
    { "Optimize", (ssize_t) OptimizeLayer, MagickFalse },
    { "OptimizeFrame", (ssize_t) OptimizeImageLayer, MagickFalse },
    { "OptimizePlus", (ssize_t) OptimizePlusLayer, MagickFalse },
    { "OptimizeTransparency", (ssize_t) OptimizeTransLayer, MagickFalse },
    { "RemoveDups", (ssize_t) RemoveDupsLayer, MagickFalse },
    { "RemoveZero", (ssize_t) RemoveZeroLayer, MagickFalse },
    { "Composite", (ssize_t) CompositeLayer, MagickFalse },
    { "Merge", (ssize_t) MergeLayer, MagickFalse },
    { "Flatten", (ssize_t) FlattenLayer, MagickFalse },
    { "Mosaic", (ssize_t) MosaicLayer, MagickFalse },
    { "TrimBounds", (ssize_t) TrimBoundsLayer, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedLayer, MagickFalse }
  },
  LineCapOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedCap, MagickTrue },
    { "Butt", (ssize_t) ButtCap, MagickFalse },
    { "Round", (ssize_t) RoundCap, MagickFalse },
    { "Square", (ssize_t) SquareCap, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedCap, MagickFalse }
  },
  LineJoinOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedJoin, MagickTrue },
    { "Bevel", (ssize_t) BevelJoin, MagickFalse },
    { "Miter", (ssize_t) MiterJoin, MagickFalse },
    { "Round", (ssize_t) RoundJoin, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedJoin, MagickFalse }
  },
  ListOptions[] =
  {
    { "Align", (ssize_t) MagickAlignOptions, MagickFalse },
    { "Alpha", (ssize_t) MagickAlphaOptions, MagickFalse },
    { "Boolean", (ssize_t) MagickBooleanOptions, MagickFalse },
    { "Channel", (ssize_t) MagickChannelOptions, MagickFalse },
    { "Class", (ssize_t) MagickClassOptions, MagickFalse },
    { "ClipPath", (ssize_t) MagickClipPathOptions, MagickFalse },
    { "Coder", (ssize_t) MagickCoderOptions, MagickFalse },
    { "Color", (ssize_t) MagickColorOptions, MagickFalse },
    { "Colorspace", (ssize_t) MagickColorspaceOptions, MagickFalse },
    { "Command", (ssize_t) MagickCommandOptions, MagickFalse },
    { "Compose", (ssize_t) MagickComposeOptions, MagickFalse },
    { "Compress", (ssize_t) MagickCompressOptions, MagickFalse },
    { "Configure", (ssize_t) MagickConfigureOptions, MagickFalse },
    { "DataType", (ssize_t) MagickDataTypeOptions, MagickFalse },
    { "Debug", (ssize_t) MagickDebugOptions, MagickFalse },
    { "Decoration", (ssize_t) MagickDecorateOptions, MagickFalse },
    { "Delegate", (ssize_t) MagickDelegateOptions, MagickFalse },
    { "Direction", (ssize_t) MagickDirectionOptions, MagickFalse },
    { "Dispose", (ssize_t) MagickDisposeOptions, MagickFalse },
    { "Distort", (ssize_t) MagickDistortOptions, MagickFalse },
    { "Dither", (ssize_t) MagickDitherOptions, MagickFalse },
    { "Endian", (ssize_t) MagickEndianOptions, MagickFalse },
    { "Evaluate", (ssize_t) MagickEvaluateOptions, MagickFalse },
    { "FillRule", (ssize_t) MagickFillRuleOptions, MagickFalse },
    { "Filter", (ssize_t) MagickFilterOptions, MagickFalse },
    { "Font", (ssize_t) MagickFontOptions, MagickFalse },
    { "Format", (ssize_t) MagickFormatOptions, MagickFalse },
    { "Function", (ssize_t) MagickFunctionOptions, MagickFalse },
    { "Gravity", (ssize_t) MagickGravityOptions, MagickFalse },
    { "ImageList", (ssize_t) MagickImageListOptions, MagickFalse },
    { "Intent", (ssize_t) MagickIntentOptions, MagickFalse },
    { "Interlace", (ssize_t) MagickInterlaceOptions, MagickFalse },
    { "Interpolate", (ssize_t) MagickInterpolateOptions, MagickFalse },
    { "Kernel", (ssize_t) MagickKernelOptions, MagickFalse },
    { "Layers", (ssize_t) MagickLayerOptions, MagickFalse },
    { "LineCap", (ssize_t) MagickLineCapOptions, MagickFalse },
    { "LineJoin", (ssize_t) MagickLineJoinOptions, MagickFalse },
    { "List", (ssize_t) MagickListOptions, MagickFalse },
    { "Locale", (ssize_t) MagickLocaleOptions, MagickFalse },
    { "LogEvent", (ssize_t) MagickLogEventOptions, MagickFalse },
    { "Log", (ssize_t) MagickLogOptions, MagickFalse },
    { "Magic", (ssize_t) MagickMagicOptions, MagickFalse },
    { "Method", (ssize_t) MagickMethodOptions, MagickFalse },
    { "Metric", (ssize_t) MagickMetricOptions, MagickFalse },
    { "Mime", (ssize_t) MagickMimeOptions, MagickFalse },
    { "Mode", (ssize_t) MagickModeOptions, MagickFalse },
    { "Morphology", (ssize_t) MagickMorphologyOptions, MagickFalse },
    { "Module", (ssize_t) MagickModuleOptions, MagickFalse },
    { "Noise", (ssize_t) MagickNoiseOptions, MagickFalse },
    { "Orientation", (ssize_t) MagickOrientationOptions, MagickFalse },
    { "Policy", (ssize_t) MagickPolicyOptions, MagickFalse },
    { "PolicyDomain", (ssize_t) MagickPolicyDomainOptions, MagickFalse },
    { "PolicyRights", (ssize_t) MagickPolicyRightsOptions, MagickFalse },
    { "Preview", (ssize_t) MagickPreviewOptions, MagickFalse },
    { "Primitive", (ssize_t) MagickPrimitiveOptions, MagickFalse },
    { "QuantumFormat", (ssize_t) MagickQuantumFormatOptions, MagickFalse },
    { "Resource", (ssize_t) MagickResourceOptions, MagickFalse },
    { "SparseColor", (ssize_t) MagickSparseColorOptions, MagickFalse },
    { "Storage", (ssize_t) MagickStorageOptions, MagickFalse },
    { "Stretch", (ssize_t) MagickStretchOptions, MagickFalse },
    { "Style", (ssize_t) MagickStyleOptions, MagickFalse },
    { "Threshold", (ssize_t) MagickThresholdOptions, MagickFalse },
    { "Type", (ssize_t) MagickTypeOptions, MagickFalse },
    { "Units", (ssize_t) MagickResolutionOptions, MagickFalse },
    { "Undefined", (ssize_t) MagickUndefinedOptions, MagickTrue },
    { "Validate", (ssize_t) MagickValidateOptions, MagickFalse },
    { "VirtualPixel", (ssize_t) MagickVirtualPixelOptions, MagickFalse },
    { (char *) NULL, (ssize_t) MagickUndefinedOptions, MagickFalse }
  },
  LogEventOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedEvents, MagickTrue },
    { "All", (ssize_t) (AllEvents &~ TraceEvent), MagickFalse },
    { "Annotate", (ssize_t) AnnotateEvent, MagickFalse },
    { "Blob", (ssize_t) BlobEvent, MagickFalse },
    { "Cache", (ssize_t) CacheEvent, MagickFalse },
    { "Coder", (ssize_t) CoderEvent, MagickFalse },
    { "Configure", (ssize_t) ConfigureEvent, MagickFalse },
    { "Deprecate", (ssize_t) DeprecateEvent, MagickFalse },
    { "Draw", (ssize_t) DrawEvent, MagickFalse },
    { "Exception", (ssize_t) ExceptionEvent, MagickFalse },
    { "Locale", (ssize_t) LocaleEvent, MagickFalse },
    { "Module", (ssize_t) ModuleEvent, MagickFalse },
    { "None", (ssize_t) NoEvents, MagickFalse },
    { "Policy", (ssize_t) PolicyEvent, MagickFalse },
    { "Resource", (ssize_t) ResourceEvent, MagickFalse },
    { "Trace", (ssize_t) TraceEvent, MagickFalse },
    { "Transform", (ssize_t) TransformEvent, MagickFalse },
    { "User", (ssize_t) UserEvent, MagickFalse },
    { "Wand", (ssize_t) WandEvent, MagickFalse },
    { "X11", (ssize_t) X11Event, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedEvents, MagickFalse }
  },
  MetricOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedMetric, MagickTrue },
    { "AE", (ssize_t) AbsoluteErrorMetric, MagickFalse },
    { "MAE", (ssize_t) MeanAbsoluteErrorMetric, MagickFalse },
    { "MEPP", (ssize_t) MeanErrorPerPixelMetric, MagickFalse },
    { "MSE", (ssize_t) MeanSquaredErrorMetric, MagickFalse },
    { "PAE", (ssize_t) PeakAbsoluteErrorMetric, MagickFalse },
    { "PSNR", (ssize_t) PeakSignalToNoiseRatioMetric, MagickFalse },
    { "RMSE", (ssize_t) RootMeanSquaredErrorMetric, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedMetric, MagickFalse }
  },
  MethodOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedMethod, MagickTrue },
    { "FillToBorder", (ssize_t) FillToBorderMethod, MagickFalse },
    { "Floodfill", (ssize_t) FloodfillMethod, MagickFalse },
    { "Point", (ssize_t) PointMethod, MagickFalse },
    { "Replace", (ssize_t) ReplaceMethod, MagickFalse },
    { "Reset", (ssize_t) ResetMethod, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedMethod, MagickFalse }
  },
  ModeOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedMode, MagickTrue },
    { "Concatenate", (ssize_t) ConcatenateMode, MagickFalse },
    { "Frame", (ssize_t) FrameMode, MagickFalse },
    { "Unframe", (ssize_t) UnframeMode, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedMode, MagickFalse }
  },
  MorphologyOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedMorphology, MagickTrue },
    { "Correlate", (ssize_t) CorrelateMorphology, MagickFalse },
    { "Convolve", (ssize_t) ConvolveMorphology, MagickFalse },
    { "Dilate", (ssize_t) DilateMorphology, MagickFalse },
    { "Erode", (ssize_t) ErodeMorphology, MagickFalse },
    { "Close", (ssize_t) CloseMorphology, MagickFalse },
    { "Open", (ssize_t) OpenMorphology, MagickFalse },
    { "DilateIntensity", (ssize_t) DilateIntensityMorphology, MagickFalse },
    { "ErodeIntensity", (ssize_t) ErodeIntensityMorphology, MagickFalse },
    { "CloseIntensity", (ssize_t) CloseIntensityMorphology, MagickFalse },
    { "OpenIntensity", (ssize_t) OpenIntensityMorphology, MagickFalse },
    { "DilateI", (ssize_t) DilateIntensityMorphology, MagickFalse },
    { "ErodeI", (ssize_t) ErodeIntensityMorphology, MagickFalse },
    { "CloseI", (ssize_t) CloseIntensityMorphology, MagickFalse },
    { "OpenI", (ssize_t) OpenIntensityMorphology, MagickFalse },
    { "Smooth", (ssize_t) SmoothMorphology, MagickFalse },
    { "EdgeOut", (ssize_t) EdgeOutMorphology, MagickFalse },
    { "EdgeIn", (ssize_t) EdgeInMorphology, MagickFalse },
    { "Edge", (ssize_t) EdgeMorphology, MagickFalse },
    { "TopHat", (ssize_t) TopHatMorphology, MagickFalse },
    { "BottomHat", (ssize_t) BottomHatMorphology, MagickFalse },
    { "Distance", (ssize_t) DistanceMorphology, MagickFalse },
    { "HitAndMiss", (ssize_t) HitAndMissMorphology, MagickFalse },
    { "Thinning", (ssize_t) ThinningMorphology, MagickFalse },
    { "Thicken", (ssize_t) ThickenMorphology, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedMorphology, MagickFalse }
  },
  NoiseOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedNoise, MagickTrue },
    { "Gaussian", (ssize_t) (ssize_t) GaussianNoise, MagickFalse },
    { "Impulse", (ssize_t) ImpulseNoise, MagickFalse },
    { "Laplacian", (ssize_t) LaplacianNoise, MagickFalse },
    { "Multiplicative", (ssize_t) MultiplicativeGaussianNoise, MagickFalse },
    { "Poisson", (ssize_t) PoissonNoise, MagickFalse },
    { "Random", (ssize_t) RandomNoise, MagickFalse },
    { "Uniform", (ssize_t) UniformNoise, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedNoise, MagickFalse }
  },
  OrientationOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedOrientation, MagickTrue },
    { "TopLeft", (ssize_t) TopLeftOrientation, MagickFalse },
    { "TopRight", (ssize_t) TopRightOrientation, MagickFalse },
    { "BottomRight", (ssize_t) BottomRightOrientation, MagickFalse },
    { "BottomLeft", (ssize_t) BottomLeftOrientation, MagickFalse },
    { "LeftTop", (ssize_t) LeftTopOrientation, MagickFalse },
    { "RightTop", (ssize_t) RightTopOrientation, MagickFalse },
    { "RightBottom", (ssize_t) RightBottomOrientation, MagickFalse },
    { "LeftBottom", (ssize_t) LeftBottomOrientation, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedOrientation, MagickFalse }
  },
  PolicyDomainOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedPolicyDomain, MagickTrue },
    { "Coder", (ssize_t) CoderPolicyDomain, MagickFalse },
    { "Delegate", (ssize_t) DelegatePolicyDomain, MagickFalse },
    { "Filter", (ssize_t) FilterPolicyDomain, MagickFalse },
    { "Path", (ssize_t) PathPolicyDomain, MagickFalse },
    { "Resource", (ssize_t) ResourcePolicyDomain, MagickFalse },
    { "System", (ssize_t) SystemPolicyDomain, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedPolicyDomain, MagickFalse }
  },
  PolicyRightsOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedPolicyRights, MagickTrue },
    { "None", (ssize_t) NoPolicyRights, MagickFalse },
    { "Read", (ssize_t) ReadPolicyRights, MagickFalse },
    { "Write", (ssize_t) WritePolicyRights, MagickFalse },
    { "Execute", (ssize_t) ExecutePolicyRights, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedPolicyRights, MagickFalse }
  },
  PreviewOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedPreview, MagickTrue },
    { "AddNoise", (ssize_t) AddNoisePreview, MagickFalse },
    { "Blur", (ssize_t) BlurPreview, MagickFalse },
    { "Brightness", (ssize_t) BrightnessPreview, MagickFalse },
    { "Charcoal", (ssize_t) CharcoalDrawingPreview, MagickFalse },
    { "Despeckle", (ssize_t) DespecklePreview, MagickFalse },
    { "Dull", (ssize_t) DullPreview, MagickFalse },
    { "EdgeDetect", (ssize_t) EdgeDetectPreview, MagickFalse },
    { "Gamma", (ssize_t) GammaPreview, MagickFalse },
    { "Grayscale", (ssize_t) GrayscalePreview, MagickFalse },
    { "Hue", (ssize_t) HuePreview, MagickFalse },
    { "Implode", (ssize_t) ImplodePreview, MagickFalse },
    { "JPEG", (ssize_t) JPEGPreview, MagickFalse },
    { "OilPaint", (ssize_t) OilPaintPreview, MagickFalse },
    { "Quantize", (ssize_t) QuantizePreview, MagickFalse },
    { "Raise", (ssize_t) RaisePreview, MagickFalse },
    { "ReduceNoise", (ssize_t) ReduceNoisePreview, MagickFalse },
    { "Roll", (ssize_t) RollPreview, MagickFalse },
    { "Rotate", (ssize_t) RotatePreview, MagickFalse },
    { "Saturation", (ssize_t) SaturationPreview, MagickFalse },
    { "Segment", (ssize_t) SegmentPreview, MagickFalse },
    { "Shade", (ssize_t) ShadePreview, MagickFalse },
    { "Sharpen", (ssize_t) SharpenPreview, MagickFalse },
    { "Shear", (ssize_t) ShearPreview, MagickFalse },
    { "Solarize", (ssize_t) SolarizePreview, MagickFalse },
    { "Spiff", (ssize_t) SpiffPreview, MagickFalse },
    { "Spread", (ssize_t) SpreadPreview, MagickFalse },
    { "Swirl", (ssize_t) SwirlPreview, MagickFalse },
    { "Threshold", (ssize_t) ThresholdPreview, MagickFalse },
    { "Wave", (ssize_t) WavePreview, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedPreview, MagickFalse }
  },
  PrimitiveOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedPrimitive, MagickTrue },
    { "Arc", (ssize_t) ArcPrimitive, MagickFalse },
    { "Bezier", (ssize_t) BezierPrimitive, MagickFalse },
    { "Circle", (ssize_t) CirclePrimitive, MagickFalse },
    { "Color", (ssize_t) ColorPrimitive, MagickFalse },
    { "Ellipse", (ssize_t) EllipsePrimitive, MagickFalse },
    { "Image", (ssize_t) ImagePrimitive, MagickFalse },
    { "Line", (ssize_t) LinePrimitive, MagickFalse },
    { "Matte", (ssize_t) MattePrimitive, MagickFalse },
    { "Path", (ssize_t) PathPrimitive, MagickFalse },
    { "Point", (ssize_t) PointPrimitive, MagickFalse },
    { "Polygon", (ssize_t) PolygonPrimitive, MagickFalse },
    { "Polyline", (ssize_t) PolylinePrimitive, MagickFalse },
    { "Rectangle", (ssize_t) RectanglePrimitive, MagickFalse },
    { "roundRectangle", (ssize_t) RoundRectanglePrimitive, MagickFalse },
    { "Text", (ssize_t) TextPrimitive, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedPrimitive, MagickFalse }
  },
  QuantumFormatOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedQuantumFormat, MagickTrue },
    { "FloatingPoint", (ssize_t) FloatingPointQuantumFormat, MagickFalse },
    { "Signed", (ssize_t) SignedQuantumFormat, MagickFalse },
    { "Unsigned", (ssize_t) UnsignedQuantumFormat, MagickFalse },
    { (char *) NULL, (ssize_t) FloatingPointQuantumFormat, MagickFalse }
  },
  ResolutionOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedResolution, MagickTrue },
    { "PixelsPerInch", (ssize_t) PixelsPerInchResolution, MagickFalse },
    { "PixelsPerCentimeter", (ssize_t) PixelsPerCentimeterResolution, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedResolution, MagickFalse }
  },
  ResourceOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedResource, MagickTrue },
    { "Area", (ssize_t) AreaResource, MagickFalse },
    { "Disk", (ssize_t) DiskResource, MagickFalse },
    { "File", (ssize_t) FileResource, MagickFalse },
    { "Map", (ssize_t) MapResource, MagickFalse },
    { "Memory", (ssize_t) MemoryResource, MagickFalse },
    { "Thread", (ssize_t) ThreadResource, MagickFalse },
    { "Time", (ssize_t) TimeResource, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedResource, MagickFalse }
  },
  SparseColorOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedDistortion, MagickTrue },
    { "Barycentric", (ssize_t) BarycentricColorInterpolate, MagickFalse },
    { "Bilinear", (ssize_t) BilinearColorInterpolate, MagickFalse },
    { "Shepards", (ssize_t) ShepardsColorInterpolate, MagickFalse },
    { "Voronoi", (ssize_t) VoronoiColorInterpolate, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedResource, MagickFalse }
  },
  StorageOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedPixel, MagickTrue },
    { "Char", (ssize_t) CharPixel, MagickFalse },
    { "Double", (ssize_t) DoublePixel, MagickFalse },
    { "Float", (ssize_t) FloatPixel, MagickFalse },
    { "Integer", (ssize_t) IntegerPixel, MagickFalse },
    { "Long", (ssize_t) LongPixel, MagickFalse },
    { "Quantum", (ssize_t) QuantumPixel, MagickFalse },
    { "Short", (ssize_t) ShortPixel, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedResource, MagickFalse }
  },
  StretchOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedStretch, MagickTrue },
    { "Any", (ssize_t) AnyStretch, MagickFalse },
    { "Condensed", (ssize_t) CondensedStretch, MagickFalse },
    { "Expanded", (ssize_t) ExpandedStretch, MagickFalse },
    { "ExtraCondensed", (ssize_t) ExtraCondensedStretch, MagickFalse },
    { "ExtraExpanded", (ssize_t) ExtraExpandedStretch, MagickFalse },
    { "Normal", (ssize_t) NormalStretch, MagickFalse },
    { "SemiCondensed", (ssize_t) SemiCondensedStretch, MagickFalse },
    { "SemiExpanded", (ssize_t) SemiExpandedStretch, MagickFalse },
    { "UltraCondensed", (ssize_t) UltraCondensedStretch, MagickFalse },
    { "UltraExpanded", (ssize_t) UltraExpandedStretch, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedStretch, MagickFalse }
  },
  StyleOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedStyle, MagickTrue },
    { "Any", (ssize_t) AnyStyle, MagickFalse },
    { "Italic", (ssize_t) ItalicStyle, MagickFalse },
    { "Normal", (ssize_t) NormalStyle, MagickFalse },
    { "Oblique", (ssize_t) ObliqueStyle, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedStyle, MagickFalse }
  },
  TypeOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedType, MagickTrue },
    { "Bilevel", (ssize_t) BilevelType, MagickFalse },
    { "ColorSeparation", (ssize_t) ColorSeparationType, MagickFalse },
    { "ColorSeparationMatte", (ssize_t) ColorSeparationMatteType, MagickFalse },
    { "Grayscale", (ssize_t) GrayscaleType, MagickFalse },
    { "GrayscaleMatte", (ssize_t) GrayscaleMatteType, MagickFalse },
    { "Optimize", (ssize_t) OptimizeType, MagickFalse },
    { "Palette", (ssize_t) PaletteType, MagickFalse },
    { "PaletteBilevelMatte", (ssize_t) PaletteBilevelMatteType, MagickFalse },
    { "PaletteMatte", (ssize_t) PaletteMatteType, MagickFalse },
    { "TrueColorMatte", (ssize_t) TrueColorMatteType, MagickFalse },
    { "TrueColor", (ssize_t) TrueColorType, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedType, MagickFalse }
  },
  ValidateOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedValidate, MagickTrue },
    { "All", (ssize_t) AllValidate, MagickFalse },
    { "Compare", (ssize_t) CompareValidate, MagickFalse },
    { "Composite", (ssize_t) CompositeValidate, MagickFalse },
    { "Convert", (ssize_t) ConvertValidate, MagickFalse },
    { "FormatsInMemory", (ssize_t) FormatsInMemoryValidate, MagickFalse },
    { "FormatsOnDisk", (ssize_t) FormatsOnDiskValidate, MagickFalse },
    { "Identify", (ssize_t) IdentifyValidate, MagickFalse },
    { "ImportExport", (ssize_t) ImportExportValidate, MagickFalse },
    { "Montage", (ssize_t) MontageValidate, MagickFalse },
    { "Stream", (ssize_t) StreamValidate, MagickFalse },
    { "None", (ssize_t) NoValidate, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedValidate, MagickFalse }
  },
  VirtualPixelOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedVirtualPixelMethod, MagickTrue },
    { "Background", (ssize_t) BackgroundVirtualPixelMethod, MagickFalse },
    { "Black", (ssize_t) BlackVirtualPixelMethod, MagickFalse },
    { "Constant", (ssize_t) BackgroundVirtualPixelMethod, MagickTrue }, /* deprecated */
    { "CheckerTile", (ssize_t) CheckerTileVirtualPixelMethod, MagickFalse },
    { "Dither", (ssize_t) DitherVirtualPixelMethod, MagickFalse },
    { "Edge", (ssize_t) EdgeVirtualPixelMethod, MagickFalse },
    { "Gray", (ssize_t) GrayVirtualPixelMethod, MagickFalse },
    { "HorizontalTile", (ssize_t) HorizontalTileVirtualPixelMethod, MagickFalse },
    { "HorizontalTileEdge", (ssize_t) HorizontalTileEdgeVirtualPixelMethod, MagickFalse },
    { "Mirror", (ssize_t) MirrorVirtualPixelMethod, MagickFalse },
    { "Random", (ssize_t) RandomVirtualPixelMethod, MagickFalse },
    { "Tile", (ssize_t) TileVirtualPixelMethod, MagickFalse },
    { "Transparent", (ssize_t) TransparentVirtualPixelMethod, MagickFalse },
    { "VerticalTile", (ssize_t) VerticalTileVirtualPixelMethod, MagickFalse },
    { "VerticalTileEdge", (ssize_t) VerticalTileEdgeVirtualPixelMethod, MagickFalse },
    { "White", (ssize_t) WhiteVirtualPixelMethod, MagickFalse },
    { (char *) NULL, (ssize_t) UndefinedVirtualPixelMethod, MagickFalse }
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
%   G e t M a g i c k O p t i o n s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickOptions() returns a list of values.
%
%  The format of the GetMagickOptions method is:
%
%      const char **GetMagickOptions(const MagickOption value)
%
%  A description of each parameter follows:
%
%    o value: the value.
%
*/

static const OptionInfo *GetOptionInfo(const MagickOption option)
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
    case MagickImageListOptions: return(ImageListOptions);
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

MagickExport char **GetMagickOptions(const MagickOption value)
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
%     I s M a g i c k O p t i o n                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickOption() returns MagickTrue if the option begins with a - or + and
%  the first character that follows is alphanumeric.
%
%  The format of the IsMagickOption method is:
%
%      MagickBooleanType IsMagickOption(const char *option)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
*/
MagickExport MagickBooleanType IsMagickOption(const char *option)
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
%   M a g i c k O p t i o n T o M n e m o n i c                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOptionToMnemonic() returns an enumerated value as a mnemonic.
%
%  The format of the MagickOptionToMnemonic method is:
%
%      const char *MagickOptionToMnemonic(const MagickOption option,
%        const ssize_t type)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
%    o type: one or more values separated by commas.
%
*/
MagickExport const char *MagickOptionToMnemonic(const MagickOption option,
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
%   L i s t M a g i c k O p t i o n s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMagickOptions() lists the contents of enumerated option type(s).
%
%  The format of the ListMagickOptions method is:
%
%      MagickBooleanType ListMagickOptions(FILE *file,const MagickOption option,
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
MagickExport MagickBooleanType ListMagickOptions(FILE *file,
  const MagickOption option,ExceptionInfo *magick_unused(exception))
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
  ssize_t
    channel;

  register ssize_t
    i;

  channel=ParseMagickOption(MagickChannelOptions,MagickTrue,channels);
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
        /*
          More channel flags follow shorthand.  For example "RGB,sync"
          Gather the additional channel flags and merge with shorthand
        */
        ssize_t
          more_channel;
        more_channel=ParseMagickOption(MagickChannelOptions,MagickTrue,
                             channels+i+1);
        if (more_channel < 0)
          return(more_channel);
        channel |= more_channel;
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
%   P a r s e M a g i c k O p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseMagickOption() parses a string and returns an enumerated option type(s).
%
%  The format of the ParseMagickOption method is:
%
%      ssize_t ParseMagickOption(const MagickOption option,
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
MagickExport ssize_t ParseMagickOption(const MagickOption option,
  const MagickBooleanType list,const char *options)
{
  char
    token[MaxTextExtent];

  const OptionInfo
    *option_info;

  int
    sentinel;

  ssize_t
    option_types;

  MagickBooleanType
    negate;

  register char
    *q;

  register const char
    *p;

  register ssize_t
    i;

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

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
    { "Undefined", (long) UndefinedAlign, MagickTrue },
    { "Center", (long) CenterAlign, MagickFalse },
    { "End", (long) RightAlign, MagickFalse },
    { "Left", (long) LeftAlign, MagickFalse },
    { "Middle", (long) CenterAlign, MagickFalse },
    { "Right", (long) RightAlign, MagickFalse },
    { "Start", (long) LeftAlign, MagickFalse },
    { (char *) NULL, (long) UndefinedAlign, MagickFalse }
  },
  AlphaOptions[] =
  {
    { "Undefined", (long) UndefinedAlphaChannel, MagickTrue },
    { "Activate", (long) ActivateAlphaChannel, MagickFalse },
    { "Background", (long) BackgroundAlphaChannel, MagickFalse },
    { "Copy", (long) CopyAlphaChannel, MagickFalse },
    { "Deactivate", (long) DeactivateAlphaChannel, MagickFalse },
    { "Extract", (long) ExtractAlphaChannel, MagickFalse },
    { "Off", (long) DeactivateAlphaChannel, MagickFalse },
    { "On", (long) ActivateAlphaChannel, MagickFalse },
    { "Opaque", (long) OpaqueAlphaChannel, MagickFalse },
    { "Set", (long) SetAlphaChannel, MagickFalse },
    { "Shape", (long) ShapeAlphaChannel, MagickFalse },
    { "Reset", (long) SetAlphaChannel, MagickTrue }, /* deprecated */
    { "Transparent", (long) TransparentAlphaChannel, MagickFalse },
    { (char *) NULL, (long) UndefinedAlphaChannel, MagickFalse }
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
    { "Undefined", (long) UndefinedChannel, MagickTrue },
    { "All", (long) AllChannels, MagickFalse },
    { "Alpha", (long) OpacityChannel, MagickFalse },
    { "Black", (long) BlackChannel, MagickFalse },
    { "Blue", (long) BlueChannel, MagickFalse },
    { "Cyan", (long) CyanChannel, MagickFalse },
    { "Default", (long) DefaultChannels, MagickFalse },
    { "Gray", (long) GrayChannel, MagickFalse },
    { "Green", (long) GreenChannel, MagickFalse },
    { "Hue", (long) RedChannel, MagickFalse },
    { "Index", (long) IndexChannel, MagickFalse },
    { "Lightness", (long) BlueChannel, MagickFalse },
    { "Luminance", (long) BlueChannel, MagickFalse },
    { "Luminosity", (long) BlueChannel, MagickFalse },  /* deprecated */
    { "Magenta", (long) MagentaChannel, MagickFalse },
    { "Matte", (long) OpacityChannel, MagickFalse },
    { "Opacity", (long) OpacityChannel, MagickFalse },
    { "Red", (long) RedChannel, MagickFalse },
    { "Saturation", (long) GreenChannel, MagickFalse },
    { "Yellow", (long) YellowChannel, MagickFalse },
    { "Sync", (long) SyncChannels, MagickFalse },   /* special channel flag */
    { (char *) NULL, (long) UndefinedChannel, MagickFalse }
  },
  ClassOptions[] =
  {
    { "Undefined", (long) UndefinedClass, MagickTrue },
    { "DirectClass", (long) DirectClass, MagickFalse },
    { "PseudoClass", (long) PseudoClass, MagickFalse },
    { (char *) NULL, (long) UndefinedClass, MagickFalse }
  },
  ClipPathOptions[] =
  {
    { "Undefined", (long) UndefinedPathUnits, MagickTrue },
    { "ObjectBoundingBox", (long) ObjectBoundingBox, MagickFalse },
    { "UserSpace", (long) UserSpace, MagickFalse },
    { "UserSpaceOnUse", (long) UserSpaceOnUse, MagickFalse },
    { (char *) NULL, (long) UndefinedPathUnits, MagickFalse }
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
    { "+auto-gamma", 0L, MagickTrue },   /* under development */
    { "-auto-gamma", 0L, MagickTrue },   /* under development */
    { "+auto-level", 0L, MagickTrue },   /* under development */
    { "-auto-level", 0L, MagickTrue },   /* under development */
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
    { (char *) NULL, (long) 0L, MagickFalse }
  },
  ComposeOptions[] =
  {
    { "Undefined", (long) UndefinedCompositeOp, MagickTrue },
    { "Add", (long) AddCompositeOp, MagickFalse },
    { "Atop", (long) AtopCompositeOp, MagickFalse },
    { "Blend", (long) BlendCompositeOp, MagickFalse },
    { "Blur", (long) BlurCompositeOp, MagickFalse },
    { "Bumpmap", (long) BumpmapCompositeOp, MagickFalse },
    { "ChangeMask", (long) ChangeMaskCompositeOp, MagickFalse },
    { "Clear", (long) ClearCompositeOp, MagickFalse },
    { "ColorBurn", (long) ColorBurnCompositeOp, MagickFalse },
    { "ColorDodge", (long) ColorDodgeCompositeOp, MagickFalse },
    { "Colorize", (long) ColorizeCompositeOp, MagickFalse },
    { "CopyBlack", (long) CopyBlackCompositeOp, MagickFalse },
    { "CopyBlue", (long) CopyBlueCompositeOp, MagickFalse },
    { "CopyCyan", (long) CopyCyanCompositeOp, MagickFalse },
    { "CopyGreen", (long) CopyGreenCompositeOp, MagickFalse },
    { "Copy", (long) CopyCompositeOp, MagickFalse },
    { "CopyMagenta", (long) CopyMagentaCompositeOp, MagickFalse },
    { "CopyOpacity", (long) CopyOpacityCompositeOp, MagickFalse },
    { "CopyRed", (long) CopyRedCompositeOp, MagickFalse },
    { "CopyYellow", (long) CopyYellowCompositeOp, MagickFalse },
    { "Darken", (long) DarkenCompositeOp, MagickFalse },
    { "Divide", (long) DivideCompositeOp, MagickFalse },
    { "Dst", (long) DstCompositeOp, MagickFalse },
    { "Difference", (long) DifferenceCompositeOp, MagickFalse },
    { "Displace", (long) DisplaceCompositeOp, MagickFalse },
    { "Dissolve", (long) DissolveCompositeOp, MagickFalse },
    { "Distort", (long) DistortCompositeOp, MagickFalse },
    { "DstAtop", (long) DstAtopCompositeOp, MagickFalse },
    { "DstIn", (long) DstInCompositeOp, MagickFalse },
    { "DstOut", (long) DstOutCompositeOp, MagickFalse },
    { "DstOver", (long) DstOverCompositeOp, MagickFalse },
    { "Dst", (long) DstCompositeOp, MagickFalse },
    { "Exclusion", (long) ExclusionCompositeOp, MagickFalse },
    { "HardLight", (long) HardLightCompositeOp, MagickFalse },
    { "Hue", (long) HueCompositeOp, MagickFalse },
    { "In", (long) InCompositeOp, MagickFalse },
    { "Lighten", (long) LightenCompositeOp, MagickFalse },
    { "LinearBurn", (long) LinearBurnCompositeOp, MagickFalse },
    { "LinearDodge", (long) LinearDodgeCompositeOp, MagickFalse },
    { "LinearLight", (long) LinearLightCompositeOp, MagickFalse },
    { "Luminize", (long) LuminizeCompositeOp, MagickFalse },
    { "Mathematics", (long) MathematicsCompositeOp, MagickFalse },
    { "Minus", (long) MinusCompositeOp, MagickFalse },
    { "Modulate", (long) ModulateCompositeOp, MagickFalse },
    { "Multiply", (long) MultiplyCompositeOp, MagickFalse },
    { "None", (long) NoCompositeOp, MagickFalse },
    { "Out", (long) OutCompositeOp, MagickFalse },
    { "Overlay", (long) OverlayCompositeOp, MagickFalse },
    { "Over", (long) OverCompositeOp, MagickFalse },
    { "PegtopLight", (long) PegtopLightCompositeOp, MagickFalse },
    { "PinLight", (long) PinLightCompositeOp, MagickFalse },
    { "Plus", (long) PlusCompositeOp, MagickFalse },
    { "Replace", (long) ReplaceCompositeOp, MagickFalse },
    { "Saturate", (long) SaturateCompositeOp, MagickFalse },
    { "Screen", (long) ScreenCompositeOp, MagickFalse },
    { "SoftLight", (long) SoftLightCompositeOp, MagickFalse },
    { "Src", (long) SrcCompositeOp, MagickFalse },
    { "SrcAtop", (long) SrcAtopCompositeOp, MagickFalse },
    { "SrcIn", (long) SrcInCompositeOp, MagickFalse },
    { "SrcOut", (long) SrcOutCompositeOp, MagickFalse },
    { "SrcOver", (long) SrcOverCompositeOp, MagickFalse },
    { "Src", (long) SrcCompositeOp, MagickFalse },
    { "Subtract", (long) SubtractCompositeOp, MagickFalse },
    { "Threshold", (long) ThresholdCompositeOp, MagickTrue }, /* depreciate */
    { "VividLight", (long) VividLightCompositeOp, MagickFalse },
    { "Xor", (long) XorCompositeOp, MagickFalse },
    { (char *) NULL, (long) UndefinedCompositeOp, MagickFalse }
  },
  CompressOptions[] =
  {
    { "Undefined", (long) UndefinedCompression, MagickTrue },
    { "B44", (long) B44Compression, MagickFalse },
    { "B44A", (long) B44ACompression, MagickFalse },
    { "BZip", (long) BZipCompression, MagickFalse },
    { "DXT1", (long) DXT1Compression, MagickFalse },
    { "DXT3", (long) DXT3Compression, MagickFalse },
    { "DXT5", (long) DXT5Compression, MagickFalse },
    { "Fax", (long) FaxCompression, MagickFalse },
    { "Group4", (long) Group4Compression, MagickFalse },
    { "JPEG", (long) JPEGCompression, MagickFalse },
    { "JPEG2000", (long) JPEG2000Compression, MagickFalse },
    { "Lossless", (long) LosslessJPEGCompression, MagickFalse },
    { "LosslessJPEG", (long) LosslessJPEGCompression, MagickFalse },
    { "LZW", (long) LZWCompression, MagickFalse },
    { "None", (long) NoCompression, MagickFalse },
    { "Piz", (long) PizCompression, MagickFalse },
    { "Pxr24", (long) Pxr24Compression, MagickFalse },
    { "RLE", (long) RLECompression, MagickFalse },
    { "Zip", (long) ZipCompression, MagickFalse },
    { "RunlengthEncoded", (long) RLECompression, MagickFalse },
    { "ZipS", (long) ZipSCompression, MagickFalse },
    { (char *) NULL, (long) UndefinedCompression, MagickFalse }
  },
  ColorspaceOptions[] =
  {
    { "Undefined", (long) UndefinedColorspace, MagickTrue },
    { "CMY", (long) CMYColorspace, MagickFalse },
    { "CMYK", (long) CMYKColorspace, MagickFalse },
    { "Gray", (long) GRAYColorspace, MagickFalse },
    { "HSB", (long) HSBColorspace, MagickFalse },
    { "HSL", (long) HSLColorspace, MagickFalse },
    { "HWB", (long) HWBColorspace, MagickFalse },
    { "Lab", (long) LabColorspace, MagickFalse },
    { "Log", (long) LogColorspace, MagickFalse },
    { "OHTA", (long) OHTAColorspace, MagickFalse },
    { "Rec601Luma", (long) Rec601LumaColorspace, MagickFalse },
    { "Rec601YCbCr", (long) Rec601YCbCrColorspace, MagickFalse },
    { "Rec709Luma", (long) Rec709LumaColorspace, MagickFalse },
    { "Rec709YCbCr", (long) Rec709YCbCrColorspace, MagickFalse },
    { "RGB", (long) RGBColorspace, MagickFalse },
    { "sRGB", (long) sRGBColorspace, MagickFalse },
    { "Transparent", (long) TransparentColorspace, MagickFalse },
    { "XYZ", (long) XYZColorspace, MagickFalse },
    { "YCbCr", (long) YCbCrColorspace, MagickFalse },
    { "YCC", (long) YCCColorspace, MagickFalse },
    { "YIQ", (long) YIQColorspace, MagickFalse },
    { "YPbPr", (long) YPbPrColorspace, MagickFalse },
    { "YUV", (long) YUVColorspace, MagickFalse },
    { (char *) NULL, (long) UndefinedColorspace, MagickFalse }
  },
  DataTypeOptions[] =
  {
    { "Undefined", (long) UndefinedData, MagickTrue },
    { "Byte", (long) ByteData, MagickFalse },
    { "Long", (long) LongData, MagickFalse },
    { "Short", (long) ShortData, MagickFalse },
    { "String", (long) StringData, MagickFalse },
    { (char *) NULL, (long) UndefinedData, MagickFalse }
  },
  DecorateOptions[] =
  {
    { "Undefined", (long) UndefinedDecoration, MagickTrue },
    { "LineThrough", (long) LineThroughDecoration, MagickFalse },
    { "None", (long) NoDecoration, MagickFalse },
    { "Overline", (long) OverlineDecoration, MagickFalse },
    { "Underline", (long) UnderlineDecoration, MagickFalse },
    { (char *) NULL, (long) UndefinedDecoration, MagickFalse }
  },
  DisposeOptions[] =
  {
    { "Background", (long) BackgroundDispose, MagickFalse },
    { "None", (long) NoneDispose, MagickFalse },
    { "Previous", (long) PreviousDispose, MagickFalse },
    { "Undefined", (long) UndefinedDispose, MagickFalse },
    { "0", (long) UndefinedDispose, MagickFalse },
    { "1", (long) NoneDispose, MagickFalse },
    { "2", (long) BackgroundDispose, MagickFalse },
    { "3", (long) PreviousDispose, MagickFalse },
    { (char *) NULL, (long) UndefinedDispose, MagickFalse }
  },
  DistortOptions[] =
  {
    { "Undefined", (long) UndefinedDistortion, MagickTrue },
    { "Affine", (long) AffineDistortion, MagickFalse },
    { "AffineProjection", (long) AffineProjectionDistortion, MagickFalse },
    { "ScaleRotateTranslate", (long) ScaleRotateTranslateDistortion, MagickFalse },
    { "SRT", (long) ScaleRotateTranslateDistortion, MagickFalse },
    { "Perspective", (long) PerspectiveDistortion, MagickFalse },
    { "PerspectiveProjection", (long) PerspectiveProjectionDistortion, MagickFalse },
    { "Bilinear", (long) BilinearForwardDistortion, MagickTrue },
    { "BilinearForward", (long) BilinearForwardDistortion, MagickFalse },
    { "BilinearReverse", (long) BilinearReverseDistortion, MagickFalse },
    { "Polynomial", (long) PolynomialDistortion, MagickFalse },
    { "Arc", (long) ArcDistortion, MagickFalse },
    { "Polar", (long) PolarDistortion, MagickFalse },
    { "DePolar", (long) DePolarDistortion, MagickFalse },
    { "Barrel", (long) BarrelDistortion, MagickFalse },
    { "BarrelInverse", (long) BarrelInverseDistortion, MagickFalse },
    { "Shepards", (long) ShepardsDistortion, MagickFalse },
    { (char *) NULL, (long) UndefinedDistortion, MagickFalse }
  },
  DitherOptions[] =
  {
    { "Undefined", (long) UndefinedDitherMethod, MagickTrue },
    { "None", (long) NoDitherMethod, MagickFalse },
    { "FloydSteinberg", (long) FloydSteinbergDitherMethod, MagickFalse },
    { "Riemersma", (long) RiemersmaDitherMethod, MagickFalse },
    { (char *) NULL, (long) UndefinedEndian, MagickFalse }
  },
  EndianOptions[] =
  {
    { "Undefined", (long) UndefinedEndian, MagickTrue },
    { "LSB", (long) LSBEndian, MagickFalse },
    { "MSB", (long) MSBEndian, MagickFalse },
    { (char *) NULL, (long) UndefinedEndian, MagickFalse }
  },
  EvaluateOptions[] =
  {
    { "Undefined", (long) UndefinedEvaluateOperator, MagickTrue },
    { "Add", (long) AddEvaluateOperator, MagickFalse },
    { "AddModulus", (long) AddModulusEvaluateOperator, MagickFalse },
    { "And", (long) AndEvaluateOperator, MagickFalse },
    { "Cos", (long) CosineEvaluateOperator, MagickFalse },
    { "Cosine", (long) CosineEvaluateOperator, MagickFalse },
    { "Divide", (long) DivideEvaluateOperator, MagickFalse },
    { "GaussianNoise", (long) GaussianNoiseEvaluateOperator, MagickFalse },
    { "ImpulseNoise", (long) ImpulseNoiseEvaluateOperator, MagickFalse },
    { "LaplacianNoise", (long) LaplacianNoiseEvaluateOperator, MagickFalse },
    { "LeftShift", (long) LeftShiftEvaluateOperator, MagickFalse },
    { "Log", (long) LogEvaluateOperator, MagickFalse },
    { "Max", (long) MaxEvaluateOperator, MagickFalse },
    { "Mean", (long) MeanEvaluateOperator, MagickFalse },
    { "Min", (long) MinEvaluateOperator, MagickFalse },
    { "MultiplicativeNoise", (long) MultiplicativeNoiseEvaluateOperator, MagickFalse },
    { "Multiply", (long) MultiplyEvaluateOperator, MagickFalse },
    { "Or", (long) OrEvaluateOperator, MagickFalse },
    { "PoissonNoise", (long) PoissonNoiseEvaluateOperator, MagickFalse },
    { "Pow", (long) PowEvaluateOperator, MagickFalse },
    { "RightShift", (long) RightShiftEvaluateOperator, MagickFalse },
    { "Set", (long) SetEvaluateOperator, MagickFalse },
    { "Sin", (long) SineEvaluateOperator, MagickFalse },
    { "Sine", (long) SineEvaluateOperator, MagickFalse },
    { "Subtract", (long) SubtractEvaluateOperator, MagickFalse },
    { "Threshold", (long) ThresholdEvaluateOperator, MagickFalse },
    { "ThresholdBlack", (long) ThresholdBlackEvaluateOperator, MagickFalse },
    { "ThresholdWhite", (long) ThresholdWhiteEvaluateOperator, MagickFalse },
    { "UniformNoise", (long) UniformNoiseEvaluateOperator, MagickFalse },
    { "Xor", (long) XorEvaluateOperator, MagickFalse },
    { (char *) NULL, (long) UndefinedEvaluateOperator, MagickFalse }
  },
  FillRuleOptions[] =
  {
    { "Undefined", (long) UndefinedRule, MagickTrue },
    { "Evenodd", (long) EvenOddRule, MagickFalse },
    { "NonZero", (long) NonZeroRule, MagickFalse },
    { (char *) NULL, (long) UndefinedRule, MagickFalse }
  },
  FilterOptions[] =
  {
    { "Undefined", (long) UndefinedFilter, MagickTrue },
    { "Bartlett", (long) BartlettFilter, MagickFalse },
    { "Bessel", (long) BesselFilter, MagickFalse },
    { "Blackman", (long) BlackmanFilter, MagickFalse },
    { "Bohman", (long) BohmanFilter, MagickFalse },
    { "Box", (long) BoxFilter, MagickFalse },
    { "Catrom", (long) CatromFilter, MagickFalse },
    { "Cubic", (long) CubicFilter, MagickFalse },
    { "Gaussian", (long) GaussianFilter, MagickFalse },
    { "Hamming", (long) HammingFilter, MagickFalse },
    { "Hanning", (long) HanningFilter, MagickFalse },
    { "Hermite", (long) HermiteFilter, MagickFalse },
    { "Kaiser", (long) KaiserFilter, MagickFalse },
    { "Lagrange", (long) LagrangeFilter, MagickFalse },
    { "Lanczos", (long) LanczosFilter, MagickFalse },
    { "Mitchell", (long) MitchellFilter, MagickFalse },
    { "Parzen", (long) ParzenFilter, MagickFalse },
    { "Point", (long) PointFilter, MagickFalse },
    { "Quadratic", (long) QuadraticFilter, MagickFalse },
    { "Sinc", (long) SincFilter, MagickFalse },
    { "Triangle", (long) TriangleFilter, MagickFalse },
    { "Welsh", (long) WelshFilter, MagickFalse },
    { (char *) NULL, (long) UndefinedFilter, MagickFalse }
  },
  FunctionOptions[] =
  {
    { "Undefined", (long) UndefinedFunction, MagickTrue },
    { "Polynomial", (long) PolynomialFunction, MagickFalse },
    { "Sinusoid", (long) SinusoidFunction, MagickFalse },
    { "ArcSin", (long) ArcsinFunction, MagickFalse },
    { "ArcTan", (long) ArctanFunction, MagickFalse },
    { (char *) NULL, (long) UndefinedFunction, MagickFalse }
  },
  GravityOptions[] =
  {
    { "Undefined", (long) UndefinedGravity, MagickTrue },
    { "None", (long) UndefinedGravity, MagickFalse },
    { "Center", (long) CenterGravity, MagickFalse },
    { "East", (long) EastGravity, MagickFalse },
    { "Forget", (long) ForgetGravity, MagickFalse },
    { "NorthEast", (long) NorthEastGravity, MagickFalse },
    { "North", (long) NorthGravity, MagickFalse },
    { "NorthWest", (long) NorthWestGravity, MagickFalse },
    { "SouthEast", (long) SouthEastGravity, MagickFalse },
    { "South", (long) SouthGravity, MagickFalse },
    { "SouthWest", (long) SouthWestGravity, MagickFalse },
    { "West", (long) WestGravity, MagickFalse },
    { "Static", (long) StaticGravity, MagickFalse },
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
    { "process", MagickTrue, MagickFalse },
    { "quiet", MagickTrue, MagickFalse },
    { "separate", MagickTrue, MagickFalse },
    { "swap", MagickTrue, MagickFalse },
    { "write", MagickTrue, MagickFalse },
    { (char *) NULL, MagickFalse, MagickFalse }
  },
  IntentOptions[] =
  {
    { "Undefined", (long) UndefinedIntent, MagickTrue },
    { "Absolute", (long) AbsoluteIntent, MagickFalse },
    { "Perceptual", (long) PerceptualIntent, MagickFalse },
    { "Relative", (long) RelativeIntent, MagickFalse },
    { "Saturation", (long) SaturationIntent, MagickFalse },
    { (char *) NULL, (long) UndefinedIntent, MagickFalse }
  },
  InterlaceOptions[] =
  {
    { "Undefined", (long) UndefinedInterlace, MagickTrue },
    { "Line", (long) LineInterlace, MagickFalse },
    { "None", (long) NoInterlace, MagickFalse },
    { "Plane", (long) PlaneInterlace, MagickFalse },
    { "Partition", (long) PartitionInterlace, MagickFalse },
    { "GIF", (long) GIFInterlace, MagickFalse },
    { "JPEG", (long) JPEGInterlace, MagickFalse },
    { "PNG", (long) PNGInterlace, MagickFalse },
    { (char *) NULL, (long) UndefinedInterlace, MagickFalse }
  },
  InterpolateOptions[] =
  {
    { "Undefined", (long) UndefinedInterpolatePixel, MagickTrue },
    { "Average", (long) AverageInterpolatePixel, MagickFalse },
    { "Bicubic", (long) BicubicInterpolatePixel, MagickFalse },
    { "Bilinear", (long) BilinearInterpolatePixel, MagickFalse },
    { "filter", (long) FilterInterpolatePixel, MagickFalse },
    { "Integer", (long) IntegerInterpolatePixel, MagickFalse },
    { "Mesh", (long) MeshInterpolatePixel, MagickFalse },
    { "NearestNeighbor", (long) NearestNeighborInterpolatePixel, MagickFalse },
    { "Spline", (long) SplineInterpolatePixel, MagickFalse },
    { (char *) NULL, (long) UndefinedInterpolatePixel, MagickFalse }
  },
  KernelOptions[] =
  {
    { "Undefined", (long) UndefinedKernel, MagickTrue },
    { "Gaussian", (long) GaussianKernel, MagickFalse },
    { "Blur", (long) BlurKernel, MagickFalse },
    { "Comet", (long) CometKernel, MagickFalse },
    { "Laplacian", (long) LaplacianKernel, MagickTrue }, /* not implemented */
    { "DOG", (long) DOGKernel, MagickTrue },             /* not implemented */
    { "LOG", (long) LOGKernel, MagickTrue },             /* not implemented */
    { "Rectangle", (long) RectangleKernel, MagickFalse },
    { "Square", (long) SquareKernel, MagickFalse },
    { "Diamond", (long) DiamondKernel, MagickFalse },
    { "Disk", (long) DiskKernel, MagickFalse },
    { "Plus", (long) PlusKernel, MagickFalse },
    { "Chebyshev", (long) ChebyshevKernel, MagickFalse },
    { "Manhatten", (long) ManhattenKernel, MagickFalse },
    { "Euclidean", (long) EuclideanKernel, MagickFalse },
    { "User Defined", (long) UserDefinedKernel, MagickTrue }, /* internel */
    { (char *) NULL, (long) UndefinedKernel, MagickFalse }
  },
  LayerOptions[] =
  {
    { "Undefined", (long) UndefinedLayer, MagickTrue },
    { "Coalesce", (long) CoalesceLayer, MagickFalse },
    { "CompareAny", (long) CompareAnyLayer, MagickFalse },
    { "CompareClear", (long) CompareClearLayer, MagickFalse },
    { "CompareOverlay", (long) CompareOverlayLayer, MagickFalse },
    { "Dispose", (long) DisposeLayer, MagickFalse },
    { "Optimize", (long) OptimizeLayer, MagickFalse },
    { "OptimizeFrame", (long) OptimizeImageLayer, MagickFalse },
    { "OptimizePlus", (long) OptimizePlusLayer, MagickFalse },
    { "OptimizeTransparency", (long) OptimizeTransLayer, MagickFalse },
    { "RemoveDups", (long) RemoveDupsLayer, MagickFalse },
    { "RemoveZero", (long) RemoveZeroLayer, MagickFalse },
    { "Composite", (long) CompositeLayer, MagickFalse },
    { "Merge", (long) MergeLayer, MagickFalse },
    { "Flatten", (long) FlattenLayer, MagickFalse },
    { "Mosaic", (long) MosaicLayer, MagickFalse },
    { "TrimBounds", (long) TrimBoundsLayer, MagickFalse },
    { (char *) NULL, (long) UndefinedLayer, MagickFalse }
  },
  LineCapOptions[] =
  {
    { "Undefined", (long) UndefinedCap, MagickTrue },
    { "Butt", (long) ButtCap, MagickFalse },
    { "Round", (long) RoundCap, MagickFalse },
    { "Square", (long) SquareCap, MagickFalse },
    { (char *) NULL, (long) UndefinedCap, MagickFalse }
  },
  LineJoinOptions[] =
  {
    { "Undefined", (long) UndefinedJoin, MagickTrue },
    { "Bevel", (long) BevelJoin, MagickFalse },
    { "Miter", (long) MiterJoin, MagickFalse },
    { "Round", (long) RoundJoin, MagickFalse },
    { (char *) NULL, (long) UndefinedJoin, MagickFalse }
  },
  ListOptions[] =
  {
    { "Align", (long) MagickAlignOptions, MagickFalse },
    { "Alpha", (long) MagickAlphaOptions, MagickFalse },
    { "Boolean", (long) MagickBooleanOptions, MagickFalse },
    { "Channel", (long) MagickChannelOptions, MagickFalse },
    { "Class", (long) MagickClassOptions, MagickFalse },
    { "ClipPath", (long) MagickClipPathOptions, MagickFalse },
    { "Coder", (long) MagickCoderOptions, MagickFalse },
    { "Color", (long) MagickColorOptions, MagickFalse },
    { "Colorspace", (long) MagickColorspaceOptions, MagickFalse },
    { "Command", (long) MagickCommandOptions, MagickFalse },
    { "Compose", (long) MagickComposeOptions, MagickFalse },
    { "Compress", (long) MagickCompressOptions, MagickFalse },
    { "Configure", (long) MagickConfigureOptions, MagickFalse },
    { "DataType", (long) MagickDataTypeOptions, MagickFalse },
    { "Debug", (long) MagickDebugOptions, MagickFalse },
    { "Decoration", (long) MagickDecorateOptions, MagickFalse },
    { "Delegate", (long) MagickDelegateOptions, MagickFalse },
    { "Dispose", (long) MagickDisposeOptions, MagickFalse },
    { "Distort", (long) MagickDistortOptions, MagickFalse },
    { "Dither", (long) MagickDitherOptions, MagickFalse },
    { "Endian", (long) MagickEndianOptions, MagickFalse },
    { "Evaluate", (long) MagickEvaluateOptions, MagickFalse },
    { "FillRule", (long) MagickFillRuleOptions, MagickFalse },
    { "Filter", (long) MagickFilterOptions, MagickFalse },
    { "Font", (long) MagickFontOptions, MagickFalse },
    { "Format", (long) MagickFormatOptions, MagickFalse },
    { "Function", (long) MagickFunctionOptions, MagickFalse },
    { "Gravity", (long) MagickGravityOptions, MagickFalse },
    { "ImageList", (long) MagickImageListOptions, MagickFalse },
    { "Intent", (long) MagickIntentOptions, MagickFalse },
    { "Interlace", (long) MagickInterlaceOptions, MagickFalse },
    { "Interpolate", (long) MagickInterpolateOptions, MagickFalse },
    { "Kernel", (long) MagickKernelOptions, MagickFalse },
    { "Layers", (long) MagickLayerOptions, MagickFalse },
    { "LineCap", (long) MagickLineCapOptions, MagickFalse },
    { "LineJoin", (long) MagickLineJoinOptions, MagickFalse },
    { "List", (long) MagickListOptions, MagickFalse },
    { "Locale", (long) MagickLocaleOptions, MagickFalse },
    { "LogEvent", (long) MagickLogEventOptions, MagickFalse },
    { "Log", (long) MagickLogOptions, MagickFalse },
    { "Magic", (long) MagickMagicOptions, MagickFalse },
    { "Method", (long) MagickMethodOptions, MagickFalse },
    { "Metric", (long) MagickMetricOptions, MagickFalse },
    { "Mime", (long) MagickMimeOptions, MagickFalse },
    { "Mode", (long) MagickModeOptions, MagickFalse },
    { "Morphology", (long) MagickMorphologyOptions, MagickFalse },
    { "Module", (long) MagickModuleOptions, MagickFalse },
    { "Noise", (long) MagickNoiseOptions, MagickFalse },
    { "Orientation", (long) MagickOrientationOptions, MagickFalse },
    { "Policy", (long) MagickPolicyOptions, MagickFalse },
    { "PolicyDomain", (long) MagickPolicyDomainOptions, MagickFalse },
    { "PolicyRights", (long) MagickPolicyRightsOptions, MagickFalse },
    { "Preview", (long) MagickPreviewOptions, MagickFalse },
    { "Primitive", (long) MagickPrimitiveOptions, MagickFalse },
    { "QuantumFormat", (long) MagickQuantumFormatOptions, MagickFalse },
    { "Resource", (long) MagickResourceOptions, MagickFalse },
    { "SparseColor", (long) MagickSparseColorOptions, MagickFalse },
    { "Storage", (long) MagickStorageOptions, MagickFalse },
    { "Stretch", (long) MagickStretchOptions, MagickFalse },
    { "Style", (long) MagickStyleOptions, MagickFalse },
    { "Threshold", (long) MagickThresholdOptions, MagickFalse },
    { "Type", (long) MagickTypeOptions, MagickFalse },
    { "Units", (long) MagickResolutionOptions, MagickFalse },
    { "Undefined", (long) MagickUndefinedOptions, MagickTrue },
    { "Validate", (long) MagickValidateOptions, MagickFalse },
    { "VirtualPixel", (long) MagickVirtualPixelOptions, MagickFalse },
    { (char *) NULL, (long) MagickUndefinedOptions, MagickFalse }
  },
  LogEventOptions[] =
  {
    { "Undefined", (long) UndefinedEvents, MagickTrue },
    { "All", (long) (AllEvents &~ TraceEvent), MagickFalse },
    { "Annotate", (long) AnnotateEvent, MagickFalse },
    { "Blob", (long) BlobEvent, MagickFalse },
    { "Cache", (long) CacheEvent, MagickFalse },
    { "Coder", (long) CoderEvent, MagickFalse },
    { "Configure", (long) ConfigureEvent, MagickFalse },
    { "Deprecate", (long) DeprecateEvent, MagickFalse },
    { "Draw", (long) DrawEvent, MagickFalse },
    { "Exception", (long) ExceptionEvent, MagickFalse },
    { "Locale", (long) LocaleEvent, MagickFalse },
    { "Module", (long) ModuleEvent, MagickFalse },
    { "None", (long) NoEvents, MagickFalse },
    { "Policy", (long) PolicyEvent, MagickFalse },
    { "Resource", (long) ResourceEvent, MagickFalse },
    { "Trace", (long) TraceEvent, MagickFalse },
    { "Transform", (long) TransformEvent, MagickFalse },
    { "User", (long) UserEvent, MagickFalse },
    { "Wand", (long) WandEvent, MagickFalse },
    { "X11", (long) X11Event, MagickFalse },
    { (char *) NULL, (long) UndefinedEvents, MagickFalse }
  },
  MetricOptions[] =
  {
    { "Undefined", (long) UndefinedMetric, MagickTrue },
    { "AE", (long) AbsoluteErrorMetric, MagickFalse },
    { "MAE", (long) MeanAbsoluteErrorMetric, MagickFalse },
    { "MEPP", (long) MeanErrorPerPixelMetric, MagickFalse },
    { "MSE", (long) MeanSquaredErrorMetric, MagickFalse },
    { "PAE", (long) PeakAbsoluteErrorMetric, MagickFalse },
    { "PSNR", (long) PeakSignalToNoiseRatioMetric, MagickFalse },
    { "RMSE", (long) RootMeanSquaredErrorMetric, MagickFalse },
    { (char *) NULL, (long) UndefinedMetric, MagickFalse }
  },
  MethodOptions[] =
  {
    { "Undefined", (long) UndefinedMethod, MagickTrue },
    { "FillToBorder", (long) FillToBorderMethod, MagickFalse },
    { "Floodfill", (long) FloodfillMethod, MagickFalse },
    { "Point", (long) PointMethod, MagickFalse },
    { "Replace", (long) ReplaceMethod, MagickFalse },
    { "Reset", (long) ResetMethod, MagickFalse },
    { (char *) NULL, (long) UndefinedMethod, MagickFalse }
  },
  ModeOptions[] =
  {
    { "Undefined", (long) UndefinedMode, MagickTrue },
    { "Concatenate", (long) ConcatenateMode, MagickFalse },
    { "Frame", (long) FrameMode, MagickFalse },
    { "Unframe", (long) UnframeMode, MagickFalse },
    { (char *) NULL, (long) UndefinedMode, MagickFalse }
  },
  MorphologyOptions[] =
  {
    { "Undefined", (long) UndefinedMorphology, MagickTrue },
    { "Correlate", (long) CorrelateMorphology, MagickFalse },
    { "Convolve", (long) ConvolveMorphology, MagickFalse },
    { "Dilate", (long) DilateMorphology, MagickFalse },
    { "Erode", (long) ErodeMorphology, MagickFalse },
    { "Close", (long) CloseMorphology, MagickFalse },
    { "Open", (long) OpenMorphology, MagickFalse },
    { "DilateIntensity", (long) DilateIntensityMorphology, MagickFalse },
    { "ErodeIntensity", (long) ErodeIntensityMorphology, MagickFalse },
    { "CloseIntensity", (long) CloseIntensityMorphology, MagickFalse },
    { "OpenIntensity", (long) OpenIntensityMorphology, MagickFalse },
    { "DilateI", (long) DilateIntensityMorphology, MagickFalse },
    { "ErodeI", (long) ErodeIntensityMorphology, MagickFalse },
    { "CloseI", (long) CloseIntensityMorphology, MagickFalse },
    { "OpenI", (long) OpenIntensityMorphology, MagickFalse },
    { "EdgeOut", (long) EdgeOutMorphology, MagickFalse },
    { "EdgeIn", (long) EdgeInMorphology, MagickFalse },
    { "Edge", (long) EdgeMorphology, MagickFalse },
    { "TopHat", (long) TopHatMorphology, MagickFalse },
    { "BottomHat", (long) BottomHatMorphology, MagickFalse },
    { "Distance", (long) DistanceMorphology, MagickFalse },
    { (char *) NULL, (long) UndefinedMorphology, MagickFalse }
  },
  NoiseOptions[] =
  {
    { "Undefined", (long) UndefinedNoise, MagickTrue },
    { "Gaussian", (long) (long) GaussianNoise, MagickFalse },
    { "Impulse", (long) ImpulseNoise, MagickFalse },
    { "Laplacian", (long) LaplacianNoise, MagickFalse },
    { "Multiplicative", (long) MultiplicativeGaussianNoise, MagickFalse },
    { "Poisson", (long) PoissonNoise, MagickFalse },
    { "Random", (long) RandomNoise, MagickFalse },
    { "Uniform", (long) UniformNoise, MagickFalse },
    { (char *) NULL, (long) UndefinedNoise, MagickFalse }
  },
  OrientationOptions[] =
  {
    { "Undefined", (long) UndefinedOrientation, MagickTrue },
    { "TopLeft", (long) TopLeftOrientation, MagickFalse },
    { "TopRight", (long) TopRightOrientation, MagickFalse },
    { "BottomRight", (long) BottomRightOrientation, MagickFalse },
    { "BottomLeft", (long) BottomLeftOrientation, MagickFalse },
    { "LeftTop", (long) LeftTopOrientation, MagickFalse },
    { "RightTop", (long) RightTopOrientation, MagickFalse },
    { "RightBottom", (long) RightBottomOrientation, MagickFalse },
    { "LeftBottom", (long) LeftBottomOrientation, MagickFalse }
  },
  PolicyDomainOptions[] =
  {
    { "Undefined", (long) UndefinedPolicyDomain, MagickTrue },
    { "Coder", (long) CoderPolicyDomain, MagickFalse },
    { "Delegate", (long) DelegatePolicyDomain, MagickFalse },
    { "Filter", (long) FilterPolicyDomain, MagickFalse },
    { "Path", (long) PathPolicyDomain, MagickFalse },
    { "Resource", (long) ResourcePolicyDomain, MagickFalse },
    { "System", (long) SystemPolicyDomain, MagickFalse }
  },
  PolicyRightsOptions[] =
  {
    { "Undefined", (long) UndefinedPolicyRights, MagickTrue },
    { "None", (long) NoPolicyRights, MagickFalse },
    { "Read", (long) ReadPolicyRights, MagickFalse },
    { "Write", (long) WritePolicyRights, MagickFalse },
    { "Execute", (long) ExecutePolicyRights, MagickFalse }
  },
  PreviewOptions[] =
  {
    { "Undefined", (long) UndefinedPreview, MagickTrue },
    { "AddNoise", (long) AddNoisePreview, MagickFalse },
    { "Blur", (long) BlurPreview, MagickFalse },
    { "Brightness", (long) BrightnessPreview, MagickFalse },
    { "Charcoal", (long) CharcoalDrawingPreview, MagickFalse },
    { "Despeckle", (long) DespecklePreview, MagickFalse },
    { "Dull", (long) DullPreview, MagickFalse },
    { "EdgeDetect", (long) EdgeDetectPreview, MagickFalse },
    { "Gamma", (long) GammaPreview, MagickFalse },
    { "Grayscale", (long) GrayscalePreview, MagickFalse },
    { "Hue", (long) HuePreview, MagickFalse },
    { "Implode", (long) ImplodePreview, MagickFalse },
    { "JPEG", (long) JPEGPreview, MagickFalse },
    { "OilPaint", (long) OilPaintPreview, MagickFalse },
    { "Quantize", (long) QuantizePreview, MagickFalse },
    { "Raise", (long) RaisePreview, MagickFalse },
    { "ReduceNoise", (long) ReduceNoisePreview, MagickFalse },
    { "Roll", (long) RollPreview, MagickFalse },
    { "Rotate", (long) RotatePreview, MagickFalse },
    { "Saturation", (long) SaturationPreview, MagickFalse },
    { "Segment", (long) SegmentPreview, MagickFalse },
    { "Shade", (long) ShadePreview, MagickFalse },
    { "Sharpen", (long) SharpenPreview, MagickFalse },
    { "Shear", (long) ShearPreview, MagickFalse },
    { "Solarize", (long) SolarizePreview, MagickFalse },
    { "Spiff", (long) SpiffPreview, MagickFalse },
    { "Spread", (long) SpreadPreview, MagickFalse },
    { "Swirl", (long) SwirlPreview, MagickFalse },
    { "Threshold", (long) ThresholdPreview, MagickFalse },
    { "Wave", (long) WavePreview, MagickFalse },
    { (char *) NULL, (long) UndefinedPreview, MagickFalse }
  },
  PrimitiveOptions[] =
  {
    { "Undefined", (long) UndefinedPrimitive, MagickTrue },
    { "Arc", (long) ArcPrimitive, MagickFalse },
    { "Bezier", (long) BezierPrimitive, MagickFalse },
    { "Circle", (long) CirclePrimitive, MagickFalse },
    { "Color", (long) ColorPrimitive, MagickFalse },
    { "Ellipse", (long) EllipsePrimitive, MagickFalse },
    { "Image", (long) ImagePrimitive, MagickFalse },
    { "Line", (long) LinePrimitive, MagickFalse },
    { "Matte", (long) MattePrimitive, MagickFalse },
    { "Path", (long) PathPrimitive, MagickFalse },
    { "Point", (long) PointPrimitive, MagickFalse },
    { "Polygon", (long) PolygonPrimitive, MagickFalse },
    { "Polyline", (long) PolylinePrimitive, MagickFalse },
    { "Rectangle", (long) RectanglePrimitive, MagickFalse },
    { "roundRectangle", (long) RoundRectanglePrimitive, MagickFalse },
    { "Text", (long) TextPrimitive, MagickFalse },
    { (char *) NULL, (long) UndefinedPrimitive, MagickFalse }
  },
  QuantumFormatOptions[] =
  {
    { "Undefined", (long) UndefinedQuantumFormat, MagickTrue },
    { "FloatingPoint", (long) FloatingPointQuantumFormat, MagickFalse },
    { "Signed", (long) SignedQuantumFormat, MagickFalse },
    { "Unsigned", (long) UnsignedQuantumFormat, MagickFalse },
    { (char *) NULL, (long) FloatingPointQuantumFormat, MagickFalse }
  },
  ResolutionOptions[] =
  {
    { "Undefined", (long) UndefinedResolution, MagickTrue },
    { "PixelsPerInch", (long) PixelsPerInchResolution, MagickFalse },
    { "PixelsPerCentimeter", (long) PixelsPerCentimeterResolution, MagickFalse },
    { (char *) NULL, (long) UndefinedResolution, MagickFalse }
  },
  ResourceOptions[] =
  {
    { "Undefined", (long) UndefinedResource, MagickTrue },
    { "Area", (long) AreaResource, MagickFalse },
    { "Disk", (long) DiskResource, MagickFalse },
    { "File", (long) FileResource, MagickFalse },
    { "Map", (long) MapResource, MagickFalse },
    { "Memory", (long) MemoryResource, MagickFalse },
    { "Thread", (long) ThreadResource, MagickFalse },
    { "Time", (long) TimeResource, MagickFalse },
    { (char *) NULL, (long) UndefinedResource, MagickFalse }
  },
  SparseColorOptions[] =
  {
    { "Undefined", (long) UndefinedDistortion, MagickTrue },
    { "Barycentric", (long) BarycentricColorInterpolate, MagickFalse },
    { "Bilinear", (long) BilinearColorInterpolate, MagickFalse },
    { "Shepards", (long) ShepardsColorInterpolate, MagickFalse },
    { "Voronoi", (long) VoronoiColorInterpolate, MagickFalse },
    { (char *) NULL, (long) UndefinedResource, MagickFalse }
  },
  StorageOptions[] =
  {
    { "Undefined", (long) UndefinedPixel, MagickTrue },
    { "Char", (long) CharPixel, MagickFalse },
    { "Double", (long) DoublePixel, MagickFalse },
    { "Float", (long) FloatPixel, MagickFalse },
    { "Integer", (long) IntegerPixel, MagickFalse },
    { "Long", (long) LongPixel, MagickFalse },
    { "Quantum", (long) QuantumPixel, MagickFalse },
    { "Short", (long) ShortPixel, MagickFalse },
    { (char *) NULL, (long) UndefinedResource, MagickFalse }
  },
  StretchOptions[] =
  {
    { "Undefined", (long) UndefinedStretch, MagickTrue },
    { "Any", (long) AnyStretch, MagickFalse },
    { "Condensed", (long) CondensedStretch, MagickFalse },
    { "Expanded", (long) ExpandedStretch, MagickFalse },
    { "ExtraCondensed", (long) ExtraCondensedStretch, MagickFalse },
    { "ExtraExpanded", (long) ExtraExpandedStretch, MagickFalse },
    { "Normal", (long) NormalStretch, MagickFalse },
    { "SemiCondensed", (long) SemiCondensedStretch, MagickFalse },
    { "SemiExpanded", (long) SemiExpandedStretch, MagickFalse },
    { "UltraCondensed", (long) UltraCondensedStretch, MagickFalse },
    { "UltraExpanded", (long) UltraExpandedStretch, MagickFalse },
    { (char *) NULL, (long) UndefinedStretch, MagickFalse }
  },
  StyleOptions[] =
  {
    { "Undefined", (long) UndefinedStyle, MagickTrue },
    { "Any", (long) AnyStyle, MagickFalse },
    { "Italic", (long) ItalicStyle, MagickFalse },
    { "Normal", (long) NormalStyle, MagickFalse },
    { "Oblique", (long) ObliqueStyle, MagickFalse },
    { (char *) NULL, (long) UndefinedStyle, MagickFalse }
  },
  TypeOptions[] =
  {
    { "Undefined", (long) UndefinedType, MagickTrue },
    { "Bilevel", (long) BilevelType, MagickFalse },
    { "ColorSeparation", (long) ColorSeparationType, MagickFalse },
    { "ColorSeparationMatte", (long) ColorSeparationMatteType, MagickFalse },
    { "Grayscale", (long) GrayscaleType, MagickFalse },
    { "GrayscaleMatte", (long) GrayscaleMatteType, MagickFalse },
    { "Optimize", (long) OptimizeType, MagickFalse },
    { "Palette", (long) PaletteType, MagickFalse },
    { "PaletteBilevelMatte", (long) PaletteBilevelMatteType, MagickFalse },
    { "PaletteMatte", (long) PaletteMatteType, MagickFalse },
    { "TrueColorMatte", (long) TrueColorMatteType, MagickFalse },
    { "TrueColor", (long) TrueColorType, MagickFalse },
    { (char *) NULL, (long) UndefinedType, MagickFalse }
  },
  ValidateOptions[] =
  {
    { "Undefined", (long) UndefinedValidate, MagickTrue },
    { "All", (long) AllValidate, MagickFalse },
    { "Compare", (long) CompareValidate, MagickFalse },
    { "Composite", (long) CompositeValidate, MagickFalse },
    { "Convert", (long) ConvertValidate, MagickFalse },
    { "FormatsInMemory", (long) FormatsInMemoryValidate, MagickFalse },
    { "FormatsOnDisk", (long) FormatsOnDiskValidate, MagickFalse },
    { "Identify", (long) IdentifyValidate, MagickFalse },
    { "ImportExport", (long) ImportExportValidate, MagickFalse },
    { "Montage", (long) MontageValidate, MagickFalse },
    { "Stream", (long) StreamValidate, MagickFalse },
    { "None", (long) NoValidate, MagickFalse },
    { (char *) NULL, (long) UndefinedValidate, MagickFalse }
  },
  VirtualPixelOptions[] =
  {
    { "Undefined", (long) UndefinedVirtualPixelMethod, MagickTrue },
    { "Background", (long) BackgroundVirtualPixelMethod, MagickFalse },
    { "Black", (long) BlackVirtualPixelMethod, MagickFalse },
    { "Constant", (long) BackgroundVirtualPixelMethod, MagickTrue }, /* deprecated */
    { "CheckerTile", (long) CheckerTileVirtualPixelMethod, MagickFalse },
    { "Dither", (long) DitherVirtualPixelMethod, MagickFalse },
    { "Edge", (long) EdgeVirtualPixelMethod, MagickFalse },
    { "Gray", (long) GrayVirtualPixelMethod, MagickFalse },
    { "HorizontalTile", (long) HorizontalTileVirtualPixelMethod, MagickFalse },
    { "HorizontalTileEdge", (long) HorizontalTileEdgeVirtualPixelMethod, MagickFalse },
    { "Mirror", (long) MirrorVirtualPixelMethod, MagickFalse },
    { "Random", (long) RandomVirtualPixelMethod, MagickFalse },
    { "Tile", (long) TileVirtualPixelMethod, MagickFalse },
    { "Transparent", (long) TransparentVirtualPixelMethod, MagickFalse },
    { "VerticalTile", (long) VerticalTileVirtualPixelMethod, MagickFalse },
    { "VerticalTileEdge", (long) VerticalTileEdgeVirtualPixelMethod, MagickFalse },
    { "White", (long) WhiteVirtualPixelMethod, MagickFalse },
    { (char *) NULL, (long) UndefinedVirtualPixelMethod, MagickFalse }
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

  register long
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
%        const long type)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
%    o type: one or more values separated by commas.
%
*/
MagickExport const char *MagickOptionToMnemonic(const MagickOption option,
  const long type)
{
  const OptionInfo
    *option_info;

  register long
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

  register long
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
%      long ParseChannelOption(const char *channels)
%
%  A description of each parameter follows:
%
%    o options: One or more values separated by commas.
%
*/
MagickExport long ParseChannelOption(const char *channels)
{
  long
    channel;

  register long
    i;

  channel=ParseMagickOption(MagickChannelOptions,MagickTrue,channels);
  if (channel >= 0)
    return(channel);
  channel=0;
  for (i=0; i < (long) strlen(channels); i++)
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
        long
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
%      long ParseMagickOption(const MagickOption option,
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
MagickExport long ParseMagickOption(const MagickOption option,
  const MagickBooleanType list,const char *options)
{
  char
    token[MaxTextExtent];

  const OptionInfo
    *option_info;

  int
    sentinel;

  long
    option_types;

  MagickBooleanType
    negate;

  register char
    *q;

  register const char
    *p;

  register long
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                       CCCC   OOO   L       OOO   RRRR                       %
%                      C      O   O  L      O   O  R   R                      %
%                      C      O   O  L      O   O  RRRR                       %
%                      C      O   O  L      O   O  R R                        %
%                       CCCC   OOO   LLLLL   OOO   R  R                       %
%                                                                             %
%                                                                             %
%                          MagickCore Color Methods                           %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
%  We use linked-lists because splay-trees do not currently support duplicate
%  key / value pairs (.e.g X11 green compliance and SVG green compliance).
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/cache-view.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/client.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define ColorFilename  "colors.xml"

/*
  Typedef declarations.
*/
typedef struct _ColorMapInfo
{
  const char
    *name;

  const unsigned char
    red,
    green,
    blue;

  const float
    alpha;

  const ssize_t
    compliance;
} ColorMapInfo;

/*
  Static declarations.
*/
static const ColorMapInfo
  ColorMap[] =
  {
    { "none", 0, 0, 0, 0, SVGCompliance | XPMCompliance },
    { "black", 0, 0, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "red", 255, 0, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "magenta", 255, 0, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "green", 0, 128, 0, 1, SVGCompliance },
    { "cyan", 0, 255, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "blue", 0, 0, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "yellow", 255, 255, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "white", 255, 255, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "AliceBlue", 240, 248, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "AntiqueWhite", 250, 235, 215, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "AntiqueWhite1", 255, 239, 219, 1, X11Compliance },
    { "AntiqueWhite2", 238, 223, 204, 1, X11Compliance },
    { "AntiqueWhite3", 205, 192, 176, 1, X11Compliance },
    { "AntiqueWhite4", 139, 131, 120, 1, X11Compliance },
    { "aqua", 0, 255, 255, 1, SVGCompliance },
    { "aquamarine", 127, 255, 212, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "aquamarine1", 127, 255, 212, 1, X11Compliance },
    { "aquamarine2", 118, 238, 198, 1, X11Compliance },
    { "aquamarine3", 102, 205, 170, 1, X11Compliance },
    { "aquamarine4", 69, 139, 116, 1, X11Compliance },
    { "azure", 240, 255, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "azure1", 240, 255, 255, 1, X11Compliance },
    { "azure2", 224, 238, 238, 1, X11Compliance },
    { "azure3", 193, 205, 205, 1, X11Compliance },
    { "azure4", 131, 139, 139, 1, X11Compliance },
    { "beige", 245, 245, 220, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "bisque", 255, 228, 196, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "bisque1", 255, 228, 196, 1, X11Compliance },
    { "bisque2", 238, 213, 183, 1, X11Compliance },
    { "bisque3", 205, 183, 158, 1, X11Compliance },
    { "bisque4", 139, 125, 107, 1, X11Compliance },
    { "BlanchedAlmond", 255, 235, 205, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "blue1", 0, 0, 255, 1, X11Compliance },
    { "blue2", 0, 0, 238, 1, X11Compliance },
    { "blue3", 0, 0, 205, 1, X11Compliance },
    { "blue4", 0, 0, 139, 1, X11Compliance },
    { "BlueViolet", 138, 43, 226, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "brown", 165, 42, 42, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "brown1", 255, 64, 64, 1, X11Compliance },
    { "brown2", 238, 59, 59, 1, X11Compliance },
    { "brown3", 205, 51, 51, 1, X11Compliance },
    { "brown4", 139, 35, 35, 1, X11Compliance },
    { "burlywood", 222, 184, 135, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "burlywood1", 255, 211, 155, 1, X11Compliance },
    { "burlywood2", 238, 197, 145, 1, X11Compliance },
    { "burlywood3", 205, 170, 125, 1, X11Compliance },
    { "burlywood4", 139, 115, 85, 1, X11Compliance },
    { "CadetBlue", 95, 158, 160, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "CadetBlue1", 152, 245, 255, 1, X11Compliance },
    { "CadetBlue2", 142, 229, 238, 1, X11Compliance },
    { "CadetBlue3", 122, 197, 205, 1, X11Compliance },
    { "CadetBlue4", 83, 134, 139, 1, X11Compliance },
    { "chartreuse", 127, 255, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "chartreuse1", 127, 255, 0, 1, X11Compliance },
    { "chartreuse2", 118, 238, 0, 1, X11Compliance },
    { "chartreuse3", 102, 205, 0, 1, X11Compliance },
    { "chartreuse4", 69, 139, 0, 1, X11Compliance },
    { "chocolate", 210, 105, 30, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "chocolate1", 255, 127, 36, 1, X11Compliance },
    { "chocolate2", 238, 118, 33, 1, X11Compliance },
    { "chocolate3", 205, 102, 29, 1, X11Compliance },
    { "chocolate4", 139, 69, 19, 1, X11Compliance },
    { "coral", 255, 127, 80, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "coral1", 255, 114, 86, 1, X11Compliance },
    { "coral2", 238, 106, 80, 1, X11Compliance },
    { "coral3", 205, 91, 69, 1, X11Compliance },
    { "coral4", 139, 62, 47, 1, X11Compliance },
    { "CornflowerBlue", 100, 149, 237, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "cornsilk", 255, 248, 220, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "cornsilk1", 255, 248, 220, 1, X11Compliance },
    { "cornsilk2", 238, 232, 205, 1, X11Compliance },
    { "cornsilk3", 205, 200, 177, 1, X11Compliance },
    { "cornsilk4", 139, 136, 120, 1, X11Compliance },
    { "crimson", 220, 20, 60, 1, SVGCompliance },
    { "cyan1", 0, 255, 255, 1, X11Compliance },
    { "cyan2", 0, 238, 238, 1, X11Compliance },
    { "cyan3", 0, 205, 205, 1, X11Compliance },
    { "cyan4", 0, 139, 139, 1, X11Compliance },
    { "DarkBlue", 0, 0, 139, 1, SVGCompliance | X11Compliance },
    { "DarkCyan", 0, 139, 139, 1, SVGCompliance | X11Compliance },
    { "DarkGoldenrod", 184, 134, 11, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkGoldenrod1", 255, 185, 15, 1, X11Compliance },
    { "DarkGoldenrod2", 238, 173, 14, 1, X11Compliance },
    { "DarkGoldenrod3", 205, 149, 12, 1, X11Compliance },
    { "DarkGoldenrod4", 139, 101, 8, 1, X11Compliance },
    { "DarkGray", 169, 169, 169, 1, SVGCompliance | X11Compliance },
    { "DarkGreen", 0, 100, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkGrey", 169, 169, 169, 1, SVGCompliance | X11Compliance },
    { "DarkKhaki", 189, 183, 107, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkMagenta", 139, 0, 139, 1, SVGCompliance | X11Compliance },
    { "DarkOliveGreen", 85, 107, 47, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkOliveGreen1", 202, 255, 112, 1, X11Compliance },
    { "DarkOliveGreen2", 188, 238, 104, 1, X11Compliance },
    { "DarkOliveGreen3", 162, 205, 90, 1, X11Compliance },
    { "DarkOliveGreen4", 110, 139, 61, 1, X11Compliance },
    { "DarkOrange", 255, 140, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkOrange1", 255, 127, 0, 1, X11Compliance },
    { "DarkOrange2", 238, 118, 0, 1, X11Compliance },
    { "DarkOrange3", 205, 102, 0, 1, X11Compliance },
    { "DarkOrange4", 139, 69, 0, 1, X11Compliance },
    { "DarkOrchid", 153, 50, 204, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkOrchid1", 191, 62, 255, 1, X11Compliance },
    { "DarkOrchid2", 178, 58, 238, 1, X11Compliance },
    { "DarkOrchid3", 154, 50, 205, 1, X11Compliance },
    { "DarkOrchid4", 104, 34, 139, 1, X11Compliance },
    { "DarkRed", 139, 0, 0, 1, SVGCompliance | X11Compliance },
    { "DarkSalmon", 233, 150, 122, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkSeaGreen", 143, 188, 143, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkSeaGreen1", 193, 255, 193, 1, X11Compliance },
    { "DarkSeaGreen2", 180, 238, 180, 1, X11Compliance },
    { "DarkSeaGreen3", 155, 205, 155, 1, X11Compliance },
    { "DarkSeaGreen4", 105, 139, 105, 1, X11Compliance },
    { "DarkSlateBlue", 72, 61, 139, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkSlateGray", 47, 79, 79, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkSlateGray1", 151, 255, 255, 1, X11Compliance },
    { "DarkSlateGray2", 141, 238, 238, 1, X11Compliance },
    { "DarkSlateGray3", 121, 205, 205, 1, X11Compliance },
    { "DarkSlateGray4", 82, 139, 139, 1, X11Compliance },
    { "DarkSlateGrey", 47, 79, 79, 1, SVGCompliance | X11Compliance },
    { "DarkTurquoise", 0, 206, 209, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DarkViolet", 148, 0, 211, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DeepPink", 255, 20, 147, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DeepPink1", 255, 20, 147, 1, X11Compliance },
    { "DeepPink2", 238, 18, 137, 1, X11Compliance },
    { "DeepPink3", 205, 16, 118, 1, X11Compliance },
    { "DeepPink4", 139, 10, 80, 1, X11Compliance },
    { "DeepSkyBlue", 0, 191, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DeepSkyBlue1", 0, 191, 255, 1, X11Compliance },
    { "DeepSkyBlue2", 0, 178, 238, 1, X11Compliance },
    { "DeepSkyBlue3", 0, 154, 205, 1, X11Compliance },
    { "DeepSkyBlue4", 0, 104, 139, 1, X11Compliance },
    { "DimGray", 105, 105, 105, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DimGrey", 105, 105, 105, 1, SVGCompliance | X11Compliance },
    { "DodgerBlue", 30, 144, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "DodgerBlue1", 30, 144, 255, 1, X11Compliance },
    { "DodgerBlue2", 28, 134, 238, 1, X11Compliance },
    { "DodgerBlue3", 24, 116, 205, 1, X11Compliance },
    { "DodgerBlue4", 16, 78, 139, 1, X11Compliance },
    { "firebrick", 178, 34, 34, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "firebrick1", 255, 48, 48, 1, X11Compliance },
    { "firebrick2", 238, 44, 44, 1, X11Compliance },
    { "firebrick3", 205, 38, 38, 1, X11Compliance },
    { "firebrick4", 139, 26, 26, 1, X11Compliance },
    { "FloralWhite", 255, 250, 240, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "ForestGreen", 34, 139, 34, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "fractal", 128, 128, 128, 1, SVGCompliance },
    { "freeze", 0, 0, 0, 0, SVGCompliance },
    { "fuchsia", 255, 0, 255, 1, SVGCompliance },
    { "gainsboro", 220, 220, 220, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "GhostWhite", 248, 248, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "gold", 255, 215, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "gold1", 255, 215, 0, 1, X11Compliance },
    { "gold2", 238, 201, 0, 1, X11Compliance },
    { "gold3", 205, 173, 0, 1, X11Compliance },
    { "gold4", 139, 117, 0, 1, X11Compliance },
    { "goldenrod", 218, 165, 32, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "goldenrod1", 255, 193, 37, 1, X11Compliance },
    { "goldenrod2", 238, 180, 34, 1, X11Compliance },
    { "goldenrod3", 205, 155, 29, 1, X11Compliance },
    { "goldenrod4", 139, 105, 20, 1, X11Compliance },
    { "gray", 126, 126, 126, 1, SVGCompliance },
    { "gray", 190, 190, 190, 1, X11Compliance | XPMCompliance },
    { "gray0", 0, 0, 0, 1, X11Compliance | XPMCompliance },
    { "gray1", 3, 3, 3, 1, X11Compliance | XPMCompliance },
    { "gray10", 26, 26, 26, 1, X11Compliance | XPMCompliance },
    { "gray100", 255, 255, 255, 1, X11Compliance | XPMCompliance },
    { "gray100", 255, 255, 255, 1, X11Compliance | XPMCompliance },
    { "gray11", 28, 28, 28, 1, X11Compliance | XPMCompliance },
    { "gray12", 31, 31, 31, 1, X11Compliance | XPMCompliance },
    { "gray13", 33, 33, 33, 1, X11Compliance | XPMCompliance },
    { "gray14", 36, 36, 36, 1, X11Compliance | XPMCompliance },
    { "gray15", 38, 38, 38, 1, X11Compliance | XPMCompliance },
    { "gray16", 41, 41, 41, 1, X11Compliance | XPMCompliance },
    { "gray17", 43, 43, 43, 1, X11Compliance | XPMCompliance },
    { "gray18", 46, 46, 46, 1, X11Compliance | XPMCompliance },
    { "gray19", 48, 48, 48, 1, X11Compliance | XPMCompliance },
    { "gray2", 5, 5, 5, 1, X11Compliance | XPMCompliance },
    { "gray20", 51, 51, 51, 1, X11Compliance | XPMCompliance },
    { "gray21", 54, 54, 54, 1, X11Compliance | XPMCompliance },
    { "gray22", 56, 56, 56, 1, X11Compliance | XPMCompliance },
    { "gray23", 59, 59, 59, 1, X11Compliance | XPMCompliance },
    { "gray24", 61, 61, 61, 1, X11Compliance | XPMCompliance },
    { "gray25", 64, 64, 64, 1, X11Compliance | XPMCompliance },
    { "gray26", 66, 66, 66, 1, X11Compliance | XPMCompliance },
    { "gray27", 69, 69, 69, 1, X11Compliance | XPMCompliance },
    { "gray28", 71, 71, 71, 1, X11Compliance | XPMCompliance },
    { "gray29", 74, 74, 74, 1, X11Compliance | XPMCompliance },
    { "gray3", 8, 8, 8, 1, X11Compliance | XPMCompliance },
    { "gray30", 77, 77, 77, 1, X11Compliance | XPMCompliance },
    { "gray31", 79, 79, 79, 1, X11Compliance | XPMCompliance },
    { "gray32", 82, 82, 82, 1, X11Compliance | XPMCompliance },
    { "gray33", 84, 84, 84, 1, X11Compliance | XPMCompliance },
    { "gray34", 87, 87, 87, 1, X11Compliance | XPMCompliance },
    { "gray35", 89, 89, 89, 1, X11Compliance | XPMCompliance },
    { "gray36", 92, 92, 92, 1, X11Compliance | XPMCompliance },
    { "gray37", 94, 94, 94, 1, X11Compliance | XPMCompliance },
    { "gray38", 97, 97, 97, 1, X11Compliance | XPMCompliance },
    { "gray39", 99, 99, 99, 1, X11Compliance | XPMCompliance },
    { "gray4", 10, 10, 10, 1, X11Compliance | XPMCompliance },
    { "gray40", 102, 102, 102, 1, X11Compliance | XPMCompliance },
    { "gray41", 105, 105, 105, 1, X11Compliance | XPMCompliance },
    { "gray42", 107, 107, 107, 1, X11Compliance | XPMCompliance },
    { "gray43", 110, 110, 110, 1, X11Compliance | XPMCompliance },
    { "gray44", 112, 112, 112, 1, X11Compliance | XPMCompliance },
    { "gray45", 115, 115, 115, 1, X11Compliance | XPMCompliance },
    { "gray46", 117, 117, 117, 1, X11Compliance | XPMCompliance },
    { "gray47", 120, 120, 120, 1, X11Compliance | XPMCompliance },
    { "gray48", 122, 122, 122, 1, X11Compliance | XPMCompliance },
    { "gray49", 125, 125, 125, 1, X11Compliance | XPMCompliance },
    { "gray5", 13, 13, 13, 1, X11Compliance | XPMCompliance },
    { "gray50", 127, 127, 127, 1, X11Compliance | XPMCompliance },
    { "gray51", 130, 130, 130, 1, X11Compliance | XPMCompliance },
    { "gray52", 133, 133, 133, 1, X11Compliance | XPMCompliance },
    { "gray53", 135, 135, 135, 1, X11Compliance | XPMCompliance },
    { "gray54", 138, 138, 138, 1, X11Compliance | XPMCompliance },
    { "gray55", 140, 140, 140, 1, X11Compliance | XPMCompliance },
    { "gray56", 143, 143, 143, 1, X11Compliance | XPMCompliance },
    { "gray57", 145, 145, 145, 1, X11Compliance | XPMCompliance },
    { "gray58", 148, 148, 148, 1, X11Compliance | XPMCompliance },
    { "gray59", 150, 150, 150, 1, X11Compliance | XPMCompliance },
    { "gray6", 15, 15, 15, 1, X11Compliance | XPMCompliance },
    { "gray60", 153, 153, 153, 1, X11Compliance | XPMCompliance },
    { "gray61", 156, 156, 156, 1, X11Compliance | XPMCompliance },
    { "gray62", 158, 158, 158, 1, X11Compliance | XPMCompliance },
    { "gray63", 161, 161, 161, 1, X11Compliance | XPMCompliance },
    { "gray64", 163, 163, 163, 1, X11Compliance | XPMCompliance },
    { "gray65", 166, 166, 166, 1, X11Compliance | XPMCompliance },
    { "gray66", 168, 168, 168, 1, X11Compliance | XPMCompliance },
    { "gray67", 171, 171, 171, 1, X11Compliance | XPMCompliance },
    { "gray68", 173, 173, 173, 1, X11Compliance | XPMCompliance },
    { "gray69", 176, 176, 176, 1, X11Compliance | XPMCompliance },
    { "gray7", 18, 18, 18, 1, X11Compliance | XPMCompliance },
    { "gray70", 179, 179, 179, 1, X11Compliance | XPMCompliance },
    { "gray71", 181, 181, 181, 1, X11Compliance | XPMCompliance },
    { "gray72", 184, 184, 184, 1, X11Compliance | XPMCompliance },
    { "gray73", 186, 186, 186, 1, X11Compliance | XPMCompliance },
    { "gray74", 189, 189, 189, 1, X11Compliance | XPMCompliance },
    { "gray75", 191, 191, 191, 1, X11Compliance | XPMCompliance },
    { "gray76", 194, 194, 194, 1, X11Compliance | XPMCompliance },
    { "gray77", 196, 196, 196, 1, X11Compliance | XPMCompliance },
    { "gray78", 199, 199, 199, 1, X11Compliance | XPMCompliance },
    { "gray79", 201, 201, 201, 1, X11Compliance | XPMCompliance },
    { "gray8", 20, 20, 20, 1, X11Compliance | XPMCompliance },
    { "gray80", 204, 204, 204, 1, X11Compliance | XPMCompliance },
    { "gray81", 207, 207, 207, 1, X11Compliance | XPMCompliance },
    { "gray82", 209, 209, 209, 1, X11Compliance | XPMCompliance },
    { "gray83", 212, 212, 212, 1, X11Compliance | XPMCompliance },
    { "gray84", 214, 214, 214, 1, X11Compliance | XPMCompliance },
    { "gray85", 217, 217, 217, 1, X11Compliance | XPMCompliance },
    { "gray86", 219, 219, 219, 1, X11Compliance | XPMCompliance },
    { "gray87", 222, 222, 222, 1, X11Compliance | XPMCompliance },
    { "gray88", 224, 224, 224, 1, X11Compliance | XPMCompliance },
    { "gray89", 227, 227, 227, 1, X11Compliance | XPMCompliance },
    { "gray9", 23, 23, 23, 1, X11Compliance | XPMCompliance },
    { "gray90", 229, 229, 229, 1, X11Compliance | XPMCompliance },
    { "gray91", 232, 232, 232, 1, X11Compliance | XPMCompliance },
    { "gray92", 235, 235, 235, 1, X11Compliance | XPMCompliance },
    { "gray93", 237, 237, 237, 1, X11Compliance | XPMCompliance },
    { "gray94", 240, 240, 240, 1, X11Compliance | XPMCompliance },
    { "gray95", 242, 242, 242, 1, X11Compliance | XPMCompliance },
    { "gray96", 245, 245, 245, 1, X11Compliance | XPMCompliance },
    { "gray97", 247, 247, 247, 1, X11Compliance | XPMCompliance },
    { "gray98", 250, 250, 250, 1, X11Compliance | XPMCompliance },
    { "gray99", 252, 252, 252, 1, X11Compliance | XPMCompliance },
    { "green", 0, 255, 0, 1, X11Compliance | XPMCompliance },
    { "green1", 0, 255, 0, 1, X11Compliance },
    { "green2", 0, 238, 0, 1, X11Compliance },
    { "green3", 0, 205, 0, 1, X11Compliance },
    { "green4", 0, 139, 0, 1, X11Compliance },
    { "GreenYellow", 173, 255, 47, 1, X11Compliance | XPMCompliance },
    { "grey", 190, 190, 190, 1, SVGCompliance | X11Compliance },
    { "grey0", 0, 0, 0, 1, SVGCompliance | X11Compliance },
    { "grey1", 3, 3, 3, 1, SVGCompliance | X11Compliance },
    { "grey10", 26, 26, 26, 1, SVGCompliance | X11Compliance },
    { "grey100", 255, 255, 255, 1, SVGCompliance | X11Compliance },
    { "grey11", 28, 28, 28, 1, SVGCompliance | X11Compliance },
    { "grey12", 31, 31, 31, 1, SVGCompliance | X11Compliance },
    { "grey13", 33, 33, 33, 1, SVGCompliance | X11Compliance },
    { "grey14", 36, 36, 36, 1, SVGCompliance | X11Compliance },
    { "grey15", 38, 38, 38, 1, SVGCompliance | X11Compliance },
    { "grey16", 41, 41, 41, 1, SVGCompliance | X11Compliance },
    { "grey17", 43, 43, 43, 1, SVGCompliance | X11Compliance },
    { "grey18", 46, 46, 46, 1, SVGCompliance | X11Compliance },
    { "grey19", 48, 48, 48, 1, SVGCompliance | X11Compliance },
    { "grey2", 5, 5, 5, 1, SVGCompliance | X11Compliance },
    { "grey20", 51, 51, 51, 1, SVGCompliance | X11Compliance },
    { "grey21", 54, 54, 54, 1, SVGCompliance | X11Compliance },
    { "grey22", 56, 56, 56, 1, SVGCompliance | X11Compliance },
    { "grey23", 59, 59, 59, 1, SVGCompliance | X11Compliance },
    { "grey24", 61, 61, 61, 1, SVGCompliance | X11Compliance },
    { "grey25", 64, 64, 64, 1, SVGCompliance | X11Compliance },
    { "grey26", 66, 66, 66, 1, SVGCompliance | X11Compliance },
    { "grey27", 69, 69, 69, 1, SVGCompliance | X11Compliance },
    { "grey28", 71, 71, 71, 1, SVGCompliance | X11Compliance },
    { "grey29", 74, 74, 74, 1, SVGCompliance | X11Compliance },
    { "grey3", 8, 8, 8, 1, SVGCompliance | X11Compliance },
    { "grey30", 77, 77, 77, 1, SVGCompliance | X11Compliance },
    { "grey31", 79, 79, 79, 1, SVGCompliance | X11Compliance },
    { "grey32", 82, 82, 82, 1, SVGCompliance | X11Compliance },
    { "grey33", 84, 84, 84, 1, SVGCompliance | X11Compliance },
    { "grey34", 87, 87, 87, 1, SVGCompliance | X11Compliance },
    { "grey35", 89, 89, 89, 1, SVGCompliance | X11Compliance },
    { "grey36", 92, 92, 92, 1, SVGCompliance | X11Compliance },
    { "grey37", 94, 94, 94, 1, SVGCompliance | X11Compliance },
    { "grey38", 97, 97, 97, 1, SVGCompliance | X11Compliance },
    { "grey39", 99, 99, 99, 1, SVGCompliance | X11Compliance },
    { "grey4", 10, 10, 10, 1, SVGCompliance | X11Compliance },
    { "grey40", 102, 102, 102, 1, SVGCompliance | X11Compliance },
    { "grey41", 105, 105, 105, 1, SVGCompliance | X11Compliance },
    { "grey42", 107, 107, 107, 1, SVGCompliance | X11Compliance },
    { "grey43", 110, 110, 110, 1, SVGCompliance | X11Compliance },
    { "grey44", 112, 112, 112, 1, SVGCompliance | X11Compliance },
    { "grey45", 115, 115, 115, 1, SVGCompliance | X11Compliance },
    { "grey46", 117, 117, 117, 1, SVGCompliance | X11Compliance },
    { "grey47", 120, 120, 120, 1, SVGCompliance | X11Compliance },
    { "grey48", 122, 122, 122, 1, SVGCompliance | X11Compliance },
    { "grey49", 125, 125, 125, 1, SVGCompliance | X11Compliance },
    { "grey5", 13, 13, 13, 1, SVGCompliance | X11Compliance },
    { "grey50", 127, 127, 127, 1, SVGCompliance | X11Compliance },
    { "grey51", 130, 130, 130, 1, SVGCompliance | X11Compliance },
    { "grey52", 133, 133, 133, 1, SVGCompliance | X11Compliance },
    { "grey53", 135, 135, 135, 1, SVGCompliance | X11Compliance },
    { "grey54", 138, 138, 138, 1, SVGCompliance | X11Compliance },
    { "grey55", 140, 140, 140, 1, SVGCompliance | X11Compliance },
    { "grey56", 143, 143, 143, 1, SVGCompliance | X11Compliance },
    { "grey57", 145, 145, 145, 1, SVGCompliance | X11Compliance },
    { "grey58", 148, 148, 148, 1, SVGCompliance | X11Compliance },
    { "grey59", 150, 150, 150, 1, SVGCompliance | X11Compliance },
    { "grey6", 15, 15, 15, 1, SVGCompliance | X11Compliance },
    { "grey60", 153, 153, 153, 1, SVGCompliance | X11Compliance },
    { "grey61", 156, 156, 156, 1, SVGCompliance | X11Compliance },
    { "grey62", 158, 158, 158, 1, SVGCompliance | X11Compliance },
    { "grey63", 161, 161, 161, 1, SVGCompliance | X11Compliance },
    { "grey64", 163, 163, 163, 1, SVGCompliance | X11Compliance },
    { "grey65", 166, 166, 166, 1, SVGCompliance | X11Compliance },
    { "grey66", 168, 168, 168, 1, SVGCompliance | X11Compliance },
    { "grey67", 171, 171, 171, 1, SVGCompliance | X11Compliance },
    { "grey68", 173, 173, 173, 1, SVGCompliance | X11Compliance },
    { "grey69", 176, 176, 176, 1, SVGCompliance | X11Compliance },
    { "grey7", 18, 18, 18, 1, SVGCompliance | X11Compliance },
    { "grey70", 179, 179, 179, 1, SVGCompliance | X11Compliance },
    { "grey71", 181, 181, 181, 1, SVGCompliance | X11Compliance },
    { "grey72", 184, 184, 184, 1, SVGCompliance | X11Compliance },
    { "grey73", 186, 186, 186, 1, SVGCompliance | X11Compliance },
    { "grey74", 189, 189, 189, 1, SVGCompliance | X11Compliance },
    { "grey75", 191, 191, 191, 1, SVGCompliance | X11Compliance },
    { "grey76", 194, 194, 194, 1, SVGCompliance | X11Compliance },
    { "grey77", 196, 196, 196, 1, SVGCompliance | X11Compliance },
    { "grey78", 199, 199, 199, 1, SVGCompliance | X11Compliance },
    { "grey79", 201, 201, 201, 1, SVGCompliance | X11Compliance },
    { "grey8", 20, 20, 20, 1, SVGCompliance | X11Compliance },
    { "grey80", 204, 204, 204, 1, SVGCompliance | X11Compliance },
    { "grey81", 207, 207, 207, 1, SVGCompliance | X11Compliance },
    { "grey82", 209, 209, 209, 1, SVGCompliance | X11Compliance },
    { "grey83", 212, 212, 212, 1, SVGCompliance | X11Compliance },
    { "grey84", 214, 214, 214, 1, SVGCompliance | X11Compliance },
    { "grey85", 217, 217, 217, 1, SVGCompliance | X11Compliance },
    { "grey86", 219, 219, 219, 1, SVGCompliance | X11Compliance },
    { "grey87", 222, 222, 222, 1, SVGCompliance | X11Compliance },
    { "grey88", 224, 224, 224, 1, SVGCompliance | X11Compliance },
    { "grey89", 227, 227, 227, 1, SVGCompliance | X11Compliance },
    { "grey9", 23, 23, 23, 1, SVGCompliance | X11Compliance },
    { "grey90", 229, 229, 229, 1, SVGCompliance | X11Compliance },
    { "grey91", 232, 232, 232, 1, SVGCompliance | X11Compliance },
    { "grey92", 235, 235, 235, 1, SVGCompliance | X11Compliance },
    { "grey93", 237, 237, 237, 1, SVGCompliance | X11Compliance },
    { "grey94", 240, 240, 240, 1, SVGCompliance | X11Compliance },
    { "grey95", 242, 242, 242, 1, SVGCompliance | X11Compliance },
    { "grey96", 245, 245, 245, 1, SVGCompliance | X11Compliance },
    { "grey97", 247, 247, 247, 1, SVGCompliance | X11Compliance },
    { "grey98", 250, 250, 250, 1, SVGCompliance | X11Compliance },
    { "grey99", 252, 252, 252, 1, SVGCompliance | X11Compliance },
    { "honeydew", 240, 255, 240, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "honeydew1", 240, 255, 240, 1, X11Compliance },
    { "honeydew2", 224, 238, 224, 1, X11Compliance },
    { "honeydew3", 193, 205, 193, 1, X11Compliance },
    { "honeydew4", 131, 139, 131, 1, X11Compliance },
    { "HotPink", 255, 105, 180, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "HotPink1", 255, 110, 180, 1, X11Compliance },
    { "HotPink2", 238, 106, 167, 1, X11Compliance },
    { "HotPink3", 205, 96, 144, 1, X11Compliance },
    { "HotPink4", 139, 58, 98, 1, X11Compliance },
    { "IndianRed", 205, 92, 92, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "IndianRed1", 255, 106, 106, 1, X11Compliance },
    { "IndianRed2", 238, 99, 99, 1, X11Compliance },
    { "IndianRed3", 205, 85, 85, 1, X11Compliance },
    { "IndianRed4", 139, 58, 58, 1, X11Compliance },
    { "indigo", 75, 0, 130, 1, SVGCompliance },
    { "ivory", 255, 255, 240, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "ivory1", 255, 255, 240, 1, X11Compliance },
    { "ivory2", 238, 238, 224, 1, X11Compliance },
    { "ivory3", 205, 205, 193, 1, X11Compliance },
    { "ivory4", 139, 139, 131, 1, X11Compliance },
    { "khaki", 240, 230, 140, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "khaki1", 255, 246, 143, 1, X11Compliance },
    { "khaki2", 238, 230, 133, 1, X11Compliance },
    { "khaki3", 205, 198, 115, 1, X11Compliance },
    { "khaki4", 139, 134, 78, 1, X11Compliance },
    { "lavender", 230, 230, 250, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LavenderBlush", 255, 240, 245, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LavenderBlush1", 255, 240, 245, 1, X11Compliance },
    { "LavenderBlush2", 238, 224, 229, 1, X11Compliance },
    { "LavenderBlush3", 205, 193, 197, 1, X11Compliance },
    { "LavenderBlush4", 139, 131, 134, 1, X11Compliance },
    { "LawnGreen", 124, 252, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LemonChiffon", 255, 250, 205, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LemonChiffon1", 255, 250, 205, 1, X11Compliance },
    { "LemonChiffon2", 238, 233, 191, 1, X11Compliance },
    { "LemonChiffon3", 205, 201, 165, 1, X11Compliance },
    { "LemonChiffon4", 139, 137, 112, 1, X11Compliance },
    { "LightBlue", 173, 216, 230, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightBlue1", 191, 239, 255, 1, X11Compliance },
    { "LightBlue2", 178, 223, 238, 1, X11Compliance },
    { "LightBlue3", 154, 192, 205, 1, X11Compliance },
    { "LightBlue4", 104, 131, 139, 1, X11Compliance },
    { "LightCoral", 240, 128, 128, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightCyan", 224, 255, 255, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightCyan1", 224, 255, 255, 1, X11Compliance },
    { "LightCyan2", 209, 238, 238, 1, X11Compliance },
    { "LightCyan3", 180, 205, 205, 1, X11Compliance },
    { "LightCyan4", 122, 139, 139, 1, X11Compliance },
    { "LightGoldenrod", 238, 221, 130, 1, X11Compliance | XPMCompliance },
    { "LightGoldenrod1", 255, 236, 139, 1, X11Compliance },
    { "LightGoldenrod2", 238, 220, 130, 1, X11Compliance },
    { "LightGoldenrod3", 205, 190, 112, 1, X11Compliance },
    { "LightGoldenrod4", 139, 129, 76, 1, X11Compliance },
    { "LightGoldenrodYellow", 250, 250, 210, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightGray", 211, 211, 211, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightGreen", 144, 238, 144, 1, SVGCompliance | X11Compliance },
    { "LightGrey", 211, 211, 211, 1, SVGCompliance | X11Compliance },
    { "LightPink", 255, 182, 193, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightPink1", 255, 174, 185, 1, X11Compliance },
    { "LightPink2", 238, 162, 173, 1, X11Compliance },
    { "LightPink3", 205, 140, 149, 1, X11Compliance },
    { "LightPink4", 139, 95, 101, 1, X11Compliance },
    { "LightSalmon", 255, 160, 122, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightSalmon1", 255, 160, 122, 1, X11Compliance },
    { "LightSalmon2", 238, 149, 114, 1, X11Compliance },
    { "LightSalmon3", 205, 129, 98, 1, X11Compliance },
    { "LightSalmon4", 139, 87, 66, 1, X11Compliance },
    { "LightSeaGreen", 32, 178, 170, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightSkyBlue", 135, 206, 250, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightSkyBlue1", 176, 226, 255, 1, X11Compliance },
    { "LightSkyBlue2", 164, 211, 238, 1, X11Compliance },
    { "LightSkyBlue3", 141, 182, 205, 1, X11Compliance },
    { "LightSkyBlue4", 96, 123, 139, 1, X11Compliance },
    { "LightSlateBlue", 132, 112, 255, 1, X11Compliance | XPMCompliance },
    { "LightSlateGray", 119, 136, 153, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightSlateGrey", 119, 136, 153, 1, SVGCompliance | X11Compliance },
    { "LightSteelBlue", 176, 196, 222, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightSteelBlue1", 202, 225, 255, 1, X11Compliance },
    { "LightSteelBlue2", 188, 210, 238, 1, X11Compliance },
    { "LightSteelBlue3", 162, 181, 205, 1, X11Compliance },
    { "LightSteelBlue4", 110, 123, 139, 1, X11Compliance },
    { "LightYellow", 255, 255, 224, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "LightYellow1", 255, 255, 224, 1, X11Compliance },
    { "LightYellow2", 238, 238, 209, 1, X11Compliance },
    { "LightYellow3", 205, 205, 180, 1, X11Compliance },
    { "LightYellow4", 139, 139, 122, 1, X11Compliance },
    { "lime", 0, 255, 0, 1, SVGCompliance },
    { "LimeGreen", 50, 205, 50, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "linen", 250, 240, 230, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "magenta1", 255, 0, 255, 1, X11Compliance },
    { "magenta2", 238, 0, 238, 1, X11Compliance },
    { "magenta3", 205, 0, 205, 1, X11Compliance },
    { "magenta4", 139, 0, 139, 1, X11Compliance },
    { "maroon", 128, 0, 0, 1, SVGCompliance },
    { "maroon", 176, 48, 96, 1, X11Compliance | XPMCompliance },
    { "maroon1", 255, 52, 179, 1, X11Compliance },
    { "maroon2", 238, 48, 167, 1, X11Compliance },
    { "maroon3", 205, 41, 144, 1, X11Compliance },
    { "maroon4", 139, 28, 98, 1, X11Compliance },
    { "MediumAquamarine", 102, 205, 170, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumBlue", 0, 0, 205, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumForestGreen", 50, 129, 75, 1, X11Compliance | XPMCompliance },
    { "MediumGoldenRod", 209, 193, 102, 1, X11Compliance | XPMCompliance },
    { "MediumOrchid", 186, 85, 211, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumOrchid1", 224, 102, 255, 1, X11Compliance },
    { "MediumOrchid2", 209, 95, 238, 1, X11Compliance },
    { "MediumOrchid3", 180, 82, 205, 1, X11Compliance },
    { "MediumOrchid4", 122, 55, 139, 1, X11Compliance },
    { "MediumPurple", 147, 112, 219, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumPurple1", 171, 130, 255, 1, X11Compliance },
    { "MediumPurple2", 159, 121, 238, 1, X11Compliance },
    { "MediumPurple3", 137, 104, 205, 1, X11Compliance },
    { "MediumPurple4", 93, 71, 139, 1, X11Compliance },
    { "MediumSeaGreen", 60, 179, 113, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumSlateBlue", 123, 104, 238, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumSpringGreen", 0, 250, 154, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumTurquoise", 72, 209, 204, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MediumVioletRed", 199, 21, 133, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MidnightBlue", 25, 25, 112, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MintCream", 245, 255, 250, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MistyRose", 255, 228, 225, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "MistyRose1", 255, 228, 225, 1, X11Compliance },
    { "MistyRose2", 238, 213, 210, 1, X11Compliance },
    { "MistyRose3", 205, 183, 181, 1, X11Compliance },
    { "MistyRose4", 139, 125, 123, 1, X11Compliance },
    { "moccasin", 255, 228, 181, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "NavajoWhite", 255, 222, 173, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "NavajoWhite1", 255, 222, 173, 1, X11Compliance },
    { "NavajoWhite2", 238, 207, 161, 1, X11Compliance },
    { "NavajoWhite3", 205, 179, 139, 1, X11Compliance },
    { "NavajoWhite4", 139, 121, 94, 1, X11Compliance },
    { "navy", 0, 0, 128, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "NavyBlue", 0, 0, 128, 1, X11Compliance | XPMCompliance },
    { "matte", 0, 0, 0, 0, SVGCompliance },
    { "OldLace", 253, 245, 230, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "olive", 128, 128, 0, 1, SVGCompliance },
    { "OliveDrab", 107, 142, 35, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "OliveDrab1", 192, 255, 62, 1, X11Compliance },
    { "OliveDrab2", 179, 238, 58, 1, X11Compliance },
    { "OliveDrab3", 154, 205, 50, 1, X11Compliance },
    { "OliveDrab4", 105, 139, 34, 1, X11Compliance },
    { "opaque", 0, 0, 0, 1, SVGCompliance },
    { "orange", 255, 165, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "orange1", 255, 165, 0, 1, X11Compliance },
    { "orange2", 238, 154, 0, 1, X11Compliance },
    { "orange3", 205, 133, 0, 1, X11Compliance },
    { "orange4", 139, 90, 0, 1, X11Compliance },
    { "OrangeRed", 255, 69, 0, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "OrangeRed1", 255, 69, 0, 1, X11Compliance },
    { "OrangeRed2", 238, 64, 0, 1, X11Compliance },
    { "OrangeRed3", 205, 55, 0, 1, X11Compliance },
    { "OrangeRed4", 139, 37, 0, 1, X11Compliance },
    { "orchid", 218, 112, 214, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "orchid1", 255, 131, 250, 1, X11Compliance },
    { "orchid2", 238, 122, 233, 1, X11Compliance },
    { "orchid3", 205, 105, 201, 1, X11Compliance },
    { "orchid4", 139, 71, 137, 1, X11Compliance },
    { "PaleGoldenrod", 238, 232, 170, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "PaleGreen", 152, 251, 152, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "PaleGreen1", 154, 255, 154, 1, X11Compliance },
    { "PaleGreen2", 144, 238, 144, 1, X11Compliance },
    { "PaleGreen3", 124, 205, 124, 1, X11Compliance },
    { "PaleGreen4", 84, 139, 84, 1, X11Compliance },
    { "PaleTurquoise", 175, 238, 238, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "PaleTurquoise1", 187, 255, 255, 1, X11Compliance },
    { "PaleTurquoise2", 174, 238, 238, 1, X11Compliance },
    { "PaleTurquoise3", 150, 205, 205, 1, X11Compliance },
    { "PaleTurquoise4", 102, 139, 139, 1, X11Compliance },
    { "PaleVioletRed", 219, 112, 147, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "PaleVioletRed1", 255, 130, 171, 1, X11Compliance },
    { "PaleVioletRed2", 238, 121, 159, 1, X11Compliance },
    { "PaleVioletRed3", 205, 104, 137, 1, X11Compliance },
    { "PaleVioletRed4", 139, 71, 93, 1, X11Compliance },
    { "PapayaWhip", 255, 239, 213, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "PeachPuff", 255, 218, 185, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "PeachPuff1", 255, 218, 185, 1, X11Compliance },
    { "PeachPuff2", 238, 203, 173, 1, X11Compliance },
    { "PeachPuff3", 205, 175, 149, 1, X11Compliance },
    { "PeachPuff4", 139, 119, 101, 1, X11Compliance },
    { "peru", 205, 133, 63, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "pink", 255, 192, 203, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "pink1", 255, 181, 197, 1, X11Compliance },
    { "pink2", 238, 169, 184, 1, X11Compliance },
    { "pink3", 205, 145, 158, 1, X11Compliance },
    { "pink4", 139, 99, 108, 1, X11Compliance },
    { "plum", 221, 160, 221, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "plum1", 255, 187, 255, 1, X11Compliance },
    { "plum2", 238, 174, 238, 1, X11Compliance },
    { "plum3", 205, 150, 205, 1, X11Compliance },
    { "plum4", 139, 102, 139, 1, X11Compliance },
    { "PowderBlue", 176, 224, 230, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "purple", 128, 0, 128, 1, SVGCompliance },
    { "purple", 160, 32, 240, 1, X11Compliance | XPMCompliance },
    { "purple1", 155, 48, 255, 1, X11Compliance },
    { "purple2", 145, 44, 238, 1, X11Compliance },
    { "purple3", 125, 38, 205, 1, X11Compliance },
    { "purple4", 85, 26, 139, 1, X11Compliance },
    { "red1", 255, 0, 0, 1, X11Compliance },
    { "red2", 238, 0, 0, 1, X11Compliance },
    { "red3", 205, 0, 0, 1, X11Compliance },
    { "red4", 139, 0, 0, 1, X11Compliance },
    { "RosyBrown", 188, 143, 143, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "RosyBrown1", 255, 193, 193, 1, X11Compliance },
    { "RosyBrown2", 238, 180, 180, 1, X11Compliance },
    { "RosyBrown3", 205, 155, 155, 1, X11Compliance },
    { "RosyBrown4", 139, 105, 105, 1, X11Compliance },
    { "RoyalBlue", 65, 105, 225, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "RoyalBlue1", 72, 118, 255, 1, X11Compliance },
    { "RoyalBlue2", 67, 110, 238, 1, X11Compliance },
    { "RoyalBlue3", 58, 95, 205, 1, X11Compliance },
    { "RoyalBlue4", 39, 64, 139, 1, X11Compliance },
    { "SaddleBrown", 139, 69, 19, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "salmon", 250, 128, 114, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "salmon1", 255, 140, 105, 1, X11Compliance },
    { "salmon2", 238, 130, 98, 1, X11Compliance },
    { "salmon3", 205, 112, 84, 1, X11Compliance },
    { "salmon4", 139, 76, 57, 1, X11Compliance },
    { "SandyBrown", 244, 164, 96, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "SeaGreen", 46, 139, 87, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "SeaGreen1", 84, 255, 159, 1, X11Compliance },
    { "SeaGreen2", 78, 238, 148, 1, X11Compliance },
    { "SeaGreen3", 67, 205, 128, 1, X11Compliance },
    { "SeaGreen4", 46, 139, 87, 1, X11Compliance },
    { "seashell", 255, 245, 238, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "seashell1", 255, 245, 238, 1, X11Compliance },
    { "seashell2", 238, 229, 222, 1, X11Compliance },
    { "seashell3", 205, 197, 191, 1, X11Compliance },
    { "seashell4", 139, 134, 130, 1, X11Compliance },
    { "sienna", 160, 82, 45, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "sienna1", 255, 130, 71, 1, X11Compliance },
    { "sienna2", 238, 121, 66, 1, X11Compliance },
    { "sienna3", 205, 104, 57, 1, X11Compliance },
    { "sienna4", 139, 71, 38, 1, X11Compliance },
    { "silver", 192, 192, 192, 1, SVGCompliance },
    { "SkyBlue", 135, 206, 235, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "SkyBlue1", 135, 206, 255, 1, X11Compliance },
    { "SkyBlue2", 126, 192, 238, 1, X11Compliance },
    { "SkyBlue3", 108, 166, 205, 1, X11Compliance },
    { "SkyBlue4", 74, 112, 139, 1, X11Compliance },
    { "SlateBlue", 106, 90, 205, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "SlateBlue1", 131, 111, 255, 1, X11Compliance },
    { "SlateBlue2", 122, 103, 238, 1, X11Compliance },
    { "SlateBlue3", 105, 89, 205, 1, X11Compliance },
    { "SlateBlue4", 71, 60, 139, 1, X11Compliance },
    { "SlateGray", 112, 128, 144, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "SlateGray1", 198, 226, 255, 1, X11Compliance },
    { "SlateGray2", 185, 211, 238, 1, X11Compliance },
    { "SlateGray3", 159, 182, 205, 1, X11Compliance },
    { "SlateGray4", 108, 123, 139, 1, X11Compliance },
    { "SlateGrey", 112, 128, 144, 1, SVGCompliance | X11Compliance },
    { "snow", 255, 250, 250, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "snow1", 255, 250, 250, 1, X11Compliance },
    { "snow2", 238, 233, 233, 1, X11Compliance },
    { "snow3", 205, 201, 201, 1, X11Compliance },
    { "snow4", 139, 137, 137, 1, X11Compliance },
    { "SpringGreen", 0, 255, 127, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "SpringGreen1", 0, 255, 127, 1, X11Compliance },
    { "SpringGreen2", 0, 238, 118, 1, X11Compliance },
    { "SpringGreen3", 0, 205, 102, 1, X11Compliance },
    { "SpringGreen4", 0, 139, 69, 1, X11Compliance },
    { "SteelBlue", 70, 130, 180, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "SteelBlue1", 99, 184, 255, 1, X11Compliance },
    { "SteelBlue2", 92, 172, 238, 1, X11Compliance },
    { "SteelBlue3", 79, 148, 205, 1, X11Compliance },
    { "SteelBlue4", 54, 100, 139, 1, X11Compliance },
    { "tan", 210, 180, 140, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "tan1", 255, 165, 79, 1, X11Compliance },
    { "tan2", 238, 154, 73, 1, X11Compliance },
    { "tan3", 205, 133, 63, 1, X11Compliance },
    { "tan4", 139, 90, 43, 1, X11Compliance },
    { "teal", 0, 128, 128, 1, SVGCompliance },
    { "thistle", 216, 191, 216, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "thistle1", 255, 225, 255, 1, X11Compliance },
    { "thistle2", 238, 210, 238, 1, X11Compliance },
    { "thistle3", 205, 181, 205, 1, X11Compliance },
    { "thistle4", 139, 123, 139, 1, X11Compliance },
    { "tomato", 255, 99, 71, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "tomato1", 255, 99, 71, 1, X11Compliance },
    { "tomato2", 238, 92, 66, 1, X11Compliance },
    { "tomato3", 205, 79, 57, 1, X11Compliance },
    { "tomato4", 139, 54, 38, 1, X11Compliance },
    { "transparent", 0, 0, 0, 0, SVGCompliance },
    { "turquoise", 64, 224, 208, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "turquoise1", 0, 245, 255, 1, X11Compliance },
    { "turquoise2", 0, 229, 238, 1, X11Compliance },
    { "turquoise3", 0, 197, 205, 1, X11Compliance },
    { "turquoise4", 0, 134, 139, 1, X11Compliance },
    { "violet", 238, 130, 238, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "VioletRed", 208, 32, 144, 1, X11Compliance | XPMCompliance },
    { "VioletRed1", 255, 62, 150, 1, X11Compliance },
    { "VioletRed2", 238, 58, 140, 1, X11Compliance },
    { "VioletRed3", 205, 50, 120, 1, X11Compliance },
    { "VioletRed4", 139, 34, 82, 1, X11Compliance },
    { "wheat", 245, 222, 179, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "wheat1", 255, 231, 186, 1, X11Compliance },
    { "wheat2", 238, 216, 174, 1, X11Compliance },
    { "wheat3", 205, 186, 150, 1, X11Compliance },
    { "wheat4", 139, 126, 102, 1, X11Compliance },
    { "WhiteSmoke", 245, 245, 245, 1, SVGCompliance | X11Compliance | XPMCompliance },
    { "yellow1", 255, 255, 0, 1, X11Compliance },
    { "yellow2", 238, 238, 0, 1, X11Compliance },
    { "yellow3", 205, 205, 0, 1, X11Compliance },
    { "yellow4", 139, 139, 0, 1, X11Compliance },
    { "YellowGreen", 154, 205, 50, 1, SVGCompliance | X11Compliance | XPMCompliance }
  };

/*
  Static declarations.
*/
static LinkedListInfo
  *color_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *color_semaphore = (SemaphoreInfo *) NULL;

static volatile MagickBooleanType
  instantiate_color = MagickFalse;

/*
  Forward declarations.
*/
static MagickBooleanType
  InitializeColorList(ExceptionInfo *),
  LoadColorLists(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o l o r C o m p o n e n t G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorComponentGenesis() instantiates the color component.
%
%  The format of the ColorComponentGenesis method is:
%
%      MagickBooleanType ColorComponentGenesis(void)
%
*/
MagickExport MagickBooleanType ColorComponentGenesis(void)
{
  AcquireSemaphoreInfo(&color_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o l o r C o m p o n e n t T e r m i n u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorComponentTerminus() destroys the color component.
%
%  The format of the ColorComponentTerminus method is:
%
%      ColorComponentTerminus(void)
%
*/

static void *DestroyColorElement(void *color_info)
{
  register ColorInfo
    *p;

  p=(ColorInfo *) color_info;
  if (p->exempt  == MagickFalse)
    {
      if (p->path != (char *) NULL)
        p->path=DestroyString(p->path);
      if (p->name != (char *) NULL)
        p->name=DestroyString(p->name);
    }
  p=(ColorInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickExport void ColorComponentTerminus(void)
{
  if (color_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&color_semaphore);
  LockSemaphoreInfo(color_semaphore);
  if (color_list != (LinkedListInfo *) NULL)
    color_list=DestroyLinkedList(color_list,DestroyColorElement);
  instantiate_color=MagickFalse;
  UnlockSemaphoreInfo(color_semaphore);
  DestroySemaphoreInfo(&color_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C o l o r C o m p l i a n c e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetColorInfo() searches the color list for the specified name and standards
%  compliance and if found returns attributes for that color.
%
%  The format of the GetColorInfo method is:
%
%      const PixelPacket *GetColorInfo(const char *name,
%        const ComplianceType compliance,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the color name.
%
%    o compliance: Adhere to this color standard: SVG, X11, or XPM.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const ColorInfo *GetColorCompliance(const char *name,
  const ComplianceType compliance,ExceptionInfo *exception)
{
  char
    colorname[MaxTextExtent];

  register const ColorInfo
    *p;

  register char
    *q;

  assert(exception != (ExceptionInfo *) NULL);
  if ((color_list == (LinkedListInfo *) NULL) ||
      (instantiate_color == MagickFalse))
    if (InitializeColorList(exception) == MagickFalse)
      return((const ColorInfo *) NULL);
  if ((color_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(color_list) != MagickFalse))
    return((const ColorInfo *) NULL);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    return((const ColorInfo *) GetValueFromLinkedList(color_list,0));
  /*
    Strip names of whitespace.
  */
  (void) CopyMagickString(colorname,name,MaxTextExtent);
  for (q=colorname; *q != '\0'; q++)
  {
    if (isspace((int) ((unsigned char) *q)) == 0)
      continue;
    (void) CopyMagickString(q,q+1,MaxTextExtent);
    q--;
  }
  /*
    Search for color tag.
  */
  LockSemaphoreInfo(color_semaphore);
  ResetLinkedListIterator(color_list);
  p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  while (p != (const ColorInfo *) NULL)
  {
    if (((p->compliance & compliance) != 0) &&
        (LocaleCompare(colorname,p->name) == 0))
      break;
    p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  }
  if (p == (ColorInfo *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
      "UnrecognizedColor","`%s'",name);
  else
    (void) InsertValueInLinkedList(color_list,0,
      RemoveElementByValueFromLinkedList(color_list,p));
  UnlockSemaphoreInfo(color_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C o l o r I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetColorInfo() searches the color list for the specified name and if found
%  returns attributes for that color.
%
%  The format of the GetColorInfo method is:
%
%      const PixelPacket *GetColorInfo(const char *name,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o color_info: search the color list for the specified name and if found
%      return attributes for that color.
%
%    o name: the color name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const ColorInfo *GetColorInfo(const char *name,
  ExceptionInfo *exception)
{
  return(GetColorCompliance(name,AllCompliance,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n c a t e n a t e C o l o r C o m p o n e n t                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConcatenateColorComponent() returns the pixel as a canonical string.
%
%  The format of the ConcatenateColorComponent() method is:
%
%      void ConcatenateColorComponent(const MagickPixelPacket *pixel,
%        const ChannelType channel,const ComplianceType compliance,char *tuple)
%
%  A description of each parameter follows.
%
%    o pixel:  The pixel.
%
%    channel:  The channel.
%
%    o compliance: Adhere to this color standard: SVG, X11, or XPM.
%
%    tuple:  The color tuple.
%
*/
MagickExport void ConcatenateColorComponent(const MagickPixelPacket *pixel,
  const ChannelType channel,const ComplianceType compliance,char *tuple)
{
  char
    component[MaxTextExtent];

  double
    color;

  color=0.0;
  switch (channel)
  {
    case RedChannel:
    {
      color=pixel->red;
      break;
    }
    case GreenChannel:
    {
      color=pixel->green;
      break;
    }
    case BlueChannel:
    {
      color=pixel->blue;
      break;
    }
    case AlphaChannel:
    {
      color=QuantumRange-pixel->opacity;
      break;
    }
    case IndexChannel:
    {
      color=pixel->index;
      break;
    }
    default:
      break;
  }
  if (compliance == NoCompliance)
    {
      (void) FormatLocaleString(component,MaxTextExtent,"%.*g",
        GetMagickPrecision(),color);
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (compliance != SVGCompliance)
    {
      if (pixel->depth > 16)
        {
          (void) FormatLocaleString(component,MaxTextExtent,"%10lu",
            (unsigned long) ScaleQuantumToLong(ClampToQuantum(color)));
          (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
          return;
        }
      if (pixel->depth > 8)
        {
          (void) FormatLocaleString(component,MaxTextExtent,"%5d",
            ScaleQuantumToShort(ClampToQuantum(color)));
          (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
          return;
        }
      (void) FormatLocaleString(component,MaxTextExtent,"%3d",
        ScaleQuantumToChar(ClampToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (channel == OpacityChannel)
    {
      (void) FormatLocaleString(component,MaxTextExtent,"%.*g",
        GetMagickPrecision(),(QuantumScale*color));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if ((pixel->colorspace == HCLColorspace) ||
      (pixel->colorspace == HCLpColorspace) ||
      (pixel->colorspace == HSBColorspace) ||
      (pixel->colorspace == HSIColorspace) ||
      (pixel->colorspace == HSLColorspace) ||
      (pixel->colorspace == HSVColorspace) ||
      (pixel->colorspace == HWBColorspace))
    {
      (void) FormatLocaleString(component,MaxTextExtent,"%.*g%%",
        GetMagickPrecision(),(100.0*QuantumScale*color));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (pixel->depth > 8)
    {
      (void) FormatLocaleString(component,MaxTextExtent,"%.*g%%",
        GetMagickPrecision(),(100.0*QuantumScale*color));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  (void) FormatLocaleString(component,MaxTextExtent,"%d",
    ScaleQuantumToChar(ClampToQuantum(color)));
  (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o l o r I n f o L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetColorInfoList() returns any colors that match the specified pattern.
%
%  The format of the GetColorInfoList function is:
%
%      const ColorInfo **GetColorInfoList(const char *pattern,
%        size_t *number_colors,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_colors:  This integer returns the number of colors in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ColorInfoCompare(const void *x,const void *y)
{
  const ColorInfo
    **p,
    **q;

  p=(const ColorInfo **) x,
  q=(const ColorInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->name,(*q)->name));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const ColorInfo **GetColorInfoList(const char *pattern,
  size_t *number_colors,ExceptionInfo *exception)
{
  const ColorInfo
    **colors;

  register const ColorInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate color list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_colors != (size_t *) NULL);
  *number_colors=0;
  p=GetColorInfo("*",exception);
  if (p == (const ColorInfo *) NULL)
    return((const ColorInfo **) NULL);
  colors=(const ColorInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(color_list)+1UL,sizeof(*colors));
  if (colors == (const ColorInfo **) NULL)
    return((const ColorInfo **) NULL);
  /*
    Generate color list.
  */
  LockSemaphoreInfo(color_semaphore);
  ResetLinkedListIterator(color_list);
  p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  for (i=0; p != (const ColorInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      colors[i++]=p;
    p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  }
  UnlockSemaphoreInfo(color_semaphore);
  qsort((void *) colors,(size_t) i,sizeof(*colors),ColorInfoCompare);
  colors[i]=(ColorInfo *) NULL;
  *number_colors=(size_t) i;
  return(colors);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o l o r L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetColorList() returns any colors that match the specified pattern.
%
%  The format of the GetColorList function is:
%
%      char **GetColorList(const char *pattern,size_t *number_colors,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_colors:  This integer returns the number of colors in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ColorCompare(const void *x,const void *y)
{
  register const char
    **p,
    **q;

  p=(const char **) x;
  q=(const char **) y;
  return(LocaleCompare(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport char **GetColorList(const char *pattern,
  size_t *number_colors,ExceptionInfo *exception)
{
  char
    **colors;

  register const ColorInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate color list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_colors != (size_t *) NULL);
  *number_colors=0;
  p=GetColorInfo("*",exception);
  if (p == (const ColorInfo *) NULL)
    return((char **) NULL);
  colors=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(color_list)+1UL,sizeof(*colors));
  if (colors == (char **) NULL)
    return((char **) NULL);
  /*
    Generate color list.
  */
  LockSemaphoreInfo(color_semaphore);
  ResetLinkedListIterator(color_list);
  p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  for (i=0; p != (const ColorInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      colors[i++]=ConstantString(p->name);
    p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  }
  UnlockSemaphoreInfo(color_semaphore);
  qsort((void *) colors,(size_t) i,sizeof(*colors),ColorCompare);
  colors[i]=(char *) NULL;
  *number_colors=(size_t) i;
  return(colors);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C o l o r T u p l e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetColorTuple() returns a color as a color tuple string (e.g. rgba(255,0,0))
%  or hex string (e.g. #FF0000).
%
%  The format of the GetColorTuple method is:
%
%      GetColorTuple(const MagickPixelPacket *pixel,const MagickBooleanType hex,
%        char *tuple)
%
%  A description of each parameter follows.
%
%    o pixel: the pixel.
%
%    o hex: A value other than zero returns the tuple in a hexidecimal format.
%
%    o tuple: Return the color tuple as this string.
%
*/

static void ConcatentateHexColorComponent(const MagickPixelPacket *pixel,
  const ChannelType channel,char *tuple)
{
  char
    component[MaxTextExtent];

  MagickRealType
    color;

  color=0.0;
  switch (channel)
  {
    case RedChannel:
    {
      color=pixel->red;
      break;
    }
    case GreenChannel:
    {
      color=pixel->green;
      break;
    }
    case BlueChannel:
    {
      color=pixel->blue;
      break;
    }
    case OpacityChannel:
    {
      color=(MagickRealType) QuantumRange-pixel->opacity;
      break;
    }
    case IndexChannel:
    {
      color=pixel->index;
      break;
    }
    default:
      break;
  }
  if (pixel->depth > 32)
    {
      (void) FormatLocaleString(component,MaxTextExtent,"%08lX%08lX",
        (unsigned long) ScaleQuantumToLong(ClampToQuantum(color)),
        (unsigned long) ScaleQuantumToLong(ClampToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (pixel->depth > 16)
    {
      (void) FormatLocaleString(component,MaxTextExtent,"%08X",
        (unsigned int) ScaleQuantumToLong(ClampToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (pixel->depth > 8)
    {
      (void) FormatLocaleString(component,MaxTextExtent,"%04X",
        ScaleQuantumToShort(ClampToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  (void) FormatLocaleString(component,MaxTextExtent,"%02X",
    ScaleQuantumToChar(ClampToQuantum(color)));
  (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
  return;
}

MagickExport void GetColorTuple(const MagickPixelPacket *pixel,
  const MagickBooleanType hex,char *tuple)
{
  MagickPixelPacket
    color;

  assert(pixel != (const MagickPixelPacket *) NULL);
  assert(tuple != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",tuple);
  *tuple='\0';
  if (hex != MagickFalse)
    {
      /*
        Convert pixel to hex color.
      */
      (void) ConcatenateMagickString(tuple,"#",MaxTextExtent);
      ConcatentateHexColorComponent(pixel,RedChannel,tuple);
      ConcatentateHexColorComponent(pixel,GreenChannel,tuple);
      ConcatentateHexColorComponent(pixel,BlueChannel,tuple);
      if (pixel->colorspace == CMYKColorspace)
        ConcatentateHexColorComponent(pixel,IndexChannel,tuple);
      if ((pixel->matte != MagickFalse) && (pixel->opacity != OpaqueOpacity))
        ConcatentateHexColorComponent(pixel,OpacityChannel,tuple);
      return;
    }
  /*
    Convert pixel to rgb() or cmyk() color.
  */
  color=(*pixel);
  if (color.depth > 8)
    {
#define SVGCompliant(component) ((MagickRealType) \
   ScaleCharToQuantum(ScaleQuantumToChar(ClampToQuantum(component))));

      MagickStatusType
        status;

      /*
        SVG requires color depths > 8 expressed as percentages.
      */
      status=color.red == SVGCompliant(color.red);
      status&=color.green == SVGCompliant(color.green);
      status&=color.blue == SVGCompliant(color.blue);
      if (color.colorspace != CMYKColorspace)
        status&=color.index == SVGCompliant(color.index);
      if (color.matte != MagickFalse)
        status&=color.opacity == SVGCompliant(color.opacity);
      if (status != MagickFalse)
        color.depth=8;
    }
  (void) ConcatenateMagickString(tuple,CommandOptionToMnemonic(
    MagickColorspaceOptions,(ssize_t) color.colorspace),MaxTextExtent);
  if (color.matte != MagickFalse)
    (void) ConcatenateMagickString(tuple,"a",MaxTextExtent);
  (void) ConcatenateMagickString(tuple,"(",MaxTextExtent);
  if (color.colorspace == GRAYColorspace)
    ConcatenateColorComponent(&color,GrayChannel,SVGCompliance,tuple);
  else
    {
      ConcatenateColorComponent(&color,RedChannel,SVGCompliance,tuple);
      (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
      ConcatenateColorComponent(&color,GreenChannel,SVGCompliance,tuple);
      (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
      ConcatenateColorComponent(&color,BlueChannel,SVGCompliance,tuple);
    }
  if (color.colorspace == CMYKColorspace)
    {
      (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
      ConcatenateColorComponent(&color,IndexChannel,SVGCompliance,tuple);
    }
  if (color.matte != MagickFalse)
    {
      (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
      ConcatenateColorComponent(&color,AlphaChannel,SVGCompliance,tuple);
    }
  (void) ConcatenateMagickString(tuple,")",MaxTextExtent);
  LocaleLower(tuple);
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e C o l o r L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeColorList() initializes the color list.
%
%  The format of the InitializeColorList method is:
%
%      MagickBooleanType InitializeColorList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType InitializeColorList(ExceptionInfo *exception)
{
  if ((color_list == (LinkedListInfo *) NULL) &&
      (instantiate_color == MagickFalse))
    {
      if (color_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&color_semaphore);
      LockSemaphoreInfo(color_semaphore);
      if ((color_list == (LinkedListInfo *) NULL) &&
          (instantiate_color == MagickFalse))
        {
          (void) LoadColorLists(ColorFilename,exception);
          instantiate_color=MagickTrue;
        }
      UnlockSemaphoreInfo(color_semaphore);
    }
  return(color_list != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s C o l o r S i m i l a r                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsColorSimilar() returns MagickTrue if the distance between two colors is
%  less than the specified distance in a linear three dimensional color space.
%  This method is used by ColorFloodFill() and other algorithms which
%  compare two colors.
%
%  The format of the IsColorSimilar method is:
%
%      void IsColorSimilar(const Image *image,const PixelPacket *p,
%        const PixelPacket *q)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

MagickExport MagickBooleanType IsColorSimilar(const Image *image,
  const PixelPacket *p,const PixelPacket *q)
{
  MagickRealType
    fuzz,
    pixel;

  register MagickRealType
    distance,
    scale;

  if ((image->fuzz == 0.0) && (image->matte == MagickFalse))
    return(IsColorEqual(p,q));
  fuzz=MagickMax(image->fuzz,MagickSQ1_2)*MagickMax(image->fuzz,MagickSQ1_2);
  scale=1.0;
  distance=0.0;
  if (image->matte != MagickFalse)
    {
      /*
        Transparencies are involved - set alpha distance
      */
      pixel=(MagickRealType) ((image->matte != MagickFalse ? GetPixelOpacity(p) :
        OpaqueOpacity)-(image->matte != MagickFalse ? q->opacity :
        OpaqueOpacity));
      distance=pixel*pixel;
      if (distance > fuzz)
        return(MagickFalse);
      /*
        Generate a alpha scaling factor to generate a 4D cone on colorspace
        Note that if one color is transparent, distance has no color component.
      */
      scale=(QuantumScale*GetPixelAlpha(p));
      scale*=(QuantumScale*GetPixelAlpha(q));
      if (scale <= MagickEpsilon)
        return(MagickTrue);
    }
  /*
    RGB or CMY color cube
  */
  distance*=3.0;  /* rescale appropriately */
  fuzz*=3.0;
  pixel=(MagickRealType) GetPixelRed(p)-GetPixelRed(q);
  if ((image->colorspace == HCLColorspace) ||
      (image->colorspace == HSBColorspace) ||
      (image->colorspace == HSLColorspace) ||
      (image->colorspace == HWBColorspace))
    {
      /* This calculates a arc distance for hue.  Really it should be a vector
         angle of 'S'/'W' length with 'L'/'B' forming appropriate cones.  In
         other words this is a hack - Anthony
      */
      if (fabs((double) pixel) > (QuantumRange/2))
        pixel-=QuantumRange;
      pixel*=2;
    }
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=(MagickRealType) GetPixelGreen(p)-q->green;
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=(MagickRealType) GetPixelBlue(p)-q->blue;
  distance+=scale*pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s I m a g e S i m i l a r                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageSimilar() returns true if the target is similar to a region of the
%  image.
%
%  The format of the IsImageSimilar method is:
%
%      MagickBooleanType IsImageSimilar(const Image *image,
%        const Image *target_image,ssize_t *x_offset,ssize_t *y_offset,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target_image: the target image.
%
%    o x_offset: On input the starting x position to search for a match;
%      on output the x position of the first match found.
%
%    o y_offset: On input the starting y position to search for a match;
%      on output the y position of the first match found.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsImageSimilar(const Image *image,
  const Image *target_image,ssize_t *x_offset,ssize_t *y_offset,
  ExceptionInfo *exception)
{
#define SearchImageText  "  Searching image...  "

  CacheView
    *image_view,
    *target_view;

  MagickBooleanType
    status;

  MagickPixelPacket
    target,
    pixel;

  register const PixelPacket
    *p,
    *q;

  register const IndexPacket
    *indexes,
    *target_indexes;

  register ssize_t
    i,
    x;

  ssize_t
    j,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(target_image != (Image *) NULL);
  assert(target_image->signature == MagickSignature);
  assert(x_offset != (ssize_t *) NULL);
  assert(y_offset != (ssize_t *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  x=0;
  status=MagickTrue;
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireVirtualCacheView(image,exception);
  target_view=AcquireVirtualCacheView(target_image,exception);
  for (y=(*y_offset); y < (ssize_t) image->rows; y++)
  {
    for (x=y == 0 ? *x_offset : 0; x < (ssize_t) image->columns; x++)
    {
      for (j=0; j < (ssize_t) target_image->rows; j++)
      {
        for (i=0; i < (ssize_t) target_image->columns; i++)
        {
          p=GetCacheViewVirtualPixels(image_view,x+i,y+j,1,1,exception);
          indexes=GetCacheViewVirtualIndexQueue(image_view);
          SetMagickPixelPacket(image,p,indexes,&pixel);
          q=GetCacheViewVirtualPixels(target_view,i,j,1,1,exception);
          target_indexes=GetCacheViewVirtualIndexQueue(target_view);
          SetMagickPixelPacket(image,q,target_indexes,&target);
          if (IsMagickColorSimilar(&pixel,&target) == MagickFalse)
            break;
        }
        if (i < (ssize_t) target_image->columns)
          break;
      }
      if (j == (ssize_t) target_image->rows)
        break;
    }
    if (x < (ssize_t) image->columns)
      break;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,SearchImageText,(MagickOffsetType) y,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  target_view=DestroyCacheView(target_view);
  image_view=DestroyCacheView(image_view);
  *x_offset=x;
  *y_offset=y;
  if (status == MagickFalse)
    return(status);
  return(y < (ssize_t) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s M a g i c k C o l o r S i m i l a r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickColorSimilar() returns true if the distance between two colors is
%  less than the specified distance in a linear three dimensional color space.
%  This method is used by ColorFloodFill() and other algorithms which
%  compare two colors.
%
%  This implements the equivalent of...
%    fuzz < sqrt( color_distance^2 * u.a*v.a  + alpha_distance^2 )
%
%  Which produces a multi-dimensional cone for that colorspace along the
%  transparency vector.
%
%  For example for an RGB
%    color_distance^2  = ( (u.r-v.r)^2 + (u.g-v.g)^2 + (u.b-v.b)^2 ) / 3
%
%  See http://www.imagemagick.org/Usage/bugs/fuzz_distance/
%
%  Hue colorspace distances need more work.  Hue is not a distance, it is an
%  angle!
%
%  A check that q is in the same color space as p should be made and the
%  appropriate mapping made.  -- Anthony Thyssen  8 December 2010
%
%  The format of the IsMagickColorSimilar method is:
%
%      MagickBooleanType IsMagickColorSimilar(const MagickPixelPacket *p,
%        const MagickPixelPacket *q)
%
%  A description of each parameter follows:
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType IsMagickColorSimilar(const MagickPixelPacket *p,
  const MagickPixelPacket *q)
{
  MagickRealType
    fuzz,
    pixel;

  register MagickRealType
    scale,
    distance;

  if ((p->fuzz == 0.0) && (q->fuzz == 0.0))
    return(IsMagickColorEqual(p,q));
  if (p->fuzz == 0.0)
    fuzz=MagickMax(q->fuzz,MagickSQ1_2)*MagickMax(q->fuzz,MagickSQ1_2);
  else if (q->fuzz == 0.0)
    fuzz=MagickMax(p->fuzz,MagickSQ1_2)*MagickMax(p->fuzz,MagickSQ1_2);
  else
    fuzz=MagickMax(p->fuzz,MagickSQ1_2)*MagickMax(q->fuzz,MagickSQ1_2);
  scale=1.0;
  distance=0.0;
  if ((p->matte != MagickFalse) || (q->matte != MagickFalse))
    {
      /*
        Transparencies are involved - set alpha distance.
      */
      pixel=(p->matte != MagickFalse ? GetPixelOpacity(p) : OpaqueOpacity)-
        (q->matte != MagickFalse ? q->opacity : OpaqueOpacity);
      distance=pixel*pixel;
      if (distance > fuzz)
        return(MagickFalse);
      /*
        Generate a alpha scaling factor to generate a 4D cone on colorspace.
        Note that if one color is transparent, distance has no color component
      */
      if (p->matte != MagickFalse)
        scale=(QuantumScale*GetPixelAlpha(p));
      if (q->matte != MagickFalse)
        scale*=(QuantumScale*GetPixelAlpha(q));
      if ( scale <= MagickEpsilon )
        return(MagickTrue);
    }
  /*
    CMYK create a CMY cube with a multi-dimensional cone toward black.
  */
  if (p->colorspace == CMYKColorspace)
    {
      pixel=p->index-q->index;
      distance+=pixel*pixel*scale;
      if (distance > fuzz)
        return(MagickFalse);
      scale*=(MagickRealType) (QuantumScale*(QuantumRange-p->index));
      scale*=(MagickRealType) (QuantumScale*(QuantumRange-q->index));
    }
  /*
    RGB or CMY color cube.
  */
  distance*=3.0;  /* rescale appropriately */
  fuzz*=3.0;
  pixel=p->red-q->red;
  if ((p->colorspace == HCLColorspace) || (p->colorspace == HSBColorspace) ||
      (p->colorspace == HSLColorspace) || (p->colorspace == HWBColorspace))
    {
      /* This calculates a arc distance for hue
         Really if should be a vector angle of 'S'/'W' length
         with 'L'/'B' forming appropriate cones.
         In other words this is a hack - Anthony
      */
      if (fabs((double) pixel) > (QuantumRange/2))
        pixel-=QuantumRange;
      pixel*=2;
    }
  distance+=pixel*pixel*scale;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=GetPixelGreen(p)-q->green;
  distance+=pixel*pixel*scale;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=GetPixelBlue(p)-q->blue;
  distance+=pixel*pixel*scale;
  if (distance > fuzz)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s O p a c i t y S i m i l a r                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsOpacitySimilar() returns true if the distance between two opacity
%  values is less than the specified distance in a linear color space.  This
%  method is used by MatteFloodFill() and other algorithms which compare
%  two opacity values.
%
%  The format of the IsOpacitySimilar method is:
%
%      void IsOpacitySimilar(const Image *image,const PixelPacket *p,
%        const PixelPacket *q)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType IsOpacitySimilar(const Image *image,
  const PixelPacket *p,const PixelPacket *q)
{
  MagickRealType
    fuzz,
    pixel;

  register MagickRealType
    distance;

  if (image->matte == MagickFalse)
    return(MagickTrue);
  if (GetPixelOpacity(p) == q->opacity)
    return(MagickTrue);
  fuzz=MagickMax(image->fuzz,MagickSQ1_2)*MagickMax(image->fuzz,MagickSQ1_2);
  pixel=(MagickRealType) GetPixelOpacity(p)-(MagickRealType) q->opacity;
  distance=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t C o l o r I n f o                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListColorInfo() lists color names to the specified file.  Color names
%  are a convenience.  Rather than defining a color by its red, green, and
%  blue intensities just use a color name such as white, blue, or yellow.
%
%  The format of the ListColorInfo method is:
%
%      MagickBooleanType ListColorInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  List color names to this file handle.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListColorInfo(FILE *file,
  ExceptionInfo *exception)
{
  char
    tuple[MaxTextExtent];

  const char
    *path;

  const ColorInfo
    **color_info;

  register ssize_t
    i;

  size_t
    number_colors;

  /*
    List name and attributes of each color in the list.
  */
  if (file == (const FILE *) NULL)
    file=stdout;
  color_info=GetColorInfoList("*",&number_colors,exception);
  if (color_info == (const ColorInfo **) NULL)
    return(MagickFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_colors; i++)
  {
    if (color_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,color_info[i]->path) != 0))
      {
        if (color_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",color_info[i]->path);
        (void) FormatLocaleFile(file,
          "Name                  Color                  "
          "                       Compliance\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=color_info[i]->path;
    (void) FormatLocaleFile(file,"%-21.21s ",color_info[i]->name);
    GetColorTuple(&color_info[i]->color,MagickFalse,tuple);
    (void) FormatLocaleFile(file,"%-45.45s ",tuple);
    if ((color_info[i]->compliance & SVGCompliance) != 0)
      (void) FormatLocaleFile(file,"SVG ");
    if ((color_info[i]->compliance & X11Compliance) != 0)
      (void) FormatLocaleFile(file,"X11 ");
    if ((color_info[i]->compliance & XPMCompliance) != 0)
      (void) FormatLocaleFile(file,"XPM ");
    (void) FormatLocaleFile(file,"\n");
  }
  color_info=(const ColorInfo **) RelinquishMagickMemory((void *) color_info);
  (void) fflush(file);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d C o l o r L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadColorList() loads the color configuration file which provides a mapping
%  between color attributes and a color name.
%
%  The format of the LoadColorList method is:
%
%      MagickBooleanType LoadColorList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The color list in XML format.
%
%    o filename:  The color list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadColorList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MaxTextExtent],
    *token;

  ColorInfo
    *color_info;

  const char
    *q;

  MagickBooleanType
    status;

  /*
    Load the color map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading color file \"%s\" ...",filename);
  if (xml == (char *) NULL)
    return(MagickFalse);
  if (color_list == (LinkedListInfo *) NULL)
    {
      color_list=NewLinkedList(0);
      if (color_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  color_info=(ColorInfo *) NULL;
  token=AcquireString(xml);
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetMagickToken(q,&q,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MaxTextExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MaxTextExtent);
          GetMagickToken(q,&q,token);
          if (*token != '=')
            continue;
          GetMagickToken(q,&q,token);
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > 200)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ConfigureError,"IncludeElementNestedTooDeeply","`%s'",token);
              else
                {
                  char
                    path[MaxTextExtent],
                    *xml;

                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateMagickString(path,DirectorySeparator,
                      MaxTextExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyMagickString(path,token,MaxTextExtent);
                  else
                    (void) ConcatenateMagickString(path,token,MaxTextExtent);
                  xml=FileToString(path,~0UL,exception);
                  if (xml != (char *) NULL)
                    {
                      status=LoadColorList(xml,path,depth+1,exception);
                      xml=(char *) RelinquishMagickMemory(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<color") == 0)
      {
        /*
          Color element.
        */
        color_info=(ColorInfo *) AcquireMagickMemory(sizeof(*color_info));
        if (color_info == (ColorInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(color_info,0,sizeof(*color_info));
        color_info->path=ConstantString(filename);
        color_info->exempt=MagickFalse;
        color_info->signature=MagickSignature;
        continue;
      }
    if (color_info == (ColorInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        status=AppendValueToLinkedList(color_list,color_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            color_info->name);
        color_info=(ColorInfo *) NULL;
      }
    GetMagickToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetMagickToken(q,&q,token);
    GetMagickToken(q,&q,token);
    switch (*keyword)
    {
      case 'C':
      case 'c':
      {
        if (LocaleCompare((char *) keyword,"color") == 0)
          {
            (void) QueryMagickColor(token,&color_info->color,exception);
            break;
          }
        if (LocaleCompare((char *) keyword,"compliance") == 0)
          {
            ssize_t
              compliance;

            compliance=color_info->compliance;
            if (GlobExpression(token,"*SVG*",MagickTrue) != MagickFalse)
              compliance|=SVGCompliance;
            if (GlobExpression(token,"*X11*",MagickTrue) != MagickFalse)
              compliance|=X11Compliance;
            if (GlobExpression(token,"*XPM*",MagickTrue) != MagickFalse)
              compliance|=XPMCompliance;
            color_info->compliance=(ComplianceType) compliance;
            break;
          }
        break;
      }
      case 'N':
      case 'n':
      {
        if (LocaleCompare((char *) keyword,"name") == 0)
          {
            color_info->name=ConstantString(token);
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            color_info->stealth=IsMagickTrue(token);
            break;
          }
        break;
      }
      default:
        break;
    }
  }
  token=(char *) RelinquishMagickMemory(token);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L o a d C o l o r L i s t s                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadColorList() loads one or more color configuration file which provides a
%  mapping between color attributes and a color name.
%
%  The format of the LoadColorLists method is:
%
%      MagickBooleanType LoadColorLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadColorLists(const char *filename,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  register ssize_t
    i;

  /*
    Load external color map.
  */
  if (color_list == (LinkedListInfo *) NULL)
    {
      color_list=NewLinkedList(0);
      if (color_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status&=LoadColorList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  /*
    Load built-in color map.
  */
  for (i=0; i < (ssize_t) (sizeof(ColorMap)/sizeof(*ColorMap)); i++)
  {
    ColorInfo
      *color_info;

    register const ColorMapInfo
      *p;

    p=ColorMap+i;
    color_info=(ColorInfo *) AcquireMagickMemory(sizeof(*color_info));
    if (color_info == (ColorInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",color_info->name);
        continue;
      }
    (void) ResetMagickMemory(color_info,0,sizeof(*color_info));
    color_info->path=(char *) "[built-in]";
    color_info->name=(char *) p->name;
    GetMagickPixelPacket((Image *) NULL,&color_info->color);
    color_info->color.red=(MagickRealType) ScaleCharToQuantum(
      GetPixelRed(p));
    color_info->color.green=(MagickRealType) ScaleCharToQuantum(
      GetPixelGreen(p));
    color_info->color.blue=(MagickRealType) ScaleCharToQuantum(
      GetPixelBlue(p));
    color_info->color.opacity=(MagickRealType) (QuantumRange-QuantumRange*
      p->alpha);
    color_info->compliance=(ComplianceType) p->compliance;
    color_info->exempt=MagickTrue;
    color_info->signature=MagickSignature;
    status&=AppendValueToLinkedList(color_list,color_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",color_info->name);
  }
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Q u e r y C o l o r C o m p l i e n c e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueryColorCompliance() returns the red, green, blue, and opacity intensities
%  for a given color name.
%
%  The format of the QueryColorCompliance method is:
%
%      MagickBooleanType QueryColorCompliance(const char *name,
%        const ComplianceType compliance,PixelPacket *color,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the color name (e.g. white, blue, yellow).
%
%    o compliance: Adhere to this color standard: SVG, X11, or XPM.
%
%    o color: the red, green, blue, and opacity intensities values of the
%      named color in this structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport MagickBooleanType QueryColorCompliance(const char *name,
  const ComplianceType compliance,PixelPacket *color,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  status=QueryMagickColorCompliance(name,compliance,&pixel,exception);
  SetPixelOpacity(color,ClampToQuantum(pixel.opacity));
  if (pixel.colorspace == CMYKColorspace)
    {
      SetPixelRed(color,ClampToQuantum((MagickRealType)
        (QuantumRange-MagickMin(QuantumRange,(MagickRealType) (QuantumScale*
        pixel.red*(QuantumRange-pixel.index)+pixel.index)))));
      SetPixelGreen(color,ClampToQuantum((MagickRealType)
        (QuantumRange-MagickMin(QuantumRange,(MagickRealType) (QuantumScale*
        pixel.green*(QuantumRange-pixel.index)+pixel.index)))));
      SetPixelBlue(color,ClampToQuantum((MagickRealType)
        (QuantumRange-MagickMin(QuantumRange,(MagickRealType) (QuantumScale*
        pixel.blue*(QuantumRange-pixel.index)+pixel.index)))));
      return(status);
    }
  SetPixelRed(color,ClampToQuantum(pixel.red));
  SetPixelGreen(color,ClampToQuantum(pixel.green));
  SetPixelBlue(color,ClampToQuantum(pixel.blue));
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Q u e r y C o l o r D a t a b a s e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueryColorDatabase() returns the red, green, blue, and opacity intensities
%  for a given color name.
%
%  The format of the QueryColorDatabase method is:
%
%      MagickBooleanType QueryColorDatabase(const char *name,PixelPacket *color,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the color name (e.g. white, blue, yellow).
%
%    o color: the red, green, blue, and opacity intensities values of the
%      named color in this structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType QueryColorDatabase(const char *name,
  PixelPacket *color,ExceptionInfo *exception)
{
  return(QueryColorCompliance(name,AllCompliance,color,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  Q u e r y C o l o r n a m e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueryColorname() returns a named color for the given color intensity.  If
%  an exact match is not found, a rgb() color is returned instead.
%
%  The format of the QueryColorname method is:
%
%      MagickBooleanType QueryColorname(const Image *image,
%        const PixelPacket *color,const ComplianceType compliance,char *name,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o color: the color intensities.
%
%    o compliance: Adhere to this color standard: SVG, X11, or XPM.
%
%    o name: Return the color name or hex value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType QueryColorname(const Image *image,
  const PixelPacket *color,const ComplianceType compliance,char *name,
  ExceptionInfo *exception)
{
  MagickPixelPacket
    pixel;

  GetMagickPixelPacket(image,&pixel);
  SetMagickPixelPacket(image,color,(IndexPacket *) NULL,&pixel);
  return(QueryMagickColorname(image,&pixel,compliance,name,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Q u e r y M a g i c k C o l o r C o m p l i a n c e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueryMagickColorCompliance() returns the red, green, blue, and opacity
%  intensities for a given color name and standards compliance.
%
%  The format of the QueryMagickColor method is:
%
%      MagickBooleanType QueryMagickColorCompilence(const char *name,
%        const ComplianceType compliance,MagickPixelPacket *color,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the color name (e.g. white, blue, yellow).
%
%    o compliance: Adhere to this color standard: SVG, X11, or XPM.
%
%    o color: the red, green, blue, and opacity intensities values of the
%      named color in this structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType QueryMagickColorCompliance(const char *name,
  const ComplianceType compliance,MagickPixelPacket *color,
  ExceptionInfo *exception)
{
  double
    scale;

  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  register const ColorInfo
    *p;

  register ssize_t
    i;

  ssize_t
    type;

  /*
    Initialize color return value.
  */
  assert(name != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",name);
  assert(color != (MagickPixelPacket *) NULL);
  GetMagickPixelPacket((Image *) NULL,color);
  if ((name == (char *) NULL) || (*name == '\0'))
    name=BackgroundColor;
  while (isspace((int) ((unsigned char) *name)) != 0)
    name++;
  if (*name == '#')
    {
      char
        c;

      LongPixelPacket
        pixel;

      QuantumAny
        range;

      size_t
        depth,
        n;

      /*
        Parse hex color.
      */
      (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
      name++;
      for (n=0; isxdigit((int) ((unsigned char) name[n])) != 0; n++) ;
      if ((n % 3) == 0)
        {
          do
          {
            pixel.red=pixel.green;
            pixel.green=pixel.blue;
            pixel.blue=0;
            for (i=(ssize_t) (n/3-1); i >= 0; i--)
            {
              c=(*name++);
              pixel.blue<<=4;
              if ((c >= '0') && (c <= '9'))
                pixel.blue|=(int) (c-'0');
              else
                if ((c >= 'A') && (c <= 'F'))
                  pixel.blue|=(int) c-((int) 'A'-10);
                else
                  if ((c >= 'a') && (c <= 'f'))
                    pixel.blue|=(int) c-((int) 'a'-10);
                  else
                    return(MagickFalse);
            }
          } while (isxdigit((int) ((unsigned char) *name)) != 0);
          depth=4*(n/3);
        }
      else
        {
          if ((n % 4) != 0)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionWarning,"UnrecognizedColor","`%s'",name);
              return(MagickFalse);
            }
          do
          {
            pixel.red=pixel.green;
            pixel.green=pixel.blue;
            pixel.blue=pixel.opacity;
            pixel.opacity=0;
            for (i=(ssize_t) (n/4-1); i >= 0; i--)
            {
              c=(*name++);
              pixel.opacity<<=4;
              if ((c >= '0') && (c <= '9'))
                pixel.opacity|=(int) (c-'0');
              else
                if ((c >= 'A') && (c <= 'F'))
                  pixel.opacity|=(int) c-((int) 'A'-10);
                else
                  if ((c >= 'a') && (c <= 'f'))
                    pixel.opacity|=(int) c-((int) 'a'-10);
                  else
                    return(MagickFalse);
            }
          } while (isxdigit((int) ((unsigned char) *name)) != MagickFalse);
          depth=4*(n/4);
        }
      color->colorspace=sRGBColorspace;
      color->depth=depth;
      color->matte=MagickFalse;
      range=GetQuantumRange(depth);
      color->red=(MagickRealType) ScaleAnyToQuantum(pixel.red,range);
      color->green=(MagickRealType) ScaleAnyToQuantum(pixel.green,range);
      color->blue=(MagickRealType) ScaleAnyToQuantum(pixel.blue,range);
      color->opacity=(MagickRealType) OpaqueOpacity;
      if ((n % 3) != 0)
        {
          color->matte=MagickTrue;
          color->opacity=(MagickRealType) (QuantumRange-ScaleAnyToQuantum(
            pixel.opacity,range));
        }
      color->index=0.0;
      return(MagickTrue);
    }
  if (strchr(name,'(') != (char *) NULL)
    {
      char
        colorspace[MaxTextExtent];

      MagickBooleanType
        icc_color;

      /*
        Parse color of the form rgb(100,255,0).
      */
      (void) CopyMagickString(colorspace,name,MaxTextExtent);
      for (i=0; colorspace[i] != '\0'; i++)
        if (colorspace[i] == '(')
          break;
      colorspace[i--]='\0';
      scale=(MagickRealType) ScaleCharToQuantum(1);
      icc_color=MagickFalse;
      if (LocaleNCompare(colorspace,"device-",7) == 0)
        {
          (void) CopyMagickString(colorspace,colorspace+7,MaxTextExtent);
          scale=(double) QuantumRange;
          icc_color=MagickTrue;
        }
      if (LocaleCompare(colorspace,"icc-color") == 0)
        {
          register ssize_t
            j;

          (void) CopyMagickString(colorspace,name+i+2,MaxTextExtent);
          for (j=0; colorspace[j] != '\0'; j++)
            if (colorspace[j] == ',')
              break;
          colorspace[j--]='\0';
          i+=j+3;
          scale=(MagickRealType) QuantumRange;
          icc_color=MagickTrue;
        }
      LocaleLower(colorspace);
      color->matte=MagickFalse;
      if ((i > 0) && (colorspace[i] == 'a'))
        {
          colorspace[i]='\0';
          color->matte=MagickTrue;
        }
      type=ParseCommandOption(MagickColorspaceOptions,MagickFalse,colorspace);
      if (type < 0)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            OptionWarning,"UnrecognizedColor","`%s'",name);
          return(MagickFalse);
        }
      color->colorspace=(ColorspaceType) type;
      if ((icc_color == MagickFalse) && (color->colorspace == RGBColorspace))
        {
          color->colorspace=sRGBColorspace;  /* as required by SVG standard */
          color->depth=8;
        }
      SetGeometryInfo(&geometry_info);
      flags=ParseGeometry(name+i+1,&geometry_info);
      if (flags == 0)
        {
          char
            *colorname;

          ColorspaceType
            colorspace;

          colorspace=color->colorspace;
          colorname=AcquireString(name+i+1);
          (void) SubstituteString(&colorname,")","");
          (void) QueryMagickColor(colorname,color,exception);
          colorname=DestroyString(colorname);
          color->colorspace=colorspace;
        }
      else
        {
          if ((flags & PercentValue) != 0)
            scale=(MagickRealType) (QuantumRange/100.0);
          if ((flags & RhoValue) != 0)
            color->red=(MagickRealType) ClampToQuantum(scale*geometry_info.rho);
          if ((flags & SigmaValue) != 0)
            color->green=(MagickRealType) ClampToQuantum(scale*
              geometry_info.sigma);
          if ((flags & XiValue) != 0)
            color->blue=(MagickRealType) ClampToQuantum(scale*geometry_info.xi);
          color->opacity=(MagickRealType) OpaqueOpacity;
          if ((flags & PsiValue) != 0)
            {
              if (color->colorspace == CMYKColorspace)
                color->index=(MagickRealType) ClampToQuantum(scale*
                  geometry_info.psi);
              else
                if (color->matte != MagickFalse)
                  color->opacity=(MagickRealType) ClampToQuantum(
                    (MagickRealType) (QuantumRange-QuantumRange*
                    geometry_info.psi));
            }
          if (((flags & ChiValue) != 0) && (color->matte != MagickFalse))
            color->opacity=(MagickRealType) ClampToQuantum((MagickRealType)
              (QuantumRange-QuantumRange*geometry_info.chi));
          if (LocaleCompare(colorspace,"gray") == 0)
            {
              color->colorspace=GRAYColorspace;
              color->green=color->red;
              color->blue=color->red;
              if (((flags & SigmaValue) != 0) && (color->matte != MagickFalse))
                color->opacity=(MagickRealType) ClampToQuantum((MagickRealType)
                  (QuantumRange-QuantumRange*geometry_info.sigma));
            }
          if ((LocaleCompare(colorspace,"HCL") == 0) ||
              (LocaleCompare(colorspace,"HSB") == 0) ||
              (LocaleCompare(colorspace,"HSL") == 0) ||
              (LocaleCompare(colorspace,"HWB") == 0))
            {
              PixelPacket
                pixel;

              scale=1.0/360.0;
              if ((flags & PercentValue) != 0)
                scale=1.0/100.0;
              geometry_info.rho*=360.0*scale;
              scale=1.0/255.0;
              if ((flags & PercentValue) != 0)
                scale=1.0/100.0;
              geometry_info.sigma*=scale;
              geometry_info.xi*=scale;
              if (LocaleCompare(colorspace,"HCL") == 0)
                ConvertHCLToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,360.0)/
                  360.0,geometry_info.sigma,geometry_info.xi,&pixel.red,
                  &pixel.green,&pixel.blue);
              else
                if (LocaleCompare(colorspace,"HSB") == 0)
                  ConvertHSBToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,
                    360.0)/360.0,geometry_info.sigma,geometry_info.xi,
                    &pixel.red,&pixel.green,&pixel.blue);
                else
                  if (LocaleCompare(colorspace,"HSL") == 0)
                    ConvertHSLToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,
                      360.0)/360.0,geometry_info.sigma,geometry_info.xi,
                      &pixel.red,&pixel.green,&pixel.blue);
                  else
                    ConvertHWBToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,
                      360.0)/360.0,geometry_info.sigma,geometry_info.xi,
                      &pixel.red,&pixel.green,&pixel.blue);
              color->colorspace=sRGBColorspace;
              color->red=(MagickRealType) pixel.red;
              color->green=(MagickRealType) pixel.green;
              color->blue=(MagickRealType) pixel.blue;
            }
        }
      return(MagickTrue);
    }
  /*
    Parse named color.
  */
  p=GetColorCompliance(name,compliance,exception);
  if (p == (const ColorInfo *) NULL)
    return(MagickFalse);
  color->colorspace=sRGBColorspace;
  color->depth=8;
  color->matte=p->color.opacity != OpaqueOpacity ? MagickTrue : MagickFalse;
  color->red=(MagickRealType) p->color.red;
  color->green=(MagickRealType) p->color.green;
  color->blue=(MagickRealType) p->color.blue;
  color->opacity=(MagickRealType) p->color.opacity;
  color->index=0.0;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Q u e r y M a g i c k C o l o r                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueryMagickColor() returns the red, green, blue, and opacity intensities
%  for a given color name.
%
%  The format of the QueryMagickColor method is:
%
%      MagickBooleanType QueryMagickColor(const char *name,
%        MagickPixelPacket *color,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the color name (e.g. white, blue, yellow).
%
%    o color: the red, green, blue, and opacity intensities values of the
%      named color in this structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType QueryMagickColor(const char *name,
  MagickPixelPacket *color,ExceptionInfo *exception)
{
  return(QueryMagickColorCompliance(name,AllCompliance,color,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  Q u e r y M a g i c k C o l o r n a m e                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueryMagickColorname() returns a named color for the given color intensity.
%  If an exact match is not found, a hex value is returned instead.  For
%  example an intensity of rgb:(0,0,0) returns black whereas rgb:(223,223,223)
%  returns #dfdfdf.
%
%  The format of the QueryMagickColorname method is:
%
%      MagickBooleanType QueryMagickColorname(const Image *image,
%        const PixelPacket *color,const ComplianceType compliance,char *name,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o color: the color intensities.
%
%    o Compliance: Adhere to this color standard: SVG, X11, or XPM.
%
%    o name: Return the color name or hex value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType QueryMagickColorname(const Image *image,
  const MagickPixelPacket *color,const ComplianceType compliance,
  char *name,ExceptionInfo *exception)
{
  MagickPixelPacket
    pixel;

  MagickRealType
    opacity;

  register const ColorInfo
    *p;

  *name='\0';
  pixel=(*color);
  if (compliance == XPMCompliance)
    {
      pixel.matte=MagickFalse;
      pixel.depth=(size_t) MagickMin(1.0*image->depth,16.0);
    }
  GetColorTuple(&pixel,compliance != SVGCompliance ? MagickTrue : MagickFalse,
    name);
  if (IssRGBColorspace(pixel.colorspace) == MagickFalse)
    return(MagickFalse);
  (void) GetColorInfo("*",exception);
  ResetLinkedListIterator(color_list);
  opacity=image->matte != MagickFalse ? color->opacity : OpaqueOpacity;
  p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  while (p != (const ColorInfo *) NULL)
  {
    if (((p->compliance & compliance) != 0) && ((p->color.red == color->red)) &&
         (p->color.green == color->green) && (p->color.blue == color->blue) &&
         (p->color.opacity == opacity))
      {
        (void) CopyMagickString(name,p->name,MaxTextExtent);
        break;
      }
    p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  }
  return(MagickTrue);
}

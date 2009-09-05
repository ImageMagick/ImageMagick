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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#define MaxTreeDepth  8
#define NodesInAList  1536

/*
  Declare color map.
*/
static const char
  *ColorMap = (const char *)
    "<?xml version=\"1.0\"?>"
    "<colormap>"
    "  <color name=\"none\" color=\"rgba(0,0,0,0)\" compliance=\"SVG\" />"
    "  <color name=\"black\" color=\"rgb(0,0,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"red\" color=\"rgb(255,0,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"magenta\" color=\"rgb(255,0,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"green\" color=\"rgb(0,128,0)\" compliance=\"SVG\" />"
    "  <color name=\"cyan\" color=\"rgb(0,255,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"blue\" color=\"rgb(0,0,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"yellow\" color=\"rgb(255,255,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"white\" color=\"rgb(255,255,255)\" compliance=\"SVG, X11\" />"
    "  <color name=\"AliceBlue\" color=\"rgb(240,248,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"AntiqueWhite\" color=\"rgb(250,235,215)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"aqua\" color=\"rgb(0,255,255)\" compliance=\"SVG\" />"
    "  <color name=\"aquamarine\" color=\"rgb(127,255,212)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"azure\" color=\"rgb(240,255,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"beige\" color=\"rgb(245,245,220)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"bisque\" color=\"rgb(255,228,196)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"BlanchedAlmond\" color=\"rgb(255,235,205)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"BlueViolet\" color=\"rgb(138,43,226)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"brown\" color=\"rgb(165,42,42)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"burlywood\" color=\"rgb(222,184,135)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"CadetBlue\" color=\"rgb(95,158,160)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"chartreuse\" color=\"rgb(127,255,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"chocolate\" color=\"rgb(210,105,30)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"coral\" color=\"rgb(255,127,80)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"CornflowerBlue\" color=\"rgb(100,149,237)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"cornsilk\" color=\"rgb(255,248,220)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"crimson\" color=\"rgb(220,20,60)\" compliance=\"SVG\" />"
    "  <color name=\"DarkBlue\" color=\"rgb(0,0,139)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DarkCyan\" color=\"rgb(0,139,139)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DarkGoldenrod\" color=\"rgb(184,134,11)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkGray\" color=\"rgb(169,169,169)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DarkGreen\" color=\"rgb(0,100,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkGrey\" color=\"rgb(169,169,169)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DarkKhaki\" color=\"rgb(189,183,107)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkMagenta\" color=\"rgb(139,0,139)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DarkOliveGreen\" color=\"rgb(85,107,47)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkOrange\" color=\"rgb(255,140,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkOrchid\" color=\"rgb(153,50,204)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkRed\" color=\"rgb(139,0,0)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DarkSalmon\" color=\"rgb(233,150,122)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkSeaGreen\" color=\"rgb(143,188,143)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkSlateBlue\" color=\"rgb(72,61,139)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkSlateGray\" color=\"rgb(47,79,79)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkSlateGrey\" color=\"rgb(47,79,79)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DarkTurquoise\" color=\"rgb(0,206,209)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DarkViolet\" color=\"rgb(148,0,211)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DeepPink\" color=\"rgb(255,20,147)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DeepSkyBlue\" color=\"rgb(0,191,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DimGray\" color=\"rgb(105,105,105)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"DimGrey\" color=\"rgb(105,105,105)\" compliance=\"SVG, X11\" />"
    "  <color name=\"DodgerBlue\" color=\"rgb(30,144,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"firebrick\" color=\"rgb(178,34,34)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"FloralWhite\" color=\"rgb(255,250,240)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"ForestGreen\" color=\"rgb(34,139,34)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"fractal\" color=\"rgb(128,128,128)\" compliance=\"SVG\" />"
    "  <color name=\"fuchsia\" color=\"rgb(255,0,255)\" compliance=\"SVG\" />"
    "  <color name=\"gainsboro\" color=\"rgb(220,220,220)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"GhostWhite\" color=\"rgb(248,248,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"gold\" color=\"rgb(255,215,0)\" compliance=\"X11, XPM\" />"
    "  <color name=\"goldenrod\" color=\"rgb(218,165,32)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"gray\" color=\"rgb(126,126,126)\" compliance=\"SVG\" />"
    "  <color name=\"gray74\" color=\"rgb(189,189,189)\" compliance=\"SVG, X11\" />"
    "  <color name=\"gray100\" color=\"rgb(255,255,255)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey\" color=\"rgb(190,190,190)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey0\" color=\"rgb(0,0,0)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey1\" color=\"rgb(3,3,3)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey10\" color=\"rgb(26,26,26)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey100\" color=\"rgb(255,255,255)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey11\" color=\"rgb(28,28,28)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey12\" color=\"rgb(31,31,31)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey13\" color=\"rgb(33,33,33)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey14\" color=\"rgb(36,36,36)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey15\" color=\"rgb(38,38,38)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey16\" color=\"rgb(41,41,41)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey17\" color=\"rgb(43,43,43)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey18\" color=\"rgb(45,45,45)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey19\" color=\"rgb(48,48,48)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey2\" color=\"rgb(5,5,5)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey20\" color=\"rgb(51,51,51)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey21\" color=\"rgb(54,54,54)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey22\" color=\"rgb(56,56,56)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey23\" color=\"rgb(59,59,59)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey24\" color=\"rgb(61,61,61)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey25\" color=\"rgb(64,64,64)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey26\" color=\"rgb(66,66,66)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey27\" color=\"rgb(69,69,69)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey28\" color=\"rgb(71,71,71)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey29\" color=\"rgb(74,74,74)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey3\" color=\"rgb(8,8,8)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey30\" color=\"rgb(77,77,77)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey31\" color=\"rgb(79,79,79)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey32\" color=\"rgb(82,82,82)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey33\" color=\"rgb(84,84,84)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey34\" color=\"rgb(87,87,87)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey35\" color=\"rgb(89,89,89)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey36\" color=\"rgb(92,92,92)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey37\" color=\"rgb(94,94,94)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey38\" color=\"rgb(97,97,97)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey39\" color=\"rgb(99,99,99)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey4\" color=\"rgb(10,10,10)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey40\" color=\"rgb(102,102,102)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey41\" color=\"rgb(105,105,105)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey42\" color=\"rgb(107,107,107)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey43\" color=\"rgb(110,110,110)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey44\" color=\"rgb(112,112,112)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey45\" color=\"rgb(115,115,115)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey45\" color=\"rgb(117,117,117)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey47\" color=\"rgb(120,120,120)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey48\" color=\"rgb(122,122,122)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey49\" color=\"rgb(125,125,125)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey5\" color=\"rgb(13,13,13)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey50\" color=\"rgb(50%,50%,50%)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey51\" color=\"rgb(130,130,130)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey52\" color=\"rgb(133,133,133)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey53\" color=\"rgb(135,135,135)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey54\" color=\"rgb(138,138,138)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey55\" color=\"rgb(140,140,140)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey56\" color=\"rgb(143,143,143)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey57\" color=\"rgb(145,145,145)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey58\" color=\"rgb(148,148,148)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey59\" color=\"rgb(150,150,150)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey6\" color=\"rgb(15,15,15)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey60\" color=\"rgb(153,153,153)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey61\" color=\"rgb(156,156,156)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey62\" color=\"rgb(158,158,158)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey63\" color=\"rgb(161,161,161)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey64\" color=\"rgb(163,163,163)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey65\" color=\"rgb(166,166,166)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey66\" color=\"rgb(168,168,168)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey67\" color=\"rgb(171,171,171)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey68\" color=\"rgb(173,173,173)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey69\" color=\"rgb(176,176,176)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey7\" color=\"rgb(18,18,18)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey70\" color=\"rgb(179,179,179)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey71\" color=\"rgb(181,181,181)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey72\" color=\"rgb(184,184,184)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey73\" color=\"rgb(186,186,186)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey74\" color=\"rgb(189,189,189)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey75\" color=\"rgb(191,191,191)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey76\" color=\"rgb(194,194,194)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey77\" color=\"rgb(196,196,196)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey78\" color=\"rgb(199,199,199)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey79\" color=\"rgb(201,201,201)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey8\" color=\"rgb(20,20,20)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey80\" color=\"rgb(204,204,204)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey81\" color=\"rgb(207,207,207)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey82\" color=\"rgb(209,209,209)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey83\" color=\"rgb(212,212,212)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey84\" color=\"rgb(214,214,214)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey85\" color=\"rgb(217,217,217)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey86\" color=\"rgb(219,219,219)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey87\" color=\"rgb(222,222,222)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey88\" color=\"rgb(224,224,224)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey89\" color=\"rgb(227,227,227)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey9\" color=\"rgb(23,23,23)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey90\" color=\"rgb(229,229,229)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey91\" color=\"rgb(232,232,232)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey92\" color=\"rgb(235,235,235)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey93\" color=\"rgb(237,237,237)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey94\" color=\"rgb(240,240,240)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey95\" color=\"rgb(242,242,242)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey96\" color=\"rgb(245,245,245)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey97\" color=\"rgb(247,247,247)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey98\" color=\"rgb(250,250,250)\" compliance=\"SVG, X11\" />"
    "  <color name=\"grey99\" color=\"rgb(252,252,252)\" compliance=\"SVG, X11\" />"
    "  <color name=\"honeydew\" color=\"rgb(240,255,240)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"HotPink\" color=\"rgb(255,105,180)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"IndianRed\" color=\"rgb(205,92,92)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"indigo\" color=\"rgb(75,0,130)\" compliance=\"SVG\" />"
    "  <color name=\"ivory\" color=\"rgb(255,255,240)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"khaki\" color=\"rgb(240,230,140)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"lavender\" color=\"rgb(230,230,250)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LavenderBlush\" color=\"rgb(255,240,245)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LawnGreen\" color=\"rgb(124,252,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LemonChiffon\" color=\"rgb(255,250,205)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightBlue\" color=\"rgb(173,216,230)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightCoral\" color=\"rgb(240,128,128)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightCyan\" color=\"rgb(224,255,255)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightGoldenrodYellow\" color=\"rgb(250,250,210)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightGray\" color=\"rgb(211,211,211)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightGreen\" color=\"rgb(144,238,144)\" compliance=\"SVG, X11\" />"
    "  <color name=\"LightGrey\" color=\"rgb(211,211,211)\" compliance=\"SVG, X11\" />"
    "  <color name=\"LightPink\" color=\"rgb(255,182,193)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightSalmon\" color=\"rgb(255,160,122)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightSeaGreen\" color=\"rgb(32,178,170)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightSkyBlue\" color=\"rgb(135,206,250)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightSlateGray\" color=\"rgb(119,136,153)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightSlateGrey\" color=\"rgb(119,136,153)\" compliance=\"SVG, X11\" />"
    "  <color name=\"LightSteelBlue\" color=\"rgb(176,196,222)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"LightYellow\" color=\"rgb(255,255,224)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"lime\" color=\"rgb(0,255,0)\" compliance=\"SVG\" />"
    "  <color name=\"LimeGreen\" color=\"rgb(50,205,50)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"linen\" color=\"rgb(250,240,230)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"maroon\" color=\"rgb(128,0,0)\" compliance=\"SVG\" />"
    "  <color name=\"MediumAquamarine\" color=\"rgb(102,205,170)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumBlue\" color=\"rgb(0,0,205)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumOrchid\" color=\"rgb(186,85,211)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumPurple\" color=\"rgb(147,112,219)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumSeaGreen\" color=\"rgb(60,179,113)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumSlateBlue\" color=\"rgb(123,104,238)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumSpringGreen\" color=\"rgb(0,250,154)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumTurquoise\" color=\"rgb(72,209,204)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MediumVioletRed\" color=\"rgb(199,21,133)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MidnightBlue\" color=\"rgb(25,25,112)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MintCream\" color=\"rgb(245,255,250)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"MistyRose\" color=\"rgb(255,228,225)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"moccasin\" color=\"rgb(255,228,181)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"NavajoWhite\" color=\"rgb(255,222,173)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"navy\" color=\"rgb(0,0,128)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"matte\" color=\"rgb(0,0,0,0)\" compliance=\"SVG\" />"
    "  <color name=\"OldLace\" color=\"rgb(253,245,230)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"olive\" color=\"rgb(128,128,0)\" compliance=\"SVG\" />"
    "  <color name=\"OliveDrab\" color=\"rgb(107,142,35)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"opaque\" color=\"rgb(0,0,0)\" compliance=\"SVG\" />"
    "  <color name=\"orange\" color=\"rgb(255,165,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"OrangeRed\" color=\"rgb(255,69,0)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"orchid\" color=\"rgb(218,112,214)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"PaleGoldenrod\" color=\"rgb(238,232,170)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"PaleGreen\" color=\"rgb(152,251,152)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"PaleTurquoise\" color=\"rgb(175,238,238)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"PaleVioletRed\" color=\"rgb(219,112,147)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"PapayaWhip\" color=\"rgb(255,239,213)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"PeachPuff\" color=\"rgb(255,218,185)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"peru\" color=\"rgb(205,133,63)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"pink\" color=\"rgb(255,192,203)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"plum\" color=\"rgb(221,160,221)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"PowderBlue\" color=\"rgb(176,224,230)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"purple\" color=\"rgb(128,0,128)\" compliance=\"SVG\" />"
    "  <color name=\"RosyBrown\" color=\"rgb(188,143,143)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"RoyalBlue\" color=\"rgb(65,105,225)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SaddleBrown\" color=\"rgb(139,69,19)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"salmon\" color=\"rgb(250,128,114)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SandyBrown\" color=\"rgb(244,164,96)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SeaGreen\" color=\"rgb(45,139,87)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"seashell\" color=\"rgb(255,245,238)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"sienna\" color=\"rgb(160,82,45)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"silver\" color=\"rgb(192,192,192)\" compliance=\"SVG\" />"
    "  <color name=\"SkyBlue\" color=\"rgb(135,206,235)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SlateBlue\" color=\"rgb(106,90,205)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SlateGray\" color=\"rgb(112,128,144)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SlateGrey\" color=\"rgb(112,128,144)\" compliance=\"SVG, X11\" />"
    "  <color name=\"snow\" color=\"rgb(255,250,250)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SpringGreen\" color=\"rgb(0,255,127)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"SteelBlue\" color=\"rgb(70,130,180)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"tan\" color=\"rgb(210,180,140)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"teal\" color=\"rgb(0,128,128)\" compliance=\"SVG\" />"
    "  <color name=\"thistle\" color=\"rgb(216,191,216)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"tomato\" color=\"rgb(255,99,71)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"transparent\" color=\"rgba(0,0,0,0)\" compliance=\"SVG\" />"
    "  <color name=\"turquoise\" color=\"rgb(64,224,208)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"violet\" color=\"rgb(238,130,238)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"wheat\" color=\"rgb(245,222,179)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"WhiteSmoke\" color=\"rgb(245,245,245)\" compliance=\"SVG, X11, XPM\" />"
    "  <color name=\"YellowGreen\" color=\"rgb(154,205,50)\" compliance=\"SVG, X11, XPM\" />"
    "</colormap>";

/*
  Typedef declarations.
*/
typedef struct _NodeInfo
{
  struct _NodeInfo
    *child[16];

  ColorPacket
    *list;

  MagickSizeType
    number_unique;

  unsigned long
    level;
} NodeInfo;

typedef struct _Nodes
{
  NodeInfo
    nodes[NodesInAList];

  struct _Nodes
    *next;
} Nodes;

typedef struct _CubeInfo
{
  NodeInfo
    *root;

  long
    x,
    progress;

  unsigned long
    colors,
    free_nodes;

  NodeInfo
    *node_info;

  Nodes
    *node_queue;
} CubeInfo;

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
static CubeInfo
  *GetCubeInfo(void);

static NodeInfo
  *GetNodeInfo(CubeInfo *,const unsigned long);

static MagickBooleanType
  InitializeColorList(ExceptionInfo *),
  LoadColorLists(const char *,ExceptionInfo *);

static void
  DestroyColorCube(const Image *,NodeInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l a s s i f y I m a g e C o l o r s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClassifyImageColors() builds a populated CubeInfo tree for the specified
%  image.  The returned tree should be deallocated using DestroyCubeInfo()
%  once it is no longer needed.
%
%  The format of the ClassifyImageColors() method is:
%
%      CubeInfo *ClassifyImageColors(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline unsigned long ColorToNodeId(const Image *image,
  const MagickPixelPacket *pixel,unsigned long index)
{
  unsigned long
    id;

  id=(unsigned long) (
    ((ScaleQuantumToChar(RoundToQuantum(pixel->red)) >> index) & 0x01) |
    ((ScaleQuantumToChar(RoundToQuantum(pixel->green)) >> index) & 0x01) << 1 |
    ((ScaleQuantumToChar(RoundToQuantum(pixel->blue)) >> index) & 0x01) << 2);
  if (image->matte != MagickFalse)
    id|=((ScaleQuantumToChar(RoundToQuantum(pixel->opacity)) >> index) &
      0x01) << 3;
  return(id);
}

static CubeInfo *ClassifyImageColors(const Image *image,
  ExceptionInfo *exception)
{
#define EvaluateImageTag  "  Compute image colors...  "

  CubeInfo
    *cube_info;

  long
    y;

  MagickBooleanType
    proceed;

  MagickPixelPacket
    pixel,
    target;

  NodeInfo
    *node_info;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  register unsigned long
    id,
    index,
    level;

  CacheView
    *image_view;

  /*
    Initialize color description tree.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cube_info=GetCubeInfo();
  if (cube_info == (CubeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(cube_info);
    }
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireCacheView(image);
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        SetMagickPixelPacket(image,p,indexes+x,&pixel);
        id=ColorToNodeId(image,&pixel,index);
        if (node_info->child[id] == (NodeInfo *) NULL)
          {
            node_info->child[id]=GetNodeInfo(cube_info,level);
            if (node_info->child[id] == (NodeInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                return(0);
              }
          }
        node_info=node_info->child[id];
        index--;
      }
      for (i=0; i < (long) node_info->number_unique; i++)
      {
        SetMagickPixelPacket(image,&node_info->list[i].pixel,
          &node_info->list[i].index,&target);
        if (IsMagickColorEqual(&pixel,&target) != MagickFalse)
          break;
      }
      if (i < (long) node_info->number_unique)
        node_info->list[i].count++;
      else
        {
          if (node_info->number_unique == 0)
            node_info->list=(ColorPacket *) AcquireMagickMemory(
              sizeof(*node_info->list));
          else
            node_info->list=(ColorPacket *) ResizeQuantumMemory(node_info->list,
              (size_t) (i+1),sizeof(*node_info->list));
          if (node_info->list == (ColorPacket *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              return(0);
            }
          node_info->list[i].pixel=(*p);
          if ((image->colorspace == CMYKColorspace) ||
              (image->storage_class == PseudoClass))
            node_info->list[i].index=indexes[x];
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
        }
      p++;
    }
    proceed=SetImageProgress(image,EvaluateImageTag,y,image->rows);
    if (proceed == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  return(cube_info);
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
  if (compliance != SVGCompliance)
    {
      if (pixel->depth > 16)
        {
          (void) FormatMagickString(component,MaxTextExtent,"%10lu",
            (unsigned long) ScaleQuantumToLong(RoundToQuantum(color)));
          (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
          return;
        }
      if (pixel->depth > 8)
        {
          (void) FormatMagickString(component,MaxTextExtent,"%5d",
            ScaleQuantumToShort(RoundToQuantum(color)));
          (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
          return;
        }
      (void) FormatMagickString(component,MaxTextExtent,"%3d",
        ScaleQuantumToChar(RoundToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (channel == OpacityChannel)
    {
      (void) FormatMagickString(component,MaxTextExtent,"%g",
        (double) (QuantumScale*color));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (pixel->depth > 8)
    {
      (void) FormatMagickString(component,MaxTextExtent,"%g%%",
        (double) (100.0*QuantumScale*color));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  (void) FormatMagickString(component,MaxTextExtent,"%d",
    ScaleQuantumToChar(RoundToQuantum(color)));
  (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f i n e I m a g e H i s t o g r a m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefineImageHistogram() traverses the color cube tree and notes each colormap
%  entry.  A colormap entry is any node in the color cube tree where the
%  of unique colors is not zero.
%
%  The format of the DefineImageHistogram method is:
%
%      DefineImageHistogram(const Image *image,NodeInfo *node_info,
%        ColorPacket **unique_colors)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o node_info: the address of a structure of type NodeInfo which points to a
%      node in the color cube tree that is to be pruned.
%
%    o histogram: the image histogram.
%
*/
static void DefineImageHistogram(const Image *image,NodeInfo *node_info,
  ColorPacket **histogram)
{
  register long
    i;

  unsigned long
    number_children;

  /*
    Traverse any children.
  */
  number_children=image->matte == MagickFalse ? 8UL : 16UL;
  for (i=0; i < (long) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      DefineImageHistogram(image,node_info->child[i],histogram);
  if (node_info->level == (MaxTreeDepth-1))
    {
      register ColorPacket
        *p;

      p=node_info->list;
      for (i=0; i < (long) node_info->number_unique; i++)
      {
        (*histogram)->pixel=p->pixel;
        (*histogram)->index=p->index;
        (*histogram)->count=p->count;
        (*histogram)++;
        p++;
      }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C o l o r L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyColorList() deallocates memory associated with the color list.
%
%  The format of the DestroyColorList method is:
%
%      DestroyColorList(void)
%
*/

static void *DestroyColorElement(void *color_info)
{
  register ColorInfo
    *p;

  p=(ColorInfo *) color_info;
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->name != (char *) NULL)
    p->name=DestroyString(p->name);
  p=(ColorInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickExport void DestroyColorList(void)
{
  AcquireSemaphoreInfo(&color_semaphore);
  if (color_list != (LinkedListInfo *) NULL)
    color_list=DestroyLinkedList(color_list,DestroyColorElement);
  instantiate_color=MagickFalse;
  RelinquishSemaphoreInfo(color_semaphore);
  DestroySemaphoreInfo(&color_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C u b e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCubeInfo() deallocates memory associated with a CubeInfo structure.
%
%  The format of the DestroyCubeInfo method is:
%
%      DestroyCubeInfo(const Image *image,CubeInfo *cube_info)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o cube_info: the address of a structure of type CubeInfo.
%
*/
static CubeInfo *DestroyCubeInfo(const Image *image,CubeInfo *cube_info)
{
  register Nodes
    *nodes;

  /*
    Release color cube tree storage.
  */
  DestroyColorCube(image,cube_info->root);
  do
  {
    nodes=cube_info->node_queue->next;
    cube_info->node_queue=(Nodes *)
      RelinquishMagickMemory(cube_info->node_queue);
    cube_info->node_queue=nodes;
  } while (cube_info->node_queue != (Nodes *) NULL);
  return((CubeInfo *) RelinquishMagickMemory(cube_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  D e s t r o y C o l o r C u b e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyColorCube() traverses the color cube tree and frees the list of
%  unique colors.
%
%  The format of the DestroyColorCube method is:
%
%      void DestroyColorCube(const Image *image,const NodeInfo *node_info)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o node_info: the address of a structure of type NodeInfo which points to a
%      node in the color cube tree that is to be pruned.
%
*/
static void DestroyColorCube(const Image *image,NodeInfo *node_info)
{
  register long
    i;

  unsigned long
    number_children;

  /*
    Traverse any children.
  */
  number_children=image->matte == MagickFalse ? 8UL : 16UL;
  for (i=0; i < (long) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      DestroyColorCube(image,node_info->child[i]);
  if (node_info->list != (ColorPacket *) NULL)
    node_info->list=(ColorPacket *) RelinquishMagickMemory(node_info->list);
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
  AcquireSemaphoreInfo(&color_semaphore);
  ResetLinkedListIterator(color_list);
  p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  while (p != (const ColorInfo *) NULL)
  {
    if (LocaleCompare(colorname,p->name) == 0)
      break;
    p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  }
  if (p == (ColorInfo *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
      "UnrecognizedColor","`%s'",name);
  else
    (void) InsertValueInLinkedList(color_list,0,
      RemoveElementByValueFromLinkedList(color_list,p));
  RelinquishSemaphoreInfo(color_semaphore);
  return(p);
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
%        unsigned long *number_colors,ExceptionInfo *exception)
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
  unsigned long *number_colors,ExceptionInfo *exception)
{
  const ColorInfo
    **colors;

  register const ColorInfo
    *p;

  register long
    i;

  /*
    Allocate color list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_colors != (unsigned long *) NULL);
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
  AcquireSemaphoreInfo(&color_semaphore);
  ResetLinkedListIterator(color_list);
  p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  for (i=0; p != (const ColorInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      colors[i++]=p;
    p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  }
  RelinquishSemaphoreInfo(color_semaphore);
  qsort((void *) colors,(size_t) i,sizeof(*colors),ColorInfoCompare);
  colors[i]=(ColorInfo *) NULL;
  *number_colors=(unsigned long) i;
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
%      char **GetColorList(const char *pattern,unsigned long *number_colors,
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
  unsigned long *number_colors,ExceptionInfo *exception)
{
  char
    **colors;

  register const ColorInfo
    *p;

  register long
    i;

  /*
    Allocate color list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_colors != (unsigned long *) NULL);
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
  AcquireSemaphoreInfo(&color_semaphore);
  ResetLinkedListIterator(color_list);
  p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  for (i=0; p != (const ColorInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      colors[i++]=ConstantString(p->name);
    p=(const ColorInfo *) GetNextValueInLinkedList(color_list);
  }
  RelinquishSemaphoreInfo(color_semaphore);
  qsort((void *) colors,(size_t) i,sizeof(*colors),ColorCompare);
  colors[i]=(char *) NULL;
  *number_colors=(unsigned long) i;
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
      (void) FormatMagickString(component,MaxTextExtent,"%08lX",
        ScaleQuantumToLong(RoundToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (pixel->depth > 16)
    {
      (void) FormatMagickString(component,MaxTextExtent,"%08X",
        (unsigned int) ScaleQuantumToLong(RoundToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  if (pixel->depth > 8)
    {
      (void) FormatMagickString(component,MaxTextExtent,"%04X",
        ScaleQuantumToShort(RoundToQuantum(color)));
      (void) ConcatenateMagickString(tuple,component,MaxTextExtent);
      return;
    }
  (void) FormatMagickString(component,MaxTextExtent,"%02X",
    ScaleQuantumToChar(RoundToQuantum(color)));
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
   ScaleCharToQuantum(ScaleQuantumToChar(RoundToQuantum(component))));

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
  (void) ConcatenateMagickString(tuple,MagickOptionToMnemonic(
    MagickColorspaceOptions,(long) color.colorspace),MaxTextExtent);
  if (color.matte != MagickFalse)
    (void) ConcatenateMagickString(tuple,"a",MaxTextExtent);
  (void) ConcatenateMagickString(tuple,"(",MaxTextExtent);
  ConcatenateColorComponent(&color,RedChannel,SVGCompliance,tuple);
  (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
  ConcatenateColorComponent(&color,GreenChannel,SVGCompliance,tuple);
  (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
  ConcatenateColorComponent(&color,BlueChannel,SVGCompliance,tuple);
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
+   G e t C u b e I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCubeInfo() initializes the CubeInfo data structure.
%
%  The format of the GetCubeInfo method is:
%
%      cube_info=GetCubeInfo()
%
%  A description of each parameter follows.
%
%    o cube_info: A pointer to the Cube structure.
%
*/
static CubeInfo *GetCubeInfo(void)
{
  CubeInfo
    *cube_info;

  /*
    Initialize tree to describe color cube.
  */
  cube_info=(CubeInfo *) AcquireMagickMemory(sizeof(*cube_info));
  if (cube_info == (CubeInfo *) NULL)
    return((CubeInfo *) NULL);
  (void) ResetMagickMemory(cube_info,0,sizeof(*cube_info));
  /*
    Initialize root node.
  */
  cube_info->root=GetNodeInfo(cube_info,0);
  if (cube_info->root == (NodeInfo *) NULL)
    return((CubeInfo *) NULL);
  return(cube_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t I m a g e H i s t o g r a m                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageHistogram() returns the unique colors in an image.
%
%  The format of the GetImageHistogram method is:
%
%      unsigned long GetImageHistogram(const Image *image,
%        unsigned long *number_colors,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o file:  Write a histogram of the color distribution to this file handle.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ColorPacket *GetImageHistogram(const Image *image,
  unsigned long *number_colors,ExceptionInfo *exception)
{
  ColorPacket
    *histogram;

  CubeInfo
    *cube_info;

  *number_colors=0;
  histogram=(ColorPacket *) NULL;
  cube_info=ClassifyImageColors(image,exception);
  if (cube_info != (CubeInfo *) NULL)
    {
      histogram=(ColorPacket *) AcquireQuantumMemory((size_t) cube_info->colors,
        sizeof(*histogram));
      if (histogram == (ColorPacket *) NULL)
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      else
        {
          ColorPacket
            *root;

          *number_colors=cube_info->colors;
          root=histogram;
          DefineImageHistogram(image,cube_info->root,&root);
        }
    }
  cube_info=DestroyCubeInfo(image,cube_info);
  return(histogram);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t N o d e I n f o                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNodeInfo() allocates memory for a new node in the color cube tree and
%  presets all fields to zero.
%
%  The format of the GetNodeInfo method is:
%
%      NodeInfo *GetNodeInfo(CubeInfo *cube_info,const unsigned long level)
%
%  A description of each parameter follows.
%
%    o cube_info: A pointer to the CubeInfo structure.
%
%    o level: Specifies the level in the storage_class the node resides.
%
*/
static NodeInfo *GetNodeInfo(CubeInfo *cube_info,const unsigned long level)
{
  NodeInfo
    *node_info;

  if (cube_info->free_nodes == 0)
    {
      Nodes
        *nodes;

      /*
        Allocate a new nodes of nodes.
      */
      nodes=(Nodes *) AcquireMagickMemory(sizeof(*nodes));
      if (nodes == (Nodes *) NULL)
        return((NodeInfo *) NULL);
      nodes->next=cube_info->node_queue;
      cube_info->node_queue=nodes;
      cube_info->node_info=nodes->nodes;
      cube_info->free_nodes=NodesInAList;
    }
  cube_info->free_nodes--;
  node_info=cube_info->node_info++;
  (void) ResetMagickMemory(node_info,0,sizeof(*node_info));
  node_info->level=level;
  return(node_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t N u m b e r C o l o r s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNumberColors() returns the number of unique colors in an image.
%
%  The format of the GetNumberColors method is:
%
%      unsigned long GetNumberColors(const Image *image,FILE *file,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o file:  Write a histogram of the color distribution to this file handle.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int HistogramCompare(const void *x,const void *y)
{
  const ColorPacket
    *color_1,
    *color_2;

  color_1=(const ColorPacket *) x;
  color_2=(const ColorPacket *) y;
  if (color_2->pixel.red != color_1->pixel.red)
    return((int) color_1->pixel.red-(int) color_2->pixel.red);
  if (color_2->pixel.green != color_1->pixel.green)
    return((int) color_1->pixel.green-(int) color_2->pixel.green);
  if (color_2->pixel.blue != color_1->pixel.blue)
    return((int) color_1->pixel.blue-(int) color_2->pixel.blue);
  return((int) color_2->count-(int) color_1->count);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport unsigned long GetNumberColors(const Image *image,FILE *file,
  ExceptionInfo *exception)
{
#define HistogramImageTag  "Histogram/Image"

  char
    color[MaxTextExtent],
    hex[MaxTextExtent],
    tuple[MaxTextExtent];

  ColorPacket
    *histogram;

  MagickPixelPacket
    pixel;

  register ColorPacket
    *p;

  register long
    i;

  unsigned long
    number_colors;

  number_colors=0;
  if (file == (FILE *) NULL)
    {
      CubeInfo
        *cube_info;

      cube_info=ClassifyImageColors(image,exception);
      if (cube_info != (CubeInfo *) NULL)
        number_colors=cube_info->colors;
      cube_info=DestroyCubeInfo(image,cube_info);
      return(number_colors);
    }
  histogram=GetImageHistogram(image,&number_colors,exception);
  if (histogram == (ColorPacket *) NULL)
    return(number_colors);
  qsort((void *) histogram,(size_t) number_colors,sizeof(*histogram),
    HistogramCompare);
  GetMagickPixelPacket(image,&pixel);
  p=histogram;
  for (i=0; i < (long) number_colors; i++)
  {
    SetMagickPixelPacket(image,&p->pixel,&p->index,&pixel);
    (void) CopyMagickString(tuple,"(",MaxTextExtent);
    ConcatenateColorComponent(&pixel,RedChannel,X11Compliance,tuple);
    (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
    ConcatenateColorComponent(&pixel,GreenChannel,X11Compliance,tuple);
    (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
    ConcatenateColorComponent(&pixel,BlueChannel,X11Compliance,tuple);
    if (pixel.colorspace == CMYKColorspace)
      {
        (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
        ConcatenateColorComponent(&pixel,IndexChannel,X11Compliance,tuple);
      }
    if (pixel.matte != MagickFalse)
      {
        (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
        ConcatenateColorComponent(&pixel,OpacityChannel,X11Compliance,tuple);
      }
    (void) ConcatenateMagickString(tuple,")",MaxTextExtent);
    (void) QueryMagickColorname(image,&pixel,SVGCompliance,color,exception);
    GetColorTuple(&pixel,MagickTrue,hex);
    (void) fprintf(file,MagickSizeFormat,p->count);
    (void) fprintf(file,": %s %s %s\n",tuple,hex,color);
    if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
        (QuantumTick(i,number_colors) != MagickFalse))
      (void) image->progress_monitor(HistogramImageTag,i,number_colors,
        image->client_data);
    p++;
  }
  (void) fflush(file);
  histogram=(ColorPacket *) RelinquishMagickMemory(histogram);
  return(number_colors);
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
      AcquireSemaphoreInfo(&color_semaphore);
      if ((color_list == (LinkedListInfo *) NULL) &&
          (instantiate_color == MagickFalse))
        {
          (void) LoadColorLists(ColorFilename,exception);
          instantiate_color=MagickTrue;
        }
      RelinquishSemaphoreInfo(color_semaphore);
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
    alpha,
    beta,
    distance;

  if ((image->fuzz == 0.0) && (image->matte == MagickFalse))
    return(IsColorEqual(p,q));
  fuzz=3.0*MagickMax(image->fuzz,MagickSQ1_2)*MagickMax(image->fuzz,
    MagickSQ1_2);
  alpha=1.0;
  beta=1.0;
  if (image->matte != MagickFalse)
    {
      alpha=(MagickRealType) (QuantumScale*(QuantumRange-p->opacity));
      beta=(MagickRealType) (QuantumScale*(QuantumRange-q->opacity));
    }
  pixel=alpha*p->red-beta*q->red;
  distance=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=alpha*p->green-beta*q->green;
  distance+=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=alpha*p->blue-beta*q->blue;
  distance+=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s G r a y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsGrayImage() returns MagickTrue if all the pixels in the image have the
%  same red, green, and blue intensities.
%
%  The format of the IsGrayImage method is:
%
%      MagickBooleanType IsGrayImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsGrayImage(const Image *image,
  ExceptionInfo *exception)
{
  ImageType
    type;

  register const PixelPacket
    *p;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->type == BilevelType) || (image->type == GrayscaleType) ||
      (image->type == GrayscaleMatteType))
    return(MagickTrue);
  if (image->colorspace == CMYKColorspace)
    return(MagickFalse);
  type=BilevelType;
  switch (image->storage_class)
  {
    case DirectClass:
    case UndefinedClass:
    {
      long
        y;

      register long
        x;

      CacheView
        *image_view;

      image_view=AcquireCacheView(image);
      for (y=0; y < (long) image->rows; y++)
      {
        p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (long) image->columns; x++)
        {
          if (IsGrayPixel(p) == MagickFalse)
            {
              type=UndefinedType;
              break;
            }
          if ((type == BilevelType) && (IsMonochromePixel(p) == MagickFalse))
            type=GrayscaleType;
          p++;
        }
        if (type == UndefinedType)
          break;
      }
      image_view=DestroyCacheView(image_view);
      break;
    }
    case PseudoClass:
    {
      register long
        i;

      p=image->colormap;
      for (i=0; i < (long) image->colors; i++)
      {
        if (IsGrayPixel(p) == MagickFalse)
          {
            type=UndefinedType;
            break;
          }
        if ((type == BilevelType) && (IsMonochromePixel(p) == MagickFalse))
          type=GrayscaleType;
        p++;
      }
      break;
    }
  }
  if (type == UndefinedType)
    return(MagickFalse);
  ((Image *) image)->type=type;
  if ((type == GrayscaleType) && (image->matte != MagickFalse))
    ((Image *) image)->type=GrayscaleMatteType;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s H i s t o g r a m I m a g e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHistogramImage() returns MagickTrue if the image has 1024 unique colors or
%  less.
%
%  The format of the IsHistogramImage method is:
%
%      MagickBooleanType IsHistogramImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsHistogramImage(const Image *image,
  ExceptionInfo *exception)
{
#define MaximumUniqueColors  1024

  CubeInfo
    *cube_info;

  long
    y;

  MagickPixelPacket
    pixel,
    target;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    x;

  register NodeInfo
    *node_info;

  register long
    i;

  unsigned long
    id,
    index,
    level;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->storage_class == PseudoClass) && (image->colors <= 256))
    return(MagickTrue);
  if (image->storage_class == PseudoClass)
    return(MagickFalse);
  /*
    Initialize color description tree.
  */
  cube_info=GetCubeInfo();
  if (cube_info == (CubeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireCacheView(image);
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        SetMagickPixelPacket(image,p,indexes+x,&pixel);
        id=ColorToNodeId(image,&pixel,index);
        if (node_info->child[id] == (NodeInfo *) NULL)
          {
            node_info->child[id]=GetNodeInfo(cube_info,level);
            if (node_info->child[id] == (NodeInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                break;
              }
          }
        node_info=node_info->child[id];
        index--;
      }
      if (level < MaxTreeDepth)
        break;
      for (i=0; i < (long) node_info->number_unique; i++)
      {
        SetMagickPixelPacket(image,&node_info->list[i].pixel,
          &node_info->list[i].index,&target);
        if (IsMagickColorEqual(&pixel,&target) != MagickFalse)
          break;
      }
      if (i < (long) node_info->number_unique)
        node_info->list[i].count++;
      else
        {
          /*
            Add this unique color to the color list.
          */
          if (node_info->number_unique == 0)
            node_info->list=(ColorPacket *) AcquireMagickMemory(
              sizeof(*node_info->list));
          else
            node_info->list=(ColorPacket *) ResizeQuantumMemory(node_info->list,
              (size_t) (i+1),sizeof(*node_info->list));
          if (node_info->list == (ColorPacket *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              break;
            }
          node_info->list[i].pixel=(*p);
          if ((image->colorspace == CMYKColorspace) ||
              (image->storage_class == PseudoClass))
            node_info->list[i].index=indexes[x];
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
          if (cube_info->colors > MaximumUniqueColors)
            break;
        }
      p++;
    }
    if (x < (long) image->columns)
      break;
  }
  image_view=DestroyCacheView(image_view);
  cube_info=DestroyCubeInfo(image,cube_info);
  return(y < (long) image->rows ? MagickFalse : MagickTrue);
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
%        const Image *target_image,long *x_offset,long *y_offset,
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
  const Image *target_image,long *x_offset,long *y_offset,
  ExceptionInfo *exception)
{
#define SearchImageText  "  Searching image...  "

  long
    j,
    y;

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

  register long
    i,
    x;

  CacheView
    *image_view,
    *target_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(target_image != (Image *) NULL);
  assert(target_image->signature == MagickSignature);
  assert(x_offset != (long *) NULL);
  assert(y_offset != (long *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  x=0;
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireCacheView(image);
  target_view=AcquireCacheView(target_image);
  for (y=(*y_offset); y < (long) image->rows; y++)
  {
    for (x=y == 0 ? *x_offset : 0; x < (long) image->columns; x++)
    {
      for (j=0; j < (long) target_image->rows; j++)
      {
        for (i=0; i < (long) target_image->columns; i++)
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
        if (i < (long) target_image->columns)
          break;
      }
      if (j == (long) target_image->rows)
        break;
    }
    if (x < (long) image->columns)
      break;
    if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
        (QuantumTick(y,image->rows) != MagickFalse))
      {
        status=image->progress_monitor(SearchImageText,y,image->rows,
          image->client_data);
        if (status == MagickFalse)
          break;
      }
  }
  target_view=DestroyCacheView(target_view);
  image_view=DestroyCacheView(image_view);
  *x_offset=x;
  *y_offset=y;
  return(y < (long) image->rows ? MagickTrue : MagickFalse);
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
    alpha,
    beta,
    distance;

  if ((p->fuzz == 0.0) && (q->fuzz == 0.0))
    return(IsMagickColorEqual(p,q));
  if (p->fuzz == 0.0)
    fuzz=MagickMax(q->fuzz,MagickSQ1_2)*MagickMax(q->fuzz,MagickSQ1_2);
  else
    if (q->fuzz == 0.0)
      fuzz=3.0*MagickMax(p->fuzz,MagickSQ1_2)*MagickMax(p->fuzz,MagickSQ1_2);
    else
      fuzz=3.0*MagickMax(p->fuzz,MagickSQ1_2)*MagickMax(q->fuzz,MagickSQ1_2);
  alpha=1.0;
  if (p->matte != MagickFalse)
    alpha=(MagickRealType) (QuantumScale*(QuantumRange-p->opacity));
  beta=1.0;
  if (q->matte != MagickFalse)
    beta=(MagickRealType) (QuantumScale*(QuantumRange-q->opacity));
  if (p->colorspace == CMYKColorspace)
    {
      alpha*=(MagickRealType) (QuantumScale*(QuantumRange-p->index));
      beta*=(MagickRealType) (QuantumScale*(QuantumRange-q->index));
    }
  pixel=alpha*p->red-beta*q->red;
  if ((p->colorspace == HSLColorspace) || (p->colorspace == HSBColorspace) ||
      (p->colorspace == HWBColorspace))
    {
      if (fabs(p->red-q->red) > (QuantumRange/2))
        {
          if (p->red > (QuantumRange/2))
            pixel=alpha*(p->red-QuantumRange)-beta*q->red;
          else
            pixel=alpha*p->red-beta*(q->red-QuantumRange);
        }
        pixel*=2;
     }
  distance=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=alpha*p->green-beta*q->green;
  distance+=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=alpha*p->blue-beta*q->blue;
  distance+=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  pixel=p->opacity-q->opacity;
  distance+=pixel*pixel;
  if (distance > fuzz)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M o n o c h r o m e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMonochromeImage() returns MagickTrue if all the pixels in the image have
%  the same red, green, and blue intensities and the intensity is either
%  0 or QuantumRange.
%
%  The format of the IsMonochromeImage method is:
%
%      MagickBooleanType IsMonochromeImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsMonochromeImage(const Image *image,
  ExceptionInfo *exception)
{
  ImageType
    type;

  register const PixelPacket
    *p;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->type == BilevelType)
    return(MagickTrue);
  if (image->colorspace == CMYKColorspace)
    return(MagickFalse);
  type=BilevelType;
  switch (image->storage_class)
  {
    case DirectClass:
    case UndefinedClass:
    {
      long
        y;

      register long
        x;

      CacheView
        *image_view;

      image_view=AcquireCacheView(image);
      for (y=0; y < (long) image->rows; y++)
      {
        p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=0; x < (long) image->columns; x++)
        {
          if (IsMonochromePixel(p) == MagickFalse)
            {
              type=UndefinedType;
              break;
            }
          p++;
        }
        if (type == UndefinedType)
          break;
      }
      image_view=DestroyCacheView(image_view);
      if (y == (long) image->rows)
        ((Image *) image)->type=BilevelType;
      break;
    }
    case PseudoClass:
    {
      register long
        i;

      p=image->colormap;
      for (i=0; i < (long) image->colors; i++)
      {
        if (IsMonochromePixel(p) == MagickFalse)
          {
            type=UndefinedType;
            break;
          }
        p++;
      }
      break;
    }
  }
  if (type == UndefinedType)
    return(MagickFalse);
  ((Image *) image)->type=type;
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
  if (p->opacity == q->opacity)
    return(MagickTrue);
  fuzz=MagickMax(image->fuzz,MagickSQ1_2)*MagickMax(image->fuzz,MagickSQ1_2);
  pixel=(MagickRealType) p->opacity-(MagickRealType) q->opacity;
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
%     I s O p a q u e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsOpaqueImage() returns MagickTrue if none of the pixels in the image have
%  an opacity value other than opaque (0).
%
%  The format of the IsOpaqueImage method is:
%
%      MagickBooleanType IsOpaqueImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsOpaqueImage(const Image *image,
  ExceptionInfo *exception)
{
  long
    y;

  register const PixelPacket
    *p;

  register long
    x;

  CacheView
    *image_view;

  /*
    Determine if image is opaque.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->matte == MagickFalse)
    return(MagickTrue);
  image_view=AcquireCacheView(image);
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < (long) image->columns; x++)
    {
      if (p->opacity != OpaqueOpacity)
        break;
      p++;
    }
    if (x < (long) image->columns)
     break;
  }
  image_view=DestroyCacheView(image_view);
  return(y < (long) image->rows ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s P a l e t t e I m a g e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPaletteImage() returns MagickTrue if the image is PseudoClass and has 256
%  unique colors or less.
%
%  The format of the IsPaletteImage method is:
%
%      MagickBooleanType IsPaletteImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsPaletteImage(const Image *image,
  ExceptionInfo *exception)
{
  CubeInfo
    *cube_info;

  long
    y;

  MagickPixelPacket
    pixel,
    target;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    x;

  register NodeInfo
    *node_info;

  register long
    i;

  unsigned long
    id,
    index,
    level;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->storage_class == PseudoClass) && (image->colors <= 256))
    return(MagickTrue);
  if (image->storage_class == PseudoClass)
    return(MagickFalse);
  /*
    Initialize color description tree.
  */
  cube_info=GetCubeInfo();
  if (cube_info == (CubeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireCacheView(image);
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        SetMagickPixelPacket(image,p,indexes+x,&pixel);
        id=ColorToNodeId(image,&pixel,index);
        if (node_info->child[id] == (NodeInfo *) NULL)
          {
            node_info->child[id]=GetNodeInfo(cube_info,level);
            if (node_info->child[id] == (NodeInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                break;
              }
          }
        node_info=node_info->child[id];
        index--;
      }
      if (level < MaxTreeDepth)
        break;
      for (i=0; i < (long) node_info->number_unique; i++)
      {
        SetMagickPixelPacket(image,&node_info->list[i].pixel,
          &node_info->list[i].index,&target);
        if (IsMagickColorEqual(&pixel,&target) != MagickFalse)
          break;
      }
      if (i < (long) node_info->number_unique)
        node_info->list[i].count++;
      else
        {
          /*
            Add this unique color to the color list.
          */
          if (node_info->number_unique == 0)
            node_info->list=(ColorPacket *) AcquireMagickMemory(
              sizeof(*node_info->list));
          else
            node_info->list=(ColorPacket *) ResizeQuantumMemory(node_info->list,
              (size_t) (i+1),sizeof(*node_info->list));
          if (node_info->list == (ColorPacket *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              break;
            }
          node_info->list[i].pixel=(*p);
          if ((image->colorspace == CMYKColorspace) ||
              (image->storage_class == PseudoClass))
            node_info->list[i].index=indexes[x];
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
          if (cube_info->colors > 256)
            break;
        }
      p++;
    }
    if (x < (long) image->columns)
      break;
  }
  image_view=DestroyCacheView(image_view);
  cube_info=DestroyCubeInfo(image,cube_info);
  return(y < (long) image->rows ? MagickFalse : MagickTrue);
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

  register long
    i;

  unsigned long
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
  for (i=0; i < (long) number_colors; i++)
  {
    if (color_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,color_info[i]->path) != 0))
      {
        if (color_info[i]->path != (char *) NULL)
          (void) fprintf(file,"\nPath: %s\n\n",color_info[i]->path);
        (void) fprintf(file,"Name                  Color                  "
          "                       Compliance\n");
        (void) fprintf(file,"-------------------------------------------------"
          "------------------------------\n");
      }
    path=color_info[i]->path;
    (void) fprintf(file,"%-21.21s ",color_info[i]->name);
    GetColorTuple(&color_info[i]->color,MagickFalse,tuple);
    (void) fprintf(file,"%-45.45s ",tuple);
    if ((color_info[i]->compliance & SVGCompliance) != 0)
      (void) fprintf(file,"SVG ");
    if ((color_info[i]->compliance & X11Compliance) != 0)
      (void) fprintf(file,"X11 ");
    if ((color_info[i]->compliance & XPMCompliance) != 0)
      (void) fprintf(file,"XPM ");
    (void) fprintf(file,"\n");
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
%        const unsigned long depth,ExceptionInfo *exception)
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
  const unsigned long depth,ExceptionInfo *exception)
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
                  xml=FileToString(path,~0,exception);
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
            long
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
#if defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
  return(LoadColorList(ColorMap,"built-in",0,exception));
#else
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  status=MagickFalse;
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status|=LoadColorList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  if ((color_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(color_list) != MagickFalse))
    status|=LoadColorList(ColorMap,"built-in",0,exception);
  return(status != 0 ? MagickTrue : MagickFalse);
#endif
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

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport MagickBooleanType QueryColorDatabase(const char *name,
  PixelPacket *color,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  status=QueryMagickColor(name,&pixel,exception);
  color->opacity=RoundToQuantum(pixel.opacity);
  if (pixel.colorspace == CMYKColorspace)
    {
      color->red=RoundToQuantum((MagickRealType) (QuantumRange-MagickMin(
        QuantumRange,(MagickRealType) (QuantumScale*pixel.red*(QuantumRange-
        pixel.index)+pixel.index))));
      color->green=RoundToQuantum((MagickRealType) (QuantumRange-MagickMin(
        QuantumRange,(MagickRealType) (QuantumScale*pixel.green*(QuantumRange-
        pixel.index)+pixel.index))));
      color->blue=RoundToQuantum((MagickRealType) (QuantumRange-MagickMin(
        QuantumRange,(MagickRealType) (QuantumScale*pixel.blue*(QuantumRange-
        pixel.index)+pixel.index))));
      return(status);
    }
  color->red=RoundToQuantum(pixel.red);
  color->green=RoundToQuantum(pixel.green);
  color->blue=RoundToQuantum(pixel.blue);
  return(status);
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
  GeometryInfo
    geometry_info;

  long
    type;

  MagickRealType
    scale;

  MagickStatusType
    flags;

  register const ColorInfo
    *p;

  register long
    i;

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

      unsigned long
        depth,
        n;

      /*
        Parse hex color.
      */
      (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
      name++;
      for (n=0; isxdigit((int) ((unsigned char) name[n])) != MagickFalse; n++) ;
      if ((n % 3) == 0)
        {
          do
          {
            pixel.red=pixel.green;
            pixel.green=pixel.blue;
            pixel.blue=0;
            for (i=(long) (n/3-1); i >= 0; i--)
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
          } while (isxdigit((int) ((unsigned char) *name)) != MagickFalse);
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
            for (i=(long) (n/4-1); i >= 0; i--)
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
      color->colorspace=RGBColorspace;
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

      /*
        Parse color of the form rgb(100,255,0).
      */
      (void) CopyMagickString(colorspace,name,MaxTextExtent);
      for (i=0; colorspace[i] != '\0'; i++)
        if (colorspace[i] == '(')
          break;
      colorspace[i--]='\0';
      LocaleLower(colorspace);
      color->matte=MagickFalse;
      if ((i > 0) && (colorspace[i] == 'a'))
        {
          colorspace[i]='\0';
          color->matte=MagickTrue;
        }
      type=ParseMagickOption(MagickColorspaceOptions,MagickFalse,colorspace);
      if (type < 0)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            OptionWarning,"UnrecognizedColor","`%s'",name);
          return(MagickFalse);
        }
      color->colorspace=(ColorspaceType) type;
      SetGeometryInfo(&geometry_info);
      flags=ParseGeometry(name+i+1,&geometry_info);
      scale=(MagickRealType) ScaleCharToQuantum(1);
      if ((flags & PercentValue) != 0)
        scale=(MagickRealType) (QuantumRange/100.0);
      if ((flags & RhoValue) != 0)
        color->red=(MagickRealType) RoundToQuantum(scale*geometry_info.rho);
      if ((flags & SigmaValue) != 0)
        color->green=(MagickRealType) RoundToQuantum(scale*geometry_info.sigma);
      if ((flags & XiValue) != 0)
        color->blue=(MagickRealType) RoundToQuantum(scale*geometry_info.xi);
      color->opacity=(MagickRealType) OpaqueOpacity;
      if ((flags & PsiValue) != 0)
        {
          if (color->colorspace == CMYKColorspace)
            color->index=(MagickRealType) RoundToQuantum(scale*
              geometry_info.psi);
          else
            if (color->matte != MagickFalse)
              color->opacity=(MagickRealType) RoundToQuantum((MagickRealType)
                (QuantumRange-QuantumRange*geometry_info.psi));
        }
      if (((flags & ChiValue) != 0) && (color->matte != MagickFalse))
        color->opacity=(MagickRealType) RoundToQuantum((MagickRealType)
          (QuantumRange-QuantumRange*geometry_info.chi));
      if (LocaleCompare(colorspace,"gray") == 0)
        {
          color->green=color->red;
          color->blue=color->red;
          if (((flags & SigmaValue) != 0) && (color->matte != MagickFalse))
            color->opacity=(MagickRealType) RoundToQuantum((MagickRealType)
              (QuantumRange-QuantumRange*geometry_info.sigma));
        }
      if (LocaleCompare(colorspace,"HSL") == 0)
        {
          PixelPacket
            pixel;

          geometry_info.rho=fmod(fmod(geometry_info.rho,360.0)+360.0,360.0)/
            360.0;
          geometry_info.sigma/=100.0;
          geometry_info.xi/=100.0;
          ConvertHSLToRGB(geometry_info.rho,geometry_info.sigma,
            geometry_info.xi,&pixel.red,&pixel.green,&pixel.blue);
          color->colorspace=RGBColorspace;
          color->red=(MagickRealType) pixel.red;
          color->green=(MagickRealType) pixel.green;
          color->blue=(MagickRealType) pixel.blue;
        }
      return(MagickTrue);
    }
  /*
    Parse named color.
  */
  p=GetColorInfo(name,exception);
  if (p == (const ColorInfo *) NULL)
    return(MagickFalse);
  color->colorspace=RGBColorspace;
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
      pixel.depth=(unsigned long) MagickMin(1.0*image->depth,16.0);
      GetColorTuple(&pixel,MagickTrue,name);
      return(MagickTrue);
    }
  GetColorTuple(&pixel,compliance != SVGCompliance ? MagickTrue : MagickFalse,
    name);
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  U n i q u e I m a g e C o l o r s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UniqueImageColors() returns the unique colors of an image.
%
%  The format of the UniqueImageColors method is:
%
%      Image *UniqueImageColors(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void UniqueColorsToImage(Image *image,CubeInfo *cube_info,
  const NodeInfo *node_info,ExceptionInfo *exception)
{
#define UniqueColorsImageTag  "UniqueColors/Image"

  register long
    i;

  unsigned long
    number_children;

  /*
    Traverse any children.
  */
  number_children=image->matte == MagickFalse ? 8UL : 16UL;
  for (i=0; i < (long) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      UniqueColorsToImage(image,cube_info,node_info->child[i],exception);
  if (node_info->level == (MaxTreeDepth-1))
    {
      register ColorPacket
        *p;

      register IndexPacket
        *__restrict indexes;

      register PixelPacket
        *__restrict q;

      p=node_info->list;
      for (i=0; i < (long) node_info->number_unique; i++)
      {
        q=QueueAuthenticPixels(image,cube_info->x,0,1,1,exception);
        if (q == (PixelPacket *) NULL)
          continue;
        indexes=GetAuthenticIndexQueue(image);
        *q=p->pixel;
        if (image->colorspace == CMYKColorspace)
          *indexes=p->index;
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        cube_info->x++;
        p++;
      }
      if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
          (QuantumTick(cube_info->progress,cube_info->colors) != MagickFalse))
        (void) image->progress_monitor(UniqueColorsImageTag,cube_info->progress,
          cube_info->colors,image->client_data);
      cube_info->progress++;
    }
}

MagickExport Image *UniqueImageColors(const Image *image,
  ExceptionInfo *exception)
{
  CubeInfo
    *cube_info;

  Image
    *unique_image;

  cube_info=ClassifyImageColors(image,exception);
  if (cube_info == (CubeInfo *) NULL)
    return((Image *) NULL);
  unique_image=CloneImage(image,cube_info->colors,1,MagickTrue,exception);
  if (unique_image == (Image *) NULL)
    return(unique_image);
  if (SetImageStorageClass(unique_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&unique_image->exception);
      unique_image=DestroyImage(unique_image);
      return((Image *) NULL);
    }
  UniqueColorsToImage(unique_image,cube_info,cube_info->root,exception);
  if (cube_info->colors < MaxColormapSize)
    {
      QuantizeInfo
        *quantize_info;

      quantize_info=AcquireQuantizeInfo((ImageInfo *) NULL);
      quantize_info->number_colors=MaxColormapSize;
      quantize_info->dither=MagickFalse;
      quantize_info->tree_depth=8;
      (void) QuantizeImage(quantize_info,unique_image);
      quantize_info=DestroyQuantizeInfo(quantize_info);
    }
  cube_info=DestroyCubeInfo(image,cube_info);
  return(unique_image);
}

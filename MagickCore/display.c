/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               DDDD   IIIII  SSSSS  PPPP   L       AAA   Y   Y               %
%               D   D    I    SS     P   P  L      A   A   Y Y                %
%               D   D    I     SSS   PPPP   L      AAAAA    Y                 %
%               D   D    I       SS  P      L      A   A    Y                 %
%               DDDD   IIIII  SSSSS  P      LLLLL  A   A    Y                 %
%                                                                             %
%                                                                             %
%        MagickCore Methods to Interactively Display and Edit an Image        %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                                July 1992                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/channel.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/composite.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/delegate.h"
#include "MagickCore/display.h"
#include "MagickCore/display-private.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/shear.h"
#include "MagickCore/segment.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/transform-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#include "MagickCore/widget.h"
#include "MagickCore/widget-private.h"
#include "MagickCore/xwindow.h"
#include "MagickCore/xwindow-private.h"

#if defined(MAGICKCORE_X11_DELEGATE)
/*
  Define declarations.
*/
#define MaxColors  MagickMin((ssize_t) windows->visual_info->colormap_size,256L)

/*
  Constant declarations.
*/
static const unsigned char
  HighlightBitmap[8] =
  {
    0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55
  },
  OpaqueBitmap[8] =
  {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  },
  ShadowBitmap[8] =
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

static const char
  *PageSizes[] =
  {
    "Letter",
    "Tabloid",
    "Ledger",
    "Legal",
    "Statement",
    "Executive",
    "A3",
    "A4",
    "A5",
    "B4",
    "B5",
    "Folio",
    "Quarto",
    "10x14",
    (char *) NULL
  };

/*
  Help widget declarations.
*/
static const char
  *ImageAnnotateHelp[] =
  {
    "In annotate mode, the Command widget has these options:",
    "",
    "    Font Name",
    "      fixed",
    "      variable",
    "      5x8",
    "      6x10",
    "      7x13bold",
    "      8x13bold",
    "      9x15bold",
    "      10x20",
    "      12x24",
    "      Browser...",
    "    Font Color",
    "      black",
    "      blue",
    "      cyan",
    "      green",
    "      gray",
    "      red",
    "      magenta",
    "      yellow",
    "      white",
    "      transparent",
    "      Browser...",
    "    Font Color",
    "      black",
    "      blue",
    "      cyan",
    "      green",
    "      gray",
    "      red",
    "      magenta",
    "      yellow",
    "      white",
    "      transparent",
    "      Browser...",
    "    Rotate Text",
    "      -90",
    "      -45",
    "      -30",
    "      0",
    "      30",
    "      45",
    "      90",
    "      180",
    "      Dialog...",
    "    Help",
    "    Dismiss",
    "",
    "Choose a font name from the Font Name sub-menu.  Additional",
    "font names can be specified with the font browser.  You can",
    "change the menu names by setting the X resources font1",
    "through font9.",
    "",
    "Choose a font color from the Font Color sub-menu.",
    "Additional font colors can be specified with the color",
    "browser.  You can change the menu colors by setting the X",
    "resources pen1 through pen9.",
    "",
    "If you select the color browser and press Grab, you can",
    "choose the font color by moving the pointer to the desired",
    "color on the screen and press any button.",
    "",
    "If you choose to rotate the text, choose Rotate Text from the",
    "menu and select an angle.  Typically you will only want to",
    "rotate one line of text at a time.  Depending on the angle you",
    "choose, subsequent lines may end up overwriting each other.",
    "",
    "Choosing a font and its color is optional.  The default font",
    "is fixed and the default color is black.  However, you must",
    "choose a location to begin entering text and press button 1.",
    "An underscore character will appear at the location of the",
    "pointer.  The cursor changes to a pencil to indicate you are",
    "in text mode.  To exit immediately, press Dismiss.",
    "",
    "In text mode, any key presses will display the character at",
    "the location of the underscore and advance the underscore",
    "cursor.  Enter your text and once completed press Apply to",
    "finish your image annotation.  To correct errors press BACK",
    "SPACE.  To delete an entire line of text, press DELETE.  Any",
    "text that exceeds the boundaries of the image window is",
    "automagically continued onto the next line.",
    "",
    "The actual color you request for the font is saved in the",
    "image.  However, the color that appears in your image window",
    "may be different.  For example, on a monochrome screen the",
    "text will appear black or white even if you choose the color",
    "red as the font color.  However, the image saved to a file",
    "with -write is written with red lettering.  To assure the",
    "correct color text in the final image, any PseudoClass image",
    "is promoted to DirectClass (see miff(5)).  To force a",
    "PseudoClass image to remain PseudoClass, use -colors.",
    (char *) NULL,
  },
  *ImageChopHelp[] =
  {
    "In chop mode, the Command widget has these options:",
    "",
    "    Direction",
    "      horizontal",
    "      vertical",
    "    Help",
    "    Dismiss",
    "",
    "If the you choose the horizontal direction (this the",
    "default), the area of the image between the two horizontal",
    "endpoints of the chop line is removed.  Otherwise, the area",
    "of the image between the two vertical endpoints of the chop",
    "line is removed.",
    "",
    "Select a location within the image window to begin your chop,",
    "press and hold any button.  Next, move the pointer to",
    "another location in the image.  As you move a line will",
    "connect the initial location and the pointer.  When you",
    "release the button, the area within the image to chop is",
    "determined by which direction you choose from the Command",
    "widget.",
    "",
    "To cancel the image chopping, move the pointer back to the",
    "starting point of the line and release the button.",
    (char *) NULL,
  },
  *ImageColorEditHelp[] =
  {
    "In color edit mode, the Command widget has these options:",
    "",
    "    Method",
    "      point",
    "      replace",
    "      floodfill",
    "      filltoborder",
    "      reset",
    "    Pixel Color",
    "      black",
    "      blue",
    "      cyan",
    "      green",
    "      gray",
    "      red",
    "      magenta",
    "      yellow",
    "      white",
    "      Browser...",
    "    Border Color",
    "      black",
    "      blue",
    "      cyan",
    "      green",
    "      gray",
    "      red",
    "      magenta",
    "      yellow",
    "      white",
    "      Browser...",
    "    Fuzz",
    "      0%",
    "      2%",
    "      5%",
    "      10%",
    "      15%",
    "      Dialog...",
    "    Undo",
    "    Help",
    "    Dismiss",
    "",
    "Choose a color editing method from the Method sub-menu",
    "of the Command widget.  The point method recolors any pixel",
    "selected with the pointer until the button is released.  The",
    "replace method recolors any pixel that matches the color of",
    "the pixel you select with a button press.  Floodfill recolors",
    "any pixel that matches the color of the pixel you select with",
    "a button press and is a neighbor.  Whereas filltoborder recolors",
    "any neighbor pixel that is not the border color.  Finally reset",
    "changes the entire image to the designated color.",
    "",
    "Next, choose a pixel color from the Pixel Color sub-menu.",
    "Additional pixel colors can be specified with the color",
    "browser.  You can change the menu colors by setting the X",
    "resources pen1 through pen9.",
    "",
    "Now press button 1 to select a pixel within the image window",
    "to change its color.  Additional pixels may be recolored as",
    "prescribed by the method you choose.",
    "",
    "If the Magnify widget is mapped, it can be helpful in positioning",
    "your pointer within the image (refer to button 2).",
    "",
    "The actual color you request for the pixels is saved in the",
    "image.  However, the color that appears in your image window",
    "may be different.  For example, on a monochrome screen the",
    "pixel will appear black or white even if you choose the",
    "color red as the pixel color.  However, the image saved to a",
    "file with -write is written with red pixels.  To assure the",
    "correct color text in the final image, any PseudoClass image",
    "is promoted to DirectClass (see miff(5)).  To force a",
    "PseudoClass image to remain PseudoClass, use -colors.",
    (char *) NULL,
  },
  *ImageCompositeHelp[] =
  {
    "First a widget window is displayed requesting you to enter an",
    "image name. Press Composite, Grab or type a file name.",
    "Press Cancel if you choose not to create a composite image.",
    "When you choose Grab, move the pointer to the desired window",
    "and press any button.",
    "",
    "If the Composite image does not have any matte information,",
    "you are informed and the file browser is displayed again.",
    "Enter the name of a mask image.  The image is typically",
    "grayscale and the same size as the composite image.  If the",
    "image is not grayscale, it is converted to grayscale and the",
    "resulting intensities are used as matte information.",
    "",
    "A small window appears showing the location of the cursor in",
    "the image window. You are now in composite mode.  To exit",
    "immediately, press Dismiss.  In composite mode, the Command",
    "widget has these options:",
    "",
    "    Operators",
    "      Over",
    "      In",
    "      Out",
    "      Atop",
    "      Xor",
    "      Plus",
    "      Minus",
    "      Add",
    "      Subtract",
    "      Difference",
    "      Multiply",
    "      Bumpmap",
    "      Copy",
    "      CopyRed",
    "      CopyGreen",
    "      CopyBlue",
    "      CopyOpacity",
    "      Clear",
    "    Dissolve",
    "    Displace",
    "    Help",
    "    Dismiss",
    "",
    "Choose a composite operation from the Operators sub-menu of",
    "the Command widget.  How each operator behaves is described",
    "below.  Image window is the image currently displayed on",
    "your X server and image is the image obtained with the File",
    "Browser widget.",
    "",
    "Over     The result is the union of the two image shapes,",
    "         with image obscuring image window in the region of",
    "         overlap.",
    "",
    "In       The result is simply image cut by the shape of",
    "         image window.  None of the image data of image",
    "         window is in the result.",
    "",
    "Out      The resulting image is image with the shape of",
    "         image window cut out.",
    "",
    "Atop     The result is the same shape as image image window,",
    "         with image obscuring image window where the image",
    "         shapes overlap.  Note this differs from over",
    "         because the portion of image outside image window's",
    "         shape does not appear in the result.",
    "",
    "Xor      The result is the image data from both image and",
    "         image window that is outside the overlap region.",
    "         The overlap region is blank.",
    "",
    "Plus     The result is just the sum of the image data.",
    "         Output values are cropped to QuantumRange (no overflow).",
    "",
    "Minus    The result of image - image window, with underflow",
    "         cropped to zero.",
    "",
    "Add      The result of image + image window, with overflow",
    "         wrapping around (mod 256).",
    "",
    "Subtract The result of image - image window, with underflow",
    "         wrapping around (mod 256).  The add and subtract",
    "         operators can be used to perform reversible",
    "         transformations.",
    "",
    "Difference",
    "         The result of abs(image - image window).  This",
    "         useful for comparing two very similar images.",
    "",
    "Multiply",
    "         The result of image * image window.  This",
    "         useful for the creation of drop-shadows.",
    "",
    "Bumpmap  The result of surface normals from image * image",
    "         window.",
    "",
    "Copy     The resulting image is image window replaced with",
    "         image.  Here the matte information is ignored.",
    "",
    "CopyRed  The red layer of the image window is replace with",
    "         the red layer of the image.  The other layers are",
    "         untouched.",
    "",
    "CopyGreen",
    "         The green layer of the image window is replace with",
    "         the green layer of the image.  The other layers are",
    "         untouched.",
    "",
    "CopyBlue The blue layer of the image window is replace with",
    "         the blue layer of the image.  The other layers are",
    "         untouched.",
    "",
    "CopyOpacity",
    "         The matte layer of the image window is replace with",
    "         the matte layer of the image.  The other layers are",
    "         untouched.",
    "",
    "The image compositor requires a matte, or alpha channel in",
    "the image for some operations.  This extra channel usually",
    "defines a mask which represents a sort of a cookie-cutter",
    "for the image.  This the case when matte is opaque (full",
    "coverage) for pixels inside the shape, zero outside, and",
    "between 0 and QuantumRange on the boundary.  If image does not",
    "have a matte channel, it is initialized with 0 for any pixel",
    "matching in color to pixel location (0,0), otherwise QuantumRange.",
    "",
    "If you choose Dissolve, the composite operator becomes Over.  The",
    "image matte channel percent transparency is initialized to factor.",
    "The image window is initialized to (100-factor). Where factor is the",
    "value you specify in the Dialog widget.",
    "",
    "Displace shifts the image pixels as defined by a displacement",
    "map.  With this option, image is used as a displacement map.",
    "Black, within the displacement map, is a maximum positive",
    "displacement.  White is a maximum negative displacement and",
    "middle gray is neutral.  The displacement is scaled to determine",
    "the pixel shift.  By default, the displacement applies in both the",
    "horizontal and vertical directions.  However, if you specify a mask,",
    "image is the horizontal X displacement and mask the vertical Y",
    "displacement.",
    "",
    "Note that matte information for image window is not retained",
    "for colormapped X server visuals (e.g. StaticColor,",
    "StaticColor, GrayScale, PseudoColor).  Correct compositing",
    "behavior may require a TrueColor or DirectColor visual or a",
    "Standard Colormap.",
    "",
    "Choosing a composite operator is optional.  The default",
    "operator is replace.  However, you must choose a location to",
    "composite your image and press button 1.  Press and hold the",
    "button before releasing and an outline of the image will",
    "appear to help you identify your location.",
    "",
    "The actual colors of the composite image is saved.  However,",
    "the color that appears in image window may be different.",
    "For example, on a monochrome screen image window will appear",
    "black or white even though your composited image may have",
    "many colors.  If the image is saved to a file it is written",
    "with the correct colors.  To assure the correct colors are",
    "saved in the final image, any PseudoClass image is promoted",
    "to DirectClass (see miff(5)).  To force a PseudoClass image",
    "to remain PseudoClass, use -colors.",
    (char *) NULL,
  },
  *ImageCutHelp[] =
  {
    "In cut mode, the Command widget has these options:",
    "",
    "    Help",
    "    Dismiss",
    "",
    "To define a cut region, press button 1 and drag.  The",
    "cut region is defined by a highlighted rectangle that",
    "expands or contracts as it follows the pointer.  Once you",
    "are satisfied with the cut region, release the button.",
    "You are now in rectify mode.  In rectify mode, the Command",
    "widget has these options:",
    "",
    "    Cut",
    "    Help",
    "    Dismiss",
    "",
    "You can make adjustments by moving the pointer to one of the",
    "cut rectangle corners, pressing a button, and dragging.",
    "Finally, press Cut to commit your copy region.  To",
    "exit without cutting the image, press Dismiss.",
    (char *) NULL,
  },
  *ImageCopyHelp[] =
  {
    "In copy mode, the Command widget has these options:",
    "",
    "    Help",
    "    Dismiss",
    "",
    "To define a copy region, press button 1 and drag.  The",
    "copy region is defined by a highlighted rectangle that",
    "expands or contracts as it follows the pointer.  Once you",
    "are satisfied with the copy region, release the button.",
    "You are now in rectify mode.  In rectify mode, the Command",
    "widget has these options:",
    "",
    "    Copy",
    "    Help",
    "    Dismiss",
    "",
    "You can make adjustments by moving the pointer to one of the",
    "copy rectangle corners, pressing a button, and dragging.",
    "Finally, press Copy to commit your copy region.  To",
    "exit without copying the image, press Dismiss.",
    (char *) NULL,
  },
  *ImageCropHelp[] =
  {
    "In crop mode, the Command widget has these options:",
    "",
    "    Help",
    "    Dismiss",
    "",
    "To define a cropping region, press button 1 and drag.  The",
    "cropping region is defined by a highlighted rectangle that",
    "expands or contracts as it follows the pointer.  Once you",
    "are satisfied with the cropping region, release the button.",
    "You are now in rectify mode.  In rectify mode, the Command",
    "widget has these options:",
    "",
    "    Crop",
    "    Help",
    "    Dismiss",
    "",
    "You can make adjustments by moving the pointer to one of the",
    "cropping rectangle corners, pressing a button, and dragging.",
    "Finally, press Crop to commit your cropping region.  To",
    "exit without cropping the image, press Dismiss.",
    (char *) NULL,
  },
  *ImageDrawHelp[] =
  {
    "The cursor changes to a crosshair to indicate you are in",
    "draw mode.  To exit immediately, press Dismiss.  In draw mode,",
    "the Command widget has these options:",
    "",
    "    Element",
    "      point",
    "      line",
    "      rectangle",
    "      fill rectangle",
    "      circle",
    "      fill circle",
    "      ellipse",
    "      fill ellipse",
    "      polygon",
    "      fill polygon",
    "    Color",
    "      black",
    "      blue",
    "      cyan",
    "      green",
    "      gray",
    "      red",
    "      magenta",
    "      yellow",
    "      white",
    "      transparent",
    "      Browser...",
    "    Stipple",
    "      Brick",
    "      Diagonal",
    "      Scales",
    "      Vertical",
    "      Wavy",
    "      Translucent",
    "      Opaque",
    "      Open...",
    "    Width",
    "      1",
    "      2",
    "      4",
    "      8",
    "      16",
    "      Dialog...",
    "    Undo",
    "    Help",
    "    Dismiss",
    "",
    "Choose a drawing primitive from the Element sub-menu.",
    "",
    "Choose a color from the Color sub-menu.  Additional",
    "colors can be specified with the color browser.",
    "",
    "If you choose the color browser and press Grab, you can",
    "select the color by moving the pointer to the desired",
    "color on the screen and press any button.  The transparent",
    "color updates the image matte channel and is useful for",
    "image compositing.",
    "",
    "Choose a stipple, if appropriate, from the Stipple sub-menu.",
    "Additional stipples can be specified with the file browser.",
    "Stipples obtained from the file browser must be on disk in the",
    "X11 bitmap format.",
    "",
    "Choose a width, if appropriate, from the Width sub-menu.  To",
    "choose a specific width select the Dialog widget.",
    "",
    "Choose a point in the Image window and press button 1 and",
    "hold.  Next, move the pointer to another location in the",
    "image.  As you move, a line connects the initial location and",
    "the pointer.  When you release the button, the image is",
    "updated with the primitive you just drew.  For polygons, the",
    "image is updated when you press and release the button without",
    "moving the pointer.",
    "",
    "To cancel image drawing, move the pointer back to the",
    "starting point of the line and release the button.",
    (char *) NULL,
  },
  *DisplayHelp[] =
  {
    "BUTTONS",
    "  The effects of each button press is described below.  Three",
    "  buttons are required.  If you have a two button mouse,",
    "  button 1 and 3 are returned.  Press ALT and button 3 to",
    "  simulate button 2.",
    "",
    "  1    Press this button to map or unmap the Command widget.",
    "",
    "  2    Press and drag to define a region of the image to",
    "       magnify.",
    "",
    "  3    Press and drag to choose from a select set of commands.",
    "       This button behaves differently if the image being",
    "       displayed is a visual image directory.  Here, choose a",
    "       particular tile of the directory and press this button and",
    "       drag to select a command from a pop-up menu.  Choose from",
    "       these menu items:",
    "",
    "           Open",
    "           Next",
    "           Former",
    "           Delete",
    "           Update",
    "",
    "       If you choose Open, the image represented by the tile is",
    "       displayed.  To return to the visual image directory, choose",
    "       Next from the Command widget.  Next and Former moves to the",
    "       next or former image respectively.  Choose Delete to delete",
    "       a particular image tile.  Finally, choose Update to",
    "       synchronize all the image tiles with their respective",
    "       images.",
    "",
    "COMMAND WIDGET",
    "  The Command widget lists a number of sub-menus and commands.",
    "  They are",
    "",
    "      File",
    "        Open...",
    "        Next",
    "        Former",
    "        Select...",
    "        Save...",
    "        Print...",
    "        Delete...",
    "        New...",
    "        Visual Directory...",
    "        Quit",
    "      Edit",
    "        Undo",
    "        Redo",
    "        Cut",
    "        Copy",
    "        Paste",
    "      View",
    "        Half Size",
    "        Original Size",
    "        Double Size",
    "        Resize...",
    "        Apply",
    "        Refresh",
    "        Restore",
    "      Transform",
    "        Crop",
    "        Chop",
    "        Flop",
    "        Flip",
    "        Rotate Right",
    "        Rotate Left",
    "        Rotate...",
    "        Shear...",
    "        Roll...",
    "        Trim Edges",
    "      Enhance",
    "        Brightness...",
    "        Saturation...",
    "        Hue...",
    "        Gamma...",
    "        Sharpen...",
    "        Dull",
    "        Contrast Stretch...",
    "        Sigmoidal Contrast...",
    "        Normalize",
    "        Equalize",
    "        Negate",
    "        Grayscale",
    "        Map...",
    "        Quantize...",
    "      Effects",
    "        Despeckle",
    "        Emboss",
    "        Reduce Noise",
    "        Add Noise",
    "        Sharpen...",
    "        Blur...",
    "        Threshold...",
    "        Edge Detect...",
    "        Spread...",
    "        Shade...",
    "        Painting...",
    "        Segment...",
    "      F/X",
    "        Solarize...",
    "        Sepia Tone...",
    "        Swirl...",
    "        Implode...",
    "        Vignette...",
    "        Wave...",
    "        Oil Painting...",
    "        Charcoal Drawing...",
    "      Image Edit",
    "        Annotate...",
    "        Draw...",
    "        Color...",
    "        Matte...",
    "        Composite...",
    "        Add Border...",
    "        Add Frame...",
    "        Comment...",
    "        Launch...",
    "        Region of Interest...",
    "      Miscellany",
    "        Image Info",
    "        Zoom Image",
    "        Show Preview...",
    "        Show Histogram",
    "        Show Matte",
    "        Background...",
    "        Slide Show",
    "        Preferences...",
    "      Help",
    "        Overview",
    "        Browse Documentation",
    "        About Display",
    "",
    "  Menu items with a indented triangle have a sub-menu.  They",
    "  are represented above as the indented items.  To access a",
    "  sub-menu item, move the pointer to the appropriate menu and",
    "  press a button and drag.  When you find the desired sub-menu",
    "  item, release the button and the command is executed.  Move",
    "  the pointer away from the sub-menu if you decide not to",
    "  execute a particular command.",
    "",
    "KEYBOARD ACCELERATORS",
    "  Accelerators are one or two key presses that effect a",
    "  particular command.  The keyboard accelerators that",
    "  display(1) understands is:",
    "",
    "  Ctl+O     Press to open an image from a file.",
    "",
    "  space     Press to display the next image.",
    "",
    "            If the image is a multi-paged document such as a Postscript",
    "            document, you can skip ahead several pages by preceding",
    "            this command with a number.  For example to display the",
    "            third page beyond the current page, press 3<space>.",
    "",
    "  backspace Press to display the former image.",
    "",
    "            If the image is a multi-paged document such as a Postscript",
    "            document, you can skip behind several pages by preceding",
    "            this command with a number.  For example to display the",
    "            third page preceding the current page, press 3<backspace>.",
    "",
    "  Ctl+S     Press to write the image to a file.",
    "",
    "  Ctl+P     Press to print the image to a Postscript printer.",
    "",
    "  Ctl+D     Press to delete an image file.",
    "",
    "  Ctl+N     Press to create a blank canvas.",
    "",
    "  Ctl+Q     Press to discard all images and exit program.",
    "",
    "  Ctl+Z     Press to undo last image transformation.",
    "",
    "  Ctl+R     Press to redo last image transformation.",
    "",
    "  Ctl+X     Press to cut a region of the image.",
    "",
    "  Ctl+C     Press to copy a region of the image.",
    "",
    "  Ctl+V     Press to paste a region to the image.",
    "",
    "  <         Press to half the image size.",
    "",
    "  -         Press to return to the original image size.",
    "",
    "  >         Press to double the image size.",
    "",
    "  %         Press to resize the image to a width and height you",
    "            specify.",
    "",
    "Cmd-A       Press to make any image transformations permanent."
    "",
    "            By default, any image size transformations are applied",
    "            to the original image to create the image displayed on",
    "            the X server.  However, the transformations are not",
    "            permanent (i.e. the original image does not change",
    "            size only the X image does).  For example, if you",
    "            press > the X image will appear to double in size,",
    "            but the original image will in fact remain the same size.",
    "            To force the original image to double in size, press >",
    "            followed by Cmd-A.",
    "",
    "  @         Press to refresh the image window.",
    "",
    "  C         Press to cut out a rectangular region of the image.",
    "",
    "  [         Press to chop the image.",
    "",
    "  H         Press to flop image in the horizontal direction.",
    "",
    "  V         Press to flip image in the vertical direction.",
    "",
    "  /         Press to rotate the image 90 degrees clockwise.",
    "",
    " \\         Press to rotate the image 90 degrees counter-clockwise.",
    "",
    "  *         Press to rotate the image the number of degrees you",
    "            specify.",
    "",
    "  S         Press to shear the image the number of degrees you",
    "            specify.",
    "",
    "  R         Press to roll the image.",
    "",
    "  T         Press to trim the image edges.",
    "",
    "  Shft-H    Press to vary the image hue.",
    "",
    "  Shft-S    Press to vary the color saturation.",
    "",
    "  Shft-L    Press to vary the color brightness.",
    "",
    "  Shft-G    Press to gamma correct the image.",
    "",
    "  Shft-C    Press to sharpen the image contrast.",
    "",
    "  Shft-Z    Press to dull the image contrast.",
    "",
    "  =         Press to perform histogram equalization on the image.",
    "",
    "  Shft-N    Press to perform histogram normalization on the image.",
    "",
    "  Shft-~    Press to negate the colors of the image.",
    "",
    "  .         Press to convert the image colors to gray.",
    "",
    "  Shft-#    Press to set the maximum number of unique colors in the",
    "            image.",
    "",
    "  F2        Press to reduce the speckles in an image.",
    "",
    "  F3        Press to eliminate peak noise from an image.",
    "",
    "  F4        Press to add noise to an image.",
    "",
    "  F5        Press to sharpen an image.",
    "",
    "  F6        Press to delete an image file.",
    "",
    "  F7        Press to threshold the image.",
    "",
    "  F8        Press to detect edges within an image.",
    "",
    "  F9        Press to emboss an image.",
    "",
    "  F10       Press to displace pixels by a random amount.",
    "",
    "  F11       Press to negate all pixels above the threshold level.",
    "",
    "  F12       Press to shade the image using a distant light source.",
    "",
    "  F13       Press to lighten or darken image edges to create a 3-D effect.",
    "",
    "  F14       Press to segment the image by color.",
    "",
    "  Meta-S    Press to swirl image pixels about the center.",
    "",
    "  Meta-I    Press to implode image pixels about the center.",
    "",
    "  Meta-W    Press to alter an image along a sine wave.",
    "",
    "  Meta-P    Press to simulate an oil painting.",
    "",
    "  Meta-C    Press to simulate a charcoal drawing.",
    "",
    "  Alt-A     Press to annotate the image with text.",
    "",
    "  Alt-D     Press to draw on an image.",
    "",
    "  Alt-P     Press to edit an image pixel color.",
    "",
    "  Alt-M     Press to edit the image matte information.",
    "",
    "  Alt-V     Press to composite the image with another.",
    "",
    "  Alt-B     Press to add a border to the image.",
    "",
    "  Alt-F     Press to add an ornamental border to the image.",
    "",
    "  Alt-Shft-!",
    "            Press to add an image comment.",
    "",
    "  Ctl-A     Press to apply image processing techniques to a region",
    "            of interest.",
    "",
    "  Shft-?    Press to display information about the image.",
    "",
    "  Shft-+    Press to map the zoom image window.",
    "",
    "  Shft-P    Press to preview an image enhancement, effect, or f/x.",
    "",
    "  F1        Press to display helpful information about display(1).",
    "",
    "  Find      Press to browse documentation about ImageMagick.",
    "",
    "  1-9       Press to change the level of magnification.",
    "",
    "  Use the arrow keys to move the image one pixel up, down,",
    "  left, or right within the magnify window.  Be sure to first",
    "  map the magnify window by pressing button 2.",
    "",
    "  Press ALT and one of the arrow keys to trim off one pixel",
    "  from any side of the image.",
    (char *) NULL,
  },
  *ImageMatteEditHelp[] =
  {
    "Matte information within an image is useful for some",
    "operations such as image compositing (See IMAGE",
    "COMPOSITING).  This extra channel usually defines a mask",
    "which represents a sort of a cookie-cutter for the image.",
    "This the case when matte is opaque (full coverage) for",
    "pixels inside the shape, zero outside, and between 0 and",
    "QuantumRange on the boundary.",
    "",
    "A small window appears showing the location of the cursor in",
    "the image window. You are now in matte edit mode.  To exit",
    "immediately, press Dismiss.  In matte edit mode, the Command",
    "widget has these options:",
    "",
    "    Method",
    "      point",
    "      replace",
    "      floodfill",
    "      filltoborder",
    "      reset",
    "    Border Color",
    "      black",
    "      blue",
    "      cyan",
    "      green",
    "      gray",
    "      red",
    "      magenta",
    "      yellow",
    "      white",
    "      Browser...",
    "    Fuzz",
    "      0%",
    "      2%",
    "      5%",
    "      10%",
    "      15%",
    "      Dialog...",
    "    Matte",
    "      Opaque",
    "      Transparent",
    "      Dialog...",
    "    Undo",
    "    Help",
    "    Dismiss",
    "",
    "Choose a matte editing method from the Method sub-menu of",
    "the Command widget.  The point method changes the matte value",
    "of any pixel selected with the pointer until the button is",
    "is released.  The replace method changes the matte value of",
    "any pixel that matches the color of the pixel you select with",
    "a button press.  Floodfill changes the matte value of any pixel",
    "that matches the color of the pixel you select with a button",
    "press and is a neighbor.  Whereas filltoborder changes the matte",
    "value any neighbor pixel that is not the border color.  Finally",
    "reset changes the entire image to the designated matte value.",
    "",
    "Choose Matte Value and pick Opaque or Transarent.  For other values",
    "select the Dialog entry.  Here a dialog appears requesting a matte",
    "value.  The value you select is assigned as the opacity value of the",
    "selected pixel or pixels.",
    "",
    "Now, press any button to select a pixel within the image",
    "window to change its matte value.",
    "",
    "If the Magnify widget is mapped, it can be helpful in positioning",
    "your pointer within the image (refer to button 2).",
    "",
    "Matte information is only valid in a DirectClass image.",
    "Therefore, any PseudoClass image is promoted to DirectClass",
    "(see miff(5)).  Note that matte information for PseudoClass",
    "is not retained for colormapped X server visuals (e.g.",
    "StaticColor, StaticColor, GrayScale, PseudoColor) unless you",
    "immediately save your image to a file (refer to Write).",
    "Correct matte editing behavior may require a TrueColor or",
    "DirectColor visual or a Standard Colormap.",
    (char *) NULL,
  },
  *ImagePanHelp[] =
  {
    "When an image exceeds the width or height of the X server",
    "screen, display maps a small panning icon.  The rectangle",
    "within the panning icon shows the area that is currently",
    "displayed in the image window.  To pan about the image,",
    "press any button and drag the pointer within the panning",
    "icon.  The pan rectangle moves with the pointer and the",
    "image window is updated to reflect the location of the",
    "rectangle within the panning icon.  When you have selected",
    "the area of the image you wish to view, release the button.",
    "",
    "Use the arrow keys to pan the image one pixel up, down,",
    "left, or right within the image window.",
    "",
    "The panning icon is withdrawn if the image becomes smaller",
    "than the dimensions of the X server screen.",
    (char *) NULL,
  },
  *ImagePasteHelp[] =
  {
    "A small window appears showing the location of the cursor in",
    "the image window. You are now in paste mode.  To exit",
    "immediately, press Dismiss.  In paste mode, the Command",
    "widget has these options:",
    "",
    "    Operators",
    "      over",
    "      in",
    "      out",
    "      atop",
    "      xor",
    "      plus",
    "      minus",
    "      add",
    "      subtract",
    "      difference",
    "      replace",
    "    Help",
    "    Dismiss",
    "",
    "Choose a composite operation from the Operators sub-menu of",
    "the Command widget.  How each operator behaves is described",
    "below.  Image window is the image currently displayed on",
    "your X server and image is the image obtained with the File",
    "Browser widget.",
    "",
    "Over     The result is the union of the two image shapes,",
    "         with image obscuring image window in the region of",
    "         overlap.",
    "",
    "In       The result is simply image cut by the shape of",
    "         image window.  None of the image data of image",
    "         window is in the result.",
    "",
    "Out      The resulting image is image with the shape of",
    "         image window cut out.",
    "",
    "Atop     The result is the same shape as image image window,",
    "         with image obscuring image window where the image",
    "         shapes overlap.  Note this differs from over",
    "         because the portion of image outside image window's",
    "         shape does not appear in the result.",
    "",
    "Xor      The result is the image data from both image and",
    "         image window that is outside the overlap region.",
    "         The overlap region is blank.",
    "",
    "Plus     The result is just the sum of the image data.",
    "         Output values are cropped to QuantumRange (no overflow).",
    "         This operation is independent of the matte",
    "         channels.",
    "",
    "Minus    The result of image - image window, with underflow",
    "         cropped to zero.",
    "",
    "Add      The result of image + image window, with overflow",
    "         wrapping around (mod 256).",
    "",
    "Subtract The result of image - image window, with underflow",
    "         wrapping around (mod 256).  The add and subtract",
    "         operators can be used to perform reversible",
    "         transformations.",
    "",
    "Difference",
    "         The result of abs(image - image window).  This",
    "         useful for comparing two very similar images.",
    "",
    "Copy     The resulting image is image window replaced with",
    "         image.  Here the matte information is ignored.",
    "",
    "CopyRed  The red layer of the image window is replace with",
    "         the red layer of the image.  The other layers are",
    "         untouched.",
    "",
    "CopyGreen",
    "         The green layer of the image window is replace with",
    "         the green layer of the image.  The other layers are",
    "         untouched.",
    "",
    "CopyBlue The blue layer of the image window is replace with",
    "         the blue layer of the image.  The other layers are",
    "         untouched.",
    "",
    "CopyOpacity",
    "         The matte layer of the image window is replace with",
    "         the matte layer of the image.  The other layers are",
    "         untouched.",
    "",
    "The image compositor requires a matte, or alpha channel in",
    "the image for some operations.  This extra channel usually",
    "defines a mask which represents a sort of a cookie-cutter",
    "for the image.  This the case when matte is opaque (full",
    "coverage) for pixels inside the shape, zero outside, and",
    "between 0 and QuantumRange on the boundary.  If image does not",
    "have a matte channel, it is initialized with 0 for any pixel",
    "matching in color to pixel location (0,0), otherwise QuantumRange.",
    "",
    "Note that matte information for image window is not retained",
    "for colormapped X server visuals (e.g. StaticColor,",
    "StaticColor, GrayScale, PseudoColor).  Correct compositing",
    "behavior may require a TrueColor or DirectColor visual or a",
    "Standard Colormap.",
    "",
    "Choosing a composite operator is optional.  The default",
    "operator is replace.  However, you must choose a location to",
    "paste your image and press button 1.  Press and hold the",
    "button before releasing and an outline of the image will",
    "appear to help you identify your location.",
    "",
    "The actual colors of the pasted image is saved.  However,",
    "the color that appears in image window may be different.",
    "For example, on a monochrome screen image window will appear",
    "black or white even though your pasted image may have",
    "many colors.  If the image is saved to a file it is written",
    "with the correct colors.  To assure the correct colors are",
    "saved in the final image, any PseudoClass image is promoted",
    "to DirectClass (see miff(5)).  To force a PseudoClass image",
    "to remain PseudoClass, use -colors.",
    (char *) NULL,
  },
  *ImageROIHelp[] =
  {
    "In region of interest mode, the Command widget has these",
    "options:",
    "",
    "    Help",
    "    Dismiss",
    "",
    "To define a region of interest, press button 1 and drag.",
    "The region of interest is defined by a highlighted rectangle",
    "that expands or contracts as it follows the pointer.  Once",
    "you are satisfied with the region of interest, release the",
    "button.  You are now in apply mode.  In apply mode the",
    "Command widget has these options:",
    "",
    "      File",
    "        Save...",
    "        Print...",
    "      Edit",
    "        Undo",
    "        Redo",
    "      Transform",
    "        Flop",
    "        Flip",
    "        Rotate Right",
    "        Rotate Left",
    "      Enhance",
    "        Hue...",
    "        Saturation...",
    "        Brightness...",
    "        Gamma...",
    "        Spiff",
    "        Dull",
    "        Contrast Stretch",
    "        Sigmoidal Contrast...",
    "        Normalize",
    "        Equalize",
    "        Negate",
    "        Grayscale",
    "        Map...",
    "        Quantize...",
    "      Effects",
    "        Despeckle",
    "        Emboss",
    "        Reduce Noise",
    "        Sharpen...",
    "        Blur...",
    "        Threshold...",
    "        Edge Detect...",
    "        Spread...",
    "        Shade...",
    "        Raise...",
    "        Segment...",
    "      F/X",
    "        Solarize...",
    "        Sepia Tone...",
    "        Swirl...",
    "        Implode...",
    "        Vignette...",
    "        Wave...",
    "        Oil Painting...",
    "        Charcoal Drawing...",
    "      Miscellany",
    "        Image Info",
    "        Zoom Image",
    "        Show Preview...",
    "        Show Histogram",
    "        Show Matte",
    "      Help",
    "      Dismiss",
    "",
    "You can make adjustments to the region of interest by moving",
    "the pointer to one of the rectangle corners, pressing a",
    "button, and dragging.  Finally, choose an image processing",
    "technique from the Command widget.  You can choose more than",
    "one image processing technique to apply to an area.",
    "Alternatively, you can move the region of interest before",
    "applying another image processing technique.  To exit, press",
    "Dismiss.",
    (char *) NULL,
  },
  *ImageRotateHelp[] =
  {
    "In rotate mode, the Command widget has these options:",
    "",
    "    Pixel Color",
    "      black",
    "      blue",
    "      cyan",
    "      green",
    "      gray",
    "      red",
    "      magenta",
    "      yellow",
    "      white",
    "      Browser...",
    "    Direction",
    "      horizontal",
    "      vertical",
    "    Help",
    "    Dismiss",
    "",
    "Choose a background color from the Pixel Color sub-menu.",
    "Additional background colors can be specified with the color",
    "browser.  You can change the menu colors by setting the X",
    "resources pen1 through pen9.",
    "",
    "If you choose the color browser and press Grab, you can",
    "select the background color by moving the pointer to the",
    "desired color on the screen and press any button.",
    "",
    "Choose a point in the image window and press this button and",
    "hold.  Next, move the pointer to another location in the",
    "image.  As you move a line connects the initial location and",
    "the pointer.  When you release the button, the degree of",
    "image rotation is determined by the slope of the line you",
    "just drew.  The slope is relative to the direction you",
    "choose from the Direction sub-menu of the Command widget.",
    "",
    "To cancel the image rotation, move the pointer back to the",
    "starting point of the line and release the button.",
    (char *) NULL,
  };

/*
  Enumeration declarations.
*/
typedef enum
{
  CopyMode,
  CropMode,
  CutMode
} ClipboardMode;

typedef enum
{
  OpenCommand,
  NextCommand,
  FormerCommand,
  SelectCommand,
  SaveCommand,
  PrintCommand,
  DeleteCommand,
  NewCommand,
  VisualDirectoryCommand,
  QuitCommand,
  UndoCommand,
  RedoCommand,
  CutCommand,
  CopyCommand,
  PasteCommand,
  HalfSizeCommand,
  OriginalSizeCommand,
  DoubleSizeCommand,
  ResizeCommand,
  ApplyCommand,
  RefreshCommand,
  RestoreCommand,
  CropCommand,
  ChopCommand,
  FlopCommand,
  FlipCommand,
  RotateRightCommand,
  RotateLeftCommand,
  RotateCommand,
  ShearCommand,
  RollCommand,
  TrimCommand,
  HueCommand,
  SaturationCommand,
  BrightnessCommand,
  GammaCommand,
  SpiffCommand,
  DullCommand,
  ContrastStretchCommand,
  SigmoidalContrastCommand,
  NormalizeCommand,
  EqualizeCommand,
  NegateCommand,
  GrayscaleCommand,
  MapCommand,
  QuantizeCommand,
  DespeckleCommand,
  EmbossCommand,
  ReduceNoiseCommand,
  AddNoiseCommand,
  SharpenCommand,
  BlurCommand,
  ThresholdCommand,
  EdgeDetectCommand,
  SpreadCommand,
  ShadeCommand,
  RaiseCommand,
  SegmentCommand,
  SolarizeCommand,
  SepiaToneCommand,
  SwirlCommand,
  ImplodeCommand,
  VignetteCommand,
  WaveCommand,
  OilPaintCommand,
  CharcoalDrawCommand,
  AnnotateCommand,
  DrawCommand,
  ColorCommand,
  MatteCommand,
  CompositeCommand,
  AddBorderCommand,
  AddFrameCommand,
  CommentCommand,
  LaunchCommand,
  RegionofInterestCommand,
  ROIHelpCommand,
  ROIDismissCommand,
  InfoCommand,
  ZoomCommand,
  ShowPreviewCommand,
  ShowHistogramCommand,
  ShowMatteCommand,
  BackgroundCommand,
  SlideShowCommand,
  PreferencesCommand,
  HelpCommand,
  BrowseDocumentationCommand,
  VersionCommand,
  SaveToUndoBufferCommand,
  FreeBuffersCommand,
  NullCommand
} CommandType;

typedef enum
{
  AnnotateNameCommand,
  AnnotateFontColorCommand,
  AnnotateBackgroundColorCommand,
  AnnotateRotateCommand,
  AnnotateHelpCommand,
  AnnotateDismissCommand,
  TextHelpCommand,
  TextApplyCommand,
  ChopDirectionCommand,
  ChopHelpCommand,
  ChopDismissCommand,
  HorizontalChopCommand,
  VerticalChopCommand,
  ColorEditMethodCommand,
  ColorEditColorCommand,
  ColorEditBorderCommand,
  ColorEditFuzzCommand,
  ColorEditUndoCommand,
  ColorEditHelpCommand,
  ColorEditDismissCommand,
  CompositeOperatorsCommand,
  CompositeDissolveCommand,
  CompositeDisplaceCommand,
  CompositeHelpCommand,
  CompositeDismissCommand,
  CropHelpCommand,
  CropDismissCommand,
  RectifyCopyCommand,
  RectifyHelpCommand,
  RectifyDismissCommand,
  DrawElementCommand,
  DrawColorCommand,
  DrawStippleCommand,
  DrawWidthCommand,
  DrawUndoCommand,
  DrawHelpCommand,
  DrawDismissCommand,
  MatteEditMethod,
  MatteEditBorderCommand,
  MatteEditFuzzCommand,
  MatteEditValueCommand,
  MatteEditUndoCommand,
  MatteEditHelpCommand,
  MatteEditDismissCommand,
  PasteOperatorsCommand,
  PasteHelpCommand,
  PasteDismissCommand,
  RotateColorCommand,
  RotateDirectionCommand,
  RotateCropCommand,
  RotateSharpenCommand,
  RotateHelpCommand,
  RotateDismissCommand,
  HorizontalRotateCommand,
  VerticalRotateCommand,
  TileLoadCommand,
  TileNextCommand,
  TileFormerCommand,
  TileDeleteCommand,
  TileUpdateCommand
} ModeType;

/*
  Stipples.
*/
#define BricksWidth  20
#define BricksHeight  20
#define DiagonalWidth  16
#define DiagonalHeight  16
#define HighlightWidth  8
#define HighlightHeight  8
#define OpaqueWidth  8
#define OpaqueHeight  8
#define ScalesWidth  16
#define ScalesHeight  16
#define ShadowWidth  8
#define ShadowHeight  8
#define VerticalWidth  16
#define VerticalHeight  16
#define WavyWidth  16
#define WavyHeight  16

/*
  Constant declaration.
*/
static const int
  RoiDelta = 8;

static const unsigned char
  BricksBitmap[] =
  {
    0xff, 0xff, 0x0f, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00,
    0x03, 0x0c, 0x00, 0xff, 0xff, 0x0f, 0x60, 0x80, 0x01, 0x60, 0x80, 0x01,
    0x60, 0x80, 0x01, 0x60, 0x80, 0x01, 0xff, 0xff, 0x0f, 0x03, 0x0c, 0x00,
    0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0xff, 0xff, 0x0f,
    0x60, 0x80, 0x01, 0x60, 0x80, 0x01, 0x60, 0x80, 0x01, 0x60, 0x80, 0x01
  },
  DiagonalBitmap[] =
  {
    0x44, 0x44, 0x88, 0x88, 0x11, 0x11, 0x22, 0x22, 0x44, 0x44, 0x88, 0x88,
    0x11, 0x11, 0x22, 0x22, 0x44, 0x44, 0x88, 0x88, 0x11, 0x11, 0x22, 0x22,
    0x44, 0x44, 0x88, 0x88, 0x11, 0x11, 0x22, 0x22
  },
  ScalesBitmap[] =
  {
    0x08, 0x08, 0x08, 0x08, 0x14, 0x14, 0xe3, 0xe3, 0x80, 0x80, 0x80, 0x80,
    0x41, 0x41, 0x3e, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x14, 0x14, 0xe3, 0xe3,
    0x80, 0x80, 0x80, 0x80, 0x41, 0x41, 0x3e, 0x3e
  },
  VerticalBitmap[] =
  {
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
  },
  WavyBitmap[] =
  {
    0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfd, 0xff, 0xfd, 0xff, 0xfb, 0xff,
    0xe7, 0xff, 0x1f, 0xff, 0xff, 0xf8, 0xff, 0xe7, 0xff, 0xdf, 0xff, 0xbf,
    0xff, 0xbf, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f
  };

/*
  Function prototypes.
*/
static CommandType
  XImageWindowCommand(Display *,XResourceInfo *,XWindows *,
    const MagickStatusType,KeySym,Image **,ExceptionInfo *);

static Image
  *XMagickCommand(Display *,XResourceInfo *,XWindows *,const CommandType,
    Image **,ExceptionInfo *),
  *XOpenImage(Display *,XResourceInfo *,XWindows *,const MagickBooleanType),
  *XTileImage(Display *,XResourceInfo *,XWindows *,Image *,XEvent *,
    ExceptionInfo *),
  *XVisualDirectoryImage(Display *,XResourceInfo *,XWindows *,
    ExceptionInfo *);

static MagickBooleanType
  XAnnotateEditImage(Display *,XResourceInfo *,XWindows *,Image *,
    ExceptionInfo *),
  XBackgroundImage(Display *,XResourceInfo *,XWindows *,Image **,
    ExceptionInfo *),
  XChopImage(Display *,XResourceInfo *,XWindows *,Image **,
    ExceptionInfo *),
  XCropImage(Display *,XResourceInfo *,XWindows *,Image *,const ClipboardMode,
    ExceptionInfo *),
  XColorEditImage(Display *,XResourceInfo *,XWindows *,Image **,
    ExceptionInfo *),
  XCompositeImage(Display *,XResourceInfo *,XWindows *,Image *,
    ExceptionInfo *),
  XConfigureImage(Display *,XResourceInfo *,XWindows *,Image *,ExceptionInfo *),
  XDrawEditImage(Display *,XResourceInfo *,XWindows *,Image **,
    ExceptionInfo *),
  XMatteEditImage(Display *,XResourceInfo *,XWindows *,Image **,
    ExceptionInfo *),
  XPasteImage(Display *,XResourceInfo *,XWindows *,Image *,ExceptionInfo *),
  XPrintImage(Display *,XResourceInfo *,XWindows *,Image *,ExceptionInfo *),
  XRotateImage(Display *,XResourceInfo *,XWindows *,double,Image **,
    ExceptionInfo *),
  XROIImage(Display *,XResourceInfo *,XWindows *,Image **,ExceptionInfo *),
  XSaveImage(Display *,XResourceInfo *,XWindows *,Image *,ExceptionInfo *),
  XTrimImage(Display *,XResourceInfo *,XWindows *,Image *,ExceptionInfo *);

static void
  XDrawPanRectangle(Display *,XWindows *),
  XImageCache(Display *,XResourceInfo *,XWindows *,const CommandType,Image **,
    ExceptionInfo *),
  XMagnifyImage(Display *,XWindows *,XEvent *,ExceptionInfo *),
  XMakePanImage(Display *,XResourceInfo *,XWindows *,Image *,ExceptionInfo *),
  XPanImage(Display *,XWindows *,XEvent *,ExceptionInfo *),
  XMagnifyWindowCommand(Display *,XWindows *,const MagickStatusType,
    const KeySym,ExceptionInfo *),
  XSetCropGeometry(Display *,XWindows *,RectangleInfo *,Image *),
  XScreenEvent(Display *,XWindows *,XEvent *,ExceptionInfo *),
  XTranslateImage(Display *,XWindows *,Image *,const KeySym);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D i s p l a y I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DisplayImages() displays an image sequence to any X window screen.  It
%  returns a value other than 0 if successful.  Check the exception member
%  of image to determine the reason for any failure.
%
%  The format of the DisplayImages method is:
%
%      MagickBooleanType DisplayImages(const ImageInfo *image_info,
%        Image *images,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType DisplayImages(const ImageInfo *image_info,
  Image *images,ExceptionInfo *exception)
{
  char
    *argv[1];

  Display
    *display;

  Image
    *image;

  register ssize_t
    i;

  size_t
    state;

  XrmDatabase
    resource_database;

  XResourceInfo
    resource_info;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (images->debug != MagickFalse )
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),XServerError,
        "UnableToOpenXServer","`%s'",XDisplayName(image_info->server_name));
      return(MagickFalse);
    }
  if (exception->severity != UndefinedException)
    CatchException(exception);
  (void) XSetErrorHandler(XError);
  resource_database=XGetResourceDatabase(display,GetClientName());
  (void) ResetMagickMemory(&resource_info,0,sizeof(resource_info));
  XGetResourceInfo(image_info,resource_database,GetClientName(),&resource_info);
  if (image_info->page != (char *) NULL)
    resource_info.image_geometry=AcquireString(image_info->page);
  resource_info.immutable=MagickTrue;
  argv[0]=AcquireString(GetClientName());
  state=DefaultState;
  for (i=0; (state & ExitState) == 0; i++)
  {
    if ((images->iterations != 0) && (i >= (ssize_t) images->iterations))
      break;
    image=GetImageFromList(images,i % GetImageListLength(images));
    (void) XDisplayImage(display,&resource_info,argv,1,&image,&state,exception);
  }
  (void) SetErrorHandler((ErrorHandler) NULL);
  (void) SetWarningHandler((WarningHandler) NULL);
  argv[0]=DestroyString(argv[0]);
  (void) XCloseDisplay(display);
  XDestroyResourceInfo(&resource_info);
  if (exception->severity != UndefinedException)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o t e D i s p l a y C o m m a n d                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoteDisplayCommand() encourages a remote display program to display the
%  specified image filename.
%
%  The format of the RemoteDisplayCommand method is:
%
%      MagickBooleanType RemoteDisplayCommand(const ImageInfo *image_info,
%        const char *window,const char *filename,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o window: Specifies the name or id of an X window.
%
%    o filename: the name of the image filename to display.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType RemoteDisplayCommand(const ImageInfo *image_info,
  const char *window,const char *filename,ExceptionInfo *exception)
{
  Display
    *display;

  MagickStatusType
    status;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(filename != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),XServerError,
        "UnableToOpenXServer","`%s'",XDisplayName(image_info->server_name));
      return(MagickFalse);
    }
  (void) XSetErrorHandler(XError);
  status=XRemoteCommand(display,window,filename);
  (void) XCloseDisplay(display);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X A n n o t a t e E d i t I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XAnnotateEditImage() annotates the image with text.
%
%  The format of the XAnnotateEditImage method is:
%
%      MagickBooleanType XAnnotateEditImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
*/

static MagickBooleanType XAnnotateEditImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  static const char
    *AnnotateMenu[] =
    {
      "Font Name",
      "Font Color",
      "Box Color",
      "Rotate Text",
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *TextMenu[] =
    {
      "Help",
      "Apply",
      (char *) NULL
    };

  static const ModeType
    AnnotateCommands[] =
    {
      AnnotateNameCommand,
      AnnotateFontColorCommand,
      AnnotateBackgroundColorCommand,
      AnnotateRotateCommand,
      AnnotateHelpCommand,
      AnnotateDismissCommand
    },
    TextCommands[] =
    {
      TextHelpCommand,
      TextApplyCommand
    };

  static MagickBooleanType
    transparent_box = MagickTrue,
    transparent_pen = MagickFalse;

  static double
    degrees = 0.0;

  static unsigned int
    box_id = MaxNumberPens-2,
    font_id = 0,
    pen_id = 0;

  char
    command[MagickPathExtent],
    text[MagickPathExtent];

  const char
    *ColorMenu[MaxNumberPens+1];

  Cursor
    cursor;

  GC
    annotate_context;

  int
    id,
    pen_number,
    status,
    x,
    y;

  KeySym
    key_symbol;

  register char
    *p;

  register ssize_t
    i;

  unsigned int
    height,
    width;

  size_t
    state;

  XAnnotateInfo
    *annotate_info,
    *previous_info;

  XColor
    color;

  XFontStruct
    *font_info;

  XEvent
    event,
    text_event;

  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"Annotate");
  windows->command.data=4;
  (void) XCommandWidget(display,windows,AnnotateMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  cursor=XCreateFontCursor(display,XC_left_side);
  (void) XCheckDefineCursor(display,windows->image.id,cursor);
  state=DefaultState;
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+d%+d ",
          x+windows->image.x,y+windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,AnnotateMenu,&event);
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        if (id < 0)
          continue;
        switch (AnnotateCommands[id])
        {
          case AnnotateNameCommand:
          {
            const char
              *FontMenu[MaxNumberFonts];

            int
              font_number;

            /*
              Initialize menu selections.
            */
            for (i=0; i < MaxNumberFonts; i++)
              FontMenu[i]=resource_info->font_name[i];
            FontMenu[MaxNumberFonts-2]="Browser...";
            FontMenu[MaxNumberFonts-1]=(const char *) NULL;
            /*
              Select a font name from the pop-up menu.
            */
            font_number=XMenuWidget(display,windows,AnnotateMenu[id],
              (const char **) FontMenu,command);
            if (font_number < 0)
              break;
            if (font_number == (MaxNumberFonts-2))
              {
                static char
                  font_name[MagickPathExtent] = "fixed";

                /*
                  Select a font name from a browser.
                */
                resource_info->font_name[font_number]=font_name;
                XFontBrowserWidget(display,windows,"Select",font_name);
                if (*font_name == '\0')
                  break;
              }
            /*
              Initialize font info.
            */
            font_info=XLoadQueryFont(display,resource_info->font_name[
              font_number]);
            if (font_info == (XFontStruct *) NULL)
              {
                XNoticeWidget(display,windows,"Unable to load font:",
                  resource_info->font_name[font_number]);
                break;
              }
            font_id=(unsigned int) font_number;
            (void) XFreeFont(display,font_info);
            break;
          }
          case AnnotateFontColorCommand:
          {
            /*
              Initialize menu selections.
            */
            for (i=0; i < (int) (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="transparent";
            ColorMenu[MaxNumberPens-1]="Browser...";
            ColorMenu[MaxNumberPens]=(const char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,AnnotateMenu[id],
              (const char **) ColorMenu,command);
            if (pen_number < 0)
              break;
            transparent_pen=pen_number == (MaxNumberPens-2) ? MagickTrue :
              MagickFalse;
            if (transparent_pen != MagickFalse )
              break;
            if (pen_number == (MaxNumberPens-1))
              {
                static char
                  color_name[MagickPathExtent] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set pen color.
            */
            (void) XParseColor(display,windows->map_info->colormap,
              resource_info->pen_colors[pen_number],&color);
            XBestPixel(display,windows->map_info->colormap,(XColor *) NULL,
              (unsigned int) MaxColors,&color);
            windows->pixel_info->pen_colors[pen_number]=color;
            pen_id=(unsigned int) pen_number;
            break;
          }
          case AnnotateBackgroundColorCommand:
          {
            /*
              Initialize menu selections.
            */
            for (i=0; i < (int) (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="transparent";
            ColorMenu[MaxNumberPens-1]="Browser...";
            ColorMenu[MaxNumberPens]=(const char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,AnnotateMenu[id],
              (const char **) ColorMenu,command);
            if (pen_number < 0)
              break;
            transparent_box=pen_number == (MaxNumberPens-2) ? MagickTrue :
              MagickFalse;
            if (transparent_box != MagickFalse )
              break;
            if (pen_number == (MaxNumberPens-1))
              {
                static char
                  color_name[MagickPathExtent] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set pen color.
            */
            (void) XParseColor(display,windows->map_info->colormap,
              resource_info->pen_colors[pen_number],&color);
            XBestPixel(display,windows->map_info->colormap,(XColor *) NULL,
              (unsigned int) MaxColors,&color);
            windows->pixel_info->pen_colors[pen_number]=color;
            box_id=(unsigned int) pen_number;
            break;
          }
          case AnnotateRotateCommand:
          {
            int
              entry;

            static char
              angle[MagickPathExtent] = "30.0";

            static const char
              *RotateMenu[] =
              {
                "-90",
                "-45",
                "-30",
                "0",
                "30",
                "45",
                "90",
                "180",
                "Dialog...",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            entry=XMenuWidget(display,windows,AnnotateMenu[id],RotateMenu,
              command);
            if (entry < 0)
              break;
            if (entry != 8)
              {
                degrees=StringToDouble(RotateMenu[entry],(char **) NULL);
                break;
              }
            (void) XDialogWidget(display,windows,"OK","Enter rotation angle:",
              angle);
            if (*angle == '\0')
              break;
            degrees=StringToDouble(angle,(char **) NULL);
            break;
          }
          case AnnotateHelpCommand:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Annotation",ImageAnnotateHelp);
            break;
          }
          case AnnotateDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        /*
          Change to text entering mode.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        state|=ExitState;
        break;
      }
      case ButtonRelease:
        break;
      case Expose:
        break;
      case KeyPress:
      {
        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Annotation",ImageAnnotateHelp);
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Map and unmap Info widget as cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
        break;
      }
      default:
        break;
    }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if ((state & EscapeState) != 0)
    return(MagickTrue);
  /*
    Set font info and check boundary conditions.
  */
  font_info=XLoadQueryFont(display,resource_info->font_name[font_id]);
  if (font_info == (XFontStruct *) NULL)
    {
      XNoticeWidget(display,windows,"Unable to load font:",
        resource_info->font_name[font_id]);
      font_info=windows->font_info;
    }
  if ((x+font_info->max_bounds.width) >= (int) windows->image.width)
    x=(int) windows->image.width-font_info->max_bounds.width;
  if (y < (int) (font_info->ascent+font_info->descent))
    y=(int) font_info->ascent+font_info->descent;
  if (((int) font_info->max_bounds.width > (int) windows->image.width) ||
      ((font_info->ascent+font_info->descent) >= (int) windows->image.height))
    return(MagickFalse);
  /*
    Initialize annotate structure.
  */
  annotate_info=(XAnnotateInfo *) AcquireMagickMemory(sizeof(*annotate_info));
  if (annotate_info == (XAnnotateInfo *) NULL)
    return(MagickFalse);
  XGetAnnotateInfo(annotate_info);
  annotate_info->x=x;
  annotate_info->y=y;
  if ((transparent_box == MagickFalse) && (transparent_pen == MagickFalse))
    annotate_info->stencil=OpaqueStencil;
  else
    if (transparent_box == MagickFalse)
      annotate_info->stencil=BackgroundStencil;
    else
      annotate_info->stencil=ForegroundStencil;
  annotate_info->height=(unsigned int) font_info->ascent+font_info->descent;
  annotate_info->degrees=degrees;
  annotate_info->font_info=font_info;
  annotate_info->text=(char *) AcquireQuantumMemory((size_t)
    windows->image.width/MagickMax((ssize_t) font_info->min_bounds.width,1)+2UL,
    sizeof(*annotate_info->text));
  if (annotate_info->text == (char *) NULL)
    return(MagickFalse);
  /*
    Create cursor and set graphic context.
  */
  cursor=XCreateFontCursor(display,XC_pencil);
  (void) XCheckDefineCursor(display,windows->image.id,cursor);
  annotate_context=windows->image.annotate_context;
  (void) XSetFont(display,annotate_context,font_info->fid);
  (void) XSetBackground(display,annotate_context,
    windows->pixel_info->pen_colors[box_id].pixel);
  (void) XSetForeground(display,annotate_context,
    windows->pixel_info->pen_colors[pen_id].pixel);
  /*
    Begin annotating the image with text.
  */
  (void) CloneString(&windows->command.name,"Text");
  windows->command.data=0;
  (void) XCommandWidget(display,windows,TextMenu,(XEvent *) NULL);
  state=DefaultState;
  (void) XDrawString(display,windows->image.id,annotate_context,x,y,"_",1);
  text_event.xexpose.width=(int) font_info->max_bounds.width;
  text_event.xexpose.height=font_info->max_bounds.ascent+
    font_info->max_bounds.descent;
  p=annotate_info->text;
  do
  {
    /*
      Display text cursor.
    */
    *p='\0';
    (void) XDrawString(display,windows->image.id,annotate_context,x,y,"_",1);
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        (void) XSetBackground(display,annotate_context,
          windows->pixel_info->background_color.pixel);
        (void) XSetForeground(display,annotate_context,
          windows->pixel_info->foreground_color.pixel);
        id=XCommandWidget(display,windows,AnnotateMenu,&event);
        (void) XSetBackground(display,annotate_context,
          windows->pixel_info->pen_colors[box_id].pixel);
        (void) XSetForeground(display,annotate_context,
          windows->pixel_info->pen_colors[pen_id].pixel);
        if (id < 0)
          continue;
        switch (TextCommands[id])
        {
          case TextHelpCommand:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Annotation",ImageAnnotateHelp);
            (void) XCheckDefineCursor(display,windows->image.id,cursor);
            break;
          }
          case TextApplyCommand:
          {
            /*
              Finished annotating.
            */
            annotate_info->width=(unsigned int) XTextWidth(font_info,
              annotate_info->text,(int) strlen(annotate_info->text));
            XRefreshWindow(display,&windows->image,&text_event);
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    /*
      Erase text cursor.
    */
    text_event.xexpose.x=x;
    text_event.xexpose.y=y-font_info->max_bounds.ascent;
    (void) XClearArea(display,windows->image.id,x,text_event.xexpose.y,
      (unsigned int) text_event.xexpose.width,(unsigned int)
      text_event.xexpose.height,MagickFalse);
    XRefreshWindow(display,&windows->image,&text_event);
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window != windows->image.id)
          break;
        if (event.xbutton.button == Button2)
          {
            /*
              Request primary selection.
            */
            (void) XConvertSelection(display,XA_PRIMARY,XA_STRING,XA_STRING,
              windows->image.id,CurrentTime);
            break;
          }
        break;
      }
      case Expose:
      {
        if (event.xexpose.count == 0)
          {
            XAnnotateInfo
              *text_info;

            /*
              Refresh Image window.
            */
            XRefreshWindow(display,&windows->image,(XEvent *) NULL);
            text_info=annotate_info;
            while (text_info != (XAnnotateInfo *) NULL)
            {
              if (annotate_info->stencil == ForegroundStencil)
                (void) XDrawString(display,windows->image.id,annotate_context,
                  text_info->x,text_info->y,text_info->text,
                  (int) strlen(text_info->text));
              else
                (void) XDrawImageString(display,windows->image.id,
                  annotate_context,text_info->x,text_info->y,text_info->text,
                  (int) strlen(text_info->text));
              text_info=text_info->previous;
            }
            (void) XDrawString(display,windows->image.id,annotate_context,
              x,y,"_",1);
          }
        break;
      }
      case KeyPress:
      {
        int
          length;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if (((event.xkey.state & ControlMask) != 0) ||
            ((event.xkey.state & Mod1Mask) != 0))
          state|=ModifierState;
        if ((state & ModifierState) != 0)
          switch ((int) key_symbol)
          {
            case XK_u:
            case XK_U:
            {
              key_symbol=DeleteCommand;
              break;
            }
            default:
              break;
          }
        switch ((int) key_symbol)
        {
          case XK_BackSpace:
          {
            /*
              Erase one character.
            */
            if (p == annotate_info->text)
              {
                if (annotate_info->previous == (XAnnotateInfo *) NULL)
                  break;
                else
                  {
                    /*
                      Go to end of the previous line of text.
                    */
                    annotate_info=annotate_info->previous;
                    p=annotate_info->text;
                    x=annotate_info->x+annotate_info->width;
                    y=annotate_info->y;
                    if (annotate_info->width != 0)
                      p+=strlen(annotate_info->text);
                    break;
                  }
              }
            p--;
            x-=XTextWidth(font_info,p,1);
            text_event.xexpose.x=x;
            text_event.xexpose.y=y-font_info->max_bounds.ascent;
            XRefreshWindow(display,&windows->image,&text_event);
            break;
          }
          case XK_bracketleft:
          {
            key_symbol=XK_Escape;
            break;
          }
          case DeleteCommand:
          {
            /*
              Erase the entire line of text.
            */
            while (p != annotate_info->text)
            {
              p--;
              x-=XTextWidth(font_info,p,1);
              text_event.xexpose.x=x;
              XRefreshWindow(display,&windows->image,&text_event);
            }
            break;
          }
          case XK_Escape:
          case XK_F20:
          {
            /*
              Finished annotating.
            */
            annotate_info->width=(unsigned int) XTextWidth(font_info,
              annotate_info->text,(int) strlen(annotate_info->text));
            XRefreshWindow(display,&windows->image,&text_event);
            state|=ExitState;
            break;
          }
          default:
          {
            /*
              Draw a single character on the Image window.
            */
            if ((state & ModifierState) != 0)
              break;
            if (*command == '\0')
              break;
            *p=(*command);
            if (annotate_info->stencil == ForegroundStencil)
              (void) XDrawString(display,windows->image.id,annotate_context,
                x,y,p,1);
            else
              (void) XDrawImageString(display,windows->image.id,
                annotate_context,x,y,p,1);
            x+=XTextWidth(font_info,p,1);
            p++;
            if ((x+font_info->max_bounds.width) < (int) windows->image.width)
              break;
          }
          case XK_Return:
          case XK_KP_Enter:
          {
            /*
              Advance to the next line of text.
            */
            *p='\0';
            annotate_info->width=(unsigned int) XTextWidth(font_info,
              annotate_info->text,(int) strlen(annotate_info->text));
            if (annotate_info->next != (XAnnotateInfo *) NULL)
              {
                /*
                  Line of text already exists.
                */
                annotate_info=annotate_info->next;
                x=annotate_info->x;
                y=annotate_info->y;
                p=annotate_info->text;
                break;
              }
            annotate_info->next=(XAnnotateInfo *) AcquireMagickMemory(
              sizeof(*annotate_info->next));
            if (annotate_info->next == (XAnnotateInfo *) NULL)
              return(MagickFalse);
            *annotate_info->next=(*annotate_info);
            annotate_info->next->previous=annotate_info;
            annotate_info=annotate_info->next;
            annotate_info->text=(char *) AcquireQuantumMemory((size_t)
              windows->image.width/MagickMax((ssize_t)
              font_info->min_bounds.width,1)+2UL,sizeof(*annotate_info->text));
            if (annotate_info->text == (char *) NULL)
              return(MagickFalse);
            annotate_info->y+=annotate_info->height;
            if (annotate_info->y > (int) windows->image.height)
              annotate_info->y=(int) annotate_info->height;
            annotate_info->next=(XAnnotateInfo *) NULL;
            x=annotate_info->x;
            y=annotate_info->y;
            p=annotate_info->text;
            break;
          }
        }
        break;
      }
      case KeyRelease:
      {
        /*
          Respond to a user key release.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        state&=(~ModifierState);
        break;
      }
      case SelectionNotify:
      {
        Atom
          type;

        int
          format;

        unsigned char
          *data;

        unsigned long
          after,
          length;

        /*
          Obtain response from primary selection.
        */
        if (event.xselection.property == (Atom) None)
          break;
        status=XGetWindowProperty(display,event.xselection.requestor,
          event.xselection.property,0L,(long) MagickPathExtent,True,XA_STRING,
          &type,&format,&length,&after,&data);
        if ((status != Success) || (type != XA_STRING) || (format == 32) ||
            (length == 0))
          break;
        /*
          Annotate Image window with primary selection.
        */
        for (i=0; i < (ssize_t) length; i++)
        {
          if ((char) data[i] != '\n')
            {
              /*
                Draw a single character on the Image window.
              */
              *p=(char) data[i];
              (void) XDrawString(display,windows->image.id,annotate_context,
                x,y,p,1);
              x+=XTextWidth(font_info,p,1);
              p++;
              if ((x+font_info->max_bounds.width) < (int) windows->image.width)
                continue;
            }
          /*
            Advance to the next line of text.
          */
          *p='\0';
          annotate_info->width=(unsigned int) XTextWidth(font_info,
            annotate_info->text,(int) strlen(annotate_info->text));
          if (annotate_info->next != (XAnnotateInfo *) NULL)
            {
              /*
                Line of text already exists.
              */
              annotate_info=annotate_info->next;
              x=annotate_info->x;
              y=annotate_info->y;
              p=annotate_info->text;
              continue;
            }
          annotate_info->next=(XAnnotateInfo *) AcquireMagickMemory(
            sizeof(*annotate_info->next));
          if (annotate_info->next == (XAnnotateInfo *) NULL)
            return(MagickFalse);
          *annotate_info->next=(*annotate_info);
          annotate_info->next->previous=annotate_info;
          annotate_info=annotate_info->next;
          annotate_info->text=(char *) AcquireQuantumMemory((size_t)
            windows->image.width/MagickMax((ssize_t)
            font_info->min_bounds.width,1)+2UL,sizeof(*annotate_info->text));
          if (annotate_info->text == (char *) NULL)
            return(MagickFalse);
          annotate_info->y+=annotate_info->height;
          if (annotate_info->y > (int) windows->image.height)
            annotate_info->y=(int) annotate_info->height;
          annotate_info->next=(XAnnotateInfo *) NULL;
          x=annotate_info->x;
          y=annotate_info->y;
          p=annotate_info->text;
        }
        (void) XFree((void *) data);
        break;
      }
      default:
        break;
    }
  } while ((state & ExitState) == 0);
  (void) XFreeCursor(display,cursor);
  /*
    Annotation is relative to image configuration.
  */
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  x=0;
  y=0;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  /*
    Initialize annotated image.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  while (annotate_info != (XAnnotateInfo *) NULL)
  {
    if (annotate_info->width == 0)
      {
        /*
          No text on this line--  go to the next line of text.
        */
        previous_info=annotate_info->previous;
        annotate_info->text=(char *)
          RelinquishMagickMemory(annotate_info->text);
        annotate_info=(XAnnotateInfo *) RelinquishMagickMemory(annotate_info);
        annotate_info=previous_info;
        continue;
      }
    /*
      Determine pixel index for box and pen color.
    */
    windows->pixel_info->box_color=windows->pixel_info->pen_colors[box_id];
    if (windows->pixel_info->colors != 0)
      for (i=0; i < (ssize_t) windows->pixel_info->colors; i++)
        if (windows->pixel_info->pixels[i] ==
            windows->pixel_info->pen_colors[box_id].pixel)
          {
            windows->pixel_info->box_index=(unsigned short) i;
            break;
          }
    windows->pixel_info->pen_color=windows->pixel_info->pen_colors[pen_id];
    if (windows->pixel_info->colors != 0)
      for (i=0; i < (ssize_t) windows->pixel_info->colors; i++)
        if (windows->pixel_info->pixels[i] ==
            windows->pixel_info->pen_colors[pen_id].pixel)
          {
            windows->pixel_info->pen_index=(unsigned short) i;
            break;
          }
    /*
      Define the annotate geometry string.
    */
    annotate_info->x=(int)
      width*(annotate_info->x+windows->image.x)/windows->image.ximage->width;
    annotate_info->y=(int) height*(annotate_info->y-font_info->ascent+
      windows->image.y)/windows->image.ximage->height;
    (void) FormatLocaleString(annotate_info->geometry,MagickPathExtent,
      "%ux%u%+d%+d",width*annotate_info->width/windows->image.ximage->width,
      height*annotate_info->height/windows->image.ximage->height,
      annotate_info->x+x,annotate_info->y+y);
    /*
      Annotate image with text.
    */
    status=XAnnotateImage(display,windows->pixel_info,annotate_info,image,
      exception);
    if (status == 0)
      return(MagickFalse);
    /*
      Free up memory.
    */
    previous_info=annotate_info->previous;
    annotate_info->text=DestroyString(annotate_info->text);
    annotate_info=(XAnnotateInfo *) RelinquishMagickMemory(annotate_info);
    annotate_info=previous_info;
  }
  (void) XSetForeground(display,annotate_context,
    windows->pixel_info->foreground_color.pixel);
  (void) XSetBackground(display,annotate_context,
    windows->pixel_info->background_color.pixel);
  (void) XSetFont(display,annotate_context,windows->font_info->fid);
  XSetCursorState(display,windows,MagickFalse);
  (void) XFreeFont(display,font_info);
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,image,exception);
  (void) XConfigureImage(display,resource_info,windows,image,exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X B a c k g r o u n d I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XBackgroundImage() displays the image in the background of a window.
%
%  The format of the XBackgroundImage method is:
%
%      MagickBooleanType XBackgroundImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image **image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XBackgroundImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image **image,
  ExceptionInfo *exception)
{
#define BackgroundImageTag  "Background/Image"

  int
    status;

  static char
    window_id[MagickPathExtent] = "root";

  XResourceInfo
    background_resources;

  /*
    Put image in background.
  */
  status=XDialogWidget(display,windows,"Background",
    "Enter window id (id 0x00 selects window with pointer):",window_id);
  if (*window_id == '\0')
    return(MagickFalse);
  (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
    exception);
  XInfoWidget(display,windows,BackgroundImageTag);
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  background_resources=(*resource_info);
  background_resources.window_id=window_id;
  background_resources.backdrop=status != 0 ? MagickTrue : MagickFalse;
  status=XDisplayBackgroundImage(display,&background_resources,*image,
    exception);
  if (status != MagickFalse)
    XClientMessage(display,windows->image.id,windows->im_protocols,
      windows->im_retain_colors,CurrentTime);
  XSetCursorState(display,windows,MagickFalse);
  (void) XMagickCommand(display,resource_info,windows,UndoCommand,image,
    exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C h o p I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XChopImage() chops the X image.
%
%  The format of the XChopImage method is:
%
%    MagickBooleanType XChopImage(Display *display,XResourceInfo *resource_info,
%      XWindows *windows,Image **image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XChopImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image **image,
  ExceptionInfo *exception)
{
  static const char
    *ChopMenu[] =
    {
      "Direction",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static ModeType
    direction = HorizontalChopCommand;

  static const ModeType
    ChopCommands[] =
    {
      ChopDirectionCommand,
      ChopHelpCommand,
      ChopDismissCommand
    },
    DirectionCommands[] =
    {
      HorizontalChopCommand,
      VerticalChopCommand
    };

  char
    text[MagickPathExtent];

  Image
    *chop_image;

  int
    id,
    x,
    y;

  double
    scale_factor;

  RectangleInfo
    chop_info;

  unsigned int
    distance,
    height,
    width;

  size_t
    state;

  XEvent
    event;

  XSegment
    segment_info;

  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"Chop");
  windows->command.data=1;
  (void) XCommandWidget(display,windows,ChopMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  state=DefaultState;
  (void) ResetMagickMemory(&segment_info,0,sizeof(segment_info));
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+d%+d ",
          x+windows->image.x,y+windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,ChopMenu,&event);
        if (id < 0)
          continue;
        switch (ChopCommands[id])
        {
          case ChopDirectionCommand:
          {
            char
              command[MagickPathExtent];

            static const char
              *Directions[] =
              {
                "horizontal",
                "vertical",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            id=XMenuWidget(display,windows,ChopMenu[id],Directions,command);
            if (id >= 0)
              direction=DirectionCommands[id];
            break;
          }
          case ChopHelpCommand:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Chop",ImageChopHelp);
            break;
          }
          case ChopDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        /*
          User has committed to start point of chopping line.
        */
        segment_info.x1=(short int) event.xbutton.x;
        segment_info.x2=(short int) event.xbutton.x;
        segment_info.y1=(short int) event.xbutton.y;
        segment_info.y2=(short int) event.xbutton.y;
        state|=ExitState;
        break;
      }
      case ButtonRelease:
        break;
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MagickPathExtent];

        KeySym
          key_symbol;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Chop",ImageChopHelp);
            (void) XSetFunction(display,windows->image.highlight_context,
              GXinvert);
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
      }
    }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if ((state & EscapeState) != 0)
    return(MagickTrue);
  /*
    Draw line as pointer moves until the mouse button is released.
  */
  chop_info.width=0;
  chop_info.height=0;
  chop_info.x=0;
  chop_info.y=0;
  distance=0;
  (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
  state=DefaultState;
  do
  {
    if (distance > 9)
      {
        /*
          Display info and draw chopping line.
        */
        if (windows->info.mapped == MagickFalse)
          (void) XMapWindow(display,windows->info.id);
        (void) FormatLocaleString(text,MagickPathExtent,
          " %.20gx%.20g%+.20g%+.20g",(double) chop_info.width,(double)
          chop_info.height,(double) chop_info.x,(double) chop_info.y);
        XInfoWidget(display,windows,text);
        XHighlightLine(display,windows->image.id,
          windows->image.highlight_context,&segment_info);
      }
    else
      if (windows->info.mapped != MagickFalse )
        (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (distance > 9)
      XHighlightLine(display,windows->image.id,
        windows->image.highlight_context,&segment_info);
    switch (event.type)
    {
      case ButtonPress:
      {
        segment_info.x2=(short int) event.xmotion.x;
        segment_info.y2=(short int) event.xmotion.y;
        break;
      }
      case ButtonRelease:
      {
        /*
          User has committed to chopping line.
        */
        segment_info.x2=(short int) event.xbutton.x;
        segment_info.y2=(short int) event.xbutton.y;
        state|=ExitState;
        break;
      }
      case Expose:
        break;
      case MotionNotify:
      {
        segment_info.x2=(short int) event.xmotion.x;
        segment_info.y2=(short int) event.xmotion.y;
      }
      default:
        break;
    }
    /*
      Check boundary conditions.
    */
    if (segment_info.x2 < 0)
      segment_info.x2=0;
    else
      if (segment_info.x2 > windows->image.ximage->width)
        segment_info.x2=windows->image.ximage->width;
    if (segment_info.y2 < 0)
      segment_info.y2=0;
    else
      if (segment_info.y2 > windows->image.ximage->height)
        segment_info.y2=windows->image.ximage->height;
    distance=(unsigned int)
      (((segment_info.x2-segment_info.x1)*(segment_info.x2-segment_info.x1))+
       ((segment_info.y2-segment_info.y1)*(segment_info.y2-segment_info.y1)));
    /*
      Compute chopping geometry.
    */
    if (direction == HorizontalChopCommand)
      {
        chop_info.width=(size_t) (segment_info.x2-segment_info.x1+1);
        chop_info.x=(ssize_t) windows->image.x+segment_info.x1;
        chop_info.height=0;
        chop_info.y=0;
        if (segment_info.x1 > (int) segment_info.x2)
          {
            chop_info.width=(size_t) (segment_info.x1-segment_info.x2+1);
            chop_info.x=(ssize_t) windows->image.x+segment_info.x2;
          }
      }
    else
      {
        chop_info.width=0;
        chop_info.height=(size_t) (segment_info.y2-segment_info.y1+1);
        chop_info.x=0;
        chop_info.y=(ssize_t) windows->image.y+segment_info.y1;
        if (segment_info.y1 > segment_info.y2)
          {
            chop_info.height=(size_t) (segment_info.y1-segment_info.y2+1);
            chop_info.y=(ssize_t) windows->image.y+segment_info.y2;
          }
      }
  } while ((state & ExitState) == 0);
  (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
  (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if (distance <= 9)
    return(MagickTrue);
  /*
    Image chopping is relative to image configuration.
  */
  (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
    exception);
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  windows->image.window_changes.width=windows->image.ximage->width-
    (unsigned int) chop_info.width;
  windows->image.window_changes.height=windows->image.ximage->height-
    (unsigned int) chop_info.height;
  width=(unsigned int) (*image)->columns;
  height=(unsigned int) (*image)->rows;
  x=0;
  y=0;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=(double) width/windows->image.ximage->width;
  chop_info.x+=x;
  chop_info.x=(ssize_t) (scale_factor*chop_info.x+0.5);
  chop_info.width=(unsigned int) (scale_factor*chop_info.width+0.5);
  scale_factor=(double) height/windows->image.ximage->height;
  chop_info.y+=y;
  chop_info.y=(ssize_t) (scale_factor*chop_info.y+0.5);
  chop_info.height=(unsigned int) (scale_factor*chop_info.height+0.5);
  /*
    Chop image.
  */
  chop_image=ChopImage(*image,&chop_info,exception);
  XSetCursorState(display,windows,MagickFalse);
  if (chop_image == (Image *) NULL)
    return(MagickFalse);
  *image=DestroyImage(*image);
  *image=chop_image;
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,*image,exception);
  (void) XConfigureImage(display,resource_info,windows,*image,exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C o l o r E d i t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XColorEditImage() allows the user to interactively change the color of one
%  pixel for a DirectColor image or one colormap entry for a PseudoClass image.
%
%  The format of the XColorEditImage method is:
%
%      MagickBooleanType XColorEditImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image **image,
%          ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XColorEditImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image **image,
  ExceptionInfo *exception)
{
  static const char
    *ColorEditMenu[] =
    {
      "Method",
      "Pixel Color",
      "Border Color",
      "Fuzz",
      "Undo",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static const ModeType
    ColorEditCommands[] =
    {
      ColorEditMethodCommand,
      ColorEditColorCommand,
      ColorEditBorderCommand,
      ColorEditFuzzCommand,
      ColorEditUndoCommand,
      ColorEditHelpCommand,
      ColorEditDismissCommand
    };

  static PaintMethod
    method = PointMethod;

  static unsigned int
    pen_id = 0;

  static XColor
    border_color = { 0, 0, 0, 0, 0, 0 };

  char
    command[MagickPathExtent],
    text[MagickPathExtent];

  Cursor
    cursor;

  int
    entry,
    id,
    x,
    x_offset,
    y,
    y_offset;

  register Quantum
    *q;

  register ssize_t
    i;

  unsigned int
    height,
    width;

  size_t
    state;

  XColor
    color;

  XEvent
    event;

  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"Color Edit");
  windows->command.data=4;
  (void) XCommandWidget(display,windows,ColorEditMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Make cursor.
  */
  cursor=XMakeCursor(display,windows->image.id,windows->map_info->colormap,
    resource_info->background_color,resource_info->foreground_color);
  (void) XCheckDefineCursor(display,windows->image.id,cursor);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+d%+d ",
          x+windows->image.x,y+windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,ColorEditMenu,&event);
        if (id < 0)
          {
            (void) XCheckDefineCursor(display,windows->image.id,cursor);
            continue;
          }
        switch (ColorEditCommands[id])
        {
          case ColorEditMethodCommand:
          {
            char
              **methods;

            /*
              Select a method from the pop-up menu.
            */
            methods=(char **) GetCommandOptions(MagickMethodOptions);
            if (methods == (char **) NULL)
              break;
            entry=XMenuWidget(display,windows,ColorEditMenu[id],
              (const char **) methods,command);
            if (entry >= 0)
              method=(PaintMethod) ParseCommandOption(MagickMethodOptions,
                MagickFalse,methods[entry]);
            methods=DestroyStringList(methods);
            break;
          }
          case ColorEditColorCommand:
          {
            const char
              *ColorMenu[MaxNumberPens];

            int
              pen_number;

            /*
              Initialize menu selections.
            */
            for (i=0; i < (int) (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="Browser...";
            ColorMenu[MaxNumberPens-1]=(const char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,ColorEditMenu[id],
              (const char **) ColorMenu,command);
            if (pen_number < 0)
              break;
            if (pen_number == (MaxNumberPens-2))
              {
                static char
                  color_name[MagickPathExtent] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set pen color.
            */
            (void) XParseColor(display,windows->map_info->colormap,
              resource_info->pen_colors[pen_number],&color);
            XBestPixel(display,windows->map_info->colormap,(XColor *) NULL,
              (unsigned int) MaxColors,&color);
            windows->pixel_info->pen_colors[pen_number]=color;
            pen_id=(unsigned int) pen_number;
            break;
          }
          case ColorEditBorderCommand:
          {
            const char
              *ColorMenu[MaxNumberPens];

            int
              pen_number;

            /*
              Initialize menu selections.
            */
            for (i=0; i < (int) (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="Browser...";
            ColorMenu[MaxNumberPens-1]=(const char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,ColorEditMenu[id],
              (const char **) ColorMenu,command);
            if (pen_number < 0)
              break;
            if (pen_number == (MaxNumberPens-2))
              {
                static char
                  color_name[MagickPathExtent] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set border color.
            */
            (void) XParseColor(display,windows->map_info->colormap,
              resource_info->pen_colors[pen_number],&border_color);
            break;
          }
          case ColorEditFuzzCommand:
          {
            static char
              fuzz[MagickPathExtent];

            static const char
              *FuzzMenu[] =
              {
                "0%",
                "2%",
                "5%",
                "10%",
                "15%",
                "Dialog...",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            entry=XMenuWidget(display,windows,ColorEditMenu[id],FuzzMenu,
              command);
            if (entry < 0)
              break;
            if (entry != 5)
              {
                (*image)->fuzz=StringToDoubleInterval(FuzzMenu[entry],(double)
                  QuantumRange+1.0);
                break;
              }
            (void) (void) CopyMagickString(fuzz,"20%",MagickPathExtent);
            (void) XDialogWidget(display,windows,"Ok",
              "Enter fuzz factor (0.0 - 99.9%):",fuzz);
            if (*fuzz == '\0')
              break;
            (void) ConcatenateMagickString(fuzz,"%",MagickPathExtent);
            (*image)->fuzz=StringToDoubleInterval(fuzz,(double) QuantumRange+
              1.0);
            break;
          }
          case ColorEditUndoCommand:
          {
            (void) XMagickCommand(display,resource_info,windows,UndoCommand,
              image,exception);
            break;
          }
          case ColorEditHelpCommand:
          default:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Annotation",ImageColorEditHelp);
            break;
          }
          case ColorEditDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
        }
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.button != Button1)
          break;
        if ((event.xbutton.window != windows->image.id) &&
            (event.xbutton.window != windows->magnify.id))
          break;
        /*
          exit loop.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        (void) XMagickCommand(display,resource_info,windows,
          SaveToUndoBufferCommand,image,exception);
        state|=UpdateConfigurationState;
        break;
      }
      case ButtonRelease:
      {
        if (event.xbutton.button != Button1)
          break;
        if ((event.xbutton.window != windows->image.id) &&
            (event.xbutton.window != windows->magnify.id))
          break;
        /*
          Update colormap information.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        XConfigureImageColormap(display,resource_info,windows,*image,exception);
        (void) XConfigureImage(display,resource_info,windows,*image,exception);
        XInfoWidget(display,windows,text);
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        state&=(~UpdateConfigurationState);
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        KeySym
          key_symbol;

        if (event.xkey.window == windows->magnify.id)
          {
            Window
              window;

            window=windows->magnify.id;
            while (XCheckWindowEvent(display,window,KeyPressMask,&event)) ;
          }
        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Annotation",ImageColorEditHelp);
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Map and unmap Info widget as cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
        break;
      }
      default:
        break;
    }
    if (event.xany.window == windows->magnify.id)
      {
        x=windows->magnify.x-windows->image.x;
        y=windows->magnify.y-windows->image.y;
      }
    x_offset=x;
    y_offset=y;
    if ((state & UpdateConfigurationState) != 0)
      {
        CacheView
          *image_view;

        int
          x,
          y;

        /*
          Pixel edit is relative to image configuration.
        */
        (void) XClearArea(display,windows->image.id,x_offset,y_offset,1,1,
          MagickTrue);
        color=windows->pixel_info->pen_colors[pen_id];
        XPutPixel(windows->image.ximage,x_offset,y_offset,color.pixel);
        width=(unsigned int) (*image)->columns;
        height=(unsigned int) (*image)->rows;
        x=0;
        y=0;
        if (windows->image.crop_geometry != (char *) NULL)
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
        x_offset=(int)
          (width*(windows->image.x+x_offset)/windows->image.ximage->width+x);
        y_offset=(int)
          (height*(windows->image.y+y_offset)/windows->image.ximage->height+y);
        if ((x_offset < 0) || (y_offset < 0))
          continue;
        if ((x_offset >= (int) (*image)->columns) ||
            (y_offset >= (int) (*image)->rows))
          continue;
        image_view=AcquireAuthenticCacheView(*image,exception);
        switch (method)
        {
          case PointMethod:
          default:
          {
            /*
              Update color information using point algorithm.
            */
            if (SetImageStorageClass(*image,DirectClass,exception) == MagickFalse)
              return(MagickFalse);
            q=GetCacheViewAuthenticPixels(image_view,(ssize_t)x_offset,
              (ssize_t) y_offset,1,1,exception);
            if (q == (Quantum *) NULL)
              break;
            SetPixelRed(*image,ScaleShortToQuantum(color.red),q);
            SetPixelGreen(*image,ScaleShortToQuantum(color.green),q);
            SetPixelBlue(*image,ScaleShortToQuantum(color.blue),q);
            (void) SyncCacheViewAuthenticPixels(image_view,exception);
            break;
          }
          case ReplaceMethod:
          {
            PixelInfo
              pixel,
              target;

            /*
              Update color information using replace algorithm.
            */
            (void) GetOneCacheViewVirtualPixelInfo(image_view,(ssize_t)
              x_offset,(ssize_t) y_offset,&target,exception);
            if ((*image)->storage_class == DirectClass)
              {
                for (y=0; y < (int) (*image)->rows; y++)
                {
                  q=GetCacheViewAuthenticPixels(image_view,0,(ssize_t) y,
                    (*image)->columns,1,exception);
                  if (q == (Quantum *) NULL)
                    break;
                  for (x=0; x < (int) (*image)->columns; x++)
                  {
                    GetPixelInfoPixel(*image,q,&pixel);
                    if (IsFuzzyEquivalencePixelInfo(&pixel,&target))
                      {
                        SetPixelRed(*image,ScaleShortToQuantum(
                          color.red),q);
                        SetPixelGreen(*image,ScaleShortToQuantum(
                          color.green),q);
                        SetPixelBlue(*image,ScaleShortToQuantum(
                          color.blue),q);
                      }
                    q+=GetPixelChannels(*image);
                  }
                  if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
                    break;
                }
              }
            else
              {
                for (i=0; i < (ssize_t) (*image)->colors; i++)
                  if (IsFuzzyEquivalencePixelInfo((*image)->colormap+i,&target))
                    {
                      (*image)->colormap[i].red=(double) ScaleShortToQuantum(
                        color.red);
                      (*image)->colormap[i].green=(double) ScaleShortToQuantum(
                        color.green);
                      (*image)->colormap[i].blue=(double) ScaleShortToQuantum(
                        color.blue);
                    }
                (void) SyncImage(*image,exception);
              }
            break;
          }
          case FloodfillMethod:
          case FillToBorderMethod:
          {
            DrawInfo
              *draw_info;

            PixelInfo
              target;

            /*
              Update color information using floodfill algorithm.
            */
            (void) GetOneVirtualPixelInfo(*image,
              GetPixelCacheVirtualMethod(*image),(ssize_t) x_offset,(ssize_t)
              y_offset,&target,exception);
            if (method == FillToBorderMethod)
              {
                target.red=(double)
                  ScaleShortToQuantum(border_color.red);
                target.green=(double)
                  ScaleShortToQuantum(border_color.green);
                target.blue=(double)
                  ScaleShortToQuantum(border_color.blue);
              }
            draw_info=CloneDrawInfo(resource_info->image_info,
              (DrawInfo *) NULL);
            (void) QueryColorCompliance(resource_info->pen_colors[pen_id],
              AllCompliance,&draw_info->fill,exception);
            (void) FloodfillPaintImage(*image,draw_info,&target,
              (ssize_t)x_offset,(ssize_t)y_offset,
              method != FloodfillMethod ? MagickTrue : MagickFalse,exception);
            draw_info=DestroyDrawInfo(draw_info);
            break;
          }
          case ResetMethod:
          {
            /*
              Update color information using reset algorithm.
            */
            if (SetImageStorageClass(*image,DirectClass,exception) == MagickFalse)
              return(MagickFalse);
            for (y=0; y < (int) (*image)->rows; y++)
            {
              q=QueueCacheViewAuthenticPixels(image_view,0,(ssize_t) y,
                (*image)->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (int) (*image)->columns; x++)
              {
                SetPixelRed(*image,ScaleShortToQuantum(color.red),q);
                SetPixelGreen(*image,ScaleShortToQuantum(color.green),q);
                SetPixelBlue(*image,ScaleShortToQuantum(color.blue),q);
                q+=GetPixelChannels(*image);
              }
              if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
                break;
            }
            break;
          }
        }
        image_view=DestroyCacheView(image_view);
        state&=(~UpdateConfigurationState);
      }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  XSetCursorState(display,windows,MagickFalse);
  (void) XFreeCursor(display,cursor);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C o m p o s i t e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XCompositeImage() requests an image name from the user, reads the image and
%  composites it with the X window image at a location the user chooses with
%  the pointer.
%
%  The format of the XCompositeImage method is:
%
%      MagickBooleanType XCompositeImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XCompositeImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  static char
    displacement_geometry[MagickPathExtent] = "30x30",
    filename[MagickPathExtent] = "\0";

  static const char
    *CompositeMenu[] =
    {
      "Operators",
      "Dissolve",
      "Displace",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static CompositeOperator
    compose = CopyCompositeOp;

  static const ModeType
    CompositeCommands[] =
    {
      CompositeOperatorsCommand,
      CompositeDissolveCommand,
      CompositeDisplaceCommand,
      CompositeHelpCommand,
      CompositeDismissCommand
    };

  char
    text[MagickPathExtent];

  Cursor
    cursor;

  Image
    *composite_image;

  int
    entry,
    id,
    x,
    y;

  double
    blend,
    scale_factor;

  RectangleInfo
    highlight_info,
    composite_info;

  unsigned int
    height,
    width;

  size_t
    state;

  XEvent
    event;

  /*
    Request image file name from user.
  */
  XFileBrowserWidget(display,windows,"Composite",filename);
  if (*filename == '\0')
    return(MagickTrue);
  /*
    Read image.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  (void) CopyMagickString(resource_info->image_info->filename,filename,
    MagickPathExtent);
  composite_image=ReadImage(resource_info->image_info,exception);
  CatchException(exception);
  XSetCursorState(display,windows,MagickFalse);
  if (composite_image == (Image *) NULL)
    return(MagickFalse);
  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"Composite");
  windows->command.data=1;
  (void) XCommandWidget(display,windows,CompositeMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  composite_info.x=(ssize_t) windows->image.x+x;
  composite_info.y=(ssize_t) windows->image.y+y;
  composite_info.width=0;
  composite_info.height=0;
  cursor=XCreateFontCursor(display,XC_ul_angle);
  (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
  blend=0.0;
  state=DefaultState;
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+ld%+ld ",
          (long) composite_info.x,(long) composite_info.y);
        XInfoWidget(display,windows,text);
      }
    highlight_info=composite_info;
    highlight_info.x=composite_info.x-windows->image.x;
    highlight_info.y=composite_info.y-windows->image.y;
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,CompositeMenu,&event);
        if (id < 0)
          continue;
        switch (CompositeCommands[id])
        {
          case CompositeOperatorsCommand:
          {
            char
              command[MagickPathExtent],
              **operators;

            /*
              Select a command from the pop-up menu.
            */
            operators=GetCommandOptions(MagickComposeOptions);
            if (operators == (char **) NULL)
              break;
            entry=XMenuWidget(display,windows,CompositeMenu[id],
              (const char **) operators,command);
            if (entry >= 0)
              compose=(CompositeOperator) ParseCommandOption(
                MagickComposeOptions,MagickFalse,operators[entry]);
            operators=DestroyStringList(operators);
            break;
          }
          case CompositeDissolveCommand:
          {
            static char
              factor[MagickPathExtent] = "20.0";

            /*
              Dissolve the two images a given percent.
            */
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            (void) XDialogWidget(display,windows,"Dissolve",
              "Enter the blend factor (0.0 - 99.9%):",factor);
            (void) XSetFunction(display,windows->image.highlight_context,
              GXinvert);
            if (*factor == '\0')
              break;
            blend=StringToDouble(factor,(char **) NULL);
            compose=DissolveCompositeOp;
            break;
          }
          case CompositeDisplaceCommand:
          {
            /*
              Get horizontal and vertical scale displacement geometry.
            */
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            (void) XDialogWidget(display,windows,"Displace",
              "Enter the horizontal and vertical scale:",displacement_geometry);
            (void) XSetFunction(display,windows->image.highlight_context,
              GXinvert);
            if (*displacement_geometry == '\0')
              break;
            compose=DisplaceCompositeOp;
            break;
          }
          case CompositeHelpCommand:
          {
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Composite",ImageCompositeHelp);
            (void) XSetFunction(display,windows->image.highlight_context,
              GXinvert);
            break;
          }
          case CompositeDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Button Press: 0x%lx %u +%d+%d",event.xbutton.window,
            event.xbutton.button,event.xbutton.x,event.xbutton.y);
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        /*
          Change cursor.
        */
        composite_info.width=composite_image->columns;
        composite_info.height=composite_image->rows;
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        composite_info.x=(ssize_t) windows->image.x+event.xbutton.x;
        composite_info.y=(ssize_t) windows->image.y+event.xbutton.y;
        break;
      }
      case ButtonRelease:
      {
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Button Release: 0x%lx %u +%d+%d",event.xbutton.window,
            event.xbutton.button,event.xbutton.x,event.xbutton.y);
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        if ((composite_info.width != 0) && (composite_info.height != 0))
          {
            /*
              User has selected the location of the composite image.
            */
            composite_info.x=(ssize_t) windows->image.x+event.xbutton.x;
            composite_info.y=(ssize_t) windows->image.y+event.xbutton.y;
            state|=ExitState;
          }
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MagickPathExtent];

        KeySym
          key_symbol;

        int
          length;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Key press: 0x%lx (%s)",(unsigned long) key_symbol,command);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            composite_image=DestroyImage(composite_image);
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Composite",ImageCompositeHelp);
            (void) XSetFunction(display,windows->image.highlight_context,
              GXinvert);
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
        composite_info.x=(ssize_t) windows->image.x+x;
        composite_info.y=(ssize_t) windows->image.y+y;
        break;
      }
      default:
      {
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),"Event type: %d",
            event.type);
        break;
      }
    }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSetCursorState(display,windows,MagickFalse);
  (void) XFreeCursor(display,cursor);
  if ((state & EscapeState) != 0)
    return(MagickTrue);
  /*
    Image compositing is relative to image configuration.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  x=0;
  y=0;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=(double) width/windows->image.ximage->width;
  composite_info.x+=x;
  composite_info.x=(ssize_t) (scale_factor*composite_info.x+0.5);
  composite_info.width=(unsigned int) (scale_factor*composite_info.width+0.5);
  scale_factor=(double) height/windows->image.ximage->height;
  composite_info.y+=y;
  composite_info.y=(ssize_t) (scale_factor*composite_info.y+0.5);
  composite_info.height=(unsigned int) (scale_factor*composite_info.height+0.5);
  if ((composite_info.width != composite_image->columns) ||
      (composite_info.height != composite_image->rows))
    {
      Image
        *resize_image;

      /*
        Scale composite image.
      */
      resize_image=ResizeImage(composite_image,composite_info.width,
        composite_info.height,composite_image->filter,exception);
      composite_image=DestroyImage(composite_image);
      if (resize_image == (Image *) NULL)
        {
          XSetCursorState(display,windows,MagickFalse);
          return(MagickFalse);
        }
      composite_image=resize_image;
    }
  if (compose == DisplaceCompositeOp)
    (void) SetImageArtifact(composite_image,"compose:args",
      displacement_geometry);
  if (blend != 0.0)
    {
      CacheView
        *image_view;

      int
        y;

      Quantum
        opacity;

      register int
        x;

      register Quantum
        *q;

      /*
        Create mattes for blending.
      */
      (void) SetImageAlphaChannel(composite_image,OpaqueAlphaChannel,exception);
      opacity=(Quantum) (ScaleQuantumToChar(QuantumRange)-
        ((ssize_t) ScaleQuantumToChar(QuantumRange)*blend)/100);
      if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
        return(MagickFalse);
      image->alpha_trait=BlendPixelTrait;
      image_view=AcquireAuthenticCacheView(image,exception);
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetCacheViewAuthenticPixels(image_view,0,(ssize_t) y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          SetPixelAlpha(image,opacity,q);
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          break;
      }
      image_view=DestroyCacheView(image_view);
    }
  /*
    Composite image with X Image window.
  */
  (void) CompositeImage(image,composite_image,compose,MagickTrue,
    composite_info.x,composite_info.y,exception);
  composite_image=DestroyImage(composite_image);
  XSetCursorState(display,windows,MagickFalse);
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,image,exception);
  (void) XConfigureImage(display,resource_info,windows,image,exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C o n f i g u r e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XConfigureImage() creates a new X image.  It also notifies the window
%  manager of the new image size and configures the transient widows.
%
%  The format of the XConfigureImage method is:
%
%      MagickBooleanType XConfigureImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XConfigureImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  char
    geometry[MagickPathExtent];

  MagickStatusType
    status;

  size_t
    mask,
    height,
    width;

  ssize_t
    x,
    y;

  XSizeHints
    *size_hints;

  XWindowChanges
    window_changes;

  /*
    Dismiss if window dimensions are zero.
  */
  width=(unsigned int) windows->image.window_changes.width;
  height=(unsigned int) windows->image.window_changes.height;
  if (image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Configure Image: %dx%d=>%.20gx%.20g",windows->image.ximage->width,
      windows->image.ximage->height,(double) width,(double) height);
  if ((width*height) == 0)
    return(MagickTrue);
  x=0;
  y=0;
  /*
    Resize image to fit Image window dimensions.
  */
  XSetCursorState(display,windows,MagickTrue);
  (void) XFlush(display);
  if (((int) width != windows->image.ximage->width) ||
      ((int) height != windows->image.ximage->height))
    image->taint=MagickTrue;
  windows->magnify.x=(int)
    width*windows->magnify.x/windows->image.ximage->width;
  windows->magnify.y=(int)
    height*windows->magnify.y/windows->image.ximage->height;
  windows->image.x=(int) (width*windows->image.x/windows->image.ximage->width);
  windows->image.y=(int)
    (height*windows->image.y/windows->image.ximage->height);
  status=XMakeImage(display,resource_info,&windows->image,image,
    (unsigned int) width,(unsigned int) height,exception);
  if (status == MagickFalse)
    XNoticeWidget(display,windows,"Unable to configure X image:",
      windows->image.name);
  /*
    Notify window manager of the new configuration.
  */
  if (resource_info->image_geometry != (char *) NULL)
    (void) FormatLocaleString(geometry,MagickPathExtent,"%s>!",
      resource_info->image_geometry);
  else
    (void) FormatLocaleString(geometry,MagickPathExtent,"%ux%u+0+0>!",
      XDisplayWidth(display,windows->image.screen),
      XDisplayHeight(display,windows->image.screen));
  (void) ParseMetaGeometry(geometry,&x,&y,&width,&height);
  window_changes.width=(int) width;
  if (window_changes.width > XDisplayWidth(display,windows->image.screen))
    window_changes.width=XDisplayWidth(display,windows->image.screen);
  window_changes.height=(int) height;
  if (window_changes.height > XDisplayHeight(display,windows->image.screen))
    window_changes.height=XDisplayHeight(display,windows->image.screen);
  mask=(size_t) (CWWidth | CWHeight);
  if (resource_info->backdrop)
    {
      mask|=CWX | CWY;
      window_changes.x=(int)
        ((XDisplayWidth(display,windows->image.screen)/2)-(width/2));
      window_changes.y=(int)
        ((XDisplayHeight(display,windows->image.screen)/2)-(height/2));
    }
  (void) XReconfigureWMWindow(display,windows->image.id,windows->image.screen,
    (unsigned int) mask,&window_changes);
  (void) XClearWindow(display,windows->image.id);
  XRefreshWindow(display,&windows->image,(XEvent *) NULL);
  /*
    Update Magnify window configuration.
  */
  if (windows->magnify.mapped != MagickFalse )
    XMakeMagnifyImage(display,windows,exception);
  windows->pan.crop_geometry=windows->image.crop_geometry;
  XBestIconSize(display,&windows->pan,image);
  while (((windows->pan.width << 1) < MaxIconSize) &&
         ((windows->pan.height << 1) < MaxIconSize))
  {
    windows->pan.width<<=1;
    windows->pan.height<<=1;
  }
  if (windows->pan.geometry != (char *) NULL)
    (void) XParseGeometry(windows->pan.geometry,&windows->pan.x,&windows->pan.y,
      &windows->pan.width,&windows->pan.height);
  window_changes.width=(int) windows->pan.width;
  window_changes.height=(int) windows->pan.height;
  size_hints=XAllocSizeHints();
  if (size_hints != (XSizeHints *) NULL)
    {
      /*
        Set new size hints.
      */
      size_hints->flags=PSize | PMinSize | PMaxSize;
      size_hints->width=window_changes.width;
      size_hints->height=window_changes.height;
      size_hints->min_width=size_hints->width;
      size_hints->min_height=size_hints->height;
      size_hints->max_width=size_hints->width;
      size_hints->max_height=size_hints->height;
      (void) XSetNormalHints(display,windows->pan.id,size_hints);
      (void) XFree((void *) size_hints);
    }
  (void) XReconfigureWMWindow(display,windows->pan.id,windows->pan.screen,
    (unsigned int) (CWWidth | CWHeight),&window_changes);
  /*
    Update icon window configuration.
  */
  windows->icon.crop_geometry=windows->image.crop_geometry;
  XBestIconSize(display,&windows->icon,image);
  window_changes.width=(int) windows->icon.width;
  window_changes.height=(int) windows->icon.height;
  (void) XReconfigureWMWindow(display,windows->icon.id,windows->icon.screen,
    (unsigned int) (CWWidth | CWHeight),&window_changes);
  XSetCursorState(display,windows,MagickFalse);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X C r o p I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XCropImage() allows the user to select a region of the image and crop, copy,
%  or cut it.  For copy or cut, the image can subsequently be composited onto
%  the image with XPasteImage.
%
%  The format of the XCropImage method is:
%
%      MagickBooleanType XCropImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        const ClipboardMode mode,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
%    o mode: This unsigned value specified whether the image should be
%      cropped, copied, or cut.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XCropImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  const ClipboardMode mode,ExceptionInfo *exception)
{
  static const char
    *CropModeMenu[] =
    {
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *RectifyModeMenu[] =
    {
      "Crop",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static const ModeType
    CropCommands[] =
    {
      CropHelpCommand,
      CropDismissCommand
    },
    RectifyCommands[] =
    {
      RectifyCopyCommand,
      RectifyHelpCommand,
      RectifyDismissCommand
    };

  CacheView
    *image_view;

  char
    command[MagickPathExtent],
    text[MagickPathExtent];

  Cursor
    cursor;

  int
    id,
    x,
    y;

  KeySym
    key_symbol;

  Image
    *crop_image;

  double
    scale_factor;

  RectangleInfo
    crop_info,
    highlight_info;

  register Quantum
    *q;

  unsigned int
    height,
    width;

  size_t
    state;

  XEvent
    event;

  /*
    Map Command widget.
  */
  switch (mode)
  {
    case CopyMode:
    {
      (void) CloneString(&windows->command.name,"Copy");
      break;
    }
    case CropMode:
    {
      (void) CloneString(&windows->command.name,"Crop");
      break;
    }
    case CutMode:
    {
      (void) CloneString(&windows->command.name,"Cut");
      break;
    }
  }
  RectifyModeMenu[0]=windows->command.name;
  windows->command.data=0;
  (void) XCommandWidget(display,windows,CropModeMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  crop_info.x=(ssize_t) windows->image.x+x;
  crop_info.y=(ssize_t) windows->image.y+y;
  crop_info.width=0;
  crop_info.height=0;
  cursor=XCreateFontCursor(display,XC_fleur);
  state=DefaultState;
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+ld%+ld ",
          (long) crop_info.x,(long) crop_info.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,CropModeMenu,&event);
        if (id < 0)
          continue;
        switch (CropCommands[id])
        {
          case CropHelpCommand:
          {
            switch (mode)
            {
              case CopyMode:
              {
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Copy",ImageCopyHelp);
                break;
              }
              case CropMode:
              {
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Crop",ImageCropHelp);
                break;
              }
              case CutMode:
              {
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Cut",ImageCutHelp);
                break;
              }
            }
            break;
          }
          case CropDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        /*
          Note first corner of cropping rectangle-- exit loop.
        */
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        crop_info.x=(ssize_t) windows->image.x+event.xbutton.x;
        crop_info.y=(ssize_t) windows->image.y+event.xbutton.y;
        state|=ExitState;
        break;
      }
      case ButtonRelease:
        break;
      case Expose:
        break;
      case KeyPress:
      {
        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            switch (mode)
            {
              case CopyMode:
              {
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Copy",ImageCopyHelp);
                break;
              }
              case CropMode:
              {
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Crop",ImageCropHelp);
                break;
              }
              case CutMode:
              {
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Cut",ImageCutHelp);
                break;
              }
            }
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        if (event.xmotion.window != windows->image.id)
          break;
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
        crop_info.x=(ssize_t) windows->image.x+x;
        crop_info.y=(ssize_t) windows->image.y+y;
        break;
      }
      default:
        break;
    }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  if ((state & EscapeState) != 0)
    {
      /*
        User want to exit without cropping.
      */
      (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      (void) XFreeCursor(display,cursor);
      return(MagickTrue);
    }
  (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
  do
  {
    /*
      Size rectangle as pointer moves until the mouse button is released.
    */
    x=(int) crop_info.x;
    y=(int) crop_info.y;
    crop_info.width=0;
    crop_info.height=0;
    state=DefaultState;
    do
    {
      highlight_info=crop_info;
      highlight_info.x=crop_info.x-windows->image.x;
      highlight_info.y=crop_info.y-windows->image.y;
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        {
          /*
            Display info and draw cropping rectangle.
          */
          if (windows->info.mapped == MagickFalse)
            (void) XMapWindow(display,windows->info.id);
          (void) FormatLocaleString(text,MagickPathExtent,
            " %.20gx%.20g%+.20g%+.20g",(double) crop_info.width,(double)
            crop_info.height,(double) crop_info.x,(double) crop_info.y);
          XInfoWidget(display,windows,text);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
        }
      else
        if (windows->info.mapped != MagickFalse )
          (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      /*
        Wait for next event.
      */
      XScreenEvent(display,windows,&event,exception);
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        XHighlightRectangle(display,windows->image.id,
          windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          crop_info.x=(ssize_t) windows->image.x+event.xbutton.x;
          crop_info.y=(ssize_t) windows->image.y+event.xbutton.y;
          break;
        }
        case ButtonRelease:
        {
          /*
            User has committed to cropping rectangle.
          */
          crop_info.x=(ssize_t) windows->image.x+event.xbutton.x;
          crop_info.y=(ssize_t) windows->image.y+event.xbutton.y;
          XSetCursorState(display,windows,MagickFalse);
          state|=ExitState;
          windows->command.data=0;
          (void) XCommandWidget(display,windows,RectifyModeMenu,
            (XEvent *) NULL);
          break;
        }
        case Expose:
          break;
        case MotionNotify:
        {
          crop_info.x=(ssize_t) windows->image.x+event.xmotion.x;
          crop_info.y=(ssize_t) windows->image.y+event.xmotion.y;
        }
        default:
          break;
      }
      if ((((int) crop_info.x != x) && ((int) crop_info.y != y)) ||
          ((state & ExitState) != 0))
        {
          /*
            Check boundary conditions.
          */
          if (crop_info.x < 0)
            crop_info.x=0;
          else
            if (crop_info.x > (ssize_t) windows->image.ximage->width)
              crop_info.x=(ssize_t) windows->image.ximage->width;
          if ((int) crop_info.x < x)
            crop_info.width=(unsigned int) (x-crop_info.x);
          else
            {
              crop_info.width=(unsigned int) (crop_info.x-x);
              crop_info.x=(ssize_t) x;
            }
          if (crop_info.y < 0)
            crop_info.y=0;
          else
            if (crop_info.y > (ssize_t) windows->image.ximage->height)
              crop_info.y=(ssize_t) windows->image.ximage->height;
          if ((int) crop_info.y < y)
            crop_info.height=(unsigned int) (y-crop_info.y);
          else
            {
              crop_info.height=(unsigned int) (crop_info.y-y);
              crop_info.y=(ssize_t) y;
            }
        }
    } while ((state & ExitState) == 0);
    /*
      Wait for user to grab a corner of the rectangle or press return.
    */
    state=DefaultState;
    (void) XMapWindow(display,windows->info.id);
    do
    {
      if (windows->info.mapped != MagickFalse )
        {
          /*
            Display pointer position.
          */
          (void) FormatLocaleString(text,MagickPathExtent,
            " %.20gx%.20g%+.20g%+.20g",(double) crop_info.width,(double)
            crop_info.height,(double) crop_info.x,(double) crop_info.y);
          XInfoWidget(display,windows,text);
        }
      highlight_info=crop_info;
      highlight_info.x=crop_info.x-windows->image.x;
      highlight_info.y=crop_info.y-windows->image.y;
      if ((highlight_info.width <= 3) || (highlight_info.height <= 3))
        {
          state|=EscapeState;
          state|=ExitState;
          break;
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      XScreenEvent(display,windows,&event,exception);
      if (event.xany.window == windows->command.id)
        {
          /*
            Select a command from the Command widget.
          */
          (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
          id=XCommandWidget(display,windows,RectifyModeMenu,&event);
          (void) XSetFunction(display,windows->image.highlight_context,
            GXinvert);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
          if (id >= 0)
            switch (RectifyCommands[id])
            {
              case RectifyCopyCommand:
              {
                state|=ExitState;
                break;
              }
              case RectifyHelpCommand:
              {
                (void) XSetFunction(display,windows->image.highlight_context,
                  GXcopy);
                switch (mode)
                {
                  case CopyMode:
                  {
                    XTextViewWidget(display,resource_info,windows,MagickFalse,
                      "Help Viewer - Image Copy",ImageCopyHelp);
                    break;
                  }
                  case CropMode:
                  {
                    XTextViewWidget(display,resource_info,windows,MagickFalse,
                      "Help Viewer - Image Crop",ImageCropHelp);
                    break;
                  }
                  case CutMode:
                  {
                    XTextViewWidget(display,resource_info,windows,MagickFalse,
                      "Help Viewer - Image Cut",ImageCutHelp);
                    break;
                  }
                }
                (void) XSetFunction(display,windows->image.highlight_context,
                  GXinvert);
                break;
              }
              case RectifyDismissCommand:
              {
                /*
                  Prematurely exit.
                */
                state|=EscapeState;
                state|=ExitState;
                break;
              }
              default:
                break;
            }
          continue;
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          if (event.xbutton.button != Button1)
            break;
          if (event.xbutton.window != windows->image.id)
            break;
          x=windows->image.x+event.xbutton.x;
          y=windows->image.y+event.xbutton.y;
          if ((x < (int) (crop_info.x+RoiDelta)) &&
              (x > (int) (crop_info.x-RoiDelta)) &&
              (y < (int) (crop_info.y+RoiDelta)) &&
              (y > (int) (crop_info.y-RoiDelta)))
            {
              crop_info.x=(ssize_t) (crop_info.x+crop_info.width);
              crop_info.y=(ssize_t) (crop_info.y+crop_info.height);
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (int) (crop_info.x+RoiDelta)) &&
              (x > (int) (crop_info.x-RoiDelta)) &&
              (y < (int) (crop_info.y+crop_info.height+RoiDelta)) &&
              (y > (int) (crop_info.y+crop_info.height-RoiDelta)))
            {
              crop_info.x=(ssize_t) (crop_info.x+crop_info.width);
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (int) (crop_info.x+crop_info.width+RoiDelta)) &&
              (x > (int) (crop_info.x+crop_info.width-RoiDelta)) &&
              (y < (int) (crop_info.y+RoiDelta)) &&
              (y > (int) (crop_info.y-RoiDelta)))
            {
              crop_info.y=(ssize_t) (crop_info.y+crop_info.height);
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (int) (crop_info.x+crop_info.width+RoiDelta)) &&
              (x > (int) (crop_info.x+crop_info.width-RoiDelta)) &&
              (y < (int) (crop_info.y+crop_info.height+RoiDelta)) &&
              (y > (int) (crop_info.y+crop_info.height-RoiDelta)))
            {
              state|=UpdateConfigurationState;
              break;
            }
        }
        case ButtonRelease:
        {
          if (event.xbutton.window == windows->pan.id)
            if ((highlight_info.x != crop_info.x-windows->image.x) ||
                (highlight_info.y != crop_info.y-windows->image.y))
              XHighlightRectangle(display,windows->image.id,
                windows->image.highlight_context,&highlight_info);
          (void) XSetSelectionOwner(display,XA_PRIMARY,windows->image.id,
            event.xbutton.time);
          break;
        }
        case Expose:
        {
          if (event.xexpose.window == windows->image.id)
            if (event.xexpose.count == 0)
              {
                event.xexpose.x=(int) highlight_info.x;
                event.xexpose.y=(int) highlight_info.y;
                event.xexpose.width=(int) highlight_info.width;
                event.xexpose.height=(int) highlight_info.height;
                XRefreshWindow(display,&windows->image,&event);
              }
          if (event.xexpose.window == windows->info.id)
            if (event.xexpose.count == 0)
              XInfoWidget(display,windows,text);
          break;
        }
        case KeyPress:
        {
          if (event.xkey.window != windows->image.id)
            break;
          /*
            Respond to a user key press.
          */
          (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
            sizeof(command),&key_symbol,(XComposeStatus *) NULL);
          switch ((int) key_symbol)
          {
            case XK_Escape:
            case XK_F20:
              state|=EscapeState;
            case XK_Return:
            {
              state|=ExitState;
              break;
            }
            case XK_Home:
            case XK_KP_Home:
            {
              crop_info.x=(ssize_t) (windows->image.width/2L-crop_info.width/
                2L);
              crop_info.y=(ssize_t) (windows->image.height/2L-crop_info.height/
                2L);
              break;
            }
            case XK_Left:
            case XK_KP_Left:
            {
              crop_info.x--;
              break;
            }
            case XK_Up:
            case XK_KP_Up:
            case XK_Next:
            {
              crop_info.y--;
              break;
            }
            case XK_Right:
            case XK_KP_Right:
            {
              crop_info.x++;
              break;
            }
            case XK_Prior:
            case XK_Down:
            case XK_KP_Down:
            {
              crop_info.y++;
              break;
            }
            case XK_F1:
            case XK_Help:
            {
              (void) XSetFunction(display,windows->image.highlight_context,
                GXcopy);
              switch (mode)
              {
                case CopyMode:
                {
                  XTextViewWidget(display,resource_info,windows,MagickFalse,
                    "Help Viewer - Image Copy",ImageCopyHelp);
                  break;
                }
                case CropMode:
                {
                  XTextViewWidget(display,resource_info,windows,MagickFalse,
                    "Help Viewer - Image Cropg",ImageCropHelp);
                  break;
                }
                case CutMode:
                {
                  XTextViewWidget(display,resource_info,windows,MagickFalse,
                    "Help Viewer - Image Cutg",ImageCutHelp);
                  break;
                }
              }
              (void) XSetFunction(display,windows->image.highlight_context,
                GXinvert);
              break;
            }
            default:
            {
              (void) XBell(display,0);
              break;
            }
          }
          (void) XSetSelectionOwner(display,XA_PRIMARY,windows->image.id,
            event.xkey.time);
          break;
        }
        case KeyRelease:
          break;
        case MotionNotify:
        {
          if (event.xmotion.window != windows->image.id)
            break;
          /*
            Map and unmap Info widget as text cursor crosses its boundaries.
          */
          x=event.xmotion.x;
          y=event.xmotion.y;
          if (windows->info.mapped != MagickFalse )
            {
              if ((x < (int) (windows->info.x+windows->info.width)) &&
                  (y < (int) (windows->info.y+windows->info.height)))
                (void) XWithdrawWindow(display,windows->info.id,
                  windows->info.screen);
            }
          else
            if ((x > (int) (windows->info.x+windows->info.width)) ||
                (y > (int) (windows->info.y+windows->info.height)))
              (void) XMapWindow(display,windows->info.id);
          crop_info.x=(ssize_t) windows->image.x+event.xmotion.x;
          crop_info.y=(ssize_t) windows->image.y+event.xmotion.y;
          break;
        }
        case SelectionRequest:
        {
          XSelectionEvent
            notify;

          XSelectionRequestEvent
            *request;

          /*
            Set primary selection.
          */
          (void) FormatLocaleString(text,MagickPathExtent,
            "%.20gx%.20g%+.20g%+.20g",(double) crop_info.width,(double)
            crop_info.height,(double) crop_info.x,(double) crop_info.y);
          request=(&(event.xselectionrequest));
          (void) XChangeProperty(request->display,request->requestor,
            request->property,request->target,8,PropModeReplace,
            (unsigned char *) text,(int) strlen(text));
          notify.type=SelectionNotify;
          notify.display=request->display;
          notify.requestor=request->requestor;
          notify.selection=request->selection;
          notify.target=request->target;
          notify.time=request->time;
          if (request->property == None)
            notify.property=request->target;
          else
            notify.property=request->property;
          (void) XSendEvent(request->display,request->requestor,False,0,
            (XEvent *) &notify);
        }
        default:
          break;
      }
      if ((state & UpdateConfigurationState) != 0)
        {
          (void) XPutBackEvent(display,&event);
          (void) XCheckDefineCursor(display,windows->image.id,cursor);
          break;
        }
    } while ((state & ExitState) == 0);
  } while ((state & ExitState) == 0);
  (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSetCursorState(display,windows,MagickFalse);
  if ((state & EscapeState) != 0)
    return(MagickTrue);
  if (mode == CropMode)
    if (((int) crop_info.width != windows->image.ximage->width) ||
        ((int) crop_info.height != windows->image.ximage->height))
      {
        /*
          Reconfigure Image window as defined by cropping rectangle.
        */
        XSetCropGeometry(display,windows,&crop_info,image);
        windows->image.window_changes.width=(int) crop_info.width;
        windows->image.window_changes.height=(int) crop_info.height;
        (void) XConfigureImage(display,resource_info,windows,image,exception);
        return(MagickTrue);
      }
  /*
    Copy image before applying image transforms.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  x=0;
  y=0;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=(double) width/windows->image.ximage->width;
  crop_info.x+=x;
  crop_info.x=(ssize_t) (scale_factor*crop_info.x+0.5);
  crop_info.width=(unsigned int) (scale_factor*crop_info.width+0.5);
  scale_factor=(double) height/windows->image.ximage->height;
  crop_info.y+=y;
  crop_info.y=(ssize_t) (scale_factor*crop_info.y+0.5);
  crop_info.height=(unsigned int) (scale_factor*crop_info.height+0.5);
  crop_image=CropImage(image,&crop_info,exception);
  XSetCursorState(display,windows,MagickFalse);
  if (crop_image == (Image *) NULL)
    return(MagickFalse);
  if (resource_info->copy_image != (Image *) NULL)
    resource_info->copy_image=DestroyImage(resource_info->copy_image);
  resource_info->copy_image=crop_image;
  if (mode == CopyMode)
    {
      (void) XConfigureImage(display,resource_info,windows,image,exception);
      return(MagickTrue);
    }
  /*
    Cut image.
  */
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  image->alpha_trait=BlendPixelTrait;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (int) crop_info.height; y++)
  {
    q=GetCacheViewAuthenticPixels(image_view,crop_info.x,y+crop_info.y,
      crop_info.width,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (int) crop_info.width; x++)
    {
      SetPixelAlpha(image,TransparentAlpha,q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,image,exception);
  (void) XConfigureImage(display,resource_info,windows,image,exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X D r a w I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDrawEditImage() draws a graphic element (point, line, rectangle, etc.) on
%  the image.
%
%  The format of the XDrawEditImage method is:
%
%      MagickBooleanType XDrawEditImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image **image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XDrawEditImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image **image,
  ExceptionInfo *exception)
{
  static const char
    *DrawMenu[] =
    {
      "Element",
      "Color",
      "Stipple",
      "Width",
      "Undo",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static ElementType
    element = PointElement;

  static const ModeType
    DrawCommands[] =
    {
      DrawElementCommand,
      DrawColorCommand,
      DrawStippleCommand,
      DrawWidthCommand,
      DrawUndoCommand,
      DrawHelpCommand,
      DrawDismissCommand
    };

  static Pixmap
    stipple = (Pixmap) NULL;

  static unsigned int
    pen_id = 0,
    line_width = 1;

  char
    command[MagickPathExtent],
    text[MagickPathExtent];

  Cursor
    cursor;

  int
    entry,
    id,
    number_coordinates,
    x,
    y;

  double
    degrees;

  MagickStatusType
    status;

  RectangleInfo
    rectangle_info;

  register int
    i;

  unsigned int
    distance,
    height,
    max_coordinates,
    width;

  size_t
    state;

  Window
    root_window;

  XDrawInfo
    draw_info;

  XEvent
    event;

  XPoint
    *coordinate_info;

  XSegment
    line_info;

  /*
    Allocate polygon info.
  */
  max_coordinates=2048;
  coordinate_info=(XPoint *) AcquireQuantumMemory((size_t) max_coordinates,
    sizeof(*coordinate_info));
  if (coordinate_info == (XPoint *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'","...");
      return(MagickFalse);
    }
  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"Draw");
  windows->command.data=4;
  (void) XCommandWidget(display,windows,DrawMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Wait for first button press.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  draw_info.stencil=OpaqueStencil;
  status=MagickTrue;
  cursor=XCreateFontCursor(display,XC_tcross);
  for ( ; ; )
  {
    XQueryPosition(display,windows->image.id,&x,&y);
    (void) XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask | PointerMotionMask);
    (void) XCheckDefineCursor(display,windows->image.id,cursor);
    state=DefaultState;
    do
    {
      if (windows->info.mapped != MagickFalse )
        {
          /*
            Display pointer position.
          */
          (void) FormatLocaleString(text,MagickPathExtent," %+d%+d ",
            x+windows->image.x,y+windows->image.y);
          XInfoWidget(display,windows,text);
        }
      /*
        Wait for next event.
      */
      XScreenEvent(display,windows,&event,exception);
      if (event.xany.window == windows->command.id)
        {
          /*
            Select a command from the Command widget.
          */
          id=XCommandWidget(display,windows,DrawMenu,&event);
          if (id < 0)
            continue;
          switch (DrawCommands[id])
          {
            case DrawElementCommand:
            {
              static const char
                *Elements[] =
                {
                  "point",
                  "line",
                  "rectangle",
                  "fill rectangle",
                  "circle",
                  "fill circle",
                  "ellipse",
                  "fill ellipse",
                  "polygon",
                  "fill polygon",
                  (char *) NULL,
                };

              /*
                Select a command from the pop-up menu.
              */
              element=(ElementType) (XMenuWidget(display,windows,
                DrawMenu[id],Elements,command)+1);
              break;
            }
            case DrawColorCommand:
            {
              const char
                *ColorMenu[MaxNumberPens+1];

              int
                pen_number;

              MagickBooleanType
                transparent;

              XColor
                color;

              /*
                Initialize menu selections.
              */
              for (i=0; i < (int) (MaxNumberPens-2); i++)
                ColorMenu[i]=resource_info->pen_colors[i];
              ColorMenu[MaxNumberPens-2]="transparent";
              ColorMenu[MaxNumberPens-1]="Browser...";
              ColorMenu[MaxNumberPens]=(char *) NULL;
              /*
                Select a pen color from the pop-up menu.
              */
              pen_number=XMenuWidget(display,windows,DrawMenu[id],
                (const char **) ColorMenu,command);
              if (pen_number < 0)
                break;
              transparent=pen_number == (MaxNumberPens-2) ? MagickTrue :
                MagickFalse;
              if (transparent != MagickFalse )
                {
                  draw_info.stencil=TransparentStencil;
                  break;
                }
              if (pen_number == (MaxNumberPens-1))
                {
                  static char
                    color_name[MagickPathExtent] = "gray";

                  /*
                    Select a pen color from a dialog.
                  */
                  resource_info->pen_colors[pen_number]=color_name;
                  XColorBrowserWidget(display,windows,"Select",color_name);
                  if (*color_name == '\0')
                    break;
                }
              /*
                Set pen color.
              */
              (void) XParseColor(display,windows->map_info->colormap,
                resource_info->pen_colors[pen_number],&color);
              XBestPixel(display,windows->map_info->colormap,(XColor *) NULL,
                (unsigned int) MaxColors,&color);
              windows->pixel_info->pen_colors[pen_number]=color;
              pen_id=(unsigned int) pen_number;
              draw_info.stencil=OpaqueStencil;
              break;
            }
            case DrawStippleCommand:
            {
              Image
                *stipple_image;

              ImageInfo
                *image_info;

              int
                status;

              static char
                filename[MagickPathExtent] = "\0";

              static const char
                *StipplesMenu[] =
                {
                  "Brick",
                  "Diagonal",
                  "Scales",
                  "Vertical",
                  "Wavy",
                  "Translucent",
                  "Opaque",
                  (char *) NULL,
                  (char *) NULL,
                };

              /*
                Select a command from the pop-up menu.
              */
              StipplesMenu[7]="Open...";
              entry=XMenuWidget(display,windows,DrawMenu[id],StipplesMenu,
                command);
              if (entry < 0)
                break;
              if (stipple != (Pixmap) NULL)
                (void) XFreePixmap(display,stipple);
              stipple=(Pixmap) NULL;
              if (entry != 7)
                {
                  switch (entry)
                  {
                    case 0:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) BricksBitmap,BricksWidth,BricksHeight);
                      break;
                    }
                    case 1:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) DiagonalBitmap,DiagonalWidth,DiagonalHeight);
                      break;
                    }
                    case 2:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) ScalesBitmap,ScalesWidth,ScalesHeight);
                      break;
                    }
                    case 3:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) VerticalBitmap,VerticalWidth,VerticalHeight);
                      break;
                    }
                    case 4:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) WavyBitmap,WavyWidth,WavyHeight);
                      break;
                    }
                    case 5:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) HighlightBitmap,HighlightWidth,
                        HighlightHeight);
                      break;
                    }
                    case 6:
                    default:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) OpaqueBitmap,OpaqueWidth,OpaqueHeight);
                      break;
                    }
                  }
                  break;
                }
              XFileBrowserWidget(display,windows,"Stipple",filename);
              if (*filename == '\0')
                break;
              /*
                Read image.
              */
              XSetCursorState(display,windows,MagickTrue);
              XCheckRefreshWindows(display,windows);
              image_info=AcquireImageInfo();
              (void) CopyMagickString(image_info->filename,filename,
                MagickPathExtent);
              stipple_image=ReadImage(image_info,exception);
              CatchException(exception);
              XSetCursorState(display,windows,MagickFalse);
              if (stipple_image == (Image *) NULL)
                break;
              (void) AcquireUniqueFileResource(filename);
              (void) FormatLocaleString(stipple_image->filename,MagickPathExtent,
                "xbm:%s",filename);
              (void) WriteImage(image_info,stipple_image,exception);
              stipple_image=DestroyImage(stipple_image);
              image_info=DestroyImageInfo(image_info);
              status=XReadBitmapFile(display,root_window,filename,&width,
                &height,&stipple,&x,&y);
              (void) RelinquishUniqueFileResource(filename);
              if ((status != BitmapSuccess) != 0)
                XNoticeWidget(display,windows,"Unable to read X bitmap image:",
                  filename);
              break;
            }
            case DrawWidthCommand:
            {
              static char
                width[MagickPathExtent] = "0";

              static const char
                *WidthsMenu[] =
                {
                  "1",
                  "2",
                  "4",
                  "8",
                  "16",
                  "Dialog...",
                  (char *) NULL,
                };

              /*
                Select a command from the pop-up menu.
              */
              entry=XMenuWidget(display,windows,DrawMenu[id],WidthsMenu,
                command);
              if (entry < 0)
                break;
              if (entry != 5)
                {
                  line_width=(unsigned int) StringToUnsignedLong(
                    WidthsMenu[entry]);
                  break;
                }
              (void) XDialogWidget(display,windows,"Ok","Enter line width:",
                width);
              if (*width == '\0')
                break;
              line_width=(unsigned int) StringToUnsignedLong(width);
              break;
            }
            case DrawUndoCommand:
            {
              (void) XMagickCommand(display,resource_info,windows,UndoCommand,
                image,exception);
              break;
            }
            case DrawHelpCommand:
            {
              XTextViewWidget(display,resource_info,windows,MagickFalse,
                "Help Viewer - Image Rotation",ImageDrawHelp);
              (void) XCheckDefineCursor(display,windows->image.id,cursor);
              break;
            }
            case DrawDismissCommand:
            {
              /*
                Prematurely exit.
              */
              state|=EscapeState;
              state|=ExitState;
              break;
            }
            default:
              break;
          }
          (void) XCheckDefineCursor(display,windows->image.id,cursor);
          continue;
        }
      switch (event.type)
      {
        case ButtonPress:
        {
          if (event.xbutton.button != Button1)
            break;
          if (event.xbutton.window != windows->image.id)
            break;
          /*
            exit loop.
          */
          x=event.xbutton.x;
          y=event.xbutton.y;
          state|=ExitState;
          break;
        }
        case ButtonRelease:
          break;
        case Expose:
          break;
        case KeyPress:
        {
          KeySym
            key_symbol;

          if (event.xkey.window != windows->image.id)
            break;
          /*
            Respond to a user key press.
          */
          (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
            sizeof(command),&key_symbol,(XComposeStatus *) NULL);
          switch ((int) key_symbol)
          {
            case XK_Escape:
            case XK_F20:
            {
              /*
                Prematurely exit.
              */
              state|=EscapeState;
              state|=ExitState;
              break;
            }
            case XK_F1:
            case XK_Help:
            {
              XTextViewWidget(display,resource_info,windows,MagickFalse,
                "Help Viewer - Image Rotation",ImageDrawHelp);
              break;
            }
            default:
            {
              (void) XBell(display,0);
              break;
            }
          }
          break;
        }
        case MotionNotify:
        {
          /*
            Map and unmap Info widget as text cursor crosses its boundaries.
          */
          x=event.xmotion.x;
          y=event.xmotion.y;
          if (windows->info.mapped != MagickFalse )
            {
              if ((x < (int) (windows->info.x+windows->info.width)) &&
                  (y < (int) (windows->info.y+windows->info.height)))
                (void) XWithdrawWindow(display,windows->info.id,
                  windows->info.screen);
            }
          else
            if ((x > (int) (windows->info.x+windows->info.width)) ||
                (y > (int) (windows->info.y+windows->info.height)))
              (void) XMapWindow(display,windows->info.id);
          break;
        }
      }
    } while ((state & ExitState) == 0);
    (void) XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask);
    (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
    if ((state & EscapeState) != 0)
      break;
    /*
      Draw element as pointer moves until the button is released.
    */
    distance=0;
    degrees=0.0;
    line_info.x1=x;
    line_info.y1=y;
    line_info.x2=x;
    line_info.y2=y;
    rectangle_info.x=(ssize_t) x;
    rectangle_info.y=(ssize_t) y;
    rectangle_info.width=0;
    rectangle_info.height=0;
    number_coordinates=1;
    coordinate_info->x=x;
    coordinate_info->y=y;
    (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
    state=DefaultState;
    do
    {
      switch (element)
      {
        case PointElement:
        default:
        {
          if (number_coordinates > 1)
            {
              (void) XDrawLines(display,windows->image.id,
                windows->image.highlight_context,coordinate_info,
                number_coordinates,CoordModeOrigin);
              (void) FormatLocaleString(text,MagickPathExtent," %+d%+d",
                coordinate_info[number_coordinates-1].x,
                coordinate_info[number_coordinates-1].y);
              XInfoWidget(display,windows,text);
            }
          break;
        }
        case LineElement:
        {
          if (distance > 9)
            {
              /*
                Display angle of the line.
              */
              degrees=RadiansToDegrees(-atan2((double) (line_info.y2-
                line_info.y1),(double) (line_info.x2-line_info.x1)));
              (void) FormatLocaleString(text,MagickPathExtent," %g",
                (double) degrees);
              XInfoWidget(display,windows,text);
              XHighlightLine(display,windows->image.id,
                windows->image.highlight_context,&line_info);
            }
          else
            if (windows->info.mapped != MagickFalse )
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          break;
        }
        case RectangleElement:
        case FillRectangleElement:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            {
              /*
                Display info and draw drawing rectangle.
              */
              (void) FormatLocaleString(text,MagickPathExtent,
                " %.20gx%.20g%+.20g%+.20g",(double) rectangle_info.width,
                (double) rectangle_info.height,(double) rectangle_info.x,
                (double) rectangle_info.y);
              XInfoWidget(display,windows,text);
              XHighlightRectangle(display,windows->image.id,
                windows->image.highlight_context,&rectangle_info);
            }
          else
            if (windows->info.mapped != MagickFalse )
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          break;
        }
        case CircleElement:
        case FillCircleElement:
        case EllipseElement:
        case FillEllipseElement:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            {
              /*
                Display info and draw drawing rectangle.
              */
              (void) FormatLocaleString(text,MagickPathExtent,
                " %.20gx%.20g%+.20g%+.20g",(double) rectangle_info.width,
                (double) rectangle_info.height,(double) rectangle_info.x,
                (double) rectangle_info.y);
              XInfoWidget(display,windows,text);
              XHighlightEllipse(display,windows->image.id,
                windows->image.highlight_context,&rectangle_info);
            }
          else
            if (windows->info.mapped != MagickFalse )
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          break;
        }
        case PolygonElement:
        case FillPolygonElement:
        {
          if (number_coordinates > 1)
            (void) XDrawLines(display,windows->image.id,
              windows->image.highlight_context,coordinate_info,
              number_coordinates,CoordModeOrigin);
          if (distance > 9)
            {
              /*
                Display angle of the line.
              */
              degrees=RadiansToDegrees(-atan2((double) (line_info.y2-
                line_info.y1),(double) (line_info.x2-line_info.x1)));
              (void) FormatLocaleString(text,MagickPathExtent," %g",
                (double) degrees);
              XInfoWidget(display,windows,text);
              XHighlightLine(display,windows->image.id,
                windows->image.highlight_context,&line_info);
            }
          else
            if (windows->info.mapped != MagickFalse )
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          break;
        }
      }
      /*
        Wait for next event.
      */
      XScreenEvent(display,windows,&event,exception);
      switch (element)
      {
        case PointElement:
        default:
        {
          if (number_coordinates > 1)
            (void) XDrawLines(display,windows->image.id,
              windows->image.highlight_context,coordinate_info,
              number_coordinates,CoordModeOrigin);
          break;
        }
        case LineElement:
        {
          if (distance > 9)
            XHighlightLine(display,windows->image.id,
              windows->image.highlight_context,&line_info);
          break;
        }
        case RectangleElement:
        case FillRectangleElement:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            XHighlightRectangle(display,windows->image.id,
              windows->image.highlight_context,&rectangle_info);
          break;
        }
        case CircleElement:
        case FillCircleElement:
        case EllipseElement:
        case FillEllipseElement:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            XHighlightEllipse(display,windows->image.id,
              windows->image.highlight_context,&rectangle_info);
          break;
        }
        case PolygonElement:
        case FillPolygonElement:
        {
          if (number_coordinates > 1)
            (void) XDrawLines(display,windows->image.id,
              windows->image.highlight_context,coordinate_info,
              number_coordinates,CoordModeOrigin);
          if (distance > 9)
            XHighlightLine(display,windows->image.id,
              windows->image.highlight_context,&line_info);
          break;
        }
      }
      switch (event.type)
      {
        case ButtonPress:
          break;
        case ButtonRelease:
        {
          /*
            User has committed to element.
          */
          line_info.x2=event.xbutton.x;
          line_info.y2=event.xbutton.y;
          rectangle_info.x=(ssize_t) event.xbutton.x;
          rectangle_info.y=(ssize_t) event.xbutton.y;
          coordinate_info[number_coordinates].x=event.xbutton.x;
          coordinate_info[number_coordinates].y=event.xbutton.y;
          if (((element != PolygonElement) &&
               (element != FillPolygonElement)) || (distance <= 9))
            {
              state|=ExitState;
              break;
            }
          number_coordinates++;
          if (number_coordinates < (int) max_coordinates)
            {
              line_info.x1=event.xbutton.x;
              line_info.y1=event.xbutton.y;
              break;
            }
          max_coordinates<<=1;
          coordinate_info=(XPoint *) ResizeQuantumMemory(coordinate_info,
            max_coordinates,sizeof(*coordinate_info));
          if (coordinate_info == (XPoint *) NULL)
            (void) ThrowMagickException(exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'","...");
          break;
        }
        case Expose:
          break;
        case MotionNotify:
        {
          if (event.xmotion.window != windows->image.id)
            break;
          if (element != PointElement)
            {
              line_info.x2=event.xmotion.x;
              line_info.y2=event.xmotion.y;
              rectangle_info.x=(ssize_t) event.xmotion.x;
              rectangle_info.y=(ssize_t) event.xmotion.y;
              break;
            }
          coordinate_info[number_coordinates].x=event.xbutton.x;
          coordinate_info[number_coordinates].y=event.xbutton.y;
          number_coordinates++;
          if (number_coordinates < (int) max_coordinates)
            break;
          max_coordinates<<=1;
          coordinate_info=(XPoint *) ResizeQuantumMemory(coordinate_info,
            max_coordinates,sizeof(*coordinate_info));
          if (coordinate_info == (XPoint *) NULL)
            (void) ThrowMagickException(exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'","...");
          break;
        }
        default:
          break;
      }
      /*
        Check boundary conditions.
      */
      if (line_info.x2 < 0)
        line_info.x2=0;
      else
        if (line_info.x2 > (int) windows->image.width)
          line_info.x2=(short) windows->image.width;
      if (line_info.y2 < 0)
        line_info.y2=0;
      else
        if (line_info.y2 > (int) windows->image.height)
          line_info.y2=(short) windows->image.height;
      distance=(unsigned int)
        (((line_info.x2-line_info.x1+1)*(line_info.x2-line_info.x1+1))+
         ((line_info.y2-line_info.y1+1)*(line_info.y2-line_info.y1+1)));
      if ((((int) rectangle_info.x != x) && ((int) rectangle_info.y != y)) ||
          ((state & ExitState) != 0))
        {
          if (rectangle_info.x < 0)
            rectangle_info.x=0;
          else
            if (rectangle_info.x > (ssize_t) windows->image.width)
              rectangle_info.x=(ssize_t) windows->image.width;
          if ((int) rectangle_info.x < x)
            rectangle_info.width=(unsigned int) (x-rectangle_info.x);
          else
            {
              rectangle_info.width=(unsigned int) (rectangle_info.x-x);
              rectangle_info.x=(ssize_t) x;
            }
          if (rectangle_info.y < 0)
            rectangle_info.y=0;
          else
            if (rectangle_info.y > (ssize_t) windows->image.height)
              rectangle_info.y=(ssize_t) windows->image.height;
          if ((int) rectangle_info.y < y)
            rectangle_info.height=(unsigned int) (y-rectangle_info.y);
          else
            {
              rectangle_info.height=(unsigned int) (rectangle_info.y-y);
              rectangle_info.y=(ssize_t) y;
            }
        }
    } while ((state & ExitState) == 0);
    (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
    if ((element == PointElement) || (element == PolygonElement) ||
        (element == FillPolygonElement))
      {
        /*
          Determine polygon bounding box.
        */
        rectangle_info.x=(ssize_t) coordinate_info->x;
        rectangle_info.y=(ssize_t) coordinate_info->y;
        x=coordinate_info->x;
        y=coordinate_info->y;
        for (i=1; i < number_coordinates; i++)
        {
          if (coordinate_info[i].x > x)
            x=coordinate_info[i].x;
          if (coordinate_info[i].y > y)
            y=coordinate_info[i].y;
          if ((ssize_t) coordinate_info[i].x < rectangle_info.x)
            rectangle_info.x=MagickMax((ssize_t) coordinate_info[i].x,0);
          if ((ssize_t) coordinate_info[i].y < rectangle_info.y)
            rectangle_info.y=MagickMax((ssize_t) coordinate_info[i].y,0);
        }
        rectangle_info.width=(size_t) (x-rectangle_info.x);
        rectangle_info.height=(size_t) (y-rectangle_info.y);
        for (i=0; i < number_coordinates; i++)
        {
          coordinate_info[i].x-=rectangle_info.x;
          coordinate_info[i].y-=rectangle_info.y;
        }
      }
    else
      if (distance <= 9)
        continue;
      else
        if ((element == RectangleElement) ||
            (element == CircleElement) || (element == EllipseElement))
          {
            rectangle_info.width--;
            rectangle_info.height--;
          }
    /*
      Drawing is relative to image configuration.
    */
    draw_info.x=(int) rectangle_info.x;
    draw_info.y=(int) rectangle_info.y;
    (void) XMagickCommand(display,resource_info,windows,SaveToUndoBufferCommand,
      image,exception);
    width=(unsigned int) (*image)->columns;
    height=(unsigned int) (*image)->rows;
    x=0;
    y=0;
    if (windows->image.crop_geometry != (char *) NULL)
      (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
    draw_info.x+=windows->image.x-(line_width/2);
    if (draw_info.x < 0)
      draw_info.x=0;
    draw_info.x=(int) (width*draw_info.x/windows->image.ximage->width);
    draw_info.y+=windows->image.y-(line_width/2);
    if (draw_info.y < 0)
      draw_info.y=0;
    draw_info.y=(int) height*draw_info.y/windows->image.ximage->height;
    draw_info.width=(unsigned int) rectangle_info.width+(line_width << 1);
    if (draw_info.width > (unsigned int) (*image)->columns)
      draw_info.width=(unsigned int) (*image)->columns;
    draw_info.height=(unsigned int) rectangle_info.height+(line_width << 1);
    if (draw_info.height > (unsigned int) (*image)->rows)
      draw_info.height=(unsigned int) (*image)->rows;
    (void) FormatLocaleString(draw_info.geometry,MagickPathExtent,"%ux%u%+d%+d",
      width*draw_info.width/windows->image.ximage->width,
      height*draw_info.height/windows->image.ximage->height,
      draw_info.x+x,draw_info.y+y);
    /*
      Initialize drawing attributes.
    */
    draw_info.degrees=0.0;
    draw_info.element=element;
    draw_info.stipple=stipple;
    draw_info.line_width=line_width;
    draw_info.line_info=line_info;
    if (line_info.x1 > (int) (line_width/2))
      draw_info.line_info.x1=(short) line_width/2;
    if (line_info.y1 > (int) (line_width/2))
      draw_info.line_info.y1=(short) line_width/2;
    draw_info.line_info.x2=(short) (line_info.x2-line_info.x1+(line_width/2));
    draw_info.line_info.y2=(short) (line_info.y2-line_info.y1+(line_width/2));
    if ((draw_info.line_info.x2 < 0) && (draw_info.line_info.y2 < 0))
      {
        draw_info.line_info.x2=(-draw_info.line_info.x2);
        draw_info.line_info.y2=(-draw_info.line_info.y2);
      }
    if (draw_info.line_info.x2 < 0)
      {
        draw_info.line_info.x2=(-draw_info.line_info.x2);
        Swap(draw_info.line_info.x1,draw_info.line_info.x2);
      }
    if (draw_info.line_info.y2 < 0)
      {
        draw_info.line_info.y2=(-draw_info.line_info.y2);
        Swap(draw_info.line_info.y1,draw_info.line_info.y2);
      }
    draw_info.rectangle_info=rectangle_info;
    if (draw_info.rectangle_info.x > (ssize_t) (line_width/2))
      draw_info.rectangle_info.x=(ssize_t) line_width/2;
    if (draw_info.rectangle_info.y > (ssize_t) (line_width/2))
      draw_info.rectangle_info.y=(ssize_t) line_width/2;
    draw_info.number_coordinates=(unsigned int) number_coordinates;
    draw_info.coordinate_info=coordinate_info;
    windows->pixel_info->pen_color=windows->pixel_info->pen_colors[pen_id];
    /*
      Draw element on image.
    */
    XSetCursorState(display,windows,MagickTrue);
    XCheckRefreshWindows(display,windows);
    status=XDrawImage(display,windows->pixel_info,&draw_info,*image,exception);
    XSetCursorState(display,windows,MagickFalse);
    /*
      Update image colormap and return to image drawing.
    */
    XConfigureImageColormap(display,resource_info,windows,*image,exception);
    (void) XConfigureImage(display,resource_info,windows,*image,exception);
  }
  XSetCursorState(display,windows,MagickFalse);
  coordinate_info=(XPoint *) RelinquishMagickMemory(coordinate_info);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X D r a w P a n R e c t a n g l e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDrawPanRectangle() draws a rectangle in the pan window.  The pan window
%  displays a zoom image and the rectangle shows which portion of the image is
%  displayed in the Image window.
%
%  The format of the XDrawPanRectangle method is:
%
%      XDrawPanRectangle(Display *display,XWindows *windows)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
*/
static void XDrawPanRectangle(Display *display,XWindows *windows)
{
  double
    scale_factor;

  RectangleInfo
    highlight_info;

  /*
    Determine dimensions of the panning rectangle.
  */
  scale_factor=(double) windows->pan.width/windows->image.ximage->width;
  highlight_info.x=(ssize_t) (scale_factor*windows->image.x+0.5);
  highlight_info.width=(unsigned int) (scale_factor*windows->image.width+0.5);
  scale_factor=(double)
    windows->pan.height/windows->image.ximage->height;
  highlight_info.y=(ssize_t) (scale_factor*windows->image.y+0.5);
  highlight_info.height=(unsigned int) (scale_factor*windows->image.height+0.5);
  /*
    Display the panning rectangle.
  */
  (void) XClearWindow(display,windows->pan.id);
  XHighlightRectangle(display,windows->pan.id,windows->pan.annotate_context,
    &highlight_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X I m a g e C a c h e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XImageCache() handles the creation, manipulation, and destruction of the
%  image cache (undo and redo buffers).
%
%  The format of the XImageCache method is:
%
%      void XImageCache(Display *display,XResourceInfo *resource_info,
%        XWindows *windows,const CommandType command,Image **image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o command: Specifies a command to perform.
%
%    o image: the image;  XImageCache may transform the image and return a new
%      image pointer.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XImageCache(Display *display,XResourceInfo *resource_info,
  XWindows *windows,const CommandType command,Image **image,
  ExceptionInfo *exception)
{
  Image
    *cache_image;

  static Image
    *redo_image = (Image *) NULL,
    *undo_image = (Image *) NULL;

  switch (command)
  {
    case FreeBuffersCommand:
    {
      /*
        Free memory from the undo and redo cache.
      */
      while (undo_image != (Image *) NULL)
      {
        cache_image=undo_image;
        undo_image=GetPreviousImageInList(undo_image);
        cache_image->list=DestroyImage(cache_image->list);
        cache_image=DestroyImage(cache_image);
      }
      undo_image=NewImageList();
      if (redo_image != (Image *) NULL)
        redo_image=DestroyImage(redo_image);
      redo_image=NewImageList();
      return;
    }
    case UndoCommand:
    {
      char
        image_geometry[MagickPathExtent];

      /*
        Undo the last image transformation.
      */
      if (undo_image == (Image *) NULL)
        {
          (void) XBell(display,0);
          return;
        }
      cache_image=undo_image;
      undo_image=GetPreviousImageInList(undo_image);
      windows->image.window_changes.width=(int) cache_image->columns;
      windows->image.window_changes.height=(int) cache_image->rows;
      (void) FormatLocaleString(image_geometry,MagickPathExtent,"%dx%d!",
        windows->image.ximage->width,windows->image.ximage->height);
      (void) TransformImage(image,windows->image.crop_geometry,image_geometry,
        exception);
      if (windows->image.crop_geometry != (char *) NULL)
        windows->image.crop_geometry=(char *) RelinquishMagickMemory(
          windows->image.crop_geometry);
      windows->image.crop_geometry=cache_image->geometry;
      if (redo_image != (Image *) NULL)
        redo_image=DestroyImage(redo_image);
      redo_image=(*image);
      *image=cache_image->list;
      cache_image=DestroyImage(cache_image);
      if (windows->image.orphan != MagickFalse )
        return;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      return;
    }
    case CutCommand:
    case PasteCommand:
    case ApplyCommand:
    case HalfSizeCommand:
    case OriginalSizeCommand:
    case DoubleSizeCommand:
    case ResizeCommand:
    case TrimCommand:
    case CropCommand:
    case ChopCommand:
    case FlipCommand:
    case FlopCommand:
    case RotateRightCommand:
    case RotateLeftCommand:
    case RotateCommand:
    case ShearCommand:
    case RollCommand:
    case NegateCommand:
    case ContrastStretchCommand:
    case SigmoidalContrastCommand:
    case NormalizeCommand:
    case EqualizeCommand:
    case HueCommand:
    case SaturationCommand:
    case BrightnessCommand:
    case GammaCommand:
    case SpiffCommand:
    case DullCommand:
    case GrayscaleCommand:
    case MapCommand:
    case QuantizeCommand:
    case DespeckleCommand:
    case EmbossCommand:
    case ReduceNoiseCommand:
    case AddNoiseCommand:
    case SharpenCommand:
    case BlurCommand:
    case ThresholdCommand:
    case EdgeDetectCommand:
    case SpreadCommand:
    case ShadeCommand:
    case RaiseCommand:
    case SegmentCommand:
    case SolarizeCommand:
    case SepiaToneCommand:
    case SwirlCommand:
    case ImplodeCommand:
    case VignetteCommand:
    case WaveCommand:
    case OilPaintCommand:
    case CharcoalDrawCommand:
    case AnnotateCommand:
    case AddBorderCommand:
    case AddFrameCommand:
    case CompositeCommand:
    case CommentCommand:
    case LaunchCommand:
    case RegionofInterestCommand:
    case SaveToUndoBufferCommand:
    case RedoCommand:
    {
      Image
        *previous_image;

      ssize_t
        bytes;

      bytes=(ssize_t) ((*image)->columns*(*image)->rows*sizeof(PixelInfo));
      if (undo_image != (Image *) NULL)
        {
          /*
            Ensure the undo cache has enough memory available.
          */
          previous_image=undo_image;
          while (previous_image != (Image *) NULL)
          {
            bytes+=previous_image->list->columns*previous_image->list->rows*
              sizeof(PixelInfo);
            if (bytes <= (ssize_t) (resource_info->undo_cache << 20))
              {
                previous_image=GetPreviousImageInList(previous_image);
                continue;
              }
            bytes-=previous_image->list->columns*previous_image->list->rows*
              sizeof(PixelInfo);
            if (previous_image == undo_image)
              undo_image=NewImageList();
            else
              previous_image->next->previous=NewImageList();
            break;
          }
          while (previous_image != (Image *) NULL)
          {
            /*
              Delete any excess memory from undo cache.
            */
            cache_image=previous_image;
            previous_image=GetPreviousImageInList(previous_image);
            cache_image->list=DestroyImage(cache_image->list);
            cache_image=DestroyImage(cache_image);
          }
        }
      if (bytes > (ssize_t) (resource_info->undo_cache << 20))
        break;
      /*
        Save image before transformations are applied.
      */
      cache_image=AcquireImage((ImageInfo *) NULL,exception);
      if (cache_image == (Image *) NULL)
        break;
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      cache_image->list=CloneImage(*image,0,0,MagickTrue,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (cache_image->list == (Image *) NULL)
        {
          cache_image=DestroyImage(cache_image);
          break;
        }
      cache_image->columns=(size_t) windows->image.ximage->width;
      cache_image->rows=(size_t) windows->image.ximage->height;
      cache_image->geometry=windows->image.crop_geometry;
      if (windows->image.crop_geometry != (char *) NULL)
        {
          cache_image->geometry=AcquireString((char *) NULL);
          (void) CopyMagickString(cache_image->geometry,
            windows->image.crop_geometry,MagickPathExtent);
        }
      if (undo_image == (Image *) NULL)
        {
          undo_image=cache_image;
          break;
        }
      undo_image->next=cache_image;
      undo_image->next->previous=undo_image;
      undo_image=undo_image->next;
      break;
    }
    default:
      break;
  }
  if (command == RedoCommand)
    {
      /*
        Redo the last image transformation.
      */
      if (redo_image == (Image *) NULL)
        {
          (void) XBell(display,0);
          return;
        }
      windows->image.window_changes.width=(int) redo_image->columns;
      windows->image.window_changes.height=(int) redo_image->rows;
      if (windows->image.crop_geometry != (char *) NULL)
        windows->image.crop_geometry=(char *)
          RelinquishMagickMemory(windows->image.crop_geometry);
      windows->image.crop_geometry=redo_image->geometry;
      *image=DestroyImage(*image);
      *image=redo_image;
      redo_image=NewImageList();
      if (windows->image.orphan != MagickFalse )
        return;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      return;
    }
  if (command != InfoCommand)
    return;
  /*
    Display image info.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  XDisplayImageInfo(display,resource_info,windows,undo_image,*image,exception);
  XSetCursorState(display,windows,MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X I m a g e W i n d o w C o m m a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XImageWindowCommand() makes a transform to the image or Image window as
%  specified by a user menu button or keyboard command.
%
%  The format of the XImageWindowCommand method is:
%
%      CommandType XImageWindowCommand(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,
%        const MagickStatusType state,KeySym key_symbol,Image **image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o nexus:  Method XImageWindowCommand returns an image when the
%      user chooses 'Open Image' from the command menu.  Otherwise a null
%      image is returned.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o state: key mask.
%
%    o key_symbol: Specifies a command to perform.
%
%    o image: the image;  XImageWIndowCommand may transform the image and
%      return a new image pointer.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static CommandType XImageWindowCommand(Display *display,
  XResourceInfo *resource_info,XWindows *windows,const MagickStatusType state,
  KeySym key_symbol,Image **image,ExceptionInfo *exception)
{
  static char
    delta[MagickPathExtent] = "";

  static const char
    Digits[] = "01234567890";

  static KeySym
    last_symbol = XK_0;

  if ((key_symbol >= XK_0) && (key_symbol <= XK_9))
    {
      if (((last_symbol < XK_0) || (last_symbol > XK_9)))
        {
          *delta='\0';
          resource_info->quantum=1;
        }
      last_symbol=key_symbol;
      delta[strlen(delta)+1]='\0';
      delta[strlen(delta)]=Digits[key_symbol-XK_0];
      resource_info->quantum=StringToLong(delta);
      return(NullCommand);
    }
  last_symbol=key_symbol;
  if (resource_info->immutable)
    {
      /*
        Virtual image window has a restricted command set.
      */
      switch (key_symbol)
      {
        case XK_question:
          return(InfoCommand);
        case XK_p:
        case XK_Print:
          return(PrintCommand);
        case XK_space:
          return(NextCommand);
        case XK_q:
        case XK_Escape:
          return(QuitCommand);
        default:
          break;
      }
      return(NullCommand);
    }
  switch ((int) key_symbol)
  {
    case XK_o:
    {
      if ((state & ControlMask) == 0)
        break;
      return(OpenCommand);
    }
    case XK_space:
      return(NextCommand);
    case XK_BackSpace:
      return(FormerCommand);
    case XK_s:
    {
      if ((state & Mod1Mask) != 0)
        return(SwirlCommand);
      if ((state & ControlMask) == 0)
        return(ShearCommand);
      return(SaveCommand);
    }
    case XK_p:
    case XK_Print:
    {
      if ((state & Mod1Mask) != 0)
        return(OilPaintCommand);
      if ((state & Mod4Mask) != 0)
        return(ColorCommand);
      if ((state & ControlMask) == 0)
        return(NullCommand);
      return(PrintCommand);
    }
    case XK_d:
    {
      if ((state & Mod4Mask) != 0)
        return(DrawCommand);
      if ((state & ControlMask) == 0)
        return(NullCommand);
      return(DeleteCommand);
    }
    case XK_Select:
    {
      if ((state & ControlMask) == 0)
        return(NullCommand);
      return(SelectCommand);
    }
    case XK_n:
    {
      if ((state & ControlMask) == 0)
        return(NullCommand);
      return(NewCommand);
    }
    case XK_q:
    case XK_Escape:
      return(QuitCommand);
    case XK_z:
    case XK_Undo:
    {
      if ((state & ControlMask) == 0)
        return(NullCommand);
      return(UndoCommand);
    }
    case XK_r:
    case XK_Redo:
    {
      if ((state & ControlMask) == 0)
        return(RollCommand);
      return(RedoCommand);
    }
    case XK_x:
    {
      if ((state & ControlMask) == 0)
        return(NullCommand);
      return(CutCommand);
    }
    case XK_c:
    {
      if ((state & Mod1Mask) != 0)
        return(CharcoalDrawCommand);
      if ((state & ControlMask) == 0)
        return(CropCommand);
      return(CopyCommand);
    }
    case XK_v:
    case XK_Insert:
    {
      if ((state & Mod4Mask) != 0)
        return(CompositeCommand);
      if ((state & ControlMask) == 0)
        return(FlipCommand);
      return(PasteCommand);
    }
    case XK_less:
      return(HalfSizeCommand);
    case XK_minus:
      return(OriginalSizeCommand);
    case XK_greater:
      return(DoubleSizeCommand);
    case XK_percent:
      return(ResizeCommand);
    case XK_at:
      return(RefreshCommand);
    case XK_bracketleft:
      return(ChopCommand);
    case XK_h:
      return(FlopCommand);
    case XK_slash:
      return(RotateRightCommand);
    case XK_backslash:
      return(RotateLeftCommand);
    case XK_asterisk:
      return(RotateCommand);
    case XK_t:
      return(TrimCommand);
    case XK_H:
      return(HueCommand);
    case XK_S:
      return(SaturationCommand);
    case XK_L:
      return(BrightnessCommand);
    case XK_G:
      return(GammaCommand);
    case XK_C:
      return(SpiffCommand);
    case XK_Z:
      return(DullCommand);
    case XK_N:
      return(NormalizeCommand);
    case XK_equal:
      return(EqualizeCommand);
    case XK_asciitilde:
      return(NegateCommand);
    case XK_period:
      return(GrayscaleCommand);
    case XK_numbersign:
      return(QuantizeCommand);
    case XK_F2:
      return(DespeckleCommand);
    case XK_F3:
      return(EmbossCommand);
    case XK_F4:
      return(ReduceNoiseCommand);
    case XK_F5:
      return(AddNoiseCommand);
    case XK_F6:
      return(SharpenCommand);
    case XK_F7:
      return(BlurCommand);
    case XK_F8:
      return(ThresholdCommand);
    case XK_F9:
      return(EdgeDetectCommand);
    case XK_F10:
      return(SpreadCommand);
    case XK_F11:
      return(ShadeCommand);
    case XK_F12:
      return(RaiseCommand);
    case XK_F13:
      return(SegmentCommand);
    case XK_i:
    {
      if ((state & Mod1Mask) == 0)
        return(NullCommand);
      return(ImplodeCommand);
    }
    case XK_w:
    {
      if ((state & Mod1Mask) == 0)
        return(NullCommand);
      return(WaveCommand);
    }
    case XK_m:
    {
      if ((state & Mod4Mask) == 0)
        return(NullCommand);
      return(MatteCommand);
    }
    case XK_b:
    {
      if ((state & Mod4Mask) == 0)
        return(NullCommand);
      return(AddBorderCommand);
    }
    case XK_f:
    {
      if ((state & Mod4Mask) == 0)
        return(NullCommand);
      return(AddFrameCommand);
    }
    case XK_exclam:
    {
      if ((state & Mod4Mask) == 0)
        return(NullCommand);
      return(CommentCommand);
    }
    case XK_a:
    {
      if ((state & Mod1Mask) != 0)
        return(ApplyCommand);
      if ((state & Mod4Mask) != 0)
        return(AnnotateCommand);
      if ((state & ControlMask) == 0)
        return(NullCommand);
      return(RegionofInterestCommand);
    }
    case XK_question:
      return(InfoCommand);
    case XK_plus:
      return(ZoomCommand);
    case XK_P:
    {
      if ((state & ShiftMask) == 0)
        return(NullCommand);
      return(ShowPreviewCommand);
    }
    case XK_Execute:
      return(LaunchCommand);
    case XK_F1:
      return(HelpCommand);
    case XK_Find:
      return(BrowseDocumentationCommand);
    case XK_Menu:
    {
      (void) XMapRaised(display,windows->command.id);
      return(NullCommand);
    }
    case XK_Next:
    case XK_Prior:
    case XK_Home:
    case XK_KP_Home:
    {
      XTranslateImage(display,windows,*image,key_symbol);
      return(NullCommand);
    }
    case XK_Up:
    case XK_KP_Up:
    case XK_Down:
    case XK_KP_Down:
    case XK_Left:
    case XK_KP_Left:
    case XK_Right:
    case XK_KP_Right:
    {
      if ((state & Mod1Mask) != 0)
        {
          RectangleInfo
            crop_info;

          /*
            Trim one pixel from edge of image.
          */
          crop_info.x=0;
          crop_info.y=0;
          crop_info.width=(size_t) windows->image.ximage->width;
          crop_info.height=(size_t) windows->image.ximage->height;
          if ((key_symbol == XK_Up) || (key_symbol == XK_KP_Up))
            {
              if (resource_info->quantum >= (int) crop_info.height)
                resource_info->quantum=(int) crop_info.height-1;
              crop_info.height-=resource_info->quantum;
            }
          if ((key_symbol == XK_Down) || (key_symbol == XK_KP_Down))
            {
              if (resource_info->quantum >= (int) (crop_info.height-crop_info.y))
                resource_info->quantum=(int) (crop_info.height-crop_info.y-1);
              crop_info.y+=resource_info->quantum;
              crop_info.height-=resource_info->quantum;
            }
          if ((key_symbol == XK_Left) || (key_symbol == XK_KP_Left))
            {
              if (resource_info->quantum >= (int) crop_info.width)
                resource_info->quantum=(int) crop_info.width-1;
              crop_info.width-=resource_info->quantum;
            }
          if ((key_symbol == XK_Right) || (key_symbol == XK_KP_Right))
            {
              if (resource_info->quantum >= (int) (crop_info.width-crop_info.x))
                resource_info->quantum=(int) (crop_info.width-crop_info.x-1);
              crop_info.x+=resource_info->quantum;
              crop_info.width-=resource_info->quantum;
            }
          if ((int) (windows->image.x+windows->image.width) >
              (int) crop_info.width)
            windows->image.x=(int) (crop_info.width-windows->image.width);
          if ((int) (windows->image.y+windows->image.height) >
              (int) crop_info.height)
            windows->image.y=(int) (crop_info.height-windows->image.height);
          XSetCropGeometry(display,windows,&crop_info,*image);
          windows->image.window_changes.width=(int) crop_info.width;
          windows->image.window_changes.height=(int) crop_info.height;
          (void) XSetWindowBackgroundPixmap(display,windows->image.id,None);
          (void) XConfigureImage(display,resource_info,windows,*image,
            exception);
          return(NullCommand);
        }
      XTranslateImage(display,windows,*image,key_symbol);
      return(NullCommand);
    }
    default:
      return(NullCommand);
  }
  return(NullCommand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X M a g i c k C o m m a n d                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMagickCommand() makes a transform to the image or Image window as
%  specified by a user menu button or keyboard command.
%
%  The format of the XMagickCommand method is:
%
%      Image *XMagickCommand(Display *display,XResourceInfo *resource_info,
%        XWindows *windows,const CommandType command,Image **image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o command: Specifies a command to perform.
%
%    o image: the image;  XMagickCommand may transform the image and return a
%      new image pointer.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *XMagickCommand(Display *display,XResourceInfo *resource_info,
  XWindows *windows,const CommandType command,Image **image,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent],
    geometry[MagickPathExtent],
    modulate_factors[MagickPathExtent];

  GeometryInfo
    geometry_info;

  Image
    *nexus;

  ImageInfo
    *image_info;

  int
    x,
    y;

  MagickStatusType
    flags,
    status;

  QuantizeInfo
    quantize_info;

  RectangleInfo
    page_geometry;

  register int
    i;

  static char
    color[MagickPathExtent] = "gray";

  unsigned int
    height,
    width;

  /*
    Process user command.
  */
  XCheckRefreshWindows(display,windows);
  XImageCache(display,resource_info,windows,command,image,exception);
  nexus=NewImageList();
  windows->image.window_changes.width=windows->image.ximage->width;
  windows->image.window_changes.height=windows->image.ximage->height;
  image_info=CloneImageInfo(resource_info->image_info);
  SetGeometryInfo(&geometry_info);
  GetQuantizeInfo(&quantize_info);
  switch (command)
  {
    case OpenCommand:
    {
      /*
        Load image.
      */
      nexus=XOpenImage(display,resource_info,windows,MagickFalse);
      break;
    }
    case NextCommand:
    {
      /*
        Display next image.
      */
      for (i=0; i < resource_info->quantum; i++)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_next_image,CurrentTime);
      break;
    }
    case FormerCommand:
    {
      /*
        Display former image.
      */
      for (i=0; i < resource_info->quantum; i++)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_former_image,CurrentTime);
      break;
    }
    case SelectCommand:
    {
      int
        status;

      /*
        Select image.
      */
      if (*resource_info->home_directory == '\0')
        (void) CopyMagickString(resource_info->home_directory,".",
          MagickPathExtent);
      status=chdir(resource_info->home_directory);
      if (status == -1)
        (void) ThrowMagickException(exception,GetMagickModule(),FileOpenError,
          "UnableToOpenFile","%s",resource_info->home_directory);
      nexus=XOpenImage(display,resource_info,windows,MagickTrue);
      break;
    }
    case SaveCommand:
    {
      /*
        Save image.
      */
      status=XSaveImage(display,resource_info,windows,*image,exception);
      if (status == MagickFalse)
        {
          char
            message[MagickPathExtent];

          (void) FormatLocaleString(message,MagickPathExtent,"%s:%s",
            exception->reason != (char *) NULL ? exception->reason : "",
            exception->description != (char *) NULL ? exception->description :
            "");
          XNoticeWidget(display,windows,"Unable to save file:",message);
          break;
        }
      break;
    }
    case PrintCommand:
    {
      /*
        Print image.
      */
      status=XPrintImage(display,resource_info,windows,*image,exception);
      if (status == MagickFalse)
        {
          char
            message[MagickPathExtent];

          (void) FormatLocaleString(message,MagickPathExtent,"%s:%s",
            exception->reason != (char *) NULL ? exception->reason : "",
            exception->description != (char *) NULL ? exception->description :
            "");
          XNoticeWidget(display,windows,"Unable to print file:",message);
          break;
        }
      break;
    }
    case DeleteCommand:
    {
      static char
        filename[MagickPathExtent] = "\0";

      /*
        Delete image file.
      */
      XFileBrowserWidget(display,windows,"Delete",filename);
      if (*filename == '\0')
        break;
      status=ShredFile(filename);
      if (status != MagickFalse )
        XNoticeWidget(display,windows,"Unable to delete image file:",filename);
      break;
    }
    case NewCommand:
    {
      int
        status;

      static char
        color[MagickPathExtent] = "gray",
        geometry[MagickPathExtent] = "640x480";

      static const char
        *format = "gradient";

      /*
        Query user for canvas geometry.
      */
      status=XDialogWidget(display,windows,"New","Enter image geometry:",
        geometry);
      if (*geometry == '\0')
        break;
      if (status == 0)
        format="xc";
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      /*
        Create canvas.
      */
      (void) FormatLocaleString(image_info->filename,MagickPathExtent,
        "%s:%s",format,color);
      (void) CloneString(&image_info->size,geometry);
      nexus=ReadImage(image_info,exception);
      CatchException(exception);
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_next_image,CurrentTime);
      break;
    }
    case VisualDirectoryCommand:
    {
      /*
        Visual Image directory.
      */
      nexus=XVisualDirectoryImage(display,resource_info,windows,exception);
      break;
    }
    case QuitCommand:
    {
      /*
        exit program.
      */
      if (resource_info->confirm_exit == MagickFalse)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_exit,CurrentTime);
      else
        {
          int
            status;

          /*
            Confirm program exit.
          */
          status=XConfirmWidget(display,windows,"Do you really want to exit",
            resource_info->client_name);
          if (status > 0)
            XClientMessage(display,windows->image.id,windows->im_protocols,
              windows->im_exit,CurrentTime);
        }
      break;
    }
    case CutCommand:
    {
      /*
        Cut image.
      */
      (void) XCropImage(display,resource_info,windows,*image,CutMode,exception);
      break;
    }
    case CopyCommand:
    {
      /*
        Copy image.
      */
      (void) XCropImage(display,resource_info,windows,*image,CopyMode,
        exception);
      break;
    }
    case PasteCommand:
    {
      /*
        Paste image.
      */
      status=XPasteImage(display,resource_info,windows,*image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to paste X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case HalfSizeCommand:
    {
      /*
        Half image size.
      */
      windows->image.window_changes.width=windows->image.ximage->width/2;
      windows->image.window_changes.height=windows->image.ximage->height/2;
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case OriginalSizeCommand:
    {
      /*
        Original image size.
      */
      windows->image.window_changes.width=(int) (*image)->columns;
      windows->image.window_changes.height=(int) (*image)->rows;
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case DoubleSizeCommand:
    {
      /*
        Double the image size.
      */
      windows->image.window_changes.width=windows->image.ximage->width << 1;
      windows->image.window_changes.height=windows->image.ximage->height << 1;
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case ResizeCommand:
    {
      int
        status;

      size_t
        height,
        width;

      ssize_t
        x,
        y;

      /*
        Resize image.
      */
      width=(size_t) windows->image.ximage->width;
      height=(size_t) windows->image.ximage->height;
      x=0;
      y=0;
      (void) FormatLocaleString(geometry,MagickPathExtent,"%.20gx%.20g+0+0",
        (double) width,(double) height);
      status=XDialogWidget(display,windows,"Resize",
        "Enter resize geometry (e.g. 640x480, 200%):",geometry);
      if (*geometry == '\0')
        break;
      if (status == 0)
        (void) ConcatenateMagickString(geometry,"!",MagickPathExtent);
      (void) ParseMetaGeometry(geometry,&x,&y,&width,&height);
      windows->image.window_changes.width=(int) width;
      windows->image.window_changes.height=(int) height;
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case ApplyCommand:
    {
      char
        image_geometry[MagickPathExtent];

      if ((windows->image.crop_geometry == (char *) NULL) &&
          ((int) (*image)->columns == windows->image.ximage->width) &&
          ((int) (*image)->rows == windows->image.ximage->height))
        break;
      /*
        Apply size transforms to image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      /*
        Crop and/or scale displayed image.
      */
      (void) FormatLocaleString(image_geometry,MagickPathExtent,"%dx%d!",
        windows->image.ximage->width,windows->image.ximage->height);
      (void) TransformImage(image,windows->image.crop_geometry,image_geometry,
        exception);
      if (windows->image.crop_geometry != (char *) NULL)
        windows->image.crop_geometry=(char *) RelinquishMagickMemory(
          windows->image.crop_geometry);
      windows->image.x=0;
      windows->image.y=0;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case RefreshCommand:
    {
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case RestoreCommand:
    {
      /*
        Restore Image window to its original size.
      */
      if ((windows->image.width == (unsigned int) (*image)->columns) &&
          (windows->image.height == (unsigned int) (*image)->rows) &&
          (windows->image.crop_geometry == (char *) NULL))
        {
          (void) XBell(display,0);
          break;
        }
      windows->image.window_changes.width=(int) (*image)->columns;
      windows->image.window_changes.height=(int) (*image)->rows;
      if (windows->image.crop_geometry != (char *) NULL)
        {
          windows->image.crop_geometry=(char *)
            RelinquishMagickMemory(windows->image.crop_geometry);
          windows->image.crop_geometry=(char *) NULL;
          windows->image.x=0;
          windows->image.y=0;
        }
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case CropCommand:
    {
      /*
        Crop image.
      */
      (void) XCropImage(display,resource_info,windows,*image,CropMode,
        exception);
      break;
    }
    case ChopCommand:
    {
      /*
        Chop image.
      */
      status=XChopImage(display,resource_info,windows,image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to cut X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case FlopCommand:
    {
      Image
        *flop_image;

      /*
        Flop image scanlines.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flop_image=FlopImage(*image,exception);
      if (flop_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=flop_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.crop_geometry != (char *) NULL)
        {
          /*
            Flop crop geometry.
          */
          width=(unsigned int) (*image)->columns;
          height=(unsigned int) (*image)->rows;
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
          (void) FormatLocaleString(windows->image.crop_geometry,MagickPathExtent,
            "%ux%u%+d%+d",width,height,(int) (*image)->columns-(int) width-x,y);
        }
      if (windows->image.orphan != MagickFalse )
        break;
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case FlipCommand:
    {
      Image
        *flip_image;

      /*
        Flip image scanlines.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flip_image=FlipImage(*image,exception);
      if (flip_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=flip_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.crop_geometry != (char *) NULL)
        {
          /*
            Flip crop geometry.
          */
          width=(unsigned int) (*image)->columns;
          height=(unsigned int) (*image)->rows;
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
          (void) FormatLocaleString(windows->image.crop_geometry,MagickPathExtent,
            "%ux%u%+d%+d",width,height,x,(int) (*image)->rows-(int) height-y);
        }
      if (windows->image.orphan != MagickFalse )
        break;
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case RotateRightCommand:
    {
      /*
        Rotate image 90 degrees clockwise.
      */
      status=XRotateImage(display,resource_info,windows,90.0,image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to rotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case RotateLeftCommand:
    {
      /*
        Rotate image 90 degrees counter-clockwise.
      */
      status=XRotateImage(display,resource_info,windows,-90.0,image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to rotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case RotateCommand:
    {
      /*
        Rotate image.
      */
      status=XRotateImage(display,resource_info,windows,0.0,image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to rotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case ShearCommand:
    {
      Image
        *shear_image;

      static char
        geometry[MagickPathExtent] = "45.0x45.0";

      /*
        Query user for shear color and geometry.
      */
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      (void) XDialogWidget(display,windows,"Shear","Enter shear geometry:",
        geometry);
      if (*geometry == '\0')
        break;
      /*
        Shear image.
      */
      (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
        exception);
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) QueryColorCompliance(color,AllCompliance,
        &(*image)->background_color,exception);
      flags=ParseGeometry(geometry,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=geometry_info.rho;
      shear_image=ShearImage(*image,geometry_info.rho,geometry_info.sigma,
        exception);
      if (shear_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=shear_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      windows->image.window_changes.width=(int) (*image)->columns;
      windows->image.window_changes.height=(int) (*image)->rows;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case RollCommand:
    {
      Image
        *roll_image;

      static char
        geometry[MagickPathExtent] = "+2+2";

      /*
        Query user for the roll geometry.
      */
      (void) XDialogWidget(display,windows,"Roll","Enter roll geometry:",
        geometry);
      if (*geometry == '\0')
        break;
      /*
        Roll image.
      */
      (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
        exception);
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) ParsePageGeometry(*image,geometry,&page_geometry,
        exception);
      roll_image=RollImage(*image,page_geometry.x,page_geometry.y,
        exception);
      if (roll_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=roll_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      windows->image.window_changes.width=(int) (*image)->columns;
      windows->image.window_changes.height=(int) (*image)->rows;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case TrimCommand:
    {
      static char
        fuzz[MagickPathExtent];

      /*
        Query user for the fuzz factor.
      */
      (void) FormatLocaleString(fuzz,MagickPathExtent,"%g%%",100.0*
        (*image)->fuzz/(QuantumRange+1.0));
      (void) XDialogWidget(display,windows,"Trim","Enter fuzz factor:",fuzz);
      if (*fuzz == '\0')
        break;
      (*image)->fuzz=StringToDoubleInterval(fuzz,(double) QuantumRange+1.0);
      /*
        Trim image.
      */
      status=XTrimImage(display,resource_info,windows,*image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to trim X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case HueCommand:
    {
      static char
        hue_percent[MagickPathExtent] = "110";

      /*
        Query user for percent hue change.
      */
      (void) XDialogWidget(display,windows,"Apply",
        "Enter percent change in image hue (0-200):",hue_percent);
      if (*hue_percent == '\0')
        break;
      /*
        Vary the image hue.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) CopyMagickString(modulate_factors,"100.0/100.0/",MagickPathExtent);
      (void) ConcatenateMagickString(modulate_factors,hue_percent,
        MagickPathExtent);
      (void) ModulateImage(*image,modulate_factors,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SaturationCommand:
    {
      static char
        saturation_percent[MagickPathExtent] = "110";

      /*
        Query user for percent saturation change.
      */
      (void) XDialogWidget(display,windows,"Apply",
        "Enter percent change in color saturation (0-200):",saturation_percent);
      if (*saturation_percent == '\0')
        break;
      /*
        Vary color saturation.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) CopyMagickString(modulate_factors,"100.0/",MagickPathExtent);
      (void) ConcatenateMagickString(modulate_factors,saturation_percent,
        MagickPathExtent);
      (void) ModulateImage(*image,modulate_factors,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case BrightnessCommand:
    {
      static char
        brightness_percent[MagickPathExtent] = "110";

      /*
        Query user for percent brightness change.
      */
      (void) XDialogWidget(display,windows,"Apply",
        "Enter percent change in color brightness (0-200):",brightness_percent);
      if (*brightness_percent == '\0')
        break;
      /*
        Vary the color brightness.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) CopyMagickString(modulate_factors,brightness_percent,
        MagickPathExtent);
      (void) ModulateImage(*image,modulate_factors,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case GammaCommand:
    {
      static char
        factor[MagickPathExtent] = "1.6";

      /*
        Query user for gamma value.
      */
      (void) XDialogWidget(display,windows,"Gamma",
        "Enter gamma value (e.g. 1.2):",factor);
      if (*factor == '\0')
        break;
      /*
        Gamma correct image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) GammaImage(*image,strtod(factor,(char **) NULL),exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SpiffCommand:
    {
      /*
        Sharpen the image contrast.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) ContrastImage(*image,MagickTrue,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case DullCommand:
    {
      /*
        Dull the image contrast.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) ContrastImage(*image,MagickFalse,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case ContrastStretchCommand:
    {
      double
        black_point,
        white_point;

      static char
        levels[MagickPathExtent] = "1%";

      /*
        Query user for gamma value.
      */
      (void) XDialogWidget(display,windows,"Contrast Stretch",
        "Enter black and white points:",levels);
      if (*levels == '\0')
        break;
      /*
        Contrast stretch image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(levels,&geometry_info);
      black_point=geometry_info.rho;
      white_point=(flags & SigmaValue) != 0 ? geometry_info.sigma : black_point;
      if ((flags & PercentValue) != 0)
        {
          black_point*=(double) (*image)->columns*(*image)->rows/100.0;
          white_point*=(double) (*image)->columns*(*image)->rows/100.0;
        }
      white_point=(double) (*image)->columns*(*image)->rows-white_point;
      (void) ContrastStretchImage(*image,black_point,white_point,
        exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SigmoidalContrastCommand:
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      static char
        levels[MagickPathExtent] = "3x50%";

      /*
        Query user for gamma value.
      */
      (void) XDialogWidget(display,windows,"Sigmoidal Contrast",
        "Enter contrast and midpoint:",levels);
      if (*levels == '\0')
        break;
      /*
        Contrast stretch image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(levels,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=1.0*QuantumRange/2.0;
      if ((flags & PercentValue) != 0)
        geometry_info.sigma=1.0*QuantumRange*geometry_info.sigma/100.0;
      (void) SigmoidalContrastImage(*image,MagickTrue,geometry_info.rho,
        geometry_info.sigma,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case NormalizeCommand:
    {
      /*
        Perform histogram normalization on the image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) NormalizeImage(*image,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case EqualizeCommand:
    {
      /*
        Perform histogram equalization on the image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) EqualizeImage(*image,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case NegateCommand:
    {
      /*
        Negate colors in image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) NegateImage(*image,MagickFalse,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case GrayscaleCommand:
    {
      /*
        Convert image to grayscale.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) SetImageType(*image,(*image)->alpha_trait == UndefinedPixelTrait ?
        GrayscaleType : GrayscaleAlphaType,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case MapCommand:
    {
      Image
        *affinity_image;

      static char
        filename[MagickPathExtent] = "\0";

      /*
        Request image file name from user.
      */
      XFileBrowserWidget(display,windows,"Map",filename);
      if (*filename == '\0')
        break;
      /*
        Map image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
      affinity_image=ReadImage(image_info,exception);
      if (affinity_image != (Image *) NULL)
        {
          (void) RemapImage(&quantize_info,*image,affinity_image,exception);
          affinity_image=DestroyImage(affinity_image);
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case QuantizeCommand:
    {
      int
        status;

      static char
        colors[MagickPathExtent] = "256";

      /*
        Query user for maximum number of colors.
      */
      status=XDialogWidget(display,windows,"Quantize",
        "Maximum number of colors:",colors);
      if (*colors == '\0')
        break;
      /*
        Color reduce the image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      quantize_info.number_colors=StringToUnsignedLong(colors);
      quantize_info.dither_method=status != 0 ? RiemersmaDitherMethod :
        NoDitherMethod;
      (void) QuantizeImage(&quantize_info,*image,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case DespeckleCommand:
    {
      Image
        *despeckle_image;

      /*
        Despeckle image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      despeckle_image=DespeckleImage(*image,exception);
      if (despeckle_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=despeckle_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case EmbossCommand:
    {
      Image
        *emboss_image;

      static char
        radius[MagickPathExtent] = "0.0x1.0";

      /*
        Query user for emboss radius.
      */
      (void) XDialogWidget(display,windows,"Emboss",
        "Enter the emboss radius and standard deviation:",radius);
      if (*radius == '\0')
        break;
      /*
        Reduce noise in the image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(radius,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=1.0;
      emboss_image=EmbossImage(*image,geometry_info.rho,geometry_info.sigma,
        exception);
      if (emboss_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=emboss_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case ReduceNoiseCommand:
    {
      Image
        *noise_image;

      static char
        radius[MagickPathExtent] = "0";

      /*
        Query user for noise radius.
      */
      (void) XDialogWidget(display,windows,"Reduce Noise",
        "Enter the noise radius:",radius);
      if (*radius == '\0')
        break;
      /*
        Reduce noise in the image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(radius,&geometry_info);
      noise_image=StatisticImage(*image,NonpeakStatistic,(size_t)
        geometry_info.rho,(size_t) geometry_info.rho,exception);
      if (noise_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=noise_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case AddNoiseCommand:
    {
      char
        **noises;

      Image
        *noise_image;

      static char
        noise_type[MagickPathExtent] = "Gaussian";

      /*
        Add noise to the image.
      */
      noises=GetCommandOptions(MagickNoiseOptions);
      if (noises == (char **) NULL)
        break;
      XListBrowserWidget(display,windows,&windows->widget,
        (const char **) noises,"Add Noise",
        "Select a type of noise to add to your image:",noise_type);
      noises=DestroyStringList(noises);
      if (*noise_type == '\0')
        break;
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      noise_image=AddNoiseImage(*image,(NoiseType) ParseCommandOption(
        MagickNoiseOptions,MagickFalse,noise_type),1.0,exception);
      if (noise_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=noise_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SharpenCommand:
    {
      Image
        *sharp_image;

      static char
        radius[MagickPathExtent] = "0.0x1.0";

      /*
        Query user for sharpen radius.
      */
      (void) XDialogWidget(display,windows,"Sharpen",
        "Enter the sharpen radius and standard deviation:",radius);
      if (*radius == '\0')
        break;
      /*
        Sharpen image scanlines.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(radius,&geometry_info);
      sharp_image=SharpenImage(*image,geometry_info.rho,geometry_info.sigma,
        exception);
      if (sharp_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=sharp_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case BlurCommand:
    {
      Image
        *blur_image;

      static char
        radius[MagickPathExtent] = "0.0x1.0";

      /*
        Query user for blur radius.
      */
      (void) XDialogWidget(display,windows,"Blur",
        "Enter the blur radius and standard deviation:",radius);
      if (*radius == '\0')
        break;
      /*
        Blur an image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(radius,&geometry_info);
      blur_image=BlurImage(*image,geometry_info.rho,geometry_info.sigma,
        exception);
      if (blur_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=blur_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case ThresholdCommand:
    {
      double
        threshold;

      static char
        factor[MagickPathExtent] = "128";

      /*
        Query user for threshold value.
      */
      (void) XDialogWidget(display,windows,"Threshold",
        "Enter threshold value:",factor);
      if (*factor == '\0')
        break;
      /*
        Gamma correct image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      threshold=StringToDoubleInterval(factor,(double) QuantumRange+1.0);
      (void) BilevelImage(*image,threshold,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case EdgeDetectCommand:
    {
      Image
        *edge_image;

      static char
        radius[MagickPathExtent] = "0";

      /*
        Query user for edge factor.
      */
      (void) XDialogWidget(display,windows,"Detect Edges",
        "Enter the edge detect radius:",radius);
      if (*radius == '\0')
        break;
      /*
        Detect edge in image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(radius,&geometry_info);
      edge_image=EdgeImage(*image,geometry_info.rho,exception);
      if (edge_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=edge_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SpreadCommand:
    {
      Image
        *spread_image;

      static char
        amount[MagickPathExtent] = "2";

      /*
        Query user for spread amount.
      */
      (void) XDialogWidget(display,windows,"Spread",
        "Enter the displacement amount:",amount);
      if (*amount == '\0')
        break;
      /*
        Displace image pixels by a random amount.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(amount,&geometry_info);
      spread_image=EdgeImage(*image,geometry_info.rho,exception);
      if (spread_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=spread_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case ShadeCommand:
    {
      Image
        *shade_image;

      int
        status;

      static char
        geometry[MagickPathExtent] = "30x30";

      /*
        Query user for the shade geometry.
      */
      status=XDialogWidget(display,windows,"Shade",
        "Enter the azimuth and elevation of the light source:",geometry);
      if (*geometry == '\0')
        break;
      /*
        Shade image pixels.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(geometry,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=1.0;
      shade_image=ShadeImage(*image,status != 0 ? MagickTrue : MagickFalse,
        geometry_info.rho,geometry_info.sigma,exception);
      if (shade_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=shade_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case RaiseCommand:
    {
      static char
        bevel_width[MagickPathExtent] = "10";

      /*
        Query user for bevel width.
      */
      (void) XDialogWidget(display,windows,"Raise","Bevel width:",bevel_width);
      if (*bevel_width == '\0')
        break;
      /*
        Raise an image.
      */
      (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
        exception);
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) ParsePageGeometry(*image,bevel_width,&page_geometry,
        exception);
      (void) RaiseImage(*image,&page_geometry,MagickTrue,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SegmentCommand:
    {
      static char
        threshold[MagickPathExtent] = "1.0x1.5";

      /*
        Query user for smoothing threshold.
      */
      (void) XDialogWidget(display,windows,"Segment","Smooth threshold:",
        threshold);
      if (*threshold == '\0')
        break;
      /*
        Segment an image.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(threshold,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=1.0;
      (void) SegmentImage(*image,sRGBColorspace,MagickFalse,geometry_info.rho,
        geometry_info.sigma,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SepiaToneCommand:
    {
      double
        threshold;

      Image
        *sepia_image;

      static char
        factor[MagickPathExtent] = "80%";

      /*
        Query user for sepia-tone factor.
      */
      (void) XDialogWidget(display,windows,"Sepia Tone",
        "Enter the sepia tone factor (0 - 99.9%):",factor);
      if (*factor == '\0')
        break;
      /*
        Sepia tone image pixels.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      threshold=StringToDoubleInterval(factor,(double) QuantumRange+1.0);
      sepia_image=SepiaToneImage(*image,threshold,exception);
      if (sepia_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=sepia_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SolarizeCommand:
    {
      double
        threshold;

      static char
        factor[MagickPathExtent] = "60%";

      /*
        Query user for solarize factor.
      */
      (void) XDialogWidget(display,windows,"Solarize",
        "Enter the solarize factor (0 - 99.9%):",factor);
      if (*factor == '\0')
        break;
      /*
        Solarize image pixels.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      threshold=StringToDoubleInterval(factor,(double) QuantumRange+1.0);
      (void) SolarizeImage(*image,threshold,exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case SwirlCommand:
    {
      Image
        *swirl_image;

      static char
        degrees[MagickPathExtent] = "60";

      /*
        Query user for swirl angle.
      */
      (void) XDialogWidget(display,windows,"Swirl","Enter the swirl angle:",
        degrees);
      if (*degrees == '\0')
        break;
      /*
        Swirl image pixels about the center.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(degrees,&geometry_info);
      swirl_image=SwirlImage(*image,geometry_info.rho,(*image)->interpolate,
        exception);
      if (swirl_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=swirl_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case ImplodeCommand:
    {
      Image
        *implode_image;

      static char
        factor[MagickPathExtent] = "0.3";

      /*
        Query user for implode factor.
      */
      (void) XDialogWidget(display,windows,"Implode",
        "Enter the implosion/explosion factor (-1.0 - 1.0):",factor);
      if (*factor == '\0')
        break;
      /*
        Implode image pixels about the center.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(factor,&geometry_info);
      implode_image=ImplodeImage(*image,geometry_info.rho,(*image)->interpolate,
        exception);
      if (implode_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=implode_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case VignetteCommand:
    {
      Image
        *vignette_image;

      static char
        geometry[MagickPathExtent] = "0x20";

      /*
        Query user for the vignette geometry.
      */
      (void) XDialogWidget(display,windows,"Vignette",
        "Enter the radius, sigma, and x and y offsets:",geometry);
      if (*geometry == '\0')
        break;
      /*
        Soften the edges of the image in vignette style
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(geometry,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=1.0;
      if ((flags & XiValue) == 0)
        geometry_info.xi=0.1*(*image)->columns;
      if ((flags & PsiValue) == 0)
        geometry_info.psi=0.1*(*image)->rows;
      vignette_image=VignetteImage(*image,geometry_info.rho,0.0,(ssize_t)
        ceil(geometry_info.xi-0.5),(ssize_t) ceil(geometry_info.psi-0.5),
        exception);
      if (vignette_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=vignette_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case WaveCommand:
    {
      Image
        *wave_image;

      static char
        geometry[MagickPathExtent] = "25x150";

      /*
        Query user for the wave geometry.
      */
      (void) XDialogWidget(display,windows,"Wave",
        "Enter the amplitude and length of the wave:",geometry);
      if (*geometry == '\0')
        break;
      /*
        Alter an image along a sine wave.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(geometry,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=1.0;
      wave_image=WaveImage(*image,geometry_info.rho,geometry_info.sigma,
        (*image)->interpolate,exception);
      if (wave_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=wave_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case OilPaintCommand:
    {
      Image
        *paint_image;

      static char
        radius[MagickPathExtent] = "0";

      /*
        Query user for circular neighborhood radius.
      */
      (void) XDialogWidget(display,windows,"Oil Paint",
        "Enter the mask radius:",radius);
      if (*radius == '\0')
        break;
      /*
        OilPaint image scanlines.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(radius,&geometry_info);
      paint_image=OilPaintImage(*image,geometry_info.rho,geometry_info.sigma,
        exception);
      if (paint_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=paint_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case CharcoalDrawCommand:
    {
      Image
        *charcoal_image;

      static char
        radius[MagickPathExtent] = "0x1";

      /*
        Query user for charcoal radius.
      */
      (void) XDialogWidget(display,windows,"Charcoal Draw",
        "Enter the charcoal radius and sigma:",radius);
      if (*radius == '\0')
        break;
      /*
        Charcoal the image.
      */
      (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
        exception);
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      flags=ParseGeometry(radius,&geometry_info);
      if ((flags & SigmaValue) == 0)
        geometry_info.sigma=geometry_info.rho;
      charcoal_image=CharcoalImage(*image,geometry_info.rho,geometry_info.sigma,
        exception);
      if (charcoal_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=charcoal_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case AnnotateCommand:
    {
      /*
        Annotate the image with text.
      */
      status=XAnnotateEditImage(display,resource_info,windows,*image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to annotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case DrawCommand:
    {
      /*
        Draw image.
      */
      status=XDrawEditImage(display,resource_info,windows,image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to draw on the X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case ColorCommand:
    {
      /*
        Color edit.
      */
      status=XColorEditImage(display,resource_info,windows,image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to pixel edit X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case MatteCommand:
    {
      /*
        Matte edit.
      */
      status=XMatteEditImage(display,resource_info,windows,image,exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to matte edit X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case CompositeCommand:
    {
      /*
        Composite image.
      */
      status=XCompositeImage(display,resource_info,windows,*image,
        exception);
      if (status == MagickFalse)
        {
          XNoticeWidget(display,windows,"Unable to composite X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case AddBorderCommand:
    {
      Image
        *border_image;

      static char
        geometry[MagickPathExtent] = "6x6";

      /*
        Query user for border color and geometry.
      */
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      (void) XDialogWidget(display,windows,"Add Border",
        "Enter border geometry:",geometry);
      if (*geometry == '\0')
        break;
      /*
        Add a border to the image.
      */
      (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
        exception);
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) QueryColorCompliance(color,AllCompliance,&(*image)->border_color,
        exception);
      (void) ParsePageGeometry(*image,geometry,&page_geometry,
        exception);
      border_image=BorderImage(*image,&page_geometry,(*image)->compose,
        exception);
      if (border_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=border_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      windows->image.window_changes.width=(int) (*image)->columns;
      windows->image.window_changes.height=(int) (*image)->rows;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case AddFrameCommand:
    {
      FrameInfo
        frame_info;

      Image
        *frame_image;

      static char
        geometry[MagickPathExtent] = "6x6";

      /*
        Query user for frame color and geometry.
      */
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      (void) XDialogWidget(display,windows,"Add Frame","Enter frame geometry:",
        geometry);
      if (*geometry == '\0')
        break;
      /*
        Surround image with an ornamental border.
      */
      (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
        exception);
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) QueryColorCompliance(color,AllCompliance,&(*image)->matte_color,
        exception);
      (void) ParsePageGeometry(*image,geometry,&page_geometry,
        exception);
      frame_info.width=page_geometry.width;
      frame_info.height=page_geometry.height;
      frame_info.outer_bevel=page_geometry.x;
      frame_info.inner_bevel=page_geometry.y;
      frame_info.x=(ssize_t) frame_info.width;
      frame_info.y=(ssize_t) frame_info.height;
      frame_info.width=(*image)->columns+2*frame_info.width;
      frame_info.height=(*image)->rows+2*frame_info.height;
      frame_image=FrameImage(*image,&frame_info,(*image)->compose,exception);
      if (frame_image != (Image *) NULL)
        {
          *image=DestroyImage(*image);
          *image=frame_image;
        }
      CatchException(exception);
      XSetCursorState(display,windows,MagickFalse);
      if (windows->image.orphan != MagickFalse )
        break;
      windows->image.window_changes.width=(int) (*image)->columns;
      windows->image.window_changes.height=(int) (*image)->rows;
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
      (void) XConfigureImage(display,resource_info,windows,*image,exception);
      break;
    }
    case CommentCommand:
    {
      const char
        *value;

      FILE
        *file;

      int
        unique_file;

      /*
        Edit image comment.
      */
      unique_file=AcquireUniqueFileResource(image_info->filename);
      if (unique_file == -1)
        XNoticeWidget(display,windows,"Unable to edit image comment",
          image_info->filename);
      value=GetImageProperty(*image,"comment",exception);
      if (value == (char *) NULL)
        unique_file=close(unique_file)-1;
      else
        {
          register const char
            *p;

          file=fdopen(unique_file,"w");
          if (file == (FILE *) NULL)
            {
              XNoticeWidget(display,windows,"Unable to edit image comment",
                image_info->filename);
              break;
            }
          for (p=value; *p != '\0'; p++)
            (void) fputc((int) *p,file);
          (void) fputc('\n',file);
          (void) fclose(file);
        }
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      status=InvokeDelegate(image_info,*image,"edit",(char *) NULL,
        exception);
      if (status == MagickFalse)
        XNoticeWidget(display,windows,"Unable to edit image comment",
          (char *) NULL);
      else
        {
          char
            *comment;

          comment=FileToString(image_info->filename,~0UL,exception);
          if (comment != (char *) NULL)
            {
              (void) SetImageProperty(*image,"comment",comment,exception);
              (*image)->taint=MagickTrue;
            }
        }
      (void) RelinquishUniqueFileResource(image_info->filename);
      XSetCursorState(display,windows,MagickFalse);
      break;
    }
    case LaunchCommand:
    {
      /*
        Launch program.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) AcquireUniqueFilename(filename);
      (void) FormatLocaleString((*image)->filename,MagickPathExtent,"launch:%s",
        filename);
      status=WriteImage(image_info,*image,exception);
      if (status == MagickFalse)
        XNoticeWidget(display,windows,"Unable to launch image editor",
          (char *) NULL);
      else
        {
          nexus=ReadImage(resource_info->image_info,exception);
          CatchException(exception);
          XClientMessage(display,windows->image.id,windows->im_protocols,
            windows->im_next_image,CurrentTime);
        }
      (void) RelinquishUniqueFileResource(filename);
      XSetCursorState(display,windows,MagickFalse);
      break;
    }
    case RegionofInterestCommand:
    {
      /*
        Apply an image processing technique to a region of interest.
      */
      (void) XROIImage(display,resource_info,windows,image,exception);
      break;
    }
    case InfoCommand:
      break;
    case ZoomCommand:
    {
      /*
        Zoom image.
      */
      if (windows->magnify.mapped != MagickFalse )
        (void) XRaiseWindow(display,windows->magnify.id);
      else
        {
          /*
            Make magnify image.
          */
          XSetCursorState(display,windows,MagickTrue);
          (void) XMapRaised(display,windows->magnify.id);
          XSetCursorState(display,windows,MagickFalse);
        }
      break;
    }
    case ShowPreviewCommand:
    {
      char
        **previews,
        value[MagickPathExtent];

      Image
        *preview_image;

      PreviewType
        preview;

      static char
        preview_type[MagickPathExtent] = "Gamma";

      /*
        Select preview type from menu.
      */
      previews=GetCommandOptions(MagickPreviewOptions);
      if (previews == (char **) NULL)
        break;
      XListBrowserWidget(display,windows,&windows->widget,
        (const char **) previews,"Preview",
        "Select an enhancement, effect, or F/X:",preview_type);
      previews=DestroyStringList(previews);
      if (*preview_type == '\0')
        break;
      /*
        Show image preview.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      preview=(PreviewType) ParseCommandOption(MagickPreviewOptions,
        MagickFalse,preview_type);
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        windows->image.id);
      (void) SetImageProperty(*image,"group",value,exception);
      (void) DeleteImageProperty(*image,"label");
      (void) SetImageProperty(*image,"label","Preview",exception);
      preview_image=PreviewImage(*image,preview,exception);
      if (preview_image == (Image *) NULL)
        break;
      (void) AcquireUniqueFilename(filename);
      (void) FormatLocaleString(preview_image->filename,MagickPathExtent,
        "show:%s",filename);
      status=WriteImage(image_info,preview_image,exception);
      (void) RelinquishUniqueFileResource(filename);
      preview_image=DestroyImage(preview_image);
      if (status == MagickFalse)
        XNoticeWidget(display,windows,"Unable to show image preview",
          (*image)->filename);
      XDelay(display,1500);
      XSetCursorState(display,windows,MagickFalse);
      break;
    }
    case ShowHistogramCommand:
    {
      char
        value[MagickPathExtent];

      Image
        *histogram_image;

      /*
        Show image histogram.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        windows->image.id);
      (void) SetImageProperty(*image,"group",value,exception);
      (void) DeleteImageProperty(*image,"label");
      (void) SetImageProperty(*image,"label","Histogram",exception);
      (void) AcquireUniqueFilename(filename);
      (void) FormatLocaleString((*image)->filename,MagickPathExtent,"histogram:%s",
        filename);
      status=WriteImage(image_info,*image,exception);
      (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
      histogram_image=ReadImage(image_info,exception);
      (void) RelinquishUniqueFileResource(filename);
      if (histogram_image == (Image *) NULL)
        break;
      (void) FormatLocaleString(histogram_image->filename,MagickPathExtent,
        "show:%s",filename);
      status=WriteImage(image_info,histogram_image,exception);
      histogram_image=DestroyImage(histogram_image);
      if (status == MagickFalse)
        XNoticeWidget(display,windows,"Unable to show histogram",
          (*image)->filename);
      XDelay(display,1500);
      XSetCursorState(display,windows,MagickFalse);
      break;
    }
    case ShowMatteCommand:
    {
      char
        value[MagickPathExtent];

      Image
        *matte_image;

      if ((*image)->alpha_trait == UndefinedPixelTrait)
        {
          XNoticeWidget(display,windows,
            "Image does not have any matte information",(*image)->filename);
          break;
        }
      /*
        Show image matte.
      */
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      (void) FormatLocaleString(value,MaxTextExtent,"%.20g",(double)
        windows->image.id);
      (void) SetImageProperty(*image,"group",value,exception);
      (void) DeleteImageProperty(*image,"label");
      (void) SetImageProperty(*image,"label","Matte",exception);
      (void) AcquireUniqueFilename(filename);
      (void) FormatLocaleString((*image)->filename,MagickPathExtent,"matte:%s",
        filename);
      status=WriteImage(image_info,*image,exception);
      (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
      matte_image=ReadImage(image_info,exception);
      (void) RelinquishUniqueFileResource(filename);
      if (matte_image == (Image *) NULL)
        break;
      (void) FormatLocaleString(matte_image->filename,MagickPathExtent,"show:%s",
        filename);
      status=WriteImage(image_info,matte_image,exception);
      matte_image=DestroyImage(matte_image);
      if (status == MagickFalse)
        XNoticeWidget(display,windows,"Unable to show matte",
          (*image)->filename);
      XDelay(display,1500);
      XSetCursorState(display,windows,MagickFalse);
      break;
    }
    case BackgroundCommand:
    {
      /*
        Background image.
      */
      status=XBackgroundImage(display,resource_info,windows,image,exception);
      if (status == MagickFalse)
        break;
      nexus=CloneImage(*image,0,0,MagickTrue,exception);
      if (nexus != (Image *) NULL)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_next_image,CurrentTime);
      break;
    }
    case SlideShowCommand:
    {
      static char
        delay[MagickPathExtent] = "5";

      /*
        Display next image after pausing.
      */
      (void) XDialogWidget(display,windows,"Slide Show",
        "Pause how many 1/100ths of a second between images:",delay);
      if (*delay == '\0')
        break;
      resource_info->delay=StringToUnsignedLong(delay);
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_next_image,CurrentTime);
      break;
    }
    case PreferencesCommand:
    {
      /*
        Set user preferences.
      */
      status=XPreferencesWidget(display,resource_info,windows);
      if (status == MagickFalse)
        break;
      nexus=CloneImage(*image,0,0,MagickTrue,exception);
      if (nexus != (Image *) NULL)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_next_image,CurrentTime);
      break;
    }
    case HelpCommand:
    {
      /*
        User requested help.
      */
      XTextViewWidget(display,resource_info,windows,MagickFalse,
        "Help Viewer - Display",DisplayHelp);
      break;
    }
    case BrowseDocumentationCommand:
    {
      Atom
        mozilla_atom;

      Window
        mozilla_window,
        root_window;

      /*
        Browse the ImageMagick documentation.
      */
      root_window=XRootWindow(display,XDefaultScreen(display));
      mozilla_atom=XInternAtom(display,"_MOZILLA_VERSION",MagickFalse);
      mozilla_window=XWindowByProperty(display,root_window,mozilla_atom);
      if (mozilla_window != (Window) NULL)
        {
          char
            command[MagickPathExtent],
            *url;

          /*
            Display documentation using Netscape remote control.
          */
          url=GetMagickHomeURL();
          (void) FormatLocaleString(command,MagickPathExtent,
            "openurl(%s,new-tab)",url);
          url=DestroyString(url);
          mozilla_atom=XInternAtom(display,"_MOZILLA_COMMAND",MagickFalse);
          (void) XChangeProperty(display,mozilla_window,mozilla_atom,XA_STRING,
            8,PropModeReplace,(unsigned char *) command,(int) strlen(command));
          XSetCursorState(display,windows,MagickFalse);
          break;
        }
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      status=InvokeDelegate(image_info,*image,"browse",(char *) NULL,
        exception);
      if (status == MagickFalse)
        XNoticeWidget(display,windows,"Unable to browse documentation",
          (char *) NULL);
      XDelay(display,1500);
      XSetCursorState(display,windows,MagickFalse);
      break;
    }
    case VersionCommand:
    {
      XNoticeWidget(display,windows,GetMagickVersion((size_t *) NULL),
        GetMagickCopyright());
      break;
    }
    case SaveToUndoBufferCommand:
      break;
    default:
    {
      (void) XBell(display,0);
      break;
    }
  }
  image_info=DestroyImageInfo(image_info);
  return(nexus);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X M a g n i f y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMagnifyImage() magnifies portions of the image as indicated by the pointer.
%  The magnified portion is displayed in a separate window.
%
%  The format of the XMagnifyImage method is:
%
%      void XMagnifyImage(Display *display,XWindows *windows,XEvent *event,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XMagnifyImage(Display *display,XWindows *windows,XEvent *event,
  ExceptionInfo *exception)
{
  char
    text[MagickPathExtent];

  register int
    x,
    y;

  size_t
    state;

  /*
    Update magnified image until the mouse button is released.
  */
  (void) XCheckDefineCursor(display,windows->image.id,windows->magnify.cursor);
  state=DefaultState;
  x=event->xbutton.x;
  y=event->xbutton.y;
  windows->magnify.x=(int) windows->image.x+x;
  windows->magnify.y=(int) windows->image.y+y;
  do
  {
    /*
      Map and unmap Info widget as text cursor crosses its boundaries.
    */
    if (windows->info.mapped != MagickFalse )
      {
        if ((x < (int) (windows->info.x+windows->info.width)) &&
            (y < (int) (windows->info.y+windows->info.height)))
          (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      }
    else
      if ((x > (int) (windows->info.x+windows->info.width)) ||
          (y > (int) (windows->info.y+windows->info.height)))
        (void) XMapWindow(display,windows->info.id);
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+d%+d ",
          windows->magnify.x,windows->magnify.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,event,exception);
    switch (event->type)
    {
      case ButtonPress:
        break;
      case ButtonRelease:
      {
        /*
          User has finished magnifying image.
        */
        x=event->xbutton.x;
        y=event->xbutton.y;
        state|=ExitState;
        break;
      }
      case Expose:
        break;
      case MotionNotify:
      {
        x=event->xmotion.x;
        y=event->xmotion.y;
        break;
      }
      default:
        break;
    }
    /*
      Check boundary conditions.
    */
    if (x < 0)
      x=0;
    else
      if (x >= (int) windows->image.width)
        x=(int) windows->image.width-1;
    if (y < 0)
      y=0;
    else
     if (y >= (int) windows->image.height)
       y=(int) windows->image.height-1;
  } while ((state & ExitState) == 0);
  /*
    Display magnified image.
  */
  XSetCursorState(display,windows,MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X M a g n i f y W i n d o w C o m m a n d                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMagnifyWindowCommand() moves the image within an Magnify window by one
%  pixel as specified by the key symbol.
%
%  The format of the XMagnifyWindowCommand method is:
%
%      void XMagnifyWindowCommand(Display *display,XWindows *windows,
%        const MagickStatusType state,const KeySym key_symbol,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o state: key mask.
%
%    o key_symbol: Specifies a KeySym which indicates which side of the image
%      to trim.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XMagnifyWindowCommand(Display *display,XWindows *windows,
  const MagickStatusType state,const KeySym key_symbol,ExceptionInfo *exception)
{
  unsigned int
    quantum;

  /*
    User specified a magnify factor or position.
  */
  quantum=1;
  if ((state & Mod1Mask) != 0)
    quantum=10;
  switch ((int) key_symbol)
  {
    case QuitCommand:
    {
      (void) XWithdrawWindow(display,windows->magnify.id,
        windows->magnify.screen);
      break;
    }
    case XK_Home:
    case XK_KP_Home:
    {
      windows->magnify.x=(int) windows->image.width/2;
      windows->magnify.y=(int) windows->image.height/2;
      break;
    }
    case XK_Left:
    case XK_KP_Left:
    {
      if (windows->magnify.x > 0)
        windows->magnify.x-=quantum;
      break;
    }
    case XK_Up:
    case XK_KP_Up:
    {
      if (windows->magnify.y > 0)
        windows->magnify.y-=quantum;
      break;
    }
    case XK_Right:
    case XK_KP_Right:
    {
      if (windows->magnify.x < (int) (windows->image.ximage->width-1))
        windows->magnify.x+=quantum;
      break;
    }
    case XK_Down:
    case XK_KP_Down:
    {
      if (windows->magnify.y < (int) (windows->image.ximage->height-1))
        windows->magnify.y+=quantum;
      break;
    }
    case XK_0:
    case XK_1:
    case XK_2:
    case XK_3:
    case XK_4:
    case XK_5:
    case XK_6:
    case XK_7:
    case XK_8:
    case XK_9:
    {
      windows->magnify.data=(key_symbol-XK_0);
      break;
    }
    case XK_KP_0:
    case XK_KP_1:
    case XK_KP_2:
    case XK_KP_3:
    case XK_KP_4:
    case XK_KP_5:
    case XK_KP_6:
    case XK_KP_7:
    case XK_KP_8:
    case XK_KP_9:
    {
      windows->magnify.data=(key_symbol-XK_KP_0);
      break;
    }
    default:
      break;
  }
  XMakeMagnifyImage(display,windows,exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X M a k e P a n I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMakePanImage() creates a thumbnail of the image and displays it in the Pan
%  icon window.
%
%  The format of the XMakePanImage method is:
%
%        void XMakePanImage(Display *display,XResourceInfo *resource_info,
%          XWindows *windows,Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XMakePanImage(Display *display,XResourceInfo *resource_info,
  XWindows *windows,Image *image,ExceptionInfo *exception)
{
  MagickStatusType
    status;

  /*
    Create and display image for panning icon.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  windows->pan.x=(int) windows->image.x;
  windows->pan.y=(int) windows->image.y;
  status=XMakeImage(display,resource_info,&windows->pan,image,
    windows->pan.width,windows->pan.height,exception);
  if (status == MagickFalse)
    ThrowXWindowException(ResourceLimitError,
     "MemoryAllocationFailed",image->filename);
  (void) XSetWindowBackgroundPixmap(display,windows->pan.id,
    windows->pan.pixmap);
  (void) XClearWindow(display,windows->pan.id);
  XDrawPanRectangle(display,windows);
  XSetCursorState(display,windows,MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X M a t t a E d i t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XMatteEditImage() allows the user to interactively change the Matte channel
%  of an image.  If the image is PseudoClass it is promoted to DirectClass
%  before the matte information is stored.
%
%  The format of the XMatteEditImage method is:
%
%      MagickBooleanType XMatteEditImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image **image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XMatteEditImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image **image,
  ExceptionInfo *exception)
{
  static char
    matte[MagickPathExtent] = "0";

  static const char
    *MatteEditMenu[] =
    {
      "Method",
      "Border Color",
      "Fuzz",
      "Matte Value",
      "Undo",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static const ModeType
    MatteEditCommands[] =
    {
      MatteEditMethod,
      MatteEditBorderCommand,
      MatteEditFuzzCommand,
      MatteEditValueCommand,
      MatteEditUndoCommand,
      MatteEditHelpCommand,
      MatteEditDismissCommand
    };

  static PaintMethod
    method = PointMethod;

  static XColor
    border_color = { 0, 0, 0, 0, 0, 0 };

  char
    command[MagickPathExtent],
    text[MagickPathExtent];

  Cursor
    cursor;

  int
    entry,
    id,
    x,
    x_offset,
    y,
    y_offset;

  register int
    i;

  register Quantum
    *q;

  unsigned int
    height,
    width;

  size_t
    state;

  XEvent
    event;

  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"Matte Edit");
  windows->command.data=4;
  (void) XCommandWidget(display,windows,MatteEditMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Make cursor.
  */
  cursor=XMakeCursor(display,windows->image.id,windows->map_info->colormap,
    resource_info->background_color,resource_info->foreground_color);
  (void) XCheckDefineCursor(display,windows->image.id,cursor);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+d%+d ",
          x+windows->image.x,y+windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,MatteEditMenu,&event);
        if (id < 0)
          {
            (void) XCheckDefineCursor(display,windows->image.id,cursor);
            continue;
          }
        switch (MatteEditCommands[id])
        {
          case MatteEditMethod:
          {
            char
              **methods;

            /*
              Select a method from the pop-up menu.
            */
            methods=GetCommandOptions(MagickMethodOptions);
            if (methods == (char **) NULL)
              break;
            entry=XMenuWidget(display,windows,MatteEditMenu[id],
              (const char **) methods,command);
            if (entry >= 0)
              method=(PaintMethod) ParseCommandOption(MagickMethodOptions,
                MagickFalse,methods[entry]);
            methods=DestroyStringList(methods);
            break;
          }
          case MatteEditBorderCommand:
          {
            const char
              *ColorMenu[MaxNumberPens];

            int
              pen_number;

            /*
              Initialize menu selections.
            */
            for (i=0; i < (int) (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="Browser...";
            ColorMenu[MaxNumberPens-1]=(const char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,MatteEditMenu[id],
              (const char **) ColorMenu,command);
            if (pen_number < 0)
              break;
            if (pen_number == (MaxNumberPens-2))
              {
                static char
                  color_name[MagickPathExtent] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set border color.
            */
            (void) XParseColor(display,windows->map_info->colormap,
              resource_info->pen_colors[pen_number],&border_color);
            break;
          }
          case MatteEditFuzzCommand:
          {
            static char
              fuzz[MagickPathExtent];

            static const char
              *FuzzMenu[] =
              {
                "0%",
                "2%",
                "5%",
                "10%",
                "15%",
                "Dialog...",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            entry=XMenuWidget(display,windows,MatteEditMenu[id],FuzzMenu,
              command);
            if (entry < 0)
              break;
            if (entry != 5)
              {
                (*image)->fuzz=StringToDoubleInterval(FuzzMenu[entry],(double)
                  QuantumRange+1.0);
                break;
              }
            (void) CopyMagickString(fuzz,"20%",MagickPathExtent);
            (void) XDialogWidget(display,windows,"Ok",
              "Enter fuzz factor (0.0 - 99.9%):",fuzz);
            if (*fuzz == '\0')
              break;
            (void) ConcatenateMagickString(fuzz,"%",MagickPathExtent);
            (*image)->fuzz=StringToDoubleInterval(fuzz,(double) QuantumRange+
              1.0);
            break;
          }
          case MatteEditValueCommand:
          {
            static char
              message[MagickPathExtent];

            static const char
              *MatteMenu[] =
              {
                "Opaque",
                "Transparent",
                "Dialog...",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            entry=XMenuWidget(display,windows,MatteEditMenu[id],MatteMenu,
              command);
            if (entry < 0)
              break;
            if (entry != 2)
              {
                (void) FormatLocaleString(matte,MagickPathExtent,QuantumFormat,
                  OpaqueAlpha);
                if (LocaleCompare(MatteMenu[entry],"Transparent") == 0)
                  (void) FormatLocaleString(matte,MagickPathExtent,QuantumFormat,
                    (Quantum) TransparentAlpha);
                break;
              }
            (void) FormatLocaleString(message,MagickPathExtent,
              "Enter matte value (0 - " QuantumFormat "):",(Quantum)
              QuantumRange);
            (void) XDialogWidget(display,windows,"Matte",message,matte);
            if (*matte == '\0')
              break;
            break;
          }
          case MatteEditUndoCommand:
          {
            (void) XMagickCommand(display,resource_info,windows,UndoCommand,
              image,exception);
            break;
          }
          case MatteEditHelpCommand:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Matte Edit",ImageMatteEditHelp);
            break;
          }
          case MatteEditDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.button != Button1)
          break;
        if ((event.xbutton.window != windows->image.id) &&
            (event.xbutton.window != windows->magnify.id))
          break;
        /*
          Update matte data.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        (void) XMagickCommand(display,resource_info,windows,
          SaveToUndoBufferCommand,image,exception);
        state|=UpdateConfigurationState;
        break;
      }
      case ButtonRelease:
      {
        if (event.xbutton.button != Button1)
          break;
        if ((event.xbutton.window != windows->image.id) &&
            (event.xbutton.window != windows->magnify.id))
          break;
        /*
          Update colormap information.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        XConfigureImageColormap(display,resource_info,windows,*image,exception);
        (void) XConfigureImage(display,resource_info,windows,*image,exception);
        XInfoWidget(display,windows,text);
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        state&=(~UpdateConfigurationState);
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MagickPathExtent];

        KeySym
          key_symbol;

        if (event.xkey.window == windows->magnify.id)
          {
            Window
              window;

            window=windows->magnify.id;
            while (XCheckWindowEvent(display,window,KeyPressMask,&event)) ;
          }
        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Matte Edit",ImageMatteEditHelp);
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Map and unmap Info widget as cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
        break;
      }
      default:
        break;
    }
    if (event.xany.window == windows->magnify.id)
      {
        x=windows->magnify.x-windows->image.x;
        y=windows->magnify.y-windows->image.y;
      }
    x_offset=x;
    y_offset=y;
    if ((state & UpdateConfigurationState) != 0)
      {
        CacheView
          *image_view;

        int
          x,
          y;

        /*
          Matte edit is relative to image configuration.
        */
        (void) XClearArea(display,windows->image.id,x_offset,y_offset,1,1,
          MagickTrue);
        XPutPixel(windows->image.ximage,x_offset,y_offset,
          windows->pixel_info->background_color.pixel);
        width=(unsigned int) (*image)->columns;
        height=(unsigned int) (*image)->rows;
        x=0;
        y=0;
        if (windows->image.crop_geometry != (char *) NULL)
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,
            &height);
        x_offset=(int) (width*(windows->image.x+x_offset)/
          windows->image.ximage->width+x);
        y_offset=(int) (height*(windows->image.y+y_offset)/
          windows->image.ximage->height+y);
        if ((x_offset < 0) || (y_offset < 0))
          continue;
        if ((x_offset >= (int) (*image)->columns) ||
            (y_offset >= (int) (*image)->rows))
          continue;
        if (SetImageStorageClass(*image,DirectClass,exception) == MagickFalse)
          return(MagickFalse);
        if ((*image)->alpha_trait == UndefinedPixelTrait)
          (void) SetImageAlphaChannel(*image,OpaqueAlphaChannel,exception);
        image_view=AcquireAuthenticCacheView(*image,exception);
        switch (method)
        {
          case PointMethod:
          default:
          {
            /*
              Update matte information using point algorithm.
            */
            q=GetCacheViewAuthenticPixels(image_view,(ssize_t) x_offset,
              (ssize_t) y_offset,1,1,exception);
            if (q == (Quantum *) NULL)
              break;
            SetPixelAlpha(*image,(Quantum) StringToLong(matte),q);
            (void) SyncCacheViewAuthenticPixels(image_view,exception);
            break;
          }
          case ReplaceMethod:
          {
            PixelInfo
              pixel,
              target;

            /*
              Update matte information using replace algorithm.
            */
            (void) GetOneCacheViewVirtualPixelInfo(image_view,(ssize_t)
              x_offset,(ssize_t) y_offset,&target,exception);
            for (y=0; y < (int) (*image)->rows; y++)
            {
              q=GetCacheViewAuthenticPixels(image_view,0,(ssize_t) y,
                (*image)->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (int) (*image)->columns; x++)
              {
                GetPixelInfoPixel(*image,q,&pixel);
                if (IsFuzzyEquivalencePixelInfo(&pixel,&target))
                  SetPixelAlpha(*image,(Quantum) StringToLong(matte),q);
                q+=GetPixelChannels(*image);
              }
              if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
                break;
            }
            break;
          }
          case FloodfillMethod:
          case FillToBorderMethod:
          {
            ChannelType
              channel_mask;

            DrawInfo
              *draw_info;

            PixelInfo
              target;

            /*
              Update matte information using floodfill algorithm.
            */
            (void) GetOneVirtualPixelInfo(*image,
              GetPixelCacheVirtualMethod(*image),(ssize_t) x_offset,(ssize_t)
              y_offset,&target,exception);
            if (method == FillToBorderMethod)
              {
                target.red=(double) ScaleShortToQuantum(
                  border_color.red);
                target.green=(double) ScaleShortToQuantum(
                  border_color.green);
                target.blue=(double) ScaleShortToQuantum(
                  border_color.blue);
              }
            draw_info=CloneDrawInfo(resource_info->image_info,
              (DrawInfo *) NULL);
            draw_info->fill.alpha=(double) ClampToQuantum(
              StringToDouble(matte,(char **) NULL));
            channel_mask=SetImageChannelMask(*image,AlphaChannel);
            (void) FloodfillPaintImage(*image,draw_info,&target,(ssize_t)
              x_offset,(ssize_t) y_offset,
              method != FloodfillMethod ? MagickTrue : MagickFalse,exception);
            (void) SetPixelChannelMask(*image,channel_mask);
            draw_info=DestroyDrawInfo(draw_info);
            break;
          }
          case ResetMethod:
          {
            /*
              Update matte information using reset algorithm.
            */
            if (SetImageStorageClass(*image,DirectClass,exception) == MagickFalse)
              return(MagickFalse);
            for (y=0; y < (int) (*image)->rows; y++)
            {
              q=QueueCacheViewAuthenticPixels(image_view,0,(ssize_t) y,
                (*image)->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (int) (*image)->columns; x++)
              {
                SetPixelAlpha(*image,(Quantum) StringToLong(matte),q);
                q+=GetPixelChannels(*image);
              }
              if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
                break;
            }
            if (StringToLong(matte) == (long) OpaqueAlpha)
              (*image)->alpha_trait=UndefinedPixelTrait;
            break;
          }
        }
        image_view=DestroyCacheView(image_view);
        state&=(~UpdateConfigurationState);
      }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  XSetCursorState(display,windows,MagickFalse);
  (void) XFreeCursor(display,cursor);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X O p e n I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XOpenImage() loads an image from a file.
%
%  The format of the XOpenImage method is:
%
%     Image *XOpenImage(Display *display,XResourceInfo *resource_info,
%       XWindows *windows,const unsigned int command)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o command: A value other than zero indicates that the file is selected
%      from the command line argument list.
%
*/
static Image *XOpenImage(Display *display,XResourceInfo *resource_info,
  XWindows *windows,const MagickBooleanType command)
{
  const MagickInfo
    *magick_info;

  ExceptionInfo
    *exception;

  Image
    *nexus;

  ImageInfo
    *image_info;

  static char
    filename[MagickPathExtent] = "\0";

  /*
    Request file name from user.
  */
  if (command == MagickFalse)
    XFileBrowserWidget(display,windows,"Open",filename);
  else
    {
      char
        **filelist,
        **files;

      int
        count,
        status;

      register int
        i,
        j;

      /*
        Select next image from the command line.
      */
      status=XGetCommand(display,windows->image.id,&files,&count);
      if (status == 0)
        {
          ThrowXWindowException(XServerError,"UnableToGetProperty","...");
          return((Image *) NULL);
        }
      filelist=(char **) AcquireQuantumMemory((size_t) count,sizeof(*filelist));
      if (filelist == (char **) NULL)
        {
          ThrowXWindowException(ResourceLimitError,
            "MemoryAllocationFailed","...");
          (void) XFreeStringList(files);
          return((Image *) NULL);
        }
      j=0;
      for (i=1; i < count; i++)
        if (*files[i] != '-')
          filelist[j++]=files[i];
      filelist[j]=(char *) NULL;
      XListBrowserWidget(display,windows,&windows->widget,
        (const char **) filelist,"Load","Select Image to Load:",filename);
      filelist=(char **) RelinquishMagickMemory(filelist);
      (void) XFreeStringList(files);
    }
  if (*filename == '\0')
    return((Image *) NULL);
  image_info=CloneImageInfo(resource_info->image_info);
  (void) SetImageInfoProgressMonitor(image_info,(MagickProgressMonitor) NULL,
    (void *) NULL);
  (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
  exception=AcquireExceptionInfo();
  (void) SetImageInfo(image_info,0,exception);
  if (LocaleCompare(image_info->magick,"X") == 0)
    {
      char
        seconds[MagickPathExtent];

      /*
        User may want to delay the X server screen grab.
      */
      (void) CopyMagickString(seconds,"0",MagickPathExtent);
      (void) XDialogWidget(display,windows,"Grab","Enter any delay in seconds:",
        seconds);
      if (*seconds == '\0')
        return((Image *) NULL);
      XDelay(display,(size_t) (1000*StringToLong(seconds)));
    }
  magick_info=GetMagickInfo(image_info->magick,exception);
  if ((magick_info != (const MagickInfo *) NULL) &&
      GetMagickRawSupport(magick_info) == MagickTrue)
    {
      char
        geometry[MagickPathExtent];

      /*
        Request image size from the user.
      */
      (void) CopyMagickString(geometry,"512x512",MagickPathExtent);
      if (image_info->size != (char *) NULL)
        (void) CopyMagickString(geometry,image_info->size,MagickPathExtent);
      (void) XDialogWidget(display,windows,"Load","Enter the image geometry:",
        geometry);
      (void) CloneString(&image_info->size,geometry);
    }
  /*
    Load the image.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
  nexus=ReadImage(image_info,exception);
  CatchException(exception);
  XSetCursorState(display,windows,MagickFalse);
  if (nexus != (Image *) NULL)
    XClientMessage(display,windows->image.id,windows->im_protocols,
      windows->im_next_image,CurrentTime);
  else
    {
      char
        *text,
        **textlist;

      /*
        Unknown image format.
      */
      text=FileToString(filename,~0UL,exception);
      if (text == (char *) NULL)
        return((Image *) NULL);
      textlist=StringToList(text);
      if (textlist != (char **) NULL)
        {
          char
            title[MagickPathExtent];

          register int
            i;

          (void) FormatLocaleString(title,MagickPathExtent,
            "Unknown format: %s",filename);
          XTextViewWidget(display,resource_info,windows,MagickTrue,title,
            (const char **) textlist);
          for (i=0; textlist[i] != (char *) NULL; i++)
            textlist[i]=DestroyString(textlist[i]);
          textlist=(char **) RelinquishMagickMemory(textlist);
        }
      text=DestroyString(text);
    }
  exception=DestroyExceptionInfo(exception);
  image_info=DestroyImageInfo(image_info);
  return(nexus);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X P a n I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XPanImage() pans the image until the mouse button is released.
%
%  The format of the XPanImage method is:
%
%      void XPanImage(Display *display,XWindows *windows,XEvent *event,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static void XPanImage(Display *display,XWindows *windows,XEvent *event,
  ExceptionInfo *exception)
{
  char
    text[MagickPathExtent];

  Cursor
    cursor;

  double
    x_factor,
    y_factor;

  RectangleInfo
    pan_info;

  size_t
    state;

  /*
    Define cursor.
  */
  if ((windows->image.ximage->width > (int) windows->image.width) &&
      (windows->image.ximage->height > (int) windows->image.height))
    cursor=XCreateFontCursor(display,XC_fleur);
  else
    if (windows->image.ximage->width > (int) windows->image.width)
      cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
    else
      if (windows->image.ximage->height > (int) windows->image.height)
        cursor=XCreateFontCursor(display,XC_sb_v_double_arrow);
      else
        cursor=XCreateFontCursor(display,XC_arrow);
  (void) XCheckDefineCursor(display,windows->pan.id,cursor);
  /*
    Pan image as pointer moves until the mouse button is released.
  */
  x_factor=(double) windows->image.ximage->width/windows->pan.width;
  y_factor=(double) windows->image.ximage->height/windows->pan.height;
  pan_info.width=windows->pan.width*windows->image.width/
    windows->image.ximage->width;
  pan_info.height=windows->pan.height*windows->image.height/
    windows->image.ximage->height;
  pan_info.x=0;
  pan_info.y=0;
  state=UpdateConfigurationState;
  do
  {
    switch (event->type)
    {
      case ButtonPress:
      {
        /*
          User choose an initial pan location.
        */
        pan_info.x=(ssize_t) event->xbutton.x;
        pan_info.y=(ssize_t) event->xbutton.y;
        state|=UpdateConfigurationState;
        break;
      }
      case ButtonRelease:
      {
        /*
          User has finished panning the image.
        */
        pan_info.x=(ssize_t) event->xbutton.x;
        pan_info.y=(ssize_t) event->xbutton.y;
        state|=UpdateConfigurationState | ExitState;
        break;
      }
      case MotionNotify:
      {
        pan_info.x=(ssize_t) event->xmotion.x;
        pan_info.y=(ssize_t) event->xmotion.y;
        state|=UpdateConfigurationState;
      }
      default:
        break;
    }
    if ((state & UpdateConfigurationState) != 0)
      {
        /*
          Check boundary conditions.
        */
        if (pan_info.x < (ssize_t) (pan_info.width/2))
          pan_info.x=0;
        else
          pan_info.x=(ssize_t) (x_factor*(pan_info.x-(pan_info.width/2)));
        if (pan_info.x < 0)
          pan_info.x=0;
        else
          if ((int) (pan_info.x+windows->image.width) >
              windows->image.ximage->width)
            pan_info.x=(ssize_t)
              (windows->image.ximage->width-windows->image.width);
        if (pan_info.y < (ssize_t) (pan_info.height/2))
          pan_info.y=0;
        else
          pan_info.y=(ssize_t) (y_factor*(pan_info.y-(pan_info.height/2)));
        if (pan_info.y < 0)
          pan_info.y=0;
        else
          if ((int) (pan_info.y+windows->image.height) >
              windows->image.ximage->height)
            pan_info.y=(ssize_t)
              (windows->image.ximage->height-windows->image.height);
        if ((windows->image.x != (int) pan_info.x) ||
            (windows->image.y != (int) pan_info.y))
          {
            /*
              Display image pan offset.
            */
            windows->image.x=(int) pan_info.x;
            windows->image.y=(int) pan_info.y;
            (void) FormatLocaleString(text,MagickPathExtent," %ux%u%+d%+d ",
              windows->image.width,windows->image.height,windows->image.x,
              windows->image.y);
            XInfoWidget(display,windows,text);
            /*
              Refresh Image window.
            */
            XDrawPanRectangle(display,windows);
            XRefreshWindow(display,&windows->image,(XEvent *) NULL);
          }
        state&=(~UpdateConfigurationState);
      }
    /*
      Wait for next event.
    */
    if ((state & ExitState) == 0)
      XScreenEvent(display,windows,event,exception);
  } while ((state & ExitState) == 0);
  /*
    Restore cursor.
  */
  (void) XCheckDefineCursor(display,windows->pan.id,windows->pan.cursor);
  (void) XFreeCursor(display,cursor);
  (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X P a s t e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XPasteImage() pastes an image previously saved with XCropImage in the X
%  window image at a location the user chooses with the pointer.
%
%  The format of the XPasteImage method is:
%
%      MagickBooleanType XPasteImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XPasteImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  static const char
    *PasteMenu[] =
    {
      "Operator",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static const ModeType
    PasteCommands[] =
    {
      PasteOperatorsCommand,
      PasteHelpCommand,
      PasteDismissCommand
    };

  static CompositeOperator
    compose = CopyCompositeOp;

  char
    text[MagickPathExtent];

  Cursor
    cursor;

  Image
    *paste_image;

  int
    entry,
    id,
    x,
    y;

  double
    scale_factor;

  RectangleInfo
    highlight_info,
    paste_info;

  unsigned int
    height,
    width;

  size_t
    state;

  XEvent
    event;

  /*
    Copy image.
  */
  if (resource_info->copy_image == (Image *) NULL)
    return(MagickFalse);
  paste_image=CloneImage(resource_info->copy_image,0,0,MagickTrue,exception);
  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"Paste");
  windows->command.data=1;
  (void) XCommandWidget(display,windows,PasteMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XSetCursorState(display,windows,MagickFalse);
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  paste_info.x=(ssize_t) windows->image.x+x;
  paste_info.y=(ssize_t) windows->image.y+y;
  paste_info.width=0;
  paste_info.height=0;
  cursor=XCreateFontCursor(display,XC_ul_angle);
  (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
  state=DefaultState;
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+ld%+ld ",
          (long) paste_info.x,(long) paste_info.y);
        XInfoWidget(display,windows,text);
      }
    highlight_info=paste_info;
    highlight_info.x=paste_info.x-windows->image.x;
    highlight_info.y=paste_info.y-windows->image.y;
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,PasteMenu,&event);
        if (id < 0)
          continue;
        switch (PasteCommands[id])
        {
          case PasteOperatorsCommand:
          {
            char
              command[MagickPathExtent],
              **operators;

            /*
              Select a command from the pop-up menu.
            */
            operators=GetCommandOptions(MagickComposeOptions);
            if (operators == (char **) NULL)
              break;
            entry=XMenuWidget(display,windows,PasteMenu[id],
              (const char **) operators,command);
            if (entry >= 0)
              compose=(CompositeOperator) ParseCommandOption(
                MagickComposeOptions,MagickFalse,operators[entry]);
            operators=DestroyStringList(operators);
            break;
          }
          case PasteHelpCommand:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Composite",ImagePasteHelp);
            break;
          }
          case PasteDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Button Press: 0x%lx %u +%d+%d",event.xbutton.window,
            event.xbutton.button,event.xbutton.x,event.xbutton.y);
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        /*
          Paste rectangle is relative to image configuration.
        */
        width=(unsigned int) image->columns;
        height=(unsigned int) image->rows;
        x=0;
        y=0;
        if (windows->image.crop_geometry != (char *) NULL)
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
        scale_factor=(double) windows->image.ximage->width/width;
        paste_info.width=(unsigned int) (scale_factor*paste_image->columns+0.5);
        scale_factor=(double) windows->image.ximage->height/height;
        paste_info.height=(unsigned int) (scale_factor*paste_image->rows+0.5);
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        paste_info.x=(ssize_t) windows->image.x+event.xbutton.x;
        paste_info.y=(ssize_t) windows->image.y+event.xbutton.y;
        break;
      }
      case ButtonRelease:
      {
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Button Release: 0x%lx %u +%d+%d",event.xbutton.window,
            event.xbutton.button,event.xbutton.x,event.xbutton.y);
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        if ((paste_info.width != 0) && (paste_info.height != 0))
          {
            /*
              User has selected the location of the paste image.
            */
            paste_info.x=(ssize_t) windows->image.x+event.xbutton.x;
            paste_info.y=(ssize_t) windows->image.y+event.xbutton.y;
            state|=ExitState;
          }
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MagickPathExtent];

        KeySym
          key_symbol;

        int
          length;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Key press: 0x%lx (%s)",(long) key_symbol,command);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            paste_image=DestroyImage(paste_image);
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Image Composite",ImagePasteHelp);
            (void) XSetFunction(display,windows->image.highlight_context,
              GXinvert);
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
        paste_info.x=(ssize_t) windows->image.x+x;
        paste_info.y=(ssize_t) windows->image.y+y;
        break;
      }
      default:
      {
        if (image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),"Event type: %d",
            event.type);
        break;
      }
    }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSetCursorState(display,windows,MagickFalse);
  (void) XFreeCursor(display,cursor);
  if ((state & EscapeState) != 0)
    return(MagickTrue);
  /*
    Image pasting is relative to image configuration.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  x=0;
  y=0;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=(double) width/windows->image.ximage->width;
  paste_info.x+=x;
  paste_info.x=(ssize_t) (scale_factor*paste_info.x+0.5);
  paste_info.width=(unsigned int) (scale_factor*paste_info.width+0.5);
  scale_factor=(double) height/windows->image.ximage->height;
  paste_info.y+=y;
  paste_info.y=(ssize_t) (scale_factor*paste_info.y*scale_factor+0.5);
  paste_info.height=(unsigned int) (scale_factor*paste_info.height+0.5);
  /*
    Paste image with X Image window.
  */
  (void) CompositeImage(image,paste_image,compose,MagickTrue,paste_info.x,
    paste_info.y,exception);
  paste_image=DestroyImage(paste_image);
  XSetCursorState(display,windows,MagickFalse);
  /*
    Update image colormap.
  */
  XConfigureImageColormap(display,resource_info,windows,image,exception);
  (void) XConfigureImage(display,resource_info,windows,image,exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X P r i n t I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XPrintImage() prints an image to a Postscript printer.
%
%  The format of the XPrintImage method is:
%
%      MagickBooleanType XPrintImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XPrintImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent],
    geometry[MagickPathExtent];

  Image
    *print_image;

  ImageInfo
    *image_info;

  MagickStatusType
    status;

  /*
    Request Postscript page geometry from user.
  */
  image_info=CloneImageInfo(resource_info->image_info);
  (void) FormatLocaleString(geometry,MagickPathExtent,"Letter");
  if (image_info->page != (char *) NULL)
    (void) CopyMagickString(geometry,image_info->page,MagickPathExtent);
  XListBrowserWidget(display,windows,&windows->widget,PageSizes,"Select",
    "Select Postscript Page Geometry:",geometry);
  if (*geometry == '\0')
    return(MagickTrue);
  image_info->page=GetPageGeometry(geometry);
  /*
    Apply image transforms.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  print_image=CloneImage(image,0,0,MagickTrue,exception);
  if (print_image == (Image *) NULL)
    return(MagickFalse);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%dx%d!",
    windows->image.ximage->width,windows->image.ximage->height);
  (void) TransformImage(&print_image,windows->image.crop_geometry,geometry,
    exception);
  /*
    Print image.
  */
  (void) AcquireUniqueFilename(filename);
  (void) FormatLocaleString(print_image->filename,MagickPathExtent,"print:%s",
    filename);
  status=WriteImage(image_info,print_image,exception);
  (void) RelinquishUniqueFileResource(filename);
  print_image=DestroyImage(print_image);
  image_info=DestroyImageInfo(image_info);
  XSetCursorState(display,windows,MagickFalse);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X R O I I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XROIImage() applies an image processing technique to a region of interest.
%
%  The format of the XROIImage method is:
%
%      MagickBooleanType XROIImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image **image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XROIImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image **image,
  ExceptionInfo *exception)
{
#define ApplyMenus  7

  static const char
    *ROIMenu[] =
    {
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *ApplyMenu[] =
    {
      "File",
      "Edit",
      "Transform",
      "Enhance",
      "Effects",
      "F/X",
      "Miscellany",
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *FileMenu[] =
    {
      "Save...",
      "Print...",
      (char *) NULL
    },
    *EditMenu[] =
    {
      "Undo",
      "Redo",
      (char *) NULL
    },
    *TransformMenu[] =
    {
      "Flop",
      "Flip",
      "Rotate Right",
      "Rotate Left",
      (char *) NULL
    },
    *EnhanceMenu[] =
    {
      "Hue...",
      "Saturation...",
      "Brightness...",
      "Gamma...",
      "Spiff",
      "Dull",
      "Contrast Stretch...",
      "Sigmoidal Contrast...",
      "Normalize",
      "Equalize",
      "Negate",
      "Grayscale",
      "Map...",
      "Quantize...",
      (char *) NULL
    },
    *EffectsMenu[] =
    {
      "Despeckle",
      "Emboss",
      "Reduce Noise",
      "Add Noise",
      "Sharpen...",
      "Blur...",
      "Threshold...",
      "Edge Detect...",
      "Spread...",
      "Shade...",
      "Raise...",
      "Segment...",
      (char *) NULL
    },
    *FXMenu[] =
    {
      "Solarize...",
      "Sepia Tone...",
      "Swirl...",
      "Implode...",
      "Vignette...",
      "Wave...",
      "Oil Paint...",
      "Charcoal Draw...",
      (char *) NULL
    },
    *MiscellanyMenu[] =
    {
      "Image Info",
      "Zoom Image",
      "Show Preview...",
      "Show Histogram",
      "Show Matte",
      (char *) NULL
    };

  static const char
    **Menus[ApplyMenus] =
    {
      FileMenu,
      EditMenu,
      TransformMenu,
      EnhanceMenu,
      EffectsMenu,
      FXMenu,
      MiscellanyMenu
    };

  static const CommandType
    ApplyCommands[] =
    {
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      HelpCommand,
      QuitCommand
    },
    FileCommands[] =
    {
      SaveCommand,
      PrintCommand
    },
    EditCommands[] =
    {
      UndoCommand,
      RedoCommand
    },
    TransformCommands[] =
    {
      FlopCommand,
      FlipCommand,
      RotateRightCommand,
      RotateLeftCommand
    },
    EnhanceCommands[] =
    {
      HueCommand,
      SaturationCommand,
      BrightnessCommand,
      GammaCommand,
      SpiffCommand,
      DullCommand,
      ContrastStretchCommand,
      SigmoidalContrastCommand,
      NormalizeCommand,
      EqualizeCommand,
      NegateCommand,
      GrayscaleCommand,
      MapCommand,
      QuantizeCommand
    },
    EffectsCommands[] =
    {
      DespeckleCommand,
      EmbossCommand,
      ReduceNoiseCommand,
      AddNoiseCommand,
      SharpenCommand,
      BlurCommand,
      EdgeDetectCommand,
      SpreadCommand,
      ShadeCommand,
      RaiseCommand,
      SegmentCommand
    },
    FXCommands[] =
    {
      SolarizeCommand,
      SepiaToneCommand,
      SwirlCommand,
      ImplodeCommand,
      VignetteCommand,
      WaveCommand,
      OilPaintCommand,
      CharcoalDrawCommand
    },
    MiscellanyCommands[] =
    {
      InfoCommand,
      ZoomCommand,
      ShowPreviewCommand,
      ShowHistogramCommand,
      ShowMatteCommand
    },
    ROICommands[] =
    {
      ROIHelpCommand,
      ROIDismissCommand
    };

  static const CommandType
    *Commands[ApplyMenus] =
    {
      FileCommands,
      EditCommands,
      TransformCommands,
      EnhanceCommands,
      EffectsCommands,
      FXCommands,
      MiscellanyCommands
    };

  char
    command[MagickPathExtent],
    text[MagickPathExtent];

  CommandType
    command_type;

  Cursor
    cursor;

  Image
    *roi_image;

  int
    entry,
    id,
    x,
    y;

  double
    scale_factor;

  MagickProgressMonitor
    progress_monitor;

  RectangleInfo
    crop_info,
    highlight_info,
    roi_info;

  unsigned int
    height,
    width;

  size_t
    state;

  XEvent
    event;

  /*
    Map Command widget.
  */
  (void) CloneString(&windows->command.name,"ROI");
  windows->command.data=0;
  (void) XCommandWidget(display,windows,ROIMenu,(XEvent *) NULL);
  (void) XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask | PointerMotionMask);
  roi_info.x=(ssize_t) windows->image.x+x;
  roi_info.y=(ssize_t) windows->image.y+y;
  roi_info.width=0;
  roi_info.height=0;
  cursor=XCreateFontCursor(display,XC_fleur);
  state=DefaultState;
  do
  {
    if (windows->info.mapped != MagickFalse )
      {
        /*
          Display pointer position.
        */
        (void) FormatLocaleString(text,MagickPathExtent," %+ld%+ld ",
          (long) roi_info.x,(long) roi_info.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XScreenEvent(display,windows,&event,exception);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,ROIMenu,&event);
        if (id < 0)
          continue;
        switch (ROICommands[id])
        {
          case ROIHelpCommand:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Region of Interest",ImageROIHelp);
            break;
          }
          case ROIDismissCommand:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.button != Button1)
          break;
        if (event.xbutton.window != windows->image.id)
          break;
        /*
          Note first corner of region of interest rectangle-- exit loop.
        */
        (void) XCheckDefineCursor(display,windows->image.id,cursor);
        roi_info.x=(ssize_t) windows->image.x+event.xbutton.x;
        roi_info.y=(ssize_t) windows->image.y+event.xbutton.y;
        state|=ExitState;
        break;
      }
      case ButtonRelease:
        break;
      case Expose:
        break;
      case KeyPress:
      {
        KeySym
          key_symbol;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        switch ((int) key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,MagickFalse,
              "Help Viewer - Region of Interest",ImageROIHelp);
            break;
          }
          default:
          {
            (void) XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        x=event.xmotion.x;
        y=event.xmotion.y;
        if (windows->info.mapped != MagickFalse )
          {
            if ((x < (int) (windows->info.x+windows->info.width)) &&
                (y < (int) (windows->info.y+windows->info.height)))
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
          }
        else
          if ((x > (int) (windows->info.x+windows->info.width)) ||
              (y > (int) (windows->info.y+windows->info.height)))
            (void) XMapWindow(display,windows->info.id);
        roi_info.x=(ssize_t) windows->image.x+x;
        roi_info.y=(ssize_t) windows->image.y+y;
        break;
      }
      default:
        break;
    }
  } while ((state & ExitState) == 0);
  (void) XSelectInput(display,windows->image.id,
    windows->image.attributes.event_mask);
  if ((state & EscapeState) != 0)
    {
      /*
        User want to exit without region of interest.
      */
      (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      (void) XFreeCursor(display,cursor);
      return(MagickTrue);
    }
  (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
  do
  {
    /*
      Size rectangle as pointer moves until the mouse button is released.
    */
    x=(int) roi_info.x;
    y=(int) roi_info.y;
    roi_info.width=0;
    roi_info.height=0;
    state=DefaultState;
    do
    {
      highlight_info=roi_info;
      highlight_info.x=roi_info.x-windows->image.x;
      highlight_info.y=roi_info.y-windows->image.y;
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        {
          /*
            Display info and draw region of interest rectangle.
          */
          if (windows->info.mapped == MagickFalse)
            (void) XMapWindow(display,windows->info.id);
          (void) FormatLocaleString(text,MagickPathExtent,
            " %.20gx%.20g%+.20g%+.20g",(double) roi_info.width,(double)
            roi_info.height,(double) roi_info.x,(double) roi_info.y);
          XInfoWidget(display,windows,text);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
        }
      else
        if (windows->info.mapped != MagickFalse )
          (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      /*
        Wait for next event.
      */
      XScreenEvent(display,windows,&event,exception);
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        XHighlightRectangle(display,windows->image.id,
          windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          roi_info.x=(ssize_t) windows->image.x+event.xbutton.x;
          roi_info.y=(ssize_t) windows->image.y+event.xbutton.y;
          break;
        }
        case ButtonRelease:
        {
          /*
            User has committed to region of interest rectangle.
          */
          roi_info.x=(ssize_t) windows->image.x+event.xbutton.x;
          roi_info.y=(ssize_t) windows->image.y+event.xbutton.y;
          XSetCursorState(display,windows,MagickFalse);
          state|=ExitState;
          if (LocaleCompare(windows->command.name,"Apply") == 0)
            break;
          (void) CloneString(&windows->command.name,"Apply");
          windows->command.data=ApplyMenus;
          (void) XCommandWidget(display,windows,ApplyMenu,(XEvent *) NULL);
          break;
        }
        case Expose:
          break;
        case MotionNotify:
        {
          roi_info.x=(ssize_t) windows->image.x+event.xmotion.x;
          roi_info.y=(ssize_t) windows->image.y+event.xmotion.y;
        }
        default:
          break;
      }
      if ((((int) roi_info.x != x) && ((int) roi_info.y != y)) ||
          ((state & ExitState) != 0))
        {
          /*
            Check boundary conditions.
          */
          if (roi_info.x < 0)
            roi_info.x=0;
          else
            if (roi_info.x > (ssize_t) windows->image.ximage->width)
              roi_info.x=(ssize_t) windows->image.ximage->width;
          if ((int) roi_info.x < x)
            roi_info.width=(unsigned int) (x-roi_info.x);
          else
            {
              roi_info.width=(unsigned int) (roi_info.x-x);
              roi_info.x=(ssize_t) x;
            }
          if (roi_info.y < 0)
            roi_info.y=0;
          else
            if (roi_info.y > (ssize_t) windows->image.ximage->height)
              roi_info.y=(ssize_t) windows->image.ximage->height;
          if ((int) roi_info.y < y)
            roi_info.height=(unsigned int) (y-roi_info.y);
          else
            {
              roi_info.height=(unsigned int) (roi_info.y-y);
              roi_info.y=(ssize_t) y;
            }
        }
    } while ((state & ExitState) == 0);
    /*
      Wait for user to grab a corner of the rectangle or press return.
    */
    state=DefaultState;
    command_type=NullCommand;
    crop_info.x=0;
    crop_info.y=0;
    (void) XMapWindow(display,windows->info.id);
    do
    {
      if (windows->info.mapped != MagickFalse )
        {
          /*
            Display pointer position.
          */
          (void) FormatLocaleString(text,MagickPathExtent,
            " %.20gx%.20g%+.20g%+.20g",(double) roi_info.width,(double)
            roi_info.height,(double) roi_info.x,(double) roi_info.y);
          XInfoWidget(display,windows,text);
        }
      highlight_info=roi_info;
      highlight_info.x=roi_info.x-windows->image.x;
      highlight_info.y=roi_info.y-windows->image.y;
      if ((highlight_info.width <= 3) || (highlight_info.height <= 3))
        {
          state|=EscapeState;
          state|=ExitState;
          break;
        }
      if ((state & UpdateRegionState) != 0)
        {
          (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
          switch (command_type)
          {
            case UndoCommand:
            case RedoCommand:
            {
              (void) XMagickCommand(display,resource_info,windows,command_type,
                image,exception);
              break;
            }
            default:
            {
              /*
                Region of interest is relative to image configuration.
              */
              progress_monitor=SetImageProgressMonitor(*image,
                (MagickProgressMonitor) NULL,(*image)->client_data);
              crop_info=roi_info;
              width=(unsigned int) (*image)->columns;
              height=(unsigned int) (*image)->rows;
              x=0;
              y=0;
              if (windows->image.crop_geometry != (char *) NULL)
                (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
                  &width,&height);
              scale_factor=(double) width/windows->image.ximage->width;
              crop_info.x+=x;
              crop_info.x=(ssize_t) (scale_factor*crop_info.x+0.5);
              crop_info.width=(unsigned int) (scale_factor*crop_info.width+0.5);
              scale_factor=(double)
                height/windows->image.ximage->height;
              crop_info.y+=y;
              crop_info.y=(ssize_t) (scale_factor*crop_info.y+0.5);
              crop_info.height=(unsigned int)
                (scale_factor*crop_info.height+0.5);
              roi_image=CropImage(*image,&crop_info,exception);
              (void) SetImageProgressMonitor(*image,progress_monitor,
                (*image)->client_data);
              if (roi_image == (Image *) NULL)
                continue;
              /*
                Apply image processing technique to the region of interest.
              */
              windows->image.orphan=MagickTrue;
              (void) XMagickCommand(display,resource_info,windows,command_type,
                &roi_image,exception);
              progress_monitor=SetImageProgressMonitor(*image,
                (MagickProgressMonitor) NULL,(*image)->client_data);
              (void) XMagickCommand(display,resource_info,windows,
                SaveToUndoBufferCommand,image,exception);
              windows->image.orphan=MagickFalse;
              (void) CompositeImage(*image,roi_image,CopyCompositeOp,
                MagickTrue,crop_info.x,crop_info.y,exception);
              roi_image=DestroyImage(roi_image);
              (void) SetImageProgressMonitor(*image,progress_monitor,
                (*image)->client_data);
              break;
            }
          }
          if (command_type != InfoCommand)
            {
              XConfigureImageColormap(display,resource_info,windows,*image,
                exception);
              (void) XConfigureImage(display,resource_info,windows,*image,
                exception);
            }
          XCheckRefreshWindows(display,windows);
          XInfoWidget(display,windows,text);
          (void) XSetFunction(display,windows->image.highlight_context,
            GXinvert);
          state&=(~UpdateRegionState);
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      XScreenEvent(display,windows,&event,exception);
      if (event.xany.window == windows->command.id)
        {
          /*
            Select a command from the Command widget.
          */
          (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
          command_type=NullCommand;
          id=XCommandWidget(display,windows,ApplyMenu,&event);
          if (id >= 0)
            {
              (void) CopyMagickString(command,ApplyMenu[id],MagickPathExtent);
              command_type=ApplyCommands[id];
              if (id < ApplyMenus)
                {
                  /*
                    Select a command from a pop-up menu.
                  */
                  entry=XMenuWidget(display,windows,ApplyMenu[id],
                    (const char **) Menus[id],command);
                  if (entry >= 0)
                    {
                      (void) CopyMagickString(command,Menus[id][entry],
                        MagickPathExtent);
                      command_type=Commands[id][entry];
                    }
                }
            }
          (void) XSetFunction(display,windows->image.highlight_context,
            GXinvert);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
          if (command_type == HelpCommand)
            {
              (void) XSetFunction(display,windows->image.highlight_context,
                GXcopy);
              XTextViewWidget(display,resource_info,windows,MagickFalse,
                "Help Viewer - Region of Interest",ImageROIHelp);
              (void) XSetFunction(display,windows->image.highlight_context,
                GXinvert);
              continue;
            }
          if (command_type == QuitCommand)
            {
              /*
                exit.
              */
              state|=EscapeState;
              state|=ExitState;
              continue;
            }
          if (command_type != NullCommand)
            state|=UpdateRegionState;
          continue;
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          x=windows->image.x;
          y=windows->image.y;
          if (event.xbutton.button != Button1)
            break;
          if (event.xbutton.window != windows->image.id)
            break;
          x=windows->image.x+event.xbutton.x;
          y=windows->image.y+event.xbutton.y;
          if ((x < (int) (roi_info.x+RoiDelta)) &&
              (x > (int) (roi_info.x-RoiDelta)) &&
              (y < (int) (roi_info.y+RoiDelta)) &&
              (y > (int) (roi_info.y-RoiDelta)))
            {
              roi_info.x=(ssize_t) (roi_info.x+roi_info.width);
              roi_info.y=(ssize_t) (roi_info.y+roi_info.height);
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (int) (roi_info.x+RoiDelta)) &&
              (x > (int) (roi_info.x-RoiDelta)) &&
              (y < (int) (roi_info.y+roi_info.height+RoiDelta)) &&
              (y > (int) (roi_info.y+roi_info.height-RoiDelta)))
            {
              roi_info.x=(ssize_t) (roi_info.x+roi_info.width);
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (int) (roi_info.x+roi_info.width+RoiDelta)) &&
              (x > (int) (roi_info.x+roi_info.width-RoiDelta)) &&
              (y < (int) (roi_info.y+RoiDelta)) &&
              (y > (int) (roi_info.y-RoiDelta)))
            {
              roi_info.y=(ssize_t) (roi_info.y+roi_info.height);
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (int) (roi_info.x+roi_info.width+RoiDelta)) &&
              (x > (int) (roi_info.x+roi_info.width-RoiDelta)) &&
              (y < (int) (roi_info.y+roi_info.height+RoiDelta)) &&
              (y > (int) (roi_info.y+roi_info.height-RoiDelta)))
            {
              state|=UpdateConfigurationState;
              break;
            }
        }
        case ButtonRelease:
        {
          if (event.xbutton.window == windows->pan.id)
            if ((highlight_info.x != crop_info.x-windows->image.x) ||
                (highlight_info.y != crop_info.y-windows->image.y))
              XHighlightRectangle(display,windows->image.id,
                windows->image.highlight_context,&highlight_info);
          (void) XSetSelectionOwner(display,XA_PRIMARY,windows->image.id,
            event.xbutton.time);
          break;
        }
        case Expose:
        {
          if (event.xexpose.window == windows->image.id)
            if (event.xexpose.count == 0)
              {
                event.xexpose.x=(int) highlight_info.x;
                event.xexpose.y=(int) highlight_info.y;
                event.xexpose.width=(int) highlight_info.width;
                event.xexpose.height=(int) highlight_info.height;
                XRefreshWindow(display,&windows->image,&event);
              }
          if (event.xexpose.window == windows->info.id)
            if (event.xexpose.count == 0)
              XInfoWidget(display,windows,text);
          break;
        }
        case KeyPress:
        {
          KeySym
            key_symbol;

          if (event.xkey.window != windows->image.id)
            break;
          /*
            Respond to a user key press.
          */
          (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
            sizeof(command),&key_symbol,(XComposeStatus *) NULL);
          switch ((int) key_symbol)
          {
            case XK_Shift_L:
            case XK_Shift_R:
              break;
            case XK_Escape:
            case XK_F20:
              state|=EscapeState;
            case XK_Return:
            {
              state|=ExitState;
              break;
            }
            case XK_Home:
            case XK_KP_Home:
            {
              roi_info.x=(ssize_t) (windows->image.width/2L-roi_info.width/2L);
              roi_info.y=(ssize_t) (windows->image.height/2L-
                roi_info.height/2L);
              break;
            }
            case XK_Left:
            case XK_KP_Left:
            {
              roi_info.x--;
              break;
            }
            case XK_Up:
            case XK_KP_Up:
            case XK_Next:
            {
              roi_info.y--;
              break;
            }
            case XK_Right:
            case XK_KP_Right:
            {
              roi_info.x++;
              break;
            }
            case XK_Prior:
            case XK_Down:
            case XK_KP_Down:
            {
              roi_info.y++;
              break;
            }
            case XK_F1:
            case XK_Help:
            {
              (void) XSetFunction(display,windows->image.highlight_context,
                GXcopy);
              XTextViewWidget(display,resource_info,windows,MagickFalse,
                "Help Viewer - Region of Interest",ImageROIHelp);
              (void) XSetFunction(display,windows->image.highlight_context,
                GXinvert);
              break;
            }
            default:
            {
              command_type=XImageWindowCommand(display,resource_info,windows,
                event.xkey.state,key_symbol,image,exception);
              if (command_type != NullCommand)
                state|=UpdateRegionState;
              break;
            }
          }
          (void) XSetSelectionOwner(display,XA_PRIMARY,windows->image.id,
            event.xkey.time);
          break;
        }
        case KeyRelease:
          break;
        case MotionNotify:
        {
          if (event.xbutton.window != windows->image.id)
            break;
          /*
            Map and unmap Info widget as text cursor crosses its boundaries.
          */
          x=event.xmotion.x;
          y=event.xmotion.y;
          if (windows->info.mapped != MagickFalse )
            {
              if ((x < (int) (windows->info.x+windows->info.width)) &&
                  (y < (int) (windows->info.y+windows->info.height)))
                (void) XWithdrawWindow(display,windows->info.id,
                  windows->info.screen);
            }
          else
            if ((x > (int) (windows->info.x+windows->info.width)) ||
                (y > (int) (windows->info.y+windows->info.height)))
              (void) XMapWindow(display,windows->info.id);
          roi_info.x=(ssize_t) windows->image.x+event.xmotion.x;
          roi_info.y=(ssize_t) windows->image.y+event.xmotion.y;
          break;
        }
        case SelectionRequest:
        {
          XSelectionEvent
            notify;

          XSelectionRequestEvent
            *request;

          /*
            Set primary selection.
          */
          (void) FormatLocaleString(text,MagickPathExtent,
            "%.20gx%.20g%+.20g%+.20g",(double) roi_info.width,(double)
            roi_info.height,(double) roi_info.x,(double) roi_info.y);
          request=(&(event.xselectionrequest));
          (void) XChangeProperty(request->display,request->requestor,
            request->property,request->target,8,PropModeReplace,
            (unsigned char *) text,(int) strlen(text));
          notify.type=SelectionNotify;
          notify.display=request->display;
          notify.requestor=request->requestor;
          notify.selection=request->selection;
          notify.target=request->target;
          notify.time=request->time;
          if (request->property == None)
            notify.property=request->target;
          else
            notify.property=request->property;
          (void) XSendEvent(request->display,request->requestor,False,0,
            (XEvent *) &notify);
        }
        default:
          break;
      }
      if ((state & UpdateConfigurationState) != 0)
        {
          (void) XPutBackEvent(display,&event);
          (void) XCheckDefineCursor(display,windows->image.id,cursor);
          break;
        }
    } while ((state & ExitState) == 0);
  } while ((state & ExitState) == 0);
  (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSetCursorState(display,windows,MagickFalse);
  if ((state & EscapeState) != 0)
    return(MagickTrue);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X R o t a t e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XRotateImage() rotates the X image.  If the degrees parameter if zero, the
%  rotation angle is computed from the slope of a line drawn by the user.
%
%  The format of the XRotateImage method is:
%
%      MagickBooleanType XRotateImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,double degrees,
%        Image **image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o degrees: Specifies the number of degrees to rotate the image.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XRotateImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,double degrees,Image **image,
  ExceptionInfo *exception)
{
  static const char
    *RotateMenu[] =
    {
      "Pixel Color",
      "Direction",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  static ModeType
    direction = HorizontalRotateCommand;

  static const ModeType
    DirectionCommands[] =
    {
      HorizontalRotateCommand,
      VerticalRotateCommand
    },
    RotateCommands[] =
    {
      RotateColorCommand,
      RotateDirectionCommand,
      RotateHelpCommand,
      RotateDismissCommand
    };

  static unsigned int
    pen_id = 0;

  char
    command[MagickPathExtent],
    text[MagickPathExtent];

  Image
    *rotate_image;

  int
    id,
    x,
    y;

  double
    normalized_degrees;

  register int
    i;

  unsigned int
    height,
    rotations,
    width;

  if (degrees == 0.0)
    {
      unsigned int
        distance;

      size_t
        state;

      XEvent
        event;

      XSegment
        rotate_info;

      /*
        Map Command widget.
      */
      (void) CloneString(&windows->command.name,"Rotate");
      windows->command.data=2;
      (void) XCommandWidget(display,windows,RotateMenu,(XEvent *) NULL);
      (void) XMapRaised(display,windows->command.id);
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_update_widget,CurrentTime);
      /*
        Wait for first button press.
      */
      (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
      XQueryPosition(display,windows->image.id,&x,&y);
      rotate_info.x1=x;
      rotate_info.y1=y;
      rotate_info.x2=x;
      rotate_info.y2=y;
      state=DefaultState;
      do
      {
        XHighlightLine(display,windows->image.id,
          windows->image.highlight_context,&rotate_info);
        /*
          Wait for next event.
        */
        XScreenEvent(display,windows,&event,exception);
        XHighlightLine(display,windows->image.id,
          windows->image.highlight_context,&rotate_info);
        if (event.xany.window == windows->command.id)
          {
            /*
              Select a command from the Command widget.
            */
            id=XCommandWidget(display,windows,RotateMenu,&event);
            if (id < 0)
              continue;
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            switch (RotateCommands[id])
            {
              case RotateColorCommand:
              {
                const char
                  *ColorMenu[MaxNumberPens];

                int
                  pen_number;

                XColor
                  color;

                /*
                  Initialize menu selections.
                */
                for (i=0; i < (int) (MaxNumberPens-2); i++)
                  ColorMenu[i]=resource_info->pen_colors[i];
                ColorMenu[MaxNumberPens-2]="Browser...";
                ColorMenu[MaxNumberPens-1]=(const char *) NULL;
                /*
                  Select a pen color from the pop-up menu.
                */
                pen_number=XMenuWidget(display,windows,RotateMenu[id],
                  (const char **) ColorMenu,command);
                if (pen_number < 0)
                  break;
                if (pen_number == (MaxNumberPens-2))
                  {
                    static char
                      color_name[MagickPathExtent] = "gray";

                    /*
                      Select a pen color from a dialog.
                    */
                    resource_info->pen_colors[pen_number]=color_name;
                    XColorBrowserWidget(display,windows,"Select",color_name);
                    if (*color_name == '\0')
                      break;
                  }
                /*
                  Set pen color.
                */
                (void) XParseColor(display,windows->map_info->colormap,
                  resource_info->pen_colors[pen_number],&color);
                XBestPixel(display,windows->map_info->colormap,(XColor *) NULL,
                  (unsigned int) MaxColors,&color);
                windows->pixel_info->pen_colors[pen_number]=color;
                pen_id=(unsigned int) pen_number;
                break;
              }
              case RotateDirectionCommand:
              {
                static const char
                  *Directions[] =
                  {
                    "horizontal",
                    "vertical",
                    (char *) NULL,
                  };

                /*
                  Select a command from the pop-up menu.
                */
                id=XMenuWidget(display,windows,RotateMenu[id],
                  Directions,command);
                if (id >= 0)
                  direction=DirectionCommands[id];
                break;
              }
              case RotateHelpCommand:
              {
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Rotation",ImageRotateHelp);
                break;
              }
              case RotateDismissCommand:
              {
                /*
                  Prematurely exit.
                */
                state|=EscapeState;
                state|=ExitState;
                break;
              }
              default:
                break;
            }
            (void) XSetFunction(display,windows->image.highlight_context,
              GXinvert);
            continue;
          }
        switch (event.type)
        {
          case ButtonPress:
          {
            if (event.xbutton.button != Button1)
              break;
            if (event.xbutton.window != windows->image.id)
              break;
            /*
              exit loop.
            */
            (void) XSetFunction(display,windows->image.highlight_context,
              GXcopy);
            rotate_info.x1=event.xbutton.x;
            rotate_info.y1=event.xbutton.y;
            state|=ExitState;
            break;
          }
          case ButtonRelease:
            break;
          case Expose:
            break;
          case KeyPress:
          {
            char
              command[MagickPathExtent];

            KeySym
              key_symbol;

            if (event.xkey.window != windows->image.id)
              break;
            /*
              Respond to a user key press.
            */
            (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
              sizeof(command),&key_symbol,(XComposeStatus *) NULL);
            switch ((int) key_symbol)
            {
              case XK_Escape:
              case XK_F20:
              {
                /*
                  Prematurely exit.
                */
                state|=EscapeState;
                state|=ExitState;
                break;
              }
              case XK_F1:
              case XK_Help:
              {
                (void) XSetFunction(display,windows->image.highlight_context,
                  GXcopy);
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Rotation",ImageRotateHelp);
                (void) XSetFunction(display,windows->image.highlight_context,
                  GXinvert);
                break;
              }
              default:
              {
                (void) XBell(display,0);
                break;
              }
            }
            break;
          }
          case MotionNotify:
          {
            rotate_info.x1=event.xmotion.x;
            rotate_info.y1=event.xmotion.y;
          }
        }
        rotate_info.x2=rotate_info.x1;
        rotate_info.y2=rotate_info.y1;
        if (direction == HorizontalRotateCommand)
          rotate_info.x2+=32;
        else
          rotate_info.y2-=32;
      } while ((state & ExitState) == 0);
      (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
      (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      if ((state & EscapeState) != 0)
        return(MagickTrue);
      /*
        Draw line as pointer moves until the mouse button is released.
      */
      distance=0;
      (void) XSetFunction(display,windows->image.highlight_context,GXinvert);
      state=DefaultState;
      do
      {
        if (distance > 9)
          {
            /*
              Display info and draw rotation line.
            */
            if (windows->info.mapped == MagickFalse)
              (void) XMapWindow(display,windows->info.id);
            (void) FormatLocaleString(text,MagickPathExtent," %g",
              direction == VerticalRotateCommand ? degrees-90.0 : degrees);
            XInfoWidget(display,windows,text);
            XHighlightLine(display,windows->image.id,
              windows->image.highlight_context,&rotate_info);
          }
        else
          if (windows->info.mapped != MagickFalse )
            (void) XWithdrawWindow(display,windows->info.id,
              windows->info.screen);
        /*
          Wait for next event.
        */
        XScreenEvent(display,windows,&event,exception);
        if (distance > 9)
          XHighlightLine(display,windows->image.id,
            windows->image.highlight_context,&rotate_info);
        switch (event.type)
        {
          case ButtonPress:
            break;
          case ButtonRelease:
          {
            /*
              User has committed to rotation line.
            */
            rotate_info.x2=event.xbutton.x;
            rotate_info.y2=event.xbutton.y;
            state|=ExitState;
            break;
          }
          case Expose:
            break;
          case MotionNotify:
          {
            rotate_info.x2=event.xmotion.x;
            rotate_info.y2=event.xmotion.y;
          }
          default:
            break;
        }
        /*
          Check boundary conditions.
        */
        if (rotate_info.x2 < 0)
          rotate_info.x2=0;
        else
          if (rotate_info.x2 > (int) windows->image.width)
            rotate_info.x2=(short) windows->image.width;
        if (rotate_info.y2 < 0)
          rotate_info.y2=0;
        else
          if (rotate_info.y2 > (int) windows->image.height)
            rotate_info.y2=(short) windows->image.height;
        /*
          Compute rotation angle from the slope of the line.
        */
        degrees=0.0;
        distance=(unsigned int)
          ((rotate_info.x2-rotate_info.x1+1)*(rotate_info.x2-rotate_info.x1+1))+
          ((rotate_info.y2-rotate_info.y1+1)*(rotate_info.y2-rotate_info.y1+1));
        if (distance > 9)
          degrees=RadiansToDegrees(-atan2((double) (rotate_info.y2-
            rotate_info.y1),(double) (rotate_info.x2-rotate_info.x1)));
      } while ((state & ExitState) == 0);
      (void) XSetFunction(display,windows->image.highlight_context,GXcopy);
      (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      if (distance <= 9)
        return(MagickTrue);
    }
  if (direction == VerticalRotateCommand)
    degrees-=90.0;
  if (degrees == 0.0)
    return(MagickTrue);
  /*
    Rotate image.
  */
  normalized_degrees=degrees;
  while (normalized_degrees < -45.0)
    normalized_degrees+=360.0;
  for (rotations=0; normalized_degrees > 45.0; rotations++)
    normalized_degrees-=90.0;
  if (normalized_degrees != 0.0)
    (void) XMagickCommand(display,resource_info,windows,ApplyCommand,image,
      exception);
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  (*image)->background_color.red=(double) ScaleShortToQuantum(
    windows->pixel_info->pen_colors[pen_id].red);
  (*image)->background_color.green=(double) ScaleShortToQuantum(
    windows->pixel_info->pen_colors[pen_id].green);
  (*image)->background_color.blue=(double) ScaleShortToQuantum(
    windows->pixel_info->pen_colors[pen_id].blue);
  rotate_image=RotateImage(*image,degrees,exception);
  XSetCursorState(display,windows,MagickFalse);
  if (rotate_image == (Image *) NULL)
    return(MagickFalse);
  *image=DestroyImage(*image);
  *image=rotate_image;
  if (windows->image.crop_geometry != (char *) NULL)
    {
      /*
        Rotate crop geometry.
      */
      width=(unsigned int) (*image)->columns;
      height=(unsigned int) (*image)->rows;
      (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
      switch (rotations % 4)
      {
        default:
        case 0:
          break;
        case 1:
        {
          /*
            Rotate 90 degrees.
          */
          (void) FormatLocaleString(windows->image.crop_geometry,MagickPathExtent,
            "%ux%u%+d%+d",height,width,(int) (*image)->columns-
            (int) height-y,x);
          break;
        }
        case 2:
        {
          /*
            Rotate 180 degrees.
          */
          (void) FormatLocaleString(windows->image.crop_geometry,MagickPathExtent,
            "%ux%u%+d%+d",width,height,(int) width-x,(int) height-y);
          break;
        }
        case 3:
        {
          /*
            Rotate 270 degrees.
          */
          (void) FormatLocaleString(windows->image.crop_geometry,MagickPathExtent,
            "%ux%u%+d%+d",height,width,y,(int) (*image)->rows-(int) width-x);
          break;
        }
      }
    }
  if (windows->image.orphan != MagickFalse )
    return(MagickTrue);
  if (normalized_degrees != 0.0)
    {
      /*
        Update image colormap.
      */
      windows->image.window_changes.width=(int) (*image)->columns;
      windows->image.window_changes.height=(int) (*image)->rows;
      if (windows->image.crop_geometry != (char *) NULL)
        {
          /*
            Obtain dimensions of image from crop geometry.
          */
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
          windows->image.window_changes.width=(int) width;
          windows->image.window_changes.height=(int) height;
        }
      XConfigureImageColormap(display,resource_info,windows,*image,exception);
    }
  else
    if (((rotations % 4) == 1) || ((rotations % 4) == 3))
      {
        windows->image.window_changes.width=windows->image.ximage->height;
        windows->image.window_changes.height=windows->image.ximage->width;
      }
  /*
    Update image configuration.
  */
  (void) XConfigureImage(display,resource_info,windows,*image,exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X S a v e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XSaveImage() saves an image to a file.
%
%  The format of the XSaveImage method is:
%
%      MagickBooleanType XSaveImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XSaveImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent],
    geometry[MagickPathExtent];

  Image
    *save_image;

  ImageInfo
    *image_info;

  MagickStatusType
    status;

  /*
    Request file name from user.
  */
  if (resource_info->write_filename != (char *) NULL)
    (void) CopyMagickString(filename,resource_info->write_filename,
      MagickPathExtent);
  else
    {
      char
        path[MagickPathExtent];

      int
        status;

      GetPathComponent(image->filename,HeadPath,path);
      GetPathComponent(image->filename,TailPath,filename);
      if (*path != '\0')
        {
          status=chdir(path);
          if (status == -1)
            (void) ThrowMagickException(exception,GetMagickModule(),
              FileOpenError,"UnableToOpenFile","%s",path);
        }
    }
  XFileBrowserWidget(display,windows,"Save",filename);
  if (*filename == '\0')
    return(MagickTrue);
  if (IsPathAccessible(filename) != MagickFalse )
    {
      int
        status;

      /*
        File exists-- seek user's permission before overwriting.
      */
      status=XConfirmWidget(display,windows,"Overwrite",filename);
      if (status <= 0)
        return(MagickTrue);
    }
  image_info=CloneImageInfo(resource_info->image_info);
  (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
  (void) SetImageInfo(image_info,1,exception);
  if ((LocaleCompare(image_info->magick,"JPEG") == 0) ||
      (LocaleCompare(image_info->magick,"JPG") == 0))
    {
      char
        quality[MagickPathExtent];

      int
        status;

      /*
        Request JPEG quality from user.
      */
      (void) FormatLocaleString(quality,MagickPathExtent,"%.20g",(double)
        image->quality);
      status=XDialogWidget(display,windows,"Save","Enter JPEG quality:",
        quality);
      if (*quality == '\0')
        return(MagickTrue);
      image->quality=StringToUnsignedLong(quality);
      image_info->interlace=status != 0 ? NoInterlace : PlaneInterlace;
    }
  if ((LocaleCompare(image_info->magick,"EPS") == 0) ||
      (LocaleCompare(image_info->magick,"PDF") == 0) ||
      (LocaleCompare(image_info->magick,"PS") == 0) ||
      (LocaleCompare(image_info->magick,"PS2") == 0))
    {
      char
        geometry[MagickPathExtent];

      /*
        Request page geometry from user.
      */
      (void) CopyMagickString(geometry,PSPageGeometry,MagickPathExtent);
      if (LocaleCompare(image_info->magick,"PDF") == 0)
        (void) CopyMagickString(geometry,PSPageGeometry,MagickPathExtent);
      if (image_info->page != (char *) NULL)
        (void) CopyMagickString(geometry,image_info->page,MagickPathExtent);
      XListBrowserWidget(display,windows,&windows->widget,PageSizes,"Select",
        "Select page geometry:",geometry);
      if (*geometry != '\0')
        image_info->page=GetPageGeometry(geometry);
    }
  /*
    Apply image transforms.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  save_image=CloneImage(image,0,0,MagickTrue,exception);
  if (save_image == (Image *) NULL)
    return(MagickFalse);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%dx%d!",
    windows->image.ximage->width,windows->image.ximage->height);
  (void) TransformImage(&save_image,windows->image.crop_geometry,geometry,
    exception);
  /*
    Write image.
  */
  (void) CopyMagickString(save_image->filename,filename,MagickPathExtent);
  status=WriteImage(image_info,save_image,exception);
  if (status != MagickFalse )
    image->taint=MagickFalse;
  save_image=DestroyImage(save_image);
  image_info=DestroyImageInfo(image_info);
  XSetCursorState(display,windows,MagickFalse);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X S c r e e n E v e n t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XScreenEvent() handles global events associated with the Pan and Magnify
%  windows.
%
%  The format of the XScreenEvent function is:
%
%      void XScreenEvent(Display *display,XWindows *windows,XEvent *event,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o event: Specifies a pointer to a X11 XEvent structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int XPredicate(Display *magick_unused(display),XEvent *event,char *data)
{
  register XWindows
    *windows;

  windows=(XWindows *) data;
  if ((event->type == ClientMessage) &&
      (event->xclient.window == windows->image.id))
    return(MagickFalse);
  return(MagickTrue);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static void XScreenEvent(Display *display,XWindows *windows,XEvent *event,
  ExceptionInfo *exception)
{
  register int
    x,
    y;

  (void) XIfEvent(display,event,XPredicate,(char *) windows);
  if (event->xany.window == windows->command.id)
    return;
  switch (event->type)
  {
    case ButtonPress:
    case ButtonRelease:
    {
      if ((event->xbutton.button == Button3) &&
          (event->xbutton.state & Mod1Mask))
        {
          /*
            Convert Alt-Button3 to Button2.
          */
          event->xbutton.button=Button2;
          event->xbutton.state&=(~Mod1Mask);
        }
      if (event->xbutton.window == windows->backdrop.id)
        {
          (void) XSetInputFocus(display,event->xbutton.window,RevertToParent,
            event->xbutton.time);
          break;
        }
      if (event->xbutton.window == windows->pan.id)
        {
          XPanImage(display,windows,event,exception);
          break;
        }
      if (event->xbutton.window == windows->image.id)
        if (event->xbutton.button == Button2)
          {
            /*
              Update magnified image.
            */
            x=event->xbutton.x;
            y=event->xbutton.y;
            if (x < 0)
              x=0;
            else
              if (x >= (int) windows->image.width)
                x=(int) (windows->image.width-1);
            windows->magnify.x=(int) windows->image.x+x;
            if (y < 0)
              y=0;
            else
             if (y >= (int) windows->image.height)
               y=(int) (windows->image.height-1);
            windows->magnify.y=windows->image.y+y;
            if (windows->magnify.mapped == MagickFalse)
              (void) XMapRaised(display,windows->magnify.id);
            XMakeMagnifyImage(display,windows,exception);
            if (event->type == ButtonRelease)
              (void) XWithdrawWindow(display,windows->info.id,
                windows->info.screen);
            break;
          }
      break;
    }
    case ClientMessage:
    {
      /*
        If client window delete message, exit.
      */
      if (event->xclient.message_type != windows->wm_protocols)
        break;
      if (*event->xclient.data.l != (long) windows->wm_delete_window)
        break;
      if (event->xclient.window == windows->magnify.id)
        {
          (void) XWithdrawWindow(display,windows->magnify.id,
            windows->magnify.screen);
          break;
        }
      break;
    }
    case ConfigureNotify:
    {
      if (event->xconfigure.window == windows->magnify.id)
        {
          unsigned int
            magnify;

          /*
            Magnify window has a new configuration.
          */
          windows->magnify.width=(unsigned int) event->xconfigure.width;
          windows->magnify.height=(unsigned int) event->xconfigure.height;
          if (windows->magnify.mapped == MagickFalse)
            break;
          magnify=1;
          while ((int) magnify <= event->xconfigure.width)
            magnify<<=1;
          while ((int) magnify <= event->xconfigure.height)
            magnify<<=1;
          magnify>>=1;
          if (((int) magnify != event->xconfigure.width) ||
              ((int) magnify != event->xconfigure.height))
            {
              XWindowChanges
                window_changes;

              window_changes.width=(int) magnify;
              window_changes.height=(int) magnify;
              (void) XReconfigureWMWindow(display,windows->magnify.id,
                windows->magnify.screen,(unsigned int) (CWWidth | CWHeight),
                &window_changes);
              break;
            }
          XMakeMagnifyImage(display,windows,exception);
          break;
        }
      break;
    }
    case Expose:
    {
      if (event->xexpose.window == windows->image.id)
        {
          XRefreshWindow(display,&windows->image,event);
          break;
        }
      if (event->xexpose.window == windows->pan.id)
        if (event->xexpose.count == 0)
          {
            XDrawPanRectangle(display,windows);
            break;
          }
      if (event->xexpose.window == windows->magnify.id)
        if (event->xexpose.count == 0)
          {
            XMakeMagnifyImage(display,windows,exception);
            break;
          }
      break;
    }
    case KeyPress:
    {
      char
        command[MagickPathExtent];

      KeySym
        key_symbol;

      if (event->xkey.window != windows->magnify.id)
        break;
      /*
        Respond to a user key press.
      */
      (void) XLookupString((XKeyEvent *) &event->xkey,command,(int)
        sizeof(command),&key_symbol,(XComposeStatus *) NULL);
      XMagnifyWindowCommand(display,windows,event->xkey.state,key_symbol,
        exception);
      break;
    }
    case MapNotify:
    {
      if (event->xmap.window == windows->magnify.id)
        {
          windows->magnify.mapped=MagickTrue;
          (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
          break;
        }
      if (event->xmap.window == windows->info.id)
        {
          windows->info.mapped=MagickTrue;
          break;
        }
      break;
    }
    case MotionNotify:
    {
      while (XCheckMaskEvent(display,ButtonMotionMask,event)) ;
      if (event->xmotion.window == windows->image.id)
        if (windows->magnify.mapped != MagickFalse )
          {
            /*
              Update magnified image.
            */
            x=event->xmotion.x;
            y=event->xmotion.y;
            if (x < 0)
              x=0;
            else
              if (x >= (int) windows->image.width)
                x=(int) (windows->image.width-1);
            windows->magnify.x=(int) windows->image.x+x;
            if (y < 0)
              y=0;
            else
             if (y >= (int) windows->image.height)
               y=(int) (windows->image.height-1);
            windows->magnify.y=windows->image.y+y;
            XMakeMagnifyImage(display,windows,exception);
          }
      break;
    }
    case UnmapNotify:
    {
      if (event->xunmap.window == windows->magnify.id)
        {
          windows->magnify.mapped=MagickFalse;
          break;
        }
      if (event->xunmap.window == windows->info.id)
        {
          windows->info.mapped=MagickFalse;
          break;
        }
      break;
    }
    default:
      break;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X S e t C r o p G e o m e t r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XSetCropGeometry() accepts a cropping geometry relative to the Image window
%  and translates it to a cropping geometry relative to the image.
%
%  The format of the XSetCropGeometry method is:
%
%      void XSetCropGeometry(Display *display,XWindows *windows,
%        RectangleInfo *crop_info,Image *image)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o crop_info:  A pointer to a RectangleInfo that defines a region of the
%      Image window to crop.
%
%    o image: the image.
%
*/
static void XSetCropGeometry(Display *display,XWindows *windows,
  RectangleInfo *crop_info,Image *image)
{
  char
    text[MagickPathExtent];

  int
    x,
    y;

  double
    scale_factor;

  unsigned int
    height,
    width;

  if (windows->info.mapped != MagickFalse )
    {
      /*
        Display info on cropping rectangle.
      */
      (void) FormatLocaleString(text,MagickPathExtent," %.20gx%.20g%+.20g%+.20g",
        (double) crop_info->width,(double) crop_info->height,(double)
        crop_info->x,(double) crop_info->y);
      XInfoWidget(display,windows,text);
    }
  /*
    Cropping geometry is relative to any previous crop geometry.
  */
  x=0;
  y=0;
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  else
    windows->image.crop_geometry=AcquireString((char *) NULL);
  /*
    Define the crop geometry string from the cropping rectangle.
  */
  scale_factor=(double) width/windows->image.ximage->width;
  if (crop_info->x > 0)
    x+=(int) (scale_factor*crop_info->x+0.5);
  width=(unsigned int) (scale_factor*crop_info->width+0.5);
  if (width == 0)
    width=1;
  scale_factor=(double) height/windows->image.ximage->height;
  if (crop_info->y > 0)
    y+=(int) (scale_factor*crop_info->y+0.5);
  height=(unsigned int) (scale_factor*crop_info->height+0.5);
  if (height == 0)
    height=1;
  (void) FormatLocaleString(windows->image.crop_geometry,MagickPathExtent,
    "%ux%u%+d%+d",width,height,x,y);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X T i l e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XTileImage() loads or deletes a selected tile from a visual image directory.
%  The load or delete command is chosen from a menu.
%
%  The format of the XTileImage method is:
%
%      Image *XTileImage(Display *display,XResourceInfo *resource_info,
%        XWindows *windows,Image *image,XEvent *event,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o tile_image:  XTileImage reads or deletes the tile image
%      and returns it.  A null image is returned if an error occurs.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image; returned from ReadImage.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *XTileImage(Display *display,XResourceInfo *resource_info,
  XWindows *windows,Image *image,XEvent *event,ExceptionInfo *exception)
{
  static const char
    *VerbMenu[] =
    {
      "Load",
      "Next",
      "Former",
      "Delete",
      "Update",
      (char *) NULL,
    };

  static const ModeType
    TileCommands[] =
    {
      TileLoadCommand,
      TileNextCommand,
      TileFormerCommand,
      TileDeleteCommand,
      TileUpdateCommand
    };

  char
    command[MagickPathExtent],
    filename[MagickPathExtent];

  Image
    *tile_image;

  int
    id,
    status,
    tile,
    x,
    y;

  double
    scale_factor;

  register char
    *p,
    *q;

  register int
    i;

  unsigned int
    height,
    width;

  /*
    Tile image is relative to montage image configuration.
  */
  x=0;
  y=0;
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=(double) width/windows->image.ximage->width;
  event->xbutton.x+=windows->image.x;
  event->xbutton.x=(int) (scale_factor*event->xbutton.x+x+0.5);
  scale_factor=(double) height/windows->image.ximage->height;
  event->xbutton.y+=windows->image.y;
  event->xbutton.y=(int) (scale_factor*event->xbutton.y+y+0.5);
  /*
    Determine size and location of each tile in the visual image directory.
  */
  width=(unsigned int) image->columns;
  height=(unsigned int) image->rows;
  x=0;
  y=0;
  (void) XParseGeometry(image->montage,&x,&y,&width,&height);
  tile=((event->xbutton.y-y)/height)*(((int) image->columns-x)/width)+
    (event->xbutton.x-x)/width;
  if (tile < 0)
    {
      /*
        Button press is outside any tile.
      */
      (void) XBell(display,0);
      return((Image *) NULL);
    }
  /*
    Determine file name from the tile directory.
  */
  p=image->directory;
  for (i=tile; (i != 0) && (*p != '\0'); )
  {
    if (*p == '\n')
      i--;
    p++;
  }
  if (*p == '\0')
    {
      /*
        Button press is outside any tile.
      */
      (void) XBell(display,0);
      return((Image *) NULL);
    }
  /*
    Select a command from the pop-up menu.
  */
  id=XMenuWidget(display,windows,"Tile Verb",VerbMenu,command);
  if (id < 0)
    return((Image *) NULL);
  q=p;
  while ((*q != '\n') && (*q != '\0'))
    q++;
  (void) CopyMagickString(filename,p,(size_t) (q-p+1));
  /*
    Perform command for the selected tile.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  tile_image=NewImageList();
  switch (TileCommands[id])
  {
    case TileLoadCommand:
    {
      /*
        Load tile image.
      */
      XCheckRefreshWindows(display,windows);
      (void) CopyMagickString(resource_info->image_info->magick,"MIFF",
        MagickPathExtent);
      (void) CopyMagickString(resource_info->image_info->filename,filename,
        MagickPathExtent);
      tile_image=ReadImage(resource_info->image_info,exception);
      CatchException(exception);
      (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      break;
    }
    case TileNextCommand:
    {
      /*
        Display next image.
      */
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_next_image,CurrentTime);
      break;
    }
    case TileFormerCommand:
    {
      /*
        Display former image.
      */
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_former_image,CurrentTime);
      break;
    }
    case TileDeleteCommand:
    {
      /*
        Delete tile image.
      */
      if (IsPathAccessible(filename) == MagickFalse)
        {
          XNoticeWidget(display,windows,"Image file does not exist:",filename);
          break;
        }
      status=XConfirmWidget(display,windows,"Really delete tile",filename);
      if (status <= 0)
        break;
      status=ShredFile(filename);
      if (status != MagickFalse )
        {
          XNoticeWidget(display,windows,"Unable to delete image file:",
            filename);
          break;
        }
    }
    case TileUpdateCommand:
    {
      int
        x_offset,
        y_offset;

      PixelInfo
        pixel;

      register int
        j;

      register Quantum
        *s;

      /*
        Ensure all the images exist.
      */
      tile=0;
      GetPixelInfo(image,&pixel);
      for (p=image->directory; *p != '\0'; p++)
      {
        CacheView
          *image_view;

        q=p;
        while ((*q != '\n') && (*q != '\0'))
          q++;
        (void) CopyMagickString(filename,p,(size_t) (q-p+1));
        p=q;
        if (IsPathAccessible(filename) != MagickFalse )
          {
            tile++;
            continue;
          }
        /*
          Overwrite tile with background color.
        */
        x_offset=(int) (width*(tile % (((int) image->columns-x)/width))+x);
        y_offset=(int) (height*(tile/(((int) image->columns-x)/width))+y);
        image_view=AcquireAuthenticCacheView(image,exception);
        (void) GetOneCacheViewVirtualPixelInfo(image_view,0,0,&pixel,exception);
        for (i=0; i < (int) height; i++)
        {
          s=GetCacheViewAuthenticPixels(image_view,(ssize_t) x_offset,(ssize_t)
            y_offset+i,width,1,exception);
          if (s == (Quantum *) NULL)
            break;
          for (j=0; j < (int) width; j++)
          {
            SetPixelViaPixelInfo(image,&pixel,s);
            s+=GetPixelChannels(image);
          }
          if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
            break;
        }
        image_view=DestroyCacheView(image_view);
        tile++;
      }
      windows->image.window_changes.width=(int) image->columns;
      windows->image.window_changes.height=(int) image->rows;
      XConfigureImageColormap(display,resource_info,windows,image,exception);
      (void) XConfigureImage(display,resource_info,windows,image,exception);
      break;
    }
    default:
      break;
  }
  XSetCursorState(display,windows,MagickFalse);
  return(tile_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X T r a n s l a t e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XTranslateImage() translates the image within an Image window by one pixel
%  as specified by the key symbol.  If the image has a montage string the
%  translation is respect to the width and height contained within the string.
%
%  The format of the XTranslateImage method is:
%
%      void XTranslateImage(Display *display,XWindows *windows,
%        Image *image,const KeySym key_symbol)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o key_symbol: Specifies a KeySym which indicates which side of the image
%      to trim.
%
*/
static void XTranslateImage(Display *display,XWindows *windows,
  Image *image,const KeySym key_symbol)
{
  char
    text[MagickPathExtent];

  int
    x,
    y;

  unsigned int
    x_offset,
    y_offset;

  /*
    User specified a pan position offset.
  */
  x_offset=windows->image.width;
  y_offset=windows->image.height;
  if (image->montage != (char *) NULL)
    (void) XParseGeometry(image->montage,&x,&y,&x_offset,&y_offset);
  switch ((int) key_symbol)
  {
    case XK_Home:
    case XK_KP_Home:
    {
      windows->image.x=(int) windows->image.width/2;
      windows->image.y=(int) windows->image.height/2;
      break;
    }
    case XK_Left:
    case XK_KP_Left:
    {
      windows->image.x-=x_offset;
      break;
    }
    case XK_Next:
    case XK_Up:
    case XK_KP_Up:
    {
      windows->image.y-=y_offset;
      break;
    }
    case XK_Right:
    case XK_KP_Right:
    {
      windows->image.x+=x_offset;
      break;
    }
    case XK_Prior:
    case XK_Down:
    case XK_KP_Down:
    {
      windows->image.y+=y_offset;
      break;
    }
    default:
      return;
  }
  /*
    Check boundary conditions.
  */
  if (windows->image.x < 0)
    windows->image.x=0;
  else
    if ((int) (windows->image.x+windows->image.width) >
        windows->image.ximage->width)
      windows->image.x=(int) windows->image.ximage->width-windows->image.width;
  if (windows->image.y < 0)
    windows->image.y=0;
  else
    if ((int) (windows->image.y+windows->image.height) >
        windows->image.ximage->height)
      windows->image.y=(int) windows->image.ximage->height-windows->image.height;
  /*
    Refresh Image window.
  */
  (void) FormatLocaleString(text,MagickPathExtent," %ux%u%+d%+d ",
    windows->image.width,windows->image.height,windows->image.x,
    windows->image.y);
  XInfoWidget(display,windows,text);
  XCheckRefreshWindows(display,windows);
  XDrawPanRectangle(display,windows);
  XRefreshWindow(display,&windows->image,(XEvent *) NULL);
  (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X T r i m I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XTrimImage() trims the edges from the Image window.
%
%  The format of the XTrimImage method is:
%
%      MagickBooleanType XTrimImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType XTrimImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image,
  ExceptionInfo *exception)
{
  RectangleInfo
    trim_info;

  register int
    x,
    y;

  size_t
    background,
    pixel;

  /*
    Trim edges from image.
  */
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  /*
    Crop the left edge.
  */
  background=XGetPixel(windows->image.ximage,0,0);
  trim_info.width=(size_t) windows->image.ximage->width;
  for (x=0; x < windows->image.ximage->width; x++)
  {
    for (y=0; y < windows->image.ximage->height; y++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (y < windows->image.ximage->height)
      break;
  }
  trim_info.x=(ssize_t) x;
  if (trim_info.x == (ssize_t) windows->image.ximage->width)
    {
      XSetCursorState(display,windows,MagickFalse);
      return(MagickFalse);
    }
  /*
    Crop the right edge.
  */
  background=XGetPixel(windows->image.ximage,windows->image.ximage->width-1,0);
  for (x=windows->image.ximage->width-1; x != 0; x--)
  {
    for (y=0; y < windows->image.ximage->height; y++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (y < windows->image.ximage->height)
      break;
  }
  trim_info.width=(size_t) (x-trim_info.x+1);
  /*
    Crop the top edge.
  */
  background=XGetPixel(windows->image.ximage,0,0);
  trim_info.height=(size_t) windows->image.ximage->height;
  for (y=0; y < windows->image.ximage->height; y++)
  {
    for (x=0; x < windows->image.ximage->width; x++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (x < windows->image.ximage->width)
      break;
  }
  trim_info.y=(ssize_t) y;
  /*
    Crop the bottom edge.
  */
  background=XGetPixel(windows->image.ximage,0,windows->image.ximage->height-1);
  for (y=windows->image.ximage->height-1; y != 0; y--)
  {
    for (x=0; x < windows->image.ximage->width; x++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (x < windows->image.ximage->width)
      break;
  }
  trim_info.height=(size_t) y-trim_info.y+1;
  if (((unsigned int) trim_info.width != windows->image.width) ||
      ((unsigned int) trim_info.height != windows->image.height))
    {
      /*
        Reconfigure Image window as defined by the trimming rectangle.
      */
      XSetCropGeometry(display,windows,&trim_info,image);
      windows->image.window_changes.width=(int) trim_info.width;
      windows->image.window_changes.height=(int) trim_info.height;
      (void) XConfigureImage(display,resource_info,windows,image,exception);
    }
  XSetCursorState(display,windows,MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X V i s u a l D i r e c t o r y I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XVisualDirectoryImage() creates a Visual Image Directory.
%
%  The format of the XVisualDirectoryImage method is:
%
%      Image *XVisualDirectoryImage(Display *display,
%        XResourceInfo *resource_info,XWindows *windows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *XVisualDirectoryImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,ExceptionInfo *exception)
{
#define TileImageTag  "Scale/Image"
#define XClientName  "montage"

  char
    **filelist;

  Image
    *images,
    *montage_image,
    *next_image,
    *thumbnail_image;

  ImageInfo
    *read_info;

  int
    number_files;

  MagickBooleanType
    backdrop;

  MagickStatusType
    status;

  MontageInfo
    *montage_info;

  RectangleInfo
    geometry;

  register int
    i;

  static char
    filename[MagickPathExtent] = "\0",
    filenames[MagickPathExtent] = "*";

  XResourceInfo
    background_resources;

  /*
    Request file name from user.
  */
  XFileBrowserWidget(display,windows,"Directory",filenames);
  if (*filenames == '\0')
    return((Image *) NULL);
  /*
    Expand the filenames.
  */
  filelist=(char **) AcquireMagickMemory(sizeof(*filelist));
  if (filelist == (char **) NULL)
    {
      ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",
        filenames);
      return((Image *) NULL);
    }
  number_files=1;
  filelist[0]=filenames;
  status=ExpandFilenames(&number_files,&filelist);
  if ((status == MagickFalse) || (number_files == 0))
    {
      if (number_files == 0)
        ThrowXWindowException(ImageError,"NoImagesWereFound",filenames)
      else
        ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",
          filenames);
      return((Image *) NULL);
    }
  /*
    Set image background resources.
  */
  background_resources=(*resource_info);
  background_resources.window_id=AcquireString("");
  (void) FormatLocaleString(background_resources.window_id,MagickPathExtent,
    "0x%lx",windows->image.id);
  background_resources.backdrop=MagickTrue;
  /*
    Read each image and convert them to a tile.
  */
  backdrop=((windows->visual_info->klass == TrueColor) ||
    (windows->visual_info->klass == DirectColor)) ? MagickTrue : MagickFalse;
  read_info=CloneImageInfo(resource_info->image_info);
  (void) SetImageOption(read_info,"jpeg:size","120x120");
  (void) CloneString(&read_info->size,DefaultTileGeometry);
  (void) SetImageInfoProgressMonitor(read_info,(MagickProgressMonitor) NULL,
    (void *) NULL);
  images=NewImageList();
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  for (i=0; i < (int) number_files; i++)
  {
    (void) CopyMagickString(read_info->filename,filelist[i],MagickPathExtent);
    filelist[i]=DestroyString(filelist[i]);
    *read_info->magick='\0';
    next_image=ReadImage(read_info,exception);
    CatchException(exception);
    if (next_image != (Image *) NULL)
      {
        (void) DeleteImageProperty(next_image,"label");
        (void) SetImageProperty(next_image,"label",InterpretImageProperties(
          read_info,next_image,DefaultTileLabel,exception),exception);
        (void) ParseRegionGeometry(next_image,read_info->size,&geometry,
          exception);
        thumbnail_image=ThumbnailImage(next_image,geometry.width,
          geometry.height,exception);
        if (thumbnail_image != (Image *) NULL)
          {
            next_image=DestroyImage(next_image);
            next_image=thumbnail_image;
          }
        if (backdrop)
          {
            (void) XDisplayBackgroundImage(display,&background_resources,
              next_image,exception);
            XSetCursorState(display,windows,MagickTrue);
          }
        AppendImageToList(&images,next_image);
        if (images->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

            proceed=SetImageProgress(images,LoadImageTag,(MagickOffsetType) i,
              (MagickSizeType) number_files);
            if (proceed == MagickFalse)
              break;
          }
      }
  }
  filelist=(char **) RelinquishMagickMemory(filelist);
  if (images == (Image *) NULL)
    {
      read_info=DestroyImageInfo(read_info);
      XSetCursorState(display,windows,MagickFalse);
      ThrowXWindowException(ImageError,"NoImagesWereLoaded",filenames);
      return((Image *) NULL);
    }
  /*
    Create the Visual Image Directory.
  */
  montage_info=CloneMontageInfo(read_info,(MontageInfo *) NULL);
  montage_info->pointsize=10;
  if (resource_info->font != (char *) NULL)
    (void) CloneString(&montage_info->font,resource_info->font);
  (void) CopyMagickString(montage_info->filename,filename,MagickPathExtent);
  montage_image=MontageImageList(read_info,montage_info,GetFirstImageInList(
    images),exception);
  images=DestroyImageList(images);
  montage_info=DestroyMontageInfo(montage_info);
  read_info=DestroyImageInfo(read_info);
  XSetCursorState(display,windows,MagickFalse);
  if (montage_image == (Image *) NULL)
    return(montage_image);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_next_image,CurrentTime);
  return(montage_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D i s p l a y B a c k g r o u n d I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDisplayBackgroundImage() displays an image in the background of a window.
%
%  The format of the XDisplayBackgroundImage method is:
%
%      MagickBooleanType XDisplayBackgroundImage(Display *display,
%        XResourceInfo *resource_info,Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType XDisplayBackgroundImage(Display *display,
  XResourceInfo *resource_info,Image *image,ExceptionInfo *exception)
{
  char
    geometry[MagickPathExtent],
    visual_type[MagickPathExtent];

  int
    height,
    status,
    width;

  RectangleInfo
    geometry_info;

  static XPixelInfo
    pixel;

  static XStandardColormap
    *map_info;

  static XVisualInfo
    *visual_info = (XVisualInfo *) NULL;

  static XWindowInfo
    window_info;

  size_t
    delay;

  Window
    root_window;

  XGCValues
    context_values;

  XResourceInfo
    resources;

  XWindowAttributes
    window_attributes;

  /*
    Determine target window.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse )
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  resources=(*resource_info);
  window_info.id=(Window) NULL;
  root_window=XRootWindow(display,XDefaultScreen(display));
  if (LocaleCompare(resources.window_id,"root") == 0)
    window_info.id=root_window;
  else
    {
      if (isdigit((int) ((unsigned char) *resources.window_id)) != 0)
        window_info.id=XWindowByID(display,root_window,
          (Window) strtol((char *) resources.window_id,(char **) NULL,0));
      if (window_info.id == (Window) NULL)
        window_info.id=XWindowByName(display,root_window,resources.window_id);
    }
  if (window_info.id == (Window) NULL)
    {
      ThrowXWindowException(XServerError,"NoWindowWithSpecifiedIDExists",
        resources.window_id);
      return(MagickFalse);
    }
  /*
    Determine window visual id.
  */
  window_attributes.width=XDisplayWidth(display,XDefaultScreen(display));
  window_attributes.height=XDisplayHeight(display,XDefaultScreen(display));
  (void) CopyMagickString(visual_type,"default",MagickPathExtent);
  status=XGetWindowAttributes(display,window_info.id,&window_attributes);
  if (status != 0)
    (void) FormatLocaleString(visual_type,MagickPathExtent,"0x%lx",
      XVisualIDFromVisual(window_attributes.visual));
  if (visual_info == (XVisualInfo *) NULL)
    {
      /*
        Allocate standard colormap.
      */
      map_info=XAllocStandardColormap();
      if (map_info == (XStandardColormap *) NULL)
        ThrowXWindowFatalException(XServerFatalError,"MemoryAllocationFailed",
          image->filename);
      map_info->colormap=(Colormap) NULL;
      pixel.pixels=(unsigned long *) NULL;
      /*
        Initialize visual info.
      */
      resources.map_type=(char *) NULL;
      resources.visual_type=visual_type;
      visual_info=XBestVisualInfo(display,map_info,&resources);
      if (visual_info == (XVisualInfo *) NULL)
        ThrowXWindowFatalException(XServerFatalError,"UnableToGetVisual",
          resources.visual_type);
      /*
        Initialize window info.
      */
      window_info.ximage=(XImage *) NULL;
      window_info.matte_image=(XImage *) NULL;
      window_info.pixmap=(Pixmap) NULL;
      window_info.matte_pixmap=(Pixmap) NULL;
    }
  /*
    Free previous root colors.
  */
  if (window_info.id == root_window)
    (void) XDestroyWindowColors(display,root_window);
  /*
    Initialize Standard Colormap.
  */
  resources.colormap=SharedColormap;
  XMakeStandardColormap(display,visual_info,&resources,image,map_info,&pixel,
    exception);
  /*
    Graphic context superclass.
  */
  context_values.background=pixel.background_color.pixel;
  context_values.foreground=pixel.foreground_color.pixel;
  pixel.annotate_context=XCreateGC(display,window_info.id,
    (size_t) (GCBackground | GCForeground),&context_values);
  if (pixel.annotate_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      image->filename);
  /*
    Initialize Image window attributes.
  */
  window_info.name=AcquireString("\0");
  window_info.icon_name=AcquireString("\0");
  XGetWindowInfo(display,visual_info,map_info,&pixel,(XFontStruct *) NULL,
    &resources,&window_info);
  /*
    Create the X image.
  */
  window_info.width=(unsigned int) image->columns;
  window_info.height=(unsigned int) image->rows;
  if ((image->columns != window_info.width) ||
      (image->rows != window_info.height))
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
      image->filename);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%ux%u+0+0>",
    window_attributes.width,window_attributes.height);
  geometry_info.width=window_info.width;
  geometry_info.height=window_info.height;
  geometry_info.x=(ssize_t) window_info.x;
  geometry_info.y=(ssize_t) window_info.y;
  (void) ParseMetaGeometry(geometry,&geometry_info.x,&geometry_info.y,
    &geometry_info.width,&geometry_info.height);
  window_info.width=(unsigned int) geometry_info.width;
  window_info.height=(unsigned int) geometry_info.height;
  window_info.x=(int) geometry_info.x;
  window_info.y=(int) geometry_info.y;
  status=XMakeImage(display,&resources,&window_info,image,window_info.width,
    window_info.height,exception);
  if (status == MagickFalse)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
      image->filename);
  window_info.x=0;
  window_info.y=0;
  if (image->debug != MagickFalse )
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "Image: %s[%.20g] %.20gx%.20g ",image->filename,(double) image->scene,
        (double) image->columns,(double) image->rows);
      if (image->colors != 0)
        (void) LogMagickEvent(X11Event,GetMagickModule(),"%.20gc ",(double)
          image->colors);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"%s",image->magick);
    }
  /*
    Adjust image dimensions as specified by backdrop or geometry options.
  */
  width=(int) window_info.width;
  height=(int) window_info.height;
  if (resources.backdrop != MagickFalse )
    {
      /*
        Center image on window.
      */
      window_info.x=(window_attributes.width/2)-
        (window_info.ximage->width/2);
      window_info.y=(window_attributes.height/2)-
        (window_info.ximage->height/2);
      width=window_attributes.width;
      height=window_attributes.height;
    }
  if ((resources.image_geometry != (char *) NULL) &&
      (*resources.image_geometry != '\0'))
    {
      char
        default_geometry[MagickPathExtent];

      int
        flags,
        gravity;

      XSizeHints
        *size_hints;

      /*
        User specified geometry.
      */
      size_hints=XAllocSizeHints();
      if (size_hints == (XSizeHints *) NULL)
        ThrowXWindowFatalException(ResourceLimitFatalError,
          "MemoryAllocationFailed",image->filename);
      size_hints->flags=0L;
      (void) FormatLocaleString(default_geometry,MagickPathExtent,"%dx%d",
        width,height);
      flags=XWMGeometry(display,visual_info->screen,resources.image_geometry,
        default_geometry,window_info.border_width,size_hints,&window_info.x,
        &window_info.y,&width,&height,&gravity);
      if (flags & (XValue | YValue))
        {
          width=window_attributes.width;
          height=window_attributes.height;
        }
      (void) XFree((void *) size_hints);
    }
  /*
    Create the X pixmap.
  */
  window_info.pixmap=XCreatePixmap(display,window_info.id,(unsigned int) width,
    (unsigned int) height,window_info.depth);
  if (window_info.pixmap == (Pixmap) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXPixmap",
      image->filename);
  /*
    Display pixmap on the window.
  */
  if (((unsigned int) width > window_info.width) ||
      ((unsigned int) height > window_info.height))
    (void) XFillRectangle(display,window_info.pixmap,
      window_info.annotate_context,0,0,(unsigned int) width,
      (unsigned int) height);
  (void) XPutImage(display,window_info.pixmap,window_info.annotate_context,
    window_info.ximage,0,0,window_info.x,window_info.y,(unsigned int)
    window_info.width,(unsigned int) window_info.height);
  (void) XSetWindowBackgroundPixmap(display,window_info.id,window_info.pixmap);
  (void) XClearWindow(display,window_info.id);
  delay=1000*image->delay/MagickMax(image->ticks_per_second,1L);
  XDelay(display,delay == 0UL ? 10UL : delay);
  (void) XSync(display,MagickFalse);
  return(window_info.id == root_window ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X D i s p l a y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XDisplayImage() displays an image via X11.  A new image is created and
%  returned if the user interactively transforms the displayed image.
%
%  The format of the XDisplayImage method is:
%
%      Image *XDisplayImage(Display *display,XResourceInfo *resource_info,
%        char **argv,int argc,Image **image,size_t *state,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o nexus:  Method XDisplayImage returns an image when the
%      user chooses 'Open Image' from the command menu or picks a tile
%      from the image directory.  Otherwise a null image is returned.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o argv: Specifies the application's argument list.
%
%    o argc: Specifies the number of arguments.
%
%    o image: Specifies an address to an address of an Image structure;
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *XDisplayImage(Display *display,XResourceInfo *resource_info,
  char **argv,int argc,Image **image,size_t *state,ExceptionInfo *exception)
{
#define MagnifySize  256  /* must be a power of 2 */
#define MagickMenus  10
#define MagickTitle  "Commands"

  static const char
    *CommandMenu[] =
    {
      "File",
      "Edit",
      "View",
      "Transform",
      "Enhance",
      "Effects",
      "F/X",
      "Image Edit",
      "Miscellany",
      "Help",
      (char *) NULL
    },
    *FileMenu[] =
    {
      "Open...",
      "Next",
      "Former",
      "Select...",
      "Save...",
      "Print...",
      "Delete...",
      "New...",
      "Visual Directory...",
      "Quit",
      (char *) NULL
    },
    *EditMenu[] =
    {
      "Undo",
      "Redo",
      "Cut",
      "Copy",
      "Paste",
      (char *) NULL
    },
    *ViewMenu[] =
    {
      "Half Size",
      "Original Size",
      "Double Size",
      "Resize...",
      "Apply",
      "Refresh",
      "Restore",
      (char *) NULL
    },
    *TransformMenu[] =
    {
      "Crop",
      "Chop",
      "Flop",
      "Flip",
      "Rotate Right",
      "Rotate Left",
      "Rotate...",
      "Shear...",
      "Roll...",
      "Trim Edges",
      (char *) NULL
    },
    *EnhanceMenu[] =
    {
      "Hue...",
      "Saturation...",
      "Brightness...",
      "Gamma...",
      "Spiff",
      "Dull",
      "Contrast Stretch...",
      "Sigmoidal Contrast...",
      "Normalize",
      "Equalize",
      "Negate",
      "Grayscale",
      "Map...",
      "Quantize...",
      (char *) NULL
    },
    *EffectsMenu[] =
    {
      "Despeckle",
      "Emboss",
      "Reduce Noise",
      "Add Noise...",
      "Sharpen...",
      "Blur...",
      "Threshold...",
      "Edge Detect...",
      "Spread...",
      "Shade...",
      "Raise...",
      "Segment...",
      (char *) NULL
    },
    *FXMenu[] =
    {
      "Solarize...",
      "Sepia Tone...",
      "Swirl...",
      "Implode...",
      "Vignette...",
      "Wave...",
      "Oil Paint...",
      "Charcoal Draw...",
      (char *) NULL
    },
    *ImageEditMenu[] =
    {
      "Annotate...",
      "Draw...",
      "Color...",
      "Matte...",
      "Composite...",
      "Add Border...",
      "Add Frame...",
      "Comment...",
      "Launch...",
      "Region of Interest...",
      (char *) NULL
    },
    *MiscellanyMenu[] =
    {
      "Image Info",
      "Zoom Image",
      "Show Preview...",
      "Show Histogram",
      "Show Matte",
      "Background...",
      "Slide Show...",
      "Preferences...",
      (char *) NULL
    },
    *HelpMenu[] =
    {
      "Overview",
      "Browse Documentation",
      "About Display",
      (char *) NULL
    },
    *ShortCutsMenu[] =
    {
      "Next",
      "Former",
      "Open...",
      "Save...",
      "Print...",
      "Undo",
      "Restore",
      "Image Info",
      "Quit",
      (char *) NULL
    },
    *VirtualMenu[] =
    {
      "Image Info",
      "Print",
      "Next",
      "Quit",
      (char *) NULL
    };

  static const char
    **Menus[MagickMenus] =
    {
      FileMenu,
      EditMenu,
      ViewMenu,
      TransformMenu,
      EnhanceMenu,
      EffectsMenu,
      FXMenu,
      ImageEditMenu,
      MiscellanyMenu,
      HelpMenu
    };

  static CommandType
    CommandMenus[] =
    {
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
    },
    FileCommands[] =
    {
      OpenCommand,
      NextCommand,
      FormerCommand,
      SelectCommand,
      SaveCommand,
      PrintCommand,
      DeleteCommand,
      NewCommand,
      VisualDirectoryCommand,
      QuitCommand
    },
    EditCommands[] =
    {
      UndoCommand,
      RedoCommand,
      CutCommand,
      CopyCommand,
      PasteCommand
    },
    ViewCommands[] =
    {
      HalfSizeCommand,
      OriginalSizeCommand,
      DoubleSizeCommand,
      ResizeCommand,
      ApplyCommand,
      RefreshCommand,
      RestoreCommand
    },
    TransformCommands[] =
    {
      CropCommand,
      ChopCommand,
      FlopCommand,
      FlipCommand,
      RotateRightCommand,
      RotateLeftCommand,
      RotateCommand,
      ShearCommand,
      RollCommand,
      TrimCommand
    },
    EnhanceCommands[] =
    {
      HueCommand,
      SaturationCommand,
      BrightnessCommand,
      GammaCommand,
      SpiffCommand,
      DullCommand,
      ContrastStretchCommand,
      SigmoidalContrastCommand,
      NormalizeCommand,
      EqualizeCommand,
      NegateCommand,
      GrayscaleCommand,
      MapCommand,
      QuantizeCommand
    },
    EffectsCommands[] =
    {
      DespeckleCommand,
      EmbossCommand,
      ReduceNoiseCommand,
      AddNoiseCommand,
      SharpenCommand,
      BlurCommand,
      ThresholdCommand,
      EdgeDetectCommand,
      SpreadCommand,
      ShadeCommand,
      RaiseCommand,
      SegmentCommand
    },
    FXCommands[] =
    {
      SolarizeCommand,
      SepiaToneCommand,
      SwirlCommand,
      ImplodeCommand,
      VignetteCommand,
      WaveCommand,
      OilPaintCommand,
      CharcoalDrawCommand
    },
    ImageEditCommands[] =
    {
      AnnotateCommand,
      DrawCommand,
      ColorCommand,
      MatteCommand,
      CompositeCommand,
      AddBorderCommand,
      AddFrameCommand,
      CommentCommand,
      LaunchCommand,
      RegionofInterestCommand
    },
    MiscellanyCommands[] =
    {
      InfoCommand,
      ZoomCommand,
      ShowPreviewCommand,
      ShowHistogramCommand,
      ShowMatteCommand,
      BackgroundCommand,
      SlideShowCommand,
      PreferencesCommand
    },
    HelpCommands[] =
    {
      HelpCommand,
      BrowseDocumentationCommand,
      VersionCommand
    },
    ShortCutsCommands[] =
    {
      NextCommand,
      FormerCommand,
      OpenCommand,
      SaveCommand,
      PrintCommand,
      UndoCommand,
      RestoreCommand,
      InfoCommand,
      QuitCommand
    },
    VirtualCommands[] =
    {
      InfoCommand,
      PrintCommand,
      NextCommand,
      QuitCommand
    };

  static CommandType
    *Commands[MagickMenus] =
    {
      FileCommands,
      EditCommands,
      ViewCommands,
      TransformCommands,
      EnhanceCommands,
      EffectsCommands,
      FXCommands,
      ImageEditCommands,
      MiscellanyCommands,
      HelpCommands
    };

  char
    command[MagickPathExtent],
    *directory,
    geometry[MagickPathExtent],
    resource_name[MagickPathExtent];

  CommandType
    command_type;

  Image
    *display_image,
    *nexus;

  int
    entry,
    id;

  KeySym
    key_symbol;

  MagickStatusType
    context_mask,
    status;

  RectangleInfo
    geometry_info;

  register int
    i;

  static char
    working_directory[MagickPathExtent];

  static XPoint
    vid_info;

  static XWindowInfo
    *magick_windows[MaxXWindows];

  static unsigned int
    number_windows;

  struct stat
    attributes;

  time_t
    timer,
    timestamp,
    update_time;

  unsigned int
    height,
    width;

  size_t
    delay;

  WarningHandler
    warning_handler;

  Window
    root_window;

  XClassHint
    *class_hints;

  XEvent
    event;

  XFontStruct
    *font_info;

  XGCValues
    context_values;

  XPixelInfo
    *icon_pixel,
    *pixel;

  XResourceInfo
    *icon_resources;

  XStandardColormap
    *icon_map,
    *map_info;

  XVisualInfo
    *icon_visual,
    *visual_info;

  XWindowChanges
    window_changes;

  XWindows
    *windows;

  XWMHints
    *manager_hints;

  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickCoreSignature);
  if ((*image)->debug != MagickFalse )
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  display_image=(*image);
  warning_handler=(WarningHandler) NULL;
  windows=XSetWindows((XWindows *) ~0);
  if (windows != (XWindows *) NULL)
    {
      int
        status;

      if (*working_directory == '\0')
        (void) CopyMagickString(working_directory,".",MagickPathExtent);
      status=chdir(working_directory);
      if (status == -1)
        (void) ThrowMagickException(exception,GetMagickModule(),FileOpenError,
          "UnableToOpenFile","%s",working_directory);
      warning_handler=resource_info->display_warnings ?
        SetErrorHandler(XWarning) : SetErrorHandler((ErrorHandler) NULL);
      warning_handler=resource_info->display_warnings ?
        SetWarningHandler(XWarning) : SetWarningHandler((WarningHandler) NULL);
    }
  else
    {
      /*
        Allocate windows structure.
      */
      resource_info->colors=display_image->colors;
      windows=XSetWindows(XInitializeWindows(display,resource_info));
      if (windows == (XWindows *) NULL)
        ThrowXWindowFatalException(XServerFatalError,"UnableToCreateWindow",
          (*image)->filename);
      /*
        Initialize window id's.
      */
      number_windows=0;
      magick_windows[number_windows++]=(&windows->icon);
      magick_windows[number_windows++]=(&windows->backdrop);
      magick_windows[number_windows++]=(&windows->image);
      magick_windows[number_windows++]=(&windows->info);
      magick_windows[number_windows++]=(&windows->command);
      magick_windows[number_windows++]=(&windows->widget);
      magick_windows[number_windows++]=(&windows->popup);
      magick_windows[number_windows++]=(&windows->magnify);
      magick_windows[number_windows++]=(&windows->pan);
      for (i=0; i < (int) number_windows; i++)
        magick_windows[i]->id=(Window) NULL;
      vid_info.x=0;
      vid_info.y=0;
    }
  /*
    Initialize font info.
  */
  if (windows->font_info != (XFontStruct *) NULL)
    (void) XFreeFont(display,windows->font_info);
  windows->font_info=XBestFont(display,resource_info,MagickFalse);
  if (windows->font_info == (XFontStruct *) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToLoadFont",
      resource_info->font);
  /*
    Initialize Standard Colormap.
  */
  map_info=windows->map_info;
  icon_map=windows->icon_map;
  visual_info=windows->visual_info;
  icon_visual=windows->icon_visual;
  pixel=windows->pixel_info;
  icon_pixel=windows->icon_pixel;
  font_info=windows->font_info;
  icon_resources=windows->icon_resources;
  class_hints=windows->class_hints;
  manager_hints=windows->manager_hints;
  root_window=XRootWindow(display,visual_info->screen);
  nexus=NewImageList();
  if (display_image->debug != MagickFalse )
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "Image: %s[%.20g] %.20gx%.20g ",display_image->filename,
        (double) display_image->scene,(double) display_image->columns,
        (double) display_image->rows);
      if (display_image->colors != 0)
        (void) LogMagickEvent(X11Event,GetMagickModule(),"%.20gc ",(double)
          display_image->colors);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"%s",
        display_image->magick);
    }
  XMakeStandardColormap(display,visual_info,resource_info,display_image,
    map_info,pixel,exception);
  display_image->taint=MagickFalse;
  /*
    Initialize graphic context.
  */
  windows->context.id=(Window) NULL;
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->context);
  (void) CloneString(&class_hints->res_name,resource_info->client_name);
  (void) CloneString(&class_hints->res_class,resource_info->client_name);
  class_hints->res_class[0]=(char) toupper((int) class_hints->res_class[0]);
  manager_hints->flags=InputHint | StateHint;
  manager_hints->input=MagickFalse;
  manager_hints->initial_state=WithdrawnState;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->context);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Window id: 0x%lx (context)",windows->context.id);
  context_values.background=pixel->background_color.pixel;
  context_values.font=font_info->fid;
  context_values.foreground=pixel->foreground_color.pixel;
  context_values.graphics_exposures=MagickFalse;
  context_mask=(MagickStatusType)
    (GCBackground | GCFont | GCForeground | GCGraphicsExposures);
  if (pixel->annotate_context != (GC) NULL)
    (void) XFreeGC(display,pixel->annotate_context);
  pixel->annotate_context=XCreateGC(display,windows->context.id,
    context_mask,&context_values);
  if (pixel->annotate_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      display_image->filename);
  context_values.background=pixel->depth_color.pixel;
  if (pixel->widget_context != (GC) NULL)
    (void) XFreeGC(display,pixel->widget_context);
  pixel->widget_context=XCreateGC(display,windows->context.id,context_mask,
    &context_values);
  if (pixel->widget_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      display_image->filename);
  context_values.background=pixel->foreground_color.pixel;
  context_values.foreground=pixel->background_color.pixel;
  context_values.plane_mask=context_values.background ^
    context_values.foreground;
  if (pixel->highlight_context != (GC) NULL)
    (void) XFreeGC(display,pixel->highlight_context);
  pixel->highlight_context=XCreateGC(display,windows->context.id,
    (size_t) (context_mask | GCPlaneMask),&context_values);
  if (pixel->highlight_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      display_image->filename);
  (void) XDestroyWindow(display,windows->context.id);
  /*
    Initialize icon window.
  */
  XGetWindowInfo(display,icon_visual,icon_map,icon_pixel,(XFontStruct *) NULL,
    icon_resources,&windows->icon);
  windows->icon.geometry=resource_info->icon_geometry;
  XBestIconSize(display,&windows->icon,display_image);
  windows->icon.attributes.colormap=XDefaultColormap(display,
    icon_visual->screen);
  windows->icon.attributes.event_mask=ExposureMask | StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint;
  manager_hints->input=MagickFalse;
  manager_hints->initial_state=IconicState;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->icon);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),"Window id: 0x%lx (icon)",
      windows->icon.id);
  /*
    Initialize graphic context for icon window.
  */
  if (icon_pixel->annotate_context != (GC) NULL)
    (void) XFreeGC(display,icon_pixel->annotate_context);
  context_values.background=icon_pixel->background_color.pixel;
  context_values.foreground=icon_pixel->foreground_color.pixel;
  icon_pixel->annotate_context=XCreateGC(display,windows->icon.id,
    (size_t) (GCBackground | GCForeground),&context_values);
  if (icon_pixel->annotate_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      display_image->filename);
  windows->icon.annotate_context=icon_pixel->annotate_context;
  /*
    Initialize Image window.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,resource_info,
    &windows->image);
  windows->image.shape=MagickTrue;  /* non-rectangular shape hint */
  if (resource_info->use_shared_memory == MagickFalse)
    windows->image.shared_memory=MagickFalse;
  if ((resource_info->title != (char *) NULL) && !(*state & MontageImageState))
    {
      char
        *title;

      title=InterpretImageProperties(resource_info->image_info,display_image,
        resource_info->title,exception);
      (void) CopyMagickString(windows->image.name,title,MagickPathExtent);
      (void) CopyMagickString(windows->image.icon_name,title,MagickPathExtent);
      title=DestroyString(title);
    }
  else
    {
      char
        filename[MagickPathExtent];

      /*
        Window name is the base of the filename.
      */
      GetPathComponent(display_image->magick_filename,TailPath,filename);
      if (display_image->scene == 0)
        (void) FormatLocaleString(windows->image.name,MagickPathExtent,
          "%s: %s",MagickPackageName,filename);
      else
        (void) FormatLocaleString(windows->image.name,MagickPathExtent,
          "%s: %s[scene: %.20g frames: %.20g]",MagickPackageName,filename,
          (double) display_image->scene,(double) GetImageListLength(
          display_image));
      (void) CopyMagickString(windows->image.icon_name,filename,
        MagickPathExtent);
    }
  if (resource_info->immutable)
    windows->image.immutable=MagickTrue;
  windows->image.use_pixmap=resource_info->use_pixmap;
  windows->image.geometry=resource_info->image_geometry;
  (void) FormatLocaleString(geometry,MagickPathExtent,"%ux%u+0+0>!",
    XDisplayWidth(display,visual_info->screen),
    XDisplayHeight(display,visual_info->screen));
  geometry_info.width=display_image->columns;
  geometry_info.height=display_image->rows;
  geometry_info.x=0;
  geometry_info.y=0;
  (void) ParseMetaGeometry(geometry,&geometry_info.x,&geometry_info.y,
    &geometry_info.width,&geometry_info.height);
  windows->image.width=(unsigned int) geometry_info.width;
  windows->image.height=(unsigned int) geometry_info.height;
  windows->image.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | KeyPressMask |
    KeyReleaseMask | LeaveWindowMask | OwnerGrabButtonMask |
    PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask;
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->backdrop);
  if ((resource_info->backdrop) || (windows->backdrop.id != (Window) NULL))
    {
      /*
        Initialize backdrop window.
      */
      windows->backdrop.x=0;
      windows->backdrop.y=0;
      (void) CloneString(&windows->backdrop.name,"Backdrop");
      windows->backdrop.flags=(size_t) (USSize | USPosition);
      windows->backdrop.width=(unsigned int)
        XDisplayWidth(display,visual_info->screen);
      windows->backdrop.height=(unsigned int)
        XDisplayHeight(display,visual_info->screen);
      windows->backdrop.border_width=0;
      windows->backdrop.immutable=MagickTrue;
      windows->backdrop.attributes.do_not_propagate_mask=ButtonPressMask |
        ButtonReleaseMask;
      windows->backdrop.attributes.event_mask=ButtonPressMask | KeyPressMask |
        StructureNotifyMask;
      manager_hints->flags=IconWindowHint | InputHint | StateHint;
      manager_hints->icon_window=windows->icon.id;
      manager_hints->input=MagickTrue;
      manager_hints->initial_state=resource_info->iconic ? IconicState :
        NormalState;
      XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
        &windows->backdrop);
      if (display_image->debug != MagickFalse )
        (void) LogMagickEvent(X11Event,GetMagickModule(),
          "Window id: 0x%lx (backdrop)",windows->backdrop.id);
      (void) XMapWindow(display,windows->backdrop.id);
      (void) XClearWindow(display,windows->backdrop.id);
      if (windows->image.id != (Window) NULL)
        {
          (void) XDestroyWindow(display,windows->image.id);
          windows->image.id=(Window) NULL;
        }
      /*
        Position image in the center the backdrop.
      */
      windows->image.flags|=USPosition;
      windows->image.x=(XDisplayWidth(display,visual_info->screen)/2)-
        (windows->image.width/2);
      windows->image.y=(XDisplayHeight(display,visual_info->screen)/2)-
        (windows->image.height/2);
    }
  manager_hints->flags=IconWindowHint | InputHint | StateHint;
  manager_hints->icon_window=windows->icon.id;
  manager_hints->input=MagickTrue;
  manager_hints->initial_state=resource_info->iconic ? IconicState :
    NormalState;
  if (windows->group_leader.id != (Window) NULL)
    {
      /*
        Follow the leader.
      */
      manager_hints->flags|=WindowGroupHint;
      manager_hints->window_group=windows->group_leader.id;
      (void) XSelectInput(display,windows->group_leader.id,StructureNotifyMask);
      if (display_image->debug != MagickFalse )
        (void) LogMagickEvent(X11Event,GetMagickModule(),
          "Window id: 0x%lx (group leader)",windows->group_leader.id);
    }
  XMakeWindow(display,
    (Window) (resource_info->backdrop ? windows->backdrop.id : root_window),
    argv,argc,class_hints,manager_hints,&windows->image);
  (void) XChangeProperty(display,windows->image.id,windows->im_protocols,
    XA_STRING,8,PropModeReplace,(unsigned char *) NULL,0);
  if (windows->group_leader.id != (Window) NULL)
    (void) XSetTransientForHint(display,windows->image.id,
      windows->group_leader.id);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),"Window id: 0x%lx (image)",
      windows->image.id);
  /*
    Initialize Info widget.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,resource_info,
    &windows->info);
  (void) CloneString(&windows->info.name,"Info");
  (void) CloneString(&windows->info.icon_name,"Info");
  windows->info.border_width=1;
  windows->info.x=2;
  windows->info.y=2;
  windows->info.flags|=PPosition;
  windows->info.attributes.win_gravity=UnmapGravity;
  windows->info.attributes.event_mask=ButtonPressMask | ExposureMask |
    StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=MagickFalse;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,windows->image.id,argv,argc,class_hints,manager_hints,
    &windows->info);
  windows->info.highlight_stipple=XCreateBitmapFromData(display,
    windows->info.id,(char *) HighlightBitmap,HighlightWidth,HighlightHeight);
  windows->info.shadow_stipple=XCreateBitmapFromData(display,
    windows->info.id,(char *) ShadowBitmap,ShadowWidth,ShadowHeight);
  (void) XSetTransientForHint(display,windows->info.id,windows->image.id);
  if (windows->image.mapped != MagickFalse )
    (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),"Window id: 0x%lx (info)",
      windows->info.id);
  /*
    Initialize Command widget.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->command);
  windows->command.data=MagickMenus;
  (void) XCommandWidget(display,windows,CommandMenu,(XEvent *) NULL);
  (void) FormatLocaleString(resource_name,MagickPathExtent,"%s.command",
    resource_info->client_name);
  windows->command.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  (void) CloneString(&windows->command.name,MagickTitle);
  windows->command.border_width=0;
  windows->command.flags|=PPosition;
  windows->command.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | LeaveWindowMask |
    OwnerGrabButtonMask | StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=MagickTrue;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->command);
  windows->command.highlight_stipple=XCreateBitmapFromData(display,
    windows->command.id,(char *) HighlightBitmap,HighlightWidth,
    HighlightHeight);
  windows->command.shadow_stipple=XCreateBitmapFromData(display,
    windows->command.id,(char *) ShadowBitmap,ShadowWidth,ShadowHeight);
  (void) XSetTransientForHint(display,windows->command.id,windows->image.id);
  if (windows->command.mapped != MagickFalse )
    (void) XMapRaised(display,windows->command.id);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Window id: 0x%lx (command)",windows->command.id);
  /*
    Initialize Widget window.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->widget);
  (void) FormatLocaleString(resource_name,MagickPathExtent,"%s.widget",
    resource_info->client_name);
  windows->widget.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  windows->widget.border_width=0;
  windows->widget.flags|=PPosition;
  windows->widget.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | KeyPressMask |
    KeyReleaseMask | LeaveWindowMask | OwnerGrabButtonMask |
    StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=MagickTrue;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->widget);
  windows->widget.highlight_stipple=XCreateBitmapFromData(display,
    windows->widget.id,(char *) HighlightBitmap,HighlightWidth,HighlightHeight);
  windows->widget.shadow_stipple=XCreateBitmapFromData(display,
    windows->widget.id,(char *) ShadowBitmap,ShadowWidth,ShadowHeight);
  (void) XSetTransientForHint(display,windows->widget.id,windows->image.id);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Window id: 0x%lx (widget)",windows->widget.id);
  /*
    Initialize popup window.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->popup);
  windows->popup.border_width=0;
  windows->popup.flags|=PPosition;
  windows->popup.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | KeyPressMask |
    KeyReleaseMask | LeaveWindowMask | StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=MagickTrue;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->popup);
  windows->popup.highlight_stipple=XCreateBitmapFromData(display,
    windows->popup.id,(char *) HighlightBitmap,HighlightWidth,HighlightHeight);
  windows->popup.shadow_stipple=XCreateBitmapFromData(display,
    windows->popup.id,(char *) ShadowBitmap,ShadowWidth,ShadowHeight);
  (void) XSetTransientForHint(display,windows->popup.id,windows->image.id);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Window id: 0x%lx (pop up)",windows->popup.id);
  /*
    Initialize Magnify window and cursor.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->magnify);
  if (resource_info->use_shared_memory == MagickFalse)
    windows->magnify.shared_memory=MagickFalse;
  (void) FormatLocaleString(resource_name,MagickPathExtent,"%s.magnify",
    resource_info->client_name);
  windows->magnify.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  (void) FormatLocaleString(windows->magnify.name,MagickPathExtent,"Magnify %uX",
    resource_info->magnify);
  if (windows->magnify.cursor != (Cursor) NULL)
    (void) XFreeCursor(display,windows->magnify.cursor);
  windows->magnify.cursor=XMakeCursor(display,windows->image.id,
    map_info->colormap,resource_info->background_color,
    resource_info->foreground_color);
  if (windows->magnify.cursor == (Cursor) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateCursor",
      display_image->filename);
  windows->magnify.width=MagnifySize;
  windows->magnify.height=MagnifySize;
  windows->magnify.flags|=PPosition;
  windows->magnify.min_width=MagnifySize;
  windows->magnify.min_height=MagnifySize;
  windows->magnify.width_inc=MagnifySize;
  windows->magnify.height_inc=MagnifySize;
  windows->magnify.data=resource_info->magnify;
  windows->magnify.attributes.cursor=windows->magnify.cursor;
  windows->magnify.attributes.event_mask=ButtonPressMask | ButtonReleaseMask |
    ExposureMask | KeyPressMask | KeyReleaseMask | OwnerGrabButtonMask |
    StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=MagickTrue;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->magnify);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Window id: 0x%lx (magnify)",windows->magnify.id);
  (void) XSetTransientForHint(display,windows->magnify.id,windows->image.id);
  /*
    Initialize panning window.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->pan);
  (void) CloneString(&windows->pan.name,"Pan Icon");
  windows->pan.width=windows->icon.width;
  windows->pan.height=windows->icon.height;
  (void) FormatLocaleString(resource_name,MagickPathExtent,"%s.pan",
    resource_info->client_name);
  windows->pan.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  (void) XParseGeometry(windows->pan.geometry,&windows->pan.x,&windows->pan.y,
    &windows->pan.width,&windows->pan.height);
  windows->pan.flags|=PPosition;
  windows->pan.immutable=MagickTrue;
  windows->pan.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | ExposureMask | KeyPressMask | KeyReleaseMask |
    StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=MagickFalse;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->pan);
  if (display_image->debug != MagickFalse )
    (void) LogMagickEvent(X11Event,GetMagickModule(),"Window id: 0x%lx (pan)",
      windows->pan.id);
  (void) XSetTransientForHint(display,windows->pan.id,windows->image.id);
  if (windows->info.mapped != MagickFalse )
    (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if ((windows->image.mapped == MagickFalse) ||
      (windows->backdrop.id != (Window) NULL))
    (void) XMapWindow(display,windows->image.id);
  /*
    Set our progress monitor and warning handlers.
  */
  if (warning_handler == (WarningHandler) NULL)
    {
      warning_handler=resource_info->display_warnings ?
        SetErrorHandler(XWarning) : SetErrorHandler((ErrorHandler) NULL);
      warning_handler=resource_info->display_warnings ?
        SetWarningHandler(XWarning) : SetWarningHandler((WarningHandler) NULL);
    }
  /*
    Initialize Image and Magnify X images.
  */
  windows->image.x=0;
  windows->image.y=0;
  windows->magnify.shape=MagickFalse;
  width=(unsigned int) display_image->columns;
  height=(unsigned int) display_image->rows;
  if ((display_image->columns != width) || (display_image->rows != height))
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
      display_image->filename);
  status=XMakeImage(display,resource_info,&windows->image,display_image,
    width,height,exception);
  if (status == MagickFalse)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
      display_image->filename);
  status=XMakeImage(display,resource_info,&windows->magnify,(Image *) NULL,
    windows->magnify.width,windows->magnify.height,exception);
  if (status == MagickFalse)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
      display_image->filename);
  if (windows->magnify.mapped != MagickFalse )
    (void) XMapRaised(display,windows->magnify.id);
  if (windows->pan.mapped != MagickFalse )
    (void) XMapRaised(display,windows->pan.id);
  windows->image.window_changes.width=(int) display_image->columns;
  windows->image.window_changes.height=(int) display_image->rows;
  (void) XConfigureImage(display,resource_info,windows,display_image,exception);
  (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
  (void) XSync(display,MagickFalse);
  /*
    Respond to events.
  */
  delay=display_image->delay/MagickMax(display_image->ticks_per_second,1L);
  timer=time((time_t *) NULL)+(delay == 0 ? 1 : delay)+1;
  update_time=0;
  if (resource_info->update != MagickFalse )
    {
      MagickBooleanType
        status;

      /*
        Determine when file data was last modified.
      */
      status=GetPathAttributes(display_image->filename,&attributes);
      if (status != MagickFalse )
        update_time=attributes.st_mtime;
    }
  *state&=(~FormerImageState);
  *state&=(~MontageImageState);
  *state&=(~NextImageState);
  do
  {
    /*
      Handle a window event.
    */
    if (windows->image.mapped != MagickFalse )
      if ((display_image->delay != 0) || (resource_info->update != 0))
        {
          if (timer < time((time_t *) NULL))
            {
              if (resource_info->update == MagickFalse)
                *state|=NextImageState | ExitState;
              else
                {
                  MagickBooleanType
                    status;

                  /*
                    Determine if image file was modified.
                  */
                  status=GetPathAttributes(display_image->filename,&attributes);
                  if (status != MagickFalse )
                    if (update_time != attributes.st_mtime)
                      {
                        /*
                          Redisplay image.
                        */
                        (void) FormatLocaleString(
                          resource_info->image_info->filename,MagickPathExtent,
                          "%s:%s",display_image->magick,
                          display_image->filename);
                        nexus=ReadImage(resource_info->image_info,exception);
                        if (nexus != (Image *) NULL)
                          *state|=NextImageState | ExitState;
                      }
                  delay=display_image->delay/MagickMax(
                    display_image->ticks_per_second,1L);
                  timer=time((time_t *) NULL)+(delay == 0 ? 1 : delay)+1;
                }
            }
          if (XEventsQueued(display,QueuedAfterFlush) == 0)
            {
              /*
                Do not block if delay > 0.
              */
              XDelay(display,SuspendTime << 2);
              continue;
            }
        }
    timestamp=time((time_t *) NULL);
    (void) XNextEvent(display,&event);
    if ((windows->image.stasis == MagickFalse) ||
        (windows->magnify.stasis == MagickFalse))
      {
        if ((time((time_t *) NULL)-timestamp) > 0)
          {
            windows->image.stasis=MagickTrue;
            windows->magnify.stasis=MagickTrue;
          }
      }
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,CommandMenu,&event);
        if (id < 0)
          continue;
        (void) CopyMagickString(command,CommandMenu[id],MagickPathExtent);
        command_type=CommandMenus[id];
        if (id < MagickMenus)
          {
            /*
              Select a command from a pop-up menu.
            */
            entry=XMenuWidget(display,windows,CommandMenu[id],Menus[id],
              command);
            if (entry < 0)
              continue;
            (void) CopyMagickString(command,Menus[id][entry],MagickPathExtent);
            command_type=Commands[id][entry];
          }
        if (command_type != NullCommand)
          nexus=XMagickCommand(display,resource_info,windows,command_type,
            &display_image,exception);
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Button Press: 0x%lx %u +%d+%d",event.xbutton.window,
            event.xbutton.button,event.xbutton.x,event.xbutton.y);
        if ((event.xbutton.button == Button3) &&
            (event.xbutton.state & Mod1Mask))
          {
            /*
              Convert Alt-Button3 to Button2.
            */
            event.xbutton.button=Button2;
            event.xbutton.state&=(~Mod1Mask);
          }
        if (event.xbutton.window == windows->backdrop.id)
          {
            (void) XSetInputFocus(display,event.xbutton.window,RevertToParent,
              event.xbutton.time);
            break;
          }
        if (event.xbutton.window == windows->image.id)
          {
            switch (event.xbutton.button)
            {
              case Button1:
              {
                if (resource_info->immutable)
                  {
                    /*
                      Select a command from the Virtual menu.
                    */
                    entry=XMenuWidget(display,windows,"Commands",VirtualMenu,
                      command);
                    if (entry >= 0)
                      nexus=XMagickCommand(display,resource_info,windows,
                        VirtualCommands[entry],&display_image,exception);
                    break;
                  }
                /*
                  Map/unmap Command widget.
                */
                if (windows->command.mapped != MagickFalse )
                  (void) XWithdrawWindow(display,windows->command.id,
                    windows->command.screen);
                else
                  {
                    (void) XCommandWidget(display,windows,CommandMenu,
                      (XEvent *) NULL);
                    (void) XMapRaised(display,windows->command.id);
                  }
                break;
              }
              case Button2:
              {
                /*
                  User pressed the image magnify button.
                */
                (void) XMagickCommand(display,resource_info,windows,ZoomCommand,
                  &display_image,exception);
                XMagnifyImage(display,windows,&event,exception);
                break;
              }
              case Button3:
              {
                if (resource_info->immutable)
                  {
                    /*
                      Select a command from the Virtual menu.
                    */
                    entry=XMenuWidget(display,windows,"Commands",VirtualMenu,
                      command);
                    if (entry >= 0)
                      nexus=XMagickCommand(display,resource_info,windows,
                        VirtualCommands[entry],&display_image,exception);
                    break;
                  }
                if (display_image->montage != (char *) NULL)
                  {
                    /*
                      Open or delete a tile from a visual image directory.
                    */
                    nexus=XTileImage(display,resource_info,windows,
                      display_image,&event,exception);
                    if (nexus != (Image *) NULL)
                      *state|=MontageImageState | NextImageState | ExitState;
                    vid_info.x=(short int) windows->image.x;
                    vid_info.y=(short int) windows->image.y;
                    break;
                  }
                /*
                  Select a command from the Short Cuts menu.
                */
                entry=XMenuWidget(display,windows,"Short Cuts",ShortCutsMenu,
                  command);
                if (entry >= 0)
                  nexus=XMagickCommand(display,resource_info,windows,
                    ShortCutsCommands[entry],&display_image,exception);
                break;
              }
              case Button4:
              {
                /*
                  Wheel up.
                */
                XTranslateImage(display,windows,*image,XK_Up);
                break;
              }
              case Button5:
              {
                /*
                  Wheel down.
                */
                XTranslateImage(display,windows,*image,XK_Down);
                break;
              }
              default:
                break;
            }
            break;
          }
        if (event.xbutton.window == windows->magnify.id)
          {
            int
              factor;

            static const char
              *MagnifyMenu[] =
              {
                "2",
                "4",
                "5",
                "6",
                "7",
                "8",
                "9",
                "3",
                (char *) NULL,
              };

            static KeySym
              MagnifyCommands[] =
              {
                XK_2,
                XK_4,
                XK_5,
                XK_6,
                XK_7,
                XK_8,
                XK_9,
                XK_3
              };

            /*
              Select a magnify factor from the pop-up menu.
            */
            factor=XMenuWidget(display,windows,"Magnify",MagnifyMenu,command);
            if (factor >= 0)
              XMagnifyWindowCommand(display,windows,0,MagnifyCommands[factor],
                exception);
            break;
          }
        if (event.xbutton.window == windows->pan.id)
          {
            switch (event.xbutton.button)
            {
              case Button4:
              {
                /*
                  Wheel up.
                */
                XTranslateImage(display,windows,*image,XK_Up);
                break;
              }
              case Button5:
              {
                /*
                  Wheel down.
                */
                XTranslateImage(display,windows,*image,XK_Down);
                break;
              }
              default:
              {
                XPanImage(display,windows,&event,exception);
                break;
              }
            }
            break;
          }
        delay=display_image->delay/MagickMax(display_image->ticks_per_second,
          1L);
        timer=time((time_t *) NULL)+(delay == 0 ? 1 : delay)+1;
        break;
      }
      case ButtonRelease:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Button Release: 0x%lx %u +%d+%d",event.xbutton.window,
            event.xbutton.button,event.xbutton.x,event.xbutton.y);
        break;
      }
      case ClientMessage:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Client Message: 0x%lx 0x%lx %d 0x%lx",event.xclient.window,
            event.xclient.message_type,event.xclient.format,(unsigned long)
            event.xclient.data.l[0]);
        if (event.xclient.message_type == windows->im_protocols)
          {
            if (*event.xclient.data.l == (long) windows->im_update_widget)
              {
                (void) CloneString(&windows->command.name,MagickTitle);
                windows->command.data=MagickMenus;
                (void) XCommandWidget(display,windows,CommandMenu,
                  (XEvent *) NULL);
                break;
              }
            if (*event.xclient.data.l == (long) windows->im_update_colormap)
              {
                /*
                  Update graphic context and window colormap.
                */
                for (i=0; i < (int) number_windows; i++)
                {
                  if (magick_windows[i]->id == windows->icon.id)
                    continue;
                  context_values.background=pixel->background_color.pixel;
                  context_values.foreground=pixel->foreground_color.pixel;
                  (void) XChangeGC(display,magick_windows[i]->annotate_context,
                    context_mask,&context_values);
                  (void) XChangeGC(display,magick_windows[i]->widget_context,
                    context_mask,&context_values);
                  context_values.background=pixel->foreground_color.pixel;
                  context_values.foreground=pixel->background_color.pixel;
                  context_values.plane_mask=context_values.background ^
                    context_values.foreground;
                  (void) XChangeGC(display,magick_windows[i]->highlight_context,
                    (size_t) (context_mask | GCPlaneMask),
                    &context_values);
                  magick_windows[i]->attributes.background_pixel=
                    pixel->background_color.pixel;
                  magick_windows[i]->attributes.border_pixel=
                    pixel->border_color.pixel;
                  magick_windows[i]->attributes.colormap=map_info->colormap;
                  (void) XChangeWindowAttributes(display,magick_windows[i]->id,
                    (unsigned long) magick_windows[i]->mask,
                    &magick_windows[i]->attributes);
                }
                if (windows->pan.mapped != MagickFalse )
                  {
                    (void) XSetWindowBackgroundPixmap(display,windows->pan.id,
                      windows->pan.pixmap);
                    (void) XClearWindow(display,windows->pan.id);
                    XDrawPanRectangle(display,windows);
                  }
                if (windows->backdrop.id != (Window) NULL)
                  (void) XInstallColormap(display,map_info->colormap);
                break;
              }
            if (*event.xclient.data.l == (long) windows->im_former_image)
              {
                *state|=FormerImageState | ExitState;
                break;
              }
            if (*event.xclient.data.l == (long) windows->im_next_image)
              {
                *state|=NextImageState | ExitState;
                break;
              }
            if (*event.xclient.data.l == (long) windows->im_retain_colors)
              {
                *state|=RetainColorsState;
                break;
              }
            if (*event.xclient.data.l == (long) windows->im_exit)
              {
                *state|=ExitState;
                break;
              }
            break;
          }
        if (event.xclient.message_type == windows->dnd_protocols)
          {
            Atom
              selection,
              type;

            int
              format,
              status;

            unsigned char
              *data;

            unsigned long
              after,
              length;

            /*
              Display image named by the Drag-and-Drop selection.
            */
            if ((*event.xclient.data.l != 2) && (*event.xclient.data.l != 128))
              break;
            selection=XInternAtom(display,"DndSelection",MagickFalse);
            status=XGetWindowProperty(display,root_window,selection,0L,(long)
              MagickPathExtent,MagickFalse,(Atom) AnyPropertyType,&type,&format,
              &length,&after,&data);
            if ((status != Success) || (length == 0))
              break;
            if (*event.xclient.data.l == 2)
              {
                /*
                  Offix DND.
                */
                (void) CopyMagickString(resource_info->image_info->filename,
                  (char *) data,MagickPathExtent);
              }
            else
              {
                /*
                  XDND.
                */
                if (strncmp((char *) data, "file:", 5) != 0)
                  {
                    (void) XFree((void *) data);
                    break;
                  }
                (void) CopyMagickString(resource_info->image_info->filename,
                  ((char *) data)+5,MagickPathExtent);
              }
            nexus=ReadImage(resource_info->image_info,exception);
            CatchException(exception);
            if (nexus != (Image *) NULL)
              *state|=NextImageState | ExitState;
            (void) XFree((void *) data);
            break;
          }
        /*
          If client window delete message, exit.
        */
        if (event.xclient.message_type != windows->wm_protocols)
          break;
        if (*event.xclient.data.l != (long) windows->wm_delete_window)
          break;
        (void) XWithdrawWindow(display,event.xclient.window,
          visual_info->screen);
        if (event.xclient.window == windows->image.id)
          {
            *state|=ExitState;
            break;
          }
        if (event.xclient.window == windows->pan.id)
          {
            /*
              Restore original image size when pan window is deleted.
            */
            windows->image.window_changes.width=windows->image.ximage->width;
            windows->image.window_changes.height=windows->image.ximage->height;
            (void) XConfigureImage(display,resource_info,windows,
              display_image,exception);
          }
        break;
      }
      case ConfigureNotify:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Configure Notify: 0x%lx %dx%d+%d+%d %d",event.xconfigure.window,
            event.xconfigure.width,event.xconfigure.height,event.xconfigure.x,
            event.xconfigure.y,event.xconfigure.send_event);
        if (event.xconfigure.window == windows->image.id)
          {
            /*
              Image window has a new configuration.
            */
            if (event.xconfigure.send_event != 0)
              {
                XWindowChanges
                  window_changes;

                /*
                  Position the transient windows relative of the Image window.
                */
                if (windows->command.geometry == (char *) NULL)
                  if (windows->command.mapped == MagickFalse)
                    {
                      windows->command.x=event.xconfigure.x-
                        windows->command.width-25;
                      windows->command.y=event.xconfigure.y;
                      XConstrainWindowPosition(display,&windows->command);
                      window_changes.x=windows->command.x;
                      window_changes.y=windows->command.y;
                      (void) XReconfigureWMWindow(display,windows->command.id,
                        windows->command.screen,(unsigned int) (CWX | CWY),
                        &window_changes);
                    }
                if (windows->widget.geometry == (char *) NULL)
                  if (windows->widget.mapped == MagickFalse)
                    {
                      windows->widget.x=event.xconfigure.x+
                        event.xconfigure.width/10;
                      windows->widget.y=event.xconfigure.y+
                        event.xconfigure.height/10;
                      XConstrainWindowPosition(display,&windows->widget);
                      window_changes.x=windows->widget.x;
                      window_changes.y=windows->widget.y;
                      (void) XReconfigureWMWindow(display,windows->widget.id,
                        windows->widget.screen,(unsigned int) (CWX | CWY),
                        &window_changes);
                    }
                if (windows->magnify.geometry == (char *) NULL)
                  if (windows->magnify.mapped == MagickFalse)
                    {
                      windows->magnify.x=event.xconfigure.x+
                        event.xconfigure.width+25;
                      windows->magnify.y=event.xconfigure.y;
                      XConstrainWindowPosition(display,&windows->magnify);
                      window_changes.x=windows->magnify.x;
                      window_changes.y=windows->magnify.y;
                      (void) XReconfigureWMWindow(display,windows->magnify.id,
                        windows->magnify.screen,(unsigned int) (CWX | CWY),
                        &window_changes);
                    }
                if (windows->pan.geometry == (char *) NULL)
                  if (windows->pan.mapped == MagickFalse)
                    {
                      windows->pan.x=event.xconfigure.x+
                        event.xconfigure.width+25;
                      windows->pan.y=event.xconfigure.y+
                        windows->magnify.height+50;
                      XConstrainWindowPosition(display,&windows->pan);
                      window_changes.x=windows->pan.x;
                      window_changes.y=windows->pan.y;
                      (void) XReconfigureWMWindow(display,windows->pan.id,
                        windows->pan.screen,(unsigned int) (CWX | CWY),
                        &window_changes);
                    }
              }
            if ((event.xconfigure.width == (int) windows->image.width) &&
                (event.xconfigure.height == (int) windows->image.height))
              break;
            windows->image.width=(unsigned int) event.xconfigure.width;
            windows->image.height=(unsigned int) event.xconfigure.height;
            windows->image.x=0;
            windows->image.y=0;
            if (display_image->montage != (char *) NULL)
              {
                windows->image.x=vid_info.x;
                windows->image.y=vid_info.y;
              }
            if (windows->image.mapped != MagickFalse &&
                windows->image.stasis != MagickFalse )
              {
                /*
                  Update image window configuration.
                */
                windows->image.window_changes.width=event.xconfigure.width;
                windows->image.window_changes.height=event.xconfigure.height;
                (void) XConfigureImage(display,resource_info,windows,
                  display_image,exception);
              }
            /*
              Update pan window configuration.
            */
            if ((event.xconfigure.width < windows->image.ximage->width) ||
                (event.xconfigure.height < windows->image.ximage->height))
              {
                (void) XMapRaised(display,windows->pan.id);
                XDrawPanRectangle(display,windows);
              }
            else
              if (windows->pan.mapped != MagickFalse )
                (void) XWithdrawWindow(display,windows->pan.id,
                  windows->pan.screen);
            break;
          }
        if (event.xconfigure.window == windows->magnify.id)
          {
            unsigned int
              magnify;

            /*
              Magnify window has a new configuration.
            */
            windows->magnify.width=(unsigned int) event.xconfigure.width;
            windows->magnify.height=(unsigned int) event.xconfigure.height;
            if (windows->magnify.mapped == MagickFalse)
              break;
            magnify=1;
            while ((int) magnify <= event.xconfigure.width)
              magnify<<=1;
            while ((int) magnify <= event.xconfigure.height)
              magnify<<=1;
            magnify>>=1;
            if (((int) magnify != event.xconfigure.width) ||
                ((int) magnify != event.xconfigure.height))
              {
                window_changes.width=(int) magnify;
                window_changes.height=(int) magnify;
                (void) XReconfigureWMWindow(display,windows->magnify.id,
                  windows->magnify.screen,(unsigned int) (CWWidth | CWHeight),
                  &window_changes);
                break;
              }
            if (windows->magnify.mapped != MagickFalse &&
                windows->magnify.stasis != MagickFalse )
              {
                status=XMakeImage(display,resource_info,&windows->magnify,
                  display_image,windows->magnify.width,windows->magnify.height,
                  exception);
                XMakeMagnifyImage(display,windows,exception);
              }
            break;
          }
        if (windows->magnify.mapped != MagickFalse &&
            (event.xconfigure.window == windows->pan.id))
          {
            /*
              Pan icon window has a new configuration.
            */
            if (event.xconfigure.send_event != 0)
              {
                windows->pan.x=event.xconfigure.x;
                windows->pan.y=event.xconfigure.y;
              }
            windows->pan.width=(unsigned int) event.xconfigure.width;
            windows->pan.height=(unsigned int) event.xconfigure.height;
            break;
          }
        if (event.xconfigure.window == windows->icon.id)
          {
            /*
              Icon window has a new configuration.
            */
            windows->icon.width=(unsigned int) event.xconfigure.width;
            windows->icon.height=(unsigned int) event.xconfigure.height;
            break;
          }
        break;
      }
      case DestroyNotify:
      {
        /*
          Group leader has exited.
        */
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Destroy Notify: 0x%lx",event.xdestroywindow.window);
        if (event.xdestroywindow.window == windows->group_leader.id)
          {
            *state|=ExitState;
            break;
          }
        break;
      }
      case EnterNotify:
      {
        /*
          Selectively install colormap.
        */
        if (map_info->colormap != XDefaultColormap(display,visual_info->screen))
          if (event.xcrossing.mode != NotifyUngrab)
            XInstallColormap(display,map_info->colormap);
        break;
      }
      case Expose:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Expose: 0x%lx %dx%d+%d+%d",event.xexpose.window,
            event.xexpose.width,event.xexpose.height,event.xexpose.x,
            event.xexpose.y);
        /*
          Refresh windows that are now exposed.
        */
        if ((event.xexpose.window == windows->image.id) &&
            windows->image.mapped != MagickFalse )
          {
            XRefreshWindow(display,&windows->image,&event);
            delay=display_image->delay/MagickMax(
              display_image->ticks_per_second,1L);
            timer=time((time_t *) NULL)+(delay == 0 ? 1 : delay)+1;
            break;
          }
        if ((event.xexpose.window == windows->magnify.id) &&
            windows->magnify.mapped != MagickFalse)
          {
            XMakeMagnifyImage(display,windows,exception);
            break;
          }
        if (event.xexpose.window == windows->pan.id)
          {
            XDrawPanRectangle(display,windows);
            break;
          }
        if (event.xexpose.window == windows->icon.id)
          {
            XRefreshWindow(display,&windows->icon,&event);
            break;
          }
        break;
      }
      case KeyPress:
      {
        int
          length;

        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Key press: %d 0x%lx (%s)",event.xkey.state,(unsigned long)
            key_symbol,command);
        if (event.xkey.window == windows->image.id)
          {
            command_type=XImageWindowCommand(display,resource_info,windows,
              event.xkey.state,key_symbol,&display_image,exception);
            if (command_type != NullCommand)
              nexus=XMagickCommand(display,resource_info,windows,command_type,
                &display_image,exception);
          }
        if (event.xkey.window == windows->magnify.id)
          XMagnifyWindowCommand(display,windows,event.xkey.state,key_symbol,
            exception);
        if (event.xkey.window == windows->pan.id)
          {
            if ((key_symbol == XK_q) || (key_symbol == XK_Escape))
              (void) XWithdrawWindow(display,windows->pan.id,
                windows->pan.screen);
            else
              if ((key_symbol == XK_F1) || (key_symbol == XK_Help))
                XTextViewWidget(display,resource_info,windows,MagickFalse,
                  "Help Viewer - Image Pan",ImagePanHelp);
              else
                XTranslateImage(display,windows,*image,key_symbol);
          }
        delay=display_image->delay/MagickMax(
          display_image->ticks_per_second,1L);
        timer=time((time_t *) NULL)+(delay == 0 ? 1 : delay)+1;
        break;
      }
      case KeyRelease:
      {
        /*
          Respond to a user key release.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Key release: 0x%lx (%c)",(unsigned long) key_symbol,*command);
        break;
      }
      case LeaveNotify:
      {
        /*
          Selectively uninstall colormap.
        */
        if (map_info->colormap != XDefaultColormap(display,visual_info->screen))
          if (event.xcrossing.mode != NotifyUngrab)
            XUninstallColormap(display,map_info->colormap);
        break;
      }
      case MapNotify:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),"Map Notify: 0x%lx",
            event.xmap.window);
        if (event.xmap.window == windows->backdrop.id)
          {
            (void) XSetInputFocus(display,event.xmap.window,RevertToParent,
              CurrentTime);
            windows->backdrop.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->image.id)
          {
            if (windows->backdrop.id != (Window) NULL)
              (void) XInstallColormap(display,map_info->colormap);
            if (LocaleCompare(display_image->magick,"LOGO") == 0)
              {
                if (LocaleCompare(display_image->filename,"LOGO") == 0)
                  nexus=XOpenImage(display,resource_info,windows,MagickFalse);
              }
            if (((int) windows->image.width < windows->image.ximage->width) ||
                ((int) windows->image.height < windows->image.ximage->height))
              (void) XMapRaised(display,windows->pan.id);
            windows->image.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->magnify.id)
          {
            XMakeMagnifyImage(display,windows,exception);
            windows->magnify.mapped=MagickTrue;
            (void) XWithdrawWindow(display,windows->info.id,
              windows->info.screen);
            break;
          }
        if (event.xmap.window == windows->pan.id)
          {
            XMakePanImage(display,resource_info,windows,display_image,
              exception);
            windows->pan.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->info.id)
          {
            windows->info.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->icon.id)
          {
            MagickBooleanType
              taint;

            /*
              Create an icon image.
            */
            taint=display_image->taint;
            XMakeStandardColormap(display,icon_visual,icon_resources,
              display_image,icon_map,icon_pixel,exception);
            (void) XMakeImage(display,icon_resources,&windows->icon,
              display_image,windows->icon.width,windows->icon.height,
              exception);
            display_image->taint=taint;
            (void) XSetWindowBackgroundPixmap(display,windows->icon.id,
              windows->icon.pixmap);
            (void) XClearWindow(display,windows->icon.id);
            (void) XWithdrawWindow(display,windows->info.id,
              windows->info.screen);
            windows->icon.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->command.id)
          {
            windows->command.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->popup.id)
          {
            windows->popup.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->widget.id)
          {
            windows->widget.mapped=MagickTrue;
            break;
          }
        break;
      }
      case MappingNotify:
      {
        (void) XRefreshKeyboardMapping(&event.xmapping);
        break;
      }
      case NoExpose:
        break;
      case PropertyNotify:
      {
        Atom
          type;

        int
          format,
          status;

        unsigned char
          *data;

        unsigned long
          after,
          length;

        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Property Notify: 0x%lx 0x%lx %d",event.xproperty.window,
            event.xproperty.atom,event.xproperty.state);
        if (event.xproperty.atom != windows->im_remote_command)
          break;
        /*
          Display image named by the remote command protocol.
        */
        status=XGetWindowProperty(display,event.xproperty.window,
          event.xproperty.atom,0L,(long) MagickPathExtent,MagickFalse,(Atom)
          AnyPropertyType,&type,&format,&length,&after,&data);
        if ((status != Success) || (length == 0))
          break;
        if (LocaleCompare((char *) data,"-quit") == 0)
          {
            XClientMessage(display,windows->image.id,windows->im_protocols,
              windows->im_exit,CurrentTime);
            (void) XFree((void *) data);
            break;
          }
        (void) CopyMagickString(resource_info->image_info->filename,
          (char *) data,MagickPathExtent);
        (void) XFree((void *) data);
        nexus=ReadImage(resource_info->image_info,exception);
        CatchException(exception);
        if (nexus != (Image *) NULL)
          *state|=NextImageState | ExitState;
        break;
      }
      case ReparentNotify:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Reparent Notify: 0x%lx=>0x%lx",event.xreparent.parent,
            event.xreparent.window);
        break;
      }
      case UnmapNotify:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Unmap Notify: 0x%lx",event.xunmap.window);
        if (event.xunmap.window == windows->backdrop.id)
          {
            windows->backdrop.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->image.id)
          {
            windows->image.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->magnify.id)
          {
            windows->magnify.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->pan.id)
          {
            windows->pan.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->info.id)
          {
            windows->info.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->icon.id)
          {
            if (map_info->colormap == icon_map->colormap)
              XConfigureImageColormap(display,resource_info,windows,
                display_image,exception);
            (void) XFreeStandardColormap(display,icon_visual,icon_map,
              icon_pixel);
            windows->icon.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->command.id)
          {
            windows->command.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->popup.id)
          {
            if (windows->backdrop.id != (Window) NULL)
              (void) XSetInputFocus(display,windows->image.id,RevertToParent,
                CurrentTime);
            windows->popup.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->widget.id)
          {
            if (windows->backdrop.id != (Window) NULL)
              (void) XSetInputFocus(display,windows->image.id,RevertToParent,
                CurrentTime);
            windows->widget.mapped=MagickFalse;
            break;
          }
        break;
      }
      default:
      {
        if (display_image->debug != MagickFalse )
          (void) LogMagickEvent(X11Event,GetMagickModule(),"Event type: %d",
            event.type);
        break;
      }
    }
  } while (!(*state & ExitState));
  if ((*state & ExitState) == 0)
    (void) XMagickCommand(display,resource_info,windows,FreeBuffersCommand,
      &display_image,exception);
  else
    if (resource_info->confirm_edit != MagickFalse )
      {
        /*
          Query user if image has changed.
        */
        if ((resource_info->immutable == MagickFalse) &&
            display_image->taint != MagickFalse)
          {
            int
              status;

            status=XConfirmWidget(display,windows,"Your image changed.",
              "Do you want to save it");
            if (status == 0)
              *state&=(~ExitState);
            else
              if (status > 0)
                (void) XMagickCommand(display,resource_info,windows,SaveCommand,
                  &display_image,exception);
          }
      }
  if ((windows->visual_info->klass == GrayScale) ||
      (windows->visual_info->klass == PseudoColor) ||
      (windows->visual_info->klass == DirectColor))
    {
      /*
        Withdraw pan and Magnify window.
      */
      if (windows->info.mapped != MagickFalse )
        (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      if (windows->magnify.mapped != MagickFalse )
        (void) XWithdrawWindow(display,windows->magnify.id,
          windows->magnify.screen);
      if (windows->command.mapped != MagickFalse )
        (void) XWithdrawWindow(display,windows->command.id,
          windows->command.screen);
    }
  if (windows->pan.mapped != MagickFalse )
    (void) XWithdrawWindow(display,windows->pan.id,windows->pan.screen);
  if (resource_info->backdrop == MagickFalse)
    if (windows->backdrop.mapped)
      {
        (void) XWithdrawWindow(display,windows->backdrop.id,
          windows->backdrop.screen);
        (void) XDestroyWindow(display,windows->backdrop.id);
        windows->backdrop.id=(Window) NULL;
        (void) XWithdrawWindow(display,windows->image.id,
          windows->image.screen);
        (void) XDestroyWindow(display,windows->image.id);
        windows->image.id=(Window) NULL;
      }
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  if (((*state & FormerImageState) != 0) || ((*state & NextImageState) != 0))
    *state&=(~ExitState);
  if (*state & ExitState)
    {
      /*
        Free Standard Colormap.
      */
      (void) XFreeStandardColormap(display,icon_visual,icon_map,icon_pixel);
      if (resource_info->map_type == (char *) NULL)
        (void) XFreeStandardColormap(display,visual_info,map_info,pixel);
      /*
        Free X resources.
      */
      if (resource_info->copy_image != (Image *) NULL)
        {
          resource_info->copy_image=DestroyImage(resource_info->copy_image);
          resource_info->copy_image=NewImageList();
        }
      DestroyXResources();
    }
  (void) XSync(display,MagickFalse);
  /*
    Restore our progress monitor and warning handlers.
  */
  (void) SetErrorHandler(warning_handler);
  (void) SetWarningHandler(warning_handler);
  /*
    Change to home directory.
  */
  directory=getcwd(working_directory,MagickPathExtent);
  (void) directory;
  {
    int
      status;

    if (*resource_info->home_directory == '\0')
      (void) CopyMagickString(resource_info->home_directory,".",MagickPathExtent);
    status=chdir(resource_info->home_directory);
    if (status == -1)
      (void) ThrowMagickException(exception,GetMagickModule(),FileOpenError,
        "UnableToOpenFile","%s",resource_info->home_directory);
  }
  *image=display_image;
  return(nexus);
}
#else

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s p l a y I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DisplayImages() displays an image sequence to any X window screen.  It
%  returns a value other than 0 if successful.  Check the exception member
%  of image to determine the reason for any failure.
%
%  The format of the DisplayImages method is:
%
%      MagickBooleanType DisplayImages(const ImageInfo *image_info,
%        Image *images,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType DisplayImages(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  (void) image_info;
  if (image->debug != MagickFalse )
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (X11)",image->filename);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e m o t e D i s p l a y C o m m a n d                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoteDisplayCommand() encourages a remote display program to display the
%  specified image filename.
%
%  The format of the RemoteDisplayCommand method is:
%
%      MagickBooleanType RemoteDisplayCommand(const ImageInfo *image,
%        const char *window,const char *filename,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o window: Specifies the name or id of an X window.
%
%    o filename: the name of the image filename to display.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType RemoteDisplayCommand(const ImageInfo *image_info,
  const char *window,const char *filename,ExceptionInfo *exception)
{
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(filename != (char *) NULL);
  (void) window;
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (X11)",image_info->filename);
  return(MagickFalse);
}
#endif

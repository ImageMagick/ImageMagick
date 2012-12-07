/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                AAA   N   N  IIIII  M   M   AAA   TTTTT  EEEEE               %
%               A   A  NN  N    I    MM MM  A   A    T    E                   %
%               AAAAA  N N N    I    M M M  AAAAA    T    EEE                 %
%               A   A  N  NN    I    M   M  A   A    T    E                   %
%               A   A  N   N  IIIII  M   M  A   A    T    EEEEE               %
%                                                                             %
%                                                                             %
%              Methods to Interactively Animate an Image Sequence             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                                July 1992                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/animate.h"
#include "magick/animate-private.h"
#include "magick/attribute.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/image.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/property.h"
#include "magick/resource_.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/transform.h"
#include "magick/utility.h"
#include "magick/version.h"
#include "magick/widget.h"
#include "magick/xwindow-private.h"

#if defined(MAGICKCORE_X11_DELEGATE)
/*
  Animate state declarations.
*/
#define AutoReverseAnimationState 0x0004
#define ForwardAnimationState 0x0008
#define HighlightState  0x0010
#define PlayAnimationState 0x0020
#define RepeatAnimationState 0x0040
#define StepAnimationState 0x0080

/*
  Static declarations.
*/
static const char
  *AnimateHelp[]=
  {
    "BUTTONS",
    "",
    "  Press any button to map or unmap the Command widget.",
    "",
    "COMMAND WIDGET",
    "  The Command widget lists a number of sub-menus and commands.",
    "  They are",
    "",
    "    Animate",
    "      Open...",
    "      Save...",
    "      Play",
    "      Step",
    "      Repeat",
    "      Auto Reverse",
    "    Speed",
    "      Slower",
    "      Faster",
    "    Direction",
    "      Forward",
    "      Reverse",
    "      Help",
    "        Overview",
    "        Browse Documentation",
    "        About Animate",
    "    Image Info",
    "    Quit",
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
    "  animate(1) understands is:",
    "",
    "  Ctl+O  Press to open an image from a file.",
    "",
    "  space  Press to display the next image in the sequence.",
    "",
    "  <      Press to speed-up the display of the images.  Refer to",
    "         -delay for more information.",
    "",
    "  >      Press to slow the display of the images.  Refer to",
    "         -delay for more information.",
    "",
    "  F1     Press to display helpful information about animate(1).",
    "",
    "  Find   Press to browse documentation about ImageMagick.",
    "",
    "  ?      Press to display information about the image.  Press",
    "         any key or button to erase the information.",
    "",
    "         This information is printed: image name;  image size;",
    "         and the total number of unique colors in the image.",
    "",
    "  Ctl-q  Press to discard all images and exit program.",
    (char *) NULL
  };

/*
  Constant declarations.
*/
static const char
  *PageSizes[]=
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

static const unsigned char
  HighlightBitmap[8] =
  {
    (unsigned char) 0xaa,
    (unsigned char) 0x55,
    (unsigned char) 0xaa,
    (unsigned char) 0x55,
    (unsigned char) 0xaa,
    (unsigned char) 0x55,
    (unsigned char) 0xaa,
    (unsigned char) 0x55
  },
  ShadowBitmap[8] =
  {
    (unsigned char) 0x00,
    (unsigned char) 0x00,
    (unsigned char) 0x00,
    (unsigned char) 0x00,
    (unsigned char) 0x00,
    (unsigned char) 0x00,
    (unsigned char) 0x00,
    (unsigned char) 0x00
  };

/*
  Enumeration declarations.
*/
typedef enum
{
  OpenCommand,
  SaveCommand,
  PlayCommand,
  StepCommand,
  RepeatCommand,
  AutoReverseCommand,
  SlowerCommand,
  FasterCommand,
  ForwardCommand,
  ReverseCommand,
  HelpCommand,
  BrowseDocumentationCommand,
  VersionCommand,
  InfoCommand,
  QuitCommand,
  StepBackwardCommand,
  StepForwardCommand,
  NullCommand
} CommandType;

/*
  Stipples.
*/
#define HighlightWidth  8
#define HighlightHeight  8
#define ShadowWidth  8
#define ShadowHeight  8

/*
  Forward declarations.
*/
static Image
  *XMagickCommand(Display *,XResourceInfo *,XWindows *,const CommandType,
    Image **,MagickStatusType *);

static MagickBooleanType
  XSaveImage(Display *,XResourceInfo *,XWindows *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A n i m a t e I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AnimateImages() repeatedly displays an image sequence to any X window
%  screen.  It returns a value other than 0 if successful.  Check the
%  exception member of image to determine the reason for any failure.
%
%  The format of the AnimateImages method is:
%
%      MagickBooleanType AnimateImages(const ImageInfo *image_info,
%        Image *images)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType AnimateImages(const ImageInfo *image_info,
  Image *images)
{
  char
    *argv[1];

  Display
    *display;

  MagickStatusType
    status;

  XrmDatabase
    resource_database;

  XResourceInfo
    resource_info;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      (void) ThrowMagickException(&images->exception,GetMagickModule(),
        XServerError,"UnableToOpenXServer","`%s'",XDisplayName(
        image_info->server_name));
      return(MagickFalse);
    }
  if (images->exception.severity != UndefinedException)
    CatchException(&images->exception);
  (void) XSetErrorHandler(XError);
  resource_database=XGetResourceDatabase(display,GetClientName());
  (void) ResetMagickMemory(&resource_info,0,sizeof(XResourceInfo));
  XGetResourceInfo(image_info,resource_database,GetClientName(),&resource_info);
  if (image_info->page != (char *) NULL)
    resource_info.image_geometry=AcquireString(image_info->page);
  resource_info.immutable=MagickTrue;
  argv[0]=AcquireString(GetClientName());
  (void) XAnimateImages(display,&resource_info,argv,1,images);
  (void) SetErrorHandler((ErrorHandler) NULL);
  (void) SetWarningHandler((WarningHandler) NULL);
  argv[0]=DestroyString(argv[0]);
  (void) XCloseDisplay(display);
  XDestroyResourceInfo(&resource_info);
  status=images->exception.severity == UndefinedException ?
    MagickTrue : MagickFalse;
  return(status != 0 ? MagickTrue : MagickFalse);
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
%  XMagickCommand() makes a transform to the image or Image window as specified
%  by a user menu button or keyboard command.
%
%  The format of the XMagickCommand method is:
%
%      Image *XMagickCommand(Display *display,XResourceInfo *resource_info,
%        XWindows *windows,const CommandType command_type,Image **image,
%        MagickStatusType *state)
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
%    o image: the image;  XMagickCommand
%      may transform the image and return a new image pointer.
%
%    o state: Specifies a MagickStatusType;  XMagickCommand may return a
%      modified state.
%
%
*/
static Image *XMagickCommand(Display *display,XResourceInfo *resource_info,
  XWindows *windows,const CommandType command_type,Image **image,
  MagickStatusType *state)
{
  Image
    *nexus;

  MagickBooleanType
    proceed;

  MagickStatusType
    status;

  XTextProperty
    window_name;

  /*
    Process user command.
  */
  nexus=NewImageList();
  switch (command_type)
  {
    case OpenCommand:
    {
      char
        **filelist;

      ExceptionInfo
        *exception;

      Image
        *images,
        *next;

      ImageInfo
        *read_info;

      int
        number_files;

      register int
        i;

      static char
        filenames[MaxTextExtent] = "*";

      if (resource_info->immutable != MagickFalse)
        break;
      /*
        Request file name from user.
      */
      XFileBrowserWidget(display,windows,"Animate",filenames);
      if (*filenames == '\0')
        return((Image *) NULL);
      /*
        Expand the filenames.
      */
      filelist=(char **) AcquireMagickMemory(sizeof(char *));
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
            {
              ThrowXWindowException(ImageError,"NoImagesWereLoaded",filenames);
             return((Image *) NULL);
            }
          ThrowXWindowException(ResourceLimitError,"MemoryAllocationFailed",
            filenames);
          return((Image *) NULL);
        }
      read_info=CloneImageInfo(resource_info->image_info);
      exception=AcquireExceptionInfo();
      images=NewImageList();
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      for (i=0; i < number_files; i++)
      {
        (void) CopyMagickString(read_info->filename,filelist[i],MaxTextExtent);
        filelist[i]=DestroyString(filelist[i]);
        *read_info->magick='\0';
        next=ReadImage(read_info,exception);
        CatchException(exception);
        if (next != (Image *) NULL)
          AppendImageToList(&images,next);
        if (number_files <= 5)
          continue;
        proceed=SetImageProgress(images,LoadImageTag,i,(MagickSizeType)
          number_files);
        if (proceed == MagickFalse)
          break;
      }
      filelist=(char **) RelinquishMagickMemory(filelist);
      exception=DestroyExceptionInfo(exception);
      read_info=DestroyImageInfo(read_info);
      if (images == (Image *) NULL)
        {
          XSetCursorState(display,windows,MagickFalse);
          ThrowXWindowException(ImageError,"NoImagesWereLoaded",filenames);
          return((Image *) NULL);
        }
      nexus=GetFirstImageInList(images);
      *state|=ExitState;
      break;
    }
    case PlayCommand:
    {
      char
        basename[MaxTextExtent];

      int
        status;

      /*
        Window name is the base of the filename.
      */
      *state|=PlayAnimationState;
      *state&=(~AutoReverseAnimationState);
      GetPathComponent((*image)->magick_filename,BasePath,basename);
      (void) FormatLocaleString(windows->image.name,MaxTextExtent,
        "%s: %s",MagickPackageName,basename);
      if (resource_info->title != (char *) NULL)
        {
          char
            *title;

          title=InterpretImageProperties(resource_info->image_info,*image,
            resource_info->title);
          (void) CopyMagickString(windows->image.name,title,MaxTextExtent);
          title=DestroyString(title);
        }
      status=XStringListToTextProperty(&windows->image.name,1,&window_name);
      if (status == 0)
        break;
      XSetWMName(display,windows->image.id,&window_name);
      (void) XFree((void *) window_name.value);
      break;
    }
    case StepCommand:
    case StepBackwardCommand:
    case StepForwardCommand:
    {
      *state|=StepAnimationState;
      *state&=(~PlayAnimationState);
      if (command_type == StepBackwardCommand)
        *state&=(~ForwardAnimationState);
      if (command_type == StepForwardCommand)
        *state|=ForwardAnimationState;
      if (resource_info->title != (char *) NULL)
        break;
      break;
    }
    case RepeatCommand:
    {
      *state|=RepeatAnimationState;
      *state&=(~AutoReverseAnimationState);
      *state|=PlayAnimationState;
      break;
    }
    case AutoReverseCommand:
    {
      *state|=AutoReverseAnimationState;
      *state&=(~RepeatAnimationState);
      *state|=PlayAnimationState;
      break;
    }
    case SaveCommand:
    {
      /*
        Save image.
      */
      status=XSaveImage(display,resource_info,windows,*image);
      if (status == MagickFalse)
        {
          char
            message[MaxTextExtent];

          (void) FormatLocaleString(message,MaxTextExtent,"%s:%s",
            (*image)->exception.reason != (char *) NULL ?
            (*image)->exception.reason : "",
            (*image)->exception.description != (char *) NULL ?
            (*image)->exception.description : "");
          XNoticeWidget(display,windows,"Unable to save file:",message);
          break;
        }
      break;
    }
    case SlowerCommand:
    {
      resource_info->delay++;
      break;
    }
    case FasterCommand:
    {
      if (resource_info->delay == 0)
        break;
      resource_info->delay--;
      break;
    }
    case ForwardCommand:
    {
      *state=ForwardAnimationState;
      *state&=(~AutoReverseAnimationState);
      break;
    }
    case ReverseCommand:
    {
      *state&=(~ForwardAnimationState);
      *state&=(~AutoReverseAnimationState);
      break;
    }
    case InfoCommand:
    {
      XDisplayImageInfo(display,resource_info,windows,(Image *) NULL,*image);
      break;
    }
    case HelpCommand:
    {
      /*
        User requested help.
      */
      XTextViewWidget(display,resource_info,windows,MagickFalse,
        "Help Viewer - Animate",AnimateHelp);
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
            command[MaxTextExtent],
            *url;

          /*
            Display documentation using Netscape remote control.
          */
          url=GetMagickHomeURL();
          (void) FormatLocaleString(command,MaxTextExtent,
            "openurl(%s,new-tab)",url);
          url=DestroyString(url);
          mozilla_atom=XInternAtom(display,"_MOZILLA_COMMAND",MagickFalse);
          (void) XChangeProperty(display,mozilla_window,mozilla_atom,
            XA_STRING,8,PropModeReplace,(unsigned char *) command,
            (int) strlen(command));
          XSetCursorState(display,windows,MagickFalse);
          break;
        }
      XSetCursorState(display,windows,MagickTrue);
      XCheckRefreshWindows(display,windows);
      status=InvokeDelegate(resource_info->image_info,*image,"browse",
        (char *) NULL,&(*image)->exception);
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
    case QuitCommand:
    {
      /*
        exit program
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
          if (status != 0)
            XClientMessage(display,windows->image.id,windows->im_protocols,
              windows->im_exit,CurrentTime);
        }
      break;
    }
    default:
      break;
  }
  return(nexus);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X A n i m a t e B a c k g r o u n d I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XAnimateBackgroundImage() animates an image sequence in the background of
%  a window.
%
%  The format of the XAnimateBackgroundImage method is:
%
%      void XAnimateBackgroundImage(Display *display,
%        XResourceInfo *resource_info,Image *images)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o images: the image list.
%
*/

static inline ssize_t MagickMax(const ssize_t x,const ssize_t y)
{
  if (x > y)
    return(x);
  return(y);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int SceneCompare(const void *x,const void *y)
{
  const Image
    **image_1,
    **image_2;

  image_1=(const Image **) x;
  image_2=(const Image **) y;
  return((int) ((*image_1)->scene-(*image_2)->scene));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport void XAnimateBackgroundImage(Display *display,
  XResourceInfo *resource_info,Image *images)
{
  char
    geometry[MaxTextExtent],
    visual_type[MaxTextExtent];

  Image
    *coalesce_image,
    *display_image,
    **image_list;

  int
    scene;

  MagickStatusType
    status;

  RectangleInfo
    geometry_info;

  register ssize_t
    i;

  size_t
    number_scenes;

  static XPixelInfo
    pixel;

  static XStandardColormap
    *map_info;

  static XVisualInfo
    *visual_info = (XVisualInfo *) NULL;

  static XWindowInfo
    window_info;

  unsigned int
    height,
    width;

  size_t
    delay;

  Window
    root_window;

  XEvent
    event;

  XGCValues
    context_values;

  XResourceInfo
    resources;

  XWindowAttributes
    window_attributes;

  /*
    Determine target window.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
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
        window_info.id=
          XWindowByName(display,root_window,resources.window_id);
    }
  if (window_info.id == (Window) NULL)
    {
      ThrowXWindowException(XServerError,"NoWindowWithSpecifiedIDExists",
        resources.window_id);
      return;
    }
  /*
    Determine window visual id.
  */
  window_attributes.width=XDisplayWidth(display,XDefaultScreen(display));
  window_attributes.height=XDisplayHeight(display,XDefaultScreen(display));
  (void) CopyMagickString(visual_type,"default",MaxTextExtent);
  status=XGetWindowAttributes(display,window_info.id,&window_attributes) != 0 ?
    MagickTrue : MagickFalse;
  if (status != MagickFalse)
    (void) FormatLocaleString(visual_type,MaxTextExtent,"0x%lx",
      XVisualIDFromVisual(window_attributes.visual));
  if (visual_info == (XVisualInfo *) NULL)
    {
      /*
        Allocate standard colormap.
      */
      map_info=XAllocStandardColormap();
      if (map_info == (XStandardColormap *) NULL)
        ThrowXWindowFatalException(ResourceLimitError,"MemoryAllocationFailed",
          images->filename);
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
          images->filename);
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
    XDestroyWindowColors(display,root_window);
  coalesce_image=CoalesceImages(images,&images->exception);
  if (coalesce_image == (Image *) NULL)
    ThrowXWindowFatalException(ResourceLimitError,"MemoryAllocationFailed",
      images->filename);
  images=coalesce_image;
  if (resources.map_type == (char *) NULL)
    if ((visual_info->klass != TrueColor) &&
        (visual_info->klass != DirectColor))
      {
        Image
          *next;

        /*
          Determine if the sequence of images has the identical colormap.
        */
        for (next=images; next != (Image *) NULL; )
        {
          next->matte=MagickFalse;
          if ((next->storage_class == DirectClass) ||
              (next->colors != images->colors) ||
              (next->colors > (size_t) visual_info->colormap_size))
            break;
          for (i=0; i < (ssize_t) images->colors; i++)
            if (IsColorEqual(next->colormap+i,images->colormap+i) == MagickFalse)
              break;
          if (i < (ssize_t) images->colors)
            break;
          next=GetNextImageInList(next);
        }
        if (next != (Image *) NULL)
          (void) RemapImages(resources.quantize_info,images,(Image *) NULL);
      }
  /*
    Sort images by increasing scene number.
  */
  number_scenes=GetImageListLength(images);
  image_list=ImageListToArray(images,&images->exception);
  if (image_list == (Image **) NULL)
    ThrowXWindowFatalException(ResourceLimitFatalError,
      "MemoryAllocationFailed",images->filename);
  for (i=0; i < (ssize_t) number_scenes; i++)
    if (image_list[i]->scene == 0)
      break;
  if (i == (ssize_t) number_scenes)
    qsort((void *) image_list,number_scenes,sizeof(Image *),SceneCompare);
  /*
    Initialize Standard Colormap.
  */
  resources.colormap=SharedColormap;
  display_image=image_list[0];
  for (scene=0; scene < (int) number_scenes; scene++)
  {
    if ((resource_info->map_type != (char *) NULL) ||
        (visual_info->klass == TrueColor) ||
        (visual_info->klass == DirectColor))
      (void) SetImageType(image_list[scene],image_list[scene]->matte ==
        MagickFalse ? TrueColorType : TrueColorMatteType);
    if ((display_image->columns < image_list[scene]->columns) &&
        (display_image->rows < image_list[scene]->rows))
      display_image=image_list[scene];
  }
  if ((resource_info->map_type != (char *) NULL) ||
      (visual_info->klass == TrueColor) || (visual_info->klass == DirectColor))
    (void) SetImageType(display_image,display_image->matte == MagickFalse ?
      TrueColorType : TrueColorMatteType);
  XMakeStandardColormap(display,visual_info,&resources,display_image,map_info,
    &pixel);
  /*
    Graphic context superclass.
  */
  context_values.background=pixel.background_color.pixel;
  context_values.foreground=pixel.foreground_color.pixel;
  pixel.annotate_context=XCreateGC(display,window_info.id,(unsigned long)
    (GCBackground | GCForeground),&context_values);
  if (pixel.annotate_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      images->filename);
  /*
    Initialize Image window attributes.
  */
  XGetWindowInfo(display,visual_info,map_info,&pixel,(XFontStruct *) NULL,
    &resources,&window_info);
  /*
    Create the X image.
  */
  window_info.width=(unsigned int) image_list[0]->columns;
  window_info.height=(unsigned int) image_list[0]->rows;
  if ((image_list[0]->columns != window_info.width) ||
      (image_list[0]->rows != window_info.height))
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
      image_list[0]->filename);
  (void) FormatLocaleString(geometry,MaxTextExtent,"%ux%u+0+0>",
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
  status=XMakeImage(display,&resources,&window_info,image_list[0],
    window_info.width,window_info.height);
  if (status == MagickFalse)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
      images->filename);
  window_info.x=0;
  window_info.y=0;
  if (display_image->debug != MagickFalse)
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "Image: %s[%.20g] %.20gx%.20g ",image_list[0]->filename,(double)
        image_list[0]->scene,(double) image_list[0]->columns,(double)
        image_list[0]->rows);
      if (image_list[0]->colors != 0)
        (void) LogMagickEvent(X11Event,GetMagickModule(),"%.20gc ",(double)
          image_list[0]->colors);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"%s",
        image_list[0]->magick);
    }
  /*
    Adjust image dimensions as specified by backdrop or geometry options.
  */
  width=window_info.width;
  height=window_info.height;
  if (resources.backdrop != MagickFalse)
    {
      /*
        Center image on window.
      */
      window_info.x=(int) (window_attributes.width/2)-
        (window_info.ximage->width/2);
      window_info.y=(int) (window_attributes.height/2)-
        (window_info.ximage->height/2);
      width=(unsigned int) window_attributes.width;
      height=(unsigned int) window_attributes.height;
    }
  if (resources.image_geometry != (char *) NULL)
    {
      char
        default_geometry[MaxTextExtent];

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
          "MemoryAllocationFailed",images->filename);
      size_hints->flags=0L;
      (void) FormatLocaleString(default_geometry,MaxTextExtent,"%ux%u",width,
        height);
      flags=XWMGeometry(display,visual_info->screen,resources.image_geometry,
        default_geometry,window_info.border_width,size_hints,&window_info.x,
        &window_info.y,(int *) &width,(int *) &height,&gravity);
      if (((flags & (XValue | YValue))) != 0)
        {
          width=(unsigned int) window_attributes.width;
          height=(unsigned int) window_attributes.height;
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
      images->filename);
  /*
    Display pixmap on the window.
  */
  if (((unsigned int) width > window_info.width) ||
      ((unsigned int) height > window_info.height))
    (void) XFillRectangle(display,window_info.pixmap,
      window_info.annotate_context,0,0,(unsigned int) width,
      (unsigned int) height);
  (void) XPutImage(display,window_info.pixmap,window_info.annotate_context,
    window_info.ximage,0,0,window_info.x,window_info.y,window_info.width,
    window_info.height);
  (void) XSetWindowBackgroundPixmap(display,window_info.id,window_info.pixmap);
  (void) XClearWindow(display,window_info.id);
  /*
    Initialize image pixmaps structure.
  */
  window_info.pixmaps=(Pixmap *) AcquireQuantumMemory(number_scenes,
    sizeof(*window_info.pixmaps));
  window_info.matte_pixmaps=(Pixmap *) AcquireQuantumMemory(number_scenes,
    sizeof(*window_info.matte_pixmaps));
  if ((window_info.pixmaps == (Pixmap *) NULL) ||
      (window_info.matte_pixmaps == (Pixmap *) NULL))
    ThrowXWindowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      images->filename);
  window_info.pixmaps[0]=window_info.pixmap;
  window_info.matte_pixmaps[0]=window_info.pixmap;
  for (scene=1; scene < (int) number_scenes; scene++)
  {
    unsigned int
      columns,
      rows;

    /*
      Create X image.
    */
    window_info.pixmap=(Pixmap) NULL;
    window_info.matte_pixmap=(Pixmap) NULL;
    if ((resources.map_type != (char *) NULL) ||
        (visual_info->klass == TrueColor) ||
        (visual_info->klass == DirectColor))
      if (image_list[scene]->storage_class == PseudoClass)
        XGetPixelPacket(display,visual_info,map_info,&resources,
          image_list[scene],window_info.pixel_info);
    columns=(unsigned int) image_list[scene]->columns;
    rows=(unsigned int) image_list[scene]->rows;
    if ((image_list[scene]->columns != columns) ||
        (image_list[scene]->rows != rows))
      ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
        image_list[scene]->filename);
    status=XMakeImage(display,&resources,&window_info,image_list[scene],
      columns,rows);
    if (status == MagickFalse)
      ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
        images->filename);
    if (display_image->debug != MagickFalse)
      {
        (void) LogMagickEvent(X11Event,GetMagickModule(),
          "Image: [%.20g] %s %.20gx%.20g ",(double) image_list[scene]->scene,
          image_list[scene]->filename,(double) columns,(double) rows);
        if (image_list[scene]->colors != 0)
          (void) LogMagickEvent(X11Event,GetMagickModule(),"%.20gc ",(double)
            image_list[scene]->colors);
        (void) LogMagickEvent(X11Event,GetMagickModule(),"%s",
          image_list[scene]->magick);
      }
    /*
      Create the X pixmap.
    */
    window_info.pixmap=XCreatePixmap(display,window_info.id,width,height,
      window_info.depth);
    if (window_info.pixmap == (Pixmap) NULL)
      ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXPixmap",
        images->filename);
    /*
      Display pixmap on the window.
    */
    if ((width > window_info.width) || (height > window_info.height))
      (void) XFillRectangle(display,window_info.pixmap,
        window_info.annotate_context,0,0,width,height);
    (void) XPutImage(display,window_info.pixmap,window_info.annotate_context,
      window_info.ximage,0,0,window_info.x,window_info.y,window_info.width,
      window_info.height);
    (void) XSetWindowBackgroundPixmap(display,window_info.id,
      window_info.pixmap);
    (void) XClearWindow(display,window_info.id);
    window_info.pixmaps[scene]=window_info.pixmap;
    window_info.matte_pixmaps[scene]=window_info.matte_pixmap;
    if (image_list[scene]->matte)
      (void) XClearWindow(display,window_info.id);
    delay=1000*image_list[scene]->delay/MagickMax(
      image_list[scene]->ticks_per_second,1L);
    XDelay(display,resources.delay*(delay == 0 ? 10 : delay));
  }
  window_info.pixel_info=(&pixel);
  /*
    Display pixmap on the window.
  */
  (void) XSelectInput(display,window_info.id,SubstructureNotifyMask);
  event.type=Expose;
  do
  {
    for (scene=0; scene < (int) number_scenes; scene++)
    {
      if (XEventsQueued(display,QueuedAfterFlush) > 0)
        {
          (void) XNextEvent(display,&event);
          if (event.type == DestroyNotify)
            break;
        }
      window_info.pixmap=window_info.pixmaps[scene];
      window_info.matte_pixmap=window_info.matte_pixmaps[scene];
      (void) XSetWindowBackgroundPixmap(display,window_info.id,
        window_info.pixmap);
      (void) XClearWindow(display,window_info.id);
      (void) XSync(display,MagickFalse);
      delay=1000*image_list[scene]->delay/MagickMax(
        image_list[scene]->ticks_per_second,1L);
      XDelay(display,resources.delay*(delay == 0 ? 10 : delay));
    }
  } while (event.type != DestroyNotify);
  (void) XSync(display,MagickFalse);
  image_list=(Image **) RelinquishMagickMemory(image_list);
  images=DestroyImageList(images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X A n i m a t e I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XAnimateImages() displays an image via X11.
%
%  The format of the XAnimateImages method is:
%
%      Image *XAnimateImages(Display *display,XResourceInfo *resource_info,
%        char **argv,const int argc,Image *images)
%
%  A description of each parameter follows:
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
%    o images: the image list.
%
*/
MagickExport Image *XAnimateImages(Display *display,
  XResourceInfo *resource_info,char **argv,const int argc,Image *images)
{
#define MagickMenus  4
#define MaXWindows  8
#define MagickTitle  "Commands"

  static const char
    *CommandMenu[]=
    {
      "Animate",
      "Speed",
      "Direction",
      "Help",
      "Image Info",
      "Quit",
      (char *) NULL
    },
    *AnimateMenu[]=
    {
      "Open...",
      "Play",
      "Step",
      "Repeat",
      "Auto Reverse",
      "Save...",
      (char *) NULL
    },
    *SpeedMenu[]=
    {
      "Faster",
      "Slower",
      (char *) NULL
    },
    *DirectionMenu[]=
    {
      "Forward",
      "Reverse",
      (char *) NULL
    },
    *HelpMenu[]=
    {
      "Overview",
      "Browse Documentation",
      "About Animate",
      (char *) NULL
    };

  static const char
    **Menus[MagickMenus]=
    {
      AnimateMenu,
      SpeedMenu,
      DirectionMenu,
      HelpMenu
    };

  static const CommandType
    CommandMenus[]=
    {
      NullCommand,
      NullCommand,
      NullCommand,
      NullCommand,
      InfoCommand,
      QuitCommand
    },
    CommandTypes[]=
    {
      OpenCommand,
      PlayCommand,
      StepCommand,
      RepeatCommand,
      AutoReverseCommand,
      SaveCommand
    },
    SpeedCommands[]=
    {
      FasterCommand,
      SlowerCommand
    },
    DirectionCommands[]=
    {
      ForwardCommand,
      ReverseCommand
    },
    HelpCommands[]=
    {
      HelpCommand,
      BrowseDocumentationCommand,
      VersionCommand
    };

  static const CommandType
    *Commands[MagickMenus]=
    {
      CommandTypes,
      SpeedCommands,
      DirectionCommands,
      HelpCommands
    };

  char
    command[MaxTextExtent],
    *directory,
    geometry[MaxTextExtent],
    resource_name[MaxTextExtent];

  CommandType
    command_type;

  Image
    *coalesce_image,
    *display_image,
    *image,
    **image_list,
    *nexus;

  int
    status;

  KeySym
    key_symbol;

  MagickStatusType
    context_mask,
    state;

  RectangleInfo
    geometry_info;

  register char
    *p;

  register ssize_t
    i;

  ssize_t
    first_scene,
    iterations,
    scene;

  static char
    working_directory[MaxTextExtent];

  static size_t
    number_windows;

  static XWindowInfo
    *magick_windows[MaXWindows];

  time_t
    timestamp;

  size_t
    delay,
    number_scenes;

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

  XTextProperty
    window_name;

  XVisualInfo
    *icon_visual,
    *visual_info;

  XWindowChanges
    window_changes;

  XWindows
    *windows;

  XWMHints
    *manager_hints;

  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  warning_handler=(WarningHandler) NULL;
  windows=XSetWindows((XWindows *) ~0);
  if (windows != (XWindows *) NULL)
    {
      int
        status;

      status=chdir(working_directory);
      if (status == -1)
        (void) ThrowMagickException(&images->exception,GetMagickModule(),
          FileOpenError,"UnableToOpenFile","%s",working_directory);
      warning_handler=resource_info->display_warnings ?
        SetErrorHandler(XWarning) : SetErrorHandler((ErrorHandler) NULL);
      warning_handler=resource_info->display_warnings ?
        SetWarningHandler(XWarning) : SetWarningHandler((WarningHandler) NULL);
    }
  else
    {
      register Image
        *p;

      /*
        Initialize window structure.
      */
      for (p=images; p != (Image *) NULL; p=GetNextImageInList(p))
      {
        if (p->storage_class == DirectClass)
          {
            resource_info->colors=0;
            break;
          }
        if (p->colors > resource_info->colors)
          resource_info->colors=p->colors;
      }
      windows=XSetWindows(XInitializeWindows(display,resource_info));
      if (windows == (XWindows *) NULL)
        ThrowXWindowFatalException(XServerFatalError,"MemoryAllocationFailed",
          images->filename);
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
      for (i=0; i < (ssize_t) number_windows; i++)
        magick_windows[i]->id=(Window) NULL;
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
  coalesce_image=CoalesceImages(images,&images->exception);
  if (coalesce_image == (Image *) NULL)
    ThrowXWindowFatalException(XServerFatalError,"MemoryAllocationFailed",
      images->filename);
  images=coalesce_image;
  if (resource_info->map_type == (char *) NULL)
    if ((visual_info->klass != TrueColor) &&
        (visual_info->klass != DirectColor))
      {
        Image
          *next;

        /*
          Determine if the sequence of images has the identical colormap.
        */
        for (next=images; next != (Image *) NULL; )
        {
          next->matte=MagickFalse;
          if ((next->storage_class == DirectClass) ||
              (next->colors != images->colors) ||
              (next->colors > (size_t) visual_info->colormap_size))
            break;
          for (i=0; i < (ssize_t) images->colors; i++)
            if (IsColorEqual(next->colormap+i,images->colormap+i) == MagickFalse)
              break;
          if (i < (ssize_t) images->colors)
            break;
          next=GetNextImageInList(next);
        }
        if (next != (Image *) NULL)
          (void) RemapImages(resource_info->quantize_info,images,
            (Image *) NULL);
      }
  /*
    Sort images by increasing scene number.
  */
  number_scenes=GetImageListLength(images);
  image_list=ImageListToArray(images,&images->exception);
  if (image_list == (Image **) NULL)
    ThrowXWindowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      images->filename);
  for (scene=0; scene < (ssize_t) number_scenes; scene++)
    if (image_list[scene]->scene == 0)
      break;
  if (scene == (ssize_t) number_scenes)
    qsort((void *) image_list,number_scenes,sizeof(Image *),SceneCompare);
  /*
    Initialize Standard Colormap.
  */
  nexus=NewImageList();
  display_image=image_list[0];
  for (scene=0; scene < (ssize_t) number_scenes; scene++)
  {
    if ((resource_info->map_type != (char *) NULL) ||
        (visual_info->klass == TrueColor) ||
        (visual_info->klass == DirectColor))
      (void) SetImageType(image_list[scene],image_list[scene]->matte ==
        MagickFalse ? TrueColorType : TrueColorMatteType);
    if ((display_image->columns < image_list[scene]->columns) &&
        (display_image->rows < image_list[scene]->rows))
      display_image=image_list[scene];
  }
  if (display_image->debug != MagickFalse)
    {
      (void) LogMagickEvent(X11Event,GetMagickModule(),
        "Image: %s[%.20g] %.20gx%.20g ",display_image->filename,(double)
        display_image->scene,(double) display_image->columns,(double)
        display_image->rows);
      if (display_image->colors != 0)
        (void) LogMagickEvent(X11Event,GetMagickModule(),"%.20gc ",(double)
          display_image->colors);
      (void) LogMagickEvent(X11Event,GetMagickModule(),"%s",
        display_image->magick);
    }
  XMakeStandardColormap(display,visual_info,resource_info,display_image,
    map_info,pixel);
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
  if (display_image->debug != MagickFalse)
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
  pixel->annotate_context=
    XCreateGC(display,windows->context.id,context_mask,&context_values);
  if (pixel->annotate_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      images->filename);
  context_values.background=pixel->depth_color.pixel;
  if (pixel->widget_context != (GC) NULL)
    (void) XFreeGC(display,pixel->widget_context);
  pixel->widget_context=
    XCreateGC(display,windows->context.id,context_mask,&context_values);
  if (pixel->widget_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      images->filename);
  context_values.background=pixel->foreground_color.pixel;
  context_values.foreground=pixel->background_color.pixel;
  context_values.plane_mask=
    context_values.background ^ context_values.foreground;
  if (pixel->highlight_context != (GC) NULL)
    (void) XFreeGC(display,pixel->highlight_context);
  pixel->highlight_context=XCreateGC(display,windows->context.id,
    (size_t) (context_mask | GCPlaneMask),&context_values);
  if (pixel->highlight_context == (GC) NULL)
    ThrowXWindowFatalException(XServerFatalError,"UnableToCreateGraphicContext",
      images->filename);
  (void) XDestroyWindow(display,windows->context.id);
  /*
    Initialize icon window.
  */
  XGetWindowInfo(display,icon_visual,icon_map,icon_pixel,(XFontStruct *) NULL,
    icon_resources,&windows->icon);
  windows->icon.geometry=resource_info->icon_geometry;
  XBestIconSize(display,&windows->icon,display_image);
  windows->icon.attributes.colormap=
    XDefaultColormap(display,icon_visual->screen);
  windows->icon.attributes.event_mask=ExposureMask | StructureNotifyMask;
  manager_hints->flags=InputHint | StateHint;
  manager_hints->input=MagickFalse;
  manager_hints->initial_state=IconicState;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->icon);
  if (display_image->debug != MagickFalse)
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
      images->filename);
  windows->icon.annotate_context=icon_pixel->annotate_context;
  /*
    Initialize Image window.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->image);
  windows->image.shape=MagickTrue;  /* non-rectangular shape hint */
  if (resource_info->use_shared_memory == MagickFalse)
    windows->image.shared_memory=MagickFalse;
  if (resource_info->title != (char *) NULL)
    {
      char
        *title;

      title=InterpretImageProperties(resource_info->image_info,display_image,
        resource_info->title);
      (void) CopyMagickString(windows->image.name,title,MaxTextExtent);
      (void) CopyMagickString(windows->image.icon_name,title,MaxTextExtent);
      title=DestroyString(title);
    }
  else
    {
      char
        filename[MaxTextExtent];

      /*
        Window name is the base of the filename.
      */
      GetPathComponent(display_image->magick_filename,TailPath,filename);
      (void) FormatLocaleString(windows->image.name,MaxTextExtent,
        "%s: %s[scene: %.20g frames: %.20g]",MagickPackageName,filename,(double)
        display_image->scene,(double) number_scenes);
      (void) CopyMagickString(windows->image.icon_name,filename,MaxTextExtent);
    }
  if (resource_info->immutable != MagickFalse)
    windows->image.immutable=MagickTrue;
  windows->image.shape=MagickTrue;
  windows->image.geometry=resource_info->image_geometry;
  (void) FormatLocaleString(geometry,MaxTextExtent,"%ux%u+0+0>!",
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
      (void) CloneString(&windows->backdrop.name,"ImageMagick Backdrop");
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
      manager_hints->initial_state=
        resource_info->iconic ? IconicState : NormalState;
      XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
        &windows->backdrop);
      if (display_image->debug != MagickFalse)
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
  manager_hints->initial_state=
    resource_info->iconic ? IconicState : NormalState;
  if (windows->group_leader.id != (Window) NULL)
    {
      /*
        Follow the leader.
      */
      manager_hints->flags|=(MagickStatusType) WindowGroupHint;
      manager_hints->window_group=windows->group_leader.id;
      (void) XSelectInput(display,windows->group_leader.id,StructureNotifyMask);
      if (display_image->debug != MagickFalse)
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
  if (display_image->debug != MagickFalse)
    (void) LogMagickEvent(X11Event,GetMagickModule(),"Window id: 0x%lx (image)",
      windows->image.id);
  /*
    Initialize Info widget.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->info);
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
  if (windows->image.mapped)
    (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if (display_image->debug != MagickFalse)
    (void) LogMagickEvent(X11Event,GetMagickModule(),"Window id: 0x%lx (info)",
      windows->info.id);
  /*
    Initialize Command widget.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->command);
  windows->command.data=MagickMenus;
  (void) XCommandWidget(display,windows,CommandMenu,(XEvent *) NULL);
  (void) FormatLocaleString(resource_name,MaxTextExtent,"%s.command",
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
  if (display_image->debug != MagickFalse)
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Window id: 0x%lx (command)",windows->command.id);
  /*
    Initialize Widget window.
  */
  XGetWindowInfo(display,visual_info,map_info,pixel,font_info,
    resource_info,&windows->widget);
  (void) FormatLocaleString(resource_name,MaxTextExtent,"%s.widget",
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
  if (display_image->debug != MagickFalse)
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
  if (display_image->debug != MagickFalse)
    (void) LogMagickEvent(X11Event,GetMagickModule(),
      "Window id: 0x%lx (pop up)",windows->popup.id);
  /*
    Set out progress and warning handlers.
  */
  if (warning_handler == (WarningHandler) NULL)
    {
      warning_handler=resource_info->display_warnings ?
        SetErrorHandler(XWarning) : SetErrorHandler((ErrorHandler) NULL);
      warning_handler=resource_info->display_warnings ?
        SetWarningHandler(XWarning) : SetWarningHandler((WarningHandler) NULL);
    }
  /*
    Initialize X image structure.
  */
  windows->image.x=0;
  windows->image.y=0;
  /*
    Initialize image pixmaps structure.
  */
  window_changes.width=(int) windows->image.width;
  window_changes.height=(int) windows->image.height;
  (void) XReconfigureWMWindow(display,windows->image.id,windows->command.screen,
    (unsigned int) (CWWidth | CWHeight),&window_changes);
  windows->image.pixmaps=(Pixmap *) AcquireQuantumMemory(number_scenes,
    sizeof(*windows->image.pixmaps));
  windows->image.matte_pixmaps=(Pixmap *) AcquireQuantumMemory(number_scenes,
    sizeof(*windows->image.pixmaps));
  if ((windows->image.pixmaps == (Pixmap *) NULL) ||
      (windows->image.matte_pixmaps == (Pixmap *) NULL))
    ThrowXWindowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      images->filename);
  if ((windows->image.mapped == MagickFalse) ||
      (windows->backdrop.id != (Window) NULL))
    (void) XMapWindow(display,windows->image.id);
  XSetCursorState(display,windows,MagickTrue);
  for (scene=0; scene < (ssize_t) number_scenes; scene++)
  {
    unsigned int
      columns,
      rows;

    /*
      Create X image.
    */
    windows->image.pixmap=(Pixmap) NULL;
    windows->image.matte_pixmap=(Pixmap) NULL;
    if ((resource_info->map_type != (char *) NULL) ||
        (visual_info->klass == TrueColor) ||
        (visual_info->klass == DirectColor))
      if (image_list[scene]->storage_class == PseudoClass)
        XGetPixelPacket(display,visual_info,map_info,resource_info,
          image_list[scene],windows->image.pixel_info);
    columns=(unsigned int) image_list[scene]->columns;
    rows=(unsigned int) image_list[scene]->rows;
    if ((image_list[scene]->columns != columns) ||
        (image_list[scene]->rows != rows))
      ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
        image_list[scene]->filename);
    status=XMakeImage(display,resource_info,&windows->image,image_list[scene],
      columns,rows);
    if (status == MagickFalse)
      ThrowXWindowFatalException(XServerFatalError,"UnableToCreateXImage",
        images->filename);
    if (image_list[scene]->debug != MagickFalse)
      {
        (void) LogMagickEvent(X11Event,GetMagickModule(),
          "Image: [%.20g] %s %.20gx%.20g ",(double) image_list[scene]->scene,
          image_list[scene]->filename,(double) columns,(double) rows);
        if (image_list[scene]->colors != 0)
          (void) LogMagickEvent(X11Event,GetMagickModule(),"%.20gc ",(double)
            image_list[scene]->colors);
        (void) LogMagickEvent(X11Event,GetMagickModule(),"%s",
          image_list[scene]->magick);
      }
    /*
      Window name is the base of the filename.
    */
    if (resource_info->title != (char *) NULL)
      {
        char
          *title;

        title=InterpretImageProperties(resource_info->image_info,
          image_list[scene],resource_info->title);
        (void) CopyMagickString(windows->image.name,title,MaxTextExtent);
        title=DestroyString(title);
      }
    else
      {
        p=image_list[scene]->magick_filename+
          strlen(image_list[scene]->magick_filename)-1;
        while ((p > image_list[scene]->magick_filename) && (*(p-1) != '/'))
          p--;
        (void) FormatLocaleString(windows->image.name,MaxTextExtent,
          "%s: %s[%.20g of %.20g]",MagickPackageName,p,(double) scene+1,
          (double) number_scenes);
      }
    status=XStringListToTextProperty(&windows->image.name,1,&window_name);
    if (status != Success)
      {
        XSetWMName(display,windows->image.id,&window_name);
        (void) XFree((void *) window_name.value);
      }
    windows->image.pixmaps[scene]=windows->image.pixmap;
    windows->image.matte_pixmaps[scene]=windows->image.matte_pixmap;
    if (scene == 0)
      {
        event.xexpose.x=0;
        event.xexpose.y=0;
        event.xexpose.width=(int) image_list[scene]->columns;
        event.xexpose.height=(int) image_list[scene]->rows;
        XRefreshWindow(display,&windows->image,&event);
        (void) XSync(display,MagickFalse);
    }
  }
  XSetCursorState(display,windows,MagickFalse);
  if (windows->command.mapped)
    (void) XMapRaised(display,windows->command.id);
  /*
    Respond to events.
  */
  nexus=NewImageList();
  scene=0;
  first_scene=0;
  iterations=0;
  image=image_list[0];
  state=(MagickStatusType) (ForwardAnimationState | RepeatAnimationState);
  (void) XMagickCommand(display,resource_info,windows,PlayCommand,&images,
    &state);
  do
  {
    if (XEventsQueued(display,QueuedAfterFlush) == 0)
      if ((state & PlayAnimationState) || (state & StepAnimationState))
        {
          MagickBooleanType
            pause;

          pause=MagickFalse;
          delay=1000*image->delay/MagickMax(image->ticks_per_second,1L);
          XDelay(display,resource_info->delay*(delay == 0 ? 10 : delay));
          if (state & ForwardAnimationState)
            {
              /*
                Forward animation:  increment scene number.
              */
              if (scene < ((ssize_t) number_scenes-1))
                scene++;
              else
                {
                  iterations++;
                  if (iterations == (ssize_t) image_list[0]->iterations)
                    {
                      iterations=0;
                      state|=ExitState;
                    }
                  if ((state & AutoReverseAnimationState) != 0)
                    {
                      state&=(~ForwardAnimationState);
                      scene--;
                    }
                  else
                    {
                      if ((state & RepeatAnimationState) == 0)
                        state&=(~PlayAnimationState);
                      scene=first_scene;
                      pause=MagickTrue;
                    }
                }
            }
          else
            {
              /*
                Reverse animation:  decrement scene number.
              */
              if (scene > first_scene)
                scene--;
              else
                {
                  iterations++;
                  if (iterations == (ssize_t) image_list[0]->iterations)
                    {
                      iterations=0;
                      state&=(~RepeatAnimationState);
                    }
                  if (state & AutoReverseAnimationState)
                    {
                      state|=ForwardAnimationState;
                      scene=first_scene;
                      pause=MagickTrue;
                    }
                  else
                    {
                      if ((state & RepeatAnimationState) == MagickFalse)
                        state&=(~PlayAnimationState);
                      scene=(ssize_t) number_scenes-1;
                    }
                }
            }
          scene=MagickMax(scene,0);
          image=image_list[scene];
          if ((image != (Image *) NULL) && (image->start_loop != 0))
            first_scene=scene;
          if ((state & StepAnimationState) ||
              (resource_info->title != (char *) NULL))
            {
              /*
                Update window title.
              */
              p=image_list[scene]->filename+
                strlen(image_list[scene]->filename)-1;
              while ((p > image_list[scene]->filename) && (*(p-1) != '/'))
                p--;
              (void) FormatLocaleString(windows->image.name,MaxTextExtent,
                "%s: %s[%.20g of %.20g]",MagickPackageName,p,(double)
                scene+1,(double) number_scenes);
              if (resource_info->title != (char *) NULL)
                {
                  char
                    *title;

                  title=InterpretImageProperties(resource_info->image_info,
                    image,resource_info->title);
                  (void) CopyMagickString(windows->image.name,title,
                    MaxTextExtent);
                  title=DestroyString(title);
                }
              status=XStringListToTextProperty(&windows->image.name,1,
                &window_name);
              if (status != Success)
                {
                  XSetWMName(display,windows->image.id,&window_name);
                  (void) XFree((void *) window_name.value);
                }
            }
          /*
            Copy X pixmap to Image window.
          */
          XGetPixelPacket(display,visual_info,map_info,resource_info,
            image_list[scene],windows->image.pixel_info);
          windows->image.ximage->width=(int) image->columns;
          windows->image.ximage->height=(int) image->rows;
          windows->image.pixmap=windows->image.pixmaps[scene];
          windows->image.matte_pixmap=windows->image.matte_pixmaps[scene];
          event.xexpose.x=0;
          event.xexpose.y=0;
          event.xexpose.width=(int) image->columns;
          event.xexpose.height=(int) image->rows;
          if ((state & ExitState) == 0)
            {
              XRefreshWindow(display,&windows->image,&event);
              (void) XSync(display,MagickFalse);
            }
          state&=(~StepAnimationState);
          if (pause != MagickFalse)
            for (i=0; i < (ssize_t) resource_info->pause; i++)
            {
              int
                status;

              status=XCheckTypedWindowEvent(display,windows->image.id,KeyPress,
                &event);
              if (status != 0)
                {
                  int
                    length;

                  length=XLookupString((XKeyEvent *) &event.xkey,command,(int)
                    sizeof(command),&key_symbol,(XComposeStatus *) NULL);
                  *(command+length)='\0';
                  if ((key_symbol == XK_q) || (key_symbol == XK_Escape))
                    {
                      XClientMessage(display,windows->image.id,
                        windows->im_protocols,windows->im_exit,CurrentTime);
                      break;
                    }
                }
              (void) sleep(1);
            }
          continue;
        }
    /*
      Handle a window event.
    */
    timestamp=time((time_t *) NULL);
    (void) XNextEvent(display,&event);
    if (windows->image.stasis == MagickFalse)
      windows->image.stasis=(time((time_t *) NULL)-timestamp) > 0 ?
        MagickTrue : MagickFalse;
    if (event.xany.window == windows->command.id)
      {
        int
          id;

        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,CommandMenu,&event);
        if (id < 0)
          continue;
        (void) CopyMagickString(command,CommandMenu[id],MaxTextExtent);
        command_type=CommandMenus[id];
        if (id < MagickMenus)
          {
            int
              entry;

            /*
              Select a command from a pop-up menu.
            */
            entry=XMenuWidget(display,windows,CommandMenu[id],Menus[id],
              command);
            if (entry < 0)
              continue;
            (void) CopyMagickString(command,Menus[id][entry],MaxTextExtent);
            command_type=Commands[id][entry];
          }
        if (command_type != NullCommand)
          nexus=XMagickCommand(display,resource_info,windows,
            command_type,&image,&state);
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (display_image->debug != MagickFalse)
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
            if (resource_info->immutable != MagickFalse)
              {
                state|=ExitState;
                break;
              }
            /*
              Map/unmap Command widget.
            */
            if (windows->command.mapped)
              (void) XWithdrawWindow(display,windows->command.id,
                windows->command.screen);
            else
              {
                (void) XCommandWidget(display,windows,CommandMenu,
                  (XEvent *) NULL);
                (void) XMapRaised(display,windows->command.id);
              }
          }
        break;
      }
      case ButtonRelease:
      {
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Button Release: 0x%lx %u +%d+%d",event.xbutton.window,
            event.xbutton.button,event.xbutton.x,event.xbutton.y);
        break;
      }
      case ClientMessage:
      {
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Client Message: 0x%lx 0x%lx %d 0x%lx",(unsigned long)
            event.xclient.window,(unsigned long) event.xclient.message_type,
            event.xclient.format,(unsigned long) event.xclient.data.l[0]);
        if (event.xclient.message_type == windows->im_protocols)
          {
            if (*event.xclient.data.l == (long) windows->im_update_colormap)
              {
                /*
                  Update graphic context and window colormap.
                */
                for (i=0; i < (ssize_t) number_windows; i++)
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
                  context_values.plane_mask=
                    context_values.background ^ context_values.foreground;
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
                if (windows->backdrop.id != (Window) NULL)
                  (void) XInstallColormap(display,map_info->colormap);
                break;
              }
            if (*event.xclient.data.l == (long) windows->im_exit)
              {
                state|=ExitState;
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
            status=XGetWindowProperty(display,root_window,selection,0L,2047L,
              MagickFalse,(Atom) AnyPropertyType,&type,&format,&length,&after,
              &data);
            if ((status != Success) || (length == 0))
              break;
            if (*event.xclient.data.l == 2)
              {
                /*
                  Offix DND.
                */
                (void) CopyMagickString(resource_info->image_info->filename,
                  (char *) data,MaxTextExtent);
              }
            else
              {
                /*
                  XDND.
                */
                if (LocaleNCompare((char *) data,"file:",5) != 0)
                  {
                    (void) XFree((void *) data);
                    break;
                  }
                (void) CopyMagickString(resource_info->image_info->filename,
                  ((char *) data)+5,MaxTextExtent);
              }
            nexus=ReadImage(resource_info->image_info,&image->exception);
            CatchException(&image->exception);
            if (nexus != (Image *) NULL)
              state|=ExitState;
            (void) XFree((void *) data);
            break;
          }
        /*
          If client window delete message, exit.
        */
        if (event.xclient.message_type != windows->wm_protocols)
          break;
        if (*event.xclient.data.l == (long) windows->wm_take_focus)
          {
            (void) XSetInputFocus(display,event.xclient.window,RevertToParent,
              (Time) event.xclient.data.l[1]);
            break;
          }
        if (*event.xclient.data.l != (long) windows->wm_delete_window)
          break;
        (void) XWithdrawWindow(display,event.xclient.window,
          visual_info->screen);
        if (event.xclient.window == windows->image.id)
          {
            state|=ExitState;
            break;
          }
        break;
      }
      case ConfigureNotify:
      {
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Configure Notify: 0x%lx %dx%d+%d+%d %d",event.xconfigure.window,
            event.xconfigure.width,event.xconfigure.height,event.xconfigure.x,
            event.xconfigure.y,event.xconfigure.send_event);
        if (event.xconfigure.window == windows->image.id)
          {
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
                       windows->command.x=
                          event.xconfigure.x-windows->command.width-25;
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
                      windows->widget.x=
                        event.xconfigure.x+event.xconfigure.width/10;
                      windows->widget.y=
                        event.xconfigure.y+event.xconfigure.height/10;
                      XConstrainWindowPosition(display,&windows->widget);
                      window_changes.x=windows->widget.x;
                      window_changes.y=windows->widget.y;
                      (void) XReconfigureWMWindow(display,windows->widget.id,
                        windows->widget.screen,(unsigned int) (CWX | CWY),
                        &window_changes);
                    }
              }
            /*
              Image window has a new configuration.
            */
            windows->image.width=(unsigned int) event.xconfigure.width;
            windows->image.height=(unsigned int) event.xconfigure.height;
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
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Destroy Notify: 0x%lx",event.xdestroywindow.window);
        if (event.xdestroywindow.window == windows->group_leader.id)
          {
            state|=ExitState;
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
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Expose: 0x%lx %dx%d+%d+%d",event.xexpose.window,
            event.xexpose.width,event.xexpose.height,event.xexpose.x,
            event.xexpose.y);
        /*
          Repaint windows that are now exposed.
        */
        if (event.xexpose.window == windows->image.id)
          {
            windows->image.pixmap=windows->image.pixmaps[scene];
            windows->image.matte_pixmap=windows->image.matte_pixmaps[scene];
            XRefreshWindow(display,&windows->image,&event);
            break;
          }
        if (event.xexpose.window == windows->icon.id)
          if (event.xexpose.count == 0)
            {
              XRefreshWindow(display,&windows->icon,&event);
              break;
            }
        break;
      }
      case KeyPress:
      {
        static int
          length;

        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Key press: 0x%lx (%c)",(unsigned long) key_symbol,*command);
        command_type=NullCommand;
        switch (key_symbol)
        {
          case XK_o:
          {
            if ((event.xkey.state & ControlMask) == MagickFalse)
              break;
            command_type=OpenCommand;
            break;
          }
          case XK_BackSpace:
          {
            command_type=StepBackwardCommand;
            break;
          }
          case XK_space:
          {
            command_type=StepForwardCommand;
            break;
          }
          case XK_less:
          {
            command_type=FasterCommand;
            break;
          }
          case XK_greater:
          {
            command_type=SlowerCommand;
            break;
          }
          case XK_F1:
          {
            command_type=HelpCommand;
            break;
          }
          case XK_Find:
          {
            command_type=BrowseDocumentationCommand;
            break;
          }
          case XK_question:
          {
            command_type=InfoCommand;
            break;
          }
          case XK_q:
          case XK_Escape:
          {
            command_type=QuitCommand;
            break;
          }
          default:
            break;
        }
        if (command_type != NullCommand)
          nexus=XMagickCommand(display,resource_info,windows,
            command_type,&image,&state);
        break;
      }
      case KeyRelease:
      {
        /*
          Respond to a user key release.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,(int)
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        if (display_image->debug != MagickFalse)
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
        if (display_image->debug != MagickFalse)
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
            if (LocaleCompare(image_list[0]->magick,"LOGO") == 0)
              {
                if (LocaleCompare(display_image->filename,"LOGO") == 0)
                  nexus=XMagickCommand(display,resource_info,windows,
                    OpenCommand,&image,&state);
                else
                  state|=ExitState;
              }
            windows->image.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->info.id)
          {
            windows->info.mapped=MagickTrue;
            break;
          }
        if (event.xmap.window == windows->icon.id)
          {
            /*
              Create an icon image.
            */
            XMakeStandardColormap(display,icon_visual,icon_resources,
              display_image,icon_map,icon_pixel);
            (void) XMakeImage(display,icon_resources,&windows->icon,
              display_image,windows->icon.width,windows->icon.height);
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

        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Property Notify: 0x%lx 0x%lx %d",(unsigned long)
            event.xproperty.window,(unsigned long) event.xproperty.atom,
            event.xproperty.state);
        if (event.xproperty.atom != windows->im_remote_command)
          break;
        /*
          Display image named by the remote command protocol.
        */
        status=XGetWindowProperty(display,event.xproperty.window,
          event.xproperty.atom,0L,(long) MaxTextExtent,MagickFalse,(Atom)
          AnyPropertyType,&type,&format,&length,&after,&data);
        if ((status != Success) || (length == 0))
          break;
        (void) CopyMagickString(resource_info->image_info->filename,
          (char *) data,MaxTextExtent);
        nexus=ReadImage(resource_info->image_info,&image->exception);
        CatchException(&image->exception);
        if (nexus != (Image *) NULL)
          state|=ExitState;
        (void) XFree((void *) data);
        break;
      }
      case ReparentNotify:
      {
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),
            "Reparent Notify: 0x%lx=>0x%lx",event.xreparent.parent,
            event.xreparent.window);
        break;
      }
      case UnmapNotify:
      {
        if (display_image->debug != MagickFalse)
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
        if (event.xunmap.window == windows->info.id)
          {
            windows->info.mapped=MagickFalse;
            break;
          }
        if (event.xunmap.window == windows->icon.id)
          {
            if (map_info->colormap == icon_map->colormap)
              XConfigureImageColormap(display,resource_info,windows,
                display_image);
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
        if (display_image->debug != MagickFalse)
          (void) LogMagickEvent(X11Event,GetMagickModule(),"Event type: %d",
            event.type);
        break;
      }
    }
  }
  while (!(state & ExitState));
  image_list=(Image **) RelinquishMagickMemory(image_list);
  images=DestroyImageList(images);
  if ((windows->visual_info->klass == GrayScale) ||
      (windows->visual_info->klass == PseudoColor) ||
      (windows->visual_info->klass == DirectColor))
    {
      /*
        Withdraw windows.
      */
      if (windows->info.mapped)
        (void) XWithdrawWindow(display,windows->info.id,windows->info.screen);
      if (windows->command.mapped)
        (void) XWithdrawWindow(display,windows->command.id,
          windows->command.screen);
    }
  if (resource_info->backdrop == MagickFalse)
    if (windows->backdrop.mapped)
      {
        (void) XWithdrawWindow(display,windows->backdrop.id,\
          windows->backdrop.screen);
        (void) XDestroyWindow(display,windows->backdrop.id);
        windows->backdrop.id=(Window) NULL;
        (void) XWithdrawWindow(display,windows->image.id,windows->image.screen);
        (void) XDestroyWindow(display,windows->image.id);
        windows->image.id=(Window) NULL;
      }
  XSetCursorState(display,windows,MagickTrue);
  XCheckRefreshWindows(display,windows);
  for (scene=1; scene < (ssize_t) number_scenes; scene++)
  {
    if (windows->image.pixmaps[scene] != (Pixmap) NULL)
      (void) XFreePixmap(display,windows->image.pixmaps[scene]);
    windows->image.pixmaps[scene]=(Pixmap) NULL;
    if (windows->image.matte_pixmaps[scene] != (Pixmap) NULL)
      (void) XFreePixmap(display,windows->image.matte_pixmaps[scene]);
    windows->image.matte_pixmaps[scene]=(Pixmap) NULL;
  }
  XSetCursorState(display,windows,MagickFalse);
  windows->image.pixmaps=(Pixmap *)
    RelinquishMagickMemory(windows->image.pixmaps);
  windows->image.matte_pixmaps=(Pixmap *)
    RelinquishMagickMemory(windows->image.matte_pixmaps);
  if (nexus == (Image *) NULL)
    {
      /*
        Free X resources.
      */
      if (windows->image.mapped != MagickFalse)
        (void) XWithdrawWindow(display,windows->image.id,windows->image.screen);      XDelay(display,SuspendTime);
      (void) XFreeStandardColormap(display,icon_visual,icon_map,icon_pixel);
      if (resource_info->map_type == (char *) NULL)
        (void) XFreeStandardColormap(display,visual_info,map_info,pixel);
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
  directory=getcwd(working_directory,MaxTextExtent);
  (void) directory;
  status=chdir(resource_info->home_directory);
  if (status == -1)
    (void) ThrowMagickException(&images->exception,GetMagickModule(),
      FileOpenError,"UnableToOpenFile","%s",resource_info->home_directory);
  return(nexus);
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
%        XResourceInfo *resource_info,XWindows *windows,Image *image)
%
%  A description of each parameter follows:
%
%    o status: Method XSaveImage return True if the image is
%      written.  False is returned is there is a memory shortage or if the
%      image fails to write.
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
*/
static MagickBooleanType XSaveImage(Display *display,
  XResourceInfo *resource_info,XWindows *windows,Image *image)
{
  char
    filename[MaxTextExtent];

  ImageInfo
    *image_info;

  MagickStatusType
    status;

  /*
    Request file name from user.
  */
  if (resource_info->write_filename != (char *) NULL)
    (void) CopyMagickString(filename,resource_info->write_filename,
      MaxTextExtent);
  else
    {
      char
        path[MaxTextExtent];

      int
        status;

      GetPathComponent(image->filename,HeadPath,path);
      GetPathComponent(image->filename,TailPath,filename);
      status=chdir(path);
      if (status == -1)
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          FileOpenError,"UnableToOpenFile","%s",path);
    }
  XFileBrowserWidget(display,windows,"Save",filename);
  if (*filename == '\0')
    return(MagickTrue);
  if (IsPathAccessible(filename) != MagickFalse)
    {
      int
        status;

      /*
        File exists-- seek user's permission before overwriting.
      */
      status=XConfirmWidget(display,windows,"Overwrite",filename);
      if (status == 0)
        return(MagickTrue);
    }
  image_info=CloneImageInfo(resource_info->image_info);
  (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
  (void) SetImageInfo(image_info,1,&image->exception);
  if ((LocaleCompare(image_info->magick,"JPEG") == 0) ||
      (LocaleCompare(image_info->magick,"JPG") == 0))
    {
      char
        quality[MaxTextExtent];

      int
        status;

      /*
        Request JPEG quality from user.
      */
      (void) FormatLocaleString(quality,MaxTextExtent,"%.20g",(double)
        image_info->quality);
      status=XDialogWidget(display,windows,"Save","Enter JPEG quality:",
        quality);
      if (*quality == '\0')
        return(MagickTrue);
      image->quality=StringToUnsignedLong(quality);
      image_info->interlace=status != MagickFalse ?  NoInterlace :
        PlaneInterlace;
    }
  if ((LocaleCompare(image_info->magick,"EPS") == 0) ||
      (LocaleCompare(image_info->magick,"PDF") == 0) ||
      (LocaleCompare(image_info->magick,"PS") == 0) ||
      (LocaleCompare(image_info->magick,"PS2") == 0))
    {
      char
        geometry[MaxTextExtent];

      /*
        Request page geometry from user.
      */
      (void) CopyMagickString(geometry,PSPageGeometry,MaxTextExtent);
      if (LocaleCompare(image_info->magick,"PDF") == 0)
        (void) CopyMagickString(geometry,PSPageGeometry,MaxTextExtent);
      if (image_info->page != (char *) NULL)
        (void) CopyMagickString(geometry,image_info->page,MaxTextExtent);
      XListBrowserWidget(display,windows,&windows->widget,PageSizes,"Select",
        "Select page geometry:",geometry);
      if (*geometry != '\0')
        image_info->page=GetPageGeometry(geometry);
    }
  /*
    Write image.
  */
  image=GetFirstImageInList(image);
  status=WriteImages(image_info,image,filename,&image->exception);
  if (status != MagickFalse)
    image->taint=MagickFalse;
  image_info=DestroyImageInfo(image_info);
  XSetCursorState(display,windows,MagickFalse);
  return(status != 0 ? MagickTrue : MagickFalse);
}
#else

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A n i m a t e I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AnimateImages() repeatedly displays an image sequence to any X window
%  screen.  It returns a value other than 0 if successful.  Check the
%  exception member of image to determine the reason for any failure.
%
%  The format of the AnimateImages method is:
%
%      MagickBooleanType AnimateImages(const ImageInfo *image_info,
%        Image *images)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType AnimateImages(const ImageInfo *image_info,
  Image *image)
{
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) ThrowMagickException(&image->exception,GetMagickModule(),
    MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (X11)",
    image->filename);
  return(MagickFalse);
}
#endif

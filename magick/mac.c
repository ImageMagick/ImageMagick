/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            M   M   AAA    CCCC                              %
%                            MM MM  A   A  C                                  %
%                            M M M  AAAAA  C                                  %
%                            M   M  A   A  C                                  %
%                            M   M  A   A   CCCC                              %
%                                                                             %
%                                                                             %
%                    Macintosh Utility Methods for MagickCore                 %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                September 1996                               %
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
%  The directory methods are strongly based on similar methods written
%  by Steve Summit, scs@eskimo.com.  The Ghostscript launch code is strongly
%  based on Dave Schooley's Mac Gnuplot and contributed by
%  schindall@wave14i.nrl.navy.mil.  Mac-centric improvements contributed by
%  leonardr@digapp.com.
%
%
*/

#if defined(macintosh)
/*
  Include declarations.
*/
#define _X_H
#define _WIDGET_H
#include <AppleEvents.h>
#include <AERegistry.h>
#include <AEObjects.h>
#include <AEPackObject.h>
#include <Processes.h>
#include <QuickDraw.h>
#include <QDOffscreen.h>
#include <Palettes.h>
#include <ImageCompression.h>
#include <PictUtils.h>
#include <Files.h>
#include <Gestalt.h>
#include <TextUtils.h>
#define ColorInfo  KolorInfo
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/client.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum.h"
#include "magick/string_.h"
#include "magick/utility.h"
#include "magick/mac.h"

/*
  Global declaractions.
*/
ImageDescriptionHandle
  image_description = nil;

/*
  Forward declaractions.
*/
static Boolean
  SearchForFile(OSType,OSType,FSSpec *,short);

static pascal void
  ArcMethod(GrafVerb,Rect *,short,short),
  BitsMethod(BitMap *,Rect *,Rect *,short,RgnHandle),
  FilenameToFSSpec(const char *filename,FSSpec *fsspec),
  LineMethod(Point),
  OvalMethod(GrafVerb,Rect *),
  PolyMethod(GrafVerb,PolyHandle),
  RRectMethod(GrafVerb,Rect *,short,short),
  RectMethod(GrafVerb,Rect *),
  RegionMethod(GrafVerb,RgnHandle),
  StandardPixmap(PixMapPtr,Rect *,MatrixRecordPtr,short,RgnHandle,PixMapPtr,
    Rect *,short),
  TextMethod(short,Ptr,Point,Point);

/*
  Static declarations
 */
#if defined(DISABLE_SIOUX)
static MACEventHookPtr
  event_hook = nil;

static MACErrorHookPtr
  exception.hook = nil;
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B o t t l e n e c k T e s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BottleneckTest() intercepts any compressed images.
%
%  The format of the BottleneckTest method is:
%
%      int BottleneckTest(const char *magick)
%
%  A description of each parameter follows:
%
%    o picture: Specifies a pointer to a PicHandle structure.
%
%    o codec: the code type is returned in this CodecType pointer structure.
%
%    o depth: the image depth is returned as an integer pointer.
%
%    o colormap_id: the colormap ID is returned in this short pointer.
%
%
*/

static pascal void ArcMethod(GrafVerb verb,Rect *r,short startAngle,
  short arcAngle)
{
#pragma unused (verb,r,startAngle,arcAngle)
}

static pascal void BitsMethod(BitMap *bitPtr,Rect *source_rectangle,
  Rect *dstRect,short mode,RgnHandle maskRgn)
{
#pragma unused (bitPtr,source_rectangle,dstRect,mode,maskRgn)
}

static pascal void LineMethod(Point newPt)
{
#pragma unused (newPt)
}

static pascal void OvalMethod(GrafVerb verb,Rect *r)
{
#pragma unused (verb,r)
}

static pascal void PolyMethod(GrafVerb verb,PolyHandle poly)
{
#pragma unused (verb,poly)
}

static pascal void RectMethod(GrafVerb verb,Rect *r)
{
#pragma unused (verb,r)
}

static pascal void RegionMethod(GrafVerb verb,RgnHandle rgn)
{
#pragma unused (verb,rgn)
}

static pascal void RRectMethod(GrafVerb verb,Rect *r,short ovalWidth,
  short ovalHeight)
{
#pragma unused (verb,r,ovalWidth,ovalHeight)
}

static pascal void StandardPixmap(PixMapPtr source,Rect *source_rectangle,
  MatrixRecordPtr matrix,short mode,RgnHandle mask,PixMapPtr matte,
  Rect *matte_rectangle,short flags)
{
#pragma unused (source_rectangle,matrix,mode,mask,matte,matte_rectangle,flags)

  Ptr
    data;

  ssize_t
    size;

  GetCompressedPixMapInfo(source,&image_description,&data,&size,nil,nil);
}

static pascal void TextMethod(short byteCount,Ptr textBuf,Point numer,
  Point denom)
{
#pragma unused (byteCount,textBuf,numer,denom)
}

#if !defined(DISABLE_QUICKTIME)
static short BottleneckTest(PicHandle picture,CodecType *codec,int *depth,
  short *colormap_id)
{
  CQDProcs
    bottlenecks;

  int
    status;

  Rect
    rectangle;

  ssize_t
    version;

  status=Gestalt(gestaltQuickTime,&version);
  if (status != noErr)
    {
      ParamText("\pQuickTime not installed.  Please install, then try again.",
        "\p","\p","\p");
      Alert(128,nil);
      return(-1);
    }
  /*
    Define our own bottlenecks to do nothing.
  */
  SetStdCProcs(&bottlenecks);
  bottlenecks.textProc=NewQDTextUPP(&TextMethod);
  bottlenecks.lineProc=NewQDLineUPP(&LineMethod);
  bottlenecks.rectProc=NewQDRectUPP(&RectMethod);
  bottlenecks.rRectProc=NewQDRRectUPP(&RRectMethod);
  bottlenecks.ovalProc=NewQDOvalUPP(&OvalMethod);
  bottlenecks.arcProc=NewQDArcUPP(&ArcMethod);
  bottlenecks.polyProc=NewQDPolyUPP(&PolyMethod);
  bottlenecks.rgnProc=NewQDRgnUPP(&RegionMethod);
  bottlenecks.bitsProc=NewQDBitsUPP(&BitsMethod);
  bottlenecks.newProc1=(UniversalProcPtr) NewStdPixUPP(&StandardPixmap);
  /*
    Install our custom bottlenecks to intercept any compressed images.
  */
  (*(qd.thePort)).grafProcs=(QDProcs *) &bottlenecks;
  DrawPicture(picture,&((**picture).picFrame));
  PaintRect(&rectangle);
  (*(qd.thePort)).grafProcs=0L;
  /*
    Initialize our return values.
  */
  *codec='unkn';
  *depth=0;
  *colormap_id=(-1);
  if (image_description != nil)
    {
      *codec=(**image_description).cType;
      *depth=(**image_description).depth;
      *colormap_id=(**image_description).clutID;
    }
  DisposeQDTextUPP(bottlenecks.textProc);
  DisposeQDLineUPP(bottlenecks.lineProc);
  DisposeQDRectUPP(bottlenecks.rectProc);
  DisposeQDRRectUPP(bottlenecks.rRectProc);
  DisposeQDOvalUPP(bottlenecks.ovalProc);
  DisposeQDArcUPP(bottlenecks.arcProc);
  DisposeQDPolyUPP(bottlenecks.polyProc);
  DisposeQDRgnUPP(bottlenecks.rgnProc);
  DisposeQDBitsUPP(bottlenecks.bitsProc);
  DisposeStdPixUPP(bottlenecks.newProc1);
  return(0);
}
#endif

#if !defined(_MAGICKCORE_POSIX_SUPPORT_VERSION)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   c l o s e d i r                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  closedir() closes the named directory stream and frees the DIR structure.
%
%  The format of the closedir method is:
%
%      closedir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
MagickExport void closedir(DIR *entry)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(entry != (DIR *) NULL);
  RelinquishMagickMemory(entry);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x i t                                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Exit() exits the process.
%
%  The format of the exit method is:
%
%      Exit(status)
%
%  A description of each parameter follows:
%
%    o status: an integer value representing the status of the terminating
%      process.
%
%
*/
MagickExport int Exit(int status)
{
#if !defined(DISABLE_SIOUX)
  (void) FormatLocaleFile(stdout,"Select File->Quit to exit.\n");
#endif
  exit(status);
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e n a m e T o F S S p e c                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FilenameToFSSpec() sets the file type of an image.
%
%  The format of the FilenameToFSSpec method is:
%
%      FilenameToFSSpec(filename,fsspec)
%
%  A description of each parameter follows:
%
%    o filename: Specifies the name of the file.
%
%    o fsspec: A pointer to type FSSpec.
%
%
*/
MagickExport void pascal FilenameToFSSpec(const char *filename,FSSpec *fsspec)
{
  Str255
    name;

  assert(filename != (char *) NULL);
  c2pstrcpy(name,filename);
  FSMakeFSSpec(0,0,name,fsspec);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M a g i c k C o n f l i c t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MACIsMagickConflict() returns true if the image format conflicts with a
%  logical drive (.e.g. X:).
%
%  Contributed by Mark Gavin of Digital Applications, Inc.
%
%  The format of the MACIsMagickConflict method is:
%
%      status=MACIsMagickConflict(magick)
%
%  A description of each parameter follows:
%
%    o magick: Specifies the image format.
%
%
*/

static OSErr HGetVInfo(short volume_index,StringPtr volume_name,short *volume,
  size_t *free_bytes,size_t *total_bytes)
{
  HParamBlockRec
    pb;

  OSErr
    result;

  size_t
    blocksize;

  unsigned short
    allocation_blocks,
    free_blocks;

  /*
    Use the File Manager to get the real vRefNum.
  */
  pb.volumeParam.ioVRefNum=0;
  pb.volumeParam.ioNamePtr=volume_name;
  pb.volumeParam.ioVolIndex=volume_index;
  result=PBHGetVInfoSync(&pb);
  if (result != noErr)
    return(result);
  *volume=pb.volumeParam.ioVRefNum;
  blocksize=(size_t) pb.volumeParam.ioVAlBlkSiz;
  allocation_blocks=(unsigned short) pb.volumeParam.ioVNmAlBlks;
  free_blocks=(unsigned short) pb.volumeParam.ioVFrBlk;
  *free_bytes=free_blocks*blocksize;
  *total_bytes=allocation_blocks*blocksize;
  return(result);
}

MagickExport MagickBooleanType MACIsMagickConflict(const char *magick)
{
  size_t
    free_bytes,
    number_bytes;

  OSErr
    status;

  short
    volume;

  Str255
    volume_name;

  assert(magick != (char *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",magick);
  (void) CopyMagickString((char *) volume_name,magick,MaxTextExtent);
  c2pstr((char *) volume_name);
  if (volume_name[volume_name[0]] != ':')
    volume_name[++volume_name[0]]=':';
  status=HGetVInfo(-1,volume_name,&volume,&free_bytes,&number_bytes);
  return(status != 0 ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M A C E r r o r H a n d l e r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MACErrorHandler() displays an error reason and then terminates the program.
%
%  The format of the MACErrorHandler method is:
%
%      void MACErrorHandler(const ExceptionType error,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o exception: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
%
*/
MagickExport void MACErrorHandler(const ExceptionType error,const char *reason,
  const char *description)
{
  char
    buffer[3*MaxTextExtent];

  if (reason == (char *) NULL)
    return;
  if (description == (char *) NULL)
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s.\n",GetClientName(),
      reason);
  else
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s (%s).\n",
      GetClientName(),reason,description);
#if defined(DISABLE_SIOUX)
  if(exception.hook != (MACErrorHookPtr) NULL)
    exception.hook(error,buffer);
  else
    {
      MagickCoreTerminus();
      exit(error);
    }
#else
  puts(buffer);
  MagickCoreTerminus();
  exit(error);
#endif
}

#if defined(DISABLE_SIOUX)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M A C F a t a l E r r o r H a n d l e r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MACFatalErrorHandler() displays an error reason and then terminates the
%  program.
%
%  The format of the MACFatalErrorHandler method is:
%
%      void MACFatalErrorHandler(const ExceptionType severity,
%        const char *reason,const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
static void MACFatalErrorHandler(const ExceptionType severity,
  const char *reason,const char *description)
{
  char
    buffer[3*MaxTextExtent];

  if (reason == (char *) NULL)
    return;
  if (description == (char *) NULL)
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s.\n",GetClientName(),
      reason);
  else
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s (%s).\n",
      GetClientName(),reason,description);
  if(exception.hook != (MACErrorHookPtr) NULL)
    exception.hook(severity, buffer);
  else
    {
      MagickCoreTerminus();
      exit(severity);
    }
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a c G S E x e c u t e C o m m a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MacGSExecuteCommand() executes the Ghostscript command.
%
%
*/
static OSErr MacGSExecuteCommand(const char *command,ssize_t length)
{
  AEAddressDesc
    event_descriptor;

  AEDesc
    reply = {typeNull, NULL};

  AppleEvent
    event = {typeNull, NULL};

  DescType
    descriptor_type;

  int
    error;

  OSType
    id = 'gsVR';

  Size
    actualSize;

  /*
    Send the Apple Event.
  */
  (void) AECreateDesc(typeApplSignature,&id,sizeof(id),&event_descriptor);
  (void) AECreateAppleEvent(id,'exec',&event_descriptor,-1,kAnyTransactionID,
    &event);
  (void) AEPutParamPtr(&event,keyDirectObject,typeChar,command,length);
  (void) AESend(&event,&reply,kAEWaitReply+kAENeverInteract,kAENormalPriority,
    kNoTimeOut,NULL,NULL);
  /*
    Handle the reply and exit.
  */
  (void) AEGetParamPtr(&reply,keyDirectObject,typeInteger,&descriptor_type,
    &error,sizeof(error),&actualSize);
  (void) AEDisposeDesc(&event_descriptor);
  (void) AEDisposeDesc(&event);
  if (reply.descriptorType != NULL)
    AEDisposeDesc(&reply);
  return((OSErr) error);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a c G S L a u n c h A p p l i c a t i o n C o r e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MacGSLaunchApplicationCore() launches the Ghostscript command.
%
%
*/
static OSErr MacGSLaunchApplicationCore(ssize_t flags)
{
  FSSpec
    file_info;

  LaunchParamBlockRec
    launch_info;

  OSErr
    error;

  if (!SearchForFile('gsVR','APPL',&file_info,1))
    return(-43);
  launch_info.launchBlockID=extendedBlock;
  launch_info.launchEPBLength=extendedBlockLen;
  launch_info.launchFileFlags=0;
  launch_info.launchControlFlags=launchContinue+launchNoFileFlags+flags;
  launch_info.launchAppSpec=(&file_info);
  launch_info.launchAppParameters=nil;
  error=LaunchApplication(&launch_info);
  return(error);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a c G S L a u n c h A p p l i c a t i o n                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MacGSLaunchApplication() launches the Ghostscript command.
%
%
*/
static OSErr MacGSLaunchApplication(void)
{
  return(MacGSLaunchApplicationCore(launchDontSwitch));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a c G S L a u n c h A p p l i c a t i o n T o F r o n t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MacGSLaunchApplicationToFront() moves the Ghostscript window to the front.
%
%
*/
static OSErr MacGSLaunchApplicationToFront(void)
{
  return(MacGSLaunchApplicationCore(0));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a c G S Q u i t A p p l i c a t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MacGSQuitApplication() quits the Ghostscript application.
%
%
*/
static void MacGSQuitApplication(void)
{
  AEAddressDesc
    event_descriptor;

  AEDesc
    reply = {typeNull, NULL};

  AppleEvent
    event = {typeNull, NULL};

  OSType
    id = 'GPLT';

  /*
    Send the Apple Event.
  */
  (void) AECreateDesc(typeApplSignature,&id,sizeof(id),&event_descriptor);
  (void) AECreateAppleEvent(typeAppleEvent,kAEQuitApplication,
    &event_descriptor,-1,kAnyTransactionID,&event);
  (void) AESend(&event,&reply,kAENoReply,kAENormalPriority,kNoTimeOut,NULL,
    NULL);
  /*
    Clean up and exit.
  */
  (void) AEDisposeDesc(&event_descriptor);
  (void) AEDisposeDesc(&event);
  if (reply.descriptorType != NULL)
    AEDisposeDesc(&reply);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a c G S S e t W o r k i n g F o l d e r                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MacGSSetWorkingFolder() set the Ghostscript working folder.
%
%
*/
static OSErr MacGSSetWorkingFolder(char *directory)
{
  AEDesc
    application_descriptor,
    event_descriptor,
    object,
    path_descriptor,
    type_descriptor,
    reply;

  AppleEvent
    event;

  DescType
    folder_type = 'wfdr';

  OSErr
    error;

  OSType
    id = 'GPLT';

  /*
    Send the Apple Event.
  */
  AECreateDesc(typeNull,NULL,0,&application_descriptor);
  AECreateDesc(typeChar,directory,strlen(directory),&path_descriptor);
  (void) AECreateDesc(typeType,&folder_type,sizeof(DescType),&type_descriptor);
  CreateObjSpecifier(cProperty,&application_descriptor,formPropertyID,
    &type_descriptor,0,&object);
  (void) AECreateDesc(typeApplSignature,&id,sizeof(id),&event_descriptor);
  (void) AECreateAppleEvent(kAECoreSuite,kAESetData,&event_descriptor,-1,
    kAnyTransactionID,&event);
  (void) AEPutParamDesc(&event,keyDirectObject,&object);
  (void) AEPutParamDesc(&event,keyAEData,&path_descriptor);
  error=AESend(&event,&reply,kAENoReply+kAENeverInteract,kAENormalPriority,
    kNoTimeOut,NULL,NULL);
  (void) AEDisposeDesc(&event);
  (void) AEDisposeDesc(&event_descriptor);
  (void) AEDisposeDesc(&object);
  (void) AEDisposeDesc(&type_descriptor);
  (void) AEDisposeDesc(&path_descriptor);
  (void) AEDisposeDesc(&application_descriptor);
  return(error);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M A C S e t E r r o r H o o k                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%   MACSetErrorHook sets a callback function which is called if any error
%   occurs within ImageMagick.
%
%  The format of the MACSetErrorHook method is:
%
%      int MACSetErrorHook(MACErrorHookPtr hook)
%
%  A description of each parameter follows:
%
%    o hook: This function pointer is the callback function.
%
%
*/
MagickExport void MACSetErrorHook(MACErrorHookPtr hook)
{
  /*
    We forget any previously set exception.hook.
  */
  exception.hook=hook;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M A C S e t E v e n t H o o k                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%   MACSetEventHook sets a callback function which is called every time
%   ImageMagick likes to release the processor.
%
%  The format of the MACSetEventHook method is:
%
%      int MACSetEventHook(MACEventHookPtr hook)
%
%  A description of each parameter follows:
%
%    o hook: This function pointer is the callback function.
%
%
*/
MagickExport void MACSetEventHook(MACEventHookPtr hook)
{
  /*
    We forget any previously set event hook.
   */
  event_hook=hook;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M A C S y s t e m C o m m a n d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%   Method MACSystemCommand executes the specified command and waits until it
%   terminates.  The returned value is the exit status of the command.
%
%  The format of the MACSystemCommand method is:
%
%      int MACSystemCommand(MagickFalse,const char * command)
%
%  A description of each parameter follows:
%
%    o command: This string is the command to execute.
%
*/
MagickExport int MACSystemCommand(const char * command)
{
  /*
    We only know how to launch Ghostscript.
  */
  if (MacGSLaunchApplicationToFront())
    return(-1);
  return(MacGSExecuteCommand(command,strlen(command)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M A C W a r n i n g H a n d l e r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MACWarningHandler() displays a warning reason.
%
%  The format of the MACWarningHandler method is:
%
+      void MACWarningHandler(const ExceptionType warning,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o warning: Specifies the numeric warning category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
%
*/
MagickExport void MACWarningHandler(const ExceptionType warning,
  const char *reason,const char *description)
{
  char
    buffer[1664];

  if (reason == (char *) NULL)
    return;
  if (description == (char *) NULL)
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s.\n",GetClientName(),
      reason);
  else
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s (%s).\n",
      GetClientName(),reason,description);
#if defined(DISABLE_SIOUX)
  if(exception.hook != (MACErrorHookPtr) NULL)
    exception.hook(warning, buffer);
#else
  (void)warning;
  puts(buffer);
#endif
}

#if !defined(_MAGICKCORE_POSIX_SUPPORT_VERSION)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   o p e n d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  opendir() opens the directory named by filename and associates a directory
%  stream with it.
%
%  The format of the opendir method is:
%
%      MagickExport DIR *opendir(char *path)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
MagickExport DIR *opendir(const char *path)
{
  Str255 pathname;

  CInfoPBRec
    search_info;

  DIR
    *entry;

  int
    error;

  search_info.hFileInfo.ioNamePtr=0;
  if ((path != (char *) NULL) || (*path != '\0'))
    if ((path[0] != '.') || (path[1] != '\0'))
      {
        c2pstrcpy(pathname,path);
        search_info.hFileInfo.ioNamePtr=pathname;
      }
  search_info.hFileInfo.ioCompletion=0;
  search_info.hFileInfo.ioVRefNum=0;
  search_info.hFileInfo.ioFDirIndex=0;
  search_info.hFileInfo.ioDirID=0;
  error=PBGetCatInfoSync(&search_info);
  if (error != noErr)
    {
      errno=error;
      return((DIR *) NULL);
    }
  entry=(DIR *) AcquireMagickMemory(sizeof(DIR));
  if (entry == (DIR *) NULL)
    return((DIR *) NULL);
  entry->d_VRefNum=search_info.hFileInfo.ioVRefNum;
  entry->d_DirID=search_info.hFileInfo.ioDirID;
  entry->d_index=1;
  return(entry);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r o c e s s P e n d i n g E v e n t s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ProcessPendingEvents() processes any pending events.  This prevents
%  ImageMagick from monopolizing the processor.
%
%  The format of the ProcessPendingEvents method is:
%
%      ProcessPendingEvents(text)
%
%  A description of each parameter follows:
%
%    o text: A character string representing the current process.
%
%
*/
MagickExport void ProcessPendingEvents(const char *text)
{
#if defined(DISABLE_SIOUX)
  if (event_hook != (MACEventHookPtr) NULL)
    event_hook(text);
#else
  static const char
    *mark = (char *) NULL;

  EventRecord
    event;

  while (WaitNextEvent(everyEvent,&event,0L,nil))
    SIOUXHandleOneEvent(&event);
  if (isatty(STDIN_FILENO) && (text != mark))
    {
      (void) puts(text);
      mark=text;
    }
#endif
}

#if !defined(_MAGICKCORE_POSIX_SUPPORT_VERSION)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   r e a d d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  readdir() returns a pointer to a structure representing the directory entry
%  at the current position in the directory stream to which entry refers.
%
%  The format of the readdir
%
%      struct dirent *readdir(DIR *entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
MagickExport struct dirent *readdir(DIR *entry)
{
  CInfoPBRec
    search_info;

  int
    error;

  static struct dirent
    dir_entry;

  static unsigned char
    pathname[MaxTextExtent];

  if (entry == (DIR *) NULL)
    return((struct dirent *) NULL);
  search_info.hFileInfo.ioCompletion=0;
  search_info.hFileInfo.ioNamePtr=pathname;
  search_info.hFileInfo.ioVRefNum=0;
  search_info.hFileInfo.ioFDirIndex=entry->d_index;
  search_info.hFileInfo.ioDirID=entry->d_DirID;
  error=PBGetCatInfoSync(&search_info);
  if (error != noErr)
    {
      errno=error;
      return((struct dirent *) NULL);
    }
  entry->d_index++;
  p2cstrcpy(dir_entry.d_name,search_info.hFileInfo.ioNamePtr);
  dir_entry.d_namlen=strlen(dir_entry.d_name);
  return(&dir_entry);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P I C T I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPICTImage() reads an Apple Macintosh QuickDraw/PICT image file using
%  MacOS QuickDraw methods and returns it.  It allocates the memory necessary
%  for the new Image structure and returns a pointer to the new image.
%
%  This method was written and contributed by spd@daphne.cps.unizar.es
%  (feel free to copy and use it as you want. No warranty).
%
%  The format of the ReadPICTImage method is:
%
%      Image *ReadPICTImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPICTImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: the image info..
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

MagickExport Image *ReadPICTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define PICTHeaderSize    512

  CodecType
    codec;

  GDHandle
    device;

  GWorldPtr
    graphic_world,
    port;

  Image
    *image;

  int
    depth,
    status;

  MagickBooleanType
    proceed,
    status;

  PicHandle
    picture_handle;

  PictInfo
    picture_info;

  QDErr
    theErr = noErr;

  Rect
    rectangle;

  RGBColor
    Pixel;

  short
    colormap_id;

  ssize_t
    y;

  /*
    Open image file.
  */
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(NULL);
  picture_handle=(PicHandle) NewHandle(MagickMax(GetBlobSize(image)-
    PICTHeaderSize,PICTHeaderSize));
  if (picture_handle == nil)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  HLock((Handle) picture_handle);
  (void) ReadBlob(image,PICTHeaderSize,*(unsigned char **) picture_handle);
  status=ReadBlob(image,GetBlobSize(image)-PICTHeaderSize,*(unsigned char **)
    picture_handle);
  if (status == MagickFalse)
    {
      DisposeHandle((Handle) picture_handle);
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    }
  GetGWorld(&port,&device);
  theErr=NewGWorld(&graphic_world,0,&(**picture_handle).picFrame,nil,nil,
    useTempMem | keepLocal);
  if ((theErr != noErr) && (graphic_world == nil))
    {
      DisposeHandle((Handle) picture_handle);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  HUnlock((Handle) picture_handle);
  SetGWorld(graphic_world,nil);
  theErr=GetPictInfo(picture_handle,&picture_info,0,1,systemMethod,0);
  if (theErr != noErr)
    {
      DisposeGWorld(graphic_world);
      DisposeHandle((Handle) picture_handle);
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    }
#if defined(DISABLE_QUICKTIME)
  codec='unkn';
  colormap_id=(-1);
  depth=picture_info.depth;
#else
  BottleneckTest(picture_handle,&codec,&depth,&colormap_id);
#endif
  switch (codec)
  {
    case 'rpza':
    case 'jpeg':
    case 'rle ':
    case 'raw ':
    case 'smc ':
    {
      if (depth > 200)
        {
          depth-=32;
          picture_info.theColorTable=GetCTable(colormap_id);
        }
      break;
    }
    default:
    {
      depth=picture_info.depth;
      if (depth <= 8)
        (void) GetPictInfo(picture_handle,&picture_info,returnColorTable,
          (short) (1 << picture_info.depth),systemMethod,0);
      break;
    }
  }
  image->x_resolution=(picture_info.hRes) >> 16;
  image->y_resolution=(picture_info.vRes) >> 16;
  image->units=PixelsPerInchResolution;
  image->columns=picture_info.sourceRect.right-picture_info.sourceRect.left;
  image->rows=picture_info.sourceRect.bottom-picture_info.sourceRect.top;
  if ((depth <= 8) && ((*(picture_info.theColorTable))->ctSize != 0))
    {
      size_t
        number_colors;

      /*
        Colormapped PICT image.
      */
      number_colors=(*(picture_info.theColorTable))->ctSize;
      if (!AcquireImageColormap(image,number_colors))
        {
          if (picture_info.theColorTable != nil)
            DisposeHandle((Handle) picture_info.theColorTable);
          DisposeGWorld(graphic_world);
          DisposeHandle((Handle) picture_handle);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      for (x=0; x < image->colors; x++)
      {
        image->colormap[x].red=
          (*(picture_info.theColorTable))->ctTable[x].rgb.red;
        image->colormap[x].green=
          (*(picture_info.theColorTable))->ctTable[x].rgb.green;
        image->colormap[x].blue=
          (*(picture_info.theColorTable))->ctTable[x].rgb.blue;
      }
    }
  SetRect(&rectangle,0,0,image->columns,image->rows);
  (void) UpdateGWorld(&graphic_world,depth,&rectangle,
    picture_info.theColorTable,nil,0);
  LockPixels(GetGWorldPixMap(graphic_world));  /*->portPixMap); */
  EraseRect(&rectangle);
  DrawPicture(picture_handle,&rectangle);
  if ((depth <= 8) && (colormap_id == -1))
    {
      DisposeHandle((Handle) picture_info.theColorTable);
      picture_info.theColorTable=nil;
    }
  DisposeHandle((Handle) picture_handle);
  /*
    Convert PICT pixels to pixel packets.
  */
  for (y=0; y < image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < image->columns; x++)
    {
      GetCPixel(x,y,&Pixel);
      SetPixelRed(q,ScaleCharToQuantum(Pixel.red & 0xff));
      SetPixelGreen(q,ScaleCharToQuantum(Pixel.green & 0xff));
      SetPixelBlue(q,ScaleCharToQuantum(Pixel.blue & 0xff));
      if (image->storage_class == PseudoClass)
        SetPixelIndex(indexes+x,Color2Index(&Pixel));
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,LoadImageTag,y,image->rows);
    if (proceed == MagickFalse)
      break;
  }
  UnlockPixels(GetGWorldPixMap(graphic_world));
  SetGWorld(port,device);
  if (picture_info.theColorTable != nil)
    DisposeHandle((Handle) picture_info.theColorTable);
  DisposeGWorld(graphic_world);
  (void) CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e a r c h F o r F i l e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SearchForFile() searches for a file.
%
%
*/
static Boolean SearchForFile(OSType creator_type,OSType file_type,FSSpec *file,
  short count)
{
  char
    *buffer;

  CInfoPBRec
    search1_info,
    search2_info;

  FSSpec
    application;

  HParamBlockRec
    parameter_info;

  OSErr
    error;

  ProcessInfoRec
    application_info;

  ProcessSerialNumber
    serial_number;

  ssize_t
    buffer_size = 16384;

  serial_number.lowLongOfPSN=kCurrentProcess;
  serial_number.highLongOfPSN=0;
  application_info.processInfoLength=sizeof(ProcessInfoRec);
  application_info.processName=NULL;
  application_info.processAppSpec=(&application);
  GetProcessInformation(&serial_number,&application_info);
  buffer=NewPtr(buffer_size);
  if (buffer == (char *) NULL)
    return(false);
  parameter_info.csParam.ioCompletion=NULL;
  parameter_info.csParam.ioNamePtr=NULL;
  parameter_info.csParam.ioVRefNum=application.vRefNum;
  parameter_info.csParam.ioMatchPtr=file;
  parameter_info.csParam.ioReqMatchCount=count;
  parameter_info.csParam.ioSearchBits=fsSBFlFndrInfo;
  parameter_info.csParam.ioSearchInfo1=&search1_info;
  parameter_info.csParam.ioSearchInfo2=&search2_info;
  parameter_info.csParam.ioSearchTime=0;
  parameter_info.csParam.ioCatPosition.initialize=0;
  parameter_info.csParam.ioOptBuffer=buffer;
  parameter_info.csParam.ioOptBufSize=buffer_size;
  search1_info.hFileInfo.ioNamePtr=NULL;
  search1_info.hFileInfo.ioFlFndrInfo.fdType=file_type;
  search1_info.hFileInfo.ioFlFndrInfo.fdCreator=creator_type;
  search1_info.hFileInfo.ioFlAttrib=0;
  search1_info.hFileInfo.ioFlParID=0;
  search2_info=search1_info;
  search2_info.hFileInfo.ioFlAttrib=0x10;
  search2_info.hFileInfo.ioFlFndrInfo.fdCreator=creator_type;
  search2_info.hFileInfo.ioFlFndrInfo.fdType=(-1);
  search2_info.hFileInfo.ioFlFndrInfo.fdFlags=0;
  search2_info.hFileInfo.ioFlFndrInfo.fdLocation.h=0;
  search2_info.hFileInfo.ioFlFndrInfo.fdLocation.v=0;
  search2_info.hFileInfo.ioFlFndrInfo.fdFldr=0;
  search2_info.hFileInfo.ioFlParID=0;
  error=PBCatSearchSync((CSParamPtr) &parameter_info);
  DisposePtr(buffer);
  if (parameter_info.csParam.ioReqMatchCount ==
      parameter_info.csParam.ioActMatchCount)
    error=eofErr;
  if (parameter_info.csParam.ioActMatchCount == 0)
    error=0;
  return(error == eofErr);
}

#if !defined(_MAGICKCORE_POSIX_SUPPORT_VERSION)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   s e e k d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  seekdir() sets the position of the next readdir() operation on the directory
%  stream.
%
%  The format of the seekdir method is:
%
%      void seekdir(DIR *entry,ssize_t position)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%    o position: specifies the position associated with the directory
%      stream.
%
%
%
*/
MagickExport void seekdir(DIR *entry,ssize_t position)
{
  assert(entry != (DIR *) NULL);
  entry->d_index=position;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t A p p l i c a t i o n T y p e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetApplicationType() sets the file type of an image.
%
%  The format of the SetApplicationType method is:
%
%      void SetApplicationType(const char *filename,const char *magick,
%        OSType application)
%
%  A description of each parameter follows:
%
%    o filename: Specifies the name of the file.
%
%    o filename: Specifies the file type.
%
%    o application: Specifies the type of the application.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport void SetApplicationType(const char *filename,const char *magick,
  OSType application)
{
  FSSpec
    file_specification;

  OSType
    filetype;

  Str255
    name;

  assert(filename != (char *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(magick != (const char *) NULL);
  filetype='    ';
  (void) CopyMagickString((char *) &filetype,magick,MagickMin(strlen(magick),
    4));
  if (LocaleCompare(magick,"JPG") == 0)
    (void) CopyMagickString((char *) &filetype,"JPEG",MaxTextExtent);
  c2pstrcpy(name,filename);
  FSMakeFSSpec(0,0,name,&file_specification);
  FSpCreate(&file_specification,application,filetype,smSystemScript);
}

#if !defined(_MAGICKCORE_POSIX_SUPPORT_VERSION)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   t e l l d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%   Method telldir returns the current location associated  with  the
%   named directory stream.
%
%  The format of the telldir method is:
%
%      telldir(DIR *entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
MagickExport ssize_t telldir(DIR *entry)
{
  return(entry->d_index);
}
#endif

#endif

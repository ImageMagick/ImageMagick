/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M   M  AA   GGG  III  CCC     W     W  AA  N   N DDD   CCC L    III     %
%     MM MM A  A G      I  C        W     W A  A NN  N D  D C    L     I      %
%     M M M AAAA G  GG  I  C        W  W  W AAAA N N N D  D C    L     I      %
%     M   M A  A G   G  I  C         W W W  A  A N  NN D  D C    L     I      %
%     M   M A  A  GGG  III  CCC       W W   A  A N   N DDD   CCC LLLL III     %
%                                                                             %
%                                                                             %
%                         WandCLI Structure Methods                           %
%                                                                             %
%                              Dragon Computing                               %
%                              Anthony Thyssen                                %
%                                 April 2011                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
% General methds for handling the WandCLI structure used for Command Line.
%
% Anthony Thyssen, April 2011
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/wand.h"
#include "MagickWand/magick-wand-private.h"
#include "MagickWand/wandcli.h"
#include "MagickWand/wandcli-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e W a n d C L I                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagickCLI() creates a new CLI wand (an expanded form of Magick
%  Wand). The given image_info and exception is included as is if provided.
%
%  Use DestroyMagickCLI() to dispose of the CLI wand when it is no longer
%  needed.
%
%  The format of the NewMagickWand method is:
%
%      MagickCLI *AcquireMagickCLI(ImageInfo *image_info,
%           ExceptionInfo *exception)
%
*/
WandExport MagickCLI *AcquireMagickCLI(ImageInfo *image_info,
    ExceptionInfo *exception)
{
  MagickCLI
    *cli_wand;

  /* precaution - as per NewMagickWand() */
  {
     size_t depth = MAGICKCORE_QUANTUM_DEPTH;
     const char *quantum = GetMagickQuantumDepth(&depth);
     if (depth != MAGICKCORE_QUANTUM_DEPTH)
       ThrowWandFatalException(WandError,"QuantumDepthMismatch",quantum);
  }

  /* allocate memory for MgaickCLI */
  cli_wand=(MagickCLI *) AcquireMagickMemory(sizeof(*cli_wand));
  if (cli_wand == (MagickCLI *) NULL)
    {
      ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
        GetExceptionMessage(errno));
      return((MagickCLI *)NULL);
    }

  /* Initialize Wand Part of MagickCLI
     FUTURE: this is a repeat of code from NewMagickWand()
     However some parts may be given fro man external source!
  */
  cli_wand->wand.id=AcquireWandId();
  (void) FormatLocaleString(cli_wand->wand.name,MaxTextExtent,
           "%s-%.20g","MagickWandCLI", (double) cli_wand->wand.id);
  cli_wand->wand.images=NewImageList();
  if ( image_info == (ImageInfo *)NULL)
    cli_wand->wand.image_info=AcquireImageInfo();
  else
    cli_wand->wand.image_info=image_info;
  if ( exception == (ExceptionInfo *)NULL)
    cli_wand->wand.exception=AcquireExceptionInfo();
  else
    cli_wand->wand.exception=exception;
  cli_wand->wand.debug=IsEventLogging();
  cli_wand->wand.signature=WandSignature;

  /* Initialize CLI Part of MagickCLI */
  cli_wand->draw_info=CloneDrawInfo(cli_wand->wand.image_info,(DrawInfo *) NULL);
  cli_wand->quantize_info=AcquireQuantizeInfo(cli_wand->wand.image_info);
  cli_wand->image_list_stack=(Stack *)NULL;
  cli_wand->image_info_stack=(Stack *)NULL;
  cli_wand->location="'%s'";      /* option location not known by default */
  cli_wand->location2="'%s' '%s'";
  cli_wand->filename=cli_wand->wand.name;
  cli_wand->line=0;
  cli_wand->column=0;
  cli_wand->signature=WandSignature;

  if (IfMagickTrue(cli_wand->wand.debug))
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);
  return(cli_wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y W a n d C L I                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMagickCLI() destorys everything in a CLI wand, including image_info
%  and any exceptions, if still present in the wand.
%
%  The format of the NewMagickWand method is:
%
%    MagickWand *DestroyMagickCLI()
%            Exception *exception)
%
*/
WandExport MagickCLI *DestroyMagickCLI(MagickCLI *cli_wand)
{
  Stack
    *node;

  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == WandSignature);
  assert(cli_wand->wand.signature == WandSignature);
  if (IfMagickTrue(cli_wand->wand.debug))
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);

  /* Destroy CLI part of MagickCLI */
  if (cli_wand->draw_info != (DrawInfo *) NULL )
    cli_wand->draw_info=DestroyDrawInfo(cli_wand->draw_info);
  if (cli_wand->quantize_info != (QuantizeInfo *) NULL )
    cli_wand->quantize_info=DestroyQuantizeInfo(cli_wand->quantize_info);
  while(cli_wand->image_list_stack != (Stack *)NULL)
    {
      node=cli_wand->image_list_stack;
      cli_wand->image_list_stack=node->next;
      (void) DestroyImageList((Image *)node->data);
      (void) RelinquishMagickMemory(node);
    }
  while(cli_wand->image_info_stack != (Stack *)NULL)
    {
      node=cli_wand->image_info_stack;
      cli_wand->image_info_stack=node->next;
      (void) DestroyImageInfo((ImageInfo *)node->data);
      (void) RelinquishMagickMemory(node);
    }
  cli_wand->signature=(~WandSignature);

  /* Destroy Wand part MagickCLI */
  cli_wand->wand.images=DestroyImageList(cli_wand->wand.images);
  if (cli_wand->wand.image_info != (ImageInfo *) NULL )
    cli_wand->wand.image_info=DestroyImageInfo(cli_wand->wand.image_info);
  if (cli_wand->wand.exception != (ExceptionInfo *) NULL )
    cli_wand->wand.exception=DestroyExceptionInfo(cli_wand->wand.exception);
  RelinquishWandId(cli_wand->wand.id);
  cli_wand->wand.signature=(~WandSignature);

  return((MagickCLI *)NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C L I C a t c h E x c e p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CLICatchException() will report exceptions, either just non-fatal warnings
%  only, or all errors, according to 'all_execeptions' boolean argument.
%
%  The function returns true is errors are fatal, in which case the caller
%  should abort and re-call with an 'all_exceptions' argument of true before
%  quitting.
%
%  The cut-off level between fatal and non-fatal may be controlled by options
%  (FUTURE), but defaults to 'Error' exceptions.
%
%  The format of the CLICatchException method is:
%
%    MagickBooleanType CLICatchException(MagickCLI *cli_wand,
%              const MagickBooleanType all_exceptions );
%
*/
WandExport MagickBooleanType CLICatchException(MagickCLI *cli_wand,
     const MagickBooleanType all_exceptions )
{
  MagickBooleanType
    status;
  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == WandSignature);
  assert(cli_wand->wand.signature == WandSignature);
  if (IfMagickTrue(cli_wand->wand.debug))
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);

  // FUTURE: '-regard_warning' should make this more sensitive.
  // Note pipelined options may like more control over this level

  status = IsMagickTrue(cli_wand->wand.exception->severity > ErrorException);

  if ( IfMagickFalse(status) || IfMagickTrue(all_exceptions) )
    CatchException(cli_wand->wand.exception); /* output and clear exceptions */

  return(status);
}

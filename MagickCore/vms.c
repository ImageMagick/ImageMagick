/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            V   V  M   M  SSSSS                              %
%                            V   V  MM MM  SS                                 %
%                            V   V  M M M   SSS                               %
%                             V V   M   M     SS                              %
%                              V    M   M  SSSSS                              %
%                                                                             %
%                                                                             %
%                         MagickCore VMS Utility Methods                      %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                October 1994                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
%  by Rich Salz.
%
*/

#if defined(vms)
/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/vms.h"

#if !defined(_AXP_) && (!defined(__VMS_VER) || (__VMS_VER < 70000000))
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
%      void closedir(DIR *directory)
%
%  A description of each parameter follows:
%
%    o directory: Specifies a pointer to a DIR structure.
%
%
*/
void closedir(DIR *directory)
{
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(directory != (DIR *) NULL);
  directory->pattern=(char *) RelinquishMagickMemory(directory->pattern);
  directory=(DIR *) RelinquishMagickMemory(directory);
}

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
%      DIR *opendir(char *name)
%
%  A description of each parameter follows:
%
%    o name: Specifies the name of the directory to open.
%
%
*/
DIR *opendir(char *name)
{
  const char
    *path;

  DIR
    *directory;

  size_t
    extent;

  assert(name != (char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",name);
  /*
    The descriptor length is an unsigned word, so bound the pattern length.
  */
  path=name;
  if (strcmp(".",path) == 0)
    path="";
  extent=strlen(path)+sizeof("*.*");
  if (extent > 65535UL)
    {
      errno=EINVAL;
      return((DIR *) NULL);
    }

  /*
    Allocate memory for handle and the pattern.
  */
  directory=(DIR *) AcquireMagickMemory(sizeof(DIR));
  if (directory == (DIR *) NULL)
    {
      errno=ENOMEM;
      return((DIR *) NULL);
    }
  (void) memset(directory,0,sizeof(*directory));
  directory->pattern=(char *) AcquireQuantumMemory(extent,
    sizeof(*directory->pattern));
  if (directory->pattern == (char *) NULL)
    {
      directory=(DIR *) RelinquishMagickMemory(directory);
      errno=ENOMEM;
      return((DIR *) NULL);
    }
  /*
    Initialize descriptor.
  */
  (void) FormatLocaleString(directory->pattern,extent,"%s*.*",path);
  directory->context=0;
  directory->pat.dsc$a_pointer=directory->pattern;
  directory->pat.dsc$w_length=(unsigned short) strlen(directory->pattern);
  directory->pat.dsc$b_dtype=DSC$K_DTYPE_T;
  directory->pat.dsc$b_class=DSC$K_CLASS_S;
  return(directory);
}

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
%  The format of the readdir method is:
%
%      struct dirent *readdir(DIR *directory)
%
%  A description of each parameter follows:
%
%    o directory: Specifies a pointer to a DIR structure.
%
%
*/
struct dirent *readdir(DIR *directory)
{
  char
    buffer[sizeof(directory->entry.d_name)];

  char
    *p;

  int
    status;

  struct dsc$descriptor_s
    result;

  assert(directory != (DIR *) NULL);
  /*
    Initialize the result descriptor.
  */
  (void) memset(buffer,0,sizeof(buffer));
  result.dsc$a_pointer=buffer;
  result.dsc$w_length=sizeof(buffer)-2;
  result.dsc$b_dtype=DSC$K_DTYPE_T;
  result.dsc$b_class=DSC$K_CLASS_S;
  status=lib$find_file(&directory->pat,&result,&directory->context);
  if (((status & 0x01) == 0) || (directory->context == 0L))
    return((struct dirent *) NULL);
  /*
    The result is blank padded: terminate it at the first blank.
  */
  buffer[sizeof(buffer)-1]='\0';
  p=buffer;
  while ((*p != '\0') && (isspace((int) ((unsigned char) *p)) == 0))
    p++;
  *p='\0';
  /*
    Lowercase all filenames.
  */
  for (p=buffer; *p != '\0'; p++)
    if (isupper((int) ((unsigned char) *p)) != 0)
      *p=(char) LocaleToLowercase((int) ((unsigned char) *p));
  /*
    Skip any directory component and just copy the name.
  */
  p=strchr(buffer,']');
  if (p != (char *) NULL)
    (void) CopyMagickString(directory->entry.d_name,p+1,
      sizeof(directory->entry.d_name));
  else
    (void) CopyMagickString(directory->entry.d_name,buffer,
      sizeof(directory->entry.d_name));
  directory->entry.d_namlen=strlen(directory->entry.d_name);
  return(&directory->entry);
}
#endif /* !defined(_AXP_) && (!defined(__VMS_VER) || (__VMS_VER < 70000000)) */

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
%  VMSIsMagickConflict() returns true if the image format conflicts with a
%  logical drive (.e.g. SYS$SCRATCH:).
%
%  Contributed by Forrest Cahoon (forrest@wiredaemons.com)
%
%  The format of the VMSIsMagickConflict method is:
%
%      MagickBooleanType VMSIsMagickConflict(const char *magick)
%
%  A description of each parameter follows:
%
%    o magick: Specifies the image format.
%
%
*/
MagickExport MagickBooleanType VMSIsMagickConflict(const char *magick)
{
  ile3
    item_list[2];

  int
    device_class = 0,
    status;

  struct dsc$descriptor_s
    device;

  assert(magick != (char *) NULL);
  if (strlen(magick) > 65535UL)
    return(MagickFalse);
  device.dsc$w_length=(unsigned short) strlen(magick);
  device.dsc$a_pointer=(char *) magick;
  device.dsc$b_class=DSC$K_CLASS_S;
  device.dsc$b_dtype=DSC$K_DTYPE_T;
  item_list[0].ile3$w_length=sizeof(device_class);
  item_list[0].ile3$w_code=DVI$_DEVCLASS;
  item_list[0].ile3$ps_bufaddr=&device_class;
  item_list[0].ile3$ps_retlen_addr=NULL;
  (void) memset(&item_list[1],0,sizeof(item_list[1]));
  status=sys$getdviw(0,0,&device,item_list,0,0,0,0);
  if ((status == SS$_NONLOCAL) ||
      ((status & 0x01) && (device_class & (DC$_DISK | DC$_TAPE))))
    return(MagickTrue);
  return(MagickFalse);
}
#endif /* defined(vms) */

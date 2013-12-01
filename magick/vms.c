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
%  by Rich Salz.
%
*/

#if defined(vms)
/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/string_.h"
#include "magick/memory_.h"
#include "magick/vms.h"

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
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
void closedir(DIR *directory)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(directory != (DIR *) NULL);
  directory->pattern=DestroyString(directory->pattern);
  directory=DestroyString(directory);
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
%      opendir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
DIR *opendir(char *name)
{
  DIR
    *directory;

  /*
    Allocate memory for handle and the pattern.
  */
  directory=(DIR *) AcquireMagickMemory(sizeof(DIR));
  if (directory == (DIR *) NULL)
    {
      errno=ENOMEM;
      return((DIR *) NULL);
    }
  if (strcmp(".",name) == 0)
    name="";
  directory->pattern=(char *) AcquireQuantumMemory(strlen(name)+sizeof("*.*")+
    1UL,sizeof(*directory->pattern));
  if (directory->pattern == (char *) NULL)
    {
      directory=DestroyString(directory);
      errno=ENOMEM;
      return(NULL);
    }
  /*
    Initialize descriptor.
  */
  (void) FormatLocaleString(directory->pattern,MaxTextExtent,"%s*.*",name);
  directory->context=0;
  directory->pat.dsc$a_pointer=directory->pattern;
  directory->pat.dsc$w_length=strlen(directory->pattern);
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
%  The format of the readdir
%
%      readdir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
struct dirent *readdir(DIR *directory)
{
  char
    buffer[sizeof(directory->entry.d_name)];

  int
    status;

  register char
    *p;

  register int
    i;

  struct dsc$descriptor_s
    result;

  /*
    Initialize the result descriptor.
  */
  result.dsc$a_pointer=buffer;
  result.dsc$w_length=sizeof(buffer)-2;
  result.dsc$b_dtype=DSC$K_DTYPE_T;
  result.dsc$b_class=DSC$K_CLASS_S;
  status=lib$find_file(&directory->pat,&result,&directory->context);
  if ((status == RMS$_NMF) || (directory->context == 0L))
    return((struct dirent *) NULL);
  /*
    Lowercase all filenames.
  */
  buffer[sizeof(buffer)-1]='\0';
  for (p=buffer; *p; p++)
    if (isupper((int) ((unsigned char) *p)))
      *p=tolower(*p);
  /*
    Skip any directory component and just copy the name.
  */
  p=buffer;
  while (isspace((int) ((unsigned char) *p)) == 0)
    p++;
  *p='\0';
  p=strchr(buffer,']');
  if (p)
    (void) CopyMagickString(directory->entry.d_name,p+1,MaxTextExtent);
  else
    (void) CopyMagickString(directory->entry.d_name,buffer,MaxTextExtent);
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
    device_class,
    status;

  struct dsc$descriptor_s
    device;

  assert(magick != (char *) NULL);
  device.dsc$w_length=strlen(magick);
  device.dsc$a_pointer=(char *) magick;
  device.dsc$b_class=DSC$K_CLASS_S;
  device.dsc$b_dtype=DSC$K_DTYPE_T;
  item_list[0].ile3$w_length=sizeof(device_class);
  item_list[0].ile3$w_code=DVI$_DEVCLASS;
  item_list[0].ile3$ps_bufaddr=&device_class;
  item_list[0].ile3$ps_retlen_addr=NULL;
  (void) ResetMagickMemory(&item_list[1],0,sizeof(item_list[1]));
  status=sys$getdviw(0,0,&device,&item_list,0,0,0,0);
  if ((status == SS$_NONLOCAL) ||
      ((status & 0x01) && (device_class & (DC$_DISK | DC$_TAPE))))
    return(MagickTrue);
  return(MagickFalse);
}
#endif /* defined(vms) */

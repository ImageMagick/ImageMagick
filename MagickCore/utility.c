/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%             U   U  TTTTT  IIIII  L      IIIII  TTTTT  Y   Y                 %
%             U   U    T      I    L        I      T     Y Y                  %
%             U   U    T      I    L        I      T      Y                   %
%             U   U    T      I    L        I      T      Y                   %
%              UUU     T    IIIII  LLLLL  IIIII    T      Y                   %
%                                                                             %
%                                                                             %
%                       MagickCore Utility Methods                            %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                              January 1993                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
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
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/color.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/policy.h"
#include "MagickCore/random_.h"
#include "MagickCore/registry.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/token-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#if defined(MAGICKCORE_HAVE_PROCESS_H)
#include <process.h>
#endif
#if defined(MAGICKCORE_HAVE_MACH_O_DYLD_H)
#include <mach-o/dyld.h>
#endif

/*
  Static declarations.
*/
static const char
  Base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
  Forward declaration.
*/
static int
  IsPathDirectory(const char *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e U n i q u e F i l e n a m e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireUniqueFilename() replaces the contents of path by a unique path name.
%
%  The format of the AcquireUniqueFilename method is:
%
%      MagickBooleanType AcquireUniqueFilename(char *path)
%
%  A description of each parameter follows.
%
%   o  path:  Specifies a pointer to an array of characters.  The unique path
%      name is returned in this array.
%
*/
MagickExport MagickBooleanType AcquireUniqueFilename(char *path)
{
  int
    file;

  file=AcquireUniqueFileResource(path);
  if (file == -1)
    return(MagickFalse);
  file=close_utf8(file)-1;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e U n i q u e S ym b o l i c L i n k                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireUniqueSymbolicLink() creates a unique symbolic link to the specified
%  source path and returns MagickTrue on success otherwise MagickFalse.  If the
%  symlink() method fails or is not available, a unique file name is generated
%  and the source file copied to it.  When you are finished with the file, use
%  RelinquishUniqueFileResource() to destroy it.
%
%  The format of the AcquireUniqueSymbolicLink method is:
%
%      MagickBooleanType AcquireUniqueSymbolicLink(const char *source,
%        char destination)
%
%  A description of each parameter follows.
%
%   o  source:  the source path.
%
%   o  destination:  the destination path.
%
*/

MagickExport MagickBooleanType AcquireUniqueSymbolicLink(const char *source,
  char *destination)
{
  int
    destination_file,
    source_file;

  MagickBooleanType
    status;

  size_t
    length,
    quantum;

  ssize_t
    count;

  struct stat
    attributes;

  unsigned char
    *buffer;

  assert(source != (const char *) NULL);
  assert(destination != (char *) NULL);
#if defined(MAGICKCORE_HAVE_SYMLINK)
  {
    char
      *passes;

    passes=GetPolicyValue("system:shred");
    if (passes != (char *) NULL)
      passes=DestroyString(passes);
    else
      {
        (void) AcquireUniqueFilename(destination);
        (void) RelinquishUniqueFileResource(destination);
        if (*source == *DirectorySeparator)
          {
            if (symlink(source,destination) == 0)
              return(MagickTrue);
          }
        else
          {
            char
              path[MagickPathExtent];

            *path='\0';
            if (getcwd(path,MagickPathExtent) == (char *) NULL)
              return(MagickFalse);
            (void) ConcatenateMagickString(path,DirectorySeparator,
              MagickPathExtent);
            (void) ConcatenateMagickString(path,source,MagickPathExtent);
            if (symlink(path,destination) == 0)
              return(MagickTrue);
          }
      }
  }
#endif
  destination_file=AcquireUniqueFileResource(destination);
  if (destination_file == -1)
    return(MagickFalse);
  source_file=open_utf8(source,O_RDONLY | O_BINARY,0);
  if (source_file == -1)
    {
      (void) close_utf8(destination_file);
      (void) RelinquishUniqueFileResource(destination);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(source_file,&attributes) == 0) && (attributes.st_size > 0))
    quantum=(size_t) MagickMin(attributes.st_size,MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      (void) close_utf8(source_file);
      (void) close_utf8(destination_file);
      (void) RelinquishUniqueFileResource(destination);
      return(MagickFalse);
    }
  status=MagickTrue;
  for (length=0; ; )
  {
    count=(ssize_t) read(source_file,buffer,quantum);
    if (count <= 0)
      break;
    length=(size_t) count;
    count=(ssize_t) write(destination_file,buffer,length);
    if ((size_t) count != length)
      {
        (void) RelinquishUniqueFileResource(destination);
        status=MagickFalse;
        break;
      }
  }
  (void) close_utf8(destination_file);
  (void) close_utf8(source_file);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A p p e n d I m a g e F o r m a t                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AppendImageFormat() appends the image format type to the filename.  If an
%  extension to the file already exists, it is first removed.
%
%  The format of the AppendImageFormat method is:
%
%      void AppendImageFormat(const char *format,char *filename)
%
%  A description of each parameter follows.
%
%   o  format:  Specifies a pointer to an array of characters.  This the
%      format of the image.
%
%   o  filename:  Specifies a pointer to an array of characters.  The unique
%      file name is returned in this array.
%
*/
MagickExport void AppendImageFormat(const char *format,char *filename)
{
  char
    extension[MagickPathExtent],
    root[MagickPathExtent];

  assert(format != (char *) NULL);
  assert(filename != (char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  if ((*format == '\0') || (*filename == '\0'))
    return;
  if (LocaleCompare(filename,"-") == 0)
    {
      char
        message[MagickPathExtent];

      (void) FormatLocaleString(message,MagickPathExtent,"%s:%s",format,
        filename);
      (void) CopyMagickString(filename,message,MagickPathExtent);
      return;
    }
  GetPathComponent(filename,ExtensionPath,extension);
  if ((LocaleCompare(extension,"Z") == 0) ||
      (LocaleCompare(extension,"bz2") == 0) ||
      (LocaleCompare(extension,"gz") == 0) ||
      (LocaleCompare(extension,"wmz") == 0) ||
      (LocaleCompare(extension,"svgz") == 0))
    {
      GetPathComponent(filename,RootPath,root);
      (void) CopyMagickString(filename,root,MagickPathExtent);
      GetPathComponent(filename,RootPath,root);
      (void) FormatLocaleString(filename,MagickPathExtent,"%s.%s.%s",root,
        format,extension);
      return;
    }
  GetPathComponent(filename,RootPath,root);
  (void) FormatLocaleString(filename,MagickPathExtent,"%s.%s",root,format);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B a s e 6 4 D e c o d e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Base64Decode() decodes Base64-encoded text and returns its binary
%  equivalent.  NULL is returned if the text is not valid Base64 data, or a
%  memory allocation failure occurs.
%
%  The format of the Base64Decode method is:
%
%      unsigned char *Base64Decode(const char *source,length_t *length)
%
%  A description of each parameter follows:
%
%    o source:  A pointer to a Base64-encoded string.
%
%    o length: the number of bytes decoded.
%
*/
MagickExport unsigned char *Base64Decode(const char *source,size_t *length)
{
  int
    state;

  const char
    *p,
    *q;

  size_t
    i;

  unsigned char
    *decode;

  assert(source != (char *) NULL);
  assert(length != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  *length=0;
  decode=(unsigned char *) AcquireQuantumMemory((strlen(source)+3)/4,
    3*sizeof(*decode));
  if (decode == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  i=0;
  state=0;
  for (p=source; *p != '\0'; p++)
  {
    if (isspace((int) ((unsigned char) *p)) != 0)
      continue;
    if (*p == '=')
      break;
    q=strchr(Base64,*p);
    if (q == (char *) NULL)
      {
        decode=(unsigned char *) RelinquishMagickMemory(decode);
        return((unsigned char *) NULL);  /* non-Base64 character */
      }
    switch (state)
    {
      case 0:
      {
        decode[i]=(unsigned char)((q-Base64) << 2);
        state++;
        break;
      }
      case 1:
      {
        decode[i++]|=(unsigned char)((q-Base64) >> 4);
        decode[i]=(unsigned char)(((q-Base64) & 0x0f) << 4);
        state++;
        break;
      }
      case 2:
      {
        decode[i++]|=(unsigned char)((q-Base64) >> 2);
        decode[i]=(unsigned char)(((q-Base64) & 0x03) << 6);
        state++;
        break;
      }
      case 3:
      {
        decode[i++]|=(unsigned char)(q-Base64);
        state=0;
        break;
      }
    }
  }
  /*
    Verify Base-64 string has proper terminal characters.
  */
  if (*p != '=')
    {
      if (state != 0)
        {
          decode=(unsigned char *) RelinquishMagickMemory(decode);
          return((unsigned char *) NULL);
        }
    }
  else
    {
      p++;
      switch (state)
      {
        case 0:
        case 1:
        {
          /*
            Unrecognized '=' character.
          */
          decode=(unsigned char *) RelinquishMagickMemory(decode);
          return((unsigned char *) NULL);
        }
        case 2:
        {
          for ( ; *p != '\0'; p++)
            if (isspace((int) ((unsigned char) *p)) == 0)
              break;
          if (*p != '=')
            {
              decode=(unsigned char *) RelinquishMagickMemory(decode);
              return((unsigned char *) NULL);
            }
          p++;
        }
        case 3:
        {
          for ( ; *p != '\0'; p++)
            if (isspace((int) ((unsigned char) *p)) == 0)
              {
                decode=(unsigned char *) RelinquishMagickMemory(decode);
                return((unsigned char *) NULL);
              }
          if ((int) decode[i] != 0)
            {
              decode=(unsigned char *) RelinquishMagickMemory(decode);
              return((unsigned char *) NULL);
            }
          break;
        }
      }
    }
  *length=i;
  return(decode);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B a s e 6 4 E n c o d e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Base64Encode() encodes arbitrary binary data to Base64 encoded format as
%  described by the "Base64 Content-Transfer-Encoding" section of RFC 2045 and
%  returns the result as a null-terminated ASCII string.  NULL is returned if
%  a memory allocation failure occurs.
%
%  The format of the Base64Encode method is:
%
%      char *Base64Encode(const unsigned char *blob,const size_t blob_length,
%        size_t *encode_length)
%
%  A description of each parameter follows:
%
%    o blob:  A pointer to binary data to encode.
%
%    o blob_length: the number of bytes to encode.
%
%    o encode_length:  The number of bytes encoded.
%
*/
MagickExport char *Base64Encode(const unsigned char *blob,
  const size_t blob_length,size_t *encode_length)
{
  char
    *encode;

  const unsigned char
    *p;

  size_t
    i;

  size_t
    remainder;

  assert(blob != (const unsigned char *) NULL);
  assert(blob_length != 0);
  assert(encode_length != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  *encode_length=0;
  encode=(char *) AcquireQuantumMemory(blob_length/3+4,4*sizeof(*encode));
  if (encode == (char *) NULL)
    return((char *) NULL);
  i=0;
  for (p=blob; p < (blob+blob_length-2); p+=(ptrdiff_t) 3)
  {
    encode[i++]=Base64[(int) (*p >> 2)];
    encode[i++]=Base64[(int) (((*p & 0x03) << 4)+(*(p+1) >> 4))];
    encode[i++]=Base64[(int) (((*(p+1) & 0x0f) << 2)+(*(p+2) >> 6))];
    encode[i++]=Base64[(int) (*(p+2) & 0x3f)];
  }
  remainder=blob_length % 3;
  if (remainder != 0)
    {
      ssize_t
        j;

      unsigned char
        code[3];

      code[0]='\0';
      code[1]='\0';
      code[2]='\0';
      for (j=0; j < (ssize_t) remainder; j++)
        code[j]=(*p++);
      encode[i++]=Base64[(int) (code[0] >> 2)];
      encode[i++]=Base64[(int) (((code[0] & 0x03) << 4)+(code[1] >> 4))];
      if (remainder == 1)
        encode[i++]='=';
      else
        encode[i++]=Base64[(int) (((code[1] & 0x0f) << 2)+(code[2] >> 6))];
      encode[i++]='=';
    }
  *encode_length=i;
  encode[i++]='\0';
  return(encode);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C h o p P a t h C o m p o n e n t s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ChopPathComponents() removes the number of specified file components from a
%  path.
%
%  The format of the ChopPathComponents method is:
%
%      ChopPathComponents(char *path,size_t components)
%
%  A description of each parameter follows:
%
%    o path:  The path.
%
%    o components:  The number of components to chop.
%
*/
MagickPrivate void ChopPathComponents(char *path,const size_t components)
{
  ssize_t
    i;

  for (i=0; i < (ssize_t) components; i++)
    GetPathComponent(path,HeadPath,path);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p a n d F i l e n a m e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandFilename() expands '~' in a path.
%
%  The format of the ExpandFilename function is:
%
%      ExpandFilename(char *path)
%
%  A description of each parameter follows:
%
%    o path: Specifies a pointer to a character array that contains the
%      path.
%
*/
MagickPrivate void ExpandFilename(char *path)
{
  char
    expand_path[MagickPathExtent];

  if (path == (char *) NULL)
    return;
  if (*path != '~')
    return;
  (void) CopyMagickString(expand_path,path,MagickPathExtent);
  if ((*(path+1) == *DirectorySeparator) || (*(path+1) == '\0'))
    {
      char
        *home;

      /*
        Substitute ~ with $HOME.
      */
      (void) CopyMagickString(expand_path,".",MagickPathExtent);
      (void) ConcatenateMagickString(expand_path,path+1,MagickPathExtent);
      home=GetEnvironmentValue("HOME");
      if (home == (char *) NULL)
        home=GetEnvironmentValue("USERPROFILE");
      if (home != (char *) NULL)
        {
          (void) CopyMagickString(expand_path,home,MagickPathExtent);
          (void) ConcatenateMagickString(expand_path,path+1,MagickPathExtent);
          home=DestroyString(home);
        }
    }
  else
    {
#if defined(MAGICKCORE_POSIX_SUPPORT) && !defined(__OS2__)
      char
#if defined(MAGICKCORE_HAVE_GETPWNAM_R)
        buffer[MagickPathExtent],
#endif
        username[MagickPathExtent];

      char
        *p;

      struct passwd
        *entry,
        pwd;

      /*
        Substitute ~ with home directory from password file.
      */
      (void) CopyMagickString(username,path+1,MagickPathExtent);
      p=strchr(username,'/');
      if (p != (char *) NULL)
        *p='\0';
#if !defined(MAGICKCORE_HAVE_GETPWNAM_R)
      entry=getpwnam(username);
#else
      entry=(struct passwd *) NULL;
      if (getpwnam_r(username,&pwd,buffer,sizeof(buffer),&entry) < 0)
        return;
#endif
      if (entry == (struct passwd *) NULL)
        return;
      (void) CopyMagickString(expand_path,entry->pw_dir,MagickPathExtent);
      if (p != (char *) NULL)
        {
          (void) ConcatenateMagickString(expand_path,"/",MagickPathExtent);
          (void) ConcatenateMagickString(expand_path,p+1,MagickPathExtent);
        }
#endif
    }
  (void) CopyMagickString(path,expand_path,MagickPathExtent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p a n d F i l e n a m e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandFilenames() checks each argument of the given argument array, and
%  expands it if they have a wildcard character.
%
%  Any coder prefix (EG: 'coder:filename') or read modifier postfix (EG:
%  'filename[...]') are ignored during the file the expansion, but will be
%  included in the final argument.  If no filename matching the meta-character
%  'glob' is found the original argument is returned.
%
%  For example, an argument of '*.gif[20x20]' will be replaced by the list
%    'abc.gif[20x20]',  'foobar.gif[20x20]',  'xyzzy.gif[20x20]'
%  if such filenames exist, (in the current directory in this case).
%
%  Meta-characters handled...
%     @    read a list of filenames (no further expansion performed)
%     ~    At start of filename expands to HOME environment variable
%     *    matches any string including an empty string
%     ?    matches by any single character
%
%  WARNING: filenames starting with '.' (hidden files in a UNIX file system)
%  will never be expanded.  Attempting to expand '.*' will produce no change.
%
%  Expansion is ignored for coders "label:" "caption:" "pango:" and "vid:".
%  Which provide their own '@' meta-character handling.
%
%  You can see the results of the expansion using "Configure" log events.
%
%  The returned list should be freed using  DestroyStringList().
%
%  However the strings in the original pointed to argv are not
%  freed  (TO BE CHECKED).  So a copy of the original pointer (and count)
%  should be kept separate if they need to be freed later.
%
%  The format of the ExpandFilenames function is:
%
%      status=ExpandFilenames(int *number_arguments,char ***arguments)
%
%  A description of each parameter follows:
%
%    o number_arguments: Specifies a pointer to an integer describing the
%      number of elements in the argument vector.
%
%    o arguments: Specifies a pointer to a text array containing the command
%      line arguments.
%
*/
MagickExport MagickBooleanType ExpandFilenames(int *number_arguments,
  char ***arguments)
{
  char
    home_directory[MagickPathExtent],
    **vector;

  ssize_t
    i,
    j;

  size_t
    number_files;

  ssize_t
    count,
    parameters;

  /*
    Allocate argument vector.
  */
  assert(number_arguments != (int *) NULL);
  assert(arguments != (char ***) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  vector=(char **) AcquireQuantumMemory((size_t) (*number_arguments+1),
    sizeof(*vector));
  if (vector == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  /*
    Expand any wildcard filenames.
  */
  *home_directory='\0';
  count=0;
  for (i=0; i < (ssize_t) *number_arguments; i++)
  {
    char
      **filelist,
      filename[MagickPathExtent],
      magick[MagickPathExtent],
      *option,
      path[MagickPathExtent],
      subimage[MagickPathExtent];

    MagickBooleanType
      destroy;

    option=(*arguments)[i];
    *magick='\0';
    *path='\0';
    *filename='\0';
    *subimage='\0';
    number_files=0;
    vector[count++]=ConstantString(option);
    destroy=MagickTrue;
    parameters=ParseCommandOption(MagickCommandOptions,MagickFalse,option);
    if (parameters > 0)
      {
        /*
          Do not expand command option parameters.
        */
        for (j=0; j < parameters; j++)
        {
          i++;
          if (i == (ssize_t) *number_arguments)
            break;
          option=(*arguments)[i];
          vector[count++]=ConstantString(option);
        }
        continue;
      }
    if ((*option == '"') || (*option == '\''))
      continue;
    GetPathComponent(option,TailPath,filename);
    GetPathComponent(option,MagickPath,magick);
    if ((LocaleCompare(magick,"CAPTION") == 0) ||
        (LocaleCompare(magick,"LABEL") == 0) ||
        (LocaleCompare(magick,"PANGO") == 0) ||
        (LocaleCompare(magick,"VID") == 0))
      continue;
    if ((IsGlob(filename) == MagickFalse) && (*option != '@'))
      continue;
    if (IsPathAccessible(option) != MagickFalse)
      continue;
    if (*option != '@')
      {
        /*
          Generate file list from wildcard filename (e.g. *.jpg).
        */
        GetPathComponent(option,HeadPath,path);
        GetPathComponent(option,SubimagePath,subimage);
        ExpandFilename(path);
        if (*home_directory == '\0')
          getcwd_utf8(home_directory,MagickPathExtent-1);
        filelist=ListFiles(*path == '\0' ? home_directory : path,filename,
          &number_files);
      }
    else
      {
        char
          *files;

        ExceptionInfo
          *exception;

        int
          length;

        /*
          Generate file list from file list (e.g. @filelist.txt).
        */
        exception=AcquireExceptionInfo();
        files=FileToString(option,~0UL,exception);
        exception=DestroyExceptionInfo(exception);
        if (files == (char *) NULL)
          continue;
        filelist=StringToArgv(files,&length);
        if (filelist == (char **) NULL)
          continue;
        files=DestroyString(files);
        filelist[0]=DestroyString(filelist[0]);
        for (j=0; j < (ssize_t) (length-1); j++)
          filelist[j]=filelist[j+1];
        number_files=(size_t) length-1;
      }
    if (filelist == (char **) NULL)
      continue;
    for (j=0; j < (ssize_t) number_files; j++)
      if (IsPathDirectory(filelist[j]) <= 0)
        break;
    if (j == (ssize_t) number_files)
      {
        for (j=0; j < (ssize_t) number_files; j++)
          filelist[j]=DestroyString(filelist[j]);
        filelist=(char **) RelinquishMagickMemory(filelist);
        continue;
      }
    /*
      Transfer file list to argument vector.
    */
    vector=(char **) ResizeQuantumMemory(vector,(size_t) ((ssize_t)
      *number_arguments+count+(ssize_t) number_files+1),sizeof(*vector));
    if (vector == (char **) NULL)
      {
        for (j=0; j < (ssize_t) number_files; j++)
          filelist[j]=DestroyString(filelist[j]);
        filelist=(char **) RelinquishMagickMemory(filelist);
        return(MagickFalse);
      }
    for (j=0; j < (ssize_t) number_files; j++)
    {
      option=filelist[j];
      parameters=ParseCommandOption(MagickCommandOptions,MagickFalse,option);
      if (parameters > 0)
        {
          ssize_t
            k;

          /*
            Do not expand command option parameters.
          */
          vector[count++]=ConstantString(option);
          for (k=0; k < parameters; k++)
          {
            j++;
            if (j == (ssize_t) number_files)
              break;
            option=filelist[j];
            vector[count++]=ConstantString(option);
          }
          continue;
        }
      (void) CopyMagickString(filename,path,MagickPathExtent);
      if (*path != '\0')
        (void) ConcatenateMagickString(filename,DirectorySeparator,
          MagickPathExtent);
      if (filelist[j] != (char *) NULL)
        (void) ConcatenateMagickString(filename,filelist[j],MagickPathExtent);
      filelist[j]=DestroyString(filelist[j]);
      if (strlen(filename) >= (MagickPathExtent-1))
        ThrowFatalException(OptionFatalError,"FilenameTruncated");
      if (IsPathDirectory(filename) <= 0)
        {
          char
            file_path[MagickPathExtent];

          *file_path='\0';
          if (*magick != '\0')
            {
              (void) ConcatenateMagickString(file_path,magick,
                MagickPathExtent);
              (void) ConcatenateMagickString(file_path,":",MagickPathExtent);
            }
          (void) ConcatenateMagickString(file_path,filename,MagickPathExtent);
          if (*subimage != '\0')
            {
              (void) ConcatenateMagickString(file_path,"[",MagickPathExtent);
              (void) ConcatenateMagickString(file_path,subimage,
                MagickPathExtent);
              (void) ConcatenateMagickString(file_path,"]",MagickPathExtent);
            }
          if (strlen(file_path) >= (MagickPathExtent-1))
            ThrowFatalException(OptionFatalError,"FilenameTruncated");
          if (destroy != MagickFalse)
            {
              count--;
              vector[count]=DestroyString(vector[count]);
              destroy=MagickFalse;
            }
          vector[count++]=ConstantString(file_path);
        }
    }
    filelist=(char **) RelinquishMagickMemory(filelist);
  }
  vector[count]=(char *) NULL;
  if (IsEventLogging() != MagickFalse)
    {
      char
        *command_line;

      command_line=AcquireString(vector[0]);
      for (i=1; i < count; i++)
      {
        (void) ConcatenateString(&command_line," {");
        (void) ConcatenateString(&command_line,vector[i]);
        (void) ConcatenateString(&command_line,"}");
      }
      (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
        "Command line: %s",command_line);
      command_line=DestroyString(command_line);
    }
  *number_arguments=(int) count;
  *arguments=vector;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E x e c u t i o n P a t h                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetExecutionPath() returns the pathname of the executable that started
%  the process.  On success MagickTrue is returned, otherwise MagickFalse.
%
%  The format of the GetExecutionPath method is:
%
%      MagickBooleanType GetExecutionPath(char *path,const size_t extent)
%
%  A description of each parameter follows:
%
%    o path: the pathname of the executable that started the process.
%
%    o extent: the maximum extent of the path.
%
*/
MagickPrivate MagickBooleanType GetExecutionPath(char *path,const size_t extent)
{
  char
    *directory;

  *path='\0';
  directory=getcwd(path,(unsigned long) extent);
  (void) directory;
#if defined(MAGICKCORE_HAVE_GETPID) && defined(MAGICKCORE_HAVE_READLINK) && defined(PATH_MAX)
  {
    char
      execution_path[PATH_MAX+1],
      link_path[MagickPathExtent];

    ssize_t
      count;

    (void) FormatLocaleString(link_path,MagickPathExtent,"/proc/%.20g/exe",
      (double) getpid());
    count=readlink(link_path,execution_path,PATH_MAX);
    if (count == -1)
      {
        (void) FormatLocaleString(link_path,MagickPathExtent,"/proc/%.20g/file",
          (double) getpid());
        count=readlink(link_path,execution_path,PATH_MAX);
      }
    if ((count > 0) && (count <= (ssize_t) PATH_MAX))
      {
        execution_path[count]='\0';
        (void) CopyMagickString(path,execution_path,extent);
      }
  }
#endif
#if defined(MAGICKCORE_HAVE__NSGETEXECUTABLEPATH)
  {
    char
      executable_path[PATH_MAX << 1],
      execution_path[PATH_MAX+1];

    uint32_t
      length;

    length=sizeof(executable_path);
    if ((_NSGetExecutablePath(executable_path,&length) == 0) &&
        (realpath(executable_path,execution_path) != (char *) NULL))
      (void) CopyMagickString(path,execution_path,extent);
  }
#endif
#if defined(MAGICKCORE_HAVE_GETEXECNAME)
  {
    const char
      *execution_path;

    execution_path=(const char *) getexecname();
    if (execution_path != (const char *) NULL)
      {
        if (*execution_path != *DirectorySeparator)
          (void) ConcatenateMagickString(path,DirectorySeparator,extent);
        (void) ConcatenateMagickString(path,execution_path,extent);
      }
  }
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  NTGetExecutionPath(path,extent);
#endif
#if defined(__GNU__)
  {
    char
      *program_name;

    ssize_t
      count;

    count=0;
    program_name=program_invocation_name;
    if (*program_invocation_name != '/')
      {
        size_t
          extent;

        extent=strlen(directory)+strlen(program_name)+2;
        program_name=AcquireQuantumMemory(extent,sizeof(*program_name));
        if (program_name == (char *) NULL)
          program_name=program_invocation_name;
        else
          count=FormatLocaleString(program_name,extent,"%s/%s",directory,
            program_invocation_name);
      }
    if (count != -1)
      {
        char
          execution_path[PATH_MAX+1];

        if (realpath(program_name,execution_path) != (char *) NULL)
          (void) CopyMagickString(path,execution_path,extent);
      }
    if (program_name != program_invocation_name)
      program_name=(char *) RelinquishMagickMemory(program_name);
  }
#endif
#if defined(__OpenBSD__)
  {
    extern char
      *__progname;

    (void) CopyMagickString(path,__progname,extent);
  }
#endif
  return(IsPathAccessible(path));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k P a g e S i z e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickPageSize() returns the memory page size.
%
%  The format of the GetMagickPageSize method is:
%
%      ssize_t GetMagickPageSize()
%
*/
MagickPrivate ssize_t GetMagickPageSize(void)
{
  static ssize_t
    page_size = -1;

  if (page_size > 0)
    return(page_size);
#if defined(MAGICKCORE_HAVE_SYSCONF) && defined(_SC_PAGE_SIZE)
  page_size=(ssize_t) sysconf(_SC_PAGE_SIZE);
#elif defined(MAGICKCORE_HAVE_GETPAGESIZE)
  page_size=(ssize_t) getpagesize();
#endif
  if (page_size <= 0)
    page_size=4096;
  return(page_size);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P a t h A t t r i b u t e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPathAttributes() returns attributes (e.g. size of file) about a path.
%
%  The path of the GetPathAttributes method is:
%
%      MagickBooleanType GetPathAttributes(const char *path,void *attributes)
%
%  A description of each parameter follows.
%
%   o  path: the file path.
%
%   o  attributes: the path attributes are returned here.
%
*/
MagickExport MagickBooleanType GetPathAttributes(const char *path,
  void *attributes)
{
  MagickBooleanType
    status;

  if (path == (const char *) NULL)
    {
      errno=EINVAL;
      return(MagickFalse);
    }
  (void) memset(attributes,0,sizeof(struct stat));
  status=stat_utf8(path,(struct stat *) attributes) == 0 ? MagickTrue :
    MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P a t h C o m p o n e n t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPathComponent() returns the parent directory name, filename, basename, or
%  extension of a file path.
%
%  The component string pointed to must have at least MagickPathExtent space
%  for the results to be stored.
%
%  The format of the GetPathComponent function is:
%
%      GetPathComponent(const char *path,PathType type,char *component)
%
%  A description of each parameter follows:
%
%    o path: Specifies a pointer to a character array that contains the
%      file path.
%
%    o type: Specifies which file path component to return.
%
%    o component: the selected file path component is returned here.
%
*/
MagickExport void GetPathComponent(const char *path,PathType type,
  char *component)
{
  char
    *q;

  char
    *p;

  size_t
    magick_length,
    subimage_offset,
    subimage_length;

  assert(path != (const char *) NULL);
  assert(component != (char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",path);
  if (*path == '\0')
    {
      *component='\0';
      return;
    }
  (void) CopyMagickString(component,path,MagickPathExtent);
  subimage_length=0;
  subimage_offset=0;
  if (type != SubcanonicalPath)
    {
      p=component+strlen(component)-1;
      if ((strlen(component) > 2) && (*p == ']'))
        {
          q=strrchr(component,'[');
          if ((q != (char *) NULL) && ((q == component) || (*(q-1) != ']')) &&
              (IsPathAccessible(path) == MagickFalse))
            {
              /*
                Look for scene specification (e.g. img0001.pcd[4]).
              */
              *p='\0';
              if ((IsSceneGeometry(q+1,MagickFalse) == MagickFalse) &&
                  (IsGeometry(q+1) == MagickFalse))
                *p=']';
              else
                {
                  subimage_length=(size_t) (p-q);
                  subimage_offset=(size_t) (q-component+1);
                  *q='\0';
                }
            }
        }
    }
  magick_length=0;
#if defined(__OS2__)
  if (path[1] != ":")
#endif
  for (p=component; *p != '\0'; p++)
  {
    if ((*p == '%') && (*(p+1) == '['))
      {
        /*
          Skip over %[...].
        */
        for (p++; (*p != ']') && (*p != '\0'); p++) ;
        if (*p == '\0')
          break;
      }
    if ((p != component) && (*p == ':') && (IsPathDirectory(component) < 0) &&
        (IsPathAccessible(component) == MagickFalse))
      {
        /*
          Look for image format specification (e.g. ps3:image).
        */
        *p='\0';
        if (IsMagickConflict(component) != MagickFalse)
          *p=':';
        else
          {
            magick_length=(size_t) (p-component+1);
            for (q=component; *(++p) != '\0'; q++)
              *q=(*p);
            *q='\0';
          }
        break;
      }
  }
  p=component;
  if (*p != '\0')
    for (p=component+strlen(component)-1; p > component; p--)
      if (IsBasenameSeparator(*p) != MagickFalse)
        break;
  switch (type)
  {
    case MagickPath:
    {
      if (magick_length != 0)
        (void) CopyMagickString(component,path,magick_length);
      else
        *component='\0';
      break;
    }
    case RootPath:
    {
      if (*component != '\0')
        {
          for (p=component+(strlen(component)-1); p > component; p--)
          {
            if (IsBasenameSeparator(*p) != MagickFalse)
              break;
            if (*p == '.')
              break;
          }
          if (*p == '.')
            *p='\0';
          break;
        }
      magick_fallthrough;
    }
    case HeadPath:
    {
      *p='\0';
      break;
    }
    case TailPath:
    {
      if (IsBasenameSeparator(*p) != MagickFalse)
        (void) CopyMagickString(component,p+1,MagickPathExtent);
      break;
    }
    case BasePath:
    {
      if (IsBasenameSeparator(*p) != MagickFalse)
        (void) CopyMagickString(component,p+1,MagickPathExtent);
      if (*component != '\0')
        for (p=component+(strlen(component)-1); p > component; p--)
          if (*p == '.')
            {
              *p='\0';
              break;
            }
      break;
    }
    case BasePathSansCompressExtension:
    {
      char
        extension[MagickPathExtent];

      /*
        Base path sans any compression extension.
      */
      GetPathComponent(path,ExtensionPath,extension);
      if ((LocaleCompare(extension,"bz2") == 0) ||
          (LocaleCompare(extension,"gz") == 0) ||
          (LocaleCompare(extension,"svgz") == 0) ||
          (LocaleCompare(extension,"wmz") == 0) ||
          (LocaleCompare(extension,"Z") == 0))
        GetPathComponent(path,BasePath,component);
      break;
    }
    case ExtensionPath:
    {
      if (IsBasenameSeparator(*p) != MagickFalse)
        (void) CopyMagickString(component,p+1,MagickPathExtent);
      if (*component != '\0')
        for (p=component+strlen(component)-1; p > component; p--)
          if (*p == '.')
            break;
      *component='\0';
      if (*p == '.')
        (void) CopyMagickString(component,p+1,MagickPathExtent);
      break;
    }
    case SubimagePath:
    {
      *component='\0';
      if ((subimage_length != 0) && (magick_length < subimage_offset))
        (void) CopyMagickString(component,path+subimage_offset,subimage_length);
      break;
    }
    case SubcanonicalPath:
    case CanonicalPath:
    case UndefinedPath:
      break;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t P a t h C o m p o n e n t s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPathComponents() returns a list of path components.
%
%  The format of the GetPathComponents method is:
%
%      char **GetPathComponents(const char *path,
%        size_t *number_components)
%
%  A description of each parameter follows:
%
%    o path:  Specifies the string to segment into a list.
%
%    o number_components:  return the number of components in the list
%
*/
MagickPrivate char **GetPathComponents(const char *path,
  size_t *number_components)
{
  char
    **components;

  const char
    *p,
    *q;

  ssize_t
    i;

  if (path == (char *) NULL)
    return((char **) NULL);
  *number_components=1;
  for (p=path; *p != '\0'; p++)
    if (IsBasenameSeparator(*p))
      (*number_components)++;
  components=(char **) AcquireQuantumMemory((size_t) *number_components+1UL,
    sizeof(*components));
  if (components == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  p=path;
  for (i=0; i < (ssize_t) *number_components; i++)
  {
    for (q=p; *q != '\0'; q++)
      if (IsBasenameSeparator(*q))
        break;
    components[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+MagickPathExtent,
      sizeof(**components));
    if (components[i] == (char *) NULL)
      ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
    (void) CopyMagickString(components[i],p,(size_t) (q-p+1));
    p=q+1;
  }
  components[i]=(char *) NULL;
  return(components);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s P a t h A c c e s s i b l e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPathAccessible() returns MagickTrue if the file as defined by the path is
%  accessible.
%
%  The format of the IsPathAccessible method is:
%
%      MagickBooleanType IsPathAccessible(const char *path)
%
%  A description of each parameter follows.
%
%    o path:  Specifies a path to a file.
%
*/
MagickExport MagickBooleanType IsPathAccessible(const char *path)
{
  MagickBooleanType
    status;

  struct stat
    attributes;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(MagickFalse);
  if (LocaleCompare(path,"-") == 0)
    return(MagickTrue);
  status=GetPathAttributes(path,&attributes);
  if (status == MagickFalse)
    return(status);
  if (S_ISREG(attributes.st_mode) == 0)
    return(MagickFalse);
  if (access_utf8(path,F_OK) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  I s P a t h D i r e c t o r y                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPathDirectory() returns -1 if the directory does not exist,  1 is returned
%  if the path represents a directory otherwise 0.
%
%  The format of the IsPathDirectory method is:
%
%      int IsPathDirectory(const char *path)
%
%  A description of each parameter follows.
%
%   o  path:  The directory path.
%
*/
static int IsPathDirectory(const char *path)
{
  MagickBooleanType
    status;

  struct stat
    attributes;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(MagickFalse);
  status=GetPathAttributes(path,&attributes);
  if (status == MagickFalse)
    return(-1);
  if (S_ISDIR(attributes.st_mode) == 0)
    return(0);
  return(1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i s t F i l e s                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListFiles() reads the directory specified and returns a list of filenames
%  contained in the directory sorted in ascending alphabetic order.
%
%  The format of the ListFiles function is:
%
%      char **ListFiles(const char *directory,const char *pattern,
%        ssize_t *number_entries)
%
%  A description of each parameter follows:
%
%    o filelist: Method ListFiles returns a list of filenames contained
%      in the directory.  If the directory specified cannot be read or it is
%      a file a NULL list is returned.
%
%    o directory: Specifies a pointer to a text string containing a directory
%      name.
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_entries:  This integer returns the number of filenames in the
%      list.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int FileCompare(const void *x,const void *y)
{
  const char
    **p,
    **q;

  p=(const char **) x;
  q=(const char **) y;
  return(LocaleCompare(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickPrivate char **ListFiles(const char *directory,const char *pattern,
  size_t *number_entries)
{
  char
    **filelist;

  DIR
    *current_directory;

  struct dirent
    *buffer,
    *entry;

  size_t
    max_entries;

  /*
    Open directory.
  */
  assert(directory != (const char *) NULL);
  assert(pattern != (const char *) NULL);
  assert(number_entries != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",directory);
  *number_entries=0;
  current_directory=opendir(directory);
  if (current_directory == (DIR *) NULL)
    return((char **) NULL);
  /*
    Allocate filelist.
  */
  max_entries=2048;
  filelist=(char **) AcquireQuantumMemory((size_t) max_entries,
    sizeof(*filelist));
  if (filelist == (char **) NULL)
    {
      (void) closedir(current_directory);
      return((char **) NULL);
    }
  /*
    Save the current and change to the new directory.
  */
  buffer=(struct dirent *) AcquireMagickMemory(sizeof(*buffer)+FILENAME_MAX+1);
  if (buffer == (struct dirent *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  while ((MagickReadDirectory(current_directory,buffer,&entry) == 0) &&
         (entry != (struct dirent *) NULL))
  {
    if ((LocaleCompare(entry->d_name,".") == 0) ||
        (LocaleCompare(entry->d_name,"..") == 0))
      continue;
    if ((IsPathDirectory(entry->d_name) > 0) ||
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
        (GlobExpression(entry->d_name,pattern,MagickTrue) != MagickFalse))
#else
        (GlobExpression(entry->d_name,pattern,MagickFalse) != MagickFalse))
#endif
      {
        if (*number_entries >= max_entries)
          {
            /*
              Extend the file list.
            */
            max_entries<<=1;
            filelist=(char **) ResizeQuantumMemory(filelist,(size_t)
              max_entries,sizeof(*filelist));
            if (filelist == (char **) NULL)
              break;
          }
#if defined(vms)
        {
          char
            *p;

          p=strchr(entry->d_name,';');
          if (p)
            *p='\0';
          if (*number_entries > 0)
            if (LocaleCompare(entry->d_name,filelist[*number_entries-1]) == 0)
              continue;
        }
#endif
        filelist[*number_entries]=(char *) AcquireString(entry->d_name);
        (*number_entries)++;
      }
  }
  buffer=(struct dirent *) RelinquishMagickMemory(buffer);
  (void) closedir(current_directory);
  if (filelist == (char **) NULL)
    return((char **) NULL);
  /*
    Sort filelist in ascending order.
  */
  qsort((void *) filelist,(size_t) *number_entries,sizeof(*filelist),
    FileCompare);
  return(filelist);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e l a y                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDelay() suspends program execution for the number of milliseconds
%  specified.
%
%  The format of the Delay method is:
%
%      void MagickDelay(const MagickSizeType milliseconds)
%
%  A description of each parameter follows:
%
%    o milliseconds: Specifies the number of milliseconds to delay before
%      returning.
%
*/
MagickExport void MagickDelay(const MagickSizeType milliseconds)
{
  if (milliseconds == 0)
    return;
#if defined(MAGICKCORE_HAVE_NANOSLEEP)
  {
    struct timespec
      timer;

    timer.tv_sec=(time_t) (milliseconds/1000);
    timer.tv_nsec=(time_t) ((milliseconds % 1000)*1000*1000);
    (void) nanosleep(&timer,(struct timespec *) NULL);
  }
#elif defined(MAGICKCORE_HAVE_USLEEP)
  usleep(1000*milliseconds);
#elif defined(MAGICKCORE_HAVE_SELECT)
  {
    struct timeval
      timer;

    timer.tv_sec=(long) milliseconds/1000;
    timer.tv_usec=(long) (milliseconds % 1000)*1000;
    (void) select(0,(XFD_SET *) NULL,(XFD_SET *) NULL,(XFD_SET *) NULL,&timer);
  }
#elif defined(MAGICKCORE_HAVE_POLL)
  (void) poll((struct pollfd *) NULL,0,(int) milliseconds);
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  Sleep((long) milliseconds);
#elif defined(vms)
  {
    float
      timer;

    timer=milliseconds/1000.0;
    lib$wait(&timer);
  }
#elif defined(__BEOS__)
  snooze(1000*milliseconds);
#else
  {
    clock_t
      time_end;

    time_end=clock()+milliseconds*CLOCKS_PER_SEC/1000;
    while (clock() < time_end)
    {
    }
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M u l t i l i n e C e n s u s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MultilineCensus() returns the number of lines within a label.  A line is
%  represented by a \n character.
%
%  The format of the MultilineCensus method is:
%
%      size_t MultilineCensus(const char *label)
%
%  A description of each parameter follows.
%
%   o  label:  This character string is the label.
%
*/
MagickExport size_t MultilineCensus(const char *label)
{
  size_t
    number_lines;

  /*
    Determine the number of lines within this label.
  */
  if (label == (char *) NULL)
    return(0);
  for (number_lines=1; *label != '\0'; label++)
    if (*label == '\n')
      number_lines++;
  return(number_lines);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S h r e d F i l e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShredFile() overwrites the specified file with random data.  The overwrite is
%  optional and is only required to help keep the contents of the file private.
%
%  The format of the ShredFile method is:
%
%      MagickBooleanType ShredFile(const char *path)
%
%  A description of each parameter follows.
%
%    o path:  Specifies a path to a file.
%
*/
MagickPrivate MagickBooleanType ShredFile(const char *path)
{
  int
    file,
    status;

  MagickSizeType
    length;

  RandomInfo
    *random_info;

  size_t
    quantum;

  ssize_t
    i;

  static ssize_t
    passes = -1;

  StringInfo
    *key;

  struct stat
    file_stats;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(MagickFalse);
  if (passes == -1)
    {
      char
        *property;
          
      passes=0;
      property=GetEnvironmentValue("MAGICK_SHRED_PASSES");
      if (property != (char *) NULL)
        {
          passes=(ssize_t) StringToInteger(property);
          property=DestroyString(property);
        }
      property=GetPolicyValue("system:shred");
      if (property != (char *) NULL)
        {
          passes=(ssize_t) StringToInteger(property);
          property=DestroyString(property);
        }
    }
  if (passes == 0)
    return(MagickTrue);
  /*
    Shred the file.
  */
  file=open_utf8(path,O_WRONLY | O_EXCL | O_BINARY,S_MODE);
  if (file == -1)
    return(MagickFalse);
  quantum=(size_t) MagickMinBufferExtent;
  if ((fstat(file,&file_stats) == 0) && (file_stats.st_size > 0))
    quantum=(size_t) MagickMin(file_stats.st_size,MagickMinBufferExtent);
  length=(MagickSizeType) file_stats.st_size;
  random_info=AcquireRandomInfo();
  key=GetRandomKey(random_info,quantum);
  for (i=0; i < passes; i++)
  {
    MagickOffsetType
      j;

    ssize_t
      count;

    if (lseek(file,0,SEEK_SET) < 0)
      break;
    for (j=0; j < (MagickOffsetType) length; j+=count)
    {
      if (i != 0)
        SetRandomKey(random_info,quantum,GetStringInfoDatum(key));
      count=write(file,GetStringInfoDatum(key),(size_t)
        MagickMin((MagickOffsetType) quantum,(MagickOffsetType) length-j));
      if (count <= 0)
        {
          count=0;
          if (errno != EINTR)
            break;
        }
    }
    if (j < (MagickOffsetType) length)
      break;
  }
  key=DestroyStringInfo(key);
  random_info=DestroyRandomInfo(random_info);
  status=close_utf8(file);
  return((status == -1 || i < passes) ? MagickFalse : MagickTrue);
}

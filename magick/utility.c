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
%                               John Cristy                                   %
%                              January 1993                                   %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/color.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/policy.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
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
  file=close(file)-1;
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
%  RelinquishUniqueFilename() to destroy it.
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

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport MagickBooleanType AcquireUniqueSymbolicLink(const char *source,
  char *destination)
{
  int
    destination_file,
    source_file;

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
        path[MaxTextExtent];

      *path='\0';
      if (getcwd(path,MaxTextExtent) == (char *) NULL)
        return(MagickFalse);
      (void) ConcatenateMagickString(path,DirectorySeparator,MaxTextExtent);
      (void) ConcatenateMagickString(path,source,MaxTextExtent);
      if (symlink(path,destination) == 0)
        return(MagickTrue);
    }
#endif
  destination_file=AcquireUniqueFileResource(destination);
  if (destination_file == -1)
    return(MagickFalse);
  source_file=open(source,O_RDONLY | O_BINARY);
  if (source_file == -1)
    {
      (void) close(destination_file);
      (void) RelinquishUniqueFileResource(destination);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(source_file,&attributes) == 0) && (attributes.st_size != 0))
    quantum=MagickMin((size_t) attributes.st_size,MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      (void) close(source_file);
      (void) close(destination_file);
      (void) RelinquishUniqueFileResource(destination);
      return(MagickFalse);
    }
  for (length=0; ; )
  {
    count=(ssize_t) read(source_file,buffer,quantum);
    if (count <= 0)
      break;
    length=(size_t) count;
    count=(ssize_t) write(destination_file,buffer,length);
    if ((size_t) count != length)
      {
        (void) close(destination_file);
        (void) close(source_file);
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
        (void) RelinquishUniqueFileResource(destination);
        return(MagickFalse);
      }
  }
  (void) close(destination_file);
  (void) close(source_file);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  return(MagickTrue);
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
    root[MaxTextExtent];

  assert(format != (char *) NULL);
  assert(filename != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  if ((*format == '\0') || (*filename == '\0'))
    return;
  if (LocaleCompare(filename,"-") == 0)
    {
      char
        message[MaxTextExtent];

      (void) FormatMagickString(message,MaxTextExtent,"%s:%s",format,filename);
      (void) CopyMagickString(filename,message,MaxTextExtent);
      return;
    }
  GetPathComponent(filename,RootPath,root);
  (void) FormatMagickString(filename,MaxTextExtent,"%s.%s",root,format);
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

  register const char
    *p,
    *q;

  register size_t
    i;

  unsigned char
    *decode;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(source != (char *) NULL);
  assert(length != (size_t *) NULL);
  *length=0;
  decode=(unsigned char *) AcquireQuantumMemory(strlen(source)/4+4,
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
        decode[i]=(q-Base64) << 2;
        state++;
        break;
      }
      case 1:
      {
        decode[i++]|=(q-Base64) >> 4;
        decode[i]=((q-Base64) & 0x0f) << 4;
        state++;
        break;
      }
      case 2:
      {
        decode[i++]|=(q-Base64) >> 2;
        decode[i]=((q-Base64) & 0x03) << 6;
        state++;
        break;
      }
      case 3:
      {
        decode[i++]|=(q-Base64);
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

  register const unsigned char
    *p;

  register size_t
    i;

  size_t
    remainder;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(blob != (const unsigned char *) NULL);
  assert(blob_length != 0);
  assert(encode_length != (size_t *) NULL);
  *encode_length=0;
  encode=(char *) AcquireQuantumMemory(blob_length/3+4,4*sizeof(*encode));
  if (encode == (char *) NULL)
    return((char *) NULL);
  i=0;
  for (p=blob; p < (blob+blob_length-2); p+=3)
  {
    encode[i++]=Base64[(int) (*p >> 2)];
    encode[i++]=Base64[(int) (((*p & 0x03) << 4)+(*(p+1) >> 4))];
    encode[i++]=Base64[(int) (((*(p+1) & 0x0f) << 2)+(*(p+2) >> 6))];
    encode[i++]=Base64[(int) (*(p+2) & 0x3f)];
  }
  remainder=blob_length % 3;
  if (remainder != 0)
    {
      long
        j;

      unsigned char
        code[3];

      code[0]='\0';
      code[1]='\0';
      code[2]='\0';
      for (j=0; j < (long) remainder; j++)
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
%      ChopPathComponents(char *path,unsigned long components)
%
%  A description of each parameter follows:
%
%    o path:  The path.
%
%    o components:  The number of components to chop.
%
*/
MagickExport void ChopPathComponents(char *path,const unsigned long components)
{
  register long
    i;

  for (i=0; i < (long) components; i++)
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
MagickExport void ExpandFilename(char *path)
{
  char
    expand_path[MaxTextExtent];

  if (path == (char *) NULL)
    return;
  if (*path != '~')
    return;
  (void) CopyMagickString(expand_path,path,MaxTextExtent);
  if ((*(path+1) == *DirectorySeparator) || (*(path+1) == '\0'))
    {
      char
        *home;

      /*
        Substitute ~ with $HOME.
      */
      (void) CopyMagickString(expand_path,".",MaxTextExtent);
      (void) ConcatenateMagickString(expand_path,path+1,MaxTextExtent);
      home=GetEnvironmentValue("HOME");
      if (home == (char *) NULL)
        home=GetEnvironmentValue("USERPROFILE");
      if (home != (char *) NULL)
        {
          (void) CopyMagickString(expand_path,home,MaxTextExtent);
          (void) ConcatenateMagickString(expand_path,path+1,MaxTextExtent);
          home=DestroyString(home);
        }
    }
  else
    {
#if defined(MAGICKCORE_POSIX_SUPPORT) && !defined(__OS2__)
      char
        username[MaxTextExtent];

      register char
        *p;

      struct passwd
        *entry;

      /*
        Substitute ~ with home directory from password file.
      */
      (void) CopyMagickString(username,path+1,MaxTextExtent);
      p=strchr(username,'/');
      if (p != (char *) NULL)
        *p='\0';
      entry=getpwnam(username);
      if (entry == (struct passwd *) NULL)
        return;
      (void) CopyMagickString(expand_path,entry->pw_dir,MaxTextExtent);
      if (p != (char *) NULL)
        {
          (void) ConcatenateMagickString(expand_path,"/",MaxTextExtent);
          (void) ConcatenateMagickString(expand_path,p+1,MaxTextExtent);
        }
#endif
    }
  (void) CopyMagickString(path,expand_path,MaxTextExtent);
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
%  ExpandFilenames() checks each argument of the command line vector and
%  expands it if they have a wildcard character.  For example, *.jpg might
%  expand to:  bird.jpg rose.jpg tiki.jpg.
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
    *cwd,
    home_directory[MaxTextExtent],
    **vector;

  long
    count,
    parameters;

  register long
    i,
    j;

  unsigned long
    number_files;

  /*
    Allocate argument vector.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(number_arguments != (int *) NULL);
  assert(arguments != (char ***) NULL);
  vector=(char **) AcquireQuantumMemory((size_t) (*number_arguments+1),
    sizeof(*vector));
  if (vector == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  /*
    Expand any wildcard filenames.
  */
  *home_directory='\0';
  cwd=getcwd(home_directory,MaxTextExtent);
  count=0;
  for (i=0; i < (long) *number_arguments; i++)
  {
    char
      **filelist,
      filename[MaxTextExtent],
      magick[MaxTextExtent],
      *option,
      path[MaxTextExtent],
      subimage[MaxTextExtent];

    MagickBooleanType
      destroy;

    option=(*arguments)[i];
    *magick='\0';
    *path='\0';
    *filename='\0';
    *subimage='\0';
    vector[count++]=ConstantString(option);
    destroy=MagickTrue;
    parameters=ParseMagickOption(MagickCommandOptions,MagickFalse,option);
    if (parameters > 0)
      {
        /*
          Do not expand command option parameters.
        */
        for (j=0; j < parameters; j++)
        {
          i++;
          if (i == (long) *number_arguments)
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
        (LocaleCompare(magick,"VID") == 0))
      continue;
    if ((IsGlob(filename) == MagickFalse) && (*filename != '@'))
      continue;
    if (*filename != '@')
      {
        /*
          Generate file list from wildcard filename (e.g. *.jpg).
        */
        GetPathComponent(option,HeadPath,path);
        GetPathComponent(option,SubimagePath,subimage);
        ExpandFilename(path);
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
          number_images;

        /*
          Generate file list from file list (e.g. @filelist.txt).
        */
        exception=AcquireExceptionInfo();
        files=FileToString(filename+1,~0,exception);
        exception=DestroyExceptionInfo(exception);
        if (files == (char *) NULL)
          continue;
        StripString(files);
        filelist=StringToArgv(files,&number_images);
        files=DestroyString(files);
        number_files=(unsigned long) number_images;
        if (filelist != (char **) NULL)
          {
            number_files--;
            for (j=0; j < (long) number_files; j++)
              filelist[j]=filelist[j+1];
          }
      }
    if (filelist == (char **) NULL)
      continue;
    for (j=0; j < (long) number_files; j++)
      if (IsPathDirectory(filelist[j]) <= 0)
        break;
    if (j == (long) number_files)
      {
        for (j=0; j < (long) number_files; j++)
          filelist[j]=DestroyString(filelist[j]);
        filelist=(char **) RelinquishMagickMemory(filelist);
        continue;
      }
    /*
      Transfer file list to argument vector.
    */
    vector=(char **) ResizeQuantumMemory(vector,(size_t) *number_arguments+
      count+number_files+1,sizeof(*vector));
    if (vector == (char **) NULL)
      return(MagickFalse);
    for (j=0; j < (long) number_files; j++)
    {
      (void) CopyMagickString(filename,path,MaxTextExtent);
      if (*path != '\0')
        (void) ConcatenateMagickString(filename,DirectorySeparator,
          MaxTextExtent);
      (void) ConcatenateMagickString(filename,filelist[j],MaxTextExtent);
      filelist[j]=DestroyString(filelist[j]);
      if (strlen(filename) >= MaxTextExtent)
        ThrowFatalException(OptionFatalError,"FilenameTruncated");
      if (IsPathDirectory(filename) == 0)
        {
          char
            path[MaxTextExtent];

          *path='\0';
          if (*magick != '\0')
            {
              (void) ConcatenateMagickString(path,magick,MaxTextExtent);
              (void) ConcatenateMagickString(path,":",MaxTextExtent);
            }
          (void) ConcatenateMagickString(path,filename,MaxTextExtent);
          if (*subimage != '\0')
            {
              (void) ConcatenateMagickString(path,"[",MaxTextExtent);
              (void) ConcatenateMagickString(path,subimage,MaxTextExtent);
              (void) ConcatenateMagickString(path,"]",MaxTextExtent);
            }
          if (strlen(path) >= MaxTextExtent)
            ThrowFatalException(OptionFatalError,"FilenameTruncated");
          if (destroy != MagickFalse)
            {
              count--;
              vector[count]=DestroyString(vector[count]);
              destroy=MagickFalse;
            }
          vector[count++]=ConstantString(path);
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
MagickExport MagickBooleanType GetExecutionPath(char *path,const size_t extent)
{
  char
    *cwd;

  *path='\0';
  cwd=getcwd(path,(unsigned long) extent);
#if defined(MAGICKCORE_HAVE_GETPID) && defined(MAGICKCORE_HAVE_READLINK)
  {
    char
      link_path[MaxTextExtent],
      real_path[PATH_MAX+1];

    ssize_t
      count;

    (void) FormatMagickString(link_path,MaxTextExtent,"/proc/%ld/exe",
      (long) getpid());
    count=readlink(link_path,real_path,PATH_MAX);
    if (count == -1)
      {
        (void) FormatMagickString(link_path,MaxTextExtent,"/proc/%ld/file",
          (long) getpid());
        count=readlink(link_path,real_path,PATH_MAX);
      }
    if ((count > 0) && (count <= (ssize_t) PATH_MAX))
      {
        real_path[count]='\0';
        (void) CopyMagickString(path,real_path,extent);
      }
  }
#endif
#if defined(MAGICKCORE_HAVE__NSGETEXECUTABLEPATH)
  {
    char
      executable_path[PATH_MAX << 1],
      real_path[PATH_MAX+1];

    uint32_t
      length;

    length=sizeof(executable_path);
    if ((_NSGetExecutablePath(executable_path,&length) == 0) &&
        (realpath(executable_path,real_path) != (char *) NULL))
      (void) CopyMagickString(path,real_path,extent);
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
#if defined(__WINDOWS__)
  NTGetExecutionPath(path,extent);
#endif
  return(IsPathAccessible(path));
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

#if defined(MAGICKCORE_HAVE__WFOPEN)
static size_t UTF8ToUTF16(const unsigned char *utf8,wchar_t *utf16)
{
  register const unsigned char
    *p;

  if (utf16 != (wchar_t *) NULL)
    {
      register wchar_t
        *q;

      wchar_t
        c;

      /*
        Convert UTF-8 to UTF-16.
      */
      q=utf16;
      for (p=utf8; *p != '\0'; p++)
      {
        if ((*p & 0x80) == 0)
          *q=(*p);
        else
          if ((*p & 0xE0) == 0xC0)
            {
              c=(*p);
              *q=(c & 0x1F) << 6;
              p++;
              if ((*p & 0xC0) != 0x80)
                return(0);
              *q|=(*p & 0x3F);
            }
          else
            if ((*p & 0xF0) == 0xE0)
              {
                c=(*p);
                *q=c << 12;
                p++;
                if ((*p & 0xC0) != 0x80)
                  return(0);
                c=(*p);
                *q|=(c & 0x3F) << 6;
                p++;
                if ((*p & 0xC0) != 0x80)
                  return(0);
                *q|=(*p & 0x3F);
              }
            else
              return(0);
        q++;
      }
      *q++='\0';
      return(q-utf16);
    }
  /*
    Compute UTF-16 string length.
  */
  for (p=utf8; *p != '\0'; p++)
  {
    if ((*p & 0x80) == 0)
      ;
    else
      if ((*p & 0xE0) == 0xC0)
        {
          p++;
          if ((*p & 0xC0) != 0x80)
            return(0);
        }
      else
        if ((*p & 0xF0) == 0xE0)
          {
            p++;
            if ((*p & 0xC0) != 0x80)
              return(0);
            p++;
            if ((*p & 0xC0) != 0x80)
              return(0);
         }
       else
         return(0);
  }
  return(p-utf8);
}

static wchar_t *ConvertUTF8ToUTF16(const unsigned char *source)
{
  size_t
    length;

  wchar_t
    *utf16;

  length=UTF8ToUTF16(source,(wchar_t *) NULL);
  if (length == 0)
    {
      register long
        i;

      /*
        Not UTF-8, just copy.
      */
      length=strlen(source);
      utf16=(wchar_t *) AcquireQuantumMemory(length+1,sizeof(*utf16));
      if (utf16 == (wchar_t *) NULL)
        return((wchar_t *) NULL);
      for (i=0; i <= (long) length; i++)
        utf16[i]=source[i];
      return(utf16);
    }
  utf16=(wchar_t *) AcquireQuantumMemory(length+1,sizeof(*utf16));
  if (utf16 == (wchar_t *) NULL)
    return((wchar_t *) NULL);
  length=UTF8ToUTF16(source,utf16);
  return(utf16);
}
#endif

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
#if !defined(MAGICKCORE_HAVE__WSTAT)
  status=stat(path,(struct stat *) attributes) == 0 ? MagickTrue : MagickFalse;
#else
  {
    wchar_t
      *unicode_path;

    unicode_path=ConvertUTF8ToUTF16(path);
    if (unicode_path == (wchar_t *) NULL)
      return(MagickFalse);
    status=wstat(unicode_path,(struct stat *) attributes) == 0 ? MagickTrue :
      MagickFalse;
    unicode_path=(wchar_t *) RelinquishMagickMemory(unicode_path);
  }
#endif
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
%  The format of the GetPathComponent function is:
%
%      GetPathComponent(const char *path,PathType type,char *component)
%
%  A description of each parameter follows:
%
%    o path: Specifies a pointer to a character array that contains the
%      file path.
%
%    o type: Specififies which file path component to return.
%
%    o component: the selected file path component is returned here.
%
*/
MagickExport void GetPathComponent(const char *path,PathType type,
  char *component)
{
  char
    magick[MaxTextExtent],
    *q,
    subimage[MaxTextExtent];

  register char
    *p;

  assert(path != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",path);
  assert(component != (char *) NULL);
  if (*path == '\0')
    {
      *component='\0';
      return;
    }
  (void) CopyMagickString(component,path,MaxTextExtent);
  *magick='\0';
#if defined(__OS2__)
  if (path[1] != ":")
#endif
  for (p=component; *p != '\0'; p++)
    if ((*p == ':') && (IsPathDirectory(path) < 0) &&
        (IsPathAccessible(path) == MagickFalse))
      {
        /*
          Look for image format specification (e.g. ps3:image).
        */
        (void) CopyMagickString(magick,component,(size_t) (p-component+1));
        if (IsMagickConflict(magick) != MagickFalse)
          *magick='\0';
        else
          for (q=component; *q != '\0'; q++)
            *q=(*++p);
        break;
      }
  *subimage='\0';
  p=component;
  if (*p != '\0')
    p=component+strlen(component)-1;
  if ((*p == ']') && (strchr(component,'[') != (char *) NULL) &&
      (IsPathAccessible(path) == MagickFalse))
    {
      /*
        Look for scene specification (e.g. img0001.pcd[4]).
      */
      for (q=p-1; q > component; q--)
        if (*q == '[')
          break;
      if (*q == '[')
        {
          (void) CopyMagickString(subimage,q+1,MaxTextExtent);
          subimage[p-q-1]='\0';
          if ((IsSceneGeometry(subimage,MagickFalse) == MagickFalse) &&
              (IsGeometry(subimage) == MagickFalse))
            *subimage='\0';
          else
            *q='\0';
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
      (void) CopyMagickString(component,magick,MaxTextExtent);
      break;
    }
    case RootPath:
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
    case HeadPath:
    {
      *p='\0';
      break;
    }
    case TailPath:
    {
      if (IsBasenameSeparator(*p) != MagickFalse)
        (void) CopyMagickMemory((unsigned char *) component,
          (const unsigned char *) (p+1),strlen(p+1)+1);
      break;
    }
    case BasePath:
    {
      if (IsBasenameSeparator(*p) != MagickFalse)
        (void) CopyMagickString(component,p+1,MaxTextExtent);
      for (p=component+(strlen(component)-1); p > component; p--)
        if (*p == '.')
          {
            *p='\0';
            break;
          }
      break;
    }
    case ExtensionPath:
    {
      if (IsBasenameSeparator(*p) != MagickFalse)
        (void) CopyMagickString(component,p+1,MaxTextExtent);
      p=component;
      if (*p != '\0')
        for (p=component+strlen(component)-1; p > component; p--)
          if (*p == '.')
            break;
      *component='\0';
      if (*p == '.')
        (void) CopyMagickString(component,p+1,MaxTextExtent);
      break;
    }
    case SubimagePath:
    {
      (void) CopyMagickString(component,subimage,MaxTextExtent);
      break;
    }
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
%        unsigned long *number_componenets)
%
%  A description of each parameter follows:
%
%    o path:  Specifies the string to segment into a list.
%
%    o number_components:  return the number of components in the list
%
*/
MagickExport char **GetPathComponents(const char *path,
  unsigned long *number_components)
{
  char
    **components;

  register const char
    *p,
    *q;

  register long
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
  for (i=0; i < (long) *number_components; i++)
  {
    for (q=p; *q != '\0'; q++)
      if (IsBasenameSeparator(*q))
        break;
    components[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+MaxTextExtent,
      sizeof(*components));
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
%      MagickBooleanType IsPathAccessible(const char *filename)
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
  status=GetPathAttributes(path,&attributes);
  if (status == MagickFalse)
    return(status);
  if (S_ISREG(attributes.st_mode) == 0)
    return(MagickFalse);
  if (access(path,F_OK) != 0)
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
%   I s M a g i c k T r u e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickTrue() returns MagickTrue if the value is "true", "on", "yes" or
%  "1".
%
%  The format of the IsMagickTrue method is:
%
%      MagickBooleanType IsMagickTrue(const char *value)
%
%  A description of each parameter follows:
%
%    o option: either MagickTrue or MagickFalse depending on the value
%      parameter.
%
%    o value: Specifies a pointer to a character array.
%
*/
MagickExport MagickBooleanType IsMagickTrue(const char *value)
{
  if (value == (const char *) NULL)
    return(MagickFalse);
  if (LocaleCompare(value,"true") == 0)
    return(MagickTrue);
  if (LocaleCompare(value,"on") == 0)
    return(MagickTrue);
  if (LocaleCompare(value,"yes") == 0)
    return(MagickTrue);
  if (LocaleCompare(value,"1") == 0)
    return(MagickTrue);
  return(MagickFalse);
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
%        long *number_entries)
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

static inline int MagickReadDirectory(DIR *directory,struct dirent *entry,
  struct dirent **result)
{
#if defined(MAGICKCORE_HAVE_READDIR_R)
  return(readdir_r(directory,entry,result));
#else
  (void) entry;
  errno=0;
  *result=readdir(directory);
  return(errno);
#endif
}

MagickExport char **ListFiles(const char *directory,const char *pattern,
  unsigned long *number_entries)
{
  char
    **filelist;

  DIR
    *current_directory;

  struct dirent
    *buffer,
    *entry;

  unsigned long
    max_entries;

  /*
    Open directory.
  */
  assert(directory != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",directory);
  assert(pattern != (const char *) NULL);
  assert(number_entries != (unsigned long *) NULL);
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
    if (*entry->d_name == '.')
      continue;
    if ((IsPathDirectory(entry->d_name) > 0) ||
#if defined(__WINDOWS__)
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
          register char
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
        if (IsPathDirectory(entry->d_name) > 0)
          (void) ConcatenateMagickString(filelist[*number_entries],
            DirectorySeparator,MaxTextExtent);
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
%  M u l t i l i n e C e n s u s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MultilineCensus() returns the number of lines within a label.  A line is
%  represented by a \n character.
%
%  The format of the MultilineCenus method is:
%
%      unsigned long MultilineCensus(const char *label)
%
%  A description of each parameter follows.
%
%   o  label:  This character string is the label.
%
%
*/
MagickExport unsigned long MultilineCensus(const char *label)
{
  unsigned long
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
%   O p e n M a g i c k S t r e a m                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenMagickStream() opens the file at the specified path and return the
%  associated stream.
%
%  The path of the OpenMagickStream method is:
%
%      FILE *OpenMagickStream(const char *path,const char *mode)
%
%  A description of each parameter follows.
%
%   o  path: the file path.
%
%   o  mode: the file mode.
%
*/
MagickExport FILE *OpenMagickStream(const char *path,const char *mode)
{
  FILE
    *file;

  if ((path == (const char *) NULL) || (mode == (const char *) NULL))
    {
      errno=EINVAL;
      return((FILE *) NULL);
    }
  file=(FILE *) NULL;
#if defined(MAGICKCORE_HAVE__WFOPEN)
  {
    wchar_t
      *unicode_mode,
      *unicode_path;

    unicode_path=ConvertUTF8ToUTF16(path);
    if (unicode_path == (wchar_t *) NULL)
      return((FILE *) NULL);
    unicode_mode=ConvertUTF8ToUTF16(mode);
    if (unicode_mode == (wchar_t *) NULL)
      {
        unicode_path=(wchar_t *) RelinquishMagickMemory(unicode_path);
        return((FILE *) NULL);
      }
    file=_wfopen(unicode_path,unicode_mode);
    unicode_mode=(wchar_t *) RelinquishMagickMemory(unicode_mode);
    unicode_path=(wchar_t *) RelinquishMagickMemory(unicode_path);
  }
#endif
  if (file == (FILE *) NULL)
    file=fopen(path,mode);
  return(file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y s t e m C o m m a n d                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SystemCommand() executes the specified command and waits until it
%  terminates.  The returned value is the exit status of the command.
%
%  The format of the SystemCommand method is:
%
%      int SystemCommand(const MagickBooleanType verbose,const char *command,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o verbose: a value other than 0 prints the executed command before it is
%      invoked.
%
%    o command: this string is the command to execute.
%
%    o exception: return any errors here.
%
*/
MagickExport int SystemCommand(const MagickBooleanType verbose,
  const char *command,ExceptionInfo *exception)
{
  char
    **arguments;

  int
    number_arguments,
    status;

  PolicyDomain
    domain;

  PolicyRights
    rights;

  register long
    i;

  status=(-1);
  arguments=StringToArgv(command,&number_arguments);
  if (arguments == (char **) NULL)
    return(status);
  domain=DelegatePolicyDomain;
  rights=ExecutePolicyRights;
  if (IsRightsAuthorized(domain,rights,arguments[1]) == MagickFalse)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",arguments[1]);
      for (i=0; i < number_arguments; i++)
        arguments[i]=DestroyString(arguments[i]);
      arguments=(char **) RelinquishMagickMemory(arguments);
      return(-1);
    }
  if (verbose != MagickFalse)
    {
      (void) fprintf(stderr,"%s\n",command);
      (void) fflush(stderr);
    }
#if defined(MAGICKCORE_POSIX_SUPPORT)
#if !defined(MAGICKCORE_HAVE_EXECVP)
  status=system(command);
#else
  if (strspn(command,"&;<>|") == 0)
    status=system(command);
  else
    {
      pid_t
        child_pid;

      /*
        Call application directly rather than from a shell.
      */
      child_pid=fork();
      if (child_pid == (pid_t) -1)
        status=system(command);
      else
        if (child_pid == 0)
          {
            status=execvp(arguments[1],arguments+1);
            _exit(1);
          }
        else
          {
            int
              child_status;

            pid_t
              pid;

            child_status=0;
            pid=waitpid(child_pid,&child_status,0);
            if (pid == -1)
              status=(-1);
            else
              {
                if (WIFEXITED(child_status) != 0)
                  status=WEXITSTATUS(child_status);
                else
                  if (WIFSIGNALED(child_status))
                    status=(-1);
              }
          }
    }
#endif
#elif defined(__WINDOWS__)
  status=NTSystemCommand(command);
#elif defined(macintosh)
  status=MACSystemCommand(command);
#elif defined(vms)
  status=system(command);
#else
#  error No suitable system() method.
#endif
  if (status < 0)
    {
      char
        *message;

      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
        "`%s': %s",command,message);
      message=DestroyString(message);
    }
  for (i=0; i < number_arguments; i++)
    arguments[i]=DestroyString(arguments[i]);
  arguments=(char **) RelinquishMagickMemory(arguments);
  return(status);
}

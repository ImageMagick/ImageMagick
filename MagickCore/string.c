/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  SSSSS   TTTTT  RRRR   IIIII  N   N   GGGG                  %
%                  SS        T    R   R    I    NN  N  G                      %
%                   SSS      T    RRRR     I    N N N  G GGG                  %
%                     SS     T    R R      I    N  NN  G   G                  %
%                  SSSSS     T    R  R   IIIII  N   N   GGGG                  %
%                                                                             %
%                                                                             %
%                        MagickCore String Methods                            %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               August 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the license.  You may  %
%  obtain a copy of the license at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
%                                                                             %
%  unless required by applicable law or agreed to in writing, software        %
%  distributed under the license is distributed on an "as is" basis,          %
%  without warranties or conditions of any kind, either express or implied.   %
%  See the license for the specific language governing permissions and        %
%  limitations under the license.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility-private.h"

/*
  static declarations.
*/
#ifdef __VMS
#define asciimap AsciiMap
#endif
#if !defined(MAGICKCORE_HAVE_STRCASECMP) || !defined(MAGICKCORE_HAVE_STRNCASECMP)
static const unsigned char
  AsciiMap[] =
  {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
    0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
    0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xe1, 0xe2, 0xe3, 0xe4, 0xc5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb,
    0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,
    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff,
  };
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S t r i n g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireString() returns an new extented string, containing a clone of the
%  given string.
%
%  An extended string is the string length, plus an extra MagickPathExtent space
%  to allow for the string to be actively worked on.
%
%  The returned string shoud be freed using DestoryString().
%
%  The format of the AcquireString method is:
%
%      char *AcquireString(const char *source)
%
%  A description of each parameter follows:
%
%    o source: A character string.
%
*/
MagickExport char *AcquireString(const char *source)
{
  char
    *destination;

  size_t
    length;

  length=0;
  if (source != (char *) NULL)
    length+=strlen(source);
  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  destination=(char *) AcquireQuantumMemory(length+MagickPathExtent,
    sizeof(*destination));
  if (destination == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  *destination='\0';
  if (source != (char *) NULL)
    (void) memcpy(destination,source,length*sizeof(*destination));
  destination[length]='\0';
  return(destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S t r i n g I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireStringInfo() allocates the StringInfo structure.
%
%  The format of the AcquireStringInfo method is:
%
%      StringInfo *AcquireStringInfo(const size_t length)
%
%  A description of each parameter follows:
%
%    o length: the string length.
%
*/

static StringInfo *AcquireStringInfoContainer()
{
  StringInfo
    *string_info;

  string_info=(StringInfo *) AcquireCriticalMemory(sizeof(*string_info));
  (void) ResetMagickMemory(string_info,0,sizeof(*string_info));
  string_info->signature=MagickCoreSignature;
  return(string_info);
}

MagickExport StringInfo *AcquireStringInfo(const size_t length)
{
  StringInfo
    *string_info;

  string_info=AcquireStringInfoContainer();
  string_info->length=length;
  if (~string_info->length >= (MagickPathExtent-1))
    string_info->datum=(unsigned char *) AcquireQuantumMemory(
      string_info->length+MagickPathExtent,sizeof(*string_info->datum));
  if (string_info->datum == (unsigned char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(string_info->datum,0,(length+MagickPathExtent)*
    sizeof(*string_info->datum));
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B l o b T o S t r i n g I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlobToStringInfo() returns the contents of a blob as a StringInfo structure
%  with MagickPathExtent extra space.
%
%  The format of the BlobToStringInfo method is:
%
%      StringInfo *BlobToStringInfo(const void *blob,const size_t length)
%
%  A description of each parameter follows:
%
%    o blob: the blob.
%
%    o length: the length of the blob.
%
*/
MagickExport StringInfo *BlobToStringInfo(const void *blob,const size_t length)
{
  StringInfo
    *string_info;

  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  string_info=AcquireStringInfoContainer();
  string_info->length=length;
  string_info->datum=(unsigned char *) AcquireQuantumMemory(length+
    MagickPathExtent,sizeof(*string_info->datum));
  if (string_info->datum == (unsigned char *) NULL)
    {
      string_info=DestroyStringInfo(string_info);
      return((StringInfo *) NULL);
    }
  if (blob != (const void *) NULL)
    (void) memcpy(string_info->datum,blob,length);
  (void) memset(string_info->datum+length,0,MagickPathExtent*
    sizeof(*string_info->datum));
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e S t r i n g                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneString() replaces or frees the destination string to make it
%  a clone of the input string plus MagickPathExtent more space so the string
%  may be worked on.
%
%  If source is a NULL pointer the destination string will be freed and set to
%  a NULL pointer.  A pointer to the stored in the destination is also returned.
%
%  When finished the non-NULL string should be freed using DestoryString()
%  or using CloneString() with a NULL pointed for the source.
%
%  The format of the CloneString method is:
%
%      char *CloneString(char **destination,const char *source)
%
%  A description of each parameter follows:
%
%    o destination:  A pointer to a character string.
%
%    o source: A character string.
%
*/
MagickExport char *CloneString(char **destination,const char *source)
{
  size_t
    length;

  assert(destination != (char **) NULL);
  if (source == (const char *) NULL)
    {
      if (*destination != (char *) NULL)
        *destination=DestroyString(*destination);
      return(*destination);
    }
  if (*destination == (char *) NULL)
    {
      *destination=AcquireString(source);
      return(*destination);
    }
  length=strlen(source);
  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  *destination=(char *) ResizeQuantumMemory(*destination,length+
    MagickPathExtent,sizeof(**destination));
  if (*destination == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  if (length != 0)
    (void) memcpy(*destination,source,length*sizeof(**destination));
  (*destination)[length]='\0';
  return(*destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneStringInfo() clones a copy of the StringInfo structure.
%
%  The format of the CloneStringInfo method is:
%
%      StringInfo *CloneStringInfo(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport StringInfo *CloneStringInfo(const StringInfo *string_info)
{
  StringInfo
    *clone_info;

  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  clone_info=AcquireStringInfo(string_info->length);
  if (string_info->length != 0)
    (void) memcpy(clone_info->datum,string_info->datum,string_info->length+1);
  return(clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e S t r i n g I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareStringInfo() compares the two datums target and source.  It returns
%  an integer less than, equal to, or greater than zero if target is found,
%  respectively, to be less than, to match, or be greater than source.
%
%  The format of the CompareStringInfo method is:
%
%      int CompareStringInfo(const StringInfo *target,const StringInfo *source)
%
%  A description of each parameter follows:
%
%    o target: the target string.
%
%    o source: the source string.
%
*/

MagickExport int CompareStringInfo(const StringInfo *target,
  const StringInfo *source)
{
  int
    status;

  assert(target != (StringInfo *) NULL);
  assert(target->signature == MagickCoreSignature);
  assert(source != (StringInfo *) NULL);
  assert(source->signature == MagickCoreSignature);
  status=memcmp(target->datum,source->datum,MagickMin(target->length,
    source->length));
  if (status != 0)
    return(status);
  if (target->length == source->length)
    return(0);
  return(target->length < source->length ? -1 : 1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n c a t e n a t e M a g i c k S t r i n g                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConcatenateMagickString() concatenates the source string to the destination
%  string.  The destination buffer is always null-terminated even if the
%  string must be truncated.
%
%  The format of the ConcatenateMagickString method is:
%
%      size_t ConcatenateMagickString(char *destination,const char *source,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o destination: the destination string.
%
%    o source: the source string.
%
%    o length: the length of the destination string.
%
*/
MagickExport size_t ConcatenateMagickString(char *destination,
  const char *source,const size_t length)
{
  register char
    *q;

  register const char
    *p;

  register size_t
    i;

  size_t
    count;

  assert(destination != (char *) NULL);
  assert(source != (const char *) NULL);
  assert(length >= 1);
  p=source;
  q=destination;
  i=length;
  while ((i-- != 0) && (*q != '\0'))
    q++;
  count=(size_t) (q-destination);
  i=length-count;
  if (i == 0)
    return(count+strlen(p));
  while (*p != '\0')
  {
    if (i != 1)
      {
        *q++=(*p);
        i--;
      }
    p++;
  }
  *q='\0';
  return(count+(p-source));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n c a t e n a t e S t r i n g                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConcatenateString() appends a copy of string source, including the
%  terminating null character, to the end of string destination.
%
%  The format of the ConcatenateString method is:
%
%      MagickBooleanType ConcatenateString(char **destination,
%        const char *source)
%
%  A description of each parameter follows:
%
%    o destination:  A pointer to a character string.
%
%    o source: A character string.
%
*/
MagickExport MagickBooleanType ConcatenateString(char **destination,
  const char *source)
{
  size_t
    destination_length,
    length,
    source_length;

  assert(destination != (char **) NULL);
  if (source == (const char *) NULL)
    return(MagickTrue);
  if (*destination == (char *) NULL)
    {
      *destination=AcquireString(source);
      return(MagickTrue);
    }
  destination_length=strlen(*destination);
  source_length=strlen(source);
  length=destination_length;
  if (~length < source_length)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConcatenateString");
  length+=source_length;
  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConcatenateString");
  *destination=(char *) ResizeQuantumMemory(*destination,length+
    MagickPathExtent,sizeof(**destination));
  if (*destination == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConcatenateString");
  if (source_length != 0)
    (void) memcpy((*destination)+destination_length,source,source_length);
  (*destination)[length]='\0';
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n c a t e n a t e S t r i n g I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConcatenateStringInfo() concatenates the source string to the destination
%  string.
%
%  The format of the ConcatenateStringInfo method is:
%
%      void ConcatenateStringInfo(StringInfo *string_info,
%        const StringInfo *source)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
%    o source: the source string.
%
*/
MagickExport void ConcatenateStringInfo(StringInfo *string_info,
  const StringInfo *source)
{
  size_t
    length;

  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  assert(source != (const StringInfo *) NULL);
  length=string_info->length;
  if (~length < source->length)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConcatenateString");
  SetStringInfoLength(string_info,length+source->length);
  (void) memcpy(string_info->datum+length,source->datum,source->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n f i g u r e F i l e T o S t r i n g I n f o                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConfigureFileToStringInfo() returns the contents of a configure file as a
%  string.
%
%  The format of the ConfigureFileToStringInfo method is:
%
%      StringInfo *ConfigureFileToStringInfo(const char *filename)
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the filename.
%
*/
MagickExport StringInfo *ConfigureFileToStringInfo(const char *filename)
{
  char
    *string;

  int
    file;

  MagickOffsetType
    offset;

  size_t
    length;

  StringInfo
    *string_info;

  void
    *map;

  assert(filename != (const char *) NULL);
  file=open_utf8(filename,O_RDONLY | O_BINARY,0);
  if (file == -1)
    return((StringInfo *) NULL);
  offset=(MagickOffsetType) lseek(file,0,SEEK_END);
  if ((offset < 0) || (offset != (MagickOffsetType) ((ssize_t) offset)))
    {
      file=close(file)-1;
      return((StringInfo *) NULL);
    }
  length=(size_t) offset;
  string=(char *) NULL;
  if (~length >= (MagickPathExtent-1))
    string=(char *) AcquireQuantumMemory(length+MagickPathExtent,
      sizeof(*string));
  if (string == (char *) NULL)
    {
      file=close(file)-1;
      return((StringInfo *) NULL);
    }
  map=MapBlob(file,ReadMode,0,length);
  if (map != (void *) NULL)
    {
      (void) memcpy(string,map,length);
      (void) UnmapBlob(map,length);
    }
  else
    {
      register size_t
        i;

      ssize_t
        count;

      (void) lseek(file,0,SEEK_SET);
      for (i=0; i < length; i+=count)
      {
        count=read(file,string+i,(size_t) MagickMin(length-i,(size_t)
          SSIZE_MAX));
        if (count <= 0)
          {
            count=0;
            if (errno != EINTR)
              break;
          }
      }
      if (i < length)
        {
          file=close(file)-1;
          string=DestroyString(string);
          return((StringInfo *) NULL);
        }
    }
  string[length]='\0';
  file=close(file)-1;
  string_info=AcquireStringInfoContainer();
  string_info->path=ConstantString(filename);
  string_info->length=length;
  string_info->datum=(unsigned char *) string;
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n s t a n t S t r i n g                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConstantString() allocates exactly the needed memory for a string and
%  copies the source string to that memory location.  A NULL string pointer
%  will allocate an empty string containing just the NUL character.
%
%  When finished the string should be freed using DestoryString()
%
%  The format of the ConstantString method is:
%
%      char *ConstantString(const char *source)
%
%  A description of each parameter follows:
%
%    o source: A character string.
%
*/
MagickExport char *ConstantString(const char *source)
{
  char
    *destination;

  size_t
    length;

  length=0;
  if (source != (char *) NULL)
    length+=strlen(source);
  destination=(char *) NULL;
  if (~length >= 1UL)
    destination=(char *) AcquireQuantumMemory(length+1UL,sizeof(*destination));
  if (destination == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  *destination='\0';
  if (source != (char *) NULL)
    (void) memcpy(destination,source,length*sizeof(*destination));
  destination[length]='\0';
  return(destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o p y M a g i c k S t r i n g                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CopyMagickString() copies the source string to the destination string, with
%  out exceeding the given pre-declared length.
%
%  The destination buffer is always null-terminated even if the string must be
%  truncated.  The return value is the minimum of the source string length or
%  the length parameter.
%
%  The format of the CopyMagickString method is:
%
%      size_t CopyMagickString(const char *destination,char *source,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o destination: the destination string.
%
%    o source: the source string.
%
%    o length: the length of the destination string.
%
*/
MagickExport size_t CopyMagickString(char *destination,const char *source,
  const size_t length)
{
  register char
    *q;

  register const char
    *p;

  register size_t
    n;

  p=source;
  q=destination;
  for (n=length; n > 4; n-=4)
  {
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
  }
  if (n != 0)
    for (n--; n != 0; n--)
    {
      *q=(*p++);
      if (*q == '\0')
        return((size_t) (p-source-1));
      q++;
    }
  if (length != 0)
    *q='\0';
  return((size_t) (p-source-1));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S t r i n g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyString() destroys memory associated with a string.
%
%  The format of the DestroyString method is:
%
%      char *DestroyString(char *string)
%
%  A description of each parameter follows:
%
%    o string: the string.
%
*/
MagickExport char *DestroyString(char *string)
{
  return((char *) RelinquishMagickMemory(string));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S t r i n g I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyStringInfo() destroys memory associated with the StringInfo structure.
%
%  The format of the DestroyStringInfo method is:
%
%      StringInfo *DestroyStringInfo(StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport StringInfo *DestroyStringInfo(StringInfo *string_info)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  if (string_info->datum != (unsigned char *) NULL)
    string_info->datum=(unsigned char *) RelinquishMagickMemory(
      string_info->datum);
  if (string_info->name != (char *) NULL)
    string_info->name=DestroyString(string_info->name);
  if (string_info->path != (char *) NULL)
    string_info->path=DestroyString(string_info->path);
  string_info->signature=(~MagickCoreSignature);
  string_info=(StringInfo *) RelinquishMagickMemory(string_info);
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S t r i n g L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyStringList() zeros memory associated with a string list.
%
%  The format of the DestroyStringList method is:
%
%      char **DestroyStringList(char **list)
%
%  A description of each parameter follows:
%
%    o list: the string list.
%
*/
MagickExport char **DestroyStringList(char **list)
{
  register ssize_t
    i;

  assert(list != (char **) NULL);
  for (i=0; list[i] != (char *) NULL; i++)
    list[i]=DestroyString(list[i]);
  list=(char **) RelinquishMagickMemory(list);
  return(list);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E s c a p e S t r i n g                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EscapeString() allocates memory for a backslash-escaped version of a
%  source text string, copies the escaped version of the text to that
%  memory location while adding backslash characters, and returns the
%  escaped string.
%
%  The format of the EscapeString method is:
%
%      char *EscapeString(const char *source,const char escape)
%
%  A description of each parameter follows:
%
%    o allocate_string:  Method EscapeString returns the escaped string.
%
%    o source: A character string.
%
%    o escape: the quoted string termination character to escape (e.g. '"').
%
*/
MagickExport char *EscapeString(const char *source,const char escape)
{
  char
    *destination;

  register char
    *q;

  register const char
    *p;

  size_t
    length;

  assert(source != (const char *) NULL);
  length=0;
  for (p=source; *p != '\0'; p++)
  {
    if ((*p == '\\') || (*p == escape))
      {
        if (~length < 1)
          ThrowFatalException(ResourceLimitFatalError,"UnableToEscapeString");
        length++;
      }
    length++;
  }
  destination=(char *) NULL;
  if (~length >= (MagickPathExtent-1))
    destination=(char *) AcquireQuantumMemory(length+MagickPathExtent,
      sizeof(*destination));
  if (destination == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToEscapeString");
  *destination='\0';
  q=destination;
  for (p=source; *p != '\0'; p++)
  {
    if ((*p == '\\') || (*p == escape))
      *q++='\\';
    *q++=(*p);
  }
  *q='\0';
  return(destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e T o S t r i n g                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FileToString() returns the contents of a file as a string.
%
%  The format of the FileToString method is:
%
%      char *FileToString(const char *filename,const size_t extent,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the filename.
%
%    o extent: Maximum length of the string.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport char *FileToString(const char *filename,const size_t extent,
  ExceptionInfo *exception)
{
  size_t
    length;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  return((char *) FileToBlob(filename,extent,&length,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e T o S t r i n g I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FileToStringInfo() returns the contents of a file as a string.
%
%  The format of the FileToStringInfo method is:
%
%      StringInfo *FileToStringInfo(const char *filename,const size_t extent,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the filename.
%
%    o extent: Maximum length of the string.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport StringInfo *FileToStringInfo(const char *filename,
  const size_t extent,ExceptionInfo *exception)
{
  StringInfo
    *string_info;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  string_info=AcquireStringInfoContainer();
  string_info->path=ConstantString(filename);
  string_info->datum=(unsigned char *) FileToBlob(filename,extent,
    &string_info->length,exception);
  if (string_info->datum == (unsigned char *) NULL)
    {
      string_info=DestroyStringInfo(string_info);
      return((StringInfo *) NULL);
    }
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t M a g i c k S i z e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatMagickSize() converts a size to a human readable format, for example,
%  14k, 234m, 2.7g, or 3.0t.  Scaling is done by repetitively dividing by
%  1000.
%
%  The format of the FormatMagickSize method is:
%
%      ssize_t FormatMagickSize(const MagickSizeType size,const char *suffix,
%        const size_t length,char *format)
%
%  A description of each parameter follows:
%
%    o size:  convert this size to a human readable format.
%
%    o bi:  use power of two rather than power of ten.
%
%    o suffix:  append suffix, typically B or P.
%
%    o length: the maximum length of the string.
%
%    o format:  human readable format.
%
*/
MagickExport ssize_t FormatMagickSize(const MagickSizeType size,
  const MagickBooleanType bi,const char *suffix,const size_t length,
  char *format)
{
  char
    p[MagickPathExtent],
    q[MagickPathExtent];

  const char
    **units;

  double
    bytes,
    extent;

  register ssize_t
    i;

  ssize_t
    count;

  static const char
    *bi_units[] =
    {
      "", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi", (char *) NULL
    },
    *traditional_units[] =
    {
      "", "K", "M", "G", "T", "P", "E", "Z", "Y", (char *) NULL
    };

  bytes=1000.0;
  units=traditional_units;
  if (bi != MagickFalse)
    {
      bytes=1024.0;
      units=bi_units;
    }
#if defined(_MSC_VER) && (_MSC_VER == 1200)
  extent=(double) ((MagickOffsetType) size);
#else
  extent=(double) size;
#endif
  (void) FormatLocaleString(p,MagickPathExtent,"%.*g",GetMagickPrecision(),
    extent);
  (void) FormatLocaleString(q,MagickPathExtent,"%.20g",extent);
  if (strtod(p,(char **) NULL) == strtod(q,(char **) NULL))
    {
      if (suffix == (const char *) NULL)
        count=FormatLocaleString(format,length,"%.20g%s",extent,units[0]);
      else
        count=FormatLocaleString(format,length,"%.20g%s%s",extent,units[0],
          suffix);
      return(count);
    }
  for (i=0; (extent >= bytes) && (units[i+1] != (const char *) NULL); i++)
    extent/=bytes;
  if (suffix == (const char *) NULL)
    count=FormatLocaleString(format,length,"%.*g%s",GetMagickPrecision(),
      extent,units[i]);
  else
    count=FormatLocaleString(format,length,"%.*g%s%s",GetMagickPrecision(),
      extent,units[i],suffix);
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t M a g i c k T i m e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatMagickTime() returns the specified time in the Internet date/time
%  format and the length of the timestamp.
%
%  The format of the FormatMagickTime method is:
%
%      ssize_t FormatMagickTime(const time_t time,const size_t length,
%        char *timestamp)
%
%  A description of each parameter follows.
%
%    o time:  the time since the Epoch (00:00:00 UTC, January 1, 1970),
%      measured in seconds.
%
%    o length: the maximum length of the string.
%
%    o timestamp:  Return the Internet date/time here.
%
*/
MagickExport ssize_t FormatMagickTime(const time_t time,const size_t length,
  char *timestamp)
{
  ssize_t
    count;

  struct tm
    gm_time,
    local_time;

  time_t
    timezone;

  assert(timestamp != (char *) NULL);
  (void) ResetMagickMemory(&local_time,0,sizeof(local_time));
  (void) ResetMagickMemory(&gm_time,0,sizeof(gm_time));
#if defined(MAGICKCORE_HAVE_LOCALTIME_R)
  (void) localtime_r(&time,&local_time);
#else
  {
    struct tm
      *my_time;

    my_time=localtime(&time);
    if (my_time != (struct tm *) NULL)
      (void) memcpy(&local_time,my_time,sizeof(local_time));
  }
#endif
#if defined(MAGICKCORE_HAVE_GMTIME_R)
  (void) gmtime_r(&time,&gm_time);
#else
  {
    struct tm
      *my_time;

    my_time=gmtime(&time);
    if (my_time != (struct tm *) NULL)
      (void) memcpy(&gm_time,my_time,sizeof(gm_time));
  }
#endif
  timezone=(time_t) ((local_time.tm_min-gm_time.tm_min)/60+
    local_time.tm_hour-gm_time.tm_hour+24*((local_time.tm_year-
    gm_time.tm_year) != 0 ? (local_time.tm_year-gm_time.tm_year) :
    (local_time.tm_yday-gm_time.tm_yday)));
  count=FormatLocaleString(timestamp,length,
    "%04d-%02d-%02dT%02d:%02d:%02d%+03ld:00",local_time.tm_year+1900,
    local_time.tm_mon+1,local_time.tm_mday,local_time.tm_hour,
    local_time.tm_min,local_time.tm_sec,(long) timezone);
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E n v i r o n m e n t V a l u e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetEnvironmentValue() returns the environment string that matches the
%  specified name.
%
%  The format of the GetEnvironmentValue method is:
%
%      char *GetEnvironmentValue(const char *name)
%
%  A description of each parameter follows:
%
%    o name: the environment name.
%
*/
MagickExport char *GetEnvironmentValue(const char *name)
{
  const char
    *environment;

  environment=getenv(name);
  if (environment == (const char *) NULL)
    return((char *) NULL);
  return(ConstantString(environment));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o D a t u m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoDatum() returns the datum associated with the string.
%
%  The format of the GetStringInfoDatum method is:
%
%      unsigned char *GetStringInfoDatum(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport unsigned char *GetStringInfoDatum(const StringInfo *string_info)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  return(string_info->datum);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o L e n g t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoLength() returns the string length.
%
%  The format of the GetStringInfoLength method is:
%
%      size_t GetStringInfoLength(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport size_t GetStringInfoLength(const StringInfo *string_info)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  return(string_info->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o N a m e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoName() returns the name associated with the string.
%
%  The format of the GetStringInfoName method is:
%
%      const char *GetStringInfoName(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport const char *GetStringInfoName(const StringInfo *string_info)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  return(string_info->name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o P a t h                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoPath() returns the path associated with the string.
%
%  The format of the GetStringInfoPath method is:
%
%      const char *GetStringInfoPath(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport const char *GetStringInfoPath(const StringInfo *string_info)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  return(string_info->path);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n t e r p r e t S i P r e f i x V a l u e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretSiPrefixValue() converts the initial portion of the string to a
%  double representation.  It also recognizes SI prefixes (e.g. B, KB, MiB,
%  etc.).
%
%  The format of the InterpretSiPrefixValue method is:
%
%      double InterpretSiPrefixValue(const char *value,char **sentinal)
%
%  A description of each parameter follows:
%
%    o value: the string value.
%
%    o sentinal:  if sentinal is not NULL, return a pointer to the character
%      after the last character used in the conversion.
%
*/
MagickExport double InterpretSiPrefixValue(const char *magick_restrict string,
  char **magick_restrict sentinal)
{
  char
    *q;

  double
    value;

  value=InterpretLocaleValue(string,&q);
  if (q != string)
    {
      if ((*q >= 'E') && (*q <= 'z'))
        {
          double
            e;

          switch ((int) ((unsigned char) *q))
          {
            case 'y': e=(-24.0); break;
            case 'z': e=(-21.0); break;
            case 'a': e=(-18.0); break;
            case 'f': e=(-15.0); break;
            case 'p': e=(-12.0); break;
            case 'n': e=(-9.0); break;
            case 'u': e=(-6.0); break;
            case 'm': e=(-3.0); break;
            case 'c': e=(-2.0); break;
            case 'd': e=(-1.0); break;
            case 'h': e=2.0; break;
            case 'k': e=3.0; break;
            case 'K': e=3.0; break;
            case 'M': e=6.0; break;
            case 'G': e=9.0; break;
            case 'T': e=12.0; break;
            case 'P': e=15.0; break;
            case 'E': e=18.0; break;
            case 'Z': e=21.0; break;
            case 'Y': e=24.0; break;
            default: e=0.0; break;
          }
          if (e >= MagickEpsilon)
            {
              if (q[1] == 'i')
                {
                  value*=pow(2.0,e/0.3);
                  q+=2;
                }
              else
                {
                  value*=pow(10.0,e);
                  q++;
                }
            }
        }
      if ((*q == 'B') || (*q == 'P'))
        q++;
    }
  if (sentinal != (char **) NULL)
    *sentinal=q;
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S t r i n g T r u e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsStringTrue() returns MagickTrue if the value is "true", "on", "yes" or
%  "1". Any other string or undefined returns MagickFalse.
%
%  Typically this is used to look at strings (options or artifacts) which
%  has a default value of "false", when not defined.
%
%  The format of the IsStringTrue method is:
%
%      MagickBooleanType IsStringTrue(const char *value)
%
%  A description of each parameter follows:
%
%    o value: Specifies a pointer to a character array.
%
*/
MagickExport MagickBooleanType IsStringTrue(const char *value)
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
%   I s S t r i n g F a l s e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsStringFalse() returns MagickTrue if the value is "false", "off", "no" or
%  "0". Any other string or undefined returns MagickFalse.
%
%  Typically this is used to look at strings (options or artifacts) which
%  has a default value of "true", when it has not been defined.
%
%  The format of the IsStringFalse method is:
%
%      MagickBooleanType IsStringFalse(const char *value)
%
%  A description of each parameter follows:
%
%    o value: Specifies a pointer to a character array.
%
*/
MagickExport MagickBooleanType IsStringFalse(const char *value)
{
  if (value == (const char *) NULL)
    return(MagickFalse);
  if (LocaleCompare(value,"false") == 0)
    return(MagickTrue);
  if (LocaleCompare(value,"off") == 0)
    return(MagickTrue);
  if (LocaleCompare(value,"no") == 0)
    return(MagickTrue);
  if (LocaleCompare(value,"0") == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r i n t S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PrintStringInfo() prints the string.
%
%  The format of the PrintStringInfo method is:
%
%      void PrintStringInfo(FILE *file,const char *id,
%        const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o file: the file, typically stdout.
%
%    o id: the string id.
%
%    o string_info: the string info.
%
*/
MagickExport void PrintStringInfo(FILE *file,const char *id,
  const StringInfo *string_info)
{
  register const char
    *p;

  register size_t
    i,
    j;

  assert(id != (const char *) NULL);
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  p=(char *) string_info->datum;
  for (i=0; i < string_info->length; i++)
  {
    if (((int) ((unsigned char) *p) < 32) &&
        (isspace((int) ((unsigned char) *p)) == 0))
      break;
    p++;
  }
  (void) FormatLocaleFile(file,"%s(%.20g):\n",id,(double) string_info->length);
  if (i == string_info->length)
    {
      for (i=0; i < string_info->length; i++)
        (void) fputc(string_info->datum[i],file);
      (void) fputc('\n',file);
      return;
    }
  /*
    Convert string to a HEX list.
  */
  p=(char *) string_info->datum;
  for (i=0; i < string_info->length; i+=0x14)
  {
    (void) FormatLocaleFile(file,"0x%08lx: ",(unsigned long) (0x14*i));
    for (j=1; j <= MagickMin(string_info->length-i,0x14); j++)
    {
      (void) FormatLocaleFile(file,"%02lx",(unsigned long) (*(p+j)) & 0xff);
      if ((j % 0x04) == 0)
        (void) fputc(' ',file);
    }
    for ( ; j <= 0x14; j++)
    {
      (void) fputc(' ',file);
      (void) fputc(' ',file);
      if ((j % 0x04) == 0)
        (void) fputc(' ',file);
    }
    (void) fputc(' ',file);
    for (j=1; j <= MagickMin(string_info->length-i,0x14); j++)
    {
      if (isprint((int) ((unsigned char) *p)) != 0)
        (void) fputc(*p,file);
      else
        (void) fputc('-',file);
      p++;
    }
    (void) fputc('\n',file);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetStringInfo() reset the string to all null bytes.
%
%  The format of the ResetStringInfo method is:
%
%      void ResetStringInfo(StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport void ResetStringInfo(StringInfo *string_info)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  (void) ResetMagickMemory(string_info->datum,0,string_info->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S a n t i z e S t r i n g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SanitizeString() returns an new string removes all characters except
%  letters, digits and !#$%&'*+-=?^_`{|}~@.[].
%
%  The returned string shoud be freed using DestoryString().
%
%  The format of the SanitizeString method is:
%
%      char *SanitizeString(const char *source)
%
%  A description of each parameter follows:
%
%    o source: A character string.
%
*/
MagickExport char *SanitizeString(const char *source)
{
  char
    *sanitize_source;

  const char
    *q;

  register char
    *p;

  static char
    whitelist[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "
      "$-_.+!*'(),{}|\\^~[]`\"><#%;/?:@&=";

  sanitize_source=AcquireString(source);
  p=sanitize_source;
  q=sanitize_source+strlen(sanitize_source);
  for (p+=strspn(p,whitelist); p != q; p+=strspn(p,whitelist))
    *p='_';
  return(sanitize_source);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfo() copies the source string to the destination string.
%
%  The format of the SetStringInfo method is:
%
%      void SetStringInfo(StringInfo *string_info,const StringInfo *source)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
%    o source: the source string.
%
*/
MagickExport void SetStringInfo(StringInfo *string_info,
  const StringInfo *source)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  assert(source != (StringInfo *) NULL);
  assert(source->signature == MagickCoreSignature);
  if (string_info->length == 0)
    return;
  (void) ResetMagickMemory(string_info->datum,0,string_info->length);
  (void) memcpy(string_info->datum,source->datum,MagickMin(string_info->length,
    source->length));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o D a t u m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfoDatum() copies bytes from the source string for the length of
%  the destination string.
%
%  The format of the SetStringInfoDatum method is:
%
%      void SetStringInfoDatum(StringInfo *string_info,
%        const unsigned char *source)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
%    o source: the source string.
%
*/
MagickExport void SetStringInfoDatum(StringInfo *string_info,
  const unsigned char *source)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  if (string_info->length != 0)
    (void) memcpy(string_info->datum,source,string_info->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o L e n g t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfoLength() set the string length to the specified value.
%
%  The format of the SetStringInfoLength method is:
%
%      void SetStringInfoLength(StringInfo *string_info,const size_t length)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
%    o length: the string length.
%
*/
MagickExport void SetStringInfoLength(StringInfo *string_info,
  const size_t length)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  if (string_info->length == length)
    return;
  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  string_info->length=length;
  if (string_info->datum == (unsigned char *) NULL)
    string_info->datum=(unsigned char *) AcquireQuantumMemory(length+
      MagickPathExtent,sizeof(*string_info->datum));
  else
    string_info->datum=(unsigned char *) ResizeQuantumMemory(string_info->datum,
      length+MagickPathExtent,sizeof(*string_info->datum));
  if (string_info->datum == (unsigned char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o N a m e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfoName() sets the name associated with the string.
%
%  The format of the SetStringInfoName method is:
%
%      void SetStringInfoName(StringInfo *string_info,const char *name)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
%    o name: the name.
%
*/
MagickExport void SetStringInfoName(StringInfo *string_info,const char *name)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  assert(name != (const char *) NULL);
  string_info->name=ConstantString(name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o P a t h                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfoPath() sets the path associated with the string.
%
%  The format of the SetStringInfoPath method is:
%
%      void SetStringInfoPath(StringInfo *string_info,const char *path)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
%    o path: the path.
%
*/
MagickExport void SetStringInfoPath(StringInfo *string_info,const char *path)
{
  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  assert(path != (const char *) NULL);
  string_info->path=ConstantString(path);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p l i t S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SplitStringInfo() splits a string into two and returns it.
%
%  The format of the SplitStringInfo method is:
%
%      StringInfo *SplitStringInfo(StringInfo *string_info,const size_t offset)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
MagickExport StringInfo *SplitStringInfo(StringInfo *string_info,
  const size_t offset)
{
  StringInfo
    *split_info;

  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  if (offset > string_info->length)
    return((StringInfo *) NULL);
  split_info=AcquireStringInfo(offset);
  SetStringInfo(split_info,string_info);
  (void) memmove(string_info->datum,string_info->datum+offset,
    string_info->length-offset+MagickPathExtent);
  SetStringInfoLength(string_info,string_info->length-offset);
  return(split_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i n g I n f o T o S t r i n g                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringInfoToString() converts a string info string to a C string.
%
%  The format of the StringInfoToString method is:
%
%      char *StringInfoToString(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string.
%
*/
MagickExport char *StringInfoToString(const StringInfo *string_info)
{
  char
    *string;

  size_t
    length;

  string=(char *) NULL;
  length=string_info->length;
  if (~length >= (MagickPathExtent-1))
    string=(char *) AcquireQuantumMemory(length+MagickPathExtent,
      sizeof(*string));
  if (string == (char *) NULL)
    return((char *) NULL);
  (void) memcpy(string,(char *) string_info->datum,length*sizeof(*string));
  string[length]='\0';
  return(string);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i n g I n f o T o H e x S t r i n g                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringInfoToHexString() converts a string info string to a C string.
%
%  The format of the StringInfoToHexString method is:
%
%      char *StringInfoToHexString(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string.
%
*/
MagickExport char *StringInfoToHexString(const StringInfo *string_info)
{
  char
    *string;

  register const unsigned char
    *p;

  register ssize_t
    i;

  register unsigned char
    *q;

  size_t
    length;

  unsigned char
    hex_digits[16];

  length=string_info->length;
  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  string=(char *) AcquireQuantumMemory(length+MagickPathExtent,2*
    sizeof(*string));
  if (string == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  hex_digits[0]='0';
  hex_digits[1]='1';
  hex_digits[2]='2';
  hex_digits[3]='3';
  hex_digits[4]='4';
  hex_digits[5]='5';
  hex_digits[6]='6';
  hex_digits[7]='7';
  hex_digits[8]='8';
  hex_digits[9]='9';
  hex_digits[10]='a';
  hex_digits[11]='b';
  hex_digits[12]='c';
  hex_digits[13]='d';
  hex_digits[14]='e';
  hex_digits[15]='f';
  p=string_info->datum;
  q=(unsigned char *) string;
  for (i=0; i < (ssize_t) string_info->length; i++)
  {
    *q++=hex_digits[(*p >> 4) & 0x0f];
    *q++=hex_digits[*p & 0x0f];
    p++;
  }
  *q='\0';
  return(string);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S t r i n g T o A r g v                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringToArgv() converts a text string into command line arguments.
%  The 'argv' array of arguments, is returned while the number of arguments
%  is returned via the provided integer variable pointer.
%
%  Simple 'word' tokenizer, which allows for each word to be optionally
%  quoted.  However it will not allow use of partial quotes, or escape
%  characters.
%
%  The format of the StringToArgv method is:
%
%      char **StringToArgv(const char *text,int *argc)
%
%  A description of each parameter follows:
%
%    o argv:  Method StringToArgv returns the string list unless an error
%      occurs, otherwise NULL.
%
%    o text:  Specifies the string to segment into a list.
%
%    o argc:  This integer pointer returns the number of arguments in the
%      list.
%
*/
MagickExport char **StringToArgv(const char *text,int *argc)
{
  char
    **argv;

  register const char
    *p,
    *q;

  register ssize_t
    i;

  *argc=0;
  if (text == (char *) NULL)
    return((char **) NULL);
  /*
    Determine the number of arguments.
  */
  for (p=text; *p != '\0'; )
  {
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;
    if (*p == '\0')
      break;
    (*argc)++;
    if (*p == '"')
      for (p++; (*p != '"') && (*p != '\0'); p++) ;
    if (*p == '\'')
      for (p++; (*p != '\'') && (*p != '\0'); p++) ;
    while ((isspace((int) ((unsigned char) *p)) == 0) && (*p != '\0'))
      p++;
  }
  (*argc)++;
  argv=(char **) AcquireQuantumMemory((size_t) (*argc+1UL),sizeof(*argv));
  if (argv == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConvertStringToARGV");
  /*
    Convert string to an ASCII list.
  */
  argv[0]=AcquireString("magick");
  p=text;
  for (i=1; i < (ssize_t) *argc; i++)
  {
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;
    q=p;
    if (*q == '"')
      {
        p++;
        for (q++; (*q != '"') && (*q != '\0'); q++) ;
      }
    else
      if (*q == '\'')
        {
          p++;
          for (q++; (*q != '\'') && (*q != '\0'); q++) ;
        }
      else
        while ((isspace((int) ((unsigned char) *q)) == 0) && (*q != '\0'))
          q++;
    argv[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+MagickPathExtent,
      sizeof(**argv));
    if (argv[i] == (char *) NULL)
      {
        for (i--; i >= 0; i--)
          argv[i]=DestroyString(argv[i]);
        argv=(char **) RelinquishMagickMemory(argv);
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToConvertStringToARGV");
      }
    (void) memcpy(argv[i],p,(size_t) (q-p));
    argv[i][q-p]='\0';
    p=q;
    while ((isspace((int) ((unsigned char) *p)) == 0) && (*p != '\0'))
      p++;
  }
  argv[i]=(char *) NULL;
  return(argv);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i n g T o A r r a y O f D o u b l e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringToArrayOfDoubles() converts a string of space or comma separated
%  numbers into array of floating point numbers (doubles). Any number that
%  failes to parse properly will produce a syntax error. As will two commas
%  without a  number between them.  However a final comma at the end will
%  not be regarded as an error so as to simplify automatic list generation.
%
%  A NULL value is returned on syntax or memory errors.
%
%  Use RelinquishMagickMemory() to free returned array when finished.
%
%  The format of the StringToArrayOfDoubles method is:
%
%     double *StringToArrayOfDoubles(const char *string,size_t *count,
%       ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o string: the string containing the comma/space separated values.
%
%    o count: returns number of arguments in returned array
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport double *StringToArrayOfDoubles(const char *string,ssize_t *count,
  ExceptionInfo *exception)
{
  char
    *q;

  const char
    *p;

  double
    *array;

  register ssize_t
    i;

  /*
    Determine count of values, and check syntax.
  */
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  *count=0;
  i=0;
  p=string;
  while (*p != '\0')
  {
    (void) StringToDouble(p,&q);  /* get value - ignores leading space */
    if (p == q)
      return((double *) NULL);  /* no value found */
    p=q;
    i++;  /* increment value count */
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;  /* skip spaces */
    if (*p == ',')
      p++;  /* skip comma */
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;  /* and more spaces */
  }
  /*
    Allocate floating point argument list.
  */
  *count=i;
  array=(double *) AcquireQuantumMemory((size_t) i,sizeof(*array));
  if (array == (double *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'","");
      return((double *) NULL);
    }
  /*
    Fill in the floating point values.
  */
  i=0;
  p=string;
  while ((*p != '\0') && (i < *count))
  {
    array[i++]=StringToDouble(p,&q);
    p=q;
    while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
      p++;
  }
  return(array);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t r i n g T o k e n                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringToken() looks for any one of given delimiters and splits the string
%  into two separate strings by replacing the delimiter character found with a
%  nul character.
%
%  The given string pointer is changed to point to the string following the
%  delimiter character found, or NULL.  A pointer to the start of the
%  string is returned, representing the token before the delimiter.
%
%  In may ways this is equivent to the strtok() C library function, but with
%  multiple delimiter characters rather than a delimiter string.
%
%  The format of the StringToken method is:
%
%      char *StringToken(const char *delimiters,char **string)
%
%  A description of each parameter follows:
%
%    o delimiters: one or more delimiters.
%
%    o string: return the first token in the string.  If none is found, return
%      NULL.
%
*/
MagickExport char *StringToken(const char *delimiters,char **string)
{
  char
    *q;

  register char
    *p;

  register const char
    *r;

  register int
    c,
    d;

  p=(*string);
  if (p == (char *) NULL)
    return((char *) NULL);
  q=p;
  for ( ; ; )
  {
    c=(*p++);
    r=delimiters;
    do
    {
      d=(*r++);
      if (c == d)
        {
          if (c == '\0')
            p=(char *) NULL;
          else
            p[-1]='\0';
          *string=p;
          return(q);
        }
    } while (d != '\0');
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S t r i n g T o L i s t                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringToList() converts a text string into a list by segmenting the text
%  string at each carriage return discovered.  The list is converted to HEX
%  characters if any control characters are discovered within the text string.
%
%  The format of the StringToList method is:
%
%      char **StringToList(const char *text)
%
%  A description of each parameter follows:
%
%    o text:  Specifies the string to segment into a list.
%
*/
MagickExport char **StringToList(const char *text)
{
  char
    **textlist;

  register const char
    *p;

  register ssize_t
    i;

  size_t
    lines;

  if (text == (char *) NULL)
    return((char **) NULL);
  for (p=text; *p != '\0'; p++)
    if (((int) ((unsigned char) *p) < 32) &&
        (isspace((int) ((unsigned char) *p)) == 0))
      break;
  if (*p == '\0')
    {
      register const char
        *q;

      /*
        Convert string to an ASCII list.
      */
      lines=1;
      for (p=text; *p != '\0'; p++)
        if (*p == '\n')
          lines++;
      textlist=(char **) AcquireQuantumMemory((size_t) lines+1UL,
        sizeof(*textlist));
      if (textlist == (char **) NULL)
        ThrowFatalException(ResourceLimitFatalError,"UnableToConvertText");
      p=text;
      for (i=0; i < (ssize_t) lines; i++)
      {
        for (q=p; *q != '\0'; q++)
          if ((*q == '\r') || (*q == '\n'))
            break;
        textlist[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+
          MagickPathExtent,sizeof(**textlist));
        if (textlist[i] == (char *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"UnableToConvertText");
        (void) memcpy(textlist[i],p,(size_t) (q-p));
        textlist[i][q-p]='\0';
        if (*q == '\r')
          q++;
        p=q+1;
      }
    }
  else
    {
      char
        hex_string[MagickPathExtent];

      register char
        *q;

      register ssize_t
        j;

      /*
        Convert string to a HEX list.
      */
      lines=(size_t) (strlen(text)/0x14)+1;
      textlist=(char **) AcquireQuantumMemory((size_t) lines+1UL,
        sizeof(*textlist));
      if (textlist == (char **) NULL)
        ThrowFatalException(ResourceLimitFatalError,"UnableToConvertText");
      p=text;
      for (i=0; i < (ssize_t) lines; i++)
      {
        textlist[i]=(char *) AcquireQuantumMemory(2UL*MagickPathExtent,
          sizeof(**textlist));
        if (textlist[i] == (char *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"UnableToConvertText");
        (void) FormatLocaleString(textlist[i],MagickPathExtent,"0x%08lx: ",
          (long) (0x14*i));
        q=textlist[i]+strlen(textlist[i]);
        for (j=1; j <= (ssize_t) MagickMin(strlen(p),0x14); j++)
        {
          (void) FormatLocaleString(hex_string,MagickPathExtent,"%02x",*(p+j));
          (void) CopyMagickString(q,hex_string,MagickPathExtent);
          q+=2;
          if ((j % 0x04) == 0)
            *q++=' ';
        }
        for ( ; j <= 0x14; j++)
        {
          *q++=' ';
          *q++=' ';
          if ((j % 0x04) == 0)
            *q++=' ';
        }
        *q++=' ';
        for (j=1; j <= (ssize_t) MagickMin(strlen(p),0x14); j++)
        {
          if (isprint((int) ((unsigned char) *p)) != 0)
            *q++=(*p);
          else
            *q++='-';
          p++;
        }
        *q='\0';
      }
    }
  textlist[i]=(char *) NULL;
  return(textlist);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i n g T o S t r i n g I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringToStringInfo() converts a string to a StringInfo type.
%
%  The format of the StringToStringInfo method is:
%
%      StringInfo *StringToStringInfo(const char *string)
%
%  A description of each parameter follows:
%
%    o string:  The string.
%
*/
MagickExport StringInfo *StringToStringInfo(const char *string)
{
  StringInfo
    *string_info;

  assert(string != (const char *) NULL);
  string_info=AcquireStringInfo(strlen(string));
  SetStringInfoDatum(string_info,(const unsigned char *) string);
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i p S t r i n g                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StripString() strips any whitespace or quotes from the beginning and end of
%  a string of characters.
%
%  The format of the StripString method is:
%
%      void StripString(char *message)
%
%  A description of each parameter follows:
%
%    o message: Specifies an array of characters.
%
*/
MagickExport void StripString(char *message)
{
  register char
    *p,
    *q;

  size_t
    length;

  assert(message != (char *) NULL);
  if (*message == '\0')
    return;
  length=strlen(message);
  p=message;
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  if ((*p == '\'') || (*p == '"'))
    p++;
  q=message+length-1;
  while ((isspace((int) ((unsigned char) *q)) != 0) && (q > p))
    q--;
  if (q > p)
    if ((*q == '\'') || (*q == '"'))
      q--;
  (void) memmove(message,p,(size_t) (q-p+1));
  message[q-p+1]='\0';
  for (p=message; *p != '\0'; p++)
    if (*p == '\n')
      *p=' ';
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S u b s t i t u t e S t r i n g                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SubstituteString() performs string substitution on a string, replacing the
%  string with the substituted version. Buffer must be allocated from the heap.
%  If the string is matched and status, MagickTrue is returned otherwise
%  MagickFalse.
%
%  The format of the SubstituteString method is:
%
%      MagickBooleanType SubstituteString(char **string,const char *search,
%        const char *replace)
%
%  A description of each parameter follows:
%
%    o string: the string to perform replacements on;  replaced with new
%      allocation if a replacement is made.
%
%    o search: search for this string.
%
%    o replace: replace any matches with this string.
%
*/
MagickExport MagickBooleanType SubstituteString(char **string,
  const char *search,const char *replace)
{
  MagickBooleanType
    status;

  register char
    *p;

  size_t
    extent,
    replace_extent,
    search_extent;

  ssize_t
    offset;

  status=MagickFalse;
  search_extent=0,
  replace_extent=0;
  for (p=strchr(*string,*search); p != (char *) NULL; p=strchr(p+1,*search))
  {
    if (search_extent == 0)
      search_extent=strlen(search);
    if (strncmp(p,search,search_extent) != 0)
      continue;
    /*
      We found a match.
    */
    status=MagickTrue;
    if (replace_extent == 0)
      replace_extent=strlen(replace);
    if (replace_extent > search_extent)
      {
        /*
          Make room for the replacement string.
        */
        offset=(ssize_t) (p-(*string));
        extent=strlen(*string)+replace_extent-search_extent+1;
        *string=(char *) ResizeQuantumMemory(*string,extent+MagickPathExtent,
          sizeof(*p));
        if (*string == (char *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
        p=(*string)+offset;
      }
    /*
      Replace string.
    */
    if (search_extent != replace_extent)
      (void) CopyMagickMemory(p+replace_extent,p+search_extent,
        strlen(p+search_extent)+1);
    (void) CopyMagickMemory(p,replace,replace_extent);
    p+=replace_extent-1;
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%                                 N   N  TTTTT                                %
%                                 NN  N    T                                  %
%                                 N N N    T                                  %
%                                 N  NN    T                                  %
%                                 N   N    T                                  %
%                                                                             %
%                                                                             %
%                   Windows NT Feature Methods for MagickCore                 %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                December 1996                                %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/locale-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/nt-base.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/nt-feature.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/utility.h"
#if defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#endif

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
%  NTIsMagickConflict() returns true if the image format conflicts with a
%  logical drive (e.g. X:).
%
%  The format of the NTIsMagickConflict method is:
%
%      MagickBooleanType IsMagickConflict(const char *magick)
%
%  A description of each parameter follows:
%
%    o magick: Specifies the image format.
%
*/
MagickExport MagickBooleanType NTIsMagickConflict(const char *magick)
{
  int
    c;

  assert(magick != (char *) NULL);
  /*
    A drive specifier is exactly one letter; reject "" and anything longer so
    the shift below can never be negative or exceed the 26 drive-letter bits.
  */
  if ((magick[0] == '\0') || (magick[1] != '\0'))
    return(MagickFalse);
  c=LocaleToUppercase((int) ((unsigned char) magick[0]));
  if ((c < 'A') || (c > 'Z'))
    return(MagickFalse);
  return((GetLogicalDrives() & (1UL << (c-'A'))) != 0 ? MagickTrue :
    MagickFalse);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T A c q u i r e T y p e C a c h e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTAcquireTypeCache() loads the installed Windows TrueType fonts.
%
%  The format of the NTAcquireTypeCache method is:
%
%      MagickBooleanType NTAcquireTypeCache(SplayTreeInfo *type_cache,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o type_cache: A linked list of fonts.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void DestroyNTTypeInfo(TypeInfo *type_info)
{
  /*
    Releases a TypeInfo that was never handed over to the type cache.
  */
  if (type_info->path != (char *) NULL)
    type_info->path=DestroyString(type_info->path);
  if (type_info->name != (char *) NULL)
    type_info->name=DestroyString(type_info->name);
  if (type_info->description != (char *) NULL)
    type_info->description=DestroyString(type_info->description);
  if (type_info->family != (char *) NULL)
    type_info->family=DestroyString(type_info->family);
  if (type_info->encoding != (char *) NULL)
    type_info->encoding=DestroyString(type_info->encoding);
  if (type_info->foundry != (char *) NULL)
    type_info->foundry=DestroyString(type_info->foundry);
  if (type_info->format != (char *) NULL)
    type_info->format=DestroyString(type_info->format);
  if (type_info->metrics != (char *) NULL)
    type_info->metrics=DestroyString(type_info->metrics);
  if (type_info->glyphs != (char *) NULL)
    type_info->glyphs=DestroyString(type_info->glyphs);
  type_info=(TypeInfo *) RelinquishMagickMemory(type_info);
}

MagickExport MagickBooleanType NTAcquireTypeCache(SplayTreeInfo *type_cache,
  ExceptionInfo *exception)
{
  static const char
    *version_keys[] =
    {
      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
    };

  char
    buffer[MagickPathExtent],
    font_root[MagickPathExtent];

  DWORD
    length,
    type;

  HKEY
    fonts_key,
    version_key;

  LONG
    res;

  MagickBooleanType
    found,
    status;

  size_t
    i,
    root_length;

  /*
    Try to find the right Windows*\CurrentVersion key, the SystemRoot and
    then the Fonts key.  Every handle that is opened is closed again.
  */
  assert(type_cache != (SplayTreeInfo *) NULL);
  found=MagickFalse;
  version_key=(HKEY) NULL;
  for (i=0; i < (sizeof(version_keys)/sizeof(*version_keys)); i++)
  {
    (void) memset(font_root,0,sizeof(font_root));
    version_key=(HKEY) NULL;
    res=RegOpenKeyExA(HKEY_LOCAL_MACHINE,version_keys[i],0,KEY_READ,
      &version_key);
    if (res != ERROR_SUCCESS)
      continue;
    length=(DWORD) (sizeof(font_root)-1);  /* leave room for a terminator */
    type=REG_NONE;
    res=RegQueryValueExA(version_key,"SystemRoot",NULL,&type,(BYTE *) font_root,
      &length);
    if ((res == ERROR_SUCCESS) && ((type == REG_SZ) || (type == REG_EXPAND_SZ)))
      {
        found=MagickTrue;
        break;
      }
    (void) RegCloseKey(version_key);
    version_key=(HKEY) NULL;
  }
  if (found == MagickFalse)
    return(MagickFalse);
  fonts_key=(HKEY) NULL;
  res=RegOpenKeyExA(version_key,"Fonts",0,KEY_READ,&fonts_key);
  (void) RegCloseKey(version_key);
  if (res != ERROR_SUCCESS)
    return(MagickFalse);
  /*
    Drop any trailing separators, then pick the fonts directory.
  */
  root_length=strlen(font_root);
  while ((root_length > 0) && (font_root[root_length-1] == '\\'))
    font_root[--root_length]='\0';
  (void) CopyMagickString(buffer,font_root,MagickPathExtent);
  (void) ConcatenateMagickString(buffer,"\\fonts\\arial.ttf",MagickPathExtent);
  if (IsPathAccessible(buffer) != MagickFalse)
    (void) ConcatenateMagickString(font_root,"\\fonts\\",MagickPathExtent);
  else
    (void) ConcatenateMagickString(font_root,"\\",MagickPathExtent);

  {
    DWORD
      registry_index;

    char
      utf8[MagickPathExtent];

    TypeInfo
      *type_info;

    wchar_t
      wide_name[MagickPathExtent],
      wide_value[MagickPathExtent];

    registry_index=0;
    res=ERROR_SUCCESS;
    while (res != ERROR_NO_MORE_ITEMS)
      {
        char
          *family_extent,
          *pos,
          *q;

        DWORD
          name_length,
          value_length;

        /*
          RegEnumValueW() counts the name in characters (including the
          terminator on input) and the data in bytes.
        */
        name_length=MagickPathExtent;
        value_length=(DWORD) (sizeof(wide_value)-sizeof(wchar_t));
        type=REG_NONE;
        res=RegEnumValueW(fonts_key,registry_index,(wchar_t *) wide_name,
          &name_length,NULL,&type,(BYTE *) wide_value,&value_length);
        registry_index++;
        if (res != ERROR_SUCCESS)
          continue;
        if (type != REG_SZ)
          continue;
        /*
          Registry strings are not guaranteed to be terminated.
        */
        wide_name[MagickPathExtent-1]=L'\0';
        wide_value[value_length/sizeof(wchar_t)]=L'\0';
        if (WideCharToMultiByte(CP_UTF8,0,wide_name,-1,utf8,(int) sizeof(utf8),
              NULL,NULL) == 0)
          continue;
        if ((pos=strstr(utf8," (TrueType)")) == (char*) NULL)
          continue;
        *pos='\0'; /* Remove (TrueType) from string */
        if (*utf8 == '\0')
          continue;
        type_info=(TypeInfo *) AcquireCriticalMemory(sizeof(*type_info));
        (void) memset(type_info,0,sizeof(TypeInfo));
        type_info->path=ConstantString("Windows Fonts");
        type_info->signature=MagickCoreSignature;
        (void) CopyMagickString(buffer,utf8,MagickPathExtent);
        for (pos=buffer; *pos != 0; pos++)
          if (*pos == ' ')
            *pos = '-';
        type_info->name=ConstantString(buffer);
        type_info->description=ConstantString(utf8);
        type_info->format=ConstantString("truetype");
        type_info->stretch=NormalStretch;
        type_info->style=NormalStyle;
        type_info->weight=400;
        /* Some fonts are known to require special encodings */
        if ((LocaleCompare(type_info->name, "Symbol") == 0) ||
            (LocaleCompare(type_info->name, "Wingdings") == 0) ||
            (LocaleCompare(type_info->name, "Wingdings-2") == 0) ||
            (LocaleCompare(type_info->name, "Wingdings-3") == 0))
          type_info->encoding=ConstantString("AppleRoman");
        family_extent=utf8;
        for (q=utf8; *q != '\0'; )
          {
            char
              token[MagickPathExtent];

            (void) GetNextToken(q,(const char **) &q,MagickPathExtent,token);
            if (*token == '\0')
              break;
            if (LocaleCompare(token,"Italic") == 0)
              type_info->style=ItalicStyle;
            else if (LocaleCompare(token,"Oblique") == 0)
              type_info->style=ObliqueStyle;
            else if (LocaleCompare(token,"Bold") == 0)
              type_info->weight=700;
            else if (LocaleCompare(token,"Thin") == 0)
              type_info->weight=100;
            else if ((LocaleCompare(token,"ExtraLight") == 0) ||
                      (LocaleCompare(token,"UltraLight") == 0))
              type_info->weight=200;
            else if (LocaleCompare(token,"Light") == 0)
              type_info->weight=300;
            else if ((LocaleCompare(token,"Normal") == 0) ||
                     (LocaleCompare(token,"Regular") == 0))
              type_info->weight=400;
            else if (LocaleCompare(token,"Medium") == 0)
              type_info->weight=500;
            else if ((LocaleCompare(token,"SemiBold") == 0) ||
                     (LocaleCompare(token,"DemiBold") == 0))
              type_info->weight=600;
            else if ((LocaleCompare(token,"ExtraBold") == 0) ||
                     (LocaleCompare(token,"UltraBold") == 0))
              type_info->weight=800;
            else if ((LocaleCompare(token,"Heavy") == 0) ||
                     (LocaleCompare(token,"Black") == 0))
              type_info->weight=900;
            else if (LocaleCompare(token,"Condensed") == 0)
              type_info->stretch = CondensedStretch;
            else if (LocaleCompare(token,"Expanded") == 0)
              type_info->stretch = ExpandedStretch;
            else if (LocaleCompare(token,"ExtraCondensed") == 0)
              type_info->stretch = ExtraCondensedStretch;
            else if (LocaleCompare(token,"ExtraExpanded") == 0)
              type_info->stretch = ExtraExpandedStretch;
            else if (LocaleCompare(token,"SemiCondensed") == 0)
              type_info->stretch = SemiCondensedStretch;
            else if (LocaleCompare(token,"SemiExpanded") == 0)
              type_info->stretch = SemiExpandedStretch;
            else if (LocaleCompare(token,"UltraCondensed") == 0)
              type_info->stretch = UltraCondensedStretch;
            else if (LocaleCompare(token,"UltraExpanded") == 0)
              type_info->stretch = UltraExpandedStretch;
            else
              family_extent=q;
          }
        (void) CopyMagickString(buffer,utf8,(size_t) (family_extent-utf8)+1);
        (void) StripMagickString(buffer);
        type_info->family=ConstantString(buffer);
        if (WideCharToMultiByte(CP_UTF8,0,wide_value,-1,utf8,(int) sizeof(utf8),
              NULL,NULL) == 0)
          {
            DestroyNTTypeInfo(type_info);
            continue;
          }
        if (strchr(utf8,'\\') != (char *) NULL)
          (void) CopyMagickString(buffer,utf8,MagickPathExtent);
        else
          {
            (void) CopyMagickString(buffer,font_root,MagickPathExtent);
            (void) ConcatenateMagickString(buffer,utf8,MagickPathExtent);
          }
        LocaleLower(buffer);
        type_info->glyphs=ConstantString(buffer);
        status=AddValueToSplayTree(type_cache,type_info->name,type_info);
        if (status == MagickFalse)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'",
              type_info->name);
            DestroyNTTypeInfo(type_info);  /* the tree did not take it */
          }
      }
  }
  (void) RegCloseKey(fonts_key);
  return(MagickTrue);
}


#endif

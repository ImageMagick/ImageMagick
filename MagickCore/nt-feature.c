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
%  IsMagickConflict() returns true if the image format conflicts with a logical
%  drive (.e.g. X:).
%
%  The format of the IsMagickConflict method is:
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
  MagickBooleanType
    status;

  assert(magick != (char *) NULL);
  if (strlen(magick) > 1)
    return(MagickFalse);
  status=(GetLogicalDrives() & (1 << ((LocaleToUppercase((int)
    (*magick)))-'A'))) != 0 ? MagickTrue : MagickFalse;
  return(status);
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
%  NTAcquireTypeCache() loads a Windows TrueType fonts.
%
%  The format of the NTAcquireTypeCache method is:
%
%      MagickBooleanType NTAcquireTypeCache(SplayTreeInfo *type_cache)
%
%  A description of each parameter follows:
%
%    o type_cache: A linked list of fonts.
%
*/
MagickExport MagickBooleanType NTAcquireTypeCache(SplayTreeInfo *type_cache,
  ExceptionInfo *exception)
{
  HKEY
    reg_key;

  LONG
    res;

  char
    buffer[MagickPathExtent],
    font_root[MagickPathExtent];

  DWORD
    type,
    length;

  MagickBooleanType
    status;

  /*
    Try to find the right Windows*\CurrentVersion key, the SystemRoot and
    then the Fonts key
  */
  reg_key=(HKEY) INVALID_HANDLE_VALUE;
  res=RegOpenKeyExA(HKEY_LOCAL_MACHINE,
    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",0,KEY_READ,&reg_key);
  length=sizeof(font_root)-1;
  if (res == ERROR_SUCCESS)
    res=RegQueryValueExA(reg_key,"SystemRoot",NULL,&type,(BYTE*) font_root,
      &length);
  if (res != ERROR_SUCCESS)
    {
      res=RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion",0,KEY_READ,&reg_key);
      if (res == ERROR_SUCCESS)
        res=RegQueryValueExA(reg_key,"SystemRoot",NULL,&type,(BYTE*) font_root,
          &length);
    }
  if (res == ERROR_SUCCESS)
    res=RegOpenKeyExA(reg_key,"Fonts",0,KEY_READ,&reg_key);
  if (res != ERROR_SUCCESS)
    return(MagickFalse);
  (void) ConcatenateMagickString(font_root,"\\fonts\\arial.ttf",
    MagickPathExtent);
  if (IsPathAccessible(font_root) != MagickFalse)
    {
      font_root[length-1]=0;
      (void) ConcatenateMagickString(font_root,"\\fonts\\",MagickPathExtent);
    }
  else
    {
      font_root[length-1]=0;
      (void) ConcatenateMagickString(font_root,"\\",MagickPathExtent);
    }

  {
    TypeInfo
      *type_info;

    DWORD
      registry_index;

    char
      utf8[MagickPathExtent];

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

        name_length=MagickPathExtent-1;
        value_length=MagickPathExtent-1;
        res=RegEnumValueW(reg_key,registry_index,(wchar_t *) wide_name,
          &name_length,0,&type,(BYTE *) wide_value,&value_length);
        registry_index++;
        if (res != ERROR_SUCCESS)
          continue;
        WideCharToMultiByte(CP_UTF8,0,wide_name,-1,utf8,sizeof(utf8),NULL,
          NULL);
        if ((pos=strstr(utf8," (TrueType)")) == (char*) NULL)
          continue;
        *pos='\0'; /* Remove (TrueType) from string */
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
        (void) CopyMagickString(buffer,utf8,family_extent-utf8+1);
        (void) StripMagickString(buffer);
        type_info->family=ConstantString(buffer);
        WideCharToMultiByte(CP_UTF8,0,wide_value,-1,utf8,sizeof(utf8),NULL,
          NULL);
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
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",type_info->name);
      }
  }
  RegCloseKey(reg_key);
  return(MagickTrue);
}

#endif

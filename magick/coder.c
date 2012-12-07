/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   CCCC   OOO   DDDD    EEEEE  RRRR                          %
%                  C      O   O  D   D   E      R   R                         %
%                  C      O   O  D   D   EEE    RRRR                          %
%                  C      O   O  D   D   E      R R                           %
%                   CCCC   OOO   DDDD    EEEEE  R  R                          %
%                                                                             %
%                                                                             %
%                     MagickCore Image Coder Methods                          %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 May 2001                                    %
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
#include "magick/blob.h"
#include "magick/client.h"
#include "magick/coder.h"
#include "magick/configure.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/splay-tree.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define MagickCoderFilename  "coder.xml"

/*
  Typedef declarations.
*/
typedef struct _CoderMapInfo
{
  const char
    *magick,
    *name;
} CoderMapInfo;

/*
  Static declarations.
*/
static const CoderMapInfo
  CoderMap[] =
  {
    { "3FR", "DNG" },
    { "8BIM", "META" },
    { "8BIMTEXT", "META" },
    { "8BIMWTEXT", "META" },
    { "AFM", "TTF" },
    { "A", "RAW" },
    { "AI", "PDF" },
    { "APP1JPEG", "META" },
    { "APP1", "META" },
    { "ARW", "DNG" },
    { "AVI", "MPEG" },
    { "BIE", "JBIG" },
    { "BMP2", "BMP" },
    { "BMP3", "BMP" },
    { "B", "RAW" },
    { "BRF", "BRAILLE" },
    { "BGRA", "BGR" },
    { "CMYKA", "CMYK" },
    { "C", "RAW" },
    { "CAL", "CALS" },
    { "CANVAS", "XC" },
    { "CR2", "DNG" },
    { "CRW", "DNG" },
    { "CUR", "ICON" },
    { "DCR", "DNG" },
    { "DCX", "PCX" },
    { "DFONT", "TTF" },
    { "EPDF", "PDF" },
    { "EPI", "PS" },
    { "EPS2", "PS2" },
    { "EPS3", "PS3" },
    { "EPSF", "PS" },
    { "EPSI", "PS" },
    { "EPS", "PS" },
    { "EPT2", "EPT" },
    { "EPT3", "EPT" },
    { "ERF", "DNG" },
    { "EXIF", "META" },
    { "FILE", "URL" },
    { "FRACTAL", "PLASMA" },
    { "FTP", "URL" },
    { "FTS", "FITS" },
    { "G3", "FAX" },
    { "GIF87", "GIF" },
    { "G", "RAW" },
    { "GRANITE", "MAGICK" },
    { "GROUP4", "TIFF" },
    { "K25", "DNG" },
    { "KDC", "DNG" },
    { "H", "MAGICK" },
    { "HTM", "HTML" },
    { "HTTP", "URL" },
    { "ICB", "TGA" },
    { "ICC", "META" },
    { "ICM", "META" },
    { "ICO", "ICON" },
    { "IMPLICIT", "***" },
    { "IPTC", "META" },
    { "IPTCTEXT", "META" },
    { "IPTCWTEXT", "META" },
    { "ISOBRL", "BRAILLE" },
    { "JBG", "JBIG" },
    { "JNG", "PNG" },
    { "JPC", "JP2" },
    { "J2C", "JP2" },
    { "J2K", "JP2" },
    { "JPG", "JPEG" },
    { "JPX", "JP2" },
    { "K", "RAW" },
    { "LOGO", "MAGICK" },
    { "M2V", "MPEG" },
    { "M4V", "MPEG" },
    { "M", "RAW" },
    { "MNG", "PNG" },
    { "MOV", "MPEG" },
    { "MP4", "MPEG" },
    { "MPG", "MPEG" },
    { "MPRI", "MPR" },
    { "MEF", "DNG" },
    { "MRW", "DNG" },
    { "MSVG", "SVG" },
    { "NEF", "DNG" },
    { "NETSCAPE", "MAGICK" },
    { "NRW", "DNG" },
    { "O", "RAW" },
    { "ORF", "DNG" },
    { "OTF", "TTF" },
    { "P7", "PNM" },
    { "PAL", "UYVY" },
    { "PAM", "PNM" },
    { "PBM", "PNM" },
    { "PCDS", "PCD" },
    { "PDFA", "PDF" },
    { "PEF", "DNG" },
    { "PEF", "DNG" },
    { "PFA", "TTF" },
    { "PFB", "TTF" },
    { "PFM", "PNM" },
    { "PGM", "PNM" },
    { "PGX", "JP2" },
    { "PICON", "XPM" },
    { "PJPEG", "JPEG" },
    { "PM", "XPM" },
    { "PNG24", "PNG" },
    { "PNG32", "PNG" },
    { "PNG8", "PNG" },
    { "PPM", "PNM" },
    { "PSB", "PSD" },
    { "PTIF", "TIFF" },
    { "RADIAL-GRADIENT", "GRADIENT" },
    { "RAF", "DNG" },
    { "RAS", "SUN" },
    { "RGBA", "RGB" },
    { "RGBO", "RGB" },
    { "R", "RAW" },
    { "ROSE", "MAGICK" },
    { "RW2", "DNG" },
    { "SHTML", "HTML" },
    { "SR2", "DNG" },
    { "SRF", "DNG" },
    { "SVGZ", "SVG" },
    { "TEXT", "TXT" },
    { "TIFF64", "TIFF" },
    { "TIF", "TIFF" },
    { "TTC", "TTF" },
    { "UBRL", "BRAILLE" },
    { "VDA", "TGA" },
    { "VST", "TGA" },
    { "WIZARD", "MAGICK" },
    { "WMV", "MPEG" },
    { "WMFWIN32", "EMF" },
    { "WMZ", "WMF" },
    { "X3f", "DNG" },
    { "XMP", "META" },
    { "XTRNARRAY", "XTRN" },
    { "XTRNBLOB", "XTRN" },
    { "XTRNFILE", "XTRN" },
    { "XTRNIMAGE", "XTRN" },
    { "XV", "VIFF" },
    { "Y", "RAW" },
    { "YCbCrA", "YCbCr" }
 };

static SemaphoreInfo
  *coder_semaphore = (SemaphoreInfo *) NULL;

static SplayTreeInfo
  *coder_list = (SplayTreeInfo *) NULL;

static volatile MagickBooleanType
  instantiate_coder = MagickFalse;

/*
  Forward declarations.
*/
static MagickBooleanType
  InitializeCoderList(ExceptionInfo *),
  LoadCoderLists(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o d e r C o m p o n e n t G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CoderComponentGenesis() instantiates the coder component.
%
%  The format of the CoderComponentGenesis method is:
%
%      MagickBooleanType CoderComponentGenesis(void)
%
*/
MagickExport MagickBooleanType CoderComponentGenesis(void)
{
  AcquireSemaphoreInfo(&coder_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o d e r C o m p o n e n t T e r m i n u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CoderComponentTerminus() destroys the coder component.
%
%  The format of the CoderComponentTerminus method is:
%
%      CoderComponentTerminus(void)
%
*/
MagickExport void CoderComponentTerminus(void)
{
  if (coder_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&coder_semaphore);
  LockSemaphoreInfo(coder_semaphore);
  if (coder_list != (SplayTreeInfo *) NULL)
    coder_list=DestroySplayTree(coder_list);
  instantiate_coder=MagickFalse;
  UnlockSemaphoreInfo(coder_semaphore);
  DestroySemaphoreInfo(&coder_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C o d e r I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCoderInfo searches the coder list for the specified name and if found
%  returns attributes for that coder.
%
%  The format of the GetCoderInfo method is:
%
%      const CoderInfo *GetCoderInfo(const char *name,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the coder name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const CoderInfo *GetCoderInfo(const char *name,
  ExceptionInfo *exception)
{
  assert(exception != (ExceptionInfo *) NULL);
  if ((coder_list == (SplayTreeInfo *) NULL) ||
      (instantiate_coder == MagickFalse))
    if (InitializeCoderList(exception) == MagickFalse)
      return((const CoderInfo *) NULL);
  if ((coder_list == (SplayTreeInfo *) NULL) ||
      (GetNumberOfNodesInSplayTree(coder_list) == 0))
    return((const CoderInfo *) NULL);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    {
      ResetSplayTreeIterator(coder_list);
      return((const CoderInfo *) GetNextValueInSplayTree(coder_list));
    }
  return((const CoderInfo *) GetValueFromSplayTree(coder_list,name));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o d e r I n f o L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCoderInfoList() returns any coder_map that match the specified pattern.
%  The format of the GetCoderInfoList function is:
%
%      const CoderInfo **GetCoderInfoList(const char *pattern,
%        size_t *number_coders,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_coders:  This integer returns the number of coders in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int CoderInfoCompare(const void *x,const void *y)
{
  const CoderInfo
    **p,
    **q;

  p=(const CoderInfo **) x,
  q=(const CoderInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->name,(*q)->name));
  return(LocaleCompare((*p)->path,(*q)->path));
}

MagickExport const CoderInfo **GetCoderInfoList(const char *pattern,
  size_t *number_coders,ExceptionInfo *exception)
{
  const CoderInfo
    **coder_map;

  register const CoderInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate coder list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_coders != (size_t *) NULL);
  *number_coders=0;
  p=GetCoderInfo("*",exception);
  if (p == (const CoderInfo *) NULL)
    return((const CoderInfo **) NULL);
  coder_map=(const CoderInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(coder_list)+1UL,sizeof(*coder_map));
  if (coder_map == (const CoderInfo **) NULL)
    return((const CoderInfo **) NULL);
  /*
    Generate coder list.
  */
  LockSemaphoreInfo(coder_semaphore);
  ResetSplayTreeIterator(coder_list);
  p=(const CoderInfo *) GetNextValueInSplayTree(coder_list);
  for (i=0; p != (const CoderInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      coder_map[i++]=p;
    p=(const CoderInfo *) GetNextValueInSplayTree(coder_list);
  }
  UnlockSemaphoreInfo(coder_semaphore);
  qsort((void *) coder_map,(size_t) i,sizeof(*coder_map),CoderInfoCompare);
  coder_map[i]=(CoderInfo *) NULL;
  *number_coders=(size_t) i;
  return(coder_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o d e r L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCoderList() returns any coder_map that match the specified pattern.
%
%  The format of the GetCoderList function is:
%
%      char **GetCoderList(const char *pattern,size_t *number_coders,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_coders:  This integer returns the number of coders in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int CoderCompare(const void *x,const void *y)
{
  register const char
    **p,
    **q;

  p=(const char **) x;
  q=(const char **) y;
  return(LocaleCompare(*p,*q));
}

MagickExport char **GetCoderList(const char *pattern,
  size_t *number_coders,ExceptionInfo *exception)
{
  char
    **coder_map;

  register const CoderInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate coder list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_coders != (size_t *) NULL);
  *number_coders=0;
  p=GetCoderInfo("*",exception);
  if (p == (const CoderInfo *) NULL)
    return((char **) NULL);
  coder_map=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(coder_list)+1UL,sizeof(*coder_map));
  if (coder_map == (char **) NULL)
    return((char **) NULL);
  /*
    Generate coder list.
  */
  LockSemaphoreInfo(coder_semaphore);
  ResetSplayTreeIterator(coder_list);
  p=(const CoderInfo *) GetNextValueInSplayTree(coder_list);
  for (i=0; p != (const CoderInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      coder_map[i++]=ConstantString(p->name);
    p=(const CoderInfo *) GetNextValueInSplayTree(coder_list);
  }
  UnlockSemaphoreInfo(coder_semaphore);
  qsort((void *) coder_map,(size_t) i,sizeof(*coder_map),CoderCompare);
  coder_map[i]=(char *) NULL;
  *number_coders=(size_t) i;
  return(coder_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e C o d e r L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeCoderList() initializes the coder list.
%
%  The format of the InitializeCoderList method is:
%
%      MagickBooleanType InitializeCoderList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType InitializeCoderList(ExceptionInfo *exception)
{
  if ((coder_list == (SplayTreeInfo *) NULL) &&
      (instantiate_coder == MagickFalse))
    {
      if (coder_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&coder_semaphore);
      LockSemaphoreInfo(coder_semaphore);
      if ((coder_list == (SplayTreeInfo *) NULL) &&
          (instantiate_coder == MagickFalse))
        {
          (void) LoadCoderLists(MagickCoderFilename,exception);
          instantiate_coder=MagickTrue;
        }
      UnlockSemaphoreInfo(coder_semaphore);
    }
  return(coder_list != (SplayTreeInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t C o d e r I n f o                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListCoderInfo() lists the coder info to a file.
%
%  The format of the ListCoderInfo coder is:
%
%      MagickBooleanType ListCoderInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListCoderInfo(FILE *file,
  ExceptionInfo *exception)
{
  const char
    *path;

  const CoderInfo
    **coder_info;

  register ssize_t
    i;

  size_t
    number_coders;

  ssize_t
    j;

  if (file == (const FILE *) NULL)
    file=stdout;
  coder_info=GetCoderInfoList("*",&number_coders,exception);
  if (coder_info == (const CoderInfo **) NULL)
    return(MagickFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_coders; i++)
  {
    if (coder_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,coder_info[i]->path) != 0))
      {
        if (coder_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",coder_info[i]->path);
        (void) FormatLocaleFile(file,"Magick      Coder\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=coder_info[i]->path;
    (void) FormatLocaleFile(file,"%s",coder_info[i]->magick);
    for (j=(ssize_t) strlen(coder_info[i]->magick); j <= 11; j++)
      (void) FormatLocaleFile(file," ");
    if (coder_info[i]->name != (char *) NULL)
      (void) FormatLocaleFile(file,"%s",coder_info[i]->name);
    (void) FormatLocaleFile(file,"\n");
  }
  coder_info=(const CoderInfo **) RelinquishMagickMemory((void *) coder_info);
  (void) fflush(file);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d C o d e r L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadCoderList() loads the coder configuration file which provides a
%  mapping between coder attributes and a coder name.
%
%  The format of the LoadCoderList coder is:
%
%      MagickBooleanType LoadCoderList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The coder list in XML format.
%
%    o filename:  The coder list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void *DestroyCoderNode(void *coder_info)
{
  register CoderInfo
    *p;

  p=(CoderInfo *) coder_info;
  if (p->exempt == MagickFalse)
    {
      if (p->path != (char *) NULL)
        p->path=DestroyString(p->path);
      if (p->name != (char *) NULL)
        p->name=DestroyString(p->name);
      if (p->magick != (char *) NULL)
        p->magick=DestroyString(p->magick);
    }
  return(RelinquishMagickMemory(p));
}

static MagickBooleanType LoadCoderList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MaxTextExtent],
    *token;

  const char
    *q;

  CoderInfo
    *coder_info;

  MagickBooleanType
    status;

  /*
    Load the coder map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading coder configuration file \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(MagickFalse);
  if (coder_list == (SplayTreeInfo *) NULL)
    {
      coder_list=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
        DestroyCoderNode);
      if (coder_list == (SplayTreeInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  coder_info=(CoderInfo *) NULL;
  token=AcquireString(xml);
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetMagickToken(q,&q,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MaxTextExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          GetMagickToken(q,&q,token);
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MaxTextExtent);
          GetMagickToken(q,&q,token);
          if (*token != '=')
            continue;
          GetMagickToken(q,&q,token);
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > 200)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ConfigureError,"IncludeNodeNestedTooDeeply","`%s'",token);
              else
                {
                  char
                    path[MaxTextExtent],
                    *xml;

                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateMagickString(path,DirectorySeparator,
                      MaxTextExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyMagickString(path,token,MaxTextExtent);
                  else
                    (void) ConcatenateMagickString(path,token,MaxTextExtent);
                  xml=FileToString(path,~0,exception);
                  if (xml != (char *) NULL)
                    {
                      status=LoadCoderList(xml,path,depth+1,exception);
                      xml=(char *) RelinquishMagickMemory(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<coder") == 0)
      {
        /*
          Coder element.
        */
        coder_info=(CoderInfo *) AcquireMagickMemory(sizeof(*coder_info));
        if (coder_info == (CoderInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(coder_info,0,sizeof(*coder_info));
        coder_info->path=ConstantString(filename);
        coder_info->exempt=MagickFalse;
        coder_info->signature=MagickSignature;
        continue;
      }
    if (coder_info == (CoderInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        status=AddValueToSplayTree(coder_list,ConstantString(
          coder_info->magick),coder_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            coder_info->magick);
        coder_info=(CoderInfo *) NULL;
      }
    GetMagickToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetMagickToken(q,&q,token);
    GetMagickToken(q,&q,token);
    switch (*keyword)
    {
      case 'M':
      case 'm':
      {
        if (LocaleCompare((char *) keyword,"magick") == 0)
          {
            coder_info->magick=ConstantString(token);
            break;
          }
        break;
      }
      case 'N':
      case 'n':
      {
        if (LocaleCompare((char *) keyword,"name") == 0)
          {
            coder_info->name=ConstantString(token);
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            coder_info->stealth=IsMagickTrue(token);
            break;
          }
        break;
      }
      default:
        break;
    }
  }
  token=(char *) RelinquishMagickMemory(token);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L o a d C o d e r L i s t s                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadCoderLists() loads one or more coder configuration file which
%  provides a mapping between coder attributes and a coder name.
%
%  The format of the LoadCoderLists coder is:
%
%      MagickBooleanType LoadCoderLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadCoderLists(const char *filename,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  register ssize_t
    i;

  /*
    Load built-in coder map.
  */
  status=MagickFalse;
  if (coder_list == (SplayTreeInfo *) NULL)
    {
      coder_list=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
        DestroyCoderNode);
      if (coder_list == (SplayTreeInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  for (i=0; i < (ssize_t) (sizeof(CoderMap)/sizeof(*CoderMap)); i++)
  {
    CoderInfo
      *coder_info;

    register const CoderMapInfo
      *p;

    p=CoderMap+i;
    coder_info=(CoderInfo *) AcquireMagickMemory(sizeof(*coder_info));
    if (coder_info == (CoderInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",coder_info->name);
        continue;
      }
    (void) ResetMagickMemory(coder_info,0,sizeof(*coder_info));
    coder_info->path=(char *) "[built-in]";
    coder_info->magick=(char *) p->magick;
    coder_info->name=(char *) p->name;
    coder_info->exempt=MagickTrue;
    coder_info->signature=MagickSignature;
    status=AddValueToSplayTree(coder_list,ConstantString(coder_info->magick),
      coder_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",coder_info->name);
  }
  /*
    Load external coder map.
  */
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status|=LoadCoderList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  return(status != 0 ? MagickTrue : MagickFalse);
}

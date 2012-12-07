/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    M   M   AAA    GGGG  IIIII   CCCC                        %
%                    MM MM  A   A  G        I    C                            %
%                    M M M  AAAAA  G GGG    I    C                            %
%                    M   M  A   A  G   G    I    C                            %
%                    M   M  A   A   GGGG  IIIII   CCCC                        %
%                                                                             %
%                                                                             %
%                      MagickCore Image Magic Methods                         %
%                                                                             %
%                              Software Design                                %
%                              Bob Friesenhahn                                %
%                                 July 2000                                   %
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
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/magic.h"
#include "magick/memory_.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define MagicFilename  "magic.xml"
#define MagickString(magic)  (const unsigned char *) (magic), sizeof(magic)-1

/*
  Typedef declarations.
*/
typedef struct _MagicMapInfo
{
  const char
    *name;

  const MagickOffsetType
    offset;

  const unsigned char
    *magic;

  const size_t
    length;
} MagicMapInfo;

/*
  Static declarations.
*/
static const MagicMapInfo
  MagicMap[] =
  {
    { "8BIMWTEXT", 0, MagickString("8\000B\000I\000M\000#") },
    { "8BIMTEXT", 0, MagickString("8BIM#") },
    { "8BIM", 0, MagickString("8BIM") },
    { "BMP", 0, MagickString("BA") },
    { "BMP", 0, MagickString("BM") },
    { "BMP", 0, MagickString("CI") },
    { "BMP", 0, MagickString("CP") },
    { "BMP", 0, MagickString("IC") },
    { "PICT", 0, MagickString("PICT") },
    { "BMP", 0, MagickString("PI") },
    { "CALS", 21, MagickString("version: MIL-STD-1840") },
    { "CALS", 0, MagickString("srcdocid:") },
    { "CALS", 9, MagickString("srcdocid:") },
    { "CALS", 8, MagickString("rorient:") },
    { "CGM", 0, MagickString("BEGMF") },
    { "CIN", 0, MagickString("\200\052\137\327") },
    { "CRW", 0, MagickString("II\x1a\x00\x00\x00HEAPCCDR") },
    { "DCM", 128, MagickString("DICM") },
    { "DCX", 0, MagickString("\261\150\336\72") },
    { "DIB", 0, MagickString("\050\000") },
    { "DDS", 0, MagickString("DDS ") },
    { "DJVU", 0, MagickString("AT&TFORM") },
    { "DOT", 0, MagickString("digraph") },
    { "DPX", 0, MagickString("SDPX") },
    { "DPX", 0, MagickString("XPDS") },
    { "EMF", 40, MagickString("\040\105\115\106\000\000\001\000") },
    { "EPT", 0, MagickString("\305\320\323\306") },
    { "EXR", 0, MagickString("\166\057\061\001") },
    { "FAX", 0, MagickString("DFAX") },
    { "FIG", 0, MagickString("#FIG") },
    { "FITS", 0, MagickString("IT0") },
    { "FITS", 0, MagickString("SIMPLE") },
    { "FPX", 0, MagickString("\320\317\021\340") },
    { "GIF", 0, MagickString("GIF8") },
    { "GPLT", 0, MagickString("#!/usr/local/bin/gnuplot") },
    { "HDF", 1, MagickString("HDF") },
    { "HDR", 0, MagickString("#?RADIANCE") },
    { "HDR", 0, MagickString("#?RGBE") },
    { "HPGL", 0, MagickString("IN;") },
    { "HTML", 1, MagickString("HTML") },
    { "HTML", 1, MagickString("html") },
    { "ILBM", 8, MagickString("ILBM") },
    { "IPTCWTEXT", 0, MagickString("\062\000#\000\060\000=\000\042\000&\000#\000\060\000;\000&\000#\000\062\000;\000\042\000") },
    { "IPTCTEXT", 0, MagickString("2#0=\042&#0;&#2;\042") },
    { "IPTC", 0, MagickString("\034\002") },
    { "JNG", 0, MagickString("\213JNG\r\n\032\n") },
    { "JPEG", 0, MagickString("\377\330\377") },
    { "JPC", 0, MagickString("\377\117") },
    { "JP2", 4, MagickString("\152\120\040\040\015") },
    { "MAT", 0, MagickString("MATLAB 5.0 MAT-file,") },
    { "MIFF", 0, MagickString("Id=ImageMagick") },
    { "MIFF", 0, MagickString("id=ImageMagick") },
    { "MNG", 0, MagickString("\212MNG\r\n\032\n") },
    { "MPC", 0, MagickString("id=MagickCache") },
    { "MPEG", 0, MagickString("\000\000\001\263") },
    { "MRW", 0, MagickString("\x00MRM") },
    { "MVG", 0, MagickString("push graphic-context") },
    { "ORF", 0, MagickString("IIRO\x08\x00\x00\x00") },
    { "PCD", 2048, MagickString("PCD_") },
    { "PCL", 0, MagickString("\033E\033") },
    { "PCX", 0, MagickString("\012\002") },
    { "PCX", 0, MagickString("\012\005") },
    { "PDB", 60, MagickString("vIMGView") },
    { "PDF", 0, MagickString("%PDF-") },
    { "PES", 0, MagickString("#PES") },
    { "PFA", 0, MagickString("%!PS-AdobeFont-1.0") },
    { "PFB", 6, MagickString("%!PS-AdobeFont-1.0") },
    { "PGX", 0, MagickString("\050\107\020\115\046") },
    { "PICT", 522, MagickString("\000\021\002\377\014\000") },
    { "PNG", 0, MagickString("\211PNG\r\n\032\n") },
    { "PBM", 0, MagickString("P1") },
    { "PGM", 0, MagickString("P2") },
    { "PPM", 0, MagickString("P3") },
    { "PBM", 0, MagickString("P4") },
    { "PGM", 0, MagickString("P5") },
    { "PPM", 0, MagickString("P6") },
    { "PAM", 0, MagickString("P7") },
    { "PFM", 0, MagickString("PF") },
    { "PFM", 0, MagickString("Pf") },
    { "PS", 0, MagickString("%!") },
    { "PS", 0, MagickString("\004%!") },
    { "PS", 0, MagickString("\305\320\323\306") },
    { "PSB", 0, MagickString("8BPB") },
    { "PSD", 0, MagickString("8BPS") },
    { "PWP", 0, MagickString("SFW95") },
    { "RAF", 0, MagickString("FUJIFILMCCD-RAW ") },
    { "RLE", 0, MagickString("\122\314") },
    { "SCT", 0, MagickString("CT") },
    { "SFW", 0, MagickString("SFW94") },
    { "SGI", 0, MagickString("\001\332") },
    { "SUN", 0, MagickString("\131\246\152\225") },
    { "SVG", 1, MagickString("?XML") },
    { "SVG", 1, MagickString("?xml") },
    { "TIFF", 0, MagickString("\115\115\000\052") },
    { "TIFF", 0, MagickString("\111\111\052\000") },
    { "TIFF64", 0, MagickString("\115\115\000\053\000\010\000\000") },
    { "TIFF64", 0, MagickString("\111\111\053\000\010\000\000\000") },
    { "TTF", 0, MagickString("\000\001\000\000\000") },
    { "TXT", 0, MagickString("# ImageMagick pixel enumeration:") },
    { "VICAR", 0, MagickString("LBLSIZE") },
    { "VICAR", 0, MagickString("NJPL1I") },
    { "VIFF", 0, MagickString("\253\001") },
    { "WEBP", 8, MagickString("WEBP") },
    { "WMF", 0, MagickString("\327\315\306\232") },
    { "WMF", 0, MagickString("\001\000\011\000") },
    { "WPG", 0, MagickString("\377WPC") },
    { "XBM", 0, MagickString("#define") },
    { "XCF", 0, MagickString("gimp xcf") },
    { "XEF", 0, MagickString("FOVb") },
    { "XPM", 1, MagickString("* XPM *") },
    { "XWD", 4, MagickString("\007\000\000") },
    { "XWD", 5, MagickString("\000\000\007") }
 };

static LinkedListInfo
  *magic_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *magic_semaphore = (SemaphoreInfo *) NULL;

static volatile MagickBooleanType
  instantiate_magic = MagickFalse;

/*
  Forward declarations.
*/
static MagickBooleanType
  InitializeMagicList(ExceptionInfo *),
  LoadMagicLists(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicInfo() searches the magic list for the specified name and if found
%  returns attributes for that magic.
%
%  The format of the GetMagicInfo method is:
%
%      const MagicInfo *GetMagicInfo(const unsigned char *magic,
%        const size_t length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o magic: A binary string generally representing the first few characters
%      of the image file or blob.
%
%    o length: the length of the binary signature.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const MagicInfo *GetMagicInfo(const unsigned char *magic,
  const size_t length,ExceptionInfo *exception)
{
  register const MagicInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if ((magic_list == (LinkedListInfo *) NULL) ||
      (instantiate_magic == MagickFalse))
    if (InitializeMagicList(exception) == MagickFalse)
      return((const MagicInfo *) NULL);
  if ((magic_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(magic_list) != MagickFalse))
    return((const MagicInfo *) NULL);
  if (magic == (const unsigned char *) NULL)
    return((const MagicInfo *) GetValueFromLinkedList(magic_list,0));
  if (length == 0)
    return((const MagicInfo *) NULL);
  /*
    Search for magic tag.
  */
  LockSemaphoreInfo(magic_semaphore);
  ResetLinkedListIterator(magic_list);
  p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  while (p != (const MagicInfo *) NULL)
  {
    assert(p->offset >= 0);
    if (((size_t) (p->offset+p->length) <= length) &&
        (memcmp(magic+p->offset,p->magic,p->length) == 0))
      break;
    p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  }
  if (p != (const MagicInfo *) NULL)
    (void) InsertValueInLinkedList(magic_list,0,
      RemoveElementByValueFromLinkedList(magic_list,p));
  UnlockSemaphoreInfo(magic_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c I n f o L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicInfoList() returns any image aliases that match the specified
%  pattern.
%
%  The magic of the GetMagicInfoList function is:
%
%      const MagicInfo **GetMagicInfoList(const char *pattern,
%        size_t *number_aliases,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of aliases in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MagicInfoCompare(const void *x,const void *y)
{
  const MagicInfo
    **p,
    **q;

  p=(const MagicInfo **) x,
  q=(const MagicInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->name,(*q)->name));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport const MagicInfo **GetMagicInfoList(const char *pattern,
  size_t *number_aliases,ExceptionInfo *exception)
{
  const MagicInfo
    **aliases;

  register const MagicInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate magic list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_aliases != (size_t *) NULL);
  *number_aliases=0;
  p=GetMagicInfo((const unsigned char *) NULL,0,exception);
  if (p == (const MagicInfo *) NULL)
    return((const MagicInfo **) NULL);
  aliases=(const MagicInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(magic_list)+1UL,sizeof(*aliases));
  if (aliases == (const MagicInfo **) NULL)
    return((const MagicInfo **) NULL);
  /*
    Generate magic list.
  */
  LockSemaphoreInfo(magic_semaphore);
  ResetLinkedListIterator(magic_list);
  p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  for (i=0; p != (const MagicInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      aliases[i++]=p;
    p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  }
  UnlockSemaphoreInfo(magic_semaphore);
  qsort((void *) aliases,(size_t) i,sizeof(*aliases),MagicInfoCompare);
  aliases[i]=(MagicInfo *) NULL;
  *number_aliases=(size_t) i;
  return(aliases);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicList() returns any image format aliases that match the specified
%  pattern.
%
%  The format of the GetMagicList function is:
%
%      char **GetMagicList(const char *pattern,size_t *number_aliases,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of image format aliases
%      in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MagicCompare(const void *x,const void *y)
{
  register const char
    *p,
    *q;

  p=(const char *) x;
  q=(const char *) y;
  return(LocaleCompare(p,q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport char **GetMagicList(const char *pattern,
  size_t *number_aliases,ExceptionInfo *exception)
{
  char
    **aliases;

  register const MagicInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate configure list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_aliases != (size_t *) NULL);
  *number_aliases=0;
  p=GetMagicInfo((const unsigned char *) NULL,0,exception);
  if (p == (const MagicInfo *) NULL)
    return((char **) NULL);
  aliases=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(magic_list)+1UL,sizeof(*aliases));
  if (aliases == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(magic_semaphore);
  ResetLinkedListIterator(magic_list);
  p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  for (i=0; p != (const MagicInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      aliases[i++]=ConstantString(p->name);
    p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  }
  UnlockSemaphoreInfo(magic_semaphore);
  qsort((void *) aliases,(size_t) i,sizeof(*aliases),MagicCompare);
  aliases[i]=(char *) NULL;
  *number_aliases=(size_t) i;
  return(aliases);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c N a m e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagicName() returns the name associated with the magic.
%
%  The format of the GetMagicName method is:
%
%      const char *GetMagicName(const MagicInfo *magic_info)
%
%  A description of each parameter follows:
%
%    o magic_info:  The magic info.
%
*/
MagickExport const char *GetMagicName(const MagicInfo *magic_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(magic_info != (MagicInfo *) NULL);
  assert(magic_info->signature == MagickSignature);
  return(magic_info->name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e M a g i c L i s t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeMagicList() initializes the magic list.
%
%  The format of the InitializeMagicList method is:
%
%      MagickBooleanType InitializeMagicList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType InitializeMagicList(ExceptionInfo *exception)
{
  if ((magic_list == (LinkedListInfo *) NULL) &&
      (instantiate_magic == MagickFalse))
    {
      if (magic_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&magic_semaphore);
      LockSemaphoreInfo(magic_semaphore);
      if ((magic_list == (LinkedListInfo *) NULL) &&
          (instantiate_magic == MagickFalse))
        {
          (void) LoadMagicLists(MagicFilename,exception);
          instantiate_magic=MagickTrue;
        }
      UnlockSemaphoreInfo(magic_semaphore);
    }
  return(magic_list != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t M a g i c I n f o                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMagicInfo() lists the magic info to a file.
%
%  The format of the ListMagicInfo method is:
%
%      MagickBooleanType ListMagicInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListMagicInfo(FILE *file,
  ExceptionInfo *exception)
{
  const char
    *path;

  const MagicInfo
    **magic_info;

  register ssize_t
    i;

  size_t
    number_aliases;

  ssize_t
    j;

  if (file == (const FILE *) NULL)
    file=stdout;
  magic_info=GetMagicInfoList("*",&number_aliases,exception);
  if (magic_info == (const MagicInfo **) NULL)
    return(MagickFalse);
  j=0;
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_aliases; i++)
  {
    if (magic_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,magic_info[i]->path) != 0))
      {
        if (magic_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",magic_info[i]->path);
        (void) FormatLocaleFile(file,"Name      Offset Target\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=magic_info[i]->path;
    (void) FormatLocaleFile(file,"%s",magic_info[i]->name);
    for (j=(ssize_t) strlen(magic_info[i]->name); j <= 9; j++)
      (void) FormatLocaleFile(file," ");
    (void) FormatLocaleFile(file,"%6ld ",(long) magic_info[i]->offset);
    if (magic_info[i]->target != (char *) NULL)
      {
        register ssize_t
          j;

        for (j=0; magic_info[i]->target[j] != '\0'; j++)
          if (isprint((int) ((unsigned char) magic_info[i]->target[j])) != 0)
            (void) FormatLocaleFile(file,"%c",magic_info[i]->target[j]);
          else
            (void) FormatLocaleFile(file,"\\%03o",(unsigned int)
              ((unsigned char) magic_info[i]->target[j]));
      }
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  magic_info=(const MagicInfo **) RelinquishMagickMemory((void *) magic_info);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d M a g i c L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadMagicList() loads the magic configuration file which provides a mapping
%  between magic attributes and a magic name.
%
%  The format of the LoadMagicList method is:
%
%      MagickBooleanType LoadMagicList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml: The magic list in XML format.
%
%    o filename: The magic list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadMagicList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MaxTextExtent],
    *token;

  const char
    *q;

  MagickBooleanType
    status;

  MagicInfo
    *magic_info;

  /*
    Load the magic map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading magic configure file \"%s\" ...",filename);
  if (xml == (char *) NULL)
    return(MagickFalse);
  if (magic_list == (LinkedListInfo *) NULL)
    {
      magic_list=NewLinkedList(0);
      if (magic_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  magic_info=(MagicInfo *) NULL;
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
                  ConfigureError,"IncludeElementNestedTooDeeply","`%s'",token);
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
                      status=LoadMagicList(xml,path,depth+1,exception);
                      xml=(char *) RelinquishMagickMemory(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<magic") == 0)
      {
        /*
          Magic element.
        */
        magic_info=(MagicInfo *) AcquireMagickMemory(sizeof(*magic_info));
        if (magic_info == (MagicInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(magic_info,0,sizeof(*magic_info));
        magic_info->path=ConstantString(filename);
        magic_info->exempt=MagickFalse;
        magic_info->signature=MagickSignature;
        continue;
      }
    if (magic_info == (MagicInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        status=AppendValueToLinkedList(magic_list,magic_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            magic_info->name);
        magic_info=(MagicInfo *) NULL;
      }
    GetMagickToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetMagickToken(q,&q,token);
    GetMagickToken(q,&q,token);
    switch (*keyword)
    {
      case 'N':
      case 'n':
      {
        if (LocaleCompare((char *) keyword,"name") == 0)
          {
            magic_info->name=ConstantString(token);
            break;
          }
        break;
      }
      case 'O':
      case 'o':
      {
        if (LocaleCompare((char *) keyword,"offset") == 0)
          {
            magic_info->offset=(MagickOffsetType) StringToLong(token);
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            magic_info->stealth=IsMagickTrue(token);
            break;
          }
        break;
      }
      case 'T':
      case 't':
      {
        if (LocaleCompare((char *) keyword,"target") == 0)
          {
            char
              *p;

            register unsigned char
              *q;

            size_t
              length;

            length=strlen(token);
            magic_info->target=ConstantString(token);
            magic_info->magic=(unsigned char *) ConstantString(token);
            q=magic_info->magic;
            for (p=magic_info->target; *p != '\0'; )
            {
              if (*p == '\\')
                {
                  p++;
                  if (isdigit((int) ((unsigned char) *p)) != 0)
                    {
                      char
                        *end;

                      *q++=(unsigned char) strtol(p,&end,8);
                      p+=(end-p);
                      magic_info->length++;
                      continue;
                    }
                  switch (*p)
                  {
                    case 'b': *q='\b'; break;
                    case 'f': *q='\f'; break;
                    case 'n': *q='\n'; break;
                    case 'r': *q='\r'; break;
                    case 't': *q='\t'; break;
                    case 'v': *q='\v'; break;
                    case 'a': *q='a'; break;
                    case '?': *q='\?'; break;
                    default: *q=(unsigned char) (*p); break;
                  }
                  p++;
                  q++;
                  magic_info->length++;
                  continue;
                }
              else
                if (LocaleNCompare(p,"&amp;",5) == 0)
                  (void) CopyMagickString(p+1,p+5,length-magic_info->length);
              *q++=(unsigned char) (*p++);
              magic_info->length++;
            }
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
%  L o a d M a g i c L i s t s                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadMagicLists() loads one or more magic configuration file which provides a
%  mapping between magic attributes and a magic name.
%
%  The format of the LoadMagicLists method is:
%
%      MagickBooleanType LoadMagicLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadMagicLists(const char *filename,
  ExceptionInfo *exception)
{
  char
    path[MaxTextExtent];

  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  register ssize_t
    i;

  /*
    Load built-in magic map.
  */
  status=MagickFalse;
  if (magic_list == (LinkedListInfo *) NULL)
    {
      magic_list=NewLinkedList(0);
      if (magic_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  for (i=0; i < (ssize_t) (sizeof(MagicMap)/sizeof(*MagicMap)); i++)
  {
    MagicInfo
      *magic_info;

    register const MagicMapInfo
      *p;

    p=MagicMap+i;
    magic_info=(MagicInfo *) AcquireMagickMemory(sizeof(*magic_info));
    if (magic_info == (MagicInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",magic_info->name);
        continue;
      }
    (void) ResetMagickMemory(magic_info,0,sizeof(*magic_info));
    magic_info->path=(char *) "[built-in]";
    magic_info->name=(char *) p->name;
    magic_info->offset=p->offset;
    magic_info->target=(char *) p->magic;
    magic_info->magic=(unsigned char *) p->magic;
    magic_info->length=p->length;
    magic_info->exempt=MagickTrue;
    magic_info->signature=MagickSignature;
    status=AppendValueToLinkedList(magic_list,magic_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",magic_info->name);
  }
  /*
    Load external magic map.
  */
  *path='\0';
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    (void) CopyMagickString(path,GetStringInfoPath(option),MaxTextExtent);
    status|=LoadMagicList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c C o m p o n e n t G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagicComponentGenesis() instantiates the magic component.
%
%  The format of the MagicComponentGenesis method is:
%
%      MagickBooleanType MagicComponentGenesis(void)
%
*/
MagickExport MagickBooleanType MagicComponentGenesis(void)
{
  AcquireSemaphoreInfo(&magic_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c C o m p o n e n t T e r m i n u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagicComponentTerminus() destroys the magic component.
%
%  The format of the MagicComponentTerminus method is:
%
%      MagicComponentTerminus(void)
%
*/

static void *DestroyMagicElement(void *magic_info)
{
  register MagicInfo
    *p;

  p=(MagicInfo *) magic_info;
  if (p->exempt == MagickFalse)
    {
      if (p->path != (char *) NULL)
        p->path=DestroyString(p->path);
      if (p->name != (char *) NULL)
        p->name=DestroyString(p->name);
      if (p->target != (char *) NULL)
        p->target=DestroyString(p->target);
      if (p->magic != (unsigned char *) NULL)
        p->magic=(unsigned char *) RelinquishMagickMemory(p->magic);
    }
  p=(MagicInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickExport void MagicComponentTerminus(void)
{
  if (magic_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&magic_semaphore);
  LockSemaphoreInfo(magic_semaphore);
  if (magic_list != (LinkedListInfo *) NULL)
    magic_list=DestroyLinkedList(magic_list,DestroyMagicElement);
  instantiate_magic=MagickFalse;
  UnlockSemaphoreInfo(magic_semaphore);
  DestroySemaphoreInfo(&magic_semaphore);
}

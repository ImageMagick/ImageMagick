/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define MagicFilename  "magic.xml"

/*
  Static declarations.
*/
typedef struct _MagicMapInfo
{
  const char
    *name;

  unsigned char
    *magic;

  size_t
    length;

  MagickOffsetType
    offset;
} MagicMapInfo;

#define MagicTransform(name,offset,magic) \
   { name, (unsigned char *) magic, sizeof(magic)-1, offset }

static const MagicMapInfo
  MagicMap[] =
  {
    MagicTransform("AVI", 0, "RIFF"),
    MagicTransform("8BIMWTEXT", 0, "8\000B\000I\000M\000#"),
    MagicTransform("8BIMTEXT", 0, "8BIM#"),
    MagicTransform("8BIM", 0, "8BIM"),
    MagicTransform("BMP", 0, "BA"),
    MagicTransform("BMP", 0, "BM"),
    MagicTransform("BMP", 0, "CI"),
    MagicTransform("BMP", 0, "CP"),
    MagicTransform("BMP", 0, "IC"),
    MagicTransform("BMP", 0, "PI"),
    MagicTransform("CALS", 21, "version: MIL-STD-1840"),
    MagicTransform("CALS", 0, "srcdocid:"),
    MagicTransform("CALS", 9, "srcdocid:"),
    MagicTransform("CALS", 8, "rorient:"),
    MagicTransform("CGM", 0, "BEGMF"),
    MagicTransform("CIN", 0, "\200\052\137\327"),
    MagicTransform("CRW", 0, "II\x1a\x00\x00\x00HEAPCCDR"),
    MagicTransform("DCM", 128, "DICM"),
    MagicTransform("DCX", 0, "\261\150\336\72"),
    MagicTransform("DIB", 0, "\050\000"),
    MagicTransform("DDS", 0, "DDS "),
    MagicTransform("DJVU", 0, "AT&TFORM"),
    MagicTransform("DOT", 0, "digraph"),
    MagicTransform("DPX", 0, "SDPX"),
    MagicTransform("DPX", 0, "XPDS"),
    MagicTransform("EMF", 40, "\040\105\115\106\000\000\001\000"),
    MagicTransform("EPT", 0, "\305\320\323\306"),
    MagicTransform("EXR", 0, "\166\057\061\001"),
    MagicTransform("FAX", 0, "DFAX"),
    MagicTransform("FIG", 0, "#FIG"),
    MagicTransform("FITS", 0, "IT0"),
    MagicTransform("FITS", 0, "SIMPLE"),
    MagicTransform("FPX", 0, "\320\317\021\340"),
    MagicTransform("GIF", 0, "GIF8"),
    MagicTransform("GPLT", 0, "#!/usr/local/bin/gnuplot"),
    MagicTransform("HDF", 1, "HDF"),
    MagicTransform("HPGL", 0, "IN;"),
    MagicTransform("HPGL", 0, "\033E\033"),
    MagicTransform("HTML", 1, "HTML"),
    MagicTransform("HTML", 1, "html"),
    MagicTransform("ILBM", 8, "ILBM"),
    MagicTransform("IPTCWTEXT", 0, "\062\000#\000\060\000=\000\042\000&\000#\000\060\000;\000&\000#\000\062\000;\000\042\000"),
    MagicTransform("IPTCTEXT", 0, "2#0=\042&#0;&#2;\042"),
    MagicTransform("IPTC", 0, "\034\002"),
    MagicTransform("JNG", 0, "\213JNG\r\n\032\n"),
    MagicTransform("JPEG", 0, "\377\330\377"),
    MagicTransform("JPC", 0, "\377\117"),
    MagicTransform("JP2", 4, "\152\120\040\040\015"),
    MagicTransform("MIFF", 0, "Id=ImageMagick"),
    MagicTransform("MIFF", 0, "id=ImageMagick"),
    MagicTransform("MNG", 0, "\212MNG\r\n\032\n"),
    MagicTransform("MPC", 0, "id=MagickCache"),
    MagicTransform("MPEG", 0, "\000\000\001\263"),
    MagicTransform("MRW", 0, "\x00MRM"),
    MagicTransform("MVG", 0, "push graphic-context"),
    MagicTransform("ORF", 0, "IIRO\x08\x00\x00\x00"),
    MagicTransform("PCD", 2048, "PCD_"),
    MagicTransform("PCL", 0, "\033E\033"),
    MagicTransform("PCX", 0, "\012\002"),
    MagicTransform("PCX", 0, "\012\005"),
    MagicTransform("PDB", 60, "vIMGView"),
    MagicTransform("PDF", 0, "%PDF-"),
    MagicTransform("PFA", 0, "%!PS-AdobeFont-1.0"),
    MagicTransform("PFB", 6, "%!PS-AdobeFont-1.0"),
    MagicTransform("PGX", 0, "\050\107\020\115\046"),
    MagicTransform("PICT", 522, "\000\021\002\377\014\000"),
    MagicTransform("PNG", 0, "\211PNG\r\n\032\n"),
    MagicTransform("PNM", 0, "P1"),
    MagicTransform("PNM", 0, "P2"),
    MagicTransform("PNM", 0, "P3"),
    MagicTransform("PNM", 0, "P4"),
    MagicTransform("PNM", 0, "P5"),
    MagicTransform("PNM", 0, "P6"),
    MagicTransform("PNM", 0, "P7"),
    MagicTransform("PNM", 0, "PF"),
    MagicTransform("PNM", 0, "Pf"),
    MagicTransform("PS", 0, "%!"),
    MagicTransform("PS", 0, "\004%!"),
    MagicTransform("PS", 0, "\305\320\323\306"),
    MagicTransform("PSD", 0, "8BPS"),
    MagicTransform("PWP", 0, "SFW95"),
    MagicTransform("RAF", 0, "FUJIFILMCCD-RAW "),
    MagicTransform("RAD", 0, "#?RADIANCE"),
    MagicTransform("RAD", 0, "VIEW= "),
    MagicTransform("RLE", 0, "\122\314"),
    MagicTransform("SCT", 0, "CT"),
    MagicTransform("SFW", 0, "SFW94"),
    MagicTransform("SGI", 0, "\001\332"),
    MagicTransform("SUN", 0, "\131\246\152\225"),
    MagicTransform("SVG", 1, "?XML"),
    MagicTransform("SVG", 1, "?xml"),
    MagicTransform("TIFF", 0, "\115\115\000\052"),
    MagicTransform("TIFF", 0, "\111\111\052\000"),
    MagicTransform("TIFF64", 0, "\115\115\000\053\000\010\000\000"),
    MagicTransform("TIFF64", 0, "\111\111\053\000\010\000\000\000"),
    MagicTransform("TXT", 0, "# ImageMagick pixel enumeration:"),
    MagicTransform("VICAR", 0, "LBLSIZE"),
    MagicTransform("VICAR", 0, "NJPL1I"),
    MagicTransform("VIFF", 0, "\253\001"),
    MagicTransform("WMF", 0, "\327\315\306\232"),
    MagicTransform("WMF", 0, "\001\000\011\000"),
    MagicTransform("WPG", 0, "\377WPC"),
    MagicTransform("XBM", 0, "#define"),
    MagicTransform("XCF", 0, "gimp xcf"),
    MagicTransform("XEF", 0, "FOVb"),
    MagicTransform("XPM", 1, "* XPM *"),
    MagicTransform("XWD", 4, "\007\000\000"),
    MagicTransform("XWD", 5, "\000\000\007"),
    MagicTransform((const char *) NULL, 0, (const char *) NULL)
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
+   D e s t r o y M a g i c L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMagicList() deallocates memory associated with the magic list.
%
%  The format of the DestroyMagicList method is:
%
%      DestroyMagicList(void)
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

MagickExport void DestroyMagicList(void)
{
  AcquireSemaphoreInfo(&magic_semaphore);
  if (magic_list != (LinkedListInfo *) NULL)
    magic_list=DestroyLinkedList(magic_list,DestroyMagicElement);
  instantiate_magic=MagickFalse;
  RelinquishSemaphoreInfo(magic_semaphore);
  DestroySemaphoreInfo(&magic_semaphore);
}

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
  AcquireSemaphoreInfo(&magic_semaphore);
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
  RelinquishSemaphoreInfo(magic_semaphore);
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
%        unsigned long *number_aliases,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of aliases in the
%      list.
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
  unsigned long *number_aliases,ExceptionInfo *exception)
{
  const MagicInfo
    **aliases;

  register const MagicInfo
    *p;

  register long
    i;

  /*
    Allocate magic list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_aliases != (unsigned long *) NULL);
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
  AcquireSemaphoreInfo(&magic_semaphore);
  ResetLinkedListIterator(magic_list);
  p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  for (i=0; p != (const MagicInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      aliases[i++]=p;
    p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  }
  RelinquishSemaphoreInfo(magic_semaphore);
  qsort((void *) aliases,(size_t) i,sizeof(*aliases),MagicInfoCompare);
  aliases[i]=(MagicInfo *) NULL;
  *number_aliases=(unsigned long) i;
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
%      char **GetMagicList(const char *pattern,unsigned long *number_aliases,
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
  unsigned long *number_aliases,ExceptionInfo *exception)
{
  char
    **aliases;

  register const MagicInfo
    *p;

  register long
    i;

  /*
    Allocate configure list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_aliases != (unsigned long *) NULL);
  *number_aliases=0;
  p=GetMagicInfo((const unsigned char *) NULL,0,exception);
  if (p == (const MagicInfo *) NULL)
    return((char **) NULL);
  aliases=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(magic_list)+1UL,sizeof(*aliases));
  if (aliases == (char **) NULL)
    return((char **) NULL);
  AcquireSemaphoreInfo(&magic_semaphore);
  ResetLinkedListIterator(magic_list);
  p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  for (i=0; p != (const MagicInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      aliases[i++]=ConstantString(p->name);
    p=(const MagicInfo *) GetNextValueInLinkedList(magic_list);
  }
  RelinquishSemaphoreInfo(magic_semaphore);
  qsort((void *) aliases,(size_t) i,sizeof(*aliases),MagicCompare);
  aliases[i]=(char *) NULL;
  *number_aliases=(unsigned long) i;
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
      AcquireSemaphoreInfo(&magic_semaphore);
      if ((magic_list == (LinkedListInfo *) NULL) &&
          (instantiate_magic == MagickFalse))
        {
          (void) LoadMagicLists(MagicFilename,exception);
          instantiate_magic=MagickTrue;
        }
      RelinquishSemaphoreInfo(magic_semaphore);
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

  long
    j;

  register long
    i;

  unsigned long
    number_aliases;

  if (file == (const FILE *) NULL)
    file=stdout;
  magic_info=GetMagicInfoList("*",&number_aliases,exception);
  if (magic_info == (const MagicInfo **) NULL)
    return(MagickFalse);
  j=0;
  path=(const char *) NULL;
  for (i=0; i < (long) number_aliases; i++)
  {
    if (magic_info[i]->stealth != MagickFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,magic_info[i]->path) != 0))
      {
        if (magic_info[i]->path != (char *) NULL)
          (void) fprintf(file,"\nPath: %s\n\n",magic_info[i]->path);
        (void) fprintf(file,"Name      Offset Target\n");
        (void) fprintf(file,"-------------------------------------------------"
          "------------------------------\n");
      }
    path=magic_info[i]->path;
    (void) fprintf(file,"%s",magic_info[i]->name);
    for (j=(long) strlen(magic_info[i]->name); j <= 9; j++)
      (void) fprintf(file," ");
    (void) fprintf(file,"%6ld ",(long) magic_info[i]->offset);
    if (magic_info[i]->target != (char *) NULL)
      {
        register long
          j;

        for (j=0; magic_info[i]->target[j] != '\0'; j++)
          if (isprint((int) ((unsigned char) magic_info[i]->target[j])) != 0)
            (void) fprintf(file,"%c",magic_info[i]->target[j]);
          else
            (void) fprintf(file,"\\%03o",(unsigned int)
              ((unsigned char) magic_info[i]->target[j]));
      }
    (void) fprintf(file,"\n");
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
%        const unsigned long depth,ExceptionInfo *exception)
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
  const unsigned long depth,ExceptionInfo *exception)
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
    magic_list=NewLinkedList(0);
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
            magic_info->offset=(MagickOffsetType) atol(token);
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
#if defined(MAGICKCORE_EMBEDDABLE_SUPPORT)
  return(LoadMagicList(MagicMap,"built-in",0,exception));
#else
  char
    path[MaxTextExtent];

  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  register const MagicMapInfo
    *p;

  /*
    Load built-in magic map.
  */
  status=MagickFalse;
  if (magic_list == (LinkedListInfo *) NULL)
    magic_list=NewLinkedList(0);
  for (p=MagicMap; p->name != (const char *) NULL; p++)
  {
    MagicInfo
      *magic_info;

    magic_info=(MagicInfo *) AcquireMagickMemory(sizeof(*magic_info));
    if (magic_info == (MagicInfo *) NULL)
      ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
    (void) ResetMagickMemory(magic_info,0,sizeof(*magic_info));
    magic_info->path="[built-in]";
    magic_info->name=(char *) p->name;
    magic_info->offset=p->offset;
    magic_info->target=(char *) p->magic;
    magic_info->magic=p->magic;
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
#endif
}

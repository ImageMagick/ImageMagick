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
static const char
  *MagicMap = (const char *)
    "<?xml version=\"1.0\"?>"
    "<magicmap>"
    "  <magic name=\"AVI\" offset=\"0\" target=\"RIFF\" />"
    "  <magic name=\"8BIMWTEXT\" offset=\"0\" target=\"8\\000B\\000I\\000M\\000#\" />"
    "  <magic name=\"8BIMTEXT\" offset=\"0\" target=\"8BIM#\" />"
    "  <magic name=\"8BIM\" offset=\"0\" target=\"8BIM\" />"
    "  <magic name=\"BMP\" offset=\"0\" target=\"BA\" />"
    "  <magic name=\"BMP\" offset=\"0\" target=\"BM\" />"
    "  <magic name=\"BMP\" offset=\"0\" target=\"CI\" />"
    "  <magic name=\"BMP\" offset=\"0\" target=\"CP\" />"
    "  <magic name=\"BMP\" offset=\"0\" target=\"IC\" />"
    "  <magic name=\"BMP\" offset=\"0\" target=\"PI\" />"
    "  <magic name=\"CIN\" offset=\"0\" target=\"\\200\\052\\137\\327\" />"
    "  <magic name=\"CGM\" offset=\"0\" target=\"BEGMF\" />"
    "  <magic name=\"DCM\" offset=\"128\" target=\"DICM\" />"
    "  <magic name=\"DCX\" offset=\"0\" target=\"\\261\\150\\336\\72\" />"
    "  <magic name=\"DDS\" offset=\"0\" target=\"DDS \" />"
    "  <magic name=\"DIB\" offset=\"0\" target=\"\\050\\000\" />"
    "  <magic name=\"DJVU\" offset=\"0\" target=\"AT&TFORM\" />"
    "  <magic name=\"DOT\" offset=\"0\" target=\"digraph\" />"
    "  <magic name=\"DPX\" offset=\"0\" target=\"SDPX\" />"
    "  <magic name=\"DPX\" offset=\"0\" target=\"XPDS\" />"
    "  <magic name=\"EMF\" offset=\"40\" target=\"\\040\\105\\115\\106\\000\\000\\001\\000\" />"
    "  <magic name=\"EPT\" offset=\"0\" target=\"\\305\\320\\323\\306\" />"
    "  <magic name=\"EXR\" offset=\"0\" target=\"\\166\\057\\061\\001\" />"
    "  <magic name=\"FAX\" offset=\"0\" target=\"DFAX\" />"
    "  <magic name=\"FIG\" offset=\"0\" target=\"#FIG\" />"
    "  <magic name=\"FITS\" offset=\"0\" target=\"IT0\" />"
    "  <magic name=\"FITS\" offset=\"0\" target=\"SIMPLE\" />"
    "  <magic name=\"FPX\" offset=\"0\" target=\"\\320\\317\\021\\340\" />"
    "  <magic name=\"GIF\" offset=\"0\" target=\"GIF8\" />"
    "  <magic name=\"GPLT\" offset=\"0\" target=\"#!/usr/local/bin/gnuplot\" />"
    "  <magic name=\"HDF\" offset=\"1\" target=\"HDF\" />"
    "  <magic name=\"HPGL\" offset=\"0\" target=\"IN;\" />"
    "  <magic name=\"HTML\" offset=\"1\" target=\"HTML\" />"
    "  <magic name=\"HTML\" offset=\"1\" target=\"html\" />"
    "  <magic name=\"ILBM\" offset=\"8\" target=\"ILBM\" />"
    "  <magic name=\"IPTCWTEXT\" offset=\"0\" target=\"\\062\\000#\\000\\060\\000=\\000\\042\\000&\\000#\\000\\060\\000;\\000&\\000#\\000\\062\\000;\\000\\042\\000\" />"
    "  <magic name=\"IPTCTEXT\" offset=\"0\" target=\"2#0=\\042&#0;&#2;\\042\" />"
    "  <magic name=\"IPTC\" offset=\"0\" target=\"\\034\\002\" />"
    "  <magic name=\"JNG\" offset=\"0\" target=\"\\213JNG\\r\\n\\032\\n\" />"
    "  <magic name=\"JPEG\" offset=\"0\" target=\"\\377\\330\\377\" />"
    "  <magic name=\"JPC\" offset=\"0\" target=\"\\377\\117\" />"
    "  <magic name=\"JP2\" offset=\"4\" target=\"\\152\\120\\040\\040\\015\" />"
    "  <magic name=\"MIFF\" offset=\"0\" target=\"Id=ImageMagick\" />"
    "  <magic name=\"MIFF\" offset=\"0\" target=\"id=ImageMagick\" />"
    "  <magic name=\"MNG\" offset=\"0\" target=\"\\212MNG\\r\\n\\032\\n\" />"
    "  <magic name=\"MPC\" offset=\"0\" target=\"id=MagickCache\" />"
    "  <magic name=\"MPEG\" offset=\"0\" target=\"\\000\\000\\001\\263\" />"
    "  <magic name=\"MVG\" offset=\"0\" target=\"push graphic-context\" />"
    "  <magic name=\"PCD\" offset=\"2048\" target=\"PCD_\" />"
    "  <magic name=\"PCL\" offset=\"0\" target=\"\\033E\\033\" />"
    "  <magic name=\"PCX\" offset=\"0\" target=\"\\012\\002\" />"
    "  <magic name=\"PCX\" offset=\"0\" target=\"\\012\\005\" />"
    "  <magic name=\"PDB\" offset=\"60\" target=\"vIMGView\" />"
    "  <magic name=\"PDF\" offset=\"0\" target=\"%PDF-\" />"
    "  <magic name=\"PFA\" offset=\"0\" target=\"%!PS-AdobeFont-1.0\" />"
    "  <magic name=\"PFB\" offset=\"6\" target=\"%!PS-AdobeFont-1.0\" />"
    "  <magic name=\"PGX\" offset=\"0\" target=\"\\050\\107\\020\\115\\046\" />"
    "  <magic name=\"PICT\" offset=\"522\" target=\"\\000\\021\\002\\377\\014\\000\" />"
    "  <magic name=\"PNG\" offset=\"0\" target=\"\\211PNG\\r\\n\\032\\n\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"P1\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"P2\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"P3\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"P4\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"P5\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"P6\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"P7\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"PF\" />"
    "  <magic name=\"PNM\" offset=\"0\" target=\"Pf\" />"
    "  <magic name=\"PS\" offset=\"0\" target=\"%!\" />"
    "  <magic name=\"PS\" offset=\"0\" target=\"\\004%!\" />"
    "  <magic name=\"PS\" offset=\"0\" target=\"\\305\\320\\323\\306\" />"
    "  <magic name=\"PSD\" offset=\"0\" target=\"8BPS\" />"
    "  <magic name=\"PWP\" offset=\"0\" target=\"SFW95\" />"
    "  <magic name=\"RAD\" offset=\"0\" target=\"#?RADIANCE\" />"
    "  <magic name=\"RAD\" offset=\"0\" target=\"VIEW= \" />"
    "  <magic name=\"RLE\" offset=\"0\" target=\"\\122\\314\" />"
    "  <magic name=\"SCT\" offset=\"0\" target=\"CT\" />"
    "  <magic name=\"SFW\" offset=\"0\" target=\"SFW94\" />"
    "  <magic name=\"SGI\" offset=\"0\" target=\"\\001\\332\" />"
    "  <magic name=\"SUN\" offset=\"0\" target=\"\\131\\246\\152\\225\" />"
    "  <magic name=\"SVG\" offset=\"1\" target=\"?XML\" />"
    "  <magic name=\"SVG\" offset=\"1\" target=\"?xml\" />"
    "  <magic name=\"TXT\" offset=\"0\" target=\"# ImageMagick pixel enumeration:\" />"
    "  <magic name=\"TIFF\" offset=\"0\" target=\"\\115\\115\\000\\052\" />"
    "  <magic name=\"TIFF\" offset=\"0\" target=\"\\111\\111\\052\\000\" />"
    "  <magic name=\"TIFF64\" offset=\"0\" target=\"\\115\\115\\000\\053\\000\\010\\000\\000\" />"
    "  <magic name=\"TIFF64\" offset=\"0\" target=\"\\115\\115\\000\\053\\000\\010\\000\\000\" />"
    "  <magic name=\"VICAR\" offset=\"0\" target=\"LBLSIZE\" />"
    "  <magic name=\"VICAR\" offset=\"0\" target=\"NJPL1I\" />"
    "  <magic name=\"VIFF\" offset=\"0\" target=\"\\253\\001\" />"
    "  <magic name=\"WMF\" offset=\"0\" target=\"\\327\\315\\306\\232\" />"
    "  <magic name=\"WMF\" offset=\"0\" target=\"\\001\\000\\011\\000\" />"
    "  <magic name=\"WPG\" offset=\"0\" target=\"\\377WPC\" />"
    "  <magic name=\"XBM\" offset=\"0\" target=\"#define\" />"
    "  <magic name=\"XCF\" offset=\"0\" target=\"gimp xcf\" />"
    "  <magic name=\"XPM\" offset=\"1\" target=\"* XPM *\" />"
    "  <magic name=\"XWD\" offset=\"4\" target=\"\\007\\000\\000\" />"
    "  <magic name=\"XWD\" offset=\"5\" target=\"\\000\\000\\007\" />"
    "</magicmap>";

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
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->name != (char *) NULL)
    p->name=DestroyString(p->name);
  if (p->target != (char *) NULL)
    p->target=DestroyString(p->target);
  if (p->magic != (unsigned char *) NULL)
    p->magic=(unsigned char *) RelinquishMagickMemory(p->magic);
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
  p=GetMagicInfo((const unsigned char *) "*",0,exception);
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
  p=GetMagicInfo((const unsigned char *) "*",0,exception);
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
      (void) fprintf(file,"%s",magic_info[i]->target);
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
%    o xml:  The magic list in XML format.
%
%    o filename:  The magic list filename.
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

  status=MagickFalse;
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
  if ((magic_list == (LinkedListInfo *) NULL) || 
      (IsLinkedListEmpty(magic_list) != MagickFalse))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ConfigureWarning,
        "UnableToOpenConfigureFile","`%s'",path);
      status|=LoadMagicList(MagicMap,"built-in",0,exception);
    }
  return(status != 0 ? MagickTrue : MagickFalse);
#endif
}

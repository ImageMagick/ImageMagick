/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                  PPPP    OOO   L      IIIII   CCCC  Y   Y                   %
%                  P   P  O   O  L        I    C       Y Y                    %
%                  PPPP   O   O  L        I    C        Y                     %
%                  P      O   O  L        I    C        Y                     %
%                  P       OOO   LLLLL  IIIII   CCCC    Y                     %
%                                                                             %
%                                                                             %
%                         MagickCore Policy Methods                           %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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
%  We use linked-lists because splay-trees do not currently support duplicate
%  key / value pairs (.e.g X11 green compliance and SVG green compliance).
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/client.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/policy.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define PolicyFilename  "policy.xml"

/*
  Typedef declarations.
*/
struct _PolicyInfo
{
  char
    *path;

  PolicyDomain
    domain;

  PolicyRights
    rights;

  char
    *name,
    *pattern,
    *value;

  MagickBooleanType
    exempt,
    stealth,
    debug;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

typedef struct _PolicyMapInfo
{
  const PolicyDomain
    domain;

  const PolicyRights
    rights;

  const char
    *name,
    *pattern,
    *value;
} PolicyMapInfo;

/*
  Static declarations.
*/
static const PolicyMapInfo
  PolicyMap[] =
  {
    { UndefinedPolicyDomain, UndefinedPolicyRights, (const char *) NULL,
      (const char *) NULL, (const char *) NULL }
  };

static LinkedListInfo
  *policy_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *policy_semaphore = (SemaphoreInfo *) NULL;

static volatile MagickBooleanType
  instantiate_policy = MagickFalse;

/*
  Forward declarations.
*/
static MagickBooleanType
  InitializePolicyList(ExceptionInfo *),
  LoadPolicyLists(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P o l i c y I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPolicyInfo() searches the policy list for the specified name and if found
%  returns attributes for that policy.
%
%  The format of the GetPolicyInfo method is:
%
%      PolicyInfo *GetPolicyInfo(const char *name,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the policy name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static PolicyInfo *GetPolicyInfo(const char *name,ExceptionInfo *exception)
{
  char
    policyname[MaxTextExtent];

  register PolicyInfo
    *p;

  register char
    *q;

  assert(exception != (ExceptionInfo *) NULL);
  if ((policy_list == (LinkedListInfo *) NULL) ||
      (instantiate_policy == MagickFalse))
    if (InitializePolicyList(exception) == MagickFalse)
      return((PolicyInfo *) NULL);
  if ((policy_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(policy_list) != MagickFalse))
    return((PolicyInfo *) NULL);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    return((PolicyInfo *) GetValueFromLinkedList(policy_list,0));
  /*
    Strip names of whitespace.
  */
  (void) CopyMagickString(policyname,name,MaxTextExtent);
  for (q=policyname; *q != '\0'; q++)
  {
    if (isspace((int) ((unsigned char) *q)) == 0)
      continue;
    (void) CopyMagickString(q,q+1,MaxTextExtent);
    q--;
  }
  /*
    Search for policy tag.
  */
  LockSemaphoreInfo(policy_semaphore);
  ResetLinkedListIterator(policy_list);
  p=(PolicyInfo *) GetNextValueInLinkedList(policy_list);
  while (p != (PolicyInfo *) NULL)
  {
    if (LocaleCompare(policyname,p->name) == 0)
      break;
    p=(PolicyInfo *) GetNextValueInLinkedList(policy_list);
  }
  if (p != (PolicyInfo *) NULL)
    (void) InsertValueInLinkedList(policy_list,0,
      RemoveElementByValueFromLinkedList(policy_list,p));
  UnlockSemaphoreInfo(policy_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P o l i c y I n f o L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPolicyInfoList() returns any policies that match the specified pattern.
%
%  The format of the GetPolicyInfoList function is:
%
%      const PolicyInfo **GetPolicyInfoList(const char *pattern,
%        size_t *number_policies,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_policies:  returns the number of policies in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const PolicyInfo **GetPolicyInfoList(const char *pattern,
  size_t *number_policies,ExceptionInfo *exception)
{
  const PolicyInfo
    **policies;

  register const PolicyInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate policy list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_policies != (size_t *) NULL);
  *number_policies=0;
  p=GetPolicyInfo("*",exception);
  if (p == (const PolicyInfo *) NULL)
    return((const PolicyInfo **) NULL);
  policies=(const PolicyInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(policy_list)+1UL,sizeof(*policies));
  if (policies == (const PolicyInfo **) NULL)
    return((const PolicyInfo **) NULL);
  /*
    Generate policy list.
  */
  LockSemaphoreInfo(policy_semaphore);
  ResetLinkedListIterator(policy_list);
  p=(const PolicyInfo *) GetNextValueInLinkedList(policy_list);
  for (i=0; p != (const PolicyInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      policies[i++]=p;
    p=(const PolicyInfo *) GetNextValueInLinkedList(policy_list);
  }
  UnlockSemaphoreInfo(policy_semaphore);
  policies[i]=(PolicyInfo *) NULL;
  *number_policies=(size_t) i;
  return(policies);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P o l i c y L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPolicyList() returns any policies that match the specified pattern.
%
%  The format of the GetPolicyList function is:
%
%      char **GetPolicyList(const char *pattern,size_t *number_policies,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: a pointer to a text string containing a pattern.
%
%    o number_policies:  returns the number of policies in the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport char **GetPolicyList(const char *pattern,
  size_t *number_policies,ExceptionInfo *exception)
{
  char
    **policies;

  register const PolicyInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate policy list.
  */
  assert(pattern != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",pattern);
  assert(number_policies != (size_t *) NULL);
  *number_policies=0;
  p=GetPolicyInfo("*",exception);
  if (p == (const PolicyInfo *) NULL)
    return((char **) NULL);
  policies=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(policy_list)+1UL,sizeof(*policies));
  if (policies == (char **) NULL)
    return((char **) NULL);
  /*
    Generate policy list.
  */
  LockSemaphoreInfo(policy_semaphore);
  ResetLinkedListIterator(policy_list);
  p=(const PolicyInfo *) GetNextValueInLinkedList(policy_list);
  for (i=0; p != (const PolicyInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      policies[i++]=ConstantString(p->name);
    p=(const PolicyInfo *) GetNextValueInLinkedList(policy_list);
  }
  UnlockSemaphoreInfo(policy_semaphore);
  policies[i]=(char *) NULL;
  *number_policies=(size_t) i;
  return(policies);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P o l i c y V a l u e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPolicyValue() returns the value associated with the named policy.
%
%  The format of the GetPolicyValue method is:
%
%      char *GetPolicyValue(const char *name)
%
%  A description of each parameter follows:
%
%    o policy_info:  The policy info.
%
*/
MagickExport char *GetPolicyValue(const char *name)
{
  const char
    *value;

  const PolicyInfo
    *policy_info;

  ExceptionInfo
    *exception;

  assert(name != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",name);
  exception=AcquireExceptionInfo();
  policy_info=GetPolicyInfo(name,exception);
  exception=DestroyExceptionInfo(exception);
  if (policy_info == (PolicyInfo *) NULL)
    return((char *) NULL);
  value=policy_info->value;
  if ((value == (const char *) NULL) || (*value == '\0'))
    return((char *) NULL);
  return(ConstantString(value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e P o l i c y L i s t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializePolicyList() initializes the policy list.
%
%  The format of the InitializePolicyList method is:
%
%      MagickBooleanType InitializePolicyList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType InitializePolicyList(ExceptionInfo *exception)
{
  if ((policy_list == (LinkedListInfo *) NULL) &&
      (instantiate_policy == MagickFalse))
    {
      if (policy_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&policy_semaphore);
      LockSemaphoreInfo(policy_semaphore);
      if ((policy_list == (LinkedListInfo *) NULL) &&
          (instantiate_policy == MagickFalse))
        {
          (void) LoadPolicyLists(PolicyFilename,exception);
          instantiate_policy=MagickTrue;
        }
      UnlockSemaphoreInfo(policy_semaphore);
    }
  return(policy_list != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s R i g h t s A u t h o r i z e d                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsRightsAuthorized() returns MagickTrue if the policy authorizes the
%  requested rights for the specified domain.
%
%  The format of the IsRightsAuthorized method is:
%
%      MagickBooleanType IsRightsAuthorized(const PolicyDomain domain,
%        const PolicyRights rights,const char *pattern)
%
%  A description of each parameter follows:
%
%    o domain: the policy domain.
%
%    o rights: the policy rights.
%
%    o pattern: the coder, delegate, filter, or path pattern.
%
*/
MagickExport MagickBooleanType IsRightsAuthorized(const PolicyDomain domain,
  const PolicyRights rights,const char *pattern)
{
  const PolicyInfo
    *policy_info;

  ExceptionInfo
    *exception;

  MagickBooleanType
    authorized;

  register PolicyInfo
    *p;

  (void) LogMagickEvent(PolicyEvent,GetMagickModule(),
    "Domain: %s; rights=%s; pattern=\"%s\" ...",
    CommandOptionToMnemonic(MagickPolicyDomainOptions,domain),
    CommandOptionToMnemonic(MagickPolicyRightsOptions,rights),pattern);
  exception=AcquireExceptionInfo();
  policy_info=GetPolicyInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  if (policy_info == (PolicyInfo *) NULL)
    return(MagickTrue);
  authorized=MagickTrue;
  LockSemaphoreInfo(policy_semaphore);
  ResetLinkedListIterator(policy_list);
  p=(PolicyInfo *) GetNextValueInLinkedList(policy_list);
  while ((p != (PolicyInfo *) NULL) && (authorized != MagickFalse))
  {
    if ((p->domain == domain) &&
        (GlobExpression(pattern,p->pattern,MagickFalse) != MagickFalse))
      {
        if (((rights & ReadPolicyRights) != 0) &&
            ((p->rights & ReadPolicyRights) == 0))
          authorized=MagickFalse;
        if (((rights & WritePolicyRights) != 0) &&
            ((p->rights & WritePolicyRights) == 0))
          authorized=MagickFalse;
        if (((rights & ExecutePolicyRights) != 0) &&
            ((p->rights & ExecutePolicyRights) == 0))
          authorized=MagickFalse;
      }
    p=(PolicyInfo *) GetNextValueInLinkedList(policy_list);
  }
  UnlockSemaphoreInfo(policy_semaphore);
  return(authorized);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t P o l i c y I n f o                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListPolicyInfo() lists policies to the specified file.
%
%  The format of the ListPolicyInfo method is:
%
%      MagickBooleanType ListPolicyInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  List policy names to this file handle.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListPolicyInfo(FILE *file,
  ExceptionInfo *exception)
{
  const char
    *path,
    *domain;

  const PolicyInfo
    **policy_info;

  register ssize_t
    i;

  size_t
    number_policies;

  /*
    List name and attributes of each policy in the list.
  */
  if (file == (const FILE *) NULL)
    file=stdout;
  policy_info=GetPolicyInfoList("*",&number_policies,exception);
  if (policy_info == (const PolicyInfo **) NULL)
    return(MagickFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_policies; i++)
  {
    if (policy_info[i]->stealth != MagickFalse)
      continue;
    if (((path == (const char *) NULL) ||
         (LocaleCompare(path,policy_info[i]->path) != 0)) &&
         (policy_info[i]->path != (char *) NULL))
      (void) FormatLocaleFile(file,"\nPath: %s\n",policy_info[i]->path);
    path=policy_info[i]->path;
    domain=CommandOptionToMnemonic(MagickPolicyDomainOptions,
      policy_info[i]->domain);
    (void) FormatLocaleFile(file,"  Policy: %s\n",domain);
    if ((policy_info[i]->domain == ResourcePolicyDomain) ||
        (policy_info[i]->domain == SystemPolicyDomain))
      {
        if (policy_info[i]->name != (char *) NULL)
          (void) FormatLocaleFile(file,"    name: %s\n",policy_info[i]->name);
        if (policy_info[i]->value != (char *) NULL)
          (void) FormatLocaleFile(file,"    value: %s\n",policy_info[i]->value);
      }
    else
      {
        (void) FormatLocaleFile(file,"    rights: ");
        if (policy_info[i]->rights == NoPolicyRights)
          (void) FormatLocaleFile(file,"None ");
        if ((policy_info[i]->rights & ReadPolicyRights) != 0)
          (void) FormatLocaleFile(file,"Read ");
        if ((policy_info[i]->rights & WritePolicyRights) != 0)
          (void) FormatLocaleFile(file,"Write ");
        if ((policy_info[i]->rights & ExecutePolicyRights) != 0)
          (void) FormatLocaleFile(file,"Execute ");
        (void) FormatLocaleFile(file,"\n");
        if (policy_info[i]->pattern != (char *) NULL)
          (void) FormatLocaleFile(file,"    pattern: %s\n",
            policy_info[i]->pattern);
      }
  }
  policy_info=(const PolicyInfo **) RelinquishMagickMemory((void *)
    policy_info);
  (void) fflush(file);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d P o l i c y L i s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadPolicyList() loads the policy configuration file which provides a mapping
%  between policy attributes and a policy domain.
%
%  The format of the LoadPolicyList method is:
%
%      MagickBooleanType LoadPolicyList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The policy list in XML format.
%
%    o filename:  The policy list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadPolicyList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MaxTextExtent],
    *token;

  PolicyInfo
    *policy_info;

  const char
    *q;

  MagickBooleanType
    status;

  /*
    Load the policy map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading policy file \"%s\" ...",filename);
  if (xml == (char *) NULL)
    return(MagickFalse);
  if (policy_list == (LinkedListInfo *) NULL)
    {
      policy_list=NewLinkedList(0);
      if (policy_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  status=MagickTrue;
  policy_info=(PolicyInfo *) NULL;
  token=AcquireString(xml);
  for (q=(const char *) xml; *q != '\0'; )
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
          Docdomain element.
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
                      status=LoadPolicyList(xml,path,depth+1,exception);
                      xml=(char *) RelinquishMagickMemory(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<policy") == 0)
      {
        /*
          Policy element.
        */
        policy_info=(PolicyInfo *) AcquireMagickMemory(sizeof(*policy_info));
        if (policy_info == (PolicyInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(policy_info,0,sizeof(*policy_info));
        policy_info->path=ConstantString(filename);
        policy_info->exempt=MagickFalse;
        policy_info->signature=MagickSignature;
        continue;
      }
    if (policy_info == (PolicyInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        status=AppendValueToLinkedList(policy_list,policy_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            policy_info->name);
        policy_info=(PolicyInfo *) NULL;
      }
    GetMagickToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetMagickToken(q,&q,token);
    GetMagickToken(q,&q,token);
    switch (*keyword)
    {
      case 'D':
      case 'd':
      {
        if (LocaleCompare((char *) keyword,"domain") == 0)
          {
            policy_info->domain=(PolicyDomain) ParseCommandOption(
              MagickPolicyDomainOptions,MagickTrue,token);
            break;
          }
        break;
      }
      case 'N':
      case 'n':
      {
        if (LocaleCompare((char *) keyword,"name") == 0)
          {
            policy_info->name=ConstantString(token);
            break;
          }
        break;
      }
      case 'P':
      case 'p':
      {
        if (LocaleCompare((char *) keyword,"pattern") == 0)
          {
            policy_info->pattern=ConstantString(token);
            break;
          }
        break;
      }
      case 'R':
      case 'r':
      {
        if (LocaleCompare((char *) keyword,"rights") == 0)
          {
            policy_info->rights=(PolicyRights) ParseCommandOption(
              MagickPolicyRightsOptions,MagickTrue,token);
            break;
          }
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"stealth") == 0)
          {
            policy_info->stealth=IsMagickTrue(token);
            break;
          }
        break;
      }
      case 'V':
      case 'v':
      {
        if (LocaleCompare((char *) keyword,"value") == 0)
          {
            policy_info->value=ConstantString(token);
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
%  L o a d P o l i c y L i s t s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadPolicyList() loads one or more policy configuration file which provides a
%  mapping between policy attributes and a policy name.
%
%  The format of the LoadPolicyLists method is:
%
%      MagickBooleanType LoadPolicyLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType LoadPolicyLists(const char *filename,
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
    Load built-in policy map.
  */
  status=MagickFalse;
  if (policy_list == (LinkedListInfo *) NULL)
    {
      policy_list=NewLinkedList(0);
      if (policy_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,ResourceLimitError,
            "MemoryAllocationFailed",filename);
          return(MagickFalse);
        }
    }
  for (i=0; i < (ssize_t) (sizeof(PolicyMap)/sizeof(*PolicyMap)); i++)
  {
    PolicyInfo
      *policy_info;

    register const PolicyMapInfo
      *p;

    p=PolicyMap+i;
    policy_info=(PolicyInfo *) AcquireMagickMemory(sizeof(*policy_info));
    if (policy_info == (PolicyInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",policy_info->name);
        continue;
      }
    (void) ResetMagickMemory(policy_info,0,sizeof(*policy_info));
    policy_info->path=(char *) "[built-in]";
    policy_info->domain=p->domain;
    policy_info->rights=p->rights;
    policy_info->name=(char *) p->name;
    policy_info->pattern=(char *) p->pattern;
    policy_info->value=(char *) p->value;
    policy_info->exempt=MagickTrue;
    policy_info->signature=MagickSignature;
    status=AppendValueToLinkedList(policy_list,policy_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",policy_info->name);
  }
  /*
    Load external policy map.
  */
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status|=LoadPolicyList((const char *) GetStringInfoDatum(option),
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
+   P o l i c y C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PolicyComponentGenesis() instantiates the policy component.
%
%  The format of the PolicyComponentGenesis method is:
%
%      MagickBooleanType PolicyComponentGenesis(void)
%
*/
MagickExport MagickBooleanType PolicyComponentGenesis(void)
{
  AcquireSemaphoreInfo(&policy_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P o l i c y C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PolicyComponentTerminus() destroys the policy component.
%
%  The format of the PolicyComponentTerminus method is:
%
%      PolicyComponentTerminus(void)
%
*/

static void *DestroyPolicyElement(void *policy_info)
{
  register PolicyInfo
    *p;

  p=(PolicyInfo *) policy_info;
  if (p->exempt == MagickFalse)
    {
      if (p->value != (char *) NULL)
        p->value=DestroyString(p->value);
      if (p->pattern != (char *) NULL)
        p->pattern=DestroyString(p->pattern);
      if (p->name != (char *) NULL)
        p->name=DestroyString(p->name);
      if (p->path != (char *) NULL)
        p->path=DestroyString(p->path);
    }
  p=(PolicyInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickExport void PolicyComponentTerminus(void)
{
  if (policy_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&policy_semaphore);
  LockSemaphoreInfo(policy_semaphore);
  if (policy_list != (LinkedListInfo *) NULL)
    policy_list=DestroyLinkedList(policy_list,DestroyPolicyElement);
  instantiate_policy=MagickFalse;
  UnlockSemaphoreInfo(policy_semaphore);
  DestroySemaphoreInfo(&policy_semaphore);
}

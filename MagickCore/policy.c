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
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/studio.h"
#include "MagickCore/client.h"
#include "MagickCore/configure.h"
#include "MagickCore/configure-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/policy.h"
#include "MagickCore/policy-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

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
  *policy_cache = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *policy_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declarations.
*/
static MagickBooleanType
  IsPolicyCacheInstantiated(ExceptionInfo *),
  LoadPolicyCache(LinkedListInfo *,const char *,const char *,const size_t,
    ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e P o l i c y C a c h e                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquirePolicyCache() caches one or more policy configurations which provides
%  a mapping between policy attributes and a policy name.
%
%  The format of the AcquirePolicyCache method is:
%
%      LinkedListInfo *AcquirePolicyCache(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file name.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static LinkedListInfo *AcquirePolicyCache(const char *filename,
  ExceptionInfo *exception)
{
  LinkedListInfo
    *cache;

  MagickStatusType
    status;

  register ssize_t
    i;

  /*
    Load external policy map.
  */
  cache=NewLinkedList(0);
  if (cache == (LinkedListInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  status=MagickTrue;
#if !defined(MAGICKCORE_ZERO_CONFIGURATION_SUPPORT)
  {
    const StringInfo
      *option;

    LinkedListInfo
      *options;

    options=GetConfigureOptions(filename,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
    while (option != (const StringInfo *) NULL)
    {
      status&=LoadPolicyCache(cache,(const char *)
        GetStringInfoDatum(option),GetStringInfoPath(option),0,exception);
      option=(const StringInfo *) GetNextValueInLinkedList(options);
    }
    options=DestroyConfigureOptions(options);
  }
#endif
  /*
    Load built-in policy map.
  */
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
          ResourceLimitError,"MemoryAllocationFailed","`%s'",p->name);
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
    policy_info->signature=MagickCoreSignature;
    status&=AppendValueToLinkedList(cache,policy_info);
    if (status == MagickFalse)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",policy_info->name);
  }
  return(cache);
}

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
    policyname[MagickPathExtent];

  PolicyDomain
    domain;

  register PolicyInfo
    *p;

  register char
    *q;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsPolicyCacheInstantiated(exception) == MagickFalse)
    return((PolicyInfo *) NULL);
  /*
    Strip names of whitespace.
  */
  *policyname='\0';
  if (name != (const char *) NULL)
    (void) CopyMagickString(policyname,name,MagickPathExtent);
  for (q=policyname; *q != '\0'; q++)
  {
    if (isspace((int) ((unsigned char) *q)) == 0)
      continue;
    (void) CopyMagickString(q,q+1,MagickPathExtent);
    q--;
  }
  /*
    Strip domain from policy name (e.g. resource:map).
  */
  domain=UndefinedPolicyDomain;
  for (q=policyname; *q != '\0'; q++)
  {
    if (*q != ':')
      continue;
    *q='\0';
    domain=(PolicyDomain) ParseCommandOption(MagickPolicyDomainOptions,
      MagickTrue,policyname);
    (void) CopyMagickString(policyname,q+1,MagickPathExtent);
    break;
  }
  /*
    Search for policy tag.
  */
  LockSemaphoreInfo(policy_semaphore);
  ResetLinkedListIterator(policy_cache);
  p=(PolicyInfo *) GetNextValueInLinkedList(policy_cache);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    {
      UnlockSemaphoreInfo(policy_semaphore);
      return(p);
    }
  while (p != (PolicyInfo *) NULL)
  {
    if ((domain == UndefinedPolicyDomain) || (p->domain == domain))
      if (LocaleCompare(policyname,p->name) == 0)
        break;
    p=(PolicyInfo *) GetNextValueInLinkedList(policy_cache);
  }
  if (p != (PolicyInfo *) NULL)
    (void) InsertValueInLinkedList(policy_cache,0,
      RemoveElementByValueFromLinkedList(policy_cache,p));
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
    GetNumberOfElementsInLinkedList(policy_cache)+1UL,sizeof(*policies));
  if (policies == (const PolicyInfo **) NULL)
    return((const PolicyInfo **) NULL);
  /*
    Generate policy list.
  */
  LockSemaphoreInfo(policy_semaphore);
  ResetLinkedListIterator(policy_cache);
  p=(const PolicyInfo *) GetNextValueInLinkedList(policy_cache);
  for (i=0; p != (const PolicyInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      policies[i++]=p;
    p=(const PolicyInfo *) GetNextValueInLinkedList(policy_cache);
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
    GetNumberOfElementsInLinkedList(policy_cache)+1UL,sizeof(*policies));
  if (policies == (char **) NULL)
    return((char **) NULL);
  /*
    Generate policy list.
  */
  LockSemaphoreInfo(policy_semaphore);
  ResetLinkedListIterator(policy_cache);
  p=(const PolicyInfo *) GetNextValueInLinkedList(policy_cache);
  for (i=0; p != (const PolicyInfo *) NULL; )
  {
    if ((p->stealth == MagickFalse) &&
        (GlobExpression(p->name,pattern,MagickFalse) != MagickFalse))
      policies[i++]=ConstantString(p->name);
    p=(const PolicyInfo *) GetNextValueInLinkedList(policy_cache);
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
+   I s P o l i c y C a c h e I n s t a n t i a t e d                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPolicyCacheInstantiated() determines if the policy list is instantiated.
%  If not, it instantiates the list and returns it.
%
%  The format of the IsPolicyInstantiated method is:
%
%      MagickBooleanType IsPolicyCacheInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType IsPolicyCacheInstantiated(ExceptionInfo *exception)
{
  if (policy_cache == (LinkedListInfo *) NULL)
    {
      if (policy_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&policy_semaphore);
      LockSemaphoreInfo(policy_semaphore);
      if (policy_cache == (LinkedListInfo *) NULL)
        policy_cache=AcquirePolicyCache(PolicyFilename,exception);
      UnlockSemaphoreInfo(policy_semaphore);
    }
  return(policy_cache != (LinkedListInfo *) NULL ? MagickTrue : MagickFalse);
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
  ResetLinkedListIterator(policy_cache);
  p=(PolicyInfo *) GetNextValueInLinkedList(policy_cache);
  while (p != (PolicyInfo *) NULL)
  {
    if ((p->domain == domain) &&
        (GlobExpression(pattern,p->pattern,MagickFalse) != MagickFalse))
      {
        if ((rights & ReadPolicyRights) != 0)
          authorized=(p->rights & ReadPolicyRights) != 0 ? MagickTrue :
            MagickFalse;
        if ((rights & WritePolicyRights) != 0)
          authorized=(p->rights & WritePolicyRights) != 0 ? MagickTrue :
            MagickFalse;
        if ((rights & ExecutePolicyRights) != 0)
          authorized=(p->rights & ExecutePolicyRights) != 0 ? MagickTrue :
            MagickFalse;
      }
    p=(PolicyInfo *) GetNextValueInLinkedList(policy_cache);
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
    if ((policy_info[i]->domain == CachePolicyDomain) ||
        (policy_info[i]->domain == ResourcePolicyDomain) ||
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
+   L o a d P o l i c y C a c h e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadPolicyCache() loads the policy configurations which provides a mapping
%  between policy attributes and a policy domain.
%
%  The format of the LoadPolicyCache method is:
%
%      MagickBooleanType LoadPolicyCache(LinkedListInfo *cache,const char *xml,
%        const char *filename,const size_t depth,ExceptionInfo *exception)
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
static MagickBooleanType LoadPolicyCache(LinkedListInfo *cache,const char *xml,
  const char *filename,const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MagickPathExtent],
    *token;

  const char
    *q;

  MagickStatusType
    status;

  PolicyInfo
    *policy_info;

  size_t
    extent;

  /*
    Load the policy map file.
  */
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading policy file \"%s\" ...",filename);
  if (xml == (char *) NULL)
    return(MagickFalse);
  status=MagickTrue;
  policy_info=(PolicyInfo *) NULL;
  token=AcquireString(xml);
  extent=strlen(token)+MagickPathExtent;
  for (q=(const char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetNextToken(q,&q,extent,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MagickPathExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Docdomain element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          GetNextToken(q,&q,extent,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          GetNextToken(q,&q,extent,token);
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyMagickString(keyword,token,MagickPathExtent);
          GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          GetNextToken(q,&q,extent,token);
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > 200)
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ConfigureError,"IncludeElementNestedTooDeeply","`%s'",token);
              else
                {
                  char
                    path[MagickPathExtent],
                    *file_xml;

                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateMagickString(path,DirectorySeparator,
                      MagickPathExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyMagickString(path,token,MagickPathExtent);
                  else
                    (void) ConcatenateMagickString(path,token,MagickPathExtent);
                  file_xml=FileToXML(path,~0UL);
                  if (file_xml != (char *) NULL)
                    {
                      status&=LoadPolicyCache(cache,file_xml,path,
                        depth+1,exception);
                      file_xml=DestroyString(file_xml);
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
        policy_info->signature=MagickCoreSignature;
        continue;
      }
    if (policy_info == (PolicyInfo *) NULL)
      continue;
    if ((LocaleCompare(keyword,"/>") == 0) ||
        (LocaleCompare(keyword,"</policy>") == 0))
      {
        status=AppendValueToLinkedList(cache,policy_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",
            policy_info->name);
        policy_info=(PolicyInfo *) NULL;
        continue;
      }
    GetNextToken(q,(const char **) NULL,extent,token);
    if (*token != '=')
      continue;
    GetNextToken(q,&q,extent,token);
    GetNextToken(q,&q,extent,token);
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
            policy_info->stealth=IsStringTrue(token);
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
MagickPrivate MagickBooleanType PolicyComponentGenesis(void)
{
  if (policy_semaphore == (SemaphoreInfo *) NULL)
    policy_semaphore=AcquireSemaphoreInfo();
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

MagickPrivate void PolicyComponentTerminus(void)
{
  if (policy_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&policy_semaphore);
  LockSemaphoreInfo(policy_semaphore);
  if (policy_cache != (LinkedListInfo *) NULL)
    policy_cache=DestroyLinkedList(policy_cache,DestroyPolicyElement);
  UnlockSemaphoreInfo(policy_semaphore);
  RelinquishSemaphoreInfo(&policy_semaphore);
}

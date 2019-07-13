/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#ifndef MAGICK_GHOSTSCRIPT_BUFFER_PRIVATE_H
#define MAGICK_GHOSTSCRIPT_BUFFER_PRIVATE_H

#include "coders/bytebuffer-private.h"

#if defined(MAGICKCORE_GS_DELEGATE) || defined(MAGICKCORE_WINDOWS_SUPPORT)
static int MagickDLLCall GhostscriptDelegateMessage(void *handle,
  const char *message,int length)
{
  char
    **messages;

  ssize_t
    offset;

  offset=0;
  messages=(char **) handle;
  if (*messages == (char *) NULL)
    *messages=(char *) AcquireQuantumMemory(length+1,sizeof(char *));
  else
    {
      offset=strlen(*messages);
      *messages=(char *) ResizeQuantumMemory(*messages,offset+length+1,
        sizeof(char *));
    }
  if (*messages == (char *) NULL)
    return(0);
  (void) memcpy(*messages+offset,message,length);
  (*messages)[length+offset] ='\0';
  return(length);
}
#endif

static MagickBooleanType InvokeGhostscriptDelegate(
  const MagickBooleanType verbose,const char *command,char *message,
  ExceptionInfo *exception)
{
  int
    status;

#if defined(MAGICKCORE_GS_DELEGATE) || defined(MAGICKCORE_WINDOWS_SUPPORT)
#define SetArgsStart(command,args_start) \
  if (args_start == (const char *) NULL) \
    { \
      if (*command != '"') \
        args_start=strchr(command,' '); \
      else \
        { \
          args_start=strchr(command+1,'"'); \
          if (args_start != (const char *) NULL) \
            args_start++; \
        } \
    }

#define ExecuteGhostscriptCommand(command,status) \
{ \
  status=ExternalDelegateCommand(MagickFalse,verbose,command,message, \
    exception); \
  if (status == 0) \
    return(MagickTrue); \
  if (status < 0) \
    return(MagickFalse); \
  (void) ThrowMagickException(exception,GetMagickModule(),DelegateError, \
    "FailedToExecuteCommand","`%s' (%d)",command,status); \
  return(MagickFalse); \
}

  char
    **argv,
    *errors;

  const char
    *args_start = (const char *) NULL;

  const GhostInfo
    *ghost_info;

  gs_main_instance
    *interpreter;

  gsapi_revision_t
    revision;

  int
    argc,
    code;

  register ssize_t
    i;

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  ghost_info=NTGhostscriptDLLVectors();
#else
  GhostInfo
    ghost_info_struct;

  ghost_info=(&ghost_info_struct);
  (void) memset(&ghost_info_struct,0,sizeof(ghost_info_struct));
  ghost_info_struct.delete_instance=(void (*)(gs_main_instance *))
    gsapi_delete_instance;
  ghost_info_struct.exit=(int (*)(gs_main_instance *)) gsapi_exit;
  ghost_info_struct.new_instance=(int (*)(gs_main_instance **,void *))
    gsapi_new_instance;
  ghost_info_struct.init_with_args=(int (*)(gs_main_instance *,int,char **))
    gsapi_init_with_args;
  ghost_info_struct.run_string=(int (*)(gs_main_instance *,const char *,int,
    int *)) gsapi_run_string;
  ghost_info_struct.set_stdio=(int (*)(gs_main_instance *,int (*)(void *,char *,
    int),int (*)(void *,const char *,int),int (*)(void *, const char *, int)))
    gsapi_set_stdio;
  ghost_info_struct.revision=(int (*)(gsapi_revision_t *,int)) gsapi_revision;
#endif
  if (ghost_info == (GhostInfo *) NULL)
    ExecuteGhostscriptCommand(command,status);
  if ((ghost_info->revision)(&revision,sizeof(revision)) != 0)
    revision.revision=0;
  if (verbose != MagickFalse)
    {
      (void) fprintf(stdout,"[ghostscript library %.2f]",(double)
        revision.revision/100.0);
      SetArgsStart(command,args_start);
      (void) fputs(args_start,stdout);
    }
  interpreter=(gs_main_instance *) NULL;
  errors=(char *) NULL;
  status=(ghost_info->new_instance)(&interpreter,(void *) &errors);
  if (status < 0)
    ExecuteGhostscriptCommand(command,status);
  code=0;
  argv=StringToArgv(command,&argc);
  if (argv == (char **) NULL)
    {
      (ghost_info->delete_instance)(interpreter);
      return(MagickFalse);
    }
  (void) (ghost_info->set_stdio)(interpreter,(int (MagickDLLCall *)(void *,
    char *,int)) NULL,GhostscriptDelegateMessage,GhostscriptDelegateMessage);
  status=(ghost_info->init_with_args)(interpreter,argc-1,argv+1);
  if (status == 0)
    status=(ghost_info->run_string)(interpreter,"systemdict /start get exec\n",
      0,&code);
  (ghost_info->exit)(interpreter);
  (ghost_info->delete_instance)(interpreter);
  for (i=0; i < (ssize_t) argc; i++)
    argv[i]=DestroyString(argv[i]);
  argv=(char **) RelinquishMagickMemory(argv);
  if (status != 0)
    {
      SetArgsStart(command,args_start);
      if (status == -101) /* quit */
        (void) FormatLocaleString(message,MaxTextExtent,
          "[ghostscript library %.2f]%s: %s",(double) revision.revision/100.0,
          args_start,errors);
      else
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            DelegateError,"PostscriptDelegateFailed",
            "`[ghostscript library %.2f]%s': %s",(double) revision.revision/
            100.0,args_start,errors);
          if (errors != (char *) NULL)
            errors=DestroyString(errors);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Ghostscript returns status %d, exit code %d",status,code);
          return(MagickFalse);
        }
    }
  if (errors != (char *) NULL)
    errors=DestroyString(errors);
  return(MagickTrue);
#else
  status=ExternalDelegateCommand(MagickFalse,verbose,command,(char *) NULL,
    exception);
  return(status == 0 ? MagickTrue : MagickFalse);
#endif
}

static MagickBooleanType IsGhostscriptRendered(const char *path)
{
  MagickBooleanType
    status;

  struct stat
    attributes;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(MagickFalse);
  status=GetPathAttributes(path,&attributes);
  if ((status != MagickFalse) && S_ISREG(attributes.st_mode) &&
      (attributes.st_size > 0))
    return(MagickTrue);
  return(MagickFalse);
}

static void ReadGhostScriptXMPProfile(MagickByteBuffer *buffer,
  StringInfo **profile)
{
#define BeginXMPPacket  "?xpacket begin="
#define EndXMPPacket  "<?xpacket end="

  int
    c;

  MagickBooleanType
    found_end,
    status;

  register char
    *p;

  size_t
    length;

  ssize_t
    count;

  if (*profile != (StringInfo *) NULL)
    return;
  status=CompareMagickByteBuffer(buffer,BeginXMPPacket,strlen(BeginXMPPacket));
  if (status == MagickFalse)
    return;
  length=8192;
  *profile=AcquireStringInfo(length);
  found_end=MagickFalse;
  p=(char *) GetStringInfoDatum(*profile);
  *p++='<';
  count=1;
  for (c=ReadMagickByteBuffer(buffer); c != EOF; c=ReadMagickByteBuffer(buffer))
  {
    if (count == (ssize_t) length)
      {
        length<<=1;
        SetStringInfoLength(*profile,length);
        p=(char *) GetStringInfoDatum(*profile)+count;
      }
    count++;
    *p++=(char) c;
    if (found_end == MagickFalse)
      found_end=CompareMagickByteBuffer(buffer,EndXMPPacket,
        strlen(EndXMPPacket));
    else
      {
        if (c == (int) '>')
          break;
      }
  }
  SetStringInfoLength(*profile,(size_t) count);
}

#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        X   X  TTTTT  RRRR   N   N                           %
%                         X X     T    R   R  NN  N                           %
%                          X      T    RRRR   N N N                           %
%                         X X     T    R R    N  NN                           %
%                        X   X    T    R  R   N   N                           %
%                                                                             %
%                                                                             %
%                    ImageMagickObject BLOB Interface.                        %
%                                                                             %
%                              Software Design                                %
%                             William Radcliffe                               %
%                                 May 2001                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization      %
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
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  This coder is a kind of backdoor used by the COM object that allows it to  %
%  pass blobs back and forth using the coder interface. It simply encodes and %
%  decodes the filename as a comma delimited string and extracts the info it  %
%  needs. The five methods of passing images are:                             %
%                                                                             %
%     FILE   - same thing as filename so it should be a NOP                   %
%     IMAGE  - passes an image and image info structure                       %
%     BLOB   - passes binary blob containining the image                      %
%     STREAM - passes pointers to stream hooks in and does the hooking        %
%     ARRAY  - passes a pointer to a Win32 smart array and streams to it      %
%                                                                             %
%  Of all of these, the only one getting any real use at the moment is the    %
%  ARRAY handler. It is the primary way that images are shuttled back and     %
%  forth as blobs via COM since this is what VB and VBSCRIPT use internally   %
%  for this purpose.                                                          %
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/MagickCore.h"
#include "MagickCore/memory_.h"
#include "MagickCore/string_.h"
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <ole2.h>

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteXTRNImage(const ImageInfo *,Image *,ExceptionInfo *exception);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X T R N I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadXTRNImage() reads a XTRN image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadXTRNImage method is:
%
%      Image *ReadXTRNImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#  pragma warning(disable : 4477)
static Image *ReadXTRNImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *clone_info;

  void
    *param1,
    *param2,
    *param3;

  param1 = param2 = param3 = (void *) NULL;
  image = (Image *) NULL;
  clone_info=CloneImageInfo(image_info);
  if (clone_info->filename == NULL)
    {
      clone_info=DestroyImageInfo(clone_info);
      ThrowReaderException(FileOpenWarning,"No filename specified");
    }
  if (LocaleCompare(image_info->magick,"XTRNFILE") == 0)
    {
      image=ReadImage(clone_info,exception);
      CatchException(exception);
    }
  else if (LocaleCompare(image_info->magick,"XTRNIMAGE") == 0)
    {
      Image
        **image_ptr;

#ifdef ALL_IMAGEINFO
      ImageInfo
        **image_info_ptr;
#endif

      (void) sscanf(clone_info->filename,"%lx,%lx",&param1,&param2);
      image_ptr=(Image **) param2;
      if (*image_ptr != (Image *) NULL)
        image=CloneImage(*image_ptr,0,0,MagickFalse,exception);
#ifdef ALL_IMAGEINFO
      image_info_ptr=(ImageInfo **) param1;
      if (*image_info_ptr != (ImageInfo *) NULL)
        image_info=*image_info_ptr;
#endif
    }
  else if (LocaleCompare(image_info->magick,"XTRNBLOB") == 0)
    {
      char
        **blob_data;

      size_t
        *blob_length;

      char
        filename[MagickPathExtent];

      (void) sscanf(clone_info->filename,"%lx,%lx,%2048s",&param1,&param2,
        filename);
      blob_data=(char **) param1;
      blob_length=(size_t *) param2;
      image=BlobToImage(clone_info,*blob_data,*blob_length,exception);
      CatchException(exception);
    }
  else if (LocaleCompare(image_info->magick,"XTRNARRAY") == 0)
    {
      char
        *blob_data,
        filename[MagickPathExtent];

      HRESULT
        hr;

      long
        lBoundl,
        lBoundu;

      SAFEARRAY
        *pSafeArray;

      size_t
        blob_length;

      *filename='\0';
      (void) sscanf(clone_info->filename,"%lx,%2048s",&param1,filename);
      hr=S_OK;
      pSafeArray=(SAFEARRAY *) param1;
      if (pSafeArray)
        {
          hr = SafeArrayGetLBound(pSafeArray, 1, &lBoundl);
          if (SUCCEEDED(hr))
            {
              hr = SafeArrayGetUBound(pSafeArray, 1, &lBoundu);
              if (SUCCEEDED(hr))
                {
                  blob_length = lBoundu - lBoundl + 1;
                  hr = SafeArrayAccessData(pSafeArray,(void**) &blob_data);
                  if(SUCCEEDED(hr))
                    {
                      *clone_info->filename='\0';
                      *clone_info->magick='\0';
                      if (*filename != '\0')
                        (void) CopyMagickString(clone_info->filename,filename,
                          MagickPathExtent);
                      image=BlobToImage(clone_info,blob_data,blob_length,
                        exception);
                      hr=SafeArrayUnaccessData(pSafeArray);
                      CatchException(exception);
                    }
                }
            }
        }
    }
  clone_info=DestroyImageInfo(clone_info);
  return(image);
}
#  pragma warning(default : 4477)
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r X T R N I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterXTRNImage() adds attributes for the XTRN image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXTRNImage method is:
%
%      size_t RegisterXTRNImage(void)
%
*/
ModuleExport size_t RegisterXTRNImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("XTRN","XTRNFILE","External transfer of a file");
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
#endif
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("XTRN","XTRNIMAGE",
    "External transfer of a image in memory");
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
#endif
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("XTRN","XTRNBLOB",
    "IExternal transfer of a blob in memory");
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
#endif
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("XTRN","XTRNARRAY",
    "External transfer via a smart array interface");
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  entry->decoder=ReadXTRNImage;
  entry->encoder=WriteXTRNImage;
#endif
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X T R N I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterXTRNImage() removes format registrations made by the
%  XTRN module from the list of supported formats.
%
%  The format of the UnregisterXTRNImage method is:
%
%      UnregisterXTRNImage(void)
%
*/
ModuleExport void UnregisterXTRNImage(void)
{
  UnregisterMagickInfo("XTRNFILE");
  UnregisterMagickInfo("XTRNIMAGE");
  UnregisterMagickInfo("XTRNBLOB");
  UnregisterMagickInfo("XTRNARRAY");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X T R N I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteXTRNImage() writes an image in the XTRN encoded image format.
%  We use GIF because it is the only format that is compressed without
%  requiring additional optional delegates (TIFF, ZIP, etc).
%
%  The format of the WriteXTRNImage method is:
%
%      MagickBooleanType WriteXTRNImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
static size_t SafeArrayFifo(const Image *image,const void *data,
  const size_t length)
{
  SAFEARRAYBOUND NewArrayBounds[1];  /* 1 Dimension */
  size_t tlen=length;
  SAFEARRAY *pSafeArray = (SAFEARRAY *)image->client_data;
  if (pSafeArray != NULL)
  {
    long lBoundl, lBoundu, lCount;
    HRESULT hr = S_OK;
    /* First see how big the buffer currently is */
    hr = SafeArrayGetLBound(pSafeArray, 1, &lBoundl);
    if (FAILED(hr))
      return MagickFalse;
    hr = SafeArrayGetUBound(pSafeArray, 1, &lBoundu);
    if (FAILED(hr))
      return MagickFalse;
    lCount = lBoundu - lBoundl + 1;

    if (length>0)
    {
      unsigned char       *pReturnBuffer = NULL;
      NewArrayBounds[0].lLbound = 0;   /* Start-Index 0 */
      NewArrayBounds[0].cElements = (unsigned long) (length+lCount);  /* # Elemente */
      hr = SafeArrayRedim(pSafeArray, NewArrayBounds);
      if (FAILED(hr))
        return 0;
      hr = SafeArrayAccessData(pSafeArray, (void**)&pReturnBuffer);
      if( FAILED(hr) )
        return 0;
      (void) memcpy(pReturnBuffer+lCount,(unsigned char *) data,length);
      hr=SafeArrayUnaccessData(pSafeArray);
      if(FAILED(hr))
        return 0;
    }
    else
    {
      /* Adjust the length of the buffer to fit */
    }
  }
  return(tlen);
}
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#  pragma warning(disable : 4477)
static MagickBooleanType WriteXTRNImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  Image *
    p;

  ImageInfo
    *clone_info;

  int
    scene;

  MagickBooleanType
    status;

  void
    *param1,
    *param2,
    *param3;

  param1 = param2 = param3 = (void *) NULL;
  status=MagickTrue;
  if (LocaleCompare(image_info->magick,"XTRNFILE") == 0)
    {
      clone_info=CloneImageInfo(image_info);
      *clone_info->magick='\0';
      status=WriteImage(clone_info,image,exception);
      if (status == MagickFalse)
        CatchImageException(image);
      clone_info=DestroyImageInfo(clone_info);
    }
  else if (LocaleCompare(image_info->magick,"XTRNIMAGE") == 0)
    {
      Image
        **image_ptr;

      ImageInfo
        **image_info_ptr;

      clone_info=CloneImageInfo(image_info);
      if (clone_info->filename[0])
        {
          (void) sscanf(clone_info->filename,"%lx,%lx",&param1,&param2);
          image_info_ptr=(ImageInfo **) param1;
          image_ptr=(Image **) param2;
          if ((image_info_ptr != (ImageInfo **) NULL) &&
              (image_ptr != (Image **) NULL))
            {
              *image_ptr=CloneImage(image,0,0,MagickFalse,exception);
              *image_info_ptr=clone_info;
            }
        }
    }
  else if (LocaleCompare(image_info->magick,"XTRNBLOB") == 0)
    {
      char
        **blob_data;

      size_t
        *blob_length;

      char
        filename[MagickPathExtent];

      clone_info=CloneImageInfo(image_info);
      if (clone_info->filename[0])
        {
          (void) sscanf(clone_info->filename,"%lx,%lx,%2048s",
            &param1,&param2,filename);

          blob_data=(char **) param1;
          blob_length=(size_t *) param2;
          scene = 0;
          (void) CopyMagickString(clone_info->filename,filename,
            MagickPathExtent);
          for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
          {
            (void) CopyMagickString(p->filename,filename,MagickPathExtent);
            p->scene=scene++;
          }
          SetImageInfo(clone_info,1,exception);
          (void) CopyMagickString(image->magick,clone_info->magick,
            MagickPathExtent);
          if (*blob_length == 0)
            *blob_length=8192;
          *blob_data=(char *) ImageToBlob(clone_info,image,blob_length,
            exception);
          if (*blob_data == NULL)
            status=MagickFalse;
          if (status == MagickFalse)
            CatchImageException(image);
        }
      clone_info=DestroyImageInfo(clone_info);
    }
  else if (LocaleCompare(image_info->magick,"XTRNARRAY") == 0)
    {
      char
        filename[MagickPathExtent];

      size_t
        blob_length;

      unsigned char
        *blob_data;

      clone_info=CloneImageInfo(image_info);
      if (*clone_info->filename != '\0')
        {
          (void) sscanf(clone_info->filename,"%lx,%2048s",&param1,filename);
          image->client_data=param1;
          scene=0;
          (void) CopyMagickString(clone_info->filename,filename,
            MagickPathExtent);
          for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
          {
            (void) CopyMagickString(p->filename,filename,MagickPathExtent);
            p->scene=scene++;
          }
          SetImageInfo(clone_info,1,exception);
          (void) CopyMagickString(image->magick,clone_info->magick,
            MagickPathExtent);
          blob_data=ImageToBlob(clone_info,image,&blob_length,
            exception);
          if (blob_data == (unsigned char *) NULL)
            status=MagickFalse;
          else
            SafeArrayFifo(image,blob_data,blob_length);
          if (status == MagickFalse)
            CatchImageException(image);
        }
      clone_info=DestroyImageInfo(clone_info);
    }
  return(MagickTrue);
}
#  pragma warning(default : 4477)
#endif

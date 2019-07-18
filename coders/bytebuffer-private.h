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
#ifndef MAGICK_BYTE_BUFFER_PRIVATE_H
#define MAGICK_BYTE_BUFFER_PRIVATE_H

#include "MagickCore/blob.h"

typedef struct _MagickByteBuffer
{
  Image
    *image;

  ssize_t
    offset,
    count;

  unsigned char
    data[MagickMinBufferExtent];
} MagickByteBuffer;

static inline int ReadMagickByteBuffer(MagickByteBuffer *buffer)
{
  if ((buffer->offset == buffer->count) && (buffer->offset > 0))
    {
      if (buffer->count != (ssize_t) sizeof(buffer->data)-1)
        return(EOF);
      buffer->offset=0;
      buffer->count=0;
    }
  if ((buffer->offset == 0) && (buffer->count == 0))
    {
      buffer->count=ReadBlob(buffer->image,sizeof(buffer->data)-1,
        buffer->data);
      if (buffer->count < 1)
        return(EOF);
    }
  return(buffer->data[buffer->offset++]);
}

static inline char *GetMagickByteBufferDatum(MagickByteBuffer *buffer)
{
  ssize_t
    count,
    i;

  if (buffer->offset != 0)
    {
      i=0;
      while (buffer->offset < buffer->count)
        buffer->data[i++]=buffer->data[buffer->offset++];
      count=ReadBlob(buffer->image,sizeof(buffer->data)-1-i,buffer->data+i);
      buffer->count=i;
      if (count > 0)
        buffer->count+=count;
      buffer->offset=0;
    }
  return((char *) buffer->data);
}

static void CheckMagickByteBuffer(MagickByteBuffer *buffer,
  const size_t length)
{
  if ((buffer->offset+length) > (ssize_t) sizeof(buffer->data))
    (void) GetMagickByteBufferDatum(buffer);
}

static MagickBooleanType CompareMagickByteBuffer(MagickByteBuffer *buffer,
  const char *p,const size_t length)
{
  const char
    *q;

  CheckMagickByteBuffer(buffer,length);
  q=(const char *) buffer->data+buffer->offset;
  if (LocaleNCompare(p,q,length) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

static inline void SkipMagickByteBuffer(MagickByteBuffer *buffer,
  const size_t length)
{
  CheckMagickByteBuffer(buffer,length);
  if ((buffer->offset+length) < buffer->count)
    buffer->offset+=length;
}

#endif

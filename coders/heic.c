/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        H   H  EEEEE  IIIII   CCCC                           %
%                        H   H  E        I    C                               %
%                        HHHHH  EEE      I    C                               %
%                        H   H  E        I    C                               %
%                        H   H  EEEEE  IIIII   CCCC                           %
%                                                                             %
%                                                                             %
%                         Read/Write Heic Image Format                        %
%                                                                             %
%                              Software Design                                %
%                               Anton Kortunov                                %
%                               December 2017                                 %
%                                                                             %
%                                                                             %
%                      Copyright 2017-2018 YANDEX LLC.                        %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/client.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/property.h"
#include "MagickCore/display.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/transform.h"
#include "MagickCore/memory_.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_HEIC_DELEGATE)
#include <libde265/de265.h>
#endif

/*
  Typedef declarations.
*/
#if defined(MAGICKCORE_HEIC_DELEGATE)

#define MAX_ASSOCS_COUNT 10
#define MAX_ITEM_PROPS 100
#define MAX_HVCC_ATOM_SIZE 1024
#define MAX_ATOMS_IN_BOX 100
#define BUFFER_SIZE 100

typedef struct _HEICItemInfo
{
  unsigned int
    type;

  unsigned int
    assocsCount;

  uint8_t
    assocs[MAX_ASSOCS_COUNT];

  unsigned int
    dataSource;

  unsigned int
    offset;

  unsigned int
    size;
} HEICItemInfo;

typedef struct _HEICItemProp
{
  unsigned int
    type;

  unsigned int
    size;

  uint8_t
    *data;
} HEICItemProp;

typedef struct _HEICGrid
{
  unsigned int
    id;

  unsigned int
    rowsMinusOne;

  unsigned int
    columnsMinusOne;

  unsigned int
    imageWidth;

  unsigned int
    imageHeight;
} HEICGrid;

typedef struct _HEICImageContext
{
  MagickBooleanType
    finished;

  int
    idsCount;

  HEICItemInfo
    *itemInfo;

  int
    itemPropsCount;

  HEICItemProp
    itemProps[MAX_ITEM_PROPS];

  unsigned int
    idatSize;

  uint8_t
    *idat;

  HEICGrid
    grid;

  de265_decoder_context
    *h265Ctx;

  Image
    *tmp;
} HEICImageContext;

typedef struct _DataBuffer {
    unsigned char
        *data;

    off_t
        pos;

    size_t
        size;
} DataBuffer;


#define ATOM(a,b,c,d) ((a << 24) + (b << 16) + (c << 8) + d)
#define ThrowImproperImageHeader(msg) { \
  (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError, \
    "ImproperImageHeader","`%s'",msg); \
}
#define ThrowAndReturn(msg) { \
  ThrowImproperImageHeader(msg) \
  return(MagickFalse); \
}

inline static unsigned int readInt(const unsigned char* data)
{
  unsigned int val = 0;

  val += (unsigned char)(data[0]) << 24;
  val += (unsigned char)(data[1]) << 16;
  val += (unsigned char)(data[2]) << 8;
  val += (unsigned char)(data[3]);

  return val;
}

inline static MagickSizeType DBChop(DataBuffer *head, DataBuffer *db, size_t size)
{
  if (size > (db->size - db->pos)) {
    return MagickFalse;
  }

  head->data = db->data + db->pos;
  head->pos = 0;
  head->size = size;

  db->pos += size;

  return MagickTrue;
}

inline static uint32_t DBReadUInt(DataBuffer *db)
{
  uint32_t val = 0;

  if (db->size - db->pos < 4) {
    db->pos = db->size;
    return 0;
  }

  val  = (unsigned char)(db->data[db->pos+0]) << 24;
  val += (unsigned char)(db->data[db->pos+1]) << 16;
  val += (unsigned char)(db->data[db->pos+2]) << 8;
  val += (unsigned char)(db->data[db->pos+3]);

  db->pos += 4;

  return val;
}

inline static uint16_t DBReadUShort(DataBuffer *db)
{
  uint16_t val = 0;

  if (db->size - db->pos < 2) {
    db->pos = db->size;
    return 0;
  }

  val  = (unsigned char)(db->data[db->pos+0]) << 8;
  val += (unsigned char)(db->data[db->pos+1]);

  db->pos += 2;

  return val;
}

inline static uint8_t DBReadUChar(DataBuffer *db)
{
  uint8_t val;

  if (db->size - db->pos < 2) {
    db->pos = db->size;
    return 0;
  }

  val = (unsigned char)(db->data[db->pos]);
  db->pos += 1;

  return val;
}

inline static size_t DBGetSize(DataBuffer *db)
{
  return db->size - db->pos;
}

inline static void DBSkip(DataBuffer *db, size_t skip)
{
  if (db->pos + skip > db->size)
  {
    db->pos = db->size;
  } else {
    db->pos += skip;
  }
}

static MagickBooleanType ParseAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception);

static MagickBooleanType ParseFullBox(Image *image, DataBuffer *db,
    unsigned int atom, HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    version, flags, i;

  flags = DBReadUInt(db);
  version = flags >> 24;
  flags &= 0xffffff;

  (void) flags;
  (void) version;

  if (DBGetSize(db) < 4) {
    ThrowAndReturn("atom is too short");
  }

  for (i = 0; i < MAX_ATOMS_IN_BOX && DBGetSize(db) > 0; i++) {
    (void) ParseAtom(image, db, ctx, exception);
  }

  return MagickTrue;
}

static MagickBooleanType ParseBox(Image *image, DataBuffer *db,
    unsigned int atom, HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    i;

  for (i = 0; i < MAX_ATOMS_IN_BOX && DBGetSize(db) > 0; i++) {
    (void) ParseAtom(image, db, ctx, exception);
  }

  return MagickTrue;
}

static MagickBooleanType ParseHvcCAtom(HEICItemProp *prop, ExceptionInfo *exception)
{
  size_t
    size, pos, count, i;

  uint8_t
    buffer[MAX_HVCC_ATOM_SIZE];

  uint8_t
    *p;

  p = prop->data;

  size = prop->size;
  memcpy(buffer, prop->data, size);

  pos = 22;
  if (pos >= size) {
    ThrowAndReturn("hvcC atom is too short");
  }

  count = buffer[pos++];

  for (i = 0; i < count && pos < size-3; i++) {
    size_t
      naluType, num, j;

    naluType = buffer[pos++] & 0x3f;
    (void) naluType;
    num = buffer[pos++] << 8;
    num += buffer[pos++];

    for (j = 0; j < num && pos < size-2; j++) {
      size_t
        naluSize;

      naluSize = buffer[pos++] << 8;
      naluSize += buffer[pos++];

      if ((pos + naluSize > size) ||
          (p + naluSize > prop->data + prop->size)) {
        ThrowAndReturn("hvcC atom is too short");
      }

      /* AnnexB NALU header */
      *p++ = 0;
      *p++ = 0;
      *p++ = 0;
      *p++ = 1;

      memcpy(p, buffer + pos, naluSize);
      p += naluSize;
      pos += naluSize;
    }
  }

  prop->size = p - prop->data;
  return MagickTrue;
}

static MagickBooleanType ParseIpcoAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    length, atom;

  HEICItemProp
    *prop;

  /*
     property indicies starts from 1
  */
  for (ctx->itemPropsCount = 1; ctx->itemPropsCount < MAX_ITEM_PROPS && DBGetSize(db) > 8; ctx->itemPropsCount++) {
    DataBuffer
      propDb;

    length = DBReadUInt(db);
    atom = DBReadUInt(db);

    if (ctx->itemPropsCount == MAX_ITEM_PROPS) {
      ThrowAndReturn("too many item properties");
    }

    prop = &(ctx->itemProps[ctx->itemPropsCount]);
    prop->type = atom;
    prop->size = length - 8;
    prop->data = (uint8_t *) AcquireMagickMemory(prop->size);
    if (DBChop(&propDb, db, prop->size) != MagickTrue) {
      ThrowAndReturn("incorrect read size");
    }
    memcpy(prop->data, propDb.data, prop->size);

    switch (prop->type) {
      case ATOM('h', 'v', 'c', 'C'):
        ParseHvcCAtom(prop, exception);
        break;
      default:
        break;
    }
  }

  return MagickTrue;
}

static MagickBooleanType ParseIinfAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    version, flags, count, i;

  if (DBGetSize(db) < 4) {
    ThrowAndReturn("atom is too short");
  }

  flags = DBReadUInt(db);
  version = flags >> 24;
  flags = 0xffffff;

  if (version == 0) {
   count = DBReadUShort(db);
  } else {
    count = DBReadUInt(db);
  }

  /*
     item indicies starts from 1
  */
  ctx->idsCount = count;
  ctx->itemInfo = (HEICItemInfo *)AcquireMagickMemory(sizeof(HEICItemInfo)*(count+1));
  if (ctx->itemInfo == (HEICItemInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);

  ResetMagickMemory(ctx->itemInfo, 0, sizeof(HEICItemInfo)*(count+1));

  for (i = 0; i < count && DBGetSize(db) > 0; i++)
  {
    (void) ParseAtom(image, db, ctx, exception);
  }

  return MagickTrue;
}

static MagickBooleanType ParseInfeAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    version, flags, id, type;

  if (DBGetSize(db) < 9) {
    ThrowAndReturn("atom is too short");
  }

  flags = DBReadUInt(db);
  version = flags >> 24;
  flags = 0xffffff;

  if (version != 2) {
    ThrowAndReturn("unsupported infe atom version");
  }

  id = DBReadUShort(db);
  DBSkip(db, 2);   /* item protection index */
  type = DBReadUInt(db);

  /*
     item indicies starts from 1
  */
  if (id > (ssize_t) ctx->idsCount) {
    ThrowAndReturn("item id is incorrect");
  }

  ctx->itemInfo[id].type = type;

  return MagickTrue;
}

static MagickBooleanType ParseIpmaAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    version, flags, count, i;

  if (DBGetSize(db) < 9) {
    ThrowAndReturn("atom is too short");
  }

  flags = DBReadUInt(db);
  version = flags >> 24;
  flags = 0xffffff;

  count = DBReadUInt(db);

  for (i = 0; i < count && DBGetSize(db) > 2; i++) {
    unsigned int
      id, assoc_count, j;

    if (version < 1) {
      id = DBReadUShort(db);
    } else {
      id = DBReadUInt(db);
    }

    /*
       item indicies starts from 1
       */
    if (id > (ssize_t) ctx->idsCount) {
      ThrowAndReturn("item id is incorrect");
    }

    assoc_count = DBReadUChar(db);

    if (assoc_count > MAX_ASSOCS_COUNT) {
      ThrowAndReturn("too many associations");
    }

    for (j = 0; j < assoc_count && DBGetSize(db) > 0; j++) {
      ctx->itemInfo[id].assocs[j] = DBReadUChar(db);
    }

    ctx->itemInfo[id].assocsCount = j;
  }

  return MagickTrue;
}

static MagickBooleanType ParseIlocAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception)
{
  unsigned int
    version, flags, tmp, count, i;

  if (DBGetSize(db) < 9) {
    ThrowAndReturn("atom is too short");
  }

  flags = DBReadUInt(db);
  version = flags >> 24;
  flags = 0xffffff;

  tmp = DBReadUChar(db);
  if (tmp != 0x44) {
    ThrowAndReturn("only offset_size=4 and length_size=4 are supported");
  }
  tmp = DBReadUChar(db);
  if (tmp != 0x00) {
    ThrowAndReturn("only base_offset_size=0 and index_size=0 are supported");
  }

  if (version < 2) {
    count = DBReadUShort(db);
  } else {
    count = DBReadUInt(db);
  }

  for (i = 0; i < count && DBGetSize(db) > 2; i++) {
    unsigned int
      id, ext_count;

    HEICItemInfo
      *item;

    id = DBReadUShort(db);

    /*
       item indicies starts from 1
    */
    if (id > (ssize_t) ctx->idsCount) {
      ThrowAndReturn("item id is incorrect");
    }

    item = &ctx->itemInfo[id];

    if (version == 1 || version == 2) {
      item->dataSource = DBReadUShort(db);
    }

    /*
     * data ref index
     */
    DBSkip(db, 2);
    ext_count = DBReadUShort(db);

    if (ext_count != 1) {
      ThrowAndReturn("only one excention per item is supported");
    }

    item->offset = DBReadUInt(db);
    item->size = DBReadUInt(db);
  }

  return MagickTrue;
}

static MagickBooleanType ParseAtom(Image *image, DataBuffer *db,
    HEICImageContext *ctx, ExceptionInfo *exception)
{
  DataBuffer
    atomDb;

  MagickBooleanType
    status;

  MagickSizeType
    atom_size;

  unsigned int
    atom;

  if (DBGetSize(db) < 8)
  {
    ThrowAndReturn("atom is too short");
  }

  atom_size = DBReadUInt(db);
  atom = DBReadUInt(db);

  if (atom_size == 1) {
    /* Only 32 bit atom size are supported */
    DBReadUInt(db);
    atom_size = DBReadUInt(db);
  }

  if (atom_size - 8 > DBGetSize(db))
  {
    ThrowAndReturn("atom is too short");
  }

  if (DBChop(&atomDb, db, atom_size - 8) != MagickTrue)
  {
    ThrowAndReturn("unable to read atom");
  }

  status = MagickTrue;

  switch (atom)
  {
    case ATOM('i', 'r', 'e', 'f'):
      status = ParseFullBox(image, &atomDb, atom, ctx, exception);
      break;
    case ATOM('i', 'p', 'r', 'p'):
      status = ParseBox(image, &atomDb, atom, ctx, exception);
      break;
    case ATOM('i', 'i', 'n', 'f'):
      status = ParseIinfAtom(image, &atomDb, ctx, exception);
      break;
    case ATOM('i', 'n', 'f', 'e'):
      status = ParseInfeAtom(image, &atomDb, ctx, exception);
      break;
    case ATOM('i', 'p', 'c', 'o'):
      status = ParseIpcoAtom(image, &atomDb, ctx, exception);
      break;
    case ATOM('i', 'p', 'm', 'a'):
      status = ParseIpmaAtom(image, &atomDb, ctx, exception);
      break;
    case ATOM('i', 'l', 'o', 'c'):
      status = ParseIlocAtom(image, &atomDb, ctx, exception);
      break;
    case ATOM('i', 'd', 'a', 't'):
      {
        ctx->idatSize = atom_size - 8;
        ctx->idat = (uint8_t *) AcquireMagickMemory(ctx->idatSize);
        if (ctx->idat == NULL)
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);

        memcpy(ctx->idat, atomDb.data, ctx->idatSize);
      }
      break;
    default:
      break;
  }

  return status;
}


static MagickBooleanType ParseRootAtom(Image *image,MagickSizeType *size,
  HEICImageContext *ctx,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickSizeType
    atom_size;

  unsigned int
    atom;

  if (*size < 8)
    ThrowAndReturn("atom is too short");

  atom_size = ReadBlobMSBLong(image);
  atom = ReadBlobMSBLong(image);

  if (atom_size == 1) {
    ReadBlobMSBLong(image);
    atom_size = ReadBlobMSBLong(image);
  }


  if (atom_size > *size)
    ThrowAndReturn("atom is too short");

  status = MagickTrue;

  switch (atom)
  {
    case ATOM('f', 't', 'y', 'p'):
      DiscardBlobBytes(image, atom_size-8);
      break;
    case ATOM('m', 'e', 't', 'a'):
      {
        DataBuffer
          db;

        size_t
          count;

        db.pos = 0;
        db.size = atom_size - 8;
        db.data = (unsigned char *) AcquireMagickMemory(db.size);
        if (db.data == NULL)
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);

        count = ReadBlob(image, db.size, db.data);
        if (count != db.size) {
          RelinquishMagickMemory((void *)db.data);
          ThrowAndReturn("unable to read data");
        }

        /*
         * Meta flags and version
         */
        /* DBSkip(&db, 4); */
        status = ParseFullBox(image, &db, atom, ctx, exception);
        RelinquishMagickMemory((void *)db.data);
      }
      break;
    case ATOM('m', 'd', 'a', 't'):
      ctx->finished = MagickTrue;
      break;
    default:
      DiscardBlobBytes(image, atom_size-8);
      break;
  }
  *size=*size-atom_size;
  return(status);
}

static MagickBooleanType decodeGrid(HEICImageContext *ctx,
  ExceptionInfo *exception)
{
  unsigned int
    i, flags;

  for (i = 1; i <= (ssize_t) ctx->idsCount; i++) {
    HEICItemInfo
      *info = &ctx->itemInfo[i];
    if (info->type != ATOM('g','r','i','d'))
      continue;
    if (info->dataSource != 1) {
      ThrowAndReturn("unsupport data source type");
    }

    if (ctx->idatSize < 8) {
      ThrowAndReturn("idat is too small");
    }

    flags = ctx->idat[1];

    ctx->grid.rowsMinusOne = ctx->idat[2];
    ctx->grid.columnsMinusOne = ctx->idat[3];

    if (flags & 1) {
      ThrowAndReturn("Only 16 bits sizes are supported");
    }

    ctx->grid.imageWidth = (ctx->idat[4] << 8) + ctx->idat[5];
    ctx->grid.imageHeight = (ctx->idat[6] << 8) + ctx->idat[7];

    ctx->grid.id = i;

    return MagickTrue;
  }
  return MagickFalse;
}

static MagickBooleanType decodeH265Image(Image *image, HEICImageContext *ctx, unsigned int id, ExceptionInfo *exception)
{
  unsigned char
    *buffer = NULL;

  unsigned char
    *p;

  size_t
    count, pos, nal_unit_size;

  int
    more, i;

  unsigned int
    x_offset, y_offset;

  de265_error
    err;

  pos = 0;
  de265_reset(ctx->h265Ctx);

  x_offset = 512 * ((id-1) % (ctx->grid.columnsMinusOne + 1));
  y_offset = 512 * ((id-1) / (ctx->grid.columnsMinusOne + 1));

  for (i = 0; i < (ssize_t) ctx->itemInfo[id].assocsCount; i++) {
    ssize_t
      assoc;

    assoc = ctx->itemInfo[id].assocs[i] & 0x7f;
    if (assoc > ctx->itemPropsCount) {
      ThrowImproperImageHeader("incorrect item property index");
      goto err_out_free;
    }

    switch (ctx->itemProps[assoc].type) {
      case ATOM('h', 'v', 'c', 'C'):
        err = de265_push_data(ctx->h265Ctx, ctx->itemProps[assoc].data, ctx->itemProps[assoc].size, pos, (void*)2);
        if (err != DE265_OK) {
          ThrowImproperImageHeader("unable to push data");
          goto err_out_free;
        }

        pos += ctx->itemProps[assoc].size;
        break;
      case ATOM('c', 'o', 'l', 'r'):
        {
          StringInfo
            *profile;

          if (ctx->itemProps[assoc].size < 16)
              continue;

          profile=BlobToStringInfo(ctx->itemProps[assoc].data + 4, ctx->itemProps[assoc].size - 4);
          (void) SetImageProfile(image, "icc", profile, exception);
          profile=DestroyStringInfo(profile);
          break;
        }
    }
  }

  buffer = (unsigned char *) AcquireMagickMemory(ctx->itemInfo[id].size);
  if (buffer == NULL) {
    (void) ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
      "MemoryAllocationFailed","`%s'",image->filename);
    goto err_out_free;
  }

  SeekBlob(image, ctx->itemInfo[id].offset, SEEK_SET);
  count = ReadBlob(image, ctx->itemInfo[id].size, buffer);
  if (count != ctx->itemInfo[id].size) {
    ThrowImproperImageHeader("unable to read data");
    goto err_out_free;
  }

  /*
   * AVCC to AnnexB
   */
  for (p = buffer; p < buffer + ctx->itemInfo[id].size; /* void */) {
    nal_unit_size = readInt(p);
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 1;
    p += nal_unit_size + 4;
  }

  err = de265_push_data(ctx->h265Ctx, buffer, ctx->itemInfo[id].size, pos, (void*)2);
  if (err != DE265_OK) {
    ThrowImproperImageHeader("unable to push data");
    goto err_out_free;
  }

  err = de265_flush_data(ctx->h265Ctx);
  if (err != DE265_OK) {
    ThrowImproperImageHeader("unable to flush data");
    goto err_out_free;
  }

  more = 0;

  do {
    err = de265_decode(ctx->h265Ctx, &more);
    if (err != DE265_OK) {
      ThrowImproperImageHeader("unable to decode data");
      goto err_out_free;
    }

    while (1) {
      de265_error warning = de265_get_warning(ctx->h265Ctx);
      if (warning==DE265_OK) {
        break;
      }

      ThrowBinaryException(CoderWarning,(const char *)NULL,
        de265_get_error_text(warning));
    }

    const struct de265_image* img = de265_get_next_picture(ctx->h265Ctx);
    if (img) {
      const uint8_t *planes[3];
      int dims[3][2];
      int strides[3];

      int c;
      for (c = 0; c < 3; c++) {
        planes[c] = de265_get_image_plane(img, c, &(strides[c]));
        dims[c][0] = de265_get_image_width(img, c);
        dims[c][1] = de265_get_image_height(img, c);
      }


      assert(dims[0][0] == 512);
      assert(dims[0][1] == 512);
      assert(dims[1][0] == 256);
      assert(dims[1][1] == 256);
      assert(dims[2][0] == 256);
      assert(dims[2][1] == 256);

      Image* chroma;

      chroma = ctx->tmp;

      int x, y;

      for (y = 0; y < 256; y++) {
        register Quantum *q;
        register const uint8_t *p1 = planes[1] + y * strides[1];
        register const uint8_t *p2 = planes[2] + y * strides[2];

        q = QueueAuthenticPixels(chroma, 0, y, 256, 1, exception);
        if (q == NULL) {
          goto err_out_free;
        }

        for (x = 0; x < 256; x++) {
          SetPixelGreen(chroma, ScaleCharToQuantum(*p1++), q);
          SetPixelBlue(chroma, ScaleCharToQuantum(*p2++), q);
          q+=GetPixelChannels(chroma);
        }

        if (SyncAuthenticPixels(chroma, exception) == MagickFalse) {
          goto err_out_free;
        }
      }

      Image* resized_chroma = ResizeImage(chroma, 512, 512, TriangleFilter, exception);
      if (resized_chroma == NULL) {
        goto err_out_free;
      }

      for (y = 0; y < 512; y++) {
        register Quantum *q;
        register const Quantum *p;
        register const uint8_t *l = planes[0] + y * strides[0];

        q = QueueAuthenticPixels(image, x_offset, y_offset + y, 512, 1, exception);
        if (q == NULL) {
          goto err_loop_free;
        }

        p = GetVirtualPixels(resized_chroma, 0, y, 512, 1, exception);
        if (p == NULL) {
          goto err_loop_free;
        }

        for (x = 0; x < 512; x++) {
          SetPixelRed(image, ScaleCharToQuantum(*l), q);
          SetPixelGreen(image, GetPixelGreen(resized_chroma, p), q);
          SetPixelBlue(image, GetPixelBlue(resized_chroma, p), q);
          l++;
          q+=GetPixelChannels(image);
          p+=GetPixelChannels(resized_chroma);
        }

        if (SyncAuthenticPixels(image, exception) == MagickFalse) {
          goto err_loop_free;
        }
      }

      if (resized_chroma)
        resized_chroma = DestroyImage(resized_chroma);

      more = 0;
      de265_release_next_picture(ctx->h265Ctx);
      break;

err_loop_free:
      if (resized_chroma)
        resized_chroma = DestroyImage(resized_chroma);

      de265_release_next_picture(ctx->h265Ctx);

      goto err_out_free;
    }
  } while (more);

  de265_reset(ctx->h265Ctx);
  buffer = (unsigned char *) RelinquishMagickMemory(buffer);
  return MagickTrue;

err_out_free:
  de265_reset(ctx->h265Ctx);
  buffer = (unsigned char *) RelinquishMagickMemory(buffer);
  return MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d H E I C I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadHEICImage retrieves an image via a file descriptor, decodes the image,
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadHEICImage method is:
%
%      Image *ReadHEICImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadHEICImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  Image
    *cropped = NULL;

  MagickBooleanType
    status;

  RectangleInfo
      crop_info;

  MagickSizeType
    length;

  ssize_t
    count,
    i;

  HEICImageContext
    ctx;

  ResetMagickMemory(&ctx, 0, sizeof(ctx));

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
  {
    image=DestroyImageList(image);
    return((Image *) NULL);
  }
  cropped=(Image *) NULL;

  length=GetBlobSize(image);
  count = MAX_ATOMS_IN_BOX;
  while (length && ctx.finished == MagickFalse && count--)
  {
    if (ParseRootAtom(image, &length, &ctx, exception) == MagickFalse)
      goto cleanup;
  }

  if (ctx.finished != MagickTrue)
    goto cleanup;

  /*
     Initialize h265 decoder
  */
  ctx.h265Ctx = de265_new_decoder();
  if (ctx.h265Ctx == NULL) {
    ThrowImproperImageHeader("unable to initialize decode");
    goto cleanup;
  }

  if (decodeGrid(&ctx, exception) != MagickTrue)
    goto cleanup;

  count = (ctx.grid.rowsMinusOne + 1) * (ctx.grid.columnsMinusOne + 1);

  image->columns = 512 * (ctx.grid.columnsMinusOne + 1);
  image->rows = 512 * (ctx.grid.rowsMinusOne + 1);
  image->depth=8;

  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    goto cleanup;

  if (image_info->ping == MagickFalse)
    {
      ctx.tmp = CloneImage(image, 256, 256, MagickTrue, exception);
      if (ctx.tmp == NULL) {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
        goto cleanup;
      }

      DuplicateBlob(ctx.tmp, image);

      for (i = 0; i < count; i++) {
        decodeH265Image(image, &ctx, i+1, exception);
      }
    }

  crop_info.x = 0;
  crop_info.y = 0;

  for (i = 0; i < ctx.itemInfo[ctx.grid.id].assocsCount; i++) {
    ssize_t
      assoc;

    assoc = ctx.itemInfo[ctx.grid.id].assocs[i] & 0x7f;
    if (assoc > ctx.itemPropsCount) {
      ThrowImproperImageHeader("incorrect item property index");
      goto cleanup;
    }

    switch (ctx.itemProps[assoc].type) {
      case ATOM('i', 's', 'p', 'e'):
        if (ctx.itemProps[assoc].size < 12) {
          ThrowImproperImageHeader("ispe atom is too short");
          goto cleanup;
        }
        crop_info.width = readInt(ctx.itemProps[assoc].data+4);
        crop_info.height = readInt(ctx.itemProps[assoc].data+8);
        break;

      case ATOM('i', 'r', 'o', 't'):
        {
          const char *value;

          if (ctx.itemProps[assoc].size < 1) {
            ThrowImproperImageHeader("irot atom is too short");
            goto cleanup;
          }

          switch (ctx.itemProps[assoc].data[0])
          {
            case 0:
              image->orientation = TopLeftOrientation;
              value = "1";
              break;
            case 1:
              image->orientation = RightTopOrientation;
              value = "8";
              break;
            case 2:
              image->orientation = BottomRightOrientation;
              value = "3";
              break;
            case 3:
              image->orientation = LeftTopOrientation;
              value = "6";
              break;
            default:
              value = "1";
          }

          SetImageProperty(image, "exif:Orientation", value, exception);
        }
        break;
    }
  }

  for (i = 1; i <= ctx.idsCount; i++) {
    unsigned char
      *buffer = NULL;

    StringInfo
      *profile;

    HEICItemInfo
      *info = &ctx.itemInfo[i];

    if (info->type != ATOM('E','x','i','f'))
      continue;

    buffer = (unsigned char *) AcquireMagickMemory(info->size);
    if (buffer == NULL) {
      (void) ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
        "MemoryAllocationFailed","`%s'",image->filename);
      goto cleanup;
    }

    SeekBlob(image, info->offset+4, SEEK_SET);
    count = ReadBlob(image, info->size-4, buffer);
    profile=BlobToStringInfo(buffer, count);
    SetImageProfile(image, "exif", profile, exception);

    profile = DestroyStringInfo(profile);
    RelinquishMagickMemory(buffer);
  }

  cropped = CropImage(image, &crop_info, exception);
  image = DestroyImage(image);
  if (cropped != NULL)
    {
      if (image_info->ping != MagickFalse)
        cropped->colorspace=YCbCrColorspace;
      else
        SetImageColorspace(cropped,YCbCrColorspace,exception);
    }

cleanup:
  if (image) {
    image = DestroyImage(image);
  }
  if (ctx.h265Ctx) {
      de265_free_decoder(ctx.h265Ctx);
  }
  if (ctx.tmp) {
      ctx.tmp = DestroyImage(ctx.tmp);
  }
  if (ctx.idat) {
      ctx.idat = (uint8_t *) RelinquishMagickMemory(ctx.idat);
  }
  if (ctx.itemInfo) {
      ctx.itemInfo = (HEICItemInfo *) RelinquishMagickMemory(ctx.itemInfo);
  }
  for (i = 1; i <= ctx.itemPropsCount; i++) {
      if (ctx.itemProps[i].data) {
          ctx.itemProps[i].data = (uint8_t *) RelinquishMagickMemory(ctx.itemProps[i].data);
      }
  }
  return cropped;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H E I C                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHEIC() returns MagickTrue if the image format type, identified by the
%  magick string, is Heic.
%
%  The format of the IsHEIC method is:
%
%      MagickBooleanType IsHEIC(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsHEIC(const unsigned char *magick,const size_t length)
{
  if (length < 12)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick+8,"heic",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H E I C I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterHEICImage() adds attributes for the HEIC image format to the list of
%  supported formats.  The attributes include the image format tag, a method
%  to read and/or write the format, whether the format supports the saving of
%  more than one frame to the same file or blob, whether the format supports
%  native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterHEICImage method is:
%
%      size_t RegisterHEICImage(void)
%
*/
ModuleExport size_t RegisterHEICImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("HEIC","HEIC","Apple High efficiency Image Format");
#if defined(MAGICKCORE_HEIC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadHEICImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/x-heic");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H E I C I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterHEICImage() removes format registrations made by the HEIC module
%  from the list of supported formats.
%
%  The format of the UnregisterHEICImage method is:
%
%      UnregisterHEICImage(void)
%
*/
ModuleExport void UnregisterHEICImage(void)
{
  (void) UnregisterMagickInfo("HEIC");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         DDDD       J  V   V  U   U                          %
%                         D   D      J  V   V  U   U                          %
%                         D   D      J  V   V  U   U                          %
%                         D   D  J   J   V V   U   U                          %
%                         DDDD    JJJ     V     UUU                           %
%                                                                             %
%                                                                             %
%                             Read DjVu Images.                               %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#if defined(MAGICKCORE_DJVU_DELEGATE)
#include <libdjvu/ddjvuapi.h>
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s D J V U                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDJVU() returns MagickTrue if the image format type, identified by the
%  magick string, is DJVU.
%
%  The format of the IsDJVU method is:
%
%      MagickBooleanType IsDJVU(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsDJVU(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick,"AT&TFORM",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_DJVU_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D J V U I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDJVUImage() reads DJVU image and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image or set of images.
%
%  The format of the ReadDJVUImage method is:
%
%      Image *ReadDJVUImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _LoadContext
   LoadContext;

struct _LoadContext
{
  ddjvu_context_t* context;
  ddjvu_document_t *document;
  ddjvu_page_t *page;
  int streamid;
  int pages;
  Image *image;
};

#define BLOCKSIZE  65536
#if 0
static void
pump_data(Image *image, LoadContext* lc)
{
        int blocksize = BLOCKSIZE;
        char data[BLOCKSIZE];
        int size;

        /* i might check for a condition! */
        while ((size = (size_t) ReadBlob(image,(size_t) blocksize,data)) == blocksize) {
                ddjvu_stream_write(lc->document, lc->streamid, data, size);
        }
        if (size)
                ddjvu_stream_write(lc->document, lc->streamid, data, size);
        ddjvu_stream_close(lc->document, lc->streamid, 0);
}
#endif

/* returns NULL only after all is delivered! */
static ddjvu_message_t*
pump_data_until_message(LoadContext *lc,Image *image) /* ddjvu_context_t *context, type ddjvu_document_type_t */
{
        unsigned long blocksize = BLOCKSIZE;
        unsigned char data[BLOCKSIZE];
        unsigned long size;
        ddjvu_message_t *message;

        /* i might check for a condition! */
        while (!(message = ddjvu_message_peek(lc->context))
               && (size = (unsigned long) ReadBlob(image,(size_t) blocksize,data)) == blocksize) {
                ddjvu_stream_write(lc->document, lc->streamid, (char *) data, size);
        }
        if (message)
                return message;
        if (size)
                ddjvu_stream_write(lc->document, lc->streamid, (char *) data, size);
        ddjvu_stream_close(lc->document, lc->streamid, 0);
        return NULL;
}
#define DEBUG 0

#if DEBUG
static const char*
message_tag_name(ddjvu_message_tag_t tag)
{
   static char* names[] =
      {
         "ERROR",
         "INFO",
         "NEWSTREAM",
         "DOCINFO",
         "PAGEINFO",
         "RELAYOUT",
         "REDISPLAY",
         "CHUNK",
         "THUMBNAIL",
         "PROGRESS",
      };
   if (tag <= DDJVU_PROGRESS)
      return names[tag];
   else {
      /* bark! */
      return 0;
   }
}
#endif

/* write out nice info on the message,
 * and store in *user* data the info on progress.
 * */
int
process_message(ddjvu_message_t *message)
{

#if 0
   ddjvu_context_t* context= message->m_any.context;
#endif

   if (! message)
      return(-1);
#if DEBUG
   printf("*** %s: %s.\n",__FUNCTION__, message_tag_name(message->m_any.tag));
#endif


   switch (message->m_any.tag){
   case DDJVU_DOCINFO:
   {
      ddjvu_document_t* document= message->m_any.document;
      /* ddjvu_document_decoding_status  is set by libdjvu! */
      /* we have some info on the document  */
      LoadContext *lc = (LoadContext *) ddjvu_document_get_user_data(document);
      lc->pages = ddjvu_document_get_pagenum(document);
#if DEBUG
      printf("the doc has %d pages\n", ddjvu_document_get_pagenum(document));
#endif
      break;
   }
   case DDJVU_CHUNK:
#if DEBUG
           printf("the name of the chunk is: %s\n", message->m_chunk.chunkid);
#endif
           break;


   case DDJVU_RELAYOUT:
   case DDJVU_PAGEINFO:
   {
#if 0
      ddjvu_page_t* page = message->m_any.page;
      page_info* info = ddjvu_page_get_user_data(page);

      printf("page decoding status: %d %s%s%s\n",
             ddjvu_page_decoding_status(page),
             status_color, status_name(ddjvu_page_decoding_status(page)), color_reset);

      printf("the page LAYOUT changed: width x height: %d x %d @ %d dpi. Version %d, type %d\n",
             // printf("page info:\n width x height: %d x %d @ %d dpi, version %d, type %d\n",
             ddjvu_page_get_width(page),
             ddjvu_page_get_height(page),
             ddjvu_page_get_resolution(page),
             ddjvu_page_get_version(page),
             /* DDJVU_PAGETYPE_BITONAL */
             ddjvu_page_get_type(page));

      info->info = 1;
#endif
      break;
   }

   case DDJVU_REDISPLAY:
   {

#if 0
    ddjvu_page_t* page = message->m_any.page;
      page_info* info = ddjvu_page_get_user_data(page);

      printf("the page can/should be REDISPLAYED\n");
      info->display = 1;
#endif
      break;
   }

   case DDJVU_PROGRESS:
#if DEBUG
           printf("PROGRESS:\n");
#endif
           break;
   case DDJVU_ERROR:
           printf("simply ERROR!\n message:\t%s\nfunction:\t%s(file %s)\nlineno:\t%d\n",
                  message->m_error.message,
                  message->m_error.function,
                  message->m_error.filename,
                  message->m_error.lineno);
           break;
   case DDJVU_INFO:
#if DEBUG
           printf("INFO: %s!\n", message->m_info.message);
#endif
           break;
   default:
      printf("unexpected\n");
   };
  return(message->m_any.tag);
}


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#define RGB 1

/*
 * DjVu advertised readiness to provide bitmap: So get it!
 * we use the RGB format!
 */
static void
get_page_image(LoadContext *lc, ddjvu_page_t *page, int x, int y, int w, int h, QuantumInfo* quantum_info)
{
  ddjvu_format_t
    *format;

  ddjvu_page_type_t
    type;

  Image
    *image;

  int
    ret,
    stride;

  unsigned char
    *q;

        ddjvu_rect_t rect;
        rect.x = x;
        rect.y = y;
        rect.w = (unsigned int) w;             /* /10 */
        rect.h = (unsigned int) h;             /* /10 */

        image = lc->image;
        type = ddjvu_page_get_type(lc->page);

        /* stride of this temporary buffer: */
        stride = (type == DDJVU_PAGETYPE_BITONAL)?
                (lc->image->columns + 7)/8:
                lc->image->columns *3;

        q = (unsigned char *) AcquireQuantumMemory(lc->image->rows,stride);


        format = ddjvu_format_create(
                (type == DDJVU_PAGETYPE_BITONAL)?DDJVU_FORMAT_LSBTOMSB : DDJVU_FORMAT_RGB24,
                /* DDJVU_FORMAT_RGB24
                 * DDJVU_FORMAT_RGBMASK32*/
                /* DDJVU_FORMAT_RGBMASK32 */
                0, NULL);

#if 0
        /* fixme:  ThrowReaderException is a macro, which uses  `exception' variable */
        if (format == NULL)
                {
                        abort();
                        /* ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed"); */
                }

#endif
        ddjvu_format_set_row_order(format, 1);
        ddjvu_format_set_y_direction(format, 1);

        ret = ddjvu_page_render(page,
                                    DDJVU_RENDER_COLOR, /* ddjvu_render_mode_t */
                                    &rect,
                                    &rect,     /* mmc: ?? */
                                    format,
                                    stride, /* ?? */
                                    (char*)q);
        ddjvu_format_release(format);


        if (type == DDJVU_PAGETYPE_BITONAL) {
                /*  */
#if DEBUG
                printf("%s: expanding BITONAL page/image\n", __FUNCTION__);
#endif
                register IndexPacket *indexes;
                unsigned long bit, byte;

                for (y=0; y < (long) image->rows; y++)
                        {
                                PixelPacket * o = QueueAuthenticPixels(image,0,y,image->columns,1,&image->exception);
                                if (o == (PixelPacket *) NULL)
                                        break;
                                indexes=GetAuthenticIndexQueue(image);
                                bit=0;
                                byte=0;

                                /* fixme:  the non-aligned, last =<7 bits ! that's ok!!!*/
                                for (x= 0; x < (long) image->columns; x++)
                                        {
                                                if (bit == 0) byte= (unsigned long) q[(y * stride) + (x / 8)];

                                                indexes[x]=(IndexPacket) (((byte & 0x01) != 0) ? 0x00 : 0x01);
                                                bit++;
                                                if (bit == 8)
                                                        bit=0;
                                                byte>>=1;
                                        }
                                if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
                                        break;
                        }
                SyncImage(image);
        } else {
#if DEBUG
                printf("%s: expanding PHOTO page/image\n", __FUNCTION__);
#endif
                /* now transfer line-wise: */
                long i;
#if 0
                /* old: */
                char* r;
#else
                register PixelPacket *r;
#endif

                for (i = 0;i< (long) lc->image->rows; i++)
                        {
#if DEBUG
                               if (i % 1000 == 0) printf("%d\n",i);
#endif
                               r = QueueAuthenticPixels(lc->image,0,i,lc->image->columns,1,&image->exception);

                                ImportQuantumPixels(lc->image,
                                                    (CacheView *) NULL,
                                                    quantum_info,
                                                    RGBQuantum, /*GrayQuantum*/
                                                    q+i*stride,&image->exception);
                                SyncAuthenticPixels(lc->image,&image->exception);
                        }
        }
        q=(unsigned char *) RelinquishMagickMemory(q);
}


#if defined(MAGICKCORE_DJVU_DELEGATE)

static int
get_page_line(LoadContext *lc, int row, QuantumInfo* quantum_info)
{
  ddjvu_format_t
    *format;

  int
    ret;

  size_t
    stride;

  unsigned char
    *q;

        ddjvu_rect_t rect, pagerect;
        rect.x = 0;
        rect.y = row;
        rect.w = lc->image->columns;             /* /10 */
        rect.h = 1;             /* /10 */

        pagerect.x = 0;
        pagerect.y = 0;
        pagerect.w = lc->image->columns;
        pagerect.h = lc->image->rows;


        format = ddjvu_format_create(
#if RGB
                DDJVU_FORMAT_RGB24
#else
                DDJVU_FORMAT_GREY8
#endif
                ,
                0, NULL);
        ddjvu_format_set_row_order(format, 1);
        ddjvu_format_set_y_direction(format, 1);

        stride=1;
#if RGB
        stride=3;
#endif
        q = (unsigned char *) AcquireQuantumMemory(lc->image->columns,stride);

        ret = ddjvu_page_render(lc->page,
                                    DDJVU_RENDER_COLOR, /* ddjvu_render_mode_t */
                                    &pagerect,
                                    &rect,     /* mmc: ?? */
                                    format,
                                    pagerect.w * 3, /* ?? */
                                    (char*)q);

        ImportQuantumPixels(lc->image,
                            (CacheView *) NULL,
                            quantum_info,
#if RGB
                            RGBQuantum
#else
                            GrayQuantum
#endif
                            ,q,&lc->image->exception);
        q=(unsigned char *) RelinquishMagickMemory(q);
        ddjvu_format_release(format);
        return ret;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d O n e D J V U I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadOneDJVUImage() reads a Portable Network Graphics (DJVU) image file
%  (minus the 8-byte signature)  and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ReadOneDJVUImage method is:
%
%      Image *ReadOneDJVUImage(MngInfo *mng_info, const ImageInfo *image_info,
%         ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o mng_info: Specifies a pointer to a MngInfo structure.
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static Image *ReadOneDJVUImage(LoadContext* lc,const int pagenum,
  const ImageInfo *image_info,ExceptionInfo *exception)
{
  ddjvu_page_type_t
     type;

  ddjvu_pageinfo_t info;
  QuantumInfo *quantum_info;
  ddjvu_message_t *message;
  Image *image;
  int logging;
  int tag;

        /* so, we know that the page is there! Get its dimension, and  */

        /* Read one DJVU image */
        image = lc->image;

        /* register PixelPacket *q; */

        logging=LogMagickEvent(CoderEvent,GetMagickModule(), "  enter ReadOneDJVUImage()");

#if DEBUG
        printf("====  Loading the page %d\n", pagenum);
#endif
        lc->page = ddjvu_page_create_by_pageno(lc->document, pagenum); /*  0? */

        /* pump data untill the page is ready for rendering. */
        tag=(-1);
        do {
                while ((message = ddjvu_message_peek(lc->context)))
                        {
                                tag=process_message(message);
                                if (tag == 0) break;
                                ddjvu_message_pop(lc->context);
                        }
                /* fixme: maybe exit? */
                /* if (lc->error) break; */

                message = pump_data_until_message(lc,image);
                if (message)
                        do {
                                tag=process_message(message);
                                if (tag == 0) break;
                                ddjvu_message_pop(lc->context);
                        } while ((message = ddjvu_message_peek(lc->context)));
        } while (!ddjvu_page_decoding_done(lc->page));

        ddjvu_document_get_pageinfo(lc->document, pagenum, &info);

        image->x_resolution = (float) info.dpi;
        image->y_resolution =(float) info.dpi;
        if (image_info->density != (char *) NULL)
          {
            int
              flags;

            GeometryInfo
              geometry_info;

            /*
              Set rendering resolution.
            */
            flags=ParseGeometry(image_info->density,&geometry_info);
            image->x_resolution=geometry_info.rho;
            image->y_resolution=geometry_info.sigma;
            if ((flags & SigmaValue) == 0)
              image->y_resolution=image->x_resolution;
            info.width*=image->x_resolution/info.dpi;
            info.height*=image->y_resolution/info.dpi;
            info.dpi=(long) MagickMax(image->x_resolution,image->y_resolution);
          }
        type = ddjvu_page_get_type(lc->page);

        /* double -> float! */
        /* image->gamma = (float)ddjvu_page_get_gamma(lc->page); */

        /* mmc:  set  image->depth  */
        /* mmc:  This from the type */

        image->columns=(unsigned long) info.width;
        image->rows=(unsigned long) info.height;

        /* mmc: bitonal should be palettized, and compressed! */
        if (type == DDJVU_PAGETYPE_BITONAL){
                image->colorspace = GRAYColorspace;
                image->storage_class = PseudoClass;
                image->depth =  8UL;    /* i only support that? */
                image->colors= 2;
                if (AcquireImageColormap(image,image->colors) == MagickFalse)
                  ThrowReaderException(ResourceLimitError,
                   "MemoryAllocationFailed");
        } else {
                image->colorspace = RGBColorspace;
                image->storage_class = DirectClass;
                /* fixme:  MAGICKCORE_QUANTUM_DEPTH ?*/
                image->depth =  8UL;    /* i only support that? */

                image->matte = MagickTrue;
                /* is this useful? */
        }
#if DEBUG
        printf("now filling %ld x %ld\n", image->columns,image->rows);
#endif


#if 1                           /* per_line */
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

        /* q = QueueAuthenticPixels(image,0,0,image->columns,image->rows); */
        get_page_image(lc, lc->page, 0, 0, info.width, info.height, quantum_info);
#else
        int i;
        for (i = 0;i< image->rows; i++)
                {
                        printf("%d\n",i);
                        q = QueueAuthenticPixels(image,0,i,image->columns,1);
                        get_page_line(lc, i, quantum_info);
                        SyncAuthenticPixels(image);
                }

#endif /* per_line */


#if DEBUG
        printf("END: finished filling %ld x %ld\n", image->columns,image->rows);
#endif

        SyncImage(image);
        quantum_info=DestroyQuantumInfo(quantum_info);
        /* indexes=GetAuthenticIndexQueue(image); */
        /* mmc: ??? Convert PNM pixels to runlength-encoded MIFF packets. */
        /* image->colors =  */

        /* how is the line padding  / stride? */

        if (lc->page) {
                ddjvu_page_release(lc->page);
                lc->page = NULL;
        }

        /* image->page.y=mng_info->y_off[mng_info->object_id]; */
        if (tag == 0)
          image=DestroyImage(image);
        return image;
        /* end of reading one DJVU page/image */
}

#if 0
/* palette */
  if (AcquireImageColormap(image,2) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Monochrome colormap.   mmc: this the default!
  */
  image->colormap[0].red=QuantumRange;
  image->colormap[0].green=QuantumRange;
  image->colormap[0].blue=QuantumRange;
  image->colormap[1].red=0;
  image->colormap[1].green=0;
  image->colormap[1].blue=0;
#endif

static void djvu_close_lc(LoadContext* lc)
{
        if (lc->document)
                ddjvu_document_release(lc->document);
        if (lc->context)
                ddjvu_context_release(lc->context);
        if (lc->page)
                ddjvu_page_release(lc->page);
        RelinquishMagickMemory(lc);
}

static Image *ReadDJVUImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  const char
    *url;

  ddjvu_message_t
    *message;

  Image
    *image,
    *images;

  int
    logging,
    use_cache;

  LoadContext
    *lc;

  MagickBooleanType
    status;

  register long
    i;

  /*
   * Open image file.
   */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);


  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s", image_info->filename);

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);


  logging = LogMagickEvent(CoderEvent,GetMagickModule(),"enter ReadDJVUImage()");

  image = AcquireImage(image_info); /* mmc: ?? */


  lc = (LoadContext *) NULL;
  status = OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    ThrowReaderException(FileOpenError,"UnableToOpenFile");
  /*
    Verify DJVU signature.
  */
#if 0
  count = ReadBlob(image,8,(unsigned char *) magic_number);

  /* IsDJVU(const unsigned char *magick,const size_t length) */
  if (memcmp(magic_number,"AT&TFORM",8) != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
#endif


  /*
   * Allocate a LoadContext structure.
   */
  lc = (LoadContext *) AcquireMagickMemory(sizeof(*lc));
  if (lc == NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");


  /*
   * Initialize members of the MngInfo structure.
   */
  (void) ResetMagickMemory(lc,0,sizeof(LoadContext));

  lc->image = image;
  lc->pages = 0;
  lc->context = ddjvu_context_create("ImageMagick djvu loader"); /* g_program_name */

  ddjvu_cache_set_size(lc->context, 1); /* right? */
  use_cache = 0;
  /* document: here we don't have a filename, but, for the sake of generality, a FILE* ! */
  url="http://maruska.dyndns.org/fake.djvu";
  lc->document = ddjvu_document_create(lc->context, url, use_cache); /* don't cache */
  ddjvu_document_set_user_data(lc->document, lc);


  /* now we wait the message-request for data: */
  message = ddjvu_message_wait(lc->context);

  if (message->m_any.tag != DDJVU_NEWSTREAM) {
          /* fixme: the djvu context, document! */

          ddjvu_document_release(lc->document);
          ddjvu_context_release(lc->context);

          RelinquishMagickMemory(lc);

          ThrowReaderException(ResourceLimitError,"Djvu initial message: unexpected type");
          return NULL;    /* error! */
  };

  lc->streamid = message->m_newstream.streamid;
  ddjvu_message_pop(lc->context);

  message = pump_data_until_message(lc,image);
  /* now process the messages: */


  if (message) do {
          process_message(message);
          ddjvu_message_pop(lc->context);
  } while ((message = ddjvu_message_peek(lc->context)));

  /* fixme: i hope we have not read any messages pertinent(?) related to the page itself!  */

  while (lc->pages == 0) {
          message = ddjvu_message_wait(lc->context);
          process_message(message);
          ddjvu_message_pop(lc->context);
  }

  images=NewImageList();
  i=0;
  if (image_info->number_scenes != 0)
    i=image_info->scene;
  for ( ; i < (long) lc->pages; i++)
  {
    image=ReadOneDJVUImage(lc,i,image_info,exception);
    if (image == (Image *) NULL)
      break;
    AppendImageToList(&images,CloneImageList(image,exception));
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
  }
  djvu_close_lc(lc);
  (void) CloseBlob(images);
  if (image != (Image *) NULL)
    image=DestroyImageList(image);

#if 0
  if ((image->page.width == 0) && (image->page.height == 0))
    {
      image->page.width = image->columns+image->page.x;
      image->page.height = image->rows+image->page.y;
    }
  if (image->columns == 0 || image->rows == 0)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadDJVUImage() with error.");
      ThrowReaderException(CorruptImageError,"CorruptImage");
    }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit ReadDJVUImage()");
#endif


  return(GetFirstImageInList(images));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D J V U I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDJVUImage() adds attributes for the DJVU image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDJVUImage method is:
%
%      unsigned long RegisterDJVUImage(void)
%
*/
ModuleExport unsigned long RegisterDJVUImage(void)
{
  char
    version[MaxTextExtent];

  MagickInfo
    *entry;

  static const char
    *DJVUNote =
    {
      "See http://www.djvuzone.org/ for details about the DJVU format.  The\n"
      "DJVU 1.2 specification is available there and at\n"
      "ftp://swrinde.nde.swri.edu/pub/djvu/documents/."
    };

  *version='\0';
#if defined(DJVU_LIBDJVU_VER_STRING)
  (void) ConcatenateMagickString(version,"libdjvu ",MaxTextExtent);
  (void) ConcatenateMagickString(version,DJVU_LIBDJVU_VER_STRING,MaxTextExtent);
#endif
  entry=SetMagickInfo("DJVU");
#if defined(MAGICKCORE_DJVU_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadDJVUImage;
#endif
  entry->raw=MagickTrue;
  entry->magick=(IsImageFormatHandler *) IsDJVU;
  entry->adjoin=MagickFalse;
  entry->thread_support=MagickTrue;
  entry->description=AcquireString("D�j� vu");
  entry->module=AcquireString("DJVU");
  if (*version != '\0')
    entry->version=AcquireString(version);
  entry->note=AcquireString(DJVUNote);
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D J V U I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDJVUImage() removes format registrations made by the
%  DJVU module from the list of supported formats.
%
%  The format of the UnregisterDJVUImage method is:
%
%      UnregisterDJVUImage(void)
%
*/
ModuleExport void UnregisterDJVUImage(void)
{
  (void) UnregisterMagickInfo("DJVU");
}

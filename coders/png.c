/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   N   N   GGGG                              %
%                            P   P  NN  N  G                                  %
%                            PPPP   N N N  G  GG                              %
%                            P      N  NN  G   G                              %
%                            P      N   N   GGG                               %
%                                                                             %
%                                                                             %
%              Read/Write Portable Network Graphics Image Format              %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                           Glenn Randers-Pehrson                             %
%                               November 1997                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/histogram.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/quantum-private.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/transform.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_PNG_DELEGATE)

/* Suppress libpng pedantic warnings that were added in
 * libpng-1.2.41 and libpng-1.4.0.  If you are working on
 * migration to libpng-2.0, remove these defines and then
 * fix any code that generates warnings.
 */
/* #define PNG_DEPRECATED   Use of this function is deprecated */
#define PNG_USE_RESULT  /* The result of this function must be checked */
#define PNG_NORETURN    /* This function does not return */
#define PNG_ALLOCATED   /* The result of the function is new memory */
#define PNG_DEPSTRUCT   /* Access to this struct member is deprecated */

#include "png.h"
#include "zlib.h"

/* ImageMagick differences */
#define first_scene scene

#if PNG_LIBPNG_VER < 10400
#    define trans_color  trans_values   /* Changed at libpng-1.4.0beta35 */
#    define trans_alpha  trans          /* Changed at libpng-1.4.0beta74 */
#else
   /* We could parse PNG_LIBPNG_VER_STRING here but it's too much bother..
    * Just don't use libpng-1.4.0beta32-34 or beta67-73
    */
#  ifndef  PNG_USER_CHUNK_CACHE_MAX     /* Added at libpng-1.4.0beta32 */
#    define trans_color  trans_values   /* Changed at libpng-1.4.0beta35 */
#  endif
#  ifndef  PNG_TRANSFORM_GRAY_TO_RGB    /* Added at libpng-1.4.0beta67 */
#    define trans_alpha  trans          /* Changed at libpng-1.4.0beta74 */
#  endif
#endif

#if PNG_LIBPNG_VER > 95
/*
  Optional declarations. Define or undefine them as you like.
*/
/* #define PNG_DEBUG -- turning this on breaks VisualC compiling */

/*
  Features under construction.  Define these to work on them.
*/
#undef MNG_OBJECT_BUFFERS
#undef MNG_BASI_SUPPORTED
#define MNG_COALESCE_LAYERS /* In 5.4.4, this interfered with MMAP'ed files. */
#define MNG_INSERT_LAYERS   /* Troublesome, but seem to work as of 5.4.4 */
#define PNG_BUILD_PALETTE   /* This works as of 5.4.3. */
#define PNG_SORT_PALETTE    /* This works as of 5.4.0. */
#if defined(MAGICKCORE_JPEG_DELEGATE)
#  define JNG_SUPPORTED /* Not finished as of 5.5.2.  See "To do" comments. */
#endif
#if !defined(RGBColorMatchExact)
#define IsPNGColorEqual(color,target) \
   (((color).red == (target).red) && \
    ((color).green == (target).green) && \
    ((color).blue == (target).blue))
#endif

/*
  Establish thread safety.
  setjmp/longjmp is claimed to be safe on these platforms:
  setjmp/longjmp is alleged to be unsafe on these platforms:
*/
#ifndef SETJMP_IS_THREAD_SAFE
#define PNG_SETJMP_NOT_THREAD_SAFE
#endif

#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
static SemaphoreInfo
  *png_semaphore = (SemaphoreInfo *) NULL;
#endif

/*
  This temporary until I set up malloc'ed object attributes array.
  Recompile with MNG_MAX_OBJECTS=65536L to avoid this limit but
  waste more memory.
*/
#define MNG_MAX_OBJECTS 256

/*
  If this not defined, spec is interpreted strictly.  If it is
  defined, an attempt will be made to recover from some errors,
  including
      o global PLTE too short
*/
#undef MNG_LOOSE

/*
  Don't try to define PNG_MNG_FEATURES_SUPPORTED here.  Make sure
  it's defined in libpng/pngconf.h, version 1.0.9 or later.  It won't work
  with earlier versions of libpng.  From libpng-1.0.3a to libpng-1.0.8,
  PNG_READ|WRITE_EMPTY_PLTE were used but those have been deprecated in
  libpng in favor of PNG_MNG_FEATURES_SUPPORTED, so we set them here.
  PNG_MNG_FEATURES_SUPPORTED is disabled by default in libpng-1.0.9 and
  will be enabled by default in libpng-1.2.0.
*/
#if (PNG_LIBPNG_VER == 10009)  /* work around libpng-1.0.9 bug */
#  undef PNG_READ_EMPTY_PLTE_SUPPORTED
#  undef PNG_WRITE_EMPTY_PLTE_SUPPORTED
#endif
#ifdef PNG_MNG_FEATURES_SUPPORTED
#  ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
#    define PNG_READ_EMPTY_PLTE_SUPPORTED
#  endif
#  ifndef PNG_WRITE_EMPTY_PLTE_SUPPORTED
#    define PNG_WRITE_EMPTY_PLTE_SUPPORTED
#  endif
#endif

/*
  Maximum valid unsigned long in PNG/MNG chunks is (2^31)-1
  This macro is only defined in libpng-1.0.3 and later.
  Previously it was PNG_MAX_UINT but that was deprecated in libpng-1.2.6
*/
#ifndef PNG_UINT_31_MAX
#define PNG_UINT_31_MAX (png_uint_32) 0x7fffffffL
#endif

/*
  Constant strings for known chunk types.  If you need to add a chunk,
  add a string holding the name here.   To make the code more
  portable, we use ASCII numbers like this, not characters.
*/

static png_byte FARDATA mng_MHDR[5]={ 77,  72,  68,  82, (png_byte) '\0'};
static png_byte FARDATA mng_BACK[5]={ 66,  65,  67,  75, (png_byte) '\0'};
static png_byte FARDATA mng_BASI[5]={ 66,  65,  83,  73, (png_byte) '\0'};
static png_byte FARDATA mng_CLIP[5]={ 67,  76,  73,  80, (png_byte) '\0'};
static png_byte FARDATA mng_CLON[5]={ 67,  76,  79,  78, (png_byte) '\0'};
static png_byte FARDATA mng_DEFI[5]={ 68,  69,  70,  73, (png_byte) '\0'};
static png_byte FARDATA mng_DHDR[5]={ 68,  72,  68,  82, (png_byte) '\0'};
static png_byte FARDATA mng_DISC[5]={ 68,  73,  83,  67, (png_byte) '\0'};
static png_byte FARDATA mng_ENDL[5]={ 69,  78,  68,  76, (png_byte) '\0'};
static png_byte FARDATA mng_FRAM[5]={ 70,  82,  65,  77, (png_byte) '\0'};
static png_byte FARDATA mng_IEND[5]={ 73,  69,  78,  68, (png_byte) '\0'};
static png_byte FARDATA mng_IHDR[5]={ 73,  72,  68,  82, (png_byte) '\0'};
static png_byte FARDATA mng_JHDR[5]={ 74,  72,  68,  82, (png_byte) '\0'};
static png_byte FARDATA mng_LOOP[5]={ 76,  79,  79,  80, (png_byte) '\0'};
static png_byte FARDATA mng_MAGN[5]={ 77,  65,  71,  78, (png_byte) '\0'};
static png_byte FARDATA mng_MEND[5]={ 77,  69,  78,  68, (png_byte) '\0'};
static png_byte FARDATA mng_MOVE[5]={ 77,  79,  86,  69, (png_byte) '\0'};
static png_byte FARDATA mng_PAST[5]={ 80,  65,  83,  84, (png_byte) '\0'};
static png_byte FARDATA mng_PLTE[5]={ 80,  76,  84,  69, (png_byte) '\0'};
static png_byte FARDATA mng_SAVE[5]={ 83,  65,  86,  69, (png_byte) '\0'};
static png_byte FARDATA mng_SEEK[5]={ 83,  69,  69,  75, (png_byte) '\0'};
static png_byte FARDATA mng_SHOW[5]={ 83,  72,  79,  87, (png_byte) '\0'};
static png_byte FARDATA mng_TERM[5]={ 84,  69,  82,  77, (png_byte) '\0'};
static png_byte FARDATA mng_bKGD[5]={ 98,  75,  71,  68, (png_byte) '\0'};
static png_byte FARDATA mng_cHRM[5]={ 99,  72,  82,  77, (png_byte) '\0'};
static png_byte FARDATA mng_gAMA[5]={103,  65,  77,  65, (png_byte) '\0'};
static png_byte FARDATA mng_iCCP[5]={105,  67,  67,  80, (png_byte) '\0'};
static png_byte FARDATA mng_nEED[5]={110,  69,  69,  68, (png_byte) '\0'};
static png_byte FARDATA mng_pHYg[5]={112,  72,  89, 103, (png_byte) '\0'};
static png_byte FARDATA mng_vpAg[5]={118, 112,  65, 103, (png_byte) '\0'};
static png_byte FARDATA mng_pHYs[5]={112,  72,  89, 115, (png_byte) '\0'};
static png_byte FARDATA mng_sBIT[5]={115,  66,  73,  84, (png_byte) '\0'};
static png_byte FARDATA mng_sRGB[5]={115,  82,  71,  66, (png_byte) '\0'};
static png_byte FARDATA mng_tRNS[5]={116,  82,  78,  83, (png_byte) '\0'};

#if defined(JNG_SUPPORTED)
static png_byte FARDATA mng_IDAT[5]={ 73,  68,  65,  84, (png_byte) '\0'};
static png_byte FARDATA mng_JDAT[5]={ 74,  68,  65,  84, (png_byte) '\0'};
static png_byte FARDATA mng_JDAA[5]={ 74,  68,  65,  65, (png_byte) '\0'};
static png_byte FARDATA mng_JdAA[5]={ 74, 100,  65,  65, (png_byte) '\0'};
static png_byte FARDATA mng_JSEP[5]={ 74,  83,  69,  80, (png_byte) '\0'};
static png_byte FARDATA mng_oFFs[5]={111,  70,  70, 115, (png_byte) '\0'};
#endif

/*
Other known chunks that are not yet supported by ImageMagick:
static png_byte FARDATA mng_hIST[5]={104,  73,  83,  84, (png_byte) '\0'};
static png_byte FARDATA mng_iCCP[5]={105,  67,  67,  80, (png_byte) '\0'};
static png_byte FARDATA mng_iTXt[5]={105,  84,  88, 116, (png_byte) '\0'};
static png_byte FARDATA mng_sPLT[5]={115,  80,  76,  84, (png_byte) '\0'};
static png_byte FARDATA mng_sTER[5]={115,  84,  69,  82, (png_byte) '\0'};
static png_byte FARDATA mng_tEXt[5]={116,  69,  88, 116, (png_byte) '\0'};
static png_byte FARDATA mng_tIME[5]={116,  73,  77,  69, (png_byte) '\0'};
static png_byte FARDATA mng_zTXt[5]={122,  84,  88, 116, (png_byte) '\0'};
*/

typedef struct _MngBox
{
  long
    left,
    right,
    top,
    bottom;
} MngBox;

typedef struct _MngPair
{
  volatile long
    a,
    b;
} MngPair;

#ifdef MNG_OBJECT_BUFFERS
typedef struct _MngBuffer
{

  unsigned long
    height,
    width;

  Image
    *image;

  png_color
    plte[256];

  int
    reference_count;

  unsigned char
    alpha_sample_depth,
    compression_method,
    color_type,
    concrete,
    filter_method,
    frozen,
    image_type,
    interlace_method,
    pixel_sample_depth,
    plte_length,
    sample_depth,
    viewable;
} MngBuffer;
#endif

typedef struct _MngInfo
{

#ifdef MNG_OBJECT_BUFFERS
  MngBuffer
    *ob[MNG_MAX_OBJECTS];
#endif

  Image *
    image;

  RectangleInfo
    page;

  int
    adjoin,
#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
    bytes_in_read_buffer,
    found_empty_plte,
#endif
    equal_backgrounds,
    equal_chrms,
    equal_gammas,
#if defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_MNG_FEATURES_SUPPORTED)
    equal_palettes,
#endif
    equal_physs,
    equal_srgbs,
    framing_mode,
    have_global_bkgd,
    have_global_chrm,
    have_global_gama,
    have_global_phys,
    have_global_sbit,
    have_global_srgb,
    have_saved_bkgd_index,
    have_write_global_chrm,
    have_write_global_gama,
    have_write_global_plte,
    have_write_global_srgb,
    need_fram,
    object_id,
    old_framing_mode,
    optimize,
    saved_bkgd_index;

  int
    new_number_colors;

  long
    image_found,
    loop_count[256],
    loop_iteration[256],
    scenes_found,
    x_off[MNG_MAX_OBJECTS],
    y_off[MNG_MAX_OBJECTS];

  MngBox
    clip,
    frame,
    image_box,
    object_clip[MNG_MAX_OBJECTS];

  unsigned char
    /* These flags could be combined into one byte */
    exists[MNG_MAX_OBJECTS],
    frozen[MNG_MAX_OBJECTS],
    loop_active[256],
    invisible[MNG_MAX_OBJECTS],
    viewable[MNG_MAX_OBJECTS];

  MagickOffsetType
    loop_jump[256];

  png_colorp
    global_plte;

  png_color_8
    global_sbit;

  png_byte
#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
    read_buffer[8],
#endif
    global_trns[256];

  float
    global_gamma;

  ChromaticityInfo
    global_chrm;

  RenderingIntent
    global_srgb_intent;

  unsigned long
    delay,
    global_plte_length,
    global_trns_length,
    global_x_pixels_per_unit,
    global_y_pixels_per_unit,
    mng_width,
    mng_height,
    ticks_per_second;

  unsigned int
    IsPalette,
    global_phys_unit_type,
    basi_warning,
    clon_warning,
    dhdr_warning,
    jhdr_warning,
    magn_warning,
    past_warning,
    phyg_warning,
    phys_warning,
    sbit_warning,
    show_warning,
    mng_type,
    write_mng,
    write_png_colortype,
    write_png_depth,
    write_png8,
    write_png24,
    write_png32;

#ifdef MNG_BASI_SUPPORTED
  unsigned long
    basi_width,
    basi_height;

  unsigned int
    basi_depth,
    basi_color_type,
    basi_compression_method,
    basi_filter_type,
    basi_interlace_method,
    basi_red,
    basi_green,
    basi_blue,
    basi_alpha,
    basi_viewable;
#endif

  png_uint_16
    magn_first,
    magn_last,
    magn_mb,
    magn_ml,
    magn_mr,
    magn_mt,
    magn_mx,
    magn_my,
    magn_methx,
    magn_methy;

  PixelPacket
    mng_global_bkgd;

} MngInfo;
#endif /* VER */

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePNGImage(const ImageInfo *,Image *);
static MagickBooleanType
  WriteMNGImage(const ImageInfo *,Image *);
#if defined(JNG_SUPPORTED)
static MagickBooleanType
  WriteJNGImage(const ImageInfo *,Image *);
#endif

static inline long MagickMax(const long x,const long y)
{
  if (x > y)
    return(x);
  return(y);
}
static inline long MagickMin(const long x,const long y)
{
  if (x < y)
    return(x);
  return(y);
}

#if PNG_LIBPNG_VER > 95
#if defined(PNG_SORT_PALETTE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p r e s s C o l o r m a p T r a n s F i r s t                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompressColormapTransFirst compresses an image colormap removing
%  any duplicate and unused color entries and putting the transparent colors
%  first.  Returns MagickTrue on success, MagickFalse on error.
%
%  The format of the CompressColormapTransFirst method is:
%
%      unsigned int CompressColormapTransFirst(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%      This function updates image->colors and image->colormap.
%
*/
static MagickBooleanType CompressColormapTransFirst(Image *image)
{
  int
    remap_needed,
    k;

  long
    j,
    new_number_colors,
    number_colors,
    y;

  PixelPacket
    *colormap;

  register const IndexPacket
    *indices;

  register const PixelPacket
    *p;

  IndexPacket
    top_used;

  register long
    i,
    x;

  IndexPacket
    *map,
    *opacity;

  unsigned char
    *marker,
    have_transparency;

  /*
    Determine if colormap can be compressed.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "    CompressColorMapTransFirst %s (%ld colors)",
           image->filename,image->colors);
  if (image->storage_class != PseudoClass || image->colors > 256 ||
      image->colors < 2)
    {
      if (image->debug != MagickFalse)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Could not compress colormap");
          if (image->colors > 256 || image->colors == 0)
            return(MagickFalse);
          else
            return(MagickTrue);
        }
    }
  marker=(unsigned char *) AcquireQuantumMemory(image->colors,sizeof(*marker));
  if (marker == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  opacity=(IndexPacket *) AcquireQuantumMemory(image->colors,sizeof(*opacity));
  if (opacity == (IndexPacket *) NULL)
    {
      marker=(unsigned char *) RelinquishMagickMemory(marker);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Mark colors that are present.
  */
  number_colors=(long) image->colors;
  for (i=0; i < number_colors; i++)
  {
    marker[i]=MagickFalse;
    opacity[i]=OpaqueOpacity;
  }
  top_used=0;
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indices=GetVirtualIndexQueue(image);
    if (image->matte != MagickFalse)
      for (x=0; x < (long) image->columns; x++)
      {
        marker[(int) indices[x]]=MagickTrue;
        opacity[(int) indices[x]]=GetOpacityPixelComponent(p);
        if (indices[x] > top_used)
           top_used=indices[x];
        p++;
      }
    else
      for (x=0; x < (long) image->columns; x++)
      {
        marker[(int) indices[x]]=MagickTrue;
        if (indices[x] > top_used)
           top_used=indices[x];
      }
  }

  if (image->matte != MagickFalse)
  {
    /*
      Mark background color, topmost occurrence if more than one.
    */
    for (i=number_colors-1; i; i--)
    {
      if (IsColorEqual(image->colormap+i,&image->background_color))
        {
          marker[i]=MagickTrue;
          break;
        }
    }
  }
  /*
    Unmark duplicates.
  */
  for (i=0; i < number_colors-1; i++)
    if (marker[i])
      {
        for (j=i+1; j < number_colors; j++)
          if ((opacity[i] == opacity[j]) &&
              (IsColorEqual(image->colormap+i,image->colormap+j)))
            marker[j]=MagickFalse;
       }
  /*
    Count colors that still remain.
  */
  have_transparency=MagickFalse;
  new_number_colors=0;
  for (i=0; i < number_colors; i++)
    if (marker[i])
      {
        new_number_colors++;
        if (opacity[i] != OpaqueOpacity)
          have_transparency=MagickTrue;
      }
  if ((!have_transparency || (marker[0] &&
      (opacity[0] == (Quantum) TransparentOpacity)))
      && (new_number_colors == number_colors))
    {
      /*
        No duplicate or unused entries, and transparency-swap not needed.
      */
      marker=(unsigned char *) RelinquishMagickMemory(marker);
      opacity=(IndexPacket *) RelinquishMagickMemory(opacity);
      return(MagickTrue);
    }

  remap_needed=MagickFalse;
  if ((long) top_used >= new_number_colors)
     remap_needed=MagickTrue;

  /*
    Compress colormap.
  */

  colormap=(PixelPacket *) AcquireQuantumMemory(image->colors,
    sizeof(*colormap));
  if (colormap == (PixelPacket *) NULL)
    {
      marker=(unsigned char *) RelinquishMagickMemory(marker);
      opacity=(IndexPacket *) RelinquishMagickMemory(opacity);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Eliminate unused colormap entries.
  */
  map=(IndexPacket *) AcquireQuantumMemory((size_t) number_colors,
    sizeof(*map));
  if (map == (IndexPacket *) NULL)
    {
      marker=(unsigned char *) RelinquishMagickMemory(marker);
      opacity=(IndexPacket *) RelinquishMagickMemory(opacity);
      colormap=(PixelPacket *) RelinquishMagickMemory(colormap);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  k=0;
  for (i=0; i < number_colors; i++)
  {
    map[i]=(IndexPacket) k;
    if (marker[i])
      {
        for (j=i+1; j < number_colors; j++)
        {
          if ((opacity[i] == opacity[j]) &&
              (IsColorEqual(image->colormap+i,image->colormap+j)))
            {
               map[j]=(IndexPacket) k;
               marker[j]=MagickFalse;
            }
        }
        k++;
      }
  }
  j=0;
  for (i=0; i < number_colors; i++)
  {
    if (marker[i])
      {
        colormap[j]=image->colormap[i];
        j++;
      }
  }
  if (have_transparency && (opacity[0] != (Quantum) TransparentOpacity))
    {
      /*
        Move the first transparent color to palette entry 0.
      */
      for (i=1; i < number_colors; i++)
      {
        if (marker[i] && opacity[i] == (Quantum) TransparentOpacity)
          {
            PixelPacket
              temp_colormap;

            temp_colormap=colormap[0];
            colormap[0]=colormap[(int) map[i]];
            colormap[(long) map[i]]=temp_colormap;
            for (j=0; j < number_colors; j++)
            {
              if (map[j] == 0)
                map[j]=map[i];
              else if (map[j] == map[i])
                map[j]=0;
            }
            remap_needed=MagickTrue;
            break;
          }
      }
   }

  opacity=(IndexPacket *) RelinquishMagickMemory(opacity);
  marker=(unsigned char *) RelinquishMagickMemory(marker);

  if (remap_needed)
    {
      ExceptionInfo
        *exception;

      register IndexPacket
        *pixels;

      register PixelPacket
        *q;

      /*
        Remap pixels.
      */
      exception=(&image->exception);
      for (y=0; y < (long) image->rows; y++)
      {
        q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        pixels=GetAuthenticIndexQueue(image);
        for (x=0; x < (long) image->columns; x++)
        {
          j=(int) pixels[x];
          pixels[x]=map[j];
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      for (i=0; i < new_number_colors; i++)
        image->colormap[i]=colormap[i];
    }
  colormap=(PixelPacket *) RelinquishMagickMemory(colormap);
  image->colors=(unsigned long) new_number_colors;
  map=(IndexPacket *) RelinquishMagickMemory(map);
  return(MagickTrue);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m a g e I s G r a y                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   Like IsGrayImage except does not change DirectClass to PseudoClass        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
static MagickBooleanType ImageIsGray(Image *image)
{
  register const PixelPacket
    *p;

  register long
    i,
    x,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  if (image->storage_class == PseudoClass)
    {
      for (i=0; i < (long) image->colors; i++)
        if (IsGray(image->colormap+i) == MagickFalse)
          return(MagickFalse);
      return(MagickTrue);
    }
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      return(MagickFalse);
    for (x=(long) image->columns-1; x >= 0; x--)
    {
       if (IsGray(p) == MagickFalse)
          return(MagickFalse);
       p++;
    }
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m a g e I s M o n o c h r o m e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   Like IsMonochromeImage except does not change DirectClass to PseudoClass  %
%   and is more accurate.                                                     %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
static MagickBooleanType ImageIsMonochrome(Image *image)
{
  register const PixelPacket
    *p;

  register long
    i,
    x,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      for (i=0; i < (long) image->colors; i++)
      {
        if ((IsGray(image->colormap+i) == MagickFalse) ||
            ((image->colormap[i].red != 0) &&
             (image->colormap[i].red != (Quantum) QuantumRange)))
          return(MagickFalse);
      }
      return(MagickTrue);
    }
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      return(MagickFalse);
    for (x=(long) image->columns-1; x >= 0; x--)
    {
      if ((p->red != 0) && (p->red != (Quantum) QuantumRange))
        return(MagickFalse);
      if (IsGray(p) == MagickFalse)
        return(MagickFalse);
      if ((p->opacity != OpaqueOpacity) &&
          (p->opacity != (Quantum) TransparentOpacity))
        return(MagickFalse);
      p++;
    }
  }
  return(MagickTrue);
}
#endif /* PNG_LIBPNG_VER > 95 */
#endif /* MAGICKCORE_PNG_DELEGATE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMNG() returns MagickTrue if the image format type, identified by the
%  magick string, is MNG.
%
%  The format of the IsMNG method is:
%
%      MagickBooleanType IsMNG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsMNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick,"\212MNG\r\n\032\n",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJNG() returns MagickTrue if the image format type, identified by the
%  magick string, is JNG.
%
%  The format of the IsJNG method is:
%
%      MagickBooleanType IsJNG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsJNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick,"\213JNG\r\n\032\n",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPNG() returns MagickTrue if the image format type, identified by the
%  magick string, is PNG.
%
%  The format of the IsPNG method is:
%
%      MagickBooleanType IsPNG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick,"\211PNG\r\n\032\n",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_PNG_DELEGATE)
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if (PNG_LIBPNG_VER > 95)
static size_t WriteBlobMSBULong(Image *image,const unsigned long value)
{
  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  buffer[0]=(unsigned char) (value >> 24);
  buffer[1]=(unsigned char) (value >> 16);
  buffer[2]=(unsigned char) (value >> 8);
  buffer[3]=(unsigned char) value;
  return((size_t) WriteBlob(image,4,buffer));
}

static void PNGLong(png_bytep p,png_uint_32 value)
{
  *p++=(png_byte) ((value >> 24) & 0xff);
  *p++=(png_byte) ((value >> 16) & 0xff);
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGsLong(png_bytep p,png_int_32 value)
{
  *p++=(png_byte) ((value >> 24) & 0xff);
  *p++=(png_byte) ((value >> 16) & 0xff);
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGShort(png_bytep p,png_uint_16 value)
{
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGType(png_bytep p,png_bytep type)
{
  (void) CopyMagickMemory(p,type,4*sizeof(png_byte));
}

static void LogPNGChunk(int logging, png_bytep type, size_t length)
{
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Writing %c%c%c%c chunk, length: %lu",
      type[0],type[1],type[2],type[3],(unsigned long) length);
}
#endif /* PNG_LIBPNG_VER > 95 */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#if PNG_LIBPNG_VER > 95
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P N G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPNGImage() reads a Portable Network Graphics (PNG) or
%  Multiple-image Network Graphics (MNG) image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image or set of images.
%
%  MNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the ReadPNGImage method is:
%
%      Image *ReadPNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
%  To do, more or less in chronological order (as of version 5.5.2,
%   November 26, 2002 -- glennrp -- see also "To do" under WriteMNGImage):
%
%    Get 16-bit cheap transparency working.
%
%    (At this point, PNG decoding is supposed to be in full MNG-LC compliance)
%
%    Preserve all unknown and not-yet-handled known chunks found in input
%    PNG file and copy them into output PNG files according to the PNG
%    copying rules.
%
%    (At this point, PNG encoding should be in full MNG compliance)
%
%    Provide options for choice of background to use when the MNG BACK
%    chunk is not present or is not mandatory (i.e., leave transparent,
%    user specified, MNG BACK, PNG bKGD)
%
%    Implement LOOP/ENDL [done, but could do discretionary loops more
%    efficiently by linking in the duplicate frames.].
%
%    Decode and act on the MHDR simplicity profile (offer option to reject
%    files or attempt to process them anyway when the profile isn't LC or VLC).
%
%    Upgrade to full MNG without Delta-PNG.
%
%        o  BACK [done a while ago except for background image ID]
%        o  MOVE [done 15 May 1999]
%        o  CLIP [done 15 May 1999]
%        o  DISC [done 19 May 1999]
%        o  SAVE [partially done 19 May 1999 (marks objects frozen)]
%        o  SEEK [partially done 19 May 1999 (discard function only)]
%        o  SHOW
%        o  PAST
%        o  BASI
%        o  MNG-level tEXt/iTXt/zTXt
%        o  pHYg
%        o  pHYs
%        o  sBIT
%        o  bKGD
%        o  iTXt (wait for libpng implementation).
%
%    Use the scene signature to discover when an identical scene is
%    being reused, and just point to the original image->exception instead
%    of storing another set of pixels.  This not specific to MNG
%    but could be applied generally.
%
%    Upgrade to full MNG with Delta-PNG.
%
%    JNG tEXt/iTXt/zTXt
%
%    We will not attempt to read files containing the CgBI chunk.
%    They are really Xcode files meant for display on the iPhone.
%    These are not valid PNG files and it is impossible to recover
%    the orginal PNG from files that have been converted to Xcode-PNG,
%    since irretrievable loss of color data has occurred due to the
%    use of premultiplied alpha.
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  This the function that does the actual reading of data.  It is
  the same as the one supplied in libpng, except that it receives the
  datastream from the ReadBlob() function instead of standard input.
*/
static void png_get_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  if (length)
    {
      png_size_t
        check;

      check=(png_size_t) ReadBlob(image,(size_t) length,data);
      if (check != length)
        {
          char
            msg[MaxTextExtent];

          (void) FormatMagickString(msg,MaxTextExtent,
            "Expected %lu bytes; found %lu bytes",(unsigned long) length,
            (unsigned long) check);
          png_warning(png_ptr,msg);
          png_error(png_ptr,"Read Exception");
        }
    }
}

#if !defined(PNG_READ_EMPTY_PLTE_SUPPORTED) && \
    !defined(PNG_MNG_FEATURES_SUPPORTED)
/* We use mng_get_data() instead of png_get_data() if we have a libpng
 * older than libpng-1.0.3a, which was the first to allow the empty
 * PLTE, or a newer libpng in which PNG_MNG_FEATURES_SUPPORTED was
 * ifdef'ed out.  Earlier versions would crash if the bKGD chunk was
 * encountered after an empty PLTE, so we have to look ahead for bKGD
 * chunks and remove them from the datastream that is passed to libpng,
 * and store their contents for later use.
 */
static void mng_get_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  MngInfo
    *mng_info;

  Image
    *image;

  png_size_t
    check;

  register long
    i;

  i=0;
  mng_info=(MngInfo *) png_get_io_ptr(png_ptr);
  image=(Image *) mng_info->image;
  while (mng_info->bytes_in_read_buffer && length)
  {
    data[i]=mng_info->read_buffer[i];
    mng_info->bytes_in_read_buffer--;
    length--;
    i++;
  }
  if (length)
    {
      check=(png_size_t) ReadBlob(image,(size_t) length,(char *) data);
      if (check != length)
        png_error(png_ptr,"Read Exception");
      if (length == 4)
        {
          if ((data[0] == 0) && (data[1] == 0) && (data[2] == 0) &&
              (data[3] == 0))
            {
              check=(png_size_t) ReadBlob(image,(size_t) length,
                (char *) mng_info->read_buffer);
              mng_info->read_buffer[4]=0;
              mng_info->bytes_in_read_buffer=4;
              if (memcmp(mng_info->read_buffer,mng_PLTE,4) == 0)
                mng_info->found_empty_plte=MagickTrue;
              if (memcmp(mng_info->read_buffer,mng_IEND,4) == 0)
                {
                  mng_info->found_empty_plte=MagickFalse;
                  mng_info->have_saved_bkgd_index=MagickFalse;
                }
            }
          if ((data[0] == 0) && (data[1] == 0) && (data[2] == 0) &&
              (data[3] == 1))
            {
              check=(png_size_t) ReadBlob(image,(size_t) length,
                (char *) mng_info->read_buffer);
              mng_info->read_buffer[4]=0;
              mng_info->bytes_in_read_buffer=4;
              if (memcmp(mng_info->read_buffer,mng_bKGD,4) == 0)
                if (mng_info->found_empty_plte)
                  {
                    /*
                      Skip the bKGD data byte and CRC.
                    */
                    check=(png_size_t)
                      ReadBlob(image,5,(char *) mng_info->read_buffer);
                    check=(png_size_t) ReadBlob(image,(size_t) length,
                      (char *) mng_info->read_buffer);
                    mng_info->saved_bkgd_index=mng_info->read_buffer[0];
                    mng_info->have_saved_bkgd_index=MagickTrue;
                    mng_info->bytes_in_read_buffer=0;
                  }
            }
        }
    }
}
#endif

static void png_put_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  if (length)
    {
      png_size_t
        check;

      check=(png_size_t) WriteBlob(image,(unsigned long) length,data);
      if (check != length)
        png_error(png_ptr,"WriteBlob Failed");
    }
}

static void png_flush_data(png_structp png_ptr)
{
  (void) png_ptr;
}

#ifdef PNG_WRITE_EMPTY_PLTE_SUPPORTED
static int PalettesAreEqual(Image *a,Image *b)
{
  long
    i;

  if ((a == (Image *) NULL) || (b == (Image *) NULL))
    return((int) MagickFalse);
  if (a->storage_class != PseudoClass || b->storage_class != PseudoClass)
    return((int) MagickFalse);
  if (a->colors != b->colors)
    return((int) MagickFalse);
  for (i=0; i < (long) a->colors; i++)
  {
    if ((a->colormap[i].red != b->colormap[i].red) ||
        (a->colormap[i].green != b->colormap[i].green) ||
        (a->colormap[i].blue != b->colormap[i].blue))
      return((int) MagickFalse);
  }
  return((int) MagickTrue);
}
#endif

static void MngInfoDiscardObject(MngInfo *mng_info,int i)
{
  if (i && (i < MNG_MAX_OBJECTS) && (mng_info != (MngInfo *) NULL) &&
      mng_info->exists[i] && !mng_info->frozen[i])
    {
#ifdef MNG_OBJECT_BUFFERS
      if (mng_info->ob[i] != (MngBuffer *) NULL)
        {
          if (mng_info->ob[i]->reference_count > 0)
            mng_info->ob[i]->reference_count--;
          if (mng_info->ob[i]->reference_count == 0)
            {
              if (mng_info->ob[i]->image != (Image *) NULL)
                mng_info->ob[i]->image=DestroyImage(mng_info->ob[i]->image);
              mng_info->ob[i]=DestroyString(mng_info->ob[i]);
            }
        }
      mng_info->ob[i]=(MngBuffer *) NULL;
#endif
      mng_info->exists[i]=MagickFalse;
      mng_info->invisible[i]=MagickFalse;
      mng_info->viewable[i]=MagickFalse;
      mng_info->frozen[i]=MagickFalse;
      mng_info->x_off[i]=0;
      mng_info->y_off[i]=0;
      mng_info->object_clip[i].left=0;
      mng_info->object_clip[i].right=(long) PNG_UINT_31_MAX;
      mng_info->object_clip[i].top=0;
      mng_info->object_clip[i].bottom=(long) PNG_UINT_31_MAX;
    }
}

static void MngInfoFreeStruct(MngInfo *mng_info,int *have_mng_structure)
{
  if (*have_mng_structure && (mng_info != (MngInfo *) NULL))
    {
      register long
        i;

      for (i=1; i < MNG_MAX_OBJECTS; i++)
        MngInfoDiscardObject(mng_info,i);
      if (mng_info->global_plte != (png_colorp) NULL)
        mng_info->global_plte=(png_colorp)
          RelinquishMagickMemory(mng_info->global_plte);
      mng_info=(MngInfo *) RelinquishMagickMemory(mng_info);
      *have_mng_structure=MagickFalse;
    }
}

static MngBox mng_minimum_box(MngBox box1,MngBox box2)
{
  MngBox
    box;

  box=box1;
  if (box.left < box2.left)
    box.left=box2.left;
  if (box.top < box2.top)
    box.top=box2.top;
  if (box.right > box2.right)
    box.right=box2.right;
  if (box.bottom > box2.bottom)
    box.bottom=box2.bottom;
  return box;
}

static MngBox mng_read_box(MngBox previous_box,char delta_type,unsigned char *p)
{
   MngBox
      box;

  /*
    Read clipping boundaries from DEFI, CLIP, FRAM, or PAST chunk.
  */
  box.left=(long) ((p[0]  << 24) | (p[1]  << 16) | (p[2]  << 8) | p[3]);
  box.right=(long) ((p[4]  << 24) | (p[5]  << 16) | (p[6]  << 8) | p[7]);
  box.top=(long) ((p[8]  << 24) | (p[9]  << 16) | (p[10] << 8) | p[11]);
  box.bottom=(long) ((p[12] << 24) | (p[13] << 16) | (p[14] << 8) | p[15]);
  if (delta_type != 0)
    {
      box.left+=previous_box.left;
      box.right+=previous_box.right;
      box.top+=previous_box.top;
      box.bottom+=previous_box.bottom;
    }
  return(box);
}

static MngPair mng_read_pair(MngPair previous_pair,int delta_type,
  unsigned char *p)
{
  MngPair
    pair;
  /*
    Read two longs from CLON, MOVE or PAST chunk
  */
  pair.a=(long) ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
  pair.b=(long) ((p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7]);
  if (delta_type != 0)
    {
      pair.a+=previous_pair.a;
      pair.b+=previous_pair.b;
    }
  return(pair);
}

static long mng_get_long(unsigned char *p)
{
  return((long) ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]));
}

static void PNGErrorHandler(png_struct *ping,png_const_charp message)
{
  Image
    *image;

  image=(Image *) png_get_error_ptr(ping);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  libpng-%s error: %s", PNG_LIBPNG_VER_STRING,message);
  (void) ThrowMagickException(&image->exception,GetMagickModule(),CoderError,
    message,"`%s'",image->filename);
  longjmp(ping->jmpbuf,1);
}

static void PNGWarningHandler(png_struct *ping,png_const_charp message)
{
  Image
    *image;

  if (LocaleCompare(message, "Missing PLTE before tRNS") == 0)
    png_error(ping, message);
  image=(Image *) png_get_error_ptr(ping);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  libpng-%s warning: %s", PNG_LIBPNG_VER_STRING,
      message);
  (void) ThrowMagickException(&image->exception,GetMagickModule(),CoderWarning,
    message,"`%s'",image->filename);
}

#ifdef PNG_USER_MEM_SUPPORTED
static png_voidp png_IM_malloc(png_structp png_ptr,png_uint_32 size)
{
#if (PNG_LIBPNG_VER < 10011)
  png_voidp
    ret;

  png_ptr=png_ptr;
  ret=((png_voidp) AcquireMagickMemory((size_t) size));
  if (ret == NULL)
    png_error("Insufficient memory.");
  return(ret);
#else
  png_ptr=png_ptr;
  return((png_voidp) AcquireMagickMemory((size_t) size));
#endif
}

/*
  Free a pointer.  It is removed from the list at the same time.
*/
static png_free_ptr png_IM_free(png_structp png_ptr,png_voidp ptr)
{
  png_ptr=png_ptr;
  ptr=RelinquishMagickMemory(ptr);
  return((png_free_ptr) NULL);
}
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static int
png_read_raw_profile(Image *image, const ImageInfo *image_info,
   png_textp text,int ii)
{
  register long
    i;

  register unsigned char
    *dp;

  register png_charp
    sp;

  png_uint_32
    length,
    nibbles;

  StringInfo
    *profile;

  unsigned char
    unhex[103]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,1, 2,3,4,5,6,7,8,9,0,0,
                 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,10,11,12,
                 13,14,15};

  sp=text[ii].text+1;
  /* look for newline */
  while (*sp != '\n')
     sp++;
  /* look for length */
  while (*sp == '\0' || *sp == ' ' || *sp == '\n')
     sp++;
  length=(png_uint_32) StringToLong(sp);
  while (*sp != ' ' && *sp != '\n')
     sp++;
  /* allocate space */
  if (length == 0)
  {
    (void) ThrowMagickException(&image->exception,GetMagickModule(),
      CoderWarning,"UnableToCopyProfile","`%s'","invalid profile length");
    return(MagickFalse);
  }
  profile=AcquireStringInfo(length);
  if (profile == (StringInfo *) NULL)
  {
    (void) ThrowMagickException(&image->exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",
      "unable to copy profile");
    return(MagickFalse);
  }
  /* copy profile, skipping white space and column 1 "=" signs */
  dp=GetStringInfoDatum(profile);
  nibbles=length*2;
  for (i=0; i < (long) nibbles; i++)
  {
    while (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f')
    {
      if (*sp == '\0')
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            CoderWarning,"UnableToCopyProfile","`%s'","ran out of data");
          profile=DestroyStringInfo(profile);
          return(MagickFalse);
        }
      sp++;
    }
    if (i%2 == 0)
      *dp=(unsigned char) (16*unhex[(int) *sp++]);
    else
      (*dp++)+=unhex[(int) *sp++];
  }
  /*
    We have already read "Raw profile type.
  */
  (void) SetImageProfile(image,&text[ii].key[17],profile);
  profile=DestroyStringInfo(profile);
  if (image_info->verbose)
    (void) printf(" Found a generic profile, type %s\n",&text[ii].key[17]);
  return MagickTrue;
}

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
static int read_vpag_chunk_callback(png_struct *ping, png_unknown_chunkp chunk)
{
  Image
    *image;


  /* The unknown chunk structure contains the chunk data:
     png_byte name[5];
     png_byte *data;
     png_size_t size;

     Note that libpng has already taken care of the CRC handling.
  */


  if (chunk->name[0] != 118 || chunk->name[1] != 112 ||
      chunk->name[2] != 65 ||chunk-> name[3] != 103)
    return(0); /* Did not recognize */

  /* recognized vpAg */

  if (chunk->size != 9)
    return(-1); /* Error return */

  if (chunk->data[8] != 0)
    return(0);  /* ImageMagick requires pixel units */

  image=(Image *) png_get_user_chunk_ptr(ping);

  image->page.width=(unsigned long) ((chunk->data[0] << 24) |
     (chunk->data[1] << 16) | (chunk->data[2] << 8) | chunk->data[3]);
  image->page.height=(unsigned long) ((chunk->data[4] << 24) |
     (chunk->data[5] << 16) | (chunk->data[6] << 8) | chunk->data[7]);

  /* Return one of the following: */
     /* return(-n);  chunk had an error */
     /* return(0);  did not recognize */
     /* return(n);  success */

  return(1);

}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d O n e P N G I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadOnePNGImage() reads a Portable Network Graphics (PNG) image file
%  (minus the 8-byte signature)  and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ReadOnePNGImage method is:
%
%      Image *ReadOnePNGImage(MngInfo *mng_info, const ImageInfo *image_info,
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
static Image *ReadOnePNGImage(MngInfo *mng_info,
    const ImageInfo *image_info, ExceptionInfo *exception)
{
  /* Read one PNG image */

  Image
    *image;

  int
    logging,
    num_text,
    num_passes,
    pass;

  MagickBooleanType
    status;

  PixelPacket
    transparent_color;

  png_info
    *end_info,
    *ping_info;

  png_struct
    *ping;

  png_textp
    text;

  QuantumInfo
    *quantum_info;

  unsigned char
    *png_pixels;

  long
    y;

  register unsigned char
    *p;

  register IndexPacket
    *indices;

  register long
    i,
    x;

  register PixelPacket
    *q;

  size_t
    length;

  unsigned long
    row_offset;

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
  png_byte unused_chunks[]=
  {
    104,  73,  83,  84, (png_byte) '\0',   /* hIST */
    105,  84,  88, 116, (png_byte) '\0',   /* iTXt */
    112,  67,  65,  76, (png_byte) '\0',   /* pCAL */
    115,  67,  65,  76, (png_byte) '\0',   /* sCAL */
    115,  80,  76,  84, (png_byte) '\0',   /* sPLT */
    116,  73,  77,  69, (png_byte) '\0',   /* tIME */
  };
#endif

  logging=LogMagickEvent(CoderEvent,GetMagickModule(),
    "  enter ReadOnePNGImage()");

#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
  LockSemaphoreInfo(png_semaphore);
#endif

#if (PNG_LIBPNG_VER < 10007)
  if (image_info->verbose)
    printf("Your PNG library (libpng-%s) is rather old.\n",
       PNG_LIBPNG_VER_STRING);
#endif

#if (PNG_LIBPNG_VER >= 10400)
#  ifndef  PNG_TRANSFORM_GRAY_TO_RGB    /* Added at libpng-1.4.0beta67 */
  if (image_info->verbose)
    {
      printf("Your PNG library (libpng-%s) is an old beta version.\n",
           PNG_LIBPNG_VER_STRING);
      printf("Please update it.\n");
    }
#  endif
#endif


  quantum_info = (QuantumInfo *) NULL;
  image=mng_info->image;

  /*
    Allocate the PNG structures
  */
#ifdef PNG_USER_MEM_SUPPORTED
 ping=png_create_read_struct_2(PNG_LIBPNG_VER_STRING, image,
   PNGErrorHandler,PNGWarningHandler, NULL,
   (png_malloc_ptr) png_IM_malloc,(png_free_ptr) png_IM_free);
#else
  ping=png_create_read_struct(PNG_LIBPNG_VER_STRING,image,
    PNGErrorHandler,PNGWarningHandler);
#endif
  if (ping == (png_struct *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  ping_info=png_create_info_struct(ping);
  if (ping_info == (png_info *) NULL)
    {
      png_destroy_read_struct(&ping,(png_info **) NULL,(png_info **) NULL);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  end_info=png_create_info_struct(ping);
  if (end_info == (png_info *) NULL)
    {
      png_destroy_read_struct(&ping,&ping_info,(png_info **) NULL);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  png_pixels=(unsigned char *) NULL;
  if (setjmp(ping->jmpbuf))
    {
      /*
        PNG image is corrupt.
      */
      png_destroy_read_struct(&ping,&ping_info,&end_info);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
      UnlockSemaphoreInfo(png_semaphore);
#endif
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  exit ReadOnePNGImage() with error.");
      if (image != (Image *) NULL)
        {
          InheritException(exception,&image->exception);
          image->columns=0;
        }
      return(GetFirstImageInList(image));
    }
  /*
    Prepare PNG for reading.
  */
  mng_info->image_found++;
  png_set_sig_bytes(ping,8);
  if (LocaleCompare(image_info->magick,"MNG") == 0)
    {
#if defined(PNG_MNG_FEATURES_SUPPORTED)
      (void) png_permit_mng_features(ping,PNG_ALL_MNG_FEATURES);
      png_set_read_fn(ping,image,png_get_data);
#else
#if defined(PNG_READ_EMPTY_PLTE_SUPPORTED)
      png_permit_empty_plte(ping,MagickTrue);
      png_set_read_fn(ping,image,png_get_data);
#else
      mng_info->image=image;
      mng_info->bytes_in_read_buffer=0;
      mng_info->found_empty_plte=MagickFalse;
      mng_info->have_saved_bkgd_index=MagickFalse;
      png_set_read_fn(ping,mng_info,mng_get_data);
#endif
#endif
    }
  else
    png_set_read_fn(ping,image,png_get_data);

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
  /* Ignore unused chunks and all unknown chunks except for vpAg */
  png_set_keep_unknown_chunks(ping, 1, NULL, 0);
  png_set_keep_unknown_chunks(ping, 2, mng_vpAg, 1);
  png_set_keep_unknown_chunks(ping, 1, unused_chunks,
     (int)sizeof(unused_chunks)/5);
  /* Callback for other unknown chunks */
  png_set_read_user_chunk_fn(ping, image, read_vpag_chunk_callback);
#endif

#if (PNG_LIBPNG_VER < 10400)
#  if defined(PNG_USE_PNGGCCRD) && defined(PNG_ASSEMBLER_CODE_SUPPORTED) && \
   (PNG_LIBPNG_VER >= 10200) && (PNG_LIBPNG_VER < 10220) && defined(__i386__)
  /* Disable thread-unsafe features of pnggccrd */
  if (png_access_version_number() >= 10200)
  {
    png_uint_32 mmx_disable_mask=0;
    png_uint_32 asm_flags;

    mmx_disable_mask |= ( PNG_ASM_FLAG_MMX_READ_COMBINE_ROW  \
                        | PNG_ASM_FLAG_MMX_READ_FILTER_SUB   \
                        | PNG_ASM_FLAG_MMX_READ_FILTER_AVG   \
                        | PNG_ASM_FLAG_MMX_READ_FILTER_PAETH );
    asm_flags=png_get_asm_flags(ping);
    png_set_asm_flags(ping, asm_flags & ~mmx_disable_mask);
  }
#  endif
#endif

  png_read_info(ping,ping_info);
  if (ping_info->bit_depth < 8)
    {
      if (((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE))
        png_set_packing(ping);
    }
  image->depth=ping_info->bit_depth;
  image->depth=GetImageQuantumDepth(image,MagickFalse);
  image->interlace=ping_info->interlace_type != 0 ? PNGInterlace : NoInterlace;
  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG width: %lu, height: %lu",
        ping_info->width, ping_info->height);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG color_type: %d, bit_depth: %d",
        ping_info->color_type, ping_info->bit_depth);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG compression_method: %d",
        ping_info->compression_type);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG interlace_method: %d, filter_method: %d",
        ping_info->interlace_type,ping_info->filter_type);
    }

#if (PNG_LIBPNG_VER > 10008) && defined(PNG_READ_iCCP_SUPPORTED)
  if (ping_info->valid & PNG_INFO_iCCP)
    {
      int
        compression;

      png_charp
        info,
        name;

      png_uint_32
        profile_length;

      (void) png_get_iCCP(ping,ping_info,&name,(int *) &compression,&info,
        &profile_length);
      if (profile_length != 0)
        {
          StringInfo
            *profile;

          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Reading PNG iCCP chunk.");
          profile=AcquireStringInfo(profile_length);
          SetStringInfoDatum(profile,(const unsigned char *) info);
          (void) SetImageProfile(image,"icc",profile);
          profile=DestroyStringInfo(profile);
      }
    }
#endif
#if defined(PNG_READ_sRGB_SUPPORTED)
  {
    int
      intent;

    if (mng_info->have_global_srgb)
      image->rendering_intent=(RenderingIntent)
        (mng_info->global_srgb_intent+1);
    if (png_get_sRGB(ping,ping_info,&intent))
      {
        image->rendering_intent=(RenderingIntent) (intent+1);
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Reading PNG sRGB chunk: rendering_intent: %d",intent+1);
      }
  }
#endif
  {
     double
        file_gamma;

     if (mng_info->have_global_gama)
       image->gamma=mng_info->global_gamma;
     if (png_get_gAMA(ping,ping_info,&file_gamma))
       {
         image->gamma=(float) file_gamma;
         if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "    Reading PNG gAMA chunk: gamma: %f",file_gamma);
       }
  }
  if (mng_info->have_global_chrm != MagickFalse)
    image->chromaticity=mng_info->global_chrm;
  if (ping_info->valid & PNG_INFO_cHRM)
    {
      (void) png_get_cHRM(ping,ping_info,
        &image->chromaticity.white_point.x,
        &image->chromaticity.white_point.y,
        &image->chromaticity.red_primary.x,
        &image->chromaticity.red_primary.y,
        &image->chromaticity.green_primary.x,
        &image->chromaticity.green_primary.y,
        &image->chromaticity.blue_primary.x,
        &image->chromaticity.blue_primary.y);
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading PNG cHRM chunk.");
    }
  if (image->rendering_intent)
    {
      image->gamma=0.45455f;
      image->chromaticity.red_primary.x=0.6400f;
      image->chromaticity.red_primary.y=0.3300f;
      image->chromaticity.green_primary.x=0.3000f;
      image->chromaticity.green_primary.y=0.6000f;
      image->chromaticity.blue_primary.x=0.1500f;
      image->chromaticity.blue_primary.y=0.0600f;
      image->chromaticity.white_point.x=0.3127f;
      image->chromaticity.white_point.y=0.3290f;
    }
  if ((mng_info->have_global_gama != MagickFalse) || image->rendering_intent)
    ping_info->valid|=PNG_INFO_gAMA;
  if ((mng_info->have_global_chrm != MagickFalse) || image->rendering_intent)
    ping_info->valid|=PNG_INFO_cHRM;
#if defined(PNG_oFFs_SUPPORTED)
  if (ping_info->valid & PNG_INFO_oFFs)
    {
      image->page.x=png_get_x_offset_pixels(ping, ping_info);
      image->page.y=png_get_y_offset_pixels(ping, ping_info);
      if (logging != MagickFalse)
        if (image->page.x || image->page.y)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Reading PNG oFFs chunk: x: %ld, y: %ld.",image->page.x,
            image->page.y);
    }
#endif
#if defined(PNG_pHYs_SUPPORTED)
  if (ping_info->valid & PNG_INFO_pHYs)
    {
      int
        unit_type;

      png_uint_32
        x_resolution,
        y_resolution;

      /*
        Set image resolution.
      */
      (void) png_get_pHYs(ping,ping_info,&x_resolution,&y_resolution,
          &unit_type);
      image->x_resolution=(float) x_resolution;
      image->y_resolution=(float) y_resolution;
      if (unit_type == PNG_RESOLUTION_METER)
        {
          image->units=PixelsPerCentimeterResolution;
          image->x_resolution=(double) x_resolution/100.0;
          image->y_resolution=(double) y_resolution/100.0;
        }
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading PNG pHYs chunk: xres: %lu, yres: %lu, units: %d.",
          x_resolution, y_resolution, unit_type);
    }
  else
    {
      if (mng_info->have_global_phys)
        {
          image->x_resolution=(float) mng_info->global_x_pixels_per_unit;
          image->y_resolution=(float) mng_info->global_y_pixels_per_unit;
          if (mng_info->global_phys_unit_type == PNG_RESOLUTION_METER)
            {
              image->units=PixelsPerCentimeterResolution;
              image->x_resolution=(double)
                mng_info->global_x_pixels_per_unit/100.0;
              image->y_resolution=(double)
                mng_info->global_y_pixels_per_unit/100.0;
            }
          ping_info->valid|=PNG_INFO_pHYs;
        }
    }
#endif
  if (ping_info->valid & PNG_INFO_PLTE)
    {
      int
        number_colors;

      png_colorp
        palette;

      (void) png_get_PLTE(ping,ping_info,&palette,&number_colors);
      if ((number_colors == 0) &&
          ((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE))
        {
          if (mng_info->global_plte_length)
            {
              png_set_PLTE(ping,ping_info,mng_info->global_plte,
                (int) mng_info->global_plte_length);
              if ((ping_info->valid & PNG_INFO_tRNS) == 0)
                if (mng_info->global_trns_length)
                  {
                    if (mng_info->global_trns_length >
                        mng_info->global_plte_length)
                      (void) ThrowMagickException(&image->exception,
                        GetMagickModule(),CoderError,
                        "global tRNS has more entries than global PLTE",
                        "`%s'",image_info->filename);
                    png_set_tRNS(ping,ping_info,mng_info->global_trns,
                      (int) mng_info->global_trns_length,NULL);
                  }
#if defined(PNG_READ_bKGD_SUPPORTED)
              if (
#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
                   mng_info->have_saved_bkgd_index ||
#endif
                   ping_info->valid & PNG_INFO_bKGD)
                    {
                      png_color_16
                         background;

#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
                      if (mng_info->have_saved_bkgd_index)
                        background.index=mng_info->saved_bkgd_index;
                      else
#endif
                        background.index=ping_info->background.index;
                      background.red=(png_uint_16)
                        mng_info->global_plte[background.index].red;
                      background.green=(png_uint_16)
                        mng_info->global_plte[background.index].green;
                      background.blue=(png_uint_16)
                        mng_info->global_plte[background.index].blue;
                      png_set_bKGD(ping,ping_info,&background);
                    }
#endif
                }
              else
                (void) ThrowMagickException(&image->exception,GetMagickModule(),
                  CoderError,"No global PLTE in file","`%s'",
                  image_info->filename);
            }
        }

#if defined(PNG_READ_bKGD_SUPPORTED)
  if (mng_info->have_global_bkgd && !(ping_info->valid & PNG_INFO_bKGD))
      image->background_color=mng_info->mng_global_bkgd;
  if (ping_info->valid & PNG_INFO_bKGD)
    {
      /*
        Set image background color.
      */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading PNG bKGD chunk.");
      if (ping_info->bit_depth <= MAGICKCORE_QUANTUM_DEPTH)
        {
          image->background_color.red=ping_info->background.red;
          image->background_color.green=ping_info->background.green;
          image->background_color.blue=ping_info->background.blue;
        }
      else
        {
          image->background_color.red=
            ScaleShortToQuantum(ping_info->background.red);
          image->background_color.green=
            ScaleShortToQuantum(ping_info->background.green);
          image->background_color.blue=
            ScaleShortToQuantum(ping_info->background.blue);
        }
    }
#endif
  transparent_color.red=0;
  transparent_color.green=0;
  transparent_color.blue=0;
  transparent_color.opacity=0;
  if (ping_info->valid & PNG_INFO_tRNS)
    {
      /*
        Image has a transparent background.
      */
      int
        max_sample;

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading PNG tRNS chunk.");

      max_sample = (1 << ping_info->bit_depth) - 1;

      if ((ping_info->color_type == PNG_COLOR_TYPE_GRAY &&
          (int)ping_info->trans_color.gray > max_sample) ||
          (ping_info->color_type == PNG_COLOR_TYPE_RGB &&
          ((int)ping_info->trans_color.red > max_sample ||
          (int)ping_info->trans_color.green > max_sample ||
          (int)ping_info->trans_color.blue > max_sample)))
        {
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Ignoring PNG tRNS chunk with out-of-range sample.");
#if (PNG_LIBPNG_VER < 10007)
          ping_info->trans_alpha=(unsigned char *) RelinquishMagickMemory(
            ping_info->trans_alpha);
          ping_info->valid&=(~PNG_INFO_tRNS);
#else
          png_free_data(ping, ping_info, PNG_FREE_TRNS, 0);
#endif
          image->matte=MagickFalse;
        }
      else
        {
          transparent_color.red= (Quantum)(ping_info->trans_color.red);
          transparent_color.green= (Quantum) (ping_info->trans_color.green);
          transparent_color.blue= (Quantum) (ping_info->trans_color.blue);
          transparent_color.opacity= (Quantum) (ping_info->trans_color.gray);
          if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
            {
              if (ping_info->bit_depth < 8)
                {
                  transparent_color.opacity=(Quantum) (((
                    ping_info->trans_color.gray)*255)/max_sample);
                }
              transparent_color.red=transparent_color.opacity;
              transparent_color.green=transparent_color.opacity;
              transparent_color.blue=transparent_color.opacity;
            }
        }
    }
#if defined(PNG_READ_sBIT_SUPPORTED)
  if (mng_info->have_global_sbit)
    {
      if (!(ping_info->valid & PNG_INFO_sBIT))
        png_set_sBIT(ping,ping_info,&mng_info->global_sbit);
    }
#endif
  num_passes=png_set_interlace_handling(ping);
  png_read_update_info(ping,ping_info);
  /*
    Initialize image structure.
  */
  mng_info->image_box.left=0;
  mng_info->image_box.right=(long) ping_info->width;
  mng_info->image_box.top=0;
  mng_info->image_box.bottom=(long) ping_info->height;
  if (mng_info->mng_type == 0)
    {
      mng_info->mng_width=ping_info->width;
      mng_info->mng_height=ping_info->height;
      mng_info->frame=mng_info->image_box;
      mng_info->clip=mng_info->image_box;
    }
  else
    {
      image->page.y=mng_info->y_off[mng_info->object_id];
    }
  image->compression=ZipCompression;
  image->columns=ping_info->width;
  image->rows=ping_info->height;
  if (((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE) ||
      ((int) ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
      ((int) ping_info->color_type == PNG_COLOR_TYPE_GRAY))
    {
      image->storage_class=PseudoClass;
      image->colors=1UL << ping_info->bit_depth;
#if (MAGICKCORE_QUANTUM_DEPTH == 8)
      if (image->colors > 256)
        image->colors=256;
#else
      if (image->colors > 65536L)
        image->colors=65536L;
#endif
      if ((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
        {
          int
            number_colors;

          png_colorp
            palette;

          (void) png_get_PLTE(ping,ping_info,&palette,&number_colors);
          image->colors=(unsigned long) number_colors;
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Reading PNG PLTE chunk: number_colors: %d.",number_colors);
        }
    }

  if (image->storage_class == PseudoClass)
    {
      /*
        Initialize image colormap.
      */
      if (AcquireImageColormap(image,image->colors) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      if ((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
        {
          int
            number_colors;

          png_colorp
            palette;

          (void) png_get_PLTE(ping,ping_info,&palette,&number_colors);
          for (i=0; i < (long) image->colors; i++)
          {
            image->colormap[i].red=ScaleCharToQuantum(palette[i].red);
            image->colormap[i].green=ScaleCharToQuantum(palette[i].green);
            image->colormap[i].blue=ScaleCharToQuantum(palette[i].blue);
          }
        }
      else
        {
          unsigned long
            scale;

          scale=(QuantumRange/((1UL << ping_info->bit_depth)-1));
          if (scale < 1)
             scale=1;
          for (i=0; i < (long) image->colors; i++)
          {
            image->colormap[i].red=(Quantum) (i*scale);
            image->colormap[i].green=(Quantum) (i*scale);
            image->colormap[i].blue=(Quantum) (i*scale);
          }
       }
    }
  /*
    Read image scanlines.
  */
  if (image->delay != 0)
    mng_info->scenes_found++;
  if ((image_info->number_scenes != 0) && (mng_info->scenes_found > (long)
      (image_info->first_scene+image_info->number_scenes)))
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Skipping PNG image data for scene %ld",
          mng_info->scenes_found-1);
      png_destroy_read_struct(&ping,&ping_info,&end_info);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
      UnlockSemaphoreInfo(png_semaphore);
#endif
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  exit ReadOnePNGImage().");
      return(image);
    }
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Reading PNG IDAT chunk(s)");
  if (num_passes > 1)
    png_pixels=(unsigned char *) AcquireQuantumMemory(image->rows,
      ping_info->rowbytes*sizeof(*png_pixels));
  else
    png_pixels=(unsigned char *) AcquireQuantumMemory(ping_info->rowbytes,
      sizeof(*png_pixels));
  if (png_pixels == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Converting PNG pixels to pixel packets");
  /*
    Convert PNG pixels to pixel packets.
  */
  if (setjmp(ping->jmpbuf))
    {
      /*
        PNG image is corrupt.
      */
      png_destroy_read_struct(&ping,&ping_info,&end_info);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
      UnlockSemaphoreInfo(png_semaphore);
#endif
      if (quantum_info != (QuantumInfo *) NULL)
        quantum_info = DestroyQuantumInfo(quantum_info);
      if (png_pixels != (unsigned char *) NULL)
        png_pixels=(unsigned char *) RelinquishMagickMemory(png_pixels);
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  exit ReadOnePNGImage() with error.");
      if (image != (Image *) NULL)
        {
          InheritException(exception,&image->exception);
          image->columns=0;
        }
      return(GetFirstImageInList(image));
    }
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (image->storage_class == DirectClass)
    for (pass=0; pass < num_passes; pass++)
    {
      /*
        Convert image to DirectClass pixel packets.
      */
#if (MAGICKCORE_QUANTUM_DEPTH == 8)
      int
        depth;

      depth=(long) ping_info->bit_depth;
#endif
      image->matte=(((int) ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA) ||
          ((int) ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
          (ping_info->valid & PNG_INFO_tRNS)) ? MagickTrue : MagickFalse;

      for (y=0; y < (long) image->rows; y++)
      {
        if (num_passes > 1)
          row_offset=ping_info->rowbytes*y;
        else
          row_offset=0;
        png_read_row(ping,png_pixels+row_offset,NULL);
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
#if (0 && (MAGICKCORE_QUANTUM_DEPTH == 8) && !defined(MAGICKCORE_HDRI_SUPPORT))
        if (depth == 16)
          {
            register Quantum
              *p,
              *r;

            r=png_pixels+row_offset;
            p=r;
            if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
              {
                for (x=(long) image->columns-1; x >= 0; x--)
                {
                  *r++=*p++;
                  p++;
                  if ((ping_info->valid & PNG_INFO_tRNS) &&
                     (((*(p-2) << 8)|*(p-1)) == transparent_color.opacity))
                    {
                       /* Cheap transparency */
                       *r++=TransparentOpacity;
                    }
                  else
                       *r++=OpaqueOpacity;
                }
              }
            else if (ping_info->color_type == PNG_COLOR_TYPE_RGB)
              {
              if (ping_info->valid & PNG_INFO_tRNS)
                for (x=(long) image->columns-1; x >= 0; x--)
                {
                  *r++=*p++;
                  p++;
                  *r++=*p++;
                  p++;
                  *r++=*p++;
                  p++;
                  if ((((*(p-6) << 8)|*(p-5)) == transparent_color.red) &&
                       (((*(p-4) << 8)|*(p-3)) == transparent_color.green) &&
                       (((*(p-2) << 8)|*(p-1)) == transparent_color.blue))
                    {
                       /* Cheap transparency */
                       *r++=TransparentOpacity;
                    }
                  else
                       *r++=OpaqueOpacity;
                }
              else
                for (x=(long) image->columns-1; x >= 0; x--)
                {
                  *r++=*p++;
                  p++;
                  *r++=*p++;
                  p++;
                  *r++=*p++;
                  p++;
                  *r++=OpaqueOpacity;
                }
              }
            else if (ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
              for (x=(long) (4*image->columns); x != 0; x--)
              {
                *r++=*p++;
                p++;
              }
            else if (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
              for (x=(long) (2*image->columns); x != 0; x--)
              {
                *r++=*p++;
                p++;
              }
          }
        if (depth == 8 && ping_info->color_type == PNG_COLOR_TYPE_GRAY)
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            GrayQuantum,png_pixels+row_offset);
        if (ping_info->color_type == PNG_COLOR_TYPE_GRAY ||
            ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
          {
            quantum_info->depth=8;
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              GrayAlphaQuantum,png_pixels+row_offset);
          }
        else if (depth == 8 && ping_info->color_type == PNG_COLOR_TYPE_RGB)
           (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
             RGBQuantum,png_pixels+row_offset);
        else if (ping_info->color_type == PNG_COLOR_TYPE_RGB ||
              ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
          {
            quantum_info->depth=8;
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              RGBAQuantum,png_pixels+row_offset);
          }
        else if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              IndexQuantum,png_pixels+row_offset);
#else /* (MAGICKCORE_QUANTUM_DEPTH != 8) */
        if ((int) ping_info->color_type == PNG_COLOR_TYPE_GRAY)
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            GrayQuantum,png_pixels+row_offset,exception);
        else if ((int) ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            GrayAlphaQuantum,png_pixels+row_offset,exception);
        else if ((int) ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            RGBAQuantum,png_pixels+row_offset,exception);
        else if ((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            IndexQuantum,png_pixels+row_offset,exception);
        else
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            RGBQuantum,png_pixels+row_offset,exception);
#endif
        if ((image->previous == (Image *) NULL) && (num_passes == 1))
          {
            status=SetImageProgress(image,LoadImageTag,y,image->rows);
            if (status == MagickFalse)
              break;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      if ((image->previous == (Image *) NULL) && (num_passes != 1))
        {
          status=SetImageProgress(image,LoadImageTag,pass,num_passes);
          if (status == MagickFalse)
            break;
        }
    }
  else /* image->storage_class != DirectClass */
    for (pass=0; pass < num_passes; pass++)
    {
      Quantum
        *quantum_scanline;

      register Quantum
        *r;

      /*
        Convert grayscale image to PseudoClass pixel packets.
      */
      image->matte=ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA ?
        MagickTrue : MagickFalse;
      quantum_scanline=(Quantum *) AcquireQuantumMemory(image->columns,
        (image->matte ?  2 : 1)*sizeof(*quantum_scanline));
      if (quantum_scanline == (Quantum *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      for (y=0; y < (long) image->rows; y++)
      {
        if (num_passes > 1)
          row_offset=ping_info->rowbytes*y;
        else
          row_offset=0;
        png_read_row(ping,png_pixels+row_offset,NULL);
        q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indices=GetAuthenticIndexQueue(image);
        p=png_pixels+row_offset;
        r=quantum_scanline;
        switch (ping_info->bit_depth)
        {
          case 1:
          {
            register long
              bit;

            for (x=(long) image->columns-7; x > 0; x-=8)
            {
              for (bit=7; bit >= 0; bit--)
                *r++=(Quantum) ((*p) & (0x01 << bit) ? 0x01 : 0x00);
              p++;
            }
            if ((image->columns % 8) != 0)
              {
                for (bit=7; bit >= (long) (8-(image->columns % 8)); bit--)
                  *r++=(Quantum) ((*p) & (0x01 << bit) ? 0x01 : 0x00);
              }
            break;
          }
          case 2:
          {
            for (x=(long) image->columns-3; x > 0; x-=4)
            {
              *r++=(*p >> 6) & 0x03;
              *r++=(*p >> 4) & 0x03;
              *r++=(*p >> 2) & 0x03;
              *r++=(*p++) & 0x03;
            }
            if ((image->columns % 4) != 0)
              {
                for (i=3; i >= (long) (4-(image->columns % 4)); i--)
                  *r++=(Quantum) ((*p >> (i*2)) & 0x03);
              }
            break;
          }
          case 4:
          {
            for (x=(long) image->columns-1; x > 0; x-=2)
            {
              *r++=(*p >> 4) & 0x0f;
              *r++=(*p++) & 0x0f;
            }
            if ((image->columns % 2) != 0)
              *r++=(*p++ >> 4) & 0x0f;
            break;
          }
          case 8:
          {
            if (ping_info->color_type == 4)
              for (x=(long) image->columns-1; x >= 0; x--)
              {
                *r++=*p++;
                /* In image.h, OpaqueOpacity is 0
                 * TransparentOpacity is QuantumRange
                 * In a PNG datastream, Opaque is QuantumRange
                 * and Transparent is 0.
                 */
                q->opacity=ScaleCharToQuantum((unsigned char) (255-(*p++)));
                q++;
              }
            else
              for (x=(long) image->columns-1; x >= 0; x--)
                *r++=*p++;
            break;
          }
          case 16:
          {
            for (x=(long) image->columns-1; x >= 0; x--)
            {
#if (MAGICKCORE_QUANTUM_DEPTH == 16)
              unsigned long
                quantum;

              if (image->colors > 256)
                *r=((*p++) << 8);
              else
                *r=0;
              quantum=(*r);
              quantum|=(*p++);
              *r=(Quantum) quantum;
              r++;
              if (ping_info->color_type == 4)
                {
                  quantum=((*p++) << 8);
                  quantum|=(*p++);
                  q->opacity=(Quantum) (QuantumRange-quantum);
                  q++;
                }
#else
#if (MAGICKCORE_QUANTUM_DEPTH == 32)
              unsigned long
                quantum;

              if (image->colors > 256)
                *r=((*p++) << 8);
              else
                *r=0;
              quantum=(*r);
              quantum|=(*p++);
              *r=quantum;
              r++;
              if (ping_info->color_type == 4)
                {
                  q->opacity=(*p << 8) | *(p+1);
                  q->opacity*=65537L;
                  q->opacity=(Quantum) GetAlphaPixelComponent(q);
                  p+=2;
                  q++;
                }
#else /* MAGICKCORE_QUANTUM_DEPTH == 8 */
              *r++=(*p++);
              p++; /* strip low byte */
              if (ping_info->color_type == 4)
                {
                  q->opacity=(Quantum) (QuantumRange-(*p++));
                  p++;
                  q++;
                }
#endif
#endif
            }
            break;
          }
          default:
            break;
        }
        /*
          Transfer image scanline.
        */
        r=quantum_scanline;
        for (x=0; x < (long) image->columns; x++)
          indices[x]=(*r++);
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        if ((image->previous == (Image *) NULL) && (num_passes == 1))
          {
            status=SetImageProgress(image,LoadImageTag,y,image->rows);
            if (status == MagickFalse)
              break;
          }
      }
      if ((image->previous == (Image *) NULL) && (num_passes != 1))
        {
          status=SetImageProgress(image,LoadImageTag,pass,num_passes);
          if (status == MagickFalse)
            break;
        }
      quantum_scanline=(Quantum *) RelinquishMagickMemory(quantum_scanline);
    }
  if (quantum_info != (QuantumInfo *) NULL)
    quantum_info=DestroyQuantumInfo(quantum_info);
  if (image->storage_class == PseudoClass)
    (void) SyncImage(image);
  png_read_end(ping,ping_info);

  if (image_info->number_scenes != 0 && mng_info->scenes_found-1 <
      (long) image_info->first_scene && image->delay != 0)
    {
      png_destroy_read_struct(&ping,&ping_info,&end_info);
      png_pixels=(unsigned char *) RelinquishMagickMemory(png_pixels);
      image->colors=2;
      (void) SetImageBackgroundColor(image);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
      UnlockSemaphoreInfo(png_semaphore);
#endif
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  exit ReadOnePNGImage() early.");
      return(image);
    }
  if (ping_info->valid & PNG_INFO_tRNS)
    {
      ClassType
        storage_class;

      /*
        Image has a transparent background.
      */
      storage_class=image->storage_class;
      image->matte=MagickTrue;
      for (y=0; y < (long) image->rows; y++)
      {
        image->storage_class=storage_class;
        q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indices=GetAuthenticIndexQueue(image);

        if (storage_class == PseudoClass)
          {
            IndexPacket
              indexpacket;

            if ((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
              for (x=0; x < (long) image->columns; x++)
              {
                indexpacket=indices[x];
                if (indexpacket < ping_info->num_trans)
                  q->opacity=ScaleCharToQuantum((unsigned char)
                    (255-ping_info->trans_alpha[(long) indexpacket]));
                else
                  SetOpacityPixelComponent(q,OpaqueOpacity);
                q++;
              }
            else if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
              for (x=0; x < (long) image->columns; x++)
              {
                indexpacket=indices[x];
                q->red=image->colormap[(long) indexpacket].red;
                q->green=q->red;
                q->blue=q->red;
                if (q->red == transparent_color.opacity)
                  q->opacity=(Quantum) TransparentOpacity;
                else
                  SetOpacityPixelComponent(q,OpaqueOpacity);
                q++;
              }
          }
        else
          for (x=(long) image->columns-1; x >= 0; x--)
          {
            if (ScaleQuantumToChar(q->red) == transparent_color.red &&
                ScaleQuantumToChar(q->green) == transparent_color.green &&
                ScaleQuantumToChar(q->blue) == transparent_color.blue)
               q->opacity=(Quantum) TransparentOpacity;
            else
              SetOpacityPixelComponent(q,OpaqueOpacity);
            q++;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      image->storage_class=DirectClass;
    }
#if (MAGICKCORE_QUANTUM_DEPTH == 8)
  if (image->depth > 8)
    image->depth=8;
#endif
  if (png_get_text(ping,ping_info,&text,&num_text) != 0)
    for (i=0; i < (long) num_text; i++)
    {
      /* Check for a profile */

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading PNG text chunk");
      if (memcmp(text[i].key, "Raw profile type ",17) == 0)
          (void) png_read_raw_profile(image,image_info,text,(int) i);
      else
        {
          char
            *value;

          length=text[i].text_length;
          value=(char *) AcquireQuantumMemory(length+MaxTextExtent,
            sizeof(*value));
          if (value == (char *) NULL)
            {
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              break;
            }
          *value='\0';
          (void) ConcatenateMagickString(value,text[i].text,length+2);
          (void) SetImageProperty(image,text[i].key,value);
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "      Keyword: %s",text[i].key);
          value=DestroyString(value);
        }
    }
#ifdef MNG_OBJECT_BUFFERS
  /*
    Store the object if necessary.
  */
  if (object_id && !mng_info->frozen[object_id])
    {
      if (mng_info->ob[object_id] == (MngBuffer *) NULL)
        {
          /*
            create a new object buffer.
          */
          mng_info->ob[object_id]=(MngBuffer *)
            AcquireAlignedMemory(1,sizeof(MngBuffer));
          if (mng_info->ob[object_id] != (MngBuffer *) NULL)
            {
              mng_info->ob[object_id]->image=(Image *) NULL;
              mng_info->ob[object_id]->reference_count=1;
            }
        }
      if ((mng_info->ob[object_id] == (MngBuffer *) NULL) ||
          mng_info->ob[object_id]->frozen)
        {
          if (mng_info->ob[object_id] == (MngBuffer *) NULL)
            (void) ThrowMagickException(&image->exception,GetMagickModule(),
              ResourceLimitError,"MemoryAllocationFailed","`%s'",
              image->filename);
          if (mng_info->ob[object_id]->frozen)
            (void) ThrowMagickException(&image->exception,GetMagickModule(),
              ResourceLimitError,"Cannot overwrite frozen MNG object buffer",
              "`%s'",image->filename);
        }
      else
        {
          png_uint_32
            width,
            height;

          int
            bit_depth,
            color_type,
            interlace_method,
            compression_method,
            filter_method;

          if (mng_info->ob[object_id]->image != (Image *) NULL)
            mng_info->ob[object_id]->image=DestroyImage
                (mng_info->ob[object_id]->image);
          mng_info->ob[object_id]->image=CloneImage(image,0,0,MagickTrue,
            &image->exception);
          if (mng_info->ob[object_id]->image != (Image *) NULL)
            mng_info->ob[object_id]->image->file=(FILE *) NULL;
          else
            (void) ThrowMagickException(&image->exception,GetMagickModule(),
              ResourceLimitError,"Cloning image for object buffer failed",
              "`%s'",image->filename);
          png_get_IHDR(ping,ping_info,&width,&height,&bit_depth,&color_type,
            &interlace_method,&compression_method,&filter_method);
          if (width > 250000L || height > 250000L)
             png_error(ping,"PNG Image dimensions are too large.");
          mng_info->ob[object_id]->width=width;
          mng_info->ob[object_id]->height=height;
          mng_info->ob[object_id]->color_type=color_type;
          mng_info->ob[object_id]->sample_depth=bit_depth;
          mng_info->ob[object_id]->interlace_method=interlace_method;
          mng_info->ob[object_id]->compression_method=compression_method;
          mng_info->ob[object_id]->filter_method=filter_method;
          if (ping_info->valid & PNG_INFO_PLTE)
            {
              int
                number_colors;

              png_colorp
                plte;

              /*
                Copy the PLTE to the object buffer.
              */
              png_get_PLTE(ping,ping_info,&plte,&number_colors);
              mng_info->ob[object_id]->plte_length=number_colors;
              for (i=0; i < number_colors; i++)
              {
                mng_info->ob[object_id]->plte[i]=plte[i];
              }
            }
          else
              mng_info->ob[object_id]->plte_length=0;
        }
    }
#endif
  /*
    Relinquish resources.
  */
  png_destroy_read_struct(&ping,&ping_info,&end_info);

  png_pixels=(unsigned char *) RelinquishMagickMemory(png_pixels);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
  UnlockSemaphoreInfo(png_semaphore);
#endif

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit ReadOnePNGImage()");
  return(image);

/* end of reading one PNG image */
}

static Image *ReadPNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image,
    *previous;

  MagickBooleanType
    status;

  MngInfo
    *mng_info;

  char
    magic_number[MaxTextExtent];

  int
    have_mng_structure,
    logging;

  ssize_t
    count;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter ReadPNGImage()");
  image=AcquireImage(image_info);
  mng_info=(MngInfo *) NULL;
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    ThrowReaderException(FileOpenError,"UnableToOpenFile");
  /*
    Verify PNG signature.
  */
  count=ReadBlob(image,8,(unsigned char *) magic_number);
  if (memcmp(magic_number,"\211PNG\r\n\032\n",8) != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Allocate a MngInfo structure.
  */
  have_mng_structure=MagickFalse;
  mng_info=(MngInfo *) AcquireAlignedMemory(1,sizeof(MngInfo));
  if (mng_info == (MngInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize members of the MngInfo structure.
  */
  (void) ResetMagickMemory(mng_info,0,sizeof(MngInfo));
  mng_info->image=image;
  have_mng_structure=MagickTrue;

  previous=image;
  image=ReadOnePNGImage(mng_info,image_info,exception);
  MngInfoFreeStruct(mng_info,&have_mng_structure);
  if (image == (Image *) NULL)
    {
      if (previous != (Image *) NULL)
        {
          if (previous->signature != MagickSignature)
            ThrowReaderException(CorruptImageError,"CorruptImage");
          (void) CloseBlob(previous);
          (void) DestroyImageList(previous);
        }
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadPNGImage() with error");
      return((Image *) NULL);
    }
  (void) CloseBlob(image);
  if ((image->columns == 0) || (image->rows == 0))
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadPNGImage() with error.");
      ThrowReaderException(CorruptImageError,"CorruptImage");
    }
  if (LocaleCompare(image_info->magick,"PNG8") == 0)
    {
      (void) SetImageType(image,PaletteType);
      if (image->matte != MagickFalse)
        {
          /* To do: Reduce to binary transparency */
        }
    }
  if (LocaleCompare(image_info->magick,"PNG24") == 0)
    {
      (void) SetImageType(image,TrueColorType);
      image->matte=MagickFalse;
    }
  if (LocaleCompare(image_info->magick,"PNG32") == 0)
    (void) SetImageType(image,TrueColorMatteType);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit ReadPNGImage()");
  return(image);
}



#if defined(JNG_SUPPORTED)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d O n e J N G I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadOneJNGImage() reads a JPEG Network Graphics (JNG) image file
%  (minus the 8-byte signature)  and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  JNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the ReadOneJNGImage method is:
%
%      Image *ReadOneJNGImage(MngInfo *mng_info, const ImageInfo *image_info,
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
static Image *ReadOneJNGImage(MngInfo *mng_info,
    const ImageInfo *image_info, ExceptionInfo *exception)
{
  Image
    *alpha_image,
    *color_image,
    *image,
    *jng_image;

  ImageInfo
    *alpha_image_info,
    *color_image_info;

  long
    y;

  MagickBooleanType
    status;

  png_uint_32
    jng_height,
    jng_width;

  png_byte
    jng_color_type,
    jng_image_sample_depth,
    jng_image_compression_method,
    jng_image_interlace_method,
    jng_alpha_sample_depth,
    jng_alpha_compression_method,
    jng_alpha_filter_method,
    jng_alpha_interlace_method;

  register const PixelPacket
    *s;

  register long
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  unsigned int
    logging,
    read_JSEP,
    reading_idat,
    skip_to_iend;

  unsigned long
    length;

  jng_alpha_compression_method=0;
  jng_alpha_sample_depth=8;
  jng_color_type=0;
  jng_height=0;
  jng_width=0;
  alpha_image=(Image *) NULL;
  color_image=(Image *) NULL;
  alpha_image_info=(ImageInfo *) NULL;
  color_image_info=(ImageInfo *) NULL;

  logging=LogMagickEvent(CoderEvent,GetMagickModule(),
    "  enter ReadOneJNGImage()");

  image=mng_info->image;
  if (GetAuthenticPixelQueue(image) != (PixelPacket *) NULL)
    {
      /*
        Allocate next image structure.
      */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  AcquireNextImage()");
      AcquireNextImage(image_info,image);
      if (GetNextImageInList(image) == (Image *) NULL)
        return((Image *) NULL);
      image=SyncNextImageInList(image);
    }
  mng_info->image=image;

  /*
    Signature bytes have already been read.
  */

  read_JSEP=MagickFalse;
  reading_idat=MagickFalse;
  skip_to_iend=MagickFalse;
  for (;;)
  {
    char
      type[MaxTextExtent];

    unsigned char
      *chunk;

    unsigned int
      count;

    /*
      Read a new JNG chunk.
    */
    status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
      2*GetBlobSize(image));
    if (status == MagickFalse)
      break;
    type[0]='\0';
    (void) ConcatenateMagickString(type,"errr",MaxTextExtent);
    length=ReadBlobMSBLong(image);
    count=(unsigned int) ReadBlob(image,4,(unsigned char *) type);

    if (logging != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Reading JNG chunk type %c%c%c%c, length: %lu",
        type[0],type[1],type[2],type[3],length);

    if (length > PNG_UINT_31_MAX || count == 0)
      ThrowReaderException(CorruptImageError,"CorruptImage");
    p=NULL;
    chunk=(unsigned char *) NULL;
    if (length)
      {
        chunk=(unsigned char *) AcquireQuantumMemory(length,sizeof(*chunk));
        if (chunk == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        for (i=0; i < (long) length; i++)
          chunk[i]=(unsigned char) ReadBlobByte(image);
        p=chunk;
      }
    (void) ReadBlobMSBLong(image);  /* read crc word */

    if (skip_to_iend)
      {
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_JHDR,4) == 0)
      {
        if (length == 16)
          {
            jng_width=(unsigned long) ((p[0] << 24) | (p[1] << 16) |
                (p[2] << 8) | p[3]);
            jng_height=(unsigned long) ((p[4] << 24) | (p[5] << 16) |
                (p[6] << 8) | p[7]);
            jng_color_type=p[8];
            jng_image_sample_depth=p[9];
            jng_image_compression_method=p[10];
            jng_image_interlace_method=p[11];
            image->interlace=jng_image_interlace_method != 0 ? PNGInterlace :
              NoInterlace;
            jng_alpha_sample_depth=p[12];
            jng_alpha_compression_method=p[13];
            jng_alpha_filter_method=p[14];
            jng_alpha_interlace_method=p[15];
            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_width:      %16lu",jng_width);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_width:      %16lu",jng_height);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_color_type: %16d",jng_color_type);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_image_sample_depth:      %3d",
                  jng_image_sample_depth);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_image_compression_method:%3d",
                  jng_image_compression_method);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_image_interlace_method:  %3d",
                  jng_image_interlace_method);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_alpha_sample_depth:      %3d",
                  jng_alpha_sample_depth);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_alpha_compression_method:%3d",
                  jng_alpha_compression_method);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_alpha_filter_method:     %3d",
                  jng_alpha_filter_method);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    jng_alpha_interlace_method:  %3d",
                  jng_alpha_interlace_method);
              }
          }
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }


    if ((reading_idat == MagickFalse) && (read_JSEP == MagickFalse) &&
        ((memcmp(type,mng_JDAT,4) == 0) || (memcmp(type,mng_JdAA,4) == 0) ||
         (memcmp(type,mng_IDAT,4) == 0) || (memcmp(type,mng_JDAA,4) == 0)))
      {
        /*
           o create color_image
           o open color_blob, attached to color_image
           o if (color type has alpha)
               open alpha_blob, attached to alpha_image
        */

        color_image_info=(ImageInfo *)AcquireAlignedMemory(1,sizeof(ImageInfo));
        if (color_image_info == (ImageInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        GetImageInfo(color_image_info);
        color_image=AcquireImage(color_image_info);
        if (color_image == (Image *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Creating color_blob.");
        (void) AcquireUniqueFilename(color_image->filename);
        status=OpenBlob(color_image_info,color_image,WriteBinaryBlobMode,
          exception);
        if (status == MagickFalse)
          return((Image *) NULL);

        if ((image_info->ping == MagickFalse) && (jng_color_type >= 12))
          {
            alpha_image_info=(ImageInfo *)
              AcquireAlignedMemory(1,sizeof(ImageInfo));
            if (alpha_image_info == (ImageInfo *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            GetImageInfo(alpha_image_info);
            alpha_image=AcquireImage(alpha_image_info);
            if (alpha_image == (Image *) NULL)
              {
                alpha_image=DestroyImage(alpha_image);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Creating alpha_blob.");
            (void) AcquireUniqueFilename(alpha_image->filename);
            status=OpenBlob(alpha_image_info,alpha_image,WriteBinaryBlobMode,
              exception);
            if (status == MagickFalse)
              return((Image *) NULL);
            if (jng_alpha_compression_method == 0)
              {
                unsigned char
                  data[18];

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Writing IHDR chunk to alpha_blob.");
                (void) WriteBlob(alpha_image,8,(const unsigned char *)
                  "\211PNG\r\n\032\n");
                (void) WriteBlobMSBULong(alpha_image,13L);
                PNGType(data,mng_IHDR);
                LogPNGChunk((int) logging,mng_IHDR,13L);
                PNGLong(data+4,jng_width);
                PNGLong(data+8,jng_height);
                data[12]=jng_alpha_sample_depth;
                data[13]=0; /* color_type gray */
                data[14]=0; /* compression method 0 */
                data[15]=0; /* filter_method 0 */
                data[16]=0; /* interlace_method 0 */
                (void) WriteBlob(alpha_image,17,data);
                (void) WriteBlobMSBULong(alpha_image,crc32(0,data,17));
              }
          }
        reading_idat=MagickTrue;
      }

    if (memcmp(type,mng_JDAT,4) == 0)
      {
        /*
           Copy chunk to color_image->blob
        */

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Copying JDAT chunk data to color_blob.");

        (void) WriteBlob(color_image,length,chunk);
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_IDAT,4) == 0)
      {
        png_byte
           data[5];

        /*
           Copy IDAT header and chunk data to alpha_image->blob
        */

        if (image_info->ping == MagickFalse)
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Copying IDAT chunk data to alpha_blob.");

            (void) WriteBlobMSBULong(alpha_image,(unsigned long) length);
            PNGType(data,mng_IDAT);
            LogPNGChunk((int) logging,mng_IDAT,length);
            (void) WriteBlob(alpha_image,4,data);
            (void) WriteBlob(alpha_image,length,chunk);
            (void) WriteBlobMSBULong(alpha_image,
              crc32(crc32(0,data,4),chunk,(uInt) length));
          }
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if ((memcmp(type,mng_JDAA,4) == 0) || (memcmp(type,mng_JdAA,4) == 0))
      {
        /*
           Copy chunk data to alpha_image->blob
        */

        if (image_info->ping == MagickFalse)
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Copying JDAA chunk data to alpha_blob.");

            (void) WriteBlob(alpha_image,length,chunk);
          }
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_JSEP,4) == 0)
      {
        read_JSEP=MagickTrue;
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_bKGD,4) == 0)
      {
        if (length == 2)
          {
            image->background_color.red=ScaleCharToQuantum(p[1]);
            image->background_color.green=image->background_color.red;
            image->background_color.blue=image->background_color.red;
          }
        if (length == 6)
          {
            image->background_color.red=ScaleCharToQuantum(p[1]);
            image->background_color.green=ScaleCharToQuantum(p[3]);
            image->background_color.blue=ScaleCharToQuantum(p[5]);
          }
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_gAMA,4) == 0)
      {
        if (length == 4)
          image->gamma=((float) mng_get_long(p))*0.00001;
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_cHRM,4) == 0)
      {
        if (length == 32)
          {
            image->chromaticity.white_point.x=0.00001*mng_get_long(p);
            image->chromaticity.white_point.y=0.00001*mng_get_long(&p[4]);
            image->chromaticity.red_primary.x=0.00001*mng_get_long(&p[8]);
            image->chromaticity.red_primary.y=0.00001*mng_get_long(&p[12]);
            image->chromaticity.green_primary.x=0.00001*mng_get_long(&p[16]);
            image->chromaticity.green_primary.y=0.00001*mng_get_long(&p[20]);
            image->chromaticity.blue_primary.x=0.00001*mng_get_long(&p[24]);
            image->chromaticity.blue_primary.y=0.00001*mng_get_long(&p[28]);
          }
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_sRGB,4) == 0)
      {
        if (length == 1)
          {
            image->rendering_intent=(RenderingIntent) (p[0]+1);
            image->gamma=0.45455f;
            image->chromaticity.red_primary.x=0.6400f;
            image->chromaticity.red_primary.y=0.3300f;
            image->chromaticity.green_primary.x=0.3000f;
            image->chromaticity.green_primary.y=0.6000f;
            image->chromaticity.blue_primary.x=0.1500f;
            image->chromaticity.blue_primary.y=0.0600f;
            image->chromaticity.white_point.x=0.3127f;
            image->chromaticity.white_point.y=0.3290f;
          }
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_oFFs,4) == 0)
      {
        if (length > 8)
          {
            image->page.x=mng_get_long(p);
            image->page.y=mng_get_long(&p[4]);
            if ((int) p[8] != 0)
              {
                image->page.x/=10000;
                image->page.y/=10000;
              }
          }
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

    if (memcmp(type,mng_pHYs,4) == 0)
      {
        if (length > 8)
          {
            image->x_resolution=(double) mng_get_long(p);
            image->y_resolution=(double) mng_get_long(&p[4]);
            if ((int) p[8] == PNG_RESOLUTION_METER)
              {
                image->units=PixelsPerCentimeterResolution;
                image->x_resolution=image->x_resolution/100.0f;
                image->y_resolution=image->y_resolution/100.0f;
              }
          }
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }

#if 0
    if (memcmp(type,mng_iCCP,4) == 0)
      {
        /* To do. */
        if (length)
          chunk=(unsigned char *) RelinquishMagickMemory(chunk);
        continue;
      }
#endif

    if (length)
      chunk=(unsigned char *) RelinquishMagickMemory(chunk);

    if (memcmp(type,mng_IEND,4))
      continue;
    break;
  }


  /* IEND found */

  /*
    Finish up reading image data:

       o read main image from color_blob.

       o close color_blob.

       o if (color_type has alpha)
            if alpha_encoding is PNG
               read secondary image from alpha_blob via ReadPNG
            if alpha_encoding is JPEG
               read secondary image from alpha_blob via ReadJPEG

       o close alpha_blob.

       o copy intensity of secondary image into
         opacity samples of main image.

       o destroy the secondary image.
  */

  (void) CloseBlob(color_image);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Reading jng_image from color_blob.");
  (void) FormatMagickString(color_image_info->filename,MaxTextExtent,"%s",
    color_image->filename);
  color_image_info->ping=MagickFalse;   /* To do: avoid this */
  jng_image=ReadImage(color_image_info,exception);
  if (jng_image == (Image *) NULL)
    return((Image *) NULL);

  (void) RelinquishUniqueFileResource(color_image->filename);
  color_image=DestroyImage(color_image);
  color_image_info=DestroyImageInfo(color_image_info);

  if (jng_image == (Image *) NULL)
    return((Image *) NULL);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Copying jng_image pixels to main image.");
  image->rows=jng_height;
  image->columns=jng_width;
  length=image->columns*sizeof(PixelPacket);
  for (y=0; y < (long) image->rows; y++)
  {
    s=GetVirtualPixels(jng_image,0,y,image->columns,1,&image->exception);
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    (void) CopyMagickMemory(q,s,length);
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  jng_image=DestroyImage(jng_image);
  if (image_info->ping == MagickFalse)
    {
     if (jng_color_type >= 12)
       {
         if (jng_alpha_compression_method == 0)
           {
             png_byte
               data[5];
             (void) WriteBlobMSBULong(alpha_image,0x00000000L);
             PNGType(data,mng_IEND);
             LogPNGChunk((int) logging,mng_IEND,0L);
             (void) WriteBlob(alpha_image,4,data);
             (void) WriteBlobMSBULong(alpha_image,crc32(0,data,4));
           }
         (void) CloseBlob(alpha_image);
         if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "    Reading opacity from alpha_blob.");

         (void) FormatMagickString(alpha_image_info->filename,MaxTextExtent,
           "%s",alpha_image->filename);

         jng_image=ReadImage(alpha_image_info,exception);
         if (jng_image != (Image *) NULL)
           for (y=0; y < (long) image->rows; y++)
           {
             s=GetVirtualPixels(jng_image,0,y,image->columns,1,
                &image->exception);
             q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
             if (image->matte != MagickFalse)
               for (x=(long) image->columns; x != 0; x--,q++,s++)
                  q->opacity=(Quantum) QuantumRange-s->red;
             else
               for (x=(long) image->columns; x != 0; x--,q++,s++)
               {
                  q->opacity=(Quantum) QuantumRange-s->red;
                  if (q->opacity != OpaqueOpacity)
                    image->matte=MagickTrue;
               }
             if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
           }
         (void) RelinquishUniqueFileResource(alpha_image->filename);
         alpha_image=DestroyImage(alpha_image);
         alpha_image_info=DestroyImageInfo(alpha_image_info);
         if (jng_image != (Image *) NULL)
           jng_image=DestroyImage(jng_image);
       }
    }

  /*
    Read the JNG image.
  */
  if (mng_info->mng_type == 0)
    {
      mng_info->mng_width=jng_width;
      mng_info->mng_height=jng_height;
    }
  if (image->page.width == 0 && image->page.height == 0)
  {
    image->page.width=jng_width;
    image->page.height=jng_height;
  }
  if (image->page.x == 0 && image->page.y == 0)
  {
    image->page.x=mng_info->x_off[mng_info->object_id];
    image->page.y=mng_info->y_off[mng_info->object_id];
  }
  else
  {
    image->page.y=mng_info->y_off[mng_info->object_id];
  }
  mng_info->image_found++;
  status=SetImageProgress(image,LoadImagesTag,2*TellBlob(image),
    2*GetBlobSize(image));
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit ReadOneJNGImage()");
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J N G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJNGImage() reads a JPEG Network Graphics (JNG) image file
%  (including the 8-byte signature)  and returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  JNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the ReadJNGImage method is:
%
%      Image *ReadJNGImage(const ImageInfo *image_info, ExceptionInfo
%         *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *ReadJNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image,
    *previous;

  MagickBooleanType
    status;

  MngInfo
    *mng_info;

  char
    magic_number[MaxTextExtent];

  int
    have_mng_structure,
    logging;

  size_t
    count;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter ReadJNGImage()");
  image=AcquireImage(image_info);
  mng_info=(MngInfo *) NULL;
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  if (LocaleCompare(image_info->magick,"JNG") != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Verify JNG signature.
  */
  count=(size_t) ReadBlob(image,8,(unsigned char *) magic_number);
  if (memcmp(magic_number,"\213JNG\r\n\032\n",8) != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Allocate a MngInfo structure.
  */
  have_mng_structure=MagickFalse;
  mng_info=(MngInfo *) AcquireAlignedMemory(1,sizeof(*mng_info));
  if (mng_info == (MngInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize members of the MngInfo structure.
  */
  (void) ResetMagickMemory(mng_info,0,sizeof(MngInfo));
  have_mng_structure=MagickTrue;

  mng_info->image=image;
  previous=image;
  image=ReadOneJNGImage(mng_info,image_info,exception);
  MngInfoFreeStruct(mng_info,&have_mng_structure);
  if (image == (Image *) NULL)
    {
      if (IsImageObject(previous) != MagickFalse)
        {
          (void) CloseBlob(previous);
          (void) DestroyImageList(previous);
        }
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadJNGImage() with error");
      return((Image *) NULL);
    }
  (void) CloseBlob(image);
  if (image->columns == 0 || image->rows == 0)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "exit ReadJNGImage() with error");
      ThrowReaderException(CorruptImageError,"CorruptImage");
    }
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit ReadJNGImage()");
  return(image);
}
#endif

static Image *ReadMNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    page_geometry[MaxTextExtent];

  Image
    *image,
    *previous;

  int
    have_mng_structure;

  volatile int
    first_mng_object,
    logging,
    object_id,
    term_chunk_found,
    skip_to_iend;

  volatile long
    image_count=0;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  MngInfo
    *mng_info;

  MngBox
    default_fb,
    fb,
    previous_fb;

#if defined(MNG_INSERT_LAYERS)
  PixelPacket
    mng_background_color;
#endif

  register unsigned char
    *p;

  register long
    i;

  size_t
    count;

  long
    loop_level;

  volatile short
    skipping_loop;

#if defined(MNG_INSERT_LAYERS)
  unsigned int
    mandatory_back=0;
#endif

  volatile unsigned int
#ifdef MNG_OBJECT_BUFFERS
    mng_background_object=0,
#endif
    mng_type=0;   /* 0: PNG or JNG; 1: MNG; 2: MNG-LC; 3: MNG-VLC */

  unsigned long
    default_frame_timeout,
    frame_timeout,
#if defined(MNG_INSERT_LAYERS)
    image_height,
    image_width,
#endif
    length;

  volatile unsigned long
    default_frame_delay,
    final_delay,
    final_image_delay,
    frame_delay,
#if defined(MNG_INSERT_LAYERS)
    insert_layers,
#endif
    mng_iterations=1,
    simplicity=0,
    subframe_height=0,
    subframe_width=0;

  previous_fb.top=0;
  previous_fb.bottom=0;
  previous_fb.left=0;
  previous_fb.right=0;
  default_fb.top=0;
  default_fb.bottom=0;
  default_fb.left=0;
  default_fb.right=0;

  /*
    Set image_info->type=OptimizeType (new in version 5.4.0) to get the
    following optimizations:

    o  16-bit depth is reduced to 8 if all pixels contain samples whose
       high byte and low byte are identical.
    o  Opaque matte channel is removed.
    o  If matte channel is present but only one transparent color is
       present, RGB+tRNS is written instead of RGBA
    o  Grayscale images are reduced to 1, 2, or 4 bit depth if
       this can be done without loss.
    o  Palette is sorted to remove unused entries and to put a
       transparent color first, if PNG_SORT_PALETTE is also defined.
   */

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter ReadMNGImage()");
  image=AcquireImage(image_info);
  mng_info=(MngInfo *) NULL;
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  first_mng_object=MagickFalse;
  skipping_loop=(-1);
  have_mng_structure=MagickFalse;
  /*
    Allocate a MngInfo structure.
  */
  mng_info=(MngInfo *) AcquireAlignedMemory(1,sizeof(MngInfo));
  if (mng_info == (MngInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize members of the MngInfo structure.
  */
  (void) ResetMagickMemory(mng_info,0,sizeof(MngInfo));
  mng_info->image=image;
  have_mng_structure=MagickTrue;
#if (MAGICKCORE_QUANTUM_DEPTH == 16)
    mng_info->optimize=image_info->type == OptimizeType;
#endif

  if (LocaleCompare(image_info->magick,"MNG") == 0)
    {
      char
        magic_number[MaxTextExtent];

      /*
        Verify MNG signature.
      */
      count=(size_t) ReadBlob(image,8,(unsigned char *) magic_number);
      if (memcmp(magic_number,"\212MNG\r\n\032\n",8) != 0)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      /*
        Initialize some nonzero members of the MngInfo structure.
      */
      for (i=0; i < MNG_MAX_OBJECTS; i++)
      {
        mng_info->object_clip[i].right=(long) PNG_UINT_31_MAX;
        mng_info->object_clip[i].bottom=(long) PNG_UINT_31_MAX;
      }
      mng_info->exists[0]=MagickTrue;
    }
  first_mng_object=MagickTrue;
  mng_type=0;
#if defined(MNG_INSERT_LAYERS)
  insert_layers=MagickFalse; /* should be False when converting or mogrifying */
#endif
  default_frame_delay=0;
  default_frame_timeout=0;
  frame_delay=0;
  final_delay=1;
  mng_info->ticks_per_second=1UL*image->ticks_per_second;
  object_id=0;
  skip_to_iend=MagickFalse;
  term_chunk_found=MagickFalse;
  mng_info->framing_mode=1;
#if defined(MNG_INSERT_LAYERS)
  mandatory_back=MagickFalse;
#endif
#if defined(MNG_INSERT_LAYERS)
  mng_background_color=image->background_color;
#endif
  default_fb=mng_info->frame;
  previous_fb=mng_info->frame;
  do
  {
    char
      type[MaxTextExtent];

    if (LocaleCompare(image_info->magick,"MNG") == 0)
      {
        unsigned char
          *chunk;

        /*
          Read a new chunk.
        */
        type[0]='\0';
        (void) ConcatenateMagickString(type,"errr",MaxTextExtent);
        length=ReadBlobMSBLong(image);
        count=(size_t) ReadBlob(image,4,(unsigned char *) type);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  Reading MNG chunk type %c%c%c%c, length: %lu",
           type[0],type[1],type[2],type[3],length);

        if (length > PNG_UINT_31_MAX)
          status=MagickFalse;
        if (count == 0)
          ThrowReaderException(CorruptImageError,"CorruptImage");
        p=NULL;
        chunk=(unsigned char *) NULL;
        if (length)
          {
            chunk=(unsigned char *) AcquireQuantumMemory(length,sizeof(*chunk));
            if (chunk == (unsigned char *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            for (i=0; i < (long) length; i++)
              chunk[i]=(unsigned char) ReadBlobByte(image);
            p=chunk;
          }
        (void) ReadBlobMSBLong(image);  /* read crc word */

#if !defined(JNG_SUPPORTED)
        if (memcmp(type,mng_JHDR,4) == 0)
          {
            skip_to_iend=MagickTrue;
            if (mng_info->jhdr_warning == 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"JNGCompressNotSupported","`%s'",image->filename);
            mng_info->jhdr_warning++;
          }
#endif
        if (memcmp(type,mng_DHDR,4) == 0)
          {
            skip_to_iend=MagickTrue;
            if (mng_info->dhdr_warning == 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"DeltaPNGNotSupported","`%s'",image->filename);
            mng_info->dhdr_warning++;
          }
        if (memcmp(type,mng_MEND,4) == 0)
          break;
        if (skip_to_iend)
          {
            if (memcmp(type,mng_IEND,4) == 0)
              skip_to_iend=MagickFalse;
            if (length)
              chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Skip to IEND.");
            continue;
          }
        if (memcmp(type,mng_MHDR,4) == 0)
          {
            mng_info->mng_width=(unsigned long) ((p[0] << 24) | (p[1] << 16) |
                (p[2] << 8) | p[3]);
            mng_info->mng_height=(unsigned long) ((p[4] << 24) | (p[5] << 16) |
                (p[6] << 8) | p[7]);
            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  MNG width: %lu",mng_info->mng_width);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  MNG height: %lu",mng_info->mng_height);
              }
            p+=8;
            mng_info->ticks_per_second=(unsigned long) mng_get_long(p);
            if (mng_info->ticks_per_second == 0)
              default_frame_delay=0;
            else
              default_frame_delay=1UL*image->ticks_per_second/
                mng_info->ticks_per_second;
            frame_delay=default_frame_delay;
            simplicity=0;
            if (length > 16)
              {
                p+=16;
                simplicity=(unsigned long) mng_get_long(p);
              }
            mng_type=1;    /* Full MNG */
            if ((simplicity != 0) && ((simplicity | 11) == 11))
              mng_type=2; /* LC */
            if ((simplicity != 0) && ((simplicity | 9) == 9))
              mng_type=3; /* VLC */
#if defined(MNG_INSERT_LAYERS)
            if (mng_type != 3)
              insert_layers=MagickTrue;
#endif
            if (GetAuthenticPixelQueue(image) != (PixelPacket *) NULL)
              {
                /*
                  Allocate next image structure.
                */
                AcquireNextImage(image_info,image);
                if (GetNextImageInList(image) == (Image *) NULL)
                  return((Image *) NULL);
                image=SyncNextImageInList(image);
                mng_info->image=image;
              }

            if ((mng_info->mng_width > 65535L) ||
                (mng_info->mng_height > 65535L))
              ThrowReaderException(ImageError,"WidthOrHeightExceedsLimit");
            (void) FormatMagickString(page_geometry,MaxTextExtent,"%lux%lu+0+0",
              mng_info->mng_width,mng_info->mng_height);
            mng_info->frame.left=0;
            mng_info->frame.right=(long) mng_info->mng_width;
            mng_info->frame.top=0;
            mng_info->frame.bottom=(long) mng_info->mng_height;
            mng_info->clip=default_fb=previous_fb=mng_info->frame;
            for (i=0; i < MNG_MAX_OBJECTS; i++)
              mng_info->object_clip[i]=mng_info->frame;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_TERM,4) == 0)
          {
            int
              repeat=0;


            if (length)
              repeat=p[0];
            if (repeat == 3)
              {
                final_delay=(png_uint_32) mng_get_long(&p[2]);
                mng_iterations=(png_uint_32) mng_get_long(&p[6]);
                if (mng_iterations == PNG_UINT_31_MAX)
                  mng_iterations=0;
                image->iterations=mng_iterations;
                term_chunk_found=MagickTrue;
              }
            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    repeat=%d",repeat);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    final_delay=%ld",final_delay);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    image->iterations=%ld",image->iterations);
              }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_DEFI,4) == 0)
          {
            if (mng_type == 3)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"DEFI chunk found in MNG-VLC datastream","`%s'",
                image->filename);
            object_id=(p[0] << 8) | p[1];
            if (mng_type == 2 && object_id != 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"Nonzero object_id in MNG-LC datastream","`%s'",
                image->filename);
            if (object_id > MNG_MAX_OBJECTS)
              {
                /*
                  Instead ofsuing a warning we should allocate a larger
                  MngInfo structure and continue.
                */
                (void) ThrowMagickException(&image->exception,GetMagickModule(),
                  CoderError,"object id too large","`%s'",image->filename);
                object_id=MNG_MAX_OBJECTS;
              }
            if (mng_info->exists[object_id])
              if (mng_info->frozen[object_id])
                {
                  chunk=(unsigned char *) RelinquishMagickMemory(chunk);
                  (void) ThrowMagickException(&image->exception,
                    GetMagickModule(),CoderError,
                    "DEFI cannot redefine a frozen MNG object","`%s'",
                    image->filename);
                  continue;
                }
            mng_info->exists[object_id]=MagickTrue;
            if (length > 2)
              mng_info->invisible[object_id]=p[2];
            /*
              Extract object offset info.
            */
            if (length > 11)
              {
                mng_info->x_off[object_id]=(long) ((p[4] << 24) | (p[5] << 16) |
                (p[6] << 8) | p[7]);
                mng_info->y_off[object_id]=(long) ((p[8] << 24) | (p[9] << 16) |
                (p[10] << 8) | p[11]);
                if (logging != MagickFalse)
                  {
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "  x_off[%d]: %lu",object_id,mng_info->x_off[object_id]);
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "  y_off[%d]: %lu",object_id,mng_info->y_off[object_id]);
                  }
              }
            /*
              Extract object clipping info.
            */
            if (length > 27)
              mng_info->object_clip[object_id]=mng_read_box(mng_info->frame,0,
                &p[12]);
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_bKGD,4) == 0)
          {
            mng_info->have_global_bkgd=MagickFalse;
            if (length > 5)
              {
                mng_info->mng_global_bkgd.red=
                  ScaleShortToQuantum((unsigned short) ((p[0] << 8) | p[1]));
                mng_info->mng_global_bkgd.green=
                  ScaleShortToQuantum((unsigned short) ((p[2] << 8) | p[3]));
                mng_info->mng_global_bkgd.blue=
                  ScaleShortToQuantum((unsigned short) ((p[4] << 8) | p[5]));
                mng_info->have_global_bkgd=MagickTrue;
              }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_BACK,4) == 0)
          {
#if defined(MNG_INSERT_LAYERS)
            if (length > 6)
              mandatory_back=p[6];
            else
              mandatory_back=0;
            if (mandatory_back && length > 5)
              {
                mng_background_color.red=
                    ScaleShortToQuantum((unsigned short) ((p[0] << 8) | p[1]));
                mng_background_color.green=
                    ScaleShortToQuantum((unsigned short) ((p[2] << 8) | p[3]));
                mng_background_color.blue=
                    ScaleShortToQuantum((unsigned short) ((p[4] << 8) | p[5]));
                mng_background_color.opacity=OpaqueOpacity;
              }
#ifdef MNG_OBJECT_BUFFERS
            if (length > 8)
              mng_background_object=(p[7] << 8) | p[8];
#endif
#endif
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_PLTE,4) == 0)
          {
            /*
              Read global PLTE.
            */
            if (length && (length < 769))
              {
                if (mng_info->global_plte == (png_colorp) NULL)
                  mng_info->global_plte=(png_colorp) AcquireQuantumMemory(256,
                    sizeof(*mng_info->global_plte));
                for (i=0; i < (long) (length/3); i++)
                {
                  mng_info->global_plte[i].red=p[3*i];
                  mng_info->global_plte[i].green=p[3*i+1];
                  mng_info->global_plte[i].blue=p[3*i+2];
                }
                mng_info->global_plte_length=length/3;
              }
#ifdef MNG_LOOSE
            for ( ; i < 256; i++)
            {
              mng_info->global_plte[i].red=i;
              mng_info->global_plte[i].green=i;
              mng_info->global_plte[i].blue=i;
            }
            if (length)
              mng_info->global_plte_length=256;
#endif
            else
              mng_info->global_plte_length=0;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_tRNS,4) == 0)
          {
            /* read global tRNS */

            if (length < 257)
              for (i=0; i < (long) length; i++)
                mng_info->global_trns[i]=p[i];

#ifdef MNG_LOOSE
            for ( ; i < 256; i++)
              mng_info->global_trns[i]=255;
#endif
            mng_info->global_trns_length=length;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_gAMA,4) == 0)
          {
            if (length == 4)
              {
                long
                  igamma;

                igamma=mng_get_long(p);
                mng_info->global_gamma=((float) igamma)*0.00001;
                mng_info->have_global_gama=MagickTrue;
              }
            else
              mng_info->have_global_gama=MagickFalse;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_cHRM,4) == 0)
          {
            /*
              Read global cHRM
            */
            if (length == 32)
              {
                mng_info->global_chrm.white_point.x=0.00001*mng_get_long(p);
                mng_info->global_chrm.white_point.y=0.00001*mng_get_long(&p[4]);
                mng_info->global_chrm.red_primary.x=0.00001*mng_get_long(&p[8]);
                mng_info->global_chrm.red_primary.y=0.00001*
                  mng_get_long(&p[12]);
                mng_info->global_chrm.green_primary.x=0.00001*
                  mng_get_long(&p[16]);
                mng_info->global_chrm.green_primary.y=0.00001*
                  mng_get_long(&p[20]);
                mng_info->global_chrm.blue_primary.x=0.00001*
                  mng_get_long(&p[24]);
                mng_info->global_chrm.blue_primary.y=0.00001*
                  mng_get_long(&p[28]);
                mng_info->have_global_chrm=MagickTrue;
              }
            else
              mng_info->have_global_chrm=MagickFalse;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_sRGB,4) == 0)
          {
            /*
              Read global sRGB.
            */
            if (length)
              {
                mng_info->global_srgb_intent=(RenderingIntent) (p[0]+1);
                mng_info->have_global_srgb=MagickTrue;
              }
            else
              mng_info->have_global_srgb=MagickFalse;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_iCCP,4) == 0)
          {
            /* To do. */

            /*
              Read global iCCP.
            */
            if (length)
              chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_FRAM,4) == 0)
          {
            if (mng_type == 3)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"FRAM chunk found in MNG-VLC datastream","`%s'",
                image->filename);
            if ((mng_info->framing_mode == 2) || (mng_info->framing_mode == 4))
              image->delay=frame_delay;
            frame_delay=default_frame_delay;
            frame_timeout=default_frame_timeout;
            fb=default_fb;
            if (length)
              if (p[0])
                mng_info->framing_mode=p[0];
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Framing_mode=%d",mng_info->framing_mode);
            if (length > 6)
              {
                /*
                  Note the delay and frame clipping boundaries.
                */
                p++; /* framing mode */
                while (*p && ((p-chunk) < (long) length))
                  p++;  /* frame name */
                p++;  /* frame name terminator */
                if ((p-chunk) < (long) (length-4))
                  {
                    int
                      change_delay,
                      change_timeout,
                      change_clipping;

                    change_delay=(*p++);
                    change_timeout=(*p++);
                    change_clipping=(*p++);
                    p++; /* change_sync */
                    if (change_delay)
                      {
                        frame_delay=(1UL*image->ticks_per_second*
                            (mng_get_long(p))/mng_info->ticks_per_second);
                        if (change_delay == 2)
                          default_frame_delay=frame_delay;
                        p+=4;
                        if (logging != MagickFalse)
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "    Framing_delay=%ld",frame_delay);
                      }
                    if (change_timeout)
                      {
                        frame_timeout=(1UL*image->ticks_per_second*
                            (mng_get_long(p))/mng_info->ticks_per_second);
                        if (change_delay == 2)
                          default_frame_timeout=frame_timeout;
                        p+=4;
                        if (logging != MagickFalse)
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "    Framing_timeout=%ld",frame_timeout);
                      }
                    if (change_clipping)
                      {
                        fb=mng_read_box(previous_fb,(char) p[0],&p[1]);
                        p+=17;
                        previous_fb=fb;
                        if (logging != MagickFalse)
                          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "    Frame_clipping: L=%ld R=%ld T=%ld B=%ld",
                              fb.left, fb.right,fb.top,fb.bottom);
                        if (change_clipping == 2)
                          default_fb=fb;
                      }
                  }
              }
            mng_info->clip=fb;
            mng_info->clip=mng_minimum_box(fb,mng_info->frame);
            subframe_width=(unsigned long) (mng_info->clip.right
               -mng_info->clip.left);
            subframe_height=(unsigned long) (mng_info->clip.bottom
               -mng_info->clip.top);
            /*
              Insert a background layer behind the frame if framing_mode is 4.
            */
#if defined(MNG_INSERT_LAYERS)
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "   subframe_width=%lu, subframe_height=%lu",
                subframe_width, subframe_height);
            if (insert_layers && (mng_info->framing_mode == 4) &&
                (subframe_width) && (subframe_height))
              {
                /*
                  Allocate next image structure.
                */
                if (GetAuthenticPixelQueue(image) != (PixelPacket *) NULL)
                  {
                    AcquireNextImage(image_info,image);
                    if (GetNextImageInList(image) == (Image *) NULL)
                      {
                        image=DestroyImageList(image);
                        MngInfoFreeStruct(mng_info,&have_mng_structure);
                        return((Image *) NULL);
                      }
                    image=SyncNextImageInList(image);
                  }
                mng_info->image=image;
                if (term_chunk_found)
                  {
                    image->start_loop=MagickTrue;
                    image->iterations=mng_iterations;
                    term_chunk_found=MagickFalse;
                  }
                else
                    image->start_loop=MagickFalse;
                image->columns=subframe_width;
                image->rows=subframe_height;
                image->page.width=subframe_width;
                image->page.height=subframe_height;
                image->page.x=mng_info->clip.left;
                image->page.y=mng_info->clip.top;
                image->background_color=mng_background_color;
                image->matte=MagickFalse;
                image->delay=0;
                (void) SetImageBackgroundColor(image);
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Inserted background layer, L=%ld, R=%ld, T=%ld, B=%ld",
                    mng_info->clip.left,mng_info->clip.right,
                    mng_info->clip.top,mng_info->clip.bottom);
              }
#endif
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_CLIP,4) == 0)
          {
            unsigned int
              first_object,
              last_object;

            /*
              Read CLIP.
            */
            first_object=(p[0] << 8) | p[1];
            last_object=(p[2] << 8) | p[3];
            for (i=(int) first_object; i <= (int) last_object; i++)
            {
              if (mng_info->exists[i] && !mng_info->frozen[i])
                {
                  MngBox
                    box;

                  box=mng_info->object_clip[i];
                  mng_info->object_clip[i]=mng_read_box(box,(char) p[4],&p[5]);
                }
            }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_SAVE,4) == 0)
          {
            for (i=1; i < MNG_MAX_OBJECTS; i++)
              if (mng_info->exists[i])
                {
                 mng_info->frozen[i]=MagickTrue;
#ifdef MNG_OBJECT_BUFFERS
                 if (mng_info->ob[i] != (MngBuffer *) NULL)
                    mng_info->ob[i]->frozen=MagickTrue;
#endif
                }
            if (length)
              chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if ((memcmp(type,mng_DISC,4) == 0) || (memcmp(type,mng_SEEK,4) == 0))
          {
            /*
              Read DISC or SEEK.
            */
            if ((length == 0) || !memcmp(type,mng_SEEK,4))
              {
                for (i=1; i < MNG_MAX_OBJECTS; i++)
                  MngInfoDiscardObject(mng_info,i);
              }
            else
              {
                register long
                  j;

                for (j=0; j < (long) length; j+=2)
                {
                  i=p[j] << 8 | p[j+1];
                  MngInfoDiscardObject(mng_info,i);
                }
              }
            if (length)
              chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_MOVE,4) == 0)
          {
            unsigned long
              first_object,
              last_object;

            /*
              read MOVE
            */
            first_object=(p[0] << 8) | p[1];
            last_object=(p[2] << 8) | p[3];
            for (i=(long) first_object; i <= (long) last_object; i++)
            {
              if (mng_info->exists[i] && !mng_info->frozen[i])
                {
                  MngPair
                    new_pair;

                  MngPair
                    old_pair;

                  old_pair.a=mng_info->x_off[i];
                  old_pair.b=mng_info->y_off[i];
                  new_pair=mng_read_pair(old_pair,(int) p[4],&p[5]);
                  mng_info->x_off[i]=new_pair.a;
                  mng_info->y_off[i]=new_pair.b;
                }
            }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }

        if (memcmp(type,mng_LOOP,4) == 0)
          {
            long loop_iters=1;
            loop_level=chunk[0];
            mng_info->loop_active[loop_level]=1;  /* mark loop active */
            /*
              Record starting point.
            */
            loop_iters=mng_get_long(&chunk[1]);
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  LOOP level %ld  has %ld iterations ",loop_level,loop_iters);
            if (loop_iters == 0)
              skipping_loop=loop_level;
            else
              {
                mng_info->loop_jump[loop_level]=TellBlob(image);
                mng_info->loop_count[loop_level]=loop_iters;
              }
            mng_info->loop_iteration[loop_level]=0;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_ENDL,4) == 0)
          {
            loop_level=chunk[0];
            if (skipping_loop > 0)
              {
                if (skipping_loop == loop_level)
                  {
                    /*
                      Found end of zero-iteration loop.
                    */
                    skipping_loop=(-1);
                    mng_info->loop_active[loop_level]=0;
                  }
              }
            else
              {
                if (mng_info->loop_active[loop_level] == 1)
                  {
                    mng_info->loop_count[loop_level]--;
                    mng_info->loop_iteration[loop_level]++;
                    if (logging != MagickFalse)
                      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "  ENDL: LOOP level %ld  has %ld remaining iterations ",
                        loop_level,mng_info->loop_count[loop_level]);
                    if (mng_info->loop_count[loop_level] != 0)
                      {
                        offset=SeekBlob(image,mng_info->loop_jump[loop_level],
                          SEEK_SET);
                        if (offset < 0)
                          ThrowReaderException(CorruptImageError,
                            "ImproperImageHeader");
                      }
                    else
                      {
                        short
                          last_level;

                        /*
                          Finished loop.
                        */
                        mng_info->loop_active[loop_level]=0;
                        last_level=(-1);
                        for (i=0; i < loop_level; i++)
                          if (mng_info->loop_active[i] == 1)
                            last_level=(short) i;
                        loop_level=last_level;
                      }
                  }
              }
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_CLON,4) == 0)
          {
            if (mng_info->clon_warning == 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"CLON is not implemented yet","`%s'",
                image->filename);
            mng_info->clon_warning++;
          }
        if (memcmp(type,mng_MAGN,4) == 0)
          {
            png_uint_16
              magn_first,
              magn_last,
              magn_mb,
              magn_ml,
              magn_mr,
              magn_mt,
              magn_mx,
              magn_my,
              magn_methx,
              magn_methy;

            if (length > 1)
              magn_first=(p[0] << 8) | p[1];
            else
              magn_first=0;
            if (length > 3)
              magn_last=(p[2] << 8) | p[3];
            else
              magn_last=magn_first;
#ifndef MNG_OBJECT_BUFFERS
            if (magn_first || magn_last)
              if (mng_info->magn_warning == 0)
                {
                  (void) ThrowMagickException(&image->exception,
                     GetMagickModule(),CoderError,
                     "MAGN is not implemented yet for nonzero objects",
                     "`%s'",image->filename);
                   mng_info->magn_warning++;
                }
#endif
            if (length > 4)
              magn_methx=p[4];
            else
              magn_methx=0;

            if (length > 6)
              magn_mx=(p[5] << 8) | p[6];
            else
              magn_mx=1;
            if (magn_mx == 0)
              magn_mx=1;

            if (length > 8)
              magn_my=(p[7] << 8) | p[8];
            else
              magn_my=magn_mx;
            if (magn_my == 0)
              magn_my=1;

            if (length > 10)
              magn_ml=(p[9] << 8) | p[10];
            else
              magn_ml=magn_mx;
            if (magn_ml == 0)
              magn_ml=1;

            if (length > 12)
              magn_mr=(p[11] << 8) | p[12];
            else
              magn_mr=magn_mx;
            if (magn_mr == 0)
              magn_mr=1;

            if (length > 14)
              magn_mt=(p[13] << 8) | p[14];
            else
              magn_mt=magn_my;
            if (magn_mt == 0)
              magn_mt=1;

            if (length > 16)
              magn_mb=(p[15] << 8) | p[16];
            else
              magn_mb=magn_my;
            if (magn_mb == 0)
              magn_mb=1;

            if (length > 17)
              magn_methy=p[17];
            else
              magn_methy=magn_methx;

            if (magn_methx > 5 || magn_methy > 5)
              if (mng_info->magn_warning == 0)
                {
                  (void) ThrowMagickException(&image->exception,
                     GetMagickModule(),CoderError,
                     "Unknown MAGN method in MNG datastream","`%s'",
                     image->filename);
                   mng_info->magn_warning++;
                }
#ifdef MNG_OBJECT_BUFFERS
          /* Magnify existing objects in the range magn_first to magn_last */
#endif
            if (magn_first == 0 || magn_last == 0)
              {
                /* Save the magnification factors for object 0 */
                mng_info->magn_mb=magn_mb;
                mng_info->magn_ml=magn_ml;
                mng_info->magn_mr=magn_mr;
                mng_info->magn_mt=magn_mt;
                mng_info->magn_mx=magn_mx;
                mng_info->magn_my=magn_my;
                mng_info->magn_methx=magn_methx;
                mng_info->magn_methy=magn_methy;
              }
          }
        if (memcmp(type,mng_PAST,4) == 0)
          {
            if (mng_info->past_warning == 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"PAST is not implemented yet","`%s'",
                image->filename);
            mng_info->past_warning++;
          }
        if (memcmp(type,mng_SHOW,4) == 0)
          {
            if (mng_info->show_warning == 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"SHOW is not implemented yet","`%s'",
                image->filename);
            mng_info->show_warning++;
          }
        if (memcmp(type,mng_sBIT,4) == 0)
          {
            if (length < 4)
              mng_info->have_global_sbit=MagickFalse;
            else
              {
                mng_info->global_sbit.gray=p[0];
                mng_info->global_sbit.red=p[0];
                mng_info->global_sbit.green=p[1];
                mng_info->global_sbit.blue=p[2];
                mng_info->global_sbit.alpha=p[3];
                mng_info->have_global_sbit=MagickTrue;
             }
          }
        if (memcmp(type,mng_pHYs,4) == 0)
          {
            if (length > 8)
              {
                mng_info->global_x_pixels_per_unit=
                    (unsigned long) mng_get_long(p);
                mng_info->global_y_pixels_per_unit=
                    (unsigned long) mng_get_long(&p[4]);
                mng_info->global_phys_unit_type=p[8];
                mng_info->have_global_phys=MagickTrue;
              }
            else
              mng_info->have_global_phys=MagickFalse;
          }
        if (memcmp(type,mng_pHYg,4) == 0)
          {
            if (mng_info->phyg_warning == 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"pHYg is not implemented.","`%s'",image->filename);
            mng_info->phyg_warning++;
          }
        if (memcmp(type,mng_BASI,4) == 0)
          {
            skip_to_iend=MagickTrue;
            if (mng_info->basi_warning == 0)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                CoderError,"BASI is not implemented yet","`%s'",
                image->filename);
            mng_info->basi_warning++;
#ifdef MNG_BASI_SUPPORTED
            basi_width=(unsigned long) ((p[0] << 24) | (p[1] << 16) |
               (p[2] << 8) | p[3]);
            basi_height=(unsigned long) ((p[4] << 24) | (p[5] << 16) |
               (p[6] << 8) | p[7]);
            basi_color_type=p[8];
            basi_compression_method=p[9];
            basi_filter_type=p[10];
            basi_interlace_method=p[11];
            if (length > 11)
              basi_red=(p[12] << 8) & p[13];
            else
              basi_red=0;
            if (length > 13)
              basi_green=(p[14] << 8) & p[15];
            else
              basi_green=0;
            if (length > 15)
              basi_blue=(p[16] << 8) & p[17];
            else
              basi_blue=0;
            if (length > 17)
              basi_alpha=(p[18] << 8) & p[19];
            else
              {
                if (basi_sample_depth == 16)
                  basi_alpha=65535L;
                else
                  basi_alpha=255;
              }
            if (length > 19)
              basi_viewable=p[20];
            else
              basi_viewable=0;
#endif
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
        if (memcmp(type,mng_IHDR,4)
#if defined(JNG_SUPPORTED)
            && memcmp(type,mng_JHDR,4)
#endif
            )
          {
            /* Not an IHDR or JHDR chunk */
            if (length)
              chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
/* Process IHDR */
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Processing %c%c%c%c chunk",type[0],type[1],type[2],type[3]);
        mng_info->exists[object_id]=MagickTrue;
        mng_info->viewable[object_id]=MagickTrue;
        if (mng_info->invisible[object_id])
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Skipping invisible object");
            skip_to_iend=MagickTrue;
            chunk=(unsigned char *) RelinquishMagickMemory(chunk);
            continue;
          }
#if defined(MNG_INSERT_LAYERS)
        if (length < 8)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        image_width=(unsigned  long) mng_get_long(p);
        image_height=(unsigned  long) mng_get_long(&p[4]);
#endif
        chunk=(unsigned char *) RelinquishMagickMemory(chunk);

        /*
          Insert a transparent background layer behind the entire animation
          if it is not full screen.
        */
#if defined(MNG_INSERT_LAYERS)
        if (insert_layers && mng_type && first_mng_object)
          {
            if ((mng_info->clip.left > 0) || (mng_info->clip.top > 0) ||
                (image_width < mng_info->mng_width) ||
                (mng_info->clip.right < (long) mng_info->mng_width) ||
                (image_height < mng_info->mng_height) ||
                (mng_info->clip.bottom < (long) mng_info->mng_height))
              {
                if (GetAuthenticPixelQueue(image) != (PixelPacket *) NULL)
                  {
                    /*
                      Allocate next image structure.
                    */
                    AcquireNextImage(image_info,image);
                    if (GetNextImageInList(image) == (Image *) NULL)
                      {
                        image=DestroyImageList(image);
                        MngInfoFreeStruct(mng_info,&have_mng_structure);
                        return((Image *) NULL);
                      }
                    image=SyncNextImageInList(image);
                  }
                mng_info->image=image;
                if (term_chunk_found)
                  {
                    image->start_loop=MagickTrue;
                    image->iterations=mng_iterations;
                    term_chunk_found=MagickFalse;
                  }
                else
                    image->start_loop=MagickFalse;
                /*
                  Make a background rectangle.
                */
                image->delay=0;
                image->columns=mng_info->mng_width;
                image->rows=mng_info->mng_height;
                image->page.width=mng_info->mng_width;
                image->page.height=mng_info->mng_height;
                image->page.x=0;
                image->page.y=0;
                image->background_color=mng_background_color;
                (void) SetImageBackgroundColor(image);
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Inserted transparent background layer, W=%lud, H=%lud",
                    mng_info->mng_width,mng_info->mng_height);
              }
          }
        /*
          Insert a background layer behind the upcoming image if
          framing_mode is 3, and we haven't already inserted one.
        */
        if (insert_layers && (mng_info->framing_mode == 3) &&
                (subframe_width) && (subframe_height) && (simplicity == 0 ||
                (simplicity & 0x08)))
          {
            if (GetAuthenticPixelQueue(image) != (PixelPacket *) NULL)
            {
              /*
                Allocate next image structure.
              */
              AcquireNextImage(image_info,image);
              if (GetNextImageInList(image) == (Image *) NULL)
                {
                  image=DestroyImageList(image);
                  MngInfoFreeStruct(mng_info,&have_mng_structure);
                  return((Image *) NULL);
                }
              image=SyncNextImageInList(image);
            }
            mng_info->image=image;
            if (term_chunk_found)
              {
                image->start_loop=MagickTrue;
                image->iterations=mng_iterations;
                term_chunk_found=MagickFalse;
              }
            else
                image->start_loop=MagickFalse;
            image->delay=0;
            image->columns=subframe_width;
            image->rows=subframe_height;
            image->page.width=subframe_width;
            image->page.height=subframe_height;
            image->page.x=mng_info->clip.left;
            image->page.y=mng_info->clip.top;
            image->background_color=mng_background_color;
            image->matte=MagickFalse;
            (void) SetImageBackgroundColor(image);
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Inserted background layer, L=%ld, R=%ld, T=%ld, B=%ld",
                mng_info->clip.left,mng_info->clip.right,
                mng_info->clip.top,mng_info->clip.bottom);
          }
#endif /* MNG_INSERT_LAYERS */
        first_mng_object=MagickFalse;
        if (GetAuthenticPixelQueue(image) != (PixelPacket *) NULL)
          {
            /*
              Allocate next image structure.
            */
            AcquireNextImage(image_info,image);
            if (GetNextImageInList(image) == (Image *) NULL)
              {
                image=DestroyImageList(image);
                MngInfoFreeStruct(mng_info,&have_mng_structure);
                return((Image *) NULL);
              }
            image=SyncNextImageInList(image);
          }
        mng_info->image=image;
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
        if (term_chunk_found)
          {
            image->start_loop=MagickTrue;
            term_chunk_found=MagickFalse;
          }
        else
            image->start_loop=MagickFalse;
        if (mng_info->framing_mode == 1 || mng_info->framing_mode == 3)
          {
            image->delay=frame_delay;
            frame_delay=default_frame_delay;
          }
        else
          image->delay=0;
        image->page.width=mng_info->mng_width;
        image->page.height=mng_info->mng_height;
        image->page.x=mng_info->x_off[object_id];
        image->page.y=mng_info->y_off[object_id];
        image->iterations=mng_iterations;
        /*
          Seek back to the beginning of the IHDR or JHDR chunk's length field.
        */
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Seeking back to beginning of %c%c%c%c chunk",type[0],type[1],
            type[2],type[3]);
        offset=SeekBlob(image,-((long) length+12),SEEK_CUR);
        if (offset < 0)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }

    previous=image;
    mng_info->image=image;
    mng_info->mng_type=mng_type;
    mng_info->object_id=object_id;

    if (memcmp(type,mng_IHDR,4) == 0)
      image=ReadOnePNGImage(mng_info,image_info,exception);
#if defined(JNG_SUPPORTED)
    else
      image=ReadOneJNGImage(mng_info,image_info,exception);
#endif

    if (image == (Image *) NULL)
      {
        if (IsImageObject(previous) != MagickFalse)
          {
            (void) DestroyImageList(previous);
            (void) CloseBlob(previous);
          }
        MngInfoFreeStruct(mng_info,&have_mng_structure);
        return((Image *) NULL);
      }
    if (image->columns == 0 || image->rows == 0)
      {
        (void) CloseBlob(image);
        image=DestroyImageList(image);
        MngInfoFreeStruct(mng_info,&have_mng_structure);
        return((Image *) NULL);
      }
    mng_info->image=image;

    if (mng_type)
      {
        MngBox
          crop_box;

        if (mng_info->magn_methx || mng_info->magn_methy)
          {
            png_uint_32
               magnified_height,
               magnified_width;

            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Processing MNG MAGN chunk");

            if (mng_info->magn_methx == 1)
              {
                magnified_width=mng_info->magn_ml;
                if (image->columns > 1)
                   magnified_width += mng_info->magn_mr;
                if (image->columns > 2)
                   magnified_width += (image->columns-2)*(mng_info->magn_mx);
              }
            else
              {
                magnified_width=image->columns;
                if (image->columns > 1)
                   magnified_width += mng_info->magn_ml-1;
                if (image->columns > 2)
                   magnified_width += mng_info->magn_mr-1;
                if (image->columns > 3)
                   magnified_width += (image->columns-3)*(mng_info->magn_mx-1);
              }
            if (mng_info->magn_methy == 1)
              {
                magnified_height=mng_info->magn_mt;
                if (image->rows > 1)
                   magnified_height += mng_info->magn_mb;
                if (image->rows > 2)
                   magnified_height += (image->rows-2)*(mng_info->magn_my);
              }
            else
              {
                magnified_height=image->rows;
                if (image->rows > 1)
                   magnified_height += mng_info->magn_mt-1;
                if (image->rows > 2)
                   magnified_height += mng_info->magn_mb-1;
                if (image->rows > 3)
                   magnified_height += (image->rows-3)*(mng_info->magn_my-1);
              }
            if (magnified_height > image->rows ||
                magnified_width > image->columns)
              {
                Image
                  *large_image;

                int
                  yy;

                long
                  m,
                  y;

                register long
                  x;

                register PixelPacket
                  *n,
                  *q;

                PixelPacket
                  *next,
                  *prev;

                png_uint_16
                  magn_methx,
                  magn_methy;

                /*
                  Allocate next image structure.
                */
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Allocate magnified image");
                AcquireNextImage(image_info,image);
                if (GetNextImageInList(image) == (Image *) NULL)
                  {
                    image=DestroyImageList(image);
                    MngInfoFreeStruct(mng_info,&have_mng_structure);
                    return((Image *) NULL);
                  }

                large_image=SyncNextImageInList(image);

                large_image->columns=magnified_width;
                large_image->rows=magnified_height;

                magn_methx=mng_info->magn_methx;
                magn_methy=mng_info->magn_methy;

#if (MAGICKCORE_QUANTUM_DEPTH == 32)
#define QM unsigned short
                if (magn_methx != 1 || magn_methy != 1)
                  {
                  /*
                     Scale pixels to unsigned shorts to prevent
                     overflow of intermediate values of interpolations
                  */
                     for (y=0; y < (long) image->rows; y++)
                     {
                       q=GetAuthenticPixels(image,0,y,image->columns,1,
                          exception);
                       for (x=(long) image->columns-1; x >= 0; x--)
                       {
                          q->red=ScaleQuantumToShort(q->red);
                          q->green=ScaleQuantumToShort(q->green);
                          q->blue=ScaleQuantumToShort(q->blue);
                          q->opacity=ScaleQuantumToShort(q->opacity);
                          q++;
                       }
                       if (SyncAuthenticPixels(image,exception) == MagickFalse)
                         break;
                     }
                  }
#else
#define QM Quantum
#endif

                if (image->matte != MagickFalse)
                   (void) SetImageBackgroundColor(large_image);
                else
                  {
                    large_image->background_color.opacity=OpaqueOpacity;
                    (void) SetImageBackgroundColor(large_image);
                    if (magn_methx == 4)
                      magn_methx=2;
                    if (magn_methx == 5)
                      magn_methx=3;
                    if (magn_methy == 4)
                      magn_methy=2;
                    if (magn_methy == 5)
                      magn_methy=3;
                  }

                /* magnify the rows into the right side of the large image */

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Magnify the rows to %lu",large_image->rows);
                m=(long) mng_info->magn_mt;
                yy=0;
                length=(size_t) image->columns;
                next=(PixelPacket *) AcquireQuantumMemory(length,sizeof(*next));
                prev=(PixelPacket *) AcquireQuantumMemory(length,sizeof(*prev));
                if ((prev == (PixelPacket *) NULL) ||
                    (next == (PixelPacket *) NULL))
                  {
                     image=DestroyImageList(image);
                     MngInfoFreeStruct(mng_info,&have_mng_structure);
                     ThrowReaderException(ResourceLimitError,
                       "MemoryAllocationFailed");
                  }
                n=GetAuthenticPixels(image,0,0,image->columns,1,exception);
                (void) CopyMagickMemory(next,n,length);
                for (y=0; y < (long) image->rows; y++)
                {
                  if (y == 0)
                    m=(long) mng_info->magn_mt;
                  else if (magn_methy > 1 && y == (long) image->rows-2)
                    m=(long) mng_info->magn_mb;
                  else if (magn_methy <= 1 && y == (long) image->rows-1)
                    m=(long) mng_info->magn_mb;
                  else if (magn_methy > 1 && y == (long) image->rows-1)
                    m=1;
                  else
                    m=(long) mng_info->magn_my;
                  n=prev;
                  prev=next;
                  next=n;
                  if (y < (long) image->rows-1)
                    {
                      n=GetAuthenticPixels(image,0,y+1,image->columns,1,
                          exception);
                      (void) CopyMagickMemory(next,n,length);
                    }
                  for (i=0; i < m; i++, yy++)
                  {
                    register PixelPacket
                      *pixels;

                    assert(yy < (long) large_image->rows);
                    pixels=prev;
                    n=next;
                    q=GetAuthenticPixels(large_image,0,yy,large_image->columns,
                          1,exception);
                    q+=(large_image->columns-image->columns);
                    for (x=(long) image->columns-1; x >= 0; x--)
                    {
                      /* TO DO: get color as function of indices[x] */
                      /*
                      if (image->storage_class == PseudoClass)
                        {
                        }
                      */

                      if (magn_methy <= 1)
                        {
                          *q=(*pixels); /* replicate previous */
                        }
                      else if (magn_methy == 2 || magn_methy == 4)
                        {
                          if (i == 0)
                             *q=(*pixels);
                          else
                            {
                              /* Interpolate */
                              (*q).red=(QM) (((long) (2*i*((*n).red
                                 -(*pixels).red)+m))/((long) (m*2))
                                 +(*pixels).red);
                              (*q).green=(QM) (((long) (2*i*((*n).green
                                 -(*pixels).green)+m))/((long) (m*2))
                                 +(*pixels).green);
                              (*q).blue=(QM) (((long) (2*i*((*n).blue
                                 -(*pixels).blue)+m))/((long) (m*2))
                                 +(*pixels).blue);
                              if (image->matte != MagickFalse)
                                 (*q).opacity=(QM) (((long)
                                 (2*i*((*n).opacity
                                 -(*pixels).opacity)+m))
                                 /((long) (m*2))+(*pixels).opacity);
                            }
                          if (magn_methy == 4)
                            {
                              /* Replicate nearest */
                              if (i <= ((m+1) << 1))
                                 (*q).opacity=(*pixels).opacity+0;
                              else
                                 (*q).opacity=(*n).opacity+0;
                            }
                        }
                      else /* if (magn_methy == 3 || magn_methy == 5) */
                        {
                          /* Replicate nearest */
                          if (i <= ((m+1) << 1))
                             *q=(*pixels);
                          else
                             *q=(*n);
                          if (magn_methy == 5)
                            {
                              (*q).opacity=(QM) (((long) (2*i*((*n).opacity
                                 -(*pixels).opacity)+m))/((long) (m*2))
                                 +(*pixels).opacity);
                            }
                        }
                      n++;
                      q++;
                      pixels++;
                    } /* x */
                    if (SyncAuthenticPixels(large_image,exception) == 0)
                      break;
                  } /* i */
                } /* y */
                prev=(PixelPacket *) RelinquishMagickMemory(prev);
                next=(PixelPacket *) RelinquishMagickMemory(next);

                length=image->columns;

                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Delete original image");

                DeleteImageFromList(&image);

                image=large_image;

                mng_info->image=image;

                /* magnify the columns */
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Magnify the columns to %lu",image->columns);

                for (y=0; y < (long) image->rows; y++)
                {
                  register PixelPacket
                    *pixels;

                  q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
                  pixels=q+(image->columns-length);
                  n=pixels+1;
                  for (x=(long) (image->columns-length);
                    x < (long) image->columns; x++)
                  {
                    if (x == (long) (image->columns-length))
                      m=(long) mng_info->magn_ml;
                    else if (magn_methx > 1 && x == (long) image->columns-2)
                      m=(long) mng_info->magn_mr;
                    else if (magn_methx <= 1 && x == (long) image->columns-1)
                      m=(long) mng_info->magn_mr;
                    else if (magn_methx > 1 && x == (long) image->columns-1)
                      m=1;
                    else
                      m=(long) mng_info->magn_mx;
                    for (i=0; i < m; i++)
                    {
                      if (magn_methx <= 1)
                        {
                          /* replicate previous */
                          *q=(*pixels);
                        }
                      else if (magn_methx == 2 || magn_methx == 4)
                        {
                          if (i == 0)
                            *q=(*pixels);
                          else
                            {
                              /* Interpolate */
                              (*q).red=(QM) ((2*i*((*n).red
                                 -(*pixels).red)+m)
                                 /((long) (m*2))+(*pixels).red);
                              (*q).green=(QM) ((2*i*((*n).green
                                 -(*pixels).green)
                                 +m)/((long) (m*2))+(*pixels).green);
                              (*q).blue=(QM) ((2*i*((*n).blue
                                 -(*pixels).blue)+m)
                                 /((long) (m*2))+(*pixels).blue);
                              if (image->matte != MagickFalse)
                                 (*q).opacity=(QM) ((2*i*((*n).opacity
                                   -(*pixels).opacity)+m)/((long) (m*2))
                                   +(*pixels).opacity);
                            }
                          if (magn_methx == 4)
                            {
                              /* Replicate nearest */
                              if (i <= ((m+1) << 1))
                                 (*q).opacity=(*pixels).opacity+0;
                              else
                                 (*q).opacity=(*n).opacity+0;
                            }
                        }
                      else /* if (magn_methx == 3 || magn_methx == 5) */
                        {
                          /* Replicate nearest */
                          if (i <= ((m+1) << 1))
                             *q=(*pixels);
                          else
                             *q=(*n);
                          if (magn_methx == 5)
                            {
                              /* Interpolate */
                              (*q).opacity=(QM) ((2*i*((*n).opacity
                                 -(*pixels).opacity)+m) /((long) (m*2))
                                 +(*pixels).opacity);
                            }
                        }
                      q++;
                    }
                    n++;
                    p++;
                  }
                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    break;
                }
#if (MAGICKCORE_QUANTUM_DEPTH == 32)
              if (magn_methx != 1 || magn_methy != 1)
                {
                /*
                   Rescale pixels to Quantum
                */
                   for (y=0; y < (long) image->rows; y++)
                   {
                     q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
                     for (x=(long) image->columns-1; x >= 0; x--)
                     {
                        q->red=ScaleShortToQuantum(q->red);
                        q->green=ScaleShortToQuantum(q->green);
                        q->blue=ScaleShortToQuantum(q->blue);
                        q->opacity=ScaleShortToQuantum(q->opacity);
                        q++;
                     }
                     if (SyncAuthenticPixels(image,exception) == MagickFalse)
                       break;
                   }
                }
#endif
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Finished MAGN processing");
              }
          }

        /*
          Crop_box is with respect to the upper left corner of the MNG.
        */
        crop_box.left=mng_info->image_box.left+mng_info->x_off[object_id];
        crop_box.right=mng_info->image_box.right+mng_info->x_off[object_id];
        crop_box.top=mng_info->image_box.top+mng_info->y_off[object_id];
        crop_box.bottom=mng_info->image_box.bottom+mng_info->y_off[object_id];
        crop_box=mng_minimum_box(crop_box,mng_info->clip);
        crop_box=mng_minimum_box(crop_box,mng_info->frame);
        crop_box=mng_minimum_box(crop_box,mng_info->object_clip[object_id]);
        if ((crop_box.left != (mng_info->image_box.left
            +mng_info->x_off[object_id])) ||
            (crop_box.right != (mng_info->image_box.right
            +mng_info->x_off[object_id])) ||
            (crop_box.top != (mng_info->image_box.top
            +mng_info->y_off[object_id])) ||
            (crop_box.bottom != (mng_info->image_box.bottom
            +mng_info->y_off[object_id])))
          {
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Crop the PNG image");
            if ((crop_box.left < crop_box.right) &&
                (crop_box.top < crop_box.bottom))
              {
                Image
                  *im;

                RectangleInfo
                  crop_info;

                /*
                  Crop_info is with respect to the upper left corner of
                  the image.
                */
                crop_info.x=(crop_box.left-mng_info->x_off[object_id]);
                crop_info.y=(crop_box.top-mng_info->y_off[object_id]);
                crop_info.width=(unsigned long) (crop_box.right-crop_box.left);
                crop_info.height=(unsigned long) (crop_box.bottom-crop_box.top);
                image->page.width=image->columns;
                image->page.height=image->rows;
                image->page.x=0;
                image->page.y=0;
                im=CropImage(image,&crop_info,exception);
                if (im != (Image *) NULL)
                  {
                    image->columns=im->columns;
                    image->rows=im->rows;
                    im=DestroyImage(im);
                    image->page.width=image->columns;
                    image->page.height=image->rows;
                    image->page.x=crop_box.left;
                    image->page.y=crop_box.top;
                  }
              }
            else
              {
                /*
                  No pixels in crop area.  The MNG spec still requires
                  a layer, though, so make a single transparent pixel in
                  the top left corner.
                */
                image->columns=1;
                image->rows=1;
                image->colors=2;
                (void) SetImageBackgroundColor(image);
                image->page.width=1;
                image->page.height=1;
                image->page.x=0;
                image->page.y=0;
              }
          }
#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
        image=mng_info->image;
#endif
      }

#if (MAGICKCORE_QUANTUM_DEPTH == 16)  /* TO DO: treat Q:32 */
    /* Determine if bit depth can be reduced from 16 to 8.
     * Note that the method GetImageDepth doesn't check background
     * and doesn't handle PseudoClass specially.  Also it uses
     * multiplication and division by 257 instead of shifting, so
     * might be slower.
     */
    if (mng_info->optimize && image->depth == 16)
      {
        int
          ok_to_reduce;

        const PixelPacket
          *p;

        ok_to_reduce=(((((unsigned long) image->background_color.red >> 8) &
                     0xff)
          == ((unsigned long) image->background_color.red & 0xff)) &&
           ((((unsigned long) image->background_color.green >> 8) & 0xff)
          == ((unsigned long) image->background_color.green & 0xff)) &&
           ((((unsigned long) image->background_color.blue >> 8) & 0xff)
          == ((unsigned long) image->background_color.blue & 0xff)));
        if (ok_to_reduce && image->storage_class == PseudoClass)
          {
            int indx;

            for (indx=0; indx < (long) image->colors; indx++)
              {
                ok_to_reduce=(((((unsigned long) image->colormap[indx].red >>
                    8) & 0xff)
                  == ((unsigned long) image->colormap[indx].red & 0xff)) &&
                  ((((unsigned long) image->colormap[indx].green >> 8) & 0xff)
                  == ((unsigned long) image->colormap[indx].green & 0xff)) &&
                  ((((unsigned long) image->colormap[indx].blue >> 8) & 0xff)
                  == ((unsigned long) image->colormap[indx].blue & 0xff)));
                if (ok_to_reduce == MagickFalse)
                  break;
              }
          }
        if ((ok_to_reduce != MagickFalse) &&
            (image->storage_class != PseudoClass))
          {
            long
              y;

            register long
              x;

            for (y=0; y < (long) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              for (x=(long) image->columns-1; x >= 0; x--)
              {
                ok_to_reduce=((
                  (((unsigned long) p->red >> 8) & 0xff) ==
                  ((unsigned long) p->red & 0xff)) &&
                  ((((unsigned long) p->green >> 8) & 0xff) ==
                  ((unsigned long) p->green & 0xff)) &&
                  ((((unsigned long) p->blue >> 8) & 0xff) ==
                  ((unsigned long) p->blue & 0xff)) &&
                  (((!image->matte ||
                  (((unsigned long) p->opacity >> 8) & 0xff) ==
                  ((unsigned long) p->opacity & 0xff)))));
                if (ok_to_reduce == 0)
                  break;
                p++;
              }
              if (x != 0)
                break;
            }
          }
        if (ok_to_reduce)
          {
            image->depth=8;
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Reducing PNG bit depth to 8 without loss of info");
          }
      }
#endif
      GetImageException(image,exception);
      if (image_info->number_scenes != 0)
        {
          if (mng_info->scenes_found >
             (long) (image_info->first_scene+image_info->number_scenes))
            break;
        }
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Finished reading image datastream.");
  } while (LocaleCompare(image_info->magick,"MNG") == 0);
  (void) CloseBlob(image);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Finished reading all image datastreams.");
#if defined(MNG_INSERT_LAYERS)
  if (insert_layers && !mng_info->image_found && (mng_info->mng_width) &&
       (mng_info->mng_height))
    {
      /*
        Insert a background layer if nothing else was found.
      */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  No images found.  Inserting a background layer.");
      if (GetAuthenticPixelQueue(image) != (PixelPacket *) NULL)
        {
          /*
            Allocate next image structure.
          */
          AcquireNextImage(image_info,image);
          if (GetNextImageInList(image) == (Image *) NULL)
            {
              image=DestroyImageList(image);
              MngInfoFreeStruct(mng_info,&have_mng_structure);
              if (logging != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Allocation failed, returning NULL.");
              return((Image *) NULL);
            }
          image=SyncNextImageInList(image);
        }
      image->columns=mng_info->mng_width;
      image->rows=mng_info->mng_height;
      image->page.width=mng_info->mng_width;
      image->page.height=mng_info->mng_height;
      image->page.x=0;
      image->page.y=0;
      image->background_color=mng_background_color;
      image->matte=MagickFalse;
      if (image_info->ping == MagickFalse)
        (void) SetImageBackgroundColor(image);
      mng_info->image_found++;
    }
#endif
  image->iterations=mng_iterations;
  if (mng_iterations == 1)
    image->start_loop=MagickTrue;
  while (GetPreviousImageInList(image) != (Image *) NULL)
  {
    image_count++;
    if (image_count > 10*mng_info->image_found)
      {
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  No beginning");
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          CoderError,"Linked list is corrupted, beginning of list not found",
          "`%s'",image_info->filename);
        return((Image *) NULL);
      }
    image=GetPreviousImageInList(image);
    if (GetNextImageInList(image) == (Image *) NULL)
      {
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  Corrupt list");
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          CoderError,"Linked list is corrupted; next_image is NULL","`%s'",
          image_info->filename);
      }
  }
  if (mng_info->ticks_per_second && mng_info->image_found > 1 &&
             GetNextImageInList(image) ==
     (Image *) NULL)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  First image null");
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        CoderError,"image->next for first image is NULL but shouldn't be.",
        "`%s'",image_info->filename);
    }
  if (mng_info->image_found == 0)
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  No visible images found.");
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        CoderError,"No visible images in file","`%s'",image_info->filename);
      if (image != (Image *) NULL)
        image=DestroyImageList(image);
      MngInfoFreeStruct(mng_info,&have_mng_structure);
      return((Image *) NULL);
    }

  if (mng_info->ticks_per_second)
    final_delay=1UL*MagickMax(image->ticks_per_second,1L)*
            final_delay/mng_info->ticks_per_second;
  else
    image->start_loop=MagickTrue;
  /* Find final nonzero image delay */
  final_image_delay=0;
  while (GetNextImageInList(image) != (Image *) NULL)
    {
      if (image->delay)
        final_image_delay=image->delay;
      image=GetNextImageInList(image);
    }
  if (final_delay < final_image_delay)
    final_delay=final_image_delay;
  image->delay=final_delay;
  if (logging != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  image->delay=%lu, final_delay=%lu",image->delay,final_delay);
  if (logging != MagickFalse)
    {
      int
        scene;

      scene=0;
      image=GetFirstImageInList(image);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Before coalesce:");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    scene 0 delay=%lu",image->delay);
      while (GetNextImageInList(image) != (Image *) NULL)
      {
        image=GetNextImageInList(image);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    scene %d delay=%lu",++scene,image->delay);
      }
    }

  image=GetFirstImageInList(image);
#ifdef MNG_COALESCE_LAYERS
  if (insert_layers)
    {
      Image
        *next_image,
        *next;

      unsigned long
        scene;

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  Coalesce Images");
      scene=image->scene;
      next_image=CoalesceImages(image,&image->exception);
      if (next_image == (Image *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      image=DestroyImageList(image);
      image=next_image;
      for (next=image; next != (Image *) NULL; next=next_image)
      {
         next->page.width=mng_info->mng_width;
         next->page.height=mng_info->mng_height;
         next->page.x=0;
         next->page.y=0;
         next->scene=scene++;
         next_image=GetNextImageInList(next);
         if (next_image == (Image *) NULL)
           break;
         if (next->delay == 0)
           {
             scene--;
             next_image->previous=GetPreviousImageInList(next);
             if (GetPreviousImageInList(next) == (Image *) NULL)
               image=next_image;
             else
               next->previous->next=next_image;
             next=DestroyImage(next);
           }
      }
    }
#endif

  while (GetNextImageInList(image) != (Image *) NULL)
      image=GetNextImageInList(image);
  image->dispose=BackgroundDispose;

  if (logging != MagickFalse)
    {
      int
        scene;

      scene=0;
      image=GetFirstImageInList(image);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  After coalesce:");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    scene 0 delay=%lu dispose=%d",image->delay,(int) image->dispose);
      while (GetNextImageInList(image) != (Image *) NULL)
        {
          image=GetNextImageInList(image);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    scene %d delay=%lu dispose=%d",++scene,
            image->delay,(int) image->dispose);
        }
    }
  image=GetFirstImageInList(image);
  MngInfoFreeStruct(mng_info,&have_mng_structure);
  have_mng_structure=MagickFalse;
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit ReadMNGImage()");
  return(GetFirstImageInList(image));
}
#else /* PNG_LIBPNG_VER > 95 */
static Image *ReadPNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  printf("Your PNG library is too old: You have libpng-%s\n",
     PNG_LIBPNG_VER_STRING);
  (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
    "PNG library is too old","`%s'",image_info->filename);
  return(Image *) NULL;
}
static Image *ReadMNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  return(ReadPNGImage(image_info,exception));
}
#endif /* PNG_LIBPNG_VER > 95 */
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P N G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPNGImage() adds properties for the PNG image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPNGImage method is:
%
%      unsigned long RegisterPNGImage(void)
%
*/
ModuleExport unsigned long RegisterPNGImage(void)
{
  char
    version[MaxTextExtent];

  MagickInfo
    *entry;

  static const char
    *PNGNote=
    {
      "See http://www.libpng.org/ for details about the PNG format."
    },
    *JNGNote=
    {
      "See http://www.libpng.org/pub/mng/ for details about the JNG\n"
      "format."
    },
    *MNGNote=
    {
      "See http://www.libpng.org/pub/mng/ for details about the MNG\n"
      "format."
    };

  *version='\0';
#if defined(PNG_LIBPNG_VER_STRING)
  (void) ConcatenateMagickString(version,"libpng ",MaxTextExtent);
  (void) ConcatenateMagickString(version,PNG_LIBPNG_VER_STRING,MaxTextExtent);
#if (PNG_LIBPNG_VER > 10005)
  if (LocaleCompare(PNG_LIBPNG_VER_STRING,png_get_header_ver(NULL)) != 0)
    {
      (void) ConcatenateMagickString(version,",",MaxTextExtent);
      (void) ConcatenateMagickString(version,png_get_libpng_ver(NULL),
            MaxTextExtent);
    }
#endif
#endif
  entry=SetMagickInfo("MNG");
  entry->seekable_stream=MagickTrue;  /* To do: eliminate this. */
#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadMNGImage;
  entry->encoder=(EncodeImageHandler *) WriteMNGImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsMNG;
  entry->description=ConstantString("Multiple-image Network Graphics");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("PNG");
  entry->note=ConstantString(MNGNote);
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("PNG");
#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Portable Network Graphics");
  entry->module=ConstantString("PNG");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->note=ConstantString(PNGNote);
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("PNG8");
#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString(
            "8-bit indexed with optional binary transparency");
  entry->module=ConstantString("PNG");
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("PNG24");
  *version='\0';
#if defined(ZLIB_VERSION)
  (void) ConcatenateMagickString(version,"zlib ",MaxTextExtent);
  (void) ConcatenateMagickString(version,ZLIB_VERSION,MaxTextExtent);
  if (LocaleCompare(ZLIB_VERSION,zlib_version) != 0)
    {
      (void) ConcatenateMagickString(version,",",MaxTextExtent);
      (void) ConcatenateMagickString(version,zlib_version,MaxTextExtent);
    }
#endif
  if (*version != '\0')
    entry->version=ConstantString(version);
#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("opaque 24-bit RGB");
  entry->module=ConstantString("PNG");
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("PNG32");
#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadPNGImage;
  entry->encoder=(EncodeImageHandler *) WritePNGImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("opaque or transparent 32-bit RGBA");
  entry->module=ConstantString("PNG");
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("JNG");
#if defined(JNG_SUPPORTED)
#if defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJNGImage;
  entry->encoder=(EncodeImageHandler *) WriteJNGImage;
#endif
#endif
  entry->magick=(IsImageFormatHandler *) IsJNG;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("JPEG Network Graphics");
  entry->module=ConstantString("PNG");
  entry->note=ConstantString(JNGNote);
  (void) RegisterMagickInfo(entry);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
  png_semaphore=AllocateSemaphoreInfo();
#endif
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P N G I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPNGImage() removes format registrations made by the
%  PNG module from the list of supported formats.
%
%  The format of the UnregisterPNGImage method is:
%
%      UnregisterPNGImage(void)
%
*/
ModuleExport void UnregisterPNGImage(void)
{
  (void) UnregisterMagickInfo("MNG");
  (void) UnregisterMagickInfo("PNG");
  (void) UnregisterMagickInfo("PNG8");
  (void) UnregisterMagickInfo("PNG24");
  (void) UnregisterMagickInfo("PNG32");
  (void) UnregisterMagickInfo("JNG");
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
  if (png_semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&png_semaphore);
#endif
}

#if defined(MAGICKCORE_PNG_DELEGATE)
#if PNG_LIBPNG_VER > 95
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMNGImage() writes an image in the Portable Network Graphics
%  Group's "Multiple-image Network Graphics" encoded image format.
%
%  MNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the WriteMNGImage method is:
%
%      MagickBooleanType WriteMNGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%
%  To do (as of version 5.5.2, November 26, 2002 -- glennrp -- see also
%    "To do" under ReadPNGImage):
%
%    Fix problem with palette sorting (when PNG_SORT_PALETTE is enabled,
%    some GIF animations don't convert properly)
%
%    Preserve all unknown and not-yet-handled known chunks found in input
%    PNG file and copy them  into output PNG files according to the PNG
%    copying rules.
%
%    Write the iCCP chunk at MNG level when (icc profile length > 0)
%
%    Improve selection of color type (use indexed-colour or indexed-colour
%    with tRNS when 256 or fewer unique RGBA values are present).
%
%    Figure out what to do with "dispose=<restore-to-previous>" (dispose == 3)
%    This will be complicated if we limit ourselves to generating MNG-LC
%    files.  For now we ignore disposal method 3 and simply overlay the next
%    image on it.
%
%    Check for identical PLTE's or PLTE/tRNS combinations and use a
%    global MNG PLTE or PLTE/tRNS combination when appropriate.
%    [mostly done 15 June 1999 but still need to take care of tRNS]
%
%    Check for identical sRGB and replace with a global sRGB (and remove
%    gAMA/cHRM if sRGB is found; check for identical gAMA/cHRM and
%    replace with global gAMA/cHRM (or with sRGB if appropriate; replace
%    local gAMA/cHRM with local sRGB if appropriate).
%
%    Check for identical sBIT chunks and write global ones.
%
%    Provide option to skip writing the signature tEXt chunks.
%
%    Use signatures to detect identical objects and reuse the first
%    instance of such objects instead of writing duplicate objects.
%
%    Use a smaller-than-32k value of compression window size when
%    appropriate.
%
%    Encode JNG datastreams.  Mostly done as of 5.5.2; need to write
%    ancillary text chunks and save profiles.
%
%    Provide an option to force LC files (to ensure exact framing rate)
%    instead of VLC.
%
%    Provide an option to force VLC files instead of LC, even when offsets
%    are present.  This will involve expanding the embedded images with a
%    transparent region at the top and/or left.
*/

#if (PNG_LIBPNG_VER > 99 && PNG_LIBPNG_VER < 10007)
/* This function became available in libpng version 1.0.6g. */
static void
png_set_compression_buffer_size(png_structp png_ptr, png_uint_32 size)
{
    if (png_ptr->zbuf)
       png_free(png_ptr, png_ptr->zbuf); png_ptr->zbuf=NULL;
    png_ptr->zbuf_size=(png_size_t) size;
    png_ptr->zbuf=(png_bytep) png_malloc(png_ptr, size);
    if (png_ptr->zbuf == 0)
       png_error(png_ptr,"Unable to allocate zbuf");
}
#endif

static void
png_write_raw_profile(const ImageInfo *image_info,png_struct *ping,
   png_info *ping_info, unsigned char *profile_type, unsigned char
   *profile_description, unsigned char *profile_data, png_uint_32 length)
{
#if (PNG_LIBPNG_VER > 10005)
   png_textp
     text;

   register long
     i;

   unsigned char
     *sp;

   png_charp
     dp;

   png_uint_32
     allocated_length,
     description_length;

   unsigned char
     hex[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
#endif

#if (PNG_LIBPNG_VER <= 10005)
   if (image_info->verbose)
     (void) printf("Not ");
   image_info=image_info;
   ping=ping;
   ping_info=ping_info;
   profile_type=profile_type;
   profile_description=profile_description;
   profile_data=profile_data;
   length=length;
#endif
   if (LocaleNCompare((char *) profile_type+1, "ng-chunk-",9) == 0)
      return;

   if (image_info->verbose)
     {
     (void) printf("writing raw profile: type=%s, length=%lu\n",
       (char *) profile_type, length);
     }
#if (PNG_LIBPNG_VER > 10005)
   text=(png_textp) png_malloc(ping,(png_uint_32) sizeof(png_text));
   description_length=(png_uint_32) strlen((const char *) profile_description);
   allocated_length=(png_uint_32) (length*2 + (length >> 5) + 20
      + description_length);
   text[0].text=(png_charp) png_malloc(ping,allocated_length);
   text[0].key=(png_charp) png_malloc(ping, (png_uint_32) 80);
   text[0].key[0]='\0';
   (void) ConcatenateMagickString(text[0].key,
      "Raw profile type ",MaxTextExtent);
   (void) ConcatenateMagickString(text[0].key,(const char *) profile_type,62);
   sp=profile_data;
   dp=text[0].text;
   *dp++='\n';
   (void) CopyMagickString(dp,(const char *) profile_description,
     allocated_length);
   dp+=description_length;
   *dp++='\n';
   (void) FormatMagickString(dp,allocated_length-
     (png_size_t) (dp-text[0].text),"%8lu ",length);
   dp+=8;
   for (i=0; i < (long) length; i++)
   {
     if (i%36 == 0)
       *dp++='\n';
     *(dp++)=(char) hex[((*sp >> 4) & 0x0f)];
     *(dp++)=(char) hex[((*sp++ ) & 0x0f)];
   }
   *dp++='\n';
   *dp='\0';
   text[0].text_length=(png_size_t) (dp-text[0].text);
   text[0].compression=image_info->compression == NoCompression ||
     (image_info->compression == UndefinedCompression &&
     text[0].text_length < 128) ? -1 : 0;
   if (text[0].text_length <= allocated_length)
     png_set_text(ping,ping_info,text,1);
   png_free(ping,text[0].text);
   png_free(ping,text[0].key);
   png_free(ping,text);
#endif
}

static MagickBooleanType png_write_chunk_from_profile(Image *image,
   const char *string, int logging)
{
  char
    *name;

  const StringInfo
    *profile;

  unsigned char
    *data;

  png_uint_32 length;

  ResetImageProfileIterator(image);
  for (name=GetNextImageProfile(image); name != (const char *) NULL; ){
    profile=GetImageProfile(image,name);
    if (profile != (const StringInfo *) NULL)
      {
        StringInfo
          *png_profile;

        if (LocaleNCompare(name,string,11) == 0) {
          if (logging != MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "  Found %s profile",name);

       png_profile=CloneStringInfo(profile);
       data=GetStringInfoDatum(png_profile),
       length=(png_uint_32) GetStringInfoLength(png_profile);
       data[4]=data[3];
       data[3]=data[2];
       data[2]=data[1];
       data[1]=data[0];
       (void) WriteBlobMSBULong(image,length-5);  /* data length */
       (void) WriteBlob(image,length-1,data+1);
       (void) WriteBlobMSBULong(image,crc32(0,data+1,(uInt) length-1));
       png_profile=DestroyStringInfo(png_profile);
        }
      }
      name=GetNextImageProfile(image);
   }
   return(MagickTrue);
}

static MagickBooleanType WriteOnePNGImage(MngInfo *mng_info,
   const ImageInfo *image_info,Image *image)
{
/* Write one PNG image */
  char
    s[2];

  const char
    *name,
    *property,
    *value;

  const StringInfo
    *profile;


  int
    image_matte,
    num_passes,
    pass;

  png_colorp
     palette;

  png_info
    *ping_info;

  png_struct
    *ping;

  long
    y;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  register IndexPacket
    *indices;

  register long
    i,
    x;

  unsigned char
    *png_pixels;

  unsigned int
    logging,
    matte;

  volatile unsigned long
    image_colors,
    image_depth;

  unsigned long
    old_bit_depth,
    quality,
    rowbytes,
    save_image_depth;

  logging=LogMagickEvent(CoderEvent,GetMagickModule(),
    "  enter WriteOnePNGImage()");

#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
  LockSemaphoreInfo(png_semaphore);
#endif

  quantum_info = (QuantumInfo *) NULL;
  image_colors=image->colors;
  image_depth=image->depth;
  image_matte=image->matte;

  if (image->colorspace != RGBColorspace)
    (void) TransformImageColorspace(image,RGBColorspace);
  mng_info->IsPalette=image->storage_class == PseudoClass && 
            image_colors <= 256 && !IsOpaqueImage(image,&image->exception);
  mng_info->optimize=image_info->type == OptimizeType;

  /*
    Allocate the PNG structures
  */
#ifdef PNG_USER_MEM_SUPPORTED
  ping=png_create_write_struct_2(PNG_LIBPNG_VER_STRING,image,
    PNGErrorHandler,PNGWarningHandler,(void *) NULL,
    (png_malloc_ptr) png_IM_malloc,(png_free_ptr) png_IM_free);
#else
  ping=png_create_write_struct(PNG_LIBPNG_VER_STRING,image,
    PNGErrorHandler,PNGWarningHandler);
#endif
  if (ping == (png_struct *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  ping_info=png_create_info_struct(ping);
  if (ping_info == (png_info *) NULL)
    {
      png_destroy_write_struct(&ping,(png_info **) NULL);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  png_set_write_fn(ping,image,png_put_data,png_flush_data);
  png_pixels=(unsigned char *) NULL;

  if (setjmp(ping->jmpbuf))
    {
      /*
        PNG write failed.
      */
#ifdef PNG_DEBUG
     if (image_info->verbose)
        (void) printf("PNG write has failed.\n");
#endif
      png_destroy_write_struct(&ping,&ping_info);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
      UnlockSemaphoreInfo(png_semaphore);
#endif
      return(MagickFalse);
    }
  /*
    Prepare PNG for writing.
  */
#if defined(PNG_MNG_FEATURES_SUPPORTED)
  if (mng_info->write_mng)
     (void) png_permit_mng_features(ping,PNG_ALL_MNG_FEATURES);
#else
# ifdef PNG_WRITE_EMPTY_PLTE_SUPPORTED
  if (mng_info->write_mng)
     png_permit_empty_plte(ping,MagickTrue);
# endif
#endif
  x=0;
  ping_info->width=image->columns;
  ping_info->height=image->rows;
  if (mng_info->write_png8 || mng_info->write_png24 || mng_info->write_png32)
     image_depth=8;
  if (mng_info->write_png_depth != 0)
     image_depth=mng_info->write_png_depth;
  /* Adjust requested depth to next higher valid depth if necessary */
  if (image_depth > 8)
     image_depth=16;
  if ((image_depth > 4) && (image_depth < 8))
     image_depth=8;
  if (image_depth == 3)
     image_depth=4;
  if (logging != MagickFalse)
    {
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    width=%lu",ping_info->width);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    height=%lu",ping_info->height);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image_matte=%u",image->matte);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image_depth=%lu",image->depth);
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    requested PNG image_depth=%lu",image->depth);
    }
  save_image_depth=image_depth;
  ping_info->bit_depth=(png_byte) save_image_depth;
#if defined(PNG_pHYs_SUPPORTED)
  if ((image->x_resolution != 0) && (image->y_resolution != 0) &&
      (!mng_info->write_mng || !mng_info->equal_physs))
    {
      int
        unit_type;

      png_uint_32
        x_resolution,
        y_resolution;

      if (image->units == PixelsPerInchResolution)
        {
          unit_type=PNG_RESOLUTION_METER;
          x_resolution=(png_uint_32) (100.0*image->x_resolution/2.54);
          y_resolution=(png_uint_32) (100.0*image->y_resolution/2.54);
        }
      else if (image->units == PixelsPerCentimeterResolution)
        {
          unit_type=PNG_RESOLUTION_METER;
          x_resolution=(png_uint_32) (100.0*image->x_resolution);
          y_resolution=(png_uint_32) (100.0*image->y_resolution);
        }
      else
        {
          unit_type=PNG_RESOLUTION_UNKNOWN;
          x_resolution=(png_uint_32) image->x_resolution;
          y_resolution=(png_uint_32) image->y_resolution;
        }
       png_set_pHYs(ping,ping_info,x_resolution,y_resolution,unit_type);
       if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "    Setting up pHYs chunk");
    }
#endif
#if defined(PNG_oFFs_SUPPORTED)
  if (image->page.x || image->page.y)
    {
       png_set_oFFs(ping,ping_info,(png_int_32) image->page.x,
          (png_int_32) image->page.y, 0);
       if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "    Setting up oFFs chunk");
    }
#endif
  if (image_matte && (!mng_info->adjoin || !mng_info->equal_backgrounds))
    {
      png_color_16
        background;

      if (image_depth < MAGICKCORE_QUANTUM_DEPTH)
        {
          unsigned long
             maxval;

          maxval=(1UL << image_depth)-1;
          background.red=(png_uint_16)
            (QuantumScale*(maxval*image->background_color.red));
          background.green=(png_uint_16)
            (QuantumScale*(maxval*image->background_color.green));
          background.blue=(png_uint_16)
            (QuantumScale*(maxval*image->background_color.blue));
          background.gray=(png_uint_16)
            (QuantumScale*(maxval*PixelIntensity(&image->background_color)));
        }
      else
        {
          background.red=image->background_color.red;
          background.green=image->background_color.green;
          background.blue=image->background_color.blue;
          background.gray=
            (png_uint_16) PixelIntensity(&image->background_color);
        }
      background.index=(png_byte) background.gray;
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Setting up bKGd chunk");
      png_set_bKGD(ping,ping_info,&background);
    }
  /*
    Select the color type.
  */
  matte=image_matte;
  old_bit_depth=0;
  if (mng_info->write_png8)
    {
      ping_info->color_type=(png_byte) PNG_COLOR_TYPE_PALETTE;
      ping_info->bit_depth=8;
      image_depth=ping_info->bit_depth;
        {
          /* TO DO: make this a function cause it's used twice, except
             for reducing the sample depth from 8. */

          QuantizeInfo
            quantize_info;

          unsigned long
             number_colors,
             save_number_colors;

          number_colors=image_colors;
          if ((image->storage_class == DirectClass) || (number_colors > 256))
            {
              GetQuantizeInfo(&quantize_info);
              quantize_info.dither=IsPaletteImage(image,&image->exception) ==
                MagickFalse ? MagickTrue : MagickFalse;
              quantize_info.number_colors= (matte != MagickFalse ? 255UL :
                256UL);
              (void) QuantizeImage(&quantize_info,image);
              number_colors=image_colors;
              (void) SyncImage(image);
              if (logging != MagickFalse)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    Colors quantized to %ld",number_colors);
            }
          if (matte)
            ping_info->valid|=PNG_INFO_tRNS;
          /*
            Set image palette.
          */
          ping_info->color_type=(png_byte) PNG_COLOR_TYPE_PALETTE;
          ping_info->valid|=PNG_INFO_PLTE;
#if defined(PNG_SORT_PALETTE)
          save_number_colors=image_colors;
          if (CompressColormapTransFirst(image) == MagickFalse)
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          number_colors=image->colors;
          image_colors=save_number_colors;
#endif
          palette=(png_color *) AcquireQuantumMemory(257,
            sizeof(*palette));
          if (palette == (png_color *) NULL)
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  Setting up PLTE chunk with %d colors",
                (int) number_colors);
          for (i=0; i < (long) number_colors; i++)
          {
            palette[i].red=ScaleQuantumToChar(image->colormap[i].red);
            palette[i].green=ScaleQuantumToChar(image->colormap[i].green);
            palette[i].blue=ScaleQuantumToChar(image->colormap[i].blue);
            if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
#if MAGICKCORE_QUANTUM_DEPTH == 8
                "    %3ld (%3d,%3d,%3d)",
#else
                "    %5ld (%5d,%5d,%5d)",
#endif
                i,palette[i].red,palette[i].green,palette[i].blue);

          }
          if (matte)
            {
              number_colors++;
              palette[i].red=ScaleQuantumToChar((Quantum) QuantumRange);
              palette[i].green=ScaleQuantumToChar((Quantum) QuantumRange);
              palette[i].blue=ScaleQuantumToChar((Quantum) QuantumRange);
            }
          png_set_PLTE(ping,ping_info,palette,(int) number_colors);
#if (PNG_LIBPNG_VER > 10008)
          palette=(png_colorp) RelinquishMagickMemory(palette);
#endif
            image_depth=ping_info->bit_depth;
            ping_info->num_trans=0;
            if (matte)
            {
              ExceptionInfo
                *exception;

              /*
                Identify which colormap entry is transparent.
              */
              ping_info->trans_alpha=(unsigned char *) AcquireQuantumMemory(
                number_colors,sizeof(*ping_info->trans_alpha));
              if (ping_info->trans_alpha == (unsigned char *) NULL)
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
              assert(number_colors <= 256);
              for (i=0; i < (long) number_colors; i++)
                 ping_info->trans_alpha[i]=255;
              exception=(&image->exception);
              for (y=0; y < (long) image->rows; y++)
              {
                register const PixelPacket
                  *p;

                p=GetAuthenticPixels(image,0,y,image->columns,1,exception);
                if (p == (PixelPacket *) NULL)
                  break;
                indices=GetAuthenticIndexQueue(image);
                for (x=0; x < (long) image->columns; x++)
                {
                  if (p->opacity != OpaqueOpacity)
                    {
                      indices[x]=(IndexPacket) (number_colors-1);
                      ping_info->trans_alpha[(long) indices[x]]=(png_byte) (255-
                        ScaleQuantumToChar(GetOpacityPixelComponent(p)));
                    }
                  p++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
              for (i=0; i < (long) number_colors; i++)
                if (ping_info->trans_alpha[i] != 255)
                  ping_info->num_trans=(unsigned short) (i+1);
              if (ping_info->num_trans == 0)
                ping_info->valid&=(~PNG_INFO_tRNS);
              if (!(ping_info->valid & PNG_INFO_tRNS))
                ping_info->num_trans=0;
              if (ping_info->num_trans == 0)
                ping_info->trans_alpha=(unsigned char *)
                  RelinquishMagickMemory(ping_info->trans_alpha);
            }
          /*
            Identify which colormap entry is the background color.
          */
          for (i=0; i < (long) MagickMax(1L*number_colors-1L,1L); i++)
            if (IsPNGColorEqual(ping_info->background,image->colormap[i]))
              break;
          ping_info->background.index=(png_byte) i;
        }
      if (image_matte != MagickFalse)
        {
          /* TO DO: reduce to binary transparency */
        }
    } /* end of write_png8 */
  else if (mng_info->write_png24)
    {
      image_matte=MagickFalse;
      ping_info->color_type=(png_byte) PNG_COLOR_TYPE_RGB;
    }
  else if (mng_info->write_png32)
    {
      image_matte=MagickTrue;
      ping_info->color_type=(png_byte) PNG_COLOR_TYPE_RGB_ALPHA;
    }
  else
    {
      image_depth=ping_info->bit_depth;
      if (mng_info->write_png_colortype)
        {
          ping_info->color_type=(png_byte) mng_info->write_png_colortype-1;
          if (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
              ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            image_matte=MagickTrue;
        }
      else
        {
          if (logging != MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "Selecting PNG colortype");
          ping_info->color_type=(png_byte) ((matte == MagickTrue)?
          PNG_COLOR_TYPE_RGB_ALPHA:PNG_COLOR_TYPE_RGB);
          if(image_info->type == TrueColorType)
            {
              ping_info->color_type=(png_byte) PNG_COLOR_TYPE_RGB;
              image_matte=MagickFalse;
            }
          if(image_info->type == TrueColorMatteType)
            {
              ping_info->color_type=(png_byte) PNG_COLOR_TYPE_RGB_ALPHA;
              image_matte=MagickTrue;
            }
          if ((image_info->type == UndefinedType || 
             image_info->type == OptimizeType || 
             image_info->type == GrayscaleType) &&
             image_matte == MagickFalse && ImageIsGray(image))
            {
              ping_info->color_type=(png_byte) PNG_COLOR_TYPE_GRAY;
              image_matte=MagickFalse;
            }
          if ((image_info->type == UndefinedType ||
             image_info->type == OptimizeType || 
              image_info->type == GrayscaleMatteType) &&
              image_matte == MagickTrue && ImageIsGray(image))
            {
              ping_info->color_type=(png_byte) PNG_COLOR_TYPE_GRAY_ALPHA;
              image_matte=MagickTrue;
            } 
        }
      if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "Selected PNG colortype=%d",ping_info->color_type);

      if (ping_info->bit_depth < 8)
       {
         if (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
             ping_info->color_type == PNG_COLOR_TYPE_RGB ||
             ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
           ping_info->bit_depth=8;
       }

      if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
        {
          if (image->matte == MagickFalse && image->colors < 256)
            {
              if (ImageIsMonochrome(image))
                {
                  ping_info->bit_depth=1;
                  if (ping_info->bit_depth < mng_info->write_png_depth)
                    ping_info->bit_depth = mng_info->write_png_depth;
                }
            }
        }
      if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
        {
           ping_info->bit_depth=1;
           while ((int) (1 << ping_info->bit_depth) < (long) image_colors)
             ping_info->bit_depth <<= 1;

           if (logging != MagickFalse)
             {
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Number of colors: %lu",image_colors);
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    Tentative PNG bit depth: %d",ping_info->bit_depth);
             }
           if (mng_info->write_png_depth)
             {
               old_bit_depth=ping_info->bit_depth;
               if (ping_info->bit_depth < mng_info->write_png_depth)
                 {
                   ping_info->bit_depth = mng_info->write_png_depth;
                   if (ping_info->bit_depth > 8)
                      ping_info->bit_depth = 8;
                   if (ping_info->bit_depth != old_bit_depth)
                     {
                       if (logging != MagickFalse)
                         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                           "    Colors increased to %ld",image_colors);
                     }
                 }
             }
        }
    }
  image_depth=ping_info->bit_depth;
  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Tentative PNG color type: %d",ping_info->color_type);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image_info->type: %d",image_info->type);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    image_depth: %lu",image_depth);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    ping_info->bit_depth: %d",ping_info->bit_depth);
    }

  if (matte && (mng_info->optimize || mng_info->IsPalette))
    {
      register const PixelPacket
        *p;

      p=GetVirtualPixels(image,0,0,image->columns,1,&image->exception);
      ping_info->color_type=PNG_COLOR_TYPE_GRAY_ALPHA;
      for (y=0; y < (long) image->rows; y++)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=(long) image->columns-1; x >= 0; x--)
        {
          if (IsGray(p) == MagickFalse)
            {
              ping_info->color_type=(png_byte) PNG_COLOR_TYPE_RGB_ALPHA;
              break;
            }
          p++;
        }
      }
      /*
        Determine if there is any transparent color.
      */
      for (y=0; y < (long) image->rows; y++)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        for (x=(long) image->columns-1; x >= 0; x--)
        {
          if (p->opacity != OpaqueOpacity)
            break;
          p++;
        }
        if (x != 0)
          break;
      }
      if ((y == (long) image->rows) && (x == (long) image->columns))
        {
          /*
            No transparent pixels are present.  Change 4 or 6 to 0 or 2,
            and do not set the PNG_INFO_tRNS flag in ping_info->valid.
          */
          image_matte=MagickFalse;
          ping_info->color_type&=0x03;
        }
      else
        {
          unsigned int
            mask;

          mask=0xffff;
          if (ping_info->bit_depth == 8)
             mask=0x00ff;
          if (ping_info->bit_depth == 4)
             mask=0x000f;
          if (ping_info->bit_depth == 2)
             mask=0x0003;
          if (ping_info->bit_depth == 1)
             mask=0x0001;
          ping_info->valid|=PNG_INFO_tRNS;
          ping_info->trans_color.red=(png_uint_16)
            (ScaleQuantumToShort(GetRedPixelComponent(p)) & mask);
          ping_info->trans_color.green=(png_uint_16)
            (ScaleQuantumToShort(GetGreenPixelComponent(p)) & mask);
          ping_info->trans_color.blue=(png_uint_16)
            (ScaleQuantumToShort(GetBluePixelComponent(p)) & mask);
          ping_info->trans_color.gray=(png_uint_16)
            (ScaleQuantumToShort(PixelIntensityToQuantum(p)) & mask);
          ping_info->trans_color.index=(png_byte)
            (ScaleQuantumToChar((Quantum) (GetAlphaPixelComponent(p))));
        }
      if (ping_info->valid & PNG_INFO_tRNS)
        {
          /*
            Determine if there is one and only one transparent color
            and if so if it is fully transparent.
          */
          for (y=0; y < (long) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,
               &image->exception);
            x=0;
            if (p == (const PixelPacket *) NULL)
              break;
            for (x=(long) image->columns-1; x >= 0; x--)
            {
              if (p->opacity != OpaqueOpacity)
                {
                  if (IsPNGColorEqual(ping_info->trans_color,*p) == 0)
                  {
                     break;  /* Can't use RGB + tRNS for multiple
                                transparent colors.  */
                  }
                  if (p->opacity != (Quantum) TransparentOpacity)
                  {
                     break;  /* Can't use RGB + tRNS for
                                semitransparency. */
                  }
                }
               else
                {
                  if (IsPNGColorEqual(ping_info->trans_color,*p))
                      break; /* Can't use RGB + tRNS when another pixel
                                having the same RGB samples is
                                transparent. */
                }
            p++;
            }
            if (x != 0)
               break;
          }
          if (x != 0)
            ping_info->valid&=(~PNG_INFO_tRNS);
        }
      if (ping_info->valid & PNG_INFO_tRNS)
        {
          ping_info->color_type &= 0x03;  /* changes 4 or 6 to 0 or 2 */
          if (image_depth == 8)
            {
              ping_info->trans_color.red&=0xff;
              ping_info->trans_color.green&=0xff;
              ping_info->trans_color.blue&=0xff;
              ping_info->trans_color.gray&=0xff;
            }
        }
    }
    matte=image_matte;
    if (ping_info->valid & PNG_INFO_tRNS)
      image_matte=MagickFalse;
    if ((mng_info->optimize || mng_info->IsPalette) &&
        mng_info->write_png_colortype-1 != PNG_COLOR_TYPE_PALETTE &&
        ImageIsGray(image) && (!image_matte || image_depth >= 8))
      {
        if (image_matte != MagickFalse)
            ping_info->color_type=PNG_COLOR_TYPE_GRAY_ALPHA;
        else
          {
            ping_info->color_type=PNG_COLOR_TYPE_GRAY;
            if (save_image_depth == 16 && image_depth == 8)
              ping_info->trans_color.gray*=0x0101;
          }
        if (image_depth > MAGICKCORE_QUANTUM_DEPTH)
          image_depth=MAGICKCORE_QUANTUM_DEPTH;
        if (image_colors == 0 || image_colors-1 > MaxColormapSize)
          image_colors=1 << image_depth;
        if (image_depth > 8)
          ping_info->bit_depth=16;
        else
          {
            ping_info->bit_depth=8;
            if ((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
              {
                if(!mng_info->write_png_depth)
                  {
                    ping_info->bit_depth=1;
                    while ((int) (1 << ping_info->bit_depth)
                        < (long) image_colors)
                      ping_info->bit_depth <<= 1;
                  }
              }
            else if (mng_info->optimize && ping_info->color_type ==
                PNG_COLOR_TYPE_GRAY && image_colors < 17 && 
                mng_info->IsPalette)
              {

              /* Check if grayscale is reducible */
                int
                  depth_4_ok=MagickTrue,
                  depth_2_ok=MagickTrue,
                  depth_1_ok=MagickTrue;

                for (i=0; i < (long) image_colors; i++)
                {
                   unsigned char
                     intensity;

                   intensity=ScaleQuantumToChar(image->colormap[i].red);

                   if ((intensity & 0x0f) != ((intensity & 0xf0) >> 4))
                     depth_4_ok=depth_2_ok=depth_1_ok=MagickFalse;
                   else if ((intensity & 0x03) != ((intensity & 0x0c) >> 2))
                     depth_2_ok=depth_1_ok=MagickFalse;
                   else if ((intensity & 0x01) != ((intensity & 0x02) >> 1))
                     depth_1_ok=MagickFalse;
                }
                if (depth_1_ok && mng_info->write_png_depth <= 1)
                   ping_info->bit_depth=1;
                else if (depth_2_ok && mng_info->write_png_depth <= 2)
                   ping_info->bit_depth=2;
                else if (depth_4_ok && mng_info->write_png_depth <= 4)
                   ping_info->bit_depth=4;
              }
          }
          image_depth=ping_info->bit_depth;
      }
    else
      if (mng_info->IsPalette)
      {
        if (image_depth <= 8)
          {
            unsigned long
               number_colors;

            number_colors=image_colors;
            if (matte)
               ping_info->valid|=PNG_INFO_tRNS;
            /*
              Set image palette.
            */
            ping_info->color_type=(png_byte) PNG_COLOR_TYPE_PALETTE;
            ping_info->valid|=PNG_INFO_PLTE;
            if (mng_info->have_write_global_plte && !matte)
              {
                 png_set_PLTE(ping,ping_info,NULL,0);
                 if (logging)
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                     "  Setting up empty PLTE chunk");
              }
            else
              {
#if defined(PNG_SORT_PALETTE)
                unsigned long
                   save_number_colors;

                if (mng_info->optimize)
                  {
                    save_number_colors=image_colors;
                    if (CompressColormapTransFirst(image) == MagickFalse)
                       ThrowWriterException(ResourceLimitError,
                                            "MemoryAllocationFailed");
                    number_colors=image->colors;
                    image_colors=save_number_colors;
                  }
#endif
                palette=(png_color *) AcquireQuantumMemory(257,
                  sizeof(*palette));
                if (palette == (png_color *) NULL)
                  ThrowWriterException(ResourceLimitError,
                     "MemoryAllocationFailed");
                for (i=0; i < (long) number_colors; i++)
                {
                  palette[i].red=ScaleQuantumToChar(image->colormap[i].red);
                  palette[i].green=ScaleQuantumToChar(image->colormap[i].green);
                  palette[i].blue=ScaleQuantumToChar(image->colormap[i].blue);
                }
                if (logging)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  Setting up PLTE chunk with %d colors",
                    (int) number_colors);
                png_set_PLTE(ping,ping_info,palette,(int) number_colors);
#if (PNG_LIBPNG_VER > 10008)
                palette=(png_colorp) RelinquishMagickMemory(palette);
#endif
              }
            /* color_type is PNG_COLOR_TYPE_PALETTE */
            if (!mng_info->write_png_depth)
              {
                ping_info->bit_depth=1;
                while ((1UL << ping_info->bit_depth) < number_colors)
                  ping_info->bit_depth <<= 1;
              }
            ping_info->num_trans=0;
            if (matte)
            {
              ExceptionInfo
                *exception;

              register const PixelPacket
                *p;

              int
                trans[256];

              register const IndexPacket
                *packet_indices;

              /*
                Identify which colormap entry is transparent.
              */
              assert(number_colors <= 256);
              for (i=0; i < (long) number_colors; i++)
                trans[i]=256;
              exception=(&image->exception);
              for (y=0; y < (long) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const PixelPacket *) NULL)
                  break;
                packet_indices=GetVirtualIndexQueue(image);
                for (x=0; x < (long) image->columns; x++)
                {
                  if (p->opacity != OpaqueOpacity)
                    {
                      IndexPacket
                        packet_index;

                      packet_index=packet_indices[x];
                      assert((unsigned long) packet_index < number_colors);
                      if (trans[(long) packet_index] != 256)
                        {
                          if (trans[(long) packet_index] != (png_byte) (255-
                             ScaleQuantumToChar(GetOpacityPixelComponent(p))))
                            {
                              ping_info->color_type=(png_byte)
                                PNG_COLOR_TYPE_RGB_ALPHA;
                              break;
                            }
                        }
                      trans[(long) packet_index]=(png_byte) (255-
                        ScaleQuantumToChar(GetOpacityPixelComponent(p)));
                    }
                  p++;
                }
                if ((int) ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
                {
                  ping_info->num_trans=0;
                  ping_info->valid&=(~PNG_INFO_tRNS);
                  ping_info->valid&=(~PNG_INFO_PLTE);
                  mng_info->IsPalette=MagickFalse;
                  (void) SyncImage(image);
                  if (logging)
                    (void) LogMagickEvent(CoderEvent, GetMagickModule(),
                      "    Cannot write image as indexed PNG, writing RGBA.");
                  break;
                }
              }
              if ((ping_info->valid & PNG_INFO_tRNS))
              {
                for (i=0; i < (long) number_colors; i++)
                {
                  if (trans[i] == 256)
                    trans[i]=255;
                  if (trans[i] != 255)
                    ping_info->num_trans=(unsigned short) (i+1);
                }
              }
              if (ping_info->num_trans == 0)
                ping_info->valid&=(~PNG_INFO_tRNS);
              if (!(ping_info->valid & PNG_INFO_tRNS))
                ping_info->num_trans=0;
              if (ping_info->num_trans != 0)
              {
                ping_info->trans_alpha=(unsigned char *) AcquireQuantumMemory(
                  number_colors,sizeof(*ping_info->trans_alpha));
                if (ping_info->trans_alpha == (unsigned char *) NULL)
                  ThrowWriterException(ResourceLimitError,
                     "MemoryAllocationFailed");
                for (i=0; i < (long) number_colors; i++)
                    ping_info->trans_alpha[i]=(png_byte) trans[i];
              }
            }

            /*
              Identify which colormap entry is the background color.
            */
            for (i=0; i < (long) MagickMax(1L*number_colors-1L,1L); i++)
              if (IsPNGColorEqual(ping_info->background,image->colormap[i]))
                break;
            ping_info->background.index=(png_byte) i;
          }
      }
    else
      {
        if (image_depth < 8)
          image_depth=8;
        if ((save_image_depth == 16) && (image_depth == 8))
          {
            ping_info->trans_color.red*=0x0101;
            ping_info->trans_color.green*=0x0101;
            ping_info->trans_color.blue*=0x0101;
            ping_info->trans_color.gray*=0x0101;
          }
      }

    /*
      Adjust background and transparency samples in sub-8-bit grayscale files.
    */
    if (ping_info->bit_depth < 8 && ping_info->color_type ==
        PNG_COLOR_TYPE_GRAY)
      {
         png_uint_16
           maxval;

         png_color_16
           background;

         maxval=(png_uint_16) ((1 << ping_info->bit_depth)-1);


         background.gray=(png_uint_16)
           (QuantumScale*(maxval*(PixelIntensity(&image->background_color))));

         if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "  Setting up bKGD chunk");
         png_set_bKGD(ping,ping_info,&background);

         ping_info->trans_color.gray=(png_uint_16) (QuantumScale*(maxval*
           ping_info->trans_color.gray));
      }
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    PNG color type: %d",ping_info->color_type);
  /*
    Initialize compression level and filtering.
  */
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Setting up deflate compression");
#if (PNG_LIBPNG_VER > 99)
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Compression buffer size: 32768");
  png_set_compression_buffer_size(ping,32768L);
#endif
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "    Compression mem level: 9");
  png_set_compression_mem_level(ping, 9);
  quality=image->quality == UndefinedCompressionQuality ? 75UL :
     image->quality;
  if (quality > 9)
    {
      int
        level;

      level=(int) MagickMin((long) quality/10,9);
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Compression level: %d",level);
      png_set_compression_level(ping,level);
    }
  else
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Compression strategy: Z_HUFFMAN_ONLY");
      png_set_compression_strategy(ping, Z_HUFFMAN_ONLY);
    }
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Setting up filtering");
#if defined(PNG_MNG_FEATURES_SUPPORTED) && defined(PNG_INTRAPIXEL_DIFFERENCING)

  /* This became available in libpng-1.0.9.  Output must be a MNG. */
  if (mng_info->write_mng && ((quality % 10) == 7))
    {
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Filter_type: PNG_INTRAPIXEL_DIFFERENCING");
      ping_info->filter_type=PNG_INTRAPIXEL_DIFFERENCING;
    }
  else
    if (logging != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Filter_type: 0");
#endif
  {
    int
      base_filter;

    if ((quality % 10) > 5)
      base_filter=PNG_ALL_FILTERS;
    else
      if ((quality % 10) != 5)
        base_filter=(int) quality % 10;
      else
        if (((int) ping_info->color_type == PNG_COLOR_TYPE_GRAY) ||
            ((int) ping_info->color_type == PNG_COLOR_TYPE_PALETTE) ||
            (quality < 50))
          base_filter=PNG_NO_FILTERS;
        else
          base_filter=PNG_ALL_FILTERS;
    if (logging != MagickFalse)
      {
        if (base_filter == PNG_ALL_FILTERS)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Base filter method: ADAPTIVE");
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Base filter method: NONE");
      }
    png_set_filter(ping,PNG_FILTER_TYPE_BASE,base_filter);
  }

  ResetImageProfileIterator(image);
  for (name=GetNextImageProfile(image); name != (const char *) NULL; )
  {
    profile=GetImageProfile(image,name);
    if (profile != (StringInfo *) NULL)
      {
#if (PNG_LIBPNG_VER > 10008) && defined(PNG_WRITE_iCCP_SUPPORTED)
        if ((LocaleCompare(name,"ICC") == 0) ||
            (LocaleCompare(name,"ICM") == 0))
          png_set_iCCP(ping,ping_info,(const png_charp) name,0,(png_charp)
            GetStringInfoDatum(profile),
                     (png_uint_32) GetStringInfoLength(profile));
        else
#endif
          png_write_raw_profile(image_info,ping,ping_info,(unsigned char *)
            name,(unsigned char *) name,GetStringInfoDatum(profile),
            (png_uint_32) GetStringInfoLength(profile));
      }
    if (logging != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Setting up text chunk with %s profile",name);
    name=GetNextImageProfile(image);
  }

#if defined(PNG_WRITE_sRGB_SUPPORTED)
  if ((mng_info->have_write_global_srgb == 0) &&
      ((image->rendering_intent != UndefinedIntent) ||
      (image->colorspace == sRGBColorspace)))
    {
      /*
        Note image rendering intent.
      */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Setting up sRGB chunk");
      (void) png_set_sRGB(ping,ping_info,(int) (image->rendering_intent-1));
      png_set_gAMA(ping,ping_info,0.45455);
    }
  if ((!mng_info->write_mng) || !(ping_info->valid & PNG_INFO_sRGB))
#endif
    {
      if ((mng_info->have_write_global_gama == 0) && (image->gamma != 0.0))
        {
          /*
            Note image gamma.
            To do: check for cHRM+gAMA == sRGB, and write sRGB instead.
          */
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Setting up gAMA chunk");
          png_set_gAMA(ping,ping_info,image->gamma);
        }
      if ((mng_info->have_write_global_chrm == 0) &&
          (image->chromaticity.red_primary.x != 0.0))
        {
          /*
            Note image chromaticity.
            To do: check for cHRM+gAMA == sRGB, and write sRGB instead.
          */
           PrimaryInfo
             bp,
             gp,
             rp,
             wp;

           wp=image->chromaticity.white_point;
           rp=image->chromaticity.red_primary;
           gp=image->chromaticity.green_primary;
           bp=image->chromaticity.blue_primary;

           if (logging != MagickFalse)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "  Setting up cHRM chunk");
           png_set_cHRM(ping,ping_info,wp.x,wp.y,rp.x,rp.y,gp.x,gp.y,
               bp.x,bp.y);
       }
    }
  ping_info->interlace_type=image_info->interlace != NoInterlace;

  if (mng_info->write_mng)
    png_set_sig_bytes(ping,8);

  /* Bail out if cannot meet defined PNG:bit-depth or PNG:color-type */

  if (mng_info->write_png_colortype)
    {
     if (mng_info->write_png_colortype-1 == PNG_COLOR_TYPE_GRAY)
       if (ImageIsGray(image) == MagickFalse)
         {
           ping_info->color_type = PNG_COLOR_TYPE_RGB;
           if (ping_info->bit_depth < 8)
             ping_info->bit_depth=8;
         }
         
     if (mng_info->write_png_colortype-1 == PNG_COLOR_TYPE_GRAY_ALPHA)
       if (ImageIsGray(image) == MagickFalse)
         ping_info->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    }

  if ((mng_info->write_png_depth &&
      mng_info->write_png_depth != ping_info->bit_depth) ||
     (mng_info->write_png_colortype &&
     (mng_info->write_png_colortype-1 != ping_info->color_type &&
      mng_info->write_png_colortype != 7 &&
      !(mng_info->write_png_colortype == 5 && ping_info->color_type == 0))))
    {
      if (logging != MagickFalse)
        {
          if (mng_info->write_png_depth)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Defined PNG:bit-depth=%u, Computed depth=%u",
                  mng_info->write_png_depth,
                  ping_info->bit_depth);
            }
          if (mng_info->write_png_colortype)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Defined PNG:color-type=%u, Computed color type=%u",
                  mng_info->write_png_colortype-1,
                  ping_info->color_type);
            }
        }
      png_error(ping,
        "Cannot write image with defined PNG:bit-depth or PNG:color-type.");
    }

  if (image_matte && !image->matte)
    {
      /* Add an opaque matte channel */
      image->matte = MagickTrue;
      (void) SetImageOpacity(image,0);
    }

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Writing PNG header chunks");

  png_write_info_before_PLTE(ping, ping_info);
  /* write any png-chunk-b profiles */
  (void) png_write_chunk_from_profile(image,"PNG-chunk-b",(int) logging);
  png_write_info(ping,ping_info);
  /* write any PNG-chunk-m profiles */
  (void) png_write_chunk_from_profile(image,"PNG-chunk-m",(int) logging);

  if (image->page.width || image->page.height)
    {
       unsigned char
         chunk[14];

       (void) WriteBlobMSBULong(image,9L);  /* data length=8 */
       PNGType(chunk,mng_vpAg);
       LogPNGChunk((int) logging,mng_vpAg,9L);
       PNGLong(chunk+4,(png_uint_32) image->page.width);
       PNGLong(chunk+8,(png_uint_32) image->page.height);
       chunk[12]=0;   /* unit = pixels */
       (void) WriteBlob(image,13,chunk);
       (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
    }

#if (PNG_LIBPNG_VER == 10206)
      /* avoid libpng-1.2.6 bug by setting PNG_HAVE_IDAT flag */
#define PNG_HAVE_IDAT               0x04
      ping->mode |= PNG_HAVE_IDAT;
#undef PNG_HAVE_IDAT
#endif

  png_set_packing(ping);
  /*
    Allocate memory.
  */
  rowbytes=image->columns;
  if (image_depth <= 8)
    {
      if (mng_info->write_png24 || (mng_info->write_png_depth == 8 &&
          mng_info->write_png_colortype-1 == PNG_COLOR_TYPE_RGB))
        rowbytes*=3;
      else if (mng_info->write_png32 || (mng_info->write_png_depth == 8 &&
           mng_info->write_png_colortype-1 == PNG_COLOR_TYPE_RGB_ALPHA))
        rowbytes*=4;
      else if ((!mng_info->write_png8 ||
           mng_info->write_png_colortype-1 == PNG_COLOR_TYPE_GRAY ||
           mng_info->write_png_colortype-1 == PNG_COLOR_TYPE_GRAY_ALPHA )&&
           ((mng_info->optimize || mng_info->IsPalette) && ImageIsGray(image)))
        rowbytes*=(image_matte ? 2 : 1);
      else
        {
          if (!mng_info->IsPalette)
            rowbytes*=(image_matte ? 4 : 3);
        }
    }
  else
    {
      if ((mng_info->optimize || mng_info->IsPalette) &&
          ImageIsGray(image))
        rowbytes*=(image_matte ? 4 : 2);
      else
        rowbytes*=(image_matte ? 8 : 6);
    }
  if (logging)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Allocating %lu bytes of memory for pixels",rowbytes);
  png_pixels=(unsigned char *) AcquireQuantumMemory(rowbytes,
    sizeof(*png_pixels));
  if (png_pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize image scanlines.
  */
  if (setjmp(ping->jmpbuf))
    {
      /*
        PNG write failed.
      */
#ifdef PNG_DEBUG
     if (image_info->verbose)
        (void) printf("PNG write has failed.\n");
#endif
      png_destroy_write_struct(&ping,&ping_info);
      if (quantum_info != (QuantumInfo *) NULL)
        quantum_info=DestroyQuantumInfo(quantum_info);
      if (png_pixels != (unsigned char *) NULL)
        png_pixels=(unsigned char *) RelinquishMagickMemory(png_pixels);
#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
      UnlockSemaphoreInfo(png_semaphore);
#endif
      return(MagickFalse);
    }
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  quantum_info->format=UndefinedQuantumFormat;
  quantum_info->depth=image_depth;
  num_passes=png_set_interlace_handling(ping);
  if ((!mng_info->write_png8 && !mng_info->write_png24 &&
                      !mng_info->write_png32) &&
      (mng_info->optimize || mng_info->IsPalette ||
       (image_info->type == BilevelType)) &&
      !image_matte && ImageIsMonochrome(image))
    {
      register const PixelPacket
        *p;

      quantum_info->depth=8;
      for (pass=0; pass < num_passes; pass++)
      {
        /*
          Convert PseudoClass image to a PNG monochrome image.
        */
        for (y=0; y < (long) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          if (mng_info->IsPalette)
            {
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,GrayQuantum,png_pixels,&image->exception);
              if (mng_info->write_png_colortype-1 == PNG_COLOR_TYPE_PALETTE &&
                  mng_info->write_png_depth &&
                  mng_info->write_png_depth != old_bit_depth)
                {
                  /* Undo pixel scaling */
                  for (i=0; i < (long) image->columns; i++)
                     *(png_pixels+i)=(unsigned char) (*(png_pixels+i)
                     >> (8-old_bit_depth));
                }
            }
          else
            {
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,RedQuantum,png_pixels,&image->exception);
            }
          if (mng_info->write_png_colortype-1 != PNG_COLOR_TYPE_PALETTE)
            for (i=0; i < (long) image->columns; i++)
               *(png_pixels+i)=(unsigned char) ((*(png_pixels+i) > 127) ?
                      255 : 0);
          png_write_row(ping,png_pixels);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,pass,num_passes);
            if (status == MagickFalse)
              break;
          }
      }
    }
  else
    for (pass=0; pass < num_passes; pass++)
    {
      register const PixelPacket
        *p;

      if ((!mng_info->write_png8 && !mng_info->write_png24 && 
         !mng_info->write_png32) &&
         (image_matte ||
         (ping_info->bit_depth >= MAGICKCORE_QUANTUM_DEPTH)) &&
         (mng_info->optimize || mng_info->IsPalette) && ImageIsGray(image))
      {
        for (y=0; y < (long) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
            {
              if (mng_info->IsPalette)
                (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                  quantum_info,GrayQuantum,png_pixels,&image->exception);
              else
                (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                  quantum_info,RedQuantum,png_pixels,&image->exception);
            }
          else /* PNG_COLOR_TYPE_GRAY_ALPHA */
            {
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,GrayAlphaQuantum,png_pixels,&image->exception);
            }
          png_write_row(ping,png_pixels);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,pass,num_passes);
            if (status == MagickFalse)
              break;
          }
      }
    else
      for (pass=0; pass < num_passes; pass++)
      {
        if ((image_depth > 8) || (mng_info->write_png24 ||
            mng_info->write_png32 ||
            (!mng_info->write_png8 && !mng_info->IsPalette)))
          for (y=0; y < (long) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
              {
                if (image->storage_class == DirectClass)
                  (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                    quantum_info,RedQuantum,png_pixels,&image->exception);
                else
                  (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                    quantum_info,GrayQuantum,png_pixels,&image->exception);
              }
            else if (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,GrayAlphaQuantum,png_pixels,&image->exception);
            else if (image_matte != MagickFalse)
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,RGBAQuantum,png_pixels,&image->exception);
            else
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,RGBQuantum,png_pixels,&image->exception);
            png_write_row(ping,png_pixels);
          }
      else
        /* not ((image_depth > 8) || (mng_info->write_png24 ||
            mng_info->write_png32 ||
            (!mng_info->write_png8 && !mng_info->IsPalette))) */
        {
          if ((ping_info->color_type != PNG_COLOR_TYPE_GRAY) &&
              (ping_info->color_type != PNG_COLOR_TYPE_GRAY_ALPHA))
            {
              if (logging)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  pass %d, Image Is not GRAY or GRAY_ALPHA",pass);
              quantum_info->depth=8;
              image_depth=8;
            }
          for (y=0; y < (long) image->rows; y++)
          {
            if (logging)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  pass %d, Image Is RGB, 16-bit GRAY, or GRAY_ALPHA",pass);
            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,GrayQuantum,png_pixels,&image->exception);
            else if (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,GrayAlphaQuantum,png_pixels,&image->exception);
            else
              (void) ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,IndexQuantum,png_pixels,&image->exception);
            png_write_row(ping,png_pixels);
          }
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,pass,num_passes);
            if (status == MagickFalse)
              break;
          }
     }
  }
  if (quantum_info != (QuantumInfo *) NULL)
    quantum_info=DestroyQuantumInfo(quantum_info);

  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Writing PNG image data");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Width: %lu",ping_info->width);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Height: %lu",ping_info->height);
      if (mng_info->write_png_depth)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Defined PNG:bit-depth: %d",mng_info->write_png_depth);
        }
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG bit-depth written: %d",ping_info->bit_depth);
      if (mng_info->write_png_colortype)
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Defined PNG:color-type: %d",mng_info->write_png_colortype-1);
        }
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG color-type written: %d",ping_info->color_type);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    PNG Interlace method: %d",ping_info->interlace_type);
    }
  /*
    Generate text chunks.
  */
#if (PNG_LIBPNG_VER <= 10005)
  ping_info->num_text=0;
#endif
  ResetImagePropertyIterator(image);
  property=GetNextImageProperty(image);
  while (property != (const char *) NULL)
  {
#if (PNG_LIBPNG_VER > 10005)
    png_textp
      text;
#endif

    value=GetImageProperty(image,property);
    if (value != (const char *) NULL)
      {
#if (PNG_LIBPNG_VER > 10005)
        text=(png_textp) png_malloc(ping,(png_uint_32) sizeof(png_text));
        text[0].key=(char *) property;
        text[0].text=(char *) value;
        text[0].text_length=strlen(value);
        text[0].compression=image_info->compression == NoCompression ||
          (image_info->compression == UndefinedCompression &&
          text[0].text_length < 128) ? -1 : 0;
        if (logging != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Setting up text chunk");
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    keyword: %s",text[0].key);
          }
        png_set_text(ping,ping_info,text,1);
        png_free(ping,text);
#else
/* Work directly with ping_info struct; png_set_text before libpng version
 * 1.0.5a is leaky */
        if (ping_info->num_text == 0)
          {
            ping_info->text=(png_text *) AcquireQuantumMemory(256,
              sizeof(*ping_info->text));
            if (ping_info->text == (png_text *) NULL)
              (void) ThrowMagickException(&image->exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
          }
        i=ping_info->num_text++;
        if (i > 255)
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ResourceLimitError,"Cannot write more than 256 PNG text chunks",
            "`%s'",image->filename);
        ping_info->text[i].key=(char *) property;
        ping_info->text[i].text=(char *) value;
        ping_info->text[i].text_length=strlen(value);
        ping_info->text[i].compression=
          image_info->compression == NoCompression ||
          (image_info->compression == UndefinedCompression &&
          ping_info->text[i].text_length < 128) ? -1 : 0;
        if (logging != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Setting up text chunk");
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    keyword: %s",ping_info->text[i].key);
          }
#endif
      }
    property=GetNextImageProperty(image);
  }

  /* write any PNG-chunk-e profiles */
  (void) png_write_chunk_from_profile(image,"PNG-chunk-e",(int) logging);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Writing PNG end info");
  png_write_end(ping,ping_info);
  if (mng_info->need_fram && (int) image->dispose == BackgroundDispose)
    {
      if (mng_info->page.x || mng_info->page.y ||
          (ping_info->width != mng_info->page.width) ||
          (ping_info->height != mng_info->page.height))
        {
          unsigned char
            chunk[32];

          /*
            Write FRAM 4 with clipping boundaries followed by FRAM 1.
          */
          (void) WriteBlobMSBULong(image,27L);  /* data length=27 */
          PNGType(chunk,mng_FRAM);
          LogPNGChunk((int) logging,mng_FRAM,27L);
          chunk[4]=4;
          chunk[5]=0;  /* frame name separator (no name) */
          chunk[6]=1;  /* flag for changing delay, for next frame only */
          chunk[7]=0;  /* flag for changing frame timeout */
          chunk[8]=1;  /* flag for changing frame clipping for next frame */
          chunk[9]=0;  /* flag for changing frame sync_id */
          PNGLong(chunk+10,(png_uint_32) (0L)); /* temporary 0 delay */
          chunk[14]=0; /* clipping boundaries delta type */
          PNGLong(chunk+15,(png_uint_32) (mng_info->page.x)); /* left cb */
          PNGLong(chunk+19,
             (png_uint_32) (mng_info->page.x + ping_info->width));
          PNGLong(chunk+23,(png_uint_32) (mng_info->page.y)); /* top cb */
          PNGLong(chunk+27,
             (png_uint_32) (mng_info->page.y + ping_info->height));
          (void) WriteBlob(image,31,chunk);
          (void) WriteBlobMSBULong(image,crc32(0,chunk,31));
          mng_info->old_framing_mode=4;
          mng_info->framing_mode=1;
        }
      else
        mng_info->framing_mode=3;
    }
  if (mng_info->write_mng && !mng_info->need_fram &&
      ((int) image->dispose == 3))
     (void) ThrowMagickException(&image->exception,GetMagickModule(),
       CoderError,"Cannot convert GIF with disposal method 3 to MNG-LC",
       "`%s'",image->filename);
  image_depth=save_image_depth;

  /* Save depth actually written */

  s[0]=(char) ping_info->bit_depth;
  s[1]='\0';

  (void) SetImageProperty(image,"png:bit-depth-written",s);

  /*
    Free PNG resources.
  */
#if (PNG_LIBPNG_VER < 10007)
  if (ping_info->valid & PNG_INFO_PLTE)
    {
      ping_info->palette=(png_colorp)
        RelinquishMagickMemory(ping_info->palette);
      ping_info->valid&=(~PNG_INFO_PLTE);
    }
  if (ping_info->valid & PNG_INFO_tRNS)
    {
      ping_info->trans_alpha=(unsigned char *) RelinquishMagickMemory(
        ping_info->trans_alpha);
      ping_info->valid&=(~PNG_INFO_tRNS);
    }
#endif

  png_destroy_write_struct(&ping,&ping_info);

  png_pixels=(unsigned char *) RelinquishMagickMemory(png_pixels);

#if defined(PNG_SETJMP_NOT_THREAD_SAFE)
  UnlockSemaphoreInfo(png_semaphore);
#endif

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit WriteOnePNGImage()");
  return(MagickTrue);
/*  End write one PNG image */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePNGImage() writes a Portable Network Graphics (PNG) or
%  Multiple-image Network Graphics (MNG) image file.
%
%  MNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the WritePNGImage method is:
%
%      MagickBooleanType WritePNGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%  Returns MagickTrue on success, MagickFalse on failure.
%
%  Communicating with the PNG encoder:
%
%  While the datastream written is always in PNG format and normally would
%  be given the "png" file extension, this method also writes the following
%  pseudo-formats which are subsets of PNG:
%
%    o PNG8:    An 8-bit indexed PNG datastream is written.  If transparency
%               is present, the tRNS chunk must only have values 0 and 255
%               (i.e., transparency is binary: fully opaque or fully
%               transparent).  The pixels contain 8-bit indices even if
%               they could be represented with 1, 2, or 4 bits. Note: grayscale
%               images will be written as indexed PNG files even though the
%               PNG grayscale type might be slightly more efficient.
%
%    o PNG24:   An 8-bit per sample RGB PNG datastream is written.  The tRNS
%               chunk can be present to convey binary transparency by naming
%               one of the colors as transparent.
%
%    o PNG32:   An 8-bit per sample RGBA PNG is written.  Partial
%               transparency is permitted, i.e., the alpha sample for
%               each pixel can have any value from 0 to 255. The alpha
%               channel is present even if the image is fully opaque. 
%
%    o -define: For more precise control of the PNG output, you can use the
%               Image options "png:bit-depth" and "png:color-type".  These
%               can be set from the commandline with "-define" and also
%               from the application programming interfaces.
%
%               png:color-type can be 0, 2, 3, 4, or 6.
%
%               When png:color-type is 0 (Grayscale), png:bit-depth can
%               be 1, 2, 4, 8, or 16.
%
%               When png:color-type is 2 (RGB), png:bit-depth can
%               be 8 or 16.
%
%               When png:color-type is 3 (Indexed), png:bit-depth can
%               be 1, 2, 4, or 8.  This refers to the number of bits
%               used to store the index.  The color samples always have
%               bit-depth 8 in indexed PNG files.
%
%               When png:color-type is 4 (Gray-Matte) or 6 (RGB-Matte),
%               png:bit-depth can be 8 or 16.
%
%  If the image cannot be written without loss in the requested PNG8, PNG24,
%  or PNG32 format or with the requested bit-depth and color-type without loss,
%  a PNG file will not be written, and the encoder will return MagickFalse.
%  Since image encoders should not be responsible for the "heavy lifting",
%  the user should make sure that ImageMagick has already reduced the
%  image depth and number of colors and limit transparency to binary
%  transparency prior to attempting to write the image in a format that
%  is subject to depth, color, or transparency limitations.
%
%  TODO: Enforce the previous paragraph.
%
%  TODO: Allow all other PNG subformats to be requested via new
%        "-define png:bit-depth -define png:color-type" options.
%
%  Note that another definition, "png:bit-depth-written" exists, but it
%  is not intended for external use.  It is only used internally by the
%  PNG encoder to inform the JNG encoder of the depth of the alpha channel.
%
%  It is possible to request that the PNG encoder write previously-formatted
%  ancillary chunks in the output PNG file, using the "-profile" commandline
%  option as shown below or by setting the profile via a programming
%  interface:
%
%     -profile PNG-chunk-x:<file>
%
%  where x is a location flag and <file> is a file containing the chunk
%  name in the first 4 bytes, then a colon (":"), followed by the chunk data.
%
%  "x" can be "b" (before PLTE), "m" (middle, i.e., between PLTE and IDAT),
%  or "e" (end, i.e., after IDAT).  If you want to write multiple chunks
%  of the same type, then add a short unique string after the "x" to prevent
%  subsequent profiles from overwriting the preceding ones:
%
%     -profile PNG-chunk-x01:file01 -profile PNG-chunk-x02:file02
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
static MagickBooleanType WritePNGImage(const ImageInfo *image_info,
  Image *image)
{
  MagickBooleanType
    status;

  MngInfo
    *mng_info;

  const char
    *value;

  int
    have_mng_structure;

  unsigned int
    logging;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter WritePNGImage()");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  /*
    Allocate a MngInfo structure.
  */
  have_mng_structure=MagickFalse;
  mng_info=(MngInfo *) AcquireAlignedMemory(1,sizeof(MngInfo));
  if (mng_info == (MngInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize members of the MngInfo structure.
  */
  (void) ResetMagickMemory(mng_info,0,sizeof(MngInfo));
  mng_info->image=image;
  have_mng_structure=MagickTrue;

  /* See if user has requested a specific PNG subformat */

  mng_info->write_png8=LocaleCompare(image_info->magick,"PNG8") == 0;
  mng_info->write_png24=LocaleCompare(image_info->magick,"PNG24") == 0;
  mng_info->write_png32=LocaleCompare(image_info->magick,"PNG32") == 0;

  if (mng_info->write_png8)
    {
         mng_info->write_png_colortype = /* 3 */ 4;
         mng_info->write_png_depth = 8;
         image->depth = 8;
#if 0 /* this does not work */
         if (image->matte == MagickTrue)
            (void) SetImageType(image,PaletteMatteType);
         else
            (void) SetImageType(image,PaletteType);
         (void) SyncImage(image);
#endif
    }

  if (mng_info->write_png24)
    {
         mng_info->write_png_colortype = /* 2 */ 3;
         mng_info->write_png_depth = 8;
         image->depth = 8;
         if (image->matte == MagickTrue)
            (void) SetImageType(image,TrueColorMatteType);
         else
            (void) SetImageType(image,TrueColorType);
         (void) SyncImage(image);
    }

  if (mng_info->write_png32)
    {
         mng_info->write_png_colortype = /* 6 */  7;
         mng_info->write_png_depth = 8;
         image->depth = 8;
         if (image->matte == MagickTrue)
            (void) SetImageType(image,TrueColorMatteType);
         else
            (void) SetImageType(image,TrueColorType);
         (void) SyncImage(image);
    }

  value=GetImageOption(image_info,"png:bit-depth");
  if (value != (char *) NULL)
    {
      if (LocaleCompare(value,"1") == 0)
         mng_info->write_png_depth = 1;
      else if (LocaleCompare(value,"2") == 0)
         mng_info->write_png_depth = 2;
      else if (LocaleCompare(value,"4") == 0)
         mng_info->write_png_depth = 4;
      else if (LocaleCompare(value,"8") == 0)
         mng_info->write_png_depth = 8;
      else if (LocaleCompare(value,"16") == 0)
         mng_info->write_png_depth = 16;
      if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "png:bit-depth=%d was defined.\n",mng_info->write_png_depth);
    }
  value=GetImageOption(image_info,"png:color-type");
  if (value != (char *) NULL)
    {
      /* We must store colortype+1 because 0 is a valid colortype */
      if (LocaleCompare(value,"0") == 0)
         mng_info->write_png_colortype = 1;
      else if (LocaleCompare(value,"2") == 0)
         mng_info->write_png_colortype = 3;
      else if (LocaleCompare(value,"3") == 0)
         mng_info->write_png_colortype = 4;
      else if (LocaleCompare(value,"4") == 0)
         mng_info->write_png_colortype = 5;
      else if (LocaleCompare(value,"6") == 0)
         mng_info->write_png_colortype = 7;
      if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "png:color-type=%d was defined.\n",mng_info->write_png_colortype-1);
    }

  status=WriteOnePNGImage(mng_info,image_info,image);

  (void) CloseBlob(image);

  MngInfoFreeStruct(mng_info,&have_mng_structure);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit WritePNGImage()");
  return(status);
}

#if defined(JNG_SUPPORTED)

/* Write one JNG image */
static MagickBooleanType WriteOneJNGImage(MngInfo *mng_info,
   const ImageInfo *image_info,Image *image)
{
  Image
    *jpeg_image;

  ImageInfo
    *jpeg_image_info;

  MagickBooleanType
    status;

  size_t
    length;

  unsigned char
    *blob,
    chunk[80],
    *p;

  unsigned int
    jng_alpha_compression_method,
    jng_alpha_sample_depth,
    jng_color_type,
    logging,
    transparent;

  unsigned long
    jng_quality;

  logging=LogMagickEvent(CoderEvent,GetMagickModule(),
    "  enter WriteOneJNGImage()");

  blob=(unsigned char *) NULL;
  jpeg_image=(Image *) NULL;
  jpeg_image_info=(ImageInfo *) NULL;

  status=MagickTrue;
  transparent=image_info->type==GrayscaleMatteType ||
     image_info->type==TrueColorMatteType;
  jng_color_type=10;
  jng_alpha_sample_depth=0;
  jng_quality=image_info->quality == 0UL ? 75UL : image_info->quality;
  jng_alpha_compression_method=0;

  if (image->matte != MagickFalse)
    {
      /* if any pixels are transparent */
      transparent=MagickTrue;
      if (image_info->compression==JPEGCompression)
        jng_alpha_compression_method=8;
    }

  if (transparent)
    {
      jng_color_type=14;
      /* Create JPEG blob, image, and image_info */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Creating jpeg_image_info for opacity.");
      jpeg_image_info=(ImageInfo *) CloneImageInfo(image_info);
      if (jpeg_image_info == (ImageInfo *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Creating jpeg_image.");
      jpeg_image=CloneImage(image,0,0,MagickTrue,&image->exception);
      if (jpeg_image == (Image *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      (void) CopyMagickString(jpeg_image->magick,"JPEG",MaxTextExtent);
      status=SeparateImageChannel(jpeg_image,OpacityChannel);
      status=NegateImage(jpeg_image,MagickFalse);
      jpeg_image->matte=MagickFalse;
      if (jng_quality >= 1000)
        jpeg_image_info->quality=jng_quality/1000;
      else
        jpeg_image_info->quality=jng_quality;
      jpeg_image_info->type=GrayscaleType;
      (void) SetImageType(jpeg_image,GrayscaleType);
      (void) AcquireUniqueFilename(jpeg_image->filename);
      (void) FormatMagickString(jpeg_image_info->filename,MaxTextExtent,
        "%s",jpeg_image->filename);
    }

  /* To do: check bit depth of PNG alpha channel */

  /* Check if image is grayscale. */
  if (image_info->type != TrueColorMatteType && image_info->type !=
    TrueColorType && ImageIsGray(image))
    jng_color_type-=2;

  if (transparent)
    {
      if (jng_alpha_compression_method==0)
        {
          const char
            *value;

          /* Encode opacity as a grayscale PNG blob */
          status=OpenBlob(jpeg_image_info,jpeg_image,WriteBinaryBlobMode,
            &image->exception);
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Creating PNG blob.");
          length=0;

          (void) CopyMagickString(jpeg_image_info->magick,"PNG",MaxTextExtent);
          (void) CopyMagickString(jpeg_image->magick,"PNG",MaxTextExtent);
          jpeg_image_info->interlace=NoInterlace;

          blob=ImageToBlob(jpeg_image_info,jpeg_image,&length,
            &image->exception);

          /* Retrieve sample depth used */
          value=GetImageProperty(jpeg_image,"png:bit-depth-written");
          if (value != (char *) NULL)
            jng_alpha_sample_depth= (unsigned int) value[0];
        }
      else
        {
          /* Encode opacity as a grayscale JPEG blob */

          status=OpenBlob(jpeg_image_info,jpeg_image,WriteBinaryBlobMode,
            &image->exception);

          (void) CopyMagickString(jpeg_image_info->magick,"JPEG",MaxTextExtent);
          (void) CopyMagickString(jpeg_image->magick,"JPEG",MaxTextExtent);
          jpeg_image_info->interlace=NoInterlace;
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Creating blob.");
          blob=ImageToBlob(jpeg_image_info,jpeg_image,&length,
             &image->exception);
          jng_alpha_sample_depth=8;
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Successfully read jpeg_image into a blob, length=%lu.",
              (unsigned long) length);

        }
      /* Destroy JPEG image and image_info */
      jpeg_image=DestroyImage(jpeg_image);
      (void) RelinquishUniqueFileResource(jpeg_image_info->filename);
      jpeg_image_info=DestroyImageInfo(jpeg_image_info);
    }

  /* Write JHDR chunk */
  (void) WriteBlobMSBULong(image,16L);  /* chunk data length=16 */
  PNGType(chunk,mng_JHDR);
  LogPNGChunk((int) logging,mng_JHDR,16L);
  PNGLong(chunk+4,image->columns);
  PNGLong(chunk+8,image->rows);
  chunk[12]=jng_color_type;
  chunk[13]=8;  /* sample depth */
  chunk[14]=8; /*jng_image_compression_method */
  chunk[15]=(unsigned char) (image_info->interlace == NoInterlace ? 0 : 8);
  chunk[16]=jng_alpha_sample_depth;
  chunk[17]=jng_alpha_compression_method;
  chunk[18]=0; /*jng_alpha_filter_method */
  chunk[19]=0; /*jng_alpha_interlace_method */
  (void) WriteBlob(image,20,chunk);
  (void) WriteBlobMSBULong(image,crc32(0,chunk,20));
  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG width:%15lu",image->columns);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG height:%14lu",image->rows);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG color type:%10d",jng_color_type);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG sample depth:%8d",8);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG compression:%9d",8);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG interlace:%11d",0);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha depth:%9d",jng_alpha_sample_depth);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha compression:%3d",jng_alpha_compression_method);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha filter:%8d",0);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    JNG alpha interlace:%5d",0);
    }

  /* Write any JNG-chunk-b profiles */ 
  (void) png_write_chunk_from_profile(image,"JNG-chunk-b",(int) logging);

  /*
     Write leading ancillary chunks
  */

  if (transparent)
  {
    /*
      Write JNG bKGD chunk
    */

    unsigned char
      blue,
      green,
      red;

    long
      num_bytes;

    if (jng_color_type == 8 || jng_color_type == 12)
      num_bytes=6L;
    else
      num_bytes=10L;
    (void) WriteBlobMSBULong(image,(unsigned long) (num_bytes-4L));
    PNGType(chunk,mng_bKGD);
    LogPNGChunk((int) logging,mng_bKGD,(unsigned long) (num_bytes-4L));
    red=ScaleQuantumToChar(image->background_color.red);
    green=ScaleQuantumToChar(image->background_color.green);
    blue=ScaleQuantumToChar(image->background_color.blue);
    *(chunk+4)=0;
    *(chunk+5)=red;
    *(chunk+6)=0;
    *(chunk+7)=green;
    *(chunk+8)=0;
    *(chunk+9)=blue;
    (void) WriteBlob(image,(size_t) num_bytes,chunk);
    (void) WriteBlobMSBULong(image,crc32(0,chunk,(uInt) num_bytes));
  }

  if ((image->colorspace == sRGBColorspace || image->rendering_intent))
    {
      /*
        Write JNG sRGB chunk
      */
      (void) WriteBlobMSBULong(image,1L);
      PNGType(chunk,mng_sRGB);
      LogPNGChunk((int) logging,mng_sRGB,1L);
      if (image->rendering_intent != UndefinedIntent)
        chunk[4]=(unsigned char) (image->rendering_intent-1);
      else
        chunk[4]=(unsigned char) (PerceptualIntent-1);
      (void) WriteBlob(image,5,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,5));
    }
  else
    {
      if (image->gamma != 0.0)
        {
          /*
             Write JNG gAMA chunk
          */
          (void) WriteBlobMSBULong(image,4L);
          PNGType(chunk,mng_gAMA);
          LogPNGChunk((int) logging,mng_gAMA,4L);
          PNGLong(chunk+4,(unsigned long) (100000*image->gamma+0.5));
          (void) WriteBlob(image,8,chunk);
          (void) WriteBlobMSBULong(image,crc32(0,chunk,8));
        }
      if ((mng_info->equal_chrms == MagickFalse) &&
          (image->chromaticity.red_primary.x != 0.0))
        {
          PrimaryInfo
            primary;

          /*
             Write JNG cHRM chunk
          */
          (void) WriteBlobMSBULong(image,32L);
          PNGType(chunk,mng_cHRM);
          LogPNGChunk((int) logging,mng_cHRM,32L);
          primary=image->chromaticity.white_point;
          PNGLong(chunk+4,(unsigned long) (100000*primary.x+0.5));
          PNGLong(chunk+8,(unsigned long) (100000*primary.y+0.5));
          primary=image->chromaticity.red_primary;
          PNGLong(chunk+12,(unsigned long) (100000*primary.x+0.5));
          PNGLong(chunk+16,(unsigned long) (100000*primary.y+0.5));
          primary=image->chromaticity.green_primary;
          PNGLong(chunk+20,(unsigned long) (100000*primary.x+0.5));
          PNGLong(chunk+24,(unsigned long) (100000*primary.y+0.5));
          primary=image->chromaticity.blue_primary;
          PNGLong(chunk+28,(unsigned long) (100000*primary.x+0.5));
          PNGLong(chunk+32,(unsigned long) (100000*primary.y+0.5));
          (void) WriteBlob(image,36,chunk);
          (void) WriteBlobMSBULong(image,crc32(0,chunk,36));
        }
    }
  if (image->x_resolution && image->y_resolution && !mng_info->equal_physs)
    {
      /*
         Write JNG pHYs chunk
      */
      (void) WriteBlobMSBULong(image,9L);
      PNGType(chunk,mng_pHYs);
      LogPNGChunk((int) logging,mng_pHYs,9L);
      if (image->units == PixelsPerInchResolution)
        {
          PNGLong(chunk+4,(unsigned long)
            (image->x_resolution*100.0/2.54+0.5));
          PNGLong(chunk+8,(unsigned long)
            (image->y_resolution*100.0/2.54+0.5));
          chunk[12]=1;
        }
      else
        {
          if (image->units == PixelsPerCentimeterResolution)
            {
              PNGLong(chunk+4,(unsigned long)
                (image->x_resolution*100.0+0.5));
              PNGLong(chunk+8,(unsigned long)
                (image->y_resolution*100.0+0.5));
              chunk[12]=1;
            }
          else
            {
              PNGLong(chunk+4,(unsigned long) (image->x_resolution+0.5));
              PNGLong(chunk+8,(unsigned long) (image->y_resolution+0.5));
              chunk[12]=0;
            }
        }
      (void) WriteBlob(image,13,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
    }

  if (mng_info->write_mng == 0 && (image->page.x || image->page.y))
    {
      /*
         Write JNG oFFs chunk
      */
      (void) WriteBlobMSBULong(image,9L);
      PNGType(chunk,mng_oFFs);
      LogPNGChunk((int) logging,mng_oFFs,9L);
      PNGsLong(chunk+4,(long) (image->page.x));
      PNGsLong(chunk+8,(long) (image->page.y));
      chunk[12]=0;
      (void) WriteBlob(image,13,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
    }
  if (mng_info->write_mng == 0 && (image->page.width || image->page.height))
    {
       (void) WriteBlobMSBULong(image,9L);  /* data length=8 */
       PNGType(chunk,mng_vpAg);
       LogPNGChunk((int) logging,mng_vpAg,9L);
       PNGLong(chunk+4,(png_uint_32) image->page.width);
       PNGLong(chunk+8,(png_uint_32) image->page.height);
       chunk[12]=0;   /* unit = pixels */
       (void) WriteBlob(image,13,chunk);
       (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
    }


  if (transparent)
    {
      if (jng_alpha_compression_method==0)
        {
          register long
            i;

          long
            len;

          /* Write IDAT chunk header */
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Write IDAT chunks from blob, length=%lu.",
              (unsigned long) length);

          /* Copy IDAT chunks */
          len=0;
          p=blob+8;
          for (i=8; i<(long) length; i+=len+12)
          {
            len=(*p<<24)|((*(p+1))<<16)|((*(p+2))<<8)|(*(p+3));
            p+=4;
            if (*(p)==73 && *(p+1)==68 && *(p+2)==65 && *(p+3)==84) /* IDAT */
              {
                /* Found an IDAT chunk. */
                (void) WriteBlobMSBULong(image,(unsigned long) len);
                LogPNGChunk((int) logging,mng_IDAT,(unsigned long) len);
                (void) WriteBlob(image,(size_t) len+4,p);
                (void) WriteBlobMSBULong(image,
                    crc32(0,p,(uInt) len+4));
              }
            else
              {
                if (logging != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "    Skipping %c%c%c%c chunk, length=%lu.",
                    *(p),*(p+1),*(p+2),*(p+3),len);
              }
            p+=(8+len);
          }
        }
      else
        {
          /* Write JDAA chunk header */
          if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Write JDAA chunk, length=%lu.",
              (unsigned long) length);
          (void) WriteBlobMSBULong(image,(unsigned long) length);
          PNGType(chunk,mng_JDAA);
          LogPNGChunk((int) logging,mng_JDAA,length);
          /* Write JDAT chunk(s) data */
          (void) WriteBlob(image,4,chunk);
          (void) WriteBlob(image,length,blob);
          (void) WriteBlobMSBULong(image,crc32(crc32(0,chunk,4),blob,
             (uInt) length));
        }
      blob=(unsigned char *) RelinquishMagickMemory(blob);
    }

  /* Encode image as a JPEG blob */
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Creating jpeg_image_info.");
  jpeg_image_info=(ImageInfo *) CloneImageInfo(image_info);
  if (jpeg_image_info == (ImageInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Creating jpeg_image.");

  jpeg_image=CloneImage(image,0,0,MagickTrue,&image->exception);
  if (jpeg_image == (Image *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) CopyMagickString(jpeg_image->magick,"JPEG",MaxTextExtent);

  (void) AcquireUniqueFilename(jpeg_image->filename);
  (void) FormatMagickString(jpeg_image_info->filename,MaxTextExtent,"%s",
    jpeg_image->filename);

  status=OpenBlob(jpeg_image_info,jpeg_image,WriteBinaryBlobMode,
    &image->exception);

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Created jpeg_image, %lu x %lu.",jpeg_image->columns,
      jpeg_image->rows);

  if (jng_color_type == 8 || jng_color_type == 12)
    jpeg_image_info->type=GrayscaleType;
  jpeg_image_info->quality=jng_quality % 1000;
  (void) CopyMagickString(jpeg_image_info->magick,"JPEG",MaxTextExtent);
  (void) CopyMagickString(jpeg_image->magick,"JPEG",MaxTextExtent);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  Creating blob.");
  blob=ImageToBlob(jpeg_image_info,jpeg_image,&length,&image->exception);
  if (logging != MagickFalse)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Successfully read jpeg_image into a blob, length=%lu.",
        (unsigned long) length);

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Write JDAT chunk, length=%lu.",
        (unsigned long) length);
    }
  /* Write JDAT chunk(s) */
  (void) WriteBlobMSBULong(image,(unsigned long) length);
  PNGType(chunk,mng_JDAT);
  LogPNGChunk((int) logging,mng_JDAT,length);
  (void) WriteBlob(image,4,chunk);
  (void) WriteBlob(image,length,blob);
  (void) WriteBlobMSBULong(image,crc32(crc32(0,chunk,4),blob,(uInt) length));

  jpeg_image=DestroyImage(jpeg_image);
  (void) RelinquishUniqueFileResource(jpeg_image_info->filename);
  jpeg_image_info=DestroyImageInfo(jpeg_image_info);
  blob=(unsigned char *) RelinquishMagickMemory(blob);

  /* Write any JNG-chunk-e profiles */
  (void) png_write_chunk_from_profile(image,"JNG-chunk-e",(int) logging);

  /* Write IEND chunk */
  (void) WriteBlobMSBULong(image,0L);
  PNGType(chunk,mng_IEND);
  LogPNGChunk((int) logging,mng_IEND,0);
  (void) WriteBlob(image,4,chunk);
  (void) WriteBlobMSBULong(image,crc32(0,chunk,4));

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit WriteOneJNGImage()");
  return(status);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e J N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJNGImage() writes a JPEG Network Graphics (JNG) image file.
%
%  JNG support written by Glenn Randers-Pehrson, glennrp@image...
%
%  The format of the WriteJNGImage method is:
%
%      MagickBooleanType WriteJNGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
static MagickBooleanType WriteJNGImage(const ImageInfo *image_info,Image *image)
{
  MagickBooleanType
    status;

  MngInfo
    *mng_info;

  int
    have_mng_structure;

  unsigned int
    logging;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter WriteJNGImage()");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);

  /*
    Allocate a MngInfo structure.
  */
  have_mng_structure=MagickFalse;
  mng_info=(MngInfo *) AcquireAlignedMemory(1,sizeof(MngInfo));
  if (mng_info == (MngInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize members of the MngInfo structure.
  */
  (void) ResetMagickMemory(mng_info,0,sizeof(MngInfo));
  mng_info->image=image;
  have_mng_structure=MagickTrue;

  (void) WriteBlob(image,8,(const unsigned char *) "\213JNG\r\n\032\n");

  status=WriteOneJNGImage(mng_info,image_info,image);
  (void) CloseBlob(image);

  (void) CatchImageException(image);
  MngInfoFreeStruct(mng_info,&have_mng_structure);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit WriteJNGImage()");
  return(status);
}
#endif



static MagickBooleanType WriteMNGImage(const ImageInfo *image_info,Image *image)
{
  const char
    *option;

  Image
    *next_image;

  MagickBooleanType
    status;

  MngInfo
    *mng_info;

  int
    have_mng_structure,
    image_count,
    need_iterations,
    need_matte;

  volatile int
#if defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_MNG_FEATURES_SUPPORTED)
    need_local_plte,
#endif
    all_images_are_gray,
    logging,
    need_defi,
    optimize,
    use_global_plte;

  register long
    i;

  unsigned char
    chunk[800];

  volatile unsigned int
    write_jng,
    write_mng;

  volatile unsigned long
    scene;

  unsigned long
    final_delay=0,
    initial_delay;

#if (PNG_LIBPNG_VER < 10007)
    if (image_info->verbose)
      printf("Your PNG library (libpng-%s) is rather old.\n",
         PNG_LIBPNG_VER_STRING);
#endif

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter WriteMNGImage()");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);

  /*
    Allocate a MngInfo structure.
  */
  have_mng_structure=MagickFalse;
  mng_info=(MngInfo *) AcquireAlignedMemory(1,sizeof(MngInfo));
  if (mng_info == (MngInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize members of the MngInfo structure.
  */
  (void) ResetMagickMemory(mng_info,0,sizeof(MngInfo));
  mng_info->image=image;
  have_mng_structure=MagickTrue;
  write_mng=LocaleCompare(image_info->magick,"MNG") == 0;

  /*
   * See if user has requested a specific PNG subformat to be used
   * for all of the PNGs in the MNG being written, e.g.,
   *
   *    convert *.png png8:animation.mng
   *
   * To do: check -define png:bit_depth and png:color_type as well,
   * or perhaps use mng:bit_depth and mng:color_type instead for
   * global settings.
   */

  mng_info->write_png8=LocaleCompare(image_info->magick,"PNG8") == 0;
  mng_info->write_png24=LocaleCompare(image_info->magick,"PNG24") == 0;
  mng_info->write_png32=LocaleCompare(image_info->magick,"PNG32") == 0;

  write_jng=MagickFalse;
  if (image_info->compression == JPEGCompression)
    write_jng=MagickTrue;

  mng_info->adjoin=image_info->adjoin &&
    (GetNextImageInList(image) != (Image *) NULL) && write_mng;

  if (mng_info->write_png8 || mng_info->write_png24 || mng_info->write_png32)
    optimize=MagickFalse;
  else
    optimize=(image_info->type == OptimizeType || image_info->type ==
      UndefinedType);

  if (logging != MagickFalse)
    {
      /* Log some info about the input */
      Image
        *p;

      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Checking input image(s)");
      if (optimize)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Optimize: TRUE");
      else
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Optimize: FALSE");
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Image_info depth: %ld",image_info->depth);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "    Type: %d",image_info->type);

      scene=0;
      for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Scene: %ld",scene++);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "      Image depth: %lu",p->depth);
        if (p->matte)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Matte: True");
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Matte: False");
        if (p->storage_class == PseudoClass)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Storage class: PseudoClass");
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Storage class: DirectClass");
        if (p->colors)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Number of colors: %lu",p->colors);
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      Number of colors: unspecified");
        if (mng_info->adjoin == MagickFalse)
          break;
      }
    }

  /*
    Sometimes we get PseudoClass images whose RGB values don't match
    the colors in the colormap.  This code syncs the RGB values.
  */
  {
    Image
      *p;

    for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
    {
      if (p->taint && p->storage_class == PseudoClass)
         (void) SyncImage(p);
      if (mng_info->adjoin == MagickFalse)
        break;
    }
  }

#ifdef PNG_BUILD_PALETTE
  if (optimize)
    {
      /*
        Sometimes we get DirectClass images that have 256 colors or fewer.
        This code will convert them to PseudoClass and build a colormap.
      */
      Image
        *p;

      for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
      {
        if (p->storage_class != PseudoClass)
          {
            p->colors=GetNumberColors(p,(FILE *) NULL,&p->exception);
            if (p->colors <= 256)
              {
                p->colors=0;
                if (p->matte != MagickFalse)
                    (void) SetImageType(p,PaletteMatteType);
                else
                    (void) SetImageType(p,PaletteType);
              }
          }
        if (mng_info->adjoin == MagickFalse)
          break;
      }
    }
#endif

  use_global_plte=MagickFalse;
  all_images_are_gray=MagickFalse;
#ifdef PNG_WRITE_EMPTY_PLTE_SUPPORTED
  need_local_plte=MagickTrue;
#endif
  need_defi=MagickFalse;
  need_matte=MagickFalse;
  mng_info->framing_mode=1;
  mng_info->old_framing_mode=1;

  if (write_mng)
      if (image_info->page != (char *) NULL)
        {
          /*
            Determine image bounding box.
          */
          SetGeometry(image,&mng_info->page);
          (void) ParseMetaGeometry(image_info->page,&mng_info->page.x,
            &mng_info->page.y,&mng_info->page.width,&mng_info->page.height);
        }
  if (write_mng)
    {
      unsigned int
        need_geom;

      unsigned short
        red,
        green,
        blue;

      mng_info->page=image->page;
      need_geom=MagickTrue;
      if (mng_info->page.width || mng_info->page.height)
         need_geom=MagickFalse;
      /*
        Check all the scenes.
      */
      initial_delay=image->delay;
      need_iterations=MagickFalse;
      mng_info->equal_chrms=image->chromaticity.red_primary.x != 0.0;
      mng_info->equal_physs=MagickTrue,
      mng_info->equal_gammas=MagickTrue;
      mng_info->equal_srgbs=MagickTrue;
      mng_info->equal_backgrounds=MagickTrue;
      image_count=0;
#if defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_MNG_FEATURES_SUPPORTED)
      all_images_are_gray=MagickTrue;
      mng_info->equal_palettes=MagickFalse;
      need_local_plte=MagickFalse;
#endif
      for (next_image=image; next_image != (Image *) NULL; )
      {
        if (need_geom)
          {
            if ((next_image->columns+next_image->page.x) > mng_info->page.width)
              mng_info->page.width=next_image->columns+next_image->page.x;
            if ((next_image->rows+next_image->page.y) > mng_info->page.height)
              mng_info->page.height=next_image->rows+next_image->page.y;
          }
        if (next_image->page.x || next_image->page.y)
          need_defi=MagickTrue;
        if (next_image->matte)
          need_matte=MagickTrue;
        if ((int) next_image->dispose >= BackgroundDispose)
          if (next_image->matte || next_image->page.x || next_image->page.y ||
              ((next_image->columns < mng_info->page.width) &&
               (next_image->rows < mng_info->page.height)))
            mng_info->need_fram=MagickTrue;
        if (next_image->iterations)
          need_iterations=MagickTrue;
        final_delay=next_image->delay;
        if (final_delay != initial_delay || final_delay > 1UL*
           next_image->ticks_per_second)
          mng_info->need_fram=1;
#if defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_MNG_FEATURES_SUPPORTED)
        /*
          check for global palette possibility.
        */
        if (image->matte != MagickFalse)
           need_local_plte=MagickTrue;
        if (need_local_plte == 0)
          {
            if (ImageIsGray(image) == MagickFalse)
              all_images_are_gray=MagickFalse;
            mng_info->equal_palettes=PalettesAreEqual(image,next_image);
            if (use_global_plte == 0)
              use_global_plte=mng_info->equal_palettes;
            need_local_plte=!mng_info->equal_palettes;
          }
#endif
        if (GetNextImageInList(next_image) != (Image *) NULL)
          {
            if (next_image->background_color.red !=
                next_image->next->background_color.red ||
                next_image->background_color.green !=
                next_image->next->background_color.green ||
                next_image->background_color.blue !=
                next_image->next->background_color.blue)
              mng_info->equal_backgrounds=MagickFalse;
            if (next_image->gamma != next_image->next->gamma)
              mng_info->equal_gammas=MagickFalse;
            if (next_image->rendering_intent !=
                next_image->next->rendering_intent)
              mng_info->equal_srgbs=MagickFalse;
            if ((next_image->units != next_image->next->units) ||
                (next_image->x_resolution != next_image->next->x_resolution) ||
                (next_image->y_resolution != next_image->next->y_resolution))
              mng_info->equal_physs=MagickFalse;
            if (mng_info->equal_chrms)
              {
                if (next_image->chromaticity.red_primary.x !=
                    next_image->next->chromaticity.red_primary.x ||
                    next_image->chromaticity.red_primary.y !=
                    next_image->next->chromaticity.red_primary.y ||
                    next_image->chromaticity.green_primary.x !=
                    next_image->next->chromaticity.green_primary.x ||
                    next_image->chromaticity.green_primary.y !=
                    next_image->next->chromaticity.green_primary.y ||
                    next_image->chromaticity.blue_primary.x !=
                    next_image->next->chromaticity.blue_primary.x ||
                    next_image->chromaticity.blue_primary.y !=
                    next_image->next->chromaticity.blue_primary.y ||
                    next_image->chromaticity.white_point.x !=
                    next_image->next->chromaticity.white_point.x ||
                    next_image->chromaticity.white_point.y !=
                    next_image->next->chromaticity.white_point.y)
                  mng_info->equal_chrms=MagickFalse;
              }
          }
        image_count++;
        next_image=GetNextImageInList(next_image);
      }
      if (image_count < 2)
        {
          mng_info->equal_backgrounds=MagickFalse;
          mng_info->equal_chrms=MagickFalse;
          mng_info->equal_gammas=MagickFalse;
          mng_info->equal_srgbs=MagickFalse;
          mng_info->equal_physs=MagickFalse;
          use_global_plte=MagickFalse;
#ifdef PNG_WRITE_EMPTY_PLTE_SUPPORTED
          need_local_plte=MagickTrue;
#endif
          need_iterations=MagickFalse;
        }
     if (mng_info->need_fram == MagickFalse)
       {
         /*
           Only certain framing rates 100/n are exactly representable without
           the FRAM chunk but we'll allow some slop in VLC files
         */
         if (final_delay == 0)
           {
             if (need_iterations != MagickFalse)
               {
                 /*
                   It's probably a GIF with loop; don't run it *too* fast.
                 */
                 final_delay=10;
                 (void) ThrowMagickException(&image->exception,
                    GetMagickModule(),CoderError,
                   "input has zero delay between all frames; assuming 10 cs",
                   "`%s'","");
               }
             else
               mng_info->ticks_per_second=0;
           }
         if (final_delay != 0)
           mng_info->ticks_per_second=1UL*image->ticks_per_second/final_delay;
         if (final_delay > 50)
           mng_info->ticks_per_second=2;
         if (final_delay > 75)
           mng_info->ticks_per_second=1;
         if (final_delay > 125)
           mng_info->need_fram=MagickTrue;
         if (need_defi && final_delay > 2 && (final_delay != 4) &&
            (final_delay != 5) && (final_delay != 10) && (final_delay != 20) &&
            (final_delay != 25) && (final_delay != 50) && (final_delay !=
               1UL*image->ticks_per_second))
           mng_info->need_fram=MagickTrue;  /* make it exact; cannot be VLC */
       }
     if (mng_info->need_fram != MagickFalse)
        mng_info->ticks_per_second=1UL*image->ticks_per_second;
     /*
        If pseudocolor, we should also check to see if all the
        palettes are identical and write a global PLTE if they are.
        ../glennrp Feb 99.
     */
     /*
        Write the MNG version 1.0 signature and MHDR chunk.
     */
     (void) WriteBlob(image,8,(const unsigned char *) "\212MNG\r\n\032\n");
     (void) WriteBlobMSBULong(image,28L);  /* chunk data length=28 */
     PNGType(chunk,mng_MHDR);
     LogPNGChunk((int) logging,mng_MHDR,28L);
     PNGLong(chunk+4,mng_info->page.width);
     PNGLong(chunk+8,mng_info->page.height);
     PNGLong(chunk+12,mng_info->ticks_per_second);
     PNGLong(chunk+16,0L);  /* layer count=unknown */
     PNGLong(chunk+20,0L);  /* frame count=unknown */
     PNGLong(chunk+24,0L);  /* play time=unknown   */
     if (write_jng)
       {
         if (need_matte)
           {
             if (need_defi || mng_info->need_fram || use_global_plte)
               PNGLong(chunk+28,27L);    /* simplicity=LC+JNG */
             else
               PNGLong(chunk+28,25L);    /* simplicity=VLC+JNG */
           }
         else
           {
             if (need_defi || mng_info->need_fram || use_global_plte)
               PNGLong(chunk+28,19L);  /* simplicity=LC+JNG, no transparency */
             else
               PNGLong(chunk+28,17L);  /* simplicity=VLC+JNG, no transparency */
           }
       }
     else
       {
         if (need_matte)
           {
             if (need_defi || mng_info->need_fram || use_global_plte)
               PNGLong(chunk+28,11L);    /* simplicity=LC */
             else
               PNGLong(chunk+28,9L);    /* simplicity=VLC */
           }
         else
           {
             if (need_defi || mng_info->need_fram || use_global_plte)
               PNGLong(chunk+28,3L);    /* simplicity=LC, no transparency */
             else
               PNGLong(chunk+28,1L);    /* simplicity=VLC, no transparency */
           }
       }
     (void) WriteBlob(image,32,chunk);
     (void) WriteBlobMSBULong(image,crc32(0,chunk,32));
     option=GetImageOption(image_info,"mng:need-cacheoff");
     if (option != (const char *) NULL)
       {
         size_t
           length;

         /*
           Write "nEED CACHEOFF" to turn playback caching off for streaming MNG.
         */
         PNGType(chunk,mng_nEED);
         length=CopyMagickString((char *) chunk+4,"CACHEOFF",20);
         (void) WriteBlobMSBULong(image,(unsigned long) length);
         LogPNGChunk((int) logging,mng_nEED,(unsigned long) length);
         length+=4;
         (void) WriteBlob(image,length,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,(uInt) length));
       }
     if ((GetPreviousImageInList(image) == (Image *) NULL) &&
         (GetNextImageInList(image) != (Image *) NULL) &&
         (image->iterations != 1))
       {
         /*
           Write MNG TERM chunk
         */
         (void) WriteBlobMSBULong(image,10L);  /* data length=10 */
         PNGType(chunk,mng_TERM);
         LogPNGChunk((int) logging,mng_TERM,10L);
         chunk[4]=3;  /* repeat animation */
         chunk[5]=0;  /* show last frame when done */
         PNGLong(chunk+6,(png_uint_32) (mng_info->ticks_per_second*
            final_delay/MagickMax(image->ticks_per_second,1)));
         if (image->iterations == 0)
           PNGLong(chunk+10,PNG_UINT_31_MAX);
         else
           PNGLong(chunk+10,(png_uint_32) image->iterations);
         if (logging != MagickFalse)
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "     TERM delay: %lu",
               (png_uint_32) (mng_info->ticks_per_second*
                  final_delay/MagickMax(image->ticks_per_second,1)));
             if (image->iterations == 0)
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "     TERM iterations: %lu",PNG_UINT_31_MAX);
             else
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "     Image iterations: %lu",image->iterations);
           }
         (void) WriteBlob(image,14,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,14));
       }
     /*
       To do: check for cHRM+gAMA == sRGB, and write sRGB instead.
     */
     if ((image->colorspace == sRGBColorspace || image->rendering_intent) &&
          mng_info->equal_srgbs)
       {
         /*
           Write MNG sRGB chunk
         */
         (void) WriteBlobMSBULong(image,1L);
         PNGType(chunk,mng_sRGB);
         LogPNGChunk((int) logging,mng_sRGB,1L);
         if (image->rendering_intent != UndefinedIntent)
           chunk[4]=(unsigned char) (image->rendering_intent-1);
         else
           chunk[4]=(unsigned char) (PerceptualIntent-1);
         (void) WriteBlob(image,5,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,5));
         mng_info->have_write_global_srgb=MagickTrue;
       }
     else
       {
         if (image->gamma && mng_info->equal_gammas)
           {
             /*
                Write MNG gAMA chunk
             */
             (void) WriteBlobMSBULong(image,4L);
             PNGType(chunk,mng_gAMA);
             LogPNGChunk((int) logging,mng_gAMA,4L);
             PNGLong(chunk+4,(unsigned long) (100000*image->gamma+0.5));
             (void) WriteBlob(image,8,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,8));
             mng_info->have_write_global_gama=MagickTrue;
           }
         if (mng_info->equal_chrms)
           {
             PrimaryInfo
               primary;

             /*
                Write MNG cHRM chunk
             */
             (void) WriteBlobMSBULong(image,32L);
             PNGType(chunk,mng_cHRM);
             LogPNGChunk((int) logging,mng_cHRM,32L);
             primary=image->chromaticity.white_point;
             PNGLong(chunk+4,(unsigned long) (100000*primary.x+0.5));
             PNGLong(chunk+8,(unsigned long) (100000*primary.y+0.5));
             primary=image->chromaticity.red_primary;
             PNGLong(chunk+12,(unsigned long) (100000*primary.x+0.5));
             PNGLong(chunk+16,(unsigned long) (100000*primary.y+0.5));
             primary=image->chromaticity.green_primary;
             PNGLong(chunk+20,(unsigned long) (100000*primary.x+0.5));
             PNGLong(chunk+24,(unsigned long) (100000*primary.y+0.5));
             primary=image->chromaticity.blue_primary;
             PNGLong(chunk+28,(unsigned long) (100000*primary.x+0.5));
             PNGLong(chunk+32,(unsigned long) (100000*primary.y+0.5));
             (void) WriteBlob(image,36,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,36));
             mng_info->have_write_global_chrm=MagickTrue;
           }
       }
     if (image->x_resolution && image->y_resolution && mng_info->equal_physs)
       {
         /*
            Write MNG pHYs chunk
         */
         (void) WriteBlobMSBULong(image,9L);
         PNGType(chunk,mng_pHYs);
         LogPNGChunk((int) logging,mng_pHYs,9L);
         if (image->units == PixelsPerInchResolution)
           {
             PNGLong(chunk+4,(unsigned long)
               (image->x_resolution*100.0/2.54+0.5));
             PNGLong(chunk+8,(unsigned long)
               (image->y_resolution*100.0/2.54+0.5));
             chunk[12]=1;
           }
         else
           {
             if (image->units == PixelsPerCentimeterResolution)
               {
                 PNGLong(chunk+4,(unsigned long)
                   (image->x_resolution*100.0+0.5));
                 PNGLong(chunk+8,(unsigned long)
                   (image->y_resolution*100.0+0.5));
                 chunk[12]=1;
               }
             else
               {
                 PNGLong(chunk+4,(unsigned long) (image->x_resolution+0.5));
                 PNGLong(chunk+8,(unsigned long) (image->y_resolution+0.5));
                 chunk[12]=0;
               }
           }
         (void) WriteBlob(image,13,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,13));
       }
     /*
       Write MNG BACK chunk and global bKGD chunk, if the image is transparent
       or does not cover the entire frame.
     */
     if (write_mng && (image->matte || image->page.x > 0 ||
         image->page.y > 0 || (image->page.width &&
         (image->page.width+image->page.x < mng_info->page.width))
         || (image->page.height && (image->page.height+image->page.y
         < mng_info->page.height))))
       {
         (void) WriteBlobMSBULong(image,6L);
         PNGType(chunk,mng_BACK);
         LogPNGChunk((int) logging,mng_BACK,6L);
         red=ScaleQuantumToShort(image->background_color.red);
         green=ScaleQuantumToShort(image->background_color.green);
         blue=ScaleQuantumToShort(image->background_color.blue);
         PNGShort(chunk+4,red);
         PNGShort(chunk+6,green);
         PNGShort(chunk+8,blue);
         (void) WriteBlob(image,10,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,10));
         if (mng_info->equal_backgrounds)
           {
             (void) WriteBlobMSBULong(image,6L);
             PNGType(chunk,mng_bKGD);
             LogPNGChunk((int) logging,mng_bKGD,6L);
             (void) WriteBlob(image,10,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,10));
           }
       }

#ifdef PNG_WRITE_EMPTY_PLTE_SUPPORTED
     if ((need_local_plte == MagickFalse) &&
         (image->storage_class == PseudoClass) &&
         (all_images_are_gray == MagickFalse))
       {
         unsigned long
           data_length;

         /*
           Write MNG PLTE chunk
         */
         data_length=3*image->colors;
         (void) WriteBlobMSBULong(image,data_length);
         PNGType(chunk,mng_PLTE);
         LogPNGChunk((int) logging,mng_PLTE,data_length);
         for (i=0; i < (long) image->colors; i++)
         {
           chunk[4+i*3]=ScaleQuantumToChar(image->colormap[i].red) & 0xff;
           chunk[5+i*3]=ScaleQuantumToChar(image->colormap[i].green) & 0xff;
           chunk[6+i*3]=ScaleQuantumToChar(image->colormap[i].blue) & 0xff;
         }
         (void) WriteBlob(image,data_length+4,chunk);
         (void) WriteBlobMSBULong(image,crc32(0,chunk,(uInt) (data_length+4)));
         mng_info->have_write_global_plte=MagickTrue;
       }
#endif
    }
  scene=0;
  mng_info->delay=0;
#if defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_MNG_FEATURES_SUPPORTED)
  mng_info->equal_palettes=MagickFalse;
#endif
  do
  {
    if (mng_info->adjoin)
    {
#if defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_MNG_FEATURES_SUPPORTED)
    /*
      If we aren't using a global palette for the entire MNG, check to
      see if we can use one for two or more consecutive images.
    */
    if (need_local_plte && use_global_plte && !all_images_are_gray)
      {
        if (mng_info->IsPalette)
          {
            /*
              When equal_palettes is true, this image has the same palette
              as the previous PseudoClass image
            */
            mng_info->have_write_global_plte=mng_info->equal_palettes;
            mng_info->equal_palettes=PalettesAreEqual(image,image->next);
            if (mng_info->equal_palettes && !mng_info->have_write_global_plte)
              {
                /*
                  Write MNG PLTE chunk
                */
                unsigned long
                  data_length;

                data_length=3*image->colors;
                (void) WriteBlobMSBULong(image,data_length);
                PNGType(chunk,mng_PLTE);
                LogPNGChunk((int) logging,mng_PLTE,data_length);
                for (i=0; i < (long) image->colors; i++)
                {
                  chunk[4+i*3]=ScaleQuantumToChar(image->colormap[i].red);
                  chunk[5+i*3]=ScaleQuantumToChar(image->colormap[i].green);
                  chunk[6+i*3]=ScaleQuantumToChar(image->colormap[i].blue);
                }
                (void) WriteBlob(image,data_length+4,chunk);
                (void) WriteBlobMSBULong(image,crc32(0,chunk,
                   (uInt) (data_length+4)));
                mng_info->have_write_global_plte=MagickTrue;
              }
          }
        else
          mng_info->have_write_global_plte=MagickFalse;
      }
#endif
    if (need_defi)
      {
        long
          previous_x,
          previous_y;

        if (scene)
          {
            previous_x=mng_info->page.x;
            previous_y=mng_info->page.y;
          }
        else
          {
            previous_x=0;
            previous_y=0;
          }
        mng_info->page=image->page;
        if ((mng_info->page.x !=  previous_x) ||
            (mng_info->page.y != previous_y))
          {
             (void) WriteBlobMSBULong(image,12L);  /* data length=12 */
             PNGType(chunk,mng_DEFI);
             LogPNGChunk((int) logging,mng_DEFI,12L);
             chunk[4]=0; /* object 0 MSB */
             chunk[5]=0; /* object 0 LSB */
             chunk[6]=0; /* visible  */
             chunk[7]=0; /* abstract */
             PNGLong(chunk+8,(png_uint_32) mng_info->page.x);
             PNGLong(chunk+12,(png_uint_32) mng_info->page.y);
             (void) WriteBlob(image,16,chunk);
             (void) WriteBlobMSBULong(image,crc32(0,chunk,16));
          }
      }
    }

   mng_info->write_mng=write_mng;

   if ((int) image->dispose >= 3)
     mng_info->framing_mode=3;

   if (mng_info->need_fram && mng_info->adjoin &&
       ((image->delay != mng_info->delay) ||
        (mng_info->framing_mode != mng_info->old_framing_mode)))
     {
       if (image->delay == mng_info->delay)
         {
           /*
             Write a MNG FRAM chunk with the new framing mode.
           */
           (void) WriteBlobMSBULong(image,1L);  /* data length=1 */
           PNGType(chunk,mng_FRAM);
           LogPNGChunk((int) logging,mng_FRAM,1L);
           chunk[4]=(unsigned char) mng_info->framing_mode;
           (void) WriteBlob(image,5,chunk);
           (void) WriteBlobMSBULong(image,crc32(0,chunk,5));
         }
       else
         {
           /*
             Write a MNG FRAM chunk with the delay.
           */
           (void) WriteBlobMSBULong(image,10L);  /* data length=10 */
           PNGType(chunk,mng_FRAM);
           LogPNGChunk((int) logging,mng_FRAM,10L);
           chunk[4]=(unsigned char) mng_info->framing_mode;
           chunk[5]=0;  /* frame name separator (no name) */
           chunk[6]=2;  /* flag for changing default delay */
           chunk[7]=0;  /* flag for changing frame timeout */
           chunk[8]=0;  /* flag for changing frame clipping */
           chunk[9]=0;  /* flag for changing frame sync_id */
           PNGLong(chunk+10,(png_uint_32)
             ((mng_info->ticks_per_second*
             image->delay)/MagickMax(image->ticks_per_second,1)));
           (void) WriteBlob(image,14,chunk);
           (void) WriteBlobMSBULong(image,crc32(0,chunk,14));
           mng_info->delay=image->delay;
         }
       mng_info->old_framing_mode=mng_info->framing_mode;
     }

#if defined(JNG_SUPPORTED)
   if (image_info->compression == JPEGCompression)
     {
       ImageInfo
         *write_info;

       if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  Writing JNG object.");
       /* To do: specify the desired alpha compression method. */
       write_info=CloneImageInfo(image_info);
       write_info->compression=UndefinedCompression;
       status=WriteOneJNGImage(mng_info,write_info,image);
       write_info=DestroyImageInfo(write_info);
     }
   else
#endif
     {
       if (logging != MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "  Writing PNG object.");
       status=WriteOnePNGImage(mng_info,image_info,image);
     }

    if (status == MagickFalse)
      {
        MngInfoFreeStruct(mng_info,&have_mng_structure);
        (void) CloseBlob(image);
        return(MagickFalse);
      }
    (void) CatchImageException(image);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (mng_info->adjoin);
  if (write_mng)
    {
      while (GetPreviousImageInList(image) != (Image *) NULL)
        image=GetPreviousImageInList(image);
      /*
        Write the MEND chunk.
      */
      (void) WriteBlobMSBULong(image,0x00000000L);
      PNGType(chunk,mng_MEND);
      LogPNGChunk((int) logging,mng_MEND,0L);
      (void) WriteBlob(image,4,chunk);
      (void) WriteBlobMSBULong(image,crc32(0,chunk,4));
    }
  /*
    Relinquish resources.
  */
  (void) CloseBlob(image);
  MngInfoFreeStruct(mng_info,&have_mng_structure);
  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit WriteMNGImage()");
  return(MagickTrue);
}
#else /* PNG_LIBPNG_VER > 95 */
static MagickBooleanType WritePNGImage(const ImageInfo *image_info,Image *image)
{
  image=image;
  printf("Your PNG library is too old: You have libpng-%s\n",
     PNG_LIBPNG_VER_STRING);
  ThrowBinaryException(CoderError,"PNG library is too old",
     image_info->filename);
}
static MagickBooleanType WriteMNGImage(const ImageInfo *image_info,Image *image)
{
  return(WritePNGImage(image_info,image));
}
#endif /* PNG_LIBPNG_VER > 95 */
#endif

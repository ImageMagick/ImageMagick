/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        V   V  IIIII  FFFFF  FFFFF                           %
%                        V   V    I    F      F                               %
%                        V   V    I    FFF    FFF                             %
%                         V V     I    F      F                               %
%                          V    IIIII  F      F                               %
%                                                                             %
%                                                                             %
%                Read/Write Khoros Visualization Image Format                 %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteVIFFImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s V I F F                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsVIFF() returns MagickTrue if the image format type, identified by the
%  magick string, is VIFF.
%
%  The format of the IsVIFF method is:
%
%      MagickBooleanType IsVIFF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsVIFF(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if (memcmp(magick,"\253\001",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I F F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadVIFFImage() reads a Khoros Visualization image file and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadVIFFImage method is:
%
%      Image *ReadVIFFImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: Method ReadVIFFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadVIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define VFF_CM_genericRGB  15
#define VFF_CM_ntscRGB  1
#define VFF_CM_NONE  0
#define VFF_DEP_DECORDER  0x4
#define VFF_DEP_NSORDER  0x8
#define VFF_DES_RAW  0
#define VFF_LOC_IMPLICIT  1
#define VFF_MAPTYP_NONE  0
#define VFF_MAPTYP_1_BYTE  1
#define VFF_MAPTYP_2_BYTE  2
#define VFF_MAPTYP_4_BYTE  4
#define VFF_MAPTYP_FLOAT  5
#define VFF_MAPTYP_DOUBLE  7
#define VFF_MS_NONE  0
#define VFF_MS_ONEPERBAND  1
#define VFF_MS_SHARED  3
#define VFF_TYP_BIT  0
#define VFF_TYP_1_BYTE  1
#define VFF_TYP_2_BYTE  2
#define VFF_TYP_4_BYTE  4
#define VFF_TYP_FLOAT  5
#define VFF_TYP_DOUBLE  9

  typedef struct _ViffInfo
  {
    unsigned char
      identifier,
      file_type,
      release,
      version,
      machine_dependency,
      reserve[3];

    char
      comment[512];

    unsigned int
      rows,
      columns,
      subrows;

    int
      x_offset,
      y_offset;

    float
      x_bits_per_pixel,
      y_bits_per_pixel;

    unsigned int
      location_type,
      location_dimension,
      number_of_images,
      number_data_bands,
      data_storage_type,
      data_encode_scheme,
      map_scheme,
      map_storage_type,
      map_rows,
      map_columns,
      map_subrows,
      map_enable,
      maps_per_cycle,
      color_space_model;
  } ViffInfo;

  double
    min_value,
    scale_factor,
    value;

  Image
    *image;

  int
    bit;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  register ssize_t
    x;

  register Quantum
    *q;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    bytes_per_pixel,
    max_packets,
    quantum;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  unsigned long
    lsb_first;

  ViffInfo
    viff_info;

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
  /*
    Read VIFF header (1024 bytes).
  */
  count=ReadBlob(image,1,&viff_info.identifier);
  do
  {
    /*
      Verify VIFF identifier.
    */
    if ((count != 1) || ((unsigned char) viff_info.identifier != 0xab))
      ThrowReaderException(CorruptImageError,"NotAVIFFImage");
    /*
      Initialize VIFF image.
    */
    (void) ReadBlob(image,sizeof(viff_info.file_type),&viff_info.file_type);
    (void) ReadBlob(image,sizeof(viff_info.release),&viff_info.release);
    (void) ReadBlob(image,sizeof(viff_info.version),&viff_info.version);
    (void) ReadBlob(image,sizeof(viff_info.machine_dependency),
      &viff_info.machine_dependency);
    (void) ReadBlob(image,sizeof(viff_info.reserve),viff_info.reserve);
    count=ReadBlob(image,512,(unsigned char *) viff_info.comment);
    if (count != 512)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    viff_info.comment[511]='\0';
    if (strlen(viff_info.comment) > 4)
      (void) SetImageProperty(image,"comment",viff_info.comment,exception);
    if ((viff_info.machine_dependency == VFF_DEP_DECORDER) ||
        (viff_info.machine_dependency == VFF_DEP_NSORDER))
      image->endian=LSBEndian;
    else
      image->endian=MSBEndian;
    viff_info.rows=ReadBlobLong(image);
    viff_info.columns=ReadBlobLong(image);
    viff_info.subrows=ReadBlobLong(image);
    viff_info.x_offset=ReadBlobSignedLong(image);
    viff_info.y_offset=ReadBlobSignedLong(image);
    viff_info.x_bits_per_pixel=(float) ReadBlobLong(image);
    viff_info.y_bits_per_pixel=(float) ReadBlobLong(image);
    viff_info.location_type=ReadBlobLong(image);
    viff_info.location_dimension=ReadBlobLong(image);
    viff_info.number_of_images=ReadBlobLong(image);
    viff_info.number_data_bands=ReadBlobLong(image);
    viff_info.data_storage_type=ReadBlobLong(image);
    viff_info.data_encode_scheme=ReadBlobLong(image);
    viff_info.map_scheme=ReadBlobLong(image);
    viff_info.map_storage_type=ReadBlobLong(image);
    viff_info.map_rows=ReadBlobLong(image);
    viff_info.map_columns=ReadBlobLong(image);
    viff_info.map_subrows=ReadBlobLong(image);
    viff_info.map_enable=ReadBlobLong(image);
    viff_info.maps_per_cycle=ReadBlobLong(image);
    viff_info.color_space_model=ReadBlobLong(image);
    for (i=0; i < 420; i++)
      (void) ReadBlobByte(image);
    if (EOFBlob(image) != MagickFalse)
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    image->columns=viff_info.rows;
    image->rows=viff_info.columns;
    image->depth=viff_info.x_bits_per_pixel <= 8 ? 8UL :
      MAGICKCORE_QUANTUM_DEPTH;
    image->alpha_trait=viff_info.number_data_bands == 4 ? BlendPixelTrait :
      UndefinedPixelTrait;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    (void) SetImageBackgroundColor(image,exception);
    /*
      Verify that we can read this VIFF image.
    */
    number_pixels=(MagickSizeType) viff_info.columns*viff_info.rows;
    if (number_pixels != (size_t) number_pixels)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    if (number_pixels == 0)
      ThrowReaderException(CoderError,"ImageColumnOrRowSizeIsNotSupported");
    if ((viff_info.number_data_bands < 1) || (viff_info.number_data_bands > 4))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((viff_info.data_storage_type != VFF_TYP_BIT) &&
        (viff_info.data_storage_type != VFF_TYP_1_BYTE) &&
        (viff_info.data_storage_type != VFF_TYP_2_BYTE) &&
        (viff_info.data_storage_type != VFF_TYP_4_BYTE) &&
        (viff_info.data_storage_type != VFF_TYP_FLOAT) &&
        (viff_info.data_storage_type != VFF_TYP_DOUBLE))
      ThrowReaderException(CoderError,"DataStorageTypeIsNotSupported");
    if (viff_info.data_encode_scheme != VFF_DES_RAW)
      ThrowReaderException(CoderError,"DataEncodingSchemeIsNotSupported");
    if ((viff_info.map_storage_type != VFF_MAPTYP_NONE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_1_BYTE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_2_BYTE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_4_BYTE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_FLOAT) &&
        (viff_info.map_storage_type != VFF_MAPTYP_DOUBLE))
      ThrowReaderException(CoderError,"MapStorageTypeIsNotSupported");
    if ((viff_info.color_space_model != VFF_CM_NONE) &&
        (viff_info.color_space_model != VFF_CM_ntscRGB) &&
        (viff_info.color_space_model != VFF_CM_genericRGB))
      ThrowReaderException(CoderError,"ColorspaceModelIsNotSupported");
    if (viff_info.location_type != VFF_LOC_IMPLICIT)
      ThrowReaderException(CoderError,"LocationTypeIsNotSupported");
    if (viff_info.number_of_images != 1)
      ThrowReaderException(CoderError,"NumberOfImagesIsNotSupported");
    if (viff_info.map_rows == 0)
      viff_info.map_scheme=VFF_MS_NONE;
    switch ((int) viff_info.map_scheme)
    {
      case VFF_MS_NONE:
      {
        if (viff_info.number_data_bands < 3)
          {
            /*
              Create linear color ramp.
            */
            if (viff_info.data_storage_type == VFF_TYP_BIT)
              image->colors=2;
            else
              if (viff_info.data_storage_type == VFF_MAPTYP_1_BYTE)
                image->colors=256UL;
              else
                image->colors=image->depth <= 8 ? 256UL : 65536UL;
            status=AcquireImageColormap(image,image->colors,exception);
            if (status == MagickFalse)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        break;
      }
      case VFF_MS_ONEPERBAND:
      case VFF_MS_SHARED:
      {
        unsigned char
          *viff_colormap;

        /*
          Allocate VIFF colormap.
        */
        switch ((int) viff_info.map_storage_type)
        {
          case VFF_MAPTYP_1_BYTE: bytes_per_pixel=1; break;
          case VFF_MAPTYP_2_BYTE: bytes_per_pixel=2; break;
          case VFF_MAPTYP_4_BYTE: bytes_per_pixel=4; break;
          case VFF_MAPTYP_FLOAT: bytes_per_pixel=4; break;
          case VFF_MAPTYP_DOUBLE: bytes_per_pixel=8; break;
          default: bytes_per_pixel=1; break;
        }
        image->colors=viff_info.map_columns;
        if ((MagickSizeType) (viff_info.map_rows*image->colors) > GetBlobSize(image))
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        if ((MagickSizeType) viff_info.map_rows > GetBlobSize(image))
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        if ((MagickSizeType) viff_info.map_rows >
            (viff_info.map_rows*bytes_per_pixel*sizeof(*viff_colormap)))
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        viff_colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          viff_info.map_rows*bytes_per_pixel*sizeof(*viff_colormap));
        if (viff_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        /*
          Read VIFF raster colormap.
        */
        count=ReadBlob(image,bytes_per_pixel*image->colors*viff_info.map_rows,
          viff_colormap);
        lsb_first=1;
        if (*(char *) &lsb_first &&
            ((viff_info.machine_dependency != VFF_DEP_DECORDER) &&
             (viff_info.machine_dependency != VFF_DEP_NSORDER)))
          switch ((int) viff_info.map_storage_type)
          {
            case VFF_MAPTYP_2_BYTE:
            {
              MSBOrderShort(viff_colormap,(bytes_per_pixel*image->colors*
                viff_info.map_rows));
              break;
            }
            case VFF_MAPTYP_4_BYTE:
            case VFF_MAPTYP_FLOAT:
            {
              MSBOrderLong(viff_colormap,(bytes_per_pixel*image->colors*
                viff_info.map_rows));
              break;
            }
            default: break;
          }
        for (i=0; i < (ssize_t) (viff_info.map_rows*image->colors); i++)
        {
          switch ((int) viff_info.map_storage_type)
          {
            case VFF_MAPTYP_2_BYTE: value=1.0*((short *) viff_colormap)[i]; break;
            case VFF_MAPTYP_4_BYTE: value=1.0*((int *) viff_colormap)[i]; break;
            case VFF_MAPTYP_FLOAT: value=((float *) viff_colormap)[i]; break;
            case VFF_MAPTYP_DOUBLE: value=((double *) viff_colormap)[i]; break;
            default: value=1.0*viff_colormap[i]; break;
          }
          if (i < (ssize_t) image->colors)
            {
              image->colormap[i].red=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
              image->colormap[i].green=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
              image->colormap[i].blue=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
            }
          else
            if (i < (ssize_t) (2*image->colors))
              image->colormap[i % image->colors].green=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
            else
              if (i < (ssize_t) (3*image->colors))
                image->colormap[i % image->colors].blue=(MagickRealType)
                  ScaleCharToQuantum((unsigned char) value);
        }
        viff_colormap=(unsigned char *) RelinquishMagickMemory(viff_colormap);
        break;
      }
      default:
        ThrowReaderException(CoderError,"ColormapTypeNotSupported");
    }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (viff_info.data_storage_type == VFF_TYP_BIT)
      {
        /*
          Create bi-level colormap.
        */
        image->colors=2;
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        image->colorspace=GRAYColorspace;
      }
    /*
      Allocate VIFF pixels.
    */
    switch ((int) viff_info.data_storage_type)
    {
      case VFF_TYP_2_BYTE: bytes_per_pixel=2; break;
      case VFF_TYP_4_BYTE: bytes_per_pixel=4; break;
      case VFF_TYP_FLOAT: bytes_per_pixel=4; break;
      case VFF_TYP_DOUBLE: bytes_per_pixel=8; break;
      default: bytes_per_pixel=1; break;
    }
    if (viff_info.data_storage_type == VFF_TYP_BIT)
      {
        if (HeapOverflowSanityCheck((image->columns+7UL) >> 3UL,image->rows) != MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        max_packets=((image->columns+7UL) >> 3UL)*image->rows;
      }
    else
      {
        if (HeapOverflowSanityCheck((size_t) number_pixels,viff_info.number_data_bands) != MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        max_packets=(size_t) (number_pixels*viff_info.number_data_bands);
      }
    if ((MagickSizeType) (bytes_per_pixel*max_packets) > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    pixels=(unsigned char *) AcquireQuantumMemory((size_t) MagickMax(
      number_pixels,max_packets),bytes_per_pixel*sizeof(*pixels));
    if (pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    (void) memset(pixels,0,MagickMax(number_pixels,max_packets)*
      bytes_per_pixel*sizeof(*pixels));
    count=ReadBlob(image,bytes_per_pixel*max_packets,pixels);
    lsb_first=1;
    if (*(char *) &lsb_first &&
        ((viff_info.machine_dependency != VFF_DEP_DECORDER) &&
         (viff_info.machine_dependency != VFF_DEP_NSORDER)))
      switch ((int) viff_info.data_storage_type)
      {
        case VFF_TYP_2_BYTE:
        {
          MSBOrderShort(pixels,bytes_per_pixel*max_packets);
          break;
        }
        case VFF_TYP_4_BYTE:
        case VFF_TYP_FLOAT:
        {
          MSBOrderLong(pixels,bytes_per_pixel*max_packets);
          break;
        }
        default: break;
      }
    min_value=0.0;
    scale_factor=1.0;
    if ((viff_info.data_storage_type != VFF_TYP_1_BYTE) &&
        (viff_info.map_scheme == VFF_MS_NONE))
      {
        double
          max_value;

        /*
          Determine scale factor.
        */
        switch ((int) viff_info.data_storage_type)
        {
          case VFF_TYP_2_BYTE: value=1.0*((short *) pixels)[0]; break;
          case VFF_TYP_4_BYTE: value=1.0*((int *) pixels)[0]; break;
          case VFF_TYP_FLOAT: value=((float *) pixels)[0]; break;
          case VFF_TYP_DOUBLE: value=((double *) pixels)[0]; break;
          default: value=1.0*pixels[0]; break;
        }
        max_value=value;
        min_value=value;
        for (i=0; i < (ssize_t) max_packets; i++)
        {
          switch ((int) viff_info.data_storage_type)
          {
            case VFF_TYP_2_BYTE: value=1.0*((short *) pixels)[i]; break;
            case VFF_TYP_4_BYTE: value=1.0*((int *) pixels)[i]; break;
            case VFF_TYP_FLOAT: value=((float *) pixels)[i]; break;
            case VFF_TYP_DOUBLE: value=((double *) pixels)[i]; break;
            default: value=1.0*pixels[i]; break;
          }
          if (value > max_value)
            max_value=value;
          else
            if (value < min_value)
              min_value=value;
        }
        if ((min_value == 0) && (max_value == 0))
          scale_factor=0;
        else
          if (min_value == max_value)
            {
              scale_factor=(double) QuantumRange/min_value;
              min_value=0;
            }
          else
            scale_factor=(double) QuantumRange/(max_value-min_value);
      }
    /*
      Convert pixels to Quantum size.
    */
    p=(unsigned char *) pixels;
    for (i=0; i < (ssize_t) max_packets; i++)
    {
      switch ((int) viff_info.data_storage_type)
      {
        case VFF_TYP_2_BYTE: value=1.0*((short *) pixels)[i]; break;
        case VFF_TYP_4_BYTE: value=1.0*((int *) pixels)[i]; break;
        case VFF_TYP_FLOAT: value=((float *) pixels)[i]; break;
        case VFF_TYP_DOUBLE: value=((double *) pixels)[i]; break;
        default: value=1.0*pixels[i]; break;
      }
      if (viff_info.map_scheme == VFF_MS_NONE)
        {
          value=(value-min_value)*scale_factor;
          if (value > QuantumRange)
            value=QuantumRange;
          else
            if (value < 0)
              value=0;
        }
      *p=(unsigned char) ((Quantum) value);
      p++;
    }
    /*
      Convert VIFF raster image to pixel packets.
    */
    p=(unsigned char *) pixels;
    if (viff_info.data_storage_type == VFF_TYP_BIT)
      {
        /*
          Convert bitmap scanline.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) (image->columns-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              quantum=(size_t) ((*p) & (0x01 << bit) ? 0 : 1);
              SetPixelRed(image,quantum == 0 ? 0 : QuantumRange,q);
              SetPixelGreen(image,quantum == 0 ? 0 : QuantumRange,q);
              SetPixelBlue(image,quantum == 0 ? 0 : QuantumRange,q);
              if (image->storage_class == PseudoClass)
                SetPixelIndex(image,(Quantum) quantum,q);
              q+=GetPixelChannels(image);
            }
            p++;
          }
          if ((image->columns % 8) != 0)
            {
              for (bit=0; bit < (int) (image->columns % 8); bit++)
              {
                quantum=(size_t) ((*p) & (0x01 << bit) ? 0 : 1);
                SetPixelRed(image,quantum == 0 ? 0 : QuantumRange,q);
                SetPixelGreen(image,quantum == 0 ? 0 : QuantumRange,q);
                SetPixelBlue(image,quantum == 0 ? 0 : QuantumRange,q);
                if (image->storage_class == PseudoClass)
                  SetPixelIndex(image,(Quantum) quantum,q);
                q+=GetPixelChannels(image);
              }
              p++;
            }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
      }
    else
      if (image->storage_class == PseudoClass)
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelIndex(image,*p++,q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
      else
        {
          /*
            Convert DirectColor scanline.
          */
          number_pixels=(MagickSizeType) image->columns*image->rows;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(*p),q);
              SetPixelGreen(image,ScaleCharToQuantum(*(p+number_pixels)),q);
              SetPixelBlue(image,ScaleCharToQuantum(*(p+2*number_pixels)),q);
              if (image->colors != 0)
                {
                  ssize_t
                    index;

                  index=(ssize_t) GetPixelRed(image,q);
                  SetPixelRed(image,ClampToQuantum(image->colormap[
                    ConstrainColormapIndex(image,index,exception)].red),q);
                  index=(ssize_t) GetPixelGreen(image,q);
                  SetPixelGreen(image,ClampToQuantum(image->colormap[
                    ConstrainColormapIndex(image,index,exception)].green),q);
                  index=(ssize_t) GetPixelBlue(image,q);
                  SetPixelBlue(image,ClampToQuantum(image->colormap[
                    ConstrainColormapIndex(image,index,exception)].blue),q);
                }
              SetPixelAlpha(image,image->alpha_trait != UndefinedPixelTrait ?
                ScaleCharToQuantum(*(p+number_pixels*3)) : OpaqueAlpha,q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        }
    pixels=(unsigned char *) RelinquishMagickMemory(pixels);
    if (image->storage_class == PseudoClass)
      (void) SyncImage(image,exception);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    count=ReadBlob(image,1,&viff_info.identifier);
    if ((count == 1) && (viff_info.identifier == 0xab))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  } while ((count != 0) && (viff_info.identifier == 0xab));
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r V I F F I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterVIFFImage() adds properties for the VIFF image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterVIFFImage method is:
%
%      size_t RegisterVIFFImage(void)
%
*/
ModuleExport size_t RegisterVIFFImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("VIFF","VIFF","Khoros Visualization image");
  entry->decoder=(DecodeImageHandler *) ReadVIFFImage;
  entry->encoder=(EncodeImageHandler *) WriteVIFFImage;
  entry->magick=(IsImageFormatHandler *) IsVIFF;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIFF","XV","Khoros Visualization image");
  entry->decoder=(DecodeImageHandler *) ReadVIFFImage;
  entry->encoder=(EncodeImageHandler *) WriteVIFFImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r V I F F I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterVIFFImage() removes format registrations made by the
%  VIFF module from the list of supported formats.
%
%  The format of the UnregisterVIFFImage method is:
%
%      UnregisterVIFFImage(void)
%
*/
ModuleExport void UnregisterVIFFImage(void)
{
  (void) UnregisterMagickInfo("VIFF");
  (void) UnregisterMagickInfo("XV");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteVIFFImage() writes an image to a file in the VIFF image format.
%
%  The format of the WriteVIFFImage method is:
%
%      MagickBooleanType WriteVIFFImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WriteVIFFImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
#define VFF_CM_genericRGB  15
#define VFF_CM_NONE  0
#define VFF_DEP_IEEEORDER  0x2
#define VFF_DES_RAW  0
#define VFF_LOC_IMPLICIT  1
#define VFF_MAPTYP_NONE  0
#define VFF_MAPTYP_1_BYTE  1
#define VFF_MS_NONE  0
#define VFF_MS_ONEPERBAND  1
#define VFF_TYP_BIT  0
#define VFF_TYP_1_BYTE  1

  typedef struct _ViffInfo
  {
    char
      identifier,
      file_type,
      release,
      version,
      machine_dependency,
      reserve[3],
      comment[512];

    size_t
      rows,
      columns,
      subrows;

    int
      x_offset,
      y_offset;

    unsigned int
      x_bits_per_pixel,
      y_bits_per_pixel,
      location_type,
      location_dimension,
      number_of_images,
      number_data_bands,
      data_storage_type,
      data_encode_scheme,
      map_scheme,
      map_storage_type,
      map_rows,
      map_columns,
      map_subrows,
      map_enable,
      maps_per_cycle,
      color_space_model;
  } ViffInfo;

  const char
    *value;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  MagickSizeType
    number_pixels,
    packets;

  MemoryInfo
    *pixel_info;

  register const Quantum
    *p;

  register ssize_t
    x;

  register ssize_t
    i;

  register unsigned char
    *q;

  size_t
    imageListLength;

  ssize_t
    y;

  unsigned char
    *pixels;

  ViffInfo
    viff_info;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) memset(&viff_info,0,sizeof(ViffInfo));
  scene=0;
  imageListLength=GetImageListLength(image);
  do
  {
    /*
      Initialize VIFF image structure.
    */
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
DisableMSCWarning(4310)
    viff_info.identifier=(char) 0xab;
RestoreMSCWarning
    viff_info.file_type=1;
    viff_info.release=1;
    viff_info.version=3;
    viff_info.machine_dependency=VFF_DEP_IEEEORDER;  /* IEEE byte ordering */
    *viff_info.comment='\0';
    value=GetImageProperty(image,"comment",exception);
    if (value != (const char *) NULL)
      (void) CopyMagickString(viff_info.comment,value,MagickMin(strlen(value),
        511)+1);
    viff_info.rows=image->columns;
    viff_info.columns=image->rows;
    viff_info.subrows=0;
    viff_info.x_offset=(~0);
    viff_info.y_offset=(~0);
    viff_info.x_bits_per_pixel=0;
    viff_info.y_bits_per_pixel=0;
    viff_info.location_type=VFF_LOC_IMPLICIT;
    viff_info.location_dimension=0;
    viff_info.number_of_images=1;
    viff_info.data_encode_scheme=VFF_DES_RAW;
    viff_info.map_scheme=VFF_MS_NONE;
    viff_info.map_storage_type=VFF_MAPTYP_NONE;
    viff_info.map_rows=0;
    viff_info.map_columns=0;
    viff_info.map_subrows=0;
    viff_info.map_enable=1;  /* no colormap */
    viff_info.maps_per_cycle=0;
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if (image->storage_class == DirectClass)
      {
        /*
          Full color VIFF raster.
        */
        viff_info.number_data_bands=image->alpha_trait ? 4U : 3U;
        viff_info.color_space_model=VFF_CM_genericRGB;
        viff_info.data_storage_type=VFF_TYP_1_BYTE;
        packets=viff_info.number_data_bands*number_pixels;
      }
    else
      {
        viff_info.number_data_bands=1;
        viff_info.color_space_model=VFF_CM_NONE;
        viff_info.data_storage_type=VFF_TYP_1_BYTE;
        packets=number_pixels;
        if (SetImageGray(image,exception) == MagickFalse)
          {
            /*
              Colormapped VIFF raster.
            */
            viff_info.map_scheme=VFF_MS_ONEPERBAND;
            viff_info.map_storage_type=VFF_MAPTYP_1_BYTE;
            viff_info.map_rows=3;
            viff_info.map_columns=(unsigned int) image->colors;
          }
        else
          if (image->colors <= 2)
            {
              /*
                Monochrome VIFF raster.
              */
              viff_info.data_storage_type=VFF_TYP_BIT;
              packets=((image->columns+7) >> 3)*image->rows;
            }
      }
    /*
      Write VIFF image header (pad to 1024 bytes).
    */
    (void) WriteBlob(image,sizeof(viff_info.identifier),(unsigned char *)
      &viff_info.identifier);
    (void) WriteBlob(image,sizeof(viff_info.file_type),(unsigned char *)
      &viff_info.file_type);
    (void) WriteBlob(image,sizeof(viff_info.release),(unsigned char *)
      &viff_info.release);
    (void) WriteBlob(image,sizeof(viff_info.version),(unsigned char *)
      &viff_info.version);
    (void) WriteBlob(image,sizeof(viff_info.machine_dependency),
      (unsigned char *) &viff_info.machine_dependency);
    (void) WriteBlob(image,sizeof(viff_info.reserve),(unsigned char *)
      viff_info.reserve);
    (void) WriteBlob(image,512,(unsigned char *) viff_info.comment);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.rows);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.columns);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.subrows);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.x_offset);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.y_offset);
    viff_info.x_bits_per_pixel=(unsigned int) ((63 << 24) | (128 << 16));
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.x_bits_per_pixel);
    viff_info.y_bits_per_pixel=(unsigned int) ((63 << 24) | (128 << 16));
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.y_bits_per_pixel);
    (void) WriteBlobMSBLong(image,viff_info.location_type);
    (void) WriteBlobMSBLong(image,viff_info.location_dimension);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.number_of_images);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.number_data_bands);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.data_storage_type);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.data_encode_scheme);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.map_scheme);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.map_storage_type);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.map_rows);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.map_columns);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.map_subrows);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.map_enable);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.maps_per_cycle);
    (void) WriteBlobMSBLong(image,(unsigned int) viff_info.color_space_model);
    for (i=0; i < 420; i++)
      (void) WriteBlobByte(image,'\0');
    /*
      Convert MIFF to VIFF raster pixels.
    */
    pixel_info=AcquireVirtualMemory((size_t) packets,sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    q=pixels;
    if (image->storage_class == DirectClass)
      {
        /*
          Convert DirectClass packet to VIFF RGB pixel.
        */
        number_pixels=(MagickSizeType) image->columns*image->rows;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q=ScaleQuantumToChar(GetPixelRed(image,p));
            *(q+number_pixels)=ScaleQuantumToChar(GetPixelGreen(image,p));
            *(q+number_pixels*2)=ScaleQuantumToChar(GetPixelBlue(image,p));
            if (image->alpha_trait != UndefinedPixelTrait)
              *(q+number_pixels*3)=ScaleQuantumToChar((Quantum)
                (GetPixelAlpha(image,p)));
            p+=GetPixelChannels(image);
            q++;
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
      }
    else
      if (SetImageGray(image,exception) == MagickFalse)
        {
          unsigned char
            *viff_colormap;

          /*
            Dump colormap to file.
          */
          viff_colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
            3*sizeof(*viff_colormap));
          if (viff_colormap == (unsigned char *) NULL)
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          q=viff_colormap;
          for (i=0; i < (ssize_t) image->colors; i++)
            *q++=ScaleQuantumToChar(ClampToQuantum(image->colormap[i].red));
          for (i=0; i < (ssize_t) image->colors; i++)
            *q++=ScaleQuantumToChar(ClampToQuantum(image->colormap[i].green));
          for (i=0; i < (ssize_t) image->colors; i++)
            *q++=ScaleQuantumToChar(ClampToQuantum(image->colormap[i].blue));
          (void) WriteBlob(image,3*image->colors,viff_colormap);
          viff_colormap=(unsigned char *) RelinquishMagickMemory(viff_colormap);
          /*
            Convert PseudoClass packet to VIFF colormapped pixels.
          */
          q=pixels;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              *q++=(unsigned char) GetPixelIndex(image,p);
              p+=GetPixelChannels(image);
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        }
      else
        if (image->colors <= 2)
          {
            ssize_t
              x,
              y;

            register unsigned char
              bit,
              byte;

            /*
              Convert PseudoClass image to a VIFF monochrome image.
            */
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              bit=0;
              byte=0;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                byte>>=1;
                if (GetPixelLuma(image,p) < (QuantumRange/2.0))
                  byte|=0x80;
                bit++;
                if (bit == 8)
                  {
                    *q++=byte;
                    bit=0;
                    byte=0;
                  }
                p+=GetPixelChannels(image);
              }
              if (bit != 0)
                *q++=byte >> (8-bit);
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
          }
        else
          {
            /*
              Convert PseudoClass packet to VIFF grayscale pixel.
            */
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                *q++=(unsigned char) ClampToQuantum(GetPixelLuma(image,p));
                p+=GetPixelChannels(image);
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
          }
    (void) WriteBlob(image,(size_t) packets,pixels);
    pixel_info=RelinquishVirtualMemory(pixel_info);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,imageListLength);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

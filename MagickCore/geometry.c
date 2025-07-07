/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           GGGG   EEEEE   OOO   M   M  EEEEE  TTTTT  RRRR   Y   Y            %
%           G      E      O   O  MM MM  E        T    R   R   Y Y             %
%           G  GG  EEE    O   O  M M M  EEE      T    RRRR     Y              %
%           G   G  E      O   O  M   M  E        T    R R      Y              %
%            GGGG  EEEEE   OOO   M   M  EEEEE    T    R  R     Y              %
%                                                                             %
%                                                                             %
%                       MagickCore Geometry Methods                           %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%  Copyright 1999 ImageMagick Studio LLC, a non-profit organization           %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
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
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/geometry-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"

/*
  Define declarations.
*/
#define MagickPagesize(name,geometry) { name, sizeof(name)-1, geometry }

/*
  Structure declarations.
*/
typedef struct _PageInfo
{
  const char
    name[12];

  size_t
    extent;

  const char
    geometry[10];
} PageInfo;

static const PageInfo
  Pagesizes[] =
  {
    MagickPagesize("4x6", "288x432"),
    MagickPagesize("5x7", "360x504"),
    MagickPagesize("7x9", "504x648"),
    MagickPagesize("8x10", "576x720"),
    MagickPagesize("9x11", "648x792"),
    MagickPagesize("9x12", "648x864"),
    MagickPagesize("10x13", "720x936"),
    MagickPagesize("10x14", "720x1008"),
    MagickPagesize("11x17", "792x1224"),
    MagickPagesize("4A0", "4768x6741"),
    MagickPagesize("2A0", "3370x4768"),
    MagickPagesize("a0", "2384x3370"),
    MagickPagesize("a10", "74x105"),
    MagickPagesize("a1", "1684x2384"),
    MagickPagesize("a2", "1191x1684"),
    MagickPagesize("a3", "842x1191"),
    MagickPagesize("a4small", "595x842"),
    MagickPagesize("a4", "595x842"),
    MagickPagesize("a5", "420x595"),
    MagickPagesize("a6", "298x420"),
    MagickPagesize("a7", "210x298"),
    MagickPagesize("a8", "147x210"),
    MagickPagesize("a9", "105x147"),
    MagickPagesize("archa", "648x864"),
    MagickPagesize("archb", "864x1296"),
    MagickPagesize("archC", "1296x1728"),
    MagickPagesize("archd", "1728x2592"),
    MagickPagesize("arche", "2592x3456"),
    MagickPagesize("b0", "2920x4127"),
    MagickPagesize("b10", "91x127"),
    MagickPagesize("b1", "2064x2920"),
    MagickPagesize("b2", "1460x2064"),
    MagickPagesize("b3", "1032x1460"),
    MagickPagesize("b4", "729x1032"),
    MagickPagesize("b5", "516x729"),
    MagickPagesize("b6", "363x516"),
    MagickPagesize("b7", "258x363"),
    MagickPagesize("b8", "181x258"),
    MagickPagesize("b9", "127x181"),
    MagickPagesize("c0", "2599x3676"),
    MagickPagesize("c1", "1837x2599"),
    MagickPagesize("c2", "1298x1837"),
    MagickPagesize("c3", "918x1296"),
    MagickPagesize("c4", "649x918"),
    MagickPagesize("c5", "459x649"),
    MagickPagesize("c6", "323x459"),
    MagickPagesize("c7", "230x323"),
    MagickPagesize("csheet", "1224x1584"),
    MagickPagesize("dsheet", "1584x2448"),
    MagickPagesize("esheet", "2448x3168"),
    MagickPagesize("executive", "540x720"),
    MagickPagesize("flsa", "612x936"),
    MagickPagesize("flse", "612x936"),
    MagickPagesize("folio", "612x936"),
    MagickPagesize("halfletter", "396x612"),
    MagickPagesize("isob0", "2835x4008"),
    MagickPagesize("isob10", "88x125"),
    MagickPagesize("isob1", "2004x2835"),
    MagickPagesize("isob2", "1417x2004"),
    MagickPagesize("isob3", "1001x1417"),
    MagickPagesize("isob4", "709x1001"),
    MagickPagesize("isob5", "499x709"),
    MagickPagesize("isob6", "354x499"),
    MagickPagesize("isob7", "249x354"),
    MagickPagesize("isob8", "176x249"),
    MagickPagesize("isob9", "125x176"),
    MagickPagesize("jisb0", "1030x1456"),
    MagickPagesize("jisb1", "728x1030"),
    MagickPagesize("jisb2", "515x728"),
    MagickPagesize("jisb3", "364x515"),
    MagickPagesize("jisb4", "257x364"),
    MagickPagesize("jisb5", "182x257"),
    MagickPagesize("jisb6", "128x182"),
    MagickPagesize("ledger", "1224x792"),
    MagickPagesize("legal", "612x1008"),
    MagickPagesize("lettersmall", "612x792"),
    MagickPagesize("letter", "612x792"),
    MagickPagesize("monarch", "279x540"),
    MagickPagesize("quarto", "610x780"),
    MagickPagesize("statement", "396x612"),
    MagickPagesize("tabloid", "792x1224"),
    MagickPagesize("", "")
  };

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t G e o m e t r y                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetGeometry() parses a geometry specification and returns the width,
%  height, x, and y values.  It also returns flags that indicates which
%  of the four values (width, height, x, y) were located in the string, and
%  whether the x or y values are negative.  In addition, there are flags to
%  report any meta characters (%, !, <, or >).
%
%  The value must form a proper geometry style specification of WxH+X+Y
%  of integers only, and values can not be separated by comma, colon, or
%  slash characters.  See ParseGeometry() below.
%
%  Offsets may be prefixed by multiple signs to make offset string
%  substitutions easier to handle from shell scripts.
%  For example: "-10-10", "-+10-+10", or "+-10+-10" will generate negative
%  offsets, while "+10+10", "++10++10", or "--10--10" will generate positive
%  offsets.
%
%  The format of the GetGeometry method is:
%
%      MagickStatusType GetGeometry(const char *geometry,ssize_t *x,ssize_t *y,
%        size_t *width,size_t *height)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry.
%
%    o x,y:  The x and y offset as determined by the geometry specification.
%
%    o width,height:  The width and height as determined by the geometry
%      specification.
%
*/
MagickExport MagickStatusType GetGeometry(const char *geometry,ssize_t *x,
  ssize_t *y,size_t *width,size_t *height)
{
  char
    *p,
    pedantic_geometry[MagickPathExtent],
    *q;

  double
    value;

  int
    c;

  MagickStatusType
    flags;

  /*
    Remove whitespace and meta characters from geometry specification.
  */
  flags=NoValue;
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(flags);
  if (strlen(geometry) >= (MagickPathExtent-1))
    return(flags);
  (void) CopyMagickString(pedantic_geometry,geometry,MagickPathExtent);
  for (p=pedantic_geometry; *p != '\0'; )
  {
    if (isspace((int) ((unsigned char) *p)) != 0)
      {
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        continue;
      }
    c=(int) *p;
    switch (c)
    {
      case '%':
      {
        flags|=PercentValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '!':
      {
        flags|=AspectValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '<':
      {
        flags|=LessValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '>':
      {
        flags|=GreaterValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '#':
      {
        flags|=MaximumValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '^':
      {
        flags|=MinimumValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '@':
      {
        flags|=AreaValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '(':
      case ')':
      {
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case 'x':
      case 'X':
      {
        flags|=SeparatorValue;
        p++;
        break;
      }
      case '-':
      case ',':
      case '+':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case 215:
      case 'e':
      case 'E':
      {
        p++;
        break;
      }
      case '.':
      {
        p++;
        flags|=DecimalValue;
        break;
      }
      case ':':
      {
        p++;
        flags|=AspectRatioValue;
        break;
      }
      default:
        return(flags);
    }
  }
  /*
    Parse width, height, x, and y.
  */
  p=pedantic_geometry;
  if (*p == '\0')
    return(flags);
  q=p;
  value=StringToDouble(p,&q);
  (void) value;
  if (LocaleNCompare(p,"0x",2) == 0)
    value=(double) strtol(p,&q,10);
  if ((*p != '+') && (*p != '-'))
    {
      c=(int) ((unsigned char) *q);
      if ((c == 215) || (*q == 'x') || (*q == 'X') || (*q == ':') ||
          (*q == '\0'))
        {
          /*
            Parse width.
          */
          q=p;
          if (width != (size_t *) NULL)
            {
              if (LocaleNCompare(p,"0x",2) == 0)
                *width=(size_t) strtol(p,&p,10);
              else
                *width=CastDoubleToSizeT(StringToDouble(p,&p)+0.5);
            }
          if (p != q)
            flags|=WidthValue;
        }
    }
  if ((*p != '+') && (*p != '-'))
    {
      c=(int) ((unsigned char) *p);
      if ((c == 215) || (*p == 'x') || (*p == 'X') || (*p == ':'))
        {
          p++;
          if ((*p != '+') && (*p != '-'))
            {
              /*
                Parse height.
              */
              q=p;
              if (height != (size_t *) NULL)
                *height=CastDoubleToSizeT(StringToDouble(p,&p)+0.5);
              if (p != q)
                flags|=HeightValue;
            }
        }
    }
  if ((*p == '+') || (*p == '-'))
    {
      /*
        Parse x value.
      */
      while ((*p == '+') || (*p == '-'))
      {
        if (*p == '-')
          flags^=XNegative;  /* negate sign */
        p++;
      }
      q=p;
      if (x != (ssize_t *) NULL)
        *x=CastDoubleToSsizeT(StringToDouble(p,&p));
      if (p != q)
        {
          flags|=XValue;
          if (((flags & XNegative) != 0) && (x != (ssize_t *) NULL))
            *x=CastDoubleToSsizeT(-1.0**x);
        }
    }
  if ((*p == '+') || (*p == '-'))
    {
      /*
        Parse y value.
      */
      while ((*p == '+') || (*p == '-'))
      {
        if (*p == '-')
          flags^=YNegative;  /* negate sign */
        p++;
      }
      q=p;
      if (y != (ssize_t *) NULL)
        *y=CastDoubleToSsizeT(StringToDouble(p,&p));
      if (p != q)
        {
          flags|=YValue;
          if (((flags & YNegative) != 0) && (y != (ssize_t *) NULL))
            *y=CastDoubleToSsizeT(-1.0**y);
        }
    }
  if ((flags & PercentValue) != 0)
    {
      if (((flags & SeparatorValue) == 0) && ((flags & HeightValue) == 0))
        {
          if ((height != (size_t *) NULL) && (width != (size_t *) NULL))
            *height=(*width);
          flags|=HeightValue;
        }
      if (((flags & SeparatorValue) != 0) && ((flags & WidthValue) == 0) &&
          (height != (size_t *) NULL) && (width != (size_t *) NULL))
        *width=(*height);
    }
#if 0
  /* Debugging Geometry */
  (void) fprintf(stderr,"GetGeometry...\n");
  (void) fprintf(stderr,"Input: %s\n",geometry);
  (void) fprintf(stderr,"Flags: %c %c %s %s\n",
    (flags & WidthValue) ? 'W' : ' ',(flags & HeightValue) ? 'H' : ' ',
    (flags & XValue) ? ((flags & XNegative) ? "-X" : "+X") : "  ",
    (flags & YValue) ? ((flags & YNegative) ? "-Y" : "+Y") : "  ");
  (void) fprintf(stderr,"Geometry: %ldx%ld%+ld%+ld\n",(long) *width,(long)
    *height,(long) *x,(long) *y);
#endif
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t P a g e G e o m e t r y                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPageGeometry() replaces any page mnemonic with the equivalent size in
%  picas.
%
%  The format of the GetPageGeometry method is:
%
%      char *GetPageGeometry(const char *page_geometry)
%
%  A description of each parameter follows.
%
%   o  page_geometry:  Specifies a pointer to an array of characters.  The
%      string is either a Postscript page name (e.g. A4) or a postscript page
%      geometry (e.g. 612x792+36+36).
%
*/
MagickExport char *GetPageGeometry(const char *page_geometry)
{
  char
    page[MagickPathExtent];

  ssize_t
    i;

  assert(page_geometry != (char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",page_geometry);
  (void) CopyMagickString(page,page_geometry,MagickPathExtent);
  for (i=0; *Pagesizes[i].name != '\0'; i++)
  {
    int
      status;

    if (Pagesizes[i].extent == 0)
      break;  /* sentinel */
    status=LocaleNCompare(Pagesizes[i].name,page_geometry,Pagesizes[i].extent);
    if (status == 0)
      {
        MagickStatusType
          flags;

        RectangleInfo
          geometry;

        /*
          Replace mnemonic with the equivalent size in dots-per-inch.
        */
        (void) FormatLocaleString(page,MagickPathExtent,"%s%.80s",
          Pagesizes[i].geometry,page_geometry+Pagesizes[i].extent);
        flags=GetGeometry(page,&geometry.x,&geometry.y,&geometry.width,
          &geometry.height);
        if ((flags & GreaterValue) == 0)
          (void) ConcatenateMagickString(page,">",MagickPathExtent);
        break;
      }
  }
  return(AcquireString(page));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G r a v i t y A d j u s t G e o m e t r y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GravityAdjustGeometry() adjusts the offset of a region with regard to the
%  given: width, height and gravity; against which it is positioned.
%
%  The region should also have an appropriate width and height to correctly
%  set the right offset of the top left corner of the region.
%
%  The format of the GravityAdjustGeometry method is:
%
%      void GravityAdjustGeometry(const size_t width, const size_t height,
%        const GravityType gravity,RectangleInfo *region);
%
%  A description of each parameter follows:
%
%    o width, height:  the larger area the region is relative to
%
%    o gravity: the edge/corner the current offset is relative to
%
%    o region:  The region requiring a offset adjustment relative to gravity
%
*/
MagickExport void GravityAdjustGeometry(const size_t width,
  const size_t height,const GravityType gravity,RectangleInfo *region)
{
  if (region->height == 0)
    region->height=height;
  if (region->width == 0)
    region->width=width;
  switch (gravity)
  {
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
    {
      region->x=CastDoubleToSsizeT((double) width-region->width-region->x);
      break;
    }
    case NorthGravity:
    case SouthGravity:
    case CenterGravity:
    {
      region->x=CastDoubleToSsizeT((double) width/2.0-region->width/2.0+
        region->x);
      break;
    }
    case ForgetGravity:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
    default:
      break;
  }
  switch (gravity)
  {
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
    {
      region->y=CastDoubleToSsizeT((double) height-region->height-region->y);
      break;
    }
    case EastGravity:
    case WestGravity:
    case CenterGravity:
    {
      region->y=CastDoubleToSsizeT((double) height/2.0-region->height/2.0+
        region->y);
      break;
    }
    case ForgetGravity:
    case NorthWestGravity:
    case NorthGravity:
    case NorthEastGravity:
    default:
      break;
  }
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     I s G e o m e t r y                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsGeometry() returns MagickTrue if the geometry specification is valid.
%  Examples are 100, 100x200, x200, 100x200+10+20, +10+20, 200%, 200x200!, etc.
%
%  The format of the IsGeometry method is:
%
%      MagickBooleanType IsGeometry(const char *geometry)
%
%  A description of each parameter follows:
%
%    o geometry: This string is the geometry specification.
%
*/
MagickExport MagickBooleanType IsGeometry(const char *geometry)
{
  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  if (geometry == (const char *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(geometry,&geometry_info);
  return(flags != NoValue ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     I s S c e n e G e o m e t r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSceneGeometry() returns MagickTrue if the geometry is a valid scene
%  specification (e.g. [1], [1-9], [1,7,4]).
%
%  The format of the IsSceneGeometry method is:
%
%      MagickBooleanType IsSceneGeometry(const char *geometry,
%        const MagickBooleanType pedantic)
%
%  A description of each parameter follows:
%
%    o geometry: This string is the geometry specification.
%
%    o pedantic: A value other than 0 invokes a more restrictive set of
%      conditions for a valid specification (e.g. [1], [1-4], [4-1]).
%
*/
MagickExport MagickBooleanType IsSceneGeometry(const char *geometry,
  const MagickBooleanType pedantic)
{
  char
    *p;

  double
    value;

  if (geometry == (const char *) NULL)
    return(MagickFalse);
  p=(char *) geometry;
  value=StringToDouble(geometry,&p);
  if (IsNaN(value) != 0)
    return(MagickFalse);
  if (value > (double) MAGICK_SSIZE_MAX)
    return(MagickFalse);
  if (value < (double) MAGICK_SSIZE_MIN)
    return(MagickFalse);
  if (p == geometry)
    return(MagickFalse);
  if (strspn(geometry,"0123456789-, ") != strlen(geometry))
    return(MagickFalse);
  if ((pedantic != MagickFalse) && (strchr(geometry,',') != (char *) NULL))
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  L i s t P a g e s i z e s                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListPagesizes() lists the pagesizes and their associated geometry.
%
%  The format of the ListPagesizes method is:
%
%      MagickBooleanType ListPagesizes(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to the output FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListPagesizes(FILE *file,
  ExceptionInfo *magick_unused(exception))
{
#define MaxMagickSpaces  ((int) sizeof(Pagesizes[0].name))

  const char
    *spacer = "                    ";

  ssize_t
    i;

  magick_unreferenced(exception);
  if (file == (FILE *) NULL)
    file=stdout;
  (void) FormatLocaleFile(file,"\nPagesize    Geometry \n");
  (void) FormatLocaleFile(file,"---------------------\n");
  for (i=0; *Pagesizes[i].name != '\0'; i++)
    (void) FormatLocaleFile(file,"%s%.*s%s\n",Pagesizes[i].name,
      MaxMagickSpaces-(int) Pagesizes[i].extent,spacer,Pagesizes[i].geometry);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e A b s o l u t e G e o m e t r y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseAbsoluteGeometry() returns a region as defined by the geometry string,
%  without any modification by percentages or gravity.
%
%  It currently just a wrapper around GetGeometry(), but may be expanded in
%  the future to handle other positioning information.
%
%  The format of the ParseAbsoluteGeometry method is:
%
%      MagickStatusType ParseAbsoluteGeometry(const char *geometry,
%        RectangleInfo *region_info)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry string (e.g. "100x100+10+10").
%
%    o region_info: the region as defined by the geometry string.
%
*/
MagickExport MagickStatusType ParseAbsoluteGeometry(const char *geometry,
  RectangleInfo *region_info)
{
  MagickStatusType
    flags;

  flags=GetGeometry(geometry,&region_info->x,&region_info->y,
    &region_info->width,&region_info->height);
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e A f f i n e G e o m e t r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseAffineGeometry() returns an affine matrix as defined by a string of 4
%  to 6 comma/space separated floating point values.
%
%  The affine matrix determinant is checked for validity of the values.
%
%  The format of the ParseAffineGeometry method is:
%
%      MagickStatusType ParseAffineGeometry(const char *geometry,
%        AffineMatrix *affine_matrix,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry string (e.g. "1.0,0.0,0.0,1.0,3.2,1.2").
%
%    o affine_matrix: the affine matrix as defined by the geometry string.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickStatusType ParseAffineGeometry(const char *geometry,
  AffineMatrix *affine_matrix,ExceptionInfo *exception)
{
  char
    token[MagickPathExtent];

  const char
    *p;

  double
    determinant;

  MagickStatusType
    flags;

  ssize_t
    i;

  GetAffineMatrix(affine_matrix);
  flags=NoValue;
  p=(char *) geometry;
  for (i=0; (*p != '\0') && (i < 6); i++)
  {
    (void) GetNextToken(p,&p,MagickPathExtent,token);
    if (*token == ',')
      (void) GetNextToken(p,&p,MagickPathExtent,token);
    switch (i)
    {
      case 0:
      {
        affine_matrix->sx=StringToDouble(token,(char **) NULL);
        break;
      }
      case 1:
      {
        affine_matrix->rx=StringToDouble(token,(char **) NULL);
        break;
      }
      case 2:
      {
        affine_matrix->ry=StringToDouble(token,(char **) NULL);
        break;
      }
      case 3:
      {
        affine_matrix->sy=StringToDouble(token,(char **) NULL);
        break;
      }
      case 4:
      {
        affine_matrix->tx=StringToDouble(token,(char **) NULL);
        flags|=XValue;
        break;
      }
      case 5:
      {
        affine_matrix->ty=StringToDouble(token,(char **) NULL);
        flags|=YValue;
        break;
      }
    }
  }
  determinant=(affine_matrix->sx*affine_matrix->sy-affine_matrix->rx*
    affine_matrix->ry);
  if (fabs(determinant) < MagickEpsilon)
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "InvalidArgument","'%s' : 'Indeterminate Matrix'",geometry);
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e G e o m e t r y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseGeometry() parses a geometry specification and returns the sigma,
%  rho, xi, and psi values.  It also returns flags that indicates which
%  of the four values (sigma, rho, xi, psi) were located in the string, and
%  whether the xi or pi values are negative.
%
%  In addition, it reports if there are any of meta characters (%, !, <, >, @,
%  and ^) flags present. It does not report the location of the percentage
%  relative to the values.
%
%  Values may also be separated by commas, colons, or slashes, and offsets.
%  Chroma subsampling definitions have to be in the form of a:b:c.  Offsets may
%  be prefixed by multiple signs to make offset string substitutions easier to
%  handle from shell scripts.  For example: "-10-10", "-+10-+10", or "+-10+-10"
%  will generate negative offsets, while "+10+10", "++10++10", or "--10--10"
%  will generate positive offsets.
%
%  The format of the ParseGeometry method is:
%
%      MagickStatusType ParseGeometry(const char *geometry,
%        GeometryInfo *geometry_info)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry string (e.g. "100x100+10+10").
%
%    o geometry_info:  returns the parsed width/height/x/y in this structure.
%
*/
MagickExport MagickStatusType ParseGeometry(const char *geometry,
  GeometryInfo *geometry_info)
{
  char
    *p,
    pedantic_geometry[MagickPathExtent],
    *q;

  double
    value;

  GeometryInfo
    coordinate;

  int
    c;

  MagickStatusType
    flags;

  /*
    Remove whitespaces meta characters from geometry specification.
  */
  assert(geometry_info != (GeometryInfo *) NULL);
  (void) memset(geometry_info,0,sizeof(*geometry_info));
  flags=NoValue;
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(flags);
  if (strlen(geometry) >= (MagickPathExtent-1))
    return(flags);
  c=MagickSscanf(geometry,"%lf%*[ ,]%lf%*[ ,]%lf%*[ ,]%lf",&coordinate.rho,
    &coordinate.sigma,&coordinate.xi,&coordinate.psi);
  if (c == 4)
    {
      /*
        Special case: coordinate (e.g. 0,0 255,255).
      */
      geometry_info->rho=coordinate.rho;
      geometry_info->sigma=coordinate.sigma;
      geometry_info->xi=coordinate.xi;
      geometry_info->psi=coordinate.psi;
      flags|=RhoValue | SigmaValue | XiValue | PsiValue;
      return(flags);
    }
  (void) CopyMagickString(pedantic_geometry,geometry,MagickPathExtent);
  for (p=pedantic_geometry; *p != '\0'; )
  {
    c=(int) ((unsigned char) *p);
    if (isspace((int) ((unsigned char) c)) != 0)
      {
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        continue;
      }
    switch (c)
    {
      case '%':
      {
        flags|=PercentValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '!':
      {
        flags|=AspectValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '<':
      {
        flags|=LessValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '>':
      {
        flags|=GreaterValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '#':
      {
        flags|=MaximumValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '^':
      {
        flags|=MinimumValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '@':
      {
        flags|=AreaValue;
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case '(':
      {
        if (*(p+1) == ')')
          return(flags);
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case ')':
      {
        (void) CopyMagickString(p,p+1,MagickPathExtent);
        break;
      }
      case 'x':
      case 'X':
      {
        flags|=SeparatorValue;
        p++;
        break;
      }
      case '-':
      case '+':
      case ',':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '/':
      case 215:
      case 'e':
      case 'E':
      {
        p++;
        break;
      }
      case '.':
      {
        p++;
        flags|=DecimalValue;
        break;
      }
      case ':':
      {
        p++;
        flags|=AspectRatioValue;
        break;
      }
      default:
        return(NoValue);
    }
  }
  /*
    Parse rho, sigma, xi, psi, and optionally chi.
  */
  p=pedantic_geometry;
  if (*p == '\0')
    return(flags);
  q=p;
  value=StringToDouble(p,&q);
  if (LocaleNCompare(p,"0x",2) == 0)
    (void) strtol(p,&q,10);
  c=(int) ((unsigned char) *q);
  if ((c == 215) || (*q == 'x') || (*q == 'X') || (*q == ':') ||
      (*q == ',') || (*q == '/') || (*q =='\0'))
    {
      /*
        Parse rho.
      */
      q=p;
      if (LocaleNCompare(p,"0x",2) == 0)
        value=(double) strtol(p,&p,10);
      else
        value=StringToDouble(p,&p);
      if (p != q)
        {
          flags|=RhoValue;
          geometry_info->rho=value;
        }
    }
  q=p;
  c=(int) ((unsigned char) *p);
  if ((c == 215) || (*p == 'x') || (*p == 'X') || (*p == ':') || (*p == ',') ||
      (*p == '/'))
    {
      /*
        Parse sigma.
      */
      p++;
      while (isspace((int) ((unsigned char) *p)) != 0)
        p++;
      c=(int) ((unsigned char) *q);
      if (((c != 215) && (*q != 'x') && (*q != 'X') && (*q != ':')) ||
          ((*p != '+') && (*p != '-')))
        {
          q=p;
          value=StringToDouble(p,&p);
          if (p != q)
            {
              flags|=SigmaValue;
              geometry_info->sigma=value;
            }
        }
    }
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  if ((*p == '+') || (*p == '-') || (*p == ',') || (*p == '/') || (*p == ':'))
    {
      /*
        Parse xi value.
      */
      if ((*p == ',') || (*p == '/') || (*p == ':') )
        p++;
      while ((*p == '+') || (*p == '-'))
      {
        if (*p == '-')
          flags^=XiNegative;  /* negate sign */
        p++;
      }
      q=p;
      value=StringToDouble(p,&p);
      if (p != q)
        {
          flags|=XiValue;
          if ((flags & XiNegative) != 0)
            value=(-value);
          geometry_info->xi=value;
        }
      while (isspace((int) ((unsigned char) *p)) != 0)
        p++;
      if ((*p == '+') || (*p == '-') || (*p == ',') || (*p == '/') ||
          (*p == ':'))
        {
          /*
            Parse psi value.
          */
          if ((*p == ',') || (*p == '/') || (*p == ':'))
            p++;
          while ((*p == '+') || (*p == '-'))
          {
            if (*p == '-')
              flags^=PsiNegative;  /* negate sign */
            p++;
          }
          q=p;
          value=StringToDouble(p,&p);
          if (p != q)
            {
              flags|=PsiValue;
              if ((flags & PsiNegative) != 0)
                value=(-value);
              geometry_info->psi=value;
            }
      }
      while (isspace((int) ((unsigned char) *p)) != 0)
        p++;
      if ((*p == '+') || (*p == '-') || (*p == ',') || (*p == '/') ||
          (*p == ':'))
        {
          /*
            Parse chi value.
          */
          if ((*p == ',') || (*p == '/') || (*p == ':'))
            p++;
          while ((*p == '+') || (*p == '-'))
          {
            if (*p == '-')
              flags^=ChiNegative;  /* negate sign */
            p++;
          }
          q=p;
          value=StringToDouble(p,&p);
          if (p != q)
            {
              flags|=ChiValue;
              if ((flags & ChiNegative) != 0)
                value=(-value);
              geometry_info->chi=value;
            }
        }
    }
  if (strchr(pedantic_geometry,':') != (char *) NULL)
    {
      /*
        Normalize sampling factor (e.g. 4:2:2 => 2x1).
      */
      if ((flags & SigmaValue) != 0)
        geometry_info->rho*=MagickSafeReciprocal(geometry_info->sigma);
      geometry_info->sigma=1.0;
      if (((flags & XiValue) != 0) && (geometry_info->xi == 0.0))
        geometry_info->sigma=2.0;
    }
  if (((flags & RhoValue) != 0) && ((flags & SigmaValue) == 0) &&
      ((flags & XiValue) != 0) && ((flags & XiNegative) != 0))
    {
      if ((flags & PsiValue) == 0)
        {
          /*
            Support negative height values (e.g. 30x-20).
          */
          geometry_info->sigma=geometry_info->xi;
          geometry_info->xi=0.0;
          flags|=SigmaValue;
          flags&=(unsigned int) (~XiValue);
        }
      else
        if ((flags & ChiValue) == 0)
          {
            /*
              Support negative height values (e.g. 30x-20+10).
            */
            geometry_info->sigma=geometry_info->xi;
            geometry_info->xi=geometry_info->psi;
            flags|=SigmaValue;
            flags|=XiValue;
            flags&=(unsigned int) (~PsiValue);
          }
        else
          {
            /*
              Support negative height values (e.g. 30x-20+10+10).
            */
            geometry_info->sigma=geometry_info->xi;
            geometry_info->xi=geometry_info->psi;
            geometry_info->psi=geometry_info->chi;
            flags|=SigmaValue;
            flags|=XiValue;
            flags|=PsiValue;
            flags&=(unsigned int) (~ChiValue);
          }
    }
  if ((flags & PercentValue) != 0)
    {
      if (((flags & SeparatorValue) == 0) && ((flags & SigmaValue) == 0))
        geometry_info->sigma=geometry_info->rho;
      if (((flags & SeparatorValue) != 0) && ((flags & RhoValue) == 0))
        geometry_info->rho=geometry_info->sigma;
    }
#if 0
  /* Debugging Geometry */
  (void) fprintf(stderr,"ParseGeometry...\n");
  (void) fprintf(stderr,"Flags: %c %c %s %s %s\n",
    (flags & RhoValue) ? 'W' : ' ',(flags & SigmaValue) ? 'H' : ' ',
    (flags & XiValue) ? ((flags & XiNegative) ? "-X" : "+X") : "  ",
    (flags & PsiValue) ? ((flags & PsiNegative) ? "-Y" : "+Y") : "  ",
    (flags & ChiValue) ? ((flags & ChiNegative) ? "-Z" : "+Z") : "  ");
  (void) fprintf(stderr,"Geometry: %lg,%lg,%lg,%lg,%lg\n",geometry_info->rho,
    geometry_info->sigma,geometry_info->xi,geometry_info->psi,
    geometry_info->chi);
#endif
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e G r a v i t y G e o m e t r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseGravityGeometry() returns a region as defined by the geometry string
%  with respect to the given image page (canvas) dimensions and the images
%  gravity setting.
%
%  This is typically used for specifying a area within a given image for
%  cropping images to a smaller size, chopping out rows and or columns, or
%  resizing and positioning overlay images.
%
%  Percentages are relative to image size and not page size, and are set to
%  nearest integer (pixel) size.
%
%  The format of the ParseGravityGeometry method is:
%
%      MagickStatusType ParseGravityGeometry(Image *image,const char *geometry,
%        RectangleInfo *region_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry string (e.g. "100x100+10+10").
%
%    o region_info: the region as defined by the geometry string with respect
%      to the image dimensions and its gravity.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickStatusType ParseGravityGeometry(const Image *image,
  const char *geometry,RectangleInfo *region_info,ExceptionInfo *exception)
{
  MagickStatusType
    flags;

  size_t
    height,
    width;

  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",geometry);
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(NoValue);
  SetGeometry(image,region_info);
  if (image->page.width != 0)
    region_info->width=image->page.width;
  if (image->page.height != 0)
    region_info->height=image->page.height;
  flags=ParseAbsoluteGeometry(geometry,region_info);
  if (flags == NoValue)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidGeometry","`%s'",geometry);
      return(flags);
    }
  if ((flags & PercentValue) != 0)
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        status;

      PointInfo
        scale;

      /*
        Geometry is a percentage of the image size, not canvas size
      */
      if (image->gravity != UndefinedGravity)
        flags|=XValue | YValue;
      status=ParseGeometry(geometry,&geometry_info);
      scale.x=geometry_info.rho;
      if ((status & RhoValue) == 0)
        scale.x=100.0;
      scale.y=geometry_info.sigma;
      if ((status & SigmaValue) == 0)
        scale.y=scale.x;
      region_info->width=CastDoubleToSizeT(scale.x*image->columns/100.0+0.5);
      region_info->height=CastDoubleToSizeT(scale.y*image->rows/100.0+0.5);
    }
  if ((flags & AspectRatioValue) != 0)
    {
      double
        geometry_ratio,
        image_ratio;

      GeometryInfo
        geometry_info;

      /*
        Geometry is a relative to image size and aspect ratio.
      */
      if (image->gravity != UndefinedGravity)
        flags|=XValue | YValue;
      (void) ParseGeometry(geometry,&geometry_info);
      geometry_ratio=geometry_info.rho;
      image_ratio=(double) image->columns/image->rows;
      region_info->width=image->columns;
      region_info->height=image->rows;
      if ((flags & MaximumValue) != 0)
        {
          if (geometry_ratio < image_ratio)
            region_info->height=CastDoubleToSizeT((double) image->rows*
              image_ratio/geometry_ratio+0.5);
          else
            region_info->width=CastDoubleToSizeT((double) image->columns*
              geometry_ratio/image_ratio+0.5);
        }
      else
        if (geometry_ratio >= image_ratio)
          region_info->height=CastDoubleToSizeT((double) image->rows*
            image_ratio/geometry_ratio+0.5);
        else
          region_info->width=CastDoubleToSizeT((double) image->columns*
            geometry_ratio/image_ratio+0.5);
    }
  /*
    Adjust offset according to gravity setting.
  */
  width=region_info->width;
  height=region_info->height;
  if (width == 0)
    region_info->width=image->page.width | image->columns;
  if (height == 0)
    region_info->height=image->page.height | image->rows;
  GravityAdjustGeometry(image->columns,image->rows,image->gravity,region_info);
  if ((flags & LessValue) != 0)
    if ((region_info->width < image->columns) &&
        (region_info->height < image->rows))
      {
        SetGeometry(image,region_info);
        return(NoValue);
      }
  if ((flags & GreaterValue) != 0)
    if ((region_info->width > image->columns) &&
        (region_info->height > image->rows))
      {
        SetGeometry(image,region_info);
        return(NoValue);
      }
  region_info->width=width;
  region_info->height=height;
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P a r s e M e t a G e o m e t r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseMetaGeometry() is similar to GetGeometry() except the returned
%  geometry is modified as determined by the meta characters:  %, !, <, >, @,
%  :, and ^ in relation to image resizing.
%
%  Final image dimensions are adjusted so as to preserve the aspect ratio as
%  much as possible, while generating a integer (pixel) size, and fitting the
%  image within the specified geometry width and height.
%
%  Flags are interpreted...
%     %   geometry size is given percentage of original width and height given
%     !   do not try to preserve aspect ratio
%     <   only enlarge images smaller that geometry
%     >   only shrink images larger than geometry
%     @   fit image to contain at most this many pixels
%     :   width and height denotes an aspect ratio
%     ^   contain the given geometry given, (minimal dimensions given)
%
%  The format of the ParseMetaGeometry method is:
%
%      MagickStatusType ParseMetaGeometry(const char *geometry,ssize_t *x,
%        ssize_t *y, size_t *width,size_t *height)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry string (e.g. "100x100+10+10").
%
%    o x,y:  The x and y offset, set according to the geometry specification.
%
%    o width,height:  The width and height of original image, modified by
%      the given geometry specification.
%
*/
MagickExport MagickStatusType ParseMetaGeometry(const char *geometry,ssize_t *x,
  ssize_t *y,size_t *width,size_t *height)
{
  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  size_t
    stasis_height,
    stasis_width;

  /*
    Ensure the image geometry is valid.
  */
  assert(x != (ssize_t *) NULL);
  assert(y != (ssize_t *) NULL);
  assert(width != (size_t *) NULL);
  assert(height != (size_t *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",geometry);
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(NoValue);
  /*
    Parse geometry using GetGeometry.
  */
  stasis_width=(*width);
  stasis_height=(*height);
  SetGeometryInfo(&geometry_info);
  flags=GetGeometry(geometry,x,y,width,height);
  if ((flags & PercentValue) != 0)
    {
      MagickStatusType
        percent_flags;

      PointInfo
        scale;

      /*
        Geometry is a percentage of the image size.
      */
      percent_flags=ParseGeometry(geometry,&geometry_info);
      scale.x=geometry_info.rho;
      if ((percent_flags & RhoValue) == 0)
        scale.x=100.0;
      scale.y=geometry_info.sigma;
      if ((percent_flags & SigmaValue) == 0)
        scale.y=scale.x;
      *width=CastDoubleToSizeT(scale.x*stasis_width/100.0+0.5);
      *height=CastDoubleToSizeT(scale.y*stasis_height/100.0+0.5);
      stasis_width=(*width);
      stasis_height=(*height);
    }
  if ((flags & AspectRatioValue) != 0)
    {
      double
        geometry_ratio,
        image_ratio;

      /*
        Geometry is a relative to image size and aspect ratio.
      */
      (void) ParseGeometry(geometry,&geometry_info);
      geometry_ratio=geometry_info.rho;
      image_ratio=(double) stasis_width*MagickSafeReciprocal((double)
        stasis_height);
      if (geometry_ratio >= image_ratio)
        {
          *width=stasis_width;
          *height=CastDoubleToSizeT((double) (MagickSafeReciprocal(
            geometry_ratio)*stasis_height*image_ratio)+0.5);
        }
      else
        {
          *width=CastDoubleToSizeT(MagickSafeReciprocal(image_ratio)*
            stasis_width*geometry_ratio+0.5);
          *height=stasis_height;
        }
      stasis_width=(*width);
      stasis_height=(*height);
    }
  if (((flags & AspectValue) != 0) || ((*width == stasis_width) &&
      (*height == stasis_height)))
    {
      if ((flags & RhoValue) == 0)
        *width=stasis_width;
      if ((flags & SigmaValue) == 0)
        *height=stasis_height;
    }
  else
    {
      double
        scale_factor;

      /*
        Respect aspect ratio of the image.
      */
      if ((stasis_width == 0) || (stasis_height == 0))
        scale_factor=1.0;
      else
        if (((flags & RhoValue) != 0) && (flags & SigmaValue) != 0)
          {
            scale_factor=(double) *width/(double) stasis_width;
            if ((flags & MinimumValue) == 0)
              {
                if (scale_factor > ((double) *height/(double) stasis_height))
                  scale_factor=(double) *height/(double) stasis_height;
              }
            else
              if (scale_factor < ((double) *height/(double) stasis_height))
                scale_factor=(double) *height/(double) stasis_height;
          }
        else
          if ((flags & RhoValue) != 0)
            {
              scale_factor=(double) *width/(double) stasis_width;
              if (((flags & MinimumValue) != 0) &&
                  (scale_factor < ((double) *width/(double) stasis_height)))
                scale_factor=(double) *width/(double) stasis_height;
            }
          else
            {
              scale_factor=(double) *height/(double) stasis_height;
              if (((flags & MinimumValue) != 0) &&
                  (scale_factor < ((double) *height/(double) stasis_width)))
                scale_factor=(double) *height/(double) stasis_width;
            }
      *width=CastDoubleToSizeT(MagickMax(floor(scale_factor*stasis_width+0.5),
        1.0));
      *height=CastDoubleToSizeT(MagickMax(floor(scale_factor*stasis_height+0.5),
        1.0));
    }
  if ((flags & GreaterValue) != 0)
    {
      if (stasis_width < *width)
        *width=stasis_width;
      if (stasis_height < *height)
        *height=stasis_height;
    }
  if ((flags & LessValue) != 0)
    {
      if (stasis_width > *width)
        *width=stasis_width;
      if (stasis_height > *height)
        *height=stasis_height;
    }
  if ((flags & AreaValue) != 0)
    {
      double
        area,
        distance;

      PointInfo
        scale;

      /*
        Geometry is a maximum area in pixels.
      */
      (void) ParseGeometry(geometry,&geometry_info);
      area=geometry_info.rho+sqrt(MagickEpsilon);
      distance=sqrt((double) stasis_width*stasis_height);
      scale.x=(double) stasis_width*MagickSafeReciprocal(distance*
        MagickSafeReciprocal(sqrt(area)));
      scale.y=(double) stasis_height*MagickSafeReciprocal(distance*
        MagickSafeReciprocal(sqrt(area)));
      if ((scale.x < (double) *width) || (scale.y < (double) *height))
        {
          *width=CastDoubleToSizeT((double) stasis_width*MagickSafeReciprocal(
            distance*MagickSafeReciprocal(sqrt(area)))+0.5);
          *height=CastDoubleToSizeT((double) stasis_height*MagickSafeReciprocal(
            distance*MagickSafeReciprocal(sqrt(area)))+0.5);
        }
    }
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e P a g e G e o m e t r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParsePageGeometry() returns a region as defined by the geometry string with
%  respect to the image page (canvas) dimensions.
%
%  WARNING: Percentage dimensions remain relative to the actual image
%  dimensions, and not canvas dimensions.
%
%  The format of the ParsePageGeometry method is:
%
%      MagickStatusType ParsePageGeometry(const Image *image,
%        const char *geometry,RectangleInfo *region_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry string (e.g. "100x100+10+10").
%
%    o region_info: the region as defined by the geometry string with
%      respect to the image and its gravity.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickStatusType ParsePageGeometry(const Image *image,
  const char *geometry,RectangleInfo *region_info,ExceptionInfo *exception)
{
  MagickStatusType
    flags;

  SetGeometry(image,region_info);
  if (image->page.width != 0)
    region_info->width=image->page.width;
  if (image->page.height != 0)
    region_info->height=image->page.height;
  flags=ParseAbsoluteGeometry(geometry,region_info);
  if (flags == NoValue)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidGeometry","`%s'",geometry);
      return(flags);
    }
  if ((flags & PercentValue) != 0)
    {
      region_info->width=image->columns;
      region_info->height=image->rows;
    }
  flags=ParseMetaGeometry(geometry,&region_info->x,&region_info->y,
    &region_info->width,&region_info->height);
  if ((((flags & WidthValue) != 0) || ((flags & HeightValue) != 0)) &&
      (((flags & PercentValue) != 0) || ((flags & SeparatorValue) == 0)))
    {
      if ((flags & WidthValue) == 0)
        region_info->width=region_info->height;
      if ((flags & HeightValue) == 0)
        region_info->height=region_info->width;
    }
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e R e g i o n G e o m e t r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseRegionGeometry() returns a region as defined by the geometry string
%  with respect to the image dimensions and aspect ratio.
%
%  This is basically a wrapper around ParseMetaGeometry.  This is typically
%  used to parse a geometry string to work out the final integer dimensions
%  for image resizing.
%
%  The format of the ParseRegionGeometry method is:
%
%      MagickStatusType ParseRegionGeometry(const Image *image,
%        const char *geometry,RectangleInfo *region_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry string (e.g. "100x100+10+10").
%
%    o region_info: the region as defined by the geometry string.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickStatusType ParseRegionGeometry(const Image *image,
  const char *geometry,RectangleInfo *region_info,ExceptionInfo *exception)
{
  MagickStatusType
    flags;

  SetGeometry(image,region_info);
  flags=ParseMetaGeometry(geometry,&region_info->x,&region_info->y,
    &region_info->width,&region_info->height);
  if (flags == NoValue)
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "InvalidGeometry","`%s'",geometry);
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t G e o m e t r y                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetGeometry() sets the geometry to its default values.
%
%  The format of the SetGeometry method is:
%
%      SetGeometry(const Image *image,RectangleInfo *geometry)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o geometry: the geometry.
%
*/
MagickExport void SetGeometry(const Image *image,RectangleInfo *geometry)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (RectangleInfo *) NULL);
  (void) memset(geometry,0,sizeof(*geometry));
  geometry->width=image->columns;
  geometry->height=image->rows;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t G e o m e t r y I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetGeometryInfo sets the GeometryInfo structure to its default values.
%
%  The format of the SetGeometryInfo method is:
%
%      SetGeometryInfo(GeometryInfo *geometry_info)
%
%  A description of each parameter follows:
%
%    o geometry_info: the geometry info structure.
%
*/
MagickExport void SetGeometryInfo(GeometryInfo *geometry_info)
{
  assert(geometry_info != (GeometryInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  (void) memset(geometry_info,0,sizeof(*geometry_info));
}

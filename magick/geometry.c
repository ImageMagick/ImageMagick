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
%                               John Cristy                                   %
%                              January 2003                                   %
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
#include "magick/constitute.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/memory_.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"

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
%  The format of the GetGeometry method is:
%
%      MagickStatusType GetGeometry(const char *geometry,long *x,long *y,
%        unsigned long *width,unsigned long *height)
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
MagickExport MagickStatusType GetGeometry(const char *geometry,long *x,long *y,
  unsigned long *width,unsigned long *height)
{
  char
    *p,
    pedantic_geometry[MaxTextExtent],
    *q;

  double
    value;

  MagickStatusType
    flags;

  /*
    Remove whitespace and meta characters from geometry specification.
  */
  flags=NoValue;
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(flags);
  if (strlen(geometry) >= MaxTextExtent)
    return(flags);
  (void) CopyMagickString(pedantic_geometry,geometry,MaxTextExtent);
  for (p=pedantic_geometry; *p != '\0'; )
  {
    if (isspace((int) ((unsigned char) *p)) != 0)
      {
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        continue;
      }
    switch (*p)
    {
      case '%':
      {
        flags|=PercentValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '!':
      {
        flags|=AspectValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '<':
      {
        flags|=LessValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '>':
      {
        flags|=GreaterValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '^':
      {
        flags|=MinimumValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '@':
      {
        flags|=AreaValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '(':
      case ')':
      {
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '-':
      case '.':
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
      case -41:
      case 'x':
      case 'X':
      {
        p++;
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
  value=strtod(p,&q);
  if (LocaleNCompare(p,"0x",2) == 0)
    value=(double) strtol(p,&q,10);
  if (((int) *q == -41) || (*q == 'x') || (*q == 'X') || (*q == '\0'))
    {
      /*
        Parse width.
      */
      q=p;
      if (LocaleNCompare(p,"0x",2) == 0)
        *width=(unsigned long) strtol(p,&p,10);
      else
        *width=(unsigned long) floor(strtod(p,&p)+0.5);
      if (p != q)
        flags|=WidthValue;
    }
  if (((int) *p == -41) || (*p == 'x') || (*p == 'X'))
    {
      p++;
      if ((*p != '+') && (*p != '-'))
        {
          /*
            Parse height.
          */
          q=p;
          *height=(unsigned long) floor(strtod(p,&p)+0.5);
          if (p != q)
            flags|=HeightValue;
        }
    }
  if ((*p == '+') || (*p == '-'))
    {
      /*
        Parse x value.
      */
      if (*p == '-')
        flags|=XNegative;
      q=p;
      *x=(long) ceil(strtod(p,&p)-0.5);
      if (p != q)
        flags|=XValue;
      if ((*p == '+') || (*p == '-'))
        {
          /*
            Parse y value.
          */
          if (*p == '-')
            flags|=YNegative;
          q=p;
          *y=(long) ceil(strtod(p,&p)-0.5);
          if (p != q)
            flags|=YValue;
        }
    }
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
%  GetPageGeometry() replaces any page mneumonic with the equivalent size in
%  picas.
%
%  The format of the GetPageGeometry method is:
%
%      char *GetPageGeometry(const char *page_geometry)
%
%  A description of each parameter follows.
%
%   o  page_geometry:  Specifies a pointer to an array of characters.
%      The string is either a Postscript page name (e.g. A4) or a postscript
%      page geometry (e.g. 612x792+36+36).
%
*/
MagickExport char *GetPageGeometry(const char *page_geometry)
{
  static const char
    *PageSizes[][2]=
    {
      { "4x6",  "288x432" },
      { "5x7",  "360x504" },
      { "7x9",  "504x648" },
      { "8x10", "576x720" },
      { "9x11",  "648x792" },
      { "9x12",  "648x864" },
      { "10x13",  "720x936" },
      { "10x14",  "720x1008" },
      { "11x17",  "792x1224" },
      { "a0",  "2384x3370" },
      { "a1",  "1684x2384" },
      { "a10", "73x105" },
      { "a2",  "1191x1684" },
      { "a3",  "842x1191" },
      { "a4",  "595x842" },
      { "a4smaLL", "595x842" },
      { "a5",  "420x595" },
      { "a6",  "297x420" },
      { "a7",  "210x297" },
      { "a8",  "148x210" },
      { "a9",  "105x148" },
      { "archa", "648x864" },
      { "archb", "864x1296" },
      { "archC", "1296x1728" },
      { "archd", "1728x2592" },
      { "arche", "2592x3456" },
      { "b0",  "2920x4127" },
      { "b1",  "2064x2920" },
      { "b10", "91x127" },
      { "b2",  "1460x2064" },
      { "b3",  "1032x1460" },
      { "b4",  "729x1032" },
      { "b5",  "516x729" },
      { "b6",  "363x516" },
      { "b7",  "258x363" },
      { "b8",  "181x258" },
      { "b9",  "127x181" },
      { "c0",  "2599x3676" },
      { "c1",  "1837x2599" },
      { "c2",  "1298x1837" },
      { "c3",  "918x1296" },
      { "c4",  "649x918" },
      { "c5",  "459x649" },
      { "c6",  "323x459" },
      { "c7",  "230x323" },
      { "executive", "540x720" },
      { "flsa", "612x936" },
      { "flse", "612x936" },
      { "folio",  "612x936" },
      { "halfletter", "396x612" },
      { "isob0", "2835x4008" },
      { "isob1", "2004x2835" },
      { "isob10", "88x125" },
      { "isob2", "1417x2004" },
      { "isob3", "1001x1417" },
      { "isob4", "709x1001" },
      { "isob5", "499x709" },
      { "isob6", "354x499" },
      { "isob7", "249x354" },
      { "isob8", "176x249" },
      { "isob9", "125x176" },
      { "jisb0", "1030x1456" },
      { "jisb1", "728x1030" },
      { "jisb2", "515x728" },
      { "jisb3", "364x515" },
      { "jisb4", "257x364" },
      { "jisb5", "182x257" },
      { "jisb6", "128x182" },
      { "ledger",  "1224x792" },
      { "legal",  "612x1008" },
      { "letter", "612x792" },
      { "lettersmaLL",  "612x792" },
      { "quarto",  "610x780" },
      { "statement",  "396x612" },
      { "tabloid",  "792x1224" },
      { (char *) NULL, (char *) NULL }
    };

  char
    *page;

  register long
    i;

  assert(page_geometry != (char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",page_geometry);
  page=AcquireString(page_geometry);
  for (i=0; *PageSizes[i] != (char *) NULL; i++)
    if (LocaleNCompare(PageSizes[i][0],page,strlen(PageSizes[i][0])) == 0)
      {
        RectangleInfo
          geometry;

        MagickStatusType
          flags;

        /*
          Replace mneumonic with the equivalent size in dots-per-inch.
        */
        (void) CopyMagickString(page,PageSizes[i][1],MaxTextExtent);
        (void) ConcatenateMagickString(page,page_geometry+
          strlen(PageSizes[i][0]),MaxTextExtent);
        flags=GetGeometry(page,&geometry.x,&geometry.y,&geometry.width,
          &geometry.height);
        if ((flags & GreaterValue) == 0)
          (void) ConcatenateMagickString(page,">",MaxTextExtent);
        break;
      }
  return(page);
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
%      void GravityAdjustGeometry(const unsigned long width,
%        const unsigned long height,const GravityType gravity,
%        RectangleInfo *region);
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
MagickExport void GravityAdjustGeometry(const unsigned long width,
  const unsigned long height,const GravityType gravity,RectangleInfo *region)
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
      region->x=(long) (width-region->width-region->x);
      break;
    }
    case NorthGravity:
    case SouthGravity:
    case CenterGravity:
    case StaticGravity:
    {
      region->x+=(long) (width/2-region->width/2);
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
      region->y=(long) (height-region->height-region->y);
      break;
    }
    case EastGravity:
    case WestGravity:
    case CenterGravity:
    case StaticGravity:
    {
      region->y+=(long) (height/2-region->height/2);
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
  if (flags == NoValue)
    flags=ParseGeometry(geometry+1,&geometry_info);  /* i.e. +-4+-4 */
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
  value=strtod(geometry,&p);
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
%   P a r s e A b s o l u t e G e o m e t r y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseAbsoluteGeometry() returns a region as defined by the geometry string.
%
%  The format of the ParseAbsoluteGeometry method is:
%
%      MagickStatusType ParseAbsoluteGeometry(const char *geometry,
%        RectangleInfo *region_info)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry (e.g. 100x100+10+10).
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
%  ParseAffineGeometry() returns an affine matrix as defined by the geometry
%  string.
%
%  The format of the ParseAffineGeometry method is:
%
%      MagickStatusType ParseAffineGeometry(const char *geometry,
%        AffineMatrix *affine_matrix,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry (e.g. 1.0,0.0,0.0,1.0,3.2,1.2).
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
    token[MaxTextExtent];

  const char
    *p;

  double
    determinant;

  MagickStatusType
    flags;

  register long
    i;

  GetAffineMatrix(affine_matrix);
  flags=NoValue;
  p=(char *) geometry;
  for (i=0; (*p != '\0') && (i < 6); i++)
  {
    GetMagickToken(p,&p,token);
    if (*token == ',')
      GetMagickToken(p,&p,token);
    switch (i)
    {
      case 0: affine_matrix->sx=StringToDouble(token); break;
      case 1: affine_matrix->rx=StringToDouble(token); break;
      case 2: affine_matrix->ry=StringToDouble(token); break;
      case 3: affine_matrix->sy=StringToDouble(token); break;
      case 4: affine_matrix->tx=StringToDouble(token); flags|=XValue; break;
      case 5: affine_matrix->ty=StringToDouble(token); flags|=YValue; break;
    }
  }
  determinant=(affine_matrix->sx*affine_matrix->sy-affine_matrix->rx*
    affine_matrix->ry);
  if (fabs(determinant) < MagickEpsilon)
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "InvalidGeometry","`%s'",geometry);
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
%  whether the xi or pi values are negative.  In addition, there are flags to
%  report any meta characters (%, !, <, or >).
%
%  The format of the ParseGeometry method is:
%
%      MagickStatusType ParseGeometry(const char *geometry,
%        GeometryInfo *geometry_info)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry.
%
%    o geometry_info:  returns the parsed width/height/x/y in this structure.
%
*/
MagickExport MagickStatusType ParseGeometry(const char *geometry,
  GeometryInfo *geometry_info)
{
  char
    *p,
    pedantic_geometry[MaxTextExtent],
    *q;

  double
    value;

  MagickStatusType
    flags;

  /*
    Remove whitespaces meta characters from geometry specification.
  */
  assert(geometry_info != (GeometryInfo *) NULL);
  flags=NoValue;
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(flags);
  if (strlen(geometry) >= MaxTextExtent)
    return(flags);
  (void) CopyMagickString(pedantic_geometry,geometry,MaxTextExtent);
  for (p=pedantic_geometry; *p != '\0'; )
  {
    if (isspace((int) ((unsigned char) *p)) != 0)
      {
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        continue;
      }
    switch (*p)
    {
      case '%':
      {
        flags|=PercentValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '!':
      {
        flags|=AspectValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '<':
      {
        flags|=LessValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '>':
      {
        flags|=GreaterValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '^':
      {
        flags|=MinimumValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '@':
      {
        flags|=AreaValue;
        (void) CopyMagickString(p,p+1,MaxTextExtent);
        break;
      }
      case '(':
      case ')':
      {
        (void) CopyMagickString(p,p+1,MaxTextExtent);
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
      case -41:
      case 'x':
      case 'X':
      case '/':
      case ':':
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
      default:
        return(flags);
    }
  }
  /*
    Parse rho, sigma, xi, psi, and optionally chi.
  */
  p=pedantic_geometry;
  if (*p == '\0')
    return(flags);
  q=p;
  value=strtod(p,&q);
  if (LocaleNCompare(p,"0x",2) == 0)
    value=(double) strtol(p,&q,10);
  if (((int) *q == -41) || (*q == 'x') || (*q == 'X') || (*q == ',') ||
      (*q == '/') || (*q == ':') || (*q =='\0'))
    {
      /*
        Parse rho.
      */
      q=p;
      if (LocaleNCompare(p,"0x",2) == 0)
        value=(double) strtol(p,&p,10);
      else
        value=strtod(p,&p);
      if (p != q)
        {
          flags|=RhoValue;
          geometry_info->rho=value;
        }
    }
  q=p;
  if (((int) *p == -41) || (*p == 'x') || (*p == 'X') || (*p == ',') ||
      (*p == '/') || (*p == ':'))
    {
      /*
        Parse sigma.
      */
      p++;
      while (isspace((int) ((unsigned char) *p)) != 0)
        p++;
      if ((((int) *q != -41) && (*q != 'x') && (*q != 'X')) ||
          ((*p != '+') && (*p != '-')))
        {
          q=p;
          value=strtod(p,&p);
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
      if ((*p == ',') || (*p == '/') || (*p == ':'))
        p++;
      q=p;
      value=strtod(p,&p);
      if (p != q)
        {
          flags|=XiValue;
          if (*q == '-')
            flags|=XiNegative;
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
          q=p;
          value=strtod(p,&p);
          if (p != q)
            {
              flags|=PsiValue;
              if (*q == '-')
                flags|=PsiNegative;
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
          q=p;
          value=strtod(p,&p);
          if (p != q)
            {
              flags|=ChiValue;
              if (*q == '-')
                flags|=ChiNegative;
              geometry_info->chi=value;
            }
        }
    }
  if (strchr(pedantic_geometry,':') != (char *) NULL)
    {
      /*
        Normalize sampling factor (e.g. 4:2:2 => 2x1).
      */
      geometry_info->rho/=geometry_info->sigma;
      geometry_info->sigma=1.0;
      if (geometry_info->xi == 0.0)
        geometry_info->sigma=2.0;
    }
  if (((flags & SigmaValue) == 0) && ((flags & XiValue) != 0) &&
      ((flags & PsiValue) == 0))
    {
      /*
        Support negative height values (e.g. 30x-20).
      */
      geometry_info->sigma=geometry_info->xi;
      geometry_info->xi=0.0;
      flags|=SigmaValue;
      flags&=(~XiValue);
    }
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
%  with respect to the image dimensions and its gravity.
%
%  The format of the ParseGravityGeometry method is:
%
%      MagickStatusType ParseGravityGeometry(Image *image,const char *geometry,
%        RectangeInfo *region_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry (e.g. 100x100+10+10).
%
%    o region_info: the region as defined by the geometry string with
%      respect to the image dimensions and its gravity.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickStatusType ParseGravityGeometry(const Image *image,
  const char *geometry,RectangleInfo *region_info,ExceptionInfo *exception)
{
  MagickStatusType
    flags;

  unsigned long
    height,
    width;

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
        Geometry is a percentage of the image size.
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
      region_info->width=(unsigned long) ((scale.x*image->columns/100.0)+0.5);
      region_info->height=(unsigned long) ((scale.y*image->rows/100.0)+0.5);
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
%  geometry is modified as determined by the meta characters:  %, !, <, >,
%  and ~.
%
%  The format of the ParseMetaGeometry method is:
%
%      MagickStatusType ParseMetaGeometry(const char *geometry,long *x,long *y,
%        unsigned long *width,unsigned long *height)
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

static inline unsigned long MagickMax(const unsigned long x,
  const unsigned long y)
{
  if (x > y)
    return(x);
  return(y);
}

MagickExport MagickStatusType ParseMetaGeometry(const char *geometry,long *x,
  long *y,unsigned long *width,unsigned long *height)
{
  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  unsigned long
    former_height,
    former_width;

  /*
    Ensure the image geometry is valid.
  */
  assert(x != (long *) NULL);
  assert(y != (long *) NULL);
  assert(width != (unsigned long *) NULL);
  assert(height != (unsigned long *) NULL);
  if ((geometry == (char *) NULL) || (*geometry == '\0'))
    return(NoValue);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",geometry);
  /*
    Parse geometry using GetGeometry.
  */
  SetGeometryInfo(&geometry_info);
  former_width=(*width);
  former_height=(*height);
  flags=GetGeometry(geometry,x,y,width,height);
  if ((flags & PercentValue) != 0)
    {
      MagickStatusType
        flags;

      PointInfo
        scale;

      /*
        Geometry is a percentage of the image size.
      */
      flags=ParseGeometry(geometry,&geometry_info);
      scale.x=geometry_info.rho;
      if ((flags & RhoValue) == 0)
        scale.x=100.0;
      scale.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        scale.y=scale.x;
      *width=(unsigned long) (scale.x*former_width/100.0+0.5);
      if (*width == 0)
        *width=1;
      *height=(unsigned long) (scale.y*former_height/100.0+0.5);
      if (*height == 0)
        *height=1;
      former_width=(*width);
      former_height=(*height);
    }
  if (((flags & AspectValue) == 0) && ((*width != former_width) ||
      (*height != former_height)))
    {
      MagickRealType
        scale_factor;

      /*
        Respect aspect ratio of the image.
      */
      if ((former_width == 0) || (former_height == 0))
        scale_factor=1.0;
      else
        if (((flags & RhoValue) != 0) && (flags & SigmaValue) != 0)
          {
            scale_factor=(MagickRealType) *width/(MagickRealType) former_width;
            if ((flags & MinimumValue) == 0)
              {
                if (scale_factor > ((MagickRealType) *height/
                    (MagickRealType) former_height))
                  scale_factor=(MagickRealType) *height/(MagickRealType)
                    former_height;
              }
            else
              if (scale_factor < ((MagickRealType) *height/
                  (MagickRealType) former_height))
                scale_factor=(MagickRealType) *height/(MagickRealType)
                  former_height;
          }
        else
          if ((flags & RhoValue) != 0)
            {
              scale_factor=(MagickRealType) *width/(MagickRealType)
                former_width;
              if (((flags & MinimumValue) != 0) &&
                  (scale_factor < ((MagickRealType) *width/
                    (MagickRealType) former_height)))
                scale_factor=(MagickRealType) *width/(MagickRealType)
                  former_height;
            }
          else
            {
              scale_factor=(MagickRealType) *height/(MagickRealType)
                former_height;
              if (((flags & MinimumValue) != 0) &&
                  (scale_factor < ((MagickRealType) *height/
                    (MagickRealType) former_width)))
                scale_factor=(MagickRealType) *height/(MagickRealType)
                  former_width;
            }
      *width=MagickMax((unsigned long) (scale_factor*former_width+0.5),1UL);
      *height=MagickMax((unsigned long) (scale_factor*former_height+0.5),1UL);
    }
  if ((flags & GreaterValue) != 0)
    {
      if (former_width < *width)
        *width=former_width;
      if (former_height < *height)
        *height=former_height;
    }
  if ((flags & LessValue) != 0)
    {
      if (former_width > *width)
        *width=former_width;
      if (former_height > *height)
        *height=former_height;
    }
  if ((flags & AreaValue) != 0)
    {
      MagickRealType
        area,
        distance;

      PointInfo
        scale;

      /*
        Geometry is a maximum area in pixels.
      */
      (void) ParseGeometry(geometry,&geometry_info);
      area=geometry_info.rho;
      distance=sqrt((double) former_width*former_height);
      scale.x=(double) former_width/(double) (distance/sqrt((double) area));
      scale.y=(double) former_height/(double) (distance/sqrt((double) area));
      if ((scale.x < (double) *width) || (scale.y < (double) *height))
        {
          *width=(unsigned long) (former_width/(distance/sqrt((double) area)));
          *height=(unsigned long) (former_height/(distance/
            sqrt((double) area)));
        }
      former_width=(*width);
      former_height=(*height);
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
%  respect to the image dimensions.
%
%  The format of the ParsePageGeometry method is:
%
%      MagickStatusType ParsePageGeometry(const Image *image,
%        const char *geometry,RectangeInfo *region_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry (e.g. 100x100+10+10).
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
%  The format of the ParseRegionGeometry method is:
%
%      MagickStatusType ParseRegionGeometry(const Image *image,
%        const char *geometry,RectangeInfo *region_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry (e.g. 100x100+10+10).
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (RectangleInfo *) NULL);
  (void) ResetMagickMemory(geometry,0,sizeof(*geometry));
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
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  (void) ResetMagickMemory(geometry_info,0,sizeof(*geometry_info));
}

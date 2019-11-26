/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         DDDD   RRRR    AAA   W   W                          %
%                         D   D  R   R  A   A  W   W                          %
%                         D   D  RRRR   AAAAA  W   W                          %
%                         D   D  R R    A   A  W W W                          %
%                         DDDD   R  R   A   A   W W                           %
%                                                                             %
%                         TTTTT  EEEEE  SSSSS  TTTTT                          %
%                           T    E      SS       T                            %
%                           T    EEE     SSS     T                            %
%                           T    E         SS    T                            %
%                           T    EEEEE  SSSSS    T                            %
%                                                                             %
%                                                                             %
%                         MagickWand Drawing Tests                            %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                              Bob Friesenhahn                                %
%                                March 2002                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

#define ThrowWandException(wand) \
{ \
  char \
    *description; \
 \
  ExceptionType \
    severity; \
 \
  description=MagickGetException(wand,&severity); \
  (void) FormatLocaleFile(stderr,"%s %s %lu %s\n",GetMagickModule(), \
    description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

static MagickBooleanType ScribbleImage(MagickWand *canvas)
{
  DrawingWand
    *picasso;

  PixelWand
    *color;

  picasso=NewDrawingWand();
  color=NewPixelWand();
  (void) PushDrawingWand(picasso);
  {
    DrawSetViewbox(picasso,0,0,(ssize_t) MagickGetImageWidth(canvas),
      (ssize_t) MagickGetImageHeight(canvas));
    DrawScale(picasso,1.101,1.08);
    DrawTranslate(picasso,-23.69,-22.97);
    DrawRotate(picasso,0);
    (void) PixelSetColor(color,"#ffffff");
    DrawSetFillColor(picasso,color);
    DrawRectangle(picasso,23.69,22.97,564.6,802.2);
    DrawSetFillOpacity(picasso,1.0);
    (void) PixelSetColor(color,"none");
    DrawSetFillColor(picasso,color);
    DrawSetStrokeColor(picasso,color);
    DrawSetStrokeAntialias(picasso,MagickTrue);
    DrawSetStrokeLineCap(picasso,RoundCap);
    DrawSetStrokeLineJoin(picasso,RoundJoin);
    DrawPushDefs(picasso);
    {
      DrawPushClipPath(picasso,"clip_1");
      {
        (void) PushDrawingWand(picasso);
        {
          DrawRectangle(picasso,0,0,595.3,841.9);
        }
        (void) PopDrawingWand(picasso);
      }
      DrawPopClipPath(picasso);
    }
    DrawPopDefs(picasso);
    (void) PushDrawingWand(picasso);
    {
      (void) DrawSetClipPath(picasso, "url(#clip_1)");

      (void) PushDrawingWand(picasso);
      {
        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,4.032);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#ff0000");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#ff00ff");
        DrawSetFillColor(picasso,color);
        DrawRectangle(picasso,72,72,144,144);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,9);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#00ff00");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#0080ff");
        DrawSetFillColor(picasso,color);
        DrawRoundRectangle(picasso,72,216,360,432,9,9);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        const PointInfo points[37] =
        {
          { 378.1,81.72 }, { 381.1,79.56 }, { 384.3,78.12 }, { 387.6,77.33 },
          { 391.1,77.11 }, { 394.6,77.62 }, { 397.8,78.77 }, { 400.9,80.57 },
          { 403.6,83.02 }, { 523.9,216.8 }, { 526.2,219.7 }, { 527.6,223 },
          { 528.4,226.4 }, { 528.6,229.8 }, { 528,233.3 },   { 526.9,236.5 },
          { 525.1,239.5 }, { 522.6,242.2 }, { 495.9,266.3 }, { 493,268.5 },
          { 489.7,269.9 }, { 486.4,270.8 }, { 482.9,270.9 }, { 479.5,270.4 },
          { 476.2,269.3 }, { 473.2,267.5 }, { 470.4,265 },   { 350,131.2 },
          { 347.8,128.3 }, { 346.4,125.1 }, { 345.6,121.7 }, {345.4,118.2 },
          { 346,114.8 },   { 347.1,111.5 }, { 348.9,108.5 }, { 351.4,105.8 },
          { 378.1,81.72 }
        };

        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,2.016);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#000080");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#c2c280");
        DrawSetFillColor(picasso,color);
        DrawPolygon(picasso,37,points);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,3.024);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#000080");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#000080");
        DrawSetFillColor(picasso,color);
        DrawEllipse(picasso,489.6,424.8,72,129.6,0,360);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        const PointInfo points[48] =
        {
          { 213.8,25.13},  { 216.7,24.48 }, {219.8,24.55 },  { 223.1,25.42 },
          { 226.7,27 },    { 230.3,29.3 },  { 234.1,32.26 }, { 237.9,35.86 },
          { 241.8,40.03 }, { 249.7,50.11 }, { 257.4,62.14 }, { 264.8,75.89 },
          { 271.6,91.15 }, { 277.3,106.8 }, { 281.6,121.8 }, { 284.4,135.9 },
          { 285.7,148.5 }, { 285.6,159.6 }, { 284.9,164.3 }, { 283.8,168.5 },
          { 282.5,172.1 }, { 280.7,175 },   { 278.5,177.3 }, { 275.9,178.7 },
          { 273,179.4 },   { 269.9,179.3 }, { 266.6,178.4 }, { 263.1,176.8 },
          { 259.5,174.5},  { 255.7,171.6 }, { 251.9,168 },   { 248,163.8 },
          { 244.1,159 },   { 240.1,153.7 }, { 232.3,141.7 }, { 225,127.9 },
          { 218.2,112.7 }, { 212.5,97.06 }, { 208.2,82.01 }, { 205.4,67.97 },
          { 204,55.3 },    { 204.3,44.35 }, { 204.9,39.6 },  { 205.9,35.42 },
          { 207.4,31.82 }, { 209.2,28.87 }, { 211.3,26.64},  { 213.8,25.13 }
        };

        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,3.024);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#ff8000");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#00ffff");
        DrawSetFillColor(picasso,color);
        DrawPolygon(picasso,48,points);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,12.02);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetStrokeColor(picasso,color);
        (void) PixelSetColor(color,"none");
        DrawSetFillColor(picasso,color);
        DrawArc(picasso,360,554.4,187.2,237.6,0,90);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,9);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetFillColor(picasso,color);
        DrawEllipse(picasso,388.8,626.4,100.8,122.4,0,90);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        const PointInfo points[6] =
        {
          { 180,504 }, { 282.7,578.6 }, { 243.5,699.4 }, { 116.5,699.4 },
          { 77.26,578.6 }, { 180,504 }
        };

        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,9);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#800000");
        DrawSetFillColor(picasso,color);
        DrawPolygon(picasso,6,points);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        const PointInfo points[11] =
        {
          { 180,504 },     { 211.8,568.3 }, { 282.7,578.6 }, { 231.3,628.7 },
          { 243.5,699.4 }, { 180,666 },     { 116.5,699.4 }, { 128.7,628.7 },
          { 77.26,578.6 }, { 148.2,568.3 }, { 180,504 }
        };

        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,9);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#800000");
        DrawSetFillColor(picasso,color);
        DrawPolygon(picasso,11,points);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        const PointInfo points[15] =
        {
          { 540,288 },     { 561.6,216 },   { 547.2,43.2 },  { 280.8,36 },
          { 302.4,194.4 }, { 331.2,64.8 },  { 504,64.8 },    { 475.2,115.2 },
          { 525.6,93.6 },  { 496.8,158.4 }, { 532.8,136.8 }, { 518.4,180 },
          { 540,172.8 },   { 540,223.2 },   { 540,288 }
        };

        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,5.976);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#ffff00");
        DrawSetFillColor(picasso,color);
        DrawPolygon(picasso,15,points);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        const PointInfo points[7] =
        {
          { 57.6,640.8 }, { 57.6,784.8 }, { 194.4,799.2 }, { 259.2,777.6 },
          { 151.2,756 }, { 86.4,748.8 }, { 57.6,640.8 }
        };

        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,5.976);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#ffff00");
        DrawSetFillColor(picasso,color);
        DrawPolygon(picasso,7,points);
      }
      (void) PopDrawingWand(picasso);
      (void) PushDrawingWand(picasso);
      {
        const PointInfo points[193] =
        {
          { 27.86,565.3 }, { 29.66,550.8 }, { 31.97,538.1 }, { 34.85,527.1 },
          { 38.09,517.7 }, { 41.83,509.8 }, { 45.86,503.1 }, { 50.33,497.6 },
          { 55.08,493.2 }, { 60.19,489.8 }, { 65.45,487.3 }, { 70.92,485.4 },
          { 76.61,484.2 }, { 88.42,483 },   { 100.4,482.9 }, { 108.4,482.2 },
          { 119.8,480.3 }, { 150.8,474.1 }, { 189.4,466.6 }, { 210.3,463 },
          { 231.5,459.9 }, { 252.4,457.8 }, { 272.7,456.6 }, { 291.8,456.9 },
          { 300.7,457.7 }, { 309.1,458.9 }, { 316.9,460.6 }, { 324.1,462.8 },
          { 330.7,465.6 }, { 336.4,469 },   { 341.3,473 },   { 345.3,477.7 },
          { 348.4,483.1 }, { 350.4,489.2},  { 352.4,495.4 }, { 355.2,500.9 },
          { 358.8,505.8 }, { 363,510 },     { 367.8,513.6 }, { 373,516.8 },
          { 378.6,519.6 }, { 384.3,521.8 }, { 396.4,525.4 }, { 408.2,527.9 },
          { 428,531.2 },   { 434.6,532.9 }, { 436.7,533.8 }, { 437.8,534.9 },
          { 437.8,536.2 }, { 436.8,537.8 }, { 434.5,539.6 }, { 430.9,541.8 },
          { 419.3,547.6 }, { 401.3,555.2 }, { 342.4,577.9 }, {325.2,584.9 },
          { 311,591.3 },   { 300,597.3 },   { 291.6,602.8 }, { 285.8,607.8 },
          { 282.3,612.3 }, { 281.4,614.4 }, { 280.9,616.2 }, { 281.2,619.6 },
          { 282.1,621.2 }, { 283.3,622.6 }, { 286.8,624.9 }, { 291.5,626.6 },
          { 297.1,627.8 }, { 303.6,628.3 }, { 310.5,628.3 }, { 317.9,627.6 },
          { 325.2,626.3 }, { 332.6,624.3 }, { 339.5,621.7 }, { 345.9,618.4 },
          { 351.4,614.4 }, { 353.9,612.2 }, { 356,609.8 }, { 357.9,607.1 },
          { 359.4,604.3 }, { 360.6,601.3 }, { 361.4,598.2 }, { 361.7,594.9 },
          { 361.7,591.3 }, { 361.2,587.7 }, { 360.1,583.7 }, { 358.6,579.7 },
          { 356.4,575.4 }, { 353.7,570.9 }, { 350.4,566.2 }, { 346.4,561.3 },
          { 341.8,556.2 }, { 336.5,550.9 }, { 330.6,545.5 }, { 323.8,539.8 },
          { 316.2,533.9 }, { 298.7,521.5 }, { 277.8,508.2 }, { 256.1,495.5 },
          { 236,484.5 },   { 217.7,475.1 }, { 200.8,467.1 }, { 185.6,460.7 },
          { 171.9,455.5 }, { 159.6,451.6 }, { 148.6,448.8 }, { 139,447 },
          { 130.5,446.2 }, { 123.3,446.2 }, { 117.1,446.9 }, { 112,448.3 },
          { 107.9,450.2 }, { 104.8,452.5 }, { 102.5,455.2 }, { 101,458.1 },
          { 100.2,461.2 }, { 100.2,464.3 }, { 100.7,467.4 }, { 101.8,470.3 },
          { 103.4,473 },   { 105.4,475.3 }, { 107.8,477.1 }, { 110.5,478.4 },
          { 113.4,479.1 }, { 116.5,478.9 }, { 119.7,478 },   { 123,476.2 },
          { 126.4,473.3 }, { 129.6,469.2 }, { 132.7,463.9 }, { 135.2,458.4 },
          { 136.6,453.7 }, { 137,449.9 },   { 136.6,446.8 }, { 135.4,444.5 },
          { 133.3,442.9 }, { 130.8,441.9 }, { 127.5,441.4 }, { 123.9,441.6 },
          { 119.8,442.3 }, { 110.7,445.1 }, { 101.1,449.5 }, { 91.37,455.2 },
          { 82.37,461.9 }, { 74.66,469.2 }, { 71.57,473 },   { 68.98,476.8 },
          { 67.03,480.7 }, { 65.81,484.4 }, { 65.45,488.2 }, { 65.95,491.7 },
          { 67.46,495.1 }, { 69.98,498.3 }, { 73.66,501.3 }, { 78.55,503.9 },
          { 84.82,506.3 }, { 92.38,508.2 }, { 107.1,511.6 }, { 118.2,514.8 },
          { 125.9,517.8 }, { 130.7,520.4 }, { 132.1,521.7 }, { 132.8,522.9 },
          { 133,524.2 },   { 132.6,525.3 }, { 131.8,526.5 }, { 130.5,527.5 },
          { 126.6,529.6 }, { 121.5,531.7 }, { 115.3,533.7 }, { 101.4,537.6 },
          { 87.55,541.8 }, { 81.36,544 },   { 76.25,546.3 }, { 71.64,549.5 },
          { 66.89,554.1 }, { 62.14,559.8 }, { 57.38,566.1 }, { 48.17,579.6 },
          { 39.96,591.4 }, { 36.43,595.9 }, { 34.78,597.6 }, { 33.26,598.8 },
          { 31.9,599.6 },  { 30.67,599.9 }, { 29.59,599.7 }, { 28.66,598.8 },
          { 27.86,597.4 }, { 27.29,595.2 }, { 26.64,588.7 }, { 26.86,578.8 },
          { 27.86,565.3 }
        };

        DrawSetStrokeAntialias(picasso,MagickTrue);
        DrawSetStrokeWidth(picasso,5.904);
        DrawSetStrokeLineCap(picasso,RoundCap);
        DrawSetStrokeLineJoin(picasso,RoundJoin);
        (void) DrawSetStrokeDashArray(picasso,0,(const double *) NULL);
        (void) PixelSetColor(color,"#4000c2");
        DrawSetStrokeColor(picasso,color);
        DrawSetFillRule(picasso,EvenOddRule);
        (void) PixelSetColor(color,"#ffff00");
        DrawSetFillColor(picasso,color);
        DrawPolygon(picasso,193,points);
      }
      (void) PopDrawingWand(picasso);
    }
    (void) PopDrawingWand(picasso);
  }
  (void) PopDrawingWand(picasso);
  (void) MagickDrawImage(canvas,picasso);
  color=DestroyPixelWand(color);
  picasso=DestroyDrawingWand(picasso);
  return(MagickTrue);
}

int main(int argc,char **argv)
{
  char
    filename[MagickPathExtent];

  MagickBooleanType
    status;

  MagickWand
    *canvas;

  if (argc != 2)
    {
      (void) printf ("Usage: %s filename\n",argv[0]);
      exit(1);
    }
  (void) CopyMagickString(filename,argv[1],MagickPathExtent);
  /*
    Create canvas image.
  */
  MagickWandGenesis();
  canvas=NewMagickWand();
  status=MagickSetSize(canvas,596,842);
  if (status == MagickFalse)
    ThrowWandException(canvas);
  status=MagickReadImage(canvas,"xc:white");
  if (status == MagickFalse)
    ThrowWandException(canvas);
  /*
    Scribble on image.
  */
  status=ScribbleImage(canvas);
  if (status == MagickFalse)
    ThrowWandException(canvas);
  /*
    Set pixel depth to 8.
  */
  status=MagickSetImageDepth(canvas,8);
  if (status == MagickFalse)
    ThrowWandException(canvas);
  /*
    Set output as RLE compressed.
  */
  status=MagickSetImageCompression(canvas,RLECompression);
  if (status == MagickFalse)
    ThrowWandException(canvas);
  /*
    Save image to file.
  */
  status=MagickWriteImage(canvas,filename);
  if (status == MagickFalse)
    ThrowWandException(canvas);
  canvas=DestroyMagickWand(canvas);
  MagickWandTerminus();
  return(0);
}

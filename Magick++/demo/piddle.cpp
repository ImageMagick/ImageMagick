// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2002, 2003
//
// PerlMagick "piddle" demo re-implemented using Magick++ methods.
// The PerlMagick "piddle" demo is written by Cristy
//

#include <Magick++.h>
#include <string>
#include <iostream>

using namespace std;

using namespace Magick;

int main( int /*argc*/, char ** argv)
{

  // Initialize ImageMagick install location for Windows
  InitializeMagick(*argv);

  try {

    string srcdir("");
    if(getenv("SRCDIR") != 0)
      srcdir = getenv("SRCDIR");

    //
    // Create a 300x300 white canvas.
    //
    Image image( "300x300", "white" );

    // Drawing list
    std::vector<Magick::Drawable> drawList;

    // Start drawing by pushing a drawing context with specified
    // viewbox size
    drawList.push_back(DrawablePushGraphicContext());
    drawList.push_back(DrawableViewbox(0,0,image.columns(),image.rows()));

    //
    // Draw blue grid
    //
    drawList.push_back(DrawableStrokeColor("#ccf"));
    for ( int i=0; i < 300; i += 10 )
      {
        drawList.push_back(DrawableLine(i,0, i,300));
        drawList.push_back(DrawableLine(0,i, 300,i));
      }

    //
    // Draw rounded rectangle.
    //
    drawList.push_back(DrawableFillColor("blue"));
    drawList.push_back(DrawableStrokeColor("red"));
    drawList.push_back(DrawableRoundRectangle(15,15, 70,70, 10,10));

    drawList.push_back(DrawableFillColor("blue"));
    drawList.push_back(DrawableStrokeColor("maroon"));
    drawList.push_back(DrawableStrokeWidth(4));
    drawList.push_back(DrawableRoundRectangle(15,15, 70,70, 10,10));

    //
    // Draw curve.
    //
    {
      drawList.push_back(DrawableStrokeColor("black"));
      drawList.push_back(DrawableStrokeWidth(4));
      drawList.push_back(DrawableFillColor(Color()));

      std::vector<Magick::Coordinate> points;
      points.push_back(Coordinate(20,20));
      points.push_back(Coordinate(100,50));
      points.push_back(Coordinate(50,100));
      points.push_back(Coordinate(160,160));
      drawList.push_back(DrawableBezier(points));
    }

    //
    // Draw line
    //
    {
      const double dash_array[] = {4.0, 3.0, 0.0};
      drawList.push_back(DrawableStrokeDashArray(dash_array));
      drawList.push_back(DrawableStrokeColor("red"));
      drawList.push_back(DrawableStrokeWidth(1));
      drawList.push_back(DrawableLine(10,200, 54,182));
      drawList.push_back(DrawableStrokeDashArray((double *) 0));
    }

    //
    // Draw arc within a circle.
    //
    drawList.push_back(DrawableStrokeColor("black"));
    drawList.push_back(DrawableFillColor("yellow"));
    drawList.push_back(DrawableStrokeWidth(4));
    drawList.push_back(DrawableCircle(160,70, 200,70));

    drawList.push_back(DrawableStrokeColor("black"));
    drawList.push_back(DrawableFillColor("blue"));
    drawList.push_back(DrawableStrokeWidth(4));
    {
      std::vector<VPath> path;
      path.push_back(PathMovetoAbs(Coordinate(160,70)));
      path.push_back(PathLinetoVerticalRel(-40));
      path.push_back(PathArcRel(PathArcArgs(40,40, 0, 0, 0, -40,40)));
      path.push_back(PathClosePath());
      drawList.push_back(DrawablePath(path));
    }

    //
    // Draw pentogram.
    //
    {
      drawList.push_back(DrawableStrokeColor("red"));
      drawList.push_back(DrawableFillColor("LimeGreen"));
      drawList.push_back(DrawableStrokeWidth(3));

      std::vector<Magick::Coordinate> points;
      points.push_back(Coordinate(160,120));
      points.push_back(Coordinate(130,190));
      points.push_back(Coordinate(210,145));
      points.push_back(Coordinate(110,145));
      points.push_back(Coordinate(190,190));
      points.push_back(Coordinate(160,120));
      drawList.push_back(DrawablePolygon(points));
    }

    //
    // Draw rectangle.
    //
    drawList.push_back(DrawableStrokeWidth(5));
    drawList.push_back(DrawableFillColor(Color())); // No fill
    drawList.push_back(DrawableStrokeColor("yellow"));
    drawList.push_back(DrawableLine(200,260, 200,200));
    drawList.push_back(DrawableLine(200,200, 260,200));
    drawList.push_back(DrawableStrokeColor("red"));
    drawList.push_back(DrawableLine(260,200, 260,260));
    drawList.push_back(DrawableStrokeColor("green"));
    drawList.push_back(DrawableLine(200,260, 260,260));

    //
    // Draw text.
    //
    if (getenv("MAGICK_FONT") != 0)
      drawList.push_back(DrawableFont(string(getenv("MAGICK_FONT"))));
    drawList.push_back(DrawableFillColor("green"));
    drawList.push_back(DrawableStrokeColor(Color())); // unset color
    drawList.push_back(DrawablePointSize(24));
    drawList.push_back(DrawableTranslation(30,140));
    drawList.push_back(DrawableRotation(45.0));
    drawList.push_back(DrawableText(0,0,"This is a test!"));

    // Finish drawing by popping back to base context.
    drawList.push_back(DrawablePopGraphicContext());

    // Draw everything using completed drawing list
    //    image.debug(true);
    image.draw(drawList);

    //     image.write( "piddle.mvg" );

    cout << "Writing image \"piddle_out.miff\" ..." << endl;
    image.depth( 8 );
    image.compressType( RLECompression );
    image.write( "piddle_out.miff" );
    cout << "Writing MVG metafile \"piddle_out.mvg\" ..." << endl;
    image.write( "piddle_out.mvg" );

    //     cout << "Display image..." << endl;
    //     image.display( );

  }
  catch( exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  
  return 0;
}

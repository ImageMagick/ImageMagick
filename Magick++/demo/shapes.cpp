// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2002, 2003
//
// GD/PerlMagick example using Magick++ methods.
//
// Concept and algorithms lifted from PerlMagick demo script
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

    //
    // Draw texture-filled polygon
    //
    // Polygon list
    std::list<Coordinate> poly_coord;
    poly_coord.push_back( Coordinate(30,30) );
    poly_coord.push_back( Coordinate(100,10) );
    poly_coord.push_back( Coordinate(190,290) );
    poly_coord.push_back( Coordinate(30,290) );

    Image texture( srcdir + "tile.miff" );
    image.penTexture( texture );
    image.draw( DrawablePolygon( poly_coord ) );
    texture.isValid( false );
    image.penTexture( texture );  // Unset texture

    //
    // Draw filled ellipse with black border, and red fill color
    //
    image.strokeColor( "black" );
    image.fillColor( "red" );
    image.strokeWidth( 5 );
    image.draw( DrawableEllipse( 100,100, 50,75, 0,360 ) );
    image.fillColor( Color() ); // Clear out fill color

    //
    // Draw ellipse, and polygon, with black stroke, strokeWidth of 5
    //
    image.strokeColor( "black" );
    image.strokeWidth( 5 );
    list<Drawable> drawlist;

    // Add polygon to list
    poly_coord.clear();
    poly_coord.push_back( Coordinate(30,30) );
    poly_coord.push_back( Coordinate(100,10) );
    poly_coord.push_back( Coordinate(190,290) );
    poly_coord.push_back( Coordinate(30,290) );
    drawlist.push_back( DrawablePolygon( poly_coord ) );
    image.draw( drawlist );

    //
    // Floodfill object with blue
    //
    image.colorFuzz( 0.5*QuantumRange );
    image.floodFillColor( "+132+62", "blue" );

    //
    // Draw text
    //
    image.strokeColor(Color());
    image.fillColor( "red" );
    image.fontPointsize( 18 );
    image.annotate( "Hello world!", "+150+20" );

    image.fillColor( "blue" );
    image.fontPointsize( 14 );
    image.annotate( "Goodbye cruel world!", "+150+38" );

    image.fillColor( "black" );
    image.fontPointsize( 14 );
    image.annotate( "I'm climbing the wall!", "+280+120",
                    NorthWestGravity, 90.0 );
    //image.display();
    //
    // Write image.
    //

    cout << "Writing image \"shapes_out.miff\" ..." << endl;
    image.depth( 8 );
    image.compressType( RLECompression );
    image.write( "shapes_out.miff" );

    // cout << "Display image..." << endl;
    // image.display( );

  }
  catch( exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  
  return 0;
}

//
// Magick++ demo to generate a simple text button
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2003
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
    // Options
    //

    string backGround = "xc:#CCCCCC"; // A solid color

    // Color to use for decorative border
    Color border = "#D4DCF3";

    // Button size
    string buttonSize = "120x20";

    // Button background texture
    string buttonTexture = "granite:";

    // Button text
    string text = "Button Text";

    // Button text color
    string textColor = "red";

    // Font point size
    int fontPointSize = 16;

    //
    // Magick++ operations
    //

    Image button;

    // Set button size
    button.size( buttonSize );

    // Read background image
    button.read( backGround );

    // Set background to buttonTexture
    Image backgroundTexture( buttonTexture );
    button.texture( backgroundTexture );

    // Add some text
    button.fillColor( textColor );
    button.fontPointsize( fontPointSize );
    if (getenv("MAGICK_FONT") != 0)
      button.font(string(getenv("MAGICK_FONT")));
    button.annotate( text, CenterGravity );

    // Add a decorative frame
    button.borderColor( border );
    button.frame( "6x6+3+3" );

    button.depth( 8 );

    // Quantize to desired colors
    // button.quantizeTreeDepth(8);
    button.quantizeDither(false);
    button.quantizeColors(64);
    button.quantize();

    // Save to file
    cout << "Writing to \"button_out.miff\" ..." << endl;
    button.compressType( RLECompression );
    button.write("button_out.miff");

    // Display on screen
    // button.display();

  }
  catch( exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }

  return 0;
}

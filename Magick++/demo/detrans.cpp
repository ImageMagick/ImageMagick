//
// Replace transparency in an image with a solid color using Magick++
//
// Useful to see how a transparent image looks on a particular
// background color, or to create a similar looking effect without
// transparency.
//
// Copyright Bob Friesenhahn, 2000
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Usage: detrans color file...
//

#include <Magick++.h>
#include <cstdlib>
#include <iostream>
using namespace std; 
using namespace Magick;
int main(int argc,char **argv) 
{
  if ( argc < 3 )
    {
      cout << "Usage: " << argv[0] << " background_color file..." << endl;
      exit( 1 );
    }

  // Initialize ImageMagick install location for Windows
  MagickPlusPlusGenesis genesis(*argv);

  {
    Color color;
    try {
      color = Color(argv[1]);
    }
    catch ( Exception &error_ )
      {
        cout << error_.what() << endl;
        cout.flush();
        exit(1);
      }

    char **arg = &argv[2];
    while ( *arg )
      {
        string fname(*arg);
        try {
          Image overlay( fname );
          Image base( overlay.size(), color );
          base.composite( overlay, 0, 0, OverCompositeOp );
          base.alpha( false );
          base.write( fname );
        }
        catch( Exception &error_ ) 
          { 
            cout << error_.what() << endl; 
          }
        ++arg;
      }
  }

  return 0; 
}

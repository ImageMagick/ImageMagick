// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Tests for throwing exceptions
//

#include <Magick++.h>
#include <string>
#include <iostream>

using namespace std;

using namespace Magick;

int main( int /*argc*/, char ** argv)
{
  // Initialize ImageMagick install location for Windows
  MagickPlusPlusGenesis genesis(*argv);
      
  volatile int failures=0;
      
  cout << "Checking for working exceptions (may crash) ... ";
  cout.flush();

  {      
    // Basic exception test
    try
      {
        failures++;
        throw int(100);
      }
    catch ( int /*value_*/ )
      {
        failures--;
      }
      
    // Throw a Magick++ exception class.
    try
      {
        failures++;
        cout << "Throwing 'Magick::WarningResourceLimit' exception" << endl;
        cout.flush();
        throw WarningResourceLimit("How now brown cow?");
      }
    catch( Exception & /*error_*/ )
      {
        cout << "Successfully caught 'Magick::WarningResourceLimit' exception" << endl;
        cout.flush();
        failures--;
      }
      
    // A more complex test
    try
      {
        size_t columns = 640;
        size_t rows = 480;
        Geometry geometry(columns,rows);
        Color canvasColor( "red" );
        Image image( geometry, canvasColor);
          
        {
          try
            {
              failures++;
              cout << "Throwing library 'Magick::Exception' exception" << endl;
              cout.flush();
              image.directory();
            }
          catch ( Exception& /*error_*/ )
            {
              cout << "Successfully caught library 'Magick::Exception' exception" << endl;
              cout.flush();
              failures--;
            }
        }
          
      }
    catch( Exception &error_ )
      {
        cout << "Bogus catch: Caught exception: " << error_.what() << endl;
        cout.flush();
        return 1;
      }
    catch( exception &error_ )
      {
        cout << "Bogus catch: Caught exception: " << error_.what() << endl;
        cout.flush();
        return 1;
      }
  
    if ( failures )
      {
        cout << failures << " failures" << endl;
        cout.flush();
        return 1;
      }
    cout << "Exception testing passed!" << endl;
  }

  return 0;
}

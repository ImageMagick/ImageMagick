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
  InitializeMagick(*argv);
      
  int failures=0;
      
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
        throw WarningResourceLimit("How now brown cow?");
      }
    catch( Exception & /*error_*/ )
      {
        failures--;
      }
      
    // A more complex test
    try
      {
        unsigned int columns = 640;
        unsigned int rows = 480;
        Geometry geometry(columns,rows);
        Color canvasColor( "red" );
        Image image( geometry, canvasColor);
          
        {
          try
            {
              failures++;
              image.directory();
            }
          catch ( Exception& /*error_*/ )
            {
              failures--;
            }
        }
          
      }
    catch( Exception &error_ )
      {
        cout << "Caught exception: " << error_.what() << endl;
        return 1;
      }
    catch( exception &error_ )
      {
        cout << "Caught exception: " << error_.what() << endl;
        return 1;
      }
  
    if ( failures )
      {
        cout << failures << " failures" << endl;
        return 1;
      }
    cout << "passed!" << endl;
  }

  return 0;
}

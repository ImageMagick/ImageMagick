// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra 2015
//
// Test Magick::Geometry class
//

#include <Magick++.h>
#include <string>
#include <iostream>

using namespace std;

using namespace Magick;

int main(int, char **argv)
{

  // Initialize ImageMagick install location for Windows
  MagickPlusPlusGenesis genesis(*argv);

  int failures=0;

  try
  {

    //
    // Verify conversion from and to string
    //

    string input="100x50+10-5!";
    Geometry geometry(input);

    if ((geometry.width() != 100) || (geometry.height() != 50) ||
        (geometry.xOff() != 10) || (geometry.yOff() != -5) ||
        (geometry.aspect() == false))
      {
        ++failures;
        cout << "Line: " << __LINE__
        << " Conversion from " << input << " failed"
        << endl;
      }

    string output=geometry;
    if (output != input)
      {
        ++failures;
        cout << "Line: " << __LINE__
        << " Output " << output << " is not the same as " << input
        << endl;
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

  return 0;
}

// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2003
//
// Test STL averageImages function
//

#include <Magick++.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>

using namespace std;

using namespace Magick;

int main( int /*argc*/, char ** argv)
{

  // Initialize ImageMagick install location for Windows
  InitializeMagick(*argv);

  int failures=0;

  try {

    string srcdir("");
    if(getenv("SRCDIR") != 0)
      srcdir = getenv("SRCDIR");

    //
    // Test averageImages
    //
    
    list<Image> imageList;
    readImages( &imageList, srcdir + "test_image_anim.miff" );
    
    Image averaged;
    averageImages( &averaged, imageList.begin(), imageList.end() );
    // averaged.display();
    if ( 0 && averaged.signature() != "d4b4ffb8b70c4e9b0e50445542deb26fbcdf8c393c793123cbc92fb35341e44d" &&
         averaged.signature() != "62d46d6d239b9fbd3b8ff2271aed1b5dde6303e0d5228dd8d833f61a7b012a79" &&
         averaged.signature() != "fdc76a2689d19061e1f7f6adfd79a2c04bc4608125a2cd2a1bce0d981774e13f" &&
         averaged.signature() != "66dfb88c21405a6bf582c9a542d87fd14db176aae1f34bc30b0b3e2443b49aa8" &&
         averaged.signature() != "f3bc318abc0b842c656b6545d1d7159eedb61f559a95fc5df671db7d0c0639de")
      {
	cout << "Line: " << __LINE__
	     << "  Averaging image failed, signature = "
	     << averaged.signature() << endl;
	averaged.display();
	++failures;
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


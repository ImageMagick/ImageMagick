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
    if (
        ( averaged.signature() != "16fac0dc82f3901de623ce9ff991c368f6752acf6cb0c11170d78412c3729730") &&
        ( averaged.signature() != "6574796b7b07d7400a7a310052eabf2b58f81952d1854a76ac9a23890ac2073b") &&
        ( averaged.signature() != "ad4861b99339d84bed685eb42bbabe657abb60d48b8fc7ddf680af866dd45ad4") &&
        ( averaged.signature() != "8e6e1a9b5f1eec5539b1f44347249f227f3e07f9acb07d80404ca6a19f88db7c") &&
        ( averaged.signature() != "a88e978776d45b73bc8c9f37f6726cc9f14a3118b9a82384ee5acf488c5c2863") &&
        ( averaged.signature() != "6bda37a8b6734ac271595f5b583d801cfb2479637401d056eae9be97127f558f") &&
        ( averaged.signature() != "919a9e18a5e5ded83c2c4e5cfcd21d654802fcc14b06b02898d96fe28f04a1a1")
       )
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


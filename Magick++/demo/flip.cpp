// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2003
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Demonstration of unary function-object based operations
//
// Reads the multi-frame file "smile_anim.miff" and writes a
// flipped and morphed version to "flip_out.miff".
//

#include <Magick++.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <list>
#include <algorithm>

using namespace std;

using namespace Magick;

int main( int /*argc*/, char ** argv)
{

  // Initialize ImageMagick install location for Windows
  MagickPlusPlusGenesis genesis(*argv);


  try {

    string srcdir("");
    if(getenv("SRCDIR") != 0)
      srcdir = getenv("SRCDIR");

    // Read images into STL list
    list<Image> imageList;
    readImages( &imageList, srcdir + "smile_anim.miff" );

    // cout << "Total scenes: " << imageList.size() << endl;

    // Flip images
    for_each( imageList.begin(), imageList.end(), flipImage() );

    // Create a morphed version, adding three frames between each
    // existing frame.
    list<Image> morphed;
    morphImages( &morphed, imageList.begin(), imageList.end(), 3 );

    // Write out images
    cout << "Writing image \"flip_out.miff\" ..." << endl;
    writeImages( morphed.begin(), morphed.end(), "flip_out.miff" );

  }
  catch( exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  
  return 0;
}

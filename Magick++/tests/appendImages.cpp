// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2003
//
// Test STL appendImages function
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
  MagickPlusPlusGenesis genesis(*argv);

  int failures=0;

  try {

    string srcdir("");
    if(getenv("SRCDIR") != 0)
      srcdir = getenv("SRCDIR");

    //
    // Test appendImages
    //

    list<Image> imageList;
    readImages( &imageList, srcdir + "test_image_anim.miff" );

    Image appended;

    // Horizontal
    appendImages( &appended, imageList.begin(), imageList.end() );
    // appended.display();
    if (
        ( appended.signature() != "f5ed4a96632126a30c353340a1ddc4e0745295bb1f4bbbb6e020138c972c2f5e" ) &&
        ( appended.signature() != "aa8789792be68dde5d686ddcbce4f551cbe8093cf3c782f5313443594abff8c0" ) &&
        ( appended.signature() != "d2b63ade27f08ba413533c56239fd5dca7ac5cdfcae7a15d48980209dbfc0a40" ) &&
        ( appended.signature() != "f48dd74b57ed277c9c62da1a65788186a910b8f2faa47851fcf1f4572640ed9c" ))
      {
	++failures;
	cout << "Line: " << __LINE__
	     << "  Horizontal append failed, signature = "
	     << appended.signature() << endl;
	appended.write("appendImages_horizontal_out.miff");
	// appended.display();
      }

    // Vertical
    appendImages( &appended, imageList.begin(), imageList.end(), true );
    if (
        ( appended.signature() != "de891eb85d168bd2177ee92940ab0e29d32c9f8e4be41906f9272a88925d9dd3" ) &&
        ( appended.signature() != "5e119331c70db1b0bc3fdf51920b85449b4b02f63653250c34b68c1528171bb2" ) &&
        ( appended.signature() != "bb411d8cc99700f29547e8ca60d925d0d3be3aaf16e70260a3506428e61339de" ) &&
        ( appended.signature() != "9cfe22dacae97e4e0fe1c12567a5d7e111f4680ec65a40da16281928cf4ba6be" ))
      {
	++failures;
	cout << "Line: " << __LINE__
	     << "  Vertical append failed, signature = "
	     << appended.signature() << endl;
	appended.write("appendImages_vertical_out.miff");
	// appended.display();
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


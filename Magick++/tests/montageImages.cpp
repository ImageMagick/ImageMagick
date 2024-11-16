// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2002, 2003
//
// Test STL montageImages function
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

  const char *const p = getenv("MAGICK_FONT");
  const string MAGICK_FONT(p ? p : "");
  
  int failures=0;

  try {

    string srcdir("");
    if(getenv("SRCDIR") != 0)
      srcdir = getenv("SRCDIR");

    //
    // Test montageImages
    //

    list<Image> imageList;
    readImages( &imageList, srcdir + "test_image_anim.miff" );

    vector<Image> montage;
    MontageFramed montageOpts;
    montageOpts.font(MAGICK_FONT);

    // Default montage
    montageImages( &montage, imageList.begin(), imageList.end(), montageOpts );

    {
      Geometry targetGeometry(128, 126 );
      if ( montage[0].montageGeometry() != targetGeometry )
        {
          ++failures;
          cout << "Line: " << __LINE__ 
               << "  Montage geometry ("
               << string(montage[0].montageGeometry())
               << ") is incorrect (expected "
               << string(targetGeometry)
               << ")"
               << endl;
        }
    }

    if ( montage[0].columns() != 768 || montage[0].rows() != 504 )
      {
	++failures;
	cout << "Line: " << __LINE__ 
	     << "  Montage columns/rows ("
	     << montage[0].columns() << "x"
	     << montage[0].rows()
	     << ") incorrect. (expected 768x504)" << endl;
      }

    // Montage with options set
    montage.clear();
    montageOpts.borderColor( "green" );
    montageOpts.borderWidth( 1 );
    montageOpts.fileName( "Montage" );
    montageOpts.frameGeometry( "6x6+3+3" );
    montageOpts.geometry("50x50+2+2>");
    montageOpts.gravity( CenterGravity );
    montageOpts.strokeColor( "yellow" );
    montageOpts.shadow( true );
    montageOpts.texture( "granite:" );
    montageOpts.tile("2x1");
    montageImages( &montage, imageList.begin(), imageList.end(), montageOpts );

    if ( montage.size() != 3 )
      {
	++failures;
	cout << "Line: " << __LINE__ 
	     << "  Montage images failed, number of montage frames is "
	     << montage.size()
	     << " rather than 3 as expected." << endl;
      }

    {
      Geometry targetGeometry( 66, 70 );
      if ( montage[0].montageGeometry() != targetGeometry )
        {
          ++failures;
          cout << "Line: " << __LINE__ 
               << "  Montage geometry ("
               << string(montage[0].montageGeometry())
               << ") is incorrect (expected "
               << string(targetGeometry)
               << ")."
               << endl;
        }
    }

    if ( montage[0].columns() != 136 || montage[0].rows() != 70 )
      {
	++failures;
	cout << "Line: " << __LINE__ 
	     << "  Montage columns/rows ("
	     << montage[0].columns() << "x"
	     << montage[0].rows()
	     << ") incorrect. (expected 136x70)" << endl;
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


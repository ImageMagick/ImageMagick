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
  InitializeMagick(*argv);

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
    if (( appended.signature() != "3a90bb0bb8f69f6788ab99e9e25598a0d6c5cdbbb797f77ad68011e0a8b1689d" ) &&
        ( appended.signature() != "c15fcd1e739b73638dc4e36837bdb53f7087359544664caf7b1763928129f3c7" ) &&
        ( appended.signature() != "229ff72f812e5f536245dc3b4502a0bc2ab2363f67c545863a85ab91ebfbfb83" ) &&
        ( appended.signature() != "b98c42c55fc4e661cb3684154256809c03c0c6b53da2738b6ce8066e1b6ddef0" ))
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
    if (( appended.signature() != "d73d25ccd6011936d08b6d0d89183b7a61790544c2195269aff4db2f782ffc08" ) &&
        ( appended.signature() != "0909f7ffa7c6ea410fb2ebfdbcb19d61b19c4bd271851ce3bd51662519dc2b58" ) &&
        ( appended.signature() != "11b97ba6ac1664aa1c2faed4c86195472ae9cce2ed75402d975bb4ffcf1de751" ) &&
        ( appended.signature() != "cae4815eeb3cb689e73b94d897a9957d3414d1d4f513e8b5e52579b05d164bfe" ))
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


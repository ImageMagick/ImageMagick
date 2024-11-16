// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2003
// Copyright Dirk Lemstra 2014
//
// Test STL readImages and writeImages functions and test
// image format when reading/writing.
//

#include <Magick++.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>

using namespace std;

using namespace Magick;

int main(int,char ** argv)
{
  int
    failures=0;

  string
    srcdir("");


  // Initialize ImageMagick install location for Windows
  MagickPlusPlusGenesis genesis(*argv);

  try
  {
    if (getenv("SRCDIR") != 0)
      srcdir=getenv("SRCDIR");

    //
    // Test readImages and writeImages
    //
    list<Image> first;
    readImages(&first,srcdir + "test_image_anim.miff");

    if (first.size() != 6)
      {
        ++failures;
        cout << "Line: " << __LINE__
          << "  Read images failed, number of frames is "
          << first.size()
          << " rather than 6 as expected." << endl;
      }

    writeImages(first.begin(),first.end(),"testmagick_anim_out.miff");

    list<Image> second;
    readImages(&second,"testmagick_anim_out.miff");

    list<Image>::iterator firstIter = first.begin();
    list<Image>::iterator secondIter = second.begin();
    while (firstIter != first.end() && secondIter != second.end())
    {
      if (firstIter->scene() != secondIter->scene())
        {
          ++failures;
          cout << "Line: " << __LINE__
            << "  Image scene: " << secondIter->scene()
            << " is not equal to original "
            << firstIter->scene()
            << endl;
        }

      if (firstIter->rows() != secondIter->rows())
        {
          ++failures;
          cout << "Line: " << __LINE__
            << "  Image rows " << secondIter->rows()
            << " are not equal to original "
            << firstIter->rows()
            << endl;
        }

      if (firstIter->columns() != secondIter->columns())
        {
          ++failures;
          cout << "Line: " << __LINE__
            << "  Image columns " << secondIter->columns()
            << " are not equal to original "
            << firstIter->rows()
            << endl;
        }

      firstIter++;
      secondIter++;
    }

    Image third(*first.begin());
    third.write("testmagick_anim_out");

    Image fourth;
    fourth.read("testmagick_anim_out");

    if (fourth.magick() != "MIFF")
      {
        ++failures;
        cout << "Line: " << __LINE__
          << "  Image magick: " << fourth.magick()
          << " is not equal to MIFF"
          << endl;
      }

    third.write("testmagick_anim_out.ico");
    fourth.read("testmagick_anim_out.ico");

    if (fourth.magick() != "ICO")
      {
        ++failures;
        cout << "Line: " << __LINE__
          << "  Image magick: " << fourth.magick()
          << " is not equal to ICO"
          << endl;
      }

    third.magick("BMP");
    third.write("testmagick_anim_out.ico");
    fourth.read("testmagick_anim_out.ico");

    if (fourth.magick() != "BMP")
      {
        ++failures;
        cout << "Line: " << __LINE__
          << "  Image magick: " << fourth.magick()
          << " is not equal to BMP"
          << endl;
      }

    third.write("PDB:testmagick_anim_out.ico");
    fourth.read("testmagick_anim_out.ico");

    if (fourth.magick() != "PDB")
      {
        ++failures;
        cout << "Line: " << __LINE__
          << "  Image magick: " << fourth.magick()
          << " is not equal to PDB"
          << endl;
      }

    third.magick("");
    third.write("testmagick_anim_out.ico");
    fourth.read("testmagick_anim_out.ico");

    if (fourth.magick() != "ICO")
      {
        ++failures;
        cout << "Line: " << __LINE__
          << "  Image magick: " << fourth.magick()
          << " is not equal to ICO"
          << endl;
      }
  }
  catch(Exception &error_)
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  catch(exception &error_)
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }

  if (failures)
    {
      cout << failures << " failures" << endl;
      return 1;
    }

  return 0;
}


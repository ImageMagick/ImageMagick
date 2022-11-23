// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002, 2003
//
// Test Magick::CoderInfo class and Magick::coderInfoList
//

#include <Magick++.h>
#include <string>
#include <iostream>
#include <list>

using namespace std;

using namespace Magick;

int test( CoderInfo::MatchType isReadable_,
          CoderInfo::MatchType isWritable_,
          CoderInfo::MatchType isMultiFrame_ )
{
  int result = 0;
  list<CoderInfo> coderList;
  coderInfoList( &coderList, isReadable_, isWritable_, isMultiFrame_ );
  list<CoderInfo>::iterator entry = coderList.begin();
  while( entry != coderList.end() )
    {
      // Readable
      if ( isReadable_ != CoderInfo::AnyMatch &&
           (( entry->isReadable() && isReadable_ != CoderInfo::TrueMatch ) ||
            ( !entry->isReadable() && isReadable_ != CoderInfo::FalseMatch )) )
        {
          cout << "Entry \""
               << entry->name()
               << "\" has unexpected readability state ("
               << static_cast<int>(entry->isReadable())
               << ")"
               << endl;
          ++result;
        }

      // Writable
      if ( isWritable_ != CoderInfo::AnyMatch &&
           (( entry->isWritable() && isWritable_ != CoderInfo::TrueMatch ) ||
            ( !entry->isWritable() && isWritable_ != CoderInfo::FalseMatch )) )
        {
          cout << "Entry \""
               << entry->name()
               << "\" has unexpected writability state ("
               << static_cast<int>(entry->isWritable())
               << ")"
               << endl;
          ++result;
        }

      // MultiFrame
      if ( isMultiFrame_ != CoderInfo::AnyMatch &&
           (( entry->isMultiFrame() && isMultiFrame_ != CoderInfo::TrueMatch ) ||
            ( !entry->isMultiFrame() && isMultiFrame_ != CoderInfo::FalseMatch )) )
        {
          cout << "Entry \""
               << entry->name()
               << "\" has unexpected multiframe state ("
               << static_cast<int>(entry->isMultiFrame())
               << ")"
               << endl;
          ++result;
        }

      entry++;
    }

  return result;
}

int main( int /*argc*/, char **argv)
{

  // Initialize ImageMagick install location for Windows
  InitializeMagick(*argv);

  int failures=0;

  try {

    CoderInfo coderInfo("GIF");
    if ( coderInfo.name() != string("GIF") )
      {
        cout << "Unexpected coder name \""
             << coderInfo.name()
             << "\""
             << endl;
        ++failures;
      }

    if( coderInfo.description() != string("CompuServe graphics interchange format") )
      {
        cout << "Unexpected coder description \""
             << coderInfo.description()
             << "\""
             << endl;
        ++failures;
      }

    failures += test(CoderInfo::AnyMatch,CoderInfo::AnyMatch,CoderInfo::AnyMatch);
    failures += test(CoderInfo::FalseMatch,CoderInfo::FalseMatch,CoderInfo::FalseMatch);

    failures += test(CoderInfo::TrueMatch,CoderInfo::AnyMatch,CoderInfo::AnyMatch);
    failures += test(CoderInfo::FalseMatch,CoderInfo::AnyMatch,CoderInfo::AnyMatch);

    failures += test(CoderInfo::AnyMatch,CoderInfo::TrueMatch,CoderInfo::AnyMatch);
    failures += test(CoderInfo::AnyMatch,CoderInfo::FalseMatch,CoderInfo::AnyMatch);

    failures += test(CoderInfo::AnyMatch,CoderInfo::AnyMatch,CoderInfo::TrueMatch);
    failures += test(CoderInfo::AnyMatch,CoderInfo::AnyMatch,CoderInfo::FalseMatch);
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

#define WINVER 0x0501
#define BUFSIZE 4096
#pragma comment(lib, "Shlwapi.lib")

#include <string>
#include <iostream>
#include <fstream>
#include <Shlwapi.h>
#include <stdint.h>
#include "encoder_format.h"
#include <Magick++/Functions.h>
using namespace std;

extern EncoderFormat encoderFormat;
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);

class FuzzingDebugger
{
public:
  bool load(wstring fileName)
  {
    ifstream
      file;

    streampos
      size;

    file = ifstream(fileName, ios::in | ios::binary | ios::ate);
    if (!file.is_open())
      return(false);

    size = file.tellg();
    _size = size;
    _data = new char[_size];
    file.seekg(0, ios::beg);
    file.read(_data, size);
    file.close();

    encoderFormat.set(fileName, wstring(PathFindExtension(fileName.c_str())));

    return(true);
  }

  void start()
  {
    const uint8_t
      *data;

    data = reinterpret_cast<const uint8_t *>(_data);
    LLVMFuzzerTestOneInput(data, _size);

    delete _data;
  }


private:
  char * _data;
  size_t _size;
};

int wmain(int argc, wchar_t *argv[])
{
  FuzzingDebugger
    debugger;

  int
    debug;

  wstring
    fileName;

  if (argc == 1)
  {
    wchar_t
      fullPath[BUFSIZE],
      **lppPart;

    lppPart = NULL;
    GetFullPathName(argv[0], BUFSIZE, fullPath, lppPart);
    PathRemoveExtension(fullPath);
    fileName = wstring(fullPath) + L".input";
  }
  else
    fileName = wstring(argv[1]);

  debug=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  debug |= _CRTDBG_DELAY_FREE_MEM_DF;
  debug |= _CRTDBG_LEAK_CHECK_DF;
  (void) _CrtSetDbgFlag(debug);

  //_CrtSetBreakAlloc(42);

  {
    if (!debugger.load(fileName))
    {
      wcerr << L"Unable to load " << fileName;
      cin.get();
    }
    else
      debugger.start();
  }

  Magick::TerminateMagick();

  _CrtCheckMemory();
}

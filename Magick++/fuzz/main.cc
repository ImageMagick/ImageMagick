/*
  Copyright @ 2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <windows.h>
#include <fileapi.h>
#include <fstream>
#include <stdint.h>

#include <Magick++/Functions.h>

#include "encoder_format.h"

extern EncoderFormat encoderFormat;
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data,size_t Size);

class FuzzingDebugger
{
public:
  bool load(const wstring fileName)
  {
    ifstream
      file;

    streampos
      size;

    file=ifstream(fileName,ios::in|ios::binary|ios::ate);
    if (!file.is_open())
      return(false);
    size=file.tellg();
    if (size < 1)
      return(false);
    _size=size;
    _data=new char[_size];
    file.seekg(0,ios::beg);
    file.read(_data,size);
    file.close();
    encoderFormat.set(fileName);
    return(true);
  }

  void start()
  {
    const uint8_t
      *data;

    data=reinterpret_cast<const uint8_t *>(_data);
    LLVMFuzzerTestOneInput(data,_size);
    delete _data;
  }


private:
  char *_data;
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
      wcerr << L"Filename must be specified as the first argument";
      return(1);
    }
  fileName=wstring(argv[1]);
  if (!debugger.load(fileName))
    {
      wcerr << L"Unable to load " << fileName;
      cin.get();
      return(1);
    }
  debug=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  debug |= _CRTDBG_DELAY_FREE_MEM_DF;
  debug |= _CRTDBG_LEAK_CHECK_DF;
  (void) _CrtSetDbgFlag(debug);
  //_CrtSetBreakAlloc(42);
  debugger.start();
  Magick::TerminateMagick();
  _CrtCheckMemory();
  return(0);
}

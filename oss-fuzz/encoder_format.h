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

#include <string.h>
#include <iostream>
using namespace std;

extern class EncoderFormat
{
public:
  const string get()
  {
    return(string(_format.begin(),_format.end()));
  }

  void set(const wstring fileName)
  {
    wstring
      format;

    size_t
      index;

    if (fileName.find(L"clusterfuzz-testcase-") == -1)
      return;

    format=fileName;
    index=format.find(L"_", 0);
    if (index == wstring::npos)
      return;

    format=format.substr(index+1);
    index=format.find(L"_",0);
    if (index != wstring::npos)
      _format=format.substr(0, index);
  }
private:
  wstring _format=L".notset";
};

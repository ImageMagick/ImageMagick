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

class EncoderFormat
{
public:
  std::string get() { return std::string(_format.begin(), _format.end()); } const
  void set(const std::wstring fileName, const std::wstring extension)
  {
    if (fileName.find(L"clusterfuzz-testcase-") == -1)
    {
      if (extension.length() > 1)
        _format = extension.substr(1, extension.size() - 1);
      return;
    }

    std::wstring format=fileName;

    size_t index = format.find(L"_", 0);
    if (index == std::wstring::npos)
      return;

    format=format.substr(index + 1);
    index = format.find(L"_", 0);
    if (index != std::wstring::npos)
      _format=format.substr(0, index);
    else if (extension.length() > 1)
      _format=extension.substr(1, extension.size() - 1);
  }
private:
  std::wstring _format = L".notset";
};

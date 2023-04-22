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

static int fuzzEncoderWithStringFileName(const std::string encoder,
  const uint8_t *data,size_t size,bool (*validate)(const std::string &) = NULL)
{
  std::string
    fileName;

  if (IsInvalidSize(size))
    return(0);
  fileName=std::string(reinterpret_cast<const char*>(data), size);
  // Can be used to deny specific file names
  if ((validate != NULL) && (validate(fileName) == false))
    return(0);
  try
  {
    Magick::Image
      image;

    image.read(encoder + ":" + fileName);
  }
  catch (Magick::Exception &e)
  {
  }
  return(0);
}

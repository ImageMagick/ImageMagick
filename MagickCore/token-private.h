/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/license/

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private token methods.
*/
#ifndef MAGICKCORE_TOKEN_PRIVATE_H
#define MAGICKCORE_TOKEN_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#ifndef EILSEQ
  #define EILSEQ  ENOENT
#endif

#define MaxMultibyteCodes  6

extern MagickPrivate MagickBooleanType
  IsGlob(const char *) magick_attribute((__pure__));

typedef struct
{
  int
    code_mask,
    code_value,
    utf_mask,
    utf_value;
} UTFInfo;

static UTFInfo
  utf_info[MaxMultibyteCodes] =
  {
    { 0x80, 0x00, 0x000007f, 0x0000000 },  /* 1 byte sequence */
    { 0xE0, 0xC0, 0x00007ff, 0x0000080 },  /* 2 byte sequence */
    { 0xF0, 0xE0, 0x000ffff, 0x0000800 },  /* 3 byte sequence */
    { 0xF8, 0xF0, 0x01fffff, 0x0010000 },  /* 4 byte sequence */
    { 0xFC, 0xF8, 0x03fffff, 0x0200000 },  /* 5 byte sequence */
    { 0xFE, 0xFC, 0x7ffffff, 0x4000000 },  /* 6 byte sequence */
  };

static inline unsigned char *ConvertLatin1ToUTF8(
  const unsigned char *magick_restrict content)
{
  int
    c;

  const unsigned char
    *magick_restrict p;

  size_t
    length;

  unsigned char
    *magick_restrict q,
    *utf8;

  length=0;
  for (p=content; *p != '\0'; p++)
    length+=(*p & 0x80) != 0 ? 2 : 1;
  utf8=(unsigned char *) NULL;
  if (~length >= 1)
    utf8=(unsigned char *) AcquireQuantumMemory(length+1UL,sizeof(*utf8));
  if (utf8 == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  q=utf8;
  for (p=content; *p != '\0'; p++)
  {
    c=(*p);
    if ((c & 0x80) == 0)
      *q++=(unsigned char) c;
    else
      {
        *q++=(unsigned char) (0xc0 | ((c >> 6) & 0x3f));
        *q++=(unsigned char) (0x80 | (c & 0x3f));
      }
  }
  *q='\0';
  return(utf8);
}

static inline unsigned char *ConvertMacRomanToUTF8(
  const unsigned char *magick_restrict content)
{
  static const uint16_t macroman_unicode[128] = {
    0x00C4,0x00C5,0x00C7,0x00C9,0x00D1,0x00D6,0x00DC,0x00E1,
    0x00E0,0x00E2,0x00E4,0x00E3,0x00E5,0x00E7,0x00E9,0x00E8,
    0x00EA,0x00EB,0x00ED,0x00EC,0x00EE,0x00EF,0x00F1,0x00F3,
    0x00F2,0x00F4,0x00F6,0x00F5,0x00FA,0x00F9,0x00FB,0x00FC,
    0x2020,0x00B0,0x00A2,0x00A3,0x00A7,0x2022,0x00B6,0x00DF,
    0x00AE,0x00A9,0x2122,0x00B4,0x00A8,0x2260,0x00C6,0x00D8,
    0x221E,0x00B1,0x2264,0x2265,0x00A5,0x00B5,0x2202,0x2211,
    0x220F,0x03C0,0x222B,0x00AA,0x00BA,0x03A9,0x00E6,0x00F8,
    0x00BF,0x00A1,0x00AC,0x221A,0x0192,0x2248,0x2206,0x00AB,
    0x00BB,0x2026,0x00A0,0x00C0,0x00C3,0x00D5,0x0152,0x0153,
    0x2013,0x2014,0x201C,0x201D,0x2018,0x2019,0x00F7,0x25CA,
    0x00FF,0x0178,0x2044,0x20AC,0x2039,0x203A,0xFB01,0xFB02,
    0x2021,0x00B7,0x201A,0x201E,0x2030,0x00C2,0x00CA,0x00C1,
    0x00CB,0x00C8,0x00CD,0x00CE,0x00CF,0x00CC,0x00D3,0x00D4,
    0xF8FF,0x00D2,0x00DA,0x00DB,0x00D9,0x0131,0x02C6,0x02DC,
    0x00AF,0x02D8,0x02D9,0x02DA,0x00B8,0x02DD,0x02DB,0x02C7
  };

  int
    c;

  const unsigned char
    *magick_restrict p;

  size_t
    length;

  unsigned char
    *magick_restrict q,
    *utf8;

  length=0;
  for (p=content; *p != '\0'; p++)
    length+=(*p & 0x80) != 0 ? 4 : 1;
  utf8=(unsigned char *) NULL;
  if (~length >= 1)
    utf8=(unsigned char *) AcquireQuantumMemory(length+1UL,sizeof(*utf8));
  if (utf8 == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  q=utf8;
  for (p=content; *p != '\0'; p++)
  {
    c=(*p);
    if (c > 128)
      c=macroman_unicode[c-128];
    if ((c & 0x80) == 0)
      *q++=(unsigned char) c;
    else
      if (c <= 0x7ff)
        {
          *q++=(unsigned char) (0xc0 | ((c >> 6) & 0x3f));
          *q++=(unsigned char) (0x80 | (c & 0x3f));
        }
      else
        if (c <= 0xffff)
          {
            *q++=(unsigned char) (0xe0 | (c >> 12));
            *q++=(unsigned char) (0x80 | ((c >> 6) & 0x3f));
            *q++=(unsigned char) (0x80 | (c & 0x3f));
          }
        else
          {
            *q++=(unsigned char) (0xf0 | (c >> 18));
            *q++=(unsigned char) (0x80 | ((c >> 12) & 0x3f));
            *q++=(unsigned char) (0x80 | ((c >> 6) & 0x3f));
            *q++=(unsigned char) (0x80 | (c & 0x3f));
          }
  }
  *q='\0';
  return(utf8);
}

static inline int GetNextUTFCode(const char *magick_restrict text,
  unsigned int *magick_restrict octets)
{
  int
    code;

  ssize_t
    i;

  int
    c,
    unicode;

  *octets=1;
  if (text == (const char *) NULL)
    {
      errno=EINVAL;
      return(-1);
    }
  code=(int) (*text++) & 0xff;
  unicode=code;
  for (i=0; i < MaxMultibyteCodes; i++)
  {
    if ((code & utf_info[i].code_mask) == utf_info[i].code_value)
      {
        unicode&=utf_info[i].utf_mask;
        if (unicode < utf_info[i].utf_value)
          break;
        *octets=(unsigned int) (i+1);
        return(unicode);
      }
    c=(int) (*text++ ^ 0x80) & 0xff;
    if ((c & 0xc0) != 0)
      break;
    if (unicode > 0x10FFFF)
      break;
    unicode=(unicode << 6) | c;
  }
  errno=EILSEQ;
  return(-1);
}

static inline int GetUTFCode(const char *magick_restrict text)
{
  unsigned int
    octets;

  return(GetNextUTFCode(text,&octets));
}

static inline unsigned int GetUTFOctets(const char *magick_restrict text)
{
  unsigned int
    octets;

  (void) GetNextUTFCode(text,&octets);
  return(octets);
}

static inline MagickBooleanType IsNonBreakingUTFSpace(const int code)
{
  if (code == 0x00a0)
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsUTFSpace(const int code)
{
  if (((code >= 0x0009) && (code <= 0x000d)) || (code == 0x0020) ||
      (code == 0x0085) || (code == 0x00a0) || (code == 0x1680) ||
      (code == 0x180e) || ((code >= 0x2000) && (code <= 0x200a)) ||
      (code == 0x2028) || (code == 0x2029) || (code == 0x202f) ||
      (code == 0x205f) || (code == 0x3000))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsUTFValid(int code)
{
  int
    mask;

  mask=(int) 0x7fffffff;
  if (((code & ~mask) != 0) && ((code < 0xd800) || (code > 0xdfff)) &&
      (code != 0xfffe) && (code != 0xffff))
    return(MagickFalse);
  return(MagickTrue);
}

static inline MagickBooleanType IsUTFAscii(int code)
{
  int
    mask;

  mask=(int) 0x7f;
  if ((code & ~mask) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

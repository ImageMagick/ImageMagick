#include <Magick++.h>
#include <algorithm>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

vector<string> split(const string &input) {
  regex re("(\\w+)");
  sregex_token_iterator first{input.begin(), input.end(), re}, last;
  return {first, last};
}

ostream &operator<<(ostream &out, const vector<string> &list) {
  for (auto const &i : list)
    cout << i << " ";
  return out;
}

int main(int argc, char *argv[]) {
  vector<string> expected(argv + 1, argv + argc);
  sort(expected.begin(), expected.end());

  cout << "EXPECTED: " << expected << endl;

  Magick::InitializeMagick(".");

  // Ensure that linking the C++ library works
  Magick::Image im;

  vector<string> actual = split(MagickCore::GetMagickDelegates());
#ifdef MAGICKCORE_HASJEMALLOC
  actual.push_back("jemalloc");
#else
  actual.push_back("std_malloc");
#endif
  sort(actual.begin(), actual.end());
  cout << "ACTUAL: " << actual << endl;

  auto i = expected.begin();
  for (auto i = expected.begin(); i != expected.end(); i++) {
    if ((*i).at(0) == '-') {
      if (find(actual.begin(), actual.end(), (*i).substr(1)) != actual.end()) {
        cerr << (*i).substr(1) << " is present" << endl;
        return 1;
      }
    } else {
      if (find(actual.begin(), actual.end(), *i) == actual.end()) {
        cerr << *i << " is missing" << endl;
        return 1;
      }
    }
  }

  // Ensure the color information has been loaded
  Magick::Color("blue");

  return 0;
}

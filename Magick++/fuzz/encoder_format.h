class EncoderFormat {
public:
  std::string get() { return std::string(_format.begin(), _format.end()); } const
  void set(const std::wstring fileName)
  {
    std::wstring format=fileName;

    size_t index = format.find(L"_", 0);
    if (index == std::wstring::npos)
      return;

    format=format.substr(index + 1);
    index = format.find(L"_", 0);
    if (index != std::wstring::npos)
      _format=format.substr(0, index);
  }
private:
  std::wstring _format = L".notset";
};
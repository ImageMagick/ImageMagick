class EncoderFormat {
public:
  std::string get() { return std::string(_format.begin(), _format.end()); } const
  void set(const std::wstring format)
  {
    if (format.length() > 1)
      _format = format.substr(1, format.size() - 1);
  }
private:
  std::wstring _format = L".notset";
};
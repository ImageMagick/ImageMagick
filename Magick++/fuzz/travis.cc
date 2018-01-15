#if BUILD_TRAVIS
int main()
{
  return LLVMFuzzerTestOneInput(0, 0);
}
#endif // BUILD_TRAVIS

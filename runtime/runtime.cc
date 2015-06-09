/*
 * This provides functions expected by either GCC or the AEABI.
 */

extern "C" {
  void __cxa_pure_virtual();
  void *memset(void *, int, unsigned);
  int __aeabi_atexit(void *, void (*)(void *), void *);

  void *__dso_handle;
}

void __cxa_pure_virtual() {
  while (1);
}

void operator delete(void *) {
  while (1);
}

int __aeabi_atexit(void *, void (*)(void *), void *) {
  return 1;
}

void *memset(void *s, int c, unsigned n) {
  unsigned char *dst = static_cast<unsigned char *>(s);
  for (unsigned i = 0; i < n; ++i) {
    dst[i] = c;
  }
  return dst;
}

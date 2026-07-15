#include "cpu_features.h"

#include <cstdlib>
#include <cstring>

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif

static bool avx2_disabled_by_environment() {
  const char *value = std::getenv("KEYHUNT_DISABLE_AVX2");
  return value != nullptr && value[0] != '\0' && std::strcmp(value, "0") != 0;
}

static bool detect_avx2() {
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
  int registers[4];
  __cpuid(registers, 1);
  const bool osxsave = (registers[2] & (1 << 27)) != 0;
  const bool avx = (registers[2] & (1 << 28)) != 0;
  if (!osxsave || !avx || (_xgetbv(0) & 0x6) != 0x6)
    return false;
  __cpuidex(registers, 7, 0);
  return (registers[1] & (1 << 5)) != 0;
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
  __builtin_cpu_init();
  return __builtin_cpu_supports("avx2");
#else
  return false;
#endif
}

bool keyhunt_avx2_available() {
  static const bool available = detect_avx2() && !avx2_disabled_by_environment();
  return available;
}

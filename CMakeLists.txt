cmake_minimum_required(VERSION 3.20)
project(KeyhuntRevived LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(keyhunt
  keyhunt.cpp
  getopt.cpp
  cpu_features.cpp
  util.c
  base58/base58.c
  rmd160/rmd160.c
  bloom/bloom.cpp
  oldbloom/bloom.cpp
  xxhash/xxhash.c
  sha3/sha3.c
  sha3/keccak.c
  secp256k1/Int.cpp
  secp256k1/Point.cpp
  secp256k1/SECP256K1.cpp
  secp256k1/IntMod.cpp
  secp256k1/Random.cpp
  secp256k1/IntGroup.cpp
  hash/ripemd160.cpp
  hash/ripemd160_sse.cpp
  hash/ripemd160_avx2.cpp
  hash/sha256.cpp
  hash/sha256_sse.cpp
  hash/sha256_avx2.cpp
)

# These files use C++ constructs despite their historical .c suffix.
set_source_files_properties(util.c rmd160/rmd160.c sha3/sha3.c sha3/keccak.c PROPERTIES LANGUAGE CXX)

if(MSVC)
  target_compile_definitions(keyhunt PRIVATE WIN64 NOMINMAX _CRT_SECURE_NO_WARNINGS)
  target_compile_options(keyhunt PRIVATE /O2 /W3 /EHsc)
  set_source_files_properties(
    hash/ripemd160_avx2.cpp hash/sha256_avx2.cpp
    PROPERTIES COMPILE_OPTIONS "/arch:AVX2"
  )
else()
  target_compile_options(keyhunt PRIVATE -O3 -mssse3)
  set_source_files_properties(
    hash/ripemd160_avx2.cpp hash/sha256_avx2.cpp
    PROPERTIES COMPILE_OPTIONS "-mavx2"
  )
  find_package(Threads REQUIRED)
  target_link_libraries(keyhunt PRIVATE Threads::Threads m)
endif()

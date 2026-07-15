# Native Windows build

## Requirements

- 64-bit Windows 10 or 11
- Visual Studio 2022 Build Tools (or Visual Studio 2022) with the
  **Desktop development with C++** workload
- CMake 3.20 or newer available on `PATH`

## Build

Open PowerShell in this directory and run:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-windows.ps1
```

The script creates a Release build and copies the result to `keyhunt.exe` in
this directory. To build manually:

```powershell
cmake -S . -B build-windows -A x64
cmake --build build-windows --config Release --parallel
```

The manual build output is `build-windows\Release\keyhunt.exe`.

## Quick verification

```powershell
.\keyhunt.exe -m rmd160 -f .\tests\1to32.rmd -r 1:400 -l compress -n 1024 -q -s 0 -t 1
```

A successful run prints ten matching private keys and ends normally.

## Notes

- The Windows build is native MSVC code; WSL, MinGW, pthreads, and external
  libraries are not required.
- Both Bitcoin and Ethereum address modes are included.
- AVX2 is detected at runtime. It is enabled on supported CPUs and the
  SSE-compatible fallback is used everywhere else. Set
  `KEYHUNT_DISABLE_AVX2=1` to force the fallback for testing.
- Use only on key ranges and targets you are authorized to test.

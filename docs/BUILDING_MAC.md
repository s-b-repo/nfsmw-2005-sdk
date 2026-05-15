# Building nfsmw_sdk mods on macOS

macOS cannot run NFSMW natively, but it can **cross-compile** the mod DLL
with MinGW-w64 from Homebrew. Run the game on a Windows box or via
CrossOver/Wine afterwards.

## 1. Install the toolchain

```bash
brew install mingw-w64 cmake
```

`brew install mingw-w64` provides `i686-w64-mingw32-gcc` /
`i686-w64-mingw32-g++` (the 32-bit target the SDK needs). Verify:

```bash
i686-w64-mingw32-g++ --version
```

If Homebrew only installed the x86_64 target, install a multilib build or
use the [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) toolchain,
which ships i686.

## 2. Build with CMake

Identical to Linux:

```bash
cd sdk
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/nfsmw-toolchain-mingw-i686.cmake
cmake --build build -j
```

## 3. llvm-mingw alternative (Apple Silicon friendly)

```bash
# download llvm-mingw release, add bin/ to PATH, then:
cmake -S . -B build \
  -DCMAKE_C_COMPILER=i686-w64-mingw32-clang \
  -DCMAKE_CXX_COMPILER=i686-w64-mingw32-clang++ \
  -DCMAKE_SYSTEM_NAME=Windows
cmake --build build -j
```

llvm-mingw runs natively on arm64 macOS and emits i686 PE — no Rosetta
needed for the build step.

## 4. Test

Copy the resulting `.asi`/`.dll` to a Windows or CrossOver NFSMW install.
The build host being macOS has no effect on the artifact — it is a plain
i386 Win32 DLL. Verify:

```bash
file build/examples/infinite_nos.dll
# PE32 executable (DLL) (console) Intel 80386, for MS Windows
```

The IDE/clangd squiggles about `windows.h` are expected (see
[BUILDING_LINUX.md](BUILDING_LINUX.md) Notes).

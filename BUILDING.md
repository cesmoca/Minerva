# Building Minerva

This document is the authoritative source-build guide for Minerva. The project
uses CMake and C++17 and currently supports two tested Windows x64 toolchains:

| Toolchain | Configure preset | Build output |
| --- | --- | --- |
| Visual Studio 2026 + vcpkg | `visual-studio-debug` | `build/visual-studio-debug/Debug` |
| MSYS2 UCRT64 | `windows-msys2-debug` | `build/windows-msys2-debug` |

Matching `-release` configure, build, and test presets are also provided.

## Build products

The default build creates:

| Target | Purpose |
| --- | --- |
| `minerva_kernel` | Static engine library |
| `minerva` | MSL authoring and packaging frontend |
| `player` | Packaged-application runtime |
| `minerva_smoke` | Interactive runtime, core, and MSL-preprocessor smoke utility |
| `minerva_kernel_tests` | GoogleTest executable |
| `minerva_tests` | Builds every executable and runs CTest |
| `minerva_generate_msl` | Regenerates the committed MSL parser and scanner |

## Common requirements

- 64-bit Windows
- CMake 3.20 or newer
- Git
- A C++17 compiler
- Network access during the first configure unless fetched dependencies are
  already cached
- Flex/Flex++'s `FlexLexer.h` runtime header

Minerva depends on Boost.Filesystem, Boost.Python, Python, libzip, OpenCV Core
and Video I/O, SDL 1.2, SDL_image 1.2, SDL_mixer 1.2, SDL_ttf 2.0, FreeType,
JPEG, PNG, OpenGL/GLU, and FreeGLUT. Bullet 2.78 and ARToolKit 2.72.1 are
included in the repository.

The build downloads and compiles lib3ds 1.3.0 from its pinned source archive.
When no installed GoogleTest package is available, test-enabled builds also
download the pinned GoogleTest source.

## Visual Studio and vcpkg

The supplied preset expects:

- Visual Studio 2026 with the Desktop development with C++ workload;
- a Windows SDK;
- vcpkg at `C:\vcpkg`;
- the `x64-windows-static-md` triplet; and
- MSYS2 Flex installed at `C:\msys64\usr\include\FlexLexer.h`.

From the repository root, install the vcpkg dependencies:

```powershell
C:\vcpkg\vcpkg.exe install boost-filesystem boost-python libzip freeglut gtest opencv4[core,dshow,msmf] sdl1 sdl1-image sdl1-mixer sdl1-ttf --triplet x64-windows-static-md --overlay-ports="$PWD\ports"
```

The repository's `ports` directory supplies pinned SDL_image and SDL_ttf
overlay ports compatible with the static SDL 1.2 triplet.

Install the Flex runtime header through MSYS2:

```powershell
C:\msys64\usr\bin\pacman.exe --noconfirm -S --needed flex
```

Configure, build, and test Debug:

```powershell
cmake --preset visual-studio-debug
cmake --build --preset visual-studio-debug
ctest --preset visual-studio-debug
```

Release:

```powershell
cmake --preset visual-studio-release
cmake --build --preset visual-studio-release
ctest --preset visual-studio-release
```

To build every production frontend and immediately run the complete test
suite:

```powershell
cmake --build --preset visual-studio-debug --target minerva_tests -- /m:1
```

The explicit `/m:1` is useful on constrained machines because the legacy
translation units and debug program databases can make parallel MSBuild
invocations resource-intensive.

### Visual Studio IDE

Open the repository with **File > Open > Folder** and select the
`Visual Studio 2026 x64 (Debug)` CMake preset. `Ctrl+B` builds the selected
configuration. In CMake Targets View, build `minerva_tests` to compile all
frontends and run CTest.

If Visual Studio reports no CMake configuration, enable CMake in
**Project > CMake Workspace Settings**, then use
**Project > Delete Cache and Reconfigure**.

## MSYS2 UCRT64

Install MSYS2 at `C:\msys64`, then install the compiler, build tools, Flex
runtime, and matching UCRT64 dependencies:

```powershell
C:\msys64\usr\bin\pacman.exe --noconfirm -S --needed flex make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-libzip mingw-w64-ucrt-x86_64-freeglut mingw-w64-ucrt-x86_64-gtest mingw-w64-ucrt-x86_64-opencv mingw-w64-ucrt-x86_64-SDL mingw-w64-ucrt-x86_64-SDL_image mingw-w64-ucrt-x86_64-SDL_mixer mingw-w64-ucrt-x86_64-SDL_ttf
```

Configure, build, and test Debug:

```powershell
cmake --preset windows-msys2-debug
cmake --build --preset windows-msys2-debug
ctest --preset windows-msys2-debug
```

Release:

```powershell
cmake --preset windows-msys2-release
cmake --build --preset windows-msys2-release
ctest --preset windows-msys2-release
```

To build every frontend and run the complete suite with limited parallelism:

```powershell
cmake --build --preset windows-msys2-debug --target minerva_tests -j 2
```

Do not mix MSYS2 headers, import libraries, or runtime DLLs with the vcpkg
MSVC dependency family.

## Running from the build tree

MSYS2 Debug:

```powershell
.\build\windows-msys2-debug\minerva_smoke.exe
.\build\windows-msys2-debug\minerva_smoke.exe --core
.\build\windows-msys2-debug\minerva_smoke.exe path\to\application.mrv
.\build\windows-msys2-debug\minerva.exe path\to\application.mrv
.\build\windows-msys2-debug\player.exe
```

Visual Studio Debug:

```powershell
.\build\visual-studio-debug\Debug\minerva_smoke.exe
.\build\visual-studio-debug\Debug\minerva_smoke.exe --core
.\build\visual-studio-debug\Debug\minerva_smoke.exe path\to\application.mrv
.\build\visual-studio-debug\Debug\minerva.exe path\to\application.mrv
.\build\visual-studio-debug\Debug\player.exe
```

`minerva_smoke` without arguments (or with `--runtime`) opens a window and
exercises the resource-free portions of SDL/OpenGL, OpenCV, Bullet, ARToolKit,
image/audio/font support, lib3ds, libzip, and Boost.Filesystem. It also tries
camera device 0 and displays its frames when available; camera failure is
non-fatal and leaves an animated background. Close the window or press Escape
to exit, or use `--runtime <seconds>` for a bounded run. `--core` performs the
deterministic lifecycle check used by CTest. With one MSL path, the utility
prints the preprocessed source without invoking the full parser or hardware
stack. On Windows, the smoke explicitly uses OpenCV's DirectShow backend
because the Media Foundation hardware-accelerated path is known to hang during
camera initialization on the currently tested system.

The `minerva` frontend initializes the engine, preprocesses and parses the MSL
file, writes `data.dat`, and attempts to copy the player as `app`. The `player`
expects the package in its application directory and immediately initializes
Python, SDL/OpenGL, camera input, AR tracking, physics, input, audio, and game
logic.

## Testing

CTest discovers the GoogleTest cases from `minerva_kernel_tests` and also runs
the smoke and command-line checks:

```powershell
ctest --preset visual-studio-debug
ctest --preset windows-msys2-debug
```

Run one test by name:

```powershell
ctest --test-dir build/windows-msys2-debug -R "MSLParserTest" --output-on-failure
```

The automated suite does not launch `player`, because a deterministic player
run requires a packaged application plus camera, graphics, audio, and tracking
hardware.

## Other CMake environments

The generic flow is:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

All required packages must be discoverable by CMake, and their compiler,
architecture, runtime, and Debug/Release families must match. The supplied
Windows presets are the only configurations currently verified.

If vcpkg or MSYS2 is installed somewhere other than the preset paths, use a
user preset or configure manually with the appropriate
`CMAKE_TOOLCHAIN_FILE`, `CMAKE_PREFIX_PATH`, compiler, generator, and triplet.
Do not commit machine-local paths to the shared presets.

## MSL parser generation

Normal builds compile the committed generated files and do not require
Bison++ or parser regeneration. They do require the Flex C++ runtime header
`FlexLexer.h`.

Only regenerate after changing:

- `source/Kernel/Parsers/MSLParser.y`; or
- `source/Kernel/Parsers/MSLScanner.l`.

The grammar uses the historical Bison++ dialect, not modern GNU Bison's C++
interface. Reproducible output requires:

- Bison++ 1.21.11 with Debian's 1.21.11-5 portability patches; and
- Flex/Flex++ 2.6.4.

After making both executables discoverable by CMake, reconfigure and run:

```powershell
cmake --build --preset windows-msys2-debug --target minerva_generate_msl
```

The target rewrites:

```text
include/Kernel/Parsers/MSLParser.h
source/Kernel/Parsers/MSLParser.cpp
source/Kernel/Parsers/MSLScanner.cpp
```

Review the complete generated diff before committing it. The existing grammar
reports shift/reduce and reduce/reduce conflicts; these diagnostics are known,
but newly introduced diagnostics should be investigated.

## Installation and deployment

The project does not currently define CMake `install()` or packaging rules.
There is therefore no separate system-install procedure.

Run tools from their build directory during development. A deployable player
needs more than `player.exe`: it also needs a valid `data.dat`, matching
runtime libraries for the selected toolchain, camera and graphics/audio
support, and any application-specific calibration or resource files.

## Troubleshooting

### `FlexLexer.h` was not found

Install the MSYS2 `flex` package or configure with a compatible Flex 2.6.4
include directory. The committed scanner still includes this runtime header.

### A dependency is found from the wrong toolchain

Delete the affected build directory or CMake cache, then reconfigure with the
correct preset. Do not place both vcpkg and MSYS2 dependency directories on the
same CMake search path.

### lib3ds or GoogleTest cannot be downloaded

Check network/proxy access or populate CMake's FetchContent cache in advance.
lib3ds is always fetched; GoogleTest is fetched only when no installed package
is found.

### Embedded Python cannot find `encodings`

Run tests through CTest. Native test properties set `PYTHONHOME` from the
vcpkg Python installation. If running the test executable directly, set
`PYTHONHOME` to the matching interpreter directory first.

### The player cannot open a camera

The legacy runtime opens camera device 0. Confirm that a camera is present,
available to desktop applications, and supported by the selected OpenCV
backend.

### Visual Studio reports SDLmain runtime or PDB warnings

Debug builds currently warn that the package-provided SDLmain object uses the
release CRT and has no matching PDB. The binaries link and command-line smoke
test successfully, but this packaging limitation remains unresolved. Do not
silence it with `/NODEFAULTLIB` unless the full runtime combination has been
validated.

### An MSL or resource error terminates the process

Several legacy parser and resource paths still use process termination for
error reporting. Validate input paths and MSL syntax before running the full
authoring frontend, and use `minerva_smoke <file>` to inspect preprocessing
separately.

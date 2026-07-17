# Building Minerva

Minerva is being restored in stages. The current CMake build compiles the
completed standard-library baseline and resource I/O phase:

- `minerva_kernel`, containing the logger, application end controller, path and
  abstract tracking state, MSL include preprocessor, and resource classes.
- `minerva_smoke`, a small executable that validates the compiled kernel.

The generated MSL parser and scanner are committed to the repository, but they
are not part of a CMake target yet. Their original semantic actions directly
depend on the world, resource, MAO, MLB, and physics domains. Keeping the files
out of the baseline target avoids accidentally activating those subsystems.

The renderer, camera, concrete tracking, physics, scripting, audio, generated
MSL parser implementation, and model-loading code are not part of the target
yet.

## Active third-party dependencies

The resource phase requires Boost.Filesystem and libzip; libzip also requires
zlib and may use bzip2. CMake uses installed packages and does not download
these production dependencies.

For Visual Studio, the presets use a standalone vcpkg installation at
`C:\vcpkg` and the `x64-windows-static-md` triplet. Install the active packages
with:

```powershell
C:\vcpkg\vcpkg.exe install boost-filesystem libzip gtest --triplet x64-windows-static-md
```

The Visual Studio preset supplies vcpkg's CMake toolchain file and triplet so
Debug and Release dependencies use the matching MSVC runtime libraries.

For MSYS2 UCRT64, install the matching packages with:

```powershell
C:\msys64\usr\bin\pacman.exe --noconfirm -S --needed mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-libzip mingw-w64-ucrt-x86_64-gtest
```

The MSYS2 preset restricts package discovery to `C:\msys64\ucrt64`, preventing
MSVC and MinGW packages from being mixed. GoogleTest retains its existing
find-first, pinned FetchContent fallback, but Boost.Filesystem and libzip must
be installed before configuration.

## Regenerating the MSL parser and scanner

Normal Visual Studio and CMake builds use the committed generated files and do
not require parser-generator tools. CMake exposes a `minerva_generate_msl`
target for intentional regeneration. If any generated output is missing and
both generators are detected, that target is added to the normal build and
recreates the files automatically.

If Bison++ or Flex++ is unavailable, CMake does not install packages or access
the network during configuration. Such system changes require an explicit user
decision and are not portable across package managers. The baseline build
continues when committed outputs exist; if outputs are missing, CMake prints a
warning directing the user to the manual procedure below.

Follow this section when setting up the tools, when `MSLParser.y` or
`MSLScanner.l` changes, or when verifying the historical generators.

The grammar uses the obsolete Bison++ dialect, not the current GNU Bison C++
interface. The verified tool versions are:

- Bison++ source package 1.21.11 with the Debian 1.21.11-5 portability patches.
- Flex/Flex++ 2.6.4.
- MSYS2 UCRT64 GCC; GCC 16.1.0 was used for the verified Bison++ build.

Bison++ has stale embedded version text and prints `1.21.9-1` when invoked with
`-V`. The downloaded source and Debian changelog are nevertheless version
1.21.11. Verify the archive hashes below instead of relying on that banner.

### 1. Install the MSYS2 build tools

Open an **MSYS2 UCRT64** terminal and run:

```sh
pacman --noconfirm -S --needed flex patch make mingw-w64-ucrt-x86_64-gcc
```

The `flex` package installs both `flex` and `flex++`. Bison++ is not available
as an MSYS2 package and must be built from source.

### 2. Download the verified Bison++ source

Use an empty disposable directory. These commands deliberately keep the old
generator outside the repository:

```sh
mkdir -p /c/tmp/minerva-parser-tools
cd /c/tmp/minerva-parser-tools

curl.exe -fL https://deb.debian.org/debian/pool/main/b/bison++/bison++_1.21.11.orig.tar.gz -o bison++_1.21.11.orig.tar.gz
curl.exe -fL https://deb.debian.org/debian/pool/main/b/bison++/bison++_1.21.11-5.debian.tar.xz -o bison++_1.21.11-5.debian.tar.xz
curl.exe -fL https://deb.debian.org/debian/pool/main/b/bison++/bison++_1.21.11-5.dsc -o bison++_1.21.11-5.dsc
```

Calculate the SHA-256 hashes:

```sh
sha256sum bison++_1.21.11.orig.tar.gz bison++_1.21.11-5.debian.tar.xz
```

The results must be:

```text
d274bd25b354b50fd64884883ee46aba22e17728ee190f063db0b7254b662517  bison++_1.21.11.orig.tar.gz
cb737fb2ce79acc968b9cc183b4d77eae9a30daed9d1fb170349b5591dd32de5  bison++_1.21.11-5.debian.tar.xz
```

The same hashes are recorded in the downloaded Debian `.dsc` descriptor.

### 3. Extract and patch Bison++

The original tarball contains a Unix `bison` symbolic link that Windows may
refuse to create. Exclude that optional alias during extraction:

```sh
cd /c/tmp/minerva-parser-tools
tar -xzf bison++_1.21.11.orig.tar.gz --exclude=bison++-1.21.11/bison
tar -xJf bison++_1.21.11-5.debian.tar.xz -C bison++-1.21.11
cd bison++-1.21.11
```

Apply Debian's published portability patches in their declared order:

```sh
while read patch_name; do
    patch -p1 < debian/patches/${patch_name}
done < debian/patches/series
```

### 4. Compile and install Bison++ into the disposable directory

Configure and compile with the UCRT64 compiler:

```sh
export PATH=/ucrt64/bin:/usr/bin
./configure --prefix=/c/tmp/minerva-parser-tools/install
make -j2
```

The build emits many warnings caused by the generator's 1990s C++ source, but
it completes with the current compiler. The old install hook expects the
excluded `bison` alias, so provide a copy before installing:

```sh
cp bison++.exe bison.exe
make install
```

Verify both generators:

```sh
/c/tmp/minerva-parser-tools/install/bin/bison++ -V
flex++ --version
```

Expected version output includes:

```text
bison++ Version 1.21.9-1
flex++ 2.6.4
```

### 5. Generate Minerva's MSL C++ files

Stay in the MSYS2 UCRT64 terminal. Change to the repository root and keep
`/ucrt64/bin` on `PATH` because the locally built Bison++ executable needs the
UCRT runtime DLLs:

```sh
cd /c/path/to/Minerva
export PATH=/ucrt64/bin:/usr/bin
```

Generate the parser implementation and header:

```sh
/c/tmp/minerva-parser-tools/install/bin/bison++ \
    -d \
    -hinclude/Kernel/Parsers/MSLParser.h \
    -o source/Kernel/Parsers/MSLParser.cpp \
    source/Kernel/Parsers/MSLParser.y
```

Generate the C++ scanner:

```sh
flex++ \
    -d \
    -osource/Kernel/Parsers/MSLScanner.cpp \
    source/Kernel/Parsers/MSLScanner.l
```

These commands create or replace exactly these committed files:

```text
include/Kernel/Parsers/MSLParser.h
source/Kernel/Parsers/MSLParser.cpp
source/Kernel/Parsers/MSLScanner.cpp
```

The verified grammar generation reports one empty typed rule, one useless
nonterminal/rule, six shift/reduce conflicts, and forty reduce/reduce
conflicts. Those diagnostics come from the existing grammar and are not a
generator failure.

### 6. Generate through CMake

After the tools are available, configure and invoke the dedicated target:

```powershell
cmake --preset visual-studio-debug
cmake --build --preset visual-studio-debug --target minerva_generate_msl
```

The MSYS2 equivalent is:

```powershell
cmake --preset windows-msys2-debug
cmake --build --preset windows-msys2-debug --target minerva_generate_msl
```

CMake searches `PATH` and also checks the verified Windows locations under
`C:\tmp\minerva-parser-tools\install\bin` and `C:\msys64\usr\bin`. Reconfigure
after installing tools so cached `NOTFOUND` results are refreshed. Invoking the
target explicitly always regenerates all three outputs. When an output is
missing, the target runs as part of the normal build if both tools were found.

Run `git diff` after regeneration. Do not add the generated sources to a CMake
target in isolation: compiling their semantic actions also requires the World,
ResourcesManager, MAOFactory, MLBFactory, and PhysicsController dependency
chains. They will be activated when those engine domains have reached the
gradual build.

## Gradual compilation roadmap

The restoration build should remain cumulative: a phase is added only after
all earlier phases compile and test with both the Visual Studio and MSYS2
presets. The goal is compilation compatibility, not redesign or new behavior.
No phase requires sample scenes, models, scripts, or other application assets.

The order below comes from the current header and implementation dependency
graph. External libraries listed for a phase must be made available to both
toolchains before that phase enters `minerva_kernel`.

| Phase | Compilation unit or subsystem | Requires from earlier phases | New external requirement |
| --- | --- | --- | --- |
| 0 | Current baseline: `Logger`, `EndController`, and `MSLPreprocessor` | None | None |
| 1 | Dependency-free leaves: `PathPoint` and the abstract `TrackingMethod` state class | Phase 0 | None |
| 2 | Current resource I/O: `Resource`, `ResourceFile`, `ResourceZip`, and `ResourcesManager` | `Logger` and `Singleton` from phase 0 | Boost.Filesystem and libzip |
| 3 | Value/property types and camera foundation: `MAOValue`, `MAOProperty`, `VideoSource`, and `VideoFactory` | `Logger`, `Singleton` | Current OpenCV headers and libraries; compatibility for the removed `cv.h` and `highgui.h` entry points |
| 4 | Python binding foundation: `WrapperTypes` | Standard-library baseline | A buildable embedded Python and matching Boost.Python. The original code targets Python 2.7, so the least-change viable toolchain must be established here |
| 5 | Core MAO objects: `MAO`, `MAOPositionator3D`, `MAOMark`, and `MAOMarksGroup` | Resources, value/property types, Python binding foundation | SDL headers plus the OpenCV and Boost.Python dependencies already introduced |
| 6 | Rendering objects: 2D base/image/text and 3D base/line/path/model classes | Core MAO, `PathPoint`, and `VideoFactory` | SDL, SDL_image, SDL_ttf, desktop compatibility OpenGL/GLU/GLUT, and Bullet collision headers for the 3D base |
| 7 | Model loading: parser base, OreJ, OBJ, and 3DS loaders | Resources and `MAORenderable3DModel` | SDL_image/OpenGL already introduced, plus lib3ds for the 3DS loader |
| 8 | AR tracking: `TrackingMethodARTK` and `TrackingMethodFactory` | Abstract tracking, resources, video, marks, and marker groups | Vendored ARToolKit 2.72.1 and its OpenGL-facing support |
| 9 | `MAOFactory` | All MAO classes, model loaders, and tracking factory | No new library beyond earlier phases |
| 10 | Physics: `GLDebugDrawer`, `PhysicObject`, `PhysicDynamicObject`, and `PhysicsController` | 3D MAOs and `MAOFactory` | Vendored Bullet 2.78 and OpenGL; only compiler-required 64-bit compatibility fixes should be made |
| 11 | Logic-brick foundation: `MLB`, base sensor/controller/actuator classes, simple sensors, Boolean controllers, and MAO-only actuators | Core/renderable MAOs, properties, resources, and Python foundation | No new library for the purely logical bricks |
| 12 | Platform logic bricks and input: keyboard/input controller, sound actuator, script controller, collision sensor, and dynamic-object actuator | Logic-brick foundation, physics, `MAOFactory`, and Python binding foundation | SDL event handling, SDL_mixer, and embedded Python already introduced |
| 13 | `MLBFactory` and `GameLogicController` | Every MLB class, `MAOFactory`, physics, input, and Python wrapper support | No new library beyond earlier phases |
| 14 | `World` | Rendering classes, video factory, MAO factory, physics, SDL, and OpenGL | SDL_mixer and the established rendering stack |
| 15 | Generated MSL parser/scanner plus `MSLProperties` | `World`, resources, both factories, physics, MAO values/properties, OpenCV, Bullet, and FlexLexer headers | Flex runtime header; the generators themselves are not required for normal builds |
| 16 | Embedded `MGE` module and `MPYWrapper` | MAO/MLB factories, script controller, and all registered wrapper types | Embedded Python and Boost.Python already introduced |
| 17 | Original `minerva` authoring executable | World, resources, MSL preprocessor/parser, Python wrapper, and packaging | Boost.Filesystem and libzip already introduced |
| 18 | Original `player` runtime executable | All runtime controllers, world, tracking, video, physics, MSL, and Python | Complete dependency set from earlier phases |

Phase 2 is complete. Phase 3 is the next dependency investigation, but it must
remain incremental because the original video classes use legacy OpenCV entry
headers and APIs.

Some later phases are necessarily clusters. In particular, the original
factory, physics, world, logic-brick, and parser headers form cycles or include
large concrete class sets. Splitting those cycles would be an architectural
refactor, so the restoration plan defers the cluster until all of its original
dependencies compile instead of introducing temporary substitutes.

## Visual Studio Code

The repository includes CMake presets and workspace configuration for the
MSYS2 UCRT64 compiler installed under `C:\msys64`.

1. Open the repository folder in VS Code.
2. Install the recommended **CMake Tools** and **C/C++** extensions when VS
   Code offers them.
3. Select `Windows MSYS2 UCRT64 (Debug)` if CMake Tools asks for a configure
   preset.
4. Press `Ctrl+Shift+B` to configure and build.

The status bar and Command Palette also expose the normal CMake Tools
`Configure`, `Build`, and `Test` actions. The following workspace tasks are
available through **Terminal: Run Task**:

- `CMake: Configure (Debug)`
- `CMake: Build (Debug)`
- `CMake: Test (Debug)`
- `CMake: Run smoke check`

The Debug executable is written to:

```text
build/windows-msys2-debug/minerva_smoke.exe
```

The generated `compile_commands.json` supplies accurate include paths and
compiler flags to IntelliSense through CMake Tools.

## Visual Studio 2026

Visual Studio has its own CMake integration; the `.vscode` files are not used
by it. Open the repository directory with **File > Open > Folder**. Do not open
the generated `.vs/Minerva.slnx` file directly.

Visual Studio should select `Visual Studio 2026 x64 (Debug)` from
`CMakePresets.json`. If it selects another preset, choose the Visual Studio
preset from the configuration dropdown. After CMake generation finishes,
`Ctrl+B` builds the active CMake configuration.

The Debug executable is written to:

```text
build/visual-studio-debug/Debug/minerva_smoke.exe
```

If Visual Studio shows **No Configurations**, use
**Project > CMake Workspace Settings**, enable CMake for the workspace, and
then select **Project > Delete Cache and Reconfigure**. Visual Studio stores
this per-workspace choice in the ignored `.vs/CMakeWorkspaceSettings.json`
file.

The equivalent command-line flow is:

```powershell
cmake --preset visual-studio-debug
cmake --build --preset visual-studio-debug
ctest --preset visual-studio-debug
```

To compile and run every active test from the IDE, open Visual Studio's
**CMake Targets View**, right-click `minerva_tests`, and select **Build**. This
target builds `minerva_smoke` and `minerva_kernel_tests`, then runs CTest with
the active Visual Studio configuration and displays failures in the build
output. The generator-provided `RUN_TESTS` target performs the same CTest run,
but `minerva_tests` is the project-specific target to use.

The kernel unit tests use GoogleTest 1.17.0. When `BUILD_TESTING` is enabled,
CMake first uses an installed GoogleTest package if one is available;
otherwise it downloads the pinned source into the selected build directory.
This does not install GoogleTest system-wide. The first test-enabled configure
therefore requires network access when GoogleTest is not already installed.

CTest dashboard projects such as `Continuous`, `Nightly`, and `Experimental`
are intentionally disabled. `ALL_BUILD`, `RUN_TESTS`, and `ZERO_CHECK` are
generated by Visual Studio's CMake generator and remain as infrastructure.
The original `minerva` authoring executable and `player` runtime will be added
only after their dependency phases compile; placeholder targets would give a
false impression that those applications are currently buildable.

## Windows with the existing MSYS2 installation

Configure:

```powershell
cmake --preset windows-msys2-debug
```

Build:

```powershell
cmake --build --preset windows-msys2-debug
```

Run:

```powershell
.\build\windows-msys2-debug\minerva_smoke.exe
```

Run the CMake test:

```powershell
ctest --preset windows-msys2-debug
```

The smoke executable can also preprocess an MSL file without invoking the
generated parser:

```powershell
.\build\windows-msys2-debug\minerva_smoke.exe path\to\application.mrv
```

## Other CMake environments

Use the normal configure/build flow with an available C++17 compiler and CMake
generator:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Boost.Filesystem and libzip must be discoverable by CMake. For custom build
trees, pass the appropriate vcpkg toolchain or package prefix described above.

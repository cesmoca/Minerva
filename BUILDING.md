# Building Minerva

Minerva is being restored in stages. The current CMake build compiles the
completed standard-library baseline and resource I/O phase plus the
value/property, camera, Python binding, core MAO/logic foundations, SDL input,
3D collision/rendering foundations, 2D image rendering, and native OreJ/OBJ
and 3DS model parsing, and 2D TrueType text rendering:

- `minerva_kernel`, containing the logger, application end controller, path and
  abstract tracking state, MSL include preprocessor, resource classes,
  `MAOValue`, `MAOProperty`, `VideoSource`, `VideoFactory`, `WrapperTypes`, the
  active MAO/MLB classes, `InputEventController`, and `MLBSensorKeyboard`.
- `minerva_smoke`, a small executable that validates the compiled kernel.

The generated MSL parser and scanner are committed to the repository, but they
are not part of a CMake target yet. Their original semantic actions directly
depend on the world, resource, MAO, MLB, and physics domains. Keeping the files
out of the baseline target avoids accidentally activating those subsystems.

Concrete tracking, physics controllers, scripting, audio, generated MSL parser
implementation, the MAO factory, and the remaining renderer classes are not
part of the target yet.

## Active third-party dependencies

The resource phase requires Boost.Filesystem and libzip; libzip also requires
zlib and may use bzip2. The value and property classes require OpenCV Core;
`VideoSource` also requires OpenCV Video I/O. `WrapperTypes` requires
embedded Python and matching Boost.Python. Input requires SDL 1.2. Rendering
requires desktop compatibility OpenGL, the vendored Bullet 2.78 sources,
SDL_image 1.2 with JPEG and PNG codecs, and SDL_ttf 2.0.11 with FreeType.
The 3DS parser requires lib3ds 1.3.0. CMake uses installed packages for the
other production dependencies.

lib3ds 1.3.0 has no current vcpkg or MSYS2 UCRT64 package matching both build
flows. CMake therefore downloads the pristine upstream archive mirrored by
Debian, verifies SHA-512
`a315bd0f75cf87d8e285b5a405fe9f033900e23b363cdcf079142dc59edc94e63666e8ab2c0097d939689cd8da0fdcacacaa15f50d0ac3e8a9f5c79b854ab23b`,
and builds its C sources as the same static target under MSVC and MSYS2.
lib3ds source files identify their license as LGPL 2.1 or later. A first
configure therefore needs network access unless CMake's FetchContent cache is
already populated.

For Visual Studio, the presets use a standalone vcpkg installation at
`C:\vcpkg` and the `x64-windows-static-md` triplet. Install the active packages
with:

```powershell
C:\vcpkg\vcpkg.exe install boost-filesystem boost-python libzip gtest opencv4[core] sdl1 sdl1-image sdl1-ttf --triplet x64-windows-static-md --overlay-ports="$PWD\ports"
```

The repository-local `sdl1-image` and `sdl1-ttf` overlay ports pin the official
SDL_image 1.2.12 and SDL_ttf 2.0.11 sources and build them statically against
the same SDL 1.2 triplet. SDL_image uses static JPEG and PNG support; SDL_ttf
uses the triplet's FreeType package. This avoids mixing the static vcpkg SDL
library with incompatible DLL runtime families.

The Visual Studio preset supplies vcpkg's CMake toolchain file and triplet so
Debug and Release dependencies use the matching MSVC runtime libraries.

For MSYS2 UCRT64, install the matching packages with:

```powershell
C:\msys64\usr\bin\pacman.exe --noconfirm -S --needed mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-libzip mingw-w64-ucrt-x86_64-gtest mingw-w64-ucrt-x86_64-opencv mingw-w64-ucrt-x86_64-SDL mingw-w64-ucrt-x86_64-SDL_image mingw-w64-ucrt-x86_64-SDL_ttf
```

The MSYS2 preset restricts package discovery to `C:\msys64\ucrt64`, preventing
MSVC and MinGW packages from being mixed. GoogleTest retains its existing
find-first, pinned FetchContent fallback, but Boost.Filesystem, Boost.Python,
embedded Python, libzip, and OpenCV Core and Video I/O must be installed before
configuration. SDL 1.2 is intentionally used for the original event/key API;
SDL2 and SDL3 are not source-compatible substitutes for this restoration
phase.

The text-rendering tests use a real installed TrueType font instead of a
repository asset. They search common Windows Arial/Caladea and Linux DejaVu
locations and report a clear test failure if none is available.

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

Bundles A through G are active and verified. Bundle H is next.
The remaining roadmap is grouped by external dependency transitions instead of
single translation units. Every bundle must compile and test as one change on
both toolchains before introducing the next dependency set. A bundle marked
"none" uses only libraries activated by earlier bundles.

The fourteen bundles below partition the 60 original restoration translation
units exactly once.

### Bundle A: current-stack closure (20 units)

New external requirement: none. This bundle is active and verified in the
current working tree.

- Core MAO: `MAOMarksGroup.cpp`.
- Logic base: `MLB.cpp`, `MLBSensor.cpp`, `MLBSensorActuator.cpp`, and
  `MLBActuator.cpp`.
- Current-stack sensors: `MLBSensorAlways.cpp`, `MLBSensorDelay.cpp`,
  `MLBSensorProperty.cpp`, and `MLBSensorRandom.cpp`.
- Current-stack actuators: `MLBActuatorAng.cpp`, `MLBActuatorDistance.cpp`,
  `MLBActuatorProperty.cpp`, `MLBActuatorQuitApp.cpp`,
  `MLBActuatorRandom.cpp`, and `MLBActuatorRelativePose.cpp`.
- Boolean control: `MLBController.cpp`, `MLBControllerAND.cpp`,
  `MLBControllerNAND.cpp`, `MLBControllerNOR.cpp`, and `MLBControllerOR.cpp`.

These units use only the standard library and the already-active
Boost.Filesystem, embedded Python/Boost.Python, OpenCV Core, resource, property,
and MAO foundations. Obsolete include spellings may still require
compiler-proven compatibility edits, but no package installation is needed.

### Bundle B: SDL input foundation (2 units)

New external requirement: an SDL 1.2-compatible development/runtime package.

This bundle is active and verified.

- `InputEventController.cpp`.
- `MLBSensorKeyboard.cpp`.

The code uses SDL 1.2 event and key APIs; substituting SDL2 would be an API
migration, not a dependency-only restoration.

### Bundle C: 3D and collision foundation (8 units)

New external requirements: desktop compatibility OpenGL and the vendored
Bullet 2.78 collision/dynamics headers and libraries.

This bundle is active and verified.

- `MAORenderable3D.cpp`, `MAORenderable3DLine.cpp`,
  `MAORenderable3DModel.cpp`, and `MAORenderable3DPath.cpp`.
- `MLBActuatorAnim.cpp`, `MLBActuatorChangePose.cpp`,
  `MLBActuatorPathAddPoint.cpp`, and `MLBActuatorPathRemovePoints.cpp`.

The 3D base exposes Bullet collision types directly, so OpenGL-only and
Bullet-only compilation phases cannot be separated without changing public
headers.

### Bundle D: 2D image rendering (3 units)

New external requirement: SDL_image. SDL and OpenGL come from bundles B and C.

This bundle is active and verified in the Visual Studio preset, root Visual
Studio build, and MSYS2 preset. All three flows pass 96 tests, including a
real SDL_image load and OpenGL texture generation test.

- `MAORenderable2D.cpp` and `MAORenderable2DImage.cpp`.
- `MLBActuatorVisibility.cpp`, which is unblocked only after both 2D and 3D
  renderable bases exist.

### Bundle E: native model parsers (3 units)

New external requirement: none; SDL_image, OpenGL, resources, and the 3D model
class are already active.

This bundle is active and verified in the Visual Studio preset, root Visual
Studio build, and MSYS2 preset. All three flows pass 98 tests. The parser tests
load real OBJ and textured OreJ files under a real OpenGL context and verify
scaled geometry through generated Bullet collision bounds.

- `Parser.cpp`, `ParserOrej.cpp`, and `ParserObj.cpp`.

### Bundle F: text rendering (1 unit)

New external requirement: SDL_ttf.

This bundle is active and verified in the Visual Studio preset, root Visual
Studio build, and MSYS2 preset. All three flows pass 100 tests. The tests load
a real TrueType font through the resource manager, render UTF-8 text under a
real SDL/OpenGL context, regenerate its texture, and verify the legacy text
properties.

- `MAORenderable2DText.cpp`.

### Bundle G: 3DS model parser (1 unit)

New external requirement: lib3ds.

This bundle is active and verified in the Visual Studio preset, root Visual
Studio build, and MSYS2 preset. All three flows pass 102 tests. The tests
generate their 3DS fixtures through lib3ds itself, then load geometry,
animation state, materials, and a real resource-backed texture under an
OpenGL context. No third-party model fixture is stored in the repository.

- `Parser3ds.cpp`.

### Bundle H: AR tracking (2 units)

New external requirements: the vendored ARToolKit 2.72.1 libraries plus their
desktop GLU/GLUT support. OpenGL and the mark classes are already active.

- `TrackingMethodARTK.cpp`.
- `TrackingMethodFactory.cpp`.

### Bundle I: MAO factory closure (1 unit)

New external requirement: none. All concrete MAOs, model parsers, and tracking
implementations are active by this point.

- `MAOFactory.cpp`.

### Bundle J: physics closure (4 units)

New external requirement: none; Bullet, OpenGL, and GLUT were introduced by
bundles C and H.

- `GLDebugDrawer.cpp`, `PhysicObject.cpp`, `PhysicDynamicObject.cpp`, and
  `PhysicsController.cpp`.

The physics controller depends on `MAOFactory`, so these units cannot move into
the earlier Bullet bundle without breaking the internal dependency order.

### Bundle K: audio and world (2 units)

New external requirement: SDL_mixer. SDL_ttf, OpenGL/GLU/GLUT, physics, and the
factories are already active.

- `MLBActuatorSound.cpp`.
- `World.cpp`.

### Bundle L: engine and Python closure (8 units)

New external requirement: none.

- `MLBSensorCollision.cpp`, `MLBSensorNear.cpp`, and
  `MLBActuatorAddDynamicObject.cpp`.
- `MLBControllerScript.cpp` and `MLBFactory.cpp`.
- `MGEModule.cpp`, `MPYWrapper.cpp`, and `GameLogicController.cpp`.

This is an intentionally ordered closure: the concrete physics/factory bricks
precede the script controller, the complete MLB set precedes `MLBFactory`, and
both factories precede the full Python wrapper and game-logic controller.

### Bundle M: generated MSL frontend (3 units)

New external requirement: a cross-toolchain-accessible `FlexLexer.h` runtime
header. The already-installed `flex++` and `bison++` generators are needed only
when regenerating the committed outputs.

- `MSLProperties.cpp`, `MSLParser.cpp`, and `MSLScanner.cpp`.

Their semantic actions require `World`, both factories, physics, MAO
properties, and the complete Bullet/OpenCV stack, so compiling generated files
earlier would not be a real dependency reduction.

### Bundle N: original executables (2 units)

New external requirement: none.

- `minerva.cpp`.
- `player.cpp`.

These are final executable targets, not members of `minerva_kernel`; they enter
only after every runtime and authoring subsystem is active.

Some bundles are necessarily clusters because original headers expose concrete
types or form factory/controller cycles. Splitting those clusters would require
architectural test seams or public-header refactors, which are outside the
compilation-first restoration methodology.

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

Boost.Filesystem, Boost.Python, embedded Python, libzip, the OpenCV Core and
Video I/O components, SDL 1.2, SDL_image 1.2, SDL_ttf 2.0.11, FreeType, and
desktop OpenGL must be discoverable by CMake. For custom build trees, pass the
appropriate vcpkg toolchain or package prefix described above.

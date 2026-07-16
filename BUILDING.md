# Building Minerva

Minerva is being restored in stages. The current CMake build intentionally
compiles only a dependency-free baseline:

- `minerva_core`, containing the logger, application end controller, and MSL
  include preprocessor.
- `minerva_smoke`, a small executable that validates the compiled core.

The original renderer, camera, tracking, physics, scripting, audio, resource
packaging, generated MSL parser, and model-loading code are not part of the
target yet.

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

No third-party libraries are required for this baseline.

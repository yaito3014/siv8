# Building Siv3D (siv8) — Linux & Windows

State of the CMake + vcpkg migration (branch `vcpkg-cmake`). This is the handoff
for running the build on **Windows**, cross-checked against the **Linux** build
that is already green. Dependencies come from vcpkg manifest mode
(`vcpkg.json` + `overlay-ports/`); there is no vendored-lib / MSBuild path anymore.

## Layout

- **Root `CMakeLists.txt`** — the engine library only (`Siv3D::Siv3DCore`, fat
  `libSiv3D.a`). It is a pure library project; it has no app awareness.
- **`App/`** — the **top-level** project you build. It drives the engine via
  `add_subdirectory` (default) or `find_package` (opt-in). Configure/build from
  here, not from the repo root.
- **`overlay-ports/`** — forked/orphan ports (bc7enc-rdo, pffft, serial-cpp,
  glfw3, r8brain-free-src, easyexif, muparser, levenshtein-sse, rectpack2d, …).
  Selected via `vcpkg-configuration.json`.
- **`Test/`** — doctest/nanobench harness, compiled into the app when the
  `tests` feature is on.

## Prerequisites

- vcpkg cloned; `VCPKG_ROOT` set (the App CMakeLists auto-picks the toolchain
  from it). On Windows: VS 2022 + MSVC, CMake ≥ 3.25.
  - Use a **full clone** (`git clone https://github.com/microsoft/vcpkg.git`),
    not a shallow one. `vcpkg.json` pins a `builtin-baseline` commit and a
    `boost-geometry` version override, and vcpkg reads those from git history; a
    `--depth 1` clone is missing the version trees and fails with
    *"failed to unpack tree object … vcpkg was cloned as a shallow repository"*.
- Triplet: **`x64-windows-static`** (static CRT `/MT`). The App CMakeLists sets
  this and `CMAKE_MSVC_RUNTIME_LIBRARY` automatically on Windows.

## Build — Windows (Release verified green)

```bat
cd App
cmake -B build
cmake --build build --config Release
```

- Default mode is `add_subdirectory`: one configure builds the engine in-tree
  from the engine's own `vcpkg.json`. No install step.
- `tests` is ON by default (`SIV3D_APP_BUILD_TESTS`), which appends the `tests`
  feature and compiles `Test/` into the app so `RunTest()` is available. The
  standalone `Siv3D-Test` exe target is **not** built on Windows (it can't link
  against the GUI entry point in `Siv3DMainHelper`); Windows runs tests in-app.
- Output: a GUI-subsystem `Siv3D-App.exe` (~37 MB, self-contained static CRT),
  post-build staged into `App/app/` (the self-contained run directory, with the
  `engine/`, `example/` assets and `dll/` next to the exe). **Run it from there**
  — `App\app\Siv3D-App.exe` — or F5 in VS (working dir is wired to `app/`). The
  build-output `build/Release/Siv3D-App.exe` is **not** runnable in place: the
  engine `chdir`s to the exe's own folder and loads `engine/...` relative to it,
  so it only works from `App/app/`.
- `Resource.rc` embeds assets by paths relative to the staged tree (`engine/…`),
  so `App/CMakeLists.txt` puts `App/app/` on the RC `/I` include path.

To consume an already-installed/exported engine instead of building it in-tree:

```bat
cmake -B build -DSIV3D_APP_USE_FIND_PACKAGE=ON -DCMAKE_PREFIX_PATH=<prefix>
```

## Build — Linux (GLFW + OpenGL 4.1; renders the full 2D scene)

Linux has a **GLFW + OpenGL 4.1 backend** that builds end-to-end and **renders
the full 2D sample**: solid/gradient shapes, premultiplied-alpha blending,
transforms, textures/sprites + emoji, MSDF text (incl. CJK), all six fill
patterns, dashed/dotted line styles, and 4× MSAA edges matching the D3D11/Metal
output. `Siv3D-App` and `Siv3D-Test` both build into runnable ELF executables.

Built and render-verified headless (Xvfb + Mesa **llvmpipe**) and live on a
physical monitor via **WSLg** (the container ELF runs unchanged in an Ubuntu WSL
distro of the same release). Tested in the **`siv8` docker container**
(Ubuntu 26.04, gcc 15.2, cmake 4.2).

```bash
# One-time container setup (Ubuntu 26.04):
apt-get install -y build-essential cmake ninja-build pkg-config git curl zip \
  unzip tar autoconf automake libtool python3 bison flex \
  libgl-dev libx11-dev libxext-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev libxkbcommon-dev uuid-dev libpulse-dev libasound2-dev \
  xvfb mesa-utils libgl1-mesa-dri imagemagick      # last row: headless render-verify only
git clone https://github.com/microsoft/vcpkg.git /root/vcpkg   # FULL clone (see Prerequisites)
/root/vcpkg/bootstrap-vcpkg.sh -disableMetrics
export VCPKG_ROOT=/root/vcpkg

# Build (-j8 to parallelize). vcpkg builds all deps from source the first time.
cd /root/siv8/App
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux
cmake --build build -j8                 # builds Siv3D-App (+ Siv3D-Test)
./app/Siv3D-App                         # post-build stages the exe into App/app/

# Headless render check (software GL):
Xvfb :99 -screen 0 1280x720x24 &
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe ./app/Siv3D-App &
import -window root shot.png            # ImageMagick
```

The app no longer has to be launched from `App/app/`: at startup it `chdir`s to
the executable's own directory, so relative resource paths resolve against the
exe regardless of the shell's CWD.

Dependencies: `glfw3` (Siv3D fork, overlay-port — provides `glfwGetKeysSiv3D`
etc.) + `glad` (GL 4.1 loader) + system OpenGL/X11. The root `CMakeLists.txt`
`elseif(UNIX)` block links `glad::glad`, `OpenGL::GL`, `X11`, and `PkgConfig::UUID`.

### Linux platform-source layout

Platform code for Linux comes from two trees (see the `elseif(UNIX)` blocks in
the root `CMakeLists.txt`):

- **`Siv3D-Platform/macOS_Linux/`** — code shared with the macOS port
  (DateTime, EnvironmentVariable, BinaryFile*, the shared `FileSystem` subset:
  `FullPath`/`CurrentDirectory`/`Size`/timestamps/`RemoveContents`, …).
- **`Siv3D-Platform/Linux/`** — **Linux-only** implementations:
  - `Time/` — `clock_gettime(CLOCK_MONOTONIC_RAW)` (macOS uses the Apple-only
    `clock_gettime_nsec_np`); `UTCOffsetMinutes` via `localtime_r().tm_gmtoff`.
  - `FileSystem/` — the `macOS/*.mm` complement ported to POSIX +
    `std::filesystem` + XDG special folders (`user-dirs.dirs`) + `/proc/self/exe`
    + a freedesktop trash impl.
  - `UserInfo/` — `getpwuid`/`gethostname`/`LANG`; `IsRunningInVisualStudio`/
    `IsRunningInXcode` → `false`.
  - `Resource/` — `Resource()`/`EnumResourceFiles()`; resource root is the
    exe's own directory (`App/app/`), so engine resources resolve to
    `App/app/engine/…` (no app bundle on Linux). A post-build step stages the
    exe into `App/app/` next to the committed `engine/`+`example/` assets, so it
    runs in place. `Siv3DMain.cpp` `chdir`s to `GetExecutableDirectory()` at
    startup (mirrors WindowsDesktop's `SetWorkingDirectory()`) so relative
    resource paths resolve against the exe, not the launch CWD.
  - `FreestandingMessageBox/` — falls back to `std::cerr` (no GUI backend).
  - `System/` — `CSystem` (engine orchestration) + `OpenInBrowser()` via
    `fork`+`execlp("xdg-open", …)`.
  - `Window/` + `GLFW/` — `CWindow` (GLFW, OpenGL 4.1 core context, requests a
    4× multisampled framebuffer via `GLFW_SAMPLES` to match the D3D11/Metal scene
    MSAA `Scene::DefaultMSAASampleCount`) + the `<Siv3D/GLFW/GLFW.hpp>` /
    `<Siv3D/Common/OpenGL.hpp>` loader headers.
  - `Cursor/` — `CCursor` (GLFW). `Mouse`/`Keyboard`/`CursorStyle` reuse the
    shared `macOS_Linux` GLFW input classes.
  - `Renderer/GL4/` — `CRenderer_GL4` (clears the scene, presents, enables
    `GL_MULTISAMPLE`) + a real `CRenderer2D_GL4`: GL programs for shape / texture
    / MSDF text / pattern / line, a draw-command batch layer (keyed by
    program/texture/subtype) over a streaming VBO/IBO, reusing the common
    `Vertex2DBuilder` for CPU tessellation. `CTexture_GL4` uploads RGBA8 textures
    via `AssetHandleManager`. `Shader`/`EngineShader`/`ConstantBuffer` remain thin
    (shaders are inlined GLSL in `CRenderer2D_GL4`).
  - `Siv3DMain.cpp` — Linux entry point (`main` → chdir to exe dir → engine init
    → user `Main`).
  - Device services (Clipboard, DragDrop, MediaTranscoder, NativeShare,
    Notifications, Pentablet, TextToSpeech) — no-op stubs.

`UUIDValue` pulls `uuid_generate` from system **libuuid**; the `elseif(UNIX)`
link block finds it with pkg-config (`pkg_check_modules(UUID REQUIRED
IMPORTED_TARGET uuid)` → `PkgConfig::UUID`) — libuuid ships `uuid.pc` but no
CMake package, and the vcpkg toolchain provides the `pkgconf` used to read it.

### Status & what's next

**Works:** window + GL 4.1 core context, the update loop, input polling, and the
full 2D pipeline — solid/gradient shapes, premultiplied-alpha blending,
`Mat3x2::Screen` transforms, textures/sprites + emoji, MSDF text (median + fwidth
AA, incl. CJK), all six fill patterns (PolkaDot/Stripe/Grid/Checker/Triangle/
HexGrid, faithful ports of the engine reference shaders with smoothstep edge AA),
dashed/dotted line styles, and 4× MSAA edges.

**Not done (`TODO(linux)`):** shadows, quad-warp, MSDF outline/glow,
render-to-texture / scene-letterbox (MSAA currently lives on the window
framebuffer; move it to an explicit scene buffer when this lands), real custom
user shaders, hardware GL via WSLg (needs Mesa d3d12 + GPU passthrough — the
default WSLg d3d12 GL path brings up a context but doesn't present this engine's
draws, so force software GL with `LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe`).
Device services (clipboard/drag-drop/notifications/…) are still no-op stubs.

`Siv3DMain.cpp` lives in `libSiv3DCore.a`, so it provides `main()` → user
`Main()`. The standalone `Siv3D-Test` exe gets its `Main()` from
`Test/TestMain/TestMain.cpp` (a subdirectory so the engine's recursive
`Test/*.cpp` glob includes it but the App's non-recursive `../Test/*.cpp` glob
does not — avoiding a clash with `App/Main.cpp`). `Siv3D-Test` stays gated off on
Windows (tests run in-app there).

## Windows-specific items that are UNVERIFIED on Linux

Each was wired blind (Linux excludes the code path) and needs a real Windows
build to confirm. If the build breaks, check these first:

1. **doctest / `tests` feature & stale cache** — `option()` does NOT override an
   already-cached value. If a `build/` dir was first configured with
   `SIV3D_APP_BUILD_TESTS=OFF` (or before tests-on-by-default), vcpkg won't
   install doctest and the test include fails. **Fix: configure a fresh build
   dir** (delete `App/build/CMakeCache.txt`), or pass
   `-DSIV3D_APP_BUILD_TESTS=ON` explicitly. A clean configure resolves
   `VCPKG_MANIFEST_FEATURES=[tests]` → doctest + nanobench install.
2. **bc7enc-rdo** — `<cstdint>` is force-included via `/FIcstdint`
   (`overlay-ports/bc7enc-rdo/build-CMakeLists.txt`); no `.patch`. Confirm the
   6 core TUs compile under MSVC.
3. **pffft** (marton78) — `PFFFT_STATIC_DEFINE` is set PUBLIC so the export macro
   doesn't resolve to `__declspec(dllimport)` on a static build. Confirm
   `pffft:x64-windows-static` builds.
4. **wintoast** — wired WIN32-only (`unofficial::wintoast::wintoast`, header
   `<wintoastlib.h>`). 1.3.0→1.3.2 minor API drift unverified, and the vcpkg
   target carries NO system libs — if it needs `runtimeobject.lib` etc., add it
   to the engine's WIN32 link block (currently only d3d11/dxgi/…).
5. **mimalloc** — `App` / engine `SivMemory.cpp` includes `<mimalloc-new-delete.h>`
   and `Memory.hpp` includes `<mimalloc.h>` (Windows-only, `SIV3D_USE_MIMALLOC=1`).
6. **`.patch` line endings** — `.gitattributes` forces `*.patch text eol=lf` to
   stop autocrlf from corrupting `git apply` in overlay ports. Keep it.
7. **PCH** — `pch.h` (just `#include <Siv3D.hpp>`) via
   `target_precompile_headers`. The double-include `.ipp` redefinition issue was
   **GCC-only** (force-include + direct include); MSVC PCH is unaffected.

## Legacy auto-link removal (2026-06-22) — Windows impact

`Siv3D/lib/` (prebuilt `.lib`/`.a`) and the third-party
`#pragma comment(lib, ...)` block in `Siv3D/include/Siv3D/Windows/Libraries.hpp`
were **removed** — those libs now come from vcpkg imported targets. `Libraries.hpp`
now auto-links ONLY the Windows SDK system libs (dwmapi, mfplat, mfuuid, mincore,
Secur32, setupapi, winmm, wininet + the Common-Controls manifest). If a link
error names a third-party symbol (boost/freetype/harfbuzz/png/jpeg/opencv/zlib/
mimalloc), it means a vcpkg target is missing from a `target_link_libraries`, not
that an autolink pragma needs restoring.

## Still intentionally vendored

`Siv3D/src/ThirdParty/boost/geometry/extensions/algorithms/dissolve.hpp` — a
Boost.Geometry extension NOT in vcpkg, used by `PolygonDetail.cpp`, tied to the
`boost-geometry` 1.88 pin in `vcpkg.json`. Do not delete.

## macOS (not yet complete)

`App/CMakeLists.txt` builds a `.app` bundle (Info.plist + icon) but does NOT yet
stage runtime assets (`app/engine`, `app/example`) or the prebuilt
dylibs into the bundle — see the `NOTE:` in the `elseif(APPLE)` block.

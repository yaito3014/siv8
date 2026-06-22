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

## Build — Linux (GLFW + OpenGL 4.1; links, not yet rendering)

Linux now has a **GLFW + OpenGL 4.1 backend** and **links end-to-end**:
`Siv3D-App` and `Siv3D-Test` both build into runnable ELF executables. This is
**Phase 0** — the window opens and clears to the scene background color; real 2D
rendering (shapes/sprites/text) is still a no-op (`TODO(linux)`). Built and
link-verified in the headless `siv8` container; **not yet render-tested** (needs
a real GPU/display).

```bash
# in the "siv8" docker container (Ubuntu, gcc 15.2). -j8 to parallelize.
cd /root/siv8/App
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux
cmake --build build -j8                 # -> App/build/Siv3D-App (and Siv3D-Test)
```

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
    `resources/` dir next to the executable (no app bundle on Linux).
  - `FreestandingMessageBox/` — falls back to `std::cerr` (no GUI backend).
  - `System/` — `CSystem` (engine orchestration) + `OpenInBrowser()` via
    `fork`+`execlp("xdg-open", …)`.
  - `Window/` + `GLFW/` — `CWindow` (GLFW, OpenGL 4.1 core context) + the
    `<Siv3D/GLFW/GLFW.hpp>` / `<Siv3D/Common/OpenGL.hpp>` loader headers.
  - `Cursor/` — `CCursor` (GLFW). `Mouse`/`Keyboard`/`CursorStyle` reuse the
    shared `macOS_Linux` GLFW input classes.
  - `Renderer/GL4/` (`CRenderer_GL4` — clears + presents), plus no-op Phase-0
    `Renderer2D`/`Shader`/`Texture`/`EngineShader`/`ConstantBuffer` GL4 stubs and
    their factories.
  - `Siv3DMain.cpp` — Linux entry point (`main` → engine init → user `Main`).
  - Device services (Clipboard, DragDrop, MediaTranscoder, NativeShare,
    Notifications, Pentablet, TextToSpeech) — no-op stubs.

`UUIDValue` pulls `uuid_generate` from system **libuuid**; the `elseif(UNIX)`
link block finds it with pkg-config (`pkg_check_modules(UUID REQUIRED
IMPORTED_TARGET uuid)` → `PkgConfig::UUID`) — libuuid ships `uuid.pc` but no
CMake package, and the vcpkg toolchain provides the `pkgconf` used to read it.

### Phase-0 status & what's next

All `ISiv3D*::Create()` factories now resolve and the engine links. What works:
window creation, GL context, the engine update loop, input polling, and a
scene-background clear each frame. What's **not** done (Phase 1+, `TODO(linux)`):
the GL4 `Renderer2D` is a no-op, so nothing actually draws yet; `Shader`/
`Texture`/`EngineShader`/`ConstantBuffer` are stubs; the device services
(clipboard/drag-drop/notifications/…) are stubs. Render validation needs a real
GPU/display — the container can only confirm the build/link.

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

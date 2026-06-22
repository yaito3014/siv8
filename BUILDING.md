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

## Build — Windows

```bat
cd App
cmake -B build
cmake --build build --config Release
```

- Default mode is `add_subdirectory`: one configure builds the engine in-tree
  from the engine's own `vcpkg.json`. No install step.
- `tests` is ON by default (`SIV3D_APP_BUILD_TESTS`), which appends the `tests`
  feature and compiles `Test/` into the app so `RunTest()` is available.
- Output: a GUI-subsystem `Siv3D-App.exe`, post-build staged into
  `App/resources/` (the working dir with `engine/`, `example/` assets). Run it
  from there, or F5 in VS (working dir is wired to `resources/`).

To consume an already-installed/exported engine instead of building it in-tree:

```bat
cmake -B build -DSIV3D_APP_USE_FIND_PACKAGE=ON -DCMAKE_PREFIX_PATH=<prefix>
```

## Build — Linux (reference, already green)

Linux has no graphics backend yet, so the **engine library** builds but the app
does not fully link. Use it to verify compilation of shared/cross-platform code.

```bash
cd /root/siv8          # in the "siv8" docker container (Ubuntu, gcc 15.2)
cmake -B build3        # triplet x64-linux, Release, SIV3D_BUILD_TESTS=ON
cmake --build build3
```

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
stage runtime assets (`resources/engine`, `resources/example`) or the prebuilt
dylibs into the bundle — see the `NOTE:` in the `elseif(APPLE)` block.

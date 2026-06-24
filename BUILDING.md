# Building Siv3D (siv8) — Linux & Windows

State of the CMake + vcpkg migration (branch `vcpkg-cmake`). **Both Linux and
Windows build green**; the Linux GLFW/OpenGL backend renders the full 2D sample.
Dependencies come from vcpkg manifest mode (`vcpkg.json` + `overlay-ports/`);
there is no vendored-lib / MSBuild path anymore.

## Layout

- **Root `CMakeLists.txt`** — the engine library only (`Siv3D::Siv3DCore`, fat
  `libSiv3D.a`). A pure library project; it has no app awareness.
- **`App/`** — the **top-level** project you build. It drives the engine via
  `add_subdirectory` (default) or `find_package` (opt-in). Configure/build from
  here, not from the repo root.
- **`overlay-ports/`** — forked/orphan ports, selected via
  `vcpkg-configuration.json`: bc7enc-rdo, pffft, serial-cpp, glfw3 (Siv3D fork),
  r8brain-free-src, easyexif, muparser, levenshtein-sse, rectpack2d, and the
  header-only set obfuscate / xoshiro-cpp / enumbitmask / libcpprime.
- **`Test/`** — doctest/nanobench harness, compiled into the app when the
  `tests` feature is on.

## Prerequisites

- vcpkg cloned and `VCPKG_ROOT` set (the App CMakeLists auto-picks the toolchain
  from it).
  - Use a **full clone** (`git clone https://github.com/microsoft/vcpkg.git`),
    not a shallow one. `vcpkg.json` pins a `builtin-baseline` commit and version
    `overrides` (e.g. `boost-geometry`), which vcpkg reads from git history; a
    `--depth 1` clone is missing the version trees and fails with *"failed to
    unpack tree object … vcpkg was cloned as a shallow repository"*.
- Toolchain: **Windows** — VS 2022 or later (2026 tested) + MSVC, CMake ≥ 3.25.
  **Linux** — gcc 15+ / CMake ≥ 3.25 + the system dev packages (see *Build →
  Linux*).
- Triplet: **`x64-windows-static`** (static CRT) on Windows, **`x64-linux`** on
  Linux. The first run builds all vcpkg deps from source (skia/opencv/boost/… —
  slow); subsequent builds reuse the cache.

## Build

`App/` is the top-level project — configure/build from there. The flow is the
same on both platforms:

```
cd App
cmake -B build            # + the platform flags shown below
cmake --build build       # + --config Release on Windows
```

Common behavior (both platforms):

- **Default `add_subdirectory` mode**: one configure builds the engine in-tree
  from its own `vcpkg.json`; no install step. To consume an already
  installed/exported engine instead, opt in with
  `-DSIV3D_APP_USE_FIND_PACKAGE=ON -DCMAKE_PREFIX_PATH=<prefix>`.
- **Staging + run dir**: the build stages `Siv3D-App` into **`App/app/`**, a
  self-contained run directory next to the committed `engine/` + `example/`
  assets (and `dll/` on Windows). **Run it from `App/app/`** — at startup the
  engine `chdir`s to the executable's own directory, so relative resource paths
  (`engine/…`) resolve regardless of the launch CWD. The `build/` output copy is
  **not** runnable in place.
- **Tests**: `SIV3D_APP_BUILD_TESTS` is ON by default → the `tests` feature
  (doctest + nanobench) is added and `Test/` is compiled into the app so
  `RunTest()` is available.

### Windows (`x64-windows-static`; verified green)

```bat
cd App
cmake -B build
cmake --build build --config Release
```

- The triplet **`x64-windows-static`** (static CRT `/MT`) and
  `CMAKE_MSVC_RUNTIME_LIBRARY` are set automatically.
- Output: a GUI-subsystem `Siv3D-App.exe` (~38 MB, self-contained). `Resource.rc`
  embeds assets by paths relative to the staged tree, so `App/CMakeLists.txt`
  adds `App/app/` to the RC `/I` include path.
- The standalone `Siv3D-Test` exe is **not** built (it can't link the GUI entry
  point in `Siv3DMainHelper`); Windows runs tests in-app via `RunTest()`.

### Linux (GLFW + OpenGL 4.1 — renders the full 2D scene)

The Linux **GLFW + OpenGL 4.1 backend** renders the full 2D sample: solid/
gradient shapes, premultiplied-alpha blending, transforms, textures/sprites +
emoji, MSDF text (incl. CJK) with SDF outline/shadow/glow, all six fill patterns,
shadows, dashed/dotted lines, quad-warp, custom user shaders, the full 2D render
state, and 4× MSAA edges matching the D3D11/Metal output. Shaders + constants go
through the engine shader registry (`CEngineShader_GL4`) and real UBOs, bound via
a GL program pipeline. Both `Siv3D-App` and `Siv3D-Test` build into runnable ELFs.

Verified headless (Xvfb + Mesa **llvmpipe**) and live on a physical monitor via
**WSLg**. Tested in the **`siv8` docker container** (Ubuntu 26.04, gcc 15.2).

```bash
# One-time dev packages (Ubuntu 26.04):
apt-get install -y build-essential cmake ninja-build pkg-config git curl zip \
  unzip tar autoconf automake autoconf-archive libtool python3 bison flex \
  libgl-dev libx11-dev libxext-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev libxkbcommon-dev uuid-dev libpulse-dev libasound2-dev \
  libwayland-dev wayland-protocols libffi-dev \
  xvfb mesa-utils libgl1-mesa-dri imagemagick      # last row: headless render-verify only

# Build (full vcpkg clone; first run builds all deps from source):
cd App
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux
cmake --build build -j8

# Headless render check (software GL):
Xvfb :99 -screen 0 1280x720x24 &
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe ./app/Siv3D-App &
import -window root shot.png            # ImageMagick
```

Dependencies: `glfw3` (Siv3D fork overlay-port — provides `glfwGetKeysSiv3D`
etc., dual X11+Wayland via the `wayland` feature) + `glad` (GL 4.1 loader) +
system OpenGL. `UUIDValue` pulls `uuid_generate` from system **libuuid** via
pkg-config (`pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)` →
`PkgConfig::UUID`; libuuid ships `uuid.pc` but no CMake package).

#### Runtime requirements (running the ELF on a Linux/WSL desktop)

The deps above are `-dev` packages; *running* the built `Siv3D-App` on a
**different** machine (e.g. copied into a WSL distro) needs the corresponding
**runtime** libraries there. Several are loaded with `dlopen()`, so a missing one
fails silently to a fallback rather than at link time.

- **Graphics** — OpenGL + DRI (`libgl1`, `libgl1-mesa-dri`) and the windowing
  client libs: X11 (`libx11-6 libxext6 libxrandr2 libxinerama1 libxcursor1
  libxi6 libxkbcommon0`) and/or Wayland (`libwayland-client0 libwayland-cursor0
  libwayland-egl1 libxkbcommon0`, plus `libdecor-0-0` **and a plugin** such as
  `libdecor-0-plugin-1-gtk` — without a plugin, older libdecor can crash GLFW's
  Wayland init).
- **Audio** — `libpulse0` (PulseAudio) and/or `libasound2` (ALSA). miniaudio
  `dlopen`s these; with neither it prints `Audio backend: Null` and runs
  silently. On WSLg, `libpulse0` is enough — WSLg supplies the PulseAudio server
  (`PULSE_SERVER`).

WSLg notes: for reliable on-screen presentation force software GL —
`LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe`. For host-GPU rendering use
`GALLIUM_DRIVER=d3d12` (the RTX/iGPU via D3D12; needs `/dev/dxg` +
`/usr/lib/wsl/lib`, present in WSL2) — it renders correctly, though the WSLg
window may not always present d3d12 frames. The two usable GL implementations on
WSL are **d3d12** (hardware, GL 4.6) and **llvmpipe** (software, GL 4.5); zink and
the native-GPU DRI drivers don't apply (no Vulkan ICD / DRM node). Package names
vary by distro release (e.g. `libasound2t64` on newer Ubuntu). The selected
windowing platform and GL renderer are logged at startup (`GLFW platform:`,
`GL_RENDERER:`).

#### Linux platform-source layout

Platform code comes from two trees (see the `elseif(UNIX)` blocks in the root
`CMakeLists.txt`):

- **`Siv3D-Platform/macOS_Linux/`** — code shared with the macOS port (DateTime,
  EnvironmentVariable, BinaryFile*, the shared `FileSystem` subset, GLFW input).
- **`Siv3D-Platform/Linux/`** — **Linux-only** implementations:
  - `Time/` — `clock_gettime(CLOCK_MONOTONIC_RAW)`; `UTCOffsetMinutes` via
    `localtime_r().tm_gmtoff`.
  - `FileSystem/` — POSIX + `std::filesystem` + XDG dirs (`user-dirs.dirs`) +
    `/proc/self/exe` + a freedesktop trash impl.
  - `UserInfo/`, `Resource/`, `FreestandingMessageBox/` (→ `std::cerr`),
    `System/` (`CSystem` + `OpenInBrowser()` via `fork`+`execlp("xdg-open")`).
  - `Window/` + `GLFW/` — `CWindow` (GLFW, OpenGL 4.1 core, 4× MSAA framebuffer);
    logs the selected GLFW platform (X11/Wayland). `Cursor/` — `CCursor`.
  - `Renderer/GL4/` — `CRenderer_GL4` (clear/present, MSAA, screenshot capture via
    a resolve-FBO) + `CRenderer2D_GL4`: selects engine shaders from
    `CEngineShader_GL4` (or a custom shader) and binds them through a GL program
    pipeline (`CShader_GL4`, separable programs) with `ConstantBuffer_GL4` UBOs;
    a draw-command batch over a streaming VBO/IBO reusing `Vertex2DBuilder`.
    `CTexture_GL4` uploads RGBA8 via `AssetHandleManager`.
  - `Siv3DMain.cpp` — entry point (`main` → chdir to exe dir → engine init →
    user `Main`).
  - Device services (Clipboard, DragDrop, MediaTranscoder, NativeShare,
    Notifications, Pentablet, TextToSpeech) — no-op stubs.

#### Status

**Works:** window + GL 4.1 core context, the update loop, input, and the full 2D
pipeline through the registry+UBO shader path — shapes/gradients, blending,
transforms, textures/sprites + emoji, MSDF text (incl. CJK) with SDF
outline/shadow/glow, all six fill patterns, shadows, dashed/dotted lines,
quad-warp, custom user shaders (incl. the `GLSL(...)` factories), the full 2D
render state, screenshots, and 4× MSAA edges.

**Remaining `TODO(linux)`:** render-to-texture / scene-letterbox (MSAA currently
lives on the window framebuffer; move it to an explicit scene buffer when this
lands), and the device-service stubs (clipboard / drag-drop / notifications / …).

The standalone `Siv3D-Test` exe gets its `Main()` from `Test/TestMain/TestMain.cpp`
(a subdirectory so the engine's recursive `Test/*.cpp` glob includes it but the
App's non-recursive `../Test/*.cpp` glob does not — avoiding a clash with
`App/Main.cpp`). `Siv3D-Test` stays gated off on Windows.

## Windows build notes (non-obvious bits)

The Windows build is verified green; these are the subtle spots to check first if
it ever breaks:

1. **doctest / `tests` feature & stale cache** — `option()` does NOT override an
   already-cached value. A `build/` dir first configured with
   `SIV3D_APP_BUILD_TESTS=OFF` won't have doctest installed and the test include
   fails. **Fix: fresh build dir** (delete `App/build/CMakeCache.txt`) or pass
   `-DSIV3D_APP_BUILD_TESTS=ON`.
2. **bc7enc-rdo** — `<cstdint>` force-included via `/FIcstdint`.
3. **pffft** (marton78) — `PFFFT_STATIC_DEFINE` set PUBLIC so the export macro
   doesn't resolve to `__declspec(dllimport)` on a static build.
4. **oscpack** — the registry port is `supports: !(windows & !static)`; fine for
   `x64-windows-static`, but it forecloses a Windows-**DLL** build. The static lib
   needs Winsock, so the engine links `ws2_32` + `winmm`.
5. **wintoast** — wired WIN32-only; carries no system libs, so add
   `runtimeobject.lib` etc. to the WIN32 link block if a link error names it.
6. **mimalloc** — `SivMemory.cpp` includes `<mimalloc-new-delete.h>` (Windows-only,
   `SIV3D_USE_MIMALLOC=1`).
7. **`.patch` line endings** — `.gitattributes` forces `*.patch text eol=lf` so
   autocrlf doesn't corrupt `git apply` in overlay ports. Keep it.
8. **PCH** — `pch.h` via `target_precompile_headers`; the double-include `.ipp`
   issue was GCC-only.

## Legacy auto-link removal — Windows impact

`Siv3D/lib/` (prebuilt libs) and the `#pragma comment(lib, …)` block in
`Siv3D/include/Siv3D/Windows/Libraries.hpp` were **removed** — those libs now come
from vcpkg imported targets. `Libraries.hpp` auto-links ONLY Windows SDK system
libs now. If a link error names a third-party symbol (boost/freetype/harfbuzz/
png/jpeg/opencv/zlib/mimalloc), a vcpkg target is missing from a
`target_link_libraries`, not an autolink pragma.

## Still intentionally vendored

Kept in-tree because they're not in any registry, are Siv3D-authored derivatives,
or are tiny header-only utilities tightly coupled to the engine:

- `Siv3D/src/ThirdParty/boost/geometry/extensions/algorithms/dissolve.hpp` — a
  Boost.Geometry extension NOT in vcpkg, tied to the `boost-geometry` 1.88 pin.
- **metal-cpp** (`Foundation`/`Metal`/`QuartzCore`, macOS), **Wintab** (Wacom SDK
  headers), **RFC1321** (public-domain MD5 wrapper), **miniutf** (a Siv3D
  `s3d::detail` adaptation of Dropbox's decoder). `miniaudio` is just the
  `MINIAUDIO_IMPLEMENTATION` TU; the library/header come from vcpkg.

Recently moved *out* of the tree to vcpkg: **oscpack** (registry port) and the
header-only **Obfuscate / Xoshiro-cpp / EnumBitmask / libcpprime** (overlay-ports).

## macOS (not yet complete)

`App/CMakeLists.txt` builds a `.app` bundle (Info.plist + icon, frameworks +
metallib wired best-effort) but the whole thing is **UNVERIFIED** (no Mac to build
on) and asset/dylib staging into the bundle may need adjustment — see the `NOTE:`
in the `elseif(APPLE)` block.

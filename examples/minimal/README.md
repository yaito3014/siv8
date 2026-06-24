# Minimal Siv3D consumer

The smallest possible project that consumes an **installed/exported Siv3D SDK**
via `find_package(Siv3D)`. Copy this `CMakeLists.txt` as the starting point for
your own app — `App/` in this repo builds the engine in-tree for development,
but a downstream consumer uses the SDK exactly like this.

## Build

1. Build **and install** the engine to a prefix. The fat `Siv3D::Siv3D` lib is
   built only for a **top-level engine build** (`SIV3D_FAT_LIB` defaults to
   `PROJECT_IS_TOP_LEVEL`), so configure the **repo root** as the project — not
   `App/`, whose `add_subdirectory` build skips the fat lib for fast iteration:

   ```bash
   cmake -S <repo-root> -B engine-build \
     -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
     -DVCPKG_TARGET_TRIPLET=x64-linux
   cmake --build engine-build
   cmake --install engine-build --prefix /path/to/siv3d-sdk
   ```

2. Configure this example with the vcpkg toolchain, the matching triplet, and the
   SDK prefix on `CMAKE_PREFIX_PATH`:

   ```bash
   cmake -B build \
     -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
     -DVCPKG_TARGET_TRIPLET=x64-linux \
     -DCMAKE_PREFIX_PATH=/path/to/siv3d-sdk
   cmake --build build
   ```

   On Windows use `-DVCPKG_TARGET_TRIPLET=x64-windows-static`.

The SDK is a static-CRT build, so consumers must use the static triplet and
matching MSVC runtime (the `CMakeLists.txt` sets sane defaults for that).

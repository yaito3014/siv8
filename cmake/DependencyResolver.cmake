# =============================================================================
# DepResolver.cmake
#
# System-first dependency resolution with a one-shot vcpkg fallback.
#
#   Phase 1  Probe every declared dependency on the system (find_package QUIET)
#   Phase 2  Generate ONE vcpkg.json for the misses, run `vcpkg install` ONCE
#   Phase 3  Resolve the misses for real (find_package REQUIRED)
#
# Usage:
#   include(cmake/DepResolver.cmake)
#
#   dep_declare(fmt    CONFIG VERSION 10)
#   dep_declare(spdlog CONFIG)
#   dep_declare(CURL   PORT curl FEATURES ssl)
#   dep_declare(GTest  CONFIG PORT gtest)
#   dep_declare(Boost  CONFIG VERSION 1.83
#               COMPONENTS  filesystem program_options unit_test_framework
#               HEADER_ONLY asio uuid)
#
#   dep_resolve(BASELINE <vcpkg-commit-sha>)   # BASELINE/TRIPLET optional
#
# dep_declare(<FindPackageName>
#   [CONFIG]                  use config mode for find_package (recommended)
#   [VERSION <min>]           minimum version; also emitted as "version>="
#                             into the manifest when BASELINE is given
#   [PORT <vcpkg-port>]       override name->port mapping (default: tolower)
#   [COMPONENTS <c>...]       find_package COMPONENTS; for Boost each maps
#                             to a boost-<c> port
#   [FEATURES <f>...]         vcpkg port features (single-port deps only)
#   [NO_DEFAULT_FEATURES]     emit "default-features": false (single-port only)
#   [HEADER_ONLY <lib>...]    Boost only: header-only libs to install from
#                             vcpkg (and verify on system) but NOT passed
#                             as find_package COMPONENTS
# )
#
# Requirements:
#   - CMake >= 3.21
#   - `vcpkg` on PATH, or -DVCPKG_EXECUTABLE=/path/to/vcpkg
#   - Do NOT combine with -DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake
#
# Notes:
#   - Call dep_resolve() AFTER project() and from the top-level CMakeLists.
#   - Probe results are cached (<pkg>_DIR); to re-detect a newly installed
#     system package, wipe the build dir or the relevant cache entry.
# =============================================================================

include_guard(GLOBAL)
cmake_minimum_required(VERSION 3.21)

# -----------------------------------------------------------------------------
# Declaration
# -----------------------------------------------------------------------------
function(dep_declare name)
  cmake_parse_arguments(D "CONFIG;NO_DEFAULT_FEATURES" "VERSION;PORT"
                        "COMPONENTS;FEATURES;HEADER_ONLY" ${ARGN})
  if(D_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "dep_declare(${name}): unknown arguments: ${D_UNPARSED_ARGUMENTS}")
  endif()
  if(D_HEADER_ONLY AND NOT name STREQUAL "Boost")
    message(FATAL_ERROR "dep_declare(${name}): HEADER_ONLY is only supported for Boost")
  endif()

  get_property(_names GLOBAL PROPERTY _DEP_NAMES)
  if("${name}" IN_LIST _names)
    message(FATAL_ERROR "dep_declare(${name}): declared twice")
  endif()

  if(NOT D_PORT)
    string(TOLOWER "${name}" D_PORT)
  endif()

  set_property(GLOBAL APPEND PROPERTY _DEP_NAMES "${name}")
  set_property(GLOBAL PROPERTY _DEP_${name}_CONFIG      "${D_CONFIG}")
  set_property(GLOBAL PROPERTY _DEP_${name}_VERSION     "${D_VERSION}")
  set_property(GLOBAL PROPERTY _DEP_${name}_PORT        "${D_PORT}")
  set_property(GLOBAL PROPERTY _DEP_${name}_COMPONENTS  "${D_COMPONENTS}")
  set_property(GLOBAL PROPERTY _DEP_${name}_FEATURES    "${D_FEATURES}")
  set_property(GLOBAL PROPERTY _DEP_${name}_HEADER_ONLY "${D_HEADER_ONLY}")
  set_property(GLOBAL PROPERTY _DEP_${name}_NO_DEFAULT_FEATURES "${D_NO_DEFAULT_FEATURES}")
endfunction()

# -----------------------------------------------------------------------------
# Internal helpers
# -----------------------------------------------------------------------------

# find_package argument list (version / CONFIG / COMPONENTS) for a declared dep
function(_dep_find_args name out)
  get_property(_v   GLOBAL PROPERTY _DEP_${name}_VERSION)
  get_property(_cfg GLOBAL PROPERTY _DEP_${name}_CONFIG)
  get_property(_cmp GLOBAL PROPERTY _DEP_${name}_COMPONENTS)
  set(_args "")
  if(_v)
    list(APPEND _args ${_v})
  endif()
  if(_cfg)
    list(APPEND _args CONFIG)
  endif()
  if(_cmp)
    list(APPEND _args COMPONENTS ${_cmp})
  endif()
  set(${out} "${_args}" PARENT_SCOPE)
endfunction()

# Boost find_package component -> vcpkg port name
function(_dep_boost_port comp out)
  # Components whose port name does not follow the tolower + _ -> - rule
  set(_exc_unit_test_framework test)
  set(_exc_prg_exec_monitor    test)
  set(_exc_test_exec_monitor   test)
  if(DEFINED _exc_${comp})
    set(${out} "boost-${_exc_${comp}}" PARENT_SCOPE)
  else()
    string(TOLOWER "${comp}" _c)
    string(REPLACE "_" "-" _c "${_c}")
    set(${out} "boost-${_c}" PARENT_SCOPE)
  endif()
endfunction()

# Expand one declared dependency to its vcpkg port list
function(_dep_vcpkg_ports name out)
  if(name STREQUAL "Boost")
    get_property(_cmp GLOBAL PROPERTY _DEP_${name}_COMPONENTS)
    get_property(_ho  GLOBAL PROPERTY _DEP_${name}_HEADER_ONLY)
    set(_ports "")
    foreach(_c IN LISTS _cmp _ho)
      _dep_boost_port(${_c} _p)
      list(APPEND _ports ${_p})
    endforeach()
    if(NOT _ports)
      message(WARNING "Boost declared without COMPONENTS/HEADER_ONLY; "
                      "falling back to the full 'boost' metaport (large!)")
      set(_ports boost)
    endif()
    list(REMOVE_DUPLICATES _ports)
  else()
    get_property(_ports GLOBAL PROPERTY _DEP_${name}_PORT)
  endif()
  set(${out} "${_ports}" PARENT_SCOPE)
endfunction()

# Check that a header-only Boost lib is present in the found include dirs.
# Some libs have no boost/<lib>.hpp umbrella (e.g. multiprecision), only a
# boost/<lib>/ directory — accept that too.
function(_dep_boost_has_header lib incdirs out)
  set(${out} FALSE PARENT_SCOPE)
  foreach(_d IN LISTS incdirs)
    if(EXISTS "${_d}/boost/${lib}.hpp" OR IS_DIRECTORY "${_d}/boost/${lib}")
      set(${out} TRUE PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

# Write vcpkg.json for the missing deps (ARGN = missing dep names)
function(_dep_write_manifest out_path baseline)
  set(_json "{\n")
  if(baseline)
    string(APPEND _json "  \"builtin-baseline\": \"${baseline}\",\n")
  endif()
  string(APPEND _json "  \"dependencies\": [\n")
  set(_sep "")
  foreach(_dep IN LISTS ARGN)
    _dep_vcpkg_ports(${_dep} _ports)
    get_property(_feats GLOBAL PROPERTY _DEP_${_dep}_FEATURES)
    get_property(_ver   GLOBAL PROPERTY _DEP_${_dep}_VERSION)
    get_property(_nodef GLOBAL PROPERTY _DEP_${_dep}_NO_DEFAULT_FEATURES)
    list(LENGTH _ports _nports)
    foreach(_port IN LISTS _ports)
      # FEATURES/NO_DEFAULT_FEATURES only make sense when one dep == one port
      set(_use_feats "")
      set(_use_nodef "")
      if(_nports EQUAL 1)
        set(_use_feats "${_feats}")
        set(_use_nodef "${_nodef}")
      endif()
      if(_use_feats OR _use_nodef OR (_ver AND baseline))
        set(_entry "    { \"name\": \"${_port}\"")
        if(_use_nodef)
          string(APPEND _entry ", \"default-features\": false")
        endif()
        if(_use_feats)
          list(JOIN _use_feats "\", \"" _fj)
          string(APPEND _entry ", \"features\": [\"${_fj}\"]")
        endif()
        if(_ver AND baseline)
          string(APPEND _entry ", \"version>=\": \"${_ver}\"")
        endif()
        string(APPEND _entry " }")
      else()
        set(_entry "    \"${_port}\"")
      endif()
      string(APPEND _json "${_sep}${_entry}")
      set(_sep ",\n")
    endforeach()
  endforeach()
  string(APPEND _json "\n  ]\n}\n")
  file(WRITE "${out_path}" "${_json}")
endfunction()

# Activate a vcpkg install tree: prefix for find_package, plus the toolchain
# context variables some port configs (e.g. harfbuzz) read. Must be a macro so
# everything lands in the caller's directory scope.
macro(_dep_use_prefix prefix triplet)
  list(PREPEND CMAKE_PREFIX_PATH "${prefix}")
  if(WIN32)
    set(ENV{CMAKE_PREFIX_PATH} "${prefix};$ENV{CMAKE_PREFIX_PATH}")
  else()
    set(ENV{CMAKE_PREFIX_PATH} "${prefix}:$ENV{CMAKE_PREFIX_PATH}")
  endif()
  set(VCPKG_INSTALLED_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed")
  set(_VCPKG_INSTALLED_DIR "${VCPKG_INSTALLED_DIR}")
  if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "${triplet}")
  endif()
endmacro()

# Best-effort default triplet (override with -DVCPKG_TARGET_TRIPLET=...)
function(_dep_default_triplet out)
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _proc)
  if(_proc MATCHES "arm64|aarch64")
    set(_arch arm64)
  else()
    set(_arch x64)
  endif()
  if(WIN32)
    set(${out} "${_arch}-windows" PARENT_SCOPE)
  elseif(APPLE)
    set(${out} "${_arch}-osx" PARENT_SCOPE)
  else()
    set(${out} "${_arch}-linux" PARENT_SCOPE)
  endif()
endfunction()

# -----------------------------------------------------------------------------
# Resolution
#
# Must be a macro: find_package result variables and imported targets need to
# land in the caller's directory scope.
# -----------------------------------------------------------------------------
macro(dep_resolve)
  cmake_parse_arguments(_DR "" "TRIPLET;BASELINE" "" ${ARGN})

  get_property(_dr_names GLOBAL PROPERTY _DEP_NAMES)
  if(NOT _dr_names)
    message(WARNING "dep_resolve() called with no dep_declare()d dependencies")
  else()
    set(_dr_missing "")
    set(_dr_system "")

    if(_DR_TRIPLET)
      set(_dr_triplet "${_DR_TRIPLET}")
    elseif(DEFINED VCPKG_TARGET_TRIPLET)
      set(_dr_triplet "${VCPKG_TARGET_TRIPLET}")
    else()
      _dep_default_triplet(_dr_triplet)
    endif()

    # A vcpkg tree from a previous configure must be active BEFORE probing:
    # cached <pkg>_DIR entries point into it, and the toolchain context
    # variables are needed on every run, not only the one that installs.
    set(_dr_prefix "${CMAKE_BINARY_DIR}/vcpkg_installed/${_dr_triplet}")
    set(_dr_prefix_active FALSE)
    if(EXISTS "${_dr_prefix}")
      _dep_use_prefix("${_dr_prefix}" "${_dr_triplet}")
      set(_dr_prefix_active TRUE)
    endif()

    # ---- Phase 1: probe the system ------------------------------------
    foreach(_dr_dep IN LISTS _dr_names)
      _dep_find_args(${_dr_dep} _dr_fargs)
      find_package(${_dr_dep} ${_dr_fargs} QUIET)
      set(_dr_ok "${${_dr_dep}_FOUND}")

      # Boost: additionally verify declared header-only libs exist
      if(_dr_ok AND _dr_dep STREQUAL "Boost")
        get_property(_dr_ho GLOBAL PROPERTY _DEP_Boost_HEADER_ONLY)
        foreach(_dr_h IN LISTS _dr_ho)
          _dep_boost_has_header(${_dr_h} "${Boost_INCLUDE_DIRS}" _dr_hok)
          if(NOT _dr_hok)
            message(STATUS "DepResolver: system Boost lacks header-only "
                           "'${_dr_h}' -> demoting Boost to vcpkg")
            set(_dr_ok FALSE)
            # The probe cached a valid Boost_DIR pointing at the system;
            # clear it so Phase 3 re-searches and picks the vcpkg copy.
            unset(Boost_DIR CACHE)
          endif()
        endforeach()
      endif()

      if(_dr_ok)
        list(APPEND _dr_system ${_dr_dep})
      else()
        list(APPEND _dr_missing ${_dr_dep})
      endif()
    endforeach()

    # ---- Phase 2: one manifest, one vcpkg install ----------------------
    if(_dr_missing)
      find_program(VCPKG_EXECUTABLE vcpkg)
      if(NOT VCPKG_EXECUTABLE)
        message(FATAL_ERROR
          "DepResolver: packages missing from system (${_dr_missing}) but "
          "vcpkg was not found. Install vcpkg and add it to PATH, or pass "
          "-DVCPKG_EXECUTABLE=/path/to/vcpkg, or install the packages "
          "system-wide.")
      endif()

      set(_dr_mandir "${CMAKE_BINARY_DIR}/vcpkg_manifest")
      file(MAKE_DIRECTORY "${_dr_mandir}")
      _dep_write_manifest("${_dr_mandir}/vcpkg.json" "${_DR_BASELINE}" ${_dr_missing})

      message(STATUS "DepResolver: from system : ${_dr_system}")
      message(STATUS "DepResolver: from vcpkg  : ${_dr_missing}")
      message(STATUS "DepResolver: running vcpkg install (manifest: "
                     "${_dr_mandir}/vcpkg.json, triplet: ${_dr_triplet}) ...")

      execute_process(
        COMMAND "${VCPKG_EXECUTABLE}" install
                --triplet=${_dr_triplet}
                --x-install-root=${CMAKE_BINARY_DIR}/vcpkg_installed
        WORKING_DIRECTORY "${_dr_mandir}"
        RESULT_VARIABLE _dr_rc)
      if(NOT _dr_rc EQUAL 0)
        message(FATAL_ERROR "DepResolver: vcpkg install failed (rc=${_dr_rc})")
      endif()

      if(NOT EXISTS "${_dr_prefix}")
        message(FATAL_ERROR
          "DepResolver: expected install tree '${_dr_prefix}' does not exist; "
          "pass the correct triplet via dep_resolve(TRIPLET ...) or "
          "-DVCPKG_TARGET_TRIPLET=...")
      endif()

      # Prepend so vcpkg wins over an inadequate (e.g. demoted) system copy.
      if(NOT _dr_prefix_active)
        _dep_use_prefix("${_dr_prefix}" "${_dr_triplet}")
        set(_dr_prefix_active TRUE)
      endif()

      # ---- Phase 3: resolve the misses for real ------------------------
      foreach(_dr_dep IN LISTS _dr_missing)
        _dep_find_args(${_dr_dep} _dr_fargs)
        find_package(${_dr_dep} ${_dr_fargs} REQUIRED)
      endforeach()
    else()
      message(STATUS "DepResolver: all dependencies satisfied by system: "
                     "${_dr_system}")
    endif()

    unset(_dr_prefix)
    unset(_dr_prefix_active)
    unset(_dr_triplet)
    unset(_dr_missing)
    unset(_dr_system)
    unset(_dr_names)
  endif()
endmacro()


# MSVC C runtime selection. Siv3D and all of its vcpkg dependencies are static
# libraries, and MSVC refuses to link objects built against different C
# runtimes (LNK2038), so the runtime is decided by the vcpkg triplet that built
# the dependencies:
#   <arch>-windows-static[-release]     static CRT (/MT, /MTd)
#   <arch>-windows-static-md[-release]  DLL CRT    (/MD, /MDd)

# Sets <out_var> to "DLL" or "static" for a CMAKE_MSVC_RUNTIME_LIBRARY value.
function(siv3d_msvc_runtime_kind runtime out_var)
    if(runtime MATCHES "DLL")
        set(${out_var} "DLL" PARENT_SCOPE)
    else()
        set(${out_var} "static" PARENT_SCOPE)
    endif()
endfunction()

# Sets <out_var> to the CMAKE_MSVC_RUNTIME_LIBRARY value matching <triplet>, or
# to an empty string when the triplet name does not tell.
function(siv3d_msvc_runtime_for_triplet triplet out_var)
    if(triplet MATCHES "-windows-static-md(-release)?$")
        set(${out_var} "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" PARENT_SCOPE)
    elseif(triplet MATCHES "-windows-static(-release)?$")
        set(${out_var} "MultiThreaded$<$<CONFIG:Debug>:Debug>" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

# Fails unless <runtime> uses the same kind of C runtime (static or DLL) as
# CMAKE_MSVC_RUNTIME_LIBRARY. <source> names where <runtime> comes from.
function(siv3d_check_msvc_runtime runtime source)
    siv3d_msvc_runtime_kind("${CMAKE_MSVC_RUNTIME_LIBRARY}" _selected)
    siv3d_msvc_runtime_kind("${runtime}" _required)
    if(NOT _selected STREQUAL _required)
        message(FATAL_ERROR
            "CMAKE_MSVC_RUNTIME_LIBRARY is '${CMAKE_MSVC_RUNTIME_LIBRARY}' (${_selected} C runtime), "
            "but ${source} uses the ${_required} C runtime. MSVC cannot link the two.")
    endif()
endfunction()

# Sets CMAKE_MSVC_RUNTIME_LIBRARY in the caller's scope from VCPKG_TARGET_TRIPLET
# unless it is already set, in which case it must agree with the triplet. A
# triplet whose name does not tell requires CMAKE_MSVC_RUNTIME_LIBRARY to be set
# to match the triplet's VCPKG_CRT_LINKAGE.
function(siv3d_select_msvc_runtime)
    siv3d_msvc_runtime_for_triplet("${VCPKG_TARGET_TRIPLET}" _triplet_runtime)
    if(NOT CMAKE_MSVC_RUNTIME_LIBRARY)
        if(_triplet_runtime STREQUAL "")
            message(FATAL_ERROR
                "Cannot tell the C runtime of vcpkg triplet '${VCPKG_TARGET_TRIPLET}'. "
                "Use <arch>-windows-static or <arch>-windows-static-md, or set "
                "CMAKE_MSVC_RUNTIME_LIBRARY to match the triplet's VCPKG_CRT_LINKAGE.")
        endif()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "${_triplet_runtime}" PARENT_SCOPE)
    elseif(NOT _triplet_runtime STREQUAL "")
        siv3d_check_msvc_runtime("${_triplet_runtime}" "vcpkg triplet '${VCPKG_TARGET_TRIPLET}'")
    endif()
endfunction()

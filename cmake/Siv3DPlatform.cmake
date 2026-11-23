# The Siv3D/src/Siv3D-Platform directories the current platform builds, in
# the engine's include order. Shared by the engine and by App, whose tests
# include engine internals.
function(siv3d_platform_dirs out_var siv3d_src)
    if(WIN32)
        set(_dirs WindowsDesktop)
    elseif(APPLE)
        set(_dirs macOS macOS_Linux)
    elseif(UNIX)
        set(_dirs Linux macOS_Linux)
    endif()
    list(TRANSFORM _dirs PREPEND "${siv3d_src}/Siv3D-Platform/")
    set(${out_var} "${_dirs}" PARENT_SCOPE)
endfunction()

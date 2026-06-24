# Merge SIV3D_MAIN_LIB + every .a in SIV3D_DEP_LIBDIR into one static archive
# (SIV3D_FATLIB_OUT), content-deduped: identical object members are kept once,
# distinct same-named ones are renamed so no symbols are lost. GNU/macOS ar.
# Run with -P; vars: SIV3D_MAIN_LIB/DEP_LIBDIR/FATLIB_OUT/AR/RANLIB/WORK.

if(NOT SIV3D_AR OR NOT SIV3D_MAIN_LIB OR NOT SIV3D_DEP_LIBDIR OR NOT SIV3D_FATLIB_OUT OR NOT SIV3D_WORK)
    message(FATAL_ERROR "Siv3DFatLib: missing required -D arguments")
endif()

file(REMOVE_RECURSE "${SIV3D_WORK}")
file(MAKE_DIRECTORY "${SIV3D_WORK}/ex")

file(GLOB _deps "${SIV3D_DEP_LIBDIR}/*.a")
set(_inputs "${SIV3D_MAIN_LIB}" ${_deps})

# 1) extract each input archive into its own dir (avoid cross-archive name clobber).
#    An archive can also contain members with DUPLICATE names within itself (e.g.
#    opencv_core ships two parallel.cpp.o from core/src/parallel.cpp and
#    core/src/parallel/parallel.cpp). A plain `ar x` extracts into a flat dir, so
#    the later member silently overwrites the earlier one and its symbols are lost
#    (the dropped parallel.cpp.o is the one defining cv::parallel_for_). Detect
#    duplicate member names and, only then, extract the Nth occurrence of each
#    (ar xN) to a uniquely-named file so nothing is lost; the SHA1 dedup below
#    still removes only byte-identical copies.
set(_idx 0)
foreach(_a IN LISTS _inputs)
    set(_d "${SIV3D_WORK}/ex/${_idx}")
    file(MAKE_DIRECTORY "${_d}")
    execute_process(COMMAND "${SIV3D_AR}" t "${_a}"
        OUTPUT_VARIABLE _members OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "Siv3DFatLib: ar t failed on ${_a}")
    endif()
    string(REPLACE "\n" ";" _members "${_members}")
    list(REMOVE_ITEM _members "")
    set(_uniqnames ${_members})
    list(REMOVE_DUPLICATES _uniqnames)
    list(LENGTH _members _nmembers)
    list(LENGTH _uniqnames _nuniq)
    if(_nmembers EQUAL _nuniq)
        # no duplicate member names: fast bulk extract
        execute_process(COMMAND "${SIV3D_AR}" x "${_a}" WORKING_DIRECTORY "${_d}" RESULT_VARIABLE _rc)
        if(NOT _rc EQUAL 0)
            message(FATAL_ERROR "Siv3DFatLib: ar x failed on ${_a}")
        endif()
    else()
        # duplicate member names: extract the Nth occurrence of each to a unique file
        set(_mno 0)
        foreach(_m IN LISTS _members)
            string(MAKE_C_IDENTIFIER "occ_${_m}" _occvar)
            if(NOT DEFINED ${_occvar})
                set(${_occvar} 0)
            endif()
            math(EXPR ${_occvar} "${${_occvar}} + 1")
            execute_process(COMMAND "${SIV3D_AR}" xN ${${_occvar}} "${_a}" "${_m}"
                WORKING_DIRECTORY "${_d}" RESULT_VARIABLE _rc)
            if(NOT _rc EQUAL 0)
                message(FATAL_ERROR "Siv3DFatLib: ar xN failed on ${_a} :: ${_m}")
            endif()
            get_filename_component(_bn "${_m}" NAME)
            file(RENAME "${_d}/${_bn}" "${_d}/${_mno}_${_bn}")
            math(EXPR _mno "${_mno} + 1")
        endforeach()
        foreach(_m IN LISTS _uniqnames)          # reset counters for the next archive
            string(MAKE_C_IDENTIFIER "occ_${_m}" _occvar)
            unset(${_occvar})
        endforeach()
    endif()
    math(EXPR _idx "${_idx} + 1")
endforeach()

# 2) content-dedup by SHA1 (variable-per-hash as an O(1) set; IN_LIST would be O(n^2)).
file(MAKE_DIRECTORY "${SIV3D_WORK}/uniq")
file(GLOB_RECURSE _objs "${SIV3D_WORK}/ex/*.o" "${SIV3D_WORK}/ex/*.obj")
set(_kept 0)
set(_objlist "")
foreach(_o IN LISTS _objs)
    file(SHA1 "${_o}" _h)
    if(NOT DEFINED _seen_${_h})
        set(_seen_${_h} 1)
        get_filename_component(_bn "${_o}" NAME)
        set(_u "${SIV3D_WORK}/uniq/${_kept}_${_bn}")
        file(RENAME "${_o}" "${_u}")
        list(APPEND _objlist "${_u}")
        math(EXPR _kept "${_kept} + 1")
    endif()
endforeach()
list(LENGTH _objs _total)
message(STATUS "Siv3DFatLib: ${_total} objects -> ${_kept} unique (dropped identical copies)")

# 3) archive in batches (the object list is far too long for one command line)
file(REMOVE "${SIV3D_FATLIB_OUT}")
set(_batch "")
set(_bn 0)
foreach(_o IN LISTS _objlist)
    list(APPEND _batch "${_o}")
    math(EXPR _bn "${_bn} + 1")
    if(_bn GREATER_EQUAL 400)
        execute_process(COMMAND "${SIV3D_AR}" qc "${SIV3D_FATLIB_OUT}" ${_batch} RESULT_VARIABLE _rc)
        if(NOT _rc EQUAL 0)
            message(FATAL_ERROR "Siv3DFatLib: ar qc failed")
        endif()
        set(_batch "")
        set(_bn 0)
    endif()
endforeach()
if(_batch)
    execute_process(COMMAND "${SIV3D_AR}" qc "${SIV3D_FATLIB_OUT}" ${_batch})
endif()
if(SIV3D_RANLIB)
    execute_process(COMMAND "${SIV3D_RANLIB}" "${SIV3D_FATLIB_OUT}")
endif()
message(STATUS "Siv3DFatLib: wrote ${SIV3D_FATLIB_OUT}")

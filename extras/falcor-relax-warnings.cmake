# falcor-relax-warnings.cmake - relax Falcor's warnings-as-errors WITHOUT
# editing any tracked Falcor file.
#
# extras/falcor.sh injects this file via
#   -DCMAKE_PROJECT_Falcor_INCLUDE=<this file>
# so CMake includes it right after Falcor's `project(Falcor ...)` call. Public
# Falcor hardcodes a blanket `/WX` (MSVC warnings-as-errors) on the `Falcor`
# target in Source/Falcor/CMakeLists.txt and exposes no CMake toggle for it, so
# a newer Slang's deprecation warning (e.g. a `[[deprecated]]` reflection
# method) would fail Falcor's build. This strips only the blanket `/WX` (and a
# defensive `-Werror`) from the target's COMPILE_OPTIONS once the target exists;
# the warnings still print, so Slang version-skew stays visible, and Falcor's
# deliberate per-warning errors (e.g. `/we4263`) are preserved. Pass `--werror`
# to falcor.sh to skip this include entirely and keep Falcor strict.
function(_falcor_sh_relax_warnings_as_errors)
    if(NOT TARGET Falcor)
        message(
            WARNING
            "falcor.sh: Falcor target not found; warnings-as-errors left unchanged."
        )
        return()
    endif()
    get_target_property(_opts Falcor COMPILE_OPTIONS)
    if(_opts)
        list(FILTER _opts EXCLUDE REGEX "^(/WX|-Werror)$")
        set_target_properties(Falcor PROPERTIES COMPILE_OPTIONS "${_opts}")
        message(
            STATUS
            "falcor.sh: relaxed Falcor warnings-as-errors (stripped /WX, -Werror; per-warning errors kept)."
        )
    endif()
endfunction()

# Defer to the end of this (Falcor top-level) directory's processing, which runs
# after add_subdirectory(Source/Falcor) has created the Falcor target. A bare
# DEFER (no DIRECTORY) is used because the Source/Falcor directory has not been
# added yet at include time.
cmake_language(DEFER CALL _falcor_sh_relax_warnings_as_errors)

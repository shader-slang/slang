# Probe for secure CRT functions that may not be available on all platforms, 
# and define macros to indicate their presence.
include(CheckSourceCompiles)

# Tests a symbol by attempting to call it with dummy arguments, which is necessary 
# to detect inline functions that may not have an address.
function(slang_probe_symbol_call macro call_expr header)
    set(test_source "#include <${header}>\nint main() { ${call_expr}; return 0; }")
    check_source_compiles(CXX "${test_source}" ${macro})
    if(${macro})
        list(APPEND SLANG_SECURE_CRT_DEFINES ${macro})
        set(SLANG_SECURE_CRT_DEFINES ${SLANG_SECURE_CRT_DEFINES} PARENT_SCOPE)
    endif()
endfunction()

slang_probe_symbol_call(HAVE_MEMCPY_S
    "char a[1]={0}; memcpy_s(a, 1, a, 1)"
    "string.h")
slang_probe_symbol_call(HAVE_FOPEN_S
    "FILE* f=nullptr; fopen_s(&f,\"x\",\"r\")"
    "stdio.h")
slang_probe_symbol_call(HAVE_FREAD_S
    "char b[1]={0}; FILE* f=nullptr; fread_s(b,1,1,1,f)"
    "stdio.h")
slang_probe_symbol_call(HAVE_WCSNLEN_S
    "wcsnlen_s(L\"x\", 1)"
    "wchar.h")
slang_probe_symbol_call(HAVE_STRNLEN_S
    "strnlen_s(\"x\", 1)"
    "string.h")
slang_probe_symbol_call(HAVE_SPRINTF_S
    "char b[1]; sprintf_s(b, 1, \"%s\", \"x\")"
    "stdio.h")
slang_probe_symbol_call(HAVE_SWPRINTF_S
    "wchar_t b[1]; swprintf_s(b, 1, L\"%ls\", L\"x\")"
    "wchar.h")
slang_probe_symbol_call(HAVE_WCSCPY_S
    "wchar_t b[1]; wcscpy_s(b, 1, L\"x\")"
    "wchar.h")
slang_probe_symbol_call(HAVE_STRCPY_S
    "char b[1]; strcpy_s(b, 1, \"x\")"
    "string.h")
slang_probe_symbol_call(HAVE_WCSNCPY_S
    "wchar_t b[1]; wcsncpy_s(b, 1, L\"x\", 1)"
    "wchar.h")
slang_probe_symbol_call(HAVE_STRNCPY_S
    "char b[1]; strncpy_s(b, 1, \"x\", 1)"
    "string.h")

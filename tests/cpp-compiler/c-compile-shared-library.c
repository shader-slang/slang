// TEST(smoke,shared-library):CPP_COMPILER_SHARED_LIBRARY:

// This is the only test on the CPU tier that in-process LoadLibrary's a
// downstream-compiler-built DLL, making it the sole regression guard for
// native Windows ARM64 host-toolchain selection (calcExecuteCompilerArgs
// in slang-win-visual-studio-util.cpp): a wrong-architecture DLL fails to
// load at LoadLibrary time, not link time, so no other cpp-compiler/
// cpu-program test in this directory reproduces that failure mode. Do not
// add this test to a skip-list/expected-failure file for an arm64 tier
// without replacing this coverage.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#define DLL_EXPORT __declspec(dllexport)
#else
// #   define DLL_EXPORT
#define DLL_EXPORT __attribute__((dllexport)) __attribute__((__visibility__("default")))
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C DLL_EXPORT int test(int intValue, const char* textValue, char* outTextValue)
{
    strcpy(outTextValue, textValue);
    return intValue;
}

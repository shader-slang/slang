// TEST(smoke,shared-library):CPP_COMPILER_SHARED_LIBRARY:

// One of several CPU-tier tests (alongside cpp-compile-shared-library.cpp
// and the -cpu host-callable COMPARE_COMPUTE path) that in-process
// LoadLibrary a downstream-compiler-built DLL, and is therefore a
// regression guard for native Windows ARM64 host-toolchain selection
// (calcExecuteCompilerArgs in slang-win-visual-studio-util.cpp): a
// wrong-architecture DLL fails at LoadLibrary time, not link time.

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

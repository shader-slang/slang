//TEST_DISABLED(smoke):CPP_COMPILER_SHARED_LIBRARY: 
// Disabled for now because breaks on some CI test targets

//#include <slang.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
using namespace std;
    
#if defined(_MSC_VER)
#   define DLL_EXPORT __declspec(dllexport)
#else
#   define DLL_EXPORT __attribute__((__visibility__("default")))
#endif    
    
extern "C" DLL_EXPORT int test(int intValue, const char* textValue, char* outTextValue)
{
    strcpy(outTextValue, textValue);
    return intValue;
}


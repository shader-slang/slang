//TEST(smoke):CPP_COMPILER_SHARED_LIBRARY: 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>   
   
#if defined(_MSC_VER)
#   define DLL_EXPORT __declspec(dllexport)
#else 
//#   define DLL_EXPORT 
#   define DLL_EXPORT __attribute__ ((dllexport)) __attribute__((__visibility__("default")))
#endif    

#ifdef __cplusplus    
#define EXTERN_C extern "C"
#else
#define EXTERN_C 
#endif    

extern int symbolNotFound;
    
EXTERN_C DLL_EXPORT int test(int intValue, const char* textValue, char* outTextValue)
{
    strcpy(outTextValue, textValue);
    return intValue + symbolNotFound;
}
   

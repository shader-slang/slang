// Diagnostic messages related to compile-time for

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NONE):        -std 2018 -DDEPRECATION=0 -DREMOVAL=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_DEPRECATION): -std 2025 -DDEPRECATION=1 -DREMOVAL=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_DEPRECATION): -std 2026 -DDEPRECATION=1 -DREMOVAL=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_REMOVAL):     -std 202c -DDEPRECATION=0 -DREMOVAL=1

RWStructuredBuffer<int> output;

void test()
{
    int sum = 0;

#if !DEPRECATION && !REMOVAL
    $for(i in Range(0,5))
#endif

#if DEPRECATION
    $for(i in Range(0,5))
/*CHECK_DEPRECATION:
     ^^^ compile-time for is deprecated
     ^^^ compile-time for is deprecated and will be removed in Slang 202c. Use '[ForceUnroll] for' instead.
*/
#endif

#if REMOVAL
    $for(i in Range(0,5))
/*CHECK_REMOVAL:
     ^^^ compile-time for has been removed
     ^^^ compile-time for has been removed from the language. Use '[ForceUnroll] for' instead.
*/
#endif

    // the $for body
    {
        sum += i;
    }

    output[0] = sum;
}

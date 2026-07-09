//DIAGNOSTIC_TEST:SIMPLE(diag=legacy-diag):  -stage compute -entry computeMain -std legacy -DCASE=0
//DIAGNOSTIC_TEST:SIMPLE(diag=warn-diag):    -stage compute -entry computeMain -std 2025   -DCASE=1
//DIAGNOSTIC_TEST:SIMPLE(diag=err-diag):     -stage compute -entry computeMain -std 2026   -DCASE=2
//
// See https://github.com/shader-slang/slang/issues/10306

#if (CASE == 0)

typedef volatile float VolatileFloat;
//legacy-diag:   ^^^^^ invalid 'volatile' usage
//legacy-diag:   ^^^^^ cannot use 'volatile' as a type modifier

#elif (CASE == 1)

typedef volatile float VolatileFloat;
//warn-diag:     ^^^^^ use of deprecated modifier
//warn-diag:     ^^^^^ modifier 'volatile' has been deprecated: to be removed in Slang 2026. Suggested replacement: Atomic<T>
//warn-diag:     ^^^^^ invalid 'volatile' usage
//warn-diag:     ^^^^^ cannot use 'volatile' as a type modifier

#else

typedef volatile float VolatileFloat;
//err-diag:      ^^^^^ use of removed modifier
//err-diag:      ^^^^^ modifier 'volatile' has been removed from the language. Suggested replacement: Atomic<T>

#endif

[numthreads(1,1,1)]
void computeMain()
{
}

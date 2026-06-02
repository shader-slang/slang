//DIAGNOSTIC_TEST:SIMPLE(diag=no-diag):   -DERR=0
//DIAGNOSTIC_TEST:SIMPLE(diag=warn-diag): -std 2025 -DERR=0
//DIAGNOSTIC_TEST:SIMPLE(diag=err-diag):  -std 2026 -DERR=1
//DIAGNOSTIC_TEST:SIMPLE(diag=no-diag):   -DERR=0 -std 2025 -allow-glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=no-diag):   -DERR=0 -std 2026 -allow-glsl
//
// See https://github.com/shader-slang/slang/issues/10614


#if (ERR == 0)

static volatile int b;
//warn-diag:    ^^^ use of deprecated modifier
//warn-diag:    ^^^ modifier 'volatile' has been deprecated: to be removed in Slang 2026. Suggested replacement: Atomic<T>

#else

static volatile int b;
//err-diag:     ^^^ use of removed modifier
//err-diag:     ^^^ modifier 'volatile' has been removed from the language. Suggested replacement: Atomic<T>

#endif

[numthreads(1,1,1)]
void computeMain()
{
}

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -entry main -stage fragment -allow-glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS): -target glsl -entry main -stage fragment -allow-glsl -ignore-capabilities
// CHECK_IGNORE_CAPS-NOT: error

float4 main()
/*CHECK:
       ^^^^ unavailable features in entry point
       ^^^^ entrypoint 'main' uses features that are not available in 'fragment' stage for 'glsl' compilation target.
*/
{
    memoryBarrierAtomicCounter();
/*CHECK:
    ^^^^^^^^^^^^^^^^^^^^^^^^^^ see using of 'memoryBarrierAtomicCounter'
*/
    return float4(1.0f);
}

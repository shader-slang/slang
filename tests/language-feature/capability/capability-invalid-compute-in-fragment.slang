//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -entry main -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS): -target glsl -entry main -stage fragment -ignore-capabilities
// non-exhaustive: cascading notes from internal glsl.meta.slang file
// CHECK_IGNORE_CAPS-NOT: error

import glsl;

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

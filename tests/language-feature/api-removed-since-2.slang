//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target cpp -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target cuda -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target glsl -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target hlsl -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target wgsl -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target spirv-asm -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target metal -stage compute -entry main

// This test verifies the [RemovedSince] logic and that, when in
// effect, it overrides [deprecated].
//
// Note: When Slang language version 2026 is removed, please bump up
// the RemovedSince language versions in this test.

import "api-removed-since-2-helper";


// attribute boundaries

[RemovedSince(-2, "Min-1")]
/*CHECK:
 ^^^^^^^^^^^^ 'RemovedSince' argument 'version' is outside allowed range
 ^^^^^^^^^^^^ valid range: [-1, 9999]
*/
struct RemovedStructSinceMinMinus1
{
}

[RemovedSince(-1, "Min")]
struct RemovedStructSinceMin
{
}

[RemovedSince(9999, "Max")]
struct RemovedStructSinceMax
{
}

[RemovedSince(10000, "Max+1")]
/*CHECK:
 ^^^^^^^^^^^^ 'RemovedSince' argument 'version' is outside allowed range
 ^^^^^^^^^^^^ valid range: [-1, 9999]
*/
struct RemovedStructSinceMaxPlus1
{
}

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    var s1 : RemovedStructSince2024;
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^ use of removed declaration
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^ RemovedStructSince2024 has been removed since language version '2024': Use something else

    var s2 : RemovedStructSince2026;
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^ use of removed declaration
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^ RemovedStructSince2026 has been removed since language version '2026': Use something else

    var s3 : RemovedStructSince2026NoDeprecation;
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ use of removed declaration
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ RemovedStructSince2026NoDeprecation has been removed since language version '2026': Use something else

    var s4 : RemovedStructSince2027;
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^ use of deprecated declaration
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^^ RemovedStructSince2027 has been deprecated: This struct is deprecated

}

// Missing a return is accepted with a warning in Slang 2026 and previous versions
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std legacy
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std 2018
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std 2025
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std 2026

// Starting from Slang 202c, a non-void function missing a return is
// always an error
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP_LANG): -std 202c

// Even if the target would also flag a missing return an error, verify that the
// source-based check has priority
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP_LANG): -std 202c -entry computeMain -stage compute -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP_LANG): -std 202c -entry computeMain -stage compute -target hlsl

// Some compilation targets allow missing returns while some do not.
// This test ensures that either errors and warnings are emitted appropriately.

RWStructuredBuffer<uint> out;

//CHECK_SUPP: non-void function does not return in all cases
uint func()
/*CHECK_NOT_SUPP_LANG:
     ^^^^ non-void function must return
     ^^^^ non-void function must return in all cases in Slang 202c.
*/
{
}

[shader("compute")]
void computeMain()
{
    out[0] = func();
}

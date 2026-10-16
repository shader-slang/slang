// Missing a return is accepted with a warning in Slang 2026 and previous versions
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std legacy
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std 2018
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std 2025
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -std 2026

// Starting from Slang 202c, a non-void function missing a return is
// always an error, even if non-essential validation is disabled.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP_LANG): -std 202c
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP_LANG): -std 202c -disable-non-essential-validations

// Even if the target would also flag a missing return as an error, verify that the
// source-based check has priority
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP_LANG): -std 202c -entry computeMain -stage compute -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP_LANG): -std 202c -entry computeMain -stage compute -target hlsl

RWStructuredBuffer<uint> out;

//CHECK_SUPP: non-void function does not return in all cases
uint func()
/*CHECK_NOT_SUPP_LANG:
     ^^^^ non-void function must return in Slang 202c and later
     ^^^^ non-void function must return in all cases in Slang 202c and later
*/
{
}

[shader("compute")]
void computeMain()
{
    out[0] = func();
}

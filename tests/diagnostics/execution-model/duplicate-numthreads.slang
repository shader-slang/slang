// Test that multiple [numthreads] attributes on one entry point are reported.
// Previously the compiler silently kept a single value with no diagnostic; a
// second, redundant or conflicting [numthreads] must now be an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

[numthreads(2, 3, 4)]
[numthreads(1, 5, 9)]
void main()
{
}

// The duplicate [numthreads] is reported as error 31202 (duplicate-modifier).
// A single diagnostic produces two machine-readable rows (title + span), each
// carrying error code E31202, so both are matched here.
//CHECK: E31202
//CHECK: E31202

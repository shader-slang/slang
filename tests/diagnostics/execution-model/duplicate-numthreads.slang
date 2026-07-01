// Test that multiple [numthreads] attributes on one entry point are reported.
// Previously the compiler silently kept a single value with no diagnostic; each
// [numthreads] past the first is now a redundant/conflicting duplicate.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

[numthreads(2, 3, 4)]
[numthreads(5, 6, 7)]
[numthreads(1, 5, 9)]
void main()
{
}

// Each duplicate [numthreads] is reported as error 31202 (duplicate-modifier):
// three attributes yield two duplicate diagnostics, and each diagnostic emits
// two machine-readable rows (title + span) carrying error code E31202.
//CHECK: E31202
//CHECK: E31202
//CHECK: E31202
//CHECK: E31202

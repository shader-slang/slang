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
// two machine-readable rows (a title row and a span row) carrying error code
// E31202. Both rows survive because the title text differs from the span
// message, so they are not de-duplicated -- if those two messages are ever
// unified, this exact four-line count would need to be revisited.
//CHECK: E31202
//CHECK: E31202
//CHECK: E31202
//CHECK: E31202

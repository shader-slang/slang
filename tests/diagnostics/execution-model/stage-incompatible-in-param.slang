//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main

// E39018 is currently emitted twice for a single invalid `in` parameter
// (once during entry-point semantic check and once during parameter binding).
// The test pins both occurrences so a future deduplication of the diagnostic
// is caught as an intentional change.
struct Payload { float3 data; };
[shader("callable")]
void main(in Payload data) {}
//CHECK:             ^^^^ stage does not support in parameters
//CHECK:             ^^^^ the 'callable' stage does not support `in` entry point parameters
//CHECK:             ^^^^ stage does not support in parameters
//CHECK:             ^^^^ the 'callable' stage does not support `in` entry point parameters

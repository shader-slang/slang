//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main

struct Payload { float3 data; };
[shader("callable")]
void main(in Payload data) {}
//CHECK:             ^^^^ stage does not support in parameters
//CHECK:             ^^^^ the 'callable' stage does not support `in` entry point parameters
//CHECK:             ^^^^ stage does not support in parameters
//CHECK:             ^^^^ the 'callable' stage does not support `in` entry point parameters
